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

#ifndef UPNP_CONTROL_POINT_P_H_
#define UPNP_CONTROL_POINT_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "upnp_udn.h"
#include "upnp_abstracthost_p.h"

#include "messaging/ssdp.h"
#include "messaging/ssdp_p.h"
#include "messaging/http_server_p.h"
#include "messaging/discovery_messages.h"
#include "messaging/ssdp_messageheader_objects_p.h"

#include "upnp_action.h"
#include "upnp_device.h"
#include "upnp_service.h"
#include "upnp_device_p.h"
#include "upnp_controlpoint.h"
#include "upnp_abstracthost_p.h"
#include "upnp_actionarguments.h"

#include <QList>
#include <QHash>
#include <QPair>
#include <QUuid>
#include <QTimer>
#include <QMutex>
#include <QTcpSocket>
#include <QSharedPointer>
#include <QScopedPointer>

class QString;
class QtSoapMessage;
class QHttpRequestHeader;

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

namespace Herqq
{

namespace Upnp
{

//
// Class for relaying action invocations across the network to the real
// HAction objects instantiated by device hosts
//
class HActionInvokeProxy
{
private:

    HService* m_service;
    QString m_actionName;
    HActionInputArguments  m_inArgs;
    HActionOutputArguments m_outArgs;

    QSharedPointer<QTcpSocket> m_sock;
    QUrl m_baseUrl;

    QSharedPointer<HHttpHandler> m_http;

    QtSoapMessage msgIO(const QtSoapMessage& soapMsg);
    void checkConnection();
    void checkConnection(QTcpSocket* sock);

public:

    HActionInvokeProxy(
        HService* service, const QString& actionName,
        const HActionInputArguments& inArgs,
        const HActionOutputArguments& outArgs);

    qint32 operator()(
        const HActionInputArguments& inArgs,
        HActionOutputArguments* outArgs);
};

class ControlPointHttpServer;
class HServiceSubscribtion;

//
// This class is used as a thread pool task when a HServiceSubscription
// is about to be deleted, since the service subscription instance should
// not wait for the completion of this operation.
//
class Unsubscribe :
    public QRunnable
{
H_DISABLE_COPY(Unsubscribe)

private:

    const QByteArray m_loggingIdentifier;

    const HSid m_sid;
    const QUrl m_eventUrl;
    HHttpHandler& m_http;
    const bool m_noWait;

public:

    Unsubscribe(
        const QByteArray& loggingIdentifier,
        const HSid& sid, const QUrl& eventUrl, HHttpHandler&, bool noWait);

    virtual void run();
};

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
friend class HControlPointPrivate;
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

    inline QUuid id() const { return m_randomIdentifier; }

    void subscribe();
    void unsubscribe(bool exiting);

    void onNotify(MessagingInfo&, const NotifyRequest&);
};

class HControlPointPrivate;

//
// The HTTP server the control point uses to receive event notifications.
//
class ControlPointHttpServer :
    public HHttpServer
{
Q_OBJECT
H_DISABLE_COPY(ControlPointHttpServer)

private:

    HControlPointPrivate* m_owner;

protected:

    virtual void incomingNotifyMessage(MessagingInfo&, const NotifyRequest&);

public:

    explicit ControlPointHttpServer(HControlPointPrivate* owner, QObject* parent = 0);
    virtual ~ControlPointHttpServer();
};

//
// Implementation details of HControlPointConfiguration class
//
class HControlPointConfigurationPrivate
{
public: // attributes

    HDeviceCreator m_deviceCreator;

public: // methods

    HControlPointConfigurationPrivate();
    virtual ~HControlPointConfigurationPrivate();
};

//
//
//
class IFetchAndAddDevice :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(IFetchAndAddDevice)

public:

    IFetchAndAddDevice();
    virtual ~IFetchAndAddDevice() = 0;

    virtual qint32 completionValue() const = 0;
    virtual QString errorString() const = 0;
    virtual HDeviceController* createdDevice() = 0;

Q_SIGNALS:

    void done(Herqq::Upnp::HUdn);
};

//
// This class is used as a thread pool task to fetch a device description, its
// accompanying service descriptions (if any) and to build the device model.
//
template<typename Msg>
class FetchAndAddDevice :
    public IFetchAndAddDevice,
    public QRunnable
{
H_DISABLE_COPY(FetchAndAddDevice)

private:

    HControlPointPrivate* m_owner;
    Msg m_msg;

    QAtomicInt m_completionValue;
    QString m_errorString;
    QScopedPointer<HDeviceController> m_createdDevice;

private:

    void createEventSubscriptions(
        HDeviceController* device,
        QList<HServiceSubscribtion* >* subscriptions);

    void deleteSubscriptions(
        const QList<HServiceSubscribtion*>& subscriptions);

public:

    FetchAndAddDevice(HControlPointPrivate* owner, const Msg& msg);
    virtual ~FetchAndAddDevice();

    virtual void run();

    virtual qint32 completionValue() const;
    virtual QString errorString() const;
    virtual HDeviceController* createdDevice();
};

