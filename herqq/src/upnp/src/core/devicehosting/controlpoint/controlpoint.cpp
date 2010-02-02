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

#include "controlpoint.h"
#include "controlpoint_p.h"
#include "service_subscription_p.h"
#include "controlpoint_configuration.h"
#include "controlpoint_dataretriever_p.h"

#include "../upnp_global_p.h"
#include "../devicemodel/service_p.h"
#include "../dataelements/deviceinfo.h"

#include "../objectcreator_p.h"
#include "../datatype_mappings_p.h"

#include "actioninvoke_proxy_p.h"

#include "../messaging/usn.h"
#include "../messaging/product_tokens.h"
#include "../messaging/resource_identifier.h"

#include "../../../../utils/src/sysutils_p.h"
#include "../../../../utils/src/logger_p.h"

#include "../../../../core/include/HExceptions"

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
    HControlPointPrivate* owner, QObject* parent) :
        HHttpServer("__CONTROL POINT HTTP SERVER__: ", parent),
            m_owner(owner)
{
    Q_ASSERT(owner);
}

ControlPointHttpServer::~ControlPointHttpServer()
{
    HLOG(H_AT, H_FUN);
}

void ControlPointHttpServer::incomingNotifyMessage(
    MessagingInfo& mi, const NotifyRequest& req)
{
    // note, that currently this method is always executed in a thread from a
    // thread pool

    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);

    HLOG_DBG(QObject::tr("Incoming event notify from [%1]").arg(
        peerAsStr(mi.socket())));

    if (m_owner->m_initializationStatus != 2)
    {
        HLOG_DBG(QObject::tr(
            "The control point is not ready to accept notifications. Ignoring."));

        return;
    }

    QMutexLocker lock(&m_owner->m_serviceSubscribtionsMutex);

    QString serviceCallbackId = req.callback().path().remove('/');
    HServiceSubscribtion* subscription =
        m_owner->m_serviceSubscribtions[serviceCallbackId];

    lock.unlock();

    if (!subscription)
    {
        HLOG_WARN(QObject::tr(
            "Ignoring notification due to invalid callback ID [%1]").arg(
                serviceCallbackId));

        mi.setKeepAlive(false);
        m_httpHandler.responseBadRequest(mi);
        return;
    }

    subscription->onNotify(mi, req);
}

/*******************************************************************************
 * FetchAndAddDevice
 ******************************************************************************/
template<typename Msg>
void FetchAndAddDevice<Msg>::createEventSubscriptions(
    HDeviceController* device, QList<HServiceSubscribtion*>* subscriptions)
{
    HLOG(H_AT, H_FUN);
    Q_ASSERT(device);
    Q_ASSERT(subscriptions);

    QList<HServiceController*> services = device->services();
    foreach(HServiceController* service, services)
    {
        if (service->m_service->isEvented())
        {
            HServiceSubscribtion* subscription =
                new HServiceSubscribtion(
                    m_owner->m_loggingIdentifier,
                    *m_owner->m_http,
                    device->m_device->locations(),
                    service,
                    m_owner->m_server->rootUrl(),
                    m_owner->m_threadPool);

            subscriptions->push_back(subscription);
        }
    }

    QList<HDeviceController*> devices = device->embeddedDevices();
    foreach(HDeviceController* embDevice, devices)
    {
        createEventSubscriptions(embDevice, subscriptions);
    }
}

template<typename Msg>
FetchAndAddDevice<Msg>::FetchAndAddDevice(
    HControlPointPrivate* owner, const Msg& msg) :
        m_owner(owner), m_msg(msg), m_completionValue(-1),
        m_createdDevice(0)
{
}

template<typename Msg>
FetchAndAddDevice<Msg>::~FetchAndAddDevice()
{
   HLOG(H_AT, H_FUN);

    if (m_createdDevice.data())
    {
        m_createdDevice->deleteLater();
    }

    m_createdDevice.take();
}

template<typename Msg>
qint32 FetchAndAddDevice<Msg>::completionValue() const
{
    return m_completionValue;
}

template<typename Msg>
QString FetchAndAddDevice<Msg>::errorString() const
{
    return m_errorString;
}

template<typename Msg>
HDeviceController* FetchAndAddDevice<Msg>::createdDevice()
{
    return m_createdDevice.take();
}

