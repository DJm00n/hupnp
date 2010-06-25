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

#ifndef HTTP_HANDLER_P_H_
#define HTTP_HANDLER_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "hhttp_p.h"
#include "../general/hdefs_p.h"
#include "../devicehosting/messages/hevent_messages_p.h"

#include <QByteArray>
#include <QAtomicInt>

class QHttpHeader;
class QHttpRequestHeader;
class QHttpResponseHeader;

class QtSoapMessage;

namespace Herqq
{

namespace Upnp
{

class MessagingInfo;
class InvokeActionRequest;

//
// Private helper class for HTTP messaging within the context of UPnP
//
class H_UPNP_CORE_EXPORT HHttpHandler
{
H_DISABLE_COPY(HHttpHandler)
friend class HHttpServer;

public:

    enum ReturnValue
    {
        Success = 0,
        InvalidData = 1,
        InvalidHeader = 2,
        ShuttingDown = 3,
        Timeout = 4,
        PeerDisconnected = 5,
        GenericSocketError = 6,
        SocketClosed = 7,
        Undefined = 0x0f000000
    };

private:

    const QByteArray m_loggingIdentifier;
    QAtomicInt m_shuttingDown;

private:

    ReturnValue readChunkedRequest(MessagingInfo&, QByteArray*);
    ReturnValue readRequestData   (MessagingInfo&, QByteArray*, qint64 contentLength);

    // return value is the body of the message, if any
    template<typename Header>
    ReturnValue receive(MessagingInfo&, Header&, QByteArray* = 0);

    ReturnValue sendBlob(MessagingInfo&, const QByteArray&);
    ReturnValue sendChunked(MessagingInfo&, const QByteArray&);

public:

    HHttpHandler (const QByteArray& loggingIdentifier);
    ~HHttpHandler();

    void shutdown();

    ReturnValue send(MessagingInfo&, StatusCode);
    ReturnValue send(MessagingInfo&, const QByteArray& data, StatusCode,
         ContentType = Herqq::Upnp::Undefined);

    // the byte array specifies the entire message, including the header.
    ReturnValue send(MessagingInfo&, const QByteArray&);

    ReturnValue send(MessagingInfo&, const SubscribeRequest&);
    ReturnValue send(MessagingInfo&, const UnsubscribeRequest&);
    ReturnValue send(MessagingInfo&, const SubscribeResponse&);
    ReturnValue send(MessagingInfo&, const NotifyRequest&);

    ReturnValue sendActionFailed(
        MessagingInfo&, qint32 actionErrCode, const QString& msg="");

    ReturnValue receive(
        MessagingInfo&,
        NotifyRequest&,
        NotifyRequest::RetVal&,
        const QHttpRequestHeader* rcvdHdr=0,
        const QByteArray* body=0);

    ReturnValue receive(
        MessagingInfo&,
        SubscribeRequest&,
        SubscribeRequest::RetVal&,
        const QHttpRequestHeader* rcvdHdr=0);

    ReturnValue receive(
        MessagingInfo&,
        UnsubscribeRequest&,
        UnsubscribeRequest::RetVal&,
        const QHttpRequestHeader* rcvdHdr=0);

    ReturnValue receive(MessagingInfo&, SubscribeResponse&);

    ReturnValue msgIO(MessagingInfo&, const SubscribeRequest&, SubscribeResponse&);

    ReturnValue msgIO(MessagingInfo&, const UnsubscribeRequest&);
    ReturnValue msgIO(MessagingInfo&, const NotifyRequest&);

    // last parameter is the response message
    ReturnValue msgIO(
        MessagingInfo&, QHttpRequestHeader&, const QtSoapMessage&, QtSoapMessage&);

    // last parameter is the body of the message, if any
    ReturnValue msgIO(
        MessagingInfo&, QHttpRequestHeader&, QHttpResponseHeader&, QByteArray* = 0);

    // last parameter is the body of the message, if any
    ReturnValue msgIO(
        MessagingInfo&, QHttpRequestHeader&, const QByteArray& requestBody,
        QHttpResponseHeader&, QByteArray* = 0);
};

}
}

#endif /* HTTP_HANDLER_P_H_ */
