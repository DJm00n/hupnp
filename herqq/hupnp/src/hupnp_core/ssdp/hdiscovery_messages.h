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

#ifndef DISCOVERY_MSGS_H_
#define DISCOVERY_MSGS_H_

#include "./../general/hdefs_p.h"

class QUrl;
class QString;
class QDateTime;

namespace Herqq
{

namespace Upnp
{

class HUsn;
class HEndpoint;
class HProductTokens;
class HResourceIdentifier;
class HResourceAvailablePrivate;

/*!
 * A class that represents the <em>resource available</em> (ssdp:alive) message.
 *
 * According to the UDA, <em>When a device is added to the network,
 * it MUST multicast discovery messages to advertise its root device, any embedded devices,
 * and any services</em>. In HUPnP, this class represents such an advertisement.
 *
 * Usually, you create instances of this class to be sent by the Herqq::Upnp::HSsdp,
 * or you receive instances of this class from the Herqq::Upnp::HSsdp.
 *
 * \headerfile hdiscovery_messages.h HResourceAvailable
 *
 * \ingroup ssdp
 *
 * \remark the class provides an assignment operator, which is not thread-safe.
 *
 * \sa HSsdp
 */
class H_UPNP_CORE_EXPORT HResourceAvailable
{
private:

    HResourceAvailablePrivate* h_ptr;

public:

    /*!
     * Constructs a new, empty instance. The constructed object is not valid,
     * i.e isValid() returns false.
     *
     * \sa isValid()
     */
    HResourceAvailable();

    /*!
     * Constructs a new instance using the specified parameters.
     *
     * \param cacheControlMaxAge specifies the number of seconds the
     * advertisement is valid.
     *
     * \param location specifies the URL to the UPnP description of the root device.
     * If the location is invalid or empty, the created object will be invalid.
     *
     * \param serverTokens specifies information about the host, the UPnP version
     * used and of the product. Note that if this parameter specifies
     * UPnP version 1.1 or later, \c bootId and \c configId have to be properly defined.
     * Otherwise the created object will be invalid.
     *
     * \param usn specifies the Unique Service Name. If the USN is valid, the created
     * object will be invalid as well.
     *
     * \param bootId specifies the \c BOOTID.UPNP.ORG header value. Note that
     * this is mandatory in UDA v1.1, whereas it is not specified at all in
     * UDA v1.0. Because of this, the class requires a valid value (>= 0) only in case
     * the mandatory \c serverTokens identify UPnP v1.1 or later.
     *
     * \param configId specifies the \c CONFIGID.UPNP.ORG header value. Note that
     * this is mandatory in UDA v1.1, whereas it is not specified at all in
     * UDA v1.0. Because of this, the class requires a valid value (>= 0) only in case
     * the mandatory \c serverTokens identify UPnP v1.1 or later.
     *
     * \param searchPort specifies the \c SEARCHPORT.UPNP.ORG header value. Note that
     * this is optional in UDA v1.1, whereas it is not specified at all in UDA v1.0.
     * If specified, this is the port at which the device must listen for unicast
     * \c M-SEARCH messages. Otherwise the port is the default \c 1900.
     * This parameter is always optional.
     *
     * \remark
     * \li if cacheControlMaxAge is smaller than 5, it it set to 5 or if it is
     * larger than 60 * 60 * 24 (a day in seconds), it is set to a day measured in seconds.
     *
     * \li if searchPort is smaller than 49152 or larger than 65535 it is set to -1.
     * The range is specified in UDA v1.1.
     *
     * \sa isValid()
     */
    HResourceAvailable(
        quint32        cacheControlMaxAge,
        const QUrl&    location,
        const HProductTokens& serverTokens,
        const HUsn&    usn,
        qint32         bootId = -1,
        qint32         configId = -1,
        qint32         searchPort = -1);

    /*!
     * Destroys the instance.
     */
    ~HResourceAvailable();

    /*!
     * Copies the contents of the other object instance to this.
     */
    HResourceAvailable(const HResourceAvailable&);

    /*!
     * Assigns the contents of the other object instance to this.
     *
     * \remark this is not thread-safe.
     */
    HResourceAvailable& operator=(const HResourceAvailable&);

