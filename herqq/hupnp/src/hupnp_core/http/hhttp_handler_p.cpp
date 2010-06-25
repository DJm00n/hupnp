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

#include "hhttp_handler_p.h"
#include "hhttp_messaginginfo_p.h"
#include "hhttp_messagecreator_p.h"
#include "hhttp_utils_p.h"

#include "../general/hupnp_global_p.h"
#include "../devicehosting/messages/hevent_messages_p.h"

#include "../../utils/hlogger_p.h"

#include <QTcpSocket>
#include <QHostAddress>
#include <QHttpRequestHeader>
#include <QHttpResponseHeader>

#include <QtSoapMessage>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HHttpHandler
 ******************************************************************************/
HHttpHandler::HHttpHandler(const QByteArray& loggingIdentifier) :
    m_loggingIdentifier(loggingIdentifier), m_shuttingDown(0)
{
}

HHttpHandler::~HHttpHandler()
{
}

void HHttpHandler::shutdown()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    m_shuttingDown = 1;
}

HHttpHandler::ReturnValue HHttpHandler::readChunkedRequest(
    MessagingInfo& mi, QByteArray* data)
{
    Q_ASSERT(data);

    QTime stopWatch; stopWatch.start();
    for(; stopWatch.elapsed() < 15000; )
    {
        // every chunk begins with a size line that ends to a mandatory CRLF

        if (mi.socket().bytesAvailable() <= 0)
        {
            if (!mi.socket().waitForReadyRead(500))
            {
                continue;
            }
        }

        QByteArray buf;

        char readChar = 0;
        qint32 linesRead = 0;
        while(linesRead < 1)
        {
            if (!mi.socket().getChar(&readChar))
            {
                // Could not read size line. It should be available at this point.
                mi.setLastErrorDescription("Could not read chunk-size line.");

                return InvalidData;
            }

            buf.push_back(readChar);

            if (readChar != '\r')
            {
                if (linesRead > 0) { linesRead = 0; }
                continue;
            }

            if (mi.socket().getChar(&readChar))
            {
                buf.push_back(readChar);

                if (readChar == '\n')
                {
                    ++linesRead;
                }
                else if (linesRead > 0)
                {
                    linesRead = 0;
                }
            }
            else
            {
                break;
            }
        }

        if (linesRead != 1)
        {
            // No size line. It should be available at this point.
            mi.setLastErrorDescription("No chunk-size line in the message body.");

            return InvalidData;
        }

        qint32 endOfSize = buf.indexOf(';');
        if (endOfSize < 0)
        {
            // no extensions
            endOfSize = buf.size() - 2; // 2 == crlf
        }
        QByteArray sizeLine =  buf.left(endOfSize);

        bool ok = false;
        qint32 chunkSize = sizeLine.toInt(&ok, 16);
        if (!ok || chunkSize < 0)
        {
            mi.setLastErrorDescription(
                QString("Invalid chunk-size line: %1.").arg(
                      QString::fromUtf8(sizeLine)));

            return InvalidData;
        }

        if (chunkSize == 0)
        {
            // the last chunk, ignore possible trailers
            break;
        }

        buf.clear();
        while (chunkSize > buf.size())
        {
            // the chunk is larger than what is currently read for the next chunk.
            // attempt to read more

            bool dataAvailable = mi.socket().bytesAvailable() ||
                                 mi.socket().waitForReadyRead(50);

            if (m_shuttingDown && (!dataAvailable || stopWatch.elapsed() > 500))
            {
                mi.setLastErrorDescription(
                    "Shutting down. Aborting HTTP message body read.");

                return ShuttingDown;
            }
            else if (!dataAvailable &&
                     mi.socket().state() != QTcpSocket::ConnectedState &&
                     mi.socket().state() != QTcpSocket::ClosingState)
            {
                mi.setLastErrorDescription(
                    "Peer has disconnected. Could not read HTTP message body.");

                return PeerDisconnected;
            }
            else if (stopWatch.elapsed() >= mi.receiveTimeoutForNoData() &&
                     mi.receiveTimeoutForNoData() >= 0)
            {
                mi.setLastErrorDescription(QString(
                    "Timeout [%1] has elapsed. Could not read chunked "
                    "HTTP message body.").arg(
                        QString::number(mi.receiveTimeoutForNoData())));

                return Timeout;
            }
            else if (!dataAvailable)
            {
                continue;
            }

            QByteArray tmp; tmp.resize(chunkSize - buf.size());
            qint32 read = mi.socket().read(tmp.data(), chunkSize - buf.size());

            if (read < 0)
            {
                mi.setLastErrorDescription(QString(
                    "Failed to read chunk: %1").arg(mi.socket().errorString()));

                return GenericSocketError;
            }
            else if (read == 0)
            {
                continue;
            }

            tmp.resize(read);
            buf.append(tmp);
        }

        // append the chunk to the return value and
        data->append(buf);

        char c;
        mi.socket().getChar(&c);
        mi.socket().getChar(&c);
        // remove the mandatory CRLF trailing the data

        stopWatch.restart();
    }

    return stopWatch.elapsed() < 15000 ? Success : Timeout;
}

