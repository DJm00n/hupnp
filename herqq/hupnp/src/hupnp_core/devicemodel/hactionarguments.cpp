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

#include "hactionarguments.h"
#include "hactionarguments_p.h"

#include "hstatevariable.h"

#include <QMetaType>

static bool registerMetaTypes()
{
    qRegisterMetaType<Herqq::Upnp::HActionArgument>("Herqq::Upnp::HActionArgument");
    qRegisterMetaType<Herqq::Upnp::HActionArguments>("Herqq::Upnp::HActionArguments");
    qRegisterMetaType<Herqq::Upnp::HActionArgument>("Herqq::Upnp::HActionArgument");
    qRegisterMetaType<Herqq::Upnp::HActionArguments>("Herqq::Upnp::HActionArguments");

    return true;
}

static bool test = registerMetaTypes();

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HActionArgument
 *******************************************************************************/
HActionArgument::HActionArgument() :
    m_name(), m_stateVariable(), m_value()
{
}

HActionArgument::HActionArgument(const HActionArgument& other) :
    m_name(other.m_name), m_stateVariable(other.m_stateVariable),
    m_value(other.m_value)
{
}

HActionArgument& HActionArgument::operator=(
    const HActionArgument& other)
{
    m_name = other.m_name;
    m_stateVariable = other.m_stateVariable;
    m_value = other.m_value;

    return *this;
}

HActionArgument::~HActionArgument()
{
}

void HActionArgument::init(
    const QString& name, HStateVariable* stateVariable)
{
    if (name.isEmpty() || (!name[0].isLetterOrNumber() && name[0] != '_'))
    {
        return;
    }

    QString::const_iterator ci = name.constBegin();
    for(; ci != name.constEnd(); ++ci)
    {
        QChar c = *ci;
        if (!c.isLetterOrNumber() && c != '_' && c != '.')
        {
            return;
        }
    }

    m_name = name;
    m_stateVariable = stateVariable;
}

QString HActionArgument::name() const
{
    return m_name;
}

HStateVariable* HActionArgument::relatedStateVariable() const
{
    return m_stateVariable;
}

HUpnpDataTypes::DataType HActionArgument::dataType() const
{
    if (m_stateVariable)
    {
        return m_stateVariable->dataType();
    }
    else
    {
        return HUpnpDataTypes::Undefined;
    }
}

QVariant HActionArgument::value() const
{
    return m_value;
}

bool HActionArgument::setValue(const QVariant& value)
{
    QVariant convertedValue;
    if (isValid() && m_stateVariable->isValidValue(value, &convertedValue))
    {
        m_value = convertedValue;
        return true;
    }

    return false;
}

bool HActionArgument::isValid() const
{
    return !m_name.isEmpty();
}

bool HActionArgument::operator!() const
{
    return !isValid();
}

QString HActionArgument::toString() const
{
    return QString("%1: %2").arg(
             name(),
             dataType() == HUpnpDataTypes::uri ? value().toUrl().toString() :
             value().toString());
}

bool HActionArgument::isValidValue(const QVariant& value)
{
    return isValid() && m_stateVariable->isValidValue(value);
}

/*******************************************************************************
 * HActionArguments
 *******************************************************************************/
HActionArguments::HActionArguments() :
    h_ptr(new HActionArgumentsPrivate())
{
}

HActionArguments::HActionArguments(
    const QVector<HActionArgument*>& args) :
        h_ptr(new HActionArgumentsPrivate(args))
{
}

HActionArguments::HActionArguments(
    const QHash<QString, HActionArgument*>& args) :
        h_ptr(new HActionArgumentsPrivate(args))
{
}

HActionArguments::~HActionArguments()
{
    delete h_ptr;
}

HActionArguments::HActionArguments(const HActionArguments& other) :
    h_ptr(new HActionArgumentsPrivate(*other.h_ptr))
{
}

HActionArguments& HActionArguments::operator=(
    const HActionArguments& other)
{
    HActionArgumentsPrivate* newHptr =
        new HActionArgumentsPrivate(*other.h_ptr);

    delete h_ptr;
    h_ptr = newHptr;

    return *this;
}

bool HActionArguments::contains(const QString& argumentName) const
{
    return h_ptr->m_arguments.contains(argumentName);
}

const HActionArgument* HActionArguments::get(qint32 index) const
{
    return h_ptr->m_argumentsOrdered.at(index);
}

HActionArgument* HActionArguments::get(qint32 index)
{
    return h_ptr->m_argumentsOrdered.at(index);
}

const HActionArgument* HActionArguments::get(const QString& argumentName) const
{
    return h_ptr->m_arguments.value(argumentName);
}

HActionArgument* HActionArguments::get(const QString& argumentName)
{
    return h_ptr->m_arguments.value(argumentName);
}

HActionArguments::const_iterator HActionArguments::constBegin() const
{
    return h_ptr->m_argumentsOrdered.constBegin();
}

HActionArguments::const_iterator HActionArguments::constEnd() const
{
    return h_ptr->m_argumentsOrdered.constEnd();
}

HActionArguments::iterator HActionArguments::begin()
{
    return h_ptr->m_argumentsOrdered.begin();
}

HActionArguments::iterator HActionArguments::end()
{
    return h_ptr->m_argumentsOrdered.end();
}

HActionArguments::const_iterator HActionArguments::begin() const
{
    return h_ptr->m_argumentsOrdered.begin();
}

HActionArguments::const_iterator HActionArguments::end() const
{
    return h_ptr->m_argumentsOrdered.end();
}

qint32 HActionArguments::size() const
{
    return h_ptr->m_argumentsOrdered.size();
}

HActionArgument* HActionArguments::operator[](qint32 index)
{
    return h_ptr->m_argumentsOrdered.at(index);
}

const HActionArgument* HActionArguments::operator[](qint32 index) const
{
    return h_ptr->m_argumentsOrdered.at(index);
}

HActionArgument* HActionArguments::operator[](const QString& argName)
{
    return h_ptr->m_arguments.value(argName);
}

const HActionArgument* HActionArguments::operator[](const QString& argName) const
{
    return h_ptr->m_arguments.value(argName);
}

QList<QString> HActionArguments::names() const
{
    return h_ptr->m_arguments.keys();
}

QString HActionArguments::toString() const
{
    QString retVal;

    HActionArguments::const_iterator ci = constBegin();
    for (; ci != constEnd(); ++ci)
    {
        retVal.append((*ci)->toString()).append("\n");
    }

    return retVal;
}

void swap(HActionArguments& a, HActionArguments& b)
{
    std::swap(a.h_ptr, b.h_ptr);
}

}
}
