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

#include "actioninvoke_proxy_p.h"

#include "../../upnp_global_p.h"
#include "../../datatype_mappings_p.h"

#include "../../dataelements/udn.h"
#include "../../dataelements/deviceinfo.h"

#include "../../devicemodel/action.h"
#include "../../devicemodel/device.h"
#include "../../devicemodel/service.h"

#include "../../../../utils/src/logger_p.h"
#include "../../../../core/include/HExceptions"

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
    QThread* runnerThread,
    HService* service, const QString& actionName,
    const HActionInputArguments& inArgs,
    const HActionOutputArguments& outArgs) :
        m_service(service),
        m_actionName(actionName),
        m_inArgs(inArgs),
        m_outArgs(outArgs),
        m_sock(new QTcpSocket(this)),
        m_baseUrl(),
        m_http(new HHttpHandler()),
        m_runnerThread(runnerThread)
{
    Q_ASSERT(service);

    verifyName(actionName);

    bool ok = connect(
        this,
        SIGNAL(invoke_sig(qint32*,Herqq::Upnp::HActionInputArguments,Herqq::Upnp::HActionOutputArguments*)),
        this,
        SLOT(invoke_slot(qint32*,Herqq::Upnp::HActionInputArguments,Herqq::Upnp::HActionOutputArguments*)),
        Qt::BlockingQueuedConnection);

    Q_ASSERT(ok); Q_UNUSED(ok)
}

bool HActionInvokeProxyConnection::checkConnection()
{
    HLOG(H_AT, H_FUN);

    if (m_sock->state() == QTcpSocket::ConnectedState)
    {
        return true;
    }

    QList<QUrl> locations = m_service->parentDevice()->locations(false);
    foreach(QUrl location, locations)
    {
        m_sock->connectToHost(location.host(), location.port());
        if (m_sock->waitForConnected(1000))
        {
            m_baseUrl = location;
            return true;
        }
    }

    m_baseUrl = QUrl();
    HLOG_WARN(QObject::tr("Couldn't connect to the device [%1]").arg(
        m_service->parentDevice()->deviceInfo().udn().toSimpleUuid()));

    return false;
}

QtSoapMessage HActionInvokeProxyConnection::msgIO(const QtSoapMessage& soapMsg)
{
    HLOG(H_AT, H_FUN);

    if (!checkConnection())
    {
        Q_ASSERT(false);
        return QtSoapMessage(); // TODO
    }

    Q_ASSERT(m_service);
    QUrl controlUrl = appendUrls(m_baseUrl.path(), m_service->controlUrl());

    QHttpRequestHeader actionInvokeRequest("POST", controlUrl.toString());
    actionInvokeRequest.setContentType("text/xml; charset=\"utf-8\"");

    QString soapActionHdrField("\"");
    soapActionHdrField.append(m_service->serviceType().toString());
    soapActionHdrField.append("#").append(m_actionName).append("\"");
    actionInvokeRequest.setValue("SOAPACTION", soapActionHdrField);

    MessagingInfo mi(*m_sock, true, 30000);
    mi.setHostInfo(m_baseUrl);
    return m_http->msgIO(mi, actionInvokeRequest, soapMsg);
}

void HActionInvokeProxyConnection::invoke_slot(
    qint32* rc, const HActionInputArguments& inArgs, HActionOutputArguments* outArgs)
{
    HLOG(H_AT, H_FUN);

    // 1) create the remote method call request
    QtSoapMessage soapMsg;
    soapMsg.setMethod(
        QtSoapQName(m_actionName, m_service->serviceType().toString()));

    HActionInputArguments::const_iterator ci = inArgs.constBegin();
    for(; ci != inArgs.constEnd(); ++ci)
    {
        const HActionInputArgument* const iarg = (*ci);
        if (!m_inArgs.contains(iarg->name()))
        {
            *rc = HAction::InvalidArgs();
            return;
        }

        QtSoapType* soapArg =
            new SoapType(iarg->name(), iarg->dataType(), iarg->value());

        soapMsg.addMethodArgument(soapArg);
    }

    // 2) send it and attempt to get a response
    QtSoapMessage response;
    try
    {
        response = msgIO(soapMsg);
        if (response.isFault())
        {
            *rc = HAction::UndefinedFailure();
            return;
        }
    }
    catch(HException& ex)
    {
        HLOG_WARN(ex.reason());
        *rc = HAction::UndefinedFailure();
        return;
    }

    if (m_outArgs.size() == 0)
    {
        // since there are not supposed to be any out arguments, this is a
        // valid scenario
        *rc = HAction::Success();
        return;
    }

    // 3) parse and verify the response

    const QtSoapType& root = response.method();
    if (!root.isValid())
    {
        *rc = HAction::UndefinedFailure();
        return;
    }

    foreach(HActionOutputArgument* oarg, m_outArgs)
    {
        const QtSoapType& arg = root[oarg->name()];
        if (!arg.isValid())
        {
            *rc = HAction::UndefinedFailure();
            return;
        }

        outArgs->get(oarg->name())->setValue(
            convertToRightVariantType(arg.value().toString(), oarg->dataType()));
    }

    *rc = HAction::Success();
}

qint32 HActionInvokeProxyConnection::invoke(
    const HActionInputArguments& inArgs,
    HActionOutputArguments* outArgs)
{
    qint32 rc = 0;
    emit invoke_sig(&rc, inArgs, outArgs);
    return rc;
}

/*******************************************************************************
 * HActionInvokeProxy
 ******************************************************************************/
HActionInvokeProxy::HActionInvokeProxy(
    QThread* runnerThread,
    HService* service,
    const QString& actionName,
    const HActionInputArguments& inArgs,
    const HActionOutputArguments& outArgs) :
        m_connection(
            new HActionInvokeProxyConnection(
                runnerThread, service, actionName, inArgs, outArgs))
{
    m_connection->moveToThread(runnerThread);

    bool ok = QObject::connect(
        runnerThread,
        SIGNAL(finished()),
        m_connection,
        SLOT(deleteLater()));

    Q_ASSERT(ok); Q_UNUSED(ok)
}

int HActionInvokeProxy::operator()(
    const HActionInputArguments& inArgs,
    HActionOutputArguments* outArgs)
{
    return m_connection->invoke(inArgs, outArgs);
}

}
}