template<typename Msg>
void FetchAndAddDevice<Msg>::deleteSubscriptions(
    const QList<HServiceSubscribtion*>& subscriptions)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);

    QMutexLocker lock(&m_owner->m_serviceSubscribtionsMutex);
    foreach(HServiceSubscribtion* ss, subscriptions)
    {
        if (m_owner->m_serviceSubscribtions.contains(ss->id()))
        {
            m_owner->m_serviceSubscribtions.remove(ss->id());
            ss->deleteLater();
        }
    }
}

template<typename Msg>
void FetchAndAddDevice<Msg>::run()
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);

    HUdn udn = m_msg.usn().udn();

    QList<HServiceSubscribtion*> subscriptions;
    try
    {
        QScopedPointer<HDeviceController> device(
            m_owner->fetchDevice(m_msg.location(), m_msg.cacheControlMaxAge()));

        // the returned device is a fully built root device containing every
        // embedded device and service advertised in the device and service descriptions
        // otherwise, the creation failed and an exception was thrown

        if (m_owner->state() != HControlPointPrivate::Initialized)
        {
            m_completionValue = -1;
            m_errorString = QObject::tr("Shutting down. Aborting device model build.");
            emit done(udn);

            return;
        }

        createEventSubscriptions(device.data(), &subscriptions);

        QMutexLocker lock(&m_owner->m_serviceSubscribtionsMutex);
        for(qint32 i = 0; i < subscriptions.size(); ++i)
        {
            subscriptions[i]->moveToThread(m_owner->thread());
            m_owner->m_serviceSubscribtions.insert(
                subscriptions[i]->id(), subscriptions[i]);
        }
        lock.unlock();

        // after the subscriptions are created, attempt to subscribe to every
        // service the subscriptions are representing.
        for(qint32 i = 0; i < subscriptions.size(); ++i)
        {
            if (m_owner->state() != HControlPointPrivate::Initialized)
            {
                break;
            }

            subscriptions[i]->subscribe();
        }

        if (m_owner->state() != HControlPointPrivate::Initialized)
        {
            deleteSubscriptions(subscriptions);
            m_completionValue = -1;
            m_errorString = QObject::tr("Shutting down. Aborting device model build.");

            emit done(udn);
        }
        else
        {
            device->moveToThread(m_owner->thread());
            device->m_device->moveToThread(m_owner->thread());

            m_completionValue = 0;
            m_createdDevice.swap(device);

            emit done(udn);
        }
    }
    catch(HException& ex)
    {
        HLOG_WARN(QObject::tr("Couldn't create a device: %1").arg(ex.reason()));

        deleteSubscriptions(subscriptions);

        m_completionValue = -1;
        m_errorString = ex.reason();

        emit done(udn);
    }
}

/*******************************************************************************
 * HControlPointPrivate
 ******************************************************************************/
HControlPointPrivate::HControlPointPrivate() :
    HAbstractHostPrivate(
        QString("__CONTROL POINT %1__: ").arg(QUuid::createUuid().toString())),
            m_buildsInProgress(),
            m_initParams(),
            m_ssdp(0),
            m_server(0),
            m_serviceSubscribtions(),
            m_serviceSubscribtionsMutex(QMutex::Recursive),
            m_deviceCreationMutex()
{
}

HControlPointPrivate::~HControlPointPrivate()
{
    HLOG(H_AT, H_FUN);

    QList<DeviceBuildProcess*> builds = m_buildsInProgress.values();
    foreach(DeviceBuildProcess* build, builds)
    {
        delete build->m_asyncOperation;
        delete build;
    }

    QList<QThread*> runners = m_actionRunnerThreads.values();
    foreach(QThread* runner, runners)
    {
        runner->quit();
        runner->wait();
    }
}

HActionInvoke HControlPointPrivate::createActionInvoker(
    HService* service,
    const QString& actionName,
    const HActionInputArguments& inArgs,
    const HActionOutputArguments& outArgs)
{
    const HDevice* device = service->parentDevice();
    while(device->parentDevice()) { device = device->parentDevice(); }

    QMutexLocker lock(&m_actionRunnerThreadsMutex);

    HUdn udn = device->deviceInfo().udn();

    QThread* runnerThread = m_actionRunnerThreads.value(udn);
    if (!runnerThread)
    {
        runnerThread = new QThread();
        runnerThread->start();
        m_actionRunnerThreads.insert(udn, runnerThread);
    }

    return HActionInvokeProxy(runnerThread, service, actionName, inArgs, outArgs);
}

