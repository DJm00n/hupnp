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

#ifndef HRESOURCE_IDENTIFIER_H_
#define HRESOURCE_IDENTIFIER_H_

#include "./../general/hdefs_p.h"

class QString;

namespace Herqq
{

namespace Upnp
{

class HUdn;
class HResourceType;
class HResourceIdentifierPrivate;

/*!
 * A class that depicts a \e resource found in several SSDP messages.
 *
 *
 *
 * \headerfile hresource_identifier.h HResourceIdentifier
 *
 * \remarks this class provides an assignment operator that is not thread-safe.
 *
 * \ingroup dataelements
 */
class H_UPNP_CORE_EXPORT HResourceIdentifier
{
friend H_UPNP_CORE_EXPORT bool operator==(
    const HResourceIdentifier&, const HResourceIdentifier&);

private:

    HResourceIdentifierPrivate* h_ptr;

public:

    /*!
     * Specifies the type of the resource. See UPnP v1.1 Device Architecture
     * specification for more information.
     */
    enum Type
    {
        /*!
         * No resource defined. This is used when the object is constructed using
         * the default constructor.
         */
        Undefined = 0,

        /*!
         * The resource is "ssdp:all".
         */
        AllDevices,

        /*!
         * The resource is "upnp:rootdevice".
         */
        RootDevices,

        /*!
         * The resource is "uuid:device-UUID::upnp:rootdevice".
         */
        SpecificRootDevice,

        /*!
         * The resource is "uuid:device-UUID".
         */
        SpecificDevice,

        /*!
         * The resource is "urn:schemas-upnp-org:device:deviceType:ver" or
         * "urn:domain-name:device:deviceType:ver".
         */
        DeviceType,

        /*!
         * The resource is "uuid-device-UUID::urn:schemas-upnp-org:device:deviceType:ver"
         * or "uuid-device-UUID::urn:domain-name:device:deviceType:ver".
         */
        SpecificDeviceWithType,

        /*!
         * The resource is "urn:schemas-upnp-org:service:serviceType:ver" or
         * "urn:domain-name:service:serviceType:ver".
         */
        ServiceType,

        /*!
         * The resource is "uuid-device-UUID::urn:schemas-upnp-org:service:serviceType:ver"
         * or "uuid-device-UUID::urn:domain-name:service:serviceType:ver".
         */
        SpecificServiceWithType
    };

    /*!
     * Creates a new empty instance.
     * The type is set to HResourceIdentifier::Undefined.
     *
     * \sa type()
     */
    HResourceIdentifier();

    /*!
     * Creates a new instance.
     *
     * A resource identifier created using a valid UDN only identifies a UPnP
     * device as the resource. The resource identifier does not identify
     * any resource types within the UPnP device.
     *
     * \param udn specifies the contents of the object. In case the provided
     * argument is valid the type() of the created object is
     * HResourceIdentifier::SpecificDevice. Otherwise the type() is set to
     * HResourceIdentifier::Undefined.
     *
     * \param isRootDevice
     *
     * \sa type()
     */
    explicit HResourceIdentifier(const HUdn& udn, bool isRootDevice=false);

    /*!
     * Creates a new instance.
     *
     * A resource identifier created using a valid resource type does not identify
     * a particular UPnP device. To identify a resource type within a UPnP device
     * the UDN has to be set as well.
     *
     * \param resourceType specifies the contents of the object. In case
     * the provided argument is valid the type() of the created object is either
     * - HResourceIdentifier::DeviceType or
     * - HResourceIdentifier::ServiceType
     * depending of the argument. In case the provided argument is invalid the
     * type() is set to HResourceIdentifier::Undefined.
     *
     * \sa type()
     */
    explicit HResourceIdentifier(const HResourceType& resourceType);

    /*!
     * Creates a new instance.
     *
     * A resource identifier created using a valid UDN and a valid resource type
     * identifies a resource within a UPnP device.
     *
     * \param udn
     * \param resourceType
     *
     * \sa type()
     */
    HResourceIdentifier(const HUdn& udn, const HResourceType& resourceType);

