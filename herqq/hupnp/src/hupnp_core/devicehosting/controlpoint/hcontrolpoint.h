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

#ifndef HCONTROL_POINT_H_
#define HCONTROL_POINT_H_

#include "./../../general/hdefs_p.h"
#include "./../../general/hupnp_fwd.h"

#include <QObject>

namespace Herqq
{

namespace Upnp
{

class HControlPointPrivate;
class HControlPointConfiguration;

/*!
 * A class for discovering and interacting with UPnP devices in the network.
 *
 * \headerfile hcontrolpoint.h HControlPoint
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
 * You can also listen for a number of events, such as the
 * HControlPoint::rootDeviceOnline(), which is emitted whenever a UPnP
 * device has become available on the network.
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
 *     void rootDeviceOnline(Herqq::Upnp::HDevice*);
 *     void rootDeviceOffline(Herqq::Upnp::HDevice*);
 *
 * public:
 *
 *     MyClass(QObject* parent = 0);
 * };
 *
 * // myclass.cpp
 *
 * #include "myclass.h"
 * #include <HDevice>
 *
 * MyClass::MyClass(QObject* parent) :
 *     QObject(parent), m_controlPoint(new Herqq::Upnp::HControlPoint(0, this))
 * {
 *     connect(
 *         m_controlPoint,
 *         SIGNAL(rootDeviceOnline(Herqq::Upnp::HDevice*)),
 *         this,
 *         SLOT(rootDeviceOnline(Herqq::Upnp::HDevice*)));
 *
 *     connect(
 *         m_controlPoint,
 *         SIGNAL(rootDeviceOffline(Herqq::Upnp::HDevice*)),
 *         this,
 *         SLOT(rootDeviceOffline(Herqq::Upnp::HDevice*)));
 *
 *     if (m_controlPoint->init() != Herqq::Upnp::HControlPoint::Success)
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
 * void MyClass::rootDeviceOnline(Herqq::Upnp::HDevice* newDevice)
 * {
 *     // device discovered, should something be done with it? Perhaps we want
 *     // to learn something from it:
 *     Herqq::Upnp::HDeviceInfo info = newDevice->deviceInfo();
 *     // do something with the info object
 * }
 *
 * void MyClass::rootDeviceOffline(Herqq::Upnp::HDevice* device)
 * {
 *     // device announced that it is going away and the control point sends
 *     // a notification of this. However, the device isn't removed from the
 *     // control point until explicitly requested:
 *
 *     m_controlPoint->removeDevice(device);
 * }
 *
 * \endcode
 *
 * Once you have obtained a pointer to a Herqq::Upnp::HDevice, you can
 * enumerate its services, invoke its actions, listen for events of changed
 * state and so on. Basically, a root HDevice object at the control point side
 * is an entry point to a very accurate object model of the real root UPnP device
 * that has been discovered. For more information about the HDevice and the object model,
 * see the page detailing the HUPnP \ref devicemodel.
 *
 * If you want to stop an initialized control point instance from listening
 * the network and to clear its state, you can call quit().
 *
 * \remarks
 * \li This class has thread affinity. You have to use it and destroy it in the
 * thread in which it is located.
 * \li You can use \c QObject::moveToThread() on the \c %HControlPoint, which causes
 * the control point and every object managed by it to be moved to the chosen thread.
 * However, you cannot move individual objects managed by \c %HControlPoint.
 * \li a control point never transfers the ownership of the HDevice objects it manages.
 * \li <b>%HControlPoint always destroys every %HDevice it manages when it is being destroyed</b>.
 *
 * \warning see notes about object deletion in ~HControlPoint().
 *
 * \sa HDevice, HDeviceList, devicemodel
 */
class H_UPNP_CORE_EXPORT HControlPoint :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HControlPoint)
H_DECLARE_PRIVATE(HControlPoint)

protected:

