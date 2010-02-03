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

#include "ssdp.h"
#include "ssdp_p.h"

#include "usn.h"
#include "endpoint.h"
#include "product_tokens.h"
#include "discovery_messages.h"
#include "resource_identifier.h"
#include "ssdp_messageheader_objects_p.h"

#include "../../../../utils/src/logger_p.h"
#include "../../../../core/include/HExceptions"

#include <QUrl>
#include <QString>
#include <QDateTime>
#include <QHostAddress>
#include <QMutexLocker>
#include <QNetworkInterface>
#include <QHttpRequestHeader>
#include <QHttpResponseHeader>

/*!
 * \defgroup ssdp Ssdp
 *
 * \brief This page provides information about the HUPnP's classes that provide the
 * SSDP functionality required for the discovery phase of the UPnP Device Architecture.
 *
 * According to the UPnP Device Architecture specification version 1.1,
 * <em>When a device is added to the network, the UPnP discovery protocol allows that
 * device to advertise its services to control points on the network. Similarly,
 * when a control point is added to the network, the UPnP discovery protocol allows
 * that control point to search for devices of interest on the network</em> (p. 19).
 *
 * The mentioned <em>discovery protocol</em> is SSDP and it is about exchanging
 * HTTP messages over UDP.
 *
 * \note As mentioned in the Herqq::Upnp::HSsdp, these classes implement the SSDP
 * as it is required by the UDA specification. The IETF SSDP draft is not
 * implemented in full.
 *
 * To send or receive SSDP messages, you need to use Herqq::Upnp::HSsdp
 * class. You can either derive from it or use it directly. In either case,
 * sending messages is straightforward:
 *
 * \code
 *
 * Herqq::Upnp::HSsdp ssdp;
 *
 * Herqq::Upnp::HResourceAvailable deviceAvailable(
 *       1800, // how long the advertisement is valid in seconds
 *       QUrl("127.0.0.1:1900/mydevice"), // where the device description can be downloaded
 *       Herqq::Upnp::HProductTokens("unix/5.1 UPnP/1.1 MyProduct/1.0"), // some information about the host and product
 *       Herqq::Upnp::HUsn("uuid:5d724fc2-5c5e-4760-a123-f04a9136b300::upnp:rootdevice")); // universally unique identifier
 *
 * ssdp.announcePresence(deviceAvailable);
 *
 * \endcode
 *
 * The above example sends a single <em>ssdp:alive</em> message indicating that a
 * UPnP root device is now available.
 *
 * \note All SSDP classes validate the provided information during
 * object construction. For instance, if the argument to the Herqq::Upnp::HUsn
 * is invalid, the constructed object will be invalid as well,
 * e.g Herqq::Upnp::HUsn::isValid() returns false. In such a case, the creation
 * of Herqq::Upnp::HResourceAvailable will fail and consequenlty, the \c %HSsdp will
 * not send anything.
 *
 * Receiving messages is almost as simple. You can use the class directly, in which
 * case you have to connect to the exposed signals. For instance, to receive signals
 * when <em>resource available</em> messages are received, you should do:
 *
 * \code
 *
 * MyClass::MyClass(QObject* parent) :
 *     QObject(parent), m_ssdp(new Herqq::Upnp::HSsdp())
 * {
 *     connect(
 *         m_ssdp, SIGNAL(resourceAvailableReceived(Herqq::Upnp::HResourceAvailable)),
 *         this  , SLOT  (resourceAvailableReceived(Herqq::Upnp::HResourceAvailable)));
 * }
 *
 * MyClass::resourceAvailableReceived(const Herqq::Upnp::HResourceAvailable&)
 * {
 *     // do something
 * }
 *
 * \endcode
 *
 * Note the used signature with the SIGNAL and SLOT macros. You can also derive
 * from \c %HSsdp, in which case you can override the various \c protected \c virtual
 * methods that will be called upon message reception.
 *
 * \attention
 * Usually you have no need to use the classes in this module directly. These classes
 * may be useful in case you are writing your own device host or control point.
 * Otherwise, the Herqq::Upnp::HControlPoint and Herqq::Upnp::HDeviceHost classes
 * may suit your needs better.
 */

