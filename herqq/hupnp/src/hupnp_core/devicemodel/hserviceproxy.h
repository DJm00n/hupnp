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

#ifndef HSERVICEPROXY_H_
#define HSERVICEPROXY_H_

#include "hservice.h"

namespace Herqq
{

namespace Upnp
{

class HServiceProxyPrivate;

/*!
 * \brief A class that is used at client-side to represent a service found in a
 * UPnP device.
 *
 * A proxy service is a pure client-side concept utilized by HControlPoint. This
 * class is instantiated by HControlPoint for every service found in every
 * discovered UPnP device that is added into the control of an \c %HControlPoint.
 *
 * As an HService, the \c %HServiceProxy relies fully on the HUPnP \ref devicemodel
 * and thus it is used similarly compared to server-side \c %HService classes.
 * Unlike the \c %HService at server-side, however, the \c %HServiceProxy at
 * client-side is usually responsible of only performing remote procedure calls to
 * the server side.
 *
 * \headerfile hserviceproxy.h HServiceProxy
 *
 * \ingroup devicemodel
 *
 * \remarks the methods introduced in this class are thread-safe, but the \c QObject
 * base class is largely not.
 *
 * \sa HControlPoint, HDeviceProxy
 */
class H_UPNP_CORE_EXPORT HServiceProxy :
    public HService
{
H_DISABLE_COPY(HServiceProxy)
H_DECLARE_PRIVATE(HServiceProxy)

private:

    /*!
     * Overriding this method has no effect. At client-side every action
     * invocation is always directed by HUPnP to the UPnP device over the network.
     */
    virtual HActionMap createActions();

protected:

    //
    // \internal
    //
    HServiceProxy(HServiceProxyPrivate& dd);

public:

    /*!
     * \brief Creates a new instance.
     *
     * Creates a new instance.
     */
    HServiceProxy();

    /*!
     * \brief Destroys the instance.
     *
     * Destroys the instance.
     */
    virtual ~HServiceProxy();

    /*!
     * Returns the parent HDeviceProxy that contains this service instance.
     *
     * This is a helper method that returns the parent HDevice of this service
     * statically cast to HDeviceProxy.
     *
     * \return the parent HDeviceProxy that contains this service instance.
     */
    HDeviceProxy* parentProxyDevice() const;
};

}
}

#endif /* HSERVICEPROXY_H_ */
