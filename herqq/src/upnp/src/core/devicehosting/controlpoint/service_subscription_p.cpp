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

#include "service_subscription_p.h"

#include "../../upnp_global_p.h"
#include "../../devicemodel/service.h"
#include "../../devicemodel/service_p.h"
#include "../../messaging/http_server_p.h"

#include "../../../../../utils/src/logger_p.h"
#include "../../../../../core/include/HExceptions"

#include <QTcpSocket>
#include <QThreadPool>

namespace Herqq
{

namespace Upnp
{


/*******************************************************************************
 * Unsubscribe definition
 ******************************************************************************/
Unsubscribe::Unsubscribe(
    const QByteArray& loggingIdentifier,
    const HSid& sid, const QUrl& eventUrl, HHttpHandler& http, bool noWait) :
        m_loggingIdentifier(loggingIdentifier),
        m_sid(sid), m_eventUrl(eventUrl), m_http(http), m_noWait(noWait)
{
    Q_ASSERT(!m_sid.isNull());
}

void Unsubscribe::run()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QTcpSocket sock;
    sock.connectToHost(m_eventUrl.host(), m_eventUrl.port());
    if (!sock.waitForConnected(m_noWait ? 500 : 5000))
    {
        HLOG_WARN(QObject::tr(
            "Failed to cancel event subscription [%1] to [%2]: couldn't connect to the target device").arg(
                m_sid.toString(), m_eventUrl.toString()));

        return;
    }

    HLOG_DBG(QObject::tr("Attempting to cancel event subscription [%1] from [%2]").arg(
        m_sid.toString(), m_eventUrl.toString()));

    try
    {
        MessagingInfo mi(sock, false, m_noWait ? 500 : 5000);
        mi.setHostInfo(m_eventUrl);

        UnsubscribeRequest req(m_eventUrl, m_sid);
        m_http.msgIO(mi, req);
    }
    catch(HException& ex)
    {
        HLOG_WARN(QObject::tr("Could not cancel subscription [%1]: %2").arg(
            m_sid.toString(), ex.reason()));
        // if the unsubscription failed, there's nothing much to do, but to log
        // the error and perhaps to retry. Then again, UPnP has expiration mechanism
        // for events and thus even if the device failed to process the request, eventually
        // the subscription will expire.
    }
}

/*******************************************************************************
 * RenewSubscription definition
 ******************************************************************************/
RenewSubscription::RenewSubscription(HServiceSubscribtion* owner) :
    m_owner(owner)
{
    Q_ASSERT(m_owner);
}

void RenewSubscription::run()
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);

    QMutexLocker lock(&m_owner->m_subscriptionMutex);

    try
    {
        if (m_owner->m_sid.isNull())
        {
            m_owner->subscribe();
        }
        else
        {
            m_owner->renewSubscription();
            Q_ASSERT(!m_owner->m_sid.isNull());
        }
    }
    catch(HException& ex)
    {
        HLOG_WARN(QObject::tr("Subscription failed: %1").arg(ex.reason()));
        m_owner->startTimer(30000);
    }
}

/*******************************************************************************
 * HServiceSubscribtion definition
 ******************************************************************************/
HServiceSubscribtion::HServiceSubscribtion(
    const QByteArray& loggingIdentifier,
    HHttpHandler& http, const QList<QUrl>& deviceLocations,
    HServiceController* service, const QUrl& serverRootUrl,
    QThreadPool* threadPool, QObject* parent) :
        QObject(parent),
            m_loggingIdentifier(loggingIdentifier),
            m_threadPool       (threadPool),
            m_subscriptionMutex(QMutex::Recursive),
            m_randomIdentifier (QUuid::createUuid()),
            m_deviceLocations  (deviceLocations),
            m_sid(),
            m_seq(0),
            m_timeout(),
            m_subscriptionTimer(this),
            m_announcementTimer(this),
            m_announcementTimedOut(false),
            m_service(service),
            m_serverRootUrl(serverRootUrl),
            m_lastConnectedLocation(),
            m_exiting(false),
            m_http(http)
{
    HLOG(H_AT, H_FUN);

    Q_ASSERT(m_service);
    Q_ASSERT(m_threadPool);
    Q_ASSERT(!m_serverRootUrl.isEmpty());
    Q_ASSERT_X(m_serverRootUrl.isValid(), H_AT,
             m_serverRootUrl.toString().toLocal8Bit());

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

    ok = connect(
        this, SIGNAL(startTimer(int)),
        &m_subscriptionTimer, SLOT(start(int)));

    Q_ASSERT(ok);

    ok = connect(
        this, SIGNAL(stopTimer()),
        &m_subscriptionTimer, SLOT(stop()));

    Q_ASSERT(ok);
}

