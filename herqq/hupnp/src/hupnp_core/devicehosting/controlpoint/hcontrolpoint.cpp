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

#include "hcontrolpoint.h"
#include "hcontrolpoint_p.h"
#include "hactioninvoke_proxy_p.h"
#include "hevent_subscription_p.h"
#include "hcontrolpoint_configuration.h"
#include "hcontrolpoint_dataretriever_p.h"

#include "./../hobjectcreator_p.h"
#include "./../../general/hupnp_global_p.h"
#include "./../../devicemodel/hservice_p.h"
#include "./../../dataelements/hdeviceinfo.h"
#include "./../../dataelements/hproduct_tokens.h"
#include "./../../datatypes/hdatatype_mappings_p.h"

#include "./../../ssdp/husn.h"
#include "./../../ssdp/hresource_identifier.h"
#include "./../../http/hhttp_messagecreator_p.h"

#include "./../../../utils/hlogger_p.h"
#include "./../../../utils/hsysutils_p.h"
#include "./../../../utils/hexceptions_p.h"

#include <QUrl>
#include <QString>
#include <QTcpSocket>
#include <QMutexLocker>
#include <QHttpRequestHeader>
#include <QHttpResponseHeader>

#include <QtSoapMessage>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * ControlPointHttpServer
 ******************************************************************************/
ControlPointHttpServer::ControlPointHttpServer(
    HControlPointPrivate* owner) :
        HHttpServer(owner->m_loggingIdentifier, owner),
            m_owner(owner)
{
    Q_ASSERT(m_owner);
}

ControlPointHttpServer::~ControlPointHttpServer()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
}

void ControlPointHttpServer::incomingNotifyMessage(
    MessagingInfo& mi, const NotifyRequest& req)
{
    // note, that currently this method is always executed in a thread from a
    // thread pool

    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);

    HLOG_DBG(QString("Incoming event notify from [%1]").arg(
        peerAsStr(mi.socket())));

    if (m_owner->m_initializationStatus != 2)
    {
        HLOG_DBG("The control point is not ready to accept notifications. Ignoring.");

        return;
    }

    QString serviceCallbackId = req.callback().path().remove('/');

    if (!m_owner->m_eventSubscriber->onNotify(serviceCallbackId, mi, req))
    {
        mi.setKeepAlive(false);
        m_httpHandler.send(mi, BadRequest);
        return;
    }
}

/*******************************************************************************
 * HControlPointSsdpHandler
 ******************************************************************************/
HControlPointSsdpHandler::HControlPointSsdpHandler(
    HControlPointPrivate* owner) :
        HSsdp(owner->m_loggingIdentifier, owner), m_owner(owner)
{
    setFilter(DiscoveryResponse | DeviceUnavailable | DeviceAvailable);
}

HControlPointSsdpHandler::~HControlPointSsdpHandler()
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
}

bool HControlPointSsdpHandler::incomingDiscoveryResponse(
    const HDiscoveryResponse& msg, const HEndpoint& source)
{
    return m_owner->processDeviceDiscovery(msg, source);
}

bool HControlPointSsdpHandler::incomingDeviceAvailableAnnouncement(
    const HResourceAvailable& msg)
{
    return m_owner->processDeviceDiscovery(msg);
}

bool HControlPointSsdpHandler::incomingDeviceUnavailableAnnouncement(
    const HResourceUnavailable& msg)
{
    return m_owner->processDeviceOffline(msg);
}

/*******************************************************************************
 * HControlPointPrivate
 ******************************************************************************/
HControlPointPrivate::HControlPointPrivate() :
    HAbstractHostPrivate(
        QString("__CONTROL POINT %1__: ").arg(QUuid::createUuid().toString())),
            m_deviceBuildTasks(),
            m_configuration(),
            m_ssdp(0),
            m_server(0),
            m_eventSubscriber(0),
            m_deviceCreationMutex()
{
}

