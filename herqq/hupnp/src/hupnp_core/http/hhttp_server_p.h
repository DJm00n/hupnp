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

#ifndef HHTTP_SERVER_H_
#define HHTTP_SERVER_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../general/hupnp_defs.h"

#include "hhttp_handler_p.h"
#include "hhttp_messaginginfo_p.h"

#include "../../utils/hthreadpool_p.h"

#include <QtNetwork/QTcpServer>

class QUrl;
class QString;
class QTcpSocket;
class QHttpHeader;
class QHttpRequestHeader;
class QHttpResponseHeader;

namespace Herqq
{

namespace Upnp
{

class HEndpoint;
class NotifyRequest;
class SubscribeRequest;
class UnsubscribeRequest;
class InvokeActionRequest;

//
// Private class for handling HTTP server duties needed in UPnP messaging
//
class H_UPNP_CORE_EXPORT HHttpServer :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HHttpServer)
friend class Server;
friend class Task;

private:

    class Task :
        public HRunnable
    {
    private:
        HHttpServer* m_owner;
        qint32 m_socketDescriptor;

    public:

        Task(HHttpServer* owner, qint32 socketDescriptor);
        virtual void run();
    };

    class Server :
        public QTcpServer
    {
    private:
        HHttpServer* m_owner;

    protected:
        virtual void incomingConnection(qint32 socketDescriptor);

    public:
        Server(HHttpServer* owner);
    };

private:

    QList<Server*> m_servers;
    HThreadPool*   m_threadPool;
    volatile bool  m_exiting;

protected:

    const QByteArray m_loggingIdentifier;
    HHttpHandler m_httpHandler;
    ChunkedInfo m_chunkedInfo;

private:

    void processRequest(qint32 socketDescriptor, HRunnable*);

    HHttpHandler::ReturnValue processNotifyMessage(
        MessagingInfo&, const QHttpRequestHeader&, const QByteArray& body,
        HRunnable*);

    HHttpHandler::ReturnValue processGet (
        MessagingInfo&, const QHttpRequestHeader&, HRunnable*);

    HHttpHandler::ReturnValue processHead(
        MessagingInfo&, const QHttpRequestHeader&, HRunnable*);

    HHttpHandler::ReturnValue processPost(
        MessagingInfo&, const QHttpRequestHeader&, const QByteArray& body,
        HRunnable* runner);

    HHttpHandler::ReturnValue processSubscription(
        MessagingInfo&, const QHttpRequestHeader&, HRunnable*);

    HHttpHandler::ReturnValue processUnsubscription(
        MessagingInfo&, const QHttpRequestHeader&, HRunnable*);

    bool setupIface(const HEndpoint&);

protected:

    virtual void incomingSubscriptionRequest(
        MessagingInfo&, const SubscribeRequest&, HRunnable*);

    virtual void incomingUnsubscriptionRequest(
        MessagingInfo&, const UnsubscribeRequest&, HRunnable*);

    virtual void incomingControlRequest(
        MessagingInfo&, const InvokeActionRequest&, HRunnable*);

    virtual void incomingNotifyMessage(
        MessagingInfo&, const NotifyRequest&, HRunnable*);

    virtual void incomingUnknownHeadRequest(
        MessagingInfo&, const QHttpRequestHeader&, HRunnable*);

    virtual void incomingUnknownGetRequest(
        MessagingInfo&, const QHttpRequestHeader&, HRunnable*);

    virtual void incomingUnknownPostRequest(
        MessagingInfo&, const QHttpRequestHeader&, const QByteArray& body,
        HRunnable* runner);

    ChunkedInfo& chunkedInfo();

public:

    HHttpServer(
        const QByteArray& loggingIdentifier, QObject* parent = 0);

    virtual ~HHttpServer();

    QList<QUrl> rootUrls() const;
    QUrl rootUrl(const QHostAddress&) const;
    QList<HEndpoint> endpoints() const;
    inline qint32 endpointCount() const { return m_servers.size(); }

    bool init();
    bool init(const HEndpoint&);
    bool init(const QList<HEndpoint>&);
    bool isInitialized() const;
    void close();

    qint32 activeClientCount() const;
};

}
}

#endif /* HHTTP_SERVER_H_ */
