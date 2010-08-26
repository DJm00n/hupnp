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

#ifndef DISCOVERY_MSGS_H_
#define DISCOVERY_MSGS_H_

#include "../general/hdefs.h"
#include "../general/hupnp_global.h"

class QUrl;
class QString;
class QDateTime;

namespace Herqq
{

namespace Upnp
{

class HEndpoint;
class HDiscoveryType;
class HProductTokens;
class HResourceAvailablePrivate;

/*!
 * A class that represents the <em>resource available</em> (ssdp:alive) message.
 *
 * According to the UDA, <em>When a device is added to the network,
 * it MUST multicast discovery messages to advertise its root device, any embedded devices,
 * and any services</em>. In HUPnP this class represents such an advertisement.
 *
 * Usually, you create instances of this class to be sent by the Herqq::Upnp::HSsdp,
 * or you receive instances of this class from the Herqq::Upnp::HSsdp.
 *
 * \headerfile hdiscovery_messages.h HResourceAvailable
 *
 * \ingroup ssdp
 *
 * \remarks the class provides an assignment operator, which is not thread-safe.
 *
 * \sa HSsdp
 */
class H_UPNP_CORE_EXPORT HResourceAvailable
{
friend H_UPNP_CORE_EXPORT bool operator==(
    const HResourceAvailable&, const HResourceAvailable&);

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
     * If the location is invalid or empty the created object will be invalid.
     *
     * \param serverTokens specifies information about the host, the UPnP version
     * used and of the product. Note that if this parameter specifies
     * UPnP version 1.1 or later, \c bootId and \c configId have to be properly defined.
     * Otherwise the created object will be invalid.
     *
     * \note Although server tokens are mandatory according to the UDA,
     * this is not enforced by this class for interoperability reasons.
     *
     * \param usn specifies the Unique Service Name. The created object is valid
     * only if the provided USN is valid.
     *
     * \param bootId specifies the \c BOOTID.UPNP.ORG header value. Note that
     * this is mandatory in UDA v1.1, whereas it is not specified at all in
     * UDA v1.0. Because of this the class requires a valid value (>= 0) only in case
     * the \c serverTokens identify UPnP v1.1 or later.
     *
     * \param configId specifies the \c CONFIGID.UPNP.ORG header value. Note that
     * this is mandatory in UDA v1.1, whereas it is not specified at all in
     * UDA v1.0. Because of this the class requires a valid value (>= 0) only in case
     * the \c serverTokens identify UPnP v1.1 or later.
     *
     * \param searchPort specifies the \c SEARCHPORT.UPNP.ORG header value. Note that
     * this is optional in UDA v1.1, whereas it is not specified at all in UDA v1.0.
     * If specified, this is the port at which the device must listen for unicast
     * \c M-SEARCH messages. Otherwise the port is by default \c 1900.
     * This parameter is optional.
     *
     * \remarks
     * \li if cacheControlMaxAge is smaller than 5 it it set to 5, or if it is
     * larger than 60 * 60 * 24 (a day in seconds) it is set to a day measured in seconds.
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
        const HDiscoveryType& usn,
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
     * \remarks this is not thread-safe.
     */
    HResourceAvailable& operator=(const HResourceAvailable&);

    /*!
     * Indicates whether or not the object contains valid announcement information.
     *
     * \param level indicates whether the check should be strictly according
     * to the UDA specification. If set to false some checks are omitted that
     * are known to be poorly implemented in some UPnP software.
     *
     * \return \e true in case the objects contains valid announcement
     * information in terms of the requested strictness.
     */
    bool isValid(HValidityCheckLevel level) const;

    /*!
     * Returns the server tokens.
     *
     * \return the server tokens. The returned object is invalid if this
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
     * is empty if this object is invalid.
     *
     * \sa isValid()
     */
    QUrl location() const;

