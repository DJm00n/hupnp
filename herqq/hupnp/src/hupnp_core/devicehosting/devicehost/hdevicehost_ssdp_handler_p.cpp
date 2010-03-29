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

#include "hdevicehost_ssdp_handler_p.h"

#include "./../../ssdp/husn.h"
#include "./../../ssdp/hssdp_p.h"
#include "./../../ssdp/hresource_identifier.h"

#include "./../../general/hupnp_global_p.h"

#include "./../../dataelements/hudn.h"
#include "./../../dataelements/hdeviceinfo.h"
#include "./../../dataelements/hproduct_tokens.h"

#include "./../../../utils/hlogger_p.h"
#include "./../../../utils/hsysutils_p.h"

#include <QDateTime>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * DeviceHostSsdpHandler
 ******************************************************************************/
DeviceHostSsdpHandler::DeviceHostSsdpHandler(
    const QByteArray& loggingIdentifier, DeviceStorage& ds, QObject* parent) :
        HSsdp(parent), m_deviceStorage(ds)
{
    Q_ASSERT(parent);
    h_ptr->m_loggingIdentifier = loggingIdentifier;
}

DeviceHostSsdpHandler::~DeviceHostSsdpHandler()
{
}

void DeviceHostSsdpHandler::processSearchRequest_specificDevice(
    const HDiscoveryRequest& req, const HEndpoint& source,
    QList<HDiscoveryResponse>* responses)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    QUuid uuid = req.searchTarget().deviceUuid();
    if (uuid.isNull())
    {
        HLOG_DBG("Invalid device-UUID");
        return;
    }

    HDeviceController* device = m_deviceStorage.searchDeviceByUdn(uuid);
    if (!device)
    {
        HLOG_DBG(QString("No device with the specified UUID: [%1]").arg(
            uuid.toString()));

        return;
    }

    QUrl location;
    if (!m_deviceStorage.searchValidLocation(device->m_device, source, &location))
    {
        HLOG_DBG(QString(
            "Found a device with uuid: [%1], but it is not "
            "available on the interface that has address: [%2]").arg(
                uuid.toString(), source.toString()));

        return;
    }

    HUsn usn(device->m_device->deviceInfo().udn(), req.searchTarget());

    responses->push_back(
        HDiscoveryResponse(
            device->deviceTimeoutInSecs() * 2,
            QDateTime::currentDateTime(),
            location,
            herqqProductTokens(),
            usn,
            device->deviceStatus()->bootId(),
            device->deviceStatus()->configId()));
}

void DeviceHostSsdpHandler::processSearchRequest_deviceType(
    const HDiscoveryRequest& req, const HEndpoint& source,
    QList<HDiscoveryResponse>* responses)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    QList<HDeviceController*> foundDevices =
        m_deviceStorage.searchDevicesByDeviceType(
            req.searchTarget().resourceType(), false);

    if (!foundDevices.size())
    {
        HLOG_DBG(QString("No devices match the specified type: [%1]").arg(
            req.searchTarget().resourceType().toString()));

        return;
    }

    foreach(HDeviceController* device, foundDevices)
    {
        QUrl location;
        if (!m_deviceStorage.searchValidLocation(
            device->m_device, source, &location))
        {
            HLOG_DBG(QString(
                "Found a matching device, but it is not "
                "available on the interface that has address: [%1]").arg(
                    source.toString()));

            continue;
        }

        HUsn usn(device->m_device->deviceInfo().udn(), req.searchTarget());

        responses->push_back(
            HDiscoveryResponse(
                device->deviceTimeoutInSecs() * 2,
                QDateTime::currentDateTime(),
                location,
                herqqProductTokens(),
                usn,
                device->deviceStatus()->bootId(),
                device->deviceStatus()->configId()));
    }
}

