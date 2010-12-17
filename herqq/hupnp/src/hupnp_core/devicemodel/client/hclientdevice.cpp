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

#include "hclientdevice.h"
#include "hclientdevice_p.h"
#include "hdefault_clientservice_p.h"
#include "hdefault_clientdevice_p.h"

#include "../../../utils/hlogger_p.h"
#include "../../general/hupnp_global_p.h"

#include "../../dataelements/hserviceid.h"
#include "../../dataelements/hdeviceinfo.h"
#include "../../dataelements/hserviceinfo.h"

#include <QtCore/QTimer>
#include <QtCore/QString>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HClientDevicePrivate
 ******************************************************************************/
HClientDevicePrivate::HClientDevicePrivate() :
    m_deviceInfo(0), m_embeddedDevices(), m_services(), m_parentDevice(0),
    q_ptr(0), m_locations(), m_deviceDescription()
{
}

HClientDevicePrivate::~HClientDevicePrivate()
{
    HLOG(H_AT, H_FUN);
}

/*******************************************************************************
 * HClientDevice
 ******************************************************************************/
HClientDevice::HClientDevice(
    const HDeviceInfo& info, HClientDevice* parentDev) :
        QObject(parentDev),
            h_ptr(new HClientDevicePrivate())
{
    h_ptr->m_parentDevice = parentDev;
    h_ptr->m_deviceInfo.reset(new HDeviceInfo(info));
    h_ptr->q_ptr = this;
}

HClientDevice::~HClientDevice()
{
    delete h_ptr;
}

HClientDevice* HClientDevice::parentDevice() const
{
    return h_ptr->m_parentDevice;
}

HClientDevice* HClientDevice::rootDevice() const
{
    HClientDevice* root = const_cast<HClientDevice*>(this);
    while(root->h_ptr->m_parentDevice)
    {
        root = root->h_ptr->m_parentDevice;
    }

    return root;
}

HClientService* HClientDevice::serviceById(const HServiceId& serviceId) const
{
    foreach(HDefaultClientService* sc, h_ptr->m_services)
    {
        if (sc->info().serviceId() == serviceId)
        {
            return sc;
        }
    }

    return 0;
}

HClientServices HClientDevice::services() const
{
    HClientServices retVal;
    foreach(HDefaultClientService* sc, h_ptr->m_services)
    {
        retVal.push_back(sc);
    }

    return retVal;
}

HClientServices HClientDevice::servicesByType(
    const HResourceType& type, HResourceType::VersionMatch versionMatch) const
{
    if (!type.isValid())
    {
        return HClientServices();
    }

    HClientServices retVal;
    foreach(HDefaultClientService* sc, h_ptr->m_services)
    {
        if (sc->info().serviceType().compare(type, versionMatch))
        {
            retVal.push_back(sc);
        }
    }

    return retVal;
}

HClientDevices HClientDevice::embeddedDevices() const
{
    HClientDevices retVal;
    foreach(HDefaultClientDevice* dev, h_ptr->m_embeddedDevices)
    {
        retVal.push_back(dev);
    }

    return retVal;
}

HClientDevices HClientDevice::embeddedDevicesByType(
    const HResourceType& type, HResourceType::VersionMatch versionMatch) const
{
    if (!type.isValid())
    {
        return HClientDevices();
    }

    HClientDevices retVal;
    foreach(HDefaultClientDevice* dev, h_ptr->m_embeddedDevices)
    {
        if (dev->info().deviceType().compare(type, versionMatch))
        {
            retVal.push_back(dev);
        }
    }

    return retVal;
}

const HDeviceInfo& HClientDevice::info() const
{
    return *h_ptr->m_deviceInfo;
}

QString HClientDevice::description() const
{
    return h_ptr->m_deviceDescription;
}

QList<QUrl> HClientDevice::locations(LocationUrlType urlType) const
{
    if (h_ptr->m_parentDevice)
    {
        // the root device "defines" the locations and they are the same for each
        // embedded device.
        return h_ptr->m_parentDevice->locations(urlType);
    }

    QList<QUrl> retVal;
    QList<QUrl>::const_iterator ci;
    for(ci = h_ptr->m_locations.begin(); ci != h_ptr->m_locations.end(); ++ci)
    {
        retVal.push_back(urlType == AbsoluteUrl ? *ci : extractBaseUrl(*ci));
    }

    return retVal;
}

