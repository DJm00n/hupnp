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

#ifndef UPNP_DEVICEHOST_P_H_
#define UPNP_DEVICEHOST_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "upnp_device.h"
#include "upnp_service.h"
#include "upnp_devicehost.h"
#include "upnp_deviceinfo.h"
#include "upnp_abstracthost_p.h"

#include "messaging/ssdp.h"
#include "messaging/http_server_p.h"

#include <QUrl>
#include <QQueue>
#include <QMutex>
#include <QObject>
#include <QAtomicInt>
#include <QTcpSocket>
#include <QScopedPointer>

namespace Herqq
{

namespace Upnp
{

//
// Internal class used to maintain information about a single event subscriber.
//
class UnicastRemoteClient :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(UnicastRemoteClient)

private:

    //
    //
    //
    class MessageSender :
        public QRunnable
    {
    H_DISABLE_COPY(MessageSender)
    friend class UnicastRemoteClient;

    private:

        UnicastRemoteClient* m_owner;

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

        MessageSender(UnicastRemoteClient* owner);
        virtual void run();
    };

friend class MessageSender;

private: // attributes

    HHttpHandler& m_http;
    HService* m_service;
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

    UnicastRemoteClient(
        HHttpHandler& http, QThreadPool& tp,
        HService* service, const QUrl location, const HTimeout& timeout,
        QObject* parent = 0);

    virtual ~UnicastRemoteClient();

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

//
// Internal class used to notify event subscribers of events.
//
class RemoteClientNotifier :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(RemoteClientNotifier)
friend class DeviceHostHttpServer;

public:

    typedef QSharedPointer<UnicastRemoteClient> RemoteClientPtrT;

private:

    HDeviceHostPrivate* m_owner;
    QList<RemoteClientPtrT> m_remoteClients;
    mutable QMutex m_remoteClientsMutex;

private Q_SLOTS:

    void stateChanged(const Herqq::Upnp::HService* source);

public:

    RemoteClientNotifier(HDeviceHostPrivate* owner);

    virtual ~RemoteClientNotifier();

    RemoteClientPtrT addSubscriber(
        HService* service, const SubscribeRequest& sreq);

    bool removeSubscriber(const UnsubscribeRequest& usreq);
    RemoteClientPtrT renewSubscription(const SubscribeRequest& sreq);
    RemoteClientPtrT remoteClient(const HSid& sid) const;
};

class HDeviceHostPrivate;

//
// Internal class that provides minimal HTTP server functionality for the needs of
// Device Host
//
class DeviceHostHttpServer :
    public HHttpServer
{
Q_OBJECT
H_DISABLE_COPY(DeviceHostHttpServer)
friend class HDeviceHostPrivate;
friend class SubscriptionRequestTask;
friend class UnsubscriptionRequestTask;

private:

    HDeviceHostPrivate* m_deviceHost;

private Q_SLOTS:

    void processSubscription_slot(
        const SubscribeRequest*, HService*, HSid*);

    void removeSubscriber_slot(const UnsubscribeRequest*, bool*);

Q_SIGNALS:

    void processSubscription_sig(
        const SubscribeRequest*, HService*, HSid*);

    void removeSubscriber_sig(const UnsubscribeRequest*, bool*);

protected:

    virtual void incomingSubscriptionRequest(
        MessagingInfo&, const SubscribeRequest&);

    virtual void incomingUnsubscriptionRequest(
        MessagingInfo&, const UnsubscribeRequest&);

    virtual void incomingControlRequest(
        MessagingInfo&, const InvokeActionRequest&);

    virtual void incomingUnknownHeadRequest(
        MessagingInfo&, const QHttpRequestHeader&);

    virtual void incomingUnknownGetRequest(
        MessagingInfo&, const QHttpRequestHeader&);

    virtual void incomingUnknownPostRequest(
        MessagingInfo&, const QHttpRequestHeader&, const QByteArray& body);

public:

    explicit DeviceHostHttpServer (
        HDeviceHostPrivate* deviceHost, QObject* parent = 0);

    virtual ~DeviceHostHttpServer();
};


//
// Implementation details of HDeviceConfiguration class
//
class HDeviceConfigurationPrivate
{
public: // attributes

    QString        m_pathToDeviceDescriptor;
    qint32         m_cacheControlMaxAgeInSecs;
    HDeviceCreator  m_deviceCreator;

public: // methods

