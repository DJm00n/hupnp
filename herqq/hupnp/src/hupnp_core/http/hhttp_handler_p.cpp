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

#include "hhttp_handler_p.h"
#include "hhttp_messaginginfo_p.h"
#include "hhttp_messagecreator_p.h"
#include "hhttp_utils_p.h"

#include "./../general/hupnp_global_p.h"
#include "./../devicehosting/messages/hevent_messages_p.h"

#include "./../../utils/hlogger_p.h"
#include "./../../utils/hexceptions_p.h"

#include <QTcpSocket>
#include <QHostAddress>
#include <QHttpRequestHeader>
#include <QHttpResponseHeader>

#include <QtSoapMessage>

namespace Herqq
{

namespace Upnp
{


namespace
{
class Sleeper : private QThread
{
public:

    static void msleep(qint32 msecs)
    {
        QThread::msleep(msecs);
    }
};
}

/*******************************************************************************
 * HHttpHandler
 ******************************************************************************/
HHttpHandler::HHttpHandler(const QByteArray& loggingIdentifier) :
    m_loggingIdentifier(loggingIdentifier), m_shuttingDown(0), m_callsInProgress(0)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
}

HHttpHandler::~HHttpHandler()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    shutdown(true);
}

void HHttpHandler::shutdown(bool wait)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    m_shuttingDown = 1;
    while(wait)
    {
        if (m_callsInProgress > 0)
        {
            Sleeper::msleep(1);
        }
        else
        {
            break;
        }
    }
}

QByteArray HHttpHandler::readChunkedRequest(MessagingInfo& mi)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QByteArray retVal;

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
            Q_ASSERT(mi.socket().getChar(&readChar));

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
                Q_ASSERT(false);
            }
        }

        if (linesRead != 1)
        {
            // No size line. It should be available at this point.
            throw HSocketException(
                QObject::tr("No chunk-size line in the message body."));
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
            throw HSocketException(
                QObject::tr("Invalid chunk-size line: %1.").arg(
                    QString::fromUtf8(sizeLine)));
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
                throw HShutdownInProgressException(
                    QObject::tr("Shutting down. Aborting HTTP message body read."));
            }
            else if (!dataAvailable &&
                     mi.socket().state() != QTcpSocket::ConnectedState &&
                     mi.socket().state() != QTcpSocket::ClosingState)
            {
                throw HSocketException(
                    QObject::tr("Peer has disconnected. Could not read HTTP message body."));
            }
            else if (stopWatch.elapsed() >= mi.receiveTimeoutForNoData() &&
                     mi.receiveTimeoutForNoData() >= 0)
            {
                throw HTimeoutException(
                    QObject::tr("Timeout [%1] has elapsed. Could not read chunked HTTP message body.").arg(
                        QString::number(mi.receiveTimeoutForNoData())));
            }
            else if (!dataAvailable)
            {
                continue;
            }

            QByteArray tmp; tmp.resize(chunkSize - buf.size());
            qint32 read = mi.socket().read(tmp.data(), chunkSize - buf.size());

            if (read < 0)
            {
                throw HSocketException(
                    QObject::tr("Failed to read chunk: %1").arg(mi.socket().errorString()));
            }
            else if (read == 0)
            {
                continue;
            }

            tmp.resize(read);
            buf.append(tmp);
        }

        // append the chunk to the return value and
        retVal.append(buf);

        char c;
        mi.socket().getChar(&c);
        mi.socket().getChar(&c);
        // remove the mandatory CRLF trailing the data

        stopWatch.restart();
    }

    return retVal;
}

