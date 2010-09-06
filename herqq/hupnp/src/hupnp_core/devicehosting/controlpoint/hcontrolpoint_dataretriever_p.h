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

#ifndef HCONTROLPOINT_DATARETRIEVER_H_
#define HCONTROLPOINT_DATARETRIEVER_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../../general/hupnp_defs.h"

#include <QtCore/QByteArray>

class QUrl;
class QImage;

namespace Herqq
{

namespace Upnp
{

class HHttpHandler;
class HDeviceController;

//
//
//
class HDataRetriever
{
H_DISABLE_COPY(HDataRetriever)

private:

    const QByteArray m_loggingIdentifier;
    HHttpHandler& m_http;

    QByteArray retrieveData(
        const QUrl& baseUrl, const QUrl& query, bool processAbsoluteUrl);

public:

    HDataRetriever(const QByteArray& loggingId, HHttpHandler&);

    QString retrieveServiceDescription(
        const QUrl& deviceLocation, const QUrl& scpdUrl);

    QImage retrieveIcon(
        const QUrl& deviceLocation, const QUrl& iconUrl);

    QString retrieveDeviceDescription(const QUrl& deviceLocation);
};

}
}

#endif /* HCONTROLPOINT_DATARETRIEVER_H_ */
