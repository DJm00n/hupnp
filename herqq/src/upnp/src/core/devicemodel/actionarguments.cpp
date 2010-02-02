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

#include "actionarguments.h"
#include "actionarguments_p.h"

#include <QMetaType>
#include <QAtomicInt>

static bool registerMetaTypes()
{
    static QAtomicInt tester(0);

    if (tester.testAndSetAcquire(0, 1))
    {
        qRegisterMetaType<Herqq::Upnp::HActionInputArgument>("Herqq::Upnp::HActionInputArgument");
        qRegisterMetaType<Herqq::Upnp::HActionInputArguments>("Herqq::Upnp::HActionInputArguments");
        qRegisterMetaType<Herqq::Upnp::HActionOutputArgument>("Herqq::Upnp::HActionOutputArgument");
        qRegisterMetaType<Herqq::Upnp::HActionOutputArguments>("Herqq::Upnp::HActionOutputArguments");
    }

    return true;
}

static bool test = registerMetaTypes();

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HActionArgumentPrivate
 *******************************************************************************/
HActionArgumentPrivate::HActionArgumentPrivate() :
    m_name(""), m_stateVariable(0), m_value()
{
    Q_UNUSED(test)
}

HActionArgumentPrivate::HActionArgumentPrivate(
    const QString& name, HStateVariable* stateVariable) :
        m_name(name.trimmed()), m_stateVariable(stateVariable), m_value()
{
    if (m_name.isEmpty() || (!m_name[0].isLetterOrNumber() && m_name[0] != '_'))
    {
        m_name = ""; m_stateVariable = 0;
        return;
    }

    QString::const_iterator ci = m_name.constBegin();
    for(; ci != m_name.constEnd(); ++ci)
    {
        QChar c = *ci;
        if (!c.isLetterOrNumber() && c != '_' && c != '.')
        {
            m_name = ""; m_stateVariable = 0;
            return;
        }
    }
}

/*******************************************************************************
 * HActionInputArgument
 *******************************************************************************/
HActionInputArgument::HActionInputArgument() :
    h_ptr(new HActionArgumentPrivate())
{
}

HActionInputArgument::HActionInputArgument(
    const QString& name, HStateVariable* stateVariable) :
        h_ptr(new HActionArgumentPrivate(name, stateVariable))
{
}

HActionInputArgument::HActionInputArgument(const HActionInputArgument& other) :
    h_ptr(new HActionArgumentPrivate(*other.h_ptr))
{
}

HActionInputArgument::~HActionInputArgument()
{
    delete h_ptr;
}

HActionInputArgument& HActionInputArgument::operator=(
    const HActionInputArgument& other)
{
    HActionArgumentPrivate* newHptr =
        new HActionArgumentPrivate(*other.h_ptr);

    delete h_ptr;
    h_ptr = newHptr;

    return *this;
}

QString HActionInputArgument::name() const
{
    return h_ptr->name();
}

HStateVariable* HActionInputArgument::relatedStateVariable() const
{
    return h_ptr->relatedStateVariable();
}

HUpnpDataTypes::DataType HActionInputArgument::dataType() const
{
    return h_ptr->dataType();
}

QVariant HActionInputArgument::value() const
{
    return h_ptr->value();
}

bool HActionInputArgument::setValue(const QVariant& value)
{
    return h_ptr->setValue(value);
}

bool HActionInputArgument::isValid() const
{
    return h_ptr->isValid();
}

bool HActionInputArgument::operator!() const
{
    return !isValid();
}

QString HActionInputArgument::toString() const
{
    return h_ptr->toString();
}

/*******************************************************************************
 * HActionOutputArgument
 *******************************************************************************/
HActionOutputArgument::HActionOutputArgument() :
    h_ptr(new HActionArgumentPrivate())
{
    qRegisterMetaType<HActionOutputArgument>("Herqq::Upnp::HActionOutputArgument");
}

HActionOutputArgument::HActionOutputArgument(
    const QString& name, HStateVariable* stateVariable) :
        h_ptr(new HActionArgumentPrivate(name, stateVariable))
{
}