    /*!
     * Specifies what to do when a device has been discovered.
     *
     * \note a discovered device may be a new device or a device that is already
     * in the control of the control point. This is the case when a device
     * goes offline, is not removed from the control point and later
     * comes back online.
     *
     * \sa acceptRootDevice()
     */
    enum DeviceDiscoveryAction
    {
        /*!
         * In case the discovered device is a new device
         * this value instructs the control point to ignore and delete it.
         *
         * In case the discovered device is already in the control of the control
         * point this value instructs the control point to remove and delete it.
         */
        Ignore = 0,

        /*!
         * In case the discovered device is a new device the new device is
         * added into the %HControlPoint. Otherwise nothing is done.
         * This value instructs the control point not to subscribe to events.
         */
        AddOnly = 1,

        /*!
         * In case the discovered device is a new device the new device is
         * added into the %HControlPoint. Furthermore, this value instructs the
         * control point to check the possibly provided configuration to
         * decide whether to subscribe to events.
         */
        AddAndFollowConfiguration = 2,

        /*!
         * In case the discovered device is a new device the new device is
         * added into the %HControlPoint. In addition, this value instructs
         * the control point to subscribe to to all events of all services contained
         * by the device.
         */
        AddAndSubscribeToAll = 3
    };

public:

    /*!
     * This enum specifies the return values the methods of
     * \c %HControlPoint can return.
     */
    enum ReturnCode
    {
        /*!
         * Return value signifying general failure.
         *
         * This return code is used when
         * an operation could not be successfully completed, but the exact
         * cause for the error could not be determined.
         */
        UndefinedFailure = -1,

        /*!
         * Return value signifying success.
         *
         * This return code is used when an operation was successfully completed.
         */
        Success = 0,

        /*!
         * Return value indicating error in networking.
         *
         * This return code is used, for instance, when the
         * HTTP server or SSDP module could not be initialized.
         */
        CommunicationsError = 1,

        /*!
         * Return value signifying that the control point is already successfully
         * initialized.
         *
         * This return code is used when init() is called after the control point
         * has already been successfully initialized.
         */
        AlreadyInitialized = 2,

        /*!
         * Return value used to indicate that a method was called with an invalid
         * argument and the call was aborted.
         */
        InvalidArgument = 3
    };

    /*!
     *
     */
    /*enum ErrorType
    {
    };*/

private:

    /*!
     * Performs the initialization of a derived class.
     *
     * The \c %HControlPoint uses two-phase initialization, in which the user
     * first constructs an instance and then calls init() in order to ready
     * the object for use. This method is called by the \c %HControlPoint
     * during its private initialization after all the private data structures
     * are constructed but before any network activity. At this point, no HTTP
     * or SSDP requests are served.
     *
     * You can override this method to perform any further initialization of a
     * derived class.
     *
     * \return HControlPoint::Success if and only if the initialization succeeded.
     * If any other value is returned, the initialization of the device host is
     * aborted with the error code returned by the derived class.
     *
     * \remark the default implementation does nothing.
     *
     * \sa init()
     */
    virtual ReturnCode doInit();

    /*!
     * Performs the de-initialization of a derived class.
     *
     * Since it is possible to shutdown a device host without actually destroying
     * the instance by calling quit(), derived classes have the possibility to
     * perform their own de-initialization procedure by overriding this method.
     * This method is called \b before the \c %HControlPoint cleans its
     * private data structures but after it has stopped listening requests
     * from the network.
     *
     * \remark the default implementation does nothing.
     *
     * \sa quit()
     */
    virtual void doQuit();

    /*!
     * This method will be called whenever a device tree has been built successfully.
     *
     * Override this method in case you want to control which devices will be
     * added to the control of this control point.
     *
     * \return \e true when the discovered device should be added to the control
     * of the instance. Return false and the device will be ignored. By default
     * every discovered and successfully built device will be added.
     */
    virtual DeviceDiscoveryAction acceptRootDevice(HDevice* device);

    /*!
     * This method will be called whenever a new a device has been detected on
     * the network.
     *
     * Override this method in case you want to control which devices get built.
     *
     * \param usn specifies the advertised resource
     * \param source specifies the source of the advertisement.
     *
     * \return \e true when the device should be built in full. If the device
     * can be built the rootDeviceDiscovered() method will be called. By default
     * every new device is accepted and built accordingly.
     */
    virtual bool acceptResourceAd(
        const HResourceIdentifier& usn, const HEndpoint& source);

protected:

