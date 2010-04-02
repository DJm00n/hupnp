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

#include "hcontrolpoint_configuration.h"
#include "hcontrolpoint_configuration_p.h"

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HControlPointConfigurationPrivate
 ******************************************************************************/
HControlPointConfigurationPrivate::HControlPointConfigurationPrivate() :
    m_deviceCreator(), m_subscribeToEvents(true),
    m_desiredSubscriptionTimeout(1800)
{
}

HControlPointConfigurationPrivate::~HControlPointConfigurationPrivate()
{
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

HControlPointConfiguration* HControlPointConfiguration::clone() const
{
    HControlPointConfiguration* clone =
        new HControlPointConfiguration(
            *new HControlPointConfigurationPrivate(*h_ptr));

    return clone;
}

HDeviceCreator HControlPointConfiguration::deviceCreator() const
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

void HControlPointConfiguration::setDeviceCreator(
    HDeviceCreator deviceCreator)
{
    h_ptr->m_deviceCreator = deviceCreator;
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

}
}
