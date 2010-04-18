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

#include "hdevicehost.h"
#include "hdevicehost_p.h"
#include "hevent_notifier_p.h"
#include "hpresence_announcer_p.h"
#include "hdevicehost_configuration.h"
#include "hdevicehost_http_server_p.h"
#include "hdevicehost_ssdp_handler_p.h"
#include "hdevicehost_dataretriever_p.h"

#include "./../hobjectcreator_p.h"
#include "./../hdevicehosting_exceptions_p.h"

#include "./../../devicemodel/hdevice_p.h"
#include "./../../devicemodel/hservice_p.h"

#include "./../../../utils/hlogger_p.h"
#include "./../../../utils/hsysutils_p.h"

#include <QDomDocument>
#include <QAbstractEventDispatcher>

#include <ctime>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HDeviceHostPrivate
 ******************************************************************************/
HDeviceHostPrivate::HDeviceHostPrivate() :
    HAbstractHostPrivate(
        QString("__DEVICE HOST %1__: ").arg(QUuid::createUuid().toString())),
            m_initParams         (),
            m_ssdp               (0),
            m_httpServer         (0),
            m_activeRequestCount (0),
            m_eventNotifier      (0),
            m_presenceAnnouncer  (0),
            q_ptr(0),
            m_lastError(HDeviceHost::UndefinedError)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    srand(time(0));
}

HDeviceHostPrivate::~HDeviceHostPrivate()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
}

void HDeviceHostPrivate::announcementTimedout(HDeviceController* rootDevice)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QList<ResourceAvailableAnnouncement> announcements;

    m_presenceAnnouncer->createAnnouncementMessagesForRootDevice(
        rootDevice, announcements);

    m_presenceAnnouncer->sendAnnouncements(announcements);

    rootDevice->startStatusNotifier(HDeviceController::ThisOnly);
}

void HDeviceHostPrivate::createRootDevices()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QList<const HDeviceConfiguration*> diParams =
        m_initParams->deviceConfigurations();

    foreach(const HDeviceConfiguration* deviceInitParams, diParams)
    {
        QString baseDir =
            extractBaseUrl(deviceInitParams->pathToDeviceDescription());

        DeviceHostDataRetriever dataRetriever(m_loggingIdentifier, baseDir);

        QDomDocument dd = dataRetriever.retrieveDeviceDescription(
            deviceInitParams->pathToDeviceDescription());

        QList<QUrl> locations;
        locations.push_back(m_httpServer->rootUrl());
        // TODO, modify ^^^ when the server component supports multi-homed devices.

        HObjectCreationParameters creatorParams;
        creatorParams.m_createDefaultObjects = false;
        creatorParams.m_deviceDescription    = dd;
        creatorParams.m_deviceCreator        = deviceInitParams->deviceCreator();
        creatorParams.m_deviceLocations      = locations;

        creatorParams.m_serviceDescriptionFetcher =
            ServiceDescriptionFetcher(
                &dataRetriever, &DeviceHostDataRetriever::retrieveServiceDescription);

        creatorParams.m_deviceTimeoutInSecs =
            deviceInitParams->cacheControlMaxAge() / 2;
        // this timeout value instructs the device host to re-announce the
        // device presence well before the advertised cache-control value
        // expires.

        creatorParams.m_appendUdnToDeviceLocation = true;
        creatorParams.m_sharedActionInvokers = &m_sharedActionInvokers;

        creatorParams.m_iconFetcher =
            IconFetcher(
                &dataRetriever, &DeviceHostDataRetriever::retrieveIcon);

        creatorParams.m_strictParsing = true;
        creatorParams.m_stateVariablesAreImmutable = false;
        creatorParams.m_threadPool = m_threadPool;
        creatorParams.m_loggingIdentifier = m_loggingIdentifier;

        HObjectCreator creator(creatorParams);
        HDeviceController* rootDevice = creator.createRootDevice();

        Q_ASSERT(rootDevice);
        m_deviceStorage->addRootDevice(rootDevice);

        rootDevice->setParent(this);
        rootDevice->m_device->setParent(this);
        connectSelfToServiceSignals(rootDevice->m_device);
    }
}

