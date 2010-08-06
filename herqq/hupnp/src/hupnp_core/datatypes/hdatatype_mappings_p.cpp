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

#include "hdatatype_mappings_p.h"

#include <QUrl>
#include <QString>

namespace Herqq
{

namespace Upnp
{

SoapType::SoapType(
    const QString& name, HUpnpDataTypes::DataType dt, const QVariant& value) :
        QtSoapSimpleType()
{
    Q_ASSERT(!name.isEmpty());
    Q_ASSERT_X(value.isValid(), "", name.toLocal8Bit());
    Q_ASSERT(dt != HUpnpDataTypes::Undefined);

    n = QtSoapQName(name);
    t = convertToSoapType(dt);

    if (dt == HUpnpDataTypes::uri)
    {
        // the qtsoap library handles its "anyURI" type as it does almost any other
        // type, as string. However, at the time of writing this (with Qt 4.5.3)
        // the QVariant does not support toString() for Url types.
        v = value.toUrl().toString();
    }
    else
    {
        v = value;
    }
}

QtSoapType::Type convertToSoapType(HUpnpDataTypes::DataType upnpDataType)
{
    switch (upnpDataType)
    {
    case HUpnpDataTypes::i1:
        return QtSoapType::Byte;

    case HUpnpDataTypes::i2:
        return QtSoapType::Short;

    case HUpnpDataTypes::i4:
    case HUpnpDataTypes::integer:
        return QtSoapType::Integer;

    case HUpnpDataTypes::ui1:
        return QtSoapType::UnsignedByte;

    case HUpnpDataTypes::ui2:
        return QtSoapType::UnsignedShort;

    case HUpnpDataTypes::ui4:
        return QtSoapType::UnsignedInt;

    case HUpnpDataTypes::r4:
    case HUpnpDataTypes::r8:
    case HUpnpDataTypes::number:
    case HUpnpDataTypes::fixed_14_4:
        return QtSoapType::Double;

    case HUpnpDataTypes::fp:
        return QtSoapType::Float;

    case HUpnpDataTypes::character:
    case HUpnpDataTypes::string:
        return QtSoapType::String;

    case HUpnpDataTypes::date:
        return QtSoapType::Date;

    case HUpnpDataTypes::dateTime:
    case HUpnpDataTypes::dateTimeTz:
        return QtSoapType::DateTime;

    case HUpnpDataTypes::time:
    case HUpnpDataTypes::timeTz:
        return QtSoapType::Time;

    case HUpnpDataTypes::boolean:
        return QtSoapType::Boolean;

    case HUpnpDataTypes::bin_base64:
        return QtSoapType::Base64Binary;

    case HUpnpDataTypes::bin_hex:
        return QtSoapType::HexBinary;

    case HUpnpDataTypes::uri:
        return QtSoapType::AnyURI;

    case HUpnpDataTypes::uuid:
        return QtSoapType::ID;

    default:
        Q_ASSERT(false);
    }

    Q_ASSERT(false);
    return QtSoapType::Other;
}

QVariant::Type convertToVariantType(HUpnpDataTypes::DataType upnpDataType)
{
    switch (upnpDataType)
    {
    case HUpnpDataTypes::character:
        return QVariant::Char;

    case HUpnpDataTypes::i1:
    case HUpnpDataTypes::i2:
    case HUpnpDataTypes::i4:
    case HUpnpDataTypes::integer:
        return QVariant::Int;

    case HUpnpDataTypes::ui1:
    case HUpnpDataTypes::ui2:
    case HUpnpDataTypes::ui4:
        return QVariant::UInt;

    case HUpnpDataTypes::fp:
    case HUpnpDataTypes::r4:
    case HUpnpDataTypes::r8:
    case HUpnpDataTypes::number:
    case HUpnpDataTypes::fixed_14_4:
        return QVariant::Double;

    case HUpnpDataTypes::string:
        return QVariant::String;

    case HUpnpDataTypes::date:
        return QVariant::Date;

    case HUpnpDataTypes::dateTime:
    case HUpnpDataTypes::dateTimeTz:
        return QVariant::DateTime;

    case HUpnpDataTypes::time:
    case HUpnpDataTypes::timeTz:
        return QVariant::Time;

    case HUpnpDataTypes::boolean:
        return QVariant::Bool;

    case HUpnpDataTypes::bin_hex:
    case HUpnpDataTypes::bin_base64:
        return QVariant::ByteArray;

    case HUpnpDataTypes::uri:
        return QVariant::Url;

    case HUpnpDataTypes::uuid:
        return QVariant::String;

    default:
        Q_ASSERT(false);
    }

    Q_ASSERT(false);
    return QVariant::Invalid;
}

QVariant convertToRightVariantType(
    const QString& value, HUpnpDataTypes::DataType upnpDataType)
{
    QVariant retVal;

    switch (upnpDataType)
    {
    case HUpnpDataTypes::character:
        return !value.isEmpty() ? QChar(value[0]) : QVariant(QVariant::Char);

    case HUpnpDataTypes::i1:
    case HUpnpDataTypes::i2:
    case HUpnpDataTypes::i4:
    case HUpnpDataTypes::integer:
    {
        bool ok = false;
        retVal = value.toInt(&ok);
        Q_ASSERT(ok);
        break;
    }

    case HUpnpDataTypes::ui1:
    case HUpnpDataTypes::ui2:
    case HUpnpDataTypes::ui4:
    {
        bool ok = false;
        retVal = value.toUInt(&ok);
        Q_ASSERT(ok);
        break;
    }

    case HUpnpDataTypes::fp:
    case HUpnpDataTypes::r4:
    case HUpnpDataTypes::r8:
    case HUpnpDataTypes::number:
    case HUpnpDataTypes::fixed_14_4:
    {
        bool ok = false;
        retVal = value.toDouble(&ok);
        Q_ASSERT(ok);
        break;
    }

    case HUpnpDataTypes::string:
        return value;

    case HUpnpDataTypes::date:
    {
        retVal = QDate::fromString(value, Qt::ISODate);
        Q_ASSERT(retVal.isValid());
        break;
    }

    case HUpnpDataTypes::dateTime:
    case HUpnpDataTypes::dateTimeTz:
    {
        retVal = QDateTime::fromString(value, Qt::ISODate);
        Q_ASSERT(retVal.isValid());
        break;
    }

    case HUpnpDataTypes::time:
    case HUpnpDataTypes::timeTz:
    {
        retVal = QTime::fromString(value, Qt::ISODate);
        Q_ASSERT(retVal.isValid());
        break;
    }

    case HUpnpDataTypes::boolean:
    {
        if (value.compare("true", Qt::CaseInsensitive) == 0 ||
            value.compare("yes", Qt::CaseInsensitive) == 0 ||
            value.compare("1") == 0)
        {
            retVal = true;
        }
        else if (value.compare("false", Qt::CaseInsensitive) == 0 ||
            value.compare("no", Qt::CaseInsensitive) == 0 ||
            value.compare("0") == 0)
        {
            retVal = false;
        }
        else
        {
            Q_ASSERT(false);
        }

        break;
    }

    case HUpnpDataTypes::bin_hex:
        return value;//.toUtf8().toHex();

    case HUpnpDataTypes::bin_base64:
        return value;//.toUtf8().toBase64();

    case HUpnpDataTypes::uri:
    {
        retVal = QUrl(value);
        Q_ASSERT(retVal.isValid());
        break;
    }

    case HUpnpDataTypes::uuid:
        return value;

    default:
        Q_ASSERT(false);
    }

    return retVal;
}

}
}

