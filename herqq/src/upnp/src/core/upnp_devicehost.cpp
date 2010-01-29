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

#include "upnp_devicehost.h"
#include "upnp_devicehost_p.h"

#include "upnp_device_p.h"
#include "upnp_global_p.h"
#include "upnp_service_p.h"
#include "datatype_mappings_p.h"
#include "upnp_objectcreator_p.h"
#include "upnp_actionarguments.h"

#include "upnp_udn.h"
#include "upnp_resourcetype.h"

#include "messaging/usn.h"
#include "messaging/endpoint.h"
#include "messaging/discovery_messages.h"
#include "messaging/resource_identifier.h"

#include "../../../utils/src/logger_p.h"
#include "../../../utils/src/sysutils_p.h"

#include "../../../core/include/HExceptions"

#include <QDateTime>
#include <QMetaType>
#include <QTcpSocket>
#include <QDomDocument>
#include <QImageReader>
#include <QHttpRequestHeader>

#include <QtSoapMessage>

#include <ctime>

static bool registerMetaTypes()
{
    static QAtomicInt tester(0);

    if (tester.testAndSetAcquire(0, 1))
    {
        qRegisterMetaType<Herqq::Upnp::HDeviceHostConfiguration>(
            "Herqq::Upnp::HDeviceHostConfiguration");
    }

    return true;
}

static bool test = registerMetaTypes();

namespace Herqq
{

namespace Upnp
{

namespace
{
void getCurrentValues(QByteArray& msgBody, const HService* service)
{
    HLOG(H_AT, H_FUN);

    QDomDocument dd;

    QDomProcessingInstruction proc =
        dd.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");

    dd.appendChild(proc);

    QDomElement propertySetElem =
        dd.createElementNS("urn:schemas-upnp-org:event-1-0", "e:propertyset");

    dd.appendChild(propertySetElem);

    QList<HStateVariable*> stateVars = service->stateVariables();
    QList<HStateVariable*>::const_iterator ci = stateVars.constBegin();
    for(; ci != stateVars.constEnd(); ++ci)
    {
        HStateVariable* stateVar = *ci;
        Q_ASSERT(stateVar);

        if (stateVar->eventingType() == HStateVariable::NoEvents)
        {
            continue;
        }

        QDomElement propertyElem =
            dd.createElementNS("urn:schemas-upnp-org:event-1-0", "e:property");

        QDomElement variableElem = dd.createElement(stateVar->name());
        variableElem.appendChild(dd.createTextNode(stateVar->value().toString()));

        propertyElem.appendChild(variableElem);
        propertySetElem.appendChild(propertyElem);
    }

    msgBody = dd.toByteArray();
}
}

/*******************************************************************************
 * UnicastRemoteClient::MessageSender
 ******************************************************************************/
namespace
{
bool notifyClient(
    HHttpHandler& http, MessagingInfo& mi,
    const QByteArray& msgBody, const QUrl& location,
    const HSid& sid, quint32 seq)
{
    HLOG2(H_AT, H_FUN, "__DEVICE HOST__: ");
    Q_ASSERT(!sid.isNull());
    Q_ASSERT(!msgBody.isEmpty() && !msgBody.isNull());

    if (mi.socket().state() != QTcpSocket::ConnectedState)
    {
        HLOG_WARN(QString("Client @ [sid: %1] is not connected. Failed to notify.").arg(
            sid.toString()));

        return false;
    }

    NotifyRequest req(location, sid, seq, msgBody);
    try
    {
        HLOG_DBG(QString("Sending notification [seq: %1] to subscriber [%2] @ [%3]").arg(
            QString::number(seq), sid.toString(), location.toString()));

        http.msgIO(mi, req);
    }
    catch(HException& ex)
    {
        HLOG_WARN(QString("An error occurred while notifying [seq: %1, sid: %2] host @ [%3]: %4").arg(
            QString::number(seq), sid.toString(), location.toString(), ex.reason()));

        return false;
    }

    return true;
}
}

UnicastRemoteClient::MessageSender::MessageSender(UnicastRemoteClient* owner) :
    m_owner(owner), m_socket(0), m_messagesToSend(),
    m_messagesToSendMutex(), m_messagesAvailable(), m_shuttingDown(false),
    m_done(false)
{
}

bool UnicastRemoteClient::MessageSender::connect()
{
    HLOG2(H_AT, H_FUN, "__DEVICE HOST__: ");

    Q_ASSERT(m_socket.data());

    if (m_socket->state() == QTcpSocket::ConnectedState)
    {
        return true;
    }

    Q_ASSERT(QThread::currentThread() == m_socket->thread());

    m_socket->connectToHost(
        m_owner->m_location.host(), m_owner->m_location.port());

    QTime stopWatch; stopWatch.start();
    while(!m_shuttingDown && stopWatch.elapsed() < 15000)
    {
        if (m_socket->waitForConnected(50))
        {
            return true;
        }
    }

    return false;
}

void UnicastRemoteClient::MessageSender::run()
{
    HLOG2(H_AT, H_FUN, "__DEVICE HOST__: ");

    m_socket.reset(new QTcpSocket());

    for(;;)
    {
        if (m_shuttingDown)
        {
            HLOG_DBG(QString("Aborting notifications to host @ [%1].").arg(
                m_owner->m_location.toString()));

            break;
        }

        QMutexLocker lock(&m_messagesToSendMutex);
        if (m_messagesToSend.isEmpty())
        {
            if (!m_messagesAvailable.wait(&m_messagesToSendMutex, 50))
            {
                continue;
            }
        }
        lock.unlock();

        if (!connect())
        {
            if (m_shuttingDown)
            {
                HLOG_DBG(QString("Aborting notifications to host @ [%1].").arg(
                    m_owner->m_location.toString()));
            }
            else
            {
                HLOG_WARN(QString("Couldn't connect to host @ [%1]. Aborting notifications.").arg(
                    m_owner->m_location.toString()));
            }

            break;
        }

        // there are messages queued and we are connected ==>
        // attempt to empty the message queue

        MessagingInfo mi(*m_socket, true, 30000);
        // timeout mandated by the UDA v 1.1

        for(;;)
        {
            lock.relock();
            if (m_messagesToSend.isEmpty())
            {
                break;
            }
            QByteArray message = m_messagesToSend.dequeue();
            lock.unlock();

            if (m_shuttingDown)
            {
                HLOG_DBG(QString("Aborting notifications to host @ [%1].").arg(
                    m_owner->m_location.toString()));

                goto end;
            }
            else if (m_socket->state() != QTcpSocket::ConnectedState)
            {
                HLOG_WARN(QString("Client [%1] has disconnected. Attempting to reconnect.").arg(
                    m_owner->m_location.toString()));

                break;
            }

            if (notifyClient(
                    m_owner->m_http, mi, message, m_owner->m_location,
                    m_owner->m_sid, m_owner->m_seq.fetchAndAddOrdered(1)))
            {
                // notify succeeded
                continue;
            }

            // notify failed
            //
            // according to UDA v1.1:
            // "the publisher SHOULD abandon sending this message to the
            // subscriber but MUST keep the subscription active and send future event
            // messages to the subscriber until the subscription expires or is canceled."

            HLOG_WARN(QObject::tr("Could not send notify [seq: %1, sid: %2] to host @ [%3].").arg(
                QString::number(m_owner->m_seq), m_owner->m_sid.toString(),
                m_owner->m_location.toString()));
        }
    }

end:

    m_socket.reset(0);
    m_owner->expire();
    m_done = true;
}

/*******************************************************************************
 * UnicastRemoteClient
 ******************************************************************************/
UnicastRemoteClient::UnicastRemoteClient(
    HHttpHandler& http, QThreadPool& tp, HService* service,
    const QUrl location, const HTimeout& timeout, QObject* parent) :
        QObject(parent),
            m_http   (http),
            m_service(service), m_location(location),
            m_sid    (QUuid::createUuid()), m_seq(0),
            m_timeout(timeout),
            m_shuttingDown(0), m_timer(this),
            m_messageSender(this), m_threadPool(tp),
            m_expirationMutex()
{
    HLOG2(H_AT, H_FUN, "__DEVICE HOST__: ");

    Q_ASSERT(service);
    Q_ASSERT(location.isValid());

    bool ok = connect(&m_timer, SIGNAL(timeout()), this, SLOT(subscriptionTimeout()));

    Q_ASSERT(ok); Q_UNUSED(ok)

    if (!timeout.isInfinite())
    {
        m_timer.start(timeout.value() * 1000);
    }

    m_messageSender.setAutoDelete(false);
    m_threadPool.start(&m_messageSender);
}

UnicastRemoteClient::~UnicastRemoteClient()
{
    HLOG(H_AT, H_FUN);

    expire();

    // TODO blocking the event loop the control point uses is not a good idea,
    // even for short durations. change this.
    while(!m_messageSender.m_done)
    {
        // cannot exit until the sender is done, since the sender holds a reference
        // back to this object.
        HSysUtils::msleep(1);
    }
}

void UnicastRemoteClient::subscriptionTimeout()
{
    HLOG(H_AT, H_FUN);

    expire();

    HLOG_DBG(QObject::tr("Subscription from [%1] with SID %2 expired").arg(
        m_location.toString(), m_sid.toString()));
}

void UnicastRemoteClient::expire()
{
    HLOG(H_AT, H_FUN);

    QMutexLocker lock(&m_expirationMutex);

    if (!m_shuttingDown.testAndSetOrdered(0, 1))
    {
        return;
    }

    if (m_timer.isActive())
    {
        m_timer.stop();
    }

    m_messageSender.m_shuttingDown = true;
}

bool UnicastRemoteClient::isInterested(const HService* service) const
{
    HLOG(H_AT, H_FUN);

    return !expired() && m_seq && m_service->isEvented() &&
            m_service->serviceId() == service->serviceId();
}

void UnicastRemoteClient::renew()
{
    HLOG(H_AT, H_FUN);

    QMutexLocker lock(&m_expirationMutex);

    if (expired())
    {
        return;
    }

    m_timer.start();
}

void UnicastRemoteClient::notify(const QByteArray& msgBody)
{
    HLOG2(H_AT, H_FUN, "__DEVICE HOST__: ");

    Q_ASSERT(m_seq);

    QMutexLocker lock(&m_messageSender.m_messagesToSendMutex);
    m_messageSender.m_messagesToSend.enqueue(msgBody);
    m_messageSender.m_messagesAvailable.wakeOne();
}

bool UnicastRemoteClient::initialNotify(
    const QByteArray& msg, MessagingInfo* mi)
{
    HLOG2(H_AT, H_FUN, "__DEVICE HOST__: ");

    Q_ASSERT(!m_seq);

    if (!mi)
    {
        QMutexLocker lock(&m_messageSender.m_messagesToSendMutex);
        m_messageSender.m_messagesToSend.enqueue(msg);
        m_messageSender.m_messagesAvailable.wakeOne();
        return true;
    }

    if (!notifyClient(m_http, *mi, msg, m_location, m_sid, m_seq))
    {
        return false;
    }

    m_seq.fetchAndAddOrdered(1);
    return true;
}

/*******************************************************************************
 * RemoteClientNotifier
 ******************************************************************************/
RemoteClientNotifier::RemoteClientNotifier(HDeviceHostPrivate* owner) :
    QObject(owner),
        m_owner(owner),
        m_remoteClients(),
        m_remoteClientsMutex(QMutex::Recursive)
{
    HLOG(H_AT, H_FUN);
    Q_ASSERT(m_owner);
}

RemoteClientNotifier::~RemoteClientNotifier()
{
    HLOG(H_AT, H_FUN);

    QMutexLocker lock(&m_remoteClientsMutex);
    m_remoteClients.clear();
}

namespace
{
inline bool isSameService(HService* srv1, HService* srv2)
{
    HLOG(H_AT, H_FUN);
    return srv1->parentDevice()->deviceInfo().udn() ==
           srv2->parentDevice()->deviceInfo().udn() &&
           srv1->scpdUrl() == srv2->scpdUrl();
}
}

RemoteClientNotifier::RemoteClientPtrT RemoteClientNotifier::remoteClient(
    const HSid& sid) const
{
    HLOG(H_AT, H_FUN);
    QMutexLocker lock(&m_remoteClientsMutex);

    QList<RemoteClientPtrT>::const_iterator it = m_remoteClients.constBegin();
    for(; it != m_remoteClients.end(); ++it)
    {
        if ((*it)->sid() == sid)
        {
            return *it;
        }
    }

    return RemoteClientPtrT(0);
}

namespace
{
void doDeleteLater(UnicastRemoteClient* obj)
{
    obj->deleteLater();
}
}

RemoteClientNotifier::RemoteClientPtrT RemoteClientNotifier::addSubscriber(
    HService* service, const SubscribeRequest& sreq)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);

