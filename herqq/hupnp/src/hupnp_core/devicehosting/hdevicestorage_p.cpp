/*
 *  Copyright (C) 2010 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP (HUPnP) library.
 *
 *  Herqq UPnP is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Herqq UPnP. If not, see <http://www.gnu.org/licenses/>.
 */

#include "hdevicestorage_p.h"

#include "../dataelements/hudn.h"
#include "../dataelements/hdeviceinfo.h"

#include "../socket/hendpoint.h"
#include "../../utils/hlogger_p.h"
#include "../../utils/hexceptions_p.h"

#include "../devicemodel/haction_p.h"
#include "../devicemodel/hdevice_p.h"
#include "../devicemodel/hservice_p.h"

#include <QUrl>
#include <QPair>
#include <QImage>
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
    inline bool operator()(HDeviceController* device) const
    {
        return m_t(device);
    }

    inline bool operator()(HServiceController* service) const
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
    inline bool operator()(HServiceController* service) const
    {
        Q_ASSERT(service);
        //return service->m_service->scpdUrl() == m_url;
        return compareUrls(m_url, service->m_service->info().scpdUrl());
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
    inline bool operator()(HServiceController* service) const
    {
        Q_ASSERT(service);
        //return service->m_service->controlUrl() == m_url;
        return compareUrls(m_url, service->m_service->info().controlUrl());
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
    inline bool operator()(HServiceController* service) const
    {
        Q_ASSERT(service);
        //return service->m_service->eventSubUrl() == m_url;
        return compareUrls(m_url, service->m_service->info().eventSubUrl());
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
    inline bool operator()(HDeviceController* device) const
    {
        Q_ASSERT(device);
        return device->m_device->info().udn() == m_udn;
    }
};

//
//
//
class ResourceTypeTester
{
private:

    HResourceType m_resourceType;
    HResourceType::VersionMatch m_versionMatch;

public:
    ResourceTypeTester(
        const HResourceType& resType, HResourceType::VersionMatch vm) :
            m_resourceType(resType), m_versionMatch(vm)
    {
    }

    bool test(const HResourceType& resType) const
    {
        return m_resourceType.compare(resType, m_versionMatch);
    }

    bool operator()(HDeviceController* device) const
    {
        Q_ASSERT(device);
        return test(device->m_device->info().deviceType());
    }

    bool operator()(HServiceController* service) const
    {
        Q_ASSERT(service);
        return test(service->m_service->info().serviceType());
    }
};

template<typename T>
void seekDevices(
    HDeviceController* device, const MatchFunctor<T>& mf,
    QList<HDeviceController*>& foundDevices, HDevice::TargetDeviceType dts)
{
    Q_ASSERT(device);

    if (dts == HDevice::RootDevices && device->parentDevice())
    {
        return;
    }

    if (mf(device))
    {
        foundDevices.push_back(device);
    }

    const QList<HDeviceController*>* devices = device->embeddedDevices();
    foreach(HDeviceController* device, *devices)
    {
        seekDevices(device, mf, foundDevices, dts);
    }
}

//
//
//
template<typename T>
void seekDevices(
    const QList<HDeviceController*>& devices, const MatchFunctor<T>& mf,
    QList<HDeviceController*>& foundDevices, HDevice::TargetDeviceType dts)
{
    foreach(HDeviceController* device, devices)
    {
        seekDevices(device, mf, foundDevices, dts);
    }
}

//
//
//
template<typename T>
HServiceController* seekService(
    const QList<HDeviceController*>& devices, const MatchFunctor<T>& mf)
{
    foreach(HDeviceController* device, devices)
    {
        const QList<HServiceController*>* services = device->services();
        foreach(HServiceController* service, *services)
        {
            if (mf(service))
            {
                return service;
            }
        }

        HServiceController* service =
            seekService(*device->embeddedDevices(), mf);

        if (service)
        {
            return service;
        }
    }

    return 0;
}

//
//
//
template<typename T>
void seekServices(
    const QList<HDeviceController*>& devices, const MatchFunctor<T>& mf,
    QList<HServiceController*>& foundServices,
    bool rootDevicesOnly)
{
    foreach(HDeviceController* device, devices)
    {
        if (rootDevicesOnly && device->m_device->parentDevice())
        {
            continue;
        }

        const QList<HServiceController*>* services = device->services();
        foreach(HServiceController* service, *services)
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

        seekServices(*device->embeddedDevices(), mf, foundServices, rootDevicesOnly);
    }
}

template<typename T>
void traverse(
    const QList<HDeviceController*>& devices, T& mf)
{
    foreach(HDeviceController* device, devices)
    {
        mf(device);

        const QList<HServiceController*>* services = device->services();
        foreach(HServiceController* service, *services)
        {
            mf(service);

            QList<HActionController*> actions = service->actions();
            foreach(HActionController* action, actions)
            {
                mf(action);
            }

            /*QList<HStateVariableController*> stateVariables =
                service->stateVariables();

            foreach(HStateVariableController* stateVariable, stateVariables)
            {
                mf(stateVariable);
            }*/
        }

        traverse(*device->embeddedDevices(), mf);
    }
}
}

/*******************************************************************************
 * DeviceStorage
 ******************************************************************************/
DeviceStorage::DeviceStorage(
    const QByteArray& loggingIdentifier) :
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
    QMutexLocker locker(&m_rootDevicesMutex);

    qDeleteAll(m_rootDevices);
    m_rootDevices.clear();
}

HDeviceController* DeviceStorage::searchDeviceByUdn(
    const HUdn& udn, HDevice::TargetDeviceType dts) const
{
    QMutexLocker lock(&m_rootDevicesMutex);

    QList<HDeviceController*> devices;
    seekDevices(m_rootDevices, MatchFunctor<UdnTester>(udn), devices, dts);

    return devices.size() > 0 ? devices[0] : 0;
}

