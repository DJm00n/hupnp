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

#include "./../hmulticast_socket.h"
#include "./../../../utils/hlogger_p.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <QNetworkProxy>

namespace Herqq
{

namespace Upnp
{

//
//
//
class HMulticastSocketPrivate
{
public:

};

HMulticastSocket::HMulticastSocket(QObject *parent) :
    QUdpSocket(parent), h_ptr(new HMulticastSocketPrivate())
{
    HLOG(H_AT, H_FUN);

    setProxy(QNetworkProxy::NoProxy);
}

HMulticastSocket::~HMulticastSocket()
{
    HLOG(H_AT, H_FUN);
    delete h_ptr;
}

HMulticastSocket::HMulticastSocket(HMulticastSocketPrivate& dd, QObject* parent) :
    QUdpSocket(parent), h_ptr(&dd)
{
    HLOG(H_AT, H_FUN);

    setProxy(QNetworkProxy::NoProxy);
}

bool HMulticastSocket::bind(quint16 port)
{
    HLOG(H_AT, H_FUN);

    return QUdpSocket::bind(
        port, QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress);
}

bool HMulticastSocket::bind(const QHostAddress& addressToBind, quint16 port)
{
    HLOG(H_AT, H_FUN);

    return QUdpSocket::bind(
        addressToBind, port,
        QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress);
}

bool HMulticastSocket::joinMulticastGroup(const QHostAddress &address)
{
    HLOG(H_AT, H_FUN);

    if (address.protocol() != QAbstractSocket::IPv4Protocol)
    {
        // TODO: IPv6 multicast
        HLOG_WARN(QObject::tr("IPv6 is not supported."));
        setSocketError(QAbstractSocket::UnknownSocketError);
        return false;
    }
    if (proxy().type() != QNetworkProxy::NoProxy)
    {
        // TODO: Proxied multicast
        HLOG_WARN(QObject::tr("Proxied multicast is not supported."));
        setSocketError(QAbstractSocket::UnknownSocketError);
        return false;
    }

    if (socketDescriptor() == -1)
    {
        HLOG_WARN(QObject::tr("Socket descriptor is invalid."));
        setSocketError(QAbstractSocket::UnknownSocketError);
        return false;
    }

    struct ip_mreq mreq;
    memset(&mreq,0,sizeof(struct ip_mreq));

    mreq.imr_multiaddr.s_addr = inet_addr(address.toString().toUtf8());
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(
            socketDescriptor(),
            IPPROTO_IP,
            IP_ADD_MEMBERSHIP,
            reinterpret_cast<char*>(&mreq),
            sizeof(struct ip_mreq)) < 0)
    {
        HLOG_WARN(QObject::tr("Failed to join the specified group @ %1.").arg(address.toString()));
        setSocketError(QAbstractSocket::UnknownSocketError);
        return false;
    }

    return true;
}

bool HMulticastSocket::leaveMulticastGroup(const QHostAddress &address)
{
    HLOG(H_AT, H_FUN);

    if (address.protocol() != QAbstractSocket::IPv4Protocol)
    {
        // TODO: IPv6 multicast
        HLOG_WARN(QObject::tr("IPv6 is not supported."));
        setSocketError(QAbstractSocket::UnknownSocketError);
        return false;
    }
    if (proxy().type() != QNetworkProxy::NoProxy)
    {
        // TODO: Proxied multicast
        HLOG_WARN(QObject::tr("Proxied multicast is not supported."));
        setSocketError(QAbstractSocket::UnknownSocketError);
        return false;
    }

    if (socketDescriptor() == -1)
    {
        HLOG_WARN(QObject::tr("Socket descriptor is invalid."));
        setSocketError(QAbstractSocket::UnknownSocketError);
        return false;
    }

    struct ip_mreq mreq;
    memset(&mreq,0,sizeof(struct ip_mreq));

    mreq.imr_multiaddr.s_addr = inet_addr(address.toString().toUtf8());
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(
            socketDescriptor(),
            IPPROTO_IP,
            IP_DROP_MEMBERSHIP,
            reinterpret_cast<char*>(&mreq),
            sizeof(struct ip_mreq)) < 0)
    {
        HLOG_WARN(QObject::tr("Failed to leave the specified group."));
        setSocketError(QAbstractSocket::UnknownSocketError);
        return false;
    }

    return true;
}

bool HMulticastSocket::setMulticastTtl(quint8 ttl)
{
    HLOG(H_AT, H_FUN);

    if (socketDescriptor() == -1)
    {
        HLOG_WARN(QObject::tr("Socket descriptor is invalid."));
        setSocketError(QAbstractSocket::UnknownSocketError);
        return false;
    }

    if (setsockopt(
            socketDescriptor(),
            IPPROTO_IP,
            IP_MULTICAST_TTL,
            reinterpret_cast<char*>(&ttl),
            sizeof(ttl)) < 0)
    {
        HLOG_WARN(QObject::tr("Could not set multicast TTL to the specified value."));
        setSocketError(QAbstractSocket::UnknownSocketError);
        return false;
    }

    return true;
}

}
}
