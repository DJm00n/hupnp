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

#include "./../../../utils/hlogger_p.h"

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

HDeviceConfigurationPrivate::~HDeviceConfigurationPrivate()
{
}

/*******************************************************************************
 * HDeviceConfiguration
 ******************************************************************************/
HDeviceConfiguration::HDeviceConfiguration() :
    h_ptr(new HDeviceConfigurationPrivate())
{
}

HDeviceConfiguration::HDeviceConfiguration(
    HDeviceConfigurationPrivate& dd) :
        h_ptr(&dd)
{
}

HDeviceConfiguration::~HDeviceConfiguration()
{
    HLOG(H_AT, H_FUN);

    delete h_ptr;
}

HDeviceConfiguration* HDeviceConfiguration::clone() const
{
    HLOG(H_AT, H_FUN);

    HDeviceConfiguration* clone =
        new HDeviceConfiguration(
            *new HDeviceConfigurationPrivate(*h_ptr));

    return clone;
}

QString HDeviceConfiguration::pathToDeviceDescription() const
{
    return h_ptr->m_pathToDeviceDescriptor;
}

bool HDeviceConfiguration::setPathToDeviceDescription(
    const QString& pathToDeviceDescriptor)
{
    HLOG(H_AT, H_FUN);

    if (!QFile::exists(pathToDeviceDescriptor))
    {
        return false;
    }

    h_ptr->m_pathToDeviceDescriptor = pathToDeviceDescriptor;
    return true;
}

void HDeviceConfiguration::setCacheControlMaxAge(quint32 maxAgeInSecs)
{
    HLOG(H_AT, H_FUN);

    if (maxAgeInSecs < 5)
    {
        maxAgeInSecs = 5;
    }
    else if (maxAgeInSecs > 60*60*24)
    {
        HLOG_WARN(QObject::tr(
            "The specified max age [%1] is too large. Defaulting to a day.").
                arg(QString::number(maxAgeInSecs)));

        maxAgeInSecs = 60*60*24; // a day
    }

    h_ptr->m_cacheControlMaxAgeInSecs = maxAgeInSecs;
}

quint32 HDeviceConfiguration::cacheControlMaxAge() const
{
    return h_ptr->m_cacheControlMaxAgeInSecs;
}

HDeviceCreator HDeviceConfiguration::deviceCreator() const
{
    return h_ptr->m_deviceCreator;
}

void HDeviceConfiguration::setDeviceCreator(
    HDeviceCreator deviceCreator)
{
    h_ptr->m_deviceCreator = deviceCreator;
}

bool HDeviceConfiguration::isValid() const
{
    return !h_ptr->m_pathToDeviceDescriptor.isEmpty() && h_ptr->m_deviceCreator;
}

/*******************************************************************************
 * HDeviceHostConfigurationPrivate
 ******************************************************************************/
HDeviceHostConfigurationPrivate::HDeviceHostConfigurationPrivate() :
    m_collection(), m_individualAdvertisementCount(2)
{
}

/*******************************************************************************
 * HDeviceHostConfiguration
 ******************************************************************************/
HDeviceHostConfiguration::HDeviceHostConfiguration() :
    h_ptr(new HDeviceHostConfigurationPrivate())
{
    HLOG(H_AT, H_FUN);
}

HDeviceHostConfiguration::HDeviceHostConfiguration(
    const HDeviceConfiguration& arg) :
        h_ptr(new HDeviceHostConfigurationPrivate())
{
    HLOG(H_AT, H_FUN);
    add(arg);
}

HDeviceHostConfiguration::HDeviceHostConfiguration(
    const HDeviceHostConfiguration& other) :
        h_ptr(new HDeviceHostConfigurationPrivate())
{
    HLOG(H_AT, H_FUN);
    foreach(HDeviceConfiguration* arg, other.h_ptr->m_collection)
    {
        add(*arg);
    }
}

HDeviceHostConfiguration& HDeviceHostConfiguration::operator=(
    const HDeviceHostConfiguration& other)
{
    HLOG(H_AT, H_FUN);
    qDeleteAll(h_ptr->m_collection);
    h_ptr->m_collection.clear();

    foreach(HDeviceConfiguration* arg, other.h_ptr->m_collection)
    {
        add(*arg);
    }

    return *this;
}

HDeviceHostConfiguration::~HDeviceHostConfiguration()
{
    HLOG(H_AT, H_FUN);

    qDeleteAll(h_ptr->m_collection);
    delete h_ptr;
}

bool HDeviceHostConfiguration::add(const HDeviceConfiguration& arg)
{
    HLOG(H_AT, H_FUN);

    if (arg.isValid())
    {
        h_ptr->m_collection.push_back(arg.clone());
        return true;
    }

    return false;
}

QList<HDeviceConfiguration*> HDeviceHostConfiguration::deviceConfigurations() const
{
    return h_ptr->m_collection;
}

quint32 HDeviceHostConfiguration::individualAdvertisementCount() const
{
    return h_ptr->m_individualAdvertisementCount;
}

void HDeviceHostConfiguration::setIndividualAdvertisementCount(quint32 arg)
{
    h_ptr->m_individualAdvertisementCount = arg;
}

bool HDeviceHostConfiguration::isEmpty() const
{
    return h_ptr->m_collection.isEmpty();
}

}
}
