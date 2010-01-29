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

#include "http_server_p.h"
#include "../upnp_action.h"
#include "../upnp_global_p.h"
#include "../../../../utils/src/logger_p.h"
#include "../../../../core/include/HExceptions"

#include <QUrl>
#include <QTime>
#include <QString>
#include <QTcpSocket>
#include <QByteArray>
#include <QThreadPool>
#include <QMutexLocker>
#include <QNetworkInterface>
#include <QHttpRequestHeader>
#include <QHttpResponseHeader>

#include <QtSoapMessage>

namespace Herqq
{

namespace Upnp
{

ChunkedInfo::ChunkedInfo() :
    m_maxChunkSize(0), m_minChunkSize(0)
{
}

namespace
{
template<typename Hdr>
bool keepAlive(const Hdr& hdr)
{
    HLOG(H_AT, H_FUN);

    QString connection = hdr.value("CONNECTION");
    if (hdr.minorVersion() == 1)
    {
        return connection.compare("close", Qt::CaseInsensitive) != 0;
    }

    return connection.compare("Keep-Alive", Qt::CaseInsensitive) == 0;
}

QString callbackAsStr(const QList<QUrl>& callbacks)
{
    HLOG(H_AT, H_FUN);

    QString retVal;

    foreach(QUrl cb, callbacks)
    {
        retVal.append(QString("<%1>").arg(cb.toString()));
    }

    return retVal;
}

QString rfc1123DateFormat()
{
    return "ddd, dd MMM yyyy HH:mm:ss";
}

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
 * MessagingInfo
 ******************************************************************************/
MessagingInfo::MessagingInfo(
    QTcpSocket& sock, qint32 receiveTimeoutForNoData) :
        m_sock(sock), m_keepAlive(false),
        m_receiveTimeoutForNoData(receiveTimeoutForNoData),
        m_chunkedInfo()
{
}

MessagingInfo::MessagingInfo(
    QTcpSocket& sock, bool keepAlive, qint32 receiveTimeoutForNoData) :
        m_sock(sock), m_keepAlive(keepAlive),
        m_receiveTimeoutForNoData(receiveTimeoutForNoData)
{
}

void MessagingInfo::setHostInfo(const QUrl& hostInfo)
{
    QString tmp(hostInfo.host());

    QHostAddress test(tmp);
    Q_ASSERT(!test.isNull()); Q_UNUSED(test)

    if (hostInfo.port(0) > 0)
    {
        tmp.append(':').append(QString::number(hostInfo.port()));
    }

    m_hostInfo = tmp;
}

QString MessagingInfo::hostInfo() const
{
    if (m_hostInfo.isEmpty())
    {
        // fall back to the ip address if no host information was provided.
        return QString("%1:%2").arg(
            m_sock.peerName(), QString::number(m_sock.peerPort()));
    }

    return m_hostInfo;
}
/*******************************************************************************
 * HHttpHandler
 ******************************************************************************/
HHttpHandler::HHttpHandler() :
    m_shuttingDown(0), m_callsInProgress(0)
{
    HLOG(H_AT, H_FUN);
}

HHttpHandler::~HHttpHandler()
{
    HLOG(H_AT, H_FUN);
    shutdown(true);
}

void HHttpHandler::shutdown(bool wait)
{
    HLOG(H_AT, H_FUN);

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
    HLOG(H_AT, H_FUN);

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
            qint32 read = mi.m_sock.read(tmp.data(), chunkSize - buf.size());

            if (read < 0)
            {
                throw HSocketException(
                    QObject::tr("Failed to read chunk: %1").arg(mi.m_sock.errorString()));
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
    HLOG(H_AT, H_FUN);

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
    HLOG(H_AT, H_FUN);
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

    mi.setKeepAlive(keepAlive(hdr));

    return body;
}

void HHttpHandler::send(MessagingInfo& mi, const QByteArray& data)
{
    HLOG(H_AT, H_FUN);
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
    HLOG(H_AT, H_FUN);
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

void HHttpHandler::send(MessagingInfo& mi, QHttpHeader& reqHdr)
{
    HLOG(H_AT, H_FUN);
    send(mi, reqHdr, QByteArray());
}

void HHttpHandler::send(
    MessagingInfo& mi, QHttpHeader& reqHdr, const QByteArray& data)
{
    HLOG(H_AT, H_FUN);
    Q_ASSERT(reqHdr.isValid());

    reqHdr.setValue("DATE", QDateTime::currentDateTime().toString(rfc1123DateFormat()));

    if (!mi.keepAlive() && reqHdr.minorVersion() == 1)
    {
        reqHdr.setValue("Connection", "close");
    }

    reqHdr.setValue("HOST", mi.hostInfo());

    bool chunked = false;

    if (mi.chunkedInfo().m_maxChunkSize > 0 &&
        data.size() > mi.chunkedInfo().m_maxChunkSize)
    {
        chunked = true;
        reqHdr.setValue("Transfer-Encoding", "chunked");
    }
    else
    {
        reqHdr.setContentLength(data.size());
    }

    QByteArray msg(reqHdr.toString().toUtf8());
    msg.append(data);

    if (chunked)
    {
        sendChunked(mi, msg);
    }
    else
    {
       send(mi, msg);
    }
}

void HHttpHandler::send(MessagingInfo& mi, const SubscribeRequest& request)
{
    HLOG(H_AT, H_FUN);
    Q_ASSERT(request.isValid());

    QHttpRequestHeader requestHdr("SUBSCRIBE", request.eventUrl().path());
    requestHdr.setValue("HOST", request.eventUrl().host());

    if (request.hasUserAgent())
    {
        requestHdr.setValue("USER-AGENT", request.userAgent().toString());
    }

    requestHdr.setValue("TIMEOUT", request.timeout().toString());
    requestHdr.setValue("NT", request.nt().typeToString());
    requestHdr.setValue("CALLBACK", callbackAsStr(request.callbacks()));

    send(mi, requestHdr);
}

void HHttpHandler::send(
    MessagingInfo& mi, const SubscribeResponse& response)
{
    HLOG(H_AT, H_FUN);
    Q_ASSERT(response.isValid());

    QHttpResponseHeader responseHdr(200, "OK");
    responseHdr.setContentLength(0);

    responseHdr.setValue("SID"    , response.sid().toString());
    responseHdr.setValue("TIMEOUT", response.timeout().toString());
    responseHdr.setValue("SERVER" , response.server().toString());

    send(mi, responseHdr);
}

void HHttpHandler::send(MessagingInfo& mi, const UnsubscribeRequest& req)
{
    HLOG(H_AT, H_FUN);
    Q_ASSERT(req.isValid());

    QHttpRequestHeader requestHdr("UNSUBSCRIBE", req.eventUrl().path());
    requestHdr.setValue("HOST", req.eventUrl().host());
    requestHdr.setValue("SID", req.sid().toString());

    send(mi, requestHdr);
}

void HHttpHandler::send(MessagingInfo& mi, const NotifyRequest& req)
{
    HLOG(H_AT, H_FUN);
    Q_ASSERT(req.isValid());

    QHttpRequestHeader reqHdr;
    reqHdr.setContentType("Content-type: text/xml; charset=\"utf-8\"");

    reqHdr.setRequest("NOTIFY", req.deliveryPath().toString());
    reqHdr.setValue  ("HOST"  , req.host().toString());
    reqHdr.setValue  ("SID"   , req.sid().toString());
    reqHdr.setValue  ("SEQ"   , QString::number(req.seq()));
    reqHdr.setValue  ("NT"    , "upnp:event");
    reqHdr.setValue  ("NTS"   , "upnp:propchange");

    send(mi, reqHdr, req.data());
}

NotifyRequest::RetVal HHttpHandler::receive(
    MessagingInfo& mi, NotifyRequest& req,
    const QHttpRequestHeader* reqHdr, const QString* body)
{
    HLOG(H_AT, H_FUN);

    QString bodyContent;
    QHttpRequestHeader requestHeader;

    if (!reqHdr && !body)
    {
        bodyContent = QString::fromUtf8(receive(mi, requestHeader));
    }
    else
    {
        Q_ASSERT(reqHdr);
        Q_ASSERT(body);

        requestHeader = *reqHdr;
        bodyContent   = *body;
    }

    QString nt     = requestHeader.value("NT" );
    QString nts    = requestHeader.value("NTS");
    QString sid    = requestHeader.value("SID");
    QString seqStr = requestHeader.value("SEQ");
    QString host   = requestHeader.value("HOST").trimmed();

    QString deliveryPath = requestHeader.path().trimmed();
    if (!deliveryPath.startsWith('/'))
    {
        deliveryPath.insert(0, '/');
    }

    QUrl callbackUrl(QString("http://%1%2").arg(host, deliveryPath));

    NotifyRequest nreq;
    NotifyRequest::RetVal retVal =
        nreq.setContents(callbackUrl, nt, nts, sid, seqStr, bodyContent);

    switch(retVal)
    {
    case NotifyRequest::Success:
        break;

    case NotifyRequest::PreConditionFailed:
        mi.setKeepAlive(false);
        responsePreconditionFailed(mi);
        break;

    case NotifyRequest::InvalidContents:
    case NotifyRequest::InvalidSequenceNr:
        mi.setKeepAlive(false);
        responseBadRequest(mi);
        break;

    default:
        Q_ASSERT(false);

        retVal = NotifyRequest::BadRequest;
        mi.setKeepAlive(false);
        responseBadRequest(mi);
    }

    req = nreq;
    return retVal;
}

SubscribeRequest::RetVal
    HHttpHandler::receive(
        MessagingInfo& mi, SubscribeRequest& req, const QHttpRequestHeader* reqHdr)
{
    HLOG(H_AT, H_FUN);

    QHttpRequestHeader requestHeader;
    if (!reqHdr)
    {
        receive(mi, requestHeader);
    }
    else
    {
        requestHeader = *reqHdr;
    }

    QString nt         = requestHeader.value("NT");
    QString callback   = requestHeader.value("CALLBACK").trimmed();
    QString timeoutStr = requestHeader.value("TIMEOUT");
    QString sid        = requestHeader.value("SID");
    QString userAgent  = requestHeader.value("USER-AGENT");
    QString host       = requestHeader.value("HOST");
    QUrl servicePath   = requestHeader.path().trimmed();

    SubscribeRequest sreq;
    SubscribeRequest::RetVal retVal =
        sreq.setContents(
            nt, appendUrls("http://"+host, servicePath),
            sid, callback, timeoutStr, userAgent);

    switch(retVal)
    {
    case SubscribeRequest::Success:
        break;

    case SubscribeRequest::PreConditionFailed:
        mi.setKeepAlive(false);
        responsePreconditionFailed(mi);
        break;

    case SubscribeRequest::IncompatibleHeaders:
        mi.setKeepAlive(false);
        responseIncompatibleHeaderFields(mi);
        break;

    case SubscribeRequest::BadRequest:
        mi.setKeepAlive(false);
        responseBadRequest(mi);
        break;

    default:
        Q_ASSERT(false);

        retVal = SubscribeRequest::BadRequest;
        mi.setKeepAlive(false);
        responseBadRequest(mi);
    }

    req = sreq;
    return retVal;
}

UnsubscribeRequest::RetVal HHttpHandler::receive(
    MessagingInfo& mi, UnsubscribeRequest& req, const QHttpRequestHeader* reqHdr)
{
    HLOG(H_AT, H_FUN);

    QHttpRequestHeader requestHeader;
    if (!reqHdr)
    {
        receive(mi, requestHeader);
    }
    else
    {
        requestHeader = *reqHdr;
    }

    QString sid     = requestHeader.value("SID");
    QUrl callback   = requestHeader.value("CALLBACK").trimmed();
    QString hostStr = requestHeader.value("HOST").trimmed();

    if (!callback.isEmpty())
    {
        mi.setKeepAlive(false);
        responseIncompatibleHeaderFields(mi);
        return UnsubscribeRequest::BadRequest;
    }

    UnsubscribeRequest usreq;
    UnsubscribeRequest::RetVal retVal =
        usreq.setContents(
            appendUrls("http://"+hostStr, requestHeader.path().trimmed()), sid);

    switch(retVal)
    {
    case UnsubscribeRequest::Success:
        break;

    case UnsubscribeRequest::PreConditionFailed:
        mi.setKeepAlive(false);
        responsePreconditionFailed(mi);
        break;

    default:
        Q_ASSERT(false);

        retVal = UnsubscribeRequest::BadRequest;
        mi.setKeepAlive(false);
        responseBadRequest(mi);
    }

    req = usreq;
    return retVal;
}

void HHttpHandler::receive(MessagingInfo& mi, SubscribeResponse& resp)
{
    HLOG(H_AT, H_FUN);

    QHttpResponseHeader respHeader;
    receive(mi, respHeader);

    HSid      sid     = HSid(respHeader.value("SID"));
    HTimeout  timeout = HTimeout(respHeader.value("TIMEOUT"));
    QString   server  = respHeader.value("SERVER");
    QDateTime date    =
        QDateTime::fromString(respHeader.value("DATE"), rfc1123DateFormat());

    resp = SubscribeResponse(sid, server, timeout, date);
}

void HHttpHandler::response(
    MessagingInfo& mi, qint32 statusCode, const QString& reasonPhrase)
{
    HLOG(H_AT, H_FUN);

    QHttpResponseHeader responseHdr(statusCode, reasonPhrase);
    send(mi, responseHdr);
}

void HHttpHandler::response(
    MessagingInfo& mi, qint32 statusCode, const QString& reasonPhrase,
    const QString& body, const QString& contentType)
{
    HLOG(H_AT, H_FUN);

    QHttpResponseHeader responseHdr(statusCode, reasonPhrase);
    responseHdr.setContentType(contentType);
    send(mi, responseHdr, body.toUtf8());
}

void HHttpHandler::response(
    MessagingInfo& mi, qint32 statusCode, const QString& reasonPhrase,
    const QByteArray& body, const QString& contentType)
{
    HLOG(H_AT, H_FUN);

    QHttpResponseHeader responseHdr(statusCode, reasonPhrase);
    responseHdr.setContentType(contentType);
    send(mi, responseHdr, body);
}

SubscribeResponse HHttpHandler::msgIO(
    MessagingInfo& mi, const SubscribeRequest& request)
{
    HLOG(H_AT, H_FUN);
    send(mi, request);

    SubscribeResponse response;
    receive(mi, response);

    return response;
}

QByteArray HHttpHandler::msgIO(
    MessagingInfo& mi, QHttpRequestHeader& requestHdr,
    const QByteArray& reqBody, QHttpResponseHeader& responseHdr)
{
    HLOG(H_AT, H_FUN);

    send(mi, requestHdr, reqBody);
    return receive(mi, responseHdr);
}

QByteArray HHttpHandler::msgIO(
    MessagingInfo& mi, QHttpRequestHeader& requestHdr,
    QHttpResponseHeader& responseHdr)
{
    HLOG(H_AT, H_FUN);
    return msgIO(mi, requestHdr, QByteArray(), responseHdr);
}

void HHttpHandler::msgIO(
    MessagingInfo& mi, const UnsubscribeRequest& request)
{
    HLOG(H_AT, H_FUN);

    Q_ASSERT(request.isValid());

    send(mi, request);

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
    HLOG(H_AT, H_FUN);

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
    HLOG(H_AT, H_FUN);

    QHttpResponseHeader responseHdr;

    QString respBody =
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

void HHttpHandler::responseBadRequest(MessagingInfo& mi)
{
    HLOG(H_AT, H_FUN);

    static QString str("Bad Request");
    response(mi, 400, str);
}

void HHttpHandler::responseMethodNotAllowed(MessagingInfo& mi)
{
    HLOG(H_AT, H_FUN);

    static QString str("Method Not Allowed");
    response(mi, 405, str);
}

void HHttpHandler::responseServiceUnavailable(MessagingInfo& mi)
{
    HLOG(H_AT, H_FUN);

    static QString str("Service Unavailable");
    response(mi, 503, str);
}

void HHttpHandler::responseInternalServerError(MessagingInfo& mi)
{
    HLOG(H_AT, H_FUN);

    static QString str("Internal Server Error");
    response(mi, 500, str);
}

void HHttpHandler::responseNotFound(MessagingInfo& mi)
{
    HLOG(H_AT, H_FUN);

    static QString str("Not Found");
    response(mi, 404, str);
}

void HHttpHandler::responseInvalidAction(
    MessagingInfo& mi, const QString& body)
{
    HLOG(H_AT, H_FUN);

    static QString str("Invalid Action");
    response(mi, 401, str, body);
}

void HHttpHandler::responseInvalidArgs(
    MessagingInfo& mi, const QString& body)
{
    HLOG(H_AT, H_FUN);

    static QString str("Invalid Args");
    response(mi, 402, str, body);
}

void HHttpHandler::responsePreconditionFailed(MessagingInfo& mi)
{
    HLOG(H_AT, H_FUN);

    static QString str("Precondition Failed");
    response(mi, 412, str);
}

void HHttpHandler::responseIncompatibleHeaderFields(MessagingInfo& mi)
{
    HLOG(H_AT, H_FUN);

    static QString str("Incompatible header fields");
    response(mi, 400, str);
}

void HHttpHandler::responseOk(MessagingInfo& mi, const QString& body)
{
    HLOG(H_AT, H_FUN);

    static QString str("OK");
    response(mi, 200, str, body);
}

void HHttpHandler::responseOk(MessagingInfo& mi, const QByteArray& body)
{
    HLOG(H_AT, H_FUN);

    static QString str("OK");
    response(mi, 200, str, body);
}

void HHttpHandler::responseOk(MessagingInfo& mi)
{
    HLOG(H_AT, H_FUN);

    static QString str("OK");
    response(mi, 200, str);
}

namespace
{
void checkForActionError(
    qint32 actionRetVal, QtSoapMessage::FaultCode* soapFault, qint32* httpStatusCode,
    QString* httpReasonPhrase)
{
    HLOG(H_AT, H_FUN);

    Q_ASSERT(httpStatusCode);
    Q_ASSERT(httpReasonPhrase);
    Q_ASSERT(soapFault);

    if (actionRetVal == HAction::InvalidArgs())
    {
        *httpStatusCode   = 402;
        *httpReasonPhrase = "Invalid Args";
        *soapFault        = QtSoapMessage::Client;
    }
    else if (actionRetVal == HAction::ActionFailed())
    {
        *httpStatusCode   = 501;
        *httpReasonPhrase = "Action Failed";
        *soapFault        = QtSoapMessage::Client;
    }
    else if (actionRetVal == HAction::ArgumentValueInvalid())
    {
        *httpStatusCode   = 600;
        *httpReasonPhrase = "Argument Value Invalid";
        *soapFault        = QtSoapMessage::Client;
    }
    else if (actionRetVal == HAction::ArgumentValueOutOfRange())
    {
        *httpStatusCode   = 601;
        *httpReasonPhrase = "Argument Value Out of Range";
        *soapFault        = QtSoapMessage::Client;
    }
    else if (actionRetVal == HAction::OptionalActionNotImplemented())
    {
        *httpStatusCode   = 602;
        *httpReasonPhrase = "Optional Action Not Implemented";
        *soapFault        = QtSoapMessage::Client;
    }
    else if (actionRetVal == HAction::OutOfMemory())
    {
        *httpStatusCode   = 603;
        *httpReasonPhrase = "Out of Memory";
        *soapFault        = QtSoapMessage::Client;
    }
    else if (actionRetVal == HAction::HumanInterventionRequired())
    {
        *httpStatusCode   = 604;
        *httpReasonPhrase = "Human Intervention Required";
        *soapFault        = QtSoapMessage::Client;
    }
    else if (actionRetVal == HAction::StringArgumentTooLong())
    {
        *httpStatusCode   = 605;
        *httpReasonPhrase = "String Argument Too Long";
        *soapFault        = QtSoapMessage::Client;
    }
    else
    {
        *httpStatusCode   = actionRetVal;
        *httpReasonPhrase = QString::number(actionRetVal);
        *soapFault        = QtSoapMessage::Client;
    }
}
}

void HHttpHandler::responseActionFailed(
    MessagingInfo& mi, qint32 actionErrCode, const QString& description)
{
    HLOG(H_AT, H_FUN);

    QtSoapMessage::FaultCode soapFault;
    qint32 httpStatusCode; QString httpReasonPhrase;
    checkForActionError(
        actionErrCode, &soapFault, &httpStatusCode, &httpReasonPhrase);

    QtSoapMessage soapFaultResponse;
    soapFaultResponse.setFaultCode(soapFault);
    soapFaultResponse.setFaultString("UPnPError");

    QtSoapStruct detail(QtSoapQName("UPnPError"));
    detail.insert(new QtSoapSimpleType(QtSoapQName("errorCode"), actionErrCode));
    detail.insert(new QtSoapSimpleType(QtSoapQName("errorDescription"), description));
    soapFaultResponse.addFaultDetail(&detail);

    response(mi, httpStatusCode, httpReasonPhrase,
             soapFaultResponse.toXmlString());
}

/*******************************************************************************
 * HHttpServer::Task
 ******************************************************************************/
HHttpServer::Task::Task(HHttpServer* owner, qint32 socketDescriptor) :
    m_owner(owner), m_socketDescriptor(socketDescriptor)
{
    HLOG(H_AT, H_FUN);
}

void HHttpServer::Task::run()
{
    HLOG(H_AT, H_FUN);
    m_owner->processRequest(m_socketDescriptor);
}

/*******************************************************************************
 * HHttpServer::Server
 ******************************************************************************/
HHttpServer::Server::Server(HHttpServer* owner) :
    QTcpServer(owner), m_owner(owner)
{
    HLOG(H_AT, H_FUN);
}

void HHttpServer::Server::incomingConnection(qint32 socketDescriptor)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);

    HLOG_DBG(QObject::tr("Incoming connection."));

    m_owner->m_threadPool->start(
        new HHttpServer::Task(m_owner, socketDescriptor));
}

/*******************************************************************************
 * HHttpServer
 ******************************************************************************/
HHttpServer::HHttpServer(
    const QString& loggingIdentifier, QObject* parent) :
        QObject(parent),
            m_server(this), m_threadPool(new QThreadPool()), m_exiting(false),
            m_loggingIdentifier(loggingIdentifier.toLocal8Bit()), m_httpHandler(),
            m_chunkedInfo()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    m_threadPool->setParent(this);
    m_threadPool->setMaxThreadCount(50);
}

HHttpServer::~HHttpServer()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    close();
    delete m_threadPool;
}

QUrl HHttpServer::rootUrl() const
{
    HLOG(H_AT, H_FUN);

    Q_ASSERT(!m_server.serverAddress().isNull());
    Q_ASSERT(m_server.serverPort() > 0);

    return QUrl(QString("http://%1:%2").arg(
        m_server.serverAddress().toString(),
        QString::number(m_server.serverPort())));
}

bool HHttpServer::listen()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT_X(
        thread() == QThread::currentThread(), H_AT,
            "The HTTP Server has to be shutdown in the thread in which it is currently located.");

    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    foreach (QNetworkInterface iface, interfaces)
    {
        if (iface.flags() & QNetworkInterface::IsUp &&
          !(iface.flags() & QNetworkInterface::IsLoopBack))
        {
            QList<QNetworkAddressEntry> entries = iface.addressEntries();
            foreach(QNetworkAddressEntry entry, entries)
            {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol)
                {
                    for (int i = 0; i < 10; ++i)
                    {
                        if (m_server.listen(entry.ip()))
                        {
                            HLOG_INFO(QObject::tr(
                                "Binding to %1").arg(entry.ip().toString()));

                            return true;
                        }
                    }
                }
            }
        }
    }

