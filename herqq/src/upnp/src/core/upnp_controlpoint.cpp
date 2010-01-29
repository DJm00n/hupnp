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

#include "upnp_controlpoint.h"
#include "upnp_controlpoint_p.h"

#include "upnp_global_p.h"
#include "upnp_action_p.h"
#include "upnp_service_p.h"
#include "upnp_deviceinfo.h"
#include "datatype_mappings_p.h"
#include "upnp_objectcreator_p.h"

#include "messaging/usn.h"
#include "messaging/product_tokens.h"
#include "messaging/event_messages_p.h"
#include "messaging/discovery_messages.h"
#include "messaging/resource_identifier.h"

#include "utils/xml_utils_p.h"
#include "../../../utils/src/sysutils_p.h"
#include "../../../utils/src/logger_p.h"

#include "../../../core/include/HExceptions"

#include <QUrl>
#include <QString>
#include <QMutexLocker>
#include <QHttpRequestHeader>
#include <QHttpResponseHeader>

#include <QtSoapMessage>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HActionInvokeProxy definition
 ******************************************************************************/
HActionInvokeProxy::HActionInvokeProxy(
    HService* service, const QString& actionName,
    const HActionInputArguments& inArgs,
    const HActionOutputArguments& outArgs) :
        m_service(service), m_actionName(actionName),
        m_inArgs(inArgs), m_outArgs(outArgs), m_sock(new QTcpSocket(service)),
        m_baseUrl(), m_http(new HHttpHandler())
{
    Q_ASSERT(service);
    verifyName(actionName);
}

void HActionInvokeProxy::checkConnection()
{
    HLOG(H_AT, H_FUN);
    Q_ASSERT(m_sock);
    //Q_ASSERT(m_sock->thread() == QThread::currentThread());

    if (m_sock->state() == QTcpSocket::ConnectedState)
    {
        return;
    }

    QList<QUrl> locations = m_service->parentDevice()->locations(false);
    foreach(QUrl location, locations)
    {
        m_sock->connectToHost(location.host(), location.port());
        if (m_sock->waitForConnected(1000))
        {
            m_baseUrl = location;
            return;
        }
    }

    m_baseUrl = QUrl();
    throw HSocketException(
        QObject::tr("Couldn't connect to the device [%1]").arg(
            m_service->parentDevice()->deviceInfo().udn().toSimpleUuid()));
}

void HActionInvokeProxy::checkConnection(QTcpSocket* sock)
{
    HLOG(H_AT, H_FUN);
    Q_ASSERT(sock);
    Q_ASSERT(sock->thread() == QThread::currentThread());

    if (sock->state() == QTcpSocket::ConnectedState)
    {
        return;
    }

    QList<QUrl> locations = m_service->parentDevice()->locations(false);
    foreach(QUrl location, locations)
    {
        sock->connectToHost(location.host(), location.port());
        if (sock->waitForConnected(1000))
        {
            m_baseUrl = location;
            return;
        }
    }

    m_baseUrl = QUrl();
    throw HSocketException(
        QObject::tr("Couldn't connect to the device [%1]").arg(
            m_service->parentDevice()->deviceInfo().udn().toSimpleUuid()));
}


QtSoapMessage HActionInvokeProxy::msgIO(const QtSoapMessage& soapMsg)
{
    HLOG(H_AT, H_FUN);

    QTcpSocket sock;
    checkConnection(&sock);
    //checkConnection();

    Q_ASSERT(m_service);
    QUrl controlUrl = appendUrls(m_baseUrl.path(), m_service->controlUrl());

    QHttpRequestHeader actionInvokeRequest("POST", controlUrl.toString());
    actionInvokeRequest.setContentType("text/xml; charset=\"utf-8\"");

    QString soapActionHdrField("\"");
    soapActionHdrField.append(m_service->serviceType().toString());
    soapActionHdrField.append("#").append(m_actionName).append("\"");
    actionInvokeRequest.setValue("SOAPACTION", soapActionHdrField);

    //MessagingInfo mi(*m_sock, true, 30000);
    MessagingInfo mi(sock, true, 30000);
    mi.setHostInfo(m_baseUrl);
    return m_http->msgIO(mi, actionInvokeRequest, soapMsg);
}