    HControlPointPrivate* h_ptr;

    //
    // \internal
    //
    HControlPoint(
        HControlPointPrivate& dd,
        const HControlPointConfiguration* initParams = 0, QObject* parent = 0);

    /*!
     * Returns the configuration used to initialize the control point.
     *
     * \return the configuration used to initialize the control point or null
     * in case no configuration was provided.
     *
     * \remark the returned object is not a copy and the ownership of the
     * object is not transferred.
     */
    const HControlPointConfiguration* configuration() const;

public:

    /*!
     * Creates a new instance.
     *
     * \param initParams specifies parameters that can be used to modify the
     * default behavior of the control point instance. This parameter is optional
     * and does not have to be provided.
     *
     * \param parent specifies the parent \c QObject, if any.
     */
    explicit HControlPoint(
        const HControlPointConfiguration* initParams = 0, QObject* parent = 0);

    /*!
     * Destroys the control point and every hosted device.
     *
     * \warning When the control point is being destroyed, the control point
     * destroys all of its child objects. You should discard any pointers retrieved
     * from the control point to prevent accidental dereference.
     *
     * \remark An \c %HControlPoint instance has to be destroyed in the thread
     * in which it is located.
     */
    virtual ~HControlPoint();

    /*!
     * Initializes the control point and searches for devices currently available.
     *
     * This has to be called for the control point to start
     * monitoring the network for UPnP devices. To stop an initialized control point
     * instance from listening network events, you can call quit() or delete
     * the object.
     *
     * \param errorString is a pointer to a \c QString, which will contain an error
     * description in case the initialization failed for some reason.
     * This parameter is optional and does not have to be provided.
     *
     * \retval HControlPoint::Success when the control point was successfully started.
     *
     * \retval HControlPoint::AlreadyInitialized when the control point has already been successfully started.
     * In this case nothing is changed and the control point continues to operate
     * normally.
     *
     * \retval HControlPoint::CommunicationsError in case the instance couldn't
     * initialize the required networking resources.
     *
     * \retval HControlPoint::UndefinedFailure in case some other initialization
     * error occurred.
     *
     * \sa quit()
     */
    ReturnCode init(QString* errorString = 0);

    /*!
     * Indicates whether or not the host is successfully started.
     *
     * \return true in case the host is successfully started.
     */
    bool isStarted() const;

    /*!
     * Returns a list of UPnP root devices the host is currently managing.
     *
     * The returned list contains pointers to root HDevice objects that are currently
     * managed by this instance.
     *
     * \return a list of pointers to HDevice objects the control point
     * is currently managing.
     *
     * \warning the returned devices will be deleted at the latest when the
     * control point is being destroyed. In addition, you can call
     * removeDevice() to remove and delete a device. However, do not delete
     * the device objects directly. The ownership of an HDevice is \b never transferred.
     */
    HDeviceList rootDevices() const;

    /*!
     * Returns a root device with the specified Unique Device Name.
     *
     * \param udn specifies the Unique Device Name of the desired root device.
     *
     * \return the root device with the specified Unique Device Name or a
     * null pointer in case no currently managed root device has the
     * specified UDN.
     *
     * \warning the returned device will be deleted at the latest when the
     * control point is being destroyed. In addition, you can call
     * removeDevice() to remove and delete a device. However, do not delete
     * the device object directly. The ownership of an HDevice is \b never transferred.
     */
    HDevice* rootDevice(const HUdn& udn) const;

    /*!
     * Removes the root device from the control point and then deletes it.
     *
     * \param rootDevice specifies the root device to be removed. Nothing is done if
     * the device is not in the control of this control point or it is not a root
     * device.
     *
     * \retval HControlPoint::Success in case the device was successfully removed
     * and deleted.
     * \retval HControlPoint::InvalidArgument in case the specified argument
     * was null or the specified device is not in control of this control point
     * or the specified device is not a root device. In this case the
     * specified device object is not deleted.
     */
    ReturnCode removeDevice(HDevice* rootDevice);

