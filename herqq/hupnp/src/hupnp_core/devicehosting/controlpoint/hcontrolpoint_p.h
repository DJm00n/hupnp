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

#ifndef HCONTROL_POINT_P_H_
#define HCONTROL_POINT_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "hcontrolpoint.h"
#include "hdevicebuild_p.h"
#include "hactioninvoke_proxy_p.h"
#include "hevent_subscriptionmanager_p.h"

#include "../habstracthost_p.h"
#include "../../devicemodel/hdevice.h"
#include "../../devicemodel/hservice.h"
#include "../../devicemodel/hactioninvoke.h"

#include "../../ssdp/hssdp.h"
#include "../../ssdp/hssdp_p.h"
#include "../../http/hhttp_server_p.h"
#include "../../ssdp/hdiscovery_messages.h"

#include <QUuid>
#include <QMutex>
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

private Q_SLOTS:

    void notify_slot(const QString*, const NotifyRequest*, StatusCode*, HRunnable*);

protected:

    virtual void incomingNotifyMessage(
        MessagingInfo&, const NotifyRequest&, HRunnable*);

public:

    explicit ControlPointHttpServer(HControlPointPrivate*);
    virtual ~ControlPointHttpServer();

Q_SIGNALS:

    void notify_sig(const QString*, const NotifyRequest*, StatusCode*, HRunnable*);
};

//
//
//
class HControlPointSsdpHandler :
    public HSsdp
{
H_DISABLE_COPY(HControlPointSsdpHandler)

private:

    HControlPointPrivate* m_owner;

protected:

    virtual bool incomingDiscoveryResponse(
        const HDiscoveryResponse& msg, const HEndpoint& source);

    virtual bool incomingDeviceAvailableAnnouncement(
        const HResourceAvailable& msg, const HEndpoint& source);

    virtual bool incomingDeviceUnavailableAnnouncement(
        const HResourceUnavailable& msg, const HEndpoint& source);

public:

    HControlPointSsdpHandler(HControlPointPrivate*);
    virtual ~HControlPointSsdpHandler();
};

//
// Thread class used by the HControlPoint to run action invocations
//
class HControlPointThread :
    public QThread
{
H_DISABLE_COPY(HControlPointThread)

private:

    volatile bool m_exit;

protected:

    virtual void run();

public:

    HControlPointThread();
    void quit();
};

//
// Implementation details of HControlPoint
//
class H_UPNP_CORE_EXPORT HControlPointPrivate :
    public HAbstractHostPrivate
{
Q_OBJECT
H_DECLARE_PUBLIC(HControlPoint)
H_DISABLE_COPY(HControlPointPrivate)
friend class DeviceBuildTask;
friend class HControlPointSsdpHandler;

private:

    DeviceBuildTasks m_deviceBuildTasks;
    // this is accessed only from the thread in which all the HUpnp objects live.

private Q_SLOTS:

    void deviceModelBuildDone(const Herqq::Upnp::HUdn&);

private:

    bool addRootDevice(HDeviceController* device);
    void subscribeToEvents(HDeviceController*);

    HActionInvokeProxy* createActionInvoker(HAction*);

    void processDeviceOnline(HDeviceController*, bool newDevice);

    bool processDeviceOffline(
        const HResourceUnavailable& msg, const HEndpoint& source,
        HControlPointSsdpHandler* origin);

    template<class Msg>
    bool processDeviceDiscovery(
        const Msg& msg, const HEndpoint& source,
        HControlPointSsdpHandler* origin);

    template<class Msg>
    bool shouldFetch(const Msg& msg);

    virtual void doClear();

private Q_SLOTS:

    void deviceExpired(HDeviceController* source);
    void unsubscribed(Herqq::Upnp::HServiceProxy*);

public:

    QScopedPointer<HControlPointConfiguration> m_configuration;
    QList<QPair<quint32, HControlPointSsdpHandler*> > m_ssdps;
    // the int is a ipv4 network address

    ControlPointHttpServer* m_server;
    HEventSubscriptionManager* m_eventSubscriber;

    QMutex m_deviceCreationMutex;
    //

    HControlPoint::ControlPointError m_lastError;

    HControlPoint* q_ptr;

    QScopedPointer<HControlPointThread> m_controlPointThread;

    HControlPointPrivate();
    virtual ~HControlPointPrivate();

    HDeviceController* buildDevice(
        const QUrl& deviceLocation, qint32 maxAge, QString* err);
};

}
}

#endif /* HCONTROL_POINT_P_H_ */
