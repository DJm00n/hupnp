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

#include "hevent_subscriptionmanager_p.h"
#include "hcontrolpoint_p.h"
#include "hcontrolpoint_configuration.h"

#include "./../../devicemodel/hservice_p.h"
#include "./../../dataelements/hdeviceinfo.h"

#include "./../../../utils/hlogger_p.h"

namespace Herqq
{

namespace Upnp
{

HEventSubscriptionManager::HEventSubscriptionManager(HControlPointPrivate* owner) :
    QObject(owner),
        m_owner(owner), m_subscribtionsByUuid(), m_subscriptionsByUdn(),
        m_subscribtionsMutex(QMutex::Recursive)
{
    Q_ASSERT(m_owner);
}

HEventSubscriptionManager::~HEventSubscriptionManager()
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
    removeAll();
}

void HEventSubscriptionManager::subscribed_slot(HEventSubscription* sub)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
    Q_ASSERT(sub);

    emit subscribed(sub->service()->m_service);
}

void HEventSubscriptionManager::subscriptionFailed_slot(HEventSubscription* sub)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
    Q_ASSERT(sub);

    HService* service = sub->service()->m_service;
    sub->resetSubscription();
    emit subscriptionFailed(service);
}

void HEventSubscriptionManager::unsubscribed(HEventSubscription* sub)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
    Q_ASSERT(sub);

    emit unsubscribed(sub->service()->m_service);
}

HEventSubscription* HEventSubscriptionManager::createSubscription(
    HServiceController* service, qint32 timeout)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
    Q_ASSERT(service);
    Q_ASSERT(thread() == QThread::currentThread());

    HEventSubscription* subscription =
        new HEventSubscription(
            m_owner->m_loggingIdentifier,
            service,
            m_owner->m_server->rootUrl(),
            HTimeout(timeout),
            this);

    bool ok = connect(
        subscription,
        SIGNAL(subscribed(HEventSubscription*)),
        this,
        SLOT(subscribed_slot(HEventSubscription*)));

    Q_ASSERT(ok); Q_UNUSED(ok)

    ok = connect(
        subscription,
        SIGNAL(subscriptionFailed(HEventSubscription*)),
        this,
        SLOT(subscriptionFailed_slot(HEventSubscription*)));

    Q_ASSERT(ok);

    ok = connect(
        subscription, SIGNAL(unsubscribed(HEventSubscription*)),
        this, SLOT(unsubscribed(HEventSubscription*)));

    Q_ASSERT(ok);

    return subscription;
}

bool HEventSubscriptionManager::subscribe(
    HDevice* device, DeviceVisitType visitType, qint32 timeout)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
    Q_ASSERT(device);

    bool ok = false;
    HServiceList services = device->services();
    foreach(HService* service, services)
    {
        if (service->isEvented())
        {
            if (subscribe(service, timeout) == Sub_Success)
            {
                ok = true;
            }
        }
    }

    if (visitType == VisitThisAndDirectChildren ||
        visitType == VisitThisRecursively)
    {
        HDeviceList devices = device->embeddedDevices();
        foreach(HDevice* embDevice, devices)
        {
            DeviceVisitType visitTypeForChildren =
                visitType == VisitThisRecursively ?
                    VisitThisRecursively : VisitThisOnly;

            if (subscribe(embDevice, visitTypeForChildren, timeout) && !ok)
            {
                ok = true;
            }
        }
    }

    return ok;
}

HEventSubscriptionManager::SubscriptionResult
    HEventSubscriptionManager::subscribe(HService* service, qint32 timeout)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
    Q_ASSERT(service);

    if (!service->isEvented())
    {
        HLOG_WARN(QString(
            "Cannot subscribe to a service [%1] that is not evented").arg(
                service->serviceId().toString()));

        return Sub_Failed_NotEvented;
    }

    QMutexLocker locker(&m_subscribtionsMutex);

    HUdn deviceUdn = service->parentDevice()->deviceInfo().udn();

    QList<HEventSubscription*>* subs = m_subscriptionsByUdn.value(deviceUdn);

    if (!subs)
    {
        subs = new QList<HEventSubscription*>();
        goto end;
    }
    else
    {
        QList<HEventSubscription*>::iterator it = subs->begin();
        for(; it != subs->end(); ++it)
        {
            HEventSubscription* sub = (*it);
            if (sub->service()->m_service == service)
            {
                if (sub->subscriptionStatus() == HEventSubscription::Status_Subscribed)
                {
                    HLOG_WARN(QString("Subscription to service [%1] exists").arg(
                        service->serviceId().toString()));
                    return Sub_AlreadySubscribed;
                }
                else
                {
                    locker.unlock();
                    sub->subscribe();
                    return Sub_Success;
                }
            }
        }
    }

end:

    HServiceController* sc = static_cast<HServiceController*>(service->parent());

    HEventSubscription* sub = createSubscription(sc, timeout);
    m_subscribtionsByUuid.insert(sub->id(), sub);
    m_subscriptionsByUdn.insert(deviceUdn, subs);
    subs->append(sub);

    locker.unlock();

    sub->subscribe();

    return Sub_Success;
}

HEventSubscription::SubscriptionStatus
    HEventSubscriptionManager::subscriptionStatus(const HService* service) const
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
    Q_ASSERT(service);

    HUdn udn = service->parentDevice()->deviceInfo().udn();

    QMutexLocker locker(&m_subscribtionsMutex);

    QList<HEventSubscription*>* subs = m_subscriptionsByUdn.value(udn);

    if (!subs)
    {
        return HEventSubscription::Status_Unsubscribed;
    }

    QList<HEventSubscription*>::iterator it = subs->begin();
    for(; it != subs->end(); ++it)
    {
        HEventSubscription* sub = (*it);

        if (sub->service()->m_service == service)
        {
            return sub->subscriptionStatus();
        }
    }

    return HEventSubscription::Status_Unsubscribed;
}

