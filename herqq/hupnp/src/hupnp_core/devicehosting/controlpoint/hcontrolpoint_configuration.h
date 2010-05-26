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

#ifndef HCONTROLPOINT_CONFIGURATION_H_
#define HCONTROLPOINT_CONFIGURATION_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../../general/hdefs_p.h"
#include "hdeviceproxy_creator.h"

class QHostAddress;

namespace Herqq
{

namespace Upnp
{

class HControlPointConfigurationPrivate;

/*!
 * Class for specifying initialization information to HControlPoint instances.
 *
 * \headerfile hcontrolpoint_configuration.h HControlPointConfiguration
 *
 * \ingroup devicehosting
 *
 * \sa HControlPoint::init()
 *
 * \remarks this class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HControlPointConfiguration
{
H_DISABLE_COPY(HControlPointConfiguration)
friend class HControlPoint;

private:

    /*!
     * Creates a clone of the object.
     *
     * \remarks you should override this in derived classes. Failing
     * to override this will result in invalid clones being made of derived classes
     * that introduce new member variables.
     */
    virtual HControlPointConfiguration* doClone() const;

protected:

    HControlPointConfigurationPrivate* h_ptr;
    HControlPointConfiguration(HControlPointConfigurationPrivate& dd);

public:

    /*!
     * Creates a new instance.
     */
    HControlPointConfiguration();

    /*!
     * Destroys the instance.
     */
    virtual ~HControlPointConfiguration();

    /*!
     * Returns a deep copy of the instance.
     *
     * \return a deep copy of the instance.
     *
     * \remarks
     * \li the ownership of the returned object is transferred to the caller.
     */
    HControlPointConfiguration* clone() const;

    /*!
     * Returns the user-defined callable entity that is used to create
     * HDeviceProxy instances.
     *
     * \return the user-defined callable entity that is used to create
     * HDeviceProxy instances.
     *
     * \sa setDeviceCreator()
     */
    HDeviceProxyCreator deviceCreator() const;

    /*!
     * Indicates whether to automatically subscribe to all events on all services
     * of a device when a new device is added into the control of an HControlPoint.
     *
     * \return \e true in case the HControlPoint instance should subscribe to all
     * events of all services of a newly added device.
     *
     * \sa setSubscribeToEvents()
     */
    bool subscribeToEvents() const;

    /*!
     * Returns the subscription timeout a control point requests when it subscribes
     * to an evented service.
     *
     * The default value is 30 minutes.
     *
     * \return the subscription timeout in seconds a control point requests
     * when it subscribes to an evented service.
     *
     * \sa setDesiredSubscriptionTimeout()
     */
    qint32 desiredSubscriptionTimeout() const;

    /*!
     * Indicates whether the control point should perform discovery upon
     * initialization.
     *
     * \return \e true in case the the control point should perform discovery upon
     * initialization. This is the default value.
     *
     * \remarks if the discovery is not performed the control point will be
     * unaware of UPnP devices that are already active in the network until they
     * re-advertise themselves.
     *
     * \sa setPerformInitialDiscovery()
     */
    bool autoDiscovery() const;

    /*!
     * Returns the network addresses a control point should use in its
     * operations.
     *
     * \return the network addresses a control point should use in its
     * operations.
     *
     * \sa setNetworkAddressesToUse()
     */
    QList<QHostAddress> networkAddressesToUse() const;

    /*!
     * Sets the callable entity that is used to create HDeviceProxy instances.
     *
     * Setting the device creator is useful when you want to create the
     * types that will be used later as HDeviceProxy instances. However, <b>this
     * is purely optional</b>. If the device creator is not set, \c %HControlPoint
     * will create and use default types. Most often custom types provide
     * value only when the custom types contain additional functionality, finer-grained
     * API or something else that the base classes of \ref devicemodel do not.
     *
     * In any case, your callable entity must be:
     *   - copyable by value
     *   - callable with a signature <c>Herqq::Upnp::HDeviceProxy* function_name(const Herqq::Upnp::HDeviceInfo&);</c>
     *
     * Note, the return value must be a pointer to a heap allocated instance of
     * Herqq::Upnp::HDeviceProxy* and the ownership of the created device will be
     * transferred to the \c %HControlPoint after returning.
     *
     * From this follows that the device creator can be a:
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
     * class MyProxyCreator
     * {
     *     public:
     *         MyProxyCreator();
     *         ~MyProxyCreator();
     *
     *         HDeviceProxy* operator()(const Herqq::Upnp::HDeviceInfo&) const
     *         {
     *             return new MyProxyClass();
     *         }
     * };
     *
     * \endcode
     *
     * and you could call the method as follows:
     *
     * \code
     *
     * setDeviceCreator(MyProxyCreator());
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
     *    Herqq:Upnp::HDeviceProxy* createMyDevice(const Herqq::Upnp::HDeviceInfo&);
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
     * create HDeviceProxy instances.
     *
     * \remarks the objects your device creator creates will be deallocated by
     * the HUPnP library when the objects are no longer needed.
     * Do NOT store them or delete them manually.
     *
     * \remarks setting a device creator is optional and unless you really want
     * to define custom HDeviceProxy types to be used, you should not set this
     * at all.
     */
    bool setDeviceCreator(const HDeviceProxyCreator& deviceCreator);

    /*!
     * Defines whether a control point should automatically subscribe to all
     * events on all services of a device when a new device is added
     * into the control of an HControlPoint.
     *
     * \param subscribeAutomatically when \e true an HControlPoint instance
     * should by default subscribe to all events of all services of a newly added
     * device.
     *
     * \sa subscribeToEvents()
     */
    void setSubscribeToEvents(bool subscribeAutomatically);

    /*!
     * Sets the subscription timeout a control point requests when it subscribes
     * to an evented service.
     *
     * Values less than or equal to zero are rejected and instead the default value
     * is used. The default value is 30 minutes.
     *
     * \param timeout specifies the requested timeout in seconds.
     *
     * \sa desiredSubscriptionTimeout()
     */
    void setDesiredSubscriptionTimeout(qint32 timeout);

    /*!
     * Defines whether the control point should perform discovery upon
     * initialization.
     *
     * \param arg when \e true an HControlPoint instance will perform discovery
     * when it is initialized. This is the default.
     *
     * \remarks if the discovery is not performed the control point will be
     * unaware of UPnP devices that are already active in the network until they
     * re-advertise themselves.
     *
     * \sa performInitialDiscovery()
     */
    void setAutoDiscovery(bool arg);

    /*!
     * Defines the network addresses the control point should use in its
     * operations.
     *
     * \param addresses specifies the network addresses the control point
     * should use in its operations.
     *
     * \return \e true in case the provided addresses are valid and can be
     * used.
     *
     * \sa networkAddressesToUse()
     */
    bool setNetworkAddressesToUse(const QList<QHostAddress>& addresses);
};

}
}

#endif /* HCONTROLPOINT_CONFIGURATION_H_ */
