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

#include "hevent_subscription_p.h"

#include "./../../http/hhttp_server_p.h"
#include "./../../devicemodel/hdevice.h"
#include "./../../devicemodel/hservice.h"
#include "./../../devicemodel/hservice_p.h"
#include "./../../general/hupnp_global_p.h"
#include "./../../http/hhttp_messagecreator_p.h"

#include "./../../../utils/hlogger_p.h"
#include "./../../../utils/hexceptions_p.h"

#include <QThread>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HServiceSubscribtion definition
 ******************************************************************************/
HServiceSubscribtion::HServiceSubscribtion(
    const QByteArray& loggingIdentifier, HServiceController* service,
    const QUrl& serverRootUrl, QObject* parent) :
        QObject(parent),
            m_loggingIdentifier(loggingIdentifier),
            m_randomIdentifier (QUuid::createUuid()),
            m_deviceLocations(),
            m_nextLocationToTry(0),
            m_eventUrl(),
            m_sid(),
            m_seq(0),
            m_timeout(),
            m_subscriptionTimer(this),
            m_announcementTimer(this),
            m_announcementTimedOut(false),
            m_service(service),
            m_serverRootUrl(serverRootUrl),
            m_http(loggingIdentifier, this),
            m_socket(),
            m_currentOpType(Op_None),
            m_nextOpType(Op_None),
            m_subscribed(false)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(m_service);
    Q_ASSERT(!m_serverRootUrl.isEmpty());
    Q_ASSERT_X(m_serverRootUrl.isValid(), H_AT,
             m_serverRootUrl.toString().toLocal8Bit());

    m_deviceLocations = service->m_service->parentDevice()->locations();

    Q_ASSERT(m_deviceLocations.size() > 0);
    for(qint32 i = 0; i < m_deviceLocations.size(); ++i)
    {
        Q_ASSERT(!m_deviceLocations[i].isEmpty());
        Q_ASSERT_X(m_deviceLocations[i].isValid(), H_AT,
                   m_deviceLocations[i].toString().toLocal8Bit());
    }

    bool ok = connect(
        &m_subscriptionTimer, SIGNAL(timeout()),
        this, SLOT(subscriptionTimeout()));

    Q_ASSERT(ok); Q_UNUSED(ok)

    ok = connect(
        &m_announcementTimer, SIGNAL(timeout()),
        this, SLOT(announcementTimeout()));

    Q_ASSERT(ok);

    ok = connect(&m_socket, SIGNAL(connected()), this, SLOT(connected()));

    Q_ASSERT(ok);

    ok = connect(
        &m_http, SIGNAL(msgIoComplete(HHttpAsyncOperation*)),
        this, SLOT(msgIoComplete(HHttpAsyncOperation*)),
        Qt::DirectConnection);

    Q_ASSERT(ok);

    ok = connect(
        this, SIGNAL(resubscribeRequired_()),
        this, SLOT(resubscribe()));

    Q_ASSERT(ok);
}

HServiceSubscribtion::~HServiceSubscribtion()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
}

void HServiceSubscribtion::subscriptionTimeout()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    m_subscriptionTimer.stop();

    if (m_sid.isNull())
    {
        subscribe();
    }
    else
    {
        renewSubscription();
    }
}

void HServiceSubscribtion::announcementTimeout()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    m_announcementTimedOut = true;
}

void HServiceSubscribtion::resetSubscription()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    m_seq = 0;
    m_sid = HSid();
    m_eventUrl = QUrl();
    m_timeout = HTimeout();
    m_nextLocationToTry = 0;
    m_currentOpType = Op_None;
    m_subscribed = false;
    m_connectErrorCount = 0;
    m_subscriptionTimer.stop();

    if (m_socket.state() == QAbstractSocket::ConnectedState)
    {
        m_socket.disconnectFromHost();
    }
}

