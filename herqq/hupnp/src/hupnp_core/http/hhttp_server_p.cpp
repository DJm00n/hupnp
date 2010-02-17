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

#include "hhttp_server_p.h"
#include "hhttp_utils_p.h"
#include "hhttp_messaginginfo_p.h"
#include "hhttp_messagecreator_p.h"

#include "./../../utils/hlogger_p.h"
#include "./../../utils/hexceptions_p.h"
#include "./../general/hupnp_global_p.h"

#include "./../devicehosting/messages/hcontrol_messages_p.h"
#include "./../devicehosting/messages/hevent_messages_p.h"

#include <QUrl>
#include <QTime>
#include <QString>
#include <QTcpSocket>
#include <QByteArray>
#include <QThreadPool>
#include <QMutexLocker>
#include <QNetworkInterface>
#include <QHttpRequestHeader>
#include <QHttpResponseHeader>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HHttpServer::Task
 ******************************************************************************/
HHttpServer::Task::Task(HHttpServer* owner, qint32 socketDescriptor) :
    m_owner(owner), m_socketDescriptor(socketDescriptor)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
}

void HHttpServer::Task::run()
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
    m_owner->processRequest(m_socketDescriptor);
}

/*******************************************************************************
 * HHttpServer::Server
 ******************************************************************************/
HHttpServer::Server::Server(HHttpServer* owner) :
    QTcpServer(owner), m_owner(owner)
{
    HLOG(H_AT, H_FUN);
}

void HHttpServer::Server::incomingConnection(qint32 socketDescriptor)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);

    HLOG_DBG(QObject::tr("Incoming connection."));

    m_owner->m_threadPool->start(
        new HHttpServer::Task(m_owner, socketDescriptor));
}

/*******************************************************************************
 * HHttpServer
 ******************************************************************************/
HHttpServer::HHttpServer(
    const QString& loggingIdentifier, QObject* parent) :
        QObject(parent),
            m_server(this), m_threadPool(new QThreadPool()), m_exiting(false),
            m_loggingIdentifier(loggingIdentifier.toLocal8Bit()),
            m_httpHandler(m_loggingIdentifier),
            m_chunkedInfo()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    m_threadPool->setParent(this);
    m_threadPool->setMaxThreadCount(25);
}

HHttpServer::~HHttpServer()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    close(false);
    delete m_threadPool;
}

ChunkedInfo& HHttpServer::chunkedInfo()
{
    return m_chunkedInfo;
}

void HHttpServer::processRequest(qint32 socketDescriptor)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QTcpSocket client;
    client.setSocketDescriptor(socketDescriptor);

    QString peer = peerAsStr(client);

    HLOG_INFO(QObject::tr("Client from [%1] accepted. Current client count: %2").
        arg(peer, QString::number(m_threadPool->activeThreadCount())));

    QTime stopWatch; stopWatch.start();
    while(!m_exiting && client.state() == QTcpSocket::ConnectedState &&
          stopWatch.elapsed() < 30000)
    {
        QByteArray body;
        QHttpRequestHeader requestHeader;

        MessagingInfo mi(client);
        mi.chunkedInfo() = m_chunkedInfo;

        try
        {
            body = m_httpHandler.receive(mi, requestHeader);
            if (!requestHeader.isValid())
            {
                m_httpHandler.send(mi, BadRequest);
                break;
            }

            QString host = requestHeader.value("HOST");
            if (host.isEmpty())
            {
                m_httpHandler.send(mi, BadRequest);
                break;
            }

            mi.setHostInfo(host);
            mi.setKeepAlive(HHttpUtils::keepAlive(requestHeader));
        }
        catch(HTimeoutException&)
        {
            continue;
        }
        catch(HSocketException&)
        {
            // no more data / client has disconnected ==> no need to do
            // (even print) anything.
            break;
        }
        catch(HException& ex)
        {
            HLOG_WARN(QObject::tr("Receive failed: %1").arg(ex.reason()));
            break;
        }

        if (m_exiting)
        {
            break;
        }

        try
        {
            QString method = requestHeader.method();
            if (method.compare("GET", Qt::CaseInsensitive) == 0)
            {
                processGet(mi, requestHeader);
            }
            else if (method.compare("HEAD"), Qt::CaseInsensitive)
            {
                processHead(mi, requestHeader);
            }
            else if (method.compare("POST", Qt::CaseInsensitive) == 0)
            {
                processPost(mi, requestHeader, body);
            }
            else if (method.compare("NOTIFY", Qt::CaseInsensitive) == 0)
            {
                processNotifyMessage(mi, requestHeader, body);
            }
            else if (method.compare("SUBSCRIBE", Qt::CaseInsensitive) == 0)
            {
                processSubscription(mi, requestHeader);
            }
            else if (method.compare("UNSUBSCRIBE", Qt::CaseInsensitive) == 0)
            {
                processUnsubscription(mi, requestHeader);
            }
            else
            {
                m_httpHandler.send(mi, MethotNotAllowed);
                break;
            }
        }
        catch(HException& ex)
        {
            HLOG_WARN(ex.reason());
            break;
        }

        if (!mi.keepAlive())
        {
            break;
        }

        stopWatch.start();
    }

    if (client.state() == QTcpSocket::ConnectedState)
    {
        for(qint32 i = 0; i < 1000 && client.flush(); ++i)
        {
            client.waitForBytesWritten(1);
        }

        client.disconnectFromHost();
    }

    HLOG_INFO(QObject::tr("Client from [%1] disconnected. Current client count: %2").
        arg(peer, QString::number(m_threadPool->activeThreadCount())));
}

