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
#include "../hactions_setupdata.h"
#include "../hstatevariables_setupdata.h"

#include "hserveraction.h"
#include "hserveraction_p.h"
#include "hserverstatevariable.h"

#include "../../../utils/hlogger_p.h"

#include "../../datatypes/hupnp_datatypes.h"
#include "../../datatypes/hdatatype_mappings_p.h"

#include <QtCore/QByteArray>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HServerServicePrivate
 ******************************************************************************/
HServerServicePrivate::HServerServicePrivate() :
    m_serviceInfo      (),
    m_serviceDescription(),
    m_actions          (),
    m_stateVariables   (),
    q_ptr              (0),
    m_eventsEnabled    (true),
    m_parentDevice     (0),
    m_evented          (false),
    m_loggingIdentifier()
{
}

HServerServicePrivate::~HServerServicePrivate()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    qDeleteAll(m_actions);
    qDeleteAll(m_stateVariables);
}

bool HServerServicePrivate::addStateVariable(HServerStateVariable* sv)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(sv);

    const HStateVariableInfo& info = sv->info();
    Q_ASSERT(info.isValid());
    Q_ASSERT(!m_stateVariables.contains(info.name()));

    m_stateVariables.insert(info.name(), sv);

    if (!m_evented && info.eventingType() != HStateVariableInfo::NoEvents)
    {
        m_evented = true;
    }

    return true;
}

bool HServerServicePrivate::updateVariable(
    const QString& stateVarName, const QVariant& value)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    HServerStateVariable* sv =
        m_stateVariables.value(stateVarName);

    return sv ? sv->setValue(value) : false;
}

bool HServerServicePrivate::updateVariables(
    const QList<QPair<QString, QString> >& variables, bool sendEvent)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    // before modifying anything, it is better to be sure that the incoming
    // data is valid and can be set completely.
    for (int i = 0; i < variables.size(); ++i)
    {
        HServerStateVariable* stateVar =
            m_stateVariables.value(variables[i].first);

        if (!stateVar)
        {
            HLOG_WARN(QString("Cannot update state variable: no state variable [%1]").arg(
                variables[i].first));

            return false;
        }

        const HStateVariableInfo& info = stateVar->info();
        if (!info.isValidValue(
            convertToRightVariantType(variables[i].second, info.dataType())))
        {
            HLOG_WARN(QString(
                "Cannot update state variable [%1]. New value is invalid: [%2]").
                    arg(info.name(), variables[i].second));

            return false;
        }
    }

    bool changed = false;
    m_eventsEnabled = false;
    for (int i = 0; i < variables.size(); ++i)
    {
        HServerStateVariable* stateVar =
            m_stateVariables.value(variables[i].first);

        Q_ASSERT(stateVar);

        const HStateVariableInfo& info = stateVar->info();

        bool ok = stateVar->setValue(
            convertToRightVariantType(variables[i].second, info.dataType()));

        if (ok)
        {
            changed = true;
        }
        else
        {
            // this is not too severe and should not be a warning. most often
            // this situation is caused by a new value being equal to the old
            // value

            HLOG_DBG(QString(
                "Failed to set the value of state variable: [%1] to [%2]").arg(
                    info.name(), variables[i].second));
        }
    }
    m_eventsEnabled = true;

    if (changed && sendEvent && m_evented)
    {
        emit q_ptr->stateChanged(q_ptr);
    }

    return true;
}

/*******************************************************************************
 *HServerService
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

bool HServerService::finalizeInit(QString*)
{
    // intentionally empty.
    return true;
}

HServerDevice* HServerService::parentDevice() const
{
    return h_ptr->m_parentDevice;
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
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (!h_ptr->m_evented || !h_ptr->m_eventsEnabled)
    {
        return;
    }

    HLOG_DBG("Notifying listeners.");

    emit stateChanged(this);
}

bool HServerService::isEvented() const
{
    return h_ptr->m_evented;
}

}
}