void HServiceSubscribtion::runNextOp()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    OperationType curOp = m_currentOpType;
    m_currentOpType = Op_None;

    switch(curOp)
    {
    case Op_None:
        break;

    case Op_Subscribe:
        subscribe();
        break;

    case Op_Renew:
        renewSubscription();
        break;

    case Op_Unsubscribe:
        unsubscribe();
        break;
    };
}

void HServiceSubscribtion::connected()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    bool ok = disconnect(
        &m_socket, SIGNAL(error(QAbstractSocket::SocketError)),
        this, SLOT(error(QAbstractSocket::SocketError)));

    Q_ASSERT(ok); Q_UNUSED(ok)

    m_connectErrorCount = 0;
    runNextOp();
}

void HServiceSubscribtion::msgIoComplete(HHttpAsyncOperation* op)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(op);

    switch(m_currentOpType)
    {
    case Op_Subscribe:
        subscribe_done(op);
        break;

    case Op_Renew:
        renewSubscription_done(op);
        break;

    case Op_Unsubscribe:
        unsubscribe_done(op);
        break;

    default:
        Q_ASSERT(false);
        break;
    };

    if (m_socket.state() == QTcpSocket::ConnectedState)
    {
        m_socket.disconnectFromHost();
    }

    delete op;

    if (m_nextOpType != Op_None)
    {
        m_currentOpType = m_nextOpType;
        m_nextOpType = Op_None;

        runNextOp();
    }
    else
    {
        m_currentOpType = Op_None;
    }
}

void HServiceSubscribtion::renewSubscription_done(HHttpAsyncOperation* op)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(!m_sid.isNull());
    Q_ASSERT(m_currentOpType == Op_Renew);

    if (op->state() == HHttpAsyncOperation::Failed)
    {
        HLOG_WARN("Event subscription renewal failed.");
        emit subscriptionFailed(this);
        return;
    }

    const QHttpResponseHeader* hdr =
        static_cast<const QHttpResponseHeader*>(op->headerRead());

    Q_ASSERT(hdr);

    SubscribeResponse response;
    if (!HHttpMessageCreator::create(*hdr, response))
    {
        HLOG_WARN("Received an invalid response to event subscription renewal.");
        emit subscriptionFailed(this);
        return;
    }

    if (response.sid() != m_sid)
    {
        // TODO, in this case could re-subscribe
        HLOG_WARN(QString(
            "Received an invalid SID [%1] to event subscription [%2] renewal").arg(
                response.sid().toString(), m_sid.toString()));
        emit subscriptionFailed(this);
        return;
    }

    m_subscribed = true;

    HLOG_DBG(QString("Subscription renewal to [%1] succeeded [sid: %2].").arg(
        m_eventUrl.toString(), m_sid.toString()));

    m_timeout = response.timeout();
    if (!m_timeout.isInfinite())
    {
        m_subscriptionTimer.start(m_timeout.value() * 1000 / 2);
    }
}

void HServiceSubscribtion::renewSubscription()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    if (m_currentOpType != Op_None || m_sid.isNull())
    {
        return;
    }

    m_announcementTimer.stop();

    m_currentOpType = Op_Renew;

    if (!connectToDevice())
    {
        return;
    }

    HLOG_DBG(QString("Renewing subscription [sid: %1].").arg(
        m_sid.toString()));

    QUrl eventUrl = resolveUri(
        extractBaseUrl(m_deviceLocations[m_nextLocationToTry]),
        m_service->m_service->eventSubUrl());

    MessagingInfo* mi = new MessagingInfo(m_socket, false);
    mi->setHostInfo(eventUrl);

    SubscribeRequest req(eventUrl, m_sid, HTimeout(1800));
    QByteArray data = HHttpMessageCreator::create(req, *mi);

    if (!m_http.msgIo(mi, data))
    {
        HLOG_WARN(QString("Failed to renew subscription [sid %1].").arg(
            m_sid.toString()));
        emit subscriptionFailed(this);
    }
}

void HServiceSubscribtion::resubscribe()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    if (m_sid.isNull())
    {
        subscribe();
    }
    else
    {
        unsubscribe();
    }
}

