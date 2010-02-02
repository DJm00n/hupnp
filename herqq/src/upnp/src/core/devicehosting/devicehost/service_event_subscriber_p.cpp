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

#include "service_event_subscriber_p.h"

#include "../../devicemodel/service.h"

#include "../../dataelements/serviceid.h"

#include "../../messaging/http_handler_p.h"

#include "../../../../utils/src/logger_p.h"
#include "../../../../utils/src/sysutils_p.h"

#include "../../../../core/include/HExceptions"

#include <QThread>
#include <QTcpSocket>
#include <QThreadPool>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * ServiceEventSubscriber::MessageSender
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
        HLOG_WARN(QString(
            "Client @ [sid: %1] is not connected. Failed to notify.").arg(
                sid.toString()));

        return false;
    }

    NotifyRequest req(location, sid, seq, msgBody);
    try
    {
        HLOG_DBG(QString(
            "Sending notification [seq: %1] to subscriber [%2] @ [%3]").arg(
                QString::number(seq), sid.toString(), location.toString()));

        http.msgIO(mi, req);
    }
    catch(HException& ex)
    {
        HLOG_WARN(QString(
            "An error occurred while notifying [seq: %1, sid: %2] host @ [%3]: %4").arg(
                QString::number(seq), sid.toString(), location.toString(), ex.reason()));

        return false;
    }

    return true;
}
}

ServiceEventSubscriber::MessageSender::MessageSender(ServiceEventSubscriber* owner) :
    m_owner(owner), m_socket(0), m_messagesToSend(),
    m_messagesToSendMutex(), m_messagesAvailable(), m_shuttingDown(false),
    m_done(false)
{
}

bool ServiceEventSubscriber::MessageSender::connect()
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

void ServiceEventSubscriber::MessageSender::run()
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

            lock.relock();
            if (m_messagesToSend.isEmpty())
            {
                break;
            }
            QByteArray message = m_messagesToSend.dequeue();
            lock.unlock();

            qint32 seq = m_owner->m_seq.fetchAndAddOrdered(1);

            if (!notifyClient(
                    m_owner->m_http, mi, message, m_owner->m_location,
                    m_owner->m_sid, seq))
            {
                // notify failed
                //
                // according to UDA v1.1:
                // "the publisher SHOULD abandon sending this message to the
                // subscriber but MUST keep the subscription active and send future event
                // messages to the subscriber until the subscription expires or is canceled."

                HLOG_WARN(QObject::tr(
                    "Could not send notify [seq: %1, sid: %2] to host @ [%3].").arg(
                        QString::number(seq), m_owner->m_sid.toString(),
                        m_owner->m_location.toString()));
            }

            // notify succeeded
        }

        m_socket->disconnectFromHost();
    }

end:

    m_socket.reset(0);
    m_owner->expire();
    m_done = true;
}

/*******************************************************************************
 * ServiceEventSubscriber
 ******************************************************************************/
ServiceEventSubscriber::ServiceEventSubscriber(
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

    bool ok = connect(
        &m_timer, SIGNAL(timeout()), this, SLOT(subscriptionTimeout()));

    Q_ASSERT(ok); Q_UNUSED(ok)

    if (!timeout.isInfinite())
    {
        m_timer.start(timeout.value() * 1000);
    }

    m_messageSender.setAutoDelete(false);
    m_threadPool.start(&m_messageSender);
}

ServiceEventSubscriber::~ServiceEventSubscriber()
{
    HLOG(H_AT, H_FUN);

    expire();

    while(!m_messageSender.m_done)
    {
        // cannot exit until the sender is done, since the sender holds a reference
        // back to this object.
        HSysUtils::msleep(1);
    }
}

void ServiceEventSubscriber::subscriptionTimeout()
{
    HLOG(H_AT, H_FUN);

    expire();

    HLOG_DBG(QObject::tr("Subscription from [%1] with SID %2 expired").arg(
        m_location.toString(), m_sid.toString()));
}

void ServiceEventSubscriber::expire()
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

bool ServiceEventSubscriber::isInterested(const HService* service) const
{
    HLOG(H_AT, H_FUN);

    return !expired() && m_seq && m_service->isEvented() &&
            m_service->serviceId() == service->serviceId();
}

void ServiceEventSubscriber::renew()
{
    HLOG(H_AT, H_FUN);

    QMutexLocker lock(&m_expirationMutex);

    if (expired())
    {
        return;
    }

    m_timer.start();
}

void ServiceEventSubscriber::notify(const QByteArray& msgBody)
{
    HLOG2(H_AT, H_FUN, "__DEVICE HOST__: ");

    Q_ASSERT(m_seq);

    QMutexLocker lock(&m_messageSender.m_messagesToSendMutex);
    m_messageSender.m_messagesToSend.enqueue(msgBody);
    m_messageSender.m_messagesAvailable.wakeOne();
}

bool ServiceEventSubscriber::initialNotify(
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

}
}