    HLOG_INFO(QObject::tr(
        "Could not find a suitable network interface. Binding to localhost."));

    return m_server.listen(QHostAddress::LocalHost);
}

bool HHttpServer::listen(const QHostAddress& ha, quint16 port)
{
    HLOG(H_AT, H_FUN);

    Q_ASSERT_X(
        thread() == QThread::currentThread(), H_AT,
            "The HTTP Server has to be shutdown in the thread in which it is currently located.");


    if (ha == QHostAddress::Null || ha == QHostAddress::Any ||
        ha == QHostAddress::Broadcast)
    {
        return false;
    }

    return m_server.listen(ha, port);
}

void HHttpServer::close()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT_X(
        thread() == QThread::currentThread(), H_AT,
            "The HTTP Server has to be shutdown in the thread in which it is currently located.");

    m_exiting = true;

    if (m_server.isListening())
    {
        m_server.close();
    }

    m_httpHandler.shutdown(false);

    m_threadPool->waitForDone();
}

ChunkedInfo& HHttpServer::chunkedInfo()
{
    return m_chunkedInfo;
}

void HHttpServer::processRequest(qint32 socketDescriptor)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QTcpSocket client;
    client.setSocketDescriptor(socketDescriptor);

    QString peer = peerAsStr(client);

    HLOG_INFO(QObject::tr("Client from [%1] accepted. Current client count: %2").
        arg(peer, QString::number(m_threadPool->activeThreadCount())));

    QTime stopWatch; stopWatch.start();
    while(!m_exiting && client.state() == QTcpSocket::ConnectedState &&
          stopWatch.elapsed() < 30000)
    {
        QByteArray body;
        QHttpRequestHeader requestHeader;

        MessagingInfo mi(client);
        mi.chunkedInfo() = m_chunkedInfo;

        try
        {
            body = m_httpHandler.receive(mi, requestHeader);
            if (!requestHeader.isValid())
            {
                m_httpHandler.responseBadRequest(mi);
                break;
            }

            QString host = requestHeader.value("HOST");
            if (host.isEmpty())
            {
                m_httpHandler.responseBadRequest(mi);
                break;
            }

            mi.setHostInfo(host);
            mi.setKeepAlive(keepAlive(requestHeader));
        }
        catch(HTimeoutException&)
        {
            continue;
        }
        catch(HSocketException&)
        {
            // no more data / client has disconnected ==> no need to do
            // (even print) anything.
            break;
        }
        catch(HException& ex)
        {
            HLOG_WARN(QObject::tr("Receive failed: %1").arg(ex.reason()));
            break;
        }

        if (m_exiting)
        {
            break;
        }

        try
        {
            QString method = requestHeader.method();
            if (method.compare("GET", Qt::CaseInsensitive) == 0)
            {
                processGet(mi, requestHeader);
            }
            else if (method.compare("HEAD"), Qt::CaseInsensitive)
            {
                processHead(mi, requestHeader);
            }
            else if (method.compare("POST", Qt::CaseInsensitive) == 0)
            {
                processPost(mi, requestHeader, body);
            }
            else if (method.compare("NOTIFY", Qt::CaseInsensitive) == 0)
            {
                processNotifyMessage(mi, requestHeader, body);
            }
            else if (method.compare("SUBSCRIBE", Qt::CaseInsensitive) == 0)
            {
                processSubscription(mi, requestHeader);
            }
            else if (method.compare("UNSUBSCRIBE", Qt::CaseInsensitive) == 0)
            {
                processUnsubscription(mi, requestHeader);
            }
            else
            {
                m_httpHandler.responseMethodNotAllowed(mi);
                break;
            }
        }
        catch(HException& ex)
        {
            HLOG_WARN(ex.reason());
            break;
        }

        if (!mi.keepAlive())
        {
            break;
        }

        stopWatch.start();
    }

    if (client.state() == QTcpSocket::ConnectedState)
    {
        for(qint32 i = 0; i < 1000 && client.flush(); ++i)
        {
            client.waitForBytesWritten(1);
        }

        client.disconnectFromHost();
    }

    HLOG_INFO(QObject::tr("Client from [%1] disconnected. Current client count: %2").
        arg(peer, QString::number(m_threadPool->activeThreadCount())));
}