HHttpHandler::ReturnValue HHttpHandler::readRequestData(
    MessagingInfo& mi, QByteArray* requestData, qint64 contentLength)
{
    if (contentLength <= 0)
    {
        return Success;
    }

    Q_ASSERT(requestData);

    qint64 bytesRead = 0;
    QByteArray buf; buf.resize(4096);

    QTime stopWatch; stopWatch.start();
    while (bytesRead < contentLength)
    {
        bool dataAvailable = mi.socket().bytesAvailable() ||
                             mi.socket().waitForReadyRead(50);

        if (m_shuttingDown && (!dataAvailable || stopWatch.elapsed() > 500))
        {
            mi.setLastErrorDescription(
                "Shutting down. Aborting HTTP message body read.");

            return ShuttingDown;
        }
        else if (!dataAvailable &&
                 mi.socket().state() != QTcpSocket::ConnectedState &&
                 mi.socket().state() != QTcpSocket::ClosingState)
        {
            mi.setLastErrorDescription(
                "Peer has disconnected. Could not read HTTP message body.");

            return PeerDisconnected;
        }
        else if (stopWatch.elapsed() >= mi.receiveTimeoutForNoData() &&
                 mi.receiveTimeoutForNoData() >= 0)
        {
            mi.setLastErrorDescription(QString(
                "Timeout [%1] has elapsed. Could not read HTTP message body.").arg(
                    QString::number(mi.receiveTimeoutForNoData())));

            return Timeout;
        }
        else if (!dataAvailable)
        {
            continue;
        }

        do
        {
            qint64 retVal = mi.socket().read(
                buf.data(),
                qMin(static_cast<qint64>(buf.size()), contentLength - bytesRead));

            if (retVal < 0)
            {
                mi.setLastErrorDescription(
                    QString("Could not read HTTP message body: .").arg(
                        mi.socket().errorString()));

                return GenericSocketError;
            }
            else if (retVal > 0)
            {
                bytesRead += retVal;
                requestData->append(QByteArray(buf.data(), retVal));
            }
            else
            {
                break;
            }
        }
        while(bytesRead < contentLength && !m_shuttingDown);

        if (!m_shuttingDown)
        {
            stopWatch.restart();
        }
    }

    return Success;
}

