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

#ifndef SSDP_MESSAGEHEADER_OBJECTS_P_H
#define SSDP_MESSAGEHEADER_OBJECTS_P_H

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../../../../core/include/HGlobal"

#include <QPair>
#include <QUuid>
#include <QString>

namespace Herqq
{

namespace Upnp
{

//
//
//
class HSid
{
friend quint32 qHash(const HSid &key);

private:

    QUuid m_value;

public:

    HSid();
    explicit HSid (const QUuid&  sid);
    explicit HSid (const QString& sid);
    HSid          (const HSid& other);

    ~HSid();

    HSid& operator=(const HSid& other);
    HSid& operator=(const QUuid& other);
    HSid& operator=(const QString& other);

    QUuid   value   () const;
    QString toString() const;
    bool    isNull  () const;
};

bool operator==(const HSid& sid1, const HSid& sid2);
bool operator!=(const HSid& sid1, const HSid& sid2);

quint32 qHash(const HSid& key);

//
//
//
class HNt
{
public:

    enum Type
    {
        Type_Undefined = 0,
        Type_UpnpEvent = 1
    };

    enum SubType
    {
        SubType_Undefined      = 0,
        SubType_UpnpPropChange = 1
    };

private:

    QPair<Type, QString>    m_typeValue;
    QPair<SubType, QString> m_subTypeValue;

public:

    HNt ();

    explicit HNt (const QString& type);
    explicit HNt (const QString& type, const QString& subTybe);
    explicit HNt (Type type);

    HNt(Type type, SubType subType);

    ~HNt();

    HNt& operator=(const QString& nt);

    QString typeToString() const;
    Type    type        () const;

    QString subTypeToString() const;
    SubType subType        () const;

    static QString toString(Type type);
    static QString toString(SubType subType);
};

//
//
//
class HTimeout
{
private:

    qint32 m_value;

public:

    HTimeout();
    explicit HTimeout (qint32 timeout);
    explicit HTimeout (const QString& timeout);

    ~HTimeout();

    HTimeout& operator=(qint32 value);
    HTimeout& operator=(const QString& value);

    qint32 value    () const;
    QString toString() const;
    bool isInfinite () const;
};

bool operator==(const HTimeout& obj1, const HTimeout& obj2);
bool operator!=(const HTimeout& obj1, const HTimeout& obj2);

}
}

#endif /* SSDP_MESSAGEHEADER_OBJECTS_P_H */