    Q_ASSERT(service->isEvented());
    // The UDA v1.1 does not specify what to do when a subscription is received
    // to a service that is not evented. A "safe" route was taken here and
    // all subscriptions are accepted rather than returning some error. However,
    // in such a case the timeout is adjusted to a day and no events are ever sent.
    // This is enforced at the HService class, which should not send any
    // events unless one or more of its state variables are evented.

    QMutexLocker lock(&m_remoteClientsMutex);

    for(qint32 i = 0; i < m_remoteClients.size(); ++i)
    {
        RemoteClientPtrT rc = m_remoteClients[i];

        if (isSameService(rc->service(), service) &&
            sreq.callbacks().contains(rc->location()))
        {
            HLOG_WARN(QObject::tr(
                "subscriber [%1] to the specified service URL [%2] already exists").arg(
                    rc->location().toString(), service->scpdUrl().toString()));

            return RemoteClientPtrT(0);
        }
    }

    HLOG_INFO(QObject::tr("adding subscriber from [%1]").arg(
        sreq.callbacks().at(0).toString()));

    HTimeout timeout = service->isEvented() ? sreq.timeout() : HTimeout(60*60*24);

    RemoteClientPtrT rc(new UnicastRemoteClient(
        m_owner->m_http, *m_owner->m_threadPool,
        service, sreq.callbacks().at(0), timeout, this),
        doDeleteLater);

    m_remoteClients.push_back(rc);

    return rc;
}

bool RemoteClientNotifier::removeSubscriber(const UnsubscribeRequest& req)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);

    QMutexLocker lock(&m_remoteClientsMutex);

    QList<RemoteClientPtrT>::iterator it = m_remoteClients.begin();
    for(; it != m_remoteClients.end(); )
    {
        if ((*it)->sid() == req.sid())
        {
            HLOG_INFO(QObject::tr("removing subscriber from [%1] with SID [%2]").arg(
                (*it)->location().toString(), req.sid().toString()));

            (*it)->expire();
            it = m_remoteClients.erase(it);

            return true;
        }
        else
        {
            ++it;
        }
    }

    HLOG_WARN(QObject::tr("Could not cancel subscription. Invalid SID [%1]").arg(
        req.sid().toString()));

    return false;
}

RemoteClientNotifier::RemoteClientPtrT RemoteClientNotifier::renewSubscription(
    const SubscribeRequest& req)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);

    QMutexLocker lock(&m_remoteClientsMutex);

    QList<RemoteClientPtrT>::iterator it = m_remoteClients.begin();
    for(; it != m_remoteClients.end(); ++it)
    {
        if ((*it)->sid() == req.sid())
        {
            HLOG_INFO(QObject::tr("renewing subscription from [%1]").arg(
                (*it)->location().toString()));

            (*it)->renew();
            return (*it);
        }
    }

    HLOG_WARN(QObject::tr("Cannot renew subscription. Invalid SID: [%1]").arg(
        req.sid().toString()));

    return RemoteClientPtrT(0);
}

