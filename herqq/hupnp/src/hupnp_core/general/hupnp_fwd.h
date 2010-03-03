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
class HAction;
class HDevice;
class HService;
class HServiceId;
class HDeviceHost;
class HDeviceInfo;
class HControlPoint;
class HAbstractHost;
class HResourceType;
class HObjectCreator;
class HStateVariable;
class HActionArgument;
class HActionArguments;
class HStateVariableEvent;
class HDeviceConfiguration;
class HActionArgument;
class HDeviceHostConfiguration;
class HControlPointConfiguration;

class HResourceUpdate;
class HDiscoveryRequest;
class HDiscoveryResponse;
class HResourceAvailable;
class HResourceUnavailable;

/*!
 * Type definition for a root HDevice wrapped inside a reference counting shared
 * pointer.
 *
 * This is an important design concept to notice; a root device is only accessible
 * as a HRootDevicePtrT. The HUPnP API does not expose a root UPnP device
 * in any other way. Because of this, a root HDevice object
 * and objects that are reachable through them are
 * accessible as long as at least one pointer to the root device object is valid.
 *
 * \ingroup devicemodel
 *
 * \sa HDevice
 */
typedef QSharedPointer<HDevice> HRootDevicePtrT;

/*!
 * Type definition for a collection of root HDevices managed by reference counting
 * shared pointers.
 *
 * \ingroup devicemodel
 *
 * \sa HRootDevicePtrT
 */
typedef QList<HRootDevicePtrT> HRootDevicePtrListT;

/*!
 * Type definition for a list of raw pointers to HDevice instances.
 *
 * Whereas root devices are always exposed only through QSharedPointers,
 * embedded devices are always exposed as raw pointers. This enforces a design
 * where the life time of a UPnP device tree is dictated by the life time of
 * the device tree's root device; when a root device is deleted, every object managed by it
 * (the UPnP device tree) is also deleted.
 *
 * \ingroup devicemodel
 *
 * \sa HDevice
 */
typedef QList<HDevice*> HDevicePtrListT;

/*!
 * Type definition for a list of raw pointers to HService instances.
 *
 * \ingroup devicemodel
 */
typedef QList<HService*> HServicePtrListT;

}
}

#endif /* UPNPFWD_H_ */
