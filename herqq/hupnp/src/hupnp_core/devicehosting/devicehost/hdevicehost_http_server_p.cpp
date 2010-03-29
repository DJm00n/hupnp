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

#include "hdevicehost_http_server_p.h"

#include "./../messages/hcontrol_messages_p.h"

#include "./../../dataelements/hudn.h"
#include "./../../general/hupnp_global_p.h"
#include "./../../datatypes/hdatatype_mappings_p.h"

#include "./../../devicemodel/haction_p.h"
#include "./../../devicemodel/hactionarguments.h"

#include "./../../../utils/hlogger_p.h"

#include <QHttpRequestHeader>

namespace Herqq
{

namespace Upnp
{

namespace
{
inline QUuid extractUdn(const QUrl& arg)
{
    QString path = extractRequestPart(arg);

    QUuid udn(path.section('/', 1, 1));
    if (udn.isNull())
    {
        return QUuid();
    }

    return udn;
}

inline QString extractRequestExludingUdn(const QUrl& arg)
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
    const QByteArray& loggingId, DeviceStorage& ds, EventNotifier& en,
    QObject* parent) :
        HHttpServer(loggingId, parent),
            m_deviceStorage(ds), m_eventNotifier(en)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    bool ok = connect(
        this,
        SIGNAL(processSubscription_sig(
            const SubscribeRequest*, HService*, HSid*, StatusCode*)),
        this,
        SLOT(processSubscription_slot(
            const SubscribeRequest*, HService*, HSid*, StatusCode*)),
        Qt::BlockingQueuedConnection);

    Q_ASSERT(ok); Q_UNUSED(ok)

    ok = connect(
        this, SIGNAL(removeSubscriber_sig(const UnsubscribeRequest*, bool*)),
        this, SLOT(removeSubscriber_slot(const UnsubscribeRequest*, bool*)),
        Qt::BlockingQueuedConnection);

    Q_ASSERT(ok);
}

DeviceHostHttpServer::~DeviceHostHttpServer()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
}

void DeviceHostHttpServer::processSubscription_slot(
    const SubscribeRequest* req, HService* service, HSid* sid, StatusCode* sc)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(req);
    Q_ASSERT(sid);
    Q_ASSERT(sc);

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
}

void DeviceHostHttpServer::removeSubscriber_slot(
    const UnsubscribeRequest* req, bool* ok)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(req);
    Q_ASSERT(ok);

    try
    {
        *ok = m_eventNotifier.removeSubscriber(*req);
    }
    catch(HException& ex)
    {
        HLOG_WARN(ex.reason());
    }
}

