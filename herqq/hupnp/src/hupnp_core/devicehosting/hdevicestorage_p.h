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

#ifndef HDEVICESTORAGE_H_
#define HDEVICESTORAGE_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "./../general/hdefs_p.h"
#include "./../socket/hendpoint.h"
#include "./../devicemodel/hdevice.h"
#include "./../devicemodel/hdevice_p.h"
#include "./../devicemodel/hservice.h"
#include "./../devicemodel/hservice_p.h"

#include <QUrl>
#include <QPair>
#include <QList>
#include <QMutex>
#include <QImage>
#include <QByteArray>

namespace Herqq
{

namespace Upnp
{

class HUdn;
class HResourceType;

//
//
//
class DeviceStorage
{
H_DISABLE_COPY(DeviceStorage)

private:

    const QByteArray m_loggingIdentifier;
    // prefix for log messages

    QList<HDeviceController*> m_rootDevices;
    // the device trees stored by this instance

private:

    void checkDeviceTreeForUdnConflicts(HDeviceController* device) const;

public: // instance methods

    mutable QMutex m_rootDevicesMutex;

    DeviceStorage(const QByteArray& loggingIdentifier);
    ~DeviceStorage();

    void clear();

    void addRootDevice   (HDeviceController* root);
    bool removeRootDevice(HDeviceController* root);

    HDeviceController* searchDeviceByUdn(const HUdn& udn) const;

    QList<HDeviceController*> searchDevicesByDeviceType(
        const HResourceType& deviceType, bool exactMatch = true) const;

    QList<HServiceController*> searchServicesByServiceType(
        const HResourceType& serviceType, bool exactMatch = true) const;

    HServiceController* searchServiceByScpdUrl(
        HDeviceController* device, const QUrl& scpdUrl) const;

    HServiceController* searchServiceByScpdUrl(
        const QUrl& scpdUrl) const;

    HServiceController* searchServiceByControlUrl(
        HDeviceController* device, const QUrl& controlUrl) const;

    HServiceController* searchServiceByControlUrl(
        const QUrl& controlUrl) const;

    HServiceController* searchServiceByEventUrl(
        HDeviceController* device, const QUrl& eventUrl) const;

    HServiceController* searchServiceByEventUrl(
        const QUrl& eventUrl) const;

    HDevicePtrList rootDevices() const;

    QList<HDeviceController*> rootDeviceControllers() const;

    QPair<QUrl, QImage> seekIcon(
        HDeviceController* device, const QString& iconUrl);

    bool searchValidLocation(
        const HDevice* device, const HEndpoint& interface, QUrl* location);
};

}
}

#endif /* HDEVICESTORAGE_H_ */