    /*!
     * Creates a new instance.
     *
     * \param resource specifies the contents of the object. In case the
     * the provided argument cannot be parsed to a valid resource identifier the
     * type() is set to HResourceIdentifier::Undefined.
     *
     * Valid string formats are:
     *
     * - <c>ssdp:all</c> for identifying "any UPnP resource",
     * - <c>[uuid:device-UUID]</c> for a root device UUID,
     * - <c>[uuid:device-UUID::]upnp:rootdevice</c> for a root device,
     * - <c>[uuid:device-UUID::]urn:schemas-upnp-org:device:deviceType:ver</c>
     * for standard <em>device type</em>
     * - <c>[uuid:device-UUID::]urn:domain-name:device:deviceType:ver</c> for
     * vendor defined <em>device type</em>
     * - <c>[uuid:device-UUID::]urn:schemas-upnp-org:service:serviceType:ver</c>
     * for standard <em>service type</em>
     * - <c>[uuid:device-UUID::]urn:domain-name:service:serviceType:ver for</c>
     * vendor defined <em>service type</em>
     *
     * The content inside square brackets (uuid:device-UUID) is optional and does
     * not have to be provided.
     */
    explicit HResourceIdentifier(const QString& resource);

    /*!
     * Destroys the instance.
     */
    ~HResourceIdentifier();

    /*!
     * Copies the contents of the other object to this.
     */
    HResourceIdentifier(const HResourceIdentifier&);

    /*!
     * Assigns the contents of the other object to this.
     *
     * \return a reference to this object.
     */
    HResourceIdentifier& operator=(const HResourceIdentifier&);

    /*!
     * Returns the type of the object.
     *
     * \return the type of the object. If the resource is not specified the type
     * returned is HResourceIdentifier::Undefined.
     */
    Type type() const;

    /*!
     * Returns the Unique Device Name of the resource.
     *
     * \return the Unique Device Name of the resource if it is set. Note,
     * the UDN is never set when the type() is either
     * HResourceIdentifier::AllDevices or HResourceIdentifier::Undefined.
     */
    HUdn udn() const;

    /*!
     * Sets the UDN of the object.
     *
     * \note Changing the UDN may change the resourceType() and the type()
     * of the object as well. For instance, if the object did not have UDN set before,
     * changing the UDN will change the type() of the object.
     *
     * \param udn specifies the new UDN.
     */
    void setUdn(const HUdn& udn);

    /*!
     * Returns the resource type associated with this identifier, if any.
     *
     * \return the resource type associated with this identifier if it is set.
     * Note, the returned object is valid only when the type() is either
     * standard or vendor specified device or service type.
     */
    HResourceType resourceType() const;

    /*!
     * Sets the resource type of the object.
     *
     * \param resourceType specifies the new resource type.
     */
    void setResourceType(const HResourceType& resourceType);

    /*!
     * Returns a string representation of the object.
     *
     * \return a string representation of the object or an empty string,
     * if the object does not specify a valid resource.
     *
     * \sa Type
     */
    QString toString() const;

    /*!
     * Returns a static instance of a UPnP root device resource identifier.
     *
     * \return a static instance of a UPnP root device resource identifier.
     *
     * \remark this is only a helper method. A logically equivalent object
     * can be constructed * with the string "upnp:rootdevice".
     */
    static HResourceIdentifier createRootDeviceIdentifier();

    /*!
     * Returns a static instance of a UPnP All Devices Resource Identifier.
     *
     * \return a static instance of a UPnP All Devices Resource Identifier.
     *
     * \remark this is only a helper method. A logically equivalent object
     * can be constructed with the string "ssdp:all".
     */
    static HResourceIdentifier createAllDevicesIdentifier();
};

/*!
 * Compares the two objects for equality.
 *
 * \return true in case the objects are logically equivalent.
 *
 * \relates HResourceIdentifier
 */
H_UPNP_CORE_EXPORT bool operator==(
    const HResourceIdentifier&, const HResourceIdentifier&);

/*!
 * Compares the two objects for inequality.
 *
 * \return true in case the objects are not logically equivalent.
 *
 * \relates HResourceIdentifier
 */
H_UPNP_CORE_EXPORT bool operator!=(
    const HResourceIdentifier&, const HResourceIdentifier&);

}
}

#endif /* HRESOURCE_IDENTIFIER_H_ */
