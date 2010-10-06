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

#include "habstracthost_p.h"
#include "../dataelements/hudn.h"
#include "../dataelements/hresourcetype.h"

#include "../../utils/hlogger_p.h"

#include <QtCore/QMetaType>

static bool registerMetaTypes()
{
    qRegisterMetaType<Herqq::Upnp::HUdn>("Herqq::Upnp::HUdn");
    qRegisterMetaType<Herqq::Upnp::HResourceType>("Herqq::Upnp::HResourceType");

    return true;
}

static bool test = registerMetaTypes();

/*!
 * \defgroup hupnp_devicehosting Device Hosting
 * \ingroup hupnp_core
 *
 * \brief This page explains the concept of device hosts, which encapsulate the
 * technical details of UPnP networking.
 *
 * \section notesaboutdesign A few notes about the design
 *
 * The logical core of HUPnP is divided into two major modules; a collection of
 * classes that enable the \e hosting of UPnP device model and the collection of
 * classes that form up the \ref hupnp_devicemodel. The separation is very distinct. The
 * device hosts provide the technical foundation for the UPnP networking. They
 * encapsulate and implement the protocols the UPnP Device Architecture
 * specification details. The device model, on the other hand, is about the logical
 * structure of the UPnP core concepts, which is clearly independent of the
 * technical details of communication. Because of this HUPnP uses the same
 * device model both at the server and client side.
 *
 * HUPnP introduces two types of \e hosts.
 * \li The Herqq::Upnp::HDeviceHost is the class
 * that enables a UPnP device to be published for UPnP control points to use.
 * \li The Herqq::Upnp::HControlPoint is the class that enables the discovery
 * and use of UPnP devices that are available on the network.
 *
 * The difference between these two classes is important to notice.
 * You could picture an \c HDeviceHost as a server and an \c HControlPoint as a
 * client. The \c HDeviceHost \e publishes instances of \c HDevice for
 * UPnP control points to use and the \c HControlPoint \e uses instances of
 * \c HDevice to communicate with UPnP devices. But as implied,
 * they both use and expose the HUPnP \ref hupnp_devicemodel.
 *
 * \note While a Herqq::Upnp::HDevice published by a \c HDeviceHost is always
 * usable by UPnP control points over the network, the same device can also
 * be accesed and used simultaneously in process.
 * See Herqq::Upnp::HDeviceHost for more information.
 *
 * When an \c HControlPoint discovers a UPnP device on the network
 * it attempts to build an object model for that device.
 * If the device is hosted by an \c HDeviceHost the \c HControlPoint will build
 * almost identical device model compared to the model the \c HDeviceHost uses.
 * The fact that some of the calls to the objects of the device model retrieved
 * from \c HControlPoint go over the network to the \em real
 * UPnP device is completely abstracted.
 *
 * \section basicuse Basic use
 *
 * The basic use of a \em device \em host is straightforward.
 * You only need to initialize it by providing information that enables one or
 * more UPnP devices to be created and hosted. Note, you can access and use the \c HDevice
 * instances the host manages.
 *
 * In other words, you could create and initialize an \c HDeviceHost like this:
 *
 * \code
 *
 * #include <HUpnpCore/HDeviceHost>
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
 *
 * Herqq::Upnp::HDeviceHost* createDeviceHost()
 * {
 *     Herqq::Upnp::HDeviceConfiguration deviceConf;
 *     deviceConf.setPathToDeviceDescription("my_hdevice_devicedescription.xml");
 *     // this is the device description for our custom UPnP device type
 *     // the device host uses this file to build the device tree.
 *
 *     Creator mydeviceCreatorFunctor;
 *     deviceConf.setDeviceCreator(mydeviceCreatorFunctor);
 *     // this functor is used to create types that represent UPnP devices found
 *     // in the device description.
 *     //
 *     // note, the creator can also be a normal or a member function
 *
 *     Herqq::Upnp::HDeviceHost* deviceHost = new HDeviceHost();
 *     if (!deviceHost->init(deviceConf))
 *     {
 *         // the initialization failed. perhaps something should be done?
 *         // call error() to check the type of the error and errorDescription()
 *         // to get a human-readable description of the error.
 *     }
 *
 *     return deviceHost;
 * }
 * }
 *
 * \endcode
 *
 * and an \c HControlPoint like this:
 *
 * \code
 *
 * #include <HUpnpCore/HControlPoint>
 *
 * #include "myhdeviceproxy.h" // your code
 *
 * namespace
 * {
 * class Creator
 * {
 * public:
 *     Herqq::Upnp::HDeviceProxy* operator()(const Herqq::Upnp::HDeviceInfo& deviceInfo)
 *     {
 *         return new MyHDeviceProxy(); // your class derived from HDeviceProxy
 *     }
 * };
 *
 * Herqq::Upnp::HControlPoint* createControlPoint()
 * {
 *     Herqq::Upnp::HControlPointConfiguration controlPointConf;
 *
 *     Creator mydeviceCreatorFunctor;
 *     controlPointConf.setDeviceCreator(mydeviceCreatorFunctor);
 *     // This functor can be used to create HDeviceProxy types for HControlPoint when
 *     // it discovers UPnP devices on the network. This can be useful if you want
 *     // to use your custom HDeviceProxy types that provide some custom
 *     // functionality / API perhaps.
 *     //
 *     // note, the creator can also be a normal or a member function.
 *
 *     Herqq::Upnp::HControlPoint* controlPoint =
 *         new HControlPoint(controlPointConf);
 *     // note, the control point configuration is optional and usually you don't
 *     // need to provide one.
 *
 *     if (!controlPoint->init())
 *     {
 *         // the initialization failed. perhaps something should be done?
 *         // call error() to check the type of the error and errorDescription()
 *         // to get a human-readable description of the error.
 *     }
 *
 *     return controlPoint;
 * }
 * }
 *
 * \endcode
 *
 * You do \b not have to provide any configuration to an \c HControlPoint.
 * An \c HControlPoint is perfectly usable without it, but you can use
 * a configuration to modify the behavior of it.
 * Furthermore, by providing configuration you have the option to decide what
 * \c HDeviceProxy types and \c HServiceProxy types are actually created when an
 * \c HControlPoint instance builds its object model for a discovered device.
 *
 * \sa Herqq::Upnp::HDeviceHost, Herqq::Upnp::HControlPoint, hupnp_devicemodel
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
        m_threadPool(new HThreadPool()),
        m_initializationStatus(0),
        m_lastErrorDescription()
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
    delete m_threadPool;

    m_deviceStorage.reset(0);
}

void HAbstractHostPrivate::clear()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(m_threadPool);

    doClear();

    Q_ASSERT(m_threadPool);

    // cannot go deleting root devices while threads that may be using them
    // are running
    m_threadPool->shutdown();

    m_deviceStorage->clear();
}

}
}