void HServiceSubscribtion::error(QAbstractSocket::SocketError /*err*/)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    // this can be called only when connecting to host

    if (++m_connectErrorCount >= m_deviceLocations.size() * 2)
    {
        return;
    }

    if (m_nextLocationToTry >= m_deviceLocations.size() - 1)
    {
        m_nextLocationToTry = 0;
    }
    else
    {
        ++m_nextLocationToTry;
    }

    connectToDevice();
}

bool HServiceSubscribtion::connectToDevice(qint32 msecsToWait)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(m_currentOpType != Op_None);
    Q_ASSERT(thread() == QThread::currentThread());

    if (m_socket.state() == QTcpSocket::ConnectedState)
    {
        return true;
    }

    QUrl lastLoc = m_deviceLocations[m_nextLocationToTry];

    bool ok = connect(
        &m_socket, SIGNAL(error(QAbstractSocket::SocketError)),
        this, SLOT(error(QAbstractSocket::SocketError)));

    Q_ASSERT(ok); Q_UNUSED(ok)

    m_socket.connectToHost(lastLoc.host(), lastLoc.port());
    if (msecsToWait > 0)
    {
        m_socket.waitForConnected(msecsToWait);
    }

    return m_socket.state() == QAbstractSocket::ConnectedState;
}

void HServiceSubscribtion::subscribe_done(HHttpAsyncOperation* op)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(m_sid.isNull());
    Q_ASSERT(m_currentOpType == Op_Subscribe);

    if (op->state() == HHttpAsyncOperation::Failed)
    {
        HLOG_WARN(QString("Event subscription failed: [%1]").arg(
            op->messagingInfo()->lastErrorDescription()));

        emit subscriptionFailed(this);
        return;
    }

    const QHttpResponseHeader* hdr =
        static_cast<const QHttpResponseHeader*>(op->headerRead());

    Q_ASSERT(hdr);

    SubscribeResponse response;
    if (!HHttpMessageCreator::create(*hdr, response))
    {
        HLOG_WARN(QString("Failed to subscribe: %1.").arg(hdr->toString()));
        emit subscriptionFailed(this);
        return;
    }

    m_seq = 0;
    m_sid = response.sid();
    m_subscribed = true;
    m_timeout = response.timeout();

    HLOG_DBG(QString("Subscription to [%1] succeeded. Received SID: [%2]").arg(
        m_eventUrl.toString(), m_sid.toString()));

    if (!m_timeout.isInfinite())
    {
        m_subscriptionTimer.start(m_timeout.value() * 1000 / 2);
    }

    emit subscribed(this);
}

void HServiceSubscribtion::subscribe()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    switch(m_currentOpType)
    {
    case Op_None:
        if (m_subscribed)
        {
            return;
        }
        m_currentOpType = Op_Subscribe;
        break;

    case Op_Renew:
    case Op_Subscribe:
        if (m_nextOpType != Op_None)
        {
            m_nextOpType = Op_None;
        }
        return;

    case Op_Unsubscribe:
        m_nextOpType = Op_Subscribe;
        return;
    }

    if (!m_sid.isNull())
    {
        HLOG_DBG("Ignoring subscription request, since subscription is already active");
        return;
    }

    if (!connectToDevice())
    {
        return;
    }

    m_eventUrl = resolveUri(
        extractBaseUrl(m_deviceLocations[m_nextLocationToTry]),
        m_service->m_service->eventSubUrl());

    MessagingInfo* mi = new MessagingInfo(m_socket, false);
    mi->setHostInfo(m_eventUrl);

    SubscribeRequest req(
        m_eventUrl, herqqProductTokens(),
        m_serverRootUrl.toString().append("/").append(
            m_randomIdentifier.toString().remove('{').remove('}')),
        HTimeout(1800));

    QByteArray data = HHttpMessageCreator::create(req, *mi);

    HLOG_DBG(QString("Attempting to subscribe to [%1]").arg(
        m_eventUrl.toString()));

    if (!m_http.msgIo(mi, data))
    {
        HLOG_WARN(QString(
            "Failed to subscribe to events @ [%1]: %2").arg(
                urlsAsStr(m_deviceLocations), m_socket.errorString()));

        emit subscriptionFailed(this);
    }
}