//
//
//
class DeviceBuildProcess
{
H_DISABLE_COPY(DeviceBuildProcess)

public:

    IFetchAndAddDevice* m_asyncOperation;
    QList<QUrl> m_locations;
    QScopedPointer<HUdn> m_udn;

    DeviceBuildProcess() :
        m_asyncOperation(0), m_locations(), m_udn(0)
    {
    }
};

//
//
//
class BuildsInProgress
{
H_DISABLE_COPY(BuildsInProgress)

private:

    QList<DeviceBuildProcess*> m_builds;

public:

    BuildsInProgress() : m_builds()
    {
    }

    template<typename Msg>
    inline DeviceBuildProcess* get(const Msg& msg) const
    {
        QList<DeviceBuildProcess*>::const_iterator ci = m_builds.constBegin();

        for(; ci != m_builds.constEnd(); ++ci)
        {
            if (*(*ci)->m_udn == msg.usn().udn())
            {
                return *ci;
            }

            QList<QUrl>::const_iterator ci2 = (*ci)->m_locations.constBegin();
            for(; ci2 != (*ci)->m_locations.constEnd(); ++ci2)
            {
                if (*ci2 == msg.location())
                {
                    return *ci;
                }
            }
        }

        return 0;
    }

    inline DeviceBuildProcess* get(const HUdn& udn) const
    {
        QList<DeviceBuildProcess*>::const_iterator ci = m_builds.constBegin();

        for(; ci != m_builds.constEnd(); ++ci)
        {
            if (*(*ci)->m_udn == udn)
            {
                return *ci;
            }
        }

        return 0;
    }

    inline void remove(const HUdn& udn)
    {
        QList<DeviceBuildProcess*>::iterator i = m_builds.begin();

        for(; i != m_builds.end(); ++i)
        {
            if (*(*i)->m_udn == udn)
            {
                m_builds.erase(i);
                return;
            }
        }

        Q_ASSERT(false);
    }

    inline void add(DeviceBuildProcess* arg)
    {
        Q_ASSERT(arg);
        m_builds.push_back(arg);
    }

    inline QList<DeviceBuildProcess*> values() const
    {
        return m_builds;
    }
};

//
// Implementation details of HControlPoint
//
class HControlPointPrivate :
    public HAbstractHostPrivate
{
Q_OBJECT
H_DECLARE_PUBLIC(HControlPoint)
H_DISABLE_COPY(HControlPointPrivate)
template<typename Msg>
friend class FetchAndAddDevice;

private:

    BuildsInProgress m_buildsInProgress;
    // this is accessed only from the thread in which all the HUpnp objects live.

private Q_SLOTS:

    void deviceModelBuildDone(Herqq::Upnp::HUdn);

private:

    void subscribeToEvents(HDeviceController* device);

    void removeRootDeviceAndSubscriptions(
        HDeviceController* device, bool unsubscribe);

    void removeRootDeviceSubscriptions(
        HDeviceController* rootDevice, bool unsubscribe);

    virtual void doClear();

private Q_SLOTS:

    void deviceExpired(HDeviceController* source);
    void addRootDevice_(HDeviceController* device);

public:

    QScopedPointer<HControlPointConfiguration> m_initParams;
    SsdpWithoutEventing<HControlPointPrivate>* m_ssdp;

    ControlPointHttpServer* m_server;
    QHash<QUuid, HServiceSubscribtion*> m_serviceSubscribtions;
    QMutex m_serviceSubscribtionsMutex;

    QMutex m_deviceCreationMutex;

    HControlPointPrivate();
    virtual ~HControlPointPrivate();

    bool readyForEvents();

    bool discoveryRequestReceived (
        const Herqq::Upnp::HDiscoveryRequest& msg,
        const HEndpoint& source,
        const HEndpoint& destination);

    bool discoveryResponseReceived(
        const Herqq::Upnp::HDiscoveryResponse& msg,
        const HEndpoint& source);

    bool resourceUnavailableReceived(const Herqq::Upnp::HResourceUnavailable& msg);
    bool resourceAvailableReceived  (const Herqq::Upnp::HResourceAvailable& msg);

    template<class Msg>
    bool processDeviceDiscovery(
        const Msg& msg, const HEndpoint& source = HEndpoint());

    template<class Msg>
    bool shouldFetch(const Msg& msg);

    HDeviceController* fetchDevice(QUrl deviceLocation, qint32 maxAge);
};

}
}

#endif /* UPNP_CONTROL_POINT_P_H_ */