template<typename Header>
HHttpHandler::ReturnValue HHttpHandler::receive(
    MessagingInfo& mi, Header& hdr, QByteArray* body)
{
    QByteArray headerData;
    QTime stopWatch; stopWatch.start();
    for(;;)
    {
        bool dataAvailable = mi.socket().bytesAvailable() ||
                             mi.socket().waitForReadyRead(50);

        if (m_shuttingDown && (!dataAvailable || stopWatch.elapsed() > 500))
        {
            mi.setLastErrorDescription(QString(
                "Shutting down. Aborting HTTP message header read."));

            return ShuttingDown;
        }
        else if (!dataAvailable &&
                 mi.socket().state() != QTcpSocket::ConnectedState &&
                 mi.socket().state() != QTcpSocket::ClosingState)
        {
            mi.setLastErrorDescription(QString(
                "Peer has disconnected. Could not read HTTP message header."));

            return PeerDisconnected;
        }
        else if (stopWatch.elapsed() >= mi.receiveTimeoutForNoData() &&
                 mi.receiveTimeoutForNoData() >= 0)
        {
            mi.setLastErrorDescription(QString(
                "Timeout [%1] has elapsed. Could not read HTTP message header.").arg(
                    QString::number(mi.receiveTimeoutForNoData())));

            return Timeout;
        }
        else if (!dataAvailable)
        {
            continue;
        }

        char readChar = 0;
        qint32 linesRead = 0;
        while(linesRead < 2 && mi.socket().getChar(&readChar))
        {
            headerData.push_back(readChar);

            if (readChar != '\r')
            {
                if (linesRead > 0) { linesRead = 0; }
                continue;
            }

            if (mi.socket().getChar(&readChar))
            {
                headerData.push_back(readChar);

                if (readChar == '\n')
                {
                    ++linesRead;
                }
                else if (linesRead > 0)
                {
                    linesRead = 0;
                }
            }
        }

        // it is here assumed that \r\n\r\n is always readable on one pass.
        // if that cannot be done, any combination of \r's and \n's
        // is treated as part of the data. For instance, if \r\n\r is read,
        // it is considered to be part of data and thus when the next iteration
        // starts, \n isn't expected to complete the end of the HTTP header mark.

        if (linesRead == 2)
        {
            break;
        }
    }

    hdr = Header(QString::fromUtf8(headerData));
    if (!hdr.isValid())
    {
        return InvalidHeader;
    }

    if (body)
    {
        bool chunked = hdr.value("TRANSFER-ENCODING") == "chunked";
        if (chunked)
        {
            if (hdr.hasContentLength())
            {
                hdr = Header();
                return InvalidHeader;
            }

            readChunkedRequest(mi, body);
        }
        else
        {
            if (hdr.hasContentLength())
            {
                quint32 clength = hdr.contentLength();
                readRequestData(mi, body, clength);
            }
            else
            {
                *body = mi.socket().readAll();
            }
        }
    }

    mi.setKeepAlive(HHttpUtils::keepAlive(hdr));

    return Success;
}

HHttpHandler::ReturnValue HHttpHandler::sendBlob(
    MessagingInfo& mi, const QByteArray& data)
{
    Q_ASSERT(!data.isEmpty());
    QHostAddress peer = mi.socket().peerAddress();

    qint64 bytesWritten   = 0, index = 0;
    qint32 errorThreshold = 0;
    while(index < data.size())
    {
        if (mi.socket().state() != QTcpSocket::ConnectedState)
        {
            mi.setLastErrorDescription(QString(
                "Failed to send data to %1. Connection closed.").arg(
                    peer.toString()));

            return SocketClosed;
        }

        bytesWritten = mi.socket().write(data.data() + index, data.size() - index);

        if (bytesWritten == 0)
        {
            if (!mi.socket().isValid() || errorThreshold > 100)
            {
                mi.setLastErrorDescription(
                    QString("Failed to send data to %1.").arg(
                        peer.toString()));

                return GenericSocketError;
            }

            ++errorThreshold;
        }
        else if (bytesWritten < 0)
        {
            mi.setLastErrorDescription(
                QString("Failed to send data to %1.").arg(
                    peer.toString()));

            return GenericSocketError;
        }

        index += bytesWritten;
    }

    for(qint32 i = 0; i < 250 && mi.socket().flush(); ++i)
    {
        mi.socket().waitForBytesWritten(1);
    }

    /*if (!mi.keepAlive() && mi.socket().state() == QTcpSocket::ConnectedState)
    {
        mi.socket().disconnectFromHost();
    }*/

    return Success;
}

