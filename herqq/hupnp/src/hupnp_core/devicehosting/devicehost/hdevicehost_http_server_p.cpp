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

#include "hdevicehost_http_server_p.h"
#include "hevent_subscriber_p.h"

#include "../messages/hcontrol_messages_p.h"

#include "../../dataelements/hudn.h"
#include "../../general/hupnp_global_p.h"
#include "../../datatypes/hdatatype_mappings_p.h"

#include "../../devicemodel/haction_p.h"
#include "../../devicemodel/hservice_p.h"
#include "../../devicemodel/hactionarguments.h"

#include "../../../utils/hlogger_p.h"

#include <QtGui/QImage>

#include <QtCore/QUrl>
#include <QtCore/QPair>

#include <QtNetwork/QHttpRequestHeader>

namespace Herqq
{

namespace Upnp
{

namespace
{
QUuid extractUdn(const QUrl& arg)
{
    QString path = extractRequestPart(arg);

    QUuid udn(path.section('/', 1, 1));
    if (udn.isNull())
    {
        return QUuid();
    }

    return udn;
}

QString extractRequestExludingUdn(const QUrl& arg)
{
    QString pathToSearch = extractRequestPart(arg).section(
        '/', 2, -1, QString::SectionIncludeLeadingSep);

    return pathToSearch;
}
}

/*******************************************************************************
 * DeviceHostHttpServer
 ******************************************************************************/
DeviceHostHttpServer::DeviceHostHttpServer(
    const QByteArray& loggingId,
    HDeviceHostConfiguration::ThreadingModel threadingModel,
    DeviceStorage& ds,
    EventNotifier& en,
    QObject* parent) :
        HHttpServer(loggingId, parent),
            m_deviceStorage(ds), m_eventNotifier(en),
            m_threadingModel(threadingModel)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    bool ok = connect(
        this,
        SIGNAL(processSubscription_sig(
                const SubscribeRequest*, HService*, HSid*, StatusCode*,
                HRunnable*)),
        this,
        SLOT(processSubscription_slot(
                const SubscribeRequest*, HService*, HSid*, StatusCode*,
                HRunnable*)));

    Q_ASSERT(ok); Q_UNUSED(ok)

    ok = connect(
        this,
        SIGNAL(removeSubscriber_sig(
            const UnsubscribeRequest*, bool*, HRunnable*)),
        this,
        SLOT(removeSubscriber_slot(
            const UnsubscribeRequest*, bool*, HRunnable*)));

    Q_ASSERT(ok);

    ok = connect(
        this,
        SIGNAL(invokeFromMainThread(HActionInvocationInfo*, HRunnable*)),
        this,
        SLOT(invokeFromMainThread_slot(HActionInvocationInfo*, HRunnable*)));

    Q_ASSERT(ok);
}

DeviceHostHttpServer::~DeviceHostHttpServer()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    close();
}

void DeviceHostHttpServer::processSubscription_slot(
    const SubscribeRequest* req, HService* service, HSid* sid, StatusCode* sc,
    HRunnable* runner)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(req);
    Q_ASSERT(sid);
    Q_ASSERT(sc);
    Q_ASSERT(runner);

    // The UDA v1.1 does not specify what to do when a subscription is received
    // to a service that is not evented. A "safe" route was taken here and
    // all subscriptions are accepted rather than returning some error. However,
    // in such a case the timeout is adjusted to a day and no events are ever sent.

    if (req->isRenewal())
    {
        *sc = m_eventNotifier.renewSubscription(*req, sid);
    }
    else
    {
        *sc = m_eventNotifier.addSubscriber(service, *req, sid);
    }

    runner->signalTaskComplete();
}

void DeviceHostHttpServer::removeSubscriber_slot(
    const UnsubscribeRequest* req, bool* ok, HRunnable* runner)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(req);
    Q_ASSERT(ok);
    Q_ASSERT(runner);

    *ok = m_eventNotifier.removeSubscriber(*req);

    runner->signalTaskComplete();
}

void DeviceHostHttpServer::invokeFromMainThread_slot(
    HActionInvocationInfo* info, HRunnable* runner)
{
    Q_ASSERT(info);
    Q_ASSERT(runner);

    info->m_retVal = info->m_action->invoke(*info->m_inArgs, info->m_outArgs);
    runner->signalTaskComplete();
}

