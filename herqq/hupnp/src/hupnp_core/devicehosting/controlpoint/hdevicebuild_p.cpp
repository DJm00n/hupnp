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

#include "hdevicebuild_p.h"
#include "hevent_subscription_p.h"
#include "hcontrolpoint_p.h"

#include "./../../devicemodel/hdevice.h"
#include "./../../devicemodel/hservice_p.h"
#include "./../../../utils/hlogger_p.h"

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * DeviceBuildTask
 ******************************************************************************/
void DeviceBuildTask::createEventSubscriptions(
    HDeviceController* device,
    QList<QSharedPointer<HServiceSubscribtion> >* subscriptions)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
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

            subscriptions->push_back(
                QSharedPointer<HServiceSubscribtion>(
                    subscription, &QObject::deleteLater));
        }
    }

    QList<HDeviceController*> devices = device->embeddedDevices();
    foreach(HDeviceController* embDevice, devices)
    {
        createEventSubscriptions(embDevice, subscriptions);
    }
}

DeviceBuildTask::~DeviceBuildTask()
{
   HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);

    if (m_createdDevice.data())
    {
        m_createdDevice->deleteLater();
    }

    m_createdDevice.take();
}

HDeviceController* DeviceBuildTask::createdDevice()
{
    return m_createdDevice.take();
}

void DeviceBuildTask::deleteSubscriptions(
    QList<QSharedPointer<HServiceSubscribtion> >* subscriptions)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);

    if (subscriptions->isEmpty())
    {
        return;
    }

    QMutexLocker lock(&m_owner->m_serviceSubscribtionsMutex);

    QList<QSharedPointer<HServiceSubscribtion> >::iterator it =
        subscriptions->begin();

    while(it != subscriptions->end())
    {
        QSharedPointer<HServiceSubscribtion> sub = (*it);

        if (m_owner->m_serviceSubscribtions.contains(sub->id()))
        {
            m_owner->m_serviceSubscribtions.remove(sub->id());
        }

        it = subscriptions->erase(it);
    }

    Q_ASSERT(subscriptions->isEmpty());
}

void DeviceBuildTask::run()
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);

    QList<QSharedPointer<HServiceSubscribtion> > subscriptions;
    try
    {
        QScopedPointer<HDeviceController> device(
            m_owner->buildDevice(m_location, m_cacheControlMaxAge));

        // the returned device is a fully built root device containing every
        // embedded device and service advertised in the device and service descriptions
        // otherwise, the creation failed and an exception was thrown

        createEventSubscriptions(device.data(), &subscriptions);

        QMutexLocker lock(&m_owner->m_serviceSubscribtionsMutex);

        if (m_owner->state() != HControlPointPrivate::Initialized)
        {
            subscriptions.clear();

            m_completionValue = -1;
            m_errorString = QObject::tr("Shutting down. Aborting device model build.");
            emit done(m_udn);

            return;
        }

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
            deleteSubscriptions(&subscriptions);

            m_completionValue = -1;

            m_errorString =
                QObject::tr("Shutting down. Aborting device model build.");

            emit done(m_udn);
        }
        else
        {
            device->moveToThread(m_owner->thread());
            device->m_device->moveToThread(m_owner->thread());

            m_completionValue = 0;
            m_createdDevice.swap(device);

            emit done(m_udn);
        }
    }
    catch(HException& ex)
    {
        HLOG_WARN(QObject::tr("Couldn't create a device: %1").arg(ex.reason()));

        deleteSubscriptions(&subscriptions);

        m_completionValue = -1;
        m_errorString = ex.reason();

        emit done(m_udn);
    }
}

/*******************************************************************************
 * DeviceBuildTasks
 ******************************************************************************/
DeviceBuildTasks::DeviceBuildTasks() :
    m_builds()
{
}

DeviceBuildTasks::~DeviceBuildTasks()
{
    qDeleteAll(m_builds);
}

DeviceBuildTask* DeviceBuildTasks::get(const HUdn& udn) const
{
    QList<DeviceBuildTask*>::const_iterator ci = m_builds.constBegin();

    for(; ci != m_builds.constEnd(); ++ci)
    {
        if ((*ci)->udn() == udn)
        {
            return *ci;
        }
    }

    return 0;
}

void DeviceBuildTasks::remove(const HUdn& udn)
{
    QList<DeviceBuildTask*>::iterator i = m_builds.begin();

    for(; i != m_builds.end(); ++i)
    {
        if ((*i)->udn() == udn)
        {
            delete (*i);
            m_builds.erase(i);
            return;
        }
    }

    Q_ASSERT(false);
}

void DeviceBuildTasks::add(DeviceBuildTask* arg)
{
    Q_ASSERT(arg);
    m_builds.push_back(arg);
}

QList<DeviceBuildTask*> DeviceBuildTasks::values() const
{
    return m_builds;
}

}
}