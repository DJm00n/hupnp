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
            m_mi(mi), m_dataToSend(data), m_dataSend(0), m_state(0),
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
}

HHttpAsyncOperation::~HHttpAsyncOperation()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    delete m_mi;
}

void HHttpAsyncOperation::bytesWritten(qint64 bw)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    if (m_state != 1)
    {
        return;
    }

    m_dataSend += bw;
    if (m_dataSend < m_dataToSend.size())
    {
        return;
    }

    m_state = 2;
    // the first part (send()) of the msgIO operation is now done
    // expecting data next
}

void HHttpAsyncOperation::readRequestData()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QByteArray buf; buf.resize(m_dataToRead+1);
    do
    {
        qint64 retVal = m_mi->socket().read(
            buf.data(), qMin(static_cast<qint64>(buf.size()), m_dataToRead));

        if (retVal < 0)
        {
            emit done(m_uuid);
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
        m_state = 4;
        emit done(m_uuid);
    }
}

void HHttpAsyncOperation::state2_readHeader()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    if (!HHttpUtils::readHttpHeader(m_mi->socket(), m_dataRead))
    {
        return;
    }

    m_headerRead = QHttpResponseHeader(QString::fromUtf8(m_dataRead));

    m_dataRead.clear();

    if (!m_headerRead.isValid())
    {
        m_state = -1;
        emit done(m_uuid);
        return;
    }

    if (m_headerRead.hasContentLength())
    {
        quint32 clength = m_headerRead.contentLength();
        m_dataToRead = clength;
        if (m_dataToRead == 0)
        {
            m_state = 4;
            emit done(m_uuid);
        }
    }

    m_state = 3;
}

void HHttpAsyncOperation::state3_readData()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    if (!m_mi->socket().bytesAvailable())
    {
        return;
    }

    QByteArray body;
    bool chunked = m_headerRead.value("TRANSFER-ENCODING") == "chunked";
    if (chunked)
    {
        if (m_headerRead.hasContentLength())
        {
            m_state = -1;
            emit done(m_uuid);
            return;
        }

        Q_ASSERT(false);
        //readChunkedRequest(mi);
    }
    else
    {
        if (m_headerRead.hasContentLength())
        {
            readRequestData();
        }
        else
        {
            // not chunked and content length is not specified ==>
            // no way to know what to expect ==> read all that is available
            body = m_mi->socket().readAll();
            m_dataRead.append(body);
            m_state = 4;
            emit done(m_uuid);
        }
    }
}

void HHttpAsyncOperation::readyRead()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    if (m_state == 2)
    {
        state2_readHeader();
    }

    if (m_state == 3)
    {
        state3_readData();
    }
}

bool HHttpAsyncOperation::run()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    qint64 dataSent = m_mi->socket().write(m_dataToSend);

    if (dataSent < 0)
    {
        return false;
    }

    m_state = 1; // "writing" started

    m_mi->socket().flush();
    return true;
}

HHttpAsyncOperation::State HHttpAsyncOperation::state() const
{
    switch(m_state)
    {
    case -1:
        return Failed;
    case 0:
        return NotStarted;
    case 1:
        return Writing;
    case 2:
    case 3:
        return Reading;
    case 4:
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
