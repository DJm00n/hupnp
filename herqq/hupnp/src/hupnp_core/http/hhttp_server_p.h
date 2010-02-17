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

#ifndef HTTP_SERVER_H_
#define HTTP_SERVER_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "./../general/hdefs_p.h"
#include "hhttp_handler_p.h"
#include "hhttp_messaginginfo_p.h"

#include <QRunnable>
#include <QTcpServer>

class QUrl;
class QString;
class QTcpSocket;
class QThreadPool;
class QHttpHeader;
class QHttpRequestHeader;
class QHttpResponseHeader;

namespace Herqq
{

namespace Upnp
{

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

    class Task : public QRunnable
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

    Server        m_server;
    QThreadPool*  m_threadPool;
    volatile bool m_exiting;

protected:

    const QByteArray m_loggingIdentifier;
    HHttpHandler m_httpHandler;
    ChunkedInfo m_chunkedInfo;

private:

    void processNotifyMessage(
        MessagingInfo&, const QHttpRequestHeader&, const QByteArray& body);

    void processRequest(qint32 socketDescriptor);

    void processGet (MessagingInfo&, const QHttpRequestHeader&);
    void processHead(MessagingInfo&, const QHttpRequestHeader&);

    void processPost(
        MessagingInfo&, const QHttpRequestHeader&, const QByteArray& body);

    void processSubscription(
        MessagingInfo&, const QHttpRequestHeader&);

    void processUnsubscription(
        MessagingInfo&, const QHttpRequestHeader&);

protected:

    virtual void incomingSubscriptionRequest(
        MessagingInfo&, const SubscribeRequest&);

    virtual void incomingUnsubscriptionRequest(
        MessagingInfo&, const UnsubscribeRequest&);

    virtual void incomingControlRequest(
        MessagingInfo&, const InvokeActionRequest&);

    virtual void incomingNotifyMessage(
        MessagingInfo&, const NotifyRequest&);

    virtual void incomingUnknownHeadRequest(
        MessagingInfo&, const QHttpRequestHeader&);

    virtual void incomingUnknownGetRequest(
        MessagingInfo&, const QHttpRequestHeader&);

    virtual void incomingUnknownPostRequest(
        MessagingInfo&, const QHttpRequestHeader&, const QByteArray& body);

    ChunkedInfo& chunkedInfo();

public:

    explicit HHttpServer(
        const QString& loggingIdentifier, QObject* parent = 0);

    virtual ~HHttpServer();

    QUrl rootUrl() const;
    bool listen();
    bool listen(const QHostAddress& ha, quint16 port);
    void close(bool wait);

    qint32 activeClientCount() const;
};

}
}

#endif /* HTTP_SERVER_H_ */
