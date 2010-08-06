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

#include "hservice.h"
#include "hservice_p.h"
#include "hactions_setupdata.h"
#include "hstatevariables_setupdata.h"

#include "haction_p.h"

#include "../../utils/hlogger_p.h"

#include "../datatypes/hupnp_datatypes.h"
#include "../datatypes/hdatatype_mappings_p.h"

#include <QByteArray>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HServiceController
 ******************************************************************************/
HServiceController::HServiceController(
    HService* service) :
        QObject(reinterpret_cast<QObject*>(service->h_ptr->m_parentDevice)),
            m_service(service)
{
    Q_ASSERT(m_service);
    m_service->setParent(this);
}

HServiceController::~HServiceController()
{
    HLOG2(H_AT, H_FUN, m_service->h_ptr->m_loggingIdentifier);
}

bool HServiceController::updateVariables(
    const QList<QPair<QString, QString> >& variables, bool sendEvent)
{
    HLOG2(H_AT, H_FUN, m_service->h_ptr->m_loggingIdentifier);

    return m_service->h_ptr->updateVariables(variables, sendEvent);
}

HActionController* HServiceController::actionByName(const QString& name)
{
    return m_service->h_ptr->m_actionsAsMap.value(name);
}

QList<HActionController*> HServiceController::actions() const
{
    return m_service->h_ptr->m_actions;
}

/*******************************************************************************
 * HServicePrivate
 ******************************************************************************/
HServicePrivate::HServicePrivate() :
    m_serviceInfo      (),
    m_serviceDescription(),
    m_actions          (),
    m_actionsAsMap     (),
    m_stateVariables   (),
    q_ptr              (0),
    m_eventsEnabled    (true),
    m_parentDevice     (0),
    m_evented          (false),
    m_updateMutex      (),
    m_loggingIdentifier()
{
}

HServicePrivate::~HServicePrivate()
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    qDeleteAll(m_actions);
    qDeleteAll(m_stateVariables);
}

bool HServicePrivate::addStateVariable(HStateVariableController* sv)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(sv);

    HStateVariableInfo info = sv->m_stateVariable->info();
    Q_ASSERT(!m_stateVariables.contains(info.name()));

    m_stateVariables.insert(info.name(), sv);

    if (!m_evented && info.eventingType() != HStateVariableInfo::NoEvents)
    {
        m_evented = true;
    }

    return true;
}

bool HServicePrivate::updateVariable(
    const QString& stateVarName, const QVariant& value)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QMutexLocker lock(&m_updateMutex);

    HStateVariableController* sv =
        m_stateVariables.value(stateVarName);

    return sv ? sv->setValue(value) : false;
}

bool HServicePrivate::updateVariables(
    const QList<QPair<QString, QString> >& variables, bool sendEvent)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QMutexLocker lock(&m_updateMutex);

    // before modifying anything, it is better to be sure that the incoming
    // data is valid and can be set completely.
    for (int i = 0; i < variables.size(); ++i)
    {
        HStateVariableController* stateVar =
            m_stateVariables.value(variables[i].first);

        if (!stateVar)
        {
            HLOG_WARN(QString("Cannot update state variable: no state variable [%1]").arg(
                variables[i].first));

            return false;
        }

        const HStateVariableInfo& info = stateVar->m_stateVariable->info();
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
        HStateVariableController* stateVar =
            m_stateVariables.value(variables[i].first);

        Q_ASSERT(stateVar);

        const HStateVariableInfo& info =
            stateVar->m_stateVariable->info();

        bool ok = stateVar->setValue(convertToRightVariantType(
            variables[i].second, info.dataType()));

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
    lock.unlock();

    if (changed && sendEvent && m_evented)
    {
        emit q_ptr->stateChanged(q_ptr);
    }

    return true;
}

/*******************************************************************************
 *HService
 ******************************************************************************/
HService::HService() :
    h_ptr(new HServicePrivate())
{
}

HService::HService(HServicePrivate& dd) :
    h_ptr(&dd)
{
}

HService::~HService()
{
    delete h_ptr;
}

HActionsSetupData HService::createActions()
{
    return HActionsSetupData();
    // intentionally empty.
}

HStateVariablesSetupData HService::stateVariablesSetupData() const
{
    return HStateVariablesSetupData();
    // intentionally empty.
}

bool HService::finalizeInit(QString*)
{
    // intentionally empty.
    return true;
}

HDevice* HService::parentDevice() const
{
    return h_ptr->m_parentDevice;
}

const HServiceInfo& HService::info() const
{
    return h_ptr->m_serviceInfo;
}

const QString& HService::description() const
{
    return h_ptr->m_serviceDescription;
}

QList<HAction*> HService::actions() const
{
    QList<HAction*> retVal;
    for(qint32 i = 0; i < h_ptr->m_actions.size(); ++i)
    {
        retVal.append(h_ptr->m_actions[i]->m_action);
    }

    return retVal;
}

HAction* HService::actionByName(const QString& name) const
{
    HActionController* retVal = h_ptr->m_actionsAsMap.value(name);
    return retVal ? retVal->m_action : 0;
}

QList<HStateVariable*> HService::stateVariables() const
{
    QList<HStateVariable*> retVal;

    QHash<QString, HStateVariableController*>::const_iterator ci =
        h_ptr->m_stateVariables.constBegin();

    for (; ci != h_ptr->m_stateVariables.constEnd(); ++ci)
    {
        retVal.append(ci.value()->m_stateVariable);
    }

    return retVal;
}

HStateVariable* HService::stateVariableByName(const QString& name) const
{
    HStateVariableController* sv = h_ptr->m_stateVariables.value(name);
    return sv ? sv->m_stateVariable : 0;
}

void HService::notifyListeners()
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (!h_ptr->m_evented || !h_ptr->m_eventsEnabled)
    {
        return;
    }

    HLOG_DBG("Notifying listeners.");

    emit stateChanged(this);
}

bool HService::isEvented() const
{
    return h_ptr->m_evented;
}

}
}