HDeviceController* HControlPointPrivate::fetchDevice(
    QUrl deviceLocation, qint32 maxAgeInSecs)
{
    HLOG(H_AT, H_FUN);

    DataRetriever dataRetriever(m_loggingIdentifier, *m_http);

    QDomDocument dd =
        dataRetriever.retrieveDeviceDescription(deviceLocation);

    QList<QUrl> deviceLocations;
    deviceLocations.push_back(deviceLocation);

    HObjectCreationParameters creatorParams;
    creatorParams.m_createDefaultObjects = true;
    creatorParams.m_deviceDescription    = dd;
    creatorParams.m_deviceCreator        = m_initParams->deviceCreator();
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

    HObjectCreator creator(creatorParams);

    return creator.createRootDevice();
}

void HControlPointPrivate::addRootDevice_(HDeviceController* newRootDevice)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    HDeviceController* existingDevice =
        m_deviceStorage->searchDeviceByUdn(
            newRootDevice->m_device->deviceInfo().udn());

    if (existingDevice)
    {
        Q_ASSERT(!existingDevice->m_device->parentDevice());

        existingDevice->addLocations(newRootDevice->m_device->locations());
        return;
    }

    newRootDevice->setParent(this);
    newRootDevice->m_device->setParent(this);
    newRootDevice->startStatusNotifier(HDeviceController::All);

    Q_ASSERT(QObject::connect(
            newRootDevice, SIGNAL(statusTimeout(HDeviceController*)),
            this, SLOT(deviceExpired(HDeviceController*))));

    try
    {
        addRootDevice(newRootDevice);
    }
    catch(HException& ex)
    {
        HLOG_WARN(QObject::tr(
            "Failed to add root device [UDN: %1]: %2").arg(
                newRootDevice->m_device->deviceInfo().udn().toSimpleUuid(),
                ex.reason()));

        removeRootDeviceSubscriptions(newRootDevice, true);
        delete newRootDevice;
    }
}

void HControlPointPrivate::deviceExpired(HDeviceController* source)
{
    HLOG(H_AT, H_FUN);
    QMutexLocker lock(&m_deviceStorage->m_rootDevicesMutex);

    // according to the UDA v1.1 a "device tree" (root, embedded and services)
    // are "timed out" only when every advertisement has timed out.

    source = source->rootDevice();

    if (source->isTimedout(HDeviceController::All))
    {
        removeRootDeviceAndSubscriptions(source, false);
    }
}

bool HControlPointPrivate::discoveryRequestReceived(
    const HDiscoveryRequest&, const HEndpoint&, const HEndpoint&)
{
    return true;
}

void HControlPointPrivate::removeRootDeviceSubscriptions(
    HDeviceController* rootDevice, bool unsubscribe)
{
    Q_ASSERT(rootDevice);
    Q_ASSERT(!rootDevice->m_device->parentDevice());
    // this method should be called only with root devices

    // when removing a root device all of the subscriptions for services contained
    // within the root device have to be removed as well.

    QMutexLocker lock(&m_serviceSubscribtionsMutex);

    QList<HServiceSubscribtion*> subscriptions = m_serviceSubscribtions.values();
    QList<HServiceSubscribtion*>::const_iterator ci = subscriptions.constBegin();

    for(; ci != subscriptions.constEnd(); ++ci)
    {
        HServiceSubscribtion* subscription = (*ci);

        // seek the root device of the device tree to which the service that contains
        // the subscription belongs.
        const HDevice* device = subscription->service()->parentDevice();
        while(device->parentDevice()) { device = device->parentDevice(); }

        if (device == rootDevice->m_device.data())
        {
            // the service appears to belong to the device tree that is about
            // to be removed

            qint32 i = m_serviceSubscribtions.remove(subscription->id());
            Q_ASSERT(i == 1); Q_UNUSED(i)

            if (unsubscribe)
            {
                subscription->unsubscribe(true);
            }

            delete subscription;
        }
    }
}

void HControlPointPrivate::removeRootDeviceAndSubscriptions(
    HDeviceController* rootDevice, bool unsubscribe)
{
    HLOG(H_AT, H_FUN);

    removeRootDeviceSubscriptions(rootDevice, unsubscribe);
    removeRootDevice(rootDevice);
}

