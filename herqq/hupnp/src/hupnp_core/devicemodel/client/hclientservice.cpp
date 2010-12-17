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

#include "hclientservice.h"
#include "hclientservice_p.h"
#include "hdefault_clientservice_p.h"

#include "hclientaction.h"
#include "hclientaction_p.h"

#include "hclientstatevariable.h"
#include "../hstatevariable_p.h"

#include "../../../utils/hlogger_p.h"

#include "../../datatypes/hupnp_datatypes.h"
#include "../../datatypes/hdatatype_mappings_p.h"

#include <QtCore/QByteArray>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HClientServicePrivate
 ******************************************************************************/
HClientServicePrivate::HClientServicePrivate() :
    m_serviceInfo      (),
    m_serviceDescription(),
    m_actions          (),
    m_stateVariables   (),
    q_ptr              (0),
    m_eventsEnabled    (true),
    m_evented          (false),
    m_loggingIdentifier()
{
}

HClientServicePrivate::~HClientServicePrivate()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    qDeleteAll(m_actions);
    qDeleteAll(m_stateVariables);
}

bool HClientServicePrivate::addStateVariable(HClientStateVariable* sv)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(sv);

    const HStateVariableInfo& info = sv->info();
    Q_ASSERT(!m_stateVariables.contains(info.name()));

    m_stateVariables.insert(info.name(), sv);
    m_stateVariablesConst.insert(info.name(), sv);

    if (!m_evented && info.eventingType() != HStateVariableInfo::NoEvents)
    {
        m_evented = true;
    }

    return true;
}

bool HClientServicePrivate::updateVariable(
    const QString& stateVarName, const QVariant& value)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    HClientStateVariable* sv = m_stateVariables.value(stateVarName);

    return sv ? sv->setValue(value) : false;
}

bool HClientServicePrivate::updateVariables(
    const QList<QPair<QString, QString> >& variables, bool sendEvent)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    // before modifying anything, it is better to be sure that the incoming
    // data is valid and can be set completely.
    for (int i = 0; i < variables.size(); ++i)
    {
        HClientStateVariable* stateVar =
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
        HClientStateVariable* stateVar =
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
 * HClientService
 ******************************************************************************/
HClientService::HClientService(
    const HServiceInfo& info, HClientDevice* parentDevice) :
        QObject(reinterpret_cast<QObject*>(parentDevice)),
            h_ptr(new HClientServicePrivate())
{
    Q_ASSERT_X(parentDevice, "", "Parent device must be defined!");

    h_ptr->m_serviceInfo = info;
    h_ptr->q_ptr = this;
}

HClientService::~HClientService()
{
    delete h_ptr;
}

HClientDevice* HClientService::parentDevice() const
{
    return reinterpret_cast<HClientDevice*>(parent());
}

const HServiceInfo& HClientService::info() const
{
    return h_ptr->m_serviceInfo;
}

QString HClientService::description() const
{
    return h_ptr->m_serviceDescription;
}

const HClientActions& HClientService::actions() const
{
    return h_ptr->m_actions;
}

const HClientStateVariables& HClientService::stateVariables() const
{
    return h_ptr->m_stateVariablesConst;
}

void HClientService::notifyListeners()
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (!h_ptr->m_evented || !h_ptr->m_eventsEnabled)
    {
        return;
    }

    HLOG_DBG("Notifying listeners.");

    emit stateChanged(this);
}

bool HClientService::isEvented() const
{
    return h_ptr->m_evented;
}

/*******************************************************************************
 * HDefaultClientService
 ******************************************************************************/
HDefaultClientService::HDefaultClientService(
    const HServiceInfo& info, HClientDevice* parentDevice) :
        HClientService(info, parentDevice)
{
}

void HDefaultClientService::addAction(HClientAction* action)
{
    Q_ASSERT(action);
    Q_ASSERT(!h_ptr->m_actions.contains(action->info().name()));
    h_ptr->m_actions.insert(action->info().name(), action);
}

void HDefaultClientService::addStateVariable(HClientStateVariable* sv)
{
    h_ptr->addStateVariable(sv);
}

void HDefaultClientService::setDescription(const QString& description)
{
    h_ptr->m_serviceDescription = description;
}

bool HDefaultClientService::updateVariables(
    const QList<QPair<QString, QString> >& variables, bool sendEvent)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    return h_ptr->updateVariables(variables, sendEvent);
}

}
}
