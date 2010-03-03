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

    HLOG_DBG(QObject::tr("Incoming event notify from [%1]").arg(
        peerAsStr(mi.socket())));

    if (m_owner->m_initializationStatus != 2)
    {
        HLOG_DBG(QObject::tr(
            "The control point is not ready to accept notifications. Ignoring."));

        return;
    }

    QString serviceCallbackId = req.callback().path().remove('/');

    QMutexLocker lock(&m_owner->m_serviceSubscribtionsMutex);

    QSharedPointer<HServiceSubscribtion> subscription =
        m_owner->m_serviceSubscribtions.value(serviceCallbackId);

    lock.unlock();

    if (!subscription)
    {
        HLOG_WARN(QObject::tr(
            "Ignoring notification due to invalid callback ID [%1]").arg(
                serviceCallbackId));

        mi.setKeepAlive(false);
        m_httpHandler.send(mi, BadRequest);
        return;
    }

    subscription->onNotify(mi, req);
}

/*******************************************************************************
 * HControlPointSsdpHandler
 ******************************************************************************/
HControlPointSsdpHandler::HControlPointSsdpHandler(
    HControlPointPrivate* owner) :
        HSsdp(owner), m_owner(owner)
{
    Q_ASSERT(m_owner);
    h_ptr->m_loggingIdentifier = m_owner->m_loggingIdentifier;
}

HControlPointSsdpHandler::~HControlPointSsdpHandler()
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
}

bool HControlPointSsdpHandler::incomingDiscoveryResponse(
    const HDiscoveryResponse& msg, const HEndpoint& source)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    return m_owner->processDeviceDiscovery(msg, source);
}

bool HControlPointSsdpHandler::incomingDeviceAvailableAnnouncement(
    const HResourceAvailable& msg)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    return m_owner->processDeviceDiscovery(msg);
}

bool HControlPointSsdpHandler::incomingDeviceUnavailableAnnouncement(
    const HResourceUnavailable& msg)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    QMutexLocker lock(&m_owner->m_deviceStorage->m_rootDevicesMutex);

    HDeviceController* device =
        m_owner->m_deviceStorage->searchDeviceByUdn(msg.usn().udn());

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

    m_owner->removeRootDeviceAndSubscriptions(root, false);

    return true;
}

/*******************************************************************************
 * HControlPointPrivate
 ******************************************************************************/
HControlPointPrivate::HControlPointPrivate() :
    HAbstractHostPrivate(
        QString("__CONTROL POINT %1__: ").arg(QUuid::createUuid().toString())),
            m_deviceBuildTasks(),
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
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
}

HActionInvoke HControlPointPrivate::createActionInvoker(HAction* action)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

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
    creatorParams.m_threadPool = m_threadPool;
    creatorParams.m_loggingIdentifier = m_loggingIdentifier;

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
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QMutexLocker lock(&m_deviceStorage->m_rootDevicesMutex);

    // according to the UDA v1.1 a "device tree" (root, embedded and services)
    // are "timed out" only when every advertisement has timed out.

    source = source->rootDevice();

    if (source->isTimedout(HDeviceController::All))
    {
        removeRootDeviceAndSubscriptions(source, false);
    }
}

void HControlPointPrivate::removeRootDeviceSubscriptions(
    HDeviceController* rootDevice, bool unsubscribe)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(rootDevice);
    Q_ASSERT(!rootDevice->m_device->parentDevice());
    // this method should be called only with root devices

    Q_ASSERT(thread() == QThread::currentThread());

    // when removing a root device all of the subscriptions for services contained
    // within the root device have to be removed as well.

    QMutexLocker lock(&m_serviceSubscribtionsMutex);

    QHash<QUuid, QSharedPointer<HServiceSubscribtion> >::iterator ci =
        m_serviceSubscribtions.begin();

    while (ci != m_serviceSubscribtions.end())
    {
        QSharedPointer<HServiceSubscribtion> subscription = (*ci);

        // seek the root device of the device tree to which the service
        // that contains the subscription belongs.
        const HDevice* device = subscription->service()->parentDevice();
        while(device->parentDevice()) { device = device->parentDevice(); }

        if (device == rootDevice->m_device.data())
        {
            // the service appears to belong to the device tree that is about
            // to be removed

            ci = m_serviceSubscribtions.erase(ci);

            if (unsubscribe)
            {
                subscription->unsubscribe(true);
            }
        }
        else
        {
            ++ci;
        }
    }
}