    HDeviceConfigurationPrivate();
    virtual ~HDeviceConfigurationPrivate();
};

//
//
//
class HDeviceHostConfigurationPrivate
{
H_DISABLE_COPY(HDeviceHostConfigurationPrivate)

public:
    QList<HDeviceConfiguration*> m_collection;

    HDeviceHostConfigurationPrivate();
};

//
//
//
class Permission
{
H_DISABLE_COPY(Permission)

private:

    HDeviceHostPrivate& m_dh;
    bool m_valid;

public:

    Permission (HDeviceHostPrivate& dh);
    ~Permission();

    bool isValid() const;
};

//
//
//
class DeviceHostSsdpHandler :
    public HSsdp
{
H_DISABLE_COPY(DeviceHostSsdpHandler)

private:

    HDeviceHostPrivate& m_dh;

protected:

    virtual bool incomingDiscoveryRequest (
        const Herqq::Upnp::HDiscoveryRequest& msg,
        const HEndpoint& source,
        const HEndpoint& destination);

    virtual bool incomingDiscoveryResponse(
        const Herqq::Upnp::HDiscoveryResponse& msg,
        const HEndpoint& source);

    virtual bool incomingDeviceAvailableAnnouncement(
        const Herqq::Upnp::HResourceAvailable& msg);

    virtual bool incomingDeviceUnavailableAnnouncement(
        const Herqq::Upnp::HResourceUnavailable& msg);

public:

    DeviceHostSsdpHandler(HDeviceHostPrivate& dh, QObject* parent = 0);
    virtual ~DeviceHostSsdpHandler();
};

//
// Implementation details of HDeviceHost class
//
class HDeviceHostPrivate :
    public HAbstractHostPrivate
{
Q_OBJECT
H_DISABLE_COPY(HDeviceHostPrivate);
H_DECLARE_PUBLIC(HDeviceHost);
friend class DeviceHostSsdpHandler;
friend class DeviHDeviceStatusNotifierpServer;
friend class HDeviceStatusNotifier;
friend class Permission;

private:

    template<typename AnnouncementType>
    void announce();

    template<typename AnnouncementType>
    void createAnnouncementMessagesForRootDevice(
        HDeviceController* rootDevice,
        QList<AnnouncementType>& notifications);

    template<typename AnnouncementType>
    void createAnnouncementMessagesForEmbeddedDevice(
        HDeviceController* device, QList<AnnouncementType>& notifications);

    template<typename AnnouncementType>
    void sendAnnouncements(const QList<AnnouncementType>& announcements);

public: // attributes

    HDeviceHostConfiguration m_initParams;
    //

    DeviceHostSsdpHandler* m_ssdp;
    //

    qint32 m_individualAdvertisementCount;
    //

    DeviceHostHttpServer* m_httpServer;
    //

    QAtomicInt m_activeRequestCount;

    QScopedPointer<RemoteClientNotifier> m_remoteClientNotifier;

    virtual void doClear();

public Q_SLOTS:

    void announcementTimedout(HDeviceController*);

public: // methods

    HDeviceHostPrivate();
    virtual ~HDeviceHostPrivate();

    QString findDeviceDescriptor (const QString& path);
    QString findServiceDescriptor(const QString& path);
    QImage  findIcon(const QString& path);

    void processSearchRequest(
        HDeviceController* device, const QUrl& location,
        QList<HDiscoveryResponse>* responses);

    void processSearchRequest_AllDevices(
        const HDiscoveryRequest& req, const HEndpoint& source,
        QList<HDiscoveryResponse>* responses);

    void processSearchRequest_RootDevice(
        const HDiscoveryRequest& req, const HEndpoint& source,
        QList<HDiscoveryResponse>* responses);

    void processSearchRequest_specificDevice(
        const HDiscoveryRequest& req, const HEndpoint& source,
        QList<HDiscoveryResponse>* responses);

    void processSearchRequest_deviceType(
        const HDiscoveryRequest& req, const HEndpoint& source,
        QList<HDiscoveryResponse>* responses);

    void processSearchRequest_serviceType(
        const HDiscoveryRequest& req, const HEndpoint& source,
        QList<HDiscoveryResponse>* responses);

    void stopNotifiers();
    void startNotifiers();
    void createRootDevices();
    void connectSelfToServiceSignals(HDevice* device);
};

}
}


#endif /* UPNP_DEVICEHOST_P_H_ */
