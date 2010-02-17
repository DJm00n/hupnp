/*
 *  Copyright (C) 2010 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP (HUPnP) library.
 *
 *  Herqq UPnP is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Herqq UPnP. If not, see <http://www.gnu.org/licenses/>.
 */

#include "hdevicestorage_p.h"

#include "./../dataelements/hudn.h"
#include "./../../utils/hlogger_p.h"
#include "./../general/hupnp_global_p.h"
#include "./../dataelements/hdeviceinfo.h"

#include <QUuid>
#include <QHostAddress>

namespace Herqq
{

namespace Upnp
{

namespace
{

//
//
//
template<typename T>
class MatchFunctor
{
private:
    T m_t;

public:

    MatchFunctor(const T& t) : m_t(t){}
    bool operator()(HDeviceController* device)
    {
        return m_t(device);
    }

    bool operator()(HServiceController* service)
    {
        return m_t(service);
    }
};

bool compareUrls(const QUrl& u1, const QUrl& u2)
{
    QString u1Str = extractRequestPart(u1);
    QString u2Str = extractRequestPart(u2);

    if (u1Str.startsWith('/')) { u1Str.remove(0, 1); }
    if (u2Str.startsWith('/')) { u2Str.remove(0, 1); }

    return u1Str == u2Str;
}

//
//
//
class ScpdUrlTester
{
private:
    QUrl m_url;

public:

    ScpdUrlTester(const QUrl& url) : m_url(url){}
    bool operator()(HServiceController* service)
    {
        Q_ASSERT(service);
        //return service->m_service->scpdUrl() == m_url;
        return compareUrls(m_url, service->m_service->scpdUrl());
    }
};

//
//
//
class ControlUrlTester
{
private:
    QUrl m_url;

public:

    ControlUrlTester(const QUrl& url) : m_url(url){}
    bool operator()(HServiceController* service)
    {
        Q_ASSERT(service);
        //return service->m_service->controlUrl() == m_url;
        return compareUrls(m_url, service->m_service->controlUrl());
    }
};

//
//
//
class EventUrlTester
{
private:
    QUrl m_url;

public:

    EventUrlTester(const QUrl& url) : m_url(url){}
    bool operator()(HServiceController* service)
    {
        Q_ASSERT(service);
        //return service->m_service->eventSubUrl() == m_url;
        return compareUrls(m_url, service->m_service->eventSubUrl());
    }
};

//
//
//
class UdnTester
{
private:

    HUdn m_udn;

public:

    UdnTester(const HUdn& udn) : m_udn(udn){}
    bool operator()(HDeviceController* device)
    {
        Q_ASSERT(device);
        return device->m_device->deviceInfo().udn() == m_udn;
    }
};

//
//
//
template<bool ExactMatch>
class ResourceTypeTester
{
private:

    HResourceType m_resourceType;

public:
    ResourceTypeTester(const HResourceType& resType) :
        m_resourceType(resType)
    {
    }

    bool test(const HResourceType& resType)
    {
        // either an exact match is searched, or the searched device type's version
        // is smaller or equals to the version number of the stored type.
        return ExactMatch ? resType == m_resourceType :
            resType.resourceUrn()     == m_resourceType.resourceUrn() &&
            resType.type()            == m_resourceType.type() &&
            resType.typeSuffix(false) == m_resourceType.typeSuffix(false) &&
            m_resourceType.version() <= resType.version();
    }

    bool operator()(HDeviceController* device)
    {
        Q_ASSERT(device);
        return test(device->m_device->deviceInfo().deviceType());
    }

