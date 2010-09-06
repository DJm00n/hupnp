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

#ifndef HDEVICE_H_
#define HDEVICE_H_

#include <HUpnpCore/HResourceType>

#include <QtCore/QObject>

class QUrl;
class QString;

template<typename T, typename U>
class QHash;

template<typename T>
class QList;

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
 * exposed services. By calling info() you get an Herqq::Upnp::HDeviceInfo object from
 * which you can read all the informational elements found in the device description.
 * Calling services() gives you a list of Herqq::Upnp::HService instances the
 * device exposes. Note that it is the services that contain the functionality
 * and runtime status of the device.
 *
 * Some devices also contain embedded devices, which you can get by calling
 * embeddedDevices().
 *
 * You can retrieve the device's description file by calling description() or
 * you can manually read it from any of the locations returned by locations(). If
 * the device is an embedded device, it always has a parent defined, which you can
 * get by calling parentDevice().
 *
 * <h2>Sub-classing</h2>
 *
 * First of all, you only need to subclass HDevice when your custom UPnP device
 * defines UPnP services. Second, if your device defines UPnP services,
 * the only thing you need to do is to override the HDevice::createServices().
 * This virtual method is used to create HService instances that
 * represent the service types defined in the device description document.
 *
 * As an example, consider the following snippet in which is shown the
 * createServices() method from a fictional device named \c DimmableLight:
 *
 * \code
 *
 * #include "switchpowerimpl.h" // this would be your code
 * #include "dimmingimpl.h"     // this would be your code
 *
 * Herqq::Upnp::HServicesSetupData* DimmableLight::createServices()
 * {
 *   Herqq::Upnp::HServicesSetupData* retVal = new HServicesSetupData();
 *
 *   retVal->insert(
 *       Herqq::Upnp::HServiceId("urn:schemas-upnp-org:serviceId:SwitchPower"),
 *       Herqq::Upnp::HResourceType("urn:schemas-upnp-org:service:SwitchPower:1"),
 *       new SwitchPowerImpl()); // your type
 *
 *   retVal->insert(
 *       Herqq::Upnp::HServiceId("urn:schemas-upnp-org:serviceId:Dimming"),
 *       Herqq::Upnp::HResourceType("urn:schemas-upnp-org:service:Dimming:1"),
 *       new DimmingImpl()); // your type
 *
 *   return retVal;
 * }
 *
 * \endcode
 *
 * The above code maps your types, namely \c SwitchPowerImpl and \c DimmingImpl
 * to the <em>UPnP service types</em> identified by the <em>service IDs</em>
 * <c>urn:schemas-upnp-org:serviceId:SwitchPower</c> and
 * <c>urn:schemas-upnp-org:serviceId:Dimming</c> respectively.
 * Normally, only HUPnP calls HDevice::createServices() and HUPnP does that
 * once when the device type is being initialized. If any of those service types
 * are not in the device description file or you didn't map a type to a
 * service type found in the device description, the device creation will fail.
 *
 * \headerfile hdevice.h HDevice
 *
 * \ingroup devicemodel
 *
 * \remarks the methods introduced in this class are thread-safe, but the \c QObject
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

public: // enums

    /*!
     * This enumeration specifies how a device tree should be traversed given a
     * starting node.
     *
     * HUPnP \ref devicemodel is organized into a tree in which there's a root HDevice
     * and it may contain embedded HDevices as its children and they may contain
     * embedded HDevices as their children recursively.
     *
     * This enumeration is used to specify how a device and its children are traversed.
     */
    enum DeviceVisitType
    {
        /*!
         * This value is used to indicate that only the HDevice in question is visited.
         */
        VisitThisOnly = 0,

        /*!
         * This value is used to indicate that this HDevice and its embedded HDevices
         * are visited.
         */
        VisitThisAndDirectChildren,

        /*!
         * This value is used to indicate that this HDevice and all of its child
         * devices are visited recursively.
         */
        VisitThisRecursively
    };

    /*!
     * This enumeration specifies the device types that are considered as
     * \e targets of an operation.
     */
    enum TargetDeviceType
    {
        /*!
         * This value is used to indicate that \b all devices, both root and
         * embedded are the targets of an operation.
         */
        AllDevices,

        /*!
         * This value is used to indicate that \b only embedded devices are the
         * targets of an operation.
         */
        EmbeddedDevices,

        /*!
         * This value is used to indicate that \b only root devices are the
         * targets of an operation.
         */
        RootDevices
    };

    /*!
     * This enumeration specifies the type of a device location URL.
     */
    enum LocationUrlType
    {
        /*!
         * The absolute URL for the device description.
         */
        AbsoluteUrl,

        /*!
         * The base URL of the device. This is the URL with which the various
         * other URLs found in a device description are resolved.
         */
        BaseUrl
    };