void HHttpServer::processGet(
    MessagingInfo& mi, const QHttpRequestHeader& requestHdr)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    HLOG_DBG(QObject::tr("Dispatching unknown GET request."));
    incomingUnknownGetRequest(mi, requestHdr);
}

void HHttpServer::processHead(
    MessagingInfo& mi, const QHttpRequestHeader& requestHdr)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    HLOG_DBG(QObject::tr("Dispatching unknown HEAD request."));
    incomingUnknownHeadRequest(mi, requestHdr);
}

void HHttpServer::processPost(
    MessagingInfo& mi, const QHttpRequestHeader& requestHdr, const QByteArray& body)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QString soapAction  = requestHdr.value("SOAPACTION");
    if (soapAction.indexOf("#") <= 0)
    {
        HLOG_DBG(QObject::tr("Dispatching unknown POST request."));
        incomingUnknownPostRequest(mi, requestHdr, body);
        return;
    }

    QString actionName = soapAction.mid(soapAction.indexOf("#"));
    if (actionName.isEmpty())
    {
        HLOG_DBG(QObject::tr("Dispatching unknown POST request."));
        incomingUnknownPostRequest(mi, requestHdr, body);
        return;
    }

    QtSoapMessage soapMsg;
    if (!soapMsg.setContent(body))
    {
        mi.setKeepAlive(false);
        m_httpHandler.send(mi, BadRequest);

        return;
    }

    QString controlUrl = requestHdr.path().simplified();
    if (controlUrl.isEmpty())
    {
        mi.setKeepAlive(false);
        m_httpHandler.send(mi, BadRequest);

        return;
    }

    InvokeActionRequest iareq(soapAction, soapMsg, controlUrl);
    HLOG_DBG(QObject::tr("Dispatching control request."));
    incomingControlRequest(mi, iareq);
}

void HHttpServer::processSubscription(
    MessagingInfo& mi, const QHttpRequestHeader& requestHdr)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    SubscribeRequest sreq;
    SubscribeRequest::RetVal rv = m_httpHandler.receive(mi, sreq, &requestHdr);

    if (rv == SubscribeRequest::Success)
    {
        HLOG_DBG(QObject::tr("Dispatching subscription request."));
        incomingSubscriptionRequest(mi, sreq);
    }
}

void HHttpServer::processUnsubscription(
    MessagingInfo& mi, const QHttpRequestHeader& requestHdr)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    UnsubscribeRequest usreq;
    UnsubscribeRequest::RetVal rv = m_httpHandler.receive(mi, usreq, &requestHdr);

    if (rv == UnsubscribeRequest::Success)
    {
        HLOG_DBG(QObject::tr("Dispatching unsubscription request."));
        incomingUnsubscriptionRequest(mi, usreq);
    }
}

void HHttpServer::processNotifyMessage(
    MessagingInfo& mi, const QHttpRequestHeader& request, const QByteArray& body)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    NotifyRequest nreq;
    NotifyRequest::RetVal rv = m_httpHandler.receive(mi, nreq, &request, &body);

    if (rv == NotifyRequest::Success)
    {
        HLOG_DBG(QObject::tr("Dispatching event notification."));
        incomingNotifyMessage(mi, nreq);
    }
}

void HHttpServer::incomingSubscriptionRequest(
    MessagingInfo& mi, const SubscribeRequest&)
{
    HLOG(H_AT, H_FUN);
    HLOG_DBG(QObject::tr("Calling default implementation, which does nothing."));
    mi.setKeepAlive(false);
    m_httpHandler.send(mi, MethotNotAllowed);
}