void HDeviceHostPrivate::connectSelfToServiceSignals(HDevice* device)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QList<HService*> services = device->services();
    foreach(HService* service, services)
    {
        bool ok = connect(
            service,
            SIGNAL(stateChanged(const Herqq::Upnp::HService*)),
            m_eventNotifier.data(),
            SLOT(stateChanged(const Herqq::Upnp::HService*)));

        Q_ASSERT(ok); Q_UNUSED(ok)
    }

    QList<HDevice*> devices = device->embeddedDevices();
    foreach(HDevice* embeddedDevice, devices)
    {
        connectSelfToServiceSignals(embeddedDevice);
    }
}

void HDeviceHostPrivate::startNotifiers()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QList<HDeviceController*> rootDevices =
        m_deviceStorage->rootDeviceControllers();

    foreach(HDeviceController* rootDevice, rootDevices)
    {
        bool ok = connect(
            rootDevice, SIGNAL(statusTimeout(HDeviceController*)),
            this, SLOT(announcementTimedout(HDeviceController*)));

        Q_ASSERT(ok); Q_UNUSED(ok)

        rootDevice->startStatusNotifier(HDeviceController::ThisOnly);
    }
}

void HDeviceHostPrivate::stopNotifiers()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QList<HDeviceController*> rootDevices =
        m_deviceStorage->rootDeviceControllers();

    foreach(HDeviceController* rootDevice, rootDevices)
    {
        rootDevice->stopStatusNotifier(HDeviceController::ThisOnly);
    }
}

void HDeviceHostPrivate::doClear()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(thread() == QThread::currentThread());

    // called by the abstract host just before it starts to delete the device
    // tree.

    Q_ASSERT(state() == Exiting);
    // this path should be traversed only when the device host has initiated
    // shut down.

    m_httpServer->close(false);
    m_http->shutdown();

    // At this point SSDP and HTTP are closed and no further requests can come in.
    // However, no objects have been deleted and the derived class can safely access
    // them if necessary.
    q_ptr->doQuit();

    m_presenceAnnouncer.reset(0);
    m_ssdp.reset(0);

    m_eventNotifier->shutdown();

    while(m_httpServer->activeClientCount() != 0 ||
          m_threadPool->activeThreadCount() > 0)
    {
        // as long as there are requests being processed, we cannot go
        // deleting objects that may be needed by the request processing. ==>
        // wait for the requests to complete

        QAbstractEventDispatcher::instance()->processEvents(
            QEventLoop::ExcludeUserInputEvents);
    }

    m_threadPool->waitForDone();

    m_http.reset(0);
    m_httpServer.reset(0);
    m_eventNotifier.reset(0);
    m_initParams.reset(0);
    m_activeRequestCount = 0;

    setState(Uninitialized);
}

/*******************************************************************************
 * HDeviceHost
 *******************************************************************************/
HDeviceHost::HDeviceHost(QObject* parent) :
    QObject(parent), h_ptr(new HDeviceHostPrivate())
{
    h_ptr->setParent(this);
    h_ptr->q_ptr = this;
}

HDeviceHost::~HDeviceHost()
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    quit();
    delete h_ptr;
}

bool HDeviceHost::doInit()
{
    // default implementation does nothing
    return true;
}

void HDeviceHost::doQuit()
{
    // default implementation does nothing
}

bool HDeviceHost::acceptSubscription(
    HService* /*targetService*/, const HEndpoint& /*source*/, bool /*renewal*/)
{
    return true;
}

const HDeviceHostConfiguration* HDeviceHost::configuration() const
{
    return h_ptr->m_initParams.data();
}

void HDeviceHost::setError(DeviceHostError error, const QString& errorStr)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    h_ptr->m_lastError = error;
    h_ptr->m_lastErrorDescription = errorStr;
}