HServiceSubscribtion::~HServiceSubscribtion()
{
    HLOG(H_AT, H_FUN);

    // cannot exit the destructor until it is certain that no thread is running
    // a RenewSubscription instance, since they hold and use a pointer to this instance

    m_exiting = true;

    QMutexLocker lock(&m_subscriptionMutex);
}

void HServiceSubscribtion::subscriptionTimeout()
{
    HLOG(H_AT, H_FUN);

    m_subscriptionTimer.stop();

    QMutexLocker lock(&m_subscriptionMutex);

    if (m_exiting)
    {
        return;
    }

    m_threadPool->start(new RenewSubscription(this));
}

void HServiceSubscribtion::announcementTimeout()
{
    HLOG(H_AT, H_FUN);

    m_announcementTimedOut = true;
}

void HServiceSubscribtion::resetSubscription()
{
    HLOG(H_AT, H_FUN);

    m_seq = 0;
    m_sid = HSid();
    m_timeout = HTimeout();
    m_lastConnectedLocation = QUrl();
}

bool HServiceSubscribtion::connectToDevice(
    QTcpSocket* sock, QUrl* connectedBaseUrl, bool useLastLocation)
{
    HLOG(H_AT, H_FUN);
    Q_ASSERT(sock);
    Q_ASSERT(connectedBaseUrl);

    QTime stopWatch;
    qint32 waitTime = m_exiting ? 500 : 5000;

    if (useLastLocation)
    {
        Q_ASSERT(m_lastConnectedLocation.isValid() &&
                !m_lastConnectedLocation.isEmpty());

        sock->connectToHost(
            m_lastConnectedLocation.host(), m_lastConnectedLocation.port());

        stopWatch.start();
        while(stopWatch.elapsed() < waitTime)
        {
            if (sock->waitForConnected(50))
            {
                *connectedBaseUrl = extractBaseUrl(m_lastConnectedLocation);
                return true;
            }
        }

        return false;
    }

    foreach(QUrl url, m_deviceLocations)
    {
        Q_ASSERT(!url.isEmpty());
        Q_ASSERT(url.isValid());

        sock->connectToHost(url.host(), url.port());

        stopWatch.start();
        while(stopWatch.elapsed() < waitTime)
        {
            if (sock->waitForConnected(50))
            {
                m_lastConnectedLocation = url;
                *connectedBaseUrl = extractBaseUrl(m_lastConnectedLocation);
                return true;
            }
        }
    }

    return false;
}

HService* HServiceSubscribtion::service() const
{
    return m_service->m_service;
}