HHttpHandler::ReturnValue HHttpHandler::sendChunked(
    MessagingInfo& mi, const QByteArray& data)
{
    Q_ASSERT(!data.isEmpty());
    Q_ASSERT(mi.chunkedInfo().m_maxChunkSize > 0);

    QHostAddress peer = mi.socket().peerAddress();

    const char crlf[] = {"\r\n"};

    // send the http header first
    qint32 endOfHdr = data.indexOf("\r\n\r\n") + 4;
    sendBlob(mi, data.left(endOfHdr));

    // then start sending the data in chunks
    qint64 bytesWritten   = 0;
    qint32 errorThreshold = 0, index = endOfHdr;
    while(index < data.size())
    {
        if (mi.socket().state() != QTcpSocket::ConnectedState)
        {
            mi.setLastErrorDescription(QString(
                "Failed to send data to %1. Connection closed.").arg(
                    peer.toString()));

            return SocketClosed;
        }

        qint32 dataToSendSize =
            qMin(data.size() - index,
                 static_cast<qint32>(mi.chunkedInfo().m_maxChunkSize));

        // write the size line
        QByteArray sizeLine;
        sizeLine.setNum(dataToSendSize, 16);
        sizeLine.append(crlf, 2);

        bytesWritten = mi.socket().write(sizeLine);
        if (bytesWritten != sizeLine.size())
        {
            mi.setLastErrorDescription(QString(
                "Failed to send data to %1.").arg(peer.toString()));

            return GenericSocketError;
        }

        while(errorThreshold < 100)
        {
            // write the chunk
            bytesWritten =
                mi.socket().write(data.data() + index, dataToSendSize);

            if (bytesWritten == 0)
            {
                if (!mi.socket().isValid() || errorThreshold > 100)
                {
                    mi.setLastErrorDescription(
                        QString("Failed to send data to %1.").arg(
                            peer.toString()));

                    return GenericSocketError;
                }

                ++errorThreshold;
            }
            else if (bytesWritten < 0)
            {
                mi.setLastErrorDescription(
                    QString("Failed to send data to %1.").arg(
                        peer.toString()));

                return GenericSocketError;
            }
            else
            {
                break;
            }
        }

        index += bytesWritten;

        // and after the chunk, write the trailing crlf and start again if there's
        // chunks left
        bytesWritten = mi.socket().write(crlf, 2);
        if (bytesWritten != 2)
        {
            mi.setLastErrorDescription(
                QString("Failed to send data to %1.").arg(peer.toString()));

            return GenericSocketError;
        }

        mi.socket().flush();
    }

    // write the "eof" == zero + crlf
    const char eof[] = "0\r\n";
    mi.socket().write(&eof[0], 3);

    for(qint32 i = 0; i < 250 && mi.socket().flush(); ++i)
    {
        mi.socket().waitForBytesWritten(1);
    }

    /*if (!mi.keepAlive() && mi.socket().state() == QTcpSocket::ConnectedState)
    {
        mi.socket().disconnectFromHost();
    }*/

    return Success;
}

HHttpHandler::ReturnValue HHttpHandler::send(
    MessagingInfo& mi, const QByteArray& data)
{
    qint32 indexOfData = data.indexOf("\r\n\r\n");
    Q_ASSERT(indexOfData > 0);

    if (mi.chunkedInfo().m_maxChunkSize > 0 &&
        data.size() - indexOfData > mi.chunkedInfo().m_maxChunkSize)
    {
        return sendChunked(mi, data);
    }

    return sendBlob(mi, data);
}

HHttpHandler::ReturnValue HHttpHandler::send(MessagingInfo& mi, StatusCode sc)
{
    return send(mi, HHttpMessageCreator::createResponse(sc, mi));
}

HHttpHandler::ReturnValue HHttpHandler::send(
    MessagingInfo& mi, const QByteArray& data, StatusCode sc, ContentType ct)
{
    return send(mi, HHttpMessageCreator::createResponse(sc, mi, data, ct));
}

HHttpHandler::ReturnValue HHttpHandler::send(
    MessagingInfo& mi, const SubscribeRequest& request)
{
    Q_ASSERT(request.isValid(false));

    QByteArray data = HHttpMessageCreator::create(request, mi);
    return send(mi, data);
}

HHttpHandler::ReturnValue HHttpHandler::send(
    MessagingInfo& mi, const SubscribeResponse& response)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(response.isValid(true));

    QByteArray data = HHttpMessageCreator::create(response, mi);
    return send(mi, data);
}

HHttpHandler::ReturnValue HHttpHandler::send(
    MessagingInfo& mi, const UnsubscribeRequest& req)
{
    Q_ASSERT(req.isValid(false));

    QByteArray data = HHttpMessageCreator::create(req, mi);
    return send(mi, data);
}

