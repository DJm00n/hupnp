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

#ifndef HUDN_H_
#define HUDN_H_

#include "./../general/hdefs_p.h"

#include <QUuid>

namespace Herqq
{

namespace Upnp
{

/*!
 * A class used to depict a <em>Unique Device Name</em> (UDN), which is a
 * unique device identifier that has to remain the same over time for a
 * specific device instance.
 *
 * A valid UDN follows the format \c "uuid:"+"xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx",
 * where the five hex fields form up a valid UUID.
 *
 * \headerfile hudn.h HUdn
 *
 * \remark this class is not thread-safe, but it is lightweight to be
 * used by-value.
 *
 * \ingroup dataelements
 */
class H_UPNP_CORE_EXPORT HUdn
{
private:

    QUuid m_value;

public:

    /*!
     * Constructs a new, empty instance.
     *
     * Instance created by this constructor is not valid, i.e. isValid() will return false.
     *
     * \sa isValid
     */
    HUdn();

    /*!
     * Constructs a new instance based on the provided value.
     *
     * \param value specifies the UUID of the UDN. If the provided UUID is invalid,
     * the created HUdn is invalid as well.
     *
     * \sa isValid
     */
    HUdn(const QUuid& value);

    /*!
     * Constructs a new instance based on the provided value.
     *
     * \param value specifies the string from which the object is constructed.
     * The argument has to contain a valid UUID and it can be prefixed with
     * "uuid:". The UUID part in turn must be formatted along the requirements of \c QUuid:
     * the string "must be formatted as five hex fields separated
     * by '-', e.g., "{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}"
     * where 'x' is a hex digit. The curly braces shown here are optional,
     * but it is normal to include them. If the argument does not form a
     * proper UUID, the created UDN is invalid.
     *
     * \sa isValid
     */
    HUdn(const QString& value);

    /*!
     * Destroys the instance.
     */
    ~HUdn();

    /*!
     * Indicates if the UDN is defined or not.
     *
     * \return true in case the UDN is defined.
     */
    inline bool isValid() const { return !m_value.isNull(); }

    /*!
     * Returns the UUID component of the UDN.
     *
     * \return the UUID component of the UDN.
     */
    inline QUuid value() const { return m_value; }

    /*!
     * Returns the complete UDN value.
     *
     * \returns the complete UDN value when the UDN is valid.
     * For instance, \c "uuid:5d794fc2-5c5e-4460-a023-f04a51363300" is a valid UDN.
     * Otherwise an empty string is returned.
     */
    QString toString() const;

    /*!
     * Returns the UUID component of the UDN as string.
     *
     * \returns the UUID component of the UDN as string when the UDN is valid. For instance,
     * if the complete UDN is \c "uuid:5d794fc2-5c5e-4460-a023-f04a51363300", this method
     * will return \c "5d794fc2-5c5e-4460-a023-f04a51363300". Otherwise an
     * empty string is returned.
     */
    QString toSimpleUuid() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return true in case the object are logically equivalent.
 *
 * \relates HUdn
 */
H_UPNP_CORE_EXPORT bool operator==(const HUdn&, const HUdn&);

/*!
 * Compares the two objects for inequality.
 *
 * \return true in case the object are not logically equivalent.
 *
 * \relates HUdn
 */
H_UPNP_CORE_EXPORT bool operator!=(const HUdn&, const HUdn&);

/*!
 * Returns a value that can be used as a unique key in a hash-map identifying
 * the UDN object.
 *
 * \param key specifies the \em UDN from which the hash value is created.
 *
 * \return a value that can be used as a unique key in a hash-map identifying
 * the UDN object.
 *
 * \relates HUdn
 */
H_UPNP_CORE_EXPORT quint32 qHash(const HUdn& key);

}
}

#endif /* HUDN_H_ */
