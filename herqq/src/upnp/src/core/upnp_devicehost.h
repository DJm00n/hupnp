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

#ifndef UPNP_DEVICEHOST_H_
#define UPNP_DEVICEHOST_H_

#include "upnp_abstracthost.h"
#include "upnp_devicecreator.h"

namespace Herqq
{

namespace Upnp
{

class HDeviceConfigurationPrivate;

/*!
 * This is a class for specifying configuration to an HDevice that is to be created
 * and hosted by an HDeviceHost.
 *
 * To host a device, you have to set:
 *
 * \li a path to a device description file (setPathToDeviceDescription()) and
 * \li a "device creator" (setDeviceCreator()).
 *
 * Otherwise, an HDeviceHost instance will fail to start.
 *
 * Other options affect the runtime behavior of a HDeviceHost in regard to
 * the HDevice that is created based on the information provided through
 * an instance of this class.
 *
 * \headerfile upnp_devicehost.h HDeviceConfiguration
 *
 * \ingroup devicehosting
 *
 * \sa HDeviceHost, HDeviceHost::init(), HDevice
 */
class H_UPNP_CORE_EXPORT HDeviceConfiguration
{
H_DISABLE_COPY(HDeviceConfiguration)
friend class HDeviceHost;

protected:

    HDeviceConfigurationPrivate* h_ptr;
    explicit HDeviceConfiguration(HDeviceConfigurationPrivate& dd);

public:

    /*!
     * Creates a new, empty instance.
     */
    HDeviceConfiguration();

    /*!
     * Destroys the instance.
     */
    virtual ~HDeviceConfiguration();

    /*!
     * Returns a deep copy of the instance.
     *
     * \return a deep copy of the instance.
     *
     * \remark you should override this in derived classes. Failing
     * to override this will result in invalid clones being made of derived classes
     * that introduce new member variables.
     */
    virtual HDeviceConfiguration* clone() const;

    /*!
     * Sets the maximum age of presence announcements and discovery responses in seconds.
     *
     * \param maxAge specifies the maximum age of presence announcements and discovery messages.
     * If negative value smaller than -1 is specified, the max age is set to -1, which
     * means that the there is no timeout for expiration. If positive value larger than
     * a day is specified, the max age is set to a day (60*60*24). The default is
     * 1800 seconds, which equals to 30 minutes.
     */
    void setCacheControlMaxAge(qint32 maxAge=1800);

    /*!
     * Sets the path to the UPnP device description.
     *
     * \param pathToDeviceDescription specifies the path to the UPnP device description.
     *
     * \retval true if the path points to an existing file.
     * \retval false otherwise.
     *
     * \remark that the device description file is not validated in anyway. The
     * method only checks the existence of the provided file. The device description
     * validation occurs during the initialization of the HDeviceHost.
     */
    bool setPathToDeviceDescription(const QString& pathToDeviceDescription);

    /*!
     * Returns the path to the device description.
     *
     * \return the path to the device description.
     */
    QString pathToDeviceDescription() const;

    /*!
     * Returns the maximum age of presence announcements and discovery responses in seconds.
     *
     * If the cache control max age has not been explicitly set, the return value is 1800.
     *
     * \return the maximum age of presence announcements and discovery responses in seconds.
     */
    qint32 cacheControlMaxAge() const;

    /*!
     * Returns the callable entity that is used to create HDevice instances.
     * \return the callable entity that is used to create HDevice instances.
     *
     * \sa setDeviceCreator()
     */
    HDeviceCreator deviceCreator() const;

