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

#ifndef HUPNPFWD_H_
#define HUPNPFWD_H_

template<typename T>
class QList;

namespace Herqq
{

namespace Upnp
{

/*!
 * \file
 * This file contains forward-declarations to every public class HUPnP exposes
 * and a few common type definitions.
 */

class HUdn;
class HAction;
class HDevice;
class HService;
class HAsyncOp;
class HExecArgs;
class HEndpoint;
class HServiceId;
class HActionInfo;
class HDeviceHost;
class HDeviceInfo;
class HServiceInfo;
class HDeviceProxy;
class HDeviceSetup;
class HServiceProxy;
class HControlPoint;
class HServiceSetup;
class HResourceType;
class HObjectCreator;
class HStateVariable;
class HProductTokens;
class HDiscoveryType;
class HActionArgument;
class HDeviceProxyList;
class HActionArguments;
class HActionsSetupData;
class HDevicesSetupData;
class HServicesSetupData;
class HStateVariableInfo;
class HServicesSetupData;
class HStateVariableEvent;
class HDeviceConfiguration;
class HWritableStateVariable;
class HReadableStateVariable;
class HStateVariablesSetupData;
class HDeviceHostConfiguration;
class HDeviceHostRuntimeStatus;
class HControlPointConfiguration;

class HResourceUpdate;
class HDiscoveryRequest;
class HDiscoveryResponse;
class HResourceAvailable;
class HResourceUnavailable;

/*!
 * This is a type definition for a collection of Herqq::Upnp::HEndpoint instances.
 *
 * \ingroup hupnp_common
 *
 * \sa HEndpoint
 */
typedef QList<HEndpoint> HEndpoints;

/*!
 * This is a type definition for a collection of pointers to
 * Herqq::Upnp::HService instances.
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HService
 */
typedef QList<HService*> HServices;

/*!
 * This is a type definition for a collection of pointers to
 * Herqq::Upnp::HServiceProxy instances.
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HServiceProxy
 */
typedef QList<HServiceProxy*> HServiceProxies;

/*!
 * This is a type definition for a collection of pointers to
 * Herqq::Upnp::HDevice instances.
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HDevice
 */
typedef QList<HDevice*> HDevices;

/*!
 * This is a type definition for a collection of pointers to
 * Herqq::Upnp::HDeviceProxy instances.
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HDeviceProxy
 */
typedef QList<HDeviceProxy*> HDeviceProxies;

/*!
 * This is a type definition for a collection of pointers to
 * Herqq::Upnp::HStateVariable instances.
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HStateVariable
 */
typedef QList<HStateVariable*> HStateVariables;

/*!
 * This is a type definition for a collection of pointers to
 * Herqq::Upnp::HAction instances.
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HAction
 */
typedef QList<HAction*> HActions;

}
}

#endif /* HUPNPFWD_H_ */