void HServiceSubscribtion::subscribe()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    emit stopTimer();

    QMutexLocker lock(&m_subscriptionMutex);

    Q_ASSERT(m_sid.isNull());

    QUrl baseUrl;
    QTcpSocket sock;
    if (!connectToDevice(&sock, &baseUrl, false))
    {
        throw HSocketException(QObject::tr(
            "Failed to subscribe to events [%1]: couldn't connect to the target device @ :\n%2").
                arg(sock.errorString(), urlsAsStr(m_deviceLocations)));
    }

    QUrl eventUrl = appendUrls(baseUrl, m_service->m_service->eventSubUrl());

    HLOG_DBG(QObject::tr("Attempting to subscribe to [%1]").arg(
        eventUrl.toString()));

    SubscribeRequest req(
        eventUrl, herqqProductTokens(),
        m_serverRootUrl.toString().append("/").append(
            m_randomIdentifier.toString().remove('{').remove('}')),
        HTimeout(1800));

    if (m_exiting)
    {
        throw HShutdownInProgressException(
            QObject::tr("Shutting down. Canceling subscription attempt."));
    }

    MessagingInfo mi(sock, true);
    mi.setHostInfo(eventUrl);
    SubscribeResponse response = m_http.msgIO(mi, req);

    if (!response.isValid())
    {
        throw HOperationFailedException(
            QObject::tr("Invalid response to event subscription."));
    }

    HLOG_DBG(QObject::tr("Subscription to [%1] succeeded. Received SID: [%2]").arg(
        eventUrl.toString(), response.sid().toString()));

    m_seq = 0;
    m_sid = response.sid();
    m_timeout = response.timeout();

    if (!m_timeout.isInfinite())
    {
        emit startTimer(m_timeout.value() * 1000 / 2);
    }

    if (!mi.keepAlive() || sock.state() != QTcpSocket::ConnectedState)
    {
        return;
    }

    // the connection is still open and the device did not specify that it will
    // close the connection. attempt to read the initial notify.
    //
    // according to the UDA spec., the device should send the initial
    // notify event using the same connection, unless the connection shouldn't
    // be kept alive, which it should, as we didn't specify otherwise. However,
    // the HTTP keep-alive appears to be something that is either misunderstood
    // and/or poorly implemented, which is why we can't be (unfortunately)
    // too strict about it. ==> we don't care if we can't read initial notify.

    try
    {
        NotifyRequest req;
        if (m_http.receive(mi, req) != NotifyRequest::Success)
        {
            HLOG_WARN(QObject::tr(
                "Failed to read initial notify event from the device."));
        }
        else
        {
            mi.setKeepAlive(false);
            onNotify(mi, req);
        }
    }
    catch(HException& ex)
    {
        HLOG_WARN(QObject::tr(
            "Failed to read initial notify event from the device: %1. "
            "The device does not appear to honor the HTTP keep-alive.").
                arg(ex.reason()));
    }
}

void HServiceSubscribtion::onNotify(
    MessagingInfo& mi, const NotifyRequest& req)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QMutexLocker lock(&m_subscriptionMutex);

    HLOG_DBG(QObject::tr("Processing notification [sid: %1, seq: %2].").arg(
        m_sid.toString(), QString::number(req.seq())));

    if (m_sid != req.sid())
    {
        HLOG_WARN(QObject::tr("Invalid SID [%1]").arg(req.sid().toString()));

        mi.setKeepAlive(false);
        m_http.responsePreconditionFailed(mi);
        return;
    }

    if (m_exiting)
    {
        throw HShutdownInProgressException(
            QObject::tr("Shutting down. Canceling notification processing."));
    }

    quint32 seq = req.seq();
    if (seq != m_seq)
    {
        HLOG_WARN(QObject::tr(
            "Received sequence number is not expected. Expected [%1], got [%2]. Re-subscribing...").arg(
                QString::number(m_seq), QString::number(seq)));

        // in this case the received sequence number does not match to what is
        // expected. UDA instructs to re-subscribe in this scenario.

        resubscribe();

        // no need to dispatch the request to a separate thread to avoid blocking
        // the control point's "main" thread, since
        // this method is already executed in a thread pool thread

        return;
    }

    if (m_service->updateVariables(req.variables(), m_seq > 0))
    {
        HLOG_DBG(QObject::tr(
            "Notify [sid: %1, seq: %2] OK. State variable(s) were updated.").arg(
                m_sid.toString(), QString::number(m_seq)));

        ++m_seq;
        m_http.responseOk(mi);
    }
    else
    {
        HLOG_WARN(QObject::tr("Notify failed. State variable(s) were not updated."));
        mi.setKeepAlive(false);
        m_http.responseInternalServerError(mi);
    }
}