    bool operator()(HServiceController* service)
    {
        Q_ASSERT(service);
        return test(service->m_service->serviceType());
    }
};

//
//
//
template<typename T>
void seekDevices(
    HDeviceController* device, MatchFunctor<T> mf,
    QList<HDeviceController*>& foundDevices,
    bool rootOnly = false)
{
    HLOG(H_AT, H_FUN);
    Q_ASSERT(device);

    if (rootOnly && device->parentDevice())
    {
        return;
    }

    if (mf(device))
    {
        foundDevices.push_back(device);
    }

    QList<HDeviceController*> devices = device->embeddedDevices();
    foreach(HDeviceController* device, devices)
    {
        seekDevices(device, mf, foundDevices);
    }
}

//
//
//
template<typename T>
void seekDevices2(
    QList<HDeviceController*> devices, MatchFunctor<T> mf,
    QList<HDeviceController*>& foundDevices, bool rootOnly)
{
    HLOG(H_AT, H_FUN);

    foreach(HDeviceController* device, devices)
    {
        seekDevices(device, mf, foundDevices, rootOnly);
    }
}

//
//
//
template<typename T>
HServiceController* seekService(
    QList<HDeviceController*> devices, MatchFunctor<T> mf)
{
    HLOG(H_AT, H_FUN);

    foreach(HDeviceController* device, devices)
    {
        QList<HServiceController*> services = device->services();
        foreach(HServiceController* service, services)
        {
            if (mf(service))
            {
                return service;
            }
        }

        HServiceController* service =
            seekService(device->embeddedDevices(), mf);

        if (service)
        {
            return service;
        }
    }

    return 0;
}

template<typename T>
void seekServices(
    QList<HDeviceController*> devices, MatchFunctor<T> mf,
    QList<HServiceController*>& foundServices,
    bool rootDevicesOnly)
{
    HLOG(H_AT, H_FUN);

    foreach(HDeviceController* device, devices)
    {
        if (rootDevicesOnly && device->m_device->parentDevice())
        {
            continue;
        }

        QList<HServiceController*> services = device->services();
        foreach(HServiceController* service, services)
        {
            if (mf(service))
            {
                foundServices.push_back(service);
            }
        }

        if (rootDevicesOnly)
        {
            continue;
        }

        seekServices(device->embeddedDevices(), mf, foundServices, rootDevicesOnly);
    }
}
}

DeviceStorage::DeviceStorage(const QByteArray& loggingIdentifier) :
    m_loggingIdentifier(loggingIdentifier), m_rootDevices(),
    m_rootDevicesMutex(QMutex::Recursive)
{
}

DeviceStorage::~DeviceStorage()
{
    clear();
}

void DeviceStorage::clear()
{
    QMutexLocker lock(&m_rootDevicesMutex);
    qDeleteAll(m_rootDevices);
    m_rootDevices.clear();
}

HDeviceController* DeviceStorage::searchDeviceByUdn(const HUdn& udn) const
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QMutexLocker lock(&m_rootDevicesMutex);

    QList<HDeviceController*> devices;
    seekDevices2(m_rootDevices, MatchFunctor<UdnTester>(udn), devices, true);

    return devices.size() > 0 ? devices[0] : 0;
}

bool DeviceStorage::searchValidLocation(
    const HDevice* device, const HEndpoint& interface, QUrl* location)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(device);

    QList<QUrl> locations = device->locations(true);
    QList<QUrl>::const_iterator ci = locations.constBegin();
    for(; ci != locations.constEnd(); ++ci)
    {
        /*if (QHostAddress(ci->host()) == interface.hostAddress())
        {
            *location = *ci;
            return true;
        }*/
// TODO
        if (interface.hostAddress().isInSubnet(
            QHostAddress::parseSubnet(ci->host().append("/24"))))
        {
            *location = *ci;
            return true;
        }
    }

    return false;
}

QList<HDeviceController*> DeviceStorage::searchDevicesByDeviceType(
    const HResourceType& deviceType, bool exactMatch) const
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QMutexLocker tmp(&m_rootDevicesMutex);

    QList<HDeviceController*> retVal;

    if (exactMatch)
    {
        seekDevices2(
            m_rootDevices,
            MatchFunctor<ResourceTypeTester<true> >(deviceType),
            retVal,
            false);
    }
    else
    {
        seekDevices2(
            m_rootDevices,
            MatchFunctor<ResourceTypeTester<false> >(deviceType),
            retVal,
            false);
    }

    return retVal;
}

QList<HServiceController*> DeviceStorage::searchServicesByServiceType(
    const HResourceType& serviceType, bool exactMatch) const
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QMutexLocker lock(&m_rootDevicesMutex);

    QList<HServiceController*> retVal;

    if (exactMatch)
    {
        seekServices(
            m_rootDevices,
            MatchFunctor<ResourceTypeTester<true> >(serviceType),
            retVal,
            false);
    }
    else
    {
        seekServices(
            m_rootDevices,
            MatchFunctor<ResourceTypeTester<false> >(serviceType),
            retVal,
            false);
    }

    return retVal;
}

