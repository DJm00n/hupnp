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

#ifndef HEVENT_NOTIFIER_P_H_
#define HEVENT_NOTIFIER_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "hevent_subscriber_p.h"

#include "./../../http/hhttp_p.h"
#include "./../../general/hdefs_p.h"
#include "./../messages/hevent_messages_p.h"

#include <QList>
#include <QMutex>
#include <QObject>
#include <QSharedPointer>

namespace Herqq
{

namespace Upnp
{

class HService;

//
// Internal class used to notify event subscribers of events.
//
class EventNotifier :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(EventNotifier)

public:

    typedef QSharedPointer<ServiceEventSubscriber> ServiceEventSubscriberPtrT;

private:

    const QByteArray m_loggingIdentifier;
    // prefix for logging

    HHttpHandler& m_httpHandler;
    // shared http messaging helper used by objects under the control of a
    // HDeviceHost

    QList<ServiceEventSubscriberPtrT> m_remoteClients;
    mutable QMutex m_remoteClientsMutex;

    volatile bool m_shutdown;

private Q_SLOTS:

    void stateChanged(const Herqq::Upnp::HService* source);

public:

    EventNotifier(
        const QByteArray& loggingIdentifier, HHttpHandler&, QObject* parent);

    virtual ~EventNotifier();

    void shutdown();

    StatusCode addSubscriber(HService*, const SubscribeRequest&, HSid*);

    bool removeSubscriber(const UnsubscribeRequest&);
    StatusCode renewSubscription(const SubscribeRequest&, HSid*);
    ServiceEventSubscriberPtrT remoteClient(const HSid&) const;

    void initialNotify(ServiceEventSubscriberPtrT, MessagingInfo& mi);
};

}
}

#endif /* HEVENT_NOTIFIER_P_H_ */
