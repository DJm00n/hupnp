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

#include <QtCore/QSet>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HDeviceSetupPrivate
 ******************************************************************************/
class HDeviceSetupPrivate
{
public:

    HResourceType m_deviceType;
    HDevice* m_device;
    qint32 m_version;
    HInclusionRequirement m_inclusionReq;

    HDeviceSetupPrivate() :
        m_deviceType(), m_device(0), m_version(0),
        m_inclusionReq(InclusionRequirementUnknown)
    {
    }

    ~HDeviceSetupPrivate()
    {
        delete m_device;
    }
};

/*******************************************************************************
 * HDeviceSetup
 ******************************************************************************/
HDeviceSetup::HDeviceSetup() :
    h_ptr(new HDeviceSetupPrivate())
{
}

HDeviceSetup::HDeviceSetup(
    const HResourceType& type, HInclusionRequirement incReq) :
        h_ptr(new HDeviceSetupPrivate())
{
    h_ptr->m_deviceType = type;
    h_ptr->m_version = 1;
    h_ptr->m_inclusionReq = incReq;
}

HDeviceSetup::HDeviceSetup(
    const HResourceType& type, qint32 version, HInclusionRequirement incReq) :
        h_ptr(new HDeviceSetupPrivate())
{
    h_ptr->m_deviceType = type;
    h_ptr->m_version = version;
    h_ptr->m_inclusionReq = incReq;
}

HDeviceSetup::HDeviceSetup(
    const HResourceType& type, HDevice* device, HInclusionRequirement ireq) :
        h_ptr(new HDeviceSetupPrivate())
{
    h_ptr->m_deviceType = type;
    h_ptr->m_device = device;
    h_ptr->m_version = 1;
    h_ptr->m_inclusionReq = ireq;
}

HDeviceSetup::HDeviceSetup(
    const HResourceType& type, HDevice* device, qint32 version,
    HInclusionRequirement ireq) :
        h_ptr(new HDeviceSetupPrivate())
{
    h_ptr->m_deviceType = type;
    h_ptr->m_device = device;
    h_ptr->m_version = version;
    h_ptr->m_inclusionReq = ireq;
}

HDeviceSetup::~HDeviceSetup()
{
    delete h_ptr;
}

const HResourceType& HDeviceSetup::deviceType() const
{
    return h_ptr->m_deviceType;
}

HDevice* HDeviceSetup::device() const
{
    return h_ptr->m_device;
}

HInclusionRequirement HDeviceSetup::inclusionRequirement() const
{
    return h_ptr->m_inclusionReq;
}

bool HDeviceSetup::isValid() const
{
    return h_ptr->m_deviceType.isValid() &&
           h_ptr->m_version > 0 &&
           h_ptr->m_inclusionReq != InclusionRequirementUnknown;
}

qint32 HDeviceSetup::version() const
{
    return h_ptr->m_version;
}

void HDeviceSetup::setInclusionRequirement(HInclusionRequirement arg)
{
    h_ptr->m_inclusionReq = arg;
}

void HDeviceSetup::setDeviceType(const HResourceType& arg)
{
    h_ptr->m_deviceType = arg;
}

void HDeviceSetup::setVersion(qint32 version)
{
    h_ptr->m_version = version;
}

HDevice* HDeviceSetup::takeDevice()
{
    HDevice* retVal = h_ptr->m_device;
    h_ptr->m_device = 0;
    return retVal;
}

void HDeviceSetup::setDevice(HDevice* arg)
{
    if (h_ptr->m_device)
    {
        delete h_ptr->m_device;
    }

    h_ptr->m_device = arg;
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