void RemoteClientNotifier::stateChanged(const HService* source)
{
    HLOG(H_AT, H_FUN);

    QByteArray msgBody;
    getCurrentValues(msgBody, source);

    QMutexLocker lock(&m_remoteClientsMutex);

    QList<RemoteClientPtrT>::iterator it = m_remoteClients.begin();
    for(; it != m_remoteClients.end(); )
    {
        if ((*it)->isInterested(source))
        {
            (*it)->notify(msgBody);
            ++it;
        }
        else if ((*it)->expired())
        {
            it = m_remoteClients.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

/*******************************************************************************
 * Server
 ******************************************************************************/
DeviceHostHttpServer::DeviceHostHttpServer(
    HDeviceHostPrivate* deviceHost, QObject* parent) :
        HHttpServer("__DEVICE HOST HTTP SERVER__: ", parent),
            m_deviceHost(deviceHost)
{
    HLOG(H_AT, H_FUN);
    Q_ASSERT(m_deviceHost);

    bool ok = connect(
        this,
        SIGNAL(processSubscription_sig(const SubscribeRequest*, HService*, HSid*)),
        this,
        SLOT(processSubscription_slot(const SubscribeRequest*, HService*, HSid*)),
        Qt::BlockingQueuedConnection);

    Q_ASSERT(ok);

    ok = connect(
        this, SIGNAL(removeSubscriber_sig(const UnsubscribeRequest*, bool*)),
        this, SLOT(removeSubscriber_slot(const UnsubscribeRequest*, bool*)),
        Qt::BlockingQueuedConnection);

    Q_ASSERT(ok);
}

DeviceHostHttpServer::~DeviceHostHttpServer()
{
    HLOG(H_AT, H_FUN);
    close();
}

void DeviceHostHttpServer::processSubscription_slot(
    const SubscribeRequest* req, HService* service, HSid* sid)
{
    HLOG(H_AT, H_FUN);
    Q_ASSERT(req);
    Q_ASSERT(sid);

    // The UDA v1.1 does not specify what to do when a subscription is received
    // to a service that is not evented. A "safe" route was taken here and
    // all subscriptions are accepted rather than returning some error. However,
    // in such a case the timeout is adjusted to a day and no events are ever sent.

    if (req->isRenewal())
    {
        RemoteClientNotifier::RemoteClientPtrT rc =
            m_deviceHost->m_remoteClientNotifier->renewSubscription(*req);

        if (rc)
        {
            *sid = rc->sid();
        }
    }
    else
    {
        RemoteClientNotifier::RemoteClientPtrT rc =
            m_deviceHost->m_remoteClientNotifier->addSubscriber(service, *req);

        if (rc)
        {
            *sid = rc->sid();
        }
    }
}

void DeviceHostHttpServer::removeSubscriber_slot(
    const UnsubscribeRequest* req, bool* ok)
{
    HLOG(H_AT, H_FUN);

    Q_ASSERT(req);
    Q_ASSERT(ok);

    try
    {
        *ok = m_deviceHost->m_remoteClientNotifier->removeSubscriber(*req);
    }
    catch(HException& ex)
    {
        HLOG_WARN(ex.reason());
    }
}

void DeviceHostHttpServer::incomingSubscriptionRequest(
    MessagingInfo& mi, const SubscribeRequest& sreq)
{
    HLOG2(H_AT, H_FUN, m_deviceHost->m_loggingIdentifier);

    Permission perm(*m_deviceHost);
    if (!perm.isValid())
    {
        mi.setKeepAlive(false);
        m_httpHandler.responseInternalServerError(mi);
        return;
    }

    HLOG_DBG(QObject::tr("Subscription received."));

    HServiceController* service =
        m_deviceHost->searchServiceByEventUrl(sreq.eventUrl());

    if (!service)
    {
        HLOG_WARN(QObject::tr("Subscription defined as [%1] is invalid.").arg(
            sreq.eventUrl().path()));

        mi.setKeepAlive(false);
        m_httpHandler.responseBadRequest(mi);
        return;
    }

    // have to perform a switch to the right thread so that an instance of
    // UnicastRemoteClient can be created into the thread where every other
    // HUpnp object resides. moveToThread() could be used as well, but the
    // accompanying setParent() will fail, since Qt cannot send
    // events to the "old" parent, because it lives in a different thread.
    HSid sid;
    emit processSubscription_sig(&sreq, service->m_service, &sid);
    // this is connected using BlockingQueuedConnection
    // to the local slot that does the processing (in the right thread)

    if (sid.isNull())
    {
        mi.setKeepAlive(false);
        m_httpHandler.responsePreconditionFailed(mi);
        return;
    }

    RemoteClientNotifier::RemoteClientPtrT rc =
        m_deviceHost->m_remoteClientNotifier->remoteClient(sid);

    if (!rc)
    {
        // this can happen (although it is *highly* unlikely)
        // if the subscriber immediately unsubscribes and the unsubscription code
        // gets to run to completion before this
        return;
    }

    SubscribeResponse response(rc->sid(), herqqProductTokens(), rc->timeout());
    m_httpHandler.send(mi, response);

    if (!service->m_service->isEvented() || sreq.isRenewal())
    {
        return;
    }

    // by now the UnicastRemoteClient for the subscriber is created if everything
    // went well and we can attempt to send the initial event message

    // The UDA v1.1 does not specify what to do when a subscription is received
    // to a service that is not evented. A "safe" route was taken here and
    // all subscriptions are accepted rather than returning some error. However,
    // in such a case the timeout is adjusted to a day and no events are ever sent.

    QByteArray msgBody;
    getCurrentValues(msgBody, service->m_service);

    if (mi.keepAlive() && mi.socket().state() == QTcpSocket::ConnectedState)
    {
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // !!slight deviation from the UDA v1.1 specification!!
        //
        // the timeout for acknowledging a initial notify request using the
        // same connection is set to 3 seconds, instead of the 30 as specified
        // in the standard. This is for two reasons:
        // 1) there exists UPnP software that do not implement and respect
        // HTTP keep-alive properly.
        // 2) initial notify using HTTP keep-alive is very fast (unless something
        // is seriously wrong) and 3 seconds should be much more than enough.

        // with the above in mind, if the subscriber seems to use HTTP keep-alive,
        // the initial notify is sent using the connection in which the
        // subscription came. However, if that fails, the initial notify is
        // re-send using a new connection.

        mi.setKeepAlive(false);
        mi.setReceiveTimeoutForNoData(3000);

        if (rc->initialNotify(msgBody, &mi))
        {
            return;
        }

        HLOG_WARN(QObject::tr(
            "Initial notify to SID [%1] failed. The device does not seem to " \
            "respect HTTP keep-alive. Re-sending the initial notify using a new connection.").arg(
                sid.toString()));
    }

    // before sending the initial event message (specified in UDA),
    // the UDA mandates that FIN has been sent to the subscriber unless
    // the connection is to be kept alive.
    if (mi.socket().state() == QTcpSocket::ConnectedState)
    {
        mi.socket().disconnectFromHost();
        mi.socket().waitForDisconnected(100);
    }

    rc->initialNotify(msgBody);
}

void DeviceHostHttpServer::incomingUnsubscriptionRequest(
    MessagingInfo& mi, const UnsubscribeRequest& usreq)
{
    HLOG2(H_AT, H_FUN, m_deviceHost->m_loggingIdentifier);

    Permission perm(*m_deviceHost);
    if (!perm.isValid())
    {
        mi.setKeepAlive(false);
        m_httpHandler.responseInternalServerError(mi);
        return;
    }

    HLOG_DBG(QObject::tr("Unsubscription received."));

    bool ok = false;
    emit removeSubscriber_sig(&usreq, &ok);

    mi.setKeepAlive(false);
    if (ok)
    {
        m_httpHandler.responseOk(mi);
    }
    else
    {
        m_httpHandler.responsePreconditionFailed(mi);
    }
}

void DeviceHostHttpServer::incomingControlRequest(
    MessagingInfo& mi, const InvokeActionRequest& invokeActionRequest)
{
    HLOG2(H_AT, H_FUN, m_deviceHost->m_loggingIdentifier);

    HLOG_DBG(QObject::tr("Control message to %1 received.").arg(invokeActionRequest.soapAction()));

    Permission perm(*m_deviceHost);
    if (!perm.isValid())
    {
        mi.setKeepAlive(false);
        m_httpHandler.responseInternalServerError(mi);
        return;
    }

    HServiceController* service =
        m_deviceHost->searchServiceByControlUrl(invokeActionRequest.serviceUrl());

    if (!service)
    {
        HLOG_WARN(QObject::tr("Control URL [%1] is invalid.").arg(
            invokeActionRequest.serviceUrl().path()));

        mi.setKeepAlive(false);
        m_httpHandler.responseBadRequest(mi);
        return;
    }

    QtSoapMessage soapMsg = invokeActionRequest.soapMsg();

    const QtSoapType& method = soapMsg.method();
    if (!method.isValid())
    {
        HLOG_WARN(QObject::tr("Invalid control method."));

        mi.setKeepAlive(false);
        m_httpHandler.responseBadRequest(mi);
        return;
    }

    try
    {
        HAction* action = service->m_service->actionByName(method.name().name());
        if (!action)
        {
            HLOG_WARN(QObject::tr("The service has no action named [%1].").arg(
                method.name().name()));

            mi.setKeepAlive(false);
            m_httpHandler.responseInvalidAction(mi, soapMsg.toXmlString());

            return;
        }

        HActionInputArguments iargs = action->inputArguments();
        QList<QString> names = iargs.names();
        foreach(QString key, names)
        {
            const QtSoapType& arg = method[key];
            if (!arg.isValid())
            {
                mi.setKeepAlive(false);
                m_httpHandler.responseInvalidArgs(mi, soapMsg.toXmlString());

                return;
            }

            HActionInputArgument* iarg = iargs[key];
            if (!iarg->setValue(
                convertToRightVariantType(arg.value().toString(), iarg->dataType())))
            {
                mi.setKeepAlive(false);
                m_httpHandler.responseInvalidArgs(mi, soapMsg.toXmlString());

                return;
            }
        }

        HActionOutputArguments outArgs = action->outputArguments();

        qint32 retVal = action->invoke(iargs, &outArgs);
        if (retVal != HAction::Success())
        {
            mi.setKeepAlive(false);
            m_httpHandler.responseActionFailed(mi, retVal);
            return;
        }

        QtSoapMessage soapResponse;
        soapResponse.setMethod(QtSoapQName(
            QString("%1%2").arg(action->name(), "Response"),
            service->m_service->serviceType().toString()));

        foreach(HActionOutputArgument* oarg, outArgs)
        {
            QtSoapType* soapArg =
                new SoapType(oarg->name(), oarg->dataType(), oarg->value());

            soapResponse.addMethodArgument(soapArg);
        }

        m_httpHandler.responseOk(mi, soapResponse.toXmlString());

        HLOG_DBG(QObject::tr("Control message successfully handled."));
    }
    catch(HException& ex)
    {
        QtSoapMessage soapFaultResponse;
        soapFaultResponse.setFaultCode(QtSoapMessage::Server);
        soapFaultResponse.setFaultString(ex.reason());

        mi.setKeepAlive(false);
        m_httpHandler.responseActionFailed(mi, 501, ex.reason());
    }
    catch(...)
    {
        mi.setKeepAlive(false);
        m_httpHandler.responseActionFailed(mi, 501);
    }
}

void DeviceHostHttpServer::incomingUnknownHeadRequest(
    MessagingInfo& mi, const QHttpRequestHeader&)
{
    HLOG2(H_AT, H_FUN, m_deviceHost->m_loggingIdentifier);
    // TODO
    Permission perm(*m_deviceHost);
    if (!perm.isValid())
    {
        mi.setKeepAlive(false);
        m_httpHandler.responseInternalServerError(mi);
        return;
    }

    mi.setKeepAlive(false);
    m_httpHandler.responseMethodNotAllowed(mi);
}

void DeviceHostHttpServer::incomingUnknownGetRequest(
    MessagingInfo& mi, const QHttpRequestHeader& requestHdr)
{
    HLOG2(H_AT, H_FUN, m_deviceHost->m_loggingIdentifier);

    Permission perm(*m_deviceHost);
    if (!perm.isValid())
    {
        mi.setKeepAlive(false);
        m_httpHandler.responseInternalServerError(mi);
        return;
    }

    HLOG_DBG(QObject::tr("HTTP GET request received from [%1] to [%2].").arg(
        peerAsStr(mi.socket()), requestHdr.path()));

    QString deviceDescriptor =
        m_deviceHost->findDeviceDescriptor(requestHdr.path());

    if (!deviceDescriptor.isEmpty())
    {
        HLOG_DBG(QObject::tr("Sending device description to [%1] as requested.").arg(
            peerAsStr(mi.socket())));

        m_httpHandler.responseOk(mi, deviceDescriptor);
        return;
    }

    QString serviceDescriptor =
        m_deviceHost->findServiceDescriptor(requestHdr.path());

    if (!serviceDescriptor.isEmpty())
    {
        HLOG_DBG(QObject::tr("Sending service description to [%1] as requested.").arg(
            peerAsStr(mi.socket())));

        m_httpHandler.responseOk(mi, serviceDescriptor);
        return;
    }

    QPair<QUrl, QImage> icon = m_deviceHost->searchIcon(requestHdr.path());
    if (!icon.second.isNull())
    {
        QByteArray ba;
        QBuffer buffer(&ba);
        if (!buffer.open(QIODevice::WriteOnly))
        {
            HLOG_WARN(QObject::tr("Failed to serialize the icon."));
            return;
        }

        if (!icon.second.save(&buffer, "png"))
        {
            HLOG_WARN(QObject::tr("Failed to serialize the icon."));
            return;
        }

        HLOG_DBG(QObject::tr("Sending icon to [%1] as requested.").arg(
            peerAsStr(mi.socket())));

        m_httpHandler.responseOk(mi, ba);
        return;
    }

    HLOG_DBG(QObject::tr("Responding NOT_FOUND [%1] to [%2].").arg(
        requestHdr.path(), peerAsStr(mi.socket())));

    m_httpHandler.responseNotFound(mi);
}

void DeviceHostHttpServer::incomingUnknownPostRequest(
    MessagingInfo& mi, const QHttpRequestHeader& /*requestHdr*/, const QByteArray& /*body*/)
{
    HLOG2(H_AT, H_FUN, m_deviceHost->m_loggingIdentifier);

    Permission perm(*m_deviceHost);
    if (!perm.isValid())
    {
        mi.setKeepAlive(false);
        m_httpHandler.responseInternalServerError(mi);
        return;
    }

    mi.setKeepAlive(false);
    m_httpHandler.responseMethodNotAllowed(mi);
}

/*******************************************************************************
 * Permission
 ******************************************************************************/
Permission::Permission(HDeviceHostPrivate& dh) :
    m_dh(dh), m_valid(false)
{
    m_dh.m_activeRequestCount.ref();
    // this ensures that the device host will not go deleting anything just yet.

    if (dh.state() == HAbstractHostPrivate::Initialized)
    {
        m_valid = true;
    }
    else
    {
        m_dh.m_activeRequestCount.deref();
    }
}

Permission::~Permission()
{
    if (m_valid)
    {
        m_dh.m_activeRequestCount.deref();
    }
}

bool Permission::isValid() const
{
    return m_valid;
}

/*******************************************************************************
 * DeviceHostSsdpHandler
 ******************************************************************************/
DeviceHostSsdpHandler::DeviceHostSsdpHandler(
    HDeviceHostPrivate& dh, QObject* parent) :
        HSsdp(parent), m_dh(dh)
{
    Q_ASSERT(parent);
}

DeviceHostSsdpHandler::~DeviceHostSsdpHandler()
{
}

bool DeviceHostSsdpHandler::incomingDiscoveryRequest(
    const HDiscoveryRequest& msg, const HEndpoint& source,
    const HEndpoint& destination)
{
    HLOG(H_AT, H_FUN);

    Permission perm(m_dh);
    if (!perm.isValid())
    {
        return true;
    }

    QList<HDiscoveryResponse> responses;
    switch (msg.searchTarget().type())
    {
        case HResourceIdentifier::AllDevices:
            m_dh.processSearchRequest_AllDevices(msg, source, &responses);
            break;

        case HResourceIdentifier::RootDevice:
            m_dh.processSearchRequest_RootDevice(msg, source, &responses);
            break;

        case HResourceIdentifier::SpecificDevice:
            m_dh.processSearchRequest_specificDevice(msg, source, &responses);
            break;

        case HResourceIdentifier::StandardDeviceType:
        case HResourceIdentifier::VendorSpecifiedDeviceType:
            m_dh.processSearchRequest_deviceType(msg, source, &responses);
            break;

        case HResourceIdentifier::StandardServiceType:
        case HResourceIdentifier::VendorSpecifiedServiceType:
            m_dh.processSearchRequest_serviceType(msg, source, &responses);
            break;

        default:
            return true;
    }

    if (destination.isMulticast())
    {
        HSysUtils::msleep((rand() % msg.mx()) * 1000);
    }

    foreach (HDiscoveryResponse resp, responses)
    {
        sendDiscoveryResponse(source, resp);
    }

    return true;
}

bool DeviceHostSsdpHandler::incomingDiscoveryResponse(
    const Herqq::Upnp::HDiscoveryResponse& /*msg*/,
    const HEndpoint& /*source*/)
{
    return true;
}

bool DeviceHostSsdpHandler::incomingDeviceAvailableAnnouncement(
    const Herqq::Upnp::HResourceAvailable& /*msg*/)
{
    return true;
}

bool DeviceHostSsdpHandler::incomingDeviceUnavailableAnnouncement(
    const Herqq::Upnp::HResourceUnavailable& /*msg*/)
{
    return true;
}

/*******************************************************************************
 * HDeviceHostPrivate
 ******************************************************************************/
HDeviceHostPrivate::HDeviceHostPrivate() :
    HAbstractHostPrivate(
        QString("__DEVICE HOST %1__: ").arg(QUuid::createUuid().toString())),
            m_initParams                  (),
            m_ssdp                        (0),
            m_individualAdvertisementCount(2),
            m_httpServer                  (0),
            m_activeRequestCount          (0),
            m_remoteClientNotifier        (0)
{
    HLOG(H_AT, H_FUN);

    m_remoteClientNotifier.reset(new RemoteClientNotifier(this));

    srand(time(0));
}

HDeviceHostPrivate::~HDeviceHostPrivate()
{
    HLOG(H_AT, H_FUN);
}

QString HDeviceHostPrivate::findDeviceDescriptor(const QString& path)
{
    HLOG(H_AT, H_FUN);

    if (!path.endsWith(HDevicePrivate::deviceDescriptionPostFix()))
    {
        return "";
    }

    QUuid searchedUdn(path.section('/', 1, 1));
    if (searchedUdn.isNull())
    {
        return "";
    }

    HDeviceController* device = searchDeviceByUdn(searchedUdn);
    return device ? device->m_device->deviceDescription() : "";
}

QString HDeviceHostPrivate::findServiceDescriptor(const QString& path)
{
    HLOG(H_AT, H_FUN);

    HServiceController* service = searchServiceByScpdUrl(path);
    return service ? service->m_service->serviceDescription() : "";
}

void HDeviceHostPrivate::processSearchRequest_specificDevice(
    const HDiscoveryRequest& req, const HEndpoint& source,
    QList<HDiscoveryResponse>* responses)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QUuid uuid = req.searchTarget().deviceUuid();
    if (uuid.isNull())
    {
        HLOG_DBG(QObject::tr("Invalid device-UUID"));
        return;
    }

    HDeviceController* device = searchDeviceByUdn(uuid);
    if (!device)
    {
        HLOG_DBG(QObject::tr("No device with the specified UUID"));
        return;
    }

    QUrl location;
    if (!searchValidLocation(device->m_device.data(), source, &location))
    {
        HLOG_DBG(QObject::tr(
            "Found a device with uuid: %1, but it is not "
            "available on the interface that has address: ").arg(
                uuid.toString(), source.toString()));

        return;
    }

    HUsn usn(device->m_device->deviceInfo().udn(), req.searchTarget());

    responses->push_back(
        HDiscoveryResponse(
            device->deviceTimeoutInSecs(),
            QDateTime::currentDateTime(), location, herqqProductTokens(), usn,
            device->deviceStatus()->bootId(), device->deviceStatus()->configId()));
}

void HDeviceHostPrivate::processSearchRequest_deviceType(
    const HDiscoveryRequest& req, const HEndpoint& source,
    QList<HDiscoveryResponse>* responses)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QList<HDeviceController*> foundDevices =
        searchDevicesByDeviceType(req.searchTarget().resourceType(), false);

    if (!foundDevices.size())
    {
        HLOG_DBG(QObject::tr("No devices match the specified type: [%1]").arg(
            req.searchTarget().resourceType().toString()));

        return;
    }

    foreach(HDeviceController* device, foundDevices)
    {
        QUrl location;
        if (!searchValidLocation(device->m_device.data(), source, &location))
        {
            HLOG_DBG(QObject::tr(
                "Found a matching device, but it is not "
                "available on the interface that has address: %1").arg(source.toString()));

            continue;
        }

        HUsn usn(device->m_device->deviceInfo().udn(), req.searchTarget());

        responses->push_back(
            HDiscoveryResponse(
                device->deviceTimeoutInSecs(),
                QDateTime::currentDateTime(), location, herqqProductTokens(), usn,
                device->deviceStatus()->bootId(), device->deviceStatus()->configId()));
    }
}

void HDeviceHostPrivate::processSearchRequest_serviceType(
    const HDiscoveryRequest& req, const HEndpoint& source,
    QList<HDiscoveryResponse>* responses)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QList<HServiceController*> foundServices =
        searchServicesByServiceType(req.searchTarget().resourceType(), false);

    if (!foundServices.size())
    {
       HLOG_DBG(QObject::tr("No services match the specified type"));
       return;
    }

    foreach(HServiceController* service, foundServices)
    {
        const HDevice* device = service->m_service->parentDevice();
        Q_ASSERT(device);

        QUrl location;
        if (!searchValidLocation(device, source, &location))
        {
            HLOG_DBG(QObject::tr(
                "Found a matching device, but it is not "
                "available on the interface that has address: %1").arg(source.toString()));

            continue;
        }

        HUsn usn(device->deviceInfo().udn(), req.searchTarget());
        HDeviceController* dc = searchDeviceByUdn(device->deviceInfo().udn());
        Q_ASSERT(dc);

        responses->push_back(
            HDiscoveryResponse(
                dc->deviceTimeoutInSecs(),
                QDateTime::currentDateTime(), location, herqqProductTokens(), usn,
                dc->deviceStatus()->bootId(), dc->deviceStatus()->configId()));
    }
}

