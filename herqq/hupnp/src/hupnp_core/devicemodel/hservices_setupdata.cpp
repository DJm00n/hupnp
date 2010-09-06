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

#include "hservices_setupdata.h"
#include "hservice.h"

#include <QtCore/QSet>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HServiceSetup
 ******************************************************************************/
HServiceSetup::HServiceSetup() :
    m_serviceId(), m_serviceType(), m_service(0), m_version(0),
    m_inclusionReq(InclusionRequirementUnknown)
{
}

HServiceSetup::HServiceSetup(
    const HServiceId& id, const HResourceType& serviceType,
    HInclusionRequirement ireq) :
        m_serviceId(id),
        m_serviceType(serviceType),
        m_service(0),
        m_version(1),
        m_inclusionReq(ireq)
{
}

HServiceSetup::HServiceSetup(
    const HServiceId& id, const HResourceType& serviceType, qint32 version,
    HInclusionRequirement ireq) :
        m_serviceId(id),
        m_serviceType(serviceType),
        m_service(0),
        m_version(version),
        m_inclusionReq(ireq)
{
}

HServiceSetup::HServiceSetup(
    const HServiceId& id, const HResourceType& serviceType,
    HService* service, HInclusionRequirement ireq) :
        m_serviceId(id),
        m_serviceType(serviceType),
        m_service(service),
        m_version(1),
        m_inclusionReq(ireq)
{
}

HServiceSetup::HServiceSetup(
    const HServiceId& id, const HResourceType& serviceType,
    HService* service, qint32 version, HInclusionRequirement ireq) :
        m_serviceId(id),
        m_serviceType(serviceType),
        m_service(service),
        m_version(version),
        m_inclusionReq(ireq)
{
}

HServiceSetup::~HServiceSetup()
{
    delete m_service;
}

bool HServiceSetup::isValid(HValidityCheckLevel checkLevel) const
{
    return m_serviceId.isValid(checkLevel) &&
           m_serviceType.isValid() &&
           m_version > 0 &&
           m_inclusionReq != InclusionRequirementUnknown;
}

/*******************************************************************************
 * HServicesSetupData
 ******************************************************************************/
HServicesSetupData::HServicesSetupData() :
    m_serviceSetupInfos()
{
}

HServicesSetupData::~HServicesSetupData()
{
    qDeleteAll(m_serviceSetupInfos);
}

bool HServicesSetupData::insert(HServiceSetup* setupInfo)
{
    if (!setupInfo || !setupInfo->isValid(StrictChecks))
    {
        delete setupInfo;
        return false;
    }

    const HServiceId& id = setupInfo->serviceId();
    if (m_serviceSetupInfos.contains(id))
    {
        delete setupInfo;
        return false;
    }

    m_serviceSetupInfos.insert(id, setupInfo);
    return true;
}

bool HServicesSetupData::remove(const HServiceId& serviceId)
{
    if (m_serviceSetupInfos.contains(serviceId))
    {
        delete m_serviceSetupInfos.value(serviceId);
        m_serviceSetupInfos.remove(serviceId);
        return true;
    }

    return false;
}

HServiceSetup* HServicesSetupData::get(const HServiceId& serviceId) const
{
    return m_serviceSetupInfos.value(serviceId);
}

HServiceSetup* HServicesSetupData::take(const HServiceId& serviceId)
{
    HServiceSetup* retVal = m_serviceSetupInfos.value(serviceId);
    if (retVal)
    {
        m_serviceSetupInfos.remove(serviceId);
    }
    return retVal;
}

bool HServicesSetupData::setService(
    const HServiceId& serviceId, HService* service)
{
    if (!m_serviceSetupInfos.contains(serviceId))
    {
        return false;
    }

    HServiceSetup* info = m_serviceSetupInfos.value(serviceId);
    info->setService(service);
    return true;
}

bool HServicesSetupData::contains(const HServiceId& id) const
{
    return m_serviceSetupInfos.contains(id);
}

QSet<HServiceId> HServicesSetupData::serviceIds() const
{
    return m_serviceSetupInfos.keys().toSet();
}

qint32 HServicesSetupData::size() const
{
    return m_serviceSetupInfos.size();
}

bool HServicesSetupData::isEmpty() const
{
    return m_serviceSetupInfos.isEmpty();
}

}
}
