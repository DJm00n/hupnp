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

#include "hasyncop.h"

#include "../../utils/hmisc_utils_p.h"

#include <QString>

namespace Herqq
{

namespace Upnp
{

HAsyncOp::HAsyncOp() :
    m_id(QUuid::createUuid()),
    m_waitTimeout(-1), m_waitCode(WaitSuccess), m_returnValue(0), 
    m_userData(0)
{
}

HAsyncOp::~HAsyncOp()
{
}

HAsyncOp::HAsyncOp(const HAsyncOp& other) :
    m_id(other.m_id), m_waitTimeout(other.m_waitTimeout),
    m_waitCode(other.m_waitCode), m_returnValue(other.m_returnValue),
    m_userData(other.m_userData)
{
}

void HAsyncOp::setUserData(void* userData)
{
    m_userData = userData;
}

volatile void* HAsyncOp::userData() const
{
    return m_userData;
}

bool operator==(const HAsyncOp& arg1, const HAsyncOp& arg2)
{
    return arg1.m_id == arg2.m_id;
}

bool operator!=(const HAsyncOp& arg1, const HAsyncOp& arg2)
{
    return !(arg1 == arg2);
}

quint32 qHash(const HAsyncOp& key)
{
    QByteArray data = key.id().toString().toLocal8Bit();
    return hash(data.constData(), data.size());
}

}
}
