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

#ifndef HPRESENCE_ANNOUNCER_H_
#define HPRESENCE_ANNOUNCER_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../../general/hupnp_global_p.h"

#include "../../devicemodel/hdevice.h"
#include "../../devicemodel/hdevice_p.h"

#include "../../devicemodel/hservice.h"
#include "../../devicemodel/hservice_p.h"

#include "../../socket/hendpoint.h"

#include "../../ssdp/hssdp.h"
#include "../../ssdp/hdiscovery_messages.h"

#include "../../dataelements/hudn.h"
#include "../../dataelements/hdeviceinfo.h"
#include "../../dataelements/hdiscoverytype.h"
#include "../../dataelements/hproduct_tokens.h"

#include <QUrl>

namespace Herqq
{

namespace Upnp
{

//
//
//
class Announcement
{

protected:

    HDeviceController* m_device;
    HDiscoveryType m_usn;
    QUrl m_location;

public:

    Announcement()
    {
    }

    Announcement(
        HDeviceController* device, const HDiscoveryType& usn,
        const QUrl& location) :
            m_device(device), m_usn(usn), m_location(location)
    {
        Q_ASSERT(m_device);
        Q_ASSERT(m_usn.type() != HDiscoveryType::Undefined);
        Q_ASSERT(m_location.isValid() && !m_location.isEmpty());
    }

    virtual ~Announcement()
    {
    }
};

//
//
//
class ResourceAvailableAnnouncement :
    private Announcement
{
public:

    ResourceAvailableAnnouncement()
    {
    }

    ResourceAvailableAnnouncement(
        HDeviceController* device, const HDiscoveryType& usn,
        const QUrl& location) :
            Announcement(device, usn, location)
    {
    }

    HResourceAvailable operator()() const
    {
        HProductTokens pt = HSysInfo::instance().herqqProductTokens();

        return HResourceAvailable(
            m_device->deviceTimeoutInSecs() * 2,
            m_location,
            pt,
            m_usn,
            m_device->deviceStatus()->bootId(),
            m_device->deviceStatus()->configId());
    }
};

//
//
//
class ResourceUnavailableAnnouncement :
    private Announcement
{
public:

    ResourceUnavailableAnnouncement()
    {
    }

    ResourceUnavailableAnnouncement(
        HDeviceController* device, const HDiscoveryType& usn,
        const QUrl& location) :
            Announcement(device, usn, location)
    {
    }

    HResourceUnavailable operator()() const
    {
        return HResourceUnavailable(
            m_usn,
            m_device->deviceStatus()->bootId(),
            m_device->deviceStatus()->configId());
    }
};

//
//
//
class PresenceAnnouncer
{
private:

    QList<DeviceHostSsdpHandler*> m_ssdps;
    quint32 m_advertisementCount;

public:

    PresenceAnnouncer(
        const QList<DeviceHostSsdpHandler*>& ssdps, quint32 advertisementCount) :
            m_ssdps(ssdps), m_advertisementCount(advertisementCount)
    {
        Q_ASSERT(m_advertisementCount > 0);
    }

    ~PresenceAnnouncer()
    {
    }

    template<typename AnnouncementType>
    void announce(const QList<HDeviceController*>& rootDevices)
    {
        QList<AnnouncementType> announcements;

        foreach(HDeviceController* rootDevice, rootDevices)
        {
            createAnnouncementMessagesForRootDevice(rootDevice, announcements);
        }

        sendAnnouncements(announcements);
    }

    template<typename AnnouncementType>
    void createAnnouncementMessagesForRootDevice(
        HDeviceController* rootDevice, QList<AnnouncementType>& announcements)
    {
        QList<QUrl> locations = rootDevice->m_device->locations();
        foreach(const QUrl& location, locations)
        {
            HUdn udn(rootDevice->m_device->info().udn());
            HDiscoveryType usn(udn, true);

            announcements.push_back(AnnouncementType(rootDevice, usn, location));
        }

        // generic device advertisement (same for both root and embedded devices)
        createAnnouncementMessagesForEmbeddedDevice(rootDevice, announcements);
    }

    template<typename AnnouncementType>
    void createAnnouncementMessagesForEmbeddedDevice(
        HDeviceController* device, QList<AnnouncementType>& announcements)
    {
        QList<QUrl> locations = device->m_device->locations();
        foreach(const QUrl& location, locations)
        {
            HDeviceInfo deviceInfo = device->m_device->info();

            HUdn udn = deviceInfo.udn();
            HDiscoveryType usn(udn);

            // device UDN advertisement
            announcements.push_back(AnnouncementType(device, usn, location));

            // device type advertisement
            usn.setResourceType(deviceInfo.deviceType());
            announcements.push_back(AnnouncementType(device, usn, location));

            // service advertisements
            const QList<HServiceController*>* services = device->services();
            foreach(HServiceController* service, *services)
            {
                usn.setResourceType(service->m_service->info().serviceType());
                announcements.push_back(AnnouncementType(device, usn, location));
            }
        }

        const QList<HDeviceController*>* devices = device->embeddedDevices();
        foreach(HDeviceController* embeddedDevice, *devices)
        {
            createAnnouncementMessagesForEmbeddedDevice(embeddedDevice, announcements);
        }
    }

    template<typename AnnouncementType>
    void sendAnnouncements(const QList<AnnouncementType>& announcements)
    {
        for (quint32 i = 0; i < m_advertisementCount; ++i)
        {
            foreach(HSsdp* ssdp, m_ssdps)
            {
                foreach(const AnnouncementType& at, announcements)
                {
                    ssdp->announcePresence(at());
                }
            }
        }
    }
};

}
}

#endif /* HPRESENCE_ANNOUNCER_H_ */
