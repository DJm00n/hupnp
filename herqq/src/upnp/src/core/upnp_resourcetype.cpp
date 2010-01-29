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

#include "upnp_resourcetype.h"

#include "../../../utils/src/logger_p.h"
#include "../../../core/include/HMiscUtils"

#include <QString>
#include <QByteArray>
#include <QStringList>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HResourceTypePrivate
 ******************************************************************************/
class HResourceTypePrivate
{
public:

    QString     m_resourceAsStr;
    QStringList m_resourceElements;

public:

    HResourceTypePrivate() :
        m_resourceAsStr(), m_resourceElements()
    {
    }

    HResourceTypePrivate(const QString& arg) :
        m_resourceAsStr(), m_resourceElements()
    {
        HLOG(H_AT, H_FUN);

        QStringList tmp = arg.simplified().split(":");
        if (tmp.size() != 5)
        {
            return;
        }

        if (tmp[0] != "urn")
        {
            return;
        }

        bool ok = false;
        tmp[4].toInt(&ok);

        if (!ok)
        {
            return;
        }

        if (tmp[1] != "schemas-upnp-org")
        {
            tmp[1] = tmp[1].replace('.', '-');
        }

        m_resourceAsStr    = tmp.join(":");
        m_resourceElements = tmp;
    }
};

/*******************************************************************************
 * HResourceType
 ******************************************************************************/
HResourceType::HResourceType() :
    h_ptr(new HResourceTypePrivate())
{
}

HResourceType::HResourceType(const QString& resourceTypeAsStr) :
    h_ptr(new HResourceTypePrivate(resourceTypeAsStr))
{
}

HResourceType::HResourceType(const HResourceType& other) :
    h_ptr(new HResourceTypePrivate(*other.h_ptr))
{
}

HResourceType& HResourceType::operator =(const HResourceType& other)
{
    HResourceTypePrivate* newHptr = new HResourceTypePrivate(*other.h_ptr);

    delete h_ptr;
    h_ptr = newHptr;

    return *this;
}

HResourceType::~HResourceType()
{
    delete h_ptr;
}

bool HResourceType::isValid() const
{
    return !h_ptr->m_resourceAsStr.isEmpty();
}

bool HResourceType::isStandardType() const
{
    if (!isValid())
    {
        return false;
    }

    return h_ptr->m_resourceElements[1] == "schemas-upnp-org";
}

QString HResourceType::resourceUrn(bool completeUrn) const
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

    retVal.append(h_ptr->m_resourceElements[1]);

    return retVal;
}

QString HResourceType::type() const
{
    if (!isValid())
    {
        return QString();
    }

    return h_ptr->m_resourceElements[2];
}

QString HResourceType::completeType(bool includeVersion) const
{
    if (!isValid())
    {
        return QString();
    }

    return type().append(":").append(typeSuffix(includeVersion));
}

QString HResourceType::completeTypeWithUrn(bool includeVersion) const
{
    if (!isValid())
    {
        return QString();
    }

    return resourceUrn(true).append(':').append(completeType(includeVersion));
}

QString HResourceType::typeSuffix(bool includeVersion) const
{
    if (!isValid())
    {
        return QString();
    }

    return includeVersion ?
        QString("%1:%2").arg(
            h_ptr->m_resourceElements[3],
            h_ptr->m_resourceElements[4]) :
                h_ptr->m_resourceElements[3];
}

qint32 HResourceType::version() const
{
    if (!isValid())
    {
        return -1;
    }

    return h_ptr->m_resourceElements[4].toInt();
}

QString HResourceType::toString() const
{
    return h_ptr->m_resourceAsStr;
}

bool operator==(const HResourceType& arg1, const HResourceType& arg2)
{
    return arg1.toString() == arg2.toString();
}

bool operator!=(const HResourceType& arg1, const HResourceType& arg2)
{
    return !(arg1 == arg2);
}

quint32 qHash(const HResourceType& key)
{
    QByteArray data = key.toString().toLocal8Bit();
    return hash(data.constData(), data.size());
}

}
}
