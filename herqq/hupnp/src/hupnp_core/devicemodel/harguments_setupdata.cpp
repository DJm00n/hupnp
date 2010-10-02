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

#include "harguments_setupdata.h"
#include "../general/hupnp_global_p.h"

#include <QtCore/QSet>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HArgumentSetup
 ******************************************************************************/
HArgumentSetup::HArgumentSetup() :
    m_name(), m_type(), m_relatedStateVariable()
{
}

HArgumentSetup::HArgumentSetup(
    const QString& name, const HStateVariableSetup& svSetup, Type type) :
        m_name(name), m_type(type), m_relatedStateVariable(svSetup)
{
}

bool HArgumentSetup::setName(const QString& name, QString* err)
{
    if (verifyName(name, err))
    {
        m_name = name;
        return true;
    }

    return false;
}

/*******************************************************************************
 * HArgumentsSetupData
 ******************************************************************************/
HArgumentsSetupData::HArgumentsSetupData() :
    m_argumentSetupData()
{
}

bool HArgumentsSetupData::insert(const HArgumentSetup& setupInfo)
{
    if (m_argumentSetupData.contains(setupInfo.name()))
    {
        return false;
    }

    m_argumentSetupData.insert(setupInfo.name(), setupInfo);
    return true;
}

bool HArgumentsSetupData::remove(const QString& actionName)
{
    if (m_argumentSetupData.contains(actionName))
    {
        m_argumentSetupData.remove(actionName);
        return true;
    }

    return false;
}

HArgumentSetup HArgumentsSetupData::get(const QString& name) const
{
    return m_argumentSetupData.value(name);
}

QSet<QString> HArgumentsSetupData::names() const
{
    return m_argumentSetupData.keys().toSet();
}

}
}
