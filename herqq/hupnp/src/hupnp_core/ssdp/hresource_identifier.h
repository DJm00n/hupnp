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

struct QUuid;
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
 * \headerfile hresource_identifier.h HResourceIdentifier
 *
 * \remark this class provides an assignment operator that is not thread-safe.
 *
 * \ingroup ssdp
 */
class H_UPNP_CORE_EXPORT HResourceIdentifier
{

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
        Undefined = -1,

        /*!
         * The resource is "all devices".
         */
        AllDevices = 0,

        /*!
         * The resource is "upnp:rootdevice".
         */
        RootDevice = 1,

        /*!
         * The resource is "uuid:device-UUID"
         */
        SpecificDevice = 2,

        /*!
         * The resource is urn:schemas-upnp-org:device:deviceType:ver.
         */
        StandardDeviceType = 3,

        /*!
         * The resource is urn:schemas-upnp-org:service:serviceType:ver.
         */
        StandardServiceType = 4,

        /*!
         * The resource is urn:domain-name:device:deviceType:ver.
         */
        VendorSpecifiedDeviceType = 5,

        /*!
         * The resource is urn:domain-name:service:serviceType:ver.
         */
        VendorSpecifiedServiceType = 6
    };

    /*!
     * Creates a new empty instance. The type is set to HResourceIdentifier::Undefined.
     */
    HResourceIdentifier();

    /*!
     * Creates a new instance, if the specified argument is valid.
     *
     * \param resource specifies the contents of the object. In case the
     * the provided argument does not contain a valid resource, the
     * object's type is set to HResourceIdentifier::Undefined.
     */
    HResourceIdentifier(const QString& resource);

    /*!
     * Creates a new instance, if the specified argument is valid.
     *
     * \param resourceType specifies the contents of the object. In case the
     * the provided argument does not contain a valid resource, the
     * object's type is set to HResourceIdentifier::Undefined.
     */
    HResourceIdentifier(const HResourceType& resourceType);

    /*!
     * Creates a new instance with type set to HResourceIdentifier::SpecificDevice, if
     * the provided argument is valid.
     *
     * \param udn specifies the contents of the object. In case the
     * the provided argument does not contain a valid resource, the
     * object's type is set to HResourceIdentifier::Undefined.
     */
    HResourceIdentifier(const HUdn& udn);

    /*!
     * Destroys the instance.
     */
    ~HResourceIdentifier();

    /*!
     * Copies the contents of the other to this.
     */
    HResourceIdentifier(const HResourceIdentifier&);

    /*!
     * Returns a static instance of a UPnP Root Device Resource Identifier.
     *
     * \return a static instance of a UPnP Root Device Resource Identifier.
     *
     * \remark this is only a helper method. Logically same object can be constructed
     * with the string "upnp:rootdevice".
     */
    static HResourceIdentifier getRootDeviceIdentifier();

    /*!
     * Returns a static instance of a UPnP All Devices Resource Identifier.
     *
     * \return a static instance of a UPnP All Devices Resource Identifier.
     *
     * \remark this is only a helper method. Logically same object can be constructed
     * with the string "ssdp:all".
     */
    static HResourceIdentifier getAllDevicesIdentifier();

    /*!
     * Assigns the contents of the other object to this.
     * \return a reference to this.
     */
    HResourceIdentifier& operator=(const HResourceIdentifier&);

    /*!
     * Returns the type of the object.
     *
     * \return the type of the object. If the object is invalid, the type
     * returned is HResourceIdentifier::Undefined.
     */
    Type type() const;

    /*!
     * Returns the device UUID, in case type() returns
     * HResourceIdentifier::SpecificDevice.
     *
     * \return the device UUID in case type() returns HResourceIdentifier::SpecificDevice.
     * Otherwise a null UUID is returned.
     */
    QUuid deviceUuid() const;

    /*!
     * Returns the resource type associated with this identifier, if any.
     *
     * \return the resource type associated with this identifier.
     *
     * \remark the returned type is valid only when the type of the instance
     * is either standard or vendor specified device or service type.
     */
    HResourceType resourceType() const;

    /*!
     * Returns a string representation of the object.
     *
     * \return a string representation of the object or an empty string,
     * if the object does not specify a valid resource.
     */
    QString toString() const;
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