void HDeviceHostPrivate::processSearchRequest(
    HDeviceController* device, const QUrl& location,
    QList<HDiscoveryResponse>* responses)
{
    HLOG(H_AT, H_FUN);

    Q_ASSERT(device);

    HDeviceInfo deviceInfo = device->m_device->deviceInfo();

    HProductTokens pt = herqqProductTokens();

    HUsn usn(deviceInfo.udn());

    // device UDN
    responses->push_back(
        HDiscoveryResponse(
            device->deviceTimeoutInSecs(),
            QDateTime::currentDateTime(), location, pt, usn,
            device->deviceStatus()->bootId(), device->deviceStatus()->configId()));

    usn.setResource(HResourceIdentifier(deviceInfo.deviceType()));

    // device type
    responses->push_back(
        HDiscoveryResponse(
            device->deviceTimeoutInSecs(),
            QDateTime::currentDateTime(), location, pt, usn,
            device->deviceStatus()->bootId(), device->deviceStatus()->configId()));

    QList<HServiceController*> services = device->services();
    foreach(HServiceController* service, services)
    {
        usn.setResource(HResourceIdentifier(service->m_service->serviceType().toString()));

        responses->push_back(
            HDiscoveryResponse(
                device->deviceTimeoutInSecs(),
                QDateTime::currentDateTime(), location, pt, usn,
                device->deviceStatus()->bootId(), device->deviceStatus()->configId()));
    }

    QList<HDeviceController*> devices = device->embeddedDevices();
    foreach(HDeviceController* embeddedDevice, devices)
    {
        processSearchRequest(embeddedDevice, location, responses);
    }
}

