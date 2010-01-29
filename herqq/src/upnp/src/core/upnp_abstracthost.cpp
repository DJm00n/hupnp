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

#include "upnp_abstracthost.h"
#include "upnp_abstracthost_p.h"

#include "upnp_device.h"
#include "upnp_service.h"
#include "upnp_device_p.h"
#include "upnp_action_p.h"
#include "upnp_service_p.h"
#include "upnp_deviceinfo.h"

#include "upnp_udn.h"
#include "upnp_resourcetype.h"

#include "../../../utils/src/logger_p.h"
#include "../../../core/include/HExceptions"

#include <QImage>
#include <QMetaType>
#include <QAtomicInt>
#include <QStringList>
#include <QHostAddress>

static bool registerMetaTypes()
{
    static QAtomicInt tester(0);

    if (tester.testAndSetAcquire(0, 1))
    {
        qRegisterMetaType<Herqq::Upnp::HUdn>("Herqq::Upnp::HUdn");
        qRegisterMetaType<Herqq::Upnp::HResourceType>("Herqq::Upnp::HResourceType");
    }

    return true;
}

static bool test = registerMetaTypes();

/*!
 * \defgroup devicehosting Device Hosting
 *
 * \brief This page explains the concept of device hosts, which encapsulate the technical
 * details of UPnP networking.
 *
 * \section notesaboutdesign A few notes about the design
 *
 * The logical core of HUPnP is divided into two major modules; a collection of
 * classes that enable the "hosting" of UPnP device model and the collection of
 * classes that form up the \ref devicemodel. The separation is very distinct. The
 * device hosts provide the technical foundation for the UPnP networking. They
 * encapsulate and implement the protocols the UPnP Device Architecture
 * specification details. The device model, on the other hand, is about the logical
 * structure of the UPnP core concepts, which is clearly independent of the technical details
 * of communication. Because of this, the same device model structure should be usable
 * everywhere, and with HUPnP, it is.
 *
 * HUPnP introduces two types of "hosts".
 * \li The Herqq::Upnp::HDeviceHost is the class
 * that enables a UPnP device to be published for UPnP control points to use.
 * \li The Herqq::Upnp::HControlPoint is the class that enables the discovery and use
 * of UPnP devices that are available on the network.
 *
 * The difference between these two classes is important to notice.
 * You could picture an \c HDeviceHost as a server and an \c HControlPoint as a client.
 * Regardless, they both use and expose an identical \ref devicemodel. If you publish
 * a Herqq::Upnp::HDevice using \c HDeviceHost, you can retrieve the device
 * and use it in process. In addition, the published device is simultaneously usable
 * over the network. Meanwhile an \c HControlPoint, or any UPnP control point
 * for that matter, can see it and use it. When an \c HControlPoint notices
 * a UPnP device on the network, it attempts to build an object model for that device.
 * If the device is hosted by an \c HDeviceHost, the \c HControlPoint will build identical
 * device model compared to that the \c HDeviceHost uses. The fact that some of the calls
 * on the device model retrieved from \c HControlPoint go over the network to the \em real
 * UPnP device is completely abstracted. In other words, if given a pointer to an \c HDevice instance,
 * it is very hard to tell if the HDevice is from an \c HControlPoint or from an \c HDeviceHost.
 * The HUPnP API does not provide that information.
 *
 * \section basicuse Basic use
 *
 * The basic use of a \em device \em host is straightforward. You only need to initialize it
 * and retrieve the devices it exposes. You can also listen for events of
 * devices added and removed.
 *
 * In other words, you could initialize an HDeviceHost like this:
 *
 * \code
 *
 * #include <HDeviceHost>
 *
 * #include "my_hdevice.h" // your code
 *
 * namespace
 * {
 * class Creator
 * {
 * public:
 *     Herqq::Upnp::HDevice* operator()(const Herqq::Upnp::HDeviceInfo& deviceInfo)
 *     {
 *         return new MyHDevice(); // your class derived from HDevice
 *     }
 * };
 * }
 *
 * void example()
 * {
 *     Herqq::Upnp::HDeviceConfiguration deviceConf;
 *     deviceConf.setPathToDeviceDescription("my_hdevice_devicedescription.xml");
 *
 *     Creator mydeviceCreatorFunctor;
 *     deviceConf.setDeviceCreator(mydeviceCreatorFunctor);
 *     // the creator can also be a free function or a member function
 *
 *     Herqq::Upnp::HDeviceHost deviceHost;
 *     deviceHost.init(deviceConf);
 * }
 *
 * \endcode
 *
 * and an HControlPoint like this:
 *
 * \code
 *
 * #include <HControlPoint>
 *
 * #include "my_hdevice.h" // your code
 *
 * namespace
 * {
 * class Creator
 * {
 * public:
 *     Herqq::Upnp::HDevice* operator()(const Herqq::Upnp::HDeviceInfo& deviceInfo)
 *     {
 *         return new MyHDevice(); // your class derived from HDevice
 *     }
 * };
 * }
 *
 * void example()
 * {
 *     Herqq::Upnp::HControlPointConfiguration controlPointConf;
 *
 *     Creator mydeviceCreatorFunctor;
 *     controlPointConf.setDeviceCreator(mydeviceCreatorFunctor);
 *     // the creator can also be a free function or a member function
 *
 *     Herqq::Upnp::HControlPoint controlPoint;
 *     controlPoint.init(controlPointConf);
 * }
 *
 * \endcode
 *
 * With an \c HControlPoint, you do \b not have to provide any configuration nor
 * a device creator. An \c HControlPoint is perfectly usable without them. Nevertheless,
 * you can. In that case, you have the option to decide what \c HDevice types
 * and \c HService types are actually created when the \c HControlPoint builds its object
 * model for a discovered device.
 *
 * In any case, since both \c HControlPoint and \c HDeviceHost are UPnP device hosts
 * derived from Herqq::Upnp::HAbstractHost, their use is similar:
 *
 * \code
 *
 * void MyQObjectDerivedClass::example()
 * {
 *     Herqq::Upnp::HAbstractHost* host = getInitializedControlPoint();
 *     // or, get an initialized HDeviceHost
 *
 *     Herqq::Upnp::HRootDevicePtrListT rootDevices = host->rootDevices();
 *
 *     // you can also use the events the HAbstractHost exposes
 *     bool ok = connect(
 *         host,
 *         SIGNAL(rootDeviceAdded(const Herqq::Upnp::HDeviceInfo&)),
 *         this,
 *         SLOT(rootDeviceAdded(const Herqq::Upnp::HDeviceInfo&)));
 *
 *     Q_ASSERT(ok);
 *
 *     ok = connect(
 *         host,
 *         SIGNAL(rootDeviceRemoved(const Herqq::Upnp::HDeviceInfo&)),
 *         this,
 *         SLOT(rootDeviceRemoved(const Herqq::Upnp::HDeviceInfo&)));
 *
 *     Q_ASSERT(ok);
 * }
 *
 *
 * void MyQObjectDerivedClass::rootDeviceAdded(const Herqq::Upnp::HDeviceInfo& deviceInfo)
 * {
 *     // do something
 * }
 *
 * void MyQObjectDerivedClass::rootDeviceRemoved(const Herqq::Upnp::HDeviceInfo& deviceInfo)
 * {
 *     // do something
 * }
 *
 * \endcode
 *
 * \sa Herqq::Upnp::HAbstractHost, Herqq::Upnp::HDeviceHost
 * \sa Herqq::Upnp::HControlPoint, devicemodel
 */