HActionOutputArgument::~HActionOutputArgument()
{
    delete h_ptr;
}

HActionOutputArgument::HActionOutputArgument(
    const HActionOutputArgument& other) :
        h_ptr(new HActionArgumentPrivate(*other.h_ptr))
{
}

HActionOutputArgument& HActionOutputArgument::operator=(
    const HActionOutputArgument& other)
{
    HActionArgumentPrivate* newHptr =
        new HActionArgumentPrivate(*other.h_ptr);

    delete h_ptr;
    h_ptr = newHptr;

    return *this;
}

QString HActionOutputArgument::name() const
{
    return h_ptr->name();
}

HStateVariable* HActionOutputArgument::relatedStateVariable() const
{
    return h_ptr->relatedStateVariable();
}

HUpnpDataTypes::DataType HActionOutputArgument::dataType() const
{
    return h_ptr->dataType();
}

QVariant HActionOutputArgument::value() const
{
    return h_ptr->value();
}

bool HActionOutputArgument::setValue(const QVariant& value)
{
    return h_ptr->setValue(value);
}

bool HActionOutputArgument::isValid() const
{
    return h_ptr->isValid();
}

bool HActionOutputArgument::operator!() const
{
    return !isValid();
}

QString HActionOutputArgument::toString() const
{
    return h_ptr->toString();
}

/*******************************************************************************
 * HActionInputArguments
 *******************************************************************************/
HActionInputArguments::HActionInputArguments() :
    h_ptr(new HActionArgumentsPrivate<HActionInputArgument>())
{
}

HActionInputArguments::HActionInputArguments(
    const QList<HActionInputArgument>& args) :
        h_ptr(new HActionArgumentsPrivate<HActionInputArgument>(args))
{
}

HActionInputArguments::HActionInputArguments(
    const QHash<QString, HActionInputArgument>& args) :
        h_ptr(new HActionArgumentsPrivate<HActionInputArgument>(args))
{
}

HActionInputArguments::~HActionInputArguments()
{
    delete h_ptr;
}

HActionInputArguments::HActionInputArguments(const HActionInputArguments& other) :
    h_ptr(new HActionArgumentsPrivate<HActionInputArgument>(*other.h_ptr))
{
}

HActionInputArguments& HActionInputArguments::operator=(
    const HActionInputArguments& other)
{
    HActionArgumentsPrivate<HActionInputArgument>* newHptr =
        new HActionArgumentsPrivate<HActionInputArgument>(*other.h_ptr);

    delete h_ptr;
    h_ptr = newHptr;

    return *this;
}

bool HActionInputArguments::contains(const QString& argumentName) const
{
    return h_ptr->contains(argumentName);
}

const HActionInputArgument* HActionInputArguments::get(qint32 index) const
{
    return h_ptr->get(index);
}

HActionInputArgument* HActionInputArguments::get(qint32 index)
{
    return h_ptr->get(index);
}

const HActionInputArgument* HActionInputArguments::get(const QString& argumentName) const
{
    return h_ptr->get(argumentName);
}

HActionInputArgument* HActionInputArguments::get(const QString& argumentName)
{
    return h_ptr->get(argumentName);
}

HActionInputArguments::const_iterator HActionInputArguments::constBegin() const
{
    return h_ptr->constBegin();
}

HActionInputArguments::const_iterator HActionInputArguments::constEnd() const
{
    return h_ptr->constEnd();
}

HActionInputArguments::iterator HActionInputArguments::begin()
{
    return h_ptr->begin();
}

HActionInputArguments::iterator HActionInputArguments::end()
{
    return h_ptr->end();
}

HActionInputArguments::const_iterator HActionInputArguments::begin() const
{
    return h_ptr->begin();
}

HActionInputArguments::const_iterator HActionInputArguments::end() const
{
    return h_ptr->end();
}

qint32 HActionInputArguments::size() const
{
    return h_ptr->size();
}

HActionInputArgument* HActionInputArguments::operator[](qint32 index)
{
    return h_ptr->operator[](index);
}

const HActionInputArgument* HActionInputArguments::operator[](qint32 index) const
{
    return h_ptr->operator[](index);
}

