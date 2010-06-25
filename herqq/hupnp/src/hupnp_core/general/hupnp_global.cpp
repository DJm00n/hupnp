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

#include "hupnp_global.h"
#include "hupnp_global_p.h"

#include "hdefs_p.h"

#include "../socket/hendpoint.h"
#include "../dataelements/hproduct_tokens.h"

#include "../../utils/hlogger_p.h"
#include "../../utils/hexceptions_p.h"

#include <QUrl>
#include <QString>
#include <QTcpSocket>
#include <QTextStream>
#include <QDomElement>
#include <QDomNodeList>
#include <QHostAddress>
#include <QNetworkInterface>

#if defined(Q_OS_LINUX)
#include <sys/utsname.h>
#endif

/*!
 * \namespace Herqq The main namespace of Herqq libraries. This namespace contains
 * the global enumerations, typedefs, functions and classes that are used
 * and referenced throughout the Herqq libraries.
 *
 * \namespace Herqq::Upnp The namespace that contains all of the Herqq UPnP
 * core functionality.
 */

namespace Herqq
{

namespace Upnp
{

void SetLoggingLevel(HLogLevel level)
{
    HLogger::setTraceLevel(static_cast<HLogger::HLogLevel>(level));
}

void EnableNonStdBehaviourWarnings(bool arg)
{
    HLogger::enableNonStdWarnings(arg);
}

QString readElementValue(
    const QString elementTagToSearch, const QDomElement& parentElement,
    bool* wasDefined)
{
    QDomElement element =
        parentElement.firstChildElement(elementTagToSearch);

    if (element.isNull())
    {
        if (wasDefined)
        {
            *wasDefined = false;
        }

        return "";
    }

    if (wasDefined)
    {
        *wasDefined = true;
    }

    return element.text();
}

QString toString(const QDomElement& e)
{
    QString buf;
    QTextStream ts(&buf, QIODevice::ReadWrite);
    e.save(ts, 0);

    return buf;
}

/*******************************************************************************
 * HSysInfo
 *******************************************************************************/
QScopedPointer<HSysInfo> HSysInfo::s_instance;
QMutex HSysInfo::s_initMutex;

HSysInfo::HSysInfo()
{
    createProductTokens();
    createLocalNetworks();
}

HSysInfo::~HSysInfo()
{
}

void HSysInfo::createProductTokens()
{
#if defined(Q_OS_WIN)
    QString server = "MicrosoftWindows/";
    switch(QSysInfo::WindowsVersion)
    {
    case QSysInfo::WV_2000:
        server.append("5.0");
        break;
    case QSysInfo::WV_XP:
        server.append("5.1");
        break;
    case QSysInfo::WV_2003:
        server.append("5.2");
        break;
    case QSysInfo::WV_VISTA:
        server.append("6.0");
        break;
    case QSysInfo::WV_WINDOWS7:
        server.append("6.1");
        break;
    default:
        server.append("-1");
    }
#elif defined(Q_OS_DARWIN)
    QString server = "AppleMacOSX/";
    switch(QSysInfo::MacintoshVersion)
    {
    case QSysInfo::MV_10_3:
        server.append("10.3");
        break;
    case QSysInfo::MV_10_4:
        server.append("10.4");
        break;
    case QSysInfo::MV_10_5:
        server.append("10.5");
        break;
    case QSysInfo::MV_10_6:
        server.append("10.6");
        break;
    default:
        server.append("-1");
    }
#elif defined(Q_OS_LINUX)
    QString server;
    struct utsname sysinfo;
    if (!uname(&sysinfo))
    {
        server = QString("%1/%2").arg(sysinfo.sysname, sysinfo.release);
    }
    else
    {
        server = "Undefined/-1";
    }
#else
    QString server = "Undefined/-1";
#endif

    m_productTokens.reset(
        new HProductTokens(QString("%1 UPnP/1.1 HUPnP/0.5").arg(server)));
}

void HSysInfo::createLocalNetworks()
{
    foreach(const QNetworkInterface& iface, QNetworkInterface::allInterfaces())
    {
        QList<QNetworkAddressEntry> entries = iface.addressEntries();
        foreach(const QNetworkAddressEntry& entry, entries)
        {
            QHostAddress ha = entry.ip();
            if (ha.protocol() != QAbstractSocket::IPv4Protocol)
            {
                continue;
            }

            quint32 nm = entry.netmask().toIPv4Address();
            m_localNetworks.append(qMakePair(ha.toIPv4Address() & nm, nm));
        }
    }
}

bool HSysInfo::localNetwork(const QHostAddress& ha, quint32* retVal) const
{
    Q_ASSERT(retVal);

    QList<QPair<quint32, quint32> >::const_iterator ci;
    for(ci = m_localNetworks.begin(); ci != m_localNetworks.end(); ++ci)
    {
        if ((ha.toIPv4Address() & ci->second) == ci->first)
        {
            *retVal = ci->first;
            return true;
        }
    }

    return false;
}

bool HSysInfo::isLocalAddress(const QHostAddress& ha) const
{
    quint32 tmp;
    return localNetwork(ha, &tmp);
}

bool HSysInfo::areLocalAddresses(const QList<QHostAddress>& addresses) const
{
    QList<QHostAddress> localAddresses = QNetworkInterface::allAddresses();
    foreach(const QHostAddress& ha, addresses)
    {
        bool matched = false;
        foreach(const QHostAddress& localAddress, localAddresses)
        {
            if (localAddress == ha)
            {
                matched = true;
                break;
            }
        }

        if (!matched)
        {
            return false;
        }
    }

    return true;
}

HEndpoints convertHostAddressesToEndpoints(const QList<QHostAddress>& addrs)
{
    HEndpoints retVal;
    foreach(const QHostAddress& ha, addrs)
    {
        retVal.append(HEndpoint(ha));
    }
    return retVal;
}

void verifySpecVersion(const QDomElement& rootElement)
{
    QDomElement specVersionElement = rootElement.firstChildElement("specVersion");
    if (specVersionElement.isNull())
    {
        throw HIllegalArgumentException(
            "Invalid device description: missing mandatory <specVersion> element");
    }

    QString minorVersion = readElementValue("minor", specVersionElement);
    QString majorVersion = readElementValue("major", specVersionElement);

    bool ok;
    qint32 major = majorVersion.toInt(&ok);
    if (!ok || major != 1)
    {
        throw HIllegalArgumentException(
            "Invalid device description: major element of <specVersion> is not 1");
    }

    qint32 minor = minorVersion.toInt(&ok);
    if (!ok || (minor != 1 && minor != 0))
    {
        throw HIllegalArgumentException(
            "Invalid device description: minor element of <specVersion> is not 0 or 1");
    }
}

qint32 readConfigId(const QDomElement& rootElement)
{
    bool ok = false;

    QString cid    = readElementValue("configId", rootElement);
    qint32 retVal  = cid.toInt(&ok);
    if (!ok || retVal < 0 || retVal > ((1 << 24)-1))
    {
        return 0;
    }

    return retVal;
}

QString verifyName(const QString& name)
{
    HLOG(H_AT, H_FUN);

    QString tmp = name;
    if (tmp.isEmpty())
    {
        throw HIllegalArgumentException("[name] cannot be empty");
    }

    if (!tmp[0].isLetterOrNumber() && tmp[0] != '_')
    {
        throw HIllegalArgumentException(
            QString("[name: %1] has invalid first character").arg(tmp));
    }

    foreach(QChar c, tmp)
    {
        if (!c.isLetterOrNumber() && c != '_' && c != '.')
        {
            throw HIllegalArgumentException(
                QString("[name: %1] contains invalid character(s)").arg(tmp));
        }
    }

    if (tmp.size() > 32)
    {
        HLOG_WARN(QString("[name: %1] longer than 32 characters").arg(tmp));
    }

    return tmp;
}

QString urlsAsStr(const QList<QUrl>& urls)
{
    QString retVal;

    for(qint32 i = 0; i < urls.size(); ++i)
    {
        retVal.append(QString("#%1 %2\n").arg(
            QString::number(i), urls[i].toString()));
    }

    return retVal;
}

QUrl resolveUri(const QUrl& baseUrl, const QUrl& other)
{
    QString otherReq(extractRequestPart(other));

    if (otherReq.startsWith('/'))
    {
        return QString("%1%2").arg(extractHostPart(baseUrl), otherReq);
    }

    QString basePath(baseUrl.toString());

    if (!basePath.endsWith('/'))  { basePath.append('/'); }
    if (otherReq.startsWith('/')) { otherReq.remove(0, 1); }

    basePath.append(otherReq);

    return basePath;
}

QUrl appendUrls(const QUrl& baseUrl, const QUrl& other)
{
    QString otherReq(extractRequestPart(other));

    QString basePath(baseUrl.toString());

    if (!basePath.endsWith('/'))  { basePath.append('/'); }
    if (otherReq.startsWith('/')) { otherReq.remove(0, 1); }

    basePath.append(otherReq);

    return basePath;
}

}
}