template<typename Msg>
bool HControlPointPrivate::processDeviceDiscovery(
    const Msg& msg, const HEndpoint& /*source*/)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(thread() == QThread::currentThread());

    HUdn resourceUdn = msg.usn().udn();

    QMutexLocker lock(&m_deviceStorage->m_rootDevicesMutex);
    HDeviceController* device = m_deviceStorage->searchDeviceByUdn(msg.usn().udn());

    if (device)
    {
        // according to the UDA v1.1 spec, if a control point receives an alive announcement
        // of any type for a device tree, the control point can assume that
        // all devices and services are available. ==> reset timeouts
        // for entire device tree and all services.

        device = device->rootDevice();
        device->startStatusNotifier(HDeviceController::All);

        // it cannot be that only some embedded device is available at certain
        // interface, since the device description is always fetched from the
        // the location that the root device specifies. ergo, the entire device
        // tree has to be available at that location.
        device->addLocation(msg.location());
        return true;
    }

    // it does not matter if the device is an embedded device, since the
    // location of the device points to the root device's description in any case
    // and the internal device model is built of that. Hence, it is only necessary
    // to get an advertisement of a root or an embedded device to build the entire
    // model correctly.

    DeviceBuildProcess* dbp = m_buildsInProgress.get(msg);
    if (dbp)
    {
        if (!dbp->m_locations.contains(msg.location()))
        {
            dbp->m_locations.push_back(msg.location());
        }

        return true;
    }

    FetchAndAddDevice<Msg>* task = new FetchAndAddDevice<Msg>(this, msg);
    task->setAutoDelete(false);

    DeviceBuildProcess* newBuildProcess = new DeviceBuildProcess();
    newBuildProcess->m_asyncOperation = task;
    newBuildProcess->m_locations.push_back(msg.location());
    newBuildProcess->m_udn.reset(new HUdn(resourceUdn));

    m_buildsInProgress.add(newBuildProcess);

    bool ok = connect(
        task, SIGNAL(done(Herqq::Upnp::HUdn)),
        this, SLOT(deviceModelBuildDone(Herqq::Upnp::HUdn)));

    Q_ASSERT(ok); Q_UNUSED(ok)

    HLOG_INFO(QObject::tr("New resource [%1] is available @ [%2]. Attempting to build the device model.").arg(
        msg.usn().resource().toString(), msg.location().toString()));

    m_threadPool->start(task);

    return true;
}

void HControlPointPrivate::deviceModelBuildDone(Herqq::Upnp::HUdn udn)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(thread() == QThread::currentThread());

    DeviceBuildProcess* build = m_buildsInProgress.get(udn);
    Q_ASSERT(build);

    if (build->m_asyncOperation->completionValue() == 0)
    {
        HLOG_INFO(QObject::tr("Device model for [%1] built successfully.").arg(
            udn.toString()));

        HDeviceController* device = build->m_asyncOperation->createdDevice();
        Q_ASSERT(device);

        for (qint32 i = 0; i < build->m_locations.size(); ++i)
        {
            device->addLocation(build->m_locations[i]);
        }

        addRootDevice_(device);
    }
    else
    {
        HLOG_WARN(QObject::tr("Device model for [%1] could not be built: %2.").arg(
            udn.toString(), build->m_asyncOperation->errorString()));
    }

    m_buildsInProgress.remove(udn);

    delete build->m_asyncOperation;
    delete build;
}

bool HControlPointPrivate::discoveryResponseReceived(
    const HDiscoveryResponse& msg, const HEndpoint& source)
{
    HLOG(H_AT, H_FUN);
    return processDeviceDiscovery(msg, source);
}

bool HControlPointPrivate::resourceUnavailableReceived(
    const HResourceUnavailable& msg)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QMutexLocker lock(&m_deviceStorage->m_rootDevicesMutex);

    HDeviceController* device = m_deviceStorage->searchDeviceByUdn(msg.usn().udn());
    if (!device)
    {
        // the device is not (for whatever reason) known by us.
        // note that even service announcements contain the "UDN", which identifies
        // the device that contains them.
        return true;
    }

    HLOG_INFO(QObject::tr("Resource [%1] is unavailable.").arg(
        msg.usn().resource().toString()));

    // according to the UDA v1.1 specification, if a bye bye message of any kind
    // is received, the control point can assume that nothing in that
    // device tree is available anymore

    HDeviceController* root = device->rootDevice();
    Q_ASSERT(root);

    removeRootDeviceAndSubscriptions(root, false);

    return true;
}

