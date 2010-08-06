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

#ifndef HDEVICEHOST_CONFIGURATION_H_
#define HDEVICEHOST_CONFIGURATION_H_

#include "../hdevicecreator.h"
#include "../../general/hdefs_p.h"
#include "../../general/hupnp_global.h"

class QString;
class QHostAddress;

template<typename T>
class QList;

namespace Herqq
{

namespace Upnp
{

class HDeviceConfigurationPrivate;

/*!
 * This is a class for specifying a configuration to an HDevice that is to be created
 * and hosted by an HDeviceHost.
 *
 * A valid device configuration contains at least:
 *
 * \li a path to a device description file (setPathToDeviceDescription()) and
 * \li a <em>device creator</em> (setDeviceCreator()).
 *
 * The other options affect the runtime behavior of a HDeviceHost in regard to
 * the HDevice that is created based on the information provided through
 * an instance of this class.
 *
 * \headerfile hdevicehost_configuration.h HDeviceConfiguration
 *
 * \ingroup devicehosting
 *
 * \sa HDeviceHostConfiguration, HDeviceHost, HDeviceHost::init(), HDevice
 */
class H_UPNP_CORE_EXPORT HDeviceConfiguration
{
H_DISABLE_COPY(HDeviceConfiguration)

protected:

    HDeviceConfigurationPrivate* h_ptr;

    /*!
     * Clones the contents of this to the \c target object.
     *
     * Every derived class should override this method, especially if new
     * member variables have been introduced. Further, the implementation
     * should be something along these lines:
     *
     * \code
     * void MyDeviceConfiguration::doClone(HDeviceConfiguration* target) const
     * {
     *    MyDeviceConfiguration* myConf =
     *        dynamic_cast<MyDeviceConfiguration*>(target);
     *    if (!target)
     *    {
     *        return;
     *    }
     *
     *    BaseClassOfMyDeviceConfiguration::doClone(myConf);
     *
     *    // copy the variables introduced in *this* MyDeviceConfiguration
     *    // instance to "myConf".
     * }
     * \endcode
     *
     * \param target specifies the target object to which the contents of
     * \c this instance are cloned.
     */
    virtual void doClone(HDeviceConfiguration* target) const;

    /*!
     * Creates a new instance.
     *
     * This method is used as part of object cloning. Because of that, it is
     * important that every descendant class overrides this method:
     *
     * \code
     * HDeviceConfiguration* MyDeviceConfiguration::newInstance() const
     * {
     *     return new MyDeviceConfiguration();
     * }
     * \endcode
     *
     * \remarks
     * \li the object has to be heap-allocated and
     * \li the ownership of the object is passed to the caller.
     */
    virtual HDeviceConfiguration* newInstance() const;

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
     * \remarks
     * \li the ownership of the returned object is transferred to the caller.
     */
    HDeviceConfiguration* clone() const;

    /*!
     * Sets the path to the UPnP device description.
     *
     * \param pathToDeviceDescription specifies the path to the UPnP
     * device description.
     *
     * \remarks The provided path or the device description document is not
     * validated in anyway. The device description validation occurs during the
     * initialization of the HDeviceHost.
     */
    void setPathToDeviceDescription(const QString& pathToDeviceDescription);

    /*!
     * Returns the path to the device description.
     *
     * \return the path to the device description.
     */
    QString pathToDeviceDescription() const;

    /*!
     * Sets the maximum age of presence announcements and discovery responses
     * in seconds.
     *
     * \param maxAge specifies the maximum age of presence announcements
     * and discovery messages. If a value smaller than 5 is specified,
     * the max age is set to 5. If positive value larger than a day is specified,
     * the max age is set to a day (60*60*24). The default is 1800 seconds,
     * which equals to 30 minutes.
     *
     * \attention the UDA instructs this value to be at least 30 minutes.
     */
    void setCacheControlMaxAge(qint32 maxAge=1800);

    /*!
     * Returns the maximum age of presence announcements and discovery
     * responses in seconds.
     *
     * If the cache control max age has not been explicitly set,
     * the return value is 1800.
     *
     * \return the maximum age of presence announcements and discovery
     * responses in seconds.
     */
    qint32 cacheControlMaxAge() const;

    /*!
     * Returns the callable entity that is used to create HDevice instances.
     *
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
     *   - callable by the operator(), with single argument of
     *   const Herqq::Upnp::HDeviceInfo& deviceInfo
     *   and that returns a pointer to a heap allocated instance of
     *   Herqq::Upnp::HDevice*.
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
     * deviceCreator(Creator());
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
     *    configuration.deviceCreator(this, &MyClass::createMyDevice);
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
     * \return \e true in case the provided device creator is valid and it was
     * successfully set.
     *
     * \remarks
     *
     * \li The objects your device creator creates will be deallocated by HUPnP
     * when the objects are no longer needed. Do \b not delete
     * them manually.
     *
     * \li The device creator has to be set for every device to be hosted.
     *
     * \sa deviceCreator()
     */
    bool setDeviceCreator(const HDeviceCreator& deviceCreator);