void HHttpServer::processGet(
    MessagingInfo& mi, const QHttpRequestHeader& requestHdr)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    HLOG_DBG(QObject::tr("Dispatching unknown GET request."));
    incomingUnknownGetRequest(mi, requestHdr);
}

void HHttpServer::processHead(
    MessagingInfo& mi, const QHttpRequestHeader& requestHdr)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    HLOG_DBG(QObject::tr("Dispatching unknown HEAD request."));
    incomingUnknownHeadRequest(mi, requestHdr);
}

void HHttpServer::processPost(
    MessagingInfo& mi, const QHttpRequestHeader& requestHdr, const QByteArray& body)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QString soapAction  = requestHdr.value("SOAPACTION");
    if (soapAction.indexOf("#") <= 0)
    {
        HLOG_DBG(QObject::tr("Dispatching unknown POST request."));
        incomingUnknownPostRequest(mi, requestHdr, body);
        return;
    }

    QString actionName = soapAction.mid(soapAction.indexOf("#"));
    if (actionName.isEmpty())
    {
        HLOG_DBG(QObject::tr("Dispatching unknown POST request."));
        incomingUnknownPostRequest(mi, requestHdr, body);
        return;
    }

    QtSoapMessage soapMsg;
    if (!soapMsg.setContent(body))
    {
        mi.setKeepAlive(false);
        m_httpHandler.responseBadRequest(mi);
        return;
    }

    QString controlUrl = requestHdr.path().simplified();
    if (controlUrl.isEmpty())
    {
        mi.setKeepAlive(false);
        m_httpHandler.responseBadRequest(mi);
        return;
    }

    InvokeActionRequest iareq(soapAction, soapMsg, controlUrl);
    HLOG_DBG(QObject::tr("Dispatching control request."));
    incomingControlRequest(mi, iareq);
}