qint32 HActionInvokeProxy::operator()(
    const HActionInputArguments& inArgs, HActionOutputArguments* outArgs)
{
    HLOG(H_AT, H_FUN);

    // 1) create the remote method call request
    QtSoapMessage soapMsg;
    soapMsg.setMethod(
        QtSoapQName(m_actionName, m_service->serviceType().toString()));

    HActionInputArguments::const_iterator ci = inArgs.constBegin();
    for(; ci != inArgs.constEnd(); ++ci)
    {
        const HActionInputArgument* const iarg = (*ci);
        if (!m_inArgs.contains(iarg->name()))
        {
            return HAction::InvalidArgs();
        }

        QtSoapType* soapArg =
            new SoapType(iarg->name(), iarg->dataType(), iarg->value());

        soapMsg.addMethodArgument(soapArg);
    }

    // 2) send it and attempt to get a response
    QtSoapMessage response;
    try
    {
        response = msgIO(soapMsg);
        if (response.isFault())
        {
            return HAction::UndefinedFailure();
        }
    }
    catch(HException& ex)
    {
        HLOG_WARN(ex.reason());
        return HAction::UndefinedFailure();
    }

    if (m_outArgs.size() == 0)
    {
        // since there are not supposed to be any out arguments, this is a
        // valid scenario
        return HAction::Success();
    }

    // 3) parse and verify the response

    const QtSoapType& root = response.method();
    if (!root.isValid())
    {
        return HAction::UndefinedFailure();
    }

    foreach(HActionOutputArgument* oarg, m_outArgs)
    {
        const QtSoapType& arg = root[oarg->name()];
        if (!arg.isValid())
        {
            return HAction::UndefinedFailure();
        }

        outArgs->get(oarg->name())->setValue(
            convertToRightVariantType(arg.value().toString(), oarg->dataType()));
    }

    return HAction::Success();
}

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
            m_sid(), m_seq(0), m_timeout(),
            m_subscriptionTimer(this),
            m_announcementTimer(this),
            m_announcementTimedOut (false), m_service(service),
            m_serverRootUrl        (serverRootUrl),
            m_lastConnectedLocation(),
            m_exiting(false),
            m_http   (http)
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

    Q_ASSERT(ok);

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

/*******************************************************************************
 * ControlPointHttpServer
 ******************************************************************************/
ControlPointHttpServer::ControlPointHttpServer(
    HControlPointPrivate* owner, QObject* parent) :
        HHttpServer("__CONTROL POINT HTTP SERVER__: ", parent), m_owner(owner)
{
    Q_ASSERT(owner);
}

ControlPointHttpServer::~ControlPointHttpServer()
{
    HLOG(H_AT, H_FUN);
    close();
}

void ControlPointHttpServer::incomingNotifyMessage(
    MessagingInfo& mi, const NotifyRequest& req)
{
    // note, that currently this method is always executed in a thread from a
    // thread pool

    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);

    HLOG_DBG(QObject::tr("Incoming event notify from [%1]").arg(
        peerAsStr(mi.socket())));

    if (m_owner->m_initializationStatus != 2)
    {
        HLOG_DBG(QObject::tr("The control point is not ready to accept notifications. Ignoring."));
        return;
    }

    QMutexLocker lock(&m_owner->m_serviceSubscribtionsMutex);

    QString serviceCallbackId = req.callback().path().remove('/');
    HServiceSubscribtion* subscription =
        m_owner->m_serviceSubscribtions[serviceCallbackId];

    lock.unlock();

    if (!subscription)
    {
        HLOG_WARN(QObject::tr("Ignoring notification due to invalid callback ID [%1]").arg(
            serviceCallbackId));

        mi.setKeepAlive(false);
        m_httpHandler.responseBadRequest(mi);
        return;
    }

    subscription->onNotify(mi, req);
}

/*******************************************************************************
 * FetchAndAddDevice
 ******************************************************************************/