    /*!
     * Indicates whether or not the object contains the necessary details
     * for hosting an HDevice class in a HDeviceHost.
     *
     * \retval true in case the object contains the necessary details
     * for hosting an HDevice class in a HDeviceHost.
     *
     * \retval false otherwise. In this case, the initialization of HDeviceHost
     * cannot succeed. Make sure you have set the device creator and path to
     * a device description file.
     */
    bool isValid() const;
};

class HDeviceHostConfigurationPrivate;

/*!
 * This class is used to specify one or more device configurations to an
 * HDeviceHost instance and to configure the functionality of the HDeviceHost
 * that affect every hosted HDevice.
 *
 * The initialization of an HDeviceHost requires a valid configuration.
 * A valid \e host \e configuration contains at least one \e device
 * \e configuration, as otherwise the host would have nothing to do. Because
 * of this the initialization of an HDeviceHost follows roughly these steps:
 *
 * - Create an HDeviceHostConfiguration instance.
 * - Create and setup one or more HDeviceConfiguration instances.
 * - Add the device configurations to the HDeviceHostConfiguration instance
 * using add().
 * - Modify the behavior of the HDeviceHost by setting other variables
 * in this class.
 * - Create an HDeviceHost and initialize it by passing the
 * HDeviceHostConfiguration to its HDeviceHost::init() method.
 *
 * Besides specifying the device configurations, you can configure an HDeviceHost
 * in following ways:
 * - Specify the threading model an HDeviceHost should use in regard to invoking
 * user code with setThreadingModel().
 * The default is HDeviceHostConfiguration::MultiThreaded, which means that the
 * user provided action implementations have to be thread-safe.
 * - Specify how many times each resource advertisement is sent with
 * setIndividualAdvertisementCount(). The default is 2.
 * - Specify the timeout for event subscriptions with
 * setSubscriptionExpirationTimeout(). The default is 0, which means that
 * an HDeviceHost respects the subscription timeouts requested by control points
 * as long as the requested values are less than a day.
 * - Specify the network addresses an HDeviceHost should use in its operations
 * with setNetworkAddressesToUse().
 * The default is the first found interface that is up. Non-loopback interfaces
 * have preference, but if none are found the loopback is used. However, in this
 * case UDP multicast is not available.
 *
 * \headerfile hdevicehost_configuration.h HDeviceHostConfiguration
 *
 * \ingroup devicehosting
 *
 * \sa HDeviceConfiguration, HDeviceHost
 */
class H_UPNP_CORE_EXPORT HDeviceHostConfiguration
{
H_DISABLE_COPY(HDeviceHostConfiguration)

protected:

    HDeviceHostConfigurationPrivate* h_ptr;

    /*!
     * Clones the contents of this to the \c target object.
     *
     * Every derived class should override this method, especially if new
     * member variables have been introduced. Further, the implementation
     * should be something along these lines:
     *
     * \code
     * void MyDeviceHostConfiguration::doClone(HDeviceHostConfiguration* target) const
     * {
     *    MyDeviceHostConfiguration* myConf =
     *        dynamic_cast<MyDeviceHostConfiguration*>(target);
     *    if (!target)
     *    {
     *        return;
     *    }
     *
     *    BaseClassOfMyDeviceHostConfiguration::doClone(myConf);
     *
     *    // copy the variables introduced in *this* MyDeviceHostConfiguration
     *    // instance to "myConf".
     * }
     * \endcode
     *
     * \param target specifies the target object to which the contents of
     * \c this instance are cloned.
     */
    virtual void doClone(HDeviceHostConfiguration* target) const;

    /*!
     * Creates a new instance.
     *
     * This method is used as part of object cloning. Because of that, it is
     * important that every descendant class overrides this method:
     *
     * \code
     * HDeviceHostConfiguration* HDeviceHostConfiguration::newInstance() const
     * {
     *     return new HDeviceHostConfiguration();
     * }
     * \endcode
     *
     * \remarks
     * \li the object has to be heap-allocated and
     * \li the ownership of the object is passed to the caller.
     */
    virtual HDeviceHostConfiguration* newInstance() const;

public:

    /*!
     * This enumeration specifies the threading models the HDeviceHost may
     * use in its operations in regard to user code invocation.
     */
    enum ThreadingModel
    {
        /*!
         * User code is invoked only from the thread in which the HDeviceHost
         * lives.
         *
         * This value is often used in situations where an HDevice being run
         * by the device host has thread affinity. For instance, this is the case
         * with \c HDevices that need to interact with GUIs.
         */
        SingleThreaded,

        /*!
         * User code may be invoked from an arbitrary thread.
         *
         * This value should be used in situations where the HDevices run by
         * the device host are thread-safe.
         */
        MultiThreaded
    };

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
     * Destroys the instance.
     */
    virtual ~HDeviceHostConfiguration();