void HHttpServer::incomingUnsubscriptionRequest(
    MessagingInfo& mi, const UnsubscribeRequest&)
{
    HLOG(H_AT, H_FUN);
    HLOG_DBG(QObject::tr("Calling default implementation, which does nothing."));
    mi.setKeepAlive(false);
    m_httpHandler.send(mi, MethotNotAllowed);
}

void HHttpServer::incomingControlRequest(
    MessagingInfo& mi, const InvokeActionRequest&)
{
    HLOG(H_AT, H_FUN);
    HLOG_DBG(QObject::tr("Calling default implementation, which does nothing."));
    mi.setKeepAlive(false);
    m_httpHandler.send(mi, MethotNotAllowed);
}

void HHttpServer::incomingNotifyMessage(
    MessagingInfo& mi, const NotifyRequest&)
{
    HLOG(H_AT, H_FUN);
    HLOG_DBG(QObject::tr("Calling default implementation, which does nothing."));
    mi.setKeepAlive(false);
    m_httpHandler.send(mi, MethotNotAllowed);
}

void HHttpServer::incomingUnknownHeadRequest(
    MessagingInfo& mi, const QHttpRequestHeader&)
{
    HLOG(H_AT, H_FUN);
    HLOG_DBG(QObject::tr("Calling default implementation, which does nothing."));
    mi.setKeepAlive(false);
    m_httpHandler.send(mi, MethotNotAllowed);
}

void HHttpServer::incomingUnknownGetRequest(
    MessagingInfo& mi, const QHttpRequestHeader&)
{
    HLOG(H_AT, H_FUN);
    HLOG_DBG(QObject::tr("Calling default implementation, which does nothing."));
    mi.setKeepAlive(false);
    m_httpHandler.send(mi, MethotNotAllowed);
}

void HHttpServer::incomingUnknownPostRequest(
    MessagingInfo& mi, const QHttpRequestHeader&, const QByteArray&)
{
    HLOG(H_AT, H_FUN);
    HLOG_DBG(QObject::tr("Calling default implementation, which does nothing."));
    mi.setKeepAlive(false);
    m_httpHandler.send(mi, MethotNotAllowed);
}

QUrl HHttpServer::rootUrl() const
{
    HLOG(H_AT, H_FUN);

    Q_ASSERT(!m_server.serverAddress().isNull());
    Q_ASSERT(m_server.serverPort() > 0);

    return QUrl(QString("http://%1:%2").arg(
        m_server.serverAddress().toString(),
        QString::number(m_server.serverPort())));
}

bool HHttpServer::listen()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT_X(
        thread() == QThread::currentThread(), H_AT,
            "The HTTP Server has to be shutdown in the thread in which it is currently located.");

    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    foreach (QNetworkInterface iface, interfaces)
    {
        if (iface.flags() & QNetworkInterface::IsUp &&
          !(iface.flags() & QNetworkInterface::IsLoopBack))
        {
            QList<QNetworkAddressEntry> entries = iface.addressEntries();
            foreach(QNetworkAddressEntry entry, entries)
            {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol)
                {
                    for (int i = 0; i < 10; ++i)
                    {
                        if (m_server.listen(entry.ip()))
                        {
                            HLOG_INFO(QObject::tr(
                                "Binding to %1").arg(entry.ip().toString()));

                            return true;
                        }
                    }
                }
            }
        }
    }

    HLOG_INFO(QObject::tr(
        "Could not find a suitable network interface. Binding to localhost."));

    return m_server.listen(QHostAddress::LocalHost);
}

bool HHttpServer::listen(const QHostAddress& ha, quint16 port)
{
    HLOG(H_AT, H_FUN);

    Q_ASSERT_X(
        thread() == QThread::currentThread(), H_AT,
            "The HTTP Server has to be shutdown in the thread in which it is currently located.");


    if (ha == QHostAddress::Null || ha == QHostAddress::Any ||
        ha == QHostAddress::Broadcast)
    {
        return false;
    }

    return m_server.listen(ha, port);
}

void HHttpServer::close(bool wait)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT_X(
        thread() == QThread::currentThread(), H_AT,
            "The HTTP Server has to be shutdown in the thread in which it is currently located.");

    m_exiting = true;

    if (m_server.isListening())
    {
        m_server.close();
    }

    m_httpHandler.shutdown(wait);

    if (wait)
    {
        m_threadPool->waitForDone();
    }
}

qint32 HHttpServer::activeClientCount() const
{
    return m_threadPool->activeThreadCount();
}

}
}
