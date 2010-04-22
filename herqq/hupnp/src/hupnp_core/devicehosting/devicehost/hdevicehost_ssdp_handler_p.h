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

#ifndef HDEVICEHOST_SSDP_HANDLER_P_H_
#define HDEVICEHOST_SSDP_HANDLER_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "./../hdevicestorage_p.h"

#include "./../../ssdp/hssdp.h"
#include "./../../ssdp/hdiscovery_messages.h"

#include <QList>

namespace Herqq
{

namespace Upnp
{

class DeviceHostSsdpHandler;

//
//
//
class HDelayedWriter :
    public QObject
{
Q_OBJECT

private:

    DeviceHostSsdpHandler& m_ssdp;
    QList<HDiscoveryResponse> m_responses;
    HEndpoint m_source;
    qint32 m_msecs;

protected:

    void timerEvent(QTimerEvent*);

public:

    HDelayedWriter(
        DeviceHostSsdpHandler&,
        const QList<HDiscoveryResponse>&,
        const HEndpoint& source,
        qint32 msecs);

    void run();

Q_SIGNALS:

    void sent();
};

//
//
//
class DeviceHostSsdpHandler :
    public HSsdp
{
H_DISABLE_COPY(DeviceHostSsdpHandler)

private:

    DeviceStorage& m_deviceStorage;

private:

    void processSearchRequest(
        const HDeviceController* device, const QUrl& location,
        QList<HDiscoveryResponse>* responses);

    bool processSearchRequest_AllDevices(
        const HDiscoveryRequest& req, const HEndpoint& source,
        QList<HDiscoveryResponse>* responses);

    bool processSearchRequest_RootDevice(
        const HDiscoveryRequest& req, const HEndpoint& source,
        QList<HDiscoveryResponse>* responses);

    bool processSearchRequest_specificDevice(
        const HDiscoveryRequest& req, const HEndpoint& source,
        QList<HDiscoveryResponse>* responses);

    bool processSearchRequest_deviceType(
        const HDiscoveryRequest& req, const HEndpoint& source,
        QList<HDiscoveryResponse>* responses);

    bool processSearchRequest_serviceType(
        const HDiscoveryRequest& req, const HEndpoint& source,
        QList<HDiscoveryResponse>* responses);

protected:

    virtual bool incomingDiscoveryRequest (
        const Herqq::Upnp::HDiscoveryRequest& msg,
        const HEndpoint& source,
        const HEndpoint& destination);

    virtual bool incomingDiscoveryResponse(
        const Herqq::Upnp::HDiscoveryResponse& msg,
        const HEndpoint& source);

    virtual bool incomingDeviceAvailableAnnouncement(
        const Herqq::Upnp::HResourceAvailable& msg);

    virtual bool incomingDeviceUnavailableAnnouncement(
        const Herqq::Upnp::HResourceUnavailable& msg);

public:

    DeviceHostSsdpHandler(
        const QByteArray& loggingIdentifier, DeviceStorage&,
        QObject* parent = 0);

    virtual ~DeviceHostSsdpHandler();
};

}
}

#endif /* HDEVICEHOST_SSDP_HANDLER_P_H_ */
