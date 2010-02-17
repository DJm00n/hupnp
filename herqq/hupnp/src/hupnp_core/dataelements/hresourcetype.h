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

#ifndef RESOURCE_TYPE_H_
#define RESOURCE_TYPE_H_

#include "./../general/hdefs_p.h"

#include <QStringList>

namespace Herqq
{

namespace Upnp
{

class HResourceTypePrivate;

/*!
 * A class used to depict a UPnP resource, which is either a
 * UPnP device or a UPnP service.
 *
 * Both UPnP device and service descriptions use the \em type concept to a give the
 * corresponding device or service context that can be used in identification.
 * In device descriptions, the device type is specified following the format
 * \verbatim urn:schemas-upnp-org:device:deviceType:ver \endverbatim  or
 * \verbatim urn:domain-name:device:deviceType:ver \endverbatim in case of a vendor defined type.
 *
 * In service descriptions, the service type is specified as
 * \verbatim urn:schemas-upnp-org:service:serviceType:ver \endverbatim  or
 * \verbatim urn:domain-name:service:serviceType:ver \endverbatim in case of a vendor defined type.
 *
 * For more information, see the <em>device type</em> and <em>service type</em> definitions
 * in UDA v1.1 at pages 44 and 46, respectively.
 *
 * This class abstracts the above service and device type concepts to a \em resource
 * and helps in handling the various elements of a <em>resource type</em>.
 *
 * \headerfile hresourcetype.h HResourceType
 *
 * \remark this class is not thread-safe, but it is lightweight to be used by-value.
 *
 * \ingroup dataelements
 */
class H_UPNP_CORE_EXPORT HResourceType
{
private:

    QStringList m_resourceElements;

public:

    /*!
     * Constructs a new, empty instance.
     *
     * Instance created by this constructor is not valid, i.e. isValid() will return false.
     *
     * \sa isValid()
     */
    HResourceType();

    /*!
     * Constructs a new, empty instance from the specified parameter.
     *
     * \param resourceTypeAsStr specifies the resource type following the one of
     * the following formats:
     *
     * \li <c>urn:schemas-upnp-org:device:deviceType:ver</c> for standard
     * <em>device type</em>
     *
     * \li <c>urn:domain-name:device:deviceType:ver</c> for vendor defined
     * <em>device type</em>
     *
     * \li <c>urn:schemas-upnp-org:service:serviceType:ver</c> for standard
     * <em>service type</em>
     *
     * \li <c>urn:domain-name:service:serviceType:ver for</c> vendor defined
     * <em>service type</em>
     *
     * \sa isValid()
     */
    HResourceType(const QString& resourceTypeAsStr);

    /*!
     * Destroys the instance.
     */
    ~HResourceType();

    /*!
     * Indicates if the object is valid.
     *
     * \returns true in case the object represents a valid resource type.
     */
    bool isValid() const;

    /*!
     * Indicates whether or not the resource type is a standard type defined
     * by the UPnP forum.
     *
     * \retval true in case the resource type is defined by the UPnP forum.
     * \retval false in case the resource type is vendor defined.
     */
    bool isStandardType() const;

    /*!
     * Returns the resource URN.
     *
     * \param completeUrn specifies whether or not the prefix \c urn is returned
     * as well. If the argument is false, only the actual URN is returned. i.e
     * if the resource type is defined as <c>urn:schemas-upnp-org:device:deviceType:ver</c>
     * only <c>schemas-upnp-org</c> is returned.
     *
     * \returns the resource URN if the object is valid. Otherwise an empty
     * string is returned.
     *
     * \sa isValid()
     */
    QString resourceUrn(bool completeUrn = true) const;

    /*!
     * Returns the type identifier of the resource.
     *
     * \returns the type identifier of the resource if the object is valid. For instance, if the
     * resource type is defined as <c>urn:schemas-upnp-org:device:deviceType:ver</c>,
     * the resource type identifier and thus the value returned is \c "device".
     * Otherwise an empty string is returned.
     *
     * \sa isValid()
     */
    QString type() const;

    /*!
     * Returns the type suffix of the resource.
     *
     * \param includeVersion specifies whether or not the \em version of the resource is included
     * in the return value.
     *
     * \return the type suffix of the resource if the object is valid. For instance, if the
     * resource type is defined as <c>urn:schemas-upnp-org:device:deviceType:ver</c>,
     * the <em>resource type suffix</em> and thus the value returned is either
     * <c>deviceType:ver</c> or <c>deviceType</c> depending on whether the
     * version information if included. If the object is invalid, an empty string is returned.
     *
     * \sa isValid()
     */
    QString typeSuffix(bool includeVersion = true) const;

    /*!
     * Returns the <em>type identifier</em> and the <em>type suffix</em> as they appear in the
     * specified resource.
     *
     * \param includeVersion specifies whether or not to include the
     * <em>version number</em> after the \em type.
     *
     * \return the <em>type identifier</em> and the <em>type suffix</em> as they appear in the
     * specified resource if the object is valid. For instance, if the
     * <em>complete resource type</em> with URN is defined as <c>urn:schemas-upnp-org:device:deviceType:ver</c>,
     * the value returned is either <c>device:deviceType:ver</c> or <c>device:deviceType</c>,
     * depending on whether or not the version information is included.
     * If the object is invalid, an empty string is returned.
     *
     * \sa isValid()
     */
    QString completeType(bool includeVersion = true) const;

    /*!
     * Returns the type identifier and the type suffix as they appear in the
     * specified resource prefixed with the URN to which the type belongs.
     *
     * \param includeVersion specifies whether or not to include the
     * <em>version number</em> after the \em type.
     *
     * \return the <em>resource URN</em>, <em>type identifier</em> and the
     * <em>type suffix</em> as they appear in the
     * specified resource if the object is valid. For instance, if the
     * resource type is defined as <c>urn:schemas-upnp-org:device:deviceType:ver</c>,
     * the value returned is either <c>urn:schemas-upnp-org:device:deviceType:ver</c>,
     * or <c>urn:schemas-upnp-org:device:deviceType</c>,
     * depending on whether or not the version information is included.
     * If the object is invalid, an empty string is returned.
     *
     * \sa isValid()
     */
    QString completeTypeWithUrn(bool includeVersion = true) const;

    /*!
     * Returns the version of the resource type.
     *
     * \returns the version of the resource type if the object is valid.
     * In case the object is invalid -1 is returned.
     *
     * \sa isValid()
     */
    qint32 version() const;

    /*!
     * Returns a string representation of the object.
     *
     * \return a string representation of the object. This is the same as calling
     * completeTypeWithUrn() with argument \c true.
     */
    QString toString() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return true in case the object are logically equivalent.
 *
 * \relates HResourceType
 */
H_UPNP_CORE_EXPORT bool operator==(const HResourceType&, const HResourceType&);

/*!
 * Compares the two objects for inequality.
 *
 * \return true in case the object are not logically equivalent.
 *
 * \relates HResourceType
 */
H_UPNP_CORE_EXPORT bool operator!=(const HResourceType&, const HResourceType&);

/*!
 * Returns a value that can be used as a unique key in a hash-map identifying
 * the resource type object.
 *
 * \param key specifies the <em>resource type</em> from which the hash value is created.
 *
 * \return a value that can be used as a unique key in a hash-map identifying
 * the resource type object.
 *
 * \relates HResourceType
 */
H_UPNP_CORE_EXPORT quint32 qHash(const HResourceType& key);

}
}

#endif /* RESOURCE_TYPE_H_ */
