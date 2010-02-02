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

#include "statevariable.h"
#include "statevariable_p.h"

#include "../upnp_global_p.h"
#include "../datatype_mappings_p.h"

#include "../../../../utils/src/logger_p.h"

#include <QMetaType>
#include <QMutexLocker>

static bool registerMetaTypes()
{
    static QAtomicInt tester(0);

    if (tester.testAndSetAcquire(0, 1))
    {
        qRegisterMetaType<Herqq::Upnp::HStateVariableEvent>(
            "Herqq::Upnp::HStateVariableEvent");
    }

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
    m_eventSource(0)
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
    HStateVariable* eventSource, const QVariant& previousValue,
    const QVariant& newValue)
        : h_ptr(new HStateVariableEventPrivate())
{
    HLOG(H_AT, H_FUN);

    if (!eventSource)
    {
        HLOG_WARN(QObject::tr("Event source is not defined"));
        return;
    }

    if (!eventSource->isValidValue(newValue))
    {
        HLOG_WARN(QObject::tr("The specified new value [%1] is invalid").arg(newValue.toString()));
        return;
    }

    h_ptr->m_eventSource   = eventSource;
    h_ptr->m_previousValue = previousValue;
    h_ptr->m_newValue      = newValue;
}

HStateVariableEvent::HStateVariableEvent(const HStateVariableEvent& other)
    : h_ptr(new HStateVariableEventPrivate(*other.h_ptr))
{
}

HStateVariableEvent::~HStateVariableEvent()
{
    delete h_ptr;
}

HStateVariableEvent& HStateVariableEvent::operator=(const HStateVariableEvent& other)
{
    HStateVariableEventPrivate* newHptr =
        new HStateVariableEventPrivate(*other.h_ptr);

    delete h_ptr;
    h_ptr = newHptr;

    return *this;
}

bool HStateVariableEvent::isEmpty() const
{
    return !h_ptr->m_eventSource && h_ptr->m_previousValue.isNull() &&
            h_ptr->m_newValue.isNull();
}

HStateVariable* HStateVariableEvent::eventSource() const
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

bool HStateVariableController::isValidValue(
    const QVariant& value, QVariant* convertedValue) const
{
    return m_stateVariable->isValidValue(value, convertedValue);
}

bool HStateVariableController::setValue(const QVariant& newValue)
{
    return m_stateVariable->setValue(newValue);
}

/*******************************************************************************
 * HStateVariablePrivate
 *******************************************************************************/
HStateVariablePrivate::HStateVariablePrivate() :
    m_name(), m_dataType(HUpnpDataTypes::Undefined),
    m_variantDataType(QVariant::Invalid), m_defaultValue(),
    m_eventingType(HStateVariable::NoEvents), m_allowedValueList(),
    m_allowedValueRange(), m_value(), m_valueMutex(), m_parentService(0)
{
}

HStateVariablePrivate::~HStateVariablePrivate()
{
}

void HStateVariablePrivate::setName(const QString& name)
{
    HLOG(H_AT, H_FUN);

    verifyName(name);
    m_name = name;
}

void HStateVariablePrivate::setDataType(HUpnpDataTypes::DataType dt)
{
    HLOG(H_AT, H_FUN);

    m_dataType = dt;
    m_variantDataType = convertToVariantType(m_dataType);
    m_defaultValue = QVariant(m_variantDataType);
    m_value = QVariant(m_variantDataType);
}

