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

#include "devicehost.h"
#include "devicehost_p.h"
#include "devicehost_dataretriever_p.h"

#include "../exceptions_p.h"
#include "../objectcreator_p.h"

#include "../devicemodel/device_p.h"
#include "../devicemodel/service_p.h"

#include "../../../../utils/src/logger_p.h"
#include "../../../../utils/src/sysutils_p.h"

#include "../../../../core/include/HExceptions"

#include <QMetaType>
#include <QDomDocument>

#include <ctime>

static bool registerMetaTypes()
{
    static QAtomicInt tester(0);

    if (tester.testAndSetAcquire(0, 1))
    {
        qRegisterMetaType<Herqq::Upnp::HDeviceHostConfiguration>(
            "Herqq::Upnp::HDeviceHostConfiguration");
    }

    return true;
}

static bool test = registerMetaTypes();

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
            m_presenceAnnouncer  (0)
{
    HLOG(H_AT, H_FUN);

    srand(time(0));
}

HDeviceHostPrivate::~HDeviceHostPrivate()
{
    HLOG(H_AT, H_FUN);
}

void HDeviceHostPrivate::announcementTimedout(HDeviceController* rootDevice)
{
    HLOG(H_AT, H_FUN);

    QList<ResourceAvailableAnnouncement> announcements;

    m_presenceAnnouncer->createAnnouncementMessagesForRootDevice(
        rootDevice, announcements);

    m_presenceAnnouncer->sendAnnouncements(announcements);

    rootDevice->startStatusNotifier(HDeviceController::ThisOnly);
}

void HDeviceHostPrivate::createRootDevices()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QList<HDeviceConfiguration*> diParams = m_initParams.deviceConfigurations();

    foreach(HDeviceConfiguration* deviceInitParams, diParams)
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
        // device presence well before the advertises cache-control value
        // expires.

        creatorParams.m_appendUdnToDeviceLocation = true;
        creatorParams.m_sharedActionInvokers = &m_sharedActionInvokers;

        creatorParams.m_iconFetcher =
            IconFetcher(
                &dataRetriever, &DeviceHostDataRetriever::retrieveIcon);

        creatorParams.m_strictParsing = true;
        creatorParams.m_stateVariablesAreImmutable = false;

        HObjectCreator creator(creatorParams);
        HDeviceController* rootDevice = creator.createRootDevice();

        Q_ASSERT(rootDevice);
        addRootDevice(rootDevice);

        rootDevice->setParent(this);
        rootDevice->m_device->setParent(this);
        connectSelfToServiceSignals(rootDevice->m_device.data());
    }
}

