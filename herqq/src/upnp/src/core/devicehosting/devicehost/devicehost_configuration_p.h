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

#ifndef DEVICEHOST_CONFIGURATION_P_H_
#define DEVICEHOST_CONFIGURATION_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

namespace Herqq
{

namespace Upnp
{

//
// Implementation details of HDeviceConfiguration class
//
class HDeviceConfigurationPrivate
{
public: // attributes

    QString        m_pathToDeviceDescriptor;
    quint32        m_cacheControlMaxAgeInSecs;
    HDeviceCreator m_deviceCreator;

public: // methods

    HDeviceConfigurationPrivate();
    virtual ~HDeviceConfigurationPrivate();
};

//
//
//
class HDeviceHostConfigurationPrivate
{
H_DISABLE_COPY(HDeviceHostConfigurationPrivate)

public:

    QList<HDeviceConfiguration*> m_collection;
    // configurations for each device to be hosted.

    quint32 m_individualAdvertisementCount;
    // how many times each announcement / advertisement is sent

    HDeviceHostConfigurationPrivate();
};

}
}

#endif /* DEVICEHOST_CONFIGURATION_H_ */
