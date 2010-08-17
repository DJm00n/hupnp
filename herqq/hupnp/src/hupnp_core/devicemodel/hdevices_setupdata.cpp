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

#include "hdevices_setupdata.h"
#include "hdevice.h"

#include <QSet>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HDeviceSetup
 ******************************************************************************/
HDeviceSetup::HDeviceSetup() :
    m_deviceType(), m_device(0),
    m_version(0),
    m_inclusionReq(InclusionRequirementUnknown)
{
}

HDeviceSetup::HDeviceSetup(
    const HResourceType& type, HInclusionRequirement incReq) :
        m_deviceType(type), m_device(0), m_version(1), m_inclusionReq(incReq)
{
}

HDeviceSetup::HDeviceSetup(
    const HResourceType& type, qint32 version, HInclusionRequirement incReq) :
        m_deviceType(type), m_device(0), m_version(version),
        m_inclusionReq(incReq)
{
}

HDeviceSetup::HDeviceSetup(
    const HResourceType& type, HDevice* device, HInclusionRequirement ireq) :
        m_deviceType(type), m_device(device), m_version(1), m_inclusionReq(ireq)
{
}

HDeviceSetup::HDeviceSetup(
    const HResourceType& type, HDevice* device, qint32 version,
    HInclusionRequirement ireq) :
        m_deviceType(type), m_device(device), m_version(version),
        m_inclusionReq(ireq)
{
}

HDeviceSetup::~HDeviceSetup()
{
    delete m_device;
}

void HDeviceSetup::setDevice(HDevice* arg)
{
    if (m_device)
    {
        delete m_device;
    }

    m_device = arg;
}

/*******************************************************************************
 * HDevicesSetupData
 ******************************************************************************/
HDevicesSetupData::HDevicesSetupData() :
    m_deviceSetupInfos()
{
}

HDevicesSetupData::~HDevicesSetupData()
{
    qDeleteAll(m_deviceSetupInfos);
}

bool HDevicesSetupData::insert(HDeviceSetup* setupInfo)
{
    if (!setupInfo || !setupInfo->isValid())
    {
        delete setupInfo;
        return false;
    }

    HResourceType id = setupInfo->deviceType();
    if (m_deviceSetupInfos.contains(id))
    {
        delete setupInfo;
        return false;
    }

    m_deviceSetupInfos.insert(id, setupInfo);
    return true;
}

bool HDevicesSetupData::remove(const HResourceType& deviceType)
{
    if (m_deviceSetupInfos.contains(deviceType))
    {
        delete m_deviceSetupInfos.value(deviceType);
        m_deviceSetupInfos.remove(deviceType);
        return true;
    }

    return false;
}

HDeviceSetup* HDevicesSetupData::get(const HResourceType& deviceType) const
{
    return m_deviceSetupInfos.value(deviceType);
}

HDeviceSetup* HDevicesSetupData::take(const HResourceType& deviceType)
{
    HDeviceSetup* retVal = m_deviceSetupInfos.value(deviceType);
    if (retVal)
    {
        m_deviceSetupInfos.remove(deviceType);
    }
    return retVal;
}

bool HDevicesSetupData::setDevice(
    const HResourceType& deviceType, HDevice* device)
{
    if (!m_deviceSetupInfos.contains(deviceType))
    {
        return false;
    }

    HDeviceSetup* info = m_deviceSetupInfos.value(deviceType);
    info->setDevice(device);
    return true;
}

bool HDevicesSetupData::contains(const HResourceType& id) const
{
    return m_deviceSetupInfos.contains(id);
}

QSet<HResourceType> HDevicesSetupData::deviceTypes() const
{
    return m_deviceSetupInfos.keys().toSet();
}

qint32 HDevicesSetupData::size() const
{
    return m_deviceSetupInfos.size();
}

bool HDevicesSetupData::isEmpty() const
{
    return m_deviceSetupInfos.isEmpty();
}

}
}