    /*!
     * Returns the Unique Service Name.
     *
     * The Unique Service Name identifies a unique \e device or \e service instance.
     *
     * \return the Unique Service Name. The returned object is invalid if this
     * object is invalid.
     *
     * \sa isValid()
     */
    HDiscoveryType usn() const;

    /*!
     * Returns the number of seconds the advertisement is valid.
     *
     * \return the number of seconds the advertisement is valid. If the object
     * is valid the return value is never smaller than 5.
     */
    qint32 cacheControlMaxAge() const;

    /*!
     * Returns the value of \c BOOTID.UPNP.ORG.
     *
     * \return the value of \c BOOTID.UPNP.ORG. If the value is not
     * specified -1 is returned.
     */
    qint32 bootId() const;

    /*!
     * Returns the value of \c CONFIGID.UPNP.ORG.
     *
     * \return the value of \c CONFIGID.UPNP.ORG. If the value is not
     * specified -1 is returned.
     */
    qint32 configId() const;

    /*!
     * Returns the value of \c SEARCHPORT.UPNP.ORG header field.
     *
     * \return the value of \c SEARCHPORT.UPNP.ORG header field. If the value is not
     * specified -1 is returned.
     */
    qint32 searchPort() const;
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
 * already expired</em>. In HUPnP this class represents such a message.
 *
 * Usually you create instances of this class to be sent by the Herqq::Upnp::HSsdp,
 * or you receive instances of this class from the Herqq::Upnp::HSsdp.
 *
 * \headerfile hdiscovery_messages.h HResourceUnavailable
 *
 * \ingroup ssdp
 *
 * \remarks the class provides an assignment operator, which is not thread-safe.
 *
 * \sa HSsdp
 */
class H_UPNP_CORE_EXPORT HResourceUnavailable
{
friend H_UPNP_CORE_EXPORT bool operator==(
    const HResourceUnavailable&, const HResourceUnavailable&);

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
     * \param usn specifies the Unique Service Name. The created object is invalid
     * if the provided USN is invalid.
     *
     * \param bootId specifies the \c BOOTID.UPNP.ORG header value. Note that
     * this is mandatory in UDA v1.1, whereas it is not specified at all in
     * UDA v1.0.
     *
     * \param configId specifies the \c CONFIGID.UPNP.ORG header value. Note that
     * this is mandatory in UDA v1.1, whereas it is not specified at all in
     * UDA v1.0.
     *
     * \sa isValid()
     */
    HResourceUnavailable(
        const HDiscoveryType& usn, qint32 bootId = -1, qint32 configId = -1);

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
     */
    HResourceUnavailable& operator=(const HResourceUnavailable&);

    /*!
     * Indicates whether or not the object contains valid announcement information.
     *
     * \param level indicates whether the check should be strictly according
     * to the UDA specification. If set to false some checks are omitted that
     * are known to be poorly implemented in some UPnP software.
     *
     * \return \e true in case the objects contains valid announcement
     * information in terms of the requested strictness.
     */
    bool isValid(HValidityCheckLevel level) const;

    /*!
     * Returns the Unique Service Name.
     *
     * The Unique Service Name identifies a unique \e device or \e service instance.
     *
     * \return the Unique Service Name. The returned object is invalid if this
     * object is invalid.
     *
     * \sa isValid()
     */
    HDiscoveryType usn() const;

    /*!
     * Returns the value of \c BOOTID.UPNP.ORG.
     *
     * \return the value of \c BOOTID.UPNP.ORG. If the value is not
     * specified -1 is returned.
     */
    qint32 bootId() const;

    /*!
     * Returns the value of \c CONFIGID.UPNP.ORG.
     *
     * \return the value of \c CONFIGID.UPNP.ORG. If the value is not
     * specified -1 is returned.
     */
    qint32 configId() const;