bool DeviceStorage::searchValidLocation(
    const HDevice* device, const HEndpoint& interface, QUrl* location)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(device);

    QList<QUrl> locations = device->locations();
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
    const HResourceType& deviceType, HResourceType::VersionMatch vm, 
    HDevice::TargetDeviceType dts) const
{
    QMutexLocker locker(&m_rootDevicesMutex);

    QList<HDeviceController*> retVal;

    seekDevices(
        m_rootDevices,
        MatchFunctor<ResourceTypeTester>(ResourceTypeTester(deviceType, vm)),
        retVal,
        dts);

    return retVal;
}

QList<HServiceController*> DeviceStorage::searchServicesByServiceType(
    const HResourceType& serviceType, HResourceType::VersionMatch vm) const
{
    QMutexLocker lock(&m_rootDevicesMutex);

    QList<HServiceController*> retVal;

    seekServices(
        m_rootDevices,
        MatchFunctor<ResourceTypeTester>(ResourceTypeTester(serviceType, vm)),
        retVal,
        false);

    return retVal;
}

void DeviceStorage::checkDeviceTreeForUdnConflicts(
    HDeviceController* device) const
{
   HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    if (searchDeviceByUdn(device->m_device->info().udn()))
    {
        throw HOperationFailedException(
            QString("Cannot host multiple devices with the same UDN [%1]").arg(
                device->m_device->info().udn().toSimpleUuid()));
    }

    const QList<HDeviceController*>* devices = device->embeddedDevices();
    foreach(HDeviceController* embeddeDevice, *devices)
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

    QMutexLocker locker(&m_rootDevicesMutex);

    checkDeviceTreeForUdnConflicts(root);
    m_rootDevices.push_back(root);

    HLOG_DBG(QString("New root device [%1] added. Current device count is %2").arg(
        root->m_device->info().friendlyName(), QString::number(m_rootDevices.size())));
}

bool DeviceStorage::removeRootDevice(HDeviceController* root)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(root);
    Q_ASSERT(!root->m_device->parentDevice());

    QMutexLocker locker(&m_rootDevicesMutex);

    HDeviceInfo devInfo = root->m_device->info();

    bool ok = m_rootDevices.removeOne(root);
    if (!ok)
    {
        HLOG_WARN(QString("Device [%1] was not found.").arg(
            devInfo.friendlyName()));

        return false;
    }

    delete root;

    HLOG_DBG(QString("Root device [%1] removed. Current device count is %2").arg(
        devInfo.friendlyName(), QString::number(m_rootDevices.size())));

    return true;
}

QPair<QUrl, QImage> DeviceStorage::seekIcon(
    HDeviceController* device, const QString& iconUrl)
{
    Q_ASSERT(device);

    QList<QPair<QUrl, QImage> > icons =
        device->m_device->info().icons();

    for (qint32 i = 0; i < icons.size(); ++i)
    {
        if (compareUrls(icons[i].first, iconUrl))
        {
            return icons[i];
        }
    }

    const QList<HDeviceController*>* devices = device->embeddedDevices();
    foreach(HDeviceController* device, *devices)
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
    QList<HDeviceController*> tmp; tmp.push_back(device);
    return seekService(tmp, MatchFunctor<ScpdUrlTester>(scpdUrl));
}

HServiceController* DeviceStorage::searchServiceByScpdUrl(
    const QUrl& scpdUrl) const
{
    QMutexLocker locker(&m_rootDevicesMutex);
    return seekService(m_rootDevices, MatchFunctor<ScpdUrlTester>(scpdUrl));
}

HServiceController* DeviceStorage::searchServiceByControlUrl(
    HDeviceController* device, const QUrl& controlUrl) const
{
    QList<HDeviceController*> tmp; tmp.push_back(device);
    return seekService(tmp, MatchFunctor<ControlUrlTester>(controlUrl));
}

HServiceController* DeviceStorage::searchServiceByControlUrl(
    const QUrl& controlUrl) const
{
    QMutexLocker locker(&m_rootDevicesMutex);
    return seekService(m_rootDevices, MatchFunctor<ControlUrlTester>(controlUrl));
}

HServiceController* DeviceStorage::searchServiceByEventUrl(
    HDeviceController* device, const QUrl& eventUrl) const
{
    QList<HDeviceController*> tmp; tmp.push_back(device);
    return seekService(tmp, MatchFunctor<EventUrlTester>(eventUrl));
}

HServiceController* DeviceStorage::searchServiceByEventUrl(
    const QUrl& eventUrl) const
{
    QMutexLocker locker(&m_rootDevicesMutex);
    return seekService(m_rootDevices, MatchFunctor<EventUrlTester>(eventUrl));
}

HDevices DeviceStorage::rootDevices() const
{
    QMutexLocker lock(&m_rootDevicesMutex);

    QList<HDevice*> retVal;
    foreach(HDeviceController* dc, m_rootDevices)
    {
        Q_ASSERT(dc->m_device);
        retVal.push_back(dc->m_device);
    }

    return retVal;
}

HDeviceProxies DeviceStorage::rootDeviceProxies() const
{
    QMutexLocker lock(&m_rootDevicesMutex);

    QList<HDeviceProxy*> retVal;
    foreach(HDeviceController* dc, m_rootDevices)
    {
        Q_ASSERT(dc->m_device);
        retVal.push_back(dc->m_deviceProxy);
    }

    return retVal;
}

QList<HDeviceController*> DeviceStorage::rootDeviceControllers() const
{
    QMutexLocker lock(&m_rootDevicesMutex);
    return m_rootDevices;
}

}
}