/*******************************************************************************
 * HDefaultClientDevice
 ******************************************************************************/
HDefaultClientDevice::HDefaultClientDevice(
    const QString& description,
    const QList<QUrl>& locations,
    const HDeviceInfo& info,
    qint32 deviceTimeoutInSecs,
    HDefaultClientDevice* parentDev) :
        HClientDevice(info, parentDev),
            m_timedout(false),
            m_statusNotifier(new QTimer(this)),
            m_deviceStatus(new HDeviceStatus()),
            m_configId(0)
{
    h_ptr->m_deviceDescription = description;
    h_ptr->m_locations = locations;

    m_statusNotifier->setInterval(deviceTimeoutInSecs * 1000);
    bool ok = connect(
        m_statusNotifier.data(), SIGNAL(timeout()), this, SLOT(timeout_()));

    Q_ASSERT(ok); Q_UNUSED(ok)
}

void HDefaultClientDevice::setServices(
    const QList<HDefaultClientService*>& services)
{
    h_ptr->m_services = services;
}

void HDefaultClientDevice::setEmbeddedDevices(
    const QList<HDefaultClientDevice*>& devices)
{
    h_ptr->m_embeddedDevices = devices;
}

quint32 HDefaultClientDevice::deviceTimeoutInSecs() const
{
    return m_statusNotifier->interval() / 1000;
}

void HDefaultClientDevice::timeout_()
{
    HLOG(H_AT, H_FUN);

    m_timedout = true;
    stopStatusNotifier(HDefaultClientDevice::ThisOnly);

    emit statusTimeout(this);
}

void HDefaultClientDevice::startStatusNotifier(SearchCriteria searchCriteria)
{
    HLOG(H_AT, H_FUN);

    m_statusNotifier->start();
    if (searchCriteria & Services)
    {
        // TODO
    }
    if (searchCriteria & EmbeddedDevices)
    {
        foreach(HDefaultClientDevice* dc, h_ptr->m_embeddedDevices)
        {
            dc->startStatusNotifier(searchCriteria);
        }
    }

    m_timedout = false;
}

void HDefaultClientDevice::stopStatusNotifier(SearchCriteria searchCriteria)
{
    HLOG(H_AT, H_FUN);

    m_statusNotifier->stop();
    if (searchCriteria & Services)
    {
        // TODO
    }
    if (searchCriteria & EmbeddedDevices)
    {
        foreach(HDefaultClientDevice* dc, h_ptr->m_embeddedDevices)
        {
            dc->stopStatusNotifier(searchCriteria);
        }
    }
}

bool HDefaultClientDevice::isTimedout(SearchCriteria searchCriteria) const
{
    if (m_timedout)
    {
        return true;
    }

    if (searchCriteria & Services)
    {
        // TODO
    }

    if (searchCriteria & EmbeddedDevices)
    {
        foreach(HDefaultClientDevice* dc, h_ptr->m_embeddedDevices)
        {
            if (dc->isTimedout(searchCriteria))
            {
                return true;
            }
        }
    }

    return false;
}

namespace
{
bool shouldAdd(const HClientDevice* device, const QUrl& location)
{
    Q_ASSERT(!device->parentDevice());
    // embedded devices always query the parent device for locations

    QList<QUrl> locations = device->locations();
    QList<QUrl>::const_iterator ci = locations.constBegin();

    for(; ci != locations.constEnd(); ++ci)
    {
        if ((*ci).host() == location.host())
        {
            return false;
        }
    }

    return true;
}
}

bool HDefaultClientDevice::addLocation(const QUrl& location)
{
    if (shouldAdd(this, location))
    {
        h_ptr->m_locations.push_back(location);
        return true;
    }

    return false;
}

void HDefaultClientDevice::addLocations(const QList<QUrl>& locations)
{
    QList<QUrl>::const_iterator ci = locations.constBegin();
    for(; ci != locations.constEnd(); ++ci)
    {
        addLocation(*ci);
    }
}

}
}
