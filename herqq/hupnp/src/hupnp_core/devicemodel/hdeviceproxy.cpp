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

#include "hdeviceproxy.h"
#include "hdeviceproxy_p.h"

#include "hservice_p.h"

namespace Herqq
{

namespace Upnp
{

HDeviceProxyPrivate::HDeviceProxyPrivate()
{
}

HDeviceProxyPrivate::~HDeviceProxyPrivate()
{
}

HDeviceProxy::HDeviceProxy() :
    HDevice(*new HDeviceProxyPrivate())
{
}

HDeviceProxy::HDeviceProxy(HDeviceProxyPrivate& dd) :
    HDevice(dd)
{
}

HDeviceProxy::~HDeviceProxy()
{
}

HServicesSetupData* HDeviceProxy::createServices()
{
    return 0;
}

HDeviceProxy* HDeviceProxy::parentProxyDevice() const
{
    return static_cast<HDeviceProxy*>(HDevice::parentDevice());
}

HDeviceProxy* HDeviceProxy::rootProxyDevice() const
{
    return static_cast<HDeviceProxy*>(HDevice::rootDevice());
}

HServiceProxy* HDeviceProxy::serviceProxyById(const HServiceId& serviceId) const
{
    return static_cast<HServiceProxy*>(HDevice::serviceById(serviceId));
}

HDeviceProxies HDeviceProxy::embeddedProxyDevices() const
{
    HDeviceProxies retVal;
    foreach(HDeviceController* dc, h_ptr->m_embeddedDevices)
    {
        retVal.push_back(dc->m_deviceProxy);
    }

    return retVal;
}

HServiceProxies HDeviceProxy::serviceProxies() const
{
    HServiceProxies retVal;
    foreach(HServiceController* sc, h_ptr->m_services)
    {
        retVal.push_back(sc->m_serviceProxy);
    }

    return retVal;
}

HServiceProxies HDeviceProxy::serviceProxiesByType(
    const HResourceType& type, HResourceType::VersionMatch versionMatch) const
{
    if (!type.isValid())
    {
        return HServiceProxies();
    }

    HServiceProxies retVal;
    foreach(HServiceController* sc, h_ptr->m_services)
    {
        if (sc->m_service->info().serviceType().compare(type, versionMatch))
        {
            retVal.push_back(sc->m_serviceProxy);
        }
    }

    return retVal;
}

}
}
