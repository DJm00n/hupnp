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

#ifndef HPRESENCE_ANNOUNCER_H_
#define HPRESENCE_ANNOUNCER_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "./../../general/hupnp_global_p.h"

#include "./../../devicemodel/hdevice.h"
#include "./../../devicemodel/hdevice_p.h"

#include "./../../devicemodel/hservice.h"
#include "./../../devicemodel/hservice_p.h"

#include "./../../socket/hendpoint.h"

#include "./../../ssdp/husn.h"
#include "./../../ssdp/hssdp.h"
#include "./../../ssdp/hdiscovery_messages.h"
#include "./../../ssdp/hresource_identifier.h"

#include "./../../dataelements/hudn.h"
#include "./../../dataelements/hdeviceinfo.h"
#include "./../../dataelements/hproduct_tokens.h"

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
    HUsn m_usn;
    QUrl m_location;

public:

    Announcement()
    {
    }

    Announcement(
        HDeviceController* device, const HUsn& usn, const QUrl& location) :
            m_device(device), m_usn(usn), m_location(location)
    {
        Q_ASSERT(m_device);
        Q_ASSERT(m_usn.isValid());
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
        HDeviceController* device, const HUsn& usn, const QUrl& location) :
            Announcement(device, usn, location)
    {
    }

    HResourceAvailable operator()()
    {
        HProductTokens pt = herqqProductTokens();

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
        HDeviceController* device, const HUsn& usn, const QUrl& location) :
            Announcement(device, usn, location)
    {
    }

    HResourceUnavailable operator()()
    {
        return HResourceUnavailable(
            m_usn,
            m_location,
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

    HSsdp* m_ssdp;
    quint32 m_advertisementCount;

public:

    PresenceAnnouncer (HSsdp* ssdp, quint32 advertisementCount) :
        m_ssdp(ssdp), m_advertisementCount(advertisementCount)
    {
        Q_ASSERT(m_ssdp);
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
        HDeviceController* rootDevice,
        QList<AnnouncementType>& announcements)
    {
        QList<QUrl> locations = rootDevice->m_device->locations(true);
        foreach(QUrl location, locations)
        {
            HUdn udn(rootDevice->m_device->deviceInfo().udn());
            HUsn usn(udn, HResourceIdentifier::getRootDeviceIdentifier());

            announcements.push_back(AnnouncementType(rootDevice, usn, location));
        }

        // generic device advertisement (same for both root and embedded devices)
        createAnnouncementMessagesForEmbeddedDevice(rootDevice, announcements);
    }

    template<typename AnnouncementType>
    void createAnnouncementMessagesForEmbeddedDevice(
        HDeviceController* device, QList<AnnouncementType>& announcements)
    {
        QList<QUrl> locations = device->m_device->locations(true);
        foreach(QUrl location, locations)
        {
            HDeviceInfo deviceInfo = device->m_device->deviceInfo();

            HUdn udn = deviceInfo.udn();
            HUsn usn = udn;

            // device UDN advertisement
            announcements.push_back(AnnouncementType(device, usn, location));

            // device type advertisement
            usn.setResource(deviceInfo.deviceType());
            announcements.push_back(AnnouncementType(device, usn, location));

            // service advertisements
            QList<HServiceController*> services = device->services();
            foreach(HServiceController* service, services)
            {
                usn.setResource(service->m_service->serviceType().toString());
                announcements.push_back(AnnouncementType(device, usn, location));
            }
        }

        QList<HDeviceController*> devices = device->embeddedDevices();
        foreach(HDeviceController* embeddedDevice, devices)
        {
            createAnnouncementMessagesForEmbeddedDevice(embeddedDevice, announcements);
        }
    }

    template<typename AnnouncementType>
    void sendAnnouncements(const QList<AnnouncementType>& announcements)
    {
        for (quint32 i = 0; i < m_advertisementCount; ++i)
        {
            foreach(AnnouncementType at, announcements)
            {
                m_ssdp->announcePresence(at());
            }
        }
    }
};

}
}

#endif /* HPRESENCE_ANNOUNCER_H_ */