bool HEventSubscriptionManager::cancel(
    HDevice* device, DeviceVisitType visitType, bool unsubscribe)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
    Q_ASSERT(device);
    Q_ASSERT(thread() == QThread::currentThread());

    HUdn udn = device->deviceInfo().udn();

    QMutexLocker locker(&m_subscribtionsMutex);

    QList<HEventSubscription*>* subs = m_subscriptionsByUdn.value(udn);

    if (!subs)
    {
        return false;
    }

    QList<HEventSubscription*>::iterator it = subs->begin();
    for(; it != subs->end(); ++it)
    {
        if (unsubscribe)
        {
            (*it)->unsubscribe();
        }
        else
        {
            (*it)->resetSubscription();
        }
    }

    if (visitType == VisitThisAndDirectChildren ||
        visitType == VisitThisRecursively)
    {
        HDeviceList devices = device->embeddedDevices();
        foreach(HDevice* embDevice, devices)
        {
            DeviceVisitType visitTypeForChildren =
                visitType == VisitThisRecursively ?
                    VisitThisRecursively : VisitThisOnly;

            cancel(embDevice, visitTypeForChildren, unsubscribe);
        }
    }

    return true;
}

bool HEventSubscriptionManager::remove(HDevice* device, bool recursive)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
    Q_ASSERT(device);
    Q_ASSERT(thread() == QThread::currentThread());

    HUdn udn = device->deviceInfo().udn();

    QMutexLocker locker(&m_subscribtionsMutex);

    QList<HEventSubscription*>* subs = m_subscriptionsByUdn.value(udn);

    if (!subs)
    {
        return false;
    }

    QList<HEventSubscription*>::iterator it = subs->begin();
    for(; it != subs->end(); ++it)
    {
        HEventSubscription* sub = (*it);
        m_subscribtionsByUuid.remove(sub->id());
        delete sub;
    }

    m_subscriptionsByUdn.remove(udn);
    delete subs;

    if (recursive)
    {
        HDeviceList devices = device->embeddedDevices();
        foreach(HDevice* embDevice, devices)
        {
            remove(embDevice, recursive);
        }
    }

    return true;
}

bool HEventSubscriptionManager::cancel(HService* service, bool unsubscribe)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
    Q_ASSERT(service);
    Q_ASSERT(thread() == QThread::currentThread());

    HDevice* parentDevice = service->parentDevice();

    HUdn udn = parentDevice->deviceInfo().udn();

    QMutexLocker locker(&m_subscribtionsMutex);

    QList<HEventSubscription*>* subs = m_subscriptionsByUdn.value(udn);

    if (!subs)
    {
        return false;
    }

    QList<HEventSubscription*>::iterator it = subs->begin();
    for(; it != subs->end(); ++it)
    {
        HEventSubscription* sub = (*it);

        if (sub->service()->m_service != service)
        {
            continue;
        }

        if (unsubscribe)
        {
            (*it)->unsubscribe();
        }
        else
        {
            (*it)->resetSubscription();
        }

        return true;
    }

    return false;
}

bool HEventSubscriptionManager::remove(HService* service)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
    Q_ASSERT(service);
    Q_ASSERT(thread() == QThread::currentThread());

    HDevice* parentDevice = service->parentDevice();

    HUdn udn = parentDevice->deviceInfo().udn();

    QMutexLocker locker(&m_subscribtionsMutex);

    QList<HEventSubscription*>* subs = m_subscriptionsByUdn.value(udn);

    if (!subs)
    {
        return false;
    }

    QList<HEventSubscription*>::iterator it = subs->begin();
    for(; it != subs->end(); ++it)
    {
        HEventSubscription* sub = (*it);

        if (sub->service()->m_service != service)
        {
            continue;
        }

        subs->erase(it);
        if (subs->isEmpty())
        {
            delete subs;
            m_subscriptionsByUdn.remove(udn);
        }

        m_subscribtionsByUuid.remove(sub->id());
        delete sub;

        return true;
    }

    return false;
}

void HEventSubscriptionManager::cancelAll(qint32 msecsToWait)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
    Q_ASSERT(thread() == QThread::currentThread());

    QMutexLocker lock(&m_subscribtionsMutex);

    QHash<QUuid, HEventSubscription*>::iterator it =
        m_subscribtionsByUuid.begin();

    for(; it != m_subscribtionsByUuid.end(); ++it)
    {
        (*it)->unsubscribe(msecsToWait);
    }
}

void HEventSubscriptionManager::removeAll()
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
    Q_ASSERT(thread() == QThread::currentThread());

    QMutexLocker lock(&m_subscribtionsMutex);

    qDeleteAll(m_subscribtionsByUuid);
    m_subscribtionsByUuid.clear();

    qDeleteAll(m_subscriptionsByUdn);
    m_subscriptionsByUdn.clear();
}

bool HEventSubscriptionManager::onNotify(
    const QUuid& id, MessagingInfo& mi, const NotifyRequest& req)
{
    HLOG2(H_AT, H_FUN, m_owner->m_loggingIdentifier);
    QMutexLocker lock(&m_subscribtionsMutex);

    HEventSubscription* sub = m_subscribtionsByUuid.value(id);
    if (!sub)
    {
        HLOG_WARN(QString(
            "Ignoring notification [seq: %1] due to invalid callback ID [%2]: "
            "no such subscription found.").arg(
                QString::number(req.seq()), id.toString()));

        return false;
    }

    return sub->onNotify(mi, req);
}

}
}
