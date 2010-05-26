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

#include "hservice.h"
#include "hservice_p.h"

#include "hdevice.h"
#include "haction_p.h"

#include "../../utils/hlogger_p.h"
#include "../general/hupnp_global_p.h"

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
        QObject(service->h_ptr->m_parentDevice), m_service(service)
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
    m_serviceId        (),
    m_serviceType      (),
    m_scpdUrl          (),
    m_controlUrl       (),
    m_eventSubUrl      (),
    m_serviceDescriptor(),
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
    Q_ASSERT(!m_stateVariables.contains(sv->m_stateVariable->name()));

    m_stateVariables.insert(sv->m_stateVariable->name(), sv);

    if (!m_evented &&
        sv->m_stateVariable->eventingType() != HStateVariable::NoEvents)
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

        if (!stateVar->isValidValue(
            convertToRightVariantType(
                variables[i].second, stateVar->m_stateVariable->dataType())))
        {
            HLOG_WARN(QString("Cannot update state variable [%1]. New value is invalid: [%2]").
                arg(stateVar->m_stateVariable->name(), variables[i].second));

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

        bool ok = stateVar->setValue(convertToRightVariantType(
            variables[i].second, stateVar->m_stateVariable->dataType()));

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
                    stateVar->m_stateVariable->name(), variables[i].second));
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
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);
    delete h_ptr;
}

void HService::finalizeInit()
{
    // intentionally empty.
}

HDevice* HService::parentDevice() const
{
    return h_ptr->m_parentDevice;
}

HServiceId HService::serviceId() const
{
    return h_ptr->m_serviceId;
}

HResourceType HService::serviceType() const
{
    return h_ptr->m_serviceType;
}

QUrl HService::scpdUrl() const
{
    return h_ptr->m_scpdUrl;
}

QUrl HService::controlUrl() const
{
    return h_ptr->m_controlUrl;
}

QUrl HService::eventSubUrl() const
{
    return h_ptr->m_eventSubUrl;
}

QString HService::serviceDescription() const
{
    return h_ptr->m_serviceDescriptor.toString();
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