QByteArray HHttpHandler::readRequestData(
    MessagingInfo& mi, qint64 contentLength)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QByteArray requestData;
    qint64 bytesRead = 0;
    QByteArray buf; buf.resize(4096);

    QTime stopWatch; stopWatch.start();
    while (bytesRead < contentLength)
    {
        bool dataAvailable = mi.socket().bytesAvailable() ||
                             mi.socket().waitForReadyRead(50);

        if (m_shuttingDown && (!dataAvailable || stopWatch.elapsed() > 500))
        {
            throw HShutdownInProgressException(
                QObject::tr("Shutting down. Aborting HTTP message body read."));
        }
        else if (!dataAvailable &&
                 mi.socket().state() != QTcpSocket::ConnectedState &&
                 mi.socket().state() != QTcpSocket::ClosingState)
        {
            throw HSocketException(
                QObject::tr("Peer has disconnected. Could not read HTTP message body."));
        }
        else if (stopWatch.elapsed() >= mi.receiveTimeoutForNoData() &&
                 mi.receiveTimeoutForNoData() >= 0)
        {
            throw HTimeoutException(
                QObject::tr("Timeout [%1] has elapsed. Could not read HTTP message body.").arg(
                    QString::number(mi.receiveTimeoutForNoData())));
        }
        else if (!dataAvailable)
        {
            continue;
        }

        do
        {
            qint64 retVal = mi.socket().read(
                buf.data(), qMin(static_cast<qint64>(buf.size()), contentLength - bytesRead));

            if (retVal < 0)
            {
                throw HSocketException(
                    QObject::tr("Could not read HTTP message body: .").arg(
                        mi.socket().errorString()));
            }
            else if (retVal > 0)
            {
                bytesRead += retVal;
                requestData.append(QByteArray(buf.data(), retVal));
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

    return requestData;
}

template<typename Header>
QByteArray HHttpHandler::receive(MessagingInfo& mi, Header& hdr)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Counter cnt(m_callsInProgress);

    QByteArray headerData;
    QTime stopWatch; stopWatch.start();
    for(;;)
    {
        bool dataAvailable = mi.socket().bytesAvailable() ||
                             mi.socket().waitForReadyRead(50);

        if (m_shuttingDown && (!dataAvailable || stopWatch.elapsed() > 500))
        {
            throw HShutdownInProgressException(
                QObject::tr("Shutting down. Aborting HTTP message header read."));
        }
        else if (!dataAvailable &&
                 mi.socket().state() != QTcpSocket::ConnectedState &&
                 mi.socket().state() != QTcpSocket::ClosingState)
        {
            throw HSocketException(
                QObject::tr("Peer has disconnected. Could not read HTTP message header."));
        }
        else if (stopWatch.elapsed() >= mi.receiveTimeoutForNoData() &&
                 mi.receiveTimeoutForNoData() >= 0)
        {
            throw HTimeoutException(
                QObject::tr("Timeout [%1] has elapsed. Could not read HTTP message header.").arg(
                    QString::number(mi.receiveTimeoutForNoData())));
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
        return QByteArray();
    }

    QByteArray body;
    bool chunked = hdr.value("TRANSFER-ENCODING") == "chunked";
    if (chunked)
    {
        if (hdr.hasContentLength())
        {
            hdr = Header();
            return QByteArray();
        }

        body = readChunkedRequest(mi);
    }
    else
    {
        if (hdr.hasContentLength())
        {
            quint32 clength = hdr.contentLength();
            body = readRequestData(mi, clength);
        }
        else
        {
            body = mi.socket().readAll();
        }
    }

    mi.setKeepAlive(HHttpUtils::keepAlive(hdr));

    return body;
}

void HHttpHandler::sendBlob(MessagingInfo& mi, const QByteArray& data)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(!data.isEmpty());
    Counter cnt(m_callsInProgress);

    QHostAddress peer = mi.socket().peerAddress();

    qint64 bytesWritten   = 0, index = 0;
    qint32 errorThreshold = 0;
    while(index < data.size())
    {
        if (mi.socket().state() != QTcpSocket::ConnectedState)
        {
            throw HSocketException(
                QObject::tr("Failed to send data to %1. Connection closed.").arg(
                    peer.toString()));
        }

        bytesWritten = mi.socket().write(data.data() + index, data.size() - index);

        if (bytesWritten == 0)
        {
            if (!mi.socket().isValid() || errorThreshold > 100)
            {
                throw HSocketException(QObject::tr("Failed to send data to %1.").arg(
                    peer.toString()));
            }

            ++errorThreshold;
        }
        else if (bytesWritten < 0)
        {
            throw HSocketException(QObject::tr("Failed to send data to %1.").arg(
                peer.toString()));
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
}

void HHttpHandler::sendChunked(MessagingInfo& mi, const QByteArray& data)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(!data.isEmpty());
    Q_ASSERT(mi.chunkedInfo().m_maxChunkSize > 0);

    Counter cnt(m_callsInProgress);

    QHostAddress peer = mi.socket().peerAddress();

    const char crlf[] = {"\r\n"};

    // send the http header first
    qint32 endOfHdr = data.indexOf("\r\n\r\n") + 4;
    send(mi, data.left(endOfHdr));

    // then start sending the data in chunks
    qint64 bytesWritten   = 0;
    qint32 errorThreshold = 0, index = endOfHdr;
    while(index < data.size())
    {
        if (mi.socket().state() != QTcpSocket::ConnectedState)
        {
            throw HSocketException(
                QObject::tr("Failed to send data to %1. Connection closed.").arg(
                    peer.toString()));
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
            throw HSocketException(
                QObject::tr("Failed to send data to %1.").arg(peer.toString()));
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
                    throw HSocketException(QObject::tr("Failed to send data to %1.").arg(
                        peer.toString()));
                }

                ++errorThreshold;
            }
            else if (bytesWritten < 0)
            {
                throw HSocketException(QObject::tr("Failed to send data to %1.").arg(
                    peer.toString()));
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
            throw HSocketException(
                QObject::tr("Failed to send data to %1.").arg(peer.toString()));
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
}

void HHttpHandler::send(MessagingInfo& mi, const QByteArray& data)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    qint32 indexOfData = data.indexOf("\r\n\r\n");
    Q_ASSERT(indexOfData > 0);

    if (mi.chunkedInfo().m_maxChunkSize > 0 &&
        data.size() - indexOfData > mi.chunkedInfo().m_maxChunkSize)
    {
        sendChunked(mi, data);
    }
    else
    {
        sendBlob(mi, data);
    }
}

void HHttpHandler::send(MessagingInfo& mi, StatusCode sc)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    send(mi, HHttpMessageCreator::createResponse(sc, mi));
}

void HHttpHandler::send(
    MessagingInfo& mi, const QByteArray& data, StatusCode sc, ContentType ct)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    send(mi, HHttpMessageCreator::createResponse(sc, mi, data, ct));
}

