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

#include "hhttp_asynchandler_p.h"
#include "hhttp_messagecreator_p.h"
#include "hhttp_utils_p.h"

#include "./../../utils/hlogger_p.h"
#include "./../general/hupnp_global_p.h"

#include <QTcpSocket>
#include <QtSoapMessage>

namespace Herqq
{

namespace Upnp
{

HHttpAsyncOperation::HHttpAsyncOperation(
    const QByteArray& loggingIdentifier, MessagingInfo* mi,
    const QByteArray& data, QObject* parent) :
        QObject(parent),
            m_mi(mi), m_dataToSend(data), m_dataSend(0), m_dataSent(0),
            m_state(Internal_NotStarted),
            m_dataRead(), m_dataToRead(0), m_uuid(QUuid::createUuid()),
            m_loggingIdentifier(loggingIdentifier)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    bool ok = connect(&m_mi->socket(), SIGNAL(bytesWritten(qint64)),
        this, SLOT(bytesWritten(qint64)));

    Q_ASSERT(ok); Q_UNUSED(ok)

    ok = connect(&m_mi->socket(), SIGNAL(readyRead()),
        this, SLOT(readyRead()));

    Q_ASSERT(ok);

    ok = connect(&m_mi->socket(), SIGNAL(error(QAbstractSocket::SocketError)),
        this, SLOT(error(QAbstractSocket::SocketError)));

    Q_ASSERT(ok);
}

HHttpAsyncOperation::~HHttpAsyncOperation()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    delete m_mi;
}

void HHttpAsyncOperation::sendChunked()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    static const char crlf[] = {"\r\n"};

    // then start sending the data in chunks
    qint64 bytesWritten = 0;

    if (m_dataSent < m_dataToSend.size())
    {
        qint32 dataToSendSize =
            m_dataSend > 0 ? m_dataSend :
                qMin(m_dataToSend.size() - m_dataSent,
                    static_cast<qint64>(m_mi->chunkedInfo().m_maxChunkSize));

        if (m_state == Internal_WritingChunkedSizeLine)
        {
            // write the size line of the next chunk
            QByteArray sizeLine;
            sizeLine.setNum(dataToSendSize, 16);
            sizeLine.append(crlf, 2);

            bytesWritten = m_mi->socket().write(sizeLine);
            if (bytesWritten != sizeLine.size())
            {
                m_mi->setLastErrorDescription(QObject::tr(
                    "Failed to send data to %1.").arg(peerAsStr(m_mi->socket())));

                done_(Internal_Failed);
                return;
            }

            m_state = Internal_WritingChunk;
        }

        // write the chunk
        bytesWritten =
            m_mi->socket().write(m_dataToSend.data() + m_dataSent, dataToSendSize);

        if (bytesWritten < 0)
        {
            m_mi->setLastErrorDescription(
                QObject::tr("Failed to send data to %1.").arg(
                    peerAsStr(m_mi->socket())));

            done_(Internal_Failed);
            return;
        }

        m_dataSent += bytesWritten;

        if (bytesWritten != dataToSendSize)
        {
            m_dataSend = dataToSendSize - bytesWritten;

            // wait for bytesWritten() and then attempt to send the data remaining
            // in the chunk
            return;
        }
        else
        {
            m_dataSend = 0;
        }

        // and after the chunk, write the trailing crlf and start again if there's
        // chunks left
        bytesWritten = m_mi->socket().write(crlf, 2);
        if (bytesWritten != 2)
        {
            m_mi->setLastErrorDescription(
                QObject::tr("Failed to send data to %1.").arg(
                    peerAsStr(m_mi->socket())));

            done_(Internal_Failed);
            return;
        }

        m_state = Internal_WritingChunkedSizeLine;
    }

    if (m_dataSent >= m_dataToSend.size())
    {
        // write the "eof" == zero + crlf
        const char eof[] = "0\r\n";
        m_mi->socket().write(&eof[0], 3);

        m_state = Internal_ReadingHeader;
        m_mi->socket().flush();
    }
}

void HHttpAsyncOperation::readBlob()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QByteArray buf; buf.resize(m_dataToRead+1);
    do
    {
        qint64 retVal = m_mi->socket().read(
            buf.data(), qMin(static_cast<qint64>(buf.size()), m_dataToRead));

        if (retVal < 0)
        {
            done_(Internal_Failed);
            return;
        }
        else if (retVal > 0)
        {
            m_dataToRead -= retVal;
            m_dataRead.append(QByteArray(buf.data(), retVal));
        }
        else
        {
            break;
        }
    }
    while(m_dataToRead > 0);

    if (m_dataToRead <= 0)
    {
        done_(Internal_FinishedSuccessfully);
    }
}

