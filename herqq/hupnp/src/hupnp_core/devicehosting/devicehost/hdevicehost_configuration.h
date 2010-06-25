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

#include "../../general/hdefs_p.h"
#include "../hdevicecreator.h"

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
 * This is a class for specifying configuration to an HDevice that is to be created
 * and hosted by an HDeviceHost.
 *
 * To host a device, you have to set:
 *
 * \li a path to a device description file (setPathToDeviceDescription()) and
 * \li a <em>device creator</em> (setDeviceCreator()).
 *
 * Otherwise, an HDeviceHost instance will fail to start.
 *
 * Other options affect the runtime behavior of a HDeviceHost in regard to
 * the HDevice that is created based on the information provided through
 * an instance of this class.
 *
 * \headerfile hdevicehost_configuration.h HDeviceConfiguration
 *
 * \ingroup devicehosting
 *
 * \sa HDeviceHost, HDeviceHost::init(), HDevice
 */
class H_UPNP_CORE_EXPORT HDeviceConfiguration
{
H_DISABLE_COPY(HDeviceConfiguration)

private:

    HDeviceConfigurationPrivate* h_ptr;

    /*!
     * Creates a clone of the object.
     *
     * \remarks you should override this in derived classes. Failing
     * to override this will result in invalid clones being made of derived classes
     * that introduce new member variables.
     */
    virtual HDeviceConfiguration* doClone() const;

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
     * \param pathToDeviceDescription specifies the path to the UPnP device description.
     *
     * \return \e true if the path points to an existing file and the path
     * was successfully set.
     *
     * \remarks that the device description file is not validated in anyway. The
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
     * \return \e true in case the provided device creator is valid and it was
     * successfully set.
     *
     * \remarks
     *
     * \li The objects your device creator creates will be deallocated by HUPnP
     * when the objects are no longer needed. Do NOT store them or delete
     * them manually.
     *
     * \li The device creator has to be set for every device to be hosted.
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
 * This is a class for specifying configurations for HDevices that should be created
 * and hosted by an HDeviceHost. The class is also used to configure the
 * operations of HDeviceHost that affect every hosted HDevice.
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

private:

    HDeviceHostConfigurationPrivate* h_ptr;

    /*!
     * Creates a clone of the object.
     *
     * \remarks you should override this in derived classes. Failing
     * to override this will result in invalid clones being made of derived classes
     * that introduce new member variables.
     */
    virtual HDeviceHostConfiguration* doClone() const;

public:

    /*!
     * todo
     */
    /*enum ThreadingModel
    {
        SingleThreaded,
        MultiThreaded
    };*/

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
     * todo
     */
    //ThreadingModel threadingModel() const;

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
     *todo
     */
    //void setThreadingModel(ThreadingModel arg);

    /*!
     * Indicates if the instance contains any device configurations.
     *
     * \return \e true in case the instance contains at least
     * one device configuration.
     */
    bool isEmpty() const;
};

}
}

#endif /* HDEVICEHOST_CONFIGURATION_H_ */
