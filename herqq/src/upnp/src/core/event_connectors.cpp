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

#include "event_connectors.h"

#include "devicemodel/service.h"
#include "devicemodel/statevariable.h"
#include "devicehosting/abstracthost.h"

namespace Herqq
{

namespace Upnp
{

class HEventListenerPrivate
{
public:

    HEventListener::AbstractHostEventCallback m_rootDeviceAdded;
    HEventListener::AbstractHostEventCallback m_rootDeviceRemoved;
    HEventListener::ServiceEventCallback m_serviceChanged;
    HEventListener::StateVariableEventCallback m_stateVariableValueChanged;

};

/*******************************************************************************
 *
 ******************************************************************************/
HEventListener::HEventListener() :
    h_ptr(new HEventListenerPrivate())
{
}

HEventListener::~HEventListener()
{
    delete h_ptr;
}

void HEventListener::rootDeviceAdded(const HDeviceInfo& deviceInfo)
{
    if (h_ptr->m_rootDeviceAdded)
    {
        h_ptr->m_rootDeviceAdded(deviceInfo);
    }
}

void HEventListener::rootDeviceRemoved(const HDeviceInfo& deviceInfo)
{
    if (h_ptr->m_rootDeviceRemoved)
    {
        h_ptr->m_rootDeviceRemoved(deviceInfo);
    }
}

void HEventListener::stateChanged(const HService* source)
{
    if (h_ptr->m_serviceChanged)
    {
        h_ptr->m_serviceChanged(source);
    }
}

void HEventListener::valueChanged(const HStateVariableEvent& eventInfo)
{
    if (h_ptr->m_stateVariableValueChanged)
    {
        h_ptr->m_stateVariableValueChanged(eventInfo);
    }
}

void HEventListener::setRootDeviceAddedListener(AbstractHostEventCallback cb)
{
    h_ptr->m_rootDeviceAdded = cb;
}

void HEventListener::setRootDeviceRemovedListener(AbstractHostEventCallback cb)
{
    h_ptr->m_rootDeviceRemoved = cb;
}

void HEventListener::setServiceStateChangedListener(ServiceEventCallback cb)
{
    h_ptr->m_serviceChanged = cb;
}

void HEventListener::setServiceStateChangedListener(StateVariableEventCallback cb)
{
    h_ptr->m_stateVariableValueChanged = cb;
}

/*******************************************************************************
 * HEventConnectorPrivate
 ******************************************************************************/
class HEventConnectorPrivate
{
public:

    QPair<HAbstractHost*, HEventListener*> m_hostConnection;
    QPair<HService*, HEventListener*> m_serviceConnection;
    QPair<HStateVariable*, HEventListener*> m_stateVariableConnection;
    QPair<HAction*, HEventListener*> m_actionConnection;
};

/*******************************************************************************
 * HEventConnector
 ******************************************************************************/
HEventConnector::HEventConnector() :
    h_ptr(new HEventConnectorPrivate())
{
}

HEventConnector::~HEventConnector()
{
    delete h_ptr;
}

void HEventConnector::rootDeviceAdded(
    const Herqq::Upnp::HDeviceInfo& newDeviceInfo)
{
    h_ptr->m_hostConnection.second->rootDeviceAdded(newDeviceInfo);
}

void HEventConnector::rootDeviceRemoved (
    const Herqq::Upnp::HDeviceInfo& deviceInfo)
{
    h_ptr->m_hostConnection.second->rootDeviceRemoved(deviceInfo);
}

void HEventConnector::stateChanged(const HService* source)
{
    h_ptr->m_serviceConnection.second->stateChanged(source);
}

void HEventConnector::valueChanged(const HStateVariableEvent& eventInfo)
{
    h_ptr->m_stateVariableConnection.second->valueChanged(eventInfo);
}

void HEventConnector::setConnection(HAbstractHost* host, HEventListener* listener)
{
    Q_ASSERT(host);
    Q_ASSERT(listener);

    h_ptr->m_hostConnection.first = host;
    h_ptr->m_hostConnection.second = listener;

    bool ok = connect(
        h_ptr->m_hostConnection.first,
        SIGNAL(rootDeviceAdded(Herqq::Upnp::HDeviceInfo)),
        this,
        SLOT(rootDeviceAdded(Herqq::Upnp::HDeviceInfo)),
        Qt::DirectConnection);

    Q_ASSERT(ok); Q_UNUSED(ok)

    ok = connect(
        h_ptr->m_hostConnection.first,
        SIGNAL(rootDeviceRemoved(Herqq::Upnp::HDeviceInfo)),
        this,
        SLOT(rootDeviceRemoved(Herqq::Upnp::HDeviceInfo)),
        Qt::DirectConnection);

    Q_ASSERT(ok);
}

void HEventConnector::setConnection(HService* source, HEventListener* listener)
{
    Q_ASSERT(source);
    Q_ASSERT(listener);

    h_ptr->m_serviceConnection.first = source;
    h_ptr->m_serviceConnection.second = listener;

    bool ok = connect(
        h_ptr->m_serviceConnection.first,
        SIGNAL(stateChanged(Herqq::Upnp::HService*)),
        this,
        SLOT(stateChanged(Herqq::Upnp::HService*)),
        Qt::DirectConnection);

    Q_ASSERT(ok); Q_UNUSED(ok)
}

void HEventConnector::setConnection(HStateVariable* stateVar, HEventListener* listener)
{
    Q_ASSERT(stateVar);
    Q_ASSERT(listener);

    h_ptr->m_stateVariableConnection.first = stateVar;
    h_ptr->m_stateVariableConnection.second = listener;

    bool ok = connect(
        h_ptr->m_stateVariableConnection.first,
        SIGNAL(valueChanged(Herqq::Upnp::HStateVariableEvent)),
        this,
        SLOT(valueChanged(Herqq::Upnp::HStateVariableEvent)),
        Qt::DirectConnection);

    Q_ASSERT(ok); Q_UNUSED(ok)
}

void HEventConnector::setConnection(HAction* action, HEventListener* listener)
{
    Q_ASSERT(action);
    Q_ASSERT(listener);

    h_ptr->m_actionConnection.first = action;
    h_ptr->m_actionConnection.second = listener;

    /*bool ok = connect(
        action,
        SIGNAL(valueChanged(Herqq::Upnp::HStateVariableEvent)),
        this,
        SLOT(valueChanged(Herqq::Upnp::HStateVariableEvent)),
        Qt::DirectConnection);

    Q_ASSERT(ok); Q_UNUSED(ok)*/
}

}
}