void HDeviceHostPrivate::processSearchRequest_AllDevices(
    const HDiscoveryRequest& /*req*/, const HEndpoint& source,
    QList<HDiscoveryResponse>* responses)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(responses);

    HProductTokens pt = herqqProductTokens();

    QMutexLocker lock(&m_rootDevicesMutex);

    HLOG_DBG(QObject::tr("Received search request for all devices."));

    foreach(HDeviceController* rootDevice, m_rootDevices)
    {
        QUrl location;
        if (!searchValidLocation(rootDevice->m_device.data(), source, &location))
        {
            HLOG_DBG(QObject::tr(
                "Found a device, but it is not "
                "available on the interface that has address: ").arg(source.toString()));

            continue;
        }

        HUsn usn(rootDevice->m_device->deviceInfo().udn(),
                 HResourceIdentifier::getRootDeviceIdentifier());

        responses->push_back(
            HDiscoveryResponse(
                rootDevice->deviceTimeoutInSecs(),
                QDateTime::currentDateTime(), location, pt, usn,
                rootDevice->deviceStatus()->bootId(), rootDevice->deviceStatus()->configId()));

        processSearchRequest(rootDevice, location, responses);

        QList<HDeviceController*> devices = rootDevice->embeddedDevices();
        foreach(HDeviceController* embeddedDevice, devices)
        {
            if (!searchValidLocation(embeddedDevice->m_device.data(), source, &location))
            {
                // highly uncommon, but possible; the root device is "active" on the network interface
                // to which the request came, but at least one of its embedded
                // devices is not.

                HLOG_DBG(QObject::tr(
                    "Skipping an embedded device that is not "
                    "available on the interface that has address: ").arg(source.toString()));

                continue;
            }

            processSearchRequest(embeddedDevice, location, responses);
        }
    }
}