HControlPointPrivate::~HControlPointPrivate()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
}

HActionInvoke HControlPointPrivate::createActionInvoker(HAction* action)
{
    return HActionInvokeProxy(m_loggingIdentifier, action);
}

HDeviceController* HControlPointPrivate::buildDevice(
    QUrl deviceLocation, qint32 maxAgeInSecs)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    DataRetriever dataRetriever(m_loggingIdentifier, *m_http);

    QDomDocument dd =
        dataRetriever.retrieveDeviceDescription(deviceLocation);

    QList<QUrl> deviceLocations;
    deviceLocations.push_back(deviceLocation);

    HObjectCreationParameters creatorParams;
    creatorParams.m_createDefaultObjects = true;
    creatorParams.m_deviceDescription    = dd;
    creatorParams.m_deviceCreator        = m_configuration->deviceCreator();
    creatorParams.m_deviceLocations      = deviceLocations;

    creatorParams.m_serviceDescriptionFetcher =
        ServiceDescriptionFetcher(
            &dataRetriever, &DataRetriever::retrieveServiceDescription);

    creatorParams.m_actionInvokeCreator  =
        ActionInvokeCreator(this, &HControlPointPrivate::createActionInvoker);

    creatorParams.m_deviceTimeoutInSecs  = maxAgeInSecs;
    creatorParams.m_appendUdnToDeviceLocation = false;
    creatorParams.m_sharedActionInvokers = &m_sharedActionInvokers;

    creatorParams.m_iconFetcher =
        IconFetcher(&dataRetriever, &DataRetriever::retrieveIcon);

    creatorParams.m_strictParsing = false;
    creatorParams.m_stateVariablesAreImmutable = true;
    creatorParams.m_threadPool = m_threadPool;
    creatorParams.m_loggingIdentifier = m_loggingIdentifier;

    HObjectCreator creator(creatorParams);

    return creator.createRootDevice();
}

void HControlPointPrivate::addRootDevice_(HDeviceController* newRootDevice)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(thread() == QThread::currentThread());

    QScopedPointer<HDeviceController> newRootDevicePtr(newRootDevice);

    HDeviceController* existingDevice =
        m_deviceStorage->searchDeviceByUdn(
            newRootDevice->m_device->deviceInfo().udn());

    if (existingDevice)
    {
        // it seems that the device we've built has already been added
        // (it is possible, although unlikely, we begin multiple device build
        // processes of the same device tree)
        // in this case we only make sure that the device's location list is
        // updated if necessary

        existingDevice = existingDevice->rootDevice();
        existingDevice->addLocations(newRootDevice->m_device->locations());
        return;
    }

    if (q_ptr->acceptRootDevice(newRootDevice->m_device) == HControlPoint::Ignore)
    {
        HLOG_DBG(QString("Device [%1] rejected").arg(
            newRootDevice->m_device->deviceInfo().udn().toString()));
        return;
    }

    newRootDevice->setParent(this);
    newRootDevice->startStatusNotifier(HDeviceController::All);

    bool ok = connect(
        newRootDevice, SIGNAL(statusTimeout(HDeviceController*)),
        this, SLOT(deviceExpired(HDeviceController*)));

    Q_ASSERT(ok); Q_UNUSED(ok)

    try
    {
        m_deviceStorage->addRootDevice(newRootDevice);
        emit q_ptr->rootDeviceOnline(newRootDevice->m_device);

        newRootDevicePtr.take();
    }
    catch(HException& ex)
    {
        HLOG_WARN(QString(
            "Failed to add root device [UDN: %1]: %2").arg(
                newRootDevice->m_device->deviceInfo().udn().toSimpleUuid(),
                ex.reason()));

        m_eventSubscriber->remove(newRootDevice->m_device, true);
    }
}

