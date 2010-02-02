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
#include "ssdp_messageheader_objects_p.h"

#include "../../../../core/include/HMiscUtils"

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HSid
 ******************************************************************************/
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

QUuid HSid::value() const
{
    return m_value;
}

QString HSid::toString() const
{
    return QString("uuid:%1").arg(m_value.toString().remove('{').remove('}'));
}

bool HSid::isNull() const
{
    return m_value.isNull();
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

/*******************************************************************************
 * HNt
 ******************************************************************************/
HNt::HNt() :
    m_typeValue   (qMakePair(Type_Undefined   , QString(""))),
    m_subTypeValue(qMakePair(SubType_Undefined, QString("")))
{
}

HNt::HNt(const QString& type) :
    m_typeValue   (qMakePair(Type_Undefined, QString(""))),
    m_subTypeValue(qMakePair(SubType_Undefined, QString("")))
{
    if (type.compare("upnp:event", Qt::CaseInsensitive) == 0)
    {
        m_typeValue.first  = Type_UpnpEvent;
        m_typeValue.second = "upnp:event";
    }
}

HNt::HNt(const QString& type, const QString& subtype) :
    m_typeValue   (qMakePair(Type_Undefined, QString(""))),
    m_subTypeValue(qMakePair(SubType_Undefined, QString("")))
{
    if (type.compare("upnp:event", Qt::CaseInsensitive) == 0)
    {
        m_typeValue.first  = Type_UpnpEvent;
        m_typeValue.second = "upnp:event";
    }

    if (subtype.compare("upnp:propchange", Qt::CaseInsensitive) == 0)
    {
        m_subTypeValue.first  = SubType_UpnpPropChange;
        m_subTypeValue.second = "upnp:propchange";
    }
}

HNt::HNt(Type type) :
    m_typeValue   (qMakePair(type, toString(type))),
    m_subTypeValue(qMakePair(SubType_Undefined, QString("")))
{
}

HNt::HNt(Type type, SubType subType) :
    m_typeValue   (qMakePair(type, toString(type))),
    m_subTypeValue(qMakePair(subType, toString(subType)))
{
}

HNt::~HNt()
{
}

HNt& HNt::operator=(const QString& nt)
{
    HNt copy(nt);
    *this = copy;
    return *this;
}

QString HNt::typeToString() const
{
    return m_typeValue.second;
}

QString HNt::subTypeToString() const
{
    return m_subTypeValue.second;
}

HNt::Type HNt::type() const
{
    return m_typeValue.first;
}

HNt::SubType HNt::subType() const
{
    return m_subTypeValue.first;
}

QString HNt::toString(Type type)
{
    switch(type)
    {
    case Type_Undefined:
        return "";
    case Type_UpnpEvent:
        return "upnp:event";
    }

    return "";
}

QString HNt::toString(SubType subType)
{
    switch(subType)
    {
    case SubType_Undefined:
        return "";
    case SubType_UpnpPropChange:
        return "upnp:propchange";
    }

    return "";
}

/*******************************************************************************
 * HTimeout
 ******************************************************************************/
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
