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

#ifndef MULTICAST_SOCKET_H_
#define MULTICAST_SOCKET_H_

#include "../../../../core/include/HGlobal"

#include <QUdpSocket>

namespace Herqq
{

namespace Upnp
{

class HMulticastSocketPrivate;

/*!
 * Class for multicast communication.
 *
 * \remark this class has thread-affinity, which mandates that the instances of this
 * class has to be used in the thread in which they are located at the time.
 */
class H_UPNP_CORE_EXPORT HMulticastSocket :
    public QUdpSocket
{
H_DISABLE_COPY(HMulticastSocket)
H_DECLARE_PRIVATE(HMulticastSocket)

protected:

    HMulticastSocketPrivate* h_ptr;
    HMulticastSocket(HMulticastSocketPrivate& dd, QObject* parent=0);

public:

    /*!
     * Constructs a new instance.
     *
     * \param parent specifies the parent \c QObject.
     */
    explicit HMulticastSocket(QObject* parent = 0);

    /*!
     * Destroys the instance.
     */
    virtual ~HMulticastSocket();

    /*!
     * Attempts to joins into the specified multicast group address.
     *
     * \param address specifies the multicast address to be joined.
     *
     * \retval true in case the operation succeeded.
     *
     * \retval false in case the operation failed. For instance,
     * this happens when the socket is not bound to a port.
     */
    bool joinMulticastGroup(const QHostAddress& address);

    /*!
     * Attempts to leave from the specified multicast group address.
     *
     * \param address specifies the multicast address from where to leave.
     *
     * \retval true in case the operation succeeded.
     *
     * \retval false in case the operation failed. For example, this happens
     * when the socket has not joined to the specified multicast address.
     */
    bool leaveMulticastGroup(const QHostAddress& address);

    /*!
     * Attempts to set the Time To Live attribute for each message.
     *
     * \param arg specifies the value for Time To Live.
     *
     * \return \e true in case the operation succeeded.
     */
    bool setMulticastTtl(quint8 arg);

    /*!
     * Attempts to bind the socket into the specified port using BindMode flags
     * and a QHostAddress value that are suitable for a multicast socket.
     *
     * \param port specifies the port to which to bind.
     *
     * \return \e true in case the operation succeeded.
     */
    bool bind(quint16 port = 0);

    /*!
     * Attempts to bind the socket into the specified port and address
     * using BindMode flags that are suitable for a multicast socket.
     *
     * \param addressToBind specifies the address to which to bind.
     *
     * \param port specifies the port to which to bind.
     *
     * \return \e true in case the operation succeeded.
     */
    bool bind(const QHostAddress& addressToBind, quint16 port = 0);
};

}
}

#endif /* MULTICAST_SOCKET_H_ */