void HServiceSubscribtion::resubscribe()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QMutexLocker lock(&m_subscriptionMutex);

    try
    {
        if (!m_sid.isNull())
        {
            unsubscribe(false);
        }

        Q_ASSERT(m_sid.isNull());

        if (m_exiting)
        {
            throw HShutdownInProgressException(
                QObject::tr("Shutting down. Canceling re-subscription"));
        }

        subscribe();

        Q_ASSERT(!m_sid.isNull());
    }
    catch(HShutdownInProgressException&)
    {
        throw;
    }
    catch(HException& ex)
    {
        HLOG_WARN(QObject::tr("Re-subscription failed: %1.").arg(ex.reason()));
        emit startTimer(30000);
    }
}

void HServiceSubscribtion::renewSubscription()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    emit stopTimer();

    QMutexLocker lock(&m_subscriptionMutex);

    Q_ASSERT(!m_sid.isNull());

    HLOG_DBG(QObject::tr("Renewing subscription [sid: %1].").arg(
        m_sid.toString()));

    QUrl baseUrl;
    QTcpSocket sock;
    if (!connectToDevice(&sock, &baseUrl, true))
    {
        throw HSocketException(QObject::tr(
            "Failed to renew event subscription [sid %1]: couldn't connect to the target device").arg(
                m_sid.toString()));
    }

    if (m_exiting)
    {
        throw HShutdownInProgressException(
            QObject::tr("Shutting down. Canceling subscription renewal [sid %1].").
                arg(m_sid.toString()));
    }

    QUrl eventUrl = appendUrls(baseUrl, m_service->m_service->eventSubUrl());

    MessagingInfo mi(sock, false);
    mi.setHostInfo(eventUrl);

    SubscribeRequest req(eventUrl, m_sid, HTimeout(1800));
    SubscribeResponse response = m_http.msgIO(mi, req);

    if (!response.isValid())
    {
        throw HOperationFailedException(
            QObject::tr("Invalid response to re-subscribe [sid %1].").arg(
                m_sid.toString()));
    }

    if (response.sid() != m_sid)
    {
        throw HOperationFailedException(
            QObject::tr("Invalid SID [%1] received while renewing subscription [%2]").arg(
                response.sid().toString(), m_sid.toString()));
    }

    HLOG_DBG(QObject::tr("Renewal to [%1] succeeded [sid: %2].").arg(
        eventUrl.toString(), m_sid.toString()));

    m_timeout = response.timeout();
    if (!m_timeout.isInfinite())
    {
        emit startTimer(m_timeout.value() * 1000 / 2);
    }
}

void HServiceSubscribtion::unsubscribe(bool exiting)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    emit stopTimer();

    QMutexLocker lock(&m_subscriptionMutex);

    m_exiting = exiting;

    Q_ASSERT(!m_sid.isNull());

    QUrl baseUrl;
    QTcpSocket sock;
    if (!connectToDevice(&sock, &baseUrl, true))
    {
        // no matter what happens here, after calling the unsubscribe() the object
        // must enter "fresh" state. there are many scenarios where unsubscription will
        // fail and there's absolutely no point in trying to make sure that the event
        // publisher has received the message.
        resetSubscription();

        throw HSocketException(QObject::tr(
            "Failed to cancel event subscription: couldn't connect to the target device"));
    }

    try
    {
        QUrl eventUrl = appendUrls(baseUrl, m_service->m_service->eventSubUrl());

        HLOG_DBG(QObject::tr("Attempting to cancel event subscription from [%1]").arg(
            eventUrl.toString()));

        MessagingInfo mi(sock, false, m_exiting ? 10000 : 1000);
        mi.setHostInfo(eventUrl);

        UnsubscribeRequest req(eventUrl, m_sid);
        m_http.msgIO(mi, req);

        HLOG_DBG(QObject::tr("Subscription to [%1] canceled").arg(
            eventUrl.toString()));
    }
    catch(HException& ex)
    {
        HLOG_WARN(QObject::tr("Encountered an error during subscription cancellation: %1").arg(
            ex.reason()));

        // if the unsubscription failed, there's nothing much to do, but to log
        // the error and perhaps to retry. Then again, UPnP has expiration mechanism
        // for events and thus even if the device failed to process the request, eventually
        // the subscription will expire.
    }

    resetSubscription();
}

}
}
