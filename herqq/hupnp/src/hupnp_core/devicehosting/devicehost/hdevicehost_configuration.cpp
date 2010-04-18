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

#include "hdevicehost_configuration.h"
#include "hdevicehost_configuration_p.h"

#include <QFile>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HDeviceConfigurationPrivate
 ******************************************************************************/
HDeviceConfigurationPrivate::HDeviceConfigurationPrivate() :
    m_pathToDeviceDescriptor(), m_cacheControlMaxAgeInSecs(1800),
    m_deviceCreator()
{
}

/*******************************************************************************
 * HDeviceConfiguration
 ******************************************************************************/
HDeviceConfiguration::HDeviceConfiguration() :
    h_ptr(new HDeviceConfigurationPrivate())
{
}

HDeviceConfiguration::~HDeviceConfiguration()
{
    delete h_ptr;
}

HDeviceConfiguration* HDeviceConfiguration::doClone() const
{
    return new HDeviceConfiguration();
}

HDeviceConfiguration* HDeviceConfiguration::clone() const
{
    HDeviceConfiguration* newClone = doClone();
    if (!newClone) { return 0; }

    *newClone->h_ptr = *h_ptr;

    return newClone;
}

QString HDeviceConfiguration::pathToDeviceDescription() const
{
    return h_ptr->m_pathToDeviceDescriptor;
}

bool HDeviceConfiguration::setPathToDeviceDescription(
    const QString& pathToDeviceDescriptor)
{
    if (!QFile::exists(pathToDeviceDescriptor))
    {
        return false;
    }

    h_ptr->m_pathToDeviceDescriptor = pathToDeviceDescriptor;
    return true;
}

void HDeviceConfiguration::setCacheControlMaxAge(qint32 maxAgeInSecs)
{
    static const qint32 max = 60*60*24;

    if (maxAgeInSecs < 5)
    {
        maxAgeInSecs = 5;
    }
    else if (maxAgeInSecs > max)
    {
        maxAgeInSecs = max;
    }

    h_ptr->m_cacheControlMaxAgeInSecs = maxAgeInSecs;
}

qint32 HDeviceConfiguration::cacheControlMaxAge() const
{
    return h_ptr->m_cacheControlMaxAgeInSecs;
}

HDeviceCreator HDeviceConfiguration::deviceCreator() const
{
    return h_ptr->m_deviceCreator;
}

bool HDeviceConfiguration::setDeviceCreator(HDeviceCreator deviceCreator)
{
    if (!deviceCreator)
    {
        return false;
    }

    h_ptr->m_deviceCreator = deviceCreator;
    return true;
}

bool HDeviceConfiguration::isValid() const
{
    return !h_ptr->m_pathToDeviceDescriptor.isEmpty() && h_ptr->m_deviceCreator;
}

/*******************************************************************************
 * HDeviceHostConfigurationPrivate
 ******************************************************************************/
HDeviceHostConfigurationPrivate::HDeviceHostConfigurationPrivate() :
    m_collection(), m_individualAdvertisementCount(2),
    m_subscriptionExpirationTimeout(0)
{
}

/*******************************************************************************
 * HDeviceHostConfiguration
 ******************************************************************************/
HDeviceHostConfiguration::HDeviceHostConfiguration() :
    h_ptr(new HDeviceHostConfigurationPrivate())
{
}

HDeviceHostConfiguration::HDeviceHostConfiguration(
    const HDeviceConfiguration& arg) :
        h_ptr(new HDeviceHostConfigurationPrivate())
{
    add(arg);
}

HDeviceHostConfiguration* HDeviceHostConfiguration::doClone() const
{
    return new HDeviceHostConfiguration();
}

HDeviceHostConfiguration* HDeviceHostConfiguration::clone() const
{
    HDeviceHostConfiguration* newClone = doClone();
    if (!newClone) { return 0; }

    foreach(const HDeviceConfiguration* arg, h_ptr->m_collection)
    {
        newClone->add(*arg);
    }

    newClone->h_ptr->m_individualAdvertisementCount =
        h_ptr->m_individualAdvertisementCount;

    return newClone;
}

HDeviceHostConfiguration::~HDeviceHostConfiguration()
{
    qDeleteAll(h_ptr->m_collection);
    delete h_ptr;
}

bool HDeviceHostConfiguration::add(const HDeviceConfiguration& arg)
{
    if (arg.isValid())
    {
        h_ptr->m_collection.push_back(arg.clone());
        return true;
    }

    return false;
}

QList<const HDeviceConfiguration*> HDeviceHostConfiguration::deviceConfigurations() const
{
    return h_ptr->m_collection;
}

qint32 HDeviceHostConfiguration::individualAdvertisementCount() const
{
    return h_ptr->m_individualAdvertisementCount;
}

void HDeviceHostConfiguration::setIndividualAdvertisementCount(qint32 arg)
{
    if (arg < 1)
    {
        arg = 1;
    }

    h_ptr->m_individualAdvertisementCount = arg;
}

qint32 HDeviceHostConfiguration::subscriptionExpirationTimeout() const
{
    return h_ptr->m_subscriptionExpirationTimeout;
}

void HDeviceHostConfiguration::setSubscriptionExpirationTimeout(qint32 arg)
{
    static const qint32 max = 60*60*24;

    if (arg > max)
    {
        arg = max;
    }

    h_ptr->m_subscriptionExpirationTimeout = arg;
}

bool HDeviceHostConfiguration::isEmpty() const
{
    return h_ptr->m_collection.isEmpty();
}

}
}
