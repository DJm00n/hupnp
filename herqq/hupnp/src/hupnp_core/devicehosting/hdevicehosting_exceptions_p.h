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

#ifndef HDEVICEHOSTING_EXCEPTIONS_P_H
#define HDEVICEHOSTING_EXCEPTIONS_P_H

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "./../../utils/hexceptions_p.h"

namespace Herqq
{

namespace Upnp
{

//
//
//
class InvalidDeviceDescription :
    public Herqq::HParseException
{
public:
    InvalidDeviceDescription();
    InvalidDeviceDescription(const QString& reason);
    virtual ~InvalidDeviceDescription() throw ();
};

//
//
//
class InvalidServiceDescription :
    public Herqq::HParseException
{
public:
    InvalidServiceDescription();
    InvalidServiceDescription(const QString& reason);
    virtual ~InvalidServiceDescription() throw ();
};

}
}

#endif /* HDEVICEHOSTING_EXCEPTIONS_P_H */