bool HDeviceHost::init(const HDeviceHostConfiguration& initParams)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    Q_ASSERT_X(
        thread() == QThread::currentThread(), H_AT,
        "The device host has to be initialized in the thread in which "
        "it is currently located.");

    if (h_ptr->state() == HAbstractHostPrivate::Initialized)
    {
        setError(AlreadyInitializedError,
            tr("The device host is already initialized"));

        return false;
    }

    Q_ASSERT(h_ptr->state() == HAbstractHostPrivate::Uninitialized);

    if (initParams.isEmpty())
    {
        setError(InvalidConfigurationError,
            tr("No UPnP device configuration provided"));

        return false;
    }

    bool ok = false;
    try
    {
        h_ptr->setState(HAbstractHostPrivate::Initializing);

        HLOG_INFO("DeviceHost Initializing.");

        h_ptr->m_initParams.reset(initParams.clone());

        h_ptr->m_http.reset(new HHttpHandler(h_ptr->m_loggingIdentifier));

        h_ptr->m_eventNotifier.reset(
            new EventNotifier(
                h_ptr->m_loggingIdentifier,
                *h_ptr->m_http,
                *h_ptr->m_initParams,
                this));

        h_ptr->m_httpServer.reset(
            new DeviceHostHttpServer(
                h_ptr->m_loggingIdentifier, *h_ptr->m_deviceStorage,
                *h_ptr->m_eventNotifier, this));

        if (!h_ptr->m_httpServer->listen())
        {
            QString err = QString("Failed to initialize HTTP server");
            setError(CommunicationsError, err);
        }
        else
        {
            h_ptr->createRootDevices();

            h_ptr->m_ssdp.reset(
                new DeviceHostSsdpHandler(
                    h_ptr->m_loggingIdentifier, *h_ptr->m_deviceStorage, this));

            if (!h_ptr->m_ssdp->bind())
            {
                throw HSocketException(tr("Failed to initialize SSDP"));
            }

            h_ptr->m_presenceAnnouncer.reset(
                new PresenceAnnouncer(
                    h_ptr->m_ssdp.data(),
                    h_ptr->m_initParams->individualAdvertisementCount()));

            // allow the derived classes to perform their initialization routines
            // before the hosted devices are announced to the network and timers
            // are started. In addition, at this time no HTTP or SSDP requests
            // are served.

            ok = doInit();
            // continue only if the derived class succeeded in initializing itself

            if (ok)
            {
                h_ptr->m_presenceAnnouncer->announce<ResourceAvailableAnnouncement>(
                    h_ptr->m_deviceStorage->rootDeviceControllers());

                h_ptr->startNotifiers();

                h_ptr->setState(HAbstractHostPrivate::Initialized);
            }
        }
    }
    catch(Herqq::Upnp::InvalidDeviceDescription& ex)
    {
        setError(InvalidDeviceDescriptionError, ex.reason());
    }
    catch(Herqq::Upnp::InvalidServiceDescription& ex)
    {
        setError(InvalidServiceDescriptionError, ex.reason());
    }
    catch(HSocketException& ex)
    {
        setError(CommunicationsError, ex.reason());
    }
    catch(HException& ex)
    {
        setError(UndefinedError, ex.reason());
    }

    if (!ok)
    {
        HLOG_WARN(tr("DeviceHost initialization failed"));

        h_ptr->setState(HAbstractHostPrivate::Exiting);
        h_ptr->clear();

        return false;
    }

    HLOG_INFO("DeviceHost initialized.");

    return true;
}

HDeviceHost::DeviceHostError HDeviceHost::error() const
{
    return h_ptr->m_lastError;
}

QString HDeviceHost::errorDescription() const
{
    return h_ptr->m_lastErrorDescription;
}

void HDeviceHost::quit()
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    Q_ASSERT_X(
        thread() == QThread::currentThread(), H_AT,
        "The device host has to be shutdown in the thread in which it is "
        "currently located.");

    if (h_ptr->state() == HAbstractHostPrivate::Uninitialized)
    {
        return;
    }

    Q_ASSERT(h_ptr->state() == HAbstractHostPrivate::Initialized);

    HLOG_INFO("Shutting down.");

    h_ptr->setState(HAbstractHostPrivate::Exiting);

    try
    {
        h_ptr->stopNotifiers();

        h_ptr->m_presenceAnnouncer->announce<ResourceUnavailableAnnouncement>(
            h_ptr->m_deviceStorage->rootDeviceControllers());
    }
    catch (HException& ex)
    {
        HLOG_WARN(ex.reason());
    }

    h_ptr->clear();

    HLOG_INFO("Shut down.");
}

bool HDeviceHost::isStarted() const
{
    return h_ptr->state() == HAbstractHostPrivate::Initialized;
}

HDeviceList HDeviceHost::rootDevices() const
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (!isStarted())
    {
        HLOG_WARN("The device host is not started");
        return HDeviceList();
    }

    return h_ptr->m_deviceStorage->rootDevices();
}

HDevice* HDeviceHost::rootDevice(const HUdn& udn) const
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (!isStarted())
    {
        HLOG_WARN("The device host is not started");
        return 0;
    }

    HDeviceController* dc = h_ptr->m_deviceStorage->searchDeviceByUdn(udn);

    return dc ?
        h_ptr->m_deviceStorage->searchDeviceByUdn(udn)->m_device : 0;
}

}
}