bool HHttpAsyncOperation::readChunkedSizeLine()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    if (m_mi->socket().bytesAvailable() <= 0)
    {
        return false;
    }

    QByteArray buf;
    if (!HHttpUtils::readLines(m_mi->socket(), buf, 1))
    {
        // No size line. It should be available at this point.
        m_mi->setLastErrorDescription(QObject::tr("Missing chunk-size line."));

        done_(Internal_Failed);
        return false;
    }

    qint32 endOfSize = buf.indexOf(';');
    if (endOfSize < 0)
    {
        // no extensions
        endOfSize = buf.size() - 2; // 2 == crlf
    }
    QByteArray sizeLine = buf.left(endOfSize);

    bool ok = false;
    qint32 chunkSize = sizeLine.toInt(&ok, 16);
    if (!ok || chunkSize < 0)
    {
        m_mi->setLastErrorDescription(
            QObject::tr("Invalid chunk-size line: %1.").arg(
                  QString::fromUtf8(sizeLine)));

        done_(Internal_Failed);
        return false;
    }

    if (chunkSize == 0)
    {
        // the last chunk, ignore possible trailers
        done_(Internal_FinishedSuccessfully);
        return false;
    }

    m_dataToRead = chunkSize;
    m_state = Internal_ReadingChunk;

    return true;
}

bool HHttpAsyncOperation::readChunk()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QByteArray tmp;
    tmp.resize(m_dataToRead);

    qint32 read = m_mi->socket().read(tmp.data(), tmp.size());

    if (read < 0)
    {
        m_mi->setLastErrorDescription(QObject::tr(
            "Failed to read chunk: %1").arg(m_mi->socket().errorString()));

        done_(Internal_Failed);
        return false;
    }
    else if (read == 0)
    {
        // couldn't read the entire chunk in one pass
        return false;
    }

    tmp.resize(read);
    m_dataRead.append(tmp);

    m_dataToRead -= read;
    if (m_dataToRead > 0)
    {
        // couldn't read the entire chunk in one pass
        return false;
    }

    // if here, the entire chunk data is read.
    // clear the remaining crlf and move to the next chunk

    char c;
    m_mi->socket().getChar(&c);
    m_mi->socket().getChar(&c);

    m_state = Internal_ReadingChunkSizeLine;

    return true;
}

void HHttpAsyncOperation::readHeader()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    if (!HHttpUtils::readLines(m_mi->socket(), m_dataRead, 2))
    {
        done_(Internal_Failed);
        return;
    }

    m_headerRead = QHttpResponseHeader(QString::fromUtf8(m_dataRead));

    m_dataRead.clear();

    if (!m_headerRead.isValid())
    {
        done_(Internal_Failed);
        return;
    }

    if (m_headerRead.hasContentLength())
    {
        m_dataToRead = m_headerRead.contentLength();
        if (m_dataToRead == 0)
        {
            done_(Internal_FinishedSuccessfully);
            return;
        }
    }

    m_state = Internal_ReadingData;
}

void HHttpAsyncOperation::readData()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    if (!m_mi->socket().bytesAvailable())
    {
        return;
    }

    bool chunked = m_headerRead.value("TRANSFER-ENCODING") == "chunked";
    if (chunked)
    {
        if (m_headerRead.hasContentLength())
        {
            done_(Internal_Failed);
            return;
        }

        m_state = Internal_ReadingChunkSizeLine;
    }
    else
    {
        if (m_headerRead.hasContentLength())
        {
            readBlob();
        }
        else
        {
            // not chunked and content length is not specified ==>
            // no way to know what to expect ==> read all that is available
            QByteArray body = m_mi->socket().readAll();
            m_dataRead.append(body);

            done_(Internal_FinishedSuccessfully);
        }
    }
}

bool HHttpAsyncOperation::run()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    qint32 indexOfData = m_dataToSend.indexOf("\r\n\r\n");
    Q_ASSERT(indexOfData > 0);

    if (m_mi->chunkedInfo().m_maxChunkSize > 0 &&
        m_dataToSend.size() - indexOfData > m_mi->chunkedInfo().m_maxChunkSize)
    {
        // send the http header first (it is expected that the header has been
        // properly setup for chunked transfer, as it should be, since this is
        // private stuff not influenced by public input)

        qint32 endOfHdr = m_dataToSend.indexOf("\r\n\r\n") + 4;
        m_dataSent = m_mi->socket().write(m_dataToSend.data(), endOfHdr);

        if (m_dataSent != endOfHdr)
        {
            m_mi->setLastErrorDescription(QObject::tr(
                "Failed to send HTTP header to %1").arg(
                    peerAsStr(m_mi->socket())));

            done_(Internal_Failed);
            return false;
        }

        m_state = Internal_WritingChunkedSizeLine;
        sendChunked();
    }
    else
    {
        m_dataSent = m_mi->socket().write(m_dataToSend);

        if (m_dataSent < 0)
        {
            return false;
        }

        m_state = Internal_WritingBlob;
    }

    return true;
}