void HDeviceHostPrivate::processSearchRequest_RootDevice(
    const HDiscoveryRequest& /*req*/, const HEndpoint& source,
    QList<HDiscoveryResponse>* responses)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(responses);

    QMutexLocker lock(&m_rootDevicesMutex);

    HLOG_DBG(QObject::tr("Received search request for root devices."));

    foreach(HDeviceController* rootDevice, m_rootDevices)
    {
        QUrl location;
        if (!searchValidLocation(rootDevice->m_device.data(), source, &location))
        {
            HLOG_DBG(QObject::tr(
                "Found a root device, but it is not "
                "available on the interface that has address: ").arg(source.toString()));

            continue;
        }

        HUsn usn(rootDevice->m_device->deviceInfo().udn(),
                 HResourceIdentifier::getRootDeviceIdentifier());

        responses->push_back(
            HDiscoveryResponse(
                rootDevice->deviceTimeoutInSecs(), QDateTime::currentDateTime(),
                location, herqqProductTokens(), usn,
                rootDevice->deviceStatus()->bootId(), rootDevice->deviceStatus()->configId()));
    }
}

namespace
{

class Announcement
{

protected:

    HDeviceController* m_device;
    HUsn m_usn;
    QUrl m_location;

public:

    Announcement(){}

    Announcement(
        HDeviceController* device, const HUsn& usn, const QUrl& location) :
            m_device(device), m_usn(usn), m_location(location)
    {
        Q_ASSERT(m_device);
        Q_ASSERT(m_usn.isValid());
        Q_ASSERT(m_location.isValid() && !m_location.isEmpty());
    }

    virtual ~Announcement() {}
};

class ResourceAvailableAnnouncement :
    private Announcement
{
public:

    ResourceAvailableAnnouncement(){}

    ResourceAvailableAnnouncement(
        HDeviceController* device, const HUsn& usn, const QUrl& location) :
            Announcement(device, usn, location)
    {
    }

    HResourceAvailable operator()()
    {
        HProductTokens pt = herqqProductTokens();

        return HResourceAvailable(
            m_device->deviceTimeoutInSecs(), m_location, pt, m_usn,
            m_device->deviceStatus()->bootId(),
            m_device->deviceStatus()->configId());
    }
};

class ResourceUnavailableAnnouncement :
    private Announcement
{
public:

    ResourceUnavailableAnnouncement(){}

    ResourceUnavailableAnnouncement(
        HDeviceController* device, const HUsn& usn, const QUrl& location) :
            Announcement(device, usn, location)
    {
    }

    HResourceUnavailable operator()()
    {
        HProductTokens pt = herqqProductTokens();

        return HResourceUnavailable(
            m_usn, m_location, m_device->deviceStatus()->bootId(),
            m_device->deviceStatus()->configId());
    }
};
}

template<typename AnnouncementType>
void HDeviceHostPrivate::sendAnnouncements(
    const QList<AnnouncementType>& announcements)
{
    HLOG(H_AT, H_FUN);

    for (qint32 i = 0; i < m_individualAdvertisementCount; ++i)
    {
        foreach(AnnouncementType at, announcements)
        {
            m_ssdp->announcePresence(at());
        }
    }
}

template<typename AnnouncementType>
void HDeviceHostPrivate::createAnnouncementMessagesForEmbeddedDevice(
    HDeviceController* device, QList<AnnouncementType>& announcements)
{
    HLOG(H_AT, H_FUN);

    QList<QUrl> locations = device->m_device->locations(true);
    foreach(QUrl location, locations)
    {
        HDeviceInfo deviceInfo = device->m_device->deviceInfo();

        HUdn udn = deviceInfo.udn();
        HUsn usn = udn;

        // device UDN advertisement
        announcements.push_back(AnnouncementType(device, usn, location));

        // device type advertisement
        usn.setResource(deviceInfo.deviceType());
        announcements.push_back(AnnouncementType(device, usn, location));

        // service advertisements
        QList<HServiceController*> services = device->services();
        foreach(HServiceController* service, services)
        {
            usn.setResource(service->m_service->serviceType().toString());
            announcements.push_back(AnnouncementType(device, usn, location));
        }
    }

    QList<HDeviceController*> devices = device->embeddedDevices();
    foreach(HDeviceController* embeddedDevice, devices)
    {
        createAnnouncementMessagesForEmbeddedDevice(embeddedDevice, announcements);
    }
}

template<typename AnnouncementType>
void HDeviceHostPrivate::createAnnouncementMessagesForRootDevice(
    HDeviceController* rootDevice, QList<AnnouncementType>& announcements)
{
    HLOG(H_AT, H_FUN);

    QList<QUrl> locations = rootDevice->m_device->locations(true);
    foreach(QUrl location, locations)
    {
        HUdn udn(rootDevice->m_device->deviceInfo().udn());
        HUsn usn(udn, HResourceIdentifier::getRootDeviceIdentifier());

        announcements.push_back(AnnouncementType(rootDevice, usn, location));
    }

    // generic device advertisement (same for both root and embedded devices)
    createAnnouncementMessagesForEmbeddedDevice(rootDevice, announcements);
}

template<typename AnnouncementType>
void HDeviceHostPrivate::announce()
{
    HLOG(H_AT, H_FUN);

    QMutexLocker lock(&m_rootDevicesMutex);

    QList<AnnouncementType> announcements;

    foreach(HDeviceController* rootDevice, m_rootDevices)
    {
        createAnnouncementMessagesForRootDevice(rootDevice, announcements);
    }

    sendAnnouncements(announcements);
}

void HDeviceHostPrivate::announcementTimedout(HDeviceController* rootDevice)
{
    HLOG(H_AT, H_FUN);

    QMutexLocker lock(&m_rootDevicesMutex);

    QList<ResourceAvailableAnnouncement> announcements;
    createAnnouncementMessagesForRootDevice(rootDevice, announcements),

    sendAnnouncements(announcements);

    rootDevice->startStatusNotifier(HDeviceController::ThisOnly);
}

namespace
{
class LocalServiceDescriptionFetcher
{
private:

    QString m_rootDir;

public:

    LocalServiceDescriptionFetcher() {}
    LocalServiceDescriptionFetcher(const QString& rootDir) : m_rootDir(rootDir) {}