void HControlPointPrivate::deviceExpired(HDeviceController* source)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(thread() == QThread::currentThread());

    if (state() == Exiting)
    {
        return;
    }

    QMutexLocker lock(&m_deviceStorage->m_rootDevicesMutex);

    // according to the UDA v1.1 a "device tree" (root, embedded and services)
    // are "timed out" only when every advertisement has timed out.

    source = source->rootDevice();

    if (source->isTimedout(HDeviceController::All))
    {
        source->deviceStatus()->setOnline(false);
        m_eventSubscriber->cancel(source->m_device, true, false);
        lock.unlock();

        emit q_ptr->rootDeviceOffline(source->m_device);
    }
}

void HControlPointPrivate::unsubscribed(HService* service)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(service);
    emit q_ptr->subscriptionCanceled(service);
}

bool HControlPointPrivate::processDeviceOffline(
    const HResourceUnavailable& msg)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(thread() == QThread::currentThread());

    if (state() == Exiting)
    {
        return true;
    }

    QMutexLocker lock(&m_deviceStorage->m_rootDevicesMutex);

    HDeviceController* device =
        m_deviceStorage->searchDeviceByUdn(msg.usn().udn());

    if (!device)
    {
        // the device is not known by us.
        // note that even service announcements contain the "UDN", which identifies
        // the device that contains them.
        return true;
    }

    HLOG_INFO(QString("Resource [%1] is unavailable.").arg(
        msg.usn().resource().toString()));

    // according to the UDA v1.1 specification, if a bye bye message of any kind
    // is received, the control point can assume that nothing in that
    // device tree is available anymore

    HDeviceController* root = device->rootDevice();
    Q_ASSERT(root);

    root->deviceStatus()->setOnline(false);
    m_eventSubscriber->cancel(root->m_device, true, false);
    emit q_ptr->rootDeviceOffline(root->m_device);

    return true;
}

template<typename Msg>
bool HControlPointPrivate::processDeviceDiscovery(
    const Msg& msg, const HEndpoint& source)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(thread() == QThread::currentThread());

    if (state() == Exiting)
    {
        return true;
    }

    HUdn resourceUdn = msg.usn().udn();

    QMutexLocker lock(&m_deviceStorage->m_rootDevicesMutex);

    HDeviceController* device =
        m_deviceStorage->searchDeviceByUdn(msg.usn().udn());

    if (device)
    {
        // according to the UDA v1.1 spec, if a control point receives an
        // alive announcement of any type for a device tree, the control point
        // can assume that all devices and services are available.
        // ==> reset timeouts for entire device tree and all services.

        device = device->rootDevice();
        device->startStatusNotifier(HDeviceController::All);

        // it cannot be that only some embedded device is available at certain
        // interface, since the device description is always fetched from the
        // the location that the root device specifies ==> the entire device
        // tree has to be available at that location.
        device->addLocation(msg.location());

        if (!device->deviceStatus()->online())
        {
            device->deviceStatus()->setOnline(true);
            emit q_ptr->rootDeviceOnline(device->m_device);
            processDeviceOnline(device, false);
        }

        return true;
    }

    // it does not matter if the device is an embedded device, since the
    // location of the device always points to the root device's description
    // and the internal device model is built of that. Hence, any advertisement
    // will do to build the entire model correctly.

    DeviceBuildTask* dbp = m_deviceBuildTasks.get(msg);
    if (dbp)
    {
        if (!dbp->m_locations.contains(msg.location()))
        {
            dbp->m_locations.push_back(msg.location());
        }

        return true;
    }

    if (!q_ptr->acceptResourceAd(msg.usn(), source))
    {
        HLOG_DBG(QString("Resource advertisement [%1] rejected").arg(
            msg.usn().toString()));

        return true;
    }

    DeviceBuildTask* newBuildTask = new DeviceBuildTask(this, msg);
    newBuildTask->setAutoDelete(false);

    newBuildTask->m_locations.push_back(msg.location());

    m_deviceBuildTasks.add(newBuildTask);

    bool ok = connect(
        newBuildTask, SIGNAL(done(Herqq::Upnp::HUdn)),
        this, SLOT(deviceModelBuildDone(Herqq::Upnp::HUdn)));

    Q_ASSERT(ok); Q_UNUSED(ok)

    HLOG_INFO(QString(
        "New resource [%1] is available @ [%2]. "
        "Attempting to build the device model.").arg(
            msg.usn().resource().toString(), msg.location().toString()));

    m_threadPool->start(newBuildTask);

    return true;
}