void HHttpAsyncOperation::done_(InternalState state)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    m_mi->socket().disconnect(this);

    m_state = state;
    emit done(m_uuid);
}

void HHttpAsyncOperation::bytesWritten(qint64)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    if (m_state == Internal_WritingBlob)
    {
        if (m_dataSent < m_dataToSend.size())
        {
            qint64 dataSent = m_mi->socket().write(
                m_dataToSend.data() + m_dataSent,
                m_dataToSend.size() - m_dataSent);

            if (dataSent < 0)
            {
                m_mi->setLastErrorDescription(QObject::tr(
                    "Failed to send data to %1").arg(peerAsStr(m_mi->socket())));

                done_(Internal_Failed);
                return;
            }

            m_dataSent += dataSent;
            if (m_dataSent < m_dataToSend.size())
            {
                return;
            }
        }

        m_state = Internal_ReadingHeader;
    }
    else if (m_state == Internal_WritingChunk ||
             m_state == Internal_WritingChunkedSizeLine)
    {
        sendChunked();
    }
}

void HHttpAsyncOperation::readyRead()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    if (m_state == Internal_ReadingHeader)
    {
        readHeader();
    }

    if (m_state == Internal_ReadingData)
    {
        readData();
    }

    for(; m_state == Internal_ReadingChunkSizeLine ||
          m_state == Internal_ReadingChunk;)
    {
        // the request contained chunked data

        if (m_state == Internal_ReadingChunkSizeLine)
        {
            if (!readChunkedSizeLine())
            {
                // no more data available at the moment
                return;
            }
        }

        if (m_state == Internal_ReadingChunk)
        {
            if (!readChunk())
            {
                // no more data available at the moment
                return;
            }
        }
    }
}

void HHttpAsyncOperation::error(QAbstractSocket::SocketError err)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    if (m_state != Internal_FinishedSuccessfully &&
        err == QAbstractSocket::RemoteHostClosedError)
    {
        QByteArray remainingData = m_mi->socket().readAll();
        m_dataRead.append(remainingData);
        m_dataToRead -= remainingData.size();
        if (m_dataToRead <= 0)
        {
            done_(Internal_FinishedSuccessfully);
            return;
        }
    }

    done_(Internal_Failed);
}

HHttpAsyncOperation::State HHttpAsyncOperation::state() const
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    switch(m_state)
    {
    case Internal_Failed:
        return Failed;

    case Internal_NotStarted:
        return NotStarted;

    case Internal_WritingBlob:
    case Internal_WritingChunkedSizeLine:
    case Internal_WritingChunk:
        return Writing;

    case Internal_ReadingHeader:
    case Internal_ReadingData:
    case Internal_ReadingChunkSizeLine:
    case Internal_ReadingChunk:
        return Reading;

    case Internal_FinishedSuccessfully:
        return Succeeded;

    default:
        Q_ASSERT(false);
        return Failed;
    }
}

/*******************************************************************************
 * HHttpAsyncHandler
 ******************************************************************************/
HHttpAsyncHandler::HHttpAsyncHandler(
    const QByteArray& loggingIdentifier, QObject* parent) :
        QObject(parent),
            m_loggingIdentifier(loggingIdentifier), m_operations()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
}

HHttpAsyncHandler::~HHttpAsyncHandler()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
}

void HHttpAsyncHandler::done(const QUuid& uuid)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    HHttpAsyncOperation* ao = m_operations.value(uuid);
    Q_ASSERT(ao);
    Q_ASSERT(ao->state() != HHttpAsyncOperation::NotStarted);

    m_operations.remove(uuid);

    emit msgIoComplete(ao);
}

HHttpAsyncOperation* HHttpAsyncHandler::msgIo(
    MessagingInfo* mi, const QByteArray& req)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(mi);
    Q_ASSERT(!req.isEmpty());

    HHttpAsyncOperation* ao =
        new HHttpAsyncOperation(m_loggingIdentifier, mi, req, this);

    bool ok = connect(ao, SIGNAL(done(QUuid)), this, SLOT(done(QUuid)));
    Q_ASSERT(ok); Q_UNUSED(ok)

    m_operations.insert(ao->uuid(), ao);

    if (!ao->run())
    {
        m_operations.remove(ao->uuid());
        delete ao;
        return 0;
    }

    return ao;
}

HHttpAsyncOperation* HHttpAsyncHandler::msgIo(
    MessagingInfo* mi, QHttpRequestHeader& reqHdr, const QtSoapMessage& soapMsg)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QByteArray dataToSend =
        HHttpMessageCreator::setupData(
            reqHdr, soapMsg.toXmlString().toUtf8(), *mi, TextXml);

    return msgIo(mi, dataToSend);
}

}
}
