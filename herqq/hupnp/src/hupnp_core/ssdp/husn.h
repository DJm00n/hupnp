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

#ifndef HUSN_H_
#define HUSN_H_

#include "./../general/hdefs_p.h"

namespace Herqq
{

namespace Upnp
{

class HUdn;
class HUsnPrivate;
class HResourceIdentifier;

/*!
 * A class used to depict a Unique Service Name (USN), which is a composite identifier
 * for an SSDP advertisement.
 *
 * \headerfile husn.h HUsn
 *
 * \remark this class provides an assignment operator that is not thread-safe.
 *
 * \ingroup ssdp
 */
class H_UPNP_CORE_EXPORT HUsn
{

private:

    HUsnPrivate* h_ptr;

public:

    /*!
     * Creates a new, empty instance.
     */
    HUsn ();

    /*!
     * Creates a new USN based on the provided argument. The created USN is valid
     * if the provided argument is valid.
     *
     * \param arg specifies the full USN as string.
     *
     * \sa isValid()
     */
    HUsn(const QString& arg);

    /*!
     * Creates a new instance based on the provided argument. The created USN
     * is valid if the provided UDN is valid. Such a USN identifies a specific device.
     *
     * \param udn specifies the UDN component of the USN.
     */
    HUsn(const HUdn& udn);

    /*!
     * Creates a new instance based on the provided elements. The created USN
     * is valid if the provided UDN is valid.
     *
     * \param udn specifies the UDN component of the USN.
     *
     * \param resource specifies the resource component of the USN.
     *
     * \sa isValid()
     */
    HUsn(const HUdn& udn, const HResourceIdentifier& resource);

    /*!
     * Creates a copy of the other object.
     */
    HUsn(const HUsn&);

    /*!
     * Assigns the contents of the other to this.
     *
     * \return a reference to this object.
     */
    HUsn& operator=(const HUsn&);

    /*!
     * Destroys the instance.
     */
    ~HUsn();

    /*!
     * Sets the resource component of the USN.
     *
     * \param resource specifies the resource component of the USN.
     *
     * \sa resource()
     */
    void setResource(const HResourceIdentifier& resource);

    /*!
     * Returns the UDN component of the USN.
     *
     * This is always set in a valid USN.
     *
     * \return the UDN component of the USN.
     */
    HUdn udn() const;

    /*!
     * Returns the resource component of the USN.
     *
     * \return the resource component of the USN.
     *
     * \sa setResource()
     */
    HResourceIdentifier resource() const;

    /*!
     * Indicates whether or not the object is a valid USN.
     *
     * A USN is valid when the UDN component is valid. The resource component
     * does not have to be defined.
     *
     * \return \e true in case the object is a valid USN.
     */
    bool isValid() const;

    /*!
     * Returns a string representation of the object.
     *
     * The string follows the format: <c>%HUdn::+%HResourceIdentifier</c>,
     * where
     * \li %HUdn represents the string representation of the HUdn component and
     * \li %HResourceIdentifier represents the string representation of the
     * HResourceIdentifier component
     *
     * \return a string representation of the object.
     */
    QString toString() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the object are logically equivalent.
 *
 * \relates HUsn
 */
H_UPNP_CORE_EXPORT bool operator==(const HUsn&, const HUsn&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the object are not logically equivalent.
 *
 * \relates HUsn
 */
H_UPNP_CORE_EXPORT bool operator!=(const HUsn&, const HUsn&);

}
}

#endif /* HUSN_H_ */