HHttpHandler::ReturnValue HHttpHandler::send(
    MessagingInfo& mi, const NotifyRequest& req)
{
    Q_ASSERT(req.isValid(true));

    QByteArray data = HHttpMessageCreator::create(req, mi);
    return send(mi, data);
}

HHttpHandler::ReturnValue HHttpHandler::receive(
    MessagingInfo& mi, NotifyRequest& req, NotifyRequest::RetVal& retVal,
    const QHttpRequestHeader* reqHdr, const QByteArray* body)
{
    QByteArray bodyContent;
    QHttpRequestHeader requestHeader;

    if (!reqHdr && !body)
    {
        ReturnValue rv = receive(mi, requestHeader, &bodyContent);
        if (rv != Success)
        {
            return rv;
        }
    }
    else
    {
        Q_ASSERT(reqHdr);
        Q_ASSERT(body);

        requestHeader = *reqHdr;
        bodyContent   = *body;
    }

    retVal = HHttpMessageCreator::create(requestHeader, bodyContent, req);

    switch(retVal)
    {
    case NotifyRequest::Success:
        break;

    case NotifyRequest::PreConditionFailed:
        mi.setKeepAlive(false);
        return send(mi, PreconditionFailed);

    case NotifyRequest::InvalidContents:
    case NotifyRequest::InvalidSequenceNr:
        mi.setKeepAlive(false);
        return send(mi, BadRequest);

    default:
        Q_ASSERT(false);

        retVal = NotifyRequest::BadRequest;
        mi.setKeepAlive(false);
        return send(mi, BadRequest);
    }

    return Success;
}

HHttpHandler::ReturnValue HHttpHandler::receive(
    MessagingInfo& mi, SubscribeRequest& req, SubscribeRequest::RetVal& retVal,
    const QHttpRequestHeader* reqHdr)
{
    QHttpRequestHeader requestHeader;
    if (!reqHdr)
    {
        ReturnValue rv = receive(mi, requestHeader);
        if (rv != Success)
        {
            return rv;
        }
    }
    else
    {
        requestHeader = *reqHdr;
    }

    retVal = HHttpMessageCreator::create(requestHeader, req);

    switch(retVal)
    {
    case SubscribeRequest::Success:
        break;

    case SubscribeRequest::PreConditionFailed:
        mi.setKeepAlive(false);
        return send(mi, BadRequest);

    case SubscribeRequest::IncompatibleHeaders:
        mi.setKeepAlive(false);
        return send(mi, BadRequest);

    case SubscribeRequest::BadRequest:
        mi.setKeepAlive(false);
        return send(mi, BadRequest);

    default:
        Q_ASSERT(false);

        retVal = SubscribeRequest::BadRequest;
        mi.setKeepAlive(false);
        return send(mi, BadRequest);
    }

    return Success;
}

HHttpHandler::ReturnValue HHttpHandler::receive(
    MessagingInfo& mi, UnsubscribeRequest& req,
    UnsubscribeRequest::RetVal& retVal, const QHttpRequestHeader* reqHdr)
{
    QHttpRequestHeader requestHeader;
    if (!reqHdr)
    {
        ReturnValue rv = receive(mi, requestHeader);
        if (rv != Success)
        {
            return rv;
        }
    }
    else
    {
        requestHeader = *reqHdr;
    }

    retVal = HHttpMessageCreator::create(requestHeader, req);

    switch(retVal)
    {
    case UnsubscribeRequest::Success:
        break;

    case UnsubscribeRequest::IncompatibleHeaders:
        mi.setKeepAlive(false);
        return send(mi, IncompatibleHeaderFields);

    case UnsubscribeRequest::PreConditionFailed:
        mi.setKeepAlive(false);
        return send(mi, PreconditionFailed);

    default:
        Q_ASSERT(false);

        retVal = UnsubscribeRequest::BadRequest;
        mi.setKeepAlive(false);
        return send(mi, BadRequest);
    }

    return Success;
}