void HHttpServer::processSubscription(
    MessagingInfo& mi, const QHttpRequestHeader& requestHdr)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    SubscribeRequest sreq;
    SubscribeRequest::RetVal rv = m_httpHandler.receive(mi, sreq, &requestHdr);

    if (rv == SubscribeRequest::Success)
    {
        HLOG_DBG(QObject::tr("Dispatching subscription request."));
        incomingSubscriptionRequest(mi, sreq);
    }
}

void HHttpServer::processUnsubscription(
    MessagingInfo& mi, const QHttpRequestHeader& requestHdr)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    UnsubscribeRequest usreq;
    UnsubscribeRequest::RetVal rv = m_httpHandler.receive(mi, usreq, &requestHdr);

    if (rv == UnsubscribeRequest::Success)
    {
        HLOG_DBG(QObject::tr("Dispatching unsubscription request."));
        incomingUnsubscriptionRequest(mi, usreq);
    }
}

void HHttpServer::processNotifyMessage(
    MessagingInfo& mi, const QHttpRequestHeader& request, const QString& body)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    NotifyRequest nreq;
    NotifyRequest::RetVal rv = m_httpHandler.receive(mi, nreq, &request, &body);

    if (rv == NotifyRequest::Success)
    {
        HLOG_DBG(QObject::tr("Dispatching event notification."));
        incomingNotifyMessage(mi, nreq);
    }
}

