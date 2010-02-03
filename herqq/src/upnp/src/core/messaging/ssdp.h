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

#ifndef SSDP_H_
#define SSDP_H_

#include "../upnp_fwd.h"
#include "../../../../core/include/HGlobal"

#include <QObject>

class QUrl;
class QString;
class QHostAddress;

namespace Herqq
{

namespace Upnp
{

class HEndpoint;
class HSsdpPrivate;

/*!
 * This class is used for sending and receiving SSDP messages defined by the
 * UPnP Device Architecture specification.
 *
 * Simple Service Discovery Protocol (SSDP) is an expired IETF Internet draft
 * on which the UPnP discovery mechanism is built. This class implements only the
 * SSDP functionality mandated by the UPnP Device Architecture specification.
 * This class does not implement the SSDP draft in full.
 *
 * To use this class, you only need to instantiate it and connect to the
 * exposed signals to receive events when SSDP messages are received. You can also
 * derive a sub-class and override the various virtual member functions to handle
 * the received messages.
 *
 * \headerfile ssdp.h HSsdp
 *
 * \ingroup ssdp
 *
 * \remark
 * \li this class requires an event loop for listening incoming messages
 * \li this class has thread-affinity, which mandates that the instances of this
 * class has to be used in the thread in which they are located at the time.
 */
class H_UPNP_CORE_EXPORT HSsdp :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HSsdp)
H_DECLARE_PRIVATE(HSsdp)


private Q_SLOTS:

    void multicastMessageReceived();
    void unicastMessageReceived  ();

protected:

    HSsdpPrivate* h_ptr;

protected:

    /*!
     * This method is called immediately after receiving a discovery request.
     *
     * Override this method if you want to handle the message. You can also connect
     * to the discoveryRequestReceived() signal.
     *
     * \param msg specifies the incoming message.
     *
     * \retval true in case the message was handled successfully and the
     * discoveryRequestReceived() signal should not be sent.
     *
     * \retval false in case the message was not handled and the
     * discoveryRequestReceived() signal should be sent.
     *
     * \sa discoveryRequestReceived()
     */
    virtual bool incomingDiscoveryRequest (
        const HDiscoveryRequest& msg, const HEndpoint& source,
        const HEndpoint& destination);

    /*!
     * This method is called immediately after receiving a discovery response.
     * Override this method if you want to handle message. You can also connect
     * to the discoveryResponseReceived() signal.
     *
     * \param msg specifies the incoming message.
     *
     * \retval true in case the message was handled successfully and the
     * discoveryResponseReceived() signal should not be sent.
     *
     * \retval false in case the message was not handled and the
     * discoveryResponseReceived() signal should be sent.
     *
     * \sa discoveryResponseReceived()
     */
    virtual bool incomingDiscoveryResponse(
        const HDiscoveryResponse& msg, const HEndpoint& source);

    /*!
     * This method is called immediately after receiving a device available announcement.
     * Override this method if you want to handle message. You can also connect
     * to the discoveryRequestReceived() signal.
     *
     * \param msg specifies the incoming message.
     *
     * \retval true in case the message was handled successfully and the
     * resourceAvailableReceived() signal should not be sent.
     *
     * \retval false in case the message was not handled and the
     * resourceAvailableReceived() signal should be sent.
     *
     * \sa resourceAvailableReceived()
     */
    virtual bool incomingDeviceAvailableAnnouncement(
        const HResourceAvailable& msg);

    /*!
     * This method is called immediately after receiving a device unavailable announcement.
     * Override this method if you want to handle message. You can also connect
     * to the resourceUnavailableReceived() signal.
     *
     * \param msg specifies the incoming message.
     *
     * \retval true in case the message was handled successfully and the
     * resourceUnavailableReceived() signal should not be sent.
     *
     * \retval false in case the message was not handled and the
     * resourceUnavailableReceived() signal should be sent.
     *
     * \sa resourceUnavailableReceived()
     */
    virtual bool incomingDeviceUnavailableAnnouncement(
        const HResourceUnavailable& msg);

    /*!
     * This method is called immediately after receiving a device update announcement.
     * Override this method if you want to handle message. You can also connect
     * to the deviceUpdateRecieved() signal.
     *
     * \param msg specifies the incoming message.
     *
     * \retval true in case the message was handled successfully and the
     * deviceUpdateRecieved() signal should not be sent.
     *
     * \retval false in case the message was not handled and the
     * deviceUpdateRecieved() signal should be sent.
     *
     * \sa deviceUpdateRecieved()
     */
    virtual bool incomingDeviceUpdateAnnouncement(
        const HResourceUpdate& msg);

public:

    /*!
     * Creates a new instance.
     *
     * The class searches for a network interface that is up and which
     * is not the loopback. If no such interface is found, the loopback address
     * is used.
     *
     * \param parent specifies the parent object.
     *
     * \throw Herqq::HSocketException if the construction failed.
     */
    HSsdp(QObject* parent=0);

    /*!
     * Creates a new instance.
     *
     * \param addressToBind specifies the address the object should use for
     * unicast communication.
     *
     * \param parent specifies the parent object.
     *
     * \throw Herqq::HSocketException if the construction failed.
     */
    HSsdp(const QHostAddress& addressToBind, QObject* parent=0);

    /*!
     * Destroys the instance.
     */
    virtual ~HSsdp();

    /*!
     * Returns the end point that is used for unicast communication.
     *
     * \return the end point that is used for unicast communication.
     */
    HEndpoint unicastEndpoint() const;

    /*!
     * Sends the specified device availability announcement.
     *
     * \param msg specifies the announcement to send.
     * \param count specifies how many times the announcement is send.
     * The default is 1.
     *
     * \retval 0 if send was successful.
     */
    qint32 announcePresence(const HResourceAvailable& msg, qint32 count = 1);

    /*!
     * Sends the specified device availability announcement.
     *
     * \param msg specifies the announcement to send.
     * \param count specifies how many times the announcement is send.
     * The default is 1.
     *
     * \retval 0 if send was successful.
     */
    qint32 announcePresence(const HResourceUnavailable& msg, qint32 count = 1);

    /*!
     * Sends the specified device update announcement.
     *
     * \param msg specifies the message to send.
     * \param count specifies how many times the announcement is send.
     * The default is 1.
     *
     * \retval 0 if send was successful.
     */
    qint32 announceUpdate(const HResourceUpdate& msg, qint32 count = 1);

    /*!
     * Sends the specified discovery request.
     *
     * \param msg specifies the announcement to send.
     * \param count specifies how many times the announcement is send.
     * The default is 1.
     *
     * \retval 0 if send was successful.
     */
    qint32 sendDiscoveryRequest(const HDiscoveryRequest& msg, qint32 count = 1);

    /*!
     * Sends the specified discovery response.
     *
     * \param receiver specifies the target of the response.
     *
     * \param msg specifies the announcement to send.
     *
     * \param count specifies how many times the announcement is send.
     * The default is 1.
     *
     * \retval 0 if send was successful.
     */
    qint32 sendDiscoveryResponse(
        const HEndpoint& receiver, const HDiscoveryResponse& msg,
        qint32 count = 1);

////
////////////////////////////////////////////////////////////////////////////////
Q_SIGNALS:

    /*!
     * This signal is emitted when a <em>discovery request</em> is received.
     *
     * \param msg specifies the received <em>discovery request</em> message.
     * \param source specifies the location where the message came.
     * \param destination specifies the target location of the message.
     */
    void discoveryRequestReceived(
        const Herqq::Upnp::HDiscoveryRequest& msg,
        const Herqq::Upnp::HEndpoint& source,
        const Herqq::Upnp::HEndpoint& destination);

    /*!
     * This signal is emitted when a <em>discovery response</em> is received.
     *
     * \param msg specifies the received <em>discovery response</em> message.
     * \param source specifies the location where the message came.
     */
    void discoveryResponseReceived(
        const Herqq::Upnp::HDiscoveryResponse& msg,
        const Herqq::Upnp::HEndpoint& source);

    /*!
     * This signal is emitted when a <em>device announcement</em> is received.
     *
     * \param msg specifies the <em>device announcement</em> message.
     */
    void resourceAvailableReceived(const Herqq::Upnp::HResourceAvailable& msg);

    /*!
     * This signal is emitted when a <em>device update</em> is received.
     *
     * \param msg specifies the <em>device update</em> message.
     */
    void deviceUpdateReceived(const Herqq::Upnp::HResourceUpdate& msg);

    /*!
     * This signal is emitted when a <em>device announcement</em> is received.
     *
     * \param msg specifies the <em>device announcement</em> message.
     */
    void resourceUnavailableReceived(const Herqq::Upnp::HResourceUnavailable& msg);
};

}
}

#endif /* SSDP_H_ */