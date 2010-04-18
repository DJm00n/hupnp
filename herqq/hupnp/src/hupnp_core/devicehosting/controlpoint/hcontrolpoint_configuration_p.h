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

#ifndef HCONTROLPOINT_CONFIGURATION_P_H_
#define HCONTROLPOINT_CONFIGURATION_P_H_

#include "./../hdevicecreator.h"
#include "./../../../utils/hglobal.h"

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

namespace Herqq
{

namespace Upnp
{

//
// Implementation details of HControlPointConfiguration class
//
class H_UPNP_CORE_EXPORT HControlPointConfigurationPrivate
{
public: // attributes

    HDeviceCreator m_deviceCreator;
    bool m_subscribeToEvents;
    qint32 m_desiredSubscriptionTimeout;
    bool m_performInitialDiscovery;

public: // methods

    HControlPointConfigurationPrivate();
    virtual ~HControlPointConfigurationPrivate();
};

}
}

#endif /* HCONTROLPOINT_CONFIGURATION_P_H_ */