    /*!
     * Indicates whether or not the object contains valid announcement information.
     *
     * \return \e true in case the objects contains valid announcement information.
     */
    bool isValid() const;

    /*!
     * Returns the server tokens.
     *
     * \return the server tokens. The returned object will be invalid if this
     * object is invalid.
     *
     * \sa isValid()
     */
    HProductTokens serverTokens() const;

    /*!
     * Returns the location of the announced device.
     *
     * \return the location of the announced device. This is the URL where
     * the <em>device description</em> can be retrieved. The returned object
     * will be empty if this object is invalid.
     *
     * \sa isValid()
     */
    QUrl location() const;

    /*!
     * Returns the Unique Service Name.
     *
     * USN identifies a unique \e device or \e service instance.
     *
     * \return the Unique Service Name. The returned object will be invalid if this
     * object is invalid.
     *
     * \sa isValid()
     */
    HUsn usn() const;

    /*!
     * Returns the number of seconds the advertisement is valid.
     *
     * \return the number of seconds the advertisement is valid.
     */
    qint32 cacheControlMaxAge() const;

    /*!
     * Returns the value of \c BOOTID.UPNP.ORG.
     *
     * \return the value of \c BOOTID.UPNP.ORG.
     */
    qint32 bootId() const;

    /*!
     * Returns the value of \c CONFIGID.UPNP.ORG.
     *
     * \return the value of \c CONFIGID.UPNP.ORG.
     */
    qint32 configId() const;

    /*!
     * Returns the value of \c SEARCHPORT.UPNP.ORG header field.
     *
     * \return the value of \c SEARCHPORT.UPNP.ORG header field. If the value is not
     * specified upon construction of the object, -1 is returned.
     */
    qint32 searchPort() const;