void HHttpServer::incomingSubscriptionRequest(
    MessagingInfo& mi, const SubscribeRequest&)
{
    HLOG(H_AT, H_FUN);
    HLOG_DBG(QObject::tr("Calling default implementation, which does nothing."));
    mi.setKeepAlive(false);
    m_httpHandler.responseMethodNotAllowed(mi);
}

void HHttpServer::incomingUnsubscriptionRequest(
    MessagingInfo& mi, const UnsubscribeRequest&)
{
    HLOG(H_AT, H_FUN);
    HLOG_DBG(QObject::tr("Calling default implementation, which does nothing."));
    mi.setKeepAlive(false);
    m_httpHandler.responseMethodNotAllowed(mi);
}

void HHttpServer::incomingControlRequest(
    MessagingInfo& mi, const InvokeActionRequest&)
{
    HLOG(H_AT, H_FUN);
    HLOG_DBG(QObject::tr("Calling default implementation, which does nothing."));
    mi.setKeepAlive(false);
    m_httpHandler.responseMethodNotAllowed(mi);
}

void HHttpServer::incomingNotifyMessage(
    MessagingInfo& mi, const NotifyRequest&)
{
    HLOG(H_AT, H_FUN);
    HLOG_DBG(QObject::tr("Calling default implementation, which does nothing."));
    mi.setKeepAlive(false);
    m_httpHandler.responseMethodNotAllowed(mi);
}

void HHttpServer::incomingUnknownHeadRequest(
    MessagingInfo& mi, const QHttpRequestHeader&)
{
    HLOG(H_AT, H_FUN);
    HLOG_DBG(QObject::tr("Calling default implementation, which does nothing."));
    mi.setKeepAlive(false);
    m_httpHandler.responseMethodNotAllowed(mi);
}

void HHttpServer::incomingUnknownGetRequest(
    MessagingInfo& mi, const QHttpRequestHeader&)
{
    HLOG(H_AT, H_FUN);
    HLOG_DBG(QObject::tr("Calling default implementation, which does nothing."));
    mi.setKeepAlive(false);
    m_httpHandler.responseMethodNotAllowed(mi);
}

void HHttpServer::incomingUnknownPostRequest(
    MessagingInfo& mi, const QHttpRequestHeader&, const QByteArray&)
{
    HLOG(H_AT, H_FUN);
    HLOG_DBG(QObject::tr("Calling default implementation, which does nothing."));
    mi.setKeepAlive(false);
    m_httpHandler.responseMethodNotAllowed(mi);
}

}
}