void HDeviceHostPrivate::connectSelfToServiceSignals(HDevice* device)
{
    HLOG(H_AT, H_FUN);

    QList<HService*> services = device->services();
    foreach(HService* service, services)
    {
        bool ok = QObject::connect(
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
    HLOG(H_AT, H_FUN);

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
    HLOG(H_AT, H_FUN);

    QList<HDeviceController*> rootDevices =
        m_deviceStorage->rootDeviceControllers();

    foreach(HDeviceController* rootDevice, rootDevices)
    {
        rootDevice->stopStatusNotifier(HDeviceController::ThisOnly);
    }
}

void HDeviceHostPrivate::doClear()
{
    HLOG(H_AT, H_FUN);

    Q_ASSERT(thread() == QThread::currentThread());

    // called by the abstract host just before it starts to delete the device
    // tree.

    Q_ASSERT(state() == Exiting);
    // this path should be traversed only when the device host has initiated
    // shut down.

    m_httpServer->close(false);
    m_http->shutdown(false);

    m_presenceAnnouncer.reset(0);
    m_ssdp.reset(0);

    m_eventNotifier->shutdown();

    while(m_httpServer->activeClientCount() != 0 ||
          m_threadPool->activeThreadCount() != 0)
    {
        // as long as there are requests being processed, we cannot go
        // deleting objects that may be needed by the request processing. ==>
        // wait for the requests to complete

        QAbstractEventDispatcher::instance()->processEvents(
            QEventLoop::ExcludeUserInputEvents);

        HSysUtils::msleep(1);
    }

    m_threadPool->waitForDone();

    m_http.reset(0);
    m_httpServer.reset(0);
    m_eventNotifier.reset(0);
    m_initParams = HDeviceHostConfiguration();
    m_activeRequestCount = 0;

    setState(Uninitialized);
}

/*******************************************************************************
 * HDeviceHost
 *******************************************************************************/
HDeviceHost::HDeviceHost(QObject* parent) :
    HAbstractHost(*new HDeviceHostPrivate(), parent)
{
    HLOG(H_AT, H_FUN);
}

HDeviceHost::~HDeviceHost()
{
    HLOG(H_AT, H_FUN);
    quit();
}

HDeviceHost::ReturnCode HDeviceHost::init(
    const HDeviceHostConfiguration& initParams, QString* errorString)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    H_D(HDeviceHost);

    Q_ASSERT_X(
        thread() == QThread::currentThread(), H_AT,
        "The device host has to be initialized in the thread in which it is currently located.");

    if (h->state() == HAbstractHostPrivate::Initialized)
    {
        return AlreadyInitialized;
    }

    Q_ASSERT(h->state() == HAbstractHostPrivate::Uninitialized);

    if (initParams.isEmpty())
    {
        if (errorString)
        {
            *errorString = QObject::tr("No UPnP device configuration provided.");
        }

        return InvalidConfiguration;
    }

    QString error;
    HDeviceHost::ReturnCode rc = Success;
    try
    {
        h->setState(HAbstractHostPrivate::Initializing);

        HLOG_INFO(QObject::tr("DeviceHost Initializing."));

        h->m_initParams = initParams;

        h->m_http.reset(new HHttpHandler());

        h->m_eventNotifier.reset(
            new EventNotifier(
                h->m_loggingIdentifier, *h->m_http, *h->m_threadPool, this));

        h->m_httpServer.reset(
            new DeviceHostHttpServer(
                h->m_loggingIdentifier, *h->m_deviceStorage,
                *h->m_eventNotifier, this));

        if (!h->m_httpServer->listen())
        {
            QString err = QObject::tr("Could not start the HTTP server.");

            if (errorString)
            {
                *errorString = err;
            }

            HLOG_WARN(QObject::tr("DeviceHost initialization failed: [%1]").arg(err));
            rc = UndefinedFailure;
        }
        else
        {
            h->createRootDevices();

            h->m_ssdp.reset(
                new DeviceHostSsdpHandler(
                    h->m_loggingIdentifier, *h->m_deviceStorage, this));

            h->m_presenceAnnouncer.reset(
                new PresenceAnnouncer(
                    h->m_ssdp.data(), h->m_initParams.individualAdvertisementCount()));

            h->m_presenceAnnouncer->announce<ResourceAvailableAnnouncement>(
                h->m_deviceStorage->rootDeviceControllers());

            h->startNotifiers();

            h->setState(HAbstractHostPrivate::Initialized);
        }
    }
    catch(Herqq::Upnp::InvalidDeviceDescription& ex)
    {
        error = ex.reason();
        rc = InvalidDeviceDescription;
    }
    catch(Herqq::Upnp::InvalidServiceDescription& ex)
    {
        error = ex.reason();
        rc = InvalidServiceDescription;
    }
    catch(HException& ex)
    {
        error = ex.reason();
        rc = UndefinedFailure;
    }

    if (rc != Success)
    {
        HLOG_WARN(QObject::tr("DeviceHost initialization failed: [%1]").arg(error));

        h->setState(HAbstractHostPrivate::Exiting);
        h->clear();

        if (errorString)
        {
            *errorString = error;
        }

        return rc;
    }

    HLOG_INFO(QObject::tr("DeviceHost initialized."));

    return rc;
}

HDeviceHost::ReturnCode HDeviceHost::quit(QString* errorString)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    H_D(HDeviceHost);

    Q_ASSERT_X(
        thread() == QThread::currentThread(), H_AT,
        "The device host has to be shutdown in the thread in which it is currently located.");

    if (h->state() == HAbstractHostPrivate::Uninitialized)
    {
        return Success;
    }

    Q_ASSERT(h->state() == HAbstractHostPrivate::Initialized);

    HLOG_INFO(QObject::tr("DeviceHost shutting down."));

    h->setState(HAbstractHostPrivate::Exiting);

    ReturnCode retVal = Success;
    try
    {
        h->stopNotifiers();

        h->m_presenceAnnouncer->announce<ResourceUnavailableAnnouncement>(
            h->m_deviceStorage->rootDeviceControllers());
    }
    catch (HException& ex)
    {
        HLOG_WARN(ex.reason());
        retVal = UndefinedFailure;

        if (errorString)
        {
            *errorString = ex.reason();
        }
    }

    h->clear();

    HLOG_INFO(QObject::tr("DeviceHost shut down."));

    return retVal;
}

}
}
