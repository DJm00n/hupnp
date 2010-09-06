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

#ifndef HDEVICEHOST_DATARETRIEVER_P_H_
#define HDEVICEHOST_DATARETRIEVER_P_H_

#include "../../general/hupnp_defs.h"

#include <QtCore/QUrl>
#include <QtCore/QByteArray>

class QImage;

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

    QString retrieveServiceDescription(
        const QUrl& deviceLocation, const QUrl& scpdUrl);

    QImage retrieveIcon(const QUrl& deviceLocation, const QUrl& iconUrl);

    QString retrieveDeviceDescription(const QString& filePath);
};

}
}

#endif /* HDEVICEHOST_DATARETRIEVER_P_H_ */
