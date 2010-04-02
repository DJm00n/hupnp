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

#include "./../../general/hdefs_p.h"
#include "./../hdevicecreator.h"

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
 * \remark this class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HControlPointConfiguration
{
H_DISABLE_COPY(HControlPointConfiguration)
friend class HControlPoint;

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
     * \return a deep copy of the instance. The ownership of the returned object
     * is transferred to the caller. Remember to delete the object.
     *
     * \remark most often you should override this in derived classes.
     */
    virtual HControlPointConfiguration* clone() const;

    /*!
     * Returns the user-defined callable entity that is used to create HDevice instances.
     *
     * \return the user-defined callable entity that is used to create HDevice instances.
     *
     * \sa setDeviceCreator()
     */
    HDeviceCreator deviceCreator() const;

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
     * Sets the callable entity that is used to create HDevice instances.
     *
     * Setting the device creator is useful when you want to create the
     * types that will be used later as HDevice instances. Perhaps you have created
     * HDevice types to be run on HDeviceHost and you want to use
     * the same custom types on a \c %HControlPoint. However, <b>this
     * is purely optional</b>. If the device creator is not set, \c %HControlPoint
     * will create and use default types. As implied, usually custom types provide
     * value only when the custom types contain additional functionality, finer-grained
     * API or something else that the base classes of \ref devicemodel do not.
     *
     * In any case, your callable entity must be:
     *   - copyable by value
     *   - callable with a signature <c>Herqq::Upnp::HDevice* function_name(const Herqq::Upnp::HDeviceInfo&);</c>
     *
     * Note, the return value must be a pointer to a heap allocated instance of
     * Herqq::Upnp::HDevice* and the ownership of the created device will be
     * transferred to the \c %HControlPoint after returning.
     *
     * From this follows, that the device creator can be a:
     *
     * \li functor,
     * \li function pointer or
     * \li member function pointer
     *
     * For example, if your callable entity is a functor, it could
     * look something like the following:
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
     * \remark the objects your device creator creates will be deallocated by the Herqq
     * library when the objects are no longer needed. Do NOT store them or delete them manually.
     *
     * \remark setting a device creator is optional and unless you really want
     * to define custom HDevice types to be used, you should not set this
     * at all. This is different with device hosts.
     */
    void setDeviceCreator(HDeviceCreator deviceCreator);

    /*!
     * Sets whether to automatically subscribe to all events on all services
     * of a device when a new device is added into the control of an HControlPoint.
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
};

}
}

#endif /* HCONTROLPOINT_CONFIGURATION_H_ */