void DeviceHostHttpServer::incomingSubscriptionRequest(
    MessagingInfo& mi, const SubscribeRequest& sreq, HRunnable* runner)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(runner);

    HLOG_DBG("Subscription received.");

    QUuid udn = extractUdn(sreq.eventUrl());

    HDeviceController* device =
        !udn.isNull() ? m_deviceStorage.searchDeviceByUdn(HUdn(udn)) : 0;

    HServiceController* service = 0;

    if (!device)
    {
        // the request did not have the UDN prefix, which means that either
        // 1) the request was for a EventUrl that was defined as an absolute URL
        //    in the device description or
        // 2) the request is invalid

        service = m_deviceStorage.searchServiceByEventUrl(sreq.eventUrl());
        if (!service)
        {
            HLOG_WARN(QString(
                "Ignoring invalid event subscription to: [%1].").arg(
                    sreq.eventUrl().toString()));

            mi.setKeepAlive(false);
            m_httpHandler.send(mi, BadRequest);
            return;
        }
    }
    else if (!service)
    {
        service = m_deviceStorage.searchServiceByEventUrl(
            device, extractRequestExludingUdn(sreq.eventUrl()));
    }

    if (!service)
    {
        HLOG_WARN(QString("Subscription defined as [%1] is invalid.").arg(
            sreq.eventUrl().path()));

        mi.setKeepAlive(false);
        m_httpHandler.send(mi, BadRequest);
        return;
    }

    // have to perform a switch to the right thread so that an instance of
    // ServiceEventSubscriber can be created into the thread where every other
    // HUpnp object resides. moveToThread() cannot be used, as the
    // accompanying setParent() will fail. This is because Qt cannot send
    // events to the "old" parent living in a different thread.
    HSid sid;
    StatusCode sc;
    emit processSubscription_sig(
        &sreq, service->m_service, &sid, &sc, runner);

    if (runner->wait() == HRunnable::Exiting)
    {
        mi.setKeepAlive(false);
        m_httpHandler.send(mi, InternalServerError);
        return;
    }
    else if (sc != Ok)
    {
        mi.setKeepAlive(false);
        m_httpHandler.send(mi, sc);
        return;
    }

    EventNotifier::ServiceEventSubscriberPtrT rc =
        m_eventNotifier.remoteClient(sid);

    if (!rc)
    {
        // this can happen (although it is *highly* unlikely)
        // if the subscriber immediately unsubscribes and the unsubscription code
        // gets to run to completion before this
        return;
    }

    SubscribeResponse response(
        rc->sid(),
        HSysInfo::instance().herqqProductTokens(),
        rc->timeout());

    m_httpHandler.send(mi, response);

    if (!service->m_service->isEvented() || sreq.isRenewal())
    {
        return;
    }

    // by now the UnicastRemoteClient for the subscriber is created if everything
    // went well and we can attempt to send the initial event message

    // The UDA v1.1 does not specify what to do when a subscription is received
    // to a service that is not evented. A "safe" route was taken here and
    // all subscriptions are accepted rather than returning some error. However,
    // in such a case the timeout is adjusted to a day and no events are ever sent.

    m_eventNotifier.initialNotify(rc, mi);
}

void DeviceHostHttpServer::incomingUnsubscriptionRequest(
    MessagingInfo& mi, const UnsubscribeRequest& usreq, HRunnable* runner)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(runner);

    HLOG_DBG("Unsubscription received.");

    bool ok = false;
    emit removeSubscriber_sig(&usreq, &ok, runner);

    if (runner->wait() == HRunnable::Exiting)
    {
        mi.setKeepAlive(false);
        m_httpHandler.send(mi, InternalServerError);
        return;
    }

    mi.setKeepAlive(false);
    if (ok)
    {
        m_httpHandler.send(mi, Ok);
    }
    else
    {
        m_httpHandler.send(mi, PreconditionFailed);
    }
}