void HControlPointPrivate::processDeviceOnline(
    HDeviceController* device, bool newDevice)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    HControlPoint::DeviceDiscoveryAction actionToTake =
        q_ptr->acceptRootDevice(device->m_device);

    bool subscribe = false;
    switch(actionToTake)
    {
    case HControlPoint::Ignore:

        HLOG_DBG(QString("Discarding device with UDN %1").arg(
            device->m_device->deviceInfo().udn().toString()));

        if (newDevice) { delete device; device = 0; }
        else { m_deviceStorage->removeRootDevice(device); }
        break;

    case HControlPoint::AddOnly:
        break;

    case HControlPoint::AddAndFollowConfiguration:
        subscribe = m_configuration->subscribeToEvents();
        break;

    case HControlPoint::AddAndSubscribeToAll:
        subscribe = true;
        break;

    default:
        Q_ASSERT(false);
        break;
    };

    if (device)
    {
        if (newDevice)
        {
            addRootDevice_(device);
        }
        if (subscribe)
        {
            m_eventSubscriber->subscribe(
                device->m_device, true,
                m_configuration->desiredSubscriptionTimeout());
        }
    }
}

void HControlPointPrivate::deviceModelBuildDone(const Herqq::Upnp::HUdn& udn)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(thread() == QThread::currentThread());

    if (state() == Exiting)
    {
        return;
    }

    DeviceBuildTask* build = m_deviceBuildTasks.get(udn);
    Q_ASSERT(build);

    if (build->completionValue() == 0)
    {
        HLOG_INFO(QString("Device model for [%1] built successfully.").arg(
            udn.toString()));

        HDeviceController* device = build->createdDevice();
        Q_ASSERT(device);

        for (qint32 i = 0; i < build->m_locations.size(); ++i)
        {
            device->addLocation(build->m_locations[i]);
        }

        processDeviceOnline(device, true);
    }
    else
    {
        HLOG_WARN(QString("Device model for [%1] could not be built: %2.").arg(
            udn.toString(), build->errorString()));
    }

    m_deviceBuildTasks.remove(udn);
}

void HControlPointPrivate::doClear()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(thread() == QThread::currentThread());

    // called by the abstract host just before it starts to delete the device
    // tree.

    Q_ASSERT(state() == Exiting);

    m_http->shutdown(false);
    // this will tell the http handler that operations should quit as
    // soon as possible.

    m_eventSubscriber->cancelAll(100);
    m_eventSubscriber->removeAll();

    QAbstractEventDispatcher* ed = QAbstractEventDispatcher::instance();
    while(m_threadPool->activeThreadCount())
    {
        ed->processEvents(QEventLoop::ExcludeUserInputEvents);
    }

    m_threadPool->waitForDone();
    // ensure that no threads created by this thread pool are running when we
    // start deleting shared objects.

    q_ptr->doQuit();
    // At this point all that is left is to delete
    // the private data structures ==> allow derived classes to run their
    // "finalizers" before cleaning up

    delete m_server; m_server = 0;
    delete m_ssdp; m_ssdp = 0;
    delete m_eventSubscriber; m_eventSubscriber = 0;

    m_http.reset(0);

    m_initializationStatus = 0;

    QAbstractEventDispatcher::instance()->processEvents(
        QEventLoop::ExcludeUserInputEvents | QEventLoop::ExcludeSocketNotifiers);
    // this is execute to ensure that if there is deferred deletion to be performed
    // on some objects, the deleters get a chance to run.

    // once this method exists, the abstract host will proceed to delete
    // the device tree, which is safe by now.
}

