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

#include "hactioninvoke_proxy_p.h"

#include "../../general/hupnp_global_p.h"
#include "../../datatypes/hdatatype_mappings_p.h"

#include "../../dataelements/hudn.h"
#include "../../dataelements/hactioninfo.h"
#include "../../dataelements/hdeviceinfo.h"
#include "../../dataelements/hserviceinfo.h"

#include "../../devicemodel/haction.h"
#include "../../devicemodel/hdevice.h"
#include "../../devicemodel/hservice.h"

#include "../../../utils/hlogger_p.h"

#include <QtCore/QList>
#include <QtSoapMessage>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HActionProxy
 ******************************************************************************/
HActionProxy::HActionProxy(
    const QByteArray& loggingIdentifier, HAction* action,
    QNetworkAccessManager& nam, HActionInvokeProxyImpl* owner) :
        QObject(),
            m_service(action->parentService()),
            m_actionName(action->info().name()),
            m_inArgs(action->info().inputArguments()),
            m_outArgs(action->info().outputArguments()),
            m_loggingIdentifier(loggingIdentifier),
            m_locations(),
            m_iNextLocationToTry(0),
            m_invocationInProgress(0),
            m_nam(nam),
            m_owner(owner)
{
    Q_ASSERT(m_owner);
    Q_ASSERT(m_service);

    bool ok = verifyName(m_actionName);
    Q_ASSERT(ok); Q_UNUSED(ok)

    ok = connect(this, SIGNAL(invoke_sig()), this, SLOT(invoke_slot()));
    Q_ASSERT(ok);
}

HActionProxy::~HActionProxy()
{
}

void HActionProxy::error(QNetworkReply::NetworkError err)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    if (err == QNetworkReply::ConnectionRefusedError ||
        err == QNetworkReply::HostNotFoundError)
    {
        HLOG_WARN(QString("Couldn't connect to the device [%1] @ [%2].").arg(
            m_service->parentDevice()->info().udn().toSimpleUuid(),
            m_locations[m_iNextLocationToTry].toString()));

        m_iNextLocationToTry =
            m_iNextLocationToTry == m_locations.size() - 1 ? 0 :
                m_iNextLocationToTry + 1;
    }
}

void HActionProxy::finished()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(m_reply);

    m_reply->deleteLater();

    QVariant statusCode = m_reply->attribute(
        QNetworkRequest::HttpStatusCodeAttribute);
    if (statusCode.toInt() != 200)
    {
        HLOG_WARN(QString("Action invocation failed. Server responded: [%1]").arg(
            statusCode.toString()));

        invocationDone(statusCode.toInt());
        return;
    }

    QByteArray data = m_reply->readAll();
    QtSoapMessage response;
    if (!response.setContent(data))
    {
        HLOG_WARN(QString(
            "Received an invalid SOAP message as a response to "
            "action invocation: [%1]").arg(QString::fromUtf8(data)));

        invocationDone(HAction::UndefinedFailure);
        return;
    }

    if (response.isFault())
    {
        HLOG_WARN(QString(
            "Action invocation failed: [%1, %2]").arg(
                response.faultString().toString(),
                response.faultDetail().toString()));

        QtSoapType errCode = response.faultDetail()["errorCode"];
        invocationDone(errCode.isValid() ?
            errCode.value().toInt() : HAction::UndefinedFailure);
        return;
    }

    if (m_outArgs.size() == 0)
    {
        // since there are not supposed to be any out arguments, this is a
        // valid scenario
        invocationDone(HAction::Success);
        return;
    }

    const QtSoapType& root = response.method();
    if (!root.isValid())
    {
        HLOG_WARN(QString(
            "Received an invalid response to action invocation: [%1]").arg(
                response.toXmlString()));

        invocationDone(HAction::UndefinedFailure);
        return;
    }

    HActionArguments::const_iterator ci =
        m_invocationInProgress->m_outArgs.constBegin();

    for(; ci != m_invocationInProgress->m_outArgs.constEnd(); ++ci)
    {
        const HActionArgument* oarg = (*ci);

        const QtSoapType& arg = root[oarg->name()];
        if (!arg.isValid())
        {
            invocationDone(HAction::UndefinedFailure);
            return;
        }

        HActionArgument* userArg =
            m_invocationInProgress->m_outArgs.get(oarg->name());

        userArg->setValue(convertToRightVariantType(
            arg.value().toString(), oarg->dataType()));
    }

    invocationDone(HAction::Success);
}