void DeviceHostHttpServer::incomingControlRequest(
    MessagingInfo& mi, const InvokeActionRequest& invokeActionRequest,
    HRunnable* runner)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(runner);

    HLOG_DBG(QString("Control message to [%1] received.").arg(
        invokeActionRequest.soapAction()));

    QUuid udn = extractUdn(invokeActionRequest.serviceUrl());

    HDeviceController* device =
        !udn.isNull() ? m_deviceStorage.searchDeviceByUdn(HUdn(udn)) : 0;

    HServiceController* service = 0;

    if (!device)
    {
        // the request did not have the UDN prefix, which means that either
        // 1) the request was for a ControlURL that was defined as an absolute URL
        //    in the device description or
        // 2) the request is invalid

        service = m_deviceStorage.searchServiceByControlUrl(
            invokeActionRequest.serviceUrl());

        if (!service)
        {
            HLOG_WARN(QString(
                "Ignoring invalid action invocation to: [%1].").arg(
                    invokeActionRequest.serviceUrl().toString()));

            mi.setKeepAlive(false);
            m_httpHandler.send(mi, BadRequest);
            return;
        }
    }
    else if (!service)
    {
        service = m_deviceStorage.searchServiceByControlUrl(
            device, extractRequestExludingUdn(invokeActionRequest.serviceUrl()));
    }

    if (!service)
    {
        HLOG_WARN(QString("Ignoring invalid action invocation to: [%1].").arg(
            invokeActionRequest.serviceUrl().toString()));

        mi.setKeepAlive(false);
        m_httpHandler.send(mi, BadRequest);
        return;
    }

    const QtSoapMessage* soapMsg = invokeActionRequest.soapMsg();
    const QtSoapType& method = soapMsg->method();
    if (!method.isValid())
    {
        HLOG_WARN("Invalid control method.");

        mi.setKeepAlive(false);
        m_httpHandler.send(mi, BadRequest);
        return;
    }

    HActionController* action = service->actionByName(method.name().name());

    if (!action)
    {
        HLOG_WARN(QString("The service has no action named [%1].").arg(
            method.name().name()));

        mi.setKeepAlive(false);
        m_httpHandler.sendActionFailed(
            mi, HAction::InvalidArgs, soapMsg->toXmlString());
        // TODO
        return;
    }

    HActionArguments iargs = action->m_action->info().inputArguments();
    HActionArguments::iterator it = iargs.begin();
    for(; it != iargs.end(); ++it)
    {
        HActionArgument* iarg = (*it);

        const QtSoapType& arg = method[iarg->name()];
        if (!arg.isValid())
        {
            mi.setKeepAlive(false);
            m_httpHandler.sendActionFailed(
                mi, HAction::InvalidArgs, soapMsg->toXmlString());
            // TODO
            return;
        }

        if (!iarg->setValue(
            convertToRightVariantType(arg.value().toString(), iarg->dataType())))
        {
            mi.setKeepAlive(false);
            m_httpHandler.sendActionFailed(
                mi, HAction::InvalidArgs, soapMsg->toXmlString());
            // TODO
            return;
        }
    }

    qint32 retVal = 0;
    HActionArguments outArgs = action->m_action->info().outputArguments();
    if (m_threadingModel == HDeviceHostConfiguration::MultiThreaded)
    {
        retVal = action->invoke(iargs, &outArgs);
    }
    else
    {
        HActionInvocationInfo invocationInfo(
            action, &iargs, &outArgs);

        emit invokeFromMainThread(&invocationInfo, runner);
        if (runner->wait() == HRunnable::Exiting)
        {
            mi.setKeepAlive(false);
            m_httpHandler.send(mi, InternalServerError);
            return;
        }
        retVal = invocationInfo.m_retVal;
    }
    if (retVal != HAction::Success)
    {
        mi.setKeepAlive(false);
        m_httpHandler.sendActionFailed(mi, retVal);
        return;
    }

    QtSoapNamespaces::instance().registerNamespace(
        "u", service->m_service->info().serviceType().toString());

    QtSoapMessage soapResponse;
    soapResponse.setMethod(QtSoapQName(
        QString("%1%2").arg(action->m_action->info().name(), "Response"),
        service->m_service->info().serviceType().toString()));

    foreach(const HActionArgument* oarg, outArgs)
    {
        QtSoapType* soapArg =
            new SoapType(oarg->name(), oarg->dataType(), oarg->value());

        soapResponse.addMethodArgument(soapArg);
    }

    m_httpHandler.send(mi, soapResponse.toXmlString().toUtf8(), Ok);
    HLOG_DBG("Control message successfully handled.");
}

