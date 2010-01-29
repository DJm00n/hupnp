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


#include "endpoint.h"

#include <QUrl>
#include <QMetaType>
#include <QHostAddress>

static bool registerMetaTypes()
{
    static QAtomicInt tester(0);

    if (tester.testAndSetAcquire(0, 1))
    {
        qRegisterMetaType<Herqq::Upnp::HEndpoint>("Herqq::Upnp::HEndpoint");
    }

    return true;
}

static bool test = registerMetaTypes();

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HEndpointPrivate
 ******************************************************************************/
class HEndpointPrivate
{
public:

    QHostAddress m_hostAddress;
    quint16      m_portNumber;

public:

    HEndpointPrivate(const QHostAddress& hostAddress, quint16 portNumber) :
        m_hostAddress(hostAddress), m_portNumber(hostAddress == QHostAddress::Null ? 0 : portNumber)
    {
    }

    HEndpointPrivate(const QHostAddress& hostAddress) :
        m_hostAddress(hostAddress), m_portNumber(0)
    {
    }

    HEndpointPrivate() :
        m_hostAddress(QHostAddress::Null), m_portNumber(0)
    {
    }

    HEndpointPrivate(const QUrl& url) :
        m_hostAddress(QHostAddress(url.host())),
        m_portNumber(m_hostAddress == QHostAddress::Null ? 0 : url.port())
    {
    }

    HEndpointPrivate(const QString& arg) :
        m_hostAddress(), m_portNumber(0)
    {
        qint32 delim = arg.indexOf(':');
        if (delim < 0)
        {
            m_hostAddress = arg;
        }
        else
        {
            m_hostAddress = arg.left(delim);
            if (m_hostAddress == QHostAddress::Null)
            {
                m_portNumber = 0;
            }
            else
            {
                m_portNumber  = arg.mid(delim+1).toUShort();
            }
        }
    }

};

/*******************************************************************************
 * HEndpoint
 ******************************************************************************/
HEndpoint::HEndpoint(const QHostAddress& hostAddress, quint16 portNumber) :
    h_ptr(new HEndpointPrivate(hostAddress, portNumber))
{
}

HEndpoint::HEndpoint(const QHostAddress& hostAddress) :
    h_ptr(new HEndpointPrivate(hostAddress))
{
}

HEndpoint::HEndpoint() :
    h_ptr(new HEndpointPrivate())
{
}

HEndpoint::HEndpoint(const QUrl& url) :
    h_ptr(new HEndpointPrivate(url))
{
}

HEndpoint::HEndpoint(const QString& arg) :
    h_ptr(new HEndpointPrivate(arg))
{
}

HEndpoint::HEndpoint(const HEndpoint& ep) :
    h_ptr(new HEndpointPrivate(*ep.h_ptr))
{
}

HEndpoint& HEndpoint::operator=(const HEndpoint& ep)
{
    HEndpointPrivate* newHptr = new HEndpointPrivate(*ep.h_ptr);

    delete h_ptr;
    h_ptr = newHptr;

    return *this;
}

HEndpoint::~HEndpoint()
{
    delete h_ptr;
}

bool HEndpoint::isNull() const
{
    return h_ptr->m_hostAddress.isNull();
}

QHostAddress HEndpoint::hostAddress() const
{
    return h_ptr->m_hostAddress;
}

quint16 HEndpoint::portNumber() const
{
    return h_ptr->m_portNumber;
}

bool HEndpoint::isMulticast() const
{
    qint32 ipaddr = h_ptr->m_hostAddress.toIPv4Address();
    return ipaddr & 0xe0000000 || ipaddr & 0xe8000000 || ipaddr & 0xef000000;
}

QString HEndpoint::toString() const
{
    return h_ptr->m_hostAddress.toString().append(":").append(
           QString::number(h_ptr->m_portNumber));
}

bool operator==(const HEndpoint& ep1, const HEndpoint& ep2)
{
    return ep1.h_ptr->m_hostAddress == ep2.h_ptr->m_hostAddress &&
           ep1.h_ptr->m_portNumber  == ep2.h_ptr->m_portNumber;
}

bool operator!=(const HEndpoint& ep1, const HEndpoint& ep2)
{
    return !(ep1 == ep2);
}

}
}
