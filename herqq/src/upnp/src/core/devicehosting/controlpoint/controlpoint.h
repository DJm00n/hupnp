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

#ifndef UPNP_CONTROL_POINT_H_
#define UPNP_CONTROL_POINT_H_

#include "../../defs_p.h"
#include "../../upnp_fwd.h"
#include "../abstracthost.h"

namespace Herqq
{

namespace Upnp
{

class HControlPointPrivate;
class HControlPointConfiguration;

/*!
 * A class for discovering and interacting with UPnP devices in the network.
 *
 * \headerfile controlpoint.h HControlPoint
 *
 * \ingroup devicehosting
 *
 * According to the UPnP Device Architecture specification, a control point is an
 * entity, which <em> "retrieves device and service descriptions, sends actions to services,
 * polls for service state variables, and receives events from services" </em>. In other words,
 * a UPnP control point discovers UPnP devices, queries their state,
 * listens their asynchronous events and invokes them to perform actions. A control point
 * is the \em client in the UPnP architecture, whereas a UPnP device is the \em server.
 *
 * \c %HControlPoint does all of the above, but mostly hiding it from the user.
 * To discover UPnP devices, all you need to do is create an instance of
 * \c %HControlPoint, initialize it and check if devices are already found.
 * You can also listen for HAbstractHost::rootDeviceAdded() events.
 * For any of this to work, however, you need to run the \c %HControlPoint in a Qt
 * event loop.
 *
 * Consider an example:
 *
 * \code
 *
 * // myclass.h

 * #include <HControlPoint>
 * #include <QObject>
 *
 * class MyClass :
 *     public QObject
 * {
 * Q_OBJECT
 *
 * private:
 *
 *     Herqq::Upnp::HControlPoint* m_controlPoint;
 *
 * private slots:
 *
 *     void rootDeviceAdded(const Herqq::Upnp::HDeviceInfo&);
 *     void rootDeviceRemoved(const Herqq::Upnp::HDeviceInfo&);
 *
 * public:
 *
 *     MyClass(QObject* parent = 0);
 * };
 *
 * // myclass.cpp
 *
 * #include "myclass.h"
 * #include <HDeviceInfo>
 *
 * MyClass::MyClass(QObject* parent) :
 *     QObject(parent), m_controlPoint(new Herqq::Upnp::HControlPoint(this))
 * {
 *     connect(
 *         m_controlPoint,
 *         SIGNAL(rootDeviceAdded(Herqq::Upnp::HDeviceInfo)),
 *         this,
 *         SLOT(rootDeviceAdded(Herqq::Upnp::HDeviceInfo)));
 *
 *     connect(
 *         m_controlPoint,
 *         SIGNAL(rootDeviceRemoved(Herqq::Upnp::HDeviceInfo)),
 *         this,
 *         SLOT(rootDeviceRemoved(Herqq::Upnp::HDeviceInfo)));
 *
 *     if (m_controlPoint->init(deviceConf) != Herqq::Upnp::HControlPoint::Success)
 *     {
 *         // the initialization failed, perhaps you should do something?
 *         return;
 *     }
 *
 *     // the control point is running and any standard-compliant UPnP device
 *     // on the same network should now be discoverable until this class is
 *     // destroyed.
 *
 *     // remember also that the current thread has to have an event loop
 * }
 *
 * void MyClass::rootDeviceAdded(const Herqq::Upnp::HDeviceInfo& newDevice)
 * {
 *     // device found, should something be done with it?
 *     // Herqq::Upnp::HDeviceInfo class contains all the information specified in
 *     // the device description, maybe some of that should be put shown somewhere? ..
 *
 *     // or, perhaps the actual device is needed, in which case:
 *
 *     Herqq::Upnp::HRootDevicePtrT newRootDevice = rootDevice(newDevice.udn());
 *
 *     // .. do something with the device
 * }
 *
 * void MyClass::rootDeviceRemoved(const Herqq::Upnp::HDeviceInfo& removedDevice)
 * {
 *     // device announced that it is going away and the control point in turn
 *     // removes the device from its internal structures and sends a notification
 *     // of this.
 * }
 *
 * \endcode
 *
 * Once you have obtained a Herqq::Upnp::HRootDevicePtrT, you can
 * enumerate its services, invoke its actions, listen for events of changed
 * state and so on. Basically, a root HDevice object at the control point side
 * is an entry point to a very accurate object model of the real root UPnP device
 * that has been discovered. For more information about the \c HDevice and the object model,
 * see the page detailing the HUPnP \ref devicemodel.
 *
 * If you want to stop an initialized control point instance from listening
 * the network and to clear its state, you can call quit().
 *
 * \remark
 * \li This class is not thread safe in regard to initialization and shutdown.
 * \li Instances of this class have to be destroyed in the thread in which they
 * are located.
 * \li You can use \c QObject::moveToThread() on the \c %HControlPoint, which causes
 * the control point and every object managed by it to be moved to the chosen thread.
 * However, you cannot move individual objects managed by \c %HControlPoint.
 *
 * \warning see notes about object deletion in ~HControlPoint().
 *
 * \sa HRootDevicePtrListT, HRootDevicePtrT, devicemodel
 */
class H_UPNP_CORE_EXPORT HControlPoint :
    public HAbstractHost
{
Q_OBJECT
H_DISABLE_COPY(HControlPoint)
H_DECLARE_PRIVATE(HControlPoint)

public:

    /*!
     * Specifies return values that some of the methods of the class may return.
     */
    enum ReturnCode
    {
        /*!
         * Return value signifying general failure. This return code is used when
         * an operation could not be successfully completed, but the exact
         * cause for the error could not be determined.
         */
        UndefinedFailure = -1,

        /*!
         *  Return value signifying success.
         */
        Success = 0,

        /*!
         * Return value signifying that the control point is already successfully
         * initialized.
         */
        AlreadyInitialized = 1,
    };

public:

    /*!
     * Creates a new instance.
     *
     * \param parent specifies the parent \c QObject, if any.
     */
    explicit HControlPoint(QObject* parent = 0);

    /*!
     * Destroys the control point and every hosted device.
     *
     * \warning When the control point is being destroyed, the control point
     * destroys all of its child objects. If you have stored a
     * Herqq::Upnp::HRootDevicePtrT, you should clear the shared pointer
     * before destroying the control point to which the root device belongs.
     * Otherwise there is a chance of access violation when the
     * Herqq::Upnp::HRootDevicePtrT is going out of scope.
     *
     * \remark An \c %HControlPoint instance has to be destroyed in the thread
     * in which it is located.
     *
     * For more information,
     *
     * \sa HAbstractHost::~HAbstractHost()
     */
    virtual ~HControlPoint();

public Q_SLOTS:

    /*!
     * Initializes the control point and searches for devices currently available.
     *
     * This has to be called for the control point to start
     * monitoring the network for UPnP devices. To stop an initialized control point
     * instance from listening network events, you can call quit() or delete
     * the object.
     *
     * \param initParams specifies parameters that can be used to modify the
     * default behavior of the control point instance. This parameter is optional
     * and does not have to be provided.
     *
     * \param errorString is a pointer to a \c QString, which will contain an error
     * description in case the initialization failed for some reason.
     * This parameter is optional and does not have to be provided.
     *
     * \retval Success when the control point was successfully started.
     *
     * \retval AlreadyInitialized when the control point has already been successfully started.
     * In this case nothing is changed and the control point continues to operate
     * normally.
     *
     * \retval UndefinedFailure in case some other initialization error occurred.
     *
     * \sa quit()
     */
    ReturnCode init(
        const HControlPointConfiguration* initParams = 0, QString* errorString = 0);

    /*!
     * Shuts down the control point.
     *
     * The control point stops listening for network events,
     * disposes all the devices it hosted and cancels all event subscriptions.
     * In essence, the control point purges it state. You can re-initialize the
     * control point by calling init() again.
     *
     * \attention Before you call this method, you should discard every pointer to
     * object retrieved from this instance. Those objects are not deleted until the
     * control point instance is deleted, but as it was mentioned, every device
     * object is disposed. For more information about <em>device disposal</em>,
     * see \ref devicedisposal.
     *
     * \sa init()
     */
    void quit();
};

}
}

#endif /* UPNP_CONTROL_POINT_H_ */
