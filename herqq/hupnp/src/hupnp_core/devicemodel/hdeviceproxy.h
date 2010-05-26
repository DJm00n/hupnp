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

#ifndef HDEVICEPROXY_H_
#define HDEVICEPROXY_H_

#include "hdevice.h"

namespace Herqq
{

namespace Upnp
{

class HDeviceProxyPrivate;

/*!
 * \brief A class that is used at client-side to represent a UPnP device.
 *
 * A proxy device is a pure client-side concept utilized by HControlPoint. This
 * class is instantiated by HControlPoint for each discovered UPnP device that
 * is added into the control of an \c %HControlPoint.
 *
 * As an HDevice, the \c %HDeviceProxy relies fully on the HUPnP \ref devicemodel
 * and thus it is used similarly to server-side \c %HDevice classes.
 * Unlike the \c %HDevice at server-side, however, the \c %HDeviceProxy at
 * client-side is usually only responsible of providing helper methods for accessing
 * the server side.
 *
 * As HControlPoint allows the creation and use of custom \c %HDeviceProxy types,
 * you can use the \c %HDeviceProxy as a base for more refined proxies that could
 * provide more fine-grained APIs to communicate with a particular device type.
 * For instance, if there is a UPnP device type you would like to use from the
 * client side using a statically typed API, you could create a class derived
 * from the \c %HDeviceProxy. This class would provide the desired API
 * hiding all the things relating to action invocation and converting
 * state variable notifications to Qt signals.
 *
 * \note Creating a custom \c %HDeviceProxy often means that you should create custom
 * HServiceProxy classes as well. In that case you should override the
 * createServices() method and instantiate the desired \c %HServiceProxy types
 * there.
 *
 * \headerfile hdeviceproxy.h HDeviceProxy
 *
 * \ingroup devicemodel
 *
 * \remarks the methods introduced in this class are thread-safe, but the \c QObject
 * base class is largely not.
 *
 * \sa HControlPoint, HServiceProxy
 */
class H_UPNP_CORE_EXPORT HDeviceProxy :
    public HDevice
{
H_DISABLE_COPY(HDeviceProxy)

private:

    /*!
     * \c %HDeviceProxy provides default implementation for this, but if you want
     * to associate specific HServiceProxy types with this \c %HDeviceProxy type
     * you have to override this method.
     *
     * \warning At the moment the API doesn't explicitly require the creation
     * of HServiceProxy types. In the future this will be changed. Furthermore,
     * creating types not derived from HServiceProxy results in undefined
     * behavior.
     */
    virtual HServiceMap createServices();

protected:

    //
    // \internal
    //
    // Constructor for derived classes for re-using the h_ptr.
    //
    HDeviceProxy(HDeviceProxyPrivate& dd);

public:

    /*!
     * \brief Creates a new instance.
     *
     * Creates a new instance.
     */
    HDeviceProxy();

    /*!
     * \brief Destroys the instance.
     *
     * Destroys the instance.
     */
    virtual ~HDeviceProxy();

    /*!
     * Returns the parent device proxy of this device.
     *
     * This is a helper method that returns the parent HDevice of this device
     * statically cast to HDeviceProxy.
     *
     * \return the parent device proxy of this device.
     */
    HDeviceProxy* parentProxyDevice() const;

    /*!
     * Returns the root device proxy of this device tree.
     *
     * This is a helper method that returns the root HDevice of this device tree
     * statically cast to HDeviceProxy.
     *
     * \return the root device proxy of this device tree.
     */
    HDeviceProxy* rootProxyDevice() const;

    /*!
     * Returns a service proxy that matches the provided service ID.
     *
     * This is a helper method that returns the a HService matching the specified
     * serviceID statically cast to HServiceProxy.
     *
     * \param serviceId specifies the service to be returned.
     *
     * \return the root proxy device of this device tree.
     */
    HServiceProxy* serviceProxyById(const HServiceId& serviceId) const;

    /*!
     * Returns the embedded device proxies of this device.
     *
     * This is a helper method that returns the embedded HDevice instances of this
     * device statically cast to HDeviceProxy instances.
     *
     * \return the embedded device proxies of this device.
     */
    HDeviceProxies embeddedProxyDevices() const;

    /*!
     * Returns service proxies of this device.
     *
     * This is a helper method that returns HService instances of this
     * device statically cast to HServiceProxy instances.
     *
     * \return the service proxies of this device.
     */
    HServiceProxies serviceProxies() const;

    /*!
     * Returns service proxies of this device.
     *
     * This is a helper method that returns HService instances of this
     * device statically cast to HServiceProxy instances.
     *
     * \param serviceType specifies the UPnP service type of interest.
     * Only services matching the type are returned.
     * \param versionMatch specifies how the version information in argument
     * \c serviceType should be used. The default is <em>inclusive match</em>,
     * which essentially means that any service with a service type version that
     * is \b less than or \b equal to the version specified in argument
     * \c serviceType is successfully matched.
     *
     * \return the services proxies of the specified type.
     */
    HServiceProxies serviceProxiesByType(
        const HResourceType& serviceType,
        HResourceType::VersionMatch versionMatch = HResourceType::InclusiveVersionMatch) const;
};

}
}

#endif /* HDEVICEPROXY_H_ */