    /*!
     * Sets the callable entity that is used to create HDevice instances.
     *
     * In any case, your callable entity must be:
     *   - copyable by value
     *   - callable by the operator(), with single argument of const Herqq::Upnp::HDeviceInfo& deviceInfo
     *   and that returns a pointer to a heap allocated instance of Herqq::Upnp::HDevice*.
     *
     * In other words, the signature is
     * Herqq::Upnp::HDevice* operator()(const Herqq::Upnp::HDeviceInfo&);
     *
     * From this follows, that the device creator can be either a:
     *
     * \li functor,
     * \li function pointer or
     * \li member function pointer
     *
     * For example, if your callable entity is a functor, it could
     * look something like the following:
     *
     * \code
     *
     * class Creator
     * {
     * public:
     *     Herqq::Upnp::HDevice* operator()(const Herqq::Upnp::HDeviceInfo& deviceInfo)
     *     {
     *         return new MyFirstHDevice();
     *         // your class derived from HDevice that implements the functionality
     *         // advertised in the description files.
     *     }
     * };
     *
     * \endcode
     *
     * and you could call the method as follows:
     *
     * \code
     *
     * setDeviceCreator(Creator());
     *
     * \endcode
     *
     * If your callable entity is a member function other than the operator(),
     * the member function declaration would look like the following:
     *
     * \code
     *
     * class MyClass
     * {
     * private:
     *    Herqq:Upnp::HDevice* createMyDevice(const Herqq::Upnp::HDeviceInfo&);
     *
     * public:
     *     MyClass();
     * };
     *
     * \endcode
     *
     * and you could set the creator as follows (this is contrived due to the
     * private access specifier):
     *
     * \code
     *
     * MyClass::MyClass()
     * {
     *    HDeviceConfiguration configuration;
     *    configuration.setDeviceCreator(this, &MyClass::createMyDevice);
     * }
     *
     * \endcode
     *
     * The example above demonstrates that:
     *
     * \li the device creator can be \em any member function with a proper signature,
     * regardless of the access specifier.
     *
     * \li the way you could set the device creator.
     *
     * \param deviceCreator specifies the callable entity that is used to
     * create HDevice instances.
     *
     * \remark
     *
     * \li The objects your device creator creates will be deallocated by the Herqq
     * library when the objects are no longer needed. Do NOT store them or delete them manually.
     *
     * \li The device creator has to be set for every device to be hosted,
     * unlike with control points.
     */
    void setDeviceCreator(HDeviceCreator deviceCreator);

    /*!
     * Indicates whether or not the object contains the necessary details
     * for hosting a HDevice class in a HDeviceHost.
     *
     * \retval true in case the object contains the necessary details
     * for hosting a HDevice class in a HDeviceHost.
     *
     * \retval false otherwise. In this case, the initialization of HDeviceHost
     * cannot succeed.
     */
    bool isValid() const;
};

class HDeviceHostConfigurationPrivate;

/*!
 * This is a class for specifying configurations for HDevices that should be created
 * and hosted by an HDeviceHost. The class is also used to configure the
 * operations of HDeviceHost that affect every hosted HDevice.
 *
 * \headerfile upnp_devicehost.h HDeviceHostConfiguration
 *
 * \ingroup devicehosting
 *
 * \sa HDeviceConfiguration, HDeviceHost
 */
class H_UPNP_CORE_EXPORT HDeviceHostConfiguration
{
private:

    HDeviceHostConfigurationPrivate* h_ptr;

public:

    /*!
     * Creates a new, empty instance.
     */
    HDeviceHostConfiguration();

    /*!
     * Creates an instance with a single device configuration. This is a convenience
     * method.
     */
    HDeviceHostConfiguration(const HDeviceConfiguration&);

    /*!
     * Copy constructor.
     */
    HDeviceHostConfiguration(const HDeviceHostConfiguration&);

    /*!
     * Assignment operator.
     */
    HDeviceHostConfiguration& operator=(const HDeviceHostConfiguration&);

    /*!
     * Destroys the instance.
     */
    ~HDeviceHostConfiguration();

    /*!
     * Adds a device configuration.
     *
     * \param deviceConfiguration specifies the device configuration to be added.
     * The configuration is added only if it is valid (\sa HDeviceConfiguration::isValid()).
     *
     * \return true in case the configuration was added.
     */
    bool add(const HDeviceConfiguration& deviceConfiguration);

    /*!
     * Returns the currently stored device configurations.
     * \return the currently stored device configurations.
     */
    QList<HDeviceConfiguration*> deviceConfigurations() const;

    /*!
     * Indicates if the instance contains any device configurations.
     * \return true in case the instance contains at least one device configuration.
     */
    bool isEmpty() const;
};

class HDeviceHostPrivate;

/*!
 * A class for creating and hosting HDevices on the network.
 *
 * \headerfile upnp_devicehost.h HDeviceHost
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
        InvalidServiceDescription = 4
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

#endif /* UPNP_DEVICEHOST_H_ */
