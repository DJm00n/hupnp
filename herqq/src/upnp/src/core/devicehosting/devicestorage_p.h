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

#ifndef DEVICESTORAGE_H_
#define DEVICESTORAGE_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../defs_p.h"
#include "../messaging/endpoint.h"
#include "../devicemodel/device.h"
#include "../devicemodel/device_p.h"
#include "../devicemodel/service.h"
#include "../devicemodel/service_p.h"

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
    QList<HDeviceController*> m_rootDevices;

private:

    void checkDeviceTreeForUdnConflicts(HDeviceController* device) const;

public: // instance methods

    mutable QMutex m_rootDevicesMutex;

    DeviceStorage(const QByteArray& loggingIdentifier);
    ~DeviceStorage();

    void clear();

    void addRootDevice   (HDeviceController* root);
    void removeRootDevice(HDeviceController* root);

    HDeviceController* searchDeviceByUdn(const HUdn& udn) const;

    QList<HDeviceController*> searchDevicesByDeviceType(
        const HResourceType& deviceType, bool exactMatch = true)const;

    QList<HServiceController*> searchServicesByServiceType(
        const HResourceType& serviceType, bool exactMatch = true) const;

    HServiceController* searchServiceByScpdUrl(
        HDeviceController* device, const QUrl& scpdUrl) const;

    HServiceController* searchServiceByControlUrl(
        HDeviceController* device, const QUrl& controlUrl) const;

    HServiceController* searchServiceByEventUrl(
        HDeviceController* device, const QUrl& eventUrl) const;

    HRootDevicePtrListT rootDevices() const;

    QList<HDeviceController*> rootDeviceControllers() const;

public: // static methods

    static QPair<QUrl, QImage> seekIcon(
        HDeviceController* device, const QString& iconUrl);

    static bool searchValidLocation(
        const HDevice* device, const HEndpoint& interface, QUrl* location);


};

}
}

#endif /* DEVICESTORAGE_H_ */