protected:

    /*!
     * Creates the services that this UPnP device provides.
     *
     * It is very important to note that every descendant that specifies
     * services \b has to override this. In addition, the override of this method
     * should always call the implementation of the super class too. For instance,
     *
     * \code
     *
     * void HServicesSetupData* MyDeviceType::createServices()
     * {
     *     HServicesSetupData* retVal = SuperClass::createServices();
     *
     *     // create and add the services of this device to the "retVal" variable
     *
     *     return retVal;
     * }
     *
     * \endcode
     *
     * Most commonly this method is called only once when the instance
     * is being initialized by the managing host (HDeviceHost or HControlPoint).
     *
     * \return the services that this HDevice provides.
     *
     * \remarks The HDevice base class takes the ownership of the created
     * objects and will delete them upon its destruction. Because of that,
     * you can store the addresses of the created objects and use them safely
     * throughout the lifetime of this device. However, you \b cannot delete them.
     */
    virtual HServicesSetupData* createServices();

    /*!
     * Creates the embedded devices that this UPnP device provides.
     *
     * It is important to note that every descendant that specifies
     * embedded devices \b should override this. In case it is overridden,
     * the override of this method should always call the implementation
     * of the super class too. For instance,
     *
     * \code
     *
     * void HDevicesSetupData* MyDeviceType::createEmbeddedDevices()
     * {
     *     HDevicesSetupData* retVal = SuperClass::createEmbeddedDevices();
     *
     *     // create and add the embedded devices of this device to the
     *     // "retVal" variable
     *
     *     return retVal;
     * }
     *
     * \endcode
     *
     * Most commonly this method is called only once when the instance
     * is being initialized by the managing host (HDeviceHost or HControlPoint).
     *
     * \return the services that this HDevice provides.
     *
     * \remarks The HDevice base class takes the ownership of the created
     * objects and will delete them upon its destruction. Because of that,
     * you can store the addresses of the created objects and use them safely
     * throughout the lifetime of this device. However, you \b cannot delete them.
     */
    virtual HDevicesSetupData* createEmbeddedDevices();

    /*!
     * Provides the opportunity to do post-construction initialization routines
     * in derived classes.
     *
     * As \c %HDevice is part of the HUPnP's \ref devicemodel
     * the object creation process is driven by HUPnP. At the time
     * of instantiation of a descendant \c %HDevice the base \c %HDevice
     * sub-object is not yet fully set up. In other words, at that time
     * it is not guaranteed that every private or protected member of a
     * \c %HDevice is set to its "final" value that is used once the object
     * is fully initialized and ready to be used.
     *
     * Because of the above, descendants of
     * \c %HDevice should not reference or rely on values of \c %HDevice at
     * the time of construction. If the initialization of a \c %HDevice
     * descendant needs to do things that rely on \c %HDevice being fully
     * set up, you can override this method. This method is called \b once
     * right after the base \c %HDevice is fully initialized.
     *
     * \param errDescription
     *
     * \return \e true in case the initialization succeeded.
     *
     * \note It is advisable to keep the constructors of the descendants of
     * \c %HDevice small and fast, and do more involved initialization routines
     * here.
     */
    virtual bool finalizeInit(QString* errDescription);

protected:

    HDevicePrivate* h_ptr;

    //
    // \internal
    //
    // Constructor for derived classes for re-using the h_ptr.
    //
    HDevice(HDevicePrivate& dd);

    /*!
     * Creates a new instance.
     *
     * Default constructor for derived classes.
     */
    HDevice();

public:

    /*!
     * Destroys the instance.
     *
     * Destroys the instance.
     */
    virtual ~HDevice() = 0;

    /*!
     * Returns the parent device of this device, if any.
     *
     * \return the parent device of this device, if any, or a null pointer
     * in case the device is a root device.
     *
     * \remarks the pointer is guaranteed to
     * point to the parent object throughout the lifetime of this object.
     */
    HDevice* parentDevice() const;

    /*!
     * Returns the root device of the device tree to which this device belongs.
     *
     * \return the root device of the device tree to which this device belongs.
     *
     * \remarks this device could be the root device of the device tree in question,
     * in which case a pointer to this instance is returned.
     */
    HDevice* rootDevice() const;

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
     * Returns the services this device exports.
     *
     * \return the services this device exports. The collection is empty
     * if the device has no services.
     *
     * \remarks the pointers are guaranteed to be valid only throughout the lifetime
     * of this object.
     */
    HServices services() const;

    /*!
     * Returns the services of a specific UPnP service type.
     *
     * \param serviceType specifies the UPnP service type of interest.
     * Only services matching the type are returned.
     * \param versionMatch specifies how the version information in argument
     * \c serviceType should be used. The default is <em>inclusive match</em>,
     * which essentially means that any service with a service type version that
     * is \b less than or \b equal to the version specified in argument
     * \c serviceType is successfully matched.
     *
     * \return the services of the specified type.
     *
     * \remarks the pointers are guaranteed to be valid only throughout the lifetime
     * of this object.
     */
    HServices servicesByType(
        const HResourceType& serviceType,
        HResourceType::VersionMatch versionMatch = HResourceType::InclusiveVersionMatch) const;

    /*!
     * Returns the embedded devices of this device.
     *
     * \return the embedded devices of this device. The collection is empty
     * if the device has no embedded devices.
     *
     * \remarks the pointers are guaranteed to be valid only throughout the lifetime
     * of this object.
     */
    HDevices embeddedDevices() const;

    /*!
     * Returns information about the device that is read from the
     * device description.
     *
     * \return information about the device that is read from the
     * device description.
     */
    const HDeviceInfo& info() const;

    /*!
     * Returns the UPnP device description of this device.
     *
     * \return the UPnP device description that is associated to this device.
     *
     * \remarks an embedded device returns the same device description as
     * its root device.
     */
    QString description() const;

    /*!
     * Returns a list of locations where the device is currently available.
     *
     * \param urlType specifies whether the returned
     * location URLs are absolute URLs for retrieving the device description.
     * By default absolute URLs are returned and from these URLs the device
     * description should be retrievable.
     *
     * \return a list of locations where the device is currently available.
     */
    QList<QUrl> locations(LocationUrlType urlType=AbsoluteUrl) const;
};

}
}

#endif /* HDEVICE_H_ */
