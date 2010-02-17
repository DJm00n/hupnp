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

#include "hcontrolpoint_dataretriever_p.h"

#include "./../hdevicehosting_exceptions_p.h"

#include "./../../http/hhttp_handler_p.h"
#include "./../../http/hhttp_messaginginfo_p.h"

#include "./../../../utils/hlogger_p.h"

#include <QUrl>
#include <QImage>
#include <QTcpSocket>
#include <QDomDocument>
#include <QHttpRequestHeader>
#include <QHttpResponseHeader>

namespace Herqq
{

namespace Upnp
{

DataRetriever::DataRetriever(
    const QByteArray& loggingId, HHttpHandler& http) :
        m_loggingIdentifier(loggingId), m_http(http)
{
}

QByteArray DataRetriever::retrieveData(const QUrl& baseUrl, const QUrl& query)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QString queryPart =
        query.toString(
            QUrl::RemoveAuthority | QUrl::RemovePassword | QUrl::RemoveUserInfo |
            QUrl::RemoveScheme | QUrl::RemovePort | QUrl::StripTrailingSlash);

    QString request(baseUrl.path());

    if (!queryPart.isEmpty())
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

    QHttpRequestHeader requestHdr("GET", request);
    QHttpResponseHeader responseHdr;

    QTcpSocket sock;
    sock.connectToHost(baseUrl.host(), baseUrl.port());
    if (!sock.waitForConnected(5000))
    {
        throw HSocketException(
            QObject::tr("Could not connect to [%1] in order to retrieve [%2]").arg(
                baseUrl.toString(), request));
    }

    MessagingInfo mi(sock, false, 5000);
    mi.setHostInfo(baseUrl);

    try
    {
        QByteArray body = m_http.msgIO(mi, requestHdr, responseHdr);
        if (!body.size())
        {
            throw HOperationFailedException(
                QObject::tr("Received no data for request: [%1]").arg(request));
        }

        return body;
    }
    catch(HException& ex)
    {
        throw HOperationFailedException(QObject::tr(
            "Failed to retrieve data from: [%1] due to: [%2]").arg(
                request, ex.reason()));
    }
}

QDomDocument DataRetriever::retrieveServiceDescription(
    const QUrl& deviceLocation, const QUrl& scpdUrl)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    HLOG_DBG(QObject::tr(
        "Attempting to fetch a service description for [%1] from: [%2]").arg(
            scpdUrl.toString(), deviceLocation.toString()));

    QByteArray data = retrieveData(deviceLocation, scpdUrl);

    QDomDocument dd;
    QString errMsg; qint32 errLine = 0;
    if (!dd.setContent(data, false, &errMsg, &errLine))
    {
        throw HParseException(
            QObject::tr("Could not parse the service description: [%1] @ line [%2]").
            arg(errMsg, QString::number(errLine)));
    }

    return dd;
}

QImage DataRetriever::retrieveIcon(const QUrl& deviceLocation, const QUrl& iconUrl)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    HLOG_DBG(QObject::tr(
        "Attempting to retrieve icon [%1] from: [%2]").arg(
            iconUrl.toString(), deviceLocation.toString()));

    QByteArray data = retrieveData(deviceLocation, iconUrl);

    QImage image;
    if (!image.loadFromData(data))
    {
        throw HParseException(
            QObject::tr("The retrieved data is not a proper icon"));
    }

    return image;
}

QDomDocument DataRetriever::retrieveDeviceDescription(QUrl deviceLocation)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    HLOG_DBG(QObject::tr(
        "Attempting to fetch a device description from: [%1]").arg(
            deviceLocation.toString()));

    QByteArray data = retrieveData(deviceLocation, QUrl());

    QDomDocument dd;
    QString errMsg; qint32 errLine = 0;
    if (!dd.setContent(data, false, &errMsg, &errLine))
    {
        throw InvalidDeviceDescription(
            QObject::tr("Could not parse the device description file: [%1] @ line [%2]:\n[%3]").
            arg(errMsg, QString::number(errLine), QString::fromUtf8(data)));
    }

    return dd;
}

}
}
