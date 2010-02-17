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

#ifndef HDEFAULTDEVICE_H_
#define HDEFAULTDEVICE_H_

#include "./../devicemodel/hdevice.h"
#include "./../devicemodel/haction.h"
#include "./../devicemodel/hservice.h"

namespace Herqq
{

namespace Upnp
{

/*!
 *
 */
class HDefaultService :
    public HService
{
H_DISABLE_COPY(HDefaultService)

private:

    virtual HActionMapT createActions();

public:

    HDefaultService();
    virtual ~HDefaultService();
};

/*!
 *
 */
class HDefaultDevice :
    public HDevice
{
H_DISABLE_COPY(HDefaultDevice)

private:

    virtual HServiceMapT createServices();

public:

    HDefaultDevice();
    virtual ~HDefaultDevice();
};

}
}

#endif /* HDEFAULTDEVICE_H_ */
