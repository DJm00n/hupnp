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

#ifndef SSDP_P_H_
#define SSDP_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "endpoint.h"
#include "multicast_socket.h"
#include "discovery_messages.h"

#include "../../../../core/include/HGlobal"

#include <QMutex>
#include <QObject>
#include <QUdpSocket>

class QUrl;
class QString;
class QHostAddress;

namespace Herqq
{

namespace Upnp
{

class HSsdp;

//
// Implementation details of HSsdp
//
class HSsdpPrivate
{
public: // attributes

    Herqq::Upnp::HMulticastSocket m_multicastSocket;
    // for listening multicast messages

    QUdpSocket       m_unicastSocket;
    // for sending datagrams and listening messages directed to this instance

    HSsdp*           q_ptr;

public: // methods

    HSsdpPrivate ();
    ~HSsdpPrivate();

    void init(const QHostAddress& addressToBind, HSsdp* qptr);

    void processNotify(const QString& msg, const HEndpoint& source);
    void processSearch(const QString& msg, const HEndpoint& source,
                       const HEndpoint& destination);

    void processResponse(const QString& msg, const HEndpoint& source);

    void send(const QString& data);
    void send(const QString& data, const HEndpoint& receiver);

    void messageReceived(
        const QString& msg, const HEndpoint& source,
        const HEndpoint& destination);
};

//
//
//
template
<
    class T,
    bool defaultReturnValue = true
>
class SsdpWithoutEventing :
    public HSsdp
{

private:

    T* m_listener;

protected:

    virtual bool incomingDiscoveryRequest (
        const Herqq::Upnp::HDiscoveryRequest& msg,
        const HEndpoint& source,
        const HEndpoint& destination)
    {
        if (!m_listener->readyForEvents())
        {
            return defaultReturnValue;
        }

        return m_listener->discoveryRequestReceived(msg, source, destination);
    }

    virtual bool incomingDiscoveryResponse(
        const Herqq::Upnp::HDiscoveryResponse& msg,
        const HEndpoint& source)
    {
        if (!m_listener->readyForEvents())
        {
            return defaultReturnValue;
        }

        return m_listener->discoveryResponseReceived(msg, source);
    }

    virtual bool incomingDeviceAvailableAnnouncement  (
        const Herqq::Upnp::HResourceAvailable& msg)
    {
        if (!m_listener->readyForEvents())
        {
            return defaultReturnValue;
        }

        return m_listener->resourceAvailableReceived(msg);
    }

    virtual bool incomingDeviceUnavailableAnnouncement(
        const Herqq::Upnp::HResourceUnavailable& msg)
    {
        if (!m_listener->readyForEvents())
        {
            return defaultReturnValue;
        }

        return m_listener->resourceUnavailableReceived(msg);
    }

public:

    explicit SsdpWithoutEventing(T* listener, QObject* parent = 0) :
        HSsdp(parent), m_listener(listener)
    {
    }

    virtual ~SsdpWithoutEventing(){}
};

}
}

#endif /* SSDP_P_H_ */
