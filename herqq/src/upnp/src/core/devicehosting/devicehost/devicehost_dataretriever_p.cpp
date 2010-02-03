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

#include "devicehost_dataretriever_p.h"

#include "../exceptions_p.h"

#include "../../../../utils/src/logger_p.h"
#include "../../../../core/include/HExceptions"

#include <QFile>
#include <QImage>
#include <QString>
#include <QDomDocument>

namespace Herqq
{

namespace Upnp
{

DeviceHostDataRetriever::DeviceHostDataRetriever(
    const QByteArray& loggingId, const QUrl& rootDir) :
        m_loggingIdentifier(loggingId), m_rootDir(rootDir)
{
}

QDomDocument DeviceHostDataRetriever::retrieveServiceDescription(
    const QUrl& /*deviceLocation*/, const QUrl& scpdUrl)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QString localScpdPath = scpdUrl.toLocalFile();
    if (localScpdPath.startsWith('/'))
    {
        localScpdPath = localScpdPath.mid(1);
    }

    QString fullScpdPath = m_rootDir.toString();
    if (!fullScpdPath.endsWith('/'))
    {
        fullScpdPath.append('/');
    }
    fullScpdPath.append(localScpdPath);
    // UDA mandates that the paths inside a device description are treated relative
    // to the device description location.

    QFile file(fullScpdPath);

    HLOG_DBG(QObject::tr(
        "Attempting to open service description from [%1]").arg(fullScpdPath));

    if (!file.open(QIODevice::ReadOnly))
    {
        throw HOperationFailedException(
            QObject::tr("Could not open the service description file [%1].").arg(
                fullScpdPath));
    }

    QDomDocument dd;
    QString errMsg; qint32 errLine = 0;
    if (!dd.setContent(&file, false, &errMsg, &errLine))
    {
        throw InvalidServiceDescription(
            QObject::tr("Could not parse the service description file [%1]: %2 @ line %3").
            arg(fullScpdPath, errMsg, QString::number(errLine)));
    }

    return dd;
}

QImage DeviceHostDataRetriever::retrieveIcon(
    const QUrl& /*devLoc*/, const QUrl& iconUrl)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QString localIconPath = iconUrl.toLocalFile();
    if (localIconPath.startsWith('/'))
    {
        localIconPath = localIconPath.mid(1);
    }

    QString fullIconPath = m_rootDir.toString();
    if (!fullIconPath.endsWith('/'))
    {
        fullIconPath.append('/');
    }
    fullIconPath.append(localIconPath);
    // UDA mandates that the paths inside a device description are treated relative
    // to the device description location.

    HLOG_DBG(QObject::tr(
        "Attempting to open a file [%1] that should contain an icon").arg(
            fullIconPath));

    QImage icon(fullIconPath);

    if (icon.isNull())
    {
        throw InvalidDeviceDescription(
            QObject::tr("Could not open the icon file [%1]").arg(fullIconPath));
    }

    return icon;
}

QDomDocument DeviceHostDataRetriever::retrieveDeviceDescription(
    const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        throw HOperationFailedException(
            QObject::tr("Could not open the device description file: [%1].").arg(
                filePath));
    }

    QDomDocument dd;
    QString errMsg; qint32 errLine = 0;
    if (!dd.setContent(&file, false, &errMsg, &errLine))
    {
        throw InvalidDeviceDescription(
            QObject::tr("Could not parse the device description file: [%1] @ line %2").
            arg(errMsg, QString::number(errLine)));
    }

    return dd;

}

}
}