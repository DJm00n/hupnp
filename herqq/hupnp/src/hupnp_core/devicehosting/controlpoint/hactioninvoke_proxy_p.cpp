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

#include "hactioninvoke_proxy_p.h"

#include "./../../general/hupnp_global_p.h"
#include "./../../datatypes/hdatatype_mappings_p.h"

#include "./../../dataelements/hudn.h"
#include "./../../dataelements/hdeviceinfo.h"

#include "./../../devicemodel/haction.h"
#include "./../../devicemodel/hdevice.h"
#include "./../../devicemodel/hservice.h"

#include "./../../../utils/hlogger_p.h"
#include "./../../../utils/hexceptions_p.h"

#include <QList>
#include <QTcpSocket>
#include <QHttpRequestHeader>

#include <QtSoapMessage>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HActionInvokeProxyConnection
 ******************************************************************************/
HActionInvokeProxyConnection::HActionInvokeProxyConnection(
    const QByteArray& loggingIdentifier, HAction* action) :
        QObject(action),
            m_service(action->parentService()),
            m_actionName(action->name()),
            m_inArgs(action->inputArguments()),
            m_outArgs(action->outputArguments()),
            m_sock(new QTcpSocket(this)),
            m_http(new HHttpAsyncHandler(loggingIdentifier, this)),
            m_loggingIdentifier(loggingIdentifier),
            m_locations(),
            m_iNextLocationToTry(0),
            m_invokeWaitMutex(),
            m_invokeWait(),
            m_invocationMutex(),
            m_invocationInProgress(0),
            m_messagingInfo(*m_sock, true, 30000)
{
    Q_ASSERT(m_service);

    verifyName(m_actionName);

    bool ok = connect(
        this,
        SIGNAL(invoke_sig(Invocation*)),
        this,
        SLOT(invoke_slot(Invocation*)));

    Q_ASSERT(ok); Q_UNUSED(ok)

    ok = connect(
        m_sock.data(), SIGNAL(connected()), this, SLOT(send()));

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

HActionInvokeProxyConnection::~HActionInvokeProxyConnection()
{
}

void HActionInvokeProxyConnection::error(
    QAbstractSocket::SocketError serr)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    if (serr == QAbstractSocket::ConnectionRefusedError ||
        serr ==  QAbstractSocket::HostNotFoundError)
    {
        HLOG_WARN(QObject::tr("Couldn't connect to the device [%1] @ [%2].").arg(
        m_service->parentDevice()->deviceInfo().udn().toSimpleUuid(),
        m_locations[m_iNextLocationToTry].toString()));

        m_iNextLocationToTry =
            m_iNextLocationToTry == m_locations.size() - 1 ? 0 :
                m_iNextLocationToTry + 1;

        connectToHost();
        return;
    }

    if (m_invocationInProgress)
    {
        invocationDone(HAction::ActionFailed());
    }
}

