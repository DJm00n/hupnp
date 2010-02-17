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

#ifndef HDEVICEHOST_H_
#define HDEVICEHOST_H_

#include "./../habstracthost.h"

namespace Herqq
{

namespace Upnp
{

class HDeviceHostPrivate;

/*!
 * A class for creating and hosting HDevices on the network.
 *
 * \headerfile hdevicehost.h HDeviceHost
 *
 * \ingroup devicehosting
 *
 * As the name implies, this is the class in the %Herqq UPnP library
 * used to expose UPnP devices to UPnP control points.
 * The class \e hosts instances of HDevice, which means that the class takes
 * care of all of the UPnP mechanics detaching the HDevice from it. This separation
 * leaves the HDevice to model the UPnP device structure and to focus on the functionality
 * of the specific device type. This is what the HUPnP \ref devicemodel
 * is all about.
 *
 * Hosting a device is simple, assuming you have the necessary device and service
 * descriptions ready and the HUPnP device and service classes implemented.
 * Basically, you only need to:
 *
 * \li instantiate an HDeviceConfiguration for each device type to be hosted and
 * pass them to the %HDeviceHost inside a HDeviceHostConfiguration instance
 * \li instantiate and initialize an %HDeviceHost
 * \li make sure a Qt event loop is present in the thread in which the
 * %HDeviceHost is run.
 *
 * As an example, consider the following:
 *
 * \code
 *
 * // myclass.h

 * #include <HDeviceHost>
 * #include <QScopedPointer>
 *
 * class MyClass
 * {
 * private:
 *     QScopedPointer<Herqq::Upnp::HDeviceHost> m_deviceHost;
 *
 * public:
 *     MyClass();
 * };
 *
 * // myclass.cpp
 *
 * #include "myclass.h"
 * #include "my_hdevice.h" // your code containing the type MyHDevice
 *
 * namespace
 * {
 * class Creator
 * {
 * public:
 *     Herqq::Upnp::HDevice* operator()(const Herqq::Upnp::HDeviceInfo&)
 *     {
 *         return new MyHDevice(); // your class derived from HDevice
 *     }
 * };
 * }
 *
 * MyClass::MyClass() :
 *     m_deviceHost(new Herqq::Upnp::HDeviceHost())
 * {
 *     Herqq::Upnp::HDeviceConfiguration deviceConf;
 *     deviceConf.setPathToDeviceDescription("my_hdevice_devicedescription.xml");
 *
 *     Creator deviceCreator;
 *     // this could also be a free function or a member function.
 *
 *     deviceConf.setDeviceCreator(deviceCreator);
 *
 *     if (m_deviceHost->init(deviceConf) != Herqq::Upnp::HDeviceHost::Success)
 *     {
 *         // the initialization failed, perhaps you should do something?
 *         return;
 *     }
 *
 *     // the host is running and your device should now be accessible to
 *     // UPnP Control points until the host is destroyed and assuming the current
 *     // thread has an event loop.
 * }
 *
 * \endcode
 *
 * There are a few noteworthy issues in the example above.
 *
 * -# The device host will fail to initialize if your HDeviceConfiguration
 * instance is invalid; for instance, the \e device \e creator is not specified or
 * the path to your UPnP Device Description is invalid. Similarly, if your
 * UPnP Device or UPnP Service description (if your UPnP device has one) is invalid, the device
 * host will fail to initialize. The point is, \b check \b the \b return \b value.
 * -# Your %HDevice is accessible only as long as your %HDeviceHost
 * is alive. When the device host is destroyed, any UPnP devices it hosted
 * are destroyed as well.
 * -# %HDeviceHost requires an event loop to function.
 * -# %HDeviceHost takes in a HDeviceHostConfiguration object, which has a constructor
 * that takes in a HDeviceConfiguration object. This is exploited in the example above,
 * since we are not interested in hosting multiple %HDevices in the same host and
 * we are not interested in modifying the default behavior of the %HDeviceHost.
 *
 * \remark
 *
 * \li %HDeviceHost has thread affinity, which mandates
 * that the %HDeviceHost and any object managed by it must be used and destroyed in the
 * thread in which the %HDeviceHost at the time lives.
 * You can use <c>QObject::moveToThread()</c> on the %HDeviceHost, which causes
 * the device host and every object managed by it to be moved to the chosen thread.
 * However, you cannot move individual objects managed by %HDeviceHost.
 *
 * \li %HDeviceHost is the owner of the instances of
 * %HDevice it manages. It takes care of the memory management
 * of every object it has created.
 *
 * \li <b>%HDeviceHost always destroys every %HDevice it manages when it is being destroyed</b>.
 *
 * \sa devicehosting, HDevice, HDeviceHostConfiguration, HDeviceConfiguration
 */
class H_UPNP_CORE_EXPORT HDeviceHost :
    public HAbstractHost
{
Q_OBJECT
H_DISABLE_COPY(HDeviceHost)
H_DECLARE_PRIVATE(HDeviceHost)

public:

    /*!
     * Specifies return values that some of the methods of the class may return.
     */
    enum ReturnCode
    {
        /*!
         * Return value signifying general failure. This return code is used when
         * an operation could not be successfully completed, but the exact
         * cause for the error could not be determined.
         */
        UndefinedFailure = -1,

        /*!
         *  Return value signifying success.
         */
        Success = 0,

        /*!
         * Return value signifying that the device host is already successfully
         * initialized.
         */
        AlreadyInitialized = 1,

        /*!
         * Return value signifying that the provided
         * host configuration was incorrect.
         */
        InvalidConfiguration = 2,

        /*!
         * Return value signifying that the provided device description document
         * was invalid.
         */
        InvalidDeviceDescription = 3,

        /*!
         * Return value signifying that the provided service description document
         * was invalid.
         */
        InvalidServiceDescription = 4,

        /*!
         * Return value used to indicate one or more more problems in communications
         * layer.
         */
        CommunicationsError
    };

public:

    /*!
     * Creates a new instance.
     *
     * \param parent specifies the parent object.
     */
    explicit HDeviceHost(QObject* parent = 0);

    /*!
     * Destroys the device host and every hosted device.
     *
     * For more information,
     *
     * \sa HAbstractHost::~HAbstractHost()
     */
    virtual ~HDeviceHost();

public Q_SLOTS:

    /*!
     * Initializes the device host and the devices it is supposed to host.
     *
     * \param configuration specifies the configuration for the instance. The
     * object has to contain at least one device configuration.
     *
     * \param errorString will contain a textual error description
     * in case the call failed and a pointer to a valid QString object was specified by the caller.
     *
     * \retval Success when the host was successfully started.
     *
     * \retval AlreadyInitialized when the host has already been successfully started.
     *
     * \retval InvalidInitParams when the provided initialization parameters
     * contained one or more erroneous values, such as missing the HDeviceCreator.
     *
     * \retval InvalidDeviceDescription when the provided device description file
     * is invalid.
     *
     * \retval InvalidServiceDescription when the provided service description file
     * is invalid.
     *
     * \retval UndefinedFailure in case some other initialization error occurred.
     */
    ReturnCode init(
        const Herqq::Upnp::HDeviceHostConfiguration& configuration,
        QString* errorString = 0);

    /*!
     * Quits the device host and destroys the UPnP devices it is hosting. Note that
     * this is automatically called when the device host is destroyed.
     *
     * \param errorString will contain a textual error description,
     * in case the call failed and a pointer to a valid QString object was specified by the caller.
     *
     * \retval Success when the device host was successfully shutdown,
     * or it was already shutdown.
     *
     * \retval UndefinedFailure when the device host could not be cleanly shutdown.
     * This could mean that the host failed to announce that the
     * devices it was hosting are leaving the network, for example.
     */
    ReturnCode quit(QString* errorString = 0);
};

}
}

#endif /* HDEVICEHOST_H_ */