void HHttpHandler::send(MessagingInfo& mi, const SubscribeRequest& request)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(request.isValid());

    QByteArray data = HHttpMessageCreator::create(request, mi);
    send(mi, data);
}

void HHttpHandler::send(
    MessagingInfo& mi, const SubscribeResponse& response)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(response.isValid());

    QByteArray data = HHttpMessageCreator::create(response, mi);
    send(mi, data);
}

void HHttpHandler::send(MessagingInfo& mi, const UnsubscribeRequest& req)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(req.isValid());

    QByteArray data = HHttpMessageCreator::create(req, mi);
    send(mi, data);
}

void HHttpHandler::send(MessagingInfo& mi, const NotifyRequest& req)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(req.isValid());

    QByteArray data = HHttpMessageCreator::create(req, mi);
    send(mi, data);
}

NotifyRequest::RetVal HHttpHandler::receive(
    MessagingInfo& mi, NotifyRequest& req,
    const QHttpRequestHeader* reqHdr, const QByteArray* body)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QByteArray bodyContent;
    QHttpRequestHeader requestHeader;

    if (!reqHdr && !body)
    {
        bodyContent = receive(mi, requestHeader);
    }
    else
    {
        Q_ASSERT(reqHdr);
        Q_ASSERT(body);

        requestHeader = *reqHdr;
        bodyContent   = *body;
    }

    NotifyRequest nreq;

    NotifyRequest::RetVal retVal =
        HHttpMessageCreator::create(requestHeader, bodyContent, nreq);

    switch(retVal)
    {
    case NotifyRequest::Success:
        break;

    case NotifyRequest::PreConditionFailed:
        mi.setKeepAlive(false);
        send(mi, PreconditionFailed);
        break;

    case NotifyRequest::InvalidContents:
    case NotifyRequest::InvalidSequenceNr:
        mi.setKeepAlive(false);
        send(mi, BadRequest);
        break;

    default:
        Q_ASSERT(false);

        retVal = NotifyRequest::BadRequest;
        mi.setKeepAlive(false);
        send(mi, BadRequest);
    }

    req = nreq;
    return retVal;
}

SubscribeRequest::RetVal
    HHttpHandler::receive(
        MessagingInfo& mi, SubscribeRequest& req, const QHttpRequestHeader* reqHdr)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QHttpRequestHeader requestHeader;
    if (!reqHdr)
    {
        receive(mi, requestHeader);
    }
    else
    {
        requestHeader = *reqHdr;
    }

    SubscribeRequest sreq;
    SubscribeRequest::RetVal retVal =
        HHttpMessageCreator::create(requestHeader, sreq);

    switch(retVal)
    {
    case SubscribeRequest::Success:
        break;

    case SubscribeRequest::PreConditionFailed:
        mi.setKeepAlive(false);
        send(mi, BadRequest);
        break;

    case SubscribeRequest::IncompatibleHeaders:
        mi.setKeepAlive(false);
        send(mi, BadRequest);
        break;

    case SubscribeRequest::BadRequest:
        mi.setKeepAlive(false);
        send(mi, BadRequest);
        break;

    default:
        Q_ASSERT(false);

        retVal = SubscribeRequest::BadRequest;
        mi.setKeepAlive(false);
        send(mi, BadRequest);
    }

    req = sreq;
    return retVal;
}

