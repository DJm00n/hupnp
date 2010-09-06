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

#include "hevent_notifier_p.h"
#include "hevent_subscriber_p.h"
#include "hdevicehost_configuration.h"

#include "../messages/hevent_messages_p.h"

#include "../../devicemodel/hdevice.h"
#include "../../devicemodel/hservice.h"
#include "../../devicemodel/hstatevariable.h"

#include "../../dataelements/hudn.h"
#include "../../dataelements/hdeviceinfo.h"
#include "../../dataelements/hserviceinfo.h"
#include "../../dataelements/hstatevariableinfo.h"

#include "../../http/hhttp_messaginginfo_p.h"

#include "../../../utils/hlogger_p.h"

#include <QtXml/QDomDocument>
#include <QtNetwork/QTcpSocket>

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

        const HStateVariableInfo& info = stateVar->info();
        if (info.eventingType() == HStateVariableInfo::NoEvents)
        {
            continue;
        }

        QDomElement propertyElem =
            dd.createElementNS("urn:schemas-upnp-org:event-1-0", "e:property");

        QDomElement variableElem = dd.createElement(info.name());
        variableElem.appendChild(dd.createTextNode(stateVar->value().toString()));

        propertyElem.appendChild(variableElem);
        propertySetElem.appendChild(propertyElem);
    }

    msgBody = dd.toByteArray();
}
}

/*******************************************************************************
 * EventNotifier
 ******************************************************************************/
EventNotifier::EventNotifier(
    const QByteArray& loggingIdentifier,
    HHttpHandler& http,
    HDeviceHostConfiguration& configuration,
    QObject* parent) :
        QObject(parent),
            m_loggingIdentifier(loggingIdentifier),
            m_httpHandler(http),
            m_remoteClients(),
            m_remoteClientsMutex(QMutex::Recursive),
            m_shutdown(false),
            m_configuration(configuration)
{
}

EventNotifier::~EventNotifier()
{
    HLOG(H_AT, H_FUN);
    shutdown();
}

HTimeout EventNotifier::getSubscriptionTimeout(const SubscribeRequest& sreq)
{
    const static qint32 max = 60*60*24;

    qint32 configuredTimeout = m_configuration.subscriptionExpirationTimeout();
    if (configuredTimeout > 0)
    {
        return HTimeout(configuredTimeout);
    }
    else if (configuredTimeout == 0)
    {
        HTimeout requested = sreq.timeout();
        if (requested.isInfinite() || requested.value() > max)
        {
            return HTimeout(max);
        }

        return requested;
    }

    return HTimeout(max);
}

void EventNotifier::shutdown()
{
    m_shutdown = true;
    QMutexLocker lock(&m_remoteClientsMutex);

    QList<ServiceEventSubscriberPtrT>::iterator it =
        m_remoteClients.begin();

    m_remoteClients.clear();
}

namespace
{
bool isSameService(HService* srv1, HService* srv2)
{
    return srv1->parentDevice()->info().udn() ==
           srv2->parentDevice()->info().udn() &&
           srv1->info().scpdUrl() == srv2->info().scpdUrl();
}
}

EventNotifier::ServiceEventSubscriberPtrT EventNotifier::remoteClient(
    const HSid& sid) const
{
    HLOG(H_AT, H_FUN);
    QMutexLocker lock(&m_remoteClientsMutex);

    QList<ServiceEventSubscriberPtrT>::const_iterator it =
        m_remoteClients.constBegin();

    for(; it != m_remoteClients.end(); ++it)
    {
        if ((*it)->sid() == sid)
        {
            return *it;
        }
    }

    return ServiceEventSubscriberPtrT(0);
}

StatusCode EventNotifier::addSubscriber(
    HService* service, const SubscribeRequest& sreq, HSid* sid)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(sid);
    Q_ASSERT(service->isEvented());
    // The UDA v1.1 does not specify what to do when a subscription is received
    // to a service that is not evented. A "safe" route was taken here and
    // all subscriptions are accepted rather than returning some error. However,
    // in such a case the timeout is adjusted to a day and no events are ever sent.
    // This is enforced at the HService class, which should not send any
    // events unless one or more of its state variables are evented.

    QMutexLocker lock(&m_remoteClientsMutex);

    if (m_shutdown)
    {
        HLOG_DBG("Shutting down, rejecting subscription");
        return InternalServerError;
    }

    for(qint32 i = 0; i < m_remoteClients.size(); ++i)
    {
        ServiceEventSubscriberPtrT rc = m_remoteClients[i];

        if (isSameService(rc->service(), service) &&
            sreq.callbacks().contains(rc->location()))
        {
            HLOG_WARN(QString(
                "subscriber [%1] to the specified service URL [%2] already "
                "exists").arg(
                    rc->location().toString(),
                    service->info().scpdUrl().toString()));

            return PreconditionFailed;
        }
    }

    HLOG_INFO(QString("adding subscriber from [%1]").arg(
        sreq.callbacks().at(0).toString()));

    HTimeout timeout = service->isEvented() ?
        getSubscriptionTimeout(sreq) : HTimeout(60*60*24);

    ServiceEventSubscriberPtrT rc(
        new ServiceEventSubscriber(
            m_httpHandler,
            m_loggingIdentifier,
            service,
            sreq.callbacks().at(0),
            timeout,
            this),
        &QObject::deleteLater);

    m_remoteClients.push_back(rc);

    *sid = rc->sid();

    return Ok;
}

