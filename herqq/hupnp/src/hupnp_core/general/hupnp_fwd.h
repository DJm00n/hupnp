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

#ifndef UPNPFWD_H_
#define UPNPFWD_H_

template<typename T>
class QList;

#include <QSharedPointer>

namespace Herqq
{

namespace Upnp
{

/*!
 * \file
 * This file contains forward-declarations to every public class HUPnP exposes
 * and a few common type definitions.
 *
 */

class HUdn;
class HUsn;
class HAction;
class HDevice;
class HService;
class HEndpoint;
class HServiceId;
class HDeviceHost;
class HDeviceInfo;
class HControlPoint;
class HResourceType;
class HObjectCreator;
class HStateVariable;
class HActionArgument;
class HActionArguments;
class HStateVariableEvent;
class HResourceIdentifier;
class HDeviceConfiguration;
class HDeviceHostConfiguration;
class HControlPointConfiguration;

class HResourceUpdate;
class HDiscoveryRequest;
class HDiscoveryResponse;
class HResourceAvailable;
class HResourceUnavailable;

/*!
 * Type definition for a list of pointers to HDevice instances.
 *
 * \ingroup devicemodel
 *
 * \sa HDevice
 */
typedef QList<HDevice*> HDevicePtrList;

/*!
 * Type definition for a list of pointers to HService instances.
 *
 * \ingroup devicemodel
 *
 * \sa HService
 */
typedef QList<HService*> HServicePtrList;

}
}

#endif /* UPNPFWD_H_ */