    /*!
     * Returns the IP endpoint of the device that went offline.
     *
     * \return the IP endpoint of the device that went offline.
     */
    HEndpoint location() const;
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
 * \remarks the class provides an assignment operator, which is not thread-safe.
 *
 * \sa HSsdp
 */
class H_UPNP_CORE_EXPORT HResourceUpdate
{
friend H_UPNP_CORE_EXPORT bool operator==(
    const HResourceUpdate&, const HResourceUpdate&);

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
     * If the location is invalid or empty the created object will be invalid.
     *
     * \param usn specifies the Unique Service Name. The created object is invalid
     * if the provided USN is invalid.
     *
     * \param bootId specifies the \c BOOTID.UPNP.ORG header value. Note that
     * this is mandatory in UDA v1.1, whereas it is not specified at all in
     * UDA v1.0.
     *
     * \param configId specifies the \c CONFIGID.UPNP.ORG header value. Note that
     * this is mandatory in UDA v1.1, whereas it is not specified at all in
     * UDA v1.0
     *
     * \param nextBootId
     *
     * \param searchPort specifies the \c SEARCHPORT.UPNP.ORG header value. Note that
     * this is optional in UDA v1.1, whereas it is not specified at all in UDA v1.0.
     * If specified, this is the port at which the device must listen for unicast
     * \c M-SEARCH messages. Otherwise the port is by default \c 1900.
     * This parameter is optional.
     *
     * \remarks if searchPort is smaller than 49152 or larger than 65535 it is set to -1.
     * This is specified in the UDA v1.1.
     */
    HResourceUpdate(
        const QUrl& location, const HDiscoveryType& usn,
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
     * \param level indicates whether the check should be strictly according
     * to the UDA specification. If set to false some checks are omitted that
     * are known to be poorly implemented in some UPnP software.
     *
     * \return \e true in case the objects contains valid announcement
     * information in terms of the requested strictness.
     */
    bool isValid(HValidityCheckLevel level) const;

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
    HDiscoveryType usn() const;

    /*!
     * Returns the value of \c BOOTID.UPNP.ORG.
     *
     * \return the value of \c BOOTID.UPNP.ORG.
     * If the value is not specified -1 is returned.
     */
    qint32 bootId() const;

    /*!
     * Returns the value of \c CONFIGID.UPNP.ORG.
     *
     * \return the value of \c CONFIGID.UPNP.ORG.
     * If the value is not specified -1 is returned.
     */
    qint32 configId() const;

    /*!
     * \return
     * If the value is not specified -1 is returned.
     */
    qint32 nextBootId() const;

    /*!
     * Returns the value of \c SEARCHPORT.UPNP.ORG header field.
     *
     * \return the value of \c SEARCHPORT.UPNP.ORG header field.
     * If the value is not specified -1 is returned.
     */
    qint32 searchPort() const;
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
 * \remarks the class provides an assignment operator, which is not thread-safe.
 *
 * \sa HSsdp
 */
class H_UPNP_CORE_EXPORT HDiscoveryRequest
{
friend H_UPNP_CORE_EXPORT bool operator==(
    const HDiscoveryRequest&, const HDiscoveryRequest&);

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
     *
     * \remarks
     * - if userAgent identifies a UPnP v1.1 requester:
     *   - if mx is smaller than 1 it is set to 1 and
     *   - if mx is larger than 5 it it set to 5.
     * - if userAgent does not specify UPnP version (it always should however) or
     * the userAgent identifies a UPnP v1.0 requester:
     *   - if mx is smaller than 0 it is set to 0 and
     *   - if mx is larger than 120 it it set to 120.
     *
     * \sa isValid(), mx(), searchTarget(), userAgent()
     */
    HDiscoveryRequest(
        qint32 mx, const HDiscoveryType& resource,
        const HProductTokens& userAgent);

    /*!
     * Destroys the instance.
     */
    ~HDiscoveryRequest();

    /*!
     * Copy constructor.
     */
    HDiscoveryRequest(const HDiscoveryRequest&);

    /*!
     * Assigns the contents of the other to this.
     *
     * \return a reference to this.
     */
    HDiscoveryRequest& operator=(const HDiscoveryRequest&);

    /*!
     * Indicates whether or not the object contains valid announcement information.
     *
     * \param level indicates whether the check should be strictly according
     * to the UDA specification. If set to false some checks are omitted that
     * are known to be poorly implemented in some UPnP software.
     *
     * \return \e true in case the objects contains valid announcement
     * information in terms of the requested strictness.
     */
    bool isValid(HValidityCheckLevel level) const;

    /*!
     * Returns the Search Target of the request.
     *
     * \return the Search Target of the request.
     */
    HDiscoveryType searchTarget() const;

    /*!
     * Returns the maximum wait time in seconds.
     *
     * According to UDA,
     * <em>Device responses SHOULD be delayed a random duration between 0 and
     * this many seconds to balance load for the control point when it
     * processes responses</em>.
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
 * \remarks the class provides an assignment operator, which is not thread-safe.
 *
 * \sa HSsdp
 */
class H_UPNP_CORE_EXPORT HDiscoveryResponse
{
friend H_UPNP_CORE_EXPORT bool operator==(
    const HDiscoveryResponse&, const HDiscoveryResponse&);

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
     * If the location is invalid or empty the created object will be invalid.
     *
     * \param serverTokens specifies information about the host, the UPnP version
     * used and of the product. Note that if this parameter specifies
     * UPnP version 1.1 or later, \c bootId and \c configId have to be properly defined.
     * Otherwise the created object will be invalid.
     *
     * \note Although server tokens are mandatory according to the UDA,
     * this is not enforced by this class for interoperability reasons.
     *
     * \param usn specifies the Unique Service Name. The created object is valid
     * only if the provided USN is valid.
     *
     * \param bootId specifies the \c BOOTID.UPNP.ORG header value. Note that
     * this is mandatory in UDA v1.1, whereas it is not specified at all in
     * UDA v1.0. Because of this the class requires a valid value (>= 0) only in case
     * the \c serverTokens identify UPnP v1.1 or later.
     *
     * \param configId specifies the \c CONFIGID.UPNP.ORG header value. Note that
     * this is mandatory in UDA v1.1, whereas it is not specified at all in
     * UDA v1.0. Because of this, the class requires a valid value (>= 0) only in case
     * the \c serverTokens identify UPnP v1.1 or later.
     *
     * \param searchPort specifies the \c SEARCHPORT.UPNP.ORG header value. Note that
     * this is optional in UDA v1.1, whereas it is not specified at all in UDA v1.0.
     * If specified, this is the port at which the device must listen for unicast
     * \c M-SEARCH messages. Otherwise the port is the default \c 1900.
     * This parameter is optional.
     *
     * \remarks
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
        const HDiscoveryType& usn,
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
     * \param level indicates whether the check should be strictly according
     * to the UDA specification. If set to false some checks are omitted that
     * are known to be poorly implemented in some UPnP software.
     *
     * \return \e true in case the objects contains valid announcement
     * information in terms of the requested strictness.
     */
    bool isValid(HValidityCheckLevel level) const;

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
     * The Unique Service Name identifies a unique \e device or \e service instance.
     *
     * \return the Unique Service Name. The returned object is invalid if this
     * object is invalid.
     *
     * \sa isValid()
     */
    HDiscoveryType usn() const;

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
     * If the value is not specified -1 is returned.
     */
    qint32 bootId() const;

    /*!
     * Returns the value of \c CONFIGID.UPNP.ORG.
     *
     * \return the value of \c CONFIGID.UPNP.ORG.
     * If the value is not specified -1 is returned.
     */
    qint32 configId() const;

    /*!
     * Returns the value of \c SEARCHPORT.UPNP.ORG header field.
     *
     * \return the value of \c SEARCHPORT.UPNP.ORG header field.
     * If the value is not specified -1 is returned.
     */
    qint32 searchPort() const;
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