void DeviceHostHttpServer::incomingSubscriptionRequest(
    MessagingInfo& mi, const SubscribeRequest& sreq)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    HLOG_DBG("Subscription received.");

    QUuid udn = extractUdn(sreq.eventUrl());

    HDeviceController* device =
        !udn.isNull() ? m_deviceStorage.searchDeviceByUdn(udn) : 0;

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
    // UnicastRemoteClient can be created into the thread where every other
    // HUpnp object resides. moveToThread() could be used as well, but the
    // accompanying setParent() will fail, since Qt cannot send
    // events to the "old" parent, because it lives in a different thread.
    HSid sid;
    StatusCode sc;
    emit processSubscription_sig(&sreq, service->m_service, &sid, &sc);
    // this is connected using BlockingQueuedConnection
    // to the local slot that does the processing (in the right thread)

    if (sc != Ok)
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

    SubscribeResponse response(rc->sid(), herqqProductTokens(), rc->timeout());
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
    MessagingInfo& mi, const UnsubscribeRequest& usreq)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    HLOG_DBG("Unsubscription received.");

    bool ok = false;
    emit removeSubscriber_sig(&usreq, &ok);

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
    MessagingInfo& mi, const InvokeActionRequest& invokeActionRequest)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    HLOG_DBG(QString("Control message to [%1] received.").arg(
        invokeActionRequest.soapAction()));

    QUuid udn = extractUdn(invokeActionRequest.serviceUrl());

    HDeviceController* device =
        !udn.isNull() ? m_deviceStorage.searchDeviceByUdn(udn) : 0;

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

    try
    {
        HActionController* action = service->actionByName(method.name().name());

        if (!action)
        {
            HLOG_WARN(QString("The service has no action named [%1].").arg(
                method.name().name()));

            mi.setKeepAlive(false);
            m_httpHandler.sendActionFailed(
                mi, HAction::InvalidArgs(), soapMsg->toXmlString());
            // TODO
            return;
        }

        HActionArguments iargs = action->m_action->inputArguments();
        HActionArguments::iterator it = iargs.begin();
        for(; it != iargs.end(); ++it)
        {
            HActionArgument* iarg = (*it);

            const QtSoapType& arg = method[iarg->name()];
            if (!arg.isValid())
            {
                mi.setKeepAlive(false);
                m_httpHandler.sendActionFailed(
                    mi, HAction::InvalidArgs(), soapMsg->toXmlString());
                // TODO
                return;
            }

            if (!iarg->setValue(
                convertToRightVariantType(arg.value().toString(), iarg->dataType())))
            {
                mi.setKeepAlive(false);
                m_httpHandler.sendActionFailed(
                    mi, HAction::InvalidArgs(), soapMsg->toXmlString());
                // TODO
                return;
            }
        }

        HActionArguments outArgs = action->m_action->outputArguments();
        qint32 retVal = action->invoke(iargs, &outArgs);
        if (retVal != HAction::Success())
        {
            mi.setKeepAlive(false);
            m_httpHandler.sendActionFailed(mi, retVal);
            return;
        }

        QtSoapMessage soapResponse;
        soapResponse.setMethod(QtSoapQName(
            QString("%1%2").arg(action->m_action->name(), "Response"),
            service->m_service->serviceType().toString()));

        foreach(const HActionArgument* oarg, outArgs)
        {
            QtSoapType* soapArg =
                new SoapType(oarg->name(), oarg->dataType(), oarg->value());

            soapResponse.addMethodArgument(soapArg);
        }

        m_httpHandler.send(mi, soapResponse.toXmlString().toUtf8(), Ok);

        HLOG_DBG("Control message successfully handled.");
    }
    catch(HException& ex)
    {
        mi.setKeepAlive(false);
        m_httpHandler.sendActionFailed(mi, 501, ex.reason());
    }
    catch(...)
    {
        mi.setKeepAlive(false);
        m_httpHandler.sendActionFailed(mi, 501);
    }
}

void DeviceHostHttpServer::incomingUnknownHeadRequest(
    MessagingInfo& mi, const QHttpRequestHeader&)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    // TODO

    mi.setKeepAlive(false);
    m_httpHandler.send(mi, MethotNotAllowed);
}

void DeviceHostHttpServer::incomingUnknownGetRequest(
    MessagingInfo& mi, const QHttpRequestHeader& requestHdr)
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
                mi, service->m_service->serviceDescription().toUtf8(), Ok);

            return;
        }

        HLOG_DBG(QString("Responding NOT_FOUND [%1] to [%2].").arg(
            requestHdr.path(), peerAsStr(mi.socket())));

        m_httpHandler.send(mi, NotFound);
        return;
    }

    HDeviceController* device = m_deviceStorage.searchDeviceByUdn(searchedUdn);

    if (!device)
    {
        HLOG_DBG(QString("Responding NOT_FOUND [%1] to [%2].").arg(
            requestHdr.path(), peerAsStr(mi.socket())));

        m_httpHandler.send(mi, NotFound);
        return;
    }

    if (requestPath.endsWith(HDevicePrivate::deviceDescriptionPostFix()))
    {
        HLOG_DBG(QString(
            "Sending device description to [%1] as requested.").arg(peer));

        m_httpHandler.send(
            mi, device->m_device->deviceDescription().toUtf8(), Ok);

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
            mi, service->m_service->serviceDescription().toUtf8(), Ok);

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

    HLOG_DBG(QString("Responding NOT_FOUND [%1] to [%2].").arg(
        requestHdr.path(), peerAsStr(mi.socket())));

    m_httpHandler.send(mi, NotFound);
}

void DeviceHostHttpServer::incomingUnknownPostRequest(
    MessagingInfo& mi, const QHttpRequestHeader& /*requestHdr*/,
    const QByteArray& /*body*/)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    mi.setKeepAlive(false);
    m_httpHandler.send(mi, MethotNotAllowed);
}

}
}
