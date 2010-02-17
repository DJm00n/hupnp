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

#ifndef UPNP_SERVICEID_H_
#define UPNP_SERVICEID_H_

#include "./../general/hdefs_p.h"

class QString;

namespace Herqq
{

namespace Upnp
{

class HServiceIdPrivate;

/*!
 * Class that represents the <em>service identifier</em> of a UPnP service.
 *
 * Service identifiers are found in UPnP device descriptions and they use
 * the following format within services defined by the UPnP Forum:
 * \verbatim urn:upnp-org:serviceId:serviceID \endverbatim
 *
 * In the above format, only the tailing \e serviceID varies.
 * Every service identifier of a standard service type has to begin with <c>urn:upnp-org:serviceId:</c>.
 *
 * With a vendor defined service the format for a service identifier is:
 * \verbatim urn:domain-name:serviceId:serviceID \endverbatim
 *
 * Note, that according to the UDA: <em>Period characters in the Vendor Domain Name
 * MUST be replaced with hyphens in accordance with RFC 2141</em>.
 *
 * In both formats, the last \e serviceID component is the
 * <em>service identifier suffix</em>.
 *
 * \headerfile hserviceid.h HServiceId
 *
 * \remark this class is not thread-safe.
 *
 * \ingroup dataelements
 */
class H_UPNP_CORE_EXPORT HServiceId
{
private:

    HServiceIdPrivate* h_ptr;

public:

    /*!
     * Constructs a new, empty instance.
     *
     * Instance created by this constructor is not valid, i.e. isValid() will return false.
     *
     * \sa isValid
     */
    HServiceId();

    /*!
     * Constructs a new instance.
     *
     * \param serviceId specifies the contents of the object. If the provided
     * argument is invalid, an empty instance is created. The parameter has to
     * follow either of the formats:
     *
     * \li <c>urn:upnp-org:serviceId:serviceID</c> for service identifiers
     * belonging to a standard <em>service type</em>.
     *
     * \li <c>urn:domain-name:serviceId:serviceID</c> for service identifiers
     * belonging to a vendor defined <em>service type</em>.
     *
     * \sa isValid()
     */
    HServiceId(const QString& serviceId);

    /*!
     * Creates a new instance based on the other instance provided.
     *
     * \param other specifies the other instance.
     */
    HServiceId(const HServiceId& other);

    /*!
     * Assigns the contents of the other instance to this.
     *
     * \param other specifies the other instance.
     *
     * \return a reference to this instance.
     */
    HServiceId& operator=(const HServiceId& other);

    /*!
     * Destroys the instance.
     */
    ~HServiceId();

    /*!
     * Indicates if the service identifier is properly defined.
     *
     * \return true in case the service identifier is valid.
     */
    bool isValid() const;

    /*!
     * Indicates whether the service identifier belongs to a standard service type defined
     * by the UPnP forum or to a vendor defined service.
     *
     * \retval true in case the service identifier belongs to a standard service type defined by the UPnP forum.
     * \retval false in case the service identifier belongs to a vendor defined service type or
     * the object is invalid.
     *
     * \sa isValid()
     */
    bool isStandardType() const;

    /*!
     * Returns the URN of the service identifier.
     *
     * \param completeUrn specifies whether the prefix \c urn is returned
     * as well. If the argument is false, only the actual URN is returned. i.e
     * if the service identifier is defined as <c>urn:upnp-org:serviceId:MyServiceId</c>
     * only <c>upnp-org</c> is returned.
     *
     * \returns the URN of the service identifier if the object is valid.
     * If the object is not valid, an empty string is returned.
     *
     * \sa isValid()
     */
    QString urn(bool completeUrn = true) const;

    /*!
     * Returns the service identifier suffix.
     *
     * \returns the service identifier suffix if the object is valid. For instance, if the
     * service identifier is defined as <c>urn:upnp-org:serviceId:MyServiceId</c>,
     * the suffix identifier and thus the value returned is \c "MyServiceId".
     * If the object is not valid, an empty string is returned.
     *
     * \sa isValid()
     */
    QString suffix() const;

    /*!
     * Returns a string representation of the instance.
     *
     * \return a string representation of the instance. The returned string
     * follows the format defined by UDA if the object is valid. In case of a valid
     * object, the return value is the string that was used to construct the object.
     * If the object is invalid, the returned string is empty.
     *
     * \sa isValid()
     */
    QString toString() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the provided objects are equal, false otherwise.
 *
 * \relates HServiceId
 */
H_UPNP_CORE_EXPORT bool operator==(const HServiceId&, const HServiceId&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the provided objects are not equal, false otherwise.
 *
 * \relates HServiceId
 */
H_UPNP_CORE_EXPORT bool operator!=(const HServiceId&, const HServiceId&);

}
}

#endif /* UPNP_SERVICEID_H_ */