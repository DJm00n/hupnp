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

#include "hactions_setupdata.h"
#include "../general/hupnp_global_p.h"

#include <QtCore/QSet>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HActionSetup
 ******************************************************************************/
HActionSetup::HActionSetup() :
    m_name(), m_version(0), m_inclusionRequirement(InclusionRequirementUnknown),
    m_actionInvoke()
{
}

HActionSetup::HActionSetup(const QString& name, HInclusionRequirement ireq) :
    m_name(name), m_version(1), m_inclusionRequirement(ireq), m_actionInvoke()
{
}

HActionSetup::HActionSetup(
    const QString& name, qint32 version, HInclusionRequirement ireq) :
        m_name(name), m_version(version), m_inclusionRequirement(ireq),
        m_actionInvoke()
{
}

HActionSetup::HActionSetup(
    const QString& name, const HActionInvoke& invoke,
    HInclusionRequirement ireq) :
        m_name(name), m_version(1), m_inclusionRequirement(ireq),
        m_actionInvoke(invoke)
{
}

HActionSetup::HActionSetup(
    const QString& name, const HActionInvoke& invoke, qint32 version,
    HInclusionRequirement ireq) :
        m_name(name), m_version(version), m_inclusionRequirement(ireq),
        m_actionInvoke(invoke)
{
}

bool HActionSetup::setName(const QString& name, QString* err)
{
    if (verifyName(name, err))
    {
        m_name = name;
        return true;
    }

    return false;
}

/*******************************************************************************
 * HActionsSetupData
 ******************************************************************************/
HActionsSetupData::HActionsSetupData() :
    m_actionSetupInfos()
{
}

bool HActionsSetupData::insert(const HActionSetup& setupInfo)
{
    if (m_actionSetupInfos.contains(setupInfo.name()))
    {
        return false;
    }

    m_actionSetupInfos.insert(setupInfo.name(), setupInfo);
    return true;
}

bool HActionsSetupData::remove(const QString& actionName)
{
    if (m_actionSetupInfos.contains(actionName))
    {
        m_actionSetupInfos.remove(actionName);
        return true;
    }

    return false;
}

HActionSetup HActionsSetupData::get(const QString& actionName) const
{
    return m_actionSetupInfos.value(actionName);
}

bool HActionsSetupData::setInvoke(
    const QString& actionName, const HActionInvoke& actionInvoke)
{
    if (m_actionSetupInfos.contains(actionName))
    {
        HActionSetup setupInfo = m_actionSetupInfos.value(actionName);
        setupInfo.setActionInvoke(actionInvoke);
        m_actionSetupInfos.insert(actionName, setupInfo);
        return true;
    }

    return false;
}

bool HActionsSetupData::setInclusionRequirement(
    const QString& actionName, HInclusionRequirement incReq)
{
    if (m_actionSetupInfos.contains(actionName))
    {
        HActionSetup setupInfo = m_actionSetupInfos.value(actionName);
        setupInfo.setInclusionRequirement(incReq);
        m_actionSetupInfos.insert(actionName, setupInfo);
        return true;
    }

    return false;
}

QSet<QString> HActionsSetupData::names() const
{
    return m_actionSetupInfos.keys().toSet();
}

}
}