HHttpHandler::ReturnValue HHttpHandler::receive(
    MessagingInfo& mi, SubscribeResponse& resp)
{
    QHttpResponseHeader respHeader;
    ReturnValue rv = receive(mi, respHeader);
    if (rv != Success)
    {
        return rv;
    }

    SubscribeResponse tmpResp;
    if (HHttpMessageCreator::create(respHeader, tmpResp))
    {
        resp = tmpResp;
    }

    return Success;
}

HHttpHandler::ReturnValue HHttpHandler::msgIO(
    MessagingInfo& mi, const SubscribeRequest& request,
    SubscribeResponse& response)
{
    ReturnValue rv = send(mi, HHttpMessageCreator::create(request, mi));
    if (rv != Success)
    {
        return rv;
    }

    return receive(mi, response);
}

HHttpHandler::ReturnValue HHttpHandler::msgIO(
    MessagingInfo& mi, QHttpRequestHeader& requestHdr,
    const QByteArray& reqBody, QHttpResponseHeader& responseHdr,
    QByteArray* respBody)
{
    QByteArray data = HHttpMessageCreator::setupData(requestHdr, reqBody, mi);
    ReturnValue rv = send(mi, data);
    if (rv != Success)
    {
        return rv;
    }

    return receive(mi, responseHdr, respBody);
}

HHttpHandler::ReturnValue HHttpHandler::msgIO(
    MessagingInfo& mi, QHttpRequestHeader& requestHdr,
    QHttpResponseHeader& responseHdr, QByteArray* respBody)
{
    return msgIO(mi, requestHdr, QByteArray(), responseHdr, respBody);
}

HHttpHandler::ReturnValue HHttpHandler::msgIO(
    MessagingInfo& mi, const UnsubscribeRequest& request)
{
    Q_ASSERT(request.isValid(false));

    ReturnValue rv = send(mi, HHttpMessageCreator::create(request, mi));
    if (rv != Success)
    {
        return rv;
    }

    QHttpResponseHeader responseHdr;
    rv = receive(mi, responseHdr);
    if (rv != Success)
    {
        return rv;
    }

    if (!responseHdr.isValid() || responseHdr.statusCode() != 200)
    {
        mi.setLastErrorDescription(
            QString("Unsubscribe failed: %1.").arg(responseHdr.reasonPhrase()));
    }

    return Success;
}

HHttpHandler::ReturnValue HHttpHandler::msgIO(
    MessagingInfo& mi, const NotifyRequest& request)
{
    ReturnValue rv = send(mi, request);
    if (rv != Success)
    {
        return rv;
    }

    QHttpResponseHeader responseHdr;
    rv = receive(mi, responseHdr);
    if (rv != Success)
    {
        return rv;
    }

    if (!responseHdr.isValid() || responseHdr.statusCode() != 200)
    {
        mi.setLastErrorDescription(
            QString("Notify failed: %1.").arg(responseHdr.reasonPhrase()));
    }

    return Success;
}

HHttpHandler::ReturnValue HHttpHandler::msgIO(
    MessagingInfo& mi, QHttpRequestHeader& reqHdr,
    const QtSoapMessage& soapMsg, QtSoapMessage& response)
{
    QHttpResponseHeader responseHdr;

    QByteArray respBody;

    ReturnValue rv =
        msgIO(mi, reqHdr, soapMsg.toXmlString().toUtf8(), responseHdr, &respBody);

    if (rv != Success)
    {
        return rv;
    }

    if (!respBody.size())
    {
        mi.setLastErrorDescription(QString(
            "No response to the sent SOAP message from host @ %1").arg(
                mi.socket().peerName()));

        return InvalidData;
    }

    QDomDocument dd;
    if (!dd.setContent(respBody, true))
    {
        mi.setLastErrorDescription(QString(
            "Invalid SOAP response from host @ %1").arg(mi.socket().peerName()));

        return InvalidData;
    }

    response.setContent(dd);

    return Success;
}

HHttpHandler::ReturnValue HHttpHandler::sendActionFailed(
    MessagingInfo& mi, qint32 actionErrCode, const QString& description)
{
    QByteArray data =
        HHttpMessageCreator::createResponse(mi, actionErrCode, description);

    return send(mi, data);
}

}
}