template<typename Msg>
void FetchAndAddDevice<Msg>::createEventSubscriptions(
    HDeviceController* device,
    QList<HServiceSubscribtion* >* subscriptions)
{
    HLOG(H_AT, H_FUN);
    Q_ASSERT(device);
    Q_ASSERT(subscriptions);

    QList<HServiceController*> services = device->services();
    foreach(HServiceController* service, services)
    {
        if (service->m_service->isEvented())
        {
            HServiceSubscribtion* subscription =
                new HServiceSubscribtion(
                    m_owner->m_loggingIdentifier, m_owner->m_http,
                    device->m_device->locations(),
                    service, m_owner->m_server->rootUrl(), m_owner->m_threadPool);

            subscriptions->push_back(subscription);
        }
    }

    QList<HDeviceController*> devices = device->embeddedDevices();
    foreach(HDeviceController* embDevice, devices)
    {
        createEventSubscriptions(embDevice, subscriptions);
    }
}

template<typename Msg>
FetchAndAddDevice<Msg>::FetchAndAddDevice(
    HControlPointPrivate* owner, const Msg& msg) :
        m_owner(owner), m_msg(msg), m_completionValue(-1),
        m_createdDevice(0)
{
}

template<typename Msg>
FetchAndAddDevice<Msg>::~FetchAndAddDevice()
{
   HLOG(H_AT, H_FUN);

    if (m_createdDevice.data())
    {
        m_createdDevice->deleteLater();
    }

    m_createdDevice.take();
}

template<typename Msg>
qint32 FetchAndAddDevice<Msg>::completionValue() const
{
    return m_completionValue;
}

template<typename Msg>
QString FetchAndAddDevice<Msg>::errorString() const
{
    return m_errorString;
}

template<typename Msg>
HDeviceController* FetchAndAddDevice<Msg>::createdDevice()
{
    return m_createdDevice.take();
}

template<typename Msg>
void FetchAndAddDevice<Msg>::deleteSubscriptions(
    const QList<HServiceSubscribtion*>& subscriptions)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);

    QMutexLocker lock(&m_owner->m_serviceSubscribtionsMutex);
    foreach(HServiceSubscribtion* ss, subscriptions)
    {
        if (m_owner->m_serviceSubscribtions.contains(ss->id()))
        {
            m_owner->m_serviceSubscribtions.remove(ss->id());
            ss->deleteLater();
        }
    }
}

template<typename Msg>
void FetchAndAddDevice<Msg>::run()
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);

    HUdn udn = m_msg.usn().udn();

    QList<HServiceSubscribtion*> subscriptions;
    try
    {
        QScopedPointer<HDeviceController> device(
            m_owner->fetchDevice(m_msg.location(), m_msg.cacheControlMaxAge()));

        // the returned device is a fully built root device containing every
        // embedded device and service advertised in the device and service descriptions
        // otherwise, the creation failed and an exception was thrown

        if (m_owner->state() != HControlPointPrivate::Initialized)
        {
            m_completionValue = -1;
            m_errorString = QObject::tr("Shutting down. Aborting device model build.");
            emit done(udn);

            return;
        }

        createEventSubscriptions(device.data(), &subscriptions);

        QMutexLocker lock(&m_owner->m_serviceSubscribtionsMutex);
        for(qint32 i = 0; i < subscriptions.size(); ++i)
        {
            subscriptions[i]->moveToThread(m_owner->thread());
            m_owner->m_serviceSubscribtions.insert(
                subscriptions[i]->id(), subscriptions[i]);
        }
        lock.unlock();

        // after the subscriptions are created, attempt to subscribe to every
        // service the subscriptions are representing.
        for(qint32 i = 0; i < subscriptions.size(); ++i)
        {
            if (m_owner->state() != HControlPointPrivate::Initialized)
            {
                break;
            }

            subscriptions[i]->subscribe();
        }

        if (m_owner->state() != HControlPointPrivate::Initialized)
        {
            deleteSubscriptions(subscriptions);
            m_completionValue = -1;
            m_errorString = QObject::tr("Shutting down. Aborting device model build.");

            emit done(udn);
        }
        else
        {
            device->moveToThread(m_owner->thread());
            device->m_device->moveToThread(m_owner->thread());

            m_completionValue = 0;
            m_createdDevice.swap(device);

            emit done(udn);
        }
    }
    catch(HException& ex)
    {
        HLOG_WARN(QObject::tr("Couldn't create a device: %1").arg(ex.reason()));

        deleteSubscriptions(subscriptions);

        m_completionValue = -1;
        m_errorString = ex.reason();

        emit done(udn);
    }
}

/*******************************************************************************
 * HControlPointPrivate
 ******************************************************************************/
namespace
{

class ServiceDescriptionRetriever
{
private:

