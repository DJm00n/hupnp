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

#ifndef HDEVICEHOST_HTTP_SERVER_H_
#define HDEVICEHOST_HTTP_SERVER_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "hevent_notifier_p.h"
#include "hdevicehost_configuration.h"

#include "../hdevicestorage_p.h"

#include "../../general/hdefs_p.h"
#include "../../http/hhttp_server_p.h"

#include "../../devicemodel/haction_p.h"

namespace Herqq
{

namespace Upnp
{

//
//
//
class HActionInvocationInfo
{
H_DISABLE_COPY(HActionInvocationInfo)

public:

    HActionInvocationInfo(
        HActionController* action, HActionArguments* inArgs,
        HActionArguments* outArgs) :
            m_action(action), m_inArgs(inArgs), m_outArgs(outArgs)
    {
        Q_ASSERT(action);
        Q_ASSERT(inArgs);
        Q_ASSERT(outArgs);
    }

    HActionController* m_action;
    HActionArguments* m_inArgs;
    HActionArguments* m_outArgs;
    qint32 m_retVal;
};

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

private:

    DeviceStorage& m_deviceStorage;
    EventNotifier& m_eventNotifier;
    HDeviceHostConfiguration::ThreadingModel m_threadingModel;

private Q_SLOTS:

    void processSubscription_slot(const SubscribeRequest*, HService*, HSid*,
            StatusCode*, HRunnable*);

    void removeSubscriber_slot(const UnsubscribeRequest*, bool*, HRunnable*);

    void invokeFromMainThread_slot(HActionInvocationInfo*, HRunnable*);

Q_SIGNALS:

    void processSubscription_sig(const SubscribeRequest*, HService*, HSid*,
            StatusCode*, HRunnable*);

    void removeSubscriber_sig(const UnsubscribeRequest*, bool*, HRunnable*);

    void invokeFromMainThread(HActionInvocationInfo*, HRunnable*);

protected:

    virtual void incomingSubscriptionRequest(
        MessagingInfo&, const SubscribeRequest&, HRunnable*);

    virtual void incomingUnsubscriptionRequest(
        MessagingInfo&, const UnsubscribeRequest&, HRunnable*);

    virtual void incomingControlRequest(
        MessagingInfo&, const InvokeActionRequest&, HRunnable*);

    virtual void incomingUnknownHeadRequest(
        MessagingInfo&, const QHttpRequestHeader&, HRunnable*);

    virtual void incomingUnknownGetRequest(
        MessagingInfo&, const QHttpRequestHeader&, HRunnable*);

    virtual void incomingUnknownPostRequest(
        MessagingInfo&, const QHttpRequestHeader&, const QByteArray& body, HRunnable*);

public:

    DeviceHostHttpServer(const QByteArray& loggingId,
            HDeviceHostConfiguration::ThreadingModel, DeviceStorage&,
            EventNotifier&, QObject* parent = 0);

    virtual ~DeviceHostHttpServer();
};

}
}

#endif /* HDEVICEHOST_HTTP_SERVER_H_ */
