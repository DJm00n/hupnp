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

#ifndef DATATYPE_MAPPINGS_H_
#define DATATYPE_MAPPINGS_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "upnp_datatypes.h"

#include <QtSoapType>
#include <QtSoapSimpleType>

class QString;
class QVariant;

namespace Herqq
{

namespace Upnp
{

//
// \internal
//
class SoapType :
    public QtSoapSimpleType
{
public:
    SoapType(const QString& name, HUpnpDataTypes::DataType dt, const QVariant& value);
};

//
// \internal
//
QtSoapType::Type convertToSoapType(
    HUpnpDataTypes::DataType upnpDataType);

//
// \internal
//
QVariant::Type convertToVariantType(
    HUpnpDataTypes::DataType upnpDataType);

//
// \internal
//
QVariant convertToRightVariantType(
    const QString& value, HUpnpDataTypes::DataType upnpDataType);

}
}

#endif /* DATATYPE_MAPPINGS_H_ */
