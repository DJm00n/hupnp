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

#include "hevent_connectors.h"

#include "./../devicemodel/hservice.h"
#include "./../devicemodel/hstatevariable.h"
#include "./../devicehosting/controlpoint/hcontrolpoint.h"

namespace Herqq
{

namespace Upnp
{

class HEventListenerPrivate
{
public:

    HEventListener::ControlPointEventCallback m_rootDeviceOnline;
    HEventListener::ControlPointEventCallback m_rootDeviceOffline;
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

void HEventListener::rootDeviceOnline(HDevice* device)
{
    if (h_ptr->m_rootDeviceOnline)
    {
        h_ptr->m_rootDeviceOnline(device);
    }
}

void HEventListener::rootDeviceOffline(HDevice* device)
{
    if (h_ptr->m_rootDeviceOffline)
    {
        h_ptr->m_rootDeviceOffline(device);
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

void HEventListener::setRootDeviceOnlineListener(ControlPointEventCallback cb)
{
    h_ptr->m_rootDeviceOnline = cb;
}

void HEventListener::setRootDeviceOfflineListener(ControlPointEventCallback cb)
{
    h_ptr->m_rootDeviceOffline = cb;
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

    QPair<HControlPoint*, HEventListener*> m_hostConnection;
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

void HEventConnector::rootDeviceOnline(Herqq::Upnp::HDevice* newDevice)
{
    h_ptr->m_hostConnection.second->rootDeviceOnline(newDevice);
}

void HEventConnector::rootDeviceOffline(Herqq::Upnp::HDevice* device)
{
    h_ptr->m_hostConnection.second->rootDeviceOffline(device);
}

void HEventConnector::stateChanged(const HService* source)
{
    h_ptr->m_serviceConnection.second->stateChanged(source);
}

void HEventConnector::valueChanged(const HStateVariableEvent& eventInfo)
{
    h_ptr->m_stateVariableConnection.second->valueChanged(eventInfo);
}

void HEventConnector::setConnection(HControlPoint* host, HEventListener* listener)
{
    Q_ASSERT(host);
    Q_ASSERT(listener);

    h_ptr->m_hostConnection.first = host;
    h_ptr->m_hostConnection.second = listener;

    bool ok = connect(
        h_ptr->m_hostConnection.first,
        SIGNAL(rootDeviceOnline(Herqq::Upnp::HDevice*)),
        this,
        SLOT(rootDeviceOnline(Herqq::Upnp::HDevice*)),
        Qt::DirectConnection);

    Q_ASSERT(ok); Q_UNUSED(ok)

    ok = connect(
        h_ptr->m_hostConnection.first,
        SIGNAL(rootDeviceOffline(Herqq::Upnp::HDevice*)),
        this,
        SLOT(rootDeviceOffline(Herqq::Upnp::HDevice*)),
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