namespace Herqq
{

namespace Upnp
{

inline static QHostAddress multicastAddress()
{
    static QHostAddress retVal("239.255.255.250");
    return retVal;
}

inline static qint16 multicastPort()
{
    static qint16 retVal = 1900;
    return retVal;
}

/*******************************************************************************
 * HSsdpPrivate
 ******************************************************************************/
HSsdpPrivate::HSsdpPrivate() :
    m_loggingIdentifier("__SSDP__: "),
    m_multicastSocket(),
    m_unicastSocket  (),
    q_ptr            (0)
{
}

HSsdpPrivate::~HSsdpPrivate()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
}

qint32 HSsdpPrivate::parseCacheControl(const QString& str)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    bool ok = false;

    QString cacheControl = str.simplified();
    QStringList slist = cacheControl.split('=');

    if (slist.size() != 2 || slist[0].simplified() != "max-age")
    {
        throw HParseException(
            QObject::tr("Invalid Cache-Control field value: %1").arg(str));
    }

    qint32 maxAge = slist[1].simplified().toInt(&ok);
    if (!ok)
    {
        throw HParseException(
            QObject::tr("Invalid Cache-Control field value: %1").arg(str));
    }

    return maxAge;
}

void HSsdpPrivate::checkHost(const QString& host)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QStringList slist = host.split(':');
    if (slist.size() < 1 || slist[0].simplified() != "239.255.255.250")
    {
        throw HParseException(
            QObject::tr("HOST header field is invalid: %1").arg(host));
    }
}

HDiscoveryResponse HSsdpPrivate::parseDiscoveryResponse(const QHttpResponseHeader& hdr)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QString   cacheControl  = hdr.value("CACHE-CONTROL");
    QDateTime date          = QDateTime::fromString(hdr.value("DATE"));
    QUrl      location      = hdr.value("LOCATION");
    QString   server        = hdr.value("SERVER");
    QString   st            = hdr.value("ST");
    QString   usn           = hdr.value("USN");
    QString   bootIdStr     = hdr.value("BOOTID.UPNP.ORG");
    QString   configIdStr   = hdr.value("CONFIGID.UPNP.ORG");
    QString   searchPortStr = hdr.value("SEARCHPORT.UPNP.ORG");

    if (!hdr.hasKey("EXT"))
    {
        throw HParseException(QObject::tr("EXT field is missing: %1").arg(
            hdr.toString()));
    }
    else if (!hdr.value("EXT").isEmpty())
    {
        throw HParseException(
            QObject::tr("EXT field is not empty, although it should be: %1").
                arg(hdr.toString()));
    }

    qint32 maxAge = parseCacheControl(cacheControl);

    bool ok = false;
    qint32 bootId = bootIdStr.toInt(&ok);
    if (!ok)
    {
        bootId = -1;
    }

    qint32 configId = configIdStr.toInt(&ok);
    if (!ok)
    {
        configId = -1;
    }

    qint32 searchPort = searchPortStr.toInt(&ok);
    if (!ok)
    {
        searchPort = -1;
    }

    return HDiscoveryResponse(
        maxAge, date, location, server, HUsn(usn), bootId,
        hdr.hasKey("CONFIGID.UPNP.ORG") ? configId : 0, searchPort);
        // ^^ configid is optional even in UDA v1.1 ==> cannot provide -1
        // unless the header field is specified and the value is invalid
}