bool HActionInvokeProxyConnection::connectToHost()
{
    Q_ASSERT(m_sock.data());

    QTcpSocket::SocketState state = m_sock->state();

    if (state == QTcpSocket::ConnectedState)
    {
        return true;
    }
    else if (state == QTcpSocket::ConnectingState)
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

void HActionInvokeProxyConnection::msgIoComplete(HHttpAsyncOperation* op)
{
    op->deleteLater();

    QtSoapMessage response;
    if (!response.setContent(op->dataRead()))
    {
        invocationDone(HAction::UndefinedFailure());
        return;
    }

    if (response.isFault())
    {
        invocationDone(HAction::UndefinedFailure());
        return;
    }

    if (m_outArgs.size() == 0)
    {
        // since there are not supposed to be any out arguments, this is a
        // valid scenario
        invocationDone(HAction::Success());
        return;
    }

    // 3) parse and verify the response

    const QtSoapType& root = response.method();
    if (!root.isValid())
    {
        invocationDone(HAction::UndefinedFailure());
        return;
    }

    HActionArguments::const_iterator ci =
        m_invocationInProgress->m_outArgs->constBegin();

    for(; ci != m_invocationInProgress->m_outArgs->constEnd(); ++ci)
    {
        const HActionArgument* oarg = (*ci);

        const QtSoapType& arg = root[oarg->name()];
        if (!arg.isValid())
        {
            invocationDone(HAction::UndefinedFailure());
        }

        m_invocationInProgress->m_outArgs->get(
            oarg->name())->setValue(
                convertToRightVariantType(
                    arg.value().toString(), oarg->dataType()));
    }

    invocationDone(HAction::Success());
}

void HActionInvokeProxyConnection::invocationDone(qint32 rc)
{
    QMutexLocker lock(&m_invokeWaitMutex);
    *m_invocationInProgress->m_rc  = rc;
    m_invocationInProgress->m_done = true;
    m_invocationInProgress = 0;
    m_invokeWait.wakeAll();
}

void HActionInvokeProxyConnection::send()
{
    // 1) create the remote method call request
    QtSoapMessage soapMsg;
    soapMsg.setMethod(
        QtSoapQName(m_actionName, m_service->serviceType().toString()));

    HActionArguments::const_iterator ci =
        m_invocationInProgress->m_inArgs.constBegin();

    for(; ci != m_invocationInProgress->m_inArgs.constEnd(); ++ci)
    {
        const HActionArgument* const iarg = (*ci);
        if (!m_inArgs.contains(iarg->name()))
        {
            invocationDone(HAction::InvalidArgs());
            return;
        }

        QtSoapType* soapArg =
            new SoapType(iarg->name(), iarg->dataType(), iarg->value());

        soapMsg.addMethodArgument(soapArg);
    }

    // 2) send it and attempt to get a response
    QUrl baseUrl = m_locations[m_iNextLocationToTry];

    QUrl controlUrl = appendUrls(baseUrl.path(), m_service->controlUrl());

    QHttpRequestHeader actionInvokeRequest("POST", controlUrl.toString());
    actionInvokeRequest.setContentType("text/xml; charset=\"utf-8\"");

    QString soapActionHdrField("\"");
    soapActionHdrField.append(m_service->serviceType().toString());
    soapActionHdrField.append("#").append(m_actionName).append("\"");
    actionInvokeRequest.setValue("SOAPACTION", soapActionHdrField);

    m_messagingInfo.setHostInfo(baseUrl);

    HHttpAsyncOperation* op =
        m_http->msgIo(&m_messagingInfo, actionInvokeRequest, soapMsg);

    if (!op)
    {
        invocationDone(HAction::ActionFailed());
    }
}

void HActionInvokeProxyConnection::invoke_slot(Invocation* invocation)
{
    if (m_locations.isEmpty())
    {
        // store the device locations only upon action invocation, and only
        // if they haven't been stored yet.
        m_locations = m_service->parentDevice()->locations(false);
        Q_ASSERT(!m_locations.isEmpty());
        m_iNextLocationToTry = 0;
    }

    m_invocationInProgress = invocation;

    if (connectToHost())
    {
        send();
    }
}

qint32 HActionInvokeProxyConnection::invoke(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    QMutexLocker lock(&m_invocationMutex);
    QMutexLocker lock2(&m_invokeWaitMutex);

    qint32 rc = 0;
    Invocation invocation(&rc, inArgs, outArgs);

    emit invoke_sig(&invocation);
    // this is done to ensure that the invocation is run in the right thread
    // (for performance reasons the same socket is used and in Qt, the sockets
    // have thread affinity)

    while(!invocation.m_done)
    {
        m_invokeWait.wait(&m_invokeWaitMutex);
    }

    return rc;
}

/*******************************************************************************
 * HActionInvokeProxy
 ******************************************************************************/
HActionInvokeProxy::HActionInvokeProxy(
    const QByteArray& loggingIdentifier, HAction* action) :
        m_connection(
            new HActionInvokeProxyConnection(loggingIdentifier, action))
{
}

int HActionInvokeProxy::operator()(
    const HActionArguments& inArgs,
    HActionArguments* outArgs)
{
    return m_connection->invoke(inArgs, outArgs);
}

}
}
