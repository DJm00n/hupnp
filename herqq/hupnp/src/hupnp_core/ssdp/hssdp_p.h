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

#ifndef HSSDP_P_H_
#define HSSDP_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "hssdp.h"
#include "./../general/hdefs_p.h"
#include "./../socket/hendpoint.h"
#include "./../socket/hmulticast_socket.h"

#include "hdiscovery_messages.h"

#include <QByteArray>

class QUrl;
class QString;
class QHostAddress;
class QHttpRequestHeader;
class QHttpResponseHeader;

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
H_DISABLE_COPY(HSsdpPrivate)

private:

    qint32 parseCacheControl(const QString& str);
    void checkHost(const QString& host);

    HDiscoveryResponse   parseDiscoveryResponse(const QHttpResponseHeader& hdr);
    HDiscoveryRequest    parseDiscoveryRequest (const QHttpRequestHeader& hdr);
    HResourceAvailable   parseDeviceAvailable  (const QHttpRequestHeader& hdr);
    HResourceUnavailable parseDeviceUnavailable(const QHttpRequestHeader& hdr);
    HResourceUpdate      parseDeviceUpdate     (const QHttpRequestHeader& hdr);

public: // attributes

    QByteArray m_loggingIdentifier;

    Herqq::Upnp::HMulticastSocket m_multicastSocket;
    // for listening multicast messages

    QUdpSocket m_unicastSocket;
    // for sending datagrams and listening messages directed to this instance

    HSsdp* q_ptr;

    HSsdp::AllowedMessages m_allowedMessages;

public: // methods

    HSsdpPrivate(const QByteArray& loggingIdentifier = QByteArray());
    ~HSsdpPrivate();

    bool init(const QHostAddress& addressToBind, HSsdp* qptr);

    void processNotify(const QString& msg, const HEndpoint& source);
    void processSearch(const QString& msg, const HEndpoint& source,
                       const HEndpoint& destination);

    void processResponse(const QString& msg, const HEndpoint& source);

    bool send(const QByteArray& data);
    bool send(const QByteArray& data, const HEndpoint& receiver);

    void messageReceived(
        const QString& msg, const HEndpoint& source,
        const HEndpoint& destination);
};

}
}

#endif /* HSSDP_P_H_ */