void HActionProxy::invocationDone(qint32 rc)
{
    m_invocationInProgress->m_invokeId.setReturnValue(rc);
    m_invocationInProgress = 0;
    m_owner->invokeCompleted();
}

void HActionProxy::send()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QtSoapNamespaces::instance().registerNamespace(
        "u", m_service->info().serviceType().toString());

    QtSoapMessage soapMsg;
    soapMsg.setMethod(
        QtSoapQName(m_actionName, m_service->info().serviceType().toString()));

    HActionArguments::const_iterator ci =
        m_invocationInProgress->m_inArgs.constBegin();

    for(; ci != m_invocationInProgress->m_inArgs.constEnd(); ++ci)
    {
        const HActionArgument* const iarg = (*ci);
        if (!m_inArgs.contains(iarg->name()))
        {
            invocationDone(HAction::InvalidArgs);
            return;
        }

        QtSoapType* soapArg =
            new SoapType(iarg->name(), iarg->dataType(), iarg->value());

        soapMsg.addMethodArgument(soapArg);
    }

    QNetworkRequest req;

    req.setHeader(
        QNetworkRequest::ContentTypeHeader,
        QString("text/xml; charset=\"utf-8\""));

    QString soapActionHdrField("\"");
    soapActionHdrField.append(m_service->info().serviceType().toString());
    soapActionHdrField.append("#").append(m_actionName).append("\"");
    req.setRawHeader("SOAPAction", soapActionHdrField.toUtf8());

    QUrl url = resolveUri(
        m_locations[m_iNextLocationToTry], m_service->info().controlUrl());

    req.setUrl(url);

    m_reply = m_nam.post(req, soapMsg.toXmlString().toUtf8());

    bool ok = connect(
        m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
        this, SLOT(error(QNetworkReply::NetworkError)));
    Q_ASSERT(ok);

    ok = connect(m_reply, SIGNAL(finished()), this, SLOT(finished()));
    Q_ASSERT(ok);
}

void HActionProxy::invoke_slot()
{
    if (m_locations.isEmpty())
    {
        // store the device locations only upon action invocation, and only
        // if they haven't been stored yet.
        m_locations = m_service->parentDevice()->locations(HDevice::BaseUrl);
        Q_ASSERT(!m_locations.isEmpty());
        m_iNextLocationToTry = 0;
    }

    send();
}

void HActionProxy::beginInvoke(HAsyncInvocation* inv)
{
    Q_ASSERT(!m_invocationInProgress);
    m_invocationInProgress = inv;

    emit invoke_sig();
    // this is done to ensure that the invocation is run in the right thread
}

/*******************************************************************************
 * HActionInvokeProxyImpl
 ******************************************************************************/
HActionInvokeProxyImpl::HActionInvokeProxyImpl(
    const QByteArray& loggingIdentifier, HAction* action,
    QNetworkAccessManager& nam, QThread* parentThread) :
        m_proxy(new HActionProxy(loggingIdentifier, action, nam, this)),
        m_invocations(), m_invocationsMutex()
{
    Q_ASSERT(parentThread);
    m_proxy->moveToThread(parentThread);
}

HActionInvokeProxyImpl::~HActionInvokeProxyImpl()
{
    m_proxy->deleteLater();
}

void HActionInvokeProxyImpl::invokeCompleted()
{
    QMutexLocker locker(&m_invocationsMutex);
    m_callback(m_invocations.dequeue()->m_invokeId);
    if (m_invocations.size())
    {
        m_proxy->beginInvoke(m_invocations.head());
    }
}

bool HActionInvokeProxyImpl::beginInvoke(HAsyncInvocation* arg)
{
    QMutexLocker locker(&m_invocationsMutex);
    m_invocations.enqueue(arg);
    if (m_invocations.size() <= 1)
    {
        m_proxy->beginInvoke(arg);
    }

    return true;
}

}
}