/*******************************************************************************
 * HControlPoint
 ******************************************************************************/
HControlPoint::HControlPoint(
    const HControlPointConfiguration* initParams, QObject* parent) :
        QObject(parent), h_ptr(new HControlPointPrivate())
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    h_ptr->m_configuration.reset(initParams ?
        initParams->clone() : new HControlPointConfiguration());

    h_ptr->setParent(this);
    h_ptr->q_ptr = this;
}

HControlPoint::HControlPoint(
    HControlPointPrivate& dd,
    const HControlPointConfiguration* initParams, QObject* parent) :
        QObject(parent), h_ptr(&dd)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    h_ptr->m_configuration.reset(initParams ?
        initParams->clone() : new HControlPointConfiguration());

    h_ptr->setParent(this);
    h_ptr->q_ptr = this;
}

HControlPoint::~HControlPoint()
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    quit();
    delete h_ptr;
}

HControlPoint::ReturnCode HControlPoint::doInit()
{
    // the default implementation does nothing.
    return Success;
}

void HControlPoint::doQuit()
{
    // the default implementation does nothing.
}

HControlPoint::DeviceDiscoveryAction HControlPoint::acceptRootDevice(
    HDevice* /*device*/)
{
    return AddAndFollowConfiguration;
}

bool HControlPoint::acceptResourceAd(
    const HUsn& /*usn*/, const HEndpoint& /*source*/)
{
    return true;
}

const HControlPointConfiguration* HControlPoint::configuration() const
{
    return h_ptr->m_configuration.data();
}

HControlPoint::ReturnCode HControlPoint::init(QString* errorString)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    Q_ASSERT_X(
        thread() == QThread::currentThread(), H_AT,
        "The control point has to be initialized in the thread in which it is "
        "currently located.");

    if (h_ptr->state() == HAbstractHostPrivate::Initialized)
    {
        return AlreadyInitialized;
    }

    Q_ASSERT(h_ptr->state() == HAbstractHostPrivate::Uninitialized);

    QString error;
    ReturnCode rc = Success;
    try
    {
        h_ptr->setState(HAbstractHostPrivate::Initializing);

        HLOG_INFO("ControlPoint initializing.");

        h_ptr->m_eventSubscriber = new HEventSubscriptionManager(h_ptr);

        bool ok = connect(
            h_ptr->m_eventSubscriber,
            SIGNAL(subscribed(Herqq::Upnp::HService*)),
            this,
            SIGNAL(subscriptionSucceeded(Herqq::Upnp::HService*)));

        Q_ASSERT(ok); Q_UNUSED(ok)

        ok = connect(
            h_ptr->m_eventSubscriber,
            SIGNAL(subscriptionFailed(Herqq::Upnp::HService*)),
            this,
            SIGNAL(subscriptionFailed(Herqq::Upnp::HService*)));

        Q_ASSERT(ok);

        ok = connect(
            h_ptr->m_eventSubscriber,
            SIGNAL(unsubscribed(Herqq::Upnp::HService*)),
            h_ptr,
            SLOT(unsubscribed(Herqq::Upnp::HService*)));

        Q_ASSERT(ok);

        h_ptr->m_ssdp = new HControlPointSsdpHandler(h_ptr);
        h_ptr->m_http.reset(new HHttpHandler(h_ptr->m_loggingIdentifier));
        h_ptr->m_server = new ControlPointHttpServer(h_ptr);

        rc = doInit();
        if (rc != Success)
        {
            goto end;
        }

        if (!h_ptr->m_server->listen())
        {
            rc = CommunicationsError;
            goto end;
        }

        if (!h_ptr->m_ssdp->bind())
        {
            rc = CommunicationsError;
            goto end;
        }

        HLOG_DBG("Searching for UPnP devices...");

        h_ptr->m_ssdp->sendDiscoveryRequest(
            HDiscoveryRequest(
                1, HResourceIdentifier::getRootDeviceIdentifier(), herqqProductTokens()));

        h_ptr->setState(HAbstractHostPrivate::Initialized);
    }
    catch(HSocketException& ex)
    {
        error = ex.reason();
        rc = CommunicationsError;
    }
    catch(HException& ex)
    {
        error = ex.reason();
        rc = UndefinedFailure;
    }

