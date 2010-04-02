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

#include "hserviceid.h"

#include "./../../utils/hlogger_p.h"

#include <QString>
#include <QStringList>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HServiceIdPrivate
 ******************************************************************************/
class HServiceIdPrivate
{
public:

    QString m_suffix;
    QStringList m_elements;

public:

    HServiceIdPrivate() :
        m_suffix(), m_elements()
    {
    }

    HServiceIdPrivate(const QString& arg) :
        m_suffix(), m_elements()
    {
        HLOG(H_AT, H_FUN);

        QStringList tmp = arg.simplified().split(":");
        if (tmp.size() < 4)
        {
            return;
        }

        if (tmp[0] != "urn")
        {
            return;
        }

        if (tmp[1] != "upnp-org")
        {
            tmp[1] = tmp[1].replace('.', '-');
            if (tmp[1].isEmpty() || !tmp[1].contains('-'))
            {
                HLOG_WARN(QString(
                    "Invalid service identifier [%1]: the URN is invalid").arg(
                        arg));

                return;
            }
        }

        if (tmp[2] != "serviceId")
        {
            HLOG_WARN_NONSTD(QString("Invalid service identifier [%1]: .").arg(arg));
            // at least some Intel software fails to specify this right
        }

        if (tmp[3].isEmpty())
        {
            HLOG_WARN(QString("Invalid service identifier [%1].").arg(arg));
            return;
        }

        m_suffix = tmp[3];
        for (qint32 i = 4; i < tmp.size(); ++i)
        {
            m_suffix.append(':').append(tmp[i]);
        }

        m_elements = tmp;
    }

    ~HServiceIdPrivate()
    {
    }
};

/*******************************************************************************
 * HServiceId
 ******************************************************************************/
HServiceId::HServiceId() :
    h_ptr(new HServiceIdPrivate())
{
}

HServiceId::HServiceId(const QString& serviceId) :
    h_ptr(new HServiceIdPrivate(serviceId))
{
}

HServiceId::HServiceId(const HServiceId& other) :
    h_ptr(new HServiceIdPrivate(*other.h_ptr))
{
}

HServiceId& HServiceId::operator=(const HServiceId& other)
{
    HServiceIdPrivate* newHptr = new HServiceIdPrivate(*other.h_ptr);
    delete h_ptr;
    h_ptr = newHptr;

    return *this;
}

HServiceId::~HServiceId()
{
    delete h_ptr;
}

bool HServiceId::isValid() const
{
    return !h_ptr->m_suffix.isEmpty();
}

bool HServiceId::isStandardType() const
{
    if (!isValid())
    {
        return false;
    }

    return h_ptr->m_elements[1] == "upnp-org";
}

QString HServiceId::urn(bool completeUrn) const
{
    if (!isValid())
    {
        return QString();
    }

    QString retVal;
    if (completeUrn)
    {
        retVal.append("urn:");
    }

    retVal.append(h_ptr->m_elements[1]);

    return retVal;
}

QString HServiceId::suffix() const
{
    if (!isValid())
    {
        return QString();
    }

    return h_ptr->m_suffix;
}

QString HServiceId::toString() const
{
    return h_ptr->m_elements.join(":");
}

bool operator==(const HServiceId& sid1, const HServiceId& sid2)
{
    return sid1.toString() == sid2.toString();
}

bool operator!=(const HServiceId& sid1, const HServiceId& sid2)
{
    return !(sid1 == sid2);
}

}
}
