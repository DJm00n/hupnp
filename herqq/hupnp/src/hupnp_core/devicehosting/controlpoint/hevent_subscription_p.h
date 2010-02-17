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

#ifndef HEVENT_SUBSCRIPTION_H_
#define HEVENT_SUBSCRIPTION_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "./../messages/hsid_p.h"
#include "./../messages/hevent_messages_p.h"

#include "./../../general/hdefs_p.h"
#include "./../../general/hupnp_fwd.h"

#include <QUrl>
#include <QList>
#include <QMutex>
#include <QTimer>
#include <QRunnable>
#include <QByteArray>

class QTcpSocket;
class QThreadPool;

namespace Herqq
{

namespace Upnp
{

class HHttpHandler;
class HServiceSubscribtion;

//
// This class is used as a thread pool task to renew a subscription or
// to re-subscribe.
//
class RenewSubscription :
    public QRunnable
{
H_DISABLE_COPY(RenewSubscription)

private:

    HServiceSubscribtion* m_owner;

public:

    RenewSubscription(HServiceSubscribtion* owner);
    virtual void run();
};

class MessagingInfo;
class HServiceController;
class HControlPointPrivate;

//
// This class represents and maintains a subscription to a service instantiated on the
// device host (server) side.
//
class HServiceSubscribtion :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HServiceSubscribtion)
friend class RenewSubscription;
friend class Unsubscribe;

private:

    QByteArray m_loggingIdentifier;

    QThreadPool* m_threadPool;

    QMutex m_subscriptionMutex;

    QUuid  m_randomIdentifier;

    QList<QUrl> m_deviceLocations;
    HSid        m_sid;
    quint32     m_seq;
    HTimeout    m_timeout;
    QTimer      m_subscriptionTimer;

    QTimer        m_announcementTimer;
    volatile bool m_announcementTimedOut;

    HServiceController* m_service;
    QUrl m_serverRootUrl;
    QUrl m_lastConnectedLocation;

    volatile bool m_exiting;

    HHttpHandler& m_http;

private Q_SLOTS:

    void subscriptionTimeout();
    void announcementTimeout();
    void resetSubscription  ();

private:

    bool connectToDevice(QTcpSocket* sock, QUrl* connectedBaseUrl, bool useLastLocation);
    void renewSubscription();
    void resubscribe();

Q_SIGNALS:

    void startTimer(int);
    void stopTimer();

public:

    HServiceSubscribtion(
        const QByteArray& loggingIdentifier, HHttpHandler& http,
        const QList<QUrl>& deviceLocations, HServiceController* service,
        const QUrl& serverRootUrl, QThreadPool* threadPool, QObject* parent = 0);

    virtual ~HServiceSubscribtion();

    inline QUuid id() const  { return m_randomIdentifier ; }
    HService* service() const;

    void subscribe();

    void unsubscribe(bool exiting);

    void onNotify(MessagingInfo&, const NotifyRequest&);
};


}
}

#endif /* HEVENT_SUBSCRIPTION_H_ */
