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

#ifndef DEVICEHOST_DATARETRIEVER_P_H_
#define DEVICEHOST_DATARETRIEVER_P_H_

#include "../../defs_p.h"

#include <QUrl>
#include <QByteArray>

class QImage;
class QDomDocument;

namespace Herqq
{

namespace Upnp
{

//
//
//
class DeviceHostDataRetriever
{
H_DISABLE_COPY(DeviceHostDataRetriever)

private:

    const QByteArray m_loggingIdentifier;
    QUrl m_rootDir;
    QByteArray retrieveData(const QUrl& baseUrl, const QUrl& query);

public:

    DeviceHostDataRetriever(
        const QByteArray& loggingId, const QUrl& rootDir);

    QDomDocument retrieveServiceDescription(
        const QUrl& deviceLocation, const QUrl& scpdUrl);

    QImage retrieveIcon(const QUrl& deviceLocation, const QUrl& iconUrl);

    QDomDocument retrieveDeviceDescription(const QString& filePath);
};

}
}

#endif /* DEVICEHOST_DATARETRIEVER_P_H_ */