HDiscoveryRequest HSsdpPrivate::parseDiscoveryRequest(const QHttpRequestHeader& hdr)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QString host = hdr.value("HOST");
    QString man  = hdr.value("MAN").simplified();

    bool    ok   = false;
    qint32  mx   = hdr.value("MX").toInt(&ok);

    if (!ok)
    {
        throw HMissingArgumentException(QObject::tr("MX is not specified."));
    }

    QString st   = hdr.value("ST");
    QString ua   = hdr.value("USER-AGENT");

    checkHost(host);

    if (man.compare(QString("\"ssdp:discover\""), Qt::CaseInsensitive) != 0)
    {
        throw HParseException(QObject::tr("MAN header field is invalid."));
    }

    return HDiscoveryRequest(mx, st, ua);
}

HResourceAvailable HSsdpPrivate::parseDeviceAvailable(const QHttpRequestHeader& hdr)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QString host          = hdr.value("HOST");
    QString server        = hdr.value("SERVER");
    QString nt            = hdr.value("NT");
    QString usn           = hdr.value("USN");
    QUrl    location      = hdr.value("LOCATION");
    QString cacheControl  = hdr.value("CACHE-CONTROL");
    QString bootIdStr     = hdr.value("BOOTID.UPNP.ORG");
    QString configIdStr   = hdr.value("CONFIGID.UPNP.ORG");
    QString searchPortStr = hdr.value("SEARCHPORT.UPNP.ORG");

    qint32 maxAge = parseCacheControl(cacheControl);

    bool ok = false;
    qint32 bootId = bootIdStr.toInt(&ok);
    if (!ok)
    {
        bootId = -1;
    }

    qint32 configId = configIdStr.toInt(&ok);
    if (!ok)
    {
        configId = -1;
    }

    checkHost(host);

    qint32 searchPort = searchPortStr.toInt(&ok);
    if (!ok)
    {
        searchPort = -1;
    }

    return HResourceAvailable(
        maxAge, location, server, usn, bootId, configId, searchPort);
}

HResourceUnavailable HSsdpPrivate::parseDeviceUnavailable(const QHttpRequestHeader& hdr)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QString host        = hdr.value("HOST");
    QString nt          = hdr.value("NT");
    QString usn         = hdr.value("USN");
    QString bootIdStr   = hdr.value("BOOTID.UPNP.ORG");
    QString configIdStr = hdr.value("CONFIGID.UPNP.ORG");

    bool ok = false;
    qint32 bootId = bootIdStr.toInt(&ok);
    if (!ok)
    {
        bootId = -1;
    }

    qint32 configId = configIdStr.toInt(&ok);
    if (!ok)
    {
        configId = -1;
    }

    checkHost(host);

    return HResourceUnavailable(usn, QUrl(), bootId, configId); // TODO
}

HResourceUpdate HSsdpPrivate::parseDeviceUpdate(const QHttpRequestHeader& hdr)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QString host          = hdr.value("HOST");
    QUrl    location      = hdr.value("LOCATION");
    QString nt            = hdr.value("NT");
    QString usn           = hdr.value("USN");
    QString bootIdStr     = hdr.value("BOOTID.UPNP.ORG");
    QString configIdStr   = hdr.value("CONFIGID.UPNP.ORG");
    QString nextBootIdStr = hdr.value("NEXTBOOTID.UPNP.ORG");
    QString searchPortStr = hdr.value("SEARCHPORT.UPNP.ORG");

    bool ok = false;
    qint32 bootId = bootIdStr.toInt(&ok);
    if (!ok)
    {
        bootId = -1;
    }

    qint32 configId = configIdStr.toInt(&ok);
    if (!ok)
    {
        configId = -1;
    }

    qint32 nextBootId = nextBootIdStr.toInt(&ok);
    if (!ok)
    {
        nextBootId = -1;
    }

    qint32 searchPort = searchPortStr.toInt(&ok);
    if (!ok)
    {
        searchPort = -1;
    }

    checkHost(host);

    return HResourceUpdate(
        location, usn, bootId, configId, nextBootId, searchPort);
}