    QByteArray m_loggingIdentifier;
    HHttpHandler* m_http;

public:

    ServiceDescriptionRetriever(
        const QByteArray& loggingIdentifier, HHttpHandler* http) :
            m_loggingIdentifier(loggingIdentifier), m_http(http)
    {
        Q_ASSERT(m_http);
    }

    QDomDocument operator()(const QUrl& deviceLocation, const QUrl& scpdUrl)
    {
        HLOG2(H_AT, H_FUN, m_loggingIdentifier);

        HLOG_DBG(QObject::tr(
            "Attempting to fetch a service description for [%1] from: [%2]").arg(
                scpdUrl.toString(), deviceLocation.toString()));

        QString scpdPath = scpdUrl.path();
        QString path(deviceLocation.path());
        if (!path.endsWith('/'))
        {
            path.append('/');
        }
        if (scpdPath.startsWith('/'))
        {
            scpdPath.remove(0, 1);
        }
        path.append(scpdPath);

        QHttpRequestHeader requestHdr("GET", path);
        QHttpResponseHeader responseHdr;

        QTcpSocket sock;
        sock.connectToHost(deviceLocation.host(), deviceLocation.port());
        if (!sock.waitForConnected(5000))
        {
            throw HSocketException(
                QObject::tr("Could not retrieve service description for [%1] from: [%2]").arg(
                    scpdUrl.toString(), deviceLocation.toString()));
        }

        MessagingInfo mi(sock, false, 5000);
        mi.setHostInfo(deviceLocation);

        QString body = m_http->msgIO(mi, requestHdr, responseHdr);
        if (!body.size())
        {
            throw HOperationFailedException(
                QObject::tr("Ignoring invalid response: no message body"));
        }

        QDomDocument dd;
        QString errMsg; qint32 errLine = 0;
        if (!dd.setContent(body, false, &errMsg, &errLine))
        {
            throw HParseException(
                QObject::tr("Could not parse the service description: [%1] @ line [%2]").
                arg(errMsg, QString::number(errLine)));
        }

        return dd;
    }
};

class IconRetriever
{
private:

    QByteArray m_loggingIdentifier;
    HHttpHandler* m_http;

public:

    IconRetriever(
        const QByteArray& loggingIdentifier, HHttpHandler* http) :
            m_loggingIdentifier(loggingIdentifier), m_http(http)
    {
        Q_ASSERT(m_http);
    }

    QImage operator()(const QUrl& deviceLocation, const QUrl& iconUrl)
    {
        HLOG2(H_AT, H_FUN, m_loggingIdentifier);

        HLOG_DBG(QObject::tr(
            "Attempting to retrieve icon [%1] from: [%2]").arg(
                iconUrl.toString(), deviceLocation.toString()));

        QString iconPath = iconUrl.path();
        QString path(deviceLocation.path());
        if (!path.endsWith('/')) { path.append('/'); }
        if (iconPath.startsWith('/')) { iconPath.remove(0, 1); }
        path.append(iconPath);

        QHttpRequestHeader requestHdr("GET", path);
        QHttpResponseHeader responseHdr;

        QTcpSocket sock;
        sock.connectToHost(deviceLocation.host(), deviceLocation.port());
        if (!sock.waitForConnected(5000))
        {
            throw HSocketException(
                QObject::tr("Could not retrieve icon for [%1] from: [%2]").arg(
                    iconUrl.toString(), deviceLocation.toString()));
        }

        MessagingInfo mi(sock, false, 5000);
        mi.setHostInfo(deviceLocation);

        QByteArray body = m_http->msgIO(mi, requestHdr, responseHdr);
        if (!body.size())
        {
            throw HOperationFailedException(
                QObject::tr("Ignoring invalid response: no icon data received"));
        }

        QImage image;
        if (!image.loadFromData(body))
        {
            throw HParseException(
                QObject::tr("The retrieved data is not a proper icon"));
        }

        return image;
    }
};

HActionInvoke actionInvokeCreator(
    HService* service, const QString& actionName,
    const HActionInputArguments& inArgs,
    const HActionOutputArguments& outArgs)
{
    return HActionInvokeProxy(service, actionName, inArgs, outArgs);
}
}

