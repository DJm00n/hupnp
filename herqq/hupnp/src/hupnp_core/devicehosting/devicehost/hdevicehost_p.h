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

#ifndef HDEVICEHOST_P_H_
#define HDEVICEHOST_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "hdevicehost.h"
#include "../habstracthost_p.h"

#include <QtCore/QScopedPointer>

namespace Herqq
{

namespace Upnp
{

class HDeviceHost;
class EventNotifier;
class PresenceAnnouncer;
class DeviceHostHttpServer;
class DeviceHostSsdpHandler;
class HDeviceHostConfiguration;

//
// Implementation details of HDeviceHost class
//
class HDeviceHostPrivate :
    public HAbstractHostPrivate
{
Q_OBJECT
H_DECLARE_PUBLIC(HDeviceHost)
H_DISABLE_COPY(HDeviceHostPrivate)

private:

    virtual void doClear();
    // ^^ this is called by the base class upon destruction

    void connectSelfToServiceSignals(HDevice* device);

public: // attributes

    QScopedPointer<HDeviceHostConfiguration> m_config;
    //

    QList<DeviceHostSsdpHandler*> m_ssdps;
    //

    QScopedPointer<DeviceHostHttpServer> m_httpServer;
    //

    QScopedPointer<EventNotifier> m_eventNotifier;
    //

    QScopedPointer<PresenceAnnouncer> m_presenceAnnouncer;
    //

    QScopedPointer<HDeviceHostRuntimeStatus> m_runtimeStatus;
    //

    HDeviceHost* q_ptr;

    HDeviceHost::DeviceHostError m_lastError;

public Q_SLOTS:

    void announcementTimedout(HDeviceController*);
    // called when it is about for the device to be re-advertised

public: // methods

    HDeviceHostPrivate();
    virtual ~HDeviceHostPrivate();

    void stopNotifiers();
    void startNotifiers();
    void createRootDevices();
};

}
}


#endif /* HDEVICEHOST_P_H_ */
