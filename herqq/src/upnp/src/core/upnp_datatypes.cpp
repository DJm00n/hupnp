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

#include "upnp_datatypes.h"
#include "../../../core/include/HExceptions"

#include <QObject>

namespace Herqq
{

namespace Upnp
{

HUpnpDataTypes::HUpnpDataTypes()
{
}

HUpnpDataTypes::~HUpnpDataTypes()
{
}

HUpnpDataTypes::DataType HUpnpDataTypes::dataType(const QString& dataTypeAsStr)
{
    if (dataTypeAsStr.compare(ui1_str()) == 0)
    {
        return ui1;
    }
    else if (dataTypeAsStr.compare(ui2_str()) == 0)
    {
        return ui2;
    }
    else if (dataTypeAsStr.compare(ui4_str()) == 0)
    {
        return ui4;
    }
    else if (dataTypeAsStr.compare(i1_str()) == 0)
    {
        return i1;
    }
    else if (dataTypeAsStr.compare(i2_str()) == 0)
    {
        return i2;
    }
    else if (dataTypeAsStr.compare(i4_str()) == 0)
    {
        return i4;
    }
    else if (dataTypeAsStr.compare(integer_str()) == 0)
    {
        return integer;
    }
    else if (dataTypeAsStr.compare(r4_str()) == 0)
    {
        return r4;
    }
    else if (dataTypeAsStr.compare(r8_str()) == 0)
    {
        return r8;
    }
    else if (dataTypeAsStr.compare(number_str()) == 0)
    {
        return number;
    }
    else if (dataTypeAsStr.compare(fixed_14_4_str()) == 0)
    {
        return fixed_14_4;
    }
    else if (dataTypeAsStr.compare(fp_str()) == 0)
    {
        return fp;
    }
    else if (dataTypeAsStr.compare(character_str()) == 0)
    {
        return character;
    }
    else if (dataTypeAsStr.compare(string_str()) == 0)
    {
        return string;
    }
    else if (dataTypeAsStr.compare(date_str()) == 0)
    {
        return date;
    }
    else if (dataTypeAsStr.compare(dateTime_str()) == 0)
    {
        return dateTime;
    }
    else if (dataTypeAsStr.compare(dateTimeTz_str()) == 0)
    {
        return dateTimeTz;
    }
    else if (dataTypeAsStr.compare(time_str()) == 0)
    {
        return time;
    }
    else if (dataTypeAsStr.compare(time_tz_str()) == 0)
    {
        return timeTz;
    }
    else if (dataTypeAsStr.compare(boolean_str()) == 0)
    {
        return boolean;
    }
    else if (dataTypeAsStr.compare(bin_base64_str()) == 0)
    {
        return bin_base64;
    }
    else if (dataTypeAsStr.compare(bin_hex_str()) == 0)
    {
        return bin_hex;
    }
    else if (dataTypeAsStr.compare(uri_str()) == 0)
    {
        return uri;
    }
    else if (dataTypeAsStr.compare(uuid_str()) == 0)
    {
        return uuid;
    }

    throw Herqq::HIllegalArgumentException(
        QObject::tr("The specified string [%1] does not correspond to any UPnP data type.").arg(
            dataTypeAsStr));
}

bool HUpnpDataTypes::isRational(DataType dataType)
{
    switch(dataType)
    {
        case r4:
        case r8:
        case number:
        case fp:
        case fixed_14_4:
            return true;
        default:
            return false;
    }
}

bool HUpnpDataTypes::isNumeric(DataType datatype)
{
    return datatype >= ui1 && datatype <= fp;
}

bool HUpnpDataTypes::isInteger(DataType datatype)
{
    return datatype >= ui1 && datatype <= integer;
}

QString HUpnpDataTypes::toString(DataType dataType)
{
    switch(dataType)
    {
    case Undefined:
        return "Undefined";
    case ui1:
        return ui1_str();
    case ui2:
        return ui2_str();
    case ui4:
        return ui4_str();
    case i1:
        return i1_str();
    case i2:
        return i2_str();
    case i4:
        return i4_str();
    case integer:
        return integer_str();
    case r4:
        return r4_str();
    case r8:
        return r8_str();
    case number:
        return number_str();
    case fixed_14_4:
        return fixed_14_4_str();
    case fp:
        return fp_str();
    case character:
        return character_str();
    case string:
        return string_str();
    case date:
        return date_str();
    case dateTime:
        return dateTime_str();
    case dateTimeTz:
        return dateTimeTz_str();
    case time:
        return time_str();
    case timeTz:
        return time_tz_str();
    case boolean:
        return boolean_str();
    case bin_base64:
        return bin_base64_str();
    case bin_hex:
        return bin_hex_str();
    case uri:
        return uri_str();
    case uuid:
        return uuid_str();
    default:
        return "Undefined";
    }
}

}
}