void HControlPointPrivate::removeRootDeviceAndSubscriptions(
    HDeviceController* rootDevice, bool unsubscribe)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(thread() == QThread::currentThread());

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
        return true;
    }

    // it does not matter if the device is an embedded device, since the
    // location of the device always points to the root device's description
    // and the internal device model is built of that. Hence, it is only necessary
    // to get an advertisement of a root or an embedded device to build the entire
    // model correctly.

    DeviceBuildTask* dbp = m_deviceBuildTasks.get(msg);
    if (dbp)
    {
        if (!dbp->m_locations.contains(msg.location()))
        {
            dbp->m_locations.push_back(msg.location());
        }

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

    HLOG_INFO(QObject::tr(
        "New resource [%1] is available @ [%2]. "
        "Attempting to build the device model.").arg(
            msg.usn().resource().toString(), msg.location().toString()));

    m_threadPool->start(newBuildTask);

    return true;
}

void HControlPointPrivate::deviceModelBuildDone(const Herqq::Upnp::HUdn& udn)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(thread() == QThread::currentThread());

    DeviceBuildTask* build = m_deviceBuildTasks.get(udn);
    Q_ASSERT(build);

    if (build->completionValue() == 0)
    {
        HLOG_INFO(QObject::tr("Device model for [%1] built successfully.").arg(
            udn.toString()));

        HDeviceController* device = build->createdDevice();
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

    QMutexLocker lock(&m_serviceSubscribtionsMutex);

    QHash<QUuid, QSharedPointer<HServiceSubscribtion> >::iterator it =
        m_serviceSubscribtions.begin();

    for(; it != m_serviceSubscribtions.end(); ++it)
    {
        try
        {
            (*it)->unsubscribe(true);
        }
        catch(HException&)
        {
            // intentional. at most could print something.
        }
    }
    m_serviceSubscribtions.clear();

    lock.unlock();

    while(m_threadPool->activeThreadCount())
    {
        QAbstractEventDispatcher::instance()->processEvents(
            QEventLoop::ExcludeUserInputEvents);
    }

    m_threadPool->waitForDone();
    // ensure that no threads created by this thread pool are running when we
    // start deleting shared objects.

    delete m_server; m_server = 0;
    delete m_ssdp; m_ssdp = 0;

    m_http.reset(0);

    m_initializationStatus = 0;

    QAbstractEventDispatcher::instance()->processEvents(
        QEventLoop::ExcludeUserInputEvents);
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
        HAbstractHost(*new HControlPointPrivate(), parent)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    H_D(HControlPoint);

    h->m_initParams.reset(initParams ?
        initParams->clone() : new HControlPointConfiguration());
}

HControlPoint::HControlPoint(
    HControlPointPrivate& dd,
    const HControlPointConfiguration* initParams, QObject* parent) :
        HAbstractHost(dd, parent)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    H_D(HControlPoint);

    h->m_initParams.reset(initParams ?
        initParams->clone() : new HControlPointConfiguration());
}

HControlPoint::~HControlPoint()
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    quit();
}

qint32 HControlPoint::doInit()
{
    return Success();
}

qint32 HControlPoint::init(QString* errorString)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    H_D(HControlPoint);

    Q_ASSERT_X(
        thread() == QThread::currentThread(), H_AT,
        "The control point has to be initialized in the thread in which it is currently located.");

    if (h->state() == HAbstractHostPrivate::Initialized)
    {
        return AlreadyInitialized();
    }

    Q_ASSERT(h->state() == HAbstractHostPrivate::Uninitialized);

    QString error;
    qint32 rc = Success();
    try
    {
        h->setState(HAbstractHostPrivate::Initializing);

        HLOG_INFO(QObject::tr("ControlPoint initializing."));

        h->m_http.reset(new HHttpHandler(h->m_loggingIdentifier));

        h->m_server = new ControlPointHttpServer(h);
        if (!h->m_server->listen())
        {
            rc = UndefinedFailure();
        }
        else
        {
            h->m_ssdp = new HControlPointSsdpHandler(h);

            if (!h->m_ssdp->bind())
            {
                throw HSocketException(QObject::tr(
                    "Failed to initialize SSDP."));
            }

            HLOG_DBG(QObject::tr("Searching for UPnP devices..."));

            h->m_ssdp->sendDiscoveryRequest(
                HDiscoveryRequest(
                    1, HResourceIdentifier("ssdp:all"), herqqProductTokens()));

            h->setState(HAbstractHostPrivate::Initialized);
        }
    }
    catch(HSocketException& ex)
    {
        error = ex.reason();
        rc = CommunicationsError();
    }
    catch(HException& ex)
    {
        error = ex.reason();
        rc = UndefinedFailure();
    }

    if (rc != Success())
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

}
}
