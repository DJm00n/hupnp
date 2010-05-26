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

#ifndef HVALUERANGE_P_H_
#define HVALUERANGE_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../../utils/hexceptions_p.h"

#include <QString>
#include <QVariant>

namespace Herqq
{

namespace Upnp
{

//
// \internal
//
class HValueRange
{
private:

    QVariant m_maximum;
    QVariant m_minimum;
    QVariant m_step;

    template<typename T>
    static void checkValues(const HValueRange& val)
    {
        T min  = val.m_minimum.value<T>();
        T max  = val.m_maximum.value<T>();
        T step = val.m_step.value<T>();

        if (min > max)
        {
            throw Herqq::HIllegalArgumentException(
                "Minimum value cannot be larger than the maximum.");
        }

        if (max - min < step)
        {
            throw HIllegalArgumentException(
                "Step value cannot be larger than the entire range.");
        }
    }

public:

    HValueRange (){}
    ~HValueRange(){}

    inline QVariant maximum() const { return m_maximum; }
    inline QVariant minimum() const { return m_minimum; }
    inline QVariant step   () const { return m_step   ; }

    inline bool isNull() const
    {
        return m_maximum.isNull();
        // if any of the attributes is null, all are null ==> the object is null
    }

    static HValueRange fromVariant(
        QVariant::Type dataType,
        const QVariant& minimum, const QVariant& maximum, const QVariant& step)
    {
        return fromString(dataType, minimum.toString(), maximum.toString(), step.toString());
    }

    static HValueRange fromString(
        QVariant::Type dataType,
        const QString& minimum, const QString& maximum, const QString& step)
    {
        HValueRange retVal;

        retVal.m_maximum = maximum;
        if (!retVal.m_maximum.convert(dataType))
        {
            throw HIllegalArgumentException("Invalid maximum value");
        }

        retVal.m_minimum = minimum;
        if (!retVal.m_minimum.convert(dataType))
        {
            throw HIllegalArgumentException("Invalid minimum value");
        }

        retVal.m_step = step;
        if (!retVal.m_step.convert(dataType))
        {
            throw HIllegalArgumentException("Invalid step value");
        }

        switch(dataType)
        {
            case QVariant::Char      :
                checkValues<char>(retVal);
                break;

            case QVariant::Int       :
                checkValues<qint32>(retVal);
                break;

            case QVariant::LongLong  :
                checkValues<qlonglong>(retVal);
                break;

            case QVariant::UInt      :
                checkValues<quint32>(retVal);
                break;

            case QVariant::ULongLong :
                checkValues<qulonglong>(retVal);
                break;

            case QVariant::Double    :
                checkValues<qreal>(retVal);
                break;

            default:
                Q_ASSERT(false);
                throw HIllegalArgumentException("Invalid data type specified");
        }

        return retVal;
    }
};

}
}

#endif /* HVALUERANGE_P_H_ */
