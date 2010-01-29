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

#include "../../../../core/include/HGlobal"

#include "event_messages_p.h"
#include "control_messages_p.h"

#include <QAtomicInt>
#include <QTcpServer>

class QUrl;
class QString;
class QRunnable;
class QTcpSocket;
class QThreadPool;
class QHttpHeader;
class QHttpRequestHeader;
class QHttpResponseHeader;

namespace Herqq
{

namespace Upnp
{

//
//
//
class H_UPNP_CORE_EXPORT ChunkedInfo
{
public:

    ChunkedInfo();

    QAtomicInt m_maxChunkSize;
    // if this is non-zero, it means that chunked-encoding should be used
    // if the data to be sent is larger than that of the specified max chunk size
    // and that the max chunk size is this

    QAtomicInt m_minChunkSize;
    // if this is non-zero, it means that when the size of the data to be sent
    // is not known in advance, how big _at least_ each chunk must be in size.
};

class HHttpHandler;

//
//
//
class H_UPNP_CORE_EXPORT MessagingInfo
{
H_DISABLE_COPY(MessagingInfo)
friend class HHttpServer;
friend class HHttpHandler;

private:

    QTcpSocket& m_sock;

    bool    m_keepAlive;
    qint32  m_receiveTimeoutForNoData;

    ChunkedInfo m_chunkedInfo;

    QString m_hostInfo;

public:

     //
    // The default timeout in milliseconds that is waited before a read operation
    // is terminated unless _some_ data is received (not necessarily the desired amount).
    //
    static inline qint32 defaultReceiveTimeoutForNoData()
    {
        const qint32 retVal = 5000;
        return retVal;
    }

    explicit MessagingInfo(
        QTcpSocket& sock,
        qint32 receiveTimeoutForNoData = defaultReceiveTimeoutForNoData());

    MessagingInfo(
        QTcpSocket& sock, bool keepAlive,
        qint32 receiveTimeoutForNoData = defaultReceiveTimeoutForNoData());

    inline QTcpSocket& socket() const { return m_sock; }

    inline void setKeepAlive(bool arg){ m_keepAlive = arg ; }
    inline bool keepAlive   () const  { return m_keepAlive; }

    void setHostInfo(const QUrl& hostInfo);
    void setHostInfo(const QString& hostInfo) { m_hostInfo = hostInfo.trimmed(); }
    QString hostInfo() const;

    inline void setReceiveTimeoutForNoData(qint32 arg)
    {
        m_receiveTimeoutForNoData = arg;
    }

    inline qint32 receiveTimeoutForNoData() const
    {
        return m_receiveTimeoutForNoData;
    }

    inline ChunkedInfo& chunkedInfo() { return m_chunkedInfo; }
};

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
        MessagingInfo&, const QHttpRequestHeader&, const QString& body);

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
    void close();
};

}
}

#endif /* HTTP_SERVER_H_ */
