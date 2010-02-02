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

#ifndef SERVICE_EVENT_SUBSCRIBER_H_
#define SERVICE_EVENT_SUBSCRIBER_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../../defs_p.h"
#include "../../messaging/event_messages_p.h"

#include <QQueue>
#include <QTimer>
#include <QMutex>
#include <QObject>
#include <QRunnable>
#include <QWaitCondition>

class QByteArray;
class QTcpSocket;
class QThreadPool;

namespace Herqq
{

namespace Upnp
{

class HService;
class HHttpHandler;
class MessagingInfo;

//
// Internal class used to maintain information about a single event subscriber.
//
class ServiceEventSubscriber :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(ServiceEventSubscriber)

private:

    //
    //
    //
    class MessageSender :
        public QRunnable
    {
    H_DISABLE_COPY(MessageSender)
    friend class ServiceEventSubscriber;

    private:

        ServiceEventSubscriber* m_owner;

        QScopedPointer<QTcpSocket> m_socket;
        QQueue<QByteArray> m_messagesToSend;
        QMutex m_messagesToSendMutex;
        QWaitCondition m_messagesAvailable;
        volatile bool m_shuttingDown;
        volatile bool m_done;

    private:

        bool connect();
        void clear();

    public:

        MessageSender(ServiceEventSubscriber* owner);
        virtual void run();
    };

friend class MessageSender;

private: // attributes

    HHttpHandler& m_http;
    HService*     m_service;
    QUrl          m_location;
    HSid          m_sid;
    QAtomicInt    m_seq;
    HTimeout      m_timeout;
    QAtomicInt    m_shuttingDown;
    QTimer        m_timer;
    MessageSender m_messageSender;
    QThreadPool&  m_threadPool;
    QMutex        m_expirationMutex;

private Q_SLOTS:

    void subscriptionTimeout();

public:

    ServiceEventSubscriber(
        HHttpHandler& http, QThreadPool& tp,
        HService* service, const QUrl location, const HTimeout& timeout,
        QObject* parent = 0);

    virtual ~ServiceEventSubscriber();

    void notify(const QByteArray& msgBody);
    bool initialNotify(const QByteArray& msgBody, MessagingInfo* = 0);

    bool isInterested(const HService* service) const;

    inline QUrl      location() const { return m_location; }
    inline HSid      sid     () const { return m_sid;      }
    inline quint32   seq     () const { return m_seq;      }
    inline HTimeout  timeout () const { return m_timeout;  }
    inline HService* service () const { return m_service;  }
    inline bool      expired () const { return m_shuttingDown; }

    void expire();
    void renew();
};

}
}

#endif /* SERVICE_EVENT_SUBSCRIBER_H_ */