QVariant HStateVariablePrivate::checkValue(const QVariant& value)
{
    HLOG(H_AT, H_FUN);

    QVariant acceptableValue(value);

    if (m_dataType == HUpnpDataTypes::Undefined)
    {
        throw HIllegalArgumentException(
            QObject::tr("Data type of the state variable [%1] is not defined.").arg(
                m_name));
    }

    if (value.type() != m_variantDataType)
    {
        if (m_variantDataType == QVariant::Url)
        {
            // for some reason, QVariant does not provide automatic conversion between
            // QUrl and other types (this includes QString, unfortunately) and until it does,
            // this has to be handled as a special case.

            QUrl valueAsUrl(value.toString());
            if (!valueAsUrl.isValid())
            {
                throw HIllegalArgumentException(
                    QObject::tr("Invalid value for a URL type: [%1]").arg(
                        value.toString()));
            }

            acceptableValue = valueAsUrl;
        }
        else if (!acceptableValue.convert(m_variantDataType))
        {
            throw HIllegalArgumentException(QObject::tr("Data type mismatch."));
        }
    }

    if (m_dataType == HUpnpDataTypes::string && m_allowedValueList.size())
    {
        if (m_allowedValueList.indexOf(value.toString()) < 0)
        {
            throw HIllegalArgumentException(
                QObject::tr("Value is not included in the allowed values list."));
        }
    }
    else if (HUpnpDataTypes::isRational(m_dataType) && !m_allowedValueRange.isNull())
    {
        qreal tmp = value.toDouble();
        if (tmp < m_allowedValueRange.minimum().toDouble() ||
            tmp > m_allowedValueRange.maximum().toDouble())
        {
            throw HIllegalArgumentException(
                QObject::tr("Value is not within the specified allowed values range."));
        }
    }
    else if (HUpnpDataTypes::isNumeric(m_dataType) && !m_allowedValueRange.isNull())
    {
        qlonglong tmp = value.toLongLong();
        if (tmp < m_allowedValueRange.minimum().toLongLong() ||
            tmp > m_allowedValueRange.maximum().toLongLong())
        {
            throw HIllegalArgumentException(
                QObject::tr("Value is not within the specified allowed values range."));
        }
    }

    return acceptableValue;
}

void HStateVariablePrivate::setDefaultValue(const QVariant& defVal)
{
    if (defVal.isNull() || !defVal.isValid() ||
       ((m_dataType == HUpnpDataTypes::string && m_allowedValueList.size()) &&
        defVal.toString().isEmpty()))
    {
        // according to the UDA, default value is OPTIONAL.
        return;
    }

    checkValue(defVal);
    m_defaultValue = defVal;
    m_value = m_defaultValue;
}

void HStateVariablePrivate::setEventingType(
    HStateVariable::EventingType eventingType)
{
    m_eventingType = eventingType;
}

void HStateVariablePrivate::setAllowedValueList(
    const QStringList& allowedValueList)
{
    HLOG(H_AT, H_FUN);

    if (m_dataType != HUpnpDataTypes::string)
    {
        throw HIllegalArgumentException(QObject::tr(
            "Cannot define allowed values list when data type is not \"string\""));
    }

    m_allowedValueList = allowedValueList;
}

void HStateVariablePrivate::setAllowedValueRange(
    HValueRange allowedValueRange)
{
    HLOG(H_AT, H_FUN);

    if (!HUpnpDataTypes::isNumeric(m_dataType))
    {
        throw HIllegalArgumentException(QObject::tr(
            "Cannot define allowed value range when data type is not numeric"));
    }

    if (allowedValueRange.minimum().type() != m_variantDataType)
    {
        throw HIllegalArgumentException(QObject::tr("Data type mismatch."));
    }

    m_allowedValueRange = allowedValueRange;
}

bool HStateVariablePrivate::setValue(const QVariant& value)
{
    HLOG(H_AT, H_FUN);

    if (value == m_value)
    {
        return false;
    }

    try
    {
        m_value = checkValue(value);
    }
    catch(HIllegalArgumentException& ex)
    {
        HLOG_WARN(ex.reason());
        return false;
    }

    return true;
}

/*******************************************************************************
 * HStateVariable
 *******************************************************************************/
HStateVariable::HStateVariable(
    const QString& name, HUpnpDataTypes::DataType datatype,
    const QVariant& defaultValue, EventingType eventingType) :
        h_ptr(new HStateVariablePrivate())
{
    HLOG(H_AT, H_FUN);

    h_ptr->setName        (name);
    h_ptr->setDataType    (datatype);
    // make sure the data type is set before setting any values; some validity
    // checks rely on it

    h_ptr->setDefaultValue(defaultValue);
    h_ptr->setEventingType(eventingType);
}

