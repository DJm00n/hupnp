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

#ifndef HTTP_MESSAGINGINFO_P_H_
#define HTTP_MESSAGINGINFO_P_H_

#include "./../general/hdefs_p.h"

#include <QString>
#include <QAtomicInt>

class QUrl;
class QTcpSocket;

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

namespace Herqq
{

namespace Upnp
{

//
//
//
class H_UPNP_CORE_EXPORT ChunkedInfo
{
public:

    ChunkedInfo();

    QAtomicInt m_maxChunkSize;
    // if this is non-zero, it means that chunked-encoding should be used
    // if the data to be sent is larger than that of the specified max chunk size
    // and that the max chunk size is this

    QAtomicInt m_minChunkSize;
    // if this is non-zero, it means that when the size of the data to be sent
    // is not known in advance, how big _at least_ each chunk must be in size.
};

//
//
//
class H_UPNP_CORE_EXPORT MessagingInfo
{
H_DISABLE_COPY(MessagingInfo)

private:

    QTcpSocket& m_sock;

    bool    m_keepAlive;
    qint32  m_receiveTimeoutForNoData;

    ChunkedInfo m_chunkedInfo;

    QString m_hostInfo;

    QString m_lastErrorDescription;

    volatile bool m_autoDelete;

public:

     //
    // The default timeout in milliseconds that is waited before a read operation
    // is terminated unless _some_ data is received (not necessarily the desired amount).
    //
    static inline qint32 defaultReceiveTimeoutForNoData()
    {
        const qint32 retVal = 5000;
        return retVal;
    }

    explicit MessagingInfo(
        QTcpSocket& sock,
        qint32 receiveTimeoutForNoData = defaultReceiveTimeoutForNoData());

    MessagingInfo(
        QTcpSocket& sock, bool keepAlive,
        qint32 receiveTimeoutForNoData = defaultReceiveTimeoutForNoData());

    inline void setAutoDelete(bool b)
    {
        m_autoDelete = b;
    }

    inline bool autoDelete() const
    {
        return m_autoDelete;
    }

    inline QTcpSocket& socket() const
    {
        return m_sock;
    }

    inline void setKeepAlive(bool arg)
    {
        m_keepAlive = arg;
    }

    inline bool keepAlive() const
    {
        return m_keepAlive;
    }

    void setHostInfo(const QUrl& hostInfo);

    inline void setHostInfo(const QString& hostInfo)
    {
        m_hostInfo = hostInfo.trimmed();
    }

    QString hostInfo() const;

    inline void setReceiveTimeoutForNoData(qint32 arg)
    {
        m_receiveTimeoutForNoData = arg;
    }

    inline qint32 receiveTimeoutForNoData() const
    {
        return m_receiveTimeoutForNoData;
    }

    inline ChunkedInfo& chunkedInfo()
    {
        return m_chunkedInfo;
    }

    inline void setLastErrorDescription(const QString& errDescr)
    {
        m_lastErrorDescription = errDescr;
    }

    QString lastErrorDescription() const;
};


}
}

#endif /* HTTP_MESSAGINGINFO_P_H_ */
