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

#include "hstatevariable.h"
#include "hstatevariable_p.h"
#include "hwritable_statevariable.h"
#include "hreadable_statevariable.h"

#include "../general/hupnp_global_p.h"

#include "../../utils/hlogger_p.h"

#include <QMetaType>
#include <QMutexLocker>

static bool registerMetaTypes()
{
    qRegisterMetaType<Herqq::Upnp::HStateVariableEvent>(
        "Herqq::Upnp::HStateVariableEvent");

    return true;
}

static bool test = registerMetaTypes();

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HStateVariableEventPrivate
 *******************************************************************************/
HStateVariableEventPrivate::HStateVariableEventPrivate() :
    m_eventSource()
{
}

HStateVariableEventPrivate::~HStateVariableEventPrivate()
{
}

/*******************************************************************************
 * HStateVariableEvent
 *******************************************************************************/
HStateVariableEvent::HStateVariableEvent() :
    h_ptr(new HStateVariableEventPrivate())
{
}

HStateVariableEvent::HStateVariableEvent(
    const HStateVariableInfo& eventSource, const QVariant& previousValue,
    const QVariant& newValue)
        : h_ptr(new HStateVariableEventPrivate())
{
    HLOG(H_AT, H_FUN);

    if (!eventSource.isValid())
    {
        HLOG_WARN("Event source is not defined");
        return;
    }

    if (!eventSource.isValidValue(newValue))
    {
        HLOG_WARN(QString("The specified new value [%1] is invalid").arg(
            newValue.toString()));

        return;
    }

    h_ptr->m_eventSource   = eventSource;
    h_ptr->m_previousValue = previousValue;
    h_ptr->m_newValue      = newValue;
}

HStateVariableEvent::HStateVariableEvent(const HStateVariableEvent& other)
    : h_ptr(0)
{
    Q_ASSERT(&other != this);
    h_ptr = new HStateVariableEventPrivate(*other.h_ptr);
}

HStateVariableEvent::~HStateVariableEvent()
{
    delete h_ptr;
}

HStateVariableEvent& HStateVariableEvent::operator=(
    const HStateVariableEvent& other)
{
    Q_ASSERT(&other != this);

    HStateVariableEventPrivate* newHptr =
        new HStateVariableEventPrivate(*other.h_ptr);

    delete h_ptr;
    h_ptr = newHptr;

    return *this;
}

bool HStateVariableEvent::isValid() const
{
    return h_ptr->m_eventSource.isValid();
}

const HStateVariableInfo& HStateVariableEvent::eventSource() const
{
    return h_ptr->m_eventSource;
}

QVariant HStateVariableEvent::previousValue() const
{
    return h_ptr->m_previousValue;
}

QVariant HStateVariableEvent::newValue() const
{
    return h_ptr->m_newValue;
}

/*******************************************************************************
 * HStateVariableController
 *******************************************************************************/
HStateVariableController::HStateVariableController(HStateVariable* stateVar) :
    m_stateVariable(stateVar)
{
    Q_ASSERT(m_stateVariable);
}

HStateVariableController::~HStateVariableController()
{
    delete m_stateVariable;
}

bool HStateVariableController::setValue(const QVariant& newValue)
{
    return m_stateVariable->setValue(newValue);
}

/*******************************************************************************
 * HStateVariablePrivate
 *******************************************************************************/
HStateVariablePrivate::HStateVariablePrivate() :
    m_info(),
    m_value(),
    m_valueMutex(QMutex::Recursive),
    m_parentService(0)
{
}

HStateVariablePrivate::~HStateVariablePrivate()
{
}

bool HStateVariablePrivate::setValue(const QVariant& value, QString* err)
{
    HLOG(H_AT, H_FUN);

    if (value == m_value)
    {
        if (err)
        {
            *err = QString("The new and the old value are equal: [%1]").arg(
                value.toString());
        }
        return false;
    }

    QVariant convertedValue;
    if (m_info.isValidValue(value, &convertedValue, err))
    {
        m_value = convertedValue;
        return true;
    }

    return false;
}

/*******************************************************************************
 * HStateVariable
 *******************************************************************************/
HStateVariable::HStateVariable(HService* parent) :
    QObject(parent),
        h_ptr(new HStateVariablePrivate())
{
    HLOG(H_AT, H_FUN);
    Q_ASSERT_X(parent, H_AT, "Parent service must be defined.");
    h_ptr->m_parentService = parent;
}

HStateVariable::HStateVariable(HStateVariablePrivate& dd, HService* parent) :
    QObject(parent),
        h_ptr(&dd)
{
    HLOG(H_AT, H_FUN);
    Q_ASSERT_X(parent, H_AT, "Parent service must be defined.");
    h_ptr->m_parentService = parent;
}

bool HStateVariable::init(const HStateVariableInfo& info)
{
    if (!info.isValid())
    {
        return false;
    }

    h_ptr->m_info = info;
    setValue(info.defaultValue());
    return true;
}

HStateVariable::~HStateVariable()
{
    delete h_ptr;
}

HService* HStateVariable::parentService() const
{
    Q_ASSERT(h_ptr->m_parentService);
    return h_ptr->m_parentService;
}

QVariant HStateVariable::value() const
{
    QMutexLocker lock(&h_ptr->m_valueMutex);
    return h_ptr->m_value;
}

const HStateVariableInfo& HStateVariable::info() const
{
    return h_ptr->m_info;
}

HWritableStateVariable* HStateVariable::writable()
{
    return dynamic_cast<HWritableStateVariable*>(this);
}

HReadableStateVariable* HStateVariable::readable()
{
    return dynamic_cast<HReadableStateVariable*>(this);
}

bool HStateVariable::setValue(const QVariant& newValue)
{
    HLOG2(H_AT, H_FUN, h_ptr->m_loggingIdentifier);

    QMutexLocker lock(&h_ptr->m_valueMutex);

    QVariant oldValue = h_ptr->m_value;

    if (!h_ptr->setValue(newValue))
    {
        return false;
    }

    if (h_ptr->m_info.eventingType() != HStateVariableInfo::NoEvents)
    {
        HStateVariableEvent event(info(), oldValue, newValue);
        lock.unlock();
        emit valueChanged(event);
    }

    return true;
}

}
}
