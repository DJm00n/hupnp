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

#ifndef HDEVICEMODEL_INFOPROVIDER_H_
#define HDEVICEMODEL_INFOPROVIDER_H_

#include <HUpnpCore/HClonable>

namespace Herqq
{

namespace Upnp
{

class HDeviceModelInfoProviderPrivate;

/*!
 *
 * \headerfile HDeviceModelInfoProvider.h HDeviceModelInfoProvider
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa hupnp_devicehosting, HServerDevice, HServerService
 */
class H_UPNP_CORE_EXPORT HDeviceModelInfoProvider :
    public HClonable
{
H_DISABLE_COPY(HDeviceModelInfoProvider)

public:

    /*!
     * Creates a new instance.
     *
     * Creates a new instance.
     */
    HDeviceModelInfoProvider();

    /*!
     * Destroys the instance.
     *
     * Destroys the instance.
     */
    virtual ~HDeviceModelInfoProvider() = 0;

    /*!
     * Returns information of the services contained or possibly contained by
     * the specified device type.
     *
     * \param info specifies the device type.
     *
     * \return information of the services contained or possibly contained by
     * the specified device type.
     */
    virtual HServicesSetupData servicesSetupData(const HDeviceInfo& info) const;

    /*!
     * Returns information of the embedded devices contained or possibly
     * contained by the specified device type.
     *
     * \param info specifies the device type.
     *
     * \return information of the embedded devices contained or possibly
     * contained by the specified device type.
     */
    virtual HDevicesSetupData embedddedDevicesSetupData(
        const HDeviceInfo& info) const;

    /*!
     * Returns information of the actions contained or possibly contained by
     * the specified service type.
     *
     * \param serviceInfo specifies the service type.
     *
     * \param parentDeviceInfo specifies information about the parent UPnP device
     * that contains this service.
     *
     * \return information of the actions contained or possibly contained by
     * the specified service type.
     */
    virtual HActionsSetupData actionsSetupData(
        const HServiceInfo& serviceInfo, const HDeviceInfo& parentDeviceInfo) const;

    /*!
     * Returns information of the state variables contained or possibly
     * contained by the specified service type.
     *
     * \param serviceInfo specifies the service type.
     *
     * \param parentDeviceInfo specifies information about the parent UPnP device
     * that contains this service.
     *
     * \return information of the state variables contained or possibly
     * contained by the specified service type.
     */
    virtual HStateVariablesSetupData stateVariablesSetupData(
        const HServiceInfo& serviceInfo, const HDeviceInfo& parentDeviceInfo) const;
};

}
}

#endif /* HDEVICEMODEL_INFOPROVIDER_H_ */