void DeviceHostSsdpHandler::processSearchRequest_serviceType(
    const HDiscoveryRequest& req, const HEndpoint& source,
    QList<HDiscoveryResponse>* responses)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    QList<HServiceController*> foundServices =
        m_deviceStorage.searchServicesByServiceType(
            req.searchTarget().resourceType(), false);

    if (!foundServices.size())
    {
       HLOG_DBG(QString(
           "No services match the specified type: [%1]").arg(
               req.searchTarget().resourceType().toString()));

       return;
    }

    foreach(HServiceController* service, foundServices)
    {
        const HDevice* device = service->m_service->parentDevice();
        Q_ASSERT(device);

        QUrl location;
        if (!m_deviceStorage.searchValidLocation(device, source, &location))
        {
            HLOG_DBG(QString(
                "Found a matching device, but it is not "
                "available on the interface that has address: [%1]").arg(
                    source.toString()));

            continue;
        }

        HUsn usn(device->deviceInfo().udn(), req.searchTarget());

        HDeviceController* dc =
            m_deviceStorage.searchDeviceByUdn(device->deviceInfo().udn());

        Q_ASSERT(dc);

        responses->push_back(
            HDiscoveryResponse(
                dc->deviceTimeoutInSecs() * 2,
                QDateTime::currentDateTime(),
                location,
                herqqProductTokens(),
                usn,
                dc->deviceStatus()->bootId(),
                dc->deviceStatus()->configId()));
    }
}

void DeviceHostSsdpHandler::processSearchRequest(
    HDeviceController* device, const QUrl& location,
    QList<HDiscoveryResponse>* responses)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    Q_ASSERT(device);

    HDeviceInfo deviceInfo = device->m_device->deviceInfo();

    HProductTokens pt = herqqProductTokens();

    HUsn usn(deviceInfo.udn());

    // device UDN
    responses->push_back(
        HDiscoveryResponse(
            device->deviceTimeoutInSecs() * 2,
            QDateTime::currentDateTime(), location, pt, usn,
            device->deviceStatus()->bootId(),
            device->deviceStatus()->configId()));

    usn.setResource(HResourceIdentifier(deviceInfo.deviceType()));

    // device type
    responses->push_back(
        HDiscoveryResponse(
            device->deviceTimeoutInSecs() * 2,
            QDateTime::currentDateTime(), location, pt, usn,
            device->deviceStatus()->bootId(),
            device->deviceStatus()->configId()));

    const QList<HServiceController*>* services = device->services();
    foreach(HServiceController* service, *services)
    {
        usn.setResource(
            HResourceIdentifier(service->m_service->serviceType().toString()));

        responses->push_back(
            HDiscoveryResponse(
                device->deviceTimeoutInSecs() * 2,
                QDateTime::currentDateTime(), location, pt, usn,
                device->deviceStatus()->bootId(),
                device->deviceStatus()->configId()));
    }

    const QList<HDeviceController*>* devices = device->embeddedDevices();
    foreach(HDeviceController* embeddedDevice, *devices)
    {
        processSearchRequest(embeddedDevice, location, responses);
    }
}

void DeviceHostSsdpHandler::processSearchRequest_AllDevices(
    const HDiscoveryRequest& /*req*/, const HEndpoint& source,
    QList<HDiscoveryResponse>* responses)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    Q_ASSERT(responses);

    HProductTokens pt = herqqProductTokens();

    QList<HDeviceController*> rootDevices =
        m_deviceStorage.rootDeviceControllers();

    HLOG_DBG(QString(
        "Received search request for all devices from: [%1].").arg(
            source.hostAddress().toString()));

    foreach(HDeviceController* rootDevice, rootDevices)
    {
        QUrl location;
        if (!m_deviceStorage.searchValidLocation(
                rootDevice->m_device, source, &location))
        {
            HLOG_DBG(QString(
                "Found a device, but it is not "
                "available on the interface that has address: [%1]").arg(
                    source.toString()));

            continue;
        }

        HUsn usn(rootDevice->m_device->deviceInfo().udn(),
                 HResourceIdentifier::getRootDeviceIdentifier());

        responses->push_back(
            HDiscoveryResponse(
                rootDevice->deviceTimeoutInSecs() * 2,
                QDateTime::currentDateTime(),
                location,
                pt,
                usn,
                rootDevice->deviceStatus()->bootId(),
                rootDevice->deviceStatus()->configId()));

        processSearchRequest(rootDevice, location, responses);

        const QList<HDeviceController*>* devices = rootDevice->embeddedDevices();
        foreach(HDeviceController* embeddedDevice, *devices)
        {
            if (!m_deviceStorage.searchValidLocation(
                embeddedDevice->m_device, source, &location))
            {
                // highly uncommon, but possible; the root device is "active" on the network interface
                // to which the request came, but at least one of its embedded
                // devices is not.

                HLOG_DBG(QString(
                    "Skipping an embedded device that is not "
                    "available on the interface that has address: [%1]").arg(
                        source.toString()));

                continue;
            }

            processSearchRequest(embeddedDevice, location, responses);
        }
    }
}