void HSsdpPrivate::send(const QString& data)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QByteArray buf = data.toUtf8();
    qint64 retVal  = m_unicastSocket.writeDatagram(
        buf, multicastAddress(), multicastPort());

    if (retVal != buf.size())
    {
        HLOG_WARN(QObject::tr("Failed to send the packet: %1. Contents:\n%2").arg(
            m_unicastSocket.errorString(), data));
    }
}

void HSsdpPrivate::send(const QString& data, const HEndpoint& receiver)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QByteArray buf = data.toUtf8();
    qint64 retVal  = m_unicastSocket.writeDatagram(
        buf, receiver.hostAddress(), receiver.portNumber());

    if (retVal != buf.size())
    {
        HLOG_WARN(QObject::tr("Failed to send the packet to %1: %2. Contents:\n%3").arg(
            receiver.toString(), m_unicastSocket.errorString(), data));
    }
}

void HSsdpPrivate::processResponse(const QString& msg, const HEndpoint& source)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QHttpResponseHeader hdr(msg);
    if (!hdr.isValid())
    {
        HLOG_WARN(QObject::tr("Ignoring an invalid HTTP response."));
        return;
    }

    HDiscoveryResponse rcvdMsg = parseDiscoveryResponse(hdr);
    if (!q_ptr->incomingDiscoveryResponse(rcvdMsg, source))
    {
        emit q_ptr->discoveryResponseReceived(rcvdMsg, source);
    }
}

void HSsdpPrivate::processNotify(const QString& msg, const HEndpoint& /*from*/)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QHttpRequestHeader hdr(msg);
    if (!hdr.isValid())
    {
        HLOG_WARN(QObject::tr("Ignoring an invalid HTTP response."));
        return;
    }

    QString nts = hdr.value("NTS");
    if (nts.compare(QString("ssdp:alive"), Qt::CaseInsensitive) == 0)
    {
        HResourceAvailable rcvdMsg = parseDeviceAvailable(hdr);
        if (!q_ptr->incomingDeviceAvailableAnnouncement(rcvdMsg))
        {
            emit q_ptr->resourceAvailableReceived(rcvdMsg);
        }
    }
    else if (nts.compare(QString("ssdp:byebye"), Qt::CaseInsensitive) == 0)
    {
        HResourceUnavailable rcvdMsg = parseDeviceUnavailable(hdr);
        if (!q_ptr->incomingDeviceUnavailableAnnouncement(rcvdMsg))
        {
            emit q_ptr->resourceUnavailableReceived(rcvdMsg);
        }
    }
    else if (nts.compare(QString("ssdp:update"), Qt::CaseInsensitive) == 0)
    {
        HResourceUpdate rcvdMsg = parseDeviceUpdate(hdr);
        if (!q_ptr->incomingDeviceUpdateAnnouncement(rcvdMsg))
        {
            emit q_ptr->deviceUpdateReceived(rcvdMsg);
        }
    }
    else
    {
        HLOG_WARN(QObject::tr(
            "Ignoring an invalid SSDP presence announcement: [%1].").arg(nts));
    }
}

void HSsdpPrivate::processSearch(
    const QString& msg, const HEndpoint& source, const HEndpoint& destination)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QHttpRequestHeader hdr(msg);
    if (!hdr.isValid())
    {
        HLOG_WARN(QObject::tr("Ignoring an invalid HTTP M-SEARCH request."));
        return;
    }

    HDiscoveryRequest rcvdMsg = parseDiscoveryRequest(hdr);
    if (!q_ptr->incomingDiscoveryRequest(rcvdMsg, source, destination))
    {
        emit q_ptr->discoveryRequestReceived(rcvdMsg, source, destination);
    }
}

