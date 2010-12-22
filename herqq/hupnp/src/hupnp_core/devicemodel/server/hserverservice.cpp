/*
 *  Copyright (C) 2010 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP (HUPnP) library.
 *
 *  Herqq UPnP is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Herqq UPnP. If not, see <http://www.gnu.org/licenses/>.
 */

#include "hserverservice.h"
#include "hserverservice_p.h"

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HServerServicePrivate
 ******************************************************************************/
HServerServicePrivate::HServerServicePrivate()
{
}

HServerServicePrivate::~HServerServicePrivate()
{
}

HServerServicePrivate::ReturnValue HServerServicePrivate::updateVariables(
    const QList<QPair<QString, QString> >& variables, bool sendEvent)
{
    ReturnValue rv =
        HServicePrivate<HServerService, HServerAction, HServerStateVariable>::updateVariables(variables);

    if (rv == Updated && sendEvent && m_evented)
    {
        emit q_ptr->stateChanged(q_ptr);
    }

    return rv;
}

/*******************************************************************************
 * HServerService
 ******************************************************************************/
HServerService::HServerService() :
    h_ptr(new HServerServicePrivate())
{
}

HServerService::HServerService(HServerServicePrivate& dd) :
    h_ptr(&dd)
{
}

HServerService::~HServerService()
{
    delete h_ptr;
}

bool HServerService::init(
    const HServiceInfo& info, HServerDevice* parentDevice)
{
    if (h_ptr->q_ptr)
    {
        return false;
    }

    Q_ASSERT_X(parentDevice, "parentDevice", "Parent device has to be defined.");
    setParent(reinterpret_cast<QObject*>(parentDevice));
    h_ptr->m_serviceInfo = info;
    h_ptr->q_ptr = this;

    return true;
}

bool HServerService::finalizeInit(QString*)
{
    // intentionally empty.
    return true;
}

HServerDevice* HServerService::parentDevice() const
{
    return reinterpret_cast<HServerDevice*>(parent());
}

const HServiceInfo& HServerService::info() const
{
    return h_ptr->m_serviceInfo;
}

const QString& HServerService::description() const
{
    return h_ptr->m_serviceDescription;
}

const HServerActions& HServerService::actions() const
{
    return h_ptr->m_actions;
}

const HServerStateVariables& HServerService::stateVariables() const
{
    return h_ptr->m_stateVariables;
}

void HServerService::notifyListeners()
{
    if (h_ptr->m_evented)
    {
        emit stateChanged(this);
    }
}

bool HServerService::isEvented() const
{
    return h_ptr->m_evented;
}

}
}