    /*!
     * Returns a deep copy of the instance.
     *
     * \return a deep copy of the instance.
     *
     * \remarks
     * \li you should override this in derived classes. Failing
     * to override this will result in invalid clones being made of derived classes
     * that introduce new member variables.
     * \li the ownership of the returned object is transferred to the caller.
     */
    HDeviceHostConfiguration* clone() const;

    /*!
     * Adds a device configuration.
     *
     * \param deviceConfiguration specifies the device configuration to be added.
     * The configuration is added only if it is valid (\sa HDeviceConfiguration::isValid()).
     *
     * \return \e true in case the configuration was added. A configuration
     * that is invalid, i.e. HDeviceConfiguration::isValid() returns false will
     * not be added and \e false is returned.
     */
    bool add(const HDeviceConfiguration& deviceConfiguration);

    /*!
     * Returns the currently stored device configurations.
     *
     * \return the currently stored device configurations. The returned list
     * contains pointers to const device configuration objects this instance
     * owns. The ownership of the objects is not transferred. You
     * should \b never delete these, as that would eventually result in double
     * deletion.
     */
    QList<const HDeviceConfiguration*> deviceConfigurations() const;

    /*!
     * Indicates how many times the device host sends each individual
     * advertisement / announcement.
     *
     * The default value is 2.
     *
     * \return how many times the device host sends each individual
     * advertisement / announcement.
     *
     * \sa setIndividualAdvertisementCount()
     */
    qint32 individualAdvertisementCount() const;

    /*!
     * Returns the network addresses a device host should use in its
     * operations.
     *
     * \return the network addresses a device host should use in its
     * operations.
     *
     * \sa setNetworkAddressesToUse()
     */
    QList<QHostAddress> networkAddressesToUse() const;

    /*!
     * Returns the timeout the device host uses for subscriptions.
     *
     * The default value is zero, which means that the device host honors the
     * timeouts requested by control points up to a day. Larger values are
     * set to a day.
     *
     * \return the timeout in seconds the device host uses for subscriptions.
     *
     * \remarks this is a low-level detail, which you shouldn't modify unless you
     * know what you are doing.
     *
     * \sa setSubscriptionExpirationTimeout()
     */
    qint32 subscriptionExpirationTimeout() const;

    /*!
     * Returns the user code threading model the HDeviceHost uses in its
     * operations.
     *
     * \return the user code threading model the HDeviceHost uses in its
     * operations.
     */
    ThreadingModel threadingModel() const;

    /*!
     * Specifies how many times the device host sends each individual
     * advertisement / announcement.
     *
     * By default, each advertisement is sent twice.
     *
     * \param count specifies how many times the device host sends each individual
     * advertisement / announcement. If the provided value is smaller than 1 the
     * advertisement count is set to 1.
     *
     * \remarks this is a low-level detail, which you shouldn't modify unless you
     * know what you are doing.
     *
     * \sa individualAdvertisementCount()
     */
    void setIndividualAdvertisementCount(qint32 count);

    /*!
     * Specifies the timeout the device host uses for subscriptions.
     *
     * The default value is zero, which means that the device host honors the
     * timeouts requested by control points.
     *
     * \param timeout specifies the desired timeout in seconds.
     * - If timeout is greater than
     * zero the device host will use the timeout as such for subscriptions.
     * - If timeout is zero the device host will honor the timeout requested
     * by control points.
     * - If timeout is negative the subscription timeout is set to a day.
     *
     * \note the maximum expiration timeout value is a day. Larger values are
     * set to a day. This applies to the timeout requests made by control points
     * as well.
     *
     * \sa subscriptionExpirationTimeout()
     */
    void setSubscriptionExpirationTimeout(qint32 timeout);

    /*!
     * Defines the network addresses the device host should use in its
     * operations.
     *
     * \param addresses specifies the network addresses the device host
     * should use in its operations.
     *
     * \sa networkAddressesToUse()
     */
    bool setNetworkAddressesToUse(const QList<QHostAddress>& addresses);

    /*!
     * Sets the user code threading model the HDeviceHost should use in its
     * operations.
     *
     * This value specifies how an HDeviceHost invokes user code in regard
     * to thread safety. If the value is HDeviceHostConfiguration::SingleThreaded
     * user code is invoked only from the thread in which the HDeviceHost instance
     * is run. If the value is HDeviceHostConfiguration::MultiThreaded user code
     * may be invoked from any thread at any time. The default is
     * HDeviceHostConfiguration::MultiThreaded.
     *
     * \param arg specifies the user code threading model the HDeviceHost
     * should use in its operations.
     */
    void setThreadingModel(ThreadingModel arg);

    /*!
     * Indicates if the instance contains any device configurations.
     *
     * \return \e true in case the instance contains no device configurations.
     * In this case the object cannot be used to initialize an HDeviceHost.
     */
    bool isEmpty() const;
};

}
}

#endif /* HDEVICEHOST_CONFIGURATION_H_ */
