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

#ifndef UPNP_DEVICEHOST_P_H_
#define UPNP_DEVICEHOST_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "devicehost.h"
#include "event_notifier_p.h"
#include "presence_announcer_p.h"
#include "devicehost_http_server_p.h"
#include "devicehost_configuration.h"
#include "devicehost_ssdp_handler_p.h"

#include "../abstracthost_p.h"

#include <QAtomicInt>
#include <QScopedPointer>

namespace Herqq
{

namespace Upnp
{

//
// Implementation details of HDeviceHost class
//
class HDeviceHostPrivate :
    public HAbstractHostPrivate
{
Q_OBJECT
H_DISABLE_COPY(HDeviceHostPrivate);
H_DECLARE_PUBLIC(HDeviceHost);

private:

    virtual void doClear();
    // ^^ this is called by the base class upon destruction

    void connectSelfToServiceSignals(HDevice* device);

public: // attributes

    HDeviceHostConfiguration m_initParams;
    //

    QScopedPointer<DeviceHostSsdpHandler> m_ssdp;
    //

    QScopedPointer<DeviceHostHttpServer> m_httpServer;
    //

    QAtomicInt m_activeRequestCount;
    //

    QScopedPointer<EventNotifier> m_eventNotifier;
    //

    QScopedPointer<PresenceAnnouncer> m_presenceAnnouncer;
    //

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


#endif /* UPNP_DEVICEHOST_P_H_ */