bool HControlPointPrivate::resourceAvailableReceived(
    const HResourceAvailable& msg)
{
    HLOG(H_AT, H_FUN);
    return processDeviceDiscovery(msg);
}

bool HControlPointPrivate::readyForEvents()
{
    return m_initializationStatus == 2;
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

    delete m_server; m_server = 0;
    // shut down the http server. this call will block until all threads
    // created by the http server have finished. however, that should be fairly
    // fast, since every operation in this class monitors the shutdown flag.

    QMutexLocker lock(&m_serviceSubscribtionsMutex);
    QList<HServiceSubscribtion*> subscriptions =
        m_serviceSubscribtions.values();

    QList<HServiceSubscribtion*>::iterator i = subscriptions.begin();
    for(; i != subscriptions.end(); ++i)
    {
        try
        {
            (*i)->unsubscribe(true);
        }
        catch(HException&)
        {
            // intentional. at most could print something.
        }

        delete *i;
    }

    m_serviceSubscribtions.clear();

    m_threadPool->waitForDone();
    // ensure that no threads created by this thread pool are running when we
    // start deleting shared objects.

    delete m_ssdp; m_ssdp = 0;

    m_initParams.reset(0);
    m_http.reset(0);

    m_initializationStatus = 0;

    // once this method exists, the abstract host will proceed to delete
    // the device tree, which is safe by now.
}

/*******************************************************************************
 * HControlPoint
 ******************************************************************************/
HControlPoint::HControlPoint(QObject* parent) :
    HAbstractHost(*new HControlPointPrivate(), parent)
{
    HLOG(H_AT, H_FUN);
}

HControlPoint::~HControlPoint()
{
    HLOG(H_AT, H_FUN);
    quit();
}

HControlPoint::ReturnCode HControlPoint::init(
    const HControlPointConfiguration* initParams, QString* errorString)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    H_D(HControlPoint);

    Q_ASSERT_X(
        thread() == QThread::currentThread(), H_AT,
        "The control point has to be initialized in the thread in which it is currently located.");

    if (h->state() == HAbstractHostPrivate::Initialized)
    {
        return AlreadyInitialized;
    }

    Q_ASSERT(h->state() == HAbstractHostPrivate::Uninitialized);

    QString error;
    HControlPoint::ReturnCode rc = Success;
    try
    {
        h->setState(HAbstractHostPrivate::Initializing);

        HLOG_INFO(QObject::tr("ControlPoint initializing."));

        h->m_initParams.reset(initParams ?
            initParams->clone() : new HControlPointConfiguration());

        h->m_http.reset(new HHttpHandler());

        h->m_server = new ControlPointHttpServer(h, this);
        if (!h->m_server->listen())
        {
            rc = UndefinedFailure;
        }
        else
        {
            h->m_ssdp = new SsdpWithoutEventing<HControlPointPrivate>(h, this);

            HLOG_DBG(QObject::tr("Searching for UPnP devices..."));

            h->m_ssdp->sendDiscoveryRequest(
                HDiscoveryRequest(1, HResourceIdentifier("ssdp:all"), herqqProductTokens()));

            h->setState(HAbstractHostPrivate::Initialized);
        }
    }
    catch(HException& ex)
    {
        error = ex.reason();
        rc = UndefinedFailure;
    }

    if (rc != Success)
    {
        HLOG_WARN(error);

        if (errorString)
        {
            *errorString = error;
        }

        h->setState(HAbstractHostPrivate::Exiting);
        h->clear();
        HLOG_INFO(QObject::tr("ControlPoint initialization failed."));

        return rc;
    }

    HLOG_INFO(QObject::tr("ControlPoint initialized."));

    return rc;
}

void HControlPoint::quit()
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    H_D(HControlPoint);

    Q_ASSERT_X(
        thread() == QThread::currentThread(), H_AT,
        "The control point has to be shutdown in the thread in which it is currently located.");

    if (!isStarted())
    {
        return;
    }

    HLOG_INFO(QObject::tr("ControlPoint shutting down."));

    h->setState(HAbstractHostPrivate::Exiting);
    h->clear();

    HLOG_INFO(QObject::tr("ControlPoint shut down."));
}

/*******************************************************************************
 * IFetchAndAddDevice
 ******************************************************************************/
IFetchAndAddDevice::IFetchAndAddDevice()
{
}

IFetchAndAddDevice::~IFetchAndAddDevice()
{
}

}
}
