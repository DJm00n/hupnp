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

#include "hcontrolpoint_configuration.h"
#include "hcontrolpoint_configuration_p.h"

#include "../../general/hupnp_global_p.h"
#include "../../devicemodel/hdeviceproxy.h"
#include "../../../utils/hmisc_utils_p.h"

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HProxyCreator
 ******************************************************************************/
HProxyCreator::HProxyCreator()
{
}

HDeviceProxy* HProxyCreator::operator()(const HDeviceInfo&) const
{
    return new HDeviceProxy();
}

HServiceProxy* HProxyCreator::operator()(const HResourceType&) const
{
    return new HServiceProxy();
}

/*******************************************************************************
 * HControlPointConfigurationPrivate
 ******************************************************************************/
HControlPointConfigurationPrivate::HControlPointConfigurationPrivate() :
    m_deviceCreator(HProxyCreator()),
    m_subscribeToEvents(true),
    m_desiredSubscriptionTimeout(1800),
    m_autoDiscovery(true),
    m_networkAddresses()
{
    QHostAddress ha = findBindableHostAddress();
    m_networkAddresses.append(ha);
}

HControlPointConfigurationPrivate::~HControlPointConfigurationPrivate()
{
}

HControlPointConfigurationPrivate* HControlPointConfigurationPrivate::clone() const
{
    HControlPointConfigurationPrivate* newObj =
        new HControlPointConfigurationPrivate();

    newObj->m_deviceCreator = m_deviceCreator;
    newObj->m_subscribeToEvents = m_subscribeToEvents;
    newObj->m_desiredSubscriptionTimeout = m_desiredSubscriptionTimeout;
    newObj->m_autoDiscovery = m_autoDiscovery;
    newObj->m_networkAddresses = m_networkAddresses;

    return newObj;
}

/*******************************************************************************
 * HControlPointConfiguration
 ******************************************************************************/
HControlPointConfiguration::HControlPointConfiguration() :
    h_ptr(new HControlPointConfigurationPrivate())
{
}

HControlPointConfiguration::HControlPointConfiguration(
    HControlPointConfigurationPrivate& dd) :
        h_ptr(&dd)
{
}

HControlPointConfiguration::~HControlPointConfiguration()
{
    delete h_ptr;
}

HControlPointConfiguration* HControlPointConfiguration::doClone() const
{
    return new HControlPointConfiguration();
}

HControlPointConfiguration* HControlPointConfiguration::clone() const
{
    HControlPointConfiguration* newClone = doClone();
    if (!newClone) { return 0; }

    newClone->h_ptr = h_ptr->clone();

    return newClone;
}

HDeviceProxyCreator HControlPointConfiguration::deviceCreator() const
{
    return h_ptr->m_deviceCreator;
}

bool HControlPointConfiguration::subscribeToEvents() const
{
    return h_ptr->m_subscribeToEvents;
}

qint32 HControlPointConfiguration::desiredSubscriptionTimeout() const
{
    return h_ptr->m_desiredSubscriptionTimeout;
}

bool HControlPointConfiguration::autoDiscovery() const
{
    return h_ptr->m_autoDiscovery;
}

QList<QHostAddress> HControlPointConfiguration::networkAddressesToUse() const
{
    return h_ptr->m_networkAddresses;
}

bool HControlPointConfiguration::setDeviceCreator(
    const HDeviceProxyCreator& deviceCreator)
{
    if (!deviceCreator)
    {
        return false;
    }

    h_ptr->m_deviceCreator = deviceCreator;
    return true;
}

void HControlPointConfiguration::setSubscribeToEvents(bool arg)
{
    h_ptr->m_subscribeToEvents = arg;
}

void HControlPointConfiguration::setDesiredSubscriptionTimeout(qint32 arg)
{
    static const qint32 def = 60*30;

    if (arg <= 0)
    {
        arg = def;
    }

    h_ptr->m_desiredSubscriptionTimeout = arg;
}

void HControlPointConfiguration::setAutoDiscovery(bool arg)
{
    h_ptr->m_autoDiscovery = arg;
}

bool HControlPointConfiguration::setNetworkAddressesToUse(
    const QList<QHostAddress>& addresses)
{
    if (!HSysInfo::instance().areLocalAddresses(addresses))
    {
        return false;
    }

    h_ptr->m_networkAddresses = addresses;
    return true;
}

}
}