HActionInputArgument* HActionInputArguments::operator[](const QString& argName)
{
    return h_ptr->operator[](argName);
}

const HActionInputArgument* HActionInputArguments::operator[](const QString& argName) const
{
    return h_ptr->operator[](argName);
}

QList<QString> HActionInputArguments::names() const
{
    return h_ptr->names();
}

QString HActionInputArguments::toString() const
{
    return h_ptr->toString();
}

void swap(HActionInputArguments& a, HActionInputArguments& b)
{
    std::swap(a.h_ptr, b.h_ptr);
}

/*******************************************************************************
 * HActionOutputArguments
 *******************************************************************************/
HActionOutputArguments::HActionOutputArguments() :
    h_ptr(new HActionArgumentsPrivate<HActionOutputArgument>())
{
}

HActionOutputArguments::HActionOutputArguments(
    const QList<HActionOutputArgument>& args) :
        h_ptr(new HActionArgumentsPrivate<HActionOutputArgument>(args))
{
}

HActionOutputArguments::HActionOutputArguments(
    const QHash<QString, HActionOutputArgument>& args) :
        h_ptr(new HActionArgumentsPrivate<HActionOutputArgument>(args))
{
}

HActionOutputArguments::~HActionOutputArguments()
{
    delete h_ptr;
}

HActionOutputArguments::HActionOutputArguments(const HActionOutputArguments& other) :
    h_ptr(new HActionArgumentsPrivate<HActionOutputArgument>(*other.h_ptr))
{
}

HActionOutputArguments& HActionOutputArguments::operator=(
    const HActionOutputArguments& other)
{
    HActionArgumentsPrivate<HActionOutputArgument>* newHptr =
        new HActionArgumentsPrivate<HActionOutputArgument>(*other.h_ptr);

    delete h_ptr;
    h_ptr = newHptr;

    return *this;
}

bool HActionOutputArguments::contains(const QString& argumentName) const
{
    return h_ptr->contains(argumentName);
}

const HActionOutputArgument* HActionOutputArguments::get(qint32 index) const
{
    return h_ptr->get(index);
}

HActionOutputArgument* HActionOutputArguments::get(qint32 index)
{
    return h_ptr->get(index);
}

const HActionOutputArgument* HActionOutputArguments::get(const QString& argumentName) const
{
    return h_ptr->get(argumentName);
}

HActionOutputArgument* HActionOutputArguments::get(const QString& argumentName)
{
    return h_ptr->get(argumentName);
}

HActionOutputArguments::const_iterator HActionOutputArguments::constBegin() const
{
    return h_ptr->constBegin();
}

HActionOutputArguments::const_iterator HActionOutputArguments::constEnd() const
{
    return h_ptr->constEnd();
}

HActionOutputArguments::iterator HActionOutputArguments::begin()
{
    return h_ptr->begin();
}

HActionOutputArguments::iterator HActionOutputArguments::end()
{
    return h_ptr->end();
}

HActionOutputArguments::const_iterator HActionOutputArguments::begin() const
{
    return h_ptr->begin();
}

HActionOutputArguments::const_iterator HActionOutputArguments::end() const
{
    return h_ptr->end();
}

qint32 HActionOutputArguments::size() const
{
    return h_ptr->size();
}

HActionOutputArgument* HActionOutputArguments::operator[](qint32 index)
{
    return h_ptr->operator[](index);
}

const HActionOutputArgument* HActionOutputArguments::operator[](qint32 index) const
{
    return h_ptr->operator[](index);
}

HActionOutputArgument* HActionOutputArguments::operator[](const QString& argName)
{
    return h_ptr->operator[](argName);
}

const HActionOutputArgument* HActionOutputArguments::operator[](const QString& argName) const
{
    return h_ptr->operator[](argName);
}

QList<QString> HActionOutputArguments::names() const
{
    return h_ptr->names();
}

QString HActionOutputArguments::toString() const
{
    return h_ptr->toString();
}

void swap(HActionOutputArguments& a, HActionOutputArguments& b)
{
    std::swap(a.h_ptr, b.h_ptr);
}

}
}
