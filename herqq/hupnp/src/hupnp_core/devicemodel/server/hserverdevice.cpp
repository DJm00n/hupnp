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

#include "hserverdevice.h"
#include "hserverdevice_p.h"
#include "hserverservice.h"
#include "hserverservice_p.h"
#include "hdefault_serverdevice_p.h"

#include "../hdevices_setupdata.h"
#include "../hservices_setupdata.h"

#include "../../../utils/hlogger_p.h"
#include "../../general/hupnp_global_p.h"

#include "../../dataelements/hserviceid.h"
#include "../../dataelements/hdeviceinfo.h"

#include <QtCore/QTimer>
#include <QtCore/QString>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HServerDevicePrivate
 ******************************************************************************/
HServerDevicePrivate::HServerDevicePrivate() :
    m_deviceInfo(0), m_embeddedDevices(), m_services(), m_parent(0),
    q_ptr(0), m_locations(), m_deviceDescription(), m_deviceStatus(0)
{
}

HServerDevicePrivate::~HServerDevicePrivate()
{
}

/*******************************************************************************
 * HServerDevice
 ******************************************************************************/
HServerDevice::HServerDevice() :
    QObject(),
        h_ptr(new HServerDevicePrivate())
{
}

HServerDevice::HServerDevice(HServerDevicePrivate& dd) :
    QObject(),
        h_ptr(&dd)
{
}

HServerDevice::~HServerDevice()
{
    delete h_ptr;
}

bool HServerDevice::finalizeInit(QString*)
{
    // intentionally empty
    Q_ASSERT(h_ptr->q_ptr);
    return true;
}

HServerDevice* HServerDevice::parentDevice() const
{
    return h_ptr->m_parent;
}

HServerDevice* HServerDevice::rootDevice() const
{
    HServerDevice* root = const_cast<HServerDevice*>(this);
    while(root->h_ptr->m_parent)
    {
        root = root->h_ptr->m_parent;
    }

    return root;
}

HServerService* HServerDevice::serviceById(const HServiceId& serviceId) const
{
    foreach(HServerService* sc, h_ptr->m_services)
    {
        if (sc->info().serviceId() == serviceId)
        {
            return sc;
        }
    }

    return 0;
}

const HServerServices& HServerDevice::services() const
{
    return h_ptr->m_services;
}

HServerServices HServerDevice::servicesByType(
    const HResourceType& type, HResourceType::VersionMatch versionMatch) const
{
    if (!type.isValid())
    {
        return HServerServices();
    }

    HServerServices retVal;
    foreach(HServerService* sc, h_ptr->m_services)
    {
        if (sc->info().serviceType().compare(type, versionMatch))
        {
            retVal.push_back(sc);
        }
    }

    return retVal;
}

const HServerDevices& HServerDevice::embeddedDevices() const
{
    return h_ptr->m_embeddedDevices;
}

const HDeviceInfo& HServerDevice::info() const
{
    return *h_ptr->m_deviceInfo;
}

QString HServerDevice::description() const
{
    return h_ptr->m_deviceDescription;
}

QList<QUrl> HServerDevice::locations(LocationUrlType urlType) const
{
    if (h_ptr->m_parent)
    {
        // the root device "defines" the locations and they are the same for each
        // embedded device.
        return h_ptr->m_parent->locations(urlType);
    }

    QList<QUrl> retVal;
    QList<QUrl>::const_iterator ci;
    for(ci = h_ptr->m_locations.begin(); ci != h_ptr->m_locations.end(); ++ci)
    {
        retVal.push_back(urlType == AbsoluteUrl ? *ci : extractBaseUrl(*ci));
    }

    return retVal;
}

const HDeviceStatus& HServerDevice::deviceStatus() const
{
    const HServerDevice* rootDev = rootDevice();
    return *rootDev->h_ptr->m_deviceStatus;
}

/*******************************************************************************
 * HDefaultServerDevice
 ******************************************************************************/
HDefaultServerDevice::HDefaultServerDevice() :
    HServerDevice()
{
}

}
}
