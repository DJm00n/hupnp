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

#ifndef HSERVERDEVICE_P_H_
#define HSERVERDEVICE_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <HUpnpCore/HDeviceStatus>
#include <HUpnpCore/HServerDevice>

#include <QtCore/QUrl>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QScopedPointer>

class QTimer;

namespace Herqq
{

namespace Upnp
{

//
// Base class for the implementation details of HDevice
//
class H_UPNP_CORE_EXPORT HServerDevicePrivate
{
H_DISABLE_COPY(HServerDevicePrivate)

public: // attributes

    QScopedPointer<HDeviceInfo> m_deviceInfo;
    // The static device information read from a device description.

    QList<HServerDevice*> m_embeddedDevices;
    // The embedded devices this instance contains.

    QList<HServerService*> m_services;
    // The services this instance contains.

    HServerDevice* m_parentDevice;
    // ^^ this is not the "QObject" parent, but rather the parent in the
    // device tree.

    HServerDevice* q_ptr;
    // The "parent" QObject

    QList<QUrl> m_locations;
    // The URLs at which this device is available

    QString m_deviceDescription;
    // The full device description.
    // CONSIDER: would it be better to load this into memory only when needed?

    QScopedPointer<HDeviceStatus> m_deviceStatus;

public: // methods

    HServerDevicePrivate();
    virtual ~HServerDevicePrivate();

    inline bool isValid() const { return m_deviceInfo.data(); }
};

}
}

#endif /* HSERVERDEVICE_P_H_ */
