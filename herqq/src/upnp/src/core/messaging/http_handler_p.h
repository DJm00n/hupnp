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

#ifndef HTTP_HANDLER_P_H_
#define HTTP_HANDLER_P_H_

#include "../defs_p.h"

#include "event_messages_p.h"
#include "http_messaginginfo_p.h"

#include <QByteArray>
#include <QAtomicInt>

class QHttpHeader;
class QHttpRequestHeader;
class QHttpResponseHeader;

class QtSoapMessage;

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

namespace Herqq
{

namespace Upnp
{

//
// Private helper class for HTTP messaging within the context of UPnP
//
class H_UPNP_CORE_EXPORT HHttpHandler
{
H_DISABLE_COPY(HHttpHandler)
friend class HHttpServer;

private:

    class Counter
    {
    H_DISABLE_COPY(Counter);
    private:

        QAtomicInt& m_counter;

    public:

        Counter(QAtomicInt& counter) : m_counter(counter)
        {
            m_counter.ref();
        }

        ~Counter()
        {
            m_counter.deref();
        }

    };

    QAtomicInt m_shuttingDown;
    QAtomicInt m_callsInProgress;

private:

    QByteArray readChunkedRequest(MessagingInfo&);
    QByteArray readRequestData   (MessagingInfo&, qint64 contentLength);

    // return value is the body of the message, if any
    template<typename Header>
    QByteArray receive(MessagingInfo&, Header&);

    void send(MessagingInfo&, const QByteArray&);
    void send(MessagingInfo&, QHttpHeader&);
    void send(MessagingInfo&, QHttpHeader&, const QByteArray&);
    void sendChunked(MessagingInfo&, const QByteArray&);

    void response(
        MessagingInfo&, qint32 statusCode, const QString& reasonPhrase);

    void response(
        MessagingInfo&, qint32 statusCode,
        const QString& reasonPhrase, const QString& body,
        const QString& contentType = "text/xml; charset=\"utf-8\"");

    void response(
        MessagingInfo&, qint32 statusCode,
        const QString& reasonPhrase, const QByteArray& body,
        const QString& contentType = "application/octet-stream");

public:

    HHttpHandler ();
    ~HHttpHandler();

    void shutdown(bool wait = false);

    void send(MessagingInfo&, const SubscribeRequest&);
    void send(MessagingInfo&, const UnsubscribeRequest&);
    void send(MessagingInfo&, const SubscribeResponse&);
    void send(MessagingInfo&, const NotifyRequest&);

    NotifyRequest::RetVal receive(
        MessagingInfo&, NotifyRequest&,
        const QHttpRequestHeader* rcvdHdr=0, const QString* body=0);

    SubscribeRequest::RetVal receive(
        MessagingInfo&, SubscribeRequest&, const QHttpRequestHeader* rcvdHdr=0);

    UnsubscribeRequest::RetVal receive(
        MessagingInfo&, UnsubscribeRequest&, const QHttpRequestHeader* rcvdHdr=0);

    void receive(MessagingInfo&, SubscribeResponse&);

    SubscribeResponse msgIO(MessagingInfo&, const SubscribeRequest&);

    void msgIO(MessagingInfo&, const UnsubscribeRequest&);
    void msgIO(MessagingInfo&, const NotifyRequest&);

    QtSoapMessage msgIO(
        MessagingInfo&, QHttpRequestHeader&, const QtSoapMessage&);

    // return value is the body of the message, if any
    QByteArray msgIO(MessagingInfo&, QHttpRequestHeader&, QHttpResponseHeader&);

    // return value is the body of the message, if any
    QByteArray msgIO(
        MessagingInfo&, QHttpRequestHeader&, const QByteArray& requestBody,
        QHttpResponseHeader&);

    void responseBadRequest              (MessagingInfo&);
    void responseMethodNotAllowed        (MessagingInfo&);
    void responseNotFound                (MessagingInfo&);
    void responseInvalidAction           (MessagingInfo&, const QString& body);
    void responseInvalidArgs             (MessagingInfo&, const QString& body);
    void responsePreconditionFailed      (MessagingInfo&);
    void responseIncompatibleHeaderFields(MessagingInfo&);
    void responseServiceUnavailable      (MessagingInfo&);
    void responseInternalServerError     (MessagingInfo&);
    void responseOk                      (MessagingInfo&, const QString& body);
    void responseOk                      (MessagingInfo&, const QByteArray& body);
    void responseOk                      (MessagingInfo&);

    void responseActionFailed(
        MessagingInfo&, qint32 actionErrCode, const QString& msg="");
};

}
}

#endif /* HTTP_HANDLER_P_H_ */