HStateVariable::HStateVariable(
    const QString& name, const QVariant& defaultValue,
    const QStringList& allowedValueList, EventingType eventingType) :
        h_ptr(new HStateVariablePrivate)
{
    HLOG(H_AT, H_FUN);

    h_ptr->setName            (name);
    h_ptr->setDataType        (HUpnpDataTypes::string);
    // make sure the data type is set before setting any values; some validity
    // checks rely on it

    h_ptr->setAllowedValueList(allowedValueList);

    h_ptr->setDefaultValue    (defaultValue);
    h_ptr->setEventingType    (eventingType);
}

HStateVariable::HStateVariable(
    const QString& name, HUpnpDataTypes::DataType datatype,
    const QVariant& defaultValue, const QVariant& minimumValue,
    const QVariant& maximumValue, const QVariant& stepValue,
    EventingType eventingType) :
        h_ptr(new HStateVariablePrivate)
{
    HLOG(H_AT, H_FUN);

    h_ptr->setName             (name);
    h_ptr->setDataType         (datatype);
    // make sure the data type is set before setting any values; some validity
    // checks rely on it

    h_ptr->setAllowedValueRange(
        HValueRange::fromVariant(
            convertToVariantType(datatype), minimumValue, maximumValue, stepValue));

    h_ptr->setDefaultValue    (defaultValue);
    h_ptr->setEventingType    (eventingType);
}

HStateVariable::~HStateVariable()
{
    HLOG(H_AT, H_FUN);

    delete h_ptr;
}

void HStateVariable::setParentService(HService* parentService)
{
    Q_ASSERT(parentService);
    Q_ASSERT(!h_ptr->m_parentService);
    h_ptr->m_parentService = parentService;
}

HService* HStateVariable::parentService() const
{
    Q_ASSERT(h_ptr->m_parentService);
    return h_ptr->m_parentService;
}

QString HStateVariable::name() const
{
    return h_ptr->m_name;
}

HUpnpDataTypes::DataType HStateVariable::dataType() const
{
    return h_ptr->m_dataType;
}

HStateVariable::EventingType HStateVariable::eventingType() const
{
    return h_ptr->m_eventingType;
}

QStringList HStateVariable::allowedValueList() const
{
    return h_ptr->m_allowedValueList;
}

QVariant HStateVariable::minimumValue() const
{
    return h_ptr->m_allowedValueRange.minimum();
}

QVariant HStateVariable::maximumValue() const
{
    return h_ptr->m_allowedValueRange.maximum();
}

QVariant HStateVariable::stepValue() const
{
    return h_ptr->m_allowedValueRange.step();
}

QVariant HStateVariable::defaultValue() const
{
    return h_ptr->m_defaultValue;
}

bool HStateVariable::isValidValue(
    const QVariant& value, QVariant* convertedValue) const
{
    HLOG(H_AT, H_FUN);

    try
    {
        if (convertedValue)
        {
            *convertedValue = h_ptr->checkValue(value);
        }
        else
        {
            h_ptr->checkValue(value);
        }
    }
    catch(HException&)
    {
        return false;
    }

    return true;
}

QVariant HStateVariable::value() const
{
    HLOG(H_AT, H_FUN);

    QMutexLocker lock(&h_ptr->m_valueMutex);
    return h_ptr->m_value;
}

bool HStateVariable::isConstrained() const
{
    return !h_ptr->m_allowedValueList.isEmpty() ||
           !h_ptr->m_allowedValueRange.isNull();
}

bool HStateVariable::setValue(const QVariant& newValue)
{
    HLOG(H_AT, H_FUN);

    QMutexLocker lock(&h_ptr->m_valueMutex);

    QVariant oldValue = h_ptr->m_value;

    if (!h_ptr->setValue(newValue))
    {
        return false;
    }

    if (h_ptr->m_eventingType != NoEvents)
    {
        HStateVariableEvent event(this, oldValue, newValue);
        lock.unlock();
        emit valueChanged(event);
    }

    return true;
}

}
}
