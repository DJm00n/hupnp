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

#include "hhttp_messaginginfo_p.h"

#include <QUrl>
#include <QList>
#include <QTcpSocket>
#include <QHostAddress>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * ChunkedInfo
 ******************************************************************************/
ChunkedInfo::ChunkedInfo() :
    m_maxChunkSize(0), m_minChunkSize(0)
{
}

/*******************************************************************************
 * MessagingInfo
 ******************************************************************************/
MessagingInfo::MessagingInfo(
    QTcpSocket& sock, qint32 receiveTimeoutForNoData) :
        m_sock(sock), m_keepAlive(false),
        m_receiveTimeoutForNoData(receiveTimeoutForNoData),
        m_chunkedInfo(), m_autoDelete(true)
{
}

MessagingInfo::MessagingInfo(
    QTcpSocket& sock, bool keepAlive, qint32 receiveTimeoutForNoData) :
        m_sock(sock), m_keepAlive(keepAlive),
        m_receiveTimeoutForNoData(receiveTimeoutForNoData), m_autoDelete(true)
{
}

void MessagingInfo::setHostInfo(const QUrl& hostInfo)
{
    QString tmp(hostInfo.host());

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

QString MessagingInfo::lastErrorDescription() const
{
    return m_lastErrorDescription.isEmpty() ? m_sock.errorString() :
           m_lastErrorDescription;
}

}
}
