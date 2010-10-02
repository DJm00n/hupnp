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

#include "hstatevariables_setupdata.h"

#include "../general/hupnp_global_p.h"

#include <QtCore/QSet>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HStateVariableSetupPrivate
 ******************************************************************************/
class HStateVariableSetupPrivate
{

public:

    QString m_name;
    HUpnpDataTypes::DataType m_dt;
    qint32 m_version;
    HInclusionRequirement m_inclusionRequirement;
    qint32 m_maxRate;

    HStateVariableSetupPrivate() :
        m_name(), m_dt(HUpnpDataTypes::Undefined), m_version(0),
        m_inclusionRequirement(), m_maxRate(-1)
    {
    }
};

/*******************************************************************************
 * HStateVariableSetup
 ******************************************************************************/
HStateVariableSetup::HStateVariableSetup() :
    h_ptr(new HStateVariableSetupPrivate())
{
}

HStateVariableSetup::HStateVariableSetup(
    const QString& name, HUpnpDataTypes::DataType dt, HInclusionRequirement ireq) :
        h_ptr(new HStateVariableSetupPrivate())
{
    h_ptr->m_dt = dt;
    h_ptr->m_version = 1;
    h_ptr->m_inclusionRequirement = ireq;

    setName(name);
}

HStateVariableSetup::HStateVariableSetup(
    const QString& name, HUpnpDataTypes::DataType dt, qint32 version,
    HInclusionRequirement ireq) :
        h_ptr(new HStateVariableSetupPrivate())
{
    h_ptr->m_dt = dt;
    h_ptr->m_version = version;
    h_ptr->m_inclusionRequirement = ireq;

    setName(name);
}

HStateVariableSetup::HStateVariableSetup(const HStateVariableSetup& other) :
    h_ptr(0)
{
    Q_ASSERT(&other != this);
    h_ptr = new HStateVariableSetupPrivate(*other.h_ptr);
}

HStateVariableSetup& HStateVariableSetup::operator=(
    const HStateVariableSetup& other)
{
    Q_ASSERT(&other != this);

    HStateVariableSetupPrivate* newHptr =
        new HStateVariableSetupPrivate(*other.h_ptr);

    delete h_ptr;
    h_ptr = newHptr;

    return *this;
}

HStateVariableSetup::~HStateVariableSetup()
{
    delete h_ptr;
}

bool HStateVariableSetup::setName(const QString& name, QString* err)
{
    if (verifyName(name, err))
    {
        h_ptr->m_name = name;
        return true;
    }

    return false;
}

HInclusionRequirement HStateVariableSetup::inclusionRequirement() const
{
    return h_ptr->m_inclusionRequirement;
}

bool HStateVariableSetup::isValid() const
{
    return !h_ptr->m_name.isEmpty() &&
            h_ptr->m_version > 0 &&
            h_ptr->m_inclusionRequirement != InclusionRequirementUnknown;
}

qint32 HStateVariableSetup::maxEventRate() const
{
    return h_ptr->m_maxRate;
}

QString HStateVariableSetup::name() const
{
    return h_ptr->m_name;
}

HUpnpDataTypes::DataType HStateVariableSetup::dataType() const
{
    return h_ptr->m_dt;
}

qint32 HStateVariableSetup::version() const
{
    return h_ptr->m_version;
}

void HStateVariableSetup::setDataType(HUpnpDataTypes::DataType dt)
{
    h_ptr->m_dt = dt;
}

void HStateVariableSetup::setMaxEventRate(qint32 arg)
{
    h_ptr->m_maxRate = arg < 0 ? -1 : arg;
}

void HStateVariableSetup::setInclusionRequirement(HInclusionRequirement arg)
{
    h_ptr->m_inclusionRequirement = arg;
}

void HStateVariableSetup::setVersion(qint32 version)
{
    h_ptr->m_version = version;
}

/*******************************************************************************
 * HStateVariablesSetupData
 ******************************************************************************/
HStateVariablesSetupData::HStateVariablesSetupData(
    DefaultInclusionPolicy policy) :
        m_setupData(), m_defaultInclusionPolicy(policy)
{
}

bool HStateVariablesSetupData::insert(const HStateVariableSetup& setupData)
{
    if (m_setupData.contains(setupData.name()))
    {
        return false;
    }

    m_setupData.insert(setupData.name(), setupData);
    return true;
}

bool HStateVariablesSetupData::remove(const QString& stateVarName)
{
    if (m_setupData.contains(stateVarName))
    {
        m_setupData.remove(stateVarName);
        return true;
    }

    return false;
}

HStateVariablesSetupData::DefaultInclusionPolicy
    HStateVariablesSetupData::defaultInclusionPolicy() const
{
    return m_defaultInclusionPolicy;
}

bool HStateVariablesSetupData::setInclusionRequirement(
    const QString& name, HInclusionRequirement incReq)
{
    if (m_setupData.contains(name))
    {
        HStateVariableSetup setupInfo = m_setupData.value(name);
        setupInfo.setInclusionRequirement(incReq);
        m_setupData.insert(name, setupInfo);
        return true;
    }

    return false;
}

HStateVariableSetup HStateVariablesSetupData::get(
    const QString& stateVarName) const
{
    return m_setupData.value(stateVarName);
}

bool HStateVariablesSetupData::contains(const QString& name) const
{
    return m_setupData.contains(name);
}

QSet<QString> HStateVariablesSetupData::names() const
{
    return m_setupData.keys().toSet();
}

qint32 HStateVariablesSetupData::size() const
{
    return m_setupData.size();
}

bool HStateVariablesSetupData::isEmpty() const
{
    return m_setupData.isEmpty();
}

}
}