namespace
{

using namespace Herqq::Upnp;

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

class ScpdUrlTester
{
private:
    QUrl m_url;

public:

    ScpdUrlTester(const QUrl& url) : m_url(url){}
    bool operator()(HServiceController* service)
    {
        Q_ASSERT(service);
        return service->m_service->scpdUrl() == m_url;
    }
};

class ControlUrlTester
{
private:
    QUrl m_url;

public:

    ControlUrlTester(const QUrl& url) : m_url(url){}
    bool operator()(HServiceController* service)
    {
        Q_ASSERT(service);
        return service->m_service->controlUrl() == m_url;
    }
};

class EventUrlTester
{
private:
    QUrl m_url;

public:

    EventUrlTester(const QUrl& url) : m_url(url){}
    bool operator()(HServiceController* service)
    {
        Q_ASSERT(service);
        return service->m_service->eventSubUrl() == m_url;
    }
};


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

QPair<QUrl, QImage> seekIcon(HDeviceController* device, const QString& iconUrl)
{
    HLOG(H_AT, H_FUN);
    Q_ASSERT(device);

    QList<QPair<QUrl, QImage> > icons =
        device->m_device->deviceInfo().icons();

    for (qint32 i = 0; i < icons.size(); ++i)
    {
        if (icons[i].first.toString() == iconUrl)
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

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HAbstractHostPrivate
 ******************************************************************************/
HAbstractHostPrivate::HAbstractHostPrivate(
    const QString& loggingIdentfier) :
        m_http(),
        m_rootDevices(), m_rootDevicesMutex(QMutex::Recursive), q_ptr(0),
        m_threadPool(new QThreadPool()),
        m_loggingIdentifier(loggingIdentfier.toLocal8Bit()),
        m_initializationStatus(0),
        m_sharedActionInvokers()
{
    HLOG(H_AT, H_FUN);
    m_threadPool->setParent(this);
    m_threadPool->setMaxThreadCount(25);
}

HAbstractHostPrivate::~HAbstractHostPrivate()
{
    HLOG(H_AT, H_FUN);
    Q_ASSERT(m_threadPool);

    // cannot go deleting root devices while threads that may be using them
    // are running
    m_threadPool->waitForDone();

    QMutexLocker lock(&m_rootDevicesMutex);

    qDeleteAll(m_rootDevices);
    m_rootDevices.clear();

    delete m_threadPool;

    qDeleteAll(m_sharedActionInvokers);
}

void HAbstractHostPrivate::clear()
{
    HLOG(H_AT, H_FUN);
    Q_ASSERT(m_threadPool);

    doClear();

    Q_ASSERT(m_threadPool);

    // cannot go deleting root devices while threads that may be using them
    // are running
    m_threadPool->waitForDone();

    QMutexLocker lock(&m_rootDevicesMutex);

    qDeleteAll(m_rootDevices);
    m_rootDevices.clear();
}

HDeviceController* HAbstractHostPrivate::searchDeviceByUdn(const HUdn& udn) const
{
    HLOG(H_AT, H_FUN);

    QMutexLocker lock(&m_rootDevicesMutex);

    QList<HDeviceController*> devices;
    seekDevices2(m_rootDevices, MatchFunctor<UdnTester>(udn), devices, true);

    return devices.size() > 0 ? devices[0] : 0;
}

bool HAbstractHostPrivate::searchValidLocation(
    const HDevice* device, const HEndpoint& interface, QUrl* location)
{
    HLOG(H_AT, H_FUN);
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
        if (interface.hostAddress().isInSubnet(QHostAddress::parseSubnet(ci->host().append("/24"))))
        {
            *location = *ci;
            return true;
        }
    }

    return false;
}

HDeviceController* HAbstractHostPrivate::searchRootDevice(const QUrl& arg) const
{
    HLOG(H_AT, H_FUN);

    QString path = arg.path();

    QUuid rootDeviceUdn(path.section('/', 1, 1));
    if (rootDeviceUdn.isNull())
    {
        return 0;
    }

    QMutexLocker lock(&m_rootDevicesMutex);
    foreach (HDeviceController* rootDevice, m_rootDevices)
    {
        if (rootDevice->m_device->deviceInfo().udn() == rootDeviceUdn)
        {
            return rootDevice;
        }
    }

    return 0;
}

QPair<QUrl, QImage> HAbstractHostPrivate::searchIcon(const QString& path) const
{
    HLOG(H_AT, H_FUN);

    QString pathToSearch =
        path.section('/', 2, -1, QString::SectionIncludeLeadingSep);

    if (pathToSearch.isEmpty())
    {
        return QPair<QUrl, QImage>();
    }

    HDeviceController* device = searchRootDevice(QUrl(path));
    if (!device)
    {
        return QPair<QUrl, QImage>();
    }

    return seekIcon(device, pathToSearch);
}

HServiceController* HAbstractHostPrivate::searchServiceByScpdUrl(
    const QUrl& scpdUrl) const
{
    HLOG(H_AT, H_FUN);

    HDeviceController* rootDevice = searchRootDevice(scpdUrl);
    if (!rootDevice)
    {
        return 0;
    }

    QString pathToSearch =
        scpdUrl.path().section('/', 2, -1, QString::SectionIncludeLeadingSep);

    if (pathToSearch.isEmpty())
    {
        return 0;
    }

    QList<HDeviceController*> tmp; tmp.push_back(rootDevice);
    return seekService(tmp, MatchFunctor<ScpdUrlTester>(QUrl(pathToSearch)));
}

HServiceController* HAbstractHostPrivate::searchServiceByControlUrl(
    const QUrl& controlUrl) const
{
    HLOG(H_AT, H_FUN);

    HDeviceController* rootDevice = searchRootDevice(controlUrl);
    if (!rootDevice)
    {
        return 0;
    }

    QString pathToSearch =
        controlUrl.path().section('/', 2, -1, QString::SectionIncludeLeadingSep);

    if (pathToSearch.isEmpty())
    {
        return 0;
    }

    QList<HDeviceController*> tmp; tmp.push_back(rootDevice);
    return seekService(tmp, MatchFunctor<ControlUrlTester>(QUrl(pathToSearch)));
}

HServiceController* HAbstractHostPrivate::searchServiceByEventUrl(
    const QUrl& eventUrl) const
{
    HLOG(H_AT, H_FUN);

    HDeviceController* rootDevice = searchRootDevice(eventUrl);
    if (!rootDevice)
    {
        return 0;
    }

    QString pathToSearch =
        eventUrl.path().section('/', 2, -1, QString::SectionIncludeLeadingSep);

    if (pathToSearch.isEmpty())
    {
        return 0;
    }

    QList<HDeviceController*> tmp; tmp.push_back(rootDevice);
    return seekService(tmp, MatchFunctor<EventUrlTester>(QUrl(pathToSearch)));
}

QList<HDeviceController*> HAbstractHostPrivate::searchDevicesByDeviceType(
    const HResourceType& deviceType, bool exactMatch) const
{
    HLOG(H_AT, H_FUN);

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

QList<HServiceController*> HAbstractHostPrivate::searchServicesByServiceType(
    const HResourceType& serviceType, bool exactMatch) const
{
    HLOG(H_AT, H_FUN);

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

HRootDevicePtrListT HAbstractHostPrivate::rootDevices() const
{
    HLOG(H_AT, H_FUN);
    QMutexLocker lock(&m_rootDevicesMutex);

    HRootDevicePtrListT retVal;
    foreach(HDeviceController* dc, m_rootDevices)
    {
        Q_ASSERT(dc->m_device);
        retVal.push_back(dc->m_device);
    }

    return retVal;
}

void HAbstractHostPrivate::checkDeviceTreeForUdnConflicts(
    HDeviceController* device) const
{
   HLOG(H_AT, H_FUN);

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

void HAbstractHostPrivate::addRootDevice(HDeviceController* root)
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

    emit q_ptr->rootDeviceAdded(root->m_device->deviceInfo());
}

void HAbstractHostPrivate::removeRootDevice(HDeviceController* root)
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

    emit q_ptr->rootDeviceRemoved(devInfo);
}

/*******************************************************************************
 * HAbstractHost
 ******************************************************************************/
HAbstractHost::HAbstractHost(HAbstractHostPrivate& dd, QObject* parent) :
    QObject(parent), h_ptr(&dd)
{
   HLOG(H_AT, H_FUN);
   h_ptr->setParent(this);
   h_ptr->q_ptr = this;
}

HAbstractHost::~HAbstractHost()
{
    HLOG(H_AT, H_FUN);
    delete h_ptr;
}

bool HAbstractHost::isStarted() const
{
    HLOG(H_AT, H_FUN);
    return h_ptr->state() == HAbstractHostPrivate::Initialized;
}

HRootDevicePtrListT HAbstractHost::rootDevices() const
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (!isStarted())
    {
        HLOG_WARN(QObject::tr("The host is not started"));
        return HRootDevicePtrListT();
    }

    return h_ptr->rootDevices();
}

HRootDevicePtrT HAbstractHost::rootDevice(const HUdn& udn) const
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (!isStarted())
    {
        HLOG_WARN(QObject::tr("The host is not started"));
        return HRootDevicePtrT();
    }

    return h_ptr->searchDeviceByUdn(udn)->m_device;
}


}
}