    /*!
     * Subscribes to events of every service contained by the device.
     *
     * Any services which events are already subscribed are skipped.
     *
     * \param device specifies the device that contains the services, which
     * events are subscribed.
     *
     * \param recursive specifies whether to subscribe to all services of all
     * embedded devices as well.
     *
     * \retval HControlPoint::Success in case the subscriptions were successfully
     * dispatched. Note, \b any subscription may still fail. You can connect to
     * subscriptionSucceeded() and subscriptionFailed() signals to be notified
     * upon subscription success and failure.
     *
     * \retval HControlPoint::InvalidArgument in case the specified argument
     * is null or it does not belong to a device held by the control point.
     *
     * \remarks
     * - the method returns immediately. Every successful subscription results
     * in subscriptionSucceeded() signal sent. Every failed subscription results in
     * subscriptionFailed() signal sent.
     * - every subscription is maintained until:
     *     - an error occurs
     *     - it is explicitly canceled
     *
     * This means that every subscription is automatically renewed before expiration.
     */
    ReturnCode subscribeEvents(HDevice* device, bool recursive = true);

    /*!
     * Subscribes to the events of the service.
     *
     * \param service specifies the service
     *
     * \retval HControlPoint::Success in case the subscription request was successfully
     * dispatched. Note, the subscription \b may still fail. You can connect to
     * subscriptionSucceeded() and subscriptionFailed() signals to be notified
     * upon subscription success and failure.
     *
     * \retval HControlPoint::InvalidArgument in case the specified argument:
     * - is null,
     * - it does not belong to a device held by the control point or
     * - there already exists a subscription for the specified service.
     *
     * \remarks
     * - the method returns immediately. A successful subscription results
     * in subscriptionSucceeded() signal sent. A failed subscription results in
     * subscriptionFailed() signal sent.
     * - a subscription is maintained until:
     *     - an error occurs
     *     - it is explicitly canceled
     *
     * This means that a subscription is automatically renewed before expiration.
     */
    ReturnCode subscribeEvents(HService* service);

    /*!
     * Cancels the event subscriptions of every service contained by the device.
     *
     * Any services which events are not subscribed are skipped.
     *
     * \param device specifies the device that contains the services, which
     * subscriptions are canceled.
     *
     * \param recursive specifies whether to cancel all subscriptions of all
     * embedded devices as well.
     *
     * \retval HControlPoint::Success in case the subscription cancellation requests
     * were successfully dispatched. Note, this does not mean that the cancellations
     * were or will be successful. That is, upon success the state of the control point
     * instance is modified appropriately, but it is never guaranteed that the
     * target UPnP device receives or processes the cancellation requests.
     *
     * \retval HControlPoint::InvalidArgument in case the specified argument
     * is null or it does not belong to a device held by the control point.
     *
     * \remarks this method returns immediately.
     */
    ReturnCode cancelEvents(HDevice* device, bool recursive = true);

    /*!
     * Cancels the event subscription of the service.
     *
     * \param service specifies the service
     *
     * \retval HControlPoint::Success in case the subscription cancellation request
     * was successfully dispatched. Note, this does not mean that the cancellation
     * was or will be successful. That is, upon success the state of the control point
     * instance is modified appropriately, but it is never guaranteed that the
     * target UPnP device receives or processes the cancellation request.
     *
     * \retval HControlPoint::InvalidArgument in case the specified argument
     * is null, it does not belong to a device held by the control point or
     * no subscription is made to the specified service.
     *
     * \remarks this method returns immediately.
     */
    ReturnCode cancelEvents(HService* service);

public Q_SLOTS:

    /*!
     * Shuts down the control point.
     *
     * The control point stops listening for network events,
     * deletes all the devices it is hosting and cancels all event subscriptions.
     * In essence, the control point purges it state. You can re-initialize the
     * control point by calling init() again.
     *
     * \attention Every pointer to object retrieved from this instance will be
     * deleted. Be sure not to use any such pointer after calling this method.
     *
     * \sa init()
     */
    void quit();

