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

#include <QtCore/QUrl>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HActionArgument
 *******************************************************************************/
HActionArgument::HActionArgument() :
    m_name(), m_stateVariableInfo(), m_value()
{
}

HActionArgument::HActionArgument(
    const QString& name, const HStateVariableInfo& stateVariableInfo)
{
    if (name.isEmpty() || (!name[0].isLetterOrNumber() && name[0] != '_'))
    {
        return;
    }
    else if (!stateVariableInfo.isValid())
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
    m_value = stateVariableInfo.defaultValue();
    m_stateVariableInfo = stateVariableInfo;
}

HActionArgument::HActionArgument(const HActionArgument& other) :
    m_name(), m_stateVariableInfo(), m_value()
{
    Q_ASSERT(&other != this);
    m_name = other.m_name;
    m_value = other.m_value;
    m_stateVariableInfo = other.m_stateVariableInfo;
}

HActionArgument& HActionArgument::operator=(const HActionArgument& other)
{
    Q_ASSERT(&other != this);

    m_name = other.m_name;
    m_stateVariableInfo = other.m_stateVariableInfo;
    m_value = other.m_value;

    return *this;
}

HActionArgument::~HActionArgument()
{
}

QString HActionArgument::name() const
{
    return m_name;
}

const HStateVariableInfo& HActionArgument::relatedStateVariable() const
{
    return m_stateVariableInfo;
}

HUpnpDataTypes::DataType HActionArgument::dataType() const
{
    return m_stateVariableInfo.dataType();
}

QVariant HActionArgument::value() const
{
    return m_value;
}

bool HActionArgument::setValue(const QVariant& value)
{
    QVariant convertedValue;
    if (isValid() && m_stateVariableInfo.isValidValue(value, &convertedValue))
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
    return isValid() && m_stateVariableInfo.isValidValue(value);
}

bool operator==(const HActionArgument& arg1, const HActionArgument& arg2)
{
    return arg1.m_name == arg2.m_name &&
           arg1.m_value == arg2.m_value &&
           arg1.m_stateVariableInfo == arg2.m_stateVariableInfo;
}

bool operator!=(const HActionArgument& arg1, const HActionArgument& arg2)
{
    return !(arg1 == arg2);
}

/*******************************************************************************
 * HActionArgumentsPrivate
 *******************************************************************************/
HActionArgumentsPrivate::HActionArgumentsPrivate()
{
}

HActionArgumentsPrivate::HActionArgumentsPrivate(
    const QVector<HActionArgument*>& args)
{
    QVector<HActionArgument*>::const_iterator ci = args.constBegin();

    for (; ci != args.constEnd(); ++ci)
    {
        Q_ASSERT_X(*ci, H_AT, "A provided action argument cannot be null");
        m_argumentsOrdered.push_back(*ci);
        m_arguments[(*ci)->name()] = *ci;
    }
}

HActionArgumentsPrivate::~HActionArgumentsPrivate()
{
    qDeleteAll(m_argumentsOrdered);
}

HActionArgumentsPrivate::HActionArgumentsPrivate(
    const HActionArgumentsPrivate& other)
{
    QVector<HActionArgument*>::const_iterator ci =
        other.m_argumentsOrdered.constBegin();

    for (; ci != other.m_argumentsOrdered.constEnd(); ++ci)
    {
        HActionArgument* arg = new HActionArgument(**ci);
        m_argumentsOrdered.push_back(arg);
        m_arguments[arg->name()] = arg;
    }
}

HActionArgumentsPrivate& HActionArgumentsPrivate::operator=(
    const HActionArgumentsPrivate& other)
{
    qDeleteAll(m_argumentsOrdered);
    m_arguments.clear(); m_argumentsOrdered.clear();

    QVector<HActionArgument*>::const_iterator ci =
        other.m_argumentsOrdered.constBegin();

    for (; ci != other.m_argumentsOrdered.constEnd(); ++ci)
    {
        HActionArgument* arg = new HActionArgument(**ci);
        m_argumentsOrdered.push_back(arg);
        m_arguments[arg->name()] = arg;
    }

    return *this;
}

/*******************************************************************************
 * HActionArguments
 *******************************************************************************/
HActionArguments::HActionArguments() :
    h_ptr(new HActionArgumentsPrivate())
{
}

HActionArguments::HActionArguments(const QVector<HActionArgument*>& args) :
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
    Q_ASSERT(&other != this);
}

HActionArguments& HActionArguments::operator=(const HActionArguments& other)
{
    Q_ASSERT(&other != this);
    delete h_ptr;
    h_ptr = new HActionArgumentsPrivate(*other.h_ptr);
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

bool HActionArguments::isEmpty() const
{
    return h_ptr->m_argumentsOrdered.isEmpty();
}

void HActionArguments::clear()
{
    qDeleteAll(h_ptr->m_argumentsOrdered);

    h_ptr->m_arguments.clear();
    h_ptr->m_argumentsOrdered.clear();
}

bool HActionArguments::remove(const QString& name)
{
    if (h_ptr->m_arguments.contains(name))
    {
        h_ptr->m_arguments.remove(name);
        HActionArguments::iterator it = h_ptr->m_argumentsOrdered.begin();
        for(; it != h_ptr->m_argumentsOrdered.end(); ++it)
        {
            if ((*it)->name() == name)
            {
                delete *it;
                h_ptr->m_argumentsOrdered.erase(it);
                return true;
            }
        }
    }

    return false;
}

bool HActionArguments::append(HActionArgument *arg)
{
    if (!arg)
    {
        return false;
    }
    else if (h_ptr->m_arguments.contains(arg->name()))
    {
        return false;
    }

    h_ptr->m_arguments.insert(arg->name(), arg);
    h_ptr->m_argumentsOrdered.append(arg);

    return true;
}

QVariant HActionArguments::value(const QString& name, bool* ok) const
{
    QVariant retVal;

    HActionArgument* arg = h_ptr->m_arguments.value(name);
    if (arg)
    {
        retVal = arg->value();
    }

    if (ok) { *ok = arg; }

    return retVal;
}

bool HActionArguments::setValue(const QString& name, const QVariant& value)
{
    HActionArgument* arg = h_ptr->m_arguments.value(name);
    if (arg)
    {
        return arg->setValue(value);
    }
    return false;
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

bool operator==(const HActionArguments& arg1, const HActionArguments& arg2)
{
    if (arg1.h_ptr->m_argumentsOrdered.size() !=
        arg2.h_ptr->m_argumentsOrdered.size())
    {
        return false;
    }

    qint32 size = arg1.h_ptr->m_argumentsOrdered.size();
    for(qint32 i = 0; i < size; ++i)
    {
        if (*arg1.h_ptr->m_argumentsOrdered.at(i) !=
            *arg2.h_ptr->m_argumentsOrdered.at(i))
        {
            return false;
        }
    }

    return true;
}

bool operator!=(const HActionArguments& arg1, const HActionArguments& arg2)
{
    return !(arg1 == arg2);
}

}
}