bool EventNotifier::removeSubscriber(const UnsubscribeRequest& req)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QMutexLocker lock(&m_remoteClientsMutex);

    if (m_shutdown)
    {
        return false;
    }

    bool found = false;

    QList<ServiceEventSubscriberPtrT>::iterator it = m_remoteClients.begin();
    for(; it != m_remoteClients.end(); )
    {
        if ((*it)->sid() == req.sid())
        {
            HLOG_INFO(QString("removing subscriber [SID [%1]] from [%2]").arg(
                req.sid().toString(), (*it)->location().toString()));

            it = m_remoteClients.erase(it);

            found = true;
        }
        else if ((*it)->expired())
        {
            HLOG_INFO(QString(
                "removing an expired subscription [SID [%1]] from [%2]").arg(
                    (*it)->sid().toString(), (*it)->location().toString()));

            it = m_remoteClients.erase(it);
        }
        else
        {
            ++it;
        }
    }

    if (!found)
    {
        HLOG_WARN(QString("Could not cancel subscription. Invalid SID [%1]").arg(
            req.sid().toString()));
        return false;
    }

    return true;
}

StatusCode EventNotifier::renewSubscription(
    const SubscribeRequest& req, HSid* sid)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(sid);

    QMutexLocker lock(&m_remoteClientsMutex);

    if (m_shutdown)
    {
        return InternalServerError;
    }

    QList<ServiceEventSubscriberPtrT>::iterator it = m_remoteClients.begin();
    for(; it != m_remoteClients.end();)
    {
        ServiceEventSubscriberPtrT sub = (*it);
        if ((*it)->sid() == req.sid())
        {
            HLOG_INFO(QString("renewing subscription from [%1]").arg(
                (*it)->location().toString()));

            (*it)->renew(getSubscriptionTimeout(req));
            *sid = (*it)->sid();
            return Ok;
        }
        else if (sub->expired())
        {
            HLOG_INFO(QString("removing subscriber [SID [%1]] from [%2]").arg(
                sub->sid().toString(), (*it)->location().toString()));

            it = m_remoteClients.erase(it);
        }
        else
        {
            ++it;
        }
    }

    HLOG_WARN(QString("Cannot renew subscription. Invalid SID: [%1]").arg(
        req.sid().toString()));

    return PreconditionFailed;
}

void EventNotifier::stateChanged(const HService* source)
{
    HLOG(H_AT, H_FUN);

    Q_ASSERT(source->isEvented());

    QByteArray msgBody;
    getCurrentValues(msgBody, source);

    QMutexLocker lock(&m_remoteClientsMutex);

    if (m_shutdown)
    {
        return;
    }

    QList<ServiceEventSubscriberPtrT>::iterator it = m_remoteClients.begin();
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

    // TODO add multicast event support
}

void EventNotifier::initialNotify(ServiceEventSubscriberPtrT rc, MessagingInfo& mi)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QByteArray msgBody;
    getCurrentValues(msgBody, rc->service());

    if (mi.keepAlive() && mi.socket().state() == QTcpSocket::ConnectedState)
    {
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // !!slight deviation from the UDA v1.1 specification!!
        //
        // the timeout for acknowledging a initial notify request using the
        // same connection is set to two seconds, instead of the 30 as specified
        // in the standard. This is for two reasons:
        // 1) there exists UPnP software that do not implement and respect
        // HTTP keep-alive properly.
        // 2) initial notify using HTTP keep-alive is very fast (unless something
        // is wrong) and even a second should be more than enough.

        // with the above in mind, if the subscriber seems to use HTTP keep-alive,
        // the initial notify is sent using the connection in which the
        // subscription came. However, if that fails, the initial notify is
        // re-send using a new connection.

        mi.setReceiveTimeoutForNoData(2000);

        if (rc->initialNotify(msgBody, &mi))
        {
            return;
        }

        HLOG_WARN_NONSTD(QString(
            "Initial notify to SID [%1] failed. The device does not seem to " \
            "respect HTTP keep-alive. Re-sending the initial notify using a new connection.").arg(
                rc->sid().toString()));
    }

    // before sending the initial event message (specified in UDA),
    // the UDA mandates that FIN has been sent to the subscriber unless
    // the connection is to be kept alive.
    if (mi.socket().state() == QTcpSocket::ConnectedState)
    {
        mi.socket().disconnectFromHost();
        if (mi.socket().state() != QAbstractSocket::UnconnectedState)
        {
            mi.socket().waitForDisconnected(100);
        }
    }

    rc->initialNotify(msgBody);
}

}
}
