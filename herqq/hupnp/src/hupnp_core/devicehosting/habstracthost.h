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

#ifndef HABSTRACTHOST_H_
#define HABSTRACTHOST_H_

#include "./../general/hdefs_p.h"
#include "./../general/hupnp_fwd.h"

#include <QObject>

namespace Herqq
{

namespace Upnp
{

class HAbstractHostPrivate;

/*!
 * Abstract base class for UPnP hosts.
 *
 * A UPnP host manages UPnP devices and provides clients access to them.
 * You can call rootDevices() to retrieve a list of shared pointers to root \c %HDevice
 * instances that the host is currently managing. If you know a <em>Unique Device Name</em>
 * you're interested, you can try to retrieve a matching device using rootDevice().
 *
 * You can also subscribe to events to be informed when a new root device
 * has been added to the host or an existing device has been removed from the host.
 *
 * For example,
 *
 * \code
 *
 * connect(
 *     pointerToAbstractHost,
 *     SIGNAL(rootDeviceAdded(Herqq::Upnp::HDeviceInfo)),
 *     this,
 *     SLOT(rootDeviceAdded(Herqq::Upnp::HDeviceInfo)));
 *
 * connect(
 *     pointerToAbstractHost,
 *     SIGNAL(rootDeviceRemoved(Herqq::Upnp::HDeviceInfo)),
 *     this,
 *     SLOT(rootDeviceRemoved(Herqq::Upnp::HDeviceInfo)));
 *
 * \endcode
 *
 * Note the signature used with SIGNAL and SLOT macros.
 *
 * \headerfile habstracthost.h HAbstractHost
 *
 * \ingroup devicehosting
 *
 * \warning see notes about host deletion in ~HAbstractHost().
 *
 * \remark
 * \li the methods introduced in this class are thread-safe, although the
 * \c QObject base class is largely not.
 * \li a host \b never transfers the ownership of the HDevice objects it hosts.
 *
  \sa HDeviceHost, HControlPoint
 */
class H_UPNP_CORE_EXPORT HAbstractHost :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HAbstractHost)
H_DECLARE_PRIVATE(HAbstractHost)

protected:

    HAbstractHostPrivate* h_ptr;
    HAbstractHost(HAbstractHostPrivate& dd, QObject* parent = 0);

public:

    /*!
     * Destroys the instance.
     *
     * Explicitly destroys every hosted HDevice even when the reference count of a
     * Herqq::Upnp::HRootDevicePtrT is non-zero.
     *
     * \warning when the host is being destroyed, the host destroys all of
     * its children, which includes every hosted HDevice. If you have stored a
     * Herqq::Upnp::HRootDevicePtrT, you should clear the shared pointer
     * before destroying the host to which the device belongs. Otherwise there is
     * a chance of access violation when the Herqq::Upnp::HRootDevicePtrT is
     * going out of scope.
     *
     * \sa devicemodel
     */
    virtual ~HAbstractHost() = 0;

    /*!
     * Indicates whether or not the host is successfully started.
     *
     * \return true in case the host is successfully started.
     */
    bool isStarted() const;

    /*!
     * Returns a list of UPnP root devices the host is currently managing.
     *
     * The returned list contains pointers to HDevice objects managed by
     * QSharedPointers. This ensures that a root HDevice object and objects
     * that are reachable through it are accessible as long as at least one
     * pointer to the root device object is valid. Because of this, you should discard
     * a Herqq::Upnp::HRootDevicePtrT once you no longer need it.
     * This enables the object be deleted once the reference count drops to zero.
     *
     * \warning do not store the raw pointer held by a \c QSharedPointer. If you need
     * to use a raw pointer, be sure that the containing \c QSharedPointer is valid
     * as long as you need to use the raw pointer. Otherwise, the object may be
     * deleted while you are using the raw pointer.
     *
     * \return a list of UPnP root devices the host is currently managing.
     */
    HRootDevicePtrListT rootDevices() const;

    /*!
     * Returns a root device with the specified Unique Device Name.
     *
     * \param udn specifies the Unique Device Name of the desired root device.
     *
     * \return the root device with the specified Unique Device Name or a
     * null pointer in case no currently managed root device has the
     * specified UDN.
     *
     * \warning do not store the raw pointer held by a \c QSharedPointer. If you need
     * to use a raw pointer, be sure that the containing \c QSharedPointer is valid
     * as long as you need to use the raw pointer. Otherwise, the object may be
     * deleted while you are using the raw pointer.
     */
    HRootDevicePtrT rootDevice(const HUdn& udn) const;

/*******************************************************************************
 * SIGNALS
 *******************************************************************************/
Q_SIGNALS:

    /*!
     * This signal is emitted when a new device has been added to the control of the
     * host.
     *
     * You can retrieve the corresponding device from the host by issuing
     *
     * \code
     *
     * HRootDevicePtrT newlyFoundDevice = pointerToAbstractHost->rootDevice(newDeviceInfo.udn());
     *
     * \endcode
     *
     * \param newDeviceInfo details information about the new device.
     */
    void rootDeviceAdded(const Herqq::Upnp::HDeviceInfo& newDeviceInfo);

    /*!
     * This signal is emitted when a device has been removed from the control of
     * the host.
     *
     * When a device has been removed, it is also \e disposed. In short, you
     * should quit using a disposed device as soon as possible and discard
     * any pointers to it. For more information about device disposal, see
     * \ref devicedisposal.
     *
     * \param deviceInfo details information about the device that was lost.
     */
    void rootDeviceRemoved(const Herqq::Upnp::HDeviceInfo& deviceInfo);
};

}
}

#endif /* HABSTRACTHOST_H_ */
