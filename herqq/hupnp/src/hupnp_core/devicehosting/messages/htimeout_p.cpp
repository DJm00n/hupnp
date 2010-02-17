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

#include "htimeout_p.h"

namespace Herqq
{

namespace Upnp
{

HTimeout::HTimeout() :
    m_value(-1)
{
}

HTimeout::HTimeout(qint32 timeout) :
    m_value(timeout < 0 ? -1 : timeout)
{
}

HTimeout::HTimeout(const QString& timeout) :
    m_value(-1)
{
    if (timeout.compare("infinite", Qt::CaseInsensitive) != 0)
    {
        QString tmp = timeout;
        if (timeout.startsWith("Second-", Qt::CaseInsensitive))
        {
            tmp = timeout.mid(7);
        }

        bool ok = false;
        qint32 tmpValue = tmp.toInt(&ok);
        if (ok)
        {
            m_value = tmpValue;
        }
    }
}

HTimeout::~HTimeout()
{
}

HTimeout& HTimeout::operator=(qint32 value)
{
    HTimeout copy(value);
    *this = copy;
    return *this;
}

HTimeout& HTimeout::operator=(const QString& value)
{
    HTimeout copy(value);
    *this = copy;
    return *this;
}

qint32 HTimeout::value() const
{
    return m_value;
}

QString HTimeout::toString() const
{
    return QString("Second-%1").arg(
        m_value < 0 ? "infinite" : QString::number(m_value));
}

bool HTimeout::isInfinite () const
{
    return m_value == -1;
}

bool operator==(const HTimeout& obj1, const HTimeout& obj2)
{
    return obj1.value() == obj2.value();
}

bool operator!=(const HTimeout& obj1, const HTimeout& obj2)
{
    return !(obj1 == obj2);
}

}
}
