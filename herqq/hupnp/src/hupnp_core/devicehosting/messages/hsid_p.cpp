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

#include "hsid_p.h"

#include "./../../../utils/hmisc_utils_p.h"

namespace Herqq
{

namespace Upnp
{

HSid::HSid() :
    m_value()
{
}

HSid::HSid(const QUuid& sid) :
    m_value(sid)
{
}

HSid::HSid(const HSid& other) :
    m_value(other.m_value)
{
}

HSid::HSid(const QString& sid) :
    m_value()
{
    if (sid.startsWith("uuid:", Qt::CaseInsensitive))
    {
        m_value = sid.trimmed().mid(5);
    }
    else if (!QUuid(sid).isNull())
    {
        m_value = QUuid(sid);
    }
}

HSid::~HSid()
{
}

HSid& HSid::operator=(const HSid& other)
{
    this->m_value = other.m_value;
    return *this;
}

HSid& HSid::operator=(const QString& other)
{
    HSid copy(other);
    *this = copy;
    return *this;
}

HSid& HSid::operator=(const QUuid& other)
{
    HSid copy(other);
    *this = copy;
    return *this;
}

QString HSid::toString() const
{
    return QString("uuid:%1").arg(m_value.toString().remove('{').remove('}'));
}

bool operator==(const HSid& sid1, const HSid& sid2)
{
    return sid1.value() == sid2.value();
}

bool operator!=(const HSid& sid1, const HSid& sid2)
{
    return !(sid1 == sid2);
}

quint32 qHash(const HSid& key)
{
    QByteArray data = key.value().toString().toLocal8Bit();
    return hash(data.constData(), data.size());
}

}
}
