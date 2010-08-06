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

#include <QList>
#include <QHttpRequestHeader>

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
    HActionInvokeProxyImpl* owner) :
        QObject(),
            m_service(action->parentService()),
            m_actionName(action->info().name()),
            m_inArgs(action->info().inputArguments()),
            m_outArgs(action->info().outputArguments()),
            m_http(new HHttpAsyncHandler(loggingIdentifier, this)),
            m_sock(new QTcpSocket(this)),
            m_loggingIdentifier(loggingIdentifier),
            m_locations(),
            m_iNextLocationToTry(0),
            m_invocationInProgress(0),
            m_messagingInfo(*m_sock, true, 30000),
            m_owner(owner)
{
    Q_ASSERT(m_owner);
    Q_ASSERT(m_service);

    bool ok = verifyName(m_actionName);
    Q_ASSERT(ok); Q_UNUSED(ok)

    ok = connect(this, SIGNAL(invoke_sig()), this, SLOT(invoke_slot()));
    Q_ASSERT(ok);

    ok = connect(m_sock.data(), SIGNAL(connected()), this, SLOT(send()));
    Q_ASSERT(ok);

    ok = connect(
        m_sock.data(), SIGNAL(error(QAbstractSocket::SocketError)),
        this, SLOT(error(QAbstractSocket::SocketError)));
    Q_ASSERT(ok);

    ok = connect(
        m_http.data(), SIGNAL(msgIoComplete(HHttpAsyncOperation*)),
        this, SLOT(msgIoComplete(HHttpAsyncOperation*)));
    Q_ASSERT(ok);

    m_messagingInfo.setAutoDelete(false);
}

HActionProxy::~HActionProxy()
{
}

void HActionProxy::error(QAbstractSocket::SocketError serr)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    if (serr == QAbstractSocket::ConnectionRefusedError ||
        serr == QAbstractSocket::HostNotFoundError)
    {
        HLOG_WARN(QString("Couldn't connect to the device [%1] @ [%2].").arg(
            m_service->parentDevice()->info().udn().toSimpleUuid(),
            m_locations[m_iNextLocationToTry].toString()));

        m_iNextLocationToTry =
            m_iNextLocationToTry == m_locations.size() - 1 ? 0 :
                m_iNextLocationToTry + 1;

        connectToHost();
    }
}

bool HActionProxy::connectToHost()
{
    Q_ASSERT(m_sock.data());

    QTcpSocket::SocketState state = m_sock->state();

    if (state == QTcpSocket::ConnectedState)
    {
        return true;
    }
    else if (state == QTcpSocket::ConnectingState ||
             state == QTcpSocket::HostLookupState)
    {
        return false;
    }

    Q_ASSERT(QThread::currentThread() == m_sock->thread());

    Q_ASSERT(m_iNextLocationToTry < m_locations.size());
    m_sock->connectToHost(
        m_locations[m_iNextLocationToTry].host(),
        m_locations[m_iNextLocationToTry].port());

    return false;
}

void HActionProxy::msgIoComplete(HHttpAsyncOperation* op)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(op);

    op->deleteLater();

    if (op->state() == HHttpAsyncOperation::Failed)
    {
        HLOG_WARN(QString("Action invocation failed: [%1]").arg(
            op->messagingInfo()->lastErrorDescription()));

        invocationDone(HAction::UndefinedFailure);
        return;
    }

    QtSoapMessage response;
    if (!response.setContent(op->dataRead()))
    {
        HLOG_WARN(QString(
            "Received an invalid SOAP message as a response to "
            "action invocation: [%1]").arg(QString::fromUtf8(op->dataRead())));

        invocationDone(HAction::UndefinedFailure);
        return;
    }

    if (response.isFault())
    {
        HLOG_WARN(QString(
            "Action invocation failed: [%1, %2]").arg(
                response.faultString().toString(),
                response.faultDetail().toString()));

        invocationDone(HAction::UndefinedFailure);
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

    QUrl baseUrl = m_locations[m_iNextLocationToTry];
    QUrl controlUrl = resolveUri(baseUrl.path(), m_service->info().controlUrl());

    QHttpRequestHeader actionInvokeRequest("POST", controlUrl.toString());
    actionInvokeRequest.setContentType("text/xml; charset=\"utf-8\"");

    QString soapActionHdrField("\"");
    soapActionHdrField.append(m_service->info().serviceType().toString());
    soapActionHdrField.append("#").append(m_actionName).append("\"");
    actionInvokeRequest.setValue("SOAPACTION", soapActionHdrField);

    m_messagingInfo.setHostInfo(baseUrl);

    HHttpAsyncOperation* op =
        m_http->msgIo(&m_messagingInfo, actionInvokeRequest, soapMsg);

    if (!op)
    {
        invocationDone(HAction::ActionFailed);
    }
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

    if (connectToHost())
    {
        send();
    }
}

void HActionProxy::beginInvoke(HAsyncInvocation* inv)
{
    Q_ASSERT(!m_invocationInProgress);
    m_invocationInProgress = inv;

    emit invoke_sig();
    // this is done to ensure that the invocation is run in the right thread
    // (for performance reasons the same socket is used and in Qt, the sockets
    // have thread affinity)
}

/*******************************************************************************
 * HActionInvokeProxyImpl
 ******************************************************************************/
HActionInvokeProxyImpl::HActionInvokeProxyImpl(
    const QByteArray& loggingIdentifier, HAction* action, QThread* parentThread) :
        m_proxy(new HActionProxy(loggingIdentifier, action, this)),
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