UnsubscribeRequest::RetVal HHttpHandler::receive(
    MessagingInfo& mi, UnsubscribeRequest& req, const QHttpRequestHeader* reqHdr)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QHttpRequestHeader requestHeader;
    if (!reqHdr)
    {
        receive(mi, requestHeader);
    }
    else
    {
        requestHeader = *reqHdr;
    }

    UnsubscribeRequest ureq;
    UnsubscribeRequest::RetVal retVal =
        HHttpMessageCreator::create(requestHeader, ureq);

    switch(retVal)
    {
    case UnsubscribeRequest::Success:
        break;

    case UnsubscribeRequest::IncompatibleHeaders:
        mi.setKeepAlive(false);
        send(mi, IncompatibleHeaderFields);

    case UnsubscribeRequest::PreConditionFailed:
        mi.setKeepAlive(false);
        send(mi, PreconditionFailed);
        break;

    default:
        Q_ASSERT(false);

        retVal = UnsubscribeRequest::BadRequest;
        mi.setKeepAlive(false);
        send(mi, BadRequest);
    }

    req = ureq;
    return retVal;
}

void HHttpHandler::receive(MessagingInfo& mi, SubscribeResponse& resp)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QHttpResponseHeader respHeader;
    receive(mi, respHeader);

    SubscribeResponse tmpResp;
    if (HHttpMessageCreator::create(respHeader, tmpResp))
    {
        resp = tmpResp;
    }
}

SubscribeResponse HHttpHandler::msgIO(
    MessagingInfo& mi, const SubscribeRequest& request)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    send(mi, HHttpMessageCreator::create(request, mi));

    SubscribeResponse response;
    receive(mi, response);

    return response;
}

QByteArray HHttpHandler::msgIO(
    MessagingInfo& mi, QHttpRequestHeader& requestHdr,
    const QByteArray& reqBody, QHttpResponseHeader& responseHdr)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QByteArray data = HHttpMessageCreator::setupData(requestHdr, reqBody, mi);
    send(mi, data);

    return receive(mi, responseHdr);
}

QByteArray HHttpHandler::msgIO(
    MessagingInfo& mi, QHttpRequestHeader& requestHdr,
    QHttpResponseHeader& responseHdr)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    return msgIO(mi, requestHdr, QByteArray(), responseHdr);
}

void HHttpHandler::msgIO(
    MessagingInfo& mi, const UnsubscribeRequest& request)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(request.isValid());

    send(mi, HHttpMessageCreator::create(request, mi));

    QHttpResponseHeader responseHdr;
    receive(mi, responseHdr);

    if (responseHdr.isValid() && responseHdr.statusCode() == 200)
    {
        return;
    }

    throw HOperationFailedException(
        QObject::tr("Unsubscribe failed: %1.").arg(responseHdr.reasonPhrase()));
}

void HHttpHandler::msgIO(MessagingInfo& mi, const NotifyRequest& request)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    send(mi, request);

    QHttpResponseHeader responseHdr;
    receive(mi, responseHdr);

    if (responseHdr.isValid() && responseHdr.statusCode() == 200)
    {
        return;
    }

    throw HOperationFailedException(
        QObject::tr("Notify failed: %1.").arg(responseHdr.reasonPhrase()));
}

QtSoapMessage HHttpHandler::msgIO(
    MessagingInfo& mi, QHttpRequestHeader& reqHdr,
    const QtSoapMessage& soapMsg)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QHttpResponseHeader responseHdr;

    QByteArray respBody =
        msgIO(mi, reqHdr, soapMsg.toXmlString().toUtf8(), responseHdr);

    if (!respBody.size())
    {
        throw HSocketException(
            QObject::tr("No response to the sent SOAP message from host @ %1").arg(
                mi.socket().peerName()));
    }

    QDomDocument dd;
    if (!dd.setContent(respBody, true))
    {
        throw HOperationFailedException(
            QObject::tr("Invalid SOAP response from host @ %1").arg(mi.socket().peerName()));
    }

    QtSoapMessage soapResponse;
    soapResponse.setContent(dd);

    return soapResponse;
}

void HHttpHandler::sendActionFailed(
    MessagingInfo& mi, qint32 actionErrCode, const QString& description)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QByteArray data =
        HHttpMessageCreator::createResponse(mi, actionErrCode, description);

    send(mi, data);
}

}
}
