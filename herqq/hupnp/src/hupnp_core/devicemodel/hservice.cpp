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
#include "hdevice.h"
#include "hservice_p.h"

#include "./../../utils/hlogger_p.h"
#include "./../general/hupnp_global_p.h"

#include "./../datatypes/hupnp_datatypes.h"
#include "./../datatypes/hdatatype_mappings_p.h"

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
    m_loggingIdentifier(),
    m_stateVariablesAreImmutable(false)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
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

    // before modifying anything, it is better to be sure that the incoming
    // data is valid and can be set completely.
    for (int i = 0; i < variables.size(); ++i)
    {
        HStateVariableController* stateVar =
            m_stateVariables.value(variables[i].first);

        if (!stateVar)
        {
            HLOG_WARN(QObject::tr("Cannot update state variable: no state variable [%1]").arg(
                variables[i].first));

            return false;
        }

        if (!stateVar->isValidValue(
            convertToRightVariantType(
                variables[i].second, stateVar->m_stateVariable->dataType())))
        {
            HLOG_WARN(QObject::tr("Cannot update state variable [%1]. New value is invalid: [%2]").
                arg(stateVar->m_stateVariable->name(), variables[i].second));

            return false;
        }
    }

    QMutexLocker lock(&m_updateMutex);
    m_eventsEnabled = false;
    for (int i = 0; i < variables.size(); ++i)
    {
        HStateVariableController* stateVar =
            m_stateVariables.value(variables[i].first);

        stateVar->setValue(convertToRightVariantType(
            variables[i].second, stateVar->m_stateVariable->dataType()));
    }
    m_eventsEnabled = true;
    lock.unlock();

    if (sendEvent && m_evented)
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

bool HService::setStateVariableValue(
    const QString& stateVarName, const QVariant& value)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (h_ptr->m_stateVariablesAreImmutable)
    {
        HLOG_WARN(QObject::tr(
            "Cannot change the value of a state variable that is hosted in a control point"));

        return false;
    }

    return h_ptr->updateVariable(stateVarName, value);
}

bool HService::setStateVariableValues(const QHash<QString, QVariant>& values)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    if (h_ptr->m_stateVariablesAreImmutable)
    {
        HLOG_WARN(QObject::tr(
            "Cannot change the value of a state variable that is hosted in a control point"));

        return false;
    }

    QList<QPair<QString, QString> > valuesAsList;
    QHash<QString, QVariant>::const_iterator ci = values.constBegin();
    for(; ci != values.constEnd(); ++ci)
    {
        valuesAsList.append(qMakePair(ci.key(), ci.value().toString()));
    }

    return h_ptr->updateVariables(valuesAsList, true);
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
    return h_ptr->m_actions;
}

HAction* HService::actionByName(const QString& name) const
{
    return h_ptr->m_actionsAsMap.value(name);
}

QList<HStateVariable*> HService::stateVariables() const
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

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
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

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

    HLOG_DBG(QObject::tr("Notifying listeners."));

    emit stateChanged(this);
}

bool HService::isEvented() const
{
    return h_ptr->m_evented;
}

}
}