    QDomDocument operator()(
        const QUrl& /*deviceLocation*/, const QUrl& scpdUrl)
    {
        HLOG2(H_AT, H_FUN, "__DEVICE HOST__: ");

        QString localScpdPath = scpdUrl.toLocalFile();
        if (localScpdPath.startsWith('/'))
        {
            localScpdPath = localScpdPath.mid(1);
        }

        QString fullScpdPath = m_rootDir;
        if (!fullScpdPath.endsWith('/'))
        {
            fullScpdPath.append('/');
        }
        fullScpdPath.append(localScpdPath);
        // UDA mandates that the paths inside a device description are treated relative
        // to the device description location.

        QFile file(fullScpdPath);

        HLOG_DBG(QObject::tr(
            "Attempting to open service description from [%1]").arg(fullScpdPath));

        if (!file.open(QIODevice::ReadOnly))
        {
            throw HOperationFailedException(
                QObject::tr("Could not open the service description file [%1].").arg(
                    fullScpdPath));
        }

        QDomDocument dd;
        QString errMsg; qint32 errLine = 0;
        if (!dd.setContent(&file, false, &errMsg, &errLine))
        {
            throw HParseException(
                QObject::tr("Could not parse the service description file [%1]: %2 @ line %3").
                arg(fullScpdPath, errMsg, QString::number(errLine)));
        }

        return dd;
    }
};

class LocalIconFetcher
{
private:

    QString m_rootDir;

public:

    LocalIconFetcher() {}
    LocalIconFetcher(const QString& rootDir) : m_rootDir(rootDir) {}

    QImage operator()(const QUrl& /*devLoc*/, const QUrl& iconUrl)
    {
        HLOG2(H_AT, H_FUN, "__DEVICE HOST__: ");

        QString localIconPath = iconUrl.toLocalFile();
        if (localIconPath.startsWith('/'))
        {
            localIconPath = localIconPath.mid(1);
        }

        QString fullIconPath = m_rootDir;
        if (!fullIconPath.endsWith('/'))
        {
            fullIconPath.append('/');
        }
        fullIconPath.append(localIconPath);
        // UDA mandates that the paths inside a device description are treated relative
        // to the device description location.

        HLOG_DBG(QObject::tr(
            "Attempting to open a file [%1] that should contain an icon").arg(fullIconPath));

        QImage icon(fullIconPath);

        if (icon.isNull())
        {
            throw HParseException(
                QObject::tr("Could not open the icon file [%1]").arg(fullIconPath));
        }

        return icon;
    }
};
}

void HDeviceHostPrivate::createRootDevices()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QList<HDeviceConfiguration*> diParams =
        m_initParams.deviceConfigurations();

    foreach(HDeviceConfiguration* deviceInitParams, diParams)
    {
        QFile file(deviceInitParams->pathToDeviceDescription());
        if (!file.open(QIODevice::ReadOnly))
        {
            throw HOperationFailedException(
                QObject::tr("Could not open the device description file: [%1].").arg(
                    deviceInitParams->pathToDeviceDescription()));
        }

        QDomDocument dd;
        QString errMsg; qint32 errLine = 0;
        if (!dd.setContent(&file, false, &errMsg, &errLine))
        {
            throw InvalidDeviceDescription(
                QObject::tr("Could not parse the device description file: [%1] @ line %2").
                arg(errMsg, QString::number(errLine)));
        }

        QList<QUrl> locations;
        locations.push_back(m_httpServer->rootUrl());
        // TODO, modify ^^^ when the server component supports multi-homed devices.

        HObjectCreationParameters creatorParams;
        creatorParams.m_createDefaultObjects = false;
        creatorParams.m_deviceDescription    = dd;
        creatorParams.m_deviceCreator        = deviceInitParams->deviceCreator();
        creatorParams.m_deviceLocations      = locations;

        QString baseDir =
            extractBaseUrl(deviceInitParams->pathToDeviceDescription());

        LocalServiceDescriptionFetcher scpdFetcher(baseDir);
        creatorParams.m_serviceDescriptionFetcher = scpdFetcher;
        creatorParams.m_deviceTimeoutInSecs  = deviceInitParams->cacheControlMaxAge() / 2;
        creatorParams.m_appendUdnToDeviceLocation = true;
        creatorParams.m_sharedActionInvokers = &m_sharedActionInvokers;

        LocalIconFetcher iconFetcher(baseDir);
        creatorParams.m_iconFetcher = iconFetcher;
        creatorParams.m_strictParsing = true;
        creatorParams.m_stateVariablesAreImmutable = false;

        HObjectCreator creator(creatorParams);
        HDeviceController* rootDevice = creator.createRootDevice();

        Q_ASSERT(rootDevice);
        addRootDevice(rootDevice);

        rootDevice->setParent(this);
        rootDevice->m_device->setParent(this);
        connectSelfToServiceSignals(rootDevice->m_device.data());
    }
}

void HDeviceHostPrivate::connectSelfToServiceSignals(HDevice* device)
{
    HLOG(H_AT, H_FUN);

    QList<HService*> services = device->services();
    foreach(HService* service, services)
    {
        bool ok = QObject::connect(
            service,
            SIGNAL(stateChanged(const Herqq::Upnp::HService*)),
            m_remoteClientNotifier.data(),
            SLOT(stateChanged(const Herqq::Upnp::HService*)));

        Q_ASSERT(ok); Q_UNUSED(ok)
    }

    QList<HDevice*> devices = device->embeddedDevices();
    foreach(HDevice* embeddedDevice, devices)
    {
        connectSelfToServiceSignals(embeddedDevice);
    }
}

void HDeviceHostPrivate::startNotifiers()
{
    HLOG(H_AT, H_FUN);

    foreach(HDeviceController* rootDevice, m_rootDevices)
    {
        bool ok = connect(
                rootDevice, SIGNAL(statusTimeout(HDeviceController*)),
                this, SLOT(announcementTimedout(HDeviceController*)));

        Q_ASSERT(ok); Q_UNUSED(ok)

        rootDevice->startStatusNotifier(HDeviceController::ThisOnly);
    }
}

void HDeviceHostPrivate::stopNotifiers()
{
    HLOG(H_AT, H_FUN);

    foreach(HDeviceController* rootDevice, m_rootDevices)
    {
        rootDevice->stopStatusNotifier(HDeviceController::ThisOnly);
    }
}

void HDeviceHostPrivate::doClear()
{
    HLOG(H_AT, H_FUN);

    Q_ASSERT(thread() == QThread::currentThread());

    // called by the abstract host just before it starts to delete the device
    // tree.

    Q_ASSERT(state() == Exiting);
    // this path should be traversed only when the device host has initiated
    // shut down.

    m_http.shutdown(false);

    delete m_httpServer; m_httpServer = 0;

    m_remoteClientNotifier.reset(0);

    m_threadPool->waitForDone();

    delete m_ssdp; m_ssdp = 0;

    m_initParams = HDeviceHostConfiguration();
    m_activeRequestCount = 0;

    setState(Uninitialized);
}

/*******************************************************************************
 * HDeviceHost
 *******************************************************************************/
HDeviceHost::HDeviceHost(QObject* parent) :
    HAbstractHost(*new HDeviceHostPrivate(), parent)
{
    HLOG(H_AT, H_FUN);
}

HDeviceHost::~HDeviceHost()
{
    HLOG(H_AT, H_FUN);
    quit();
}

