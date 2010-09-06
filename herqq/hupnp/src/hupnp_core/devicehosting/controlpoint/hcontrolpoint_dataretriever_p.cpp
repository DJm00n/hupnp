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

#include "hcontrolpoint_dataretriever_p.h"

#include "../hdevicehosting_exceptions_p.h"

#include "../../http/hhttp_handler_p.h"
#include "../../http/hhttp_messaginginfo_p.h"

#include "../../../utils/hlogger_p.h"
#include "../../general/hupnp_global_p.h"

#include <QtGui/QImage>

#include <QtCore/QUrl>

#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHttpRequestHeader>
#include <QtNetwork/QHttpResponseHeader>

namespace Herqq
{

namespace Upnp
{

HDataRetriever::HDataRetriever(
    const QByteArray& loggingId, HHttpHandler& http) :
        m_loggingIdentifier(loggingId), m_http(http)
{
}

QByteArray HDataRetriever::retrieveData(
    const QUrl& baseUrl, const QUrl& query, bool processAbsoluteUrl)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QString request;
    if (query.isEmpty())
    {
        request = extractRequestPart(baseUrl);
    }
    else
    {
        request = baseUrl.path();

        QString queryPart = extractRequestPart(query);
        if (processAbsoluteUrl && queryPart.startsWith('/'))
        {
            request = queryPart;
        }
        else if (!queryPart.isEmpty())
        {
            if (!request.endsWith('/'))
            {
                request.append('/');
            }

            if (queryPart.startsWith('/'))
            {
                queryPart.remove(0, 1);
            }
            request.append(queryPart);
        }
    }

    if (request.isEmpty())
    {
        request.append('/');
    }

    QHttpRequestHeader requestHdr("GET", request);
    QHttpResponseHeader responseHdr;

    QTcpSocket sock;
    qint32 port = baseUrl.port();
    sock.connectToHost(baseUrl.host(), port <= 0 ? 80 : port);
    if (!sock.waitForConnected(5000))
    {
        throw HSocketException(
            QString("Could not connect to [%1] in order to retrieve [%2]").arg(
                baseUrl.toString(), request));
    }

    MessagingInfo mi(sock, false, 5000);
    mi.setHostInfo(baseUrl);

    QByteArray body;
    HHttpHandler::ReturnValue rv =
        m_http.msgIO(mi, requestHdr, responseHdr, &body);

    if (rv)
    {
        throw HOperationFailedException(QString(
            "Failed to retrieve data from: [%1] due to: [%2]").arg(
                request, mi.lastErrorDescription()));
    }

    if (!body.size())
    {
        throw HOperationFailedException(
            QString("Did not receive any data for request: [%1]").arg(request));
    }

    return body;
}

QString HDataRetriever::retrieveServiceDescription(
    const QUrl& deviceLocation, const QUrl& scpdUrl)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    HLOG_DBG(QString(
        "Attempting to fetch a service description for [%1] from: [%2]").arg(
            scpdUrl.toString(), deviceLocation.toString()));

    /*QDomDocument dd;
    QString errMsg; qint32 errLine = 0;
    if (!dd.setContent(data, false, &errMsg, &errLine))
    {
        throw HParseException(
            QString("Could not parse the service description: [%1] @ line [%2]").
            arg(errMsg, QString::number(errLine)));
    }*/

    return QString::fromUtf8(retrieveData(deviceLocation, scpdUrl, true));
}

QImage HDataRetriever::retrieveIcon(
    const QUrl& deviceLocation, const QUrl& iconUrl)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    HLOG_DBG(QString(
        "Attempting to retrieve icon [%1] from: [%2]").arg(
            iconUrl.toString(), deviceLocation.toString()));

    QByteArray data = retrieveData(deviceLocation, iconUrl, true);

    QImage image;
    if (!image.loadFromData(data))
    {
        throw HParseException(
            QString("The retrieved data is not a proper icon"));
    }

    return image;
}

QString HDataRetriever::retrieveDeviceDescription(
    const QUrl& deviceLocation)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    HLOG_DBG(QString(
        "Attempting to fetch a device description from: [%1]").arg(
            deviceLocation.toString()));

    return QString::fromUtf8(retrieveData(deviceLocation, QUrl(), false));
}

}
}