void DeviceStorage::checkDeviceTreeForUdnConflicts(
    HDeviceController* device) const
{
   HLOG2(H_AT, H_FUN, m_loggingIdentifier);

   if (searchDeviceByUdn(device->m_device->deviceInfo().udn()))
    {
        throw HOperationFailedException(
            QObject::tr("Cannot host multiple devices with the same UDN [%1]").arg(
                device->m_device->deviceInfo().udn().toSimpleUuid()));
    }

   QList<HDeviceController*> devices = device->embeddedDevices();
    foreach(HDeviceController* embeddeDevice, devices)
    {
        checkDeviceTreeForUdnConflicts(embeddeDevice);
    }
}

void DeviceStorage::addRootDevice(HDeviceController* root)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(root);
    Q_ASSERT(root->m_device);
    Q_ASSERT(!root->m_device->parentDevice());

    QMutexLocker lock(&m_rootDevicesMutex);

    checkDeviceTreeForUdnConflicts(root);
    m_rootDevices.push_back(root);

    lock.unlock();

    HLOG_DBG(QObject::tr("New root device [%1] added. Current device count is %2").arg(
        root->m_device->deviceInfo().friendlyName(), QString::number(m_rootDevices.size())));
}

void DeviceStorage::removeRootDevice(HDeviceController* root)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(root);
    Q_ASSERT(!root->m_device->parentDevice());

    QMutexLocker lock(&m_rootDevicesMutex);

    bool ok = m_rootDevices.removeOne(root);
    Q_ASSERT(ok); Q_UNUSED(ok)

    HDeviceInfo devInfo = root->m_device->deviceInfo();

    delete root;
    // after this the device controller is gone, but the device and service
    // objects still exist. As mentioned, however, they all are in "disposed" state.
    // The objects will be deleted when the reference counts of their wrapping
    // smart pointers drop to zero.

    lock.unlock();

    HLOG_DBG(QObject::tr("Root device [%1] removed. Current device count is %2").arg(
        devInfo.friendlyName(), QString::number(m_rootDevices.size())));
}

QPair<QUrl, QImage> DeviceStorage::seekIcon(
    HDeviceController* device, const QString& iconUrl)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(device);

    QList<QPair<QUrl, QImage> > icons =
        device->m_device->deviceInfo().icons();

    for (qint32 i = 0; i < icons.size(); ++i)
    {
        if (compareUrls(icons[i].first, iconUrl))
        {
            return icons[i];
        }
    }

    QList<HDeviceController*> devices = device->embeddedDevices();
    foreach(HDeviceController* device, devices)
    {
        QPair<QUrl, QImage> icon = seekIcon(device, iconUrl);
        if (icon != QPair<QUrl, QImage>())
        {
            return icon;
        }
    }

    return QPair<QUrl, QImage>();
}

HServiceController* DeviceStorage::searchServiceByScpdUrl(
    HDeviceController* device, const QUrl& scpdUrl) const
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QList<HDeviceController*> tmp; tmp.push_back(device);
    return seekService(tmp, MatchFunctor<ScpdUrlTester>(scpdUrl));
}

HServiceController* DeviceStorage::searchServiceByControlUrl(
    HDeviceController* device, const QUrl& controlUrl) const
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QList<HDeviceController*> tmp; tmp.push_back(device);
    return seekService(tmp, MatchFunctor<ControlUrlTester>(controlUrl));
}

HServiceController* DeviceStorage::searchServiceByEventUrl(
    HDeviceController* device, const QUrl& eventUrl) const
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    QList<HDeviceController*> tmp; tmp.push_back(device);
    return seekService(tmp, MatchFunctor<EventUrlTester>(eventUrl));
}

HRootDevicePtrListT DeviceStorage::rootDevices() const
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    QMutexLocker lock(&m_rootDevicesMutex);

    HRootDevicePtrListT retVal;
    foreach(HDeviceController* dc, m_rootDevices)
    {
        Q_ASSERT(dc->m_device);
        retVal.push_back(dc->m_device);
    }

    return retVal;
}

QList<HDeviceController*> DeviceStorage::rootDeviceControllers() const
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    QMutexLocker lock(&m_rootDevicesMutex);

    return m_rootDevices;
}

}
}