HDeviceHost::ReturnCode HDeviceHost::init(
    const HDeviceHostConfiguration& initParams, QString* errorString)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    H_D(HDeviceHost);

    Q_ASSERT_X(
        thread() == QThread::currentThread(), H_AT,
        "The device host has to be initialized in the thread in which it is currently located.");

    if (h->state() == HAbstractHostPrivate::Initialized)
    {
        return AlreadyInitialized;
    }

    Q_ASSERT(h->state() == HAbstractHostPrivate::Uninitialized);

    if (initParams.isEmpty())
    {
		if (errorString)
		{
			*errorString = QObject::tr("No UPnP device configuration provided.");
		}

        return InvalidConfiguration;
    }

    QString error;
    HDeviceHost::ReturnCode rc = Success;
    try
    {
        h->setState(HAbstractHostPrivate::Initializing);

        HLOG_INFO(QObject::tr("DeviceHost Initializing."));

        h->m_initParams = initParams;

        h->m_httpServer = new DeviceHostHttpServer(h, this);
        if (!h->m_httpServer->listen())
        {
            QString err = QObject::tr("Could not start the HTTP server.");

            if (errorString)
            {
                *errorString = err;
            }

            HLOG_WARN(QObject::tr("DeviceHost initialization failed: [%1]").arg(err));
            rc = UndefinedFailure;
        }
        else
        {
            h->createRootDevices();

            h->m_ssdp = new DeviceHostSsdpHandler(*h, this);
            h->announce<ResourceAvailableAnnouncement>();

            h->startNotifiers();

            h->setState(HAbstractHostPrivate::Initialized);
        }
    }
    catch(Herqq::Upnp::InvalidDeviceDescription& ex)
    {
        error = ex.reason();
        rc = InvalidDeviceDescription;
    }
    catch(Herqq::Upnp::InvalidServiceDescription& ex)
    {
        error = ex.reason();
        rc = InvalidServiceDescription;
    }
    catch(HException& ex)
    {
        error = ex.reason();
        rc = UndefinedFailure;
    }

    if (rc != Success)
    {
        HLOG_WARN(QObject::tr("DeviceHost initialization failed: [%1]").arg(error));

        h->setState(HAbstractHostPrivate::Exiting);
        h->clear();

        if (errorString)
        {
            *errorString = error;
        }

        return rc;
    }

    HLOG_INFO(QObject::tr("DeviceHost initialized."));

    return rc;
}

HDeviceHost::ReturnCode HDeviceHost::quit(QString* errorString)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    H_D(HDeviceHost);

    Q_ASSERT_X(
        thread() == QThread::currentThread(), H_AT,
        "The device host has to be shutdown in the thread in which it is currently located.");

    if (h->state() == HAbstractHostPrivate::Uninitialized)
    {
        return Success;
    }

    Q_ASSERT(h->state() == HAbstractHostPrivate::Initialized);

    HLOG_INFO(QObject::tr("DeviceHost shutting down."));

    h->setState(HAbstractHostPrivate::Exiting);
    while(h->m_activeRequestCount)
    {
        // as long as there are requests being processed, we cannot go
        // deleting objects that may be needed by the request processing. ==>
        // wait for the requests to complete

        QAbstractEventDispatcher::instance()->processEvents(
            QEventLoop::ExcludeUserInputEvents);

        HSysUtils::msleep(1);
    }

    ReturnCode retVal = Success;
    try
    {
        h->stopNotifiers();
        h->announce<ResourceUnavailableAnnouncement>();
    }
    catch (HException& ex)
    {
        HLOG_WARN(ex.reason());
        retVal = UndefinedFailure;

        if (errorString)
        {
            *errorString = ex.reason();
        }
    }

    h->clear();

    HLOG_INFO(QObject::tr("DeviceHost shut down."));

    return retVal;
}

/*******************************************************************************
 * HDeviceConfigurationPrivate
 ******************************************************************************/
HDeviceConfigurationPrivate::HDeviceConfigurationPrivate() :
    m_pathToDeviceDescriptor(), m_cacheControlMaxAgeInSecs(1800),
    m_deviceCreator()
{
}

HDeviceConfigurationPrivate::~HDeviceConfigurationPrivate()
{
}

/*******************************************************************************
 * HDeviceConfiguration
 ******************************************************************************/
HDeviceConfiguration::HDeviceConfiguration() :
    h_ptr(new HDeviceConfigurationPrivate())
{
}

HDeviceConfiguration::HDeviceConfiguration(
    HDeviceConfigurationPrivate& dd) :
        h_ptr(&dd)
{
}

HDeviceConfiguration::~HDeviceConfiguration()
{
    HLOG(H_AT, H_FUN);

    delete h_ptr;
}

HDeviceConfiguration* HDeviceConfiguration::clone() const
{
    HLOG(H_AT, H_FUN);

    HDeviceConfiguration* clone =
        new HDeviceConfiguration(
            *new HDeviceConfigurationPrivate(*h_ptr));

    return clone;
}

QString HDeviceConfiguration::pathToDeviceDescription() const
{
    return h_ptr->m_pathToDeviceDescriptor;
}

bool HDeviceConfiguration::setPathToDeviceDescription(
    const QString& pathToDeviceDescriptor)
{
    HLOG(H_AT, H_FUN);

    if (!QFile::exists(pathToDeviceDescriptor))
    {
        return false;
    }

    h_ptr->m_pathToDeviceDescriptor = pathToDeviceDescriptor;
    return true;
}

void HDeviceConfiguration::setCacheControlMaxAge(qint32 maxAgeInSecs)
{
    HLOG(H_AT, H_FUN);

    if (maxAgeInSecs < -1)
    {
        maxAgeInSecs = -1;
    }
    else if (maxAgeInSecs > 60*60*24)
    {
        HLOG_WARN(QObject::tr(
            "The specified max age [%1] is too large. Defaulting to a day.").
                arg(QString::number(maxAgeInSecs)));

        maxAgeInSecs = 60*60*24; // a day
    }

    h_ptr->m_cacheControlMaxAgeInSecs = maxAgeInSecs;
}

qint32 HDeviceConfiguration::cacheControlMaxAge() const
{
    return h_ptr->m_cacheControlMaxAgeInSecs;
}

HDeviceCreator HDeviceConfiguration::deviceCreator() const
{
    return h_ptr->m_deviceCreator;
}

void HDeviceConfiguration::setDeviceCreator(
    HDeviceCreator deviceCreator)
{
    h_ptr->m_deviceCreator = deviceCreator;
}

bool HDeviceConfiguration::isValid() const
{
    return !h_ptr->m_pathToDeviceDescriptor.isEmpty() && h_ptr->m_deviceCreator;
}

/*******************************************************************************
 * HDeviceHostConfigurationPrivate
 ******************************************************************************/
HDeviceHostConfigurationPrivate::HDeviceHostConfigurationPrivate() :
    m_collection()
{
}

/*******************************************************************************
 * HDeviceHostConfiguration
 ******************************************************************************/
HDeviceHostConfiguration::HDeviceHostConfiguration() :
    h_ptr(new HDeviceHostConfigurationPrivate())
{
    HLOG(H_AT, H_FUN);
}

HDeviceHostConfiguration::HDeviceHostConfiguration(
    const HDeviceConfiguration& arg) :
        h_ptr(new HDeviceHostConfigurationPrivate())
{
    HLOG(H_AT, H_FUN);
    add(arg);
}

HDeviceHostConfiguration::HDeviceHostConfiguration(
    const HDeviceHostConfiguration& other) :
        h_ptr(new HDeviceHostConfigurationPrivate())
{
    HLOG(H_AT, H_FUN);
    foreach(HDeviceConfiguration* arg, other.h_ptr->m_collection)
    {
        add(*arg);
    }
}

HDeviceHostConfiguration& HDeviceHostConfiguration::operator=(
    const HDeviceHostConfiguration& other)
{
    HLOG(H_AT, H_FUN);
    qDeleteAll(h_ptr->m_collection);
    h_ptr->m_collection.clear();

    foreach(HDeviceConfiguration* arg, other.h_ptr->m_collection)
    {
        add(*arg);
    }

    return *this;
}

HDeviceHostConfiguration::~HDeviceHostConfiguration()
{
    HLOG(H_AT, H_FUN);

    qDeleteAll(h_ptr->m_collection);
    delete h_ptr;
}

bool HDeviceHostConfiguration::add(const HDeviceConfiguration& arg)
{
    HLOG(H_AT, H_FUN);

    if (arg.isValid())
    {
        h_ptr->m_collection.push_back(arg.clone());
        return true;
    }

    return false;
}

QList<HDeviceConfiguration*> HDeviceHostConfiguration::deviceConfigurations() const
{
    return h_ptr->m_collection;
}

bool HDeviceHostConfiguration::isEmpty() const
{
    return h_ptr->m_collection.isEmpty();
}

}
}