void DeviceHostSsdpHandler::processSearchRequest_RootDevice(
    const HDiscoveryRequest& /*req*/, const HEndpoint& source,
    QList<HDiscoveryResponse>* responses)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    Q_ASSERT(responses);

    QList<HDeviceController*> rootDevices =
        m_deviceStorage.rootDeviceControllers();

    HLOG_DBG(QString(
        "Received search request for root devices from: [%1].").arg(
            source.toString()));

    foreach(HDeviceController* rootDevice, rootDevices)
    {
        QUrl location;
        if (!m_deviceStorage.searchValidLocation(
            rootDevice->m_device, source, &location))
        {
            HLOG_DBG(QString(
                "Found a root device, but it is not "
                "available on the interface that has address: [%1]").arg(
                    source.hostAddress().toString()));

            continue;
        }

        HUsn usn(rootDevice->m_device->deviceInfo().udn(),
                 HResourceIdentifier::getRootDeviceIdentifier());

        responses->push_back(
            HDiscoveryResponse(
                rootDevice->deviceTimeoutInSecs() * 2,
                QDateTime::currentDateTime(),
                location, herqqProductTokens(), usn,
                rootDevice->deviceStatus()->bootId(),
                rootDevice->deviceStatus()->configId()));
    }
}

bool DeviceHostSsdpHandler::incomingDiscoveryRequest(
    const HDiscoveryRequest& msg, const HEndpoint& source,
    const HEndpoint& destination)
{
    HLOG(H_AT, H_FUN);

    QList<HDiscoveryResponse> responses;
    switch (msg.searchTarget().type())
    {
        case HResourceIdentifier::AllDevices:
            processSearchRequest_AllDevices(msg, source, &responses);
            break;

        case HResourceIdentifier::RootDevice:
            processSearchRequest_RootDevice(msg, source, &responses);
            break;

        case HResourceIdentifier::SpecificDevice:
            processSearchRequest_specificDevice(msg, source, &responses);
            break;

        case HResourceIdentifier::StandardDeviceType:
        case HResourceIdentifier::VendorSpecifiedDeviceType:
            processSearchRequest_deviceType(msg, source, &responses);
            break;

        case HResourceIdentifier::StandardServiceType:
        case HResourceIdentifier::VendorSpecifiedServiceType:
            processSearchRequest_serviceType(msg, source, &responses);
            break;

        default:
            return true;
    }

    if (destination.isMulticast())
    {
        HSysUtils::msleep((rand() % msg.mx()) * 1000); // TODO
    }

    foreach (HDiscoveryResponse resp, responses)
    {
        sendDiscoveryResponse(source, resp);
    }

    return true;
}

bool DeviceHostSsdpHandler::incomingDiscoveryResponse(
    const Herqq::Upnp::HDiscoveryResponse& /*msg*/,
    const HEndpoint& /*source*/)
{
    return true;
}

bool DeviceHostSsdpHandler::incomingDeviceAvailableAnnouncement(
    const Herqq::Upnp::HResourceAvailable& /*msg*/)
{
    return true;
}

bool DeviceHostSsdpHandler::incomingDeviceUnavailableAnnouncement(
    const Herqq::Upnp::HResourceUnavailable& /*msg*/)
{
    return true;
}

}
}
