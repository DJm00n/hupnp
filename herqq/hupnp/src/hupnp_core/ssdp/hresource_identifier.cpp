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

#include "hresource_identifier.h"

#include "./../dataelements/hudn.h"
#include "./../dataelements/hresourcetype.h"

#include <QUuid>
#include <QString>
#include <QStringList>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HResourceIdentifierPrivate
 ******************************************************************************/
class HResourceIdentifierPrivate
{
public:

    HResourceIdentifier::Type m_type;
    QString       m_contents;

    QUuid         m_deviceUuid;
    HResourceType m_resourceType;

public:

    HResourceIdentifierPrivate() :
        m_type(HResourceIdentifier::Undefined), m_contents(), m_deviceUuid(),
        m_resourceType()
    {
    }

    HResourceIdentifierPrivate(const QString& res) :
        m_type(HResourceIdentifier::Undefined), m_contents(), m_deviceUuid(),
        m_resourceType()
    {
        if (parse(res))
        {
            m_contents = res;
        }
    }

    HResourceIdentifierPrivate(const HResourceType& rt) :
        m_type(HResourceIdentifier::Undefined), m_contents(), m_deviceUuid(),
        m_resourceType()
    {
        if (parse(rt))
        {
            m_contents = rt.toString();
        }
    }

    HResourceIdentifierPrivate(const HUdn& udn) :
        m_type(HResourceIdentifier::Undefined), m_contents(), m_deviceUuid(),
        m_resourceType()
    {
        if (udn.isValid())
        {
            m_type = HResourceIdentifier::SpecificDevice;
            m_contents = udn.toString();
            m_deviceUuid = udn.value();
        }
    }

    bool parse(const HResourceType& rt)
    {
        if (!rt.isValid())
        {
            return false;
        }

        if (rt.isStandardType())
        {
            if (rt.type() == "device")
            {
                m_type = HResourceIdentifier::StandardDeviceType;
            }
            else if (rt.type() == "service")
            {
                m_type = HResourceIdentifier::StandardServiceType;
            }
            else
            {
                return false;
            }
        }
        else
        {
            if (rt.type() == "device")
            {
                m_type = HResourceIdentifier::VendorSpecifiedDeviceType;
            }
            else if (rt.type() == "service")
            {
                m_type = HResourceIdentifier::VendorSpecifiedServiceType;
            }
            else
            {
                return false;
            }
        }

        m_resourceType = rt;

        return true;
    }

    bool parse(const QString& nt)
    {
        QStringList parsed = nt.split(':');
        if (parsed[0] == "ssdp" && parsed[1] == "all")
        {
            m_type = HResourceIdentifier::AllDevices;
            return true;
        }
        else if (parsed[0] == "upnp" && parsed[1] == "rootdevice")
        {
            m_type = HResourceIdentifier::RootDevice;
            return true;
        }
        else if (parsed[0] == "uuid")
        {
            QUuid uuid(parsed[1]);
            if (uuid.isNull())
            {
                return false;
            }

            m_type       = HResourceIdentifier::SpecificDevice;
            m_deviceUuid = uuid;
            return true;
        }

        return parse(HResourceType(nt));
    }

};

/*******************************************************************************
 * HResourceIdentifier
 ******************************************************************************/
HResourceIdentifier::HResourceIdentifier() :
    h_ptr(new HResourceIdentifierPrivate())
{
}

HResourceIdentifier::HResourceIdentifier(const QString& resource) :
    h_ptr(new HResourceIdentifierPrivate(resource))
{
}

HResourceIdentifier::HResourceIdentifier(const HResourceType& resourceType) :
    h_ptr(new HResourceIdentifierPrivate(resourceType))
{
}

HResourceIdentifier::HResourceIdentifier(const HUdn& udn) :
    h_ptr(new HResourceIdentifierPrivate(udn))
{
}

HResourceIdentifier::~HResourceIdentifier()
{
    delete h_ptr;
}

HResourceIdentifier::HResourceIdentifier(const HResourceIdentifier& other) :
    h_ptr(new HResourceIdentifierPrivate(*other.h_ptr))
{
}

HResourceIdentifier HResourceIdentifier::getRootDeviceIdentifier()
{
    static HResourceIdentifier retVal("upnp:rootdevice");
    return retVal;
}

HResourceIdentifier HResourceIdentifier::getAllDevicesIdentifier()
{
    static HResourceIdentifier retVal("ssdp:all");
    return retVal;
}

HResourceIdentifier& HResourceIdentifier::operator=(const HResourceIdentifier& other)
{
    HResourceIdentifierPrivate* newHptr =
        new HResourceIdentifierPrivate(*other.h_ptr);

    delete h_ptr;
    h_ptr = newHptr;

    return *this;
}

HResourceIdentifier::Type HResourceIdentifier::type() const
{
    return h_ptr->m_type;
}

QUuid HResourceIdentifier::deviceUuid() const
{
    if (type() != SpecificDevice)
    {
        return QUuid();
    }

    return h_ptr->m_deviceUuid;
}

HResourceType HResourceIdentifier::resourceType() const
{
    if (type() != StandardDeviceType && type() != VendorSpecifiedDeviceType &&
        type() != StandardServiceType && type() != VendorSpecifiedServiceType)
    {
        return QString();
    }

    return h_ptr->m_resourceType;
}

QString HResourceIdentifier::toString() const
{
    return h_ptr->m_contents;
}

bool operator==(const HResourceIdentifier& nt1, const HResourceIdentifier& nt2)
{
    return nt1.toString() == nt2.toString();
}

bool operator!=(const HResourceIdentifier& nt1, const HResourceIdentifier& nt2)
{
    return !(nt1 == nt2);
}

}
}
