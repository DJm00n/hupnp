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

#ifndef HPRODUCT_TOKENS_H
#define HPRODUCT_TOKENS_H

#include "../../../../core/include/HGlobal"

#include <QUuid>

namespace Herqq
{

namespace Upnp
{

class HVersionTokenPrivate;

/*!
 * This class represents te \e version \e token part of a \e product \e token.
 *
 * \headerfile product_tokens.h HVersionToken
 *
 * \remark this class provides an assignment operator that is not thread-safe.
 *
 * \ingroup ssdp
 */
class H_UPNP_CORE_EXPORT HVersionToken
{
friend H_UPNP_CORE_EXPORT bool operator==(
    const HVersionToken& obj1, const HVersionToken& obj2);

private:

    HVersionTokenPrivate* h_ptr;

public:

    /*!
     * Creates a new, empty instance. An object created with the default
     * constructor is invalid.
     *
     * \sa isValid()
     */
    HVersionToken();

    /*!
     * Creates a new object based on the provided token data. If the token data
     * is invalid, the object will be invalid as well.
     *
     * \param token specifies the token data in the format
     * \c "UPnP"/majorVersion.minorVersion.
     *
     * \sa isValid()
     */
    HVersionToken(const QString& token);

    /*!
     * Copy constructor.
     */
    HVersionToken(const HVersionToken& other);

    /*!
     * Assignment operator.
     */
    HVersionToken& operator=(const HVersionToken& other);

    /*!
     * Destroys the instance.
     */
    ~HVersionToken();

    /*!
     * Returns the major version of the token.
     *
     * \return the major version of the token when the object is valid.
     * Otherwise -1 is returned.
     */
    qint32 majorVersion() const;

    /*!
     * Returns the minor version of the token.
     *
     * \return the minor version of the token when the object is valid.
     * Otherwise -1 is returned.
     */
    qint32 minorVersion() const;

    /*!
     * Indicates if the object represents a valid UPnP token.
     *
     * \return true in case the object represents a valid UPnP token.
     */
    bool isValid() const;

    /*!
     * Returns a string representation of the object.
     *
     * The format of the returned string is \c "UPnP"/majorversion.minorversion
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
 * \relates HVersionToken
 */
H_UPNP_CORE_EXPORT bool operator==(const HVersionToken&, const HVersionToken&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the object are not logically equivalent.
 *
 * \relates HVersionToken
 *
 * \ingroup ssdp
 */
H_UPNP_CORE_EXPORT bool operator!=(const HVersionToken&, const HVersionToken&);

class HProductTokensPrivate;

/*!
 * This class is used to parse the <em>product tokens</em> defined by HTTP/1.1.
 *
 * According to the HTTP/1.1, <em>Product tokens are used to allow communicating applications
 * to identify themselves by software name and version</em>. In UDA, the <em>product tokens</em>
 * consist of three tokens, in which <em>The first product token identifes the operating system in
 * the form OS name/OS version, the second token represents the UPnP version and
 * MUST be UPnP/1.1, and the third token identifes the product using the form
 * product name/product version</em>. For example, "SERVER: unix/5.1 UPnP/1.1 MyProduct/1.0".
 *
 * \headerfile product_tokens.h HProductTokens
 *
 * \remark this class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HProductTokens
{
private:

    HProductTokensPrivate* h_ptr;

public:

    /*!
     * Constructs a new, empty instance.
     */
    HProductTokens();

    /*!
     * Creates a new instance based on the provided argument.
     *
     * \param arg specifies the product tokens.
     */
    HProductTokens(const QString& arg);

    /*!
     * Creates a copy of the other object.
     */
    HProductTokens(const HProductTokens&);

    /*!
     * Destroys the instance.
     */
    ~HProductTokens();

    /*!
     * Assigns the contents of the other to this.
     *
     * \return a reference to this object.
     */
    HProductTokens& operator=(const HProductTokens&);

    /*!
     * Indicates whether or not the instance is contains a valid set of product
     * tokens or not.
     *
     * \retval true in case the instance contains a valid set of product tokens.
     * \retval false otherwise.
     */
    bool isValid() const;

    /*!
     * Returns the product token that defines Operating System in the form
     * OS name/OS version
     *
     * \return the product token that defines Operating System in the form
     * OS name/OS version.
     */
    QString osToken() const;

    /*!
     * Returns the product token that defines UPnP version.
     *
     * Returns the product token that defines UPnP version.
     */
    HVersionToken upnpToken() const;

    /*!
     * Returns the product token that defines the actual product in the form
     * product name/product version.
     *
     * \return the product token that defines the actual product in the form
     * product name/product version.
     */
    QString productToken() const;

    /*!
     * Returns a string representation of the object.
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
 * \relates HProductTokens
 */
H_UPNP_CORE_EXPORT bool operator==(
    const HProductTokens&, const HProductTokens&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the object are not logically equivalent.
 *
 * \relates HProductTokens
 */
H_UPNP_CORE_EXPORT bool operator!=(
    const HProductTokens&, const HProductTokens&);

}
}

#endif /* HPRODUCT_TOKENS_H */