    /*!
     * Scans the network for resources of interest.
     *
     * Using the default configuration the %HControlPoint automatically searches
     * and adds every device it finds. In that case the device list of
     * %HControlPoint usually reflects the UPnP device status of the network.
     * However, there are situations where you may want to explicitly ask
     * the %HControlPoint to update its status.
     *
     * \param resource specifies the resource or resources to be searched.
     *
     * \remarks
     * \li as a result of this call there may be any number of rootDeviceOnline()
     * signals emitted as a consequence of newly found devices.
     * \li the call will not affect the expiration of existing devices.
     * More specifically, any device or resource in general that does not respond
     * to the scan will not be considered as expired and no rootDeviceOffline()
     * will be sent consequently.
     */
    //void scan(const Herqq::Upnp::HResourceIdentifier& resource);

Q_SIGNALS:

    /*!
     * This signal is emitted when the initial subscription to the specified
     * service succeeded.
     *
     * \param service specifies the target service of the event subscription.
     *
     * \sa subscriptionFailed()
     */
    void subscriptionSucceeded(Herqq::Upnp::HService* service);

    /*!
     * This signal is emitted when an event subscription to the specified
     * service failed.
     *
     * \note this signal may be emitted in three different scenarios:
     * - the initial subscription failed
     * - a subscription renewal failed
     * - a re-subscription failed
     * If you want to try to re-subscribe to the service you can
     * call subscribe() again.
     *
     * \param service specifies the target service of the event subscription that
     * failed.
     *
     * \sa subscriptionSucceeded()
     */
    void subscriptionFailed(Herqq::Upnp::HService* service);

    /*!
     * This signal is emitted when the event subscription to the specified
     * service has been canceled.
     *
     * \param service specifies the target service of the subscription cancellation.
     */
    void subscriptionCanceled(Herqq::Upnp::HService* service);

    /*!
     * This signal is emitted when a device has been discovered and
     * added to the control point.
     *
     * \param device is the discovered device.
     *
     * \remarks the discovered device may already be in control of the control point.
     * This is the case when device goes offline and comes back online before
     * it is removed from the control point.
     *
     * \sa rootDeviceOffline(), removeDevice()
     */
    void rootDeviceOnline(Herqq::Upnp::HDevice* device);

    /*!
     * This signal is sent when a root device has announced that it is going
     * offline or the expiration timeout associated with the device tree has elapsed.
     *
     * After a device has gone offline you may want to remove the device from the
     * control point using removeDevice(). Alternatively, if you do not remove the
     * device and the device comes online later:
     *
     * \li a rootDeviceOnline() signal is emitted in case the device uses the
     * same configuration as it did before going offline or
     *
     * \li a rootDeviceInvalidated() signal is emitted in case the device uses
     * a different configuration as it did before going offline. If this is the case
     * you should remove the device as it no longer reflects the real device
     * accurately.
     *
     * \param device is the device that went offline and is not reachable
     * at the moment.
     *
     * \sa rootDeviceOnline(), rootDeviceInvalidated()
     */
    void rootDeviceOffline(Herqq::Upnp::HDevice* device);

    /*!
     * This signal is emitted when a previously discovered device has changed its
     * configuration and must be discarded.
     *
     * The UDA v1.1 specifies the configuration of a root device to consist of
     * the device description documents of all the devices in the device tree and
     * all the service description documents of the services in the device tree.
     * If the configuration changes the old device tree has to be discarded in
     * place of the new one.
     *
     * After this signal has been emitted the HDevice object identified by the
     * UDN has become invalid and should be discarded and removed immediately.
     * In addition, rootDeviceOnline() signal may be emitted shortly after this
     * signal if this control point instance accepts the new configuration of
     * the device.
     *
     * \param device is the device that has been invalidated.
     */
    void rootDeviceInvalidated(Herqq::Upnp::HDevice* device);

    /*!
     * This signal is emitted when a run-time error has occurred.
     *
     * \param err specifies the type of the error that occurred
     * \param errStr specifies a human-readable description of the error that
     * occurred.
     */
    //void error(ErrorType err, const QString& errStr);
};

}
}

#endif /* HCONTROL_POINT_H_ */