void HSsdpPrivate::init(const QHostAddress& addressToBind, HSsdp* qptr)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    HLOG_DBG(QObject::tr("Using address [%1]").arg(addressToBind.toString()));

    if (!m_multicastSocket.bind(1900))
    {
        throw HSocketException(
            QObject::tr("Failed to bind multicast socket for listening"));
    }

    if (!m_multicastSocket.joinMulticastGroup(multicastAddress()))
    {
        throw HSocketException(QObject::tr("Could not join %1").arg(
            multicastAddress().toString()));
    }

    HLOG_DBG(QObject::tr("Attempting to bind to port 1900"));

    // always attempt to bind to the 1900 first
    if (!m_unicastSocket.bind(addressToBind, 1900))
    {
        HLOG_DBG(QObject::tr("Failed. Searching suitable port."));

        // the range is specified by the UDA 1.1 standard
        for(qint32 i = 49152; i < 65535; ++i)
        {
            if (m_unicastSocket.bind(addressToBind, i))
            {
                HLOG_DBG(QObject::tr("Binding to [%1].").arg(QString::number(i)));
                break;
            }
        }
    }
    else
    {
        HLOG_DBG(QObject::tr("Success"));
    }

    if (m_unicastSocket.state() != QUdpSocket::BoundState)
    {
        throw HSocketException(
            QObject::tr("Failed to bind UDP socket for listening"));
    }

    m_multicastSocket.setParent(qptr);
    m_unicastSocket.setParent(qptr);
    q_ptr = qptr;

    bool ok = QObject::connect(
        &m_multicastSocket, SIGNAL(readyRead()),
        q_ptr, SLOT(multicastMessageReceived()));

    Q_ASSERT(ok);

    ok = QObject::connect(
        &m_unicastSocket, SIGNAL(readyRead()),
        q_ptr, SLOT(unicastMessageReceived()));

    Q_ASSERT(ok);
}

void HSsdpPrivate::messageReceived(
    const QString& msg, const HEndpoint& source, const HEndpoint& destination)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    try
    {
        if (msg.startsWith("NOTIFY * HTTP/1.1", Qt::CaseInsensitive))
        {
            // Possible presence announcement
            processNotify(msg, source);
        }
        else if (msg.startsWith("M-SEARCH * HTTP/1.1", Qt::CaseInsensitive))
        {
            // Possible discovery request.
            processSearch(msg, source, destination);
        }
        else
        {
            // Possible discovery response
            processResponse(msg, source);
        }
    }
    catch(HException& ex)
    {
        HLOG_WARN(ex.reason());
    }
}

/*******************************************************************************
 * HSsdp
 ******************************************************************************/
HSsdp::HSsdp(QObject* parent) :
    QObject(parent),
    h_ptr  (new HSsdpPrivate())
{
    HLOG(H_AT, H_FUN);

    QHostAddress addressToBind = QHostAddress::LocalHost;
    foreach (QNetworkInterface iface, QNetworkInterface::allInterfaces())
    {
        if (iface.flags() & QNetworkInterface::IsUp &&
          !(iface.flags() & QNetworkInterface::IsLoopBack))
        {
            QList<QNetworkAddressEntry> entries = iface.addressEntries();
            foreach(QNetworkAddressEntry entry, entries)
            {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol)
                {
                    addressToBind = entry.ip();
                    goto end;
                }
            }
        }
    }

end:
    h_ptr->init(addressToBind, this);
}

HSsdp::HSsdp(const QHostAddress& addressToBind, QObject* parent) :
    QObject(parent),
    h_ptr  (new HSsdpPrivate())
{
    HLOG(H_AT, H_FUN);

    h_ptr->init(addressToBind, this);
}

HSsdp::~HSsdp()
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    try
    {
        h_ptr->m_multicastSocket.leaveMulticastGroup(multicastAddress());
    }
    catch(HException& ex)
    {
        Q_ASSERT(false);
        HLOG_WARN(ex.reason());
    }

    delete h_ptr;
}

HEndpoint HSsdp::unicastEndpoint() const
{
    return HEndpoint(
        h_ptr->m_unicastSocket.localAddress(),
        h_ptr->m_unicastSocket.localPort());
}

bool HSsdp::incomingDiscoveryRequest(
    const HDiscoveryRequest&, const HEndpoint&, /*source*/
    const HEndpoint& /*dest*/)
{
    return false;
}

