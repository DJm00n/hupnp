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

#include "upnp_global.h"
#include "upnp_global_p.h"

#include "messaging/product_tokens.h"

#include "../../../utils/src/logger_p.h"

#include "../../../core/include/HGlobal"
#include "../../../core/include/HExceptions"

#include <QUrl>
#include <QString>
#include <QTcpSocket>
#include <QTextStream>
#include <QDomElement>
#include <QDomNodeList>
#include <QHostAddress>

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

void SetLoggingLevel(LogLevel level)
{
    HLogger::setTraceLevel(static_cast<HLogger::LogLevel>(level));
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

void verifySpecVersion(const QDomElement& rootElement)
{
    HLOG(H_AT, H_FUN);

    QDomElement specVersionElement = rootElement.firstChildElement("specVersion");
    if (specVersionElement.isNull())
    {
        throw HIllegalArgumentException(QObject::tr(
            "Invalid device description: missing mandatory <specVersion> element"));
    }

    QString minorVersion = readElementValue("minor", specVersionElement);
    QString majorVersion = readElementValue("major", specVersionElement);

    bool ok;
    qint32 major = majorVersion.toInt(&ok);
    if (!ok || major != 1)
    {
        throw HIllegalArgumentException(QObject::tr(
            "Invalid device description: major element of <specVersion> is not 1"));
    }

    qint32 minor = minorVersion.toInt(&ok);
    if (!ok || (minor != 1 && minor != 0))
    {
        throw HIllegalArgumentException(QObject::tr(
            "Invalid device description: minor element of <specVersion> is not 0 or 1"));
    }
}

qint32 readConfigId(const QDomElement& rootElement)
{
    HLOG(H_AT, H_FUN);

    bool ok = false;

    QString cid    = readElementValue("configId", rootElement);
    qint32 retVal  = cid.toInt(&ok);
    if (!ok || retVal < 0 || retVal > ((1 << 24)-1))
    {
        HLOG_DBG(QObject::tr("Missing or invalid configId element"));
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
        throw HIllegalArgumentException(QObject::tr("[name] cannot be empty"));
    }

    if (!tmp[0].isLetterOrNumber() && tmp[0] != '_')
    {
        throw HIllegalArgumentException(
            QObject::tr("[name: %1] has invalid first character").arg(tmp));
    }

    foreach(QChar c, tmp)
    {
        if (!c.isLetterOrNumber() && c != '_' && c != '.')
        {
            throw HIllegalArgumentException(
                QObject::tr("[name: %1] contains invalid character(s)").arg(tmp));
        }
    }

    if (tmp.size() > 32)
    {
        HLOG_WARN(QObject::tr("[name: %1] longer than 32 characters").arg(tmp));
    }

    return tmp;
}

HProductTokens herqqProductTokens()
{
    HLOG(H_AT, H_FUN);

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
    QString server = "AppleMacOSX/10";
#elif defined(Q_OS_LINUX)
    QString server = "Linux/2.6";
#else
    QString server = "Undefined/-1";
#endif

    return HProductTokens(QString("%1 UPnP/1.1 HerqqUPnP/0.1").arg(server));
}

QString peerAsStr(const QTcpSocket& sock)
{
    return QString("%1:%2").arg(
        sock.peerAddress().toString(), QString::number(sock.peerPort()));
}

QString extractBaseUrl(const QString& url)
{
    HLOG(H_AT, H_FUN);

    if (url.endsWith('/'))
    {
        return url;
    }
    else if (!url.contains('/'))
    {
        return "";
    }

    QString base = url.section(
        '/', 0, -2, QString::SectionIncludeTrailingSep);

    return base;
}

QUrl extractBaseUrl(const QUrl& url)
{
    HLOG(H_AT, H_FUN);

    QString urlAsStr = url.toString();
    return extractBaseUrl(urlAsStr);
}

QUrl appendUrls(const QUrl& baseUrl, const QUrl& relativeUrl)
{
    QString relativePath(extractRequestPart(relativeUrl));
    QString basePath(baseUrl.toString());

    if (!basePath.endsWith('/')) { basePath.append('/'); }
    if (relativePath.startsWith('/')) { relativePath.remove(0, 1); }

    basePath.append(relativePath);

    return basePath;
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

QString extractRequestPart(const QUrl& arg)
{
    return arg.toString(
        QUrl::RemoveAuthority | QUrl::RemovePassword | QUrl::RemoveUserInfo |
        QUrl::RemoveScheme | QUrl::RemovePort | QUrl::StripTrailingSlash);
}

}
}
