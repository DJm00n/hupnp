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

#include "./../general/hdefs_p.h"

#include <QString>

namespace Herqq
{

namespace Upnp
{

/*!
 * This class represents a <em>product token</em> as defined in the RFC 2616,
 * section 3.8.
 *
 * \headerfile hproduct_tokens.h HProductToken
 *
 * \remark this class is not thread-safe, but it is lightweight to be used by-value.
 *
 * \ingroup dataelements
 */
class H_UPNP_CORE_EXPORT HProductToken
{
friend H_UPNP_CORE_EXPORT bool operator==(
    const HProductToken& obj1, const HProductToken& obj2);

private:

    QString m_token;
    QString m_productVersion;

public:

    /*!
     * Creates a new, empty instance. An object created with the default
     * constructor is invalid.
     *
     * \sa isValid()
     */
    HProductToken();

    /*!
     * Creates a new object based on the provided token data. If the token data
     * is invalid, the object will be invalid as well.
     *
     * \param token specifies the token part, which is supposed to identify
     * a product. If this is empty, the created object will be invalid.
     *
     * \param productVersion specifies the version part. If this is empty,
     * the created object will be invalid.
     *
     * \sa isValid()
     */
    HProductToken(const QString& token, const QString& productVersion);

    /*!
     * Destroys the instance.
     */
    ~HProductToken();

    /*!
     * Indicates if the object is valid, i.e both the token and the product
     * version are defined.
     *
     * \return true in case both the \e token and <em>product version</em> are appropriately
     * specified.
     *
     * \sa token(), productVersion()
     */
    bool isValid() const;

    /*!
     * Returns the \e token part.
     *
     * \return the \e token part in case the object is valid.
     * The token part is used to identify the product and an example
     * of a token is for instance \c "Apache". An empty string is returned in case
     * the object is invalid.
     *
     * \sa isValid()
     */
    QString token() const;

    /*!
     * Returns the \e version part.
     *
     * \return the \e version part in case the object is valid. An example of a
     * version part is \c "1.0". An empty string is returned in case
     * the object is invalid.
     *
     * \sa isValid()
     */
    QString version() const;

    /*!
     * Returns a string representation of the object.
     *
     * The format of the returned string is \c "token"/"version".
     *
     * \return a string representation of the object.
     */
    QString toString() const;

    /*!
     * Indicates if the provided product token is a valid \e UPnP \ token.
     *
     * A valid \e UPnP \e token takes the form <c>UPnP/majorVersion.minorVersion</c>.
     *
     * \return \e true in case the provided token object represents a valid
     * \e UPnP \e token.
     */
    static bool isValidUpnpToken(const HProductToken&);

    /*!
     * Attempts to parse the \e version part of a product token to a major and
     * minor component and returns the minor component if the parse was successful.
     *
     * \return the minor version component of the specified product token or -1
     * if the specified token does not contain a minor version component that
     * can be represented as an integer.
     */
    static qint32 minorVersion(const HProductToken&);

    /*!
     * Attempts to parse the \e version part of a product token to a major and
     * minor component and returns the major component if the parse was successful.
     *
     * \return the major version component of the specified product token or -1
     * if the specified token does not contain a major version component that
     * can be represented as an integer.
     */
    static qint32 majorVersion(const HProductToken&);
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the object are logically equivalent.
 *
 * \relates HProductToken
 */
H_UPNP_CORE_EXPORT bool operator==(const HProductToken&, const HProductToken&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the object are not logically equivalent.
 *
 * \relates HProductToken
 */
H_UPNP_CORE_EXPORT bool operator!=(const HProductToken&, const HProductToken&);

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
 * \headerfile hproduct_tokens.h HProductTokens
 *
 * \remark this class is not thread-safe.
 *
 * \ingroup dataelements
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
     * \param arg specifies the product tokens. In case the specified argument
     * does not contain three product tokens as specified in the UDA, the created
     * object will be invalid.
     *
     * \sa isValid()
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
     * Indicates whether or not the object represents product tokens as
     * defined in UDA.
     *
     * \return \e true in case the object represents product tokens as
     * defined in UDA.
     */
    bool isValid() const;

    /*!
     * Returns the product token that defines information of an operating system.
     *
     * \return the product token that defines information of an operating system.
     *
     * \remark The returned object is invalid in case this object is invalid.
     */
    HProductToken osToken() const;

    /*!
     * Returns the product token that defines UPnP version.
     *
     * \return the product token that defines UPnP version. This token always
     * follows the format "UPnP"/majorVersion.minorVersion, where \e majorVersion
     * and \e minorVersion are positive integers. Furthermore, at the moment
     * the \e majorVersion is \b always 1 and the \e minorVersion is either 0
     * or 1.
     *
     * \remark The returned object is invalid in case this object is invalid.
     */
    HProductToken upnpToken() const;

    /*!
     * Returns the product token that defines the actual product in the form
     * product name/product version.
     *
     * \return the product token that defines the actual product in the form
     * product name/product version.
     *
     * \remark The returned object is invalid in case this object is invalid.
     */
    HProductToken productToken() const;

    /*!
     * Returns a string representation of the object.
     *
     * \return a string representation of the object. An empty string is returned
     * in case the object is invalid.
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