void DeviceHostHttpServer::incomingUnknownHeadRequest(
    MessagingInfo& mi, const QHttpRequestHeader&, HRunnable*)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    // TODO

    mi.setKeepAlive(false);
    m_httpHandler.send(mi, MethotNotAllowed);
}

void DeviceHostHttpServer::incomingUnknownGetRequest(
    MessagingInfo& mi, const QHttpRequestHeader& requestHdr, HRunnable*)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QString peer = peerAsStr(mi.socket());

    QString requestPath = requestHdr.path();

    HLOG_DBG(QString(
        "HTTP GET request received from [%1] to [%2].").arg(peer, requestPath));

    QUuid searchedUdn(requestPath.section('/', 1, 1));
    if (searchedUdn.isNull())
    {
        // the request did not have the UDN prefix, which means that either
        // 1) the request was for a SCPD that was defined with an absolute URL
        //    in the device description or
        // 2) the request is invalid

        HServiceController* service =
            m_deviceStorage.searchServiceByScpdUrl(requestPath);

        if (service)
        {
            HLOG_DBG(QString(
                "Sending service description to [%1] as requested.").arg(peer));

            m_httpHandler.send(
                mi, service->m_service->description().toUtf8(), Ok);

            return;
        }

        HLOG_WARN(QString("Responding NOT_FOUND [%1] to [%2].").arg(
            requestHdr.path(), peerAsStr(mi.socket())));

        m_httpHandler.send(mi, NotFound);
        return;
    }

    HDeviceController* device =
        m_deviceStorage.searchDeviceByUdn(HUdn(searchedUdn));

    if (!device)
    {
        HLOG_WARN(QString("Responding NOT_FOUND [%1] to [%2].").arg(
            requestHdr.path(), peerAsStr(mi.socket())));

        m_httpHandler.send(mi, NotFound);
        return;
    }

    if (requestPath.endsWith(HDevicePrivate::deviceDescriptionPostFix()))
    {
        HLOG_DBG(QString(
            "Sending device description to [%1] as requested.").arg(peer));

        m_httpHandler.send(
            mi, device->m_device->description().toUtf8(), Ok);

        return;
    }

    QString extractedRequestPart = extractRequestExludingUdn(requestPath);

    HServiceController* service =
        m_deviceStorage.searchServiceByScpdUrl(
            device, extractedRequestPart);

    if (service)
    {
        HLOG_DBG(QString(
            "Sending service description to [%1] as requested.").arg(peer));

        m_httpHandler.send(
            mi, service->m_service->description().toUtf8(), Ok);

        return;
    }

    QPair<QUrl, QImage> icon =
        m_deviceStorage.seekIcon(device, extractedRequestPart);

    if (!icon.second.isNull())
    {
        QByteArray ba;
        QBuffer buffer(&ba);
        if (!buffer.open(QIODevice::WriteOnly))
        {
            HLOG_WARN("Failed to serialize the icon.");
            return;
        }

        if (!icon.second.save(&buffer, "png"))
        {
            HLOG_WARN("Failed to serialize the icon.");
            return;
        }

        HLOG_DBG(QString("Sending icon to [%1] as requested.").arg(peer));

        m_httpHandler.send(mi, ba, Ok);
        return;
    }

    HLOG_WARN(QString("Responding NOT_FOUND [%1] to [%2].").arg(
        requestHdr.path(), peerAsStr(mi.socket())));

    m_httpHandler.send(mi, NotFound);
}

void DeviceHostHttpServer::incomingUnknownPostRequest(
    MessagingInfo& mi, const QHttpRequestHeader& /*requestHdr*/,
    const QByteArray& /*body*/, HRunnable*)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    mi.setKeepAlive(false);
    m_httpHandler.send(mi, MethotNotAllowed);
}

}
}