bool HSsdp::incomingDiscoveryResponse(
    const HDiscoveryResponse&, const HEndpoint& /*source*/)
{
    return false;
}

bool HSsdp::incomingDeviceAvailableAnnouncement(
    const HResourceAvailable&)
{
    return false;
}

bool HSsdp::incomingDeviceUnavailableAnnouncement(
    const HResourceUnavailable&)
{
    return false;
}

bool HSsdp::incomingDeviceUpdateAnnouncement(
    const HResourceUpdate&)
{
    return false;
}

void HSsdp::unicastMessageReceived()
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    QHostAddress ha; quint16 port;

    QByteArray buf; buf.resize(h_ptr->m_unicastSocket.pendingDatagramSize() + 1);

    qint64 read = h_ptr->m_unicastSocket.readDatagram(buf.data(), buf.size(), &ha, &port);
    if (read < 0)
    {
        HLOG_WARN(QObject::tr("Read failed: %1").arg(h_ptr->m_unicastSocket.errorString()));
        return;
    }

    QString msg(QString::fromUtf8(buf, read));
    HEndpoint source     (ha, port);
    HEndpoint destination(
        h_ptr->m_unicastSocket.localAddress(), h_ptr->m_unicastSocket.localPort());

    h_ptr->messageReceived(msg, source, destination);
}

void HSsdp::multicastMessageReceived()
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    QHostAddress ha; quint16 port;

    QByteArray buf; buf.resize(h_ptr->m_multicastSocket.pendingDatagramSize() + 1);

    qint64 read = h_ptr->m_multicastSocket.readDatagram(buf.data(), buf.size(), &ha, &port);
    if (read < 0)
    {
        HLOG_WARN(QObject::tr("Read failed: %1").arg(h_ptr->m_multicastSocket.errorString()));
        return;
    }

    QString msg(QString::fromUtf8(buf, read));
    HEndpoint source     (ha, port);
    HEndpoint destination(
        h_ptr->m_multicastSocket.localAddress(), h_ptr->m_multicastSocket.localPort());

    h_ptr->messageReceived(msg, source, destination);
}

namespace
{
template<class Msg>
qint32 send(HSsdpPrivate* hptr, Msg msg, qint32 count)
{
    HLOG2(H_AT, H_FUN, hptr->m_loggingIdentifier);
    if (!msg.isValid())
    {
        HLOG_WARN(QObject::tr("Not sending invalid message: ").arg(msg.toString()));
        return -1;
    }

    for (qint32 i = 0; i < count; ++i)
    {
        hptr->send(msg.toString());
    }

    return 0;
}

template<class Msg>
qint32 send(HSsdpPrivate* hptr, Msg msg, const HEndpoint& receiver, qint32 count)
{
    HLOG2(H_AT, H_FUN, hptr->m_loggingIdentifier);
    if (!msg.isValid())
    {
        HLOG_WARN(QObject::tr("Not sending invalid message: ").arg(msg.toString()));
        return -1;
    }

    for (qint32 i = 0; i < count; ++i)
    {
        hptr->send(msg.toString(), receiver);
    }

    return 0;
}
}

qint32 HSsdp::announcePresence(const HResourceAvailable& msg, qint32 count)
{
    return send(h_ptr, msg, count);
}

qint32 HSsdp::announcePresence(const HResourceUnavailable& msg, qint32 count)
{
    return send(h_ptr, msg, count);
}

qint32 HSsdp::announceUpdate(const HResourceUpdate& msg, qint32 count)
{
    return send(h_ptr, msg, count);
}

qint32 HSsdp::sendDiscoveryRequest(const HDiscoveryRequest& msg, qint32 count)
{
    return send(h_ptr, msg, count);
}

qint32 HSsdp::sendDiscoveryResponse(
    const HEndpoint& receiver, const HDiscoveryResponse& msg, qint32 count)
{
    return send(h_ptr, msg, receiver, count);
}

}
}