    /*!
     * Returns the string representation of the contents of the message.
     *
     * \return the string representation of the contents of the message.
     *
     * The string is formatted to be sent as a SSDP (over HTTP) message as such.
     */
    QString toString() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HResourceAvailable
 */
H_UPNP_CORE_EXPORT bool operator==(
    const HResourceAvailable&, const HResourceAvailable&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HResourceAvailable
 */
H_UPNP_CORE_EXPORT bool operator!=(
    const HResourceAvailable&, const HResourceAvailable&);

class HResourceUnavailablePrivate;

/*!
 * Class that represents the device unavailable (ssdp:byebye) message.
 *
 * According to the UDA, <em>When a device and its services are going to be
 * removed from the network, the device SHOULD multicast an ssdp:byebye message
 * corresponding to each of the ssdp:alive messages it multicasted that have not
 * already expired</em>. In HUPnP, this class represents such a message.
 *
 * Usually, you create instances of this class to be sent by the Herqq::Upnp::HSsdp,
 * or you receive instances of this class from the Herqq::Upnp::HSsdp.
 *
 * \headerfile hdiscovery_messages.h HResourceUnavailable
 *
 * \ingroup ssdp
 *
 * \remark the class provides an assignment operator, which is not thread-safe.
 *
 * \sa HSsdp
 */
class H_UPNP_CORE_EXPORT HResourceUnavailable
{
private:

    HResourceUnavailablePrivate* h_ptr;

public:

    /*!
     * Constructs a new, empty instance. The constructed object is not valid,
     * i.e isValid() returns false.
     *
     * \sa isValid()
     */
    HResourceUnavailable();

    /*!
     * Creates a new instance.
     *
     * \param usn specifies the Unique Service Name. If the USN is valid, the created
     * object will be invalid as well.
     *
     * \param sourceLocation
     *
     * \param bootId specifies the \c BOOTID.UPNP.ORG header value. Note that
     * this is mandatory in UDA v1.1, whereas it is not specified at all in
     * UDA v1.0. Because of this, the class requires a valid value (>= 0) only in case
     * the mandatory \c serverTokens identify UPnP v1.1 or later.
     *
     * \param configId specifies the \c CONFIGID.UPNP.ORG header value. Note that
     * this is mandatory in UDA v1.1, whereas it is not specified at all in
     * UDA v1.0. Because of this, the class requires a valid value (>= 0) only in case
     * the mandatory \c serverTokens identify UPnP v1.1 or later.
     *
     * \sa isValid()
     */
    HResourceUnavailable(
        const HUsn& usn, const HEndpoint& sourceLocation,
        qint32 bootId = -1, qint32 configId = -1);

    /*!
     * Destroys the instance.
     */
    ~HResourceUnavailable();

    /*!
     * Copies the contents of the other to this object.
     */
    HResourceUnavailable(const HResourceUnavailable&);

    /*!
     * Assigns the contents of the other to this.
     *
     * \return a reference to this object.
     *
     * \remark this method is not thread-safe.
     */
    HResourceUnavailable& operator=(const HResourceUnavailable&);

    /*!
     * Indicates whether or not the object contains valid announcement information.
     *
     * \return \e true in case the objects contains valid announcement information.
     */
    bool isValid() const;

    /*!
     * Returns the Unique Service Name.
     *
     * USN identifies a unique \e device or \e service instance.
     *
     * \return the Unique Service Name. The returned object will be invalid if this
     * object is invalid.
     *
     * \sa isValid()
     */
    HUsn usn() const;

    /*!
     * Returns the value of \c BOOTID.UPNP.ORG.
     *
     * \return the value of \c BOOTID.UPNP.ORG.
     */
    qint32 bootId() const;

    /*!
     * Returns the value of \c CONFIGID.UPNP.ORG.
     *
     * \return the value of \c CONFIGID.UPNP.ORG.
     */
    qint32 configId() const;

    /*!
     * Returns the location of the device that went offline.
     *
     * \return the location of the device that went offline. This is the URL from where
     * the device description could be retrieved when the device was online.
     */
    HEndpoint location() const;

    /*!
     * Returns the string representation of the contents of the message.
     *
     * \return the string representation of the contents of the message.
     *
     * The string is formatted to be sent as a SSDP (over HTTP) message as such.
     */
    QString toString() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HResourceUnavailable
 */
H_UPNP_CORE_EXPORT bool operator==(
    const HResourceUnavailable&, const HResourceUnavailable&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HResourceUnavailable
 */
H_UPNP_CORE_EXPORT bool operator!=(
    const HResourceUnavailable&, const HResourceUnavailable&);

class HResourceUpdatePrivate;

/*!
 * Class representing the device update (ssdp:update) message.
 *
 * Usually, you create instances of this class to be sent by the Herqq::Upnp::HSsdp,
 * or you receive instances of this class from the Herqq::Upnp::HSsdp.
 *
 * \headerfile hdiscovery_messages.h HResourceUpdate
 *
 * \ingroup ssdp
 *
 * \remark the class provides an assignment operator, which is not thread-safe.
 *
 * \sa HSsdp
 */
class H_UPNP_CORE_EXPORT HResourceUpdate
{
private:

    HResourceUpdatePrivate* h_ptr;

public:

    /*!
     * Constructs a new, empty instance. The constructed object is not valid,
     * i.e isValid() returns false.
     *
     * \sa isValid()
     */
    HResourceUpdate();

    /*!
     * Constructs a new instance using the specified parameters.
     *
     * \param location specifies the URL to the UPnP description of the root device.
     * If the location is invalid or empty, the created object will be invalid.
     *
     * \param usn specifies the Unique Service Name. If the USN is valid, the created
     * object will be invalid as well.
     *
     * \param bootId specifies the \c BOOTID.UPNP.ORG header value. Note that
     * this is mandatory in UDA v1.1, whereas it is not specified at all in
     * UDA v1.0. Because of this, the class requires a valid value (>= 0) only in case
     * the mandatory \c serverTokens identify UPnP v1.1 or later.
     *
     * \param configId specifies the \c CONFIGID.UPNP.ORG header value. Note that
     * this is mandatory in UDA v1.1, whereas it is not specified at all in
     * UDA v1.0. Because of this, the class requires a valid value (>= 0) only in case
     * the mandatory \c serverTokens identify UPnP v1.1 or later.
     *
     * \param nextBootId
     *
     * \param searchPort specifies the \c SEARCHPORT.UPNP.ORG header value. Note that
     * this is optional in UDA v1.1, whereas it is not specified at all in UDA v1.0.
     * If specified, this is the port at which the device must listen for unicast
     * \c M-SEARCH messages. Otherwise the port is the default \c 1900.
     * This parameter is always optional.
     *
     * \throws Herqq::HIllegalArgumentException in case bootId, configId or
     * nextBootId is negative, but not all. All of these values are mandatory
     * in UDA v1.1 and if any of these is specified, it is assumed
     * that the message is supposed to be in accordance with the UDA v1.1.
     *
     * \remark if searchPort is smaller than 49152 or larger than 65535 it is set to -1.
     * It is specified in the UPnP v1.1 specification that the value should be
     * in the above mentioned range.
     */
    HResourceUpdate(
        const QUrl& location, const HUsn& usn,
        qint32 bootId = -1, qint32 configId = -1, qint32 nextBootId  = -1,
        qint32 searchPort = -1);

    /*!
     * Destroys the instance.
     */
    ~HResourceUpdate();

    /*!
     * Copies the contents of the other to this.
     */
    HResourceUpdate(const HResourceUpdate&);

    /*!
     * Assigns the contents of the other to this.
     *
     * \return a reference to this object.
     */
    HResourceUpdate& operator=(const HResourceUpdate&);

    /*!
     * Indicates whether or not the object contains valid announcement information.
     *
     * \return \e true in case the objects contains valid announcement information.
     */
    bool isValid() const;

    /*!
     * Returns the location of the announced device.
     *
     * \return the location of the announced device. This is the URL where
     * the <em>device description</em> can be retrieved. The returned object
     * will be empty if this object is invalid.
     *
     * \sa isValid()
     */
    QUrl location() const;

    /*!
     * Returns the Unique Service Name.
     *
     * \return the Unique Service Name.
     */
    HUsn usn() const;

    /*!
     * Returns the value of \c BOOTID.UPNP.ORG.
     *
     * \return the value of \c BOOTID.UPNP.ORG.
     */
    qint32 bootId() const;

    /*!
     * Returns the value of \c CONFIGID.UPNP.ORG.
     *
     * \return the value of \c CONFIGID.UPNP.ORG.
     */
    qint32 configId() const;

    /*!
     * \return
     */
    qint32 nextBootId() const;

    /*!
     * Returns the value of \c SEARCHPORT.UPNP.ORG header field.
     *
     * \return the value of \c SEARCHPORT.UPNP.ORG header field. If the value is not
     * specified upon construction of the object, -1 is returned.
     */
    qint32 searchPort() const;

    /*!
     * Returns the string representation of the contents of the message.
     *
     * \return the string representation of the contents of the message.
     *
     * The string is properly formatted to be sent as a SSDP (over HTTP) message as such.
     */
    QString toString() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically equivalent.
 *
 * \relates HResourceUpdate
 */
H_UPNP_CORE_EXPORT bool operator==(
    const HResourceUpdate&, const HResourceUpdate&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically equivalent.
 *
 * \relates HResourceUpdate
 */
H_UPNP_CORE_EXPORT bool operator!=(
    const HResourceUpdate&, const HResourceUpdate&);

class HDiscoveryRequestPrivate;

/*!
 * Class representing an M-SEARCH (ssdp:discover) message.
 *
 * Usually, you create instances of this class to be sent by the Herqq::Upnp::HSsdp,
 * or you receive instances of this class from the Herqq::Upnp::HSsdp.
 *
 * \headerfile hdiscovery_messages.h HDiscoveryRequest
 *
 * \ingroup ssdp
 *
 * \remark the class provides an assignment operator, which is not thread-safe.
 *
 * \sa HSsdp
 */
class H_UPNP_CORE_EXPORT HDiscoveryRequest
{

private:

    HDiscoveryRequestPrivate* h_ptr;

public:

    /*!
     * Constructs a new, empty instance. The constructed object is not valid,
     * i.e isValid() returns false.
     *
     * \sa isValid()
     */
    HDiscoveryRequest();

    /*!
     * Creates a new instance based on the provided parameters. The constructed
     * object will be invalid, i.e. isValid() returns false in case the provided
     * information is invalid.
     *
     * \param mx specifies the maximum wait time in seconds.
     *
     * \param resource specifies the Search Target (ST). If the object is invalid,
     * the created object will be invalid.
     *
     * \param userAgent specifies information about the requester.
     * If the object is invalid, the created object will be invalid.
     *
     * \remark if mx is smaller than 1, it is set to 1. If mx is larger than 5, it
     * it set to 5.
     *
     * \sa isValid(), mx(), searchTarget(), userAgent()
     */
    HDiscoveryRequest(
        qint32 mx, const HResourceIdentifier& resource, const HProductTokens& userAgent);

    /*!
     * Destroys the instance.
     */
    ~HDiscoveryRequest();

    /*!
     * Copy constructor. Makes a deep copy.
     */
    HDiscoveryRequest(const HDiscoveryRequest&);

    /*!
     * Assigns the contents of the other to this. Makes a deep copy.
     *
     * \return a reference to this.
     */
    HDiscoveryRequest& operator=(const HDiscoveryRequest&);

    /*!
     * Indicates whether or not the object contains valid announcement information.
     *
     * \return \e true in case the objects contains valid announcement information.
     */
    bool isValid() const;

    /*!
     * Returns the Search Target of the request.
     *
     * \return the Search Target of the request.
     */
    HResourceIdentifier searchTarget() const;

    /*!
     * Returns the maximum wait time in seconds.
     *
     * According to UDA,
     * <em>Device responses SHOULD be delayed a random duration between 0 and
     * this many seconds to balance load for the control point when it processes responses</em>.
     *
     * \return the maximum wait time in seconds.
     */
    qint32 mx() const;

    /*!
     * Returns information about the maker of the request.
     *
     * \return information about the maker of the request.
     */
    HProductTokens userAgent() const;

    /*!
     * Returns a string representation of the contents of the message.
     *
     * \return a string representation of the contents of the message.
     * The string is formatted to be sent as a SSDP message as such.
     */
    QString toString() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return true in case the objects are logically equivalent.
 *
 * \relates HDiscoveryRequest
 */
H_UPNP_CORE_EXPORT bool operator==(
    const HDiscoveryRequest&, const HDiscoveryRequest&);

/*!
 * Compares the two objects for inequality.
 *
 * \return true in case the objects are not logically equivalent.
 *
 * \relates HDiscoveryRequest
 */
H_UPNP_CORE_EXPORT bool operator!=(
    const HDiscoveryRequest&, const HDiscoveryRequest&);

class HDiscoveryResponsePrivate;

/*!
 * A class that represents a response to a HDiscoveryRequest.
 *
 * Usually, you create instances of this class to be sent by the Herqq::Upnp::HSsdp,
 * or you receive instances of this class from the Herqq::Upnp::HSsdp.
 *
 * \headerfile hdiscovery_messages.h HDiscoveryResponse
 *
 * \ingroup ssdp
 *
 * \remark the class provides an assignment operator, which is not thread-safe.
 *
 * \sa HSsdp
 */
class H_UPNP_CORE_EXPORT HDiscoveryResponse
{
private:

    HDiscoveryResponsePrivate* h_ptr;

public:

    /*!
     * Constructs a new, empty instance. The constructed object is not valid,
     * i.e isValid() returns false.
     *
     * \sa isValid()
     */
    HDiscoveryResponse();

    /*!
     * Constructs a new instance using the specified parameters. The constructed
     * object will be invalid, i.e. isValid() returns false in case the provided
     * information is invalid.
     *
     * \param cacheControlMaxAge specifies the number of seconds the
     * advertisement is valid.
     *
     * \param date
     *
     * \param location specifies the URL to the UPnP description of the root device.
     * If the location is invalid or empty, the created object will be invalid.
     *
     * \param serverTokens specifies information about the host, the UPnP version
     * used and of the product. Note that if this parameter specifies
     * UPnP version 1.1 or later, \c bootId and \c configId have to be properly defined.
     * Otherwise the created object will be invalid.
     *
     * \param usn specifies the Unique Service Name. If the USN is valid, the created
     * object will be invalid as well.
     *
     * \param bootId specifies the \c BOOTID.UPNP.ORG header value. Note that
     * this is mandatory in UDA v1.1, whereas it is not specified at all in
     * UDA v1.0. Because of this, the class requires a valid value (>= 0) only in case
     * the mandatory \c serverTokens identify UPnP v1.1 or later.
     *
     * \param configId specifies the \c CONFIGID.UPNP.ORG header value. Note that
     * this is mandatory in UDA v1.1, whereas it is not specified at all in
     * UDA v1.0. Because of this, the class requires a valid value (>= 0) only in case
     * the mandatory \c serverTokens identify UPnP v1.1 or later.
     *
     * \param searchPort specifies the \c SEARCHPORT.UPNP.ORG header value. Note that
     * this is optional in UDA v1.1, whereas it is not specified at all in UDA v1.0.
     * If specified, this is the port at which the device must listen for unicast
     * \c M-SEARCH messages. Otherwise the port is the default \c 1900.
     * This parameter is always optional.
     *
     * \remark
     * \li if cacheControlMaxAge is smaller than 5, it it set to 5 or if it is
     * larger than 60 * 60 * 24 (a day in seconds), it is set to a day measured in seconds.
     *
     * \li if searchPort is smaller than 49152 or larger than 65535 it is set to -1.
     * The range is specified in UDA v1.1.
     *
     * \sa isValid()
     */
    HDiscoveryResponse(
        quint32 cacheControlMaxAge,
        const QDateTime& date,
        const QUrl& location,
        const HProductTokens& serverTokens,
        const HUsn& usn,
        qint32 bootId = -1,
        qint32 configId = 0,
        qint32 searchPort = -1);

    /*!
     * Copy constructor. Makes a deep copy.
     */
    HDiscoveryResponse(const HDiscoveryResponse&);

    /*!
     * Assigns the contents of the other to this. Makes a deep copy.
     *
     * \return reference to this object.
     */
    HDiscoveryResponse& operator=(const HDiscoveryResponse&);

    /*!
     * Destroys the instance.
     */
    ~HDiscoveryResponse();

    /*!
     * Indicates whether or not the object contains valid announcement information.
     *
     * \return true in case the objects contains valid announcement information.
     */
    bool isValid() const;

    /*!
     * Returns the server tokens.
     *
     * \return the server tokens.
     */
    HProductTokens serverTokens() const;

    /*!
     * Returns the date when the response was generated.
     *
     * \return the date when the response was generated.
     */
    QDateTime date() const;

    /*!
     * Returns the Unique Service Name.
     *
     * USN identifies a unique \e device or \e service instance.
     *
     * \return the Unique Service Name. The returned object will be invalid if this
     * object is invalid.
     *
     * \sa isValid()
     */
    HUsn usn() const;

    /*!
     * Returns the location of the announced device.
     *
     * \return the location of the announced device. This is the URL where
     * the <em>device description</em> can be retrieved. The returned object
     * will be empty if this object is invalid.
     *
     * \sa isValid()
     */
    QUrl location() const;

    /*!
     * Returns the number of seconds the advertisement is valid.
     *
     * \return the number of seconds the advertisement is valid.
     */
    qint32 cacheControlMaxAge() const;

    /*!
     * Returns the value of \c BOOTID.UPNP.ORG.
     *
     * \return the value of \c BOOTID.UPNP.ORG.
     */
    qint32 bootId() const;

    /*!
     * Returns the value of \c CONFIGID.UPNP.ORG.
     *
     * \return the value of \c CONFIGID.UPNP.ORG.
     */
    qint32 configId() const;

    /*!
     * Returns the value of \c SEARCHPORT.UPNP.ORG header field.
     *
     * \return the value of \c SEARCHPORT.UPNP.ORG header field. If the value is not
     * specified upon construction of the object, -1 is returned.
     */
    qint32 searchPort() const;

    /*!
     * Returns the string representation of the contents of the message.
     *
     * \return the string representation of the contents of the message.
     *
     * The string is formatted to be sent as a SSDP message (over HTTP) as such.
     */
    QString toString() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the objects are logically the equivalent.
 *
 * \relates HDiscoveryResponse
 */
H_UPNP_CORE_EXPORT bool operator==(
    const HDiscoveryResponse&, const HDiscoveryResponse&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the objects are not logically the equivalent.
 *
 * \relates HDiscoveryResponse
 */
H_UPNP_CORE_EXPORT bool operator!=(
    const HDiscoveryResponse&, const HDiscoveryResponse&);

}
}

#endif /* DISCOVERY_MSGS_H_ */
