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

#ifndef HUPNP_DEVICE_H_
#define HUPNP_DEVICE_H_

#include "./../general/hdefs_p.h"
#include "./../general/hupnp_fwd.h"
#include "./../dataelements/hresourcetype.h"

template<typename T, typename U>
class QHash;

template<typename T>
class QList;

#include <QObject>

class QUrl;
class QString;

namespace Herqq
{

namespace Upnp
{

class HDevicePrivate;
class HObjectCreator;
class HDeviceController;

/*!
 * \file
 * This file contains the declaration of the HDevice component.
 */

/*!
 * \brief An abstract base class that represents a UPnP device hosted by the HUPnP
 * library.
 *
 * \c %HDevice is a core component of the HUPnP \ref devicemodel
 * and it models a UPnP device, both root and embedded.
 * As detailed in the UPnP Device Architecture specification,
 * a UPnP device is essentially a container for services and possibly for other
 * (embedded) UPnP devices.
 *
 * <h2>Using the class</h2>
 *
 * The most common uses of \c %HDevice involve reading the various device information
 * elements originally set in the device description file and enumerating the
 * exposed services. By calling deviceInfo() you get an Herqq::Upnp::HDeviceInfo object from
 * which you can read all the informational elements found in the device description.
 * Calling services() gives you a list of Herqq::Upnp::HService instances the device exposes.
 * Note that it is the services that contain the functionality and runtime status of the device.
 *
 * Some devices also contain embedded devices, which you can get by calling
 * embeddedDevices().
 *
 * You can retrieve the device's description file by calling deviceDescription() or
 * you can manually read it from any of the locations returned by locations(). If
 * the device is an embedded device, it always has a parent defined, which you can
 * get by calling parentDevice().
 *
 * <h2>Sub-classing</h2>
 *
 * Sub-classing an \c %HDevice is simple. All you need to do is override the
 * HDevice::createServices() method, in which you create objects of service types according
 * to the device's device description file.
 *
 * As an example, consider the following snippet in which is shown the
 * createServices() method from a fictional device named \c DimmableLight:
 *
 * \code
 *
 * #include "switchpowerimpl.h" // this would be your code
 * #include "dimmingimpl.h"     // this would be your code
 *
 * Herqq::Upnp::HDevice::HServiceMap DimmableLight::createServices()
 * {
 *   Herqq::Upnp::HDevice::HServiceMap retVal;
 *
 *   retVal[Herqq::Upnp::HResourceType("urn:schemas-upnp-org:service:SwitchPower:1")]   =
 *       new SwitchPowerImpl(); // your type
 *
 *   retVal[Herqq::Upnp::HResourceType("urn:schemas-upnp-org:service:Dimming:1")] =
 *       new DimmingImpl(); // your type
 *
 *   return retVal;
 * }
 *
 * \endcode
 *
 * The above code maps your types, namely \c SwitchPowerImpl and \c DimmingImpl
 * to the <em>UPnP service types</em> identified by the strings
 * <c>urn:schemas-upnp-org:service:SwitchPower:1</c> and
 * <c>urn:schemas-upnp-org:service:Dimming:1</c>, respectively.
 * The HDevice::createServices() method is called when the device type is being
 * initialized. If any of those service types are not in the device description file,
 * or you didn't map a type to a service type found in the device description, the
 * device creation will fail.
 *
 * \headerfile hdevice.h HDevice
 *
 * \ingroup devicemodel
 *
 * \remark the methods introduced in this class are thread-safe, but the \c QObject
 * base class is largely not.
 */
class H_UPNP_CORE_EXPORT HDevice :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HDevice)
H_DECLARE_PRIVATE(HDevice)
friend class HObjectCreator;
friend class HDeviceController;

public: // typedefs

    /*!
     * A type definition for a map holding HResourceTypes as keys and
     * pointers to HServices as values.
     *
     * This type definition is used as a return value of private HDevice::createServices()
     * method to convey information about the UPnP services the device exports
     * to HUPnP during the object initialization.
     *
     * \sa HDevice, HService
     */
    typedef QHash<HResourceType, HService*> HServiceMap;

private:

    /*!
     * Creates the services that this UPnP device provides.
     *
     * Every descendant has to override this.
     *
     * This method is called once when the device is being initialized by the managing device host.
     *
     * \return the services that this HDevice provides.
     *
     * \remark The base class takes the ownership of the created services and will
     * delete them upon destruction. Because of that, you can store the
     * addresses of the created services and use them safely throughout the lifetime
     * of the containing device object. However, you cannot delete them.
     */
    virtual HServiceMap createServices() = 0;

protected:

    HDevicePrivate* h_ptr;

    //
    // \internal
    //
    // Constructor for derived classes for re-using the h_ptr.
    //
    HDevice(HDevicePrivate& dd);

    /*!
     * Default constructor for derived classes.
     */
    HDevice();

public:

    /*!
     * Destroys the instance.
     *
     * \warning An HDevice is always owned and destroyed by HUPnP.
     * You should never destroy an HDevice.
     */
    virtual ~HDevice() = 0;

    /*!
     * Returns information about the device that is read from the device description.
     *
     * \return information about the device that is read from the device description.
     */
    HDeviceInfo deviceInfo() const;

    /*!
     * Returns the parent device of this device, if any.
     *
     * \return the parent device of this device, if any, or a null pointer
     * in case the device is a root device.
     *
     * \remarks the pointer is guaranteed to
     * point to the parent object throughout the lifetime of this object.
     */
    const HDevice* parentDevice() const;

    /*!
     * Returns the root device of the device tree to which this device belongs.
     *
     * \return the root device of the device tree to which this device belongs.
     *
     * \remark this device could be the root device of the device tree in question,
     * in which case a pointer to this instance is returned.
     */
    const HDevice* rootDevice() const;

    /*!
     * Returns the full device description of the device.
     *
     * \return full device description that is associated to this device.
     *
     * \remarks an embedded device returns the same device description as
     * its root device.
     *
     */
    QString deviceDescription() const;

    /*!
     * Returns the embedded devices of this device.
     *
     * \return the embedded devices of this device. The collection is empty
     * if the device has no embedded devices.
     *
     * \remarks the pointers are guaranteed to be valid only throughout the lifetime
     * of this object.
     */
    HDeviceList embeddedDevices() const;

    /*!
     * Returns the services this device exports.
     *
     * \return the services this device exports. The collection is empty
     * if the device has no services.
     *
     * \remarks the pointers are guaranteed to be valid only throughout the lifetime
     * of this object.
     */
    HServiceList services() const;

    /*!
     * Returns the service that has the specified service ID.
     *
     * \param serviceId specifies the service to be returned.
     *
     * \return the service that has the specified service ID or a null pointer
     * in case there is no service with the specified ID.
     *
     * \remarks the pointer is guaranteed to be valid only throughout the lifetime
     * of this object.
     */
    HService* serviceById(const HServiceId& serviceId) const;

    /*!
     * Returns the list of locations where the device is currently available.
     *
     * \param includeDeviceDescriptionPostfix specifies whether or not the returned
     * URLs are absolute URLs for retrieving the device description. The default
     * is true. If the parameter is specified as false, the returned URLs contain
     * only the base URLs of the device.
     *
     * \return the list location where the device is currently available.
     */
    QList<QUrl> locations(bool includeDeviceDescriptionPostfix=true) const;
};

}
}

#endif /* HUPNP_DEVICE_H_ */