HDeviceController* HControlPointPrivate::fetchDevice(
    QUrl deviceLocation, qint32 maxAgeInSecs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    HLOG_DBG(QObject::tr(
        "Attempting to fetch a device description from: [%1]").arg(
            deviceLocation.toString()));

    QTcpSocket tcp;
    QHttpRequestHeader requestHdr("GET", deviceLocation.path());
    tcp.connectToHost(deviceLocation.host(), deviceLocation.port());
    if (!tcp.waitForConnected(5000))
    {
        QString err = QObject::tr(
            "Failed to fetch device description: couldn't connect to host @ [%1]").arg(
                deviceLocation.toString());

        throw HSocketException(err);
    }

    MessagingInfo mi(tcp, false);
    mi.setHostInfo(deviceLocation);

    QString body;
    QHttpResponseHeader responseHdr;
    try
    {
        body = m_http.msgIO(mi, requestHdr, responseHdr);
    }
    catch(HException& ex)
    {
        QString err = QObject::tr(
            "Failed to fetch device description: ").arg(ex.reason());

        throw HOperationFailedException(ex, err);
    }

    if (!body.size())
    {
        throw HOperationFailedException(
            QObject::tr("Ignoring invalid response: no message body."));
    }

    QDomDocument dd;
    QString errMsg; qint32 errLine = 0;
    if (!dd.setContent(body, false, &errMsg, &errLine))
    {
        throw InvalidDeviceDescription(
            QObject::tr("Could not parse the device description file: [%1] @ line [%2]:\n[%3]").
            arg(errMsg, QString::number(errLine), body));
    }

    QList<QUrl> deviceLocations;
    deviceLocations.push_back(deviceLocation);

    HObjectCreationParameters creatorParams;
    creatorParams.m_createDefaultObjects = true;
    creatorParams.m_deviceDescription    = dd;
    creatorParams.m_deviceCreator        = m_initParams->deviceCreator();
    creatorParams.m_deviceLocations      = deviceLocations;

    creatorParams.m_serviceDescriptionFetcher =
        ServiceDescriptionRetriever(m_loggingIdentifier, &m_http);

    creatorParams.m_actionInvokeCreator  = actionInvokeCreator;
    creatorParams.m_deviceTimeoutInSecs  = maxAgeInSecs;
    creatorParams.m_appendUdnToDeviceLocation = false;
    creatorParams.m_sharedActionInvokers = &m_sharedActionInvokers;

    IconRetriever iconFetcher(m_loggingIdentifier, &m_http);
    creatorParams.m_iconFetcher = iconFetcher;
    creatorParams.m_strictParsing = false;
    creatorParams.m_stateVariablesAreImmutable = true;

    HObjectCreator creator(creatorParams);

    return creator.createRootDevice();
}

HControlPointPrivate::HControlPointPrivate() :
    HAbstractHostPrivate(
        QString("__CONTROL POINT %1__: ").arg(QUuid::createUuid().toString())),
            m_buildsInProgress(),
            m_initParams(),  m_ssdp(0),
            m_server    (0), m_serviceSubscribtions(),
            m_serviceSubscribtionsMutex(QMutex::Recursive),
            m_deviceCreationMutex()
{
    HLOG(H_AT, H_FUN);
}

HControlPointPrivate::~HControlPointPrivate()
{
    HLOG(H_AT, H_FUN);

    QList<DeviceBuildProcess*> builds = m_buildsInProgress.values();
    foreach(DeviceBuildProcess* build, builds)
    {
        delete build->m_asyncOperation;
        delete build;
    }
}

void HControlPointPrivate::addRootDevice_(
    HDeviceController* newRootDevice)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    HDeviceController* existingDevice =
        searchDeviceByUdn(newRootDevice->m_device->deviceInfo().udn());

    if (existingDevice)
    {
        Q_ASSERT(!existingDevice->m_device->parentDevice());

        existingDevice->addLocations(newRootDevice->m_device->locations());
        return;
    }

    newRootDevice->setParent(this);
    newRootDevice->m_device->setParent(this);
    newRootDevice->startStatusNotifier(HDeviceController::All);

    Q_ASSERT(QObject::connect(
            newRootDevice, SIGNAL(statusTimeout(HDeviceController*)),
            this, SLOT(deviceExpired(HDeviceController*))));

    try
    {
        addRootDevice(newRootDevice);
    }
    catch(HException& ex)
    {
        HLOG_WARN(QObject::tr(
            "Failed to add root device [UDN: %1]: %2").arg(
                newRootDevice->m_device->deviceInfo().udn().toSimpleUuid(),
                ex.reason()));

        removeRootDeviceSubscriptions(newRootDevice, true);
        delete newRootDevice;
    }
}

