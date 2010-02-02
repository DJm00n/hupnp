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

#ifndef UPNP_ACTIONINVOKE_PROXY_P_H_
#define UPNP_ACTIONINVOKE_PROXY_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../upnp_fwd.h"
#include "../../messaging/http_handler_p.h"
#include "../devicemodel/actionarguments.h"

#include <QUrl>
#include <QString>
#include <QTcpSocket>
#include <QScopedPointer>

class QtSoapMessage;

namespace Herqq
{

namespace Upnp
{

//
//
//
class HActionInvokeProxyConnection :
    public QObject
{
Q_OBJECT

private:

    HService* m_service;
    QString m_actionName;
    HActionInputArguments  m_inArgs;
    HActionOutputArguments m_outArgs;

    QScopedPointer<QTcpSocket> m_sock;
    QUrl m_baseUrl;

    QScopedPointer<HHttpHandler> m_http;

    QThread* m_runnerThread;

private:

    QtSoapMessage msgIO(const QtSoapMessage& soapMsg);
    bool checkConnection();

private slots:

    void invoke_slot(
        qint32* retVal,
        const Herqq::Upnp::HActionInputArguments& inArgs,
        Herqq::Upnp::HActionOutputArguments* outArgs);

Q_SIGNALS:

    void invoke_sig(
        qint32* retVal,
        const Herqq::Upnp::HActionInputArguments& inArgs,
        Herqq::Upnp::HActionOutputArguments* outArgs);

public:

    HActionInvokeProxyConnection(
        QThread* runnerThread,
        HService* service, const QString& actionName,
        const HActionInputArguments& inArgs,
        const HActionOutputArguments& outArgs);

    qint32 invoke(
        const HActionInputArguments& inArgs,
        HActionOutputArguments* outArgs);
};

//
// Class for relaying action invocations across the network to the real
// HAction objects instantiated by device hosts
//
class HActionInvokeProxy
{
private:

    HActionInvokeProxyConnection* m_connection;

public:

    HActionInvokeProxy(
        QThread* runnerThread,
        HService* service, const QString& actionName,
        const HActionInputArguments& inArgs,
        const HActionOutputArguments& outArgs);

    int operator()(
        const HActionInputArguments& inArgs,
        HActionOutputArguments* outArgs);
};

}
}

#endif /* UPNP_ACTIONINVOKE_PROXY_P_H_ */
