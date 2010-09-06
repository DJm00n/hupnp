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

#include "hwritable_statevariable.h"
#include "hstatevariable_p.h"

#include <QtCore/QMutexLocker>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HWritableStateVariablePrivate
 ******************************************************************************/
class HWritableStateVariablePrivate :
    public HStateVariablePrivate
{
public:

};

/*******************************************************************************
 * HWritableStateVariable
 ******************************************************************************/
HWritableStateVariable::HWritableStateVariable(
    HService* parent) :
        HStateVariable(*new HWritableStateVariablePrivate(), parent)
{
}

HWritableStateVariable::~HWritableStateVariable()
{
}

bool HWritableStateVariable::setValue(const QVariant& newValue)
{
    return HStateVariable::setValue(newValue);
}

/*******************************************************************************
 * HStateVariableLocker
 ******************************************************************************/
HStateVariableLocker::HStateVariableLocker(
    HWritableStateVariable* var) :
        m_stateVariable(var),
        m_locker(new QMutexLocker(&m_stateVariable->h_ptr->m_valueMutex))
{
    Q_ASSERT(m_stateVariable);
}

HStateVariableLocker::~HStateVariableLocker()
{
    delete m_locker;
}

void HStateVariableLocker::unlock()
{
    m_locker->unlock();
}

void HStateVariableLocker::relock()
{
    m_locker->relock();
}

}
}