void HControlPointPrivate::deviceExpired(HDeviceController* source)
{
    HLOG(H_AT, H_FUN);
    QMutexLocker lock(&m_rootDevicesMutex);

    // according to the UDA v1.1 a "device tree" (root, embedded and services)
    // are "timed out" only when every advertisement has timed out.

    source = source->rootDevice();

    if (source->isTimedout(HDeviceController::All))
    {
        removeRootDeviceAndSubscriptions(source, false);
    }
}

bool HControlPointPrivate::discoveryRequestReceived(
    const HDiscoveryRequest&, const HEndpoint&, const HEndpoint&)
{
    return true;
}

void HControlPointPrivate::removeRootDeviceSubscriptions(
    HDeviceController* rootDevice, bool unsubscribe)
{
    Q_ASSERT(rootDevice);
    Q_ASSERT(!rootDevice->m_device->parentDevice());
    // this method should be called only with root devices

    // when removing a root device all of the subscriptions for services contained
    // within the root device have to be removed as well.

    QMutexLocker lock(&m_serviceSubscribtionsMutex);

    QList<HServiceSubscribtion*> subscriptions = m_serviceSubscribtions.values();
    foreach(HServiceSubscribtion* subscription, subscriptions)
    {
        // seek the root device of the device tree to which the service that contains
        // the subscription belongs.
        const HDevice* device = subscription->m_service->m_service->parentDevice();
        while(device->parentDevice()) { device = device->parentDevice(); }

        if (device == rootDevice->m_device.data())
        {
            // the service appears to belong to the device tree that is about
            // to be removed

            qint32 i = m_serviceSubscribtions.remove(subscription->m_randomIdentifier);
            Q_ASSERT(i == 1); Q_UNUSED(i)

            if (unsubscribe)
            {
                subscription->unsubscribe(true);
            }

            delete subscription;
        }
    }
}

void HControlPointPrivate::removeRootDeviceAndSubscriptions(
    HDeviceController* rootDevice, bool unsubscribe)
{
    HLOG(H_AT, H_FUN);

    removeRootDeviceSubscriptions(rootDevice, unsubscribe);
    removeRootDevice(rootDevice);
}

template<typename Msg>
bool HControlPointPrivate::processDeviceDiscovery(
    const Msg& msg, const HEndpoint& /*source*/)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(thread() == QThread::currentThread());

    HUdn resourceUdn = msg.usn().udn();

    QMutexLocker lock(&m_rootDevicesMutex);
    HDeviceController* device = searchDeviceByUdn(msg.usn().udn());

    if (device)
    {
        // according to the UDA v1.1 spec, if a control point receives an alive announcement
        // of any type for a device tree, the control point can assume that
        // all devices and services are available. ==> reset timeouts
        // for entire device tree and all services.

        device = device->rootDevice();
        device->startStatusNotifier(HDeviceController::All);

        // it cannot be that only some embedded device is available at certain
        // interface, since the device description is always fetched from the
        // the location that the root device specifies. ergo, the entire device
        // tree has to be available at that location.
        device->addLocation(msg.location());
        return true;
    }

    // it does not matter if the device is an embedded device, since the
    // location of the device points to the root device's description in any case
    // and the internal device model is built of that. Hence, it is only necessary
    // to get an advertisement of a root or an embedded device to build the entire
    // model correctly.

    DeviceBuildProcess* dbp = m_buildsInProgress.get(msg);
    if (dbp)
    {
        if (!dbp->m_locations.contains(msg.location()))
        {
            dbp->m_locations.push_back(msg.location());
        }

        return true;
    }

    FetchAndAddDevice<Msg>* task = new FetchAndAddDevice<Msg>(this, msg);
    task->setAutoDelete(false);

    DeviceBuildProcess* newBuildProcess = new DeviceBuildProcess();
    newBuildProcess->m_asyncOperation = task;
    newBuildProcess->m_locations.push_back(msg.location());
    newBuildProcess->m_udn.reset(new HUdn(resourceUdn));

    m_buildsInProgress.add(newBuildProcess);

    bool ok = connect(
        task, SIGNAL(done(Herqq::Upnp::HUdn)),
        this, SLOT(deviceModelBuildDone(Herqq::Upnp::HUdn)));

    Q_ASSERT(ok); Q_UNUSED(ok)

    HLOG_INFO(QObject::tr("New resource [%1] is available @ [%2]. Attempting to build the device model.").arg(
        msg.usn().resource().toString(), msg.location().toString()));

    m_threadPool->start(task);

    return true;
}