end:

    if (rc != Success)
    {
        HLOG_WARN(error);

        if (errorString)
        {
            *errorString = error;
        }

        h_ptr->setState(HAbstractHostPrivate::Exiting);
        h_ptr->clear();

        HLOG_INFO("ControlPoint initialization failed.");

        return rc;
    }

    HLOG_INFO("ControlPoint initialized.");

    return rc;
}

void HControlPoint::quit()
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    Q_ASSERT_X(
        thread() == QThread::currentThread(), H_AT,
        "The control point has to be shutdown in the thread in which it is "
        "currently located.");

    if (!isStarted())
    {
        return;
    }

    HLOG_INFO("Shutting down.");

    h_ptr->setState(HAbstractHostPrivate::Exiting);
    h_ptr->clear();

    HLOG_INFO("Shut down.");
}

bool HControlPoint::isStarted() const
{
    return h_ptr->state() == HAbstractHostPrivate::Initialized;
}

HDeviceList HControlPoint::rootDevices() const
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (!isStarted())
    {
        HLOG_WARN("The control point is not started");
        return HDeviceList();
    }

    return h_ptr->m_deviceStorage->rootDevices();
}

HDevice* HControlPoint::rootDevice(const HUdn& udn) const
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (!isStarted())
    {
        HLOG_WARN("The control point is not started");
        return 0;
    }

    HDeviceController* dc = h_ptr->m_deviceStorage->searchDeviceByUdn(udn);

    return dc ?
        h_ptr->m_deviceStorage->searchDeviceByUdn(udn)->m_device : 0;
}

HControlPoint::ReturnCode HControlPoint::subscribeEvents(
    HDevice* device, bool recursive)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (!device)
    {
        return InvalidArgument;
    }

    h_ptr->m_eventSubscriber->subscribe(
        device, recursive, h_ptr->m_configuration->desiredSubscriptionTimeout());

    return Success;
}

HControlPoint::ReturnCode HControlPoint::subscribeEvents(HService* service)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (!service)
    {
        return InvalidArgument;
    }

    return h_ptr->m_eventSubscriber->subscribe(
        service, h_ptr->m_configuration->desiredSubscriptionTimeout()) ?
            Success : InvalidArgument;;
}

HControlPoint::ReturnCode HControlPoint::cancelEvents(
    HDevice* device, bool recursive)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (!device)
    {
        return InvalidArgument;
    }

    return h_ptr->m_eventSubscriber->cancel(
        device, recursive, true) ? Success : InvalidArgument;
}

HControlPoint::ReturnCode HControlPoint::cancelEvents(HService* service)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (!service)
    {
        return InvalidArgument;
    }

    return h_ptr->m_eventSubscriber->cancel(service, true) ? Success : InvalidArgument;
}

HControlPoint::ReturnCode HControlPoint::removeDevice(HDevice* rootDevice)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (!rootDevice || rootDevice->parentDevice())
    {
        return InvalidArgument;
    }

    Q_ASSERT(thread() == QThread::currentThread());

    HDeviceController* controller =
        static_cast<HDeviceController*>(rootDevice->parent());

    h_ptr->m_eventSubscriber->remove(rootDevice, true);
    // TODO should send unsubscription to the UPnP device?

    return h_ptr->m_deviceStorage->removeRootDevice(controller) ?
            Success : InvalidArgument;
}

//void HControlPoint::scan(const HResourceIdentifier& /*resource*/)
//{

//}

}
}
