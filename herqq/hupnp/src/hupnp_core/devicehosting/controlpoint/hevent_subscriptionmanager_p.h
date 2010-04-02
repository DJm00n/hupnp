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

#ifndef HEVENT_SUBSCRIPTIONMANAGER_P_H_
#define HEVENT_SUBSCRIPTIONMANAGER_P_H_

#include "hevent_subscription_p.h"
#include "../../general/hupnp_fwd.h"

#include <QList>
#include <QHash>
#include <QUuid>
#include <QMutex>
#include <QObject>
#include <QSharedPointer>

namespace Herqq
{

namespace Upnp
{

class HServiceController;
class HControlPointPrivate;

//
//
//
class HEventSubscriptionManager :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HEventSubscriptionManager)

private:

    HControlPointPrivate* m_owner;

    QHash<QUuid, HServiceSubscribtion*> m_subscribtionsByUuid;
    QHash<HUdn, QList<HServiceSubscribtion*>* > m_subscriptionsByUdn;
    QMutex m_subscribtionsMutex;

private:

    HServiceSubscribtion* createSubscription(HServiceController*, qint32 timeout);

public Q_SLOTS:

    void subscribed_slot(HServiceSubscribtion*);
    void subscriptionFailed_slot(HServiceSubscribtion*);
    void unsubscribed(HServiceSubscribtion*);

Q_SIGNALS:

    void subscribed(Herqq::Upnp::HService*);
    void subscriptionFailed(Herqq::Upnp::HService*);
    void unsubscribed(Herqq::Upnp::HService*);

public:

    HEventSubscriptionManager(HControlPointPrivate*);
    virtual ~HEventSubscriptionManager();

    enum SubscriptionResult
    {
        Sub_Success = 0,
        Sub_AlreadySubscribed = 1,
        Sub_Failed_NotEvented = 2,
    };

    void subscribe(HDevice*, bool recursive, qint32 timeout);
    SubscriptionResult subscribe(HService*, qint32 timeout);

    // the unsubscribe flag specifies whether to send unsubscription to the UPnP device
    // if not, the subscription is just reset to default state (in which it does nothing)
    bool cancel(HDevice*, bool recursive, bool unsubscribe);
    bool cancel(HService*, bool unsubscribe);
    void cancelAll(qint32 msecsToWait);

    bool remove(HDevice*, bool recursive);
    bool remove(HService*);
    void removeAll();

    bool onNotify(const QUuid& id, MessagingInfo& mi, const NotifyRequest& req);
};

}
}

#endif /* HEVENT_SUBSCRIPTIONMANAGER_P_H_ */