void HControlPointPrivate::deviceModelBuildDone(Herqq::Upnp::HUdn udn)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(thread() == QThread::currentThread());

    DeviceBuildProcess* build = m_buildsInProgress.get(udn);
    Q_ASSERT(build);

    if (build->m_asyncOperation->completionValue() == 0)
    {
        HLOG_INFO(QObject::tr("Device model for [%1] built successfully.").arg(
            udn.toString()));

        HDeviceController* device = build->m_asyncOperation->createdDevice();
        Q_ASSERT(device);

        for (qint32 i = 0; i < build->m_locations.size(); ++i)
        {
            device->addLocation(build->m_locations[i]);
        }

        addRootDevice_(device);
    }
    else
    {
        HLOG_WARN(QObject::tr("Device model for [%1] could not be built: %2.").arg(
            udn.toString(), build->m_asyncOperation->errorString()));
    }

    m_buildsInProgress.remove(udn);

    delete build->m_asyncOperation;
    delete build;
}

bool HControlPointPrivate::discoveryResponseReceived(
    const HDiscoveryResponse& msg, const HEndpoint& source)
{
    HLOG(H_AT, H_FUN);
    return processDeviceDiscovery(msg, source);
}

bool HControlPointPrivate::resourceUnavailableReceived(
    const HResourceUnavailable& msg)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QMutexLocker lock(&m_rootDevicesMutex);

    HDeviceController* device = searchDeviceByUdn(msg.usn().udn());
    if (!device)
    {
        // the device is not (for whatever reason) known by us.
        // note that even service announcements contain the "UDN", which identifies
        // the device that contains them.
        return true;
    }

    HLOG_INFO(QObject::tr("Resource [%1] is unavailable.").arg(
        msg.usn().resource().toString()));

    // according to the UDA v1.1 specification, if a bye bye message of any kind
    // is received, the control point can assume that nothing in that
    // device tree is available anymore

    HDeviceController* root = device->rootDevice();
    Q_ASSERT(root);

    removeRootDeviceAndSubscriptions(root, false);

    return true;
}

bool HControlPointPrivate::resourceAvailableReceived(
    const HResourceAvailable& msg)
{
    HLOG(H_AT, H_FUN);
    return processDeviceDiscovery(msg);
}

bool HControlPointPrivate::readyForEvents()
{
    return m_initializationStatus == 2;
}

void HControlPointPrivate::doClear()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(thread() == QThread::currentThread());

    // called by the abstract host just before it starts to delete the device
    // tree.

    Q_ASSERT(state() == Exiting);

    m_http.shutdown(false);
    // this will tell the http handler that operations should quit as
    // soon as possible.

    delete m_server; m_server = 0;
    // shut down the http server. this call will block until all threads
    // created by the http server have finished. however, that should be fairly
    // fast, since every operation in this class monitors the shutdown flag.

    QMutexLocker lock(&m_serviceSubscribtionsMutex);
    QList<HServiceSubscribtion*> subscriptions =
        m_serviceSubscribtions.values();

    QList<HServiceSubscribtion*>::iterator i = subscriptions.begin();
    for(; i != subscriptions.end(); ++i)
    {
        try
        {
            (*i)->unsubscribe(true);
        }
        catch(HException&)
        {
            // intentional. at most could print something.
        }

        delete *i;
    }

    m_serviceSubscribtions.clear();

    m_threadPool->waitForDone();
    // ensure that no threads created by this thread pool are running when we
    // start deleting shared objects.

    delete m_ssdp; m_ssdp = 0;

    m_initParams.reset(0);

    m_initializationStatus = 0;

    // once this method exists, the abstract host will proceed to delete
    // the device tree, which is safe by now.
}

/*******************************************************************************
 * HControlPoint
 ******************************************************************************/