void HServiceSubscribtion::onNotify(
    MessagingInfo& mi, const NotifyRequest& req)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    if (!isSubscribed())
    {
        HLOG_WARN("Ignoring notify: subscription inactive.");
        return;
    }

    HLOG_DBG(QString("Processing notification [sid: %1, seq: %2].").arg(
        m_sid.toString(), QString::number(req.seq())));

    HHttpHandler http(m_loggingIdentifier);

    if (m_sid != req.sid())
    {
        HLOG_WARN(QString("Invalid SID [%1]").arg(req.sid().toString()));

        mi.setKeepAlive(false);
        http.send(mi, PreconditionFailed);
        return;
    }

    QMutexLocker locker(&m_seqLock);
    qint32 seq = req.seq();
    if (seq != m_seq)
    {
        HLOG_WARN(QString(
            "Received sequence number is not expected. Expected [%1], got [%2]. "
            "Re-subscribing...").arg(
                QString::number(m_seq), QString::number(seq)));

        // in this case the received sequence number does not match to what is
        // expected. UDA instructs to re-subscribe in this scenario.

        emit resubscribeRequired_();
        return;
    }

    if (m_service->updateVariables(req.variables(), m_seq > 0))
    {
        HLOG_DBG(QString(
            "Notify [sid: %1, seq: %2] OK. State variable(s) were updated.").arg(
                m_sid.toString(), QString::number(m_seq)));

        ++m_seq;
        http.send(mi, Ok);
    }
    else
    {
        HLOG_WARN(QString("Notify failed. State variable(s) were not updated."));
        mi.setKeepAlive(false);
        http.send(mi, InternalServerError);
    }
}

void HServiceSubscribtion::unsubscribe_done(HHttpAsyncOperation* /*op*/)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(!m_sid.isNull());
    Q_ASSERT(m_currentOpType == Op_Unsubscribe);

    HLOG_DBG(QString("Subscription to [%1] canceled").arg(
        m_eventUrl.toString()));

    resetSubscription();
    emit unsubscribed(this);
}

void HServiceSubscribtion::unsubscribe(qint32 msecsToWait)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    switch(m_currentOpType)
    {
    case Op_None:
        if (!m_subscribed)
        {
            return;
        }
        m_currentOpType = Op_Unsubscribe;
        break;

    case Op_Renew:
    case Op_Subscribe:
        m_nextOpType = Op_Unsubscribe;
        return;

    case Op_Unsubscribe:
        if (m_nextOpType != Op_None)
        {
            m_nextOpType = Op_None;
        }
        return;
    }

    m_subscriptionTimer.stop();

    if (!connectToDevice(msecsToWait))
    {
        return;
    }

    m_eventUrl = resolveUri(
        extractBaseUrl(m_deviceLocations[m_nextLocationToTry]),
        m_service->m_service->eventSubUrl());

    HLOG_DBG(QString(
        "Attempting to cancel event subscription from [%1]").arg(
            m_eventUrl.toString()));

    MessagingInfo* mi = new MessagingInfo(m_socket, false);
    mi->setHostInfo(m_eventUrl);

    UnsubscribeRequest req(m_eventUrl, m_sid);
    QByteArray data = HHttpMessageCreator::create(req, *mi);

    if (!m_http.msgIo(mi, data))
    {
        HLOG_WARN(QString(
            "Encountered an error during subscription cancellation: %1").arg(
                mi->lastErrorDescription()));

        // if the unsubscription "failed", there's nothing much to do, but to log
        // the error and perhaps to retry. Then again, UPnP has expiration mechanism
        // for events and thus even if the device failed to process the request, eventually
        // the subscription will expire.

        resetSubscription();
        emit unsubscribed(this);
    }
}

}
}
