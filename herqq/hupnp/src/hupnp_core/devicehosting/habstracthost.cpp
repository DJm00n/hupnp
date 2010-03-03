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

#include "habstracthost.h"
#include "habstracthost_p.h"

#include "./../devicemodel/hdevice.h"
#include "./../devicemodel/hservice.h"
#include "./../devicemodel/hdevice_p.h"
#include "./../devicemodel/haction_p.h"
#include "./../devicemodel/hservice_p.h"
#include "./../dataelements/hdeviceinfo.h"

#include "./../dataelements/hudn.h"
#include "./../dataelements/hresourcetype.h"

#include "./../../utils/hlogger_p.h"

#include <QImage>
#include <QMetaType>
#include <QStringList>
#include <QHostAddress>

static bool registerMetaTypes()
{
    qRegisterMetaType<Herqq::Upnp::HUdn>("Herqq::Upnp::HUdn");
    qRegisterMetaType<Herqq::Upnp::HResourceType>("Herqq::Upnp::HResourceType");

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

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HAbstractHostPrivate
 ******************************************************************************/
HAbstractHostPrivate::HAbstractHostPrivate(
    const QString& loggingIdentfier) :
        m_loggingIdentifier(loggingIdentfier.toLocal8Bit()),
        m_http(),
        m_deviceStorage(0),
        q_ptr(0),
        m_threadPool(new QThreadPool()),
        m_initializationStatus(0),
        m_sharedActionInvokers()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    m_deviceStorage.reset(new DeviceStorage(m_loggingIdentifier));

    m_threadPool->setParent(this);
    m_threadPool->setMaxThreadCount(10);
}

HAbstractHostPrivate::~HAbstractHostPrivate()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(m_threadPool);

    // cannot go deleting root devices while threads that may be using them
    // are running

    m_threadPool->waitForDone();
    delete m_threadPool;

    m_deviceStorage.reset(0);

    qDeleteAll(m_sharedActionInvokers);
}

void HAbstractHostPrivate::addRootDevice(HDeviceController* root)
{
    m_deviceStorage->addRootDevice(root);
    emit q_ptr->rootDeviceAdded(root->m_device->deviceInfo());
}

void HAbstractHostPrivate::removeRootDevice(HDeviceController* root)
{
    HDeviceInfo devInfo = root->m_device->deviceInfo();
    m_deviceStorage->removeRootDevice(root);
    emit q_ptr->rootDeviceRemoved(devInfo);
}

void HAbstractHostPrivate::clear()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(m_threadPool);

    doClear();

    Q_ASSERT(m_threadPool);

    // cannot go deleting root devices while threads that may be using them
    // are running
    m_threadPool->waitForDone();

    m_deviceStorage->clear();
}

/*******************************************************************************
 * HAbstractHost
 ******************************************************************************/
HAbstractHost::HAbstractHost(HAbstractHostPrivate& dd, QObject* parent) :
    QObject(parent), h_ptr(&dd)
{
   h_ptr->setParent(this);
   h_ptr->q_ptr = this;
}

HAbstractHost::~HAbstractHost()
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    delete h_ptr;
}

bool HAbstractHost::isStarted() const
{
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

    return h_ptr->m_deviceStorage->rootDevices();
}

HRootDevicePtrT HAbstractHost::rootDevice(const HUdn& udn) const
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (!isStarted())
    {
        HLOG_WARN(QObject::tr("The host is not started"));
        return HRootDevicePtrT();
    }

    return h_ptr->m_deviceStorage->searchDeviceByUdn(udn)->m_device;
}


}
}