HControlPoint::HControlPoint(QObject* parent) :
    HAbstractHost(*new HControlPointPrivate(), parent)
{
    HLOG(H_AT, H_FUN);
}

HControlPoint::~HControlPoint()
{
    HLOG(H_AT, H_FUN);
    quit();
}

HControlPoint::ReturnCode HControlPoint::init(
    const HControlPointConfiguration* initParams, QString* errorString)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    H_D(HControlPoint);

    Q_ASSERT_X(
        thread() == QThread::currentThread(), H_AT,
        "The control point has to be initialized in the thread in which it is currently located.");

    if (h->state() == HAbstractHostPrivate::Initialized)
    {
        return AlreadyInitialized;
    }

    Q_ASSERT(h->state() == HAbstractHostPrivate::Uninitialized);

    QString error;
    HControlPoint::ReturnCode rc = Success;
    try
    {
        h->setState(HAbstractHostPrivate::Initializing);

        HLOG_INFO(QObject::tr("ControlPoint initializing."));

        h->m_initParams.reset(initParams ?
            initParams->clone() : new HControlPointConfiguration());

        h->m_server = new ControlPointHttpServer(h, this);
        if (!h->m_server->listen())
        {
            rc = UndefinedFailure;
        }
        else
        {
            h->m_ssdp = new SsdpWithoutEventing<HControlPointPrivate>(h, this);

            HLOG_DBG(QObject::tr("Searching for UPnP devices..."));

            h->m_ssdp->sendDiscoveryRequest(
                HDiscoveryRequest(1, HResourceIdentifier("ssdp:all"), herqqProductTokens()));

            h->setState(HAbstractHostPrivate::Initialized);
        }
    }
    catch(HException& ex)
    {
        error = ex.reason();
        rc = UndefinedFailure;
    }

    if (rc != Success)
    {
        HLOG_WARN(error);

        if (errorString)
        {
            *errorString = error;
        }

        h->setState(HAbstractHostPrivate::Exiting);
        h->clear();
        HLOG_INFO(QObject::tr("ControlPoint initialization failed."));

        return rc;
    }

    HLOG_INFO(QObject::tr("ControlPoint initialized."));

    return rc;
}

void HControlPoint::quit()
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    H_D(HControlPoint);

    Q_ASSERT_X(
        thread() == QThread::currentThread(), H_AT,
        "The control point has to be shutdown in the thread in which it is currently located.");

    if (!isStarted())
    {
        return;
    }

    HLOG_INFO(QObject::tr("ControlPoint shutting down."));

    h->setState(HAbstractHostPrivate::Exiting);
    h->clear();

    HLOG_INFO(QObject::tr("ControlPoint shut down."));
}

/*******************************************************************************
 * IFetchAndAddDevice
 ******************************************************************************/
IFetchAndAddDevice::IFetchAndAddDevice()
{
}

IFetchAndAddDevice::~IFetchAndAddDevice()
{
}

/*******************************************************************************
 * HControlPointConfigurationPrivate
 ******************************************************************************/
HControlPointConfigurationPrivate::HControlPointConfigurationPrivate() :
    m_deviceCreator()
{
}

HControlPointConfigurationPrivate::~HControlPointConfigurationPrivate()
{
}

/*******************************************************************************
 * HControlPointConfiguration
 ******************************************************************************/
HControlPointConfiguration::HControlPointConfiguration() :
    h_ptr(new HControlPointConfigurationPrivate())
{
}

HControlPointConfiguration::HControlPointConfiguration(
    HControlPointConfigurationPrivate& dd) :
        h_ptr(&dd)
{
}

HControlPointConfiguration::~HControlPointConfiguration()
{
    delete h_ptr;
}

HControlPointConfiguration* HControlPointConfiguration::clone() const
{
    HLOG(H_AT, H_FUN);

    HControlPointConfiguration* clone =
        new HControlPointConfiguration(
            *new HControlPointConfigurationPrivate(*h_ptr));

    return clone;
}

HDeviceCreator HControlPointConfiguration::deviceCreator() const
{
    return h_ptr->m_deviceCreator;
}

void HControlPointConfiguration::setDeviceCreator(
    HDeviceCreator deviceCreator)
{
    h_ptr->m_deviceCreator = deviceCreator;
}

}
}
