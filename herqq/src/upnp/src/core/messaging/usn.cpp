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

#include "usn.h"
#include "resource_identifier.h"
#include "../dataelements/udn.h"
#include "../../../../utils/src/logger_p.h"
#include "../../../../core/include/HExceptions"

#include <QString>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HUsnPrivate
 ******************************************************************************/
class HUsnPrivate
{
public: // attributes

    HUdn m_udn;
    HResourceIdentifier m_resource;

public: // methods

    HUsnPrivate() :
        m_udn(), m_resource()
    {
    }

    HUsnPrivate(const QString& arg) :
        m_udn(), m_resource()
    {
        qint32 i = arg.indexOf("::");
        if (i < 0)
        {
            HUdn tmpUdn(arg);
            if (tmpUdn.isValid())
            {
                m_udn      = tmpUdn;
                m_resource = m_udn;
            }

            return;
        }

        HUdn udn = HUdn(arg.left(i));

        HResourceIdentifier nt;
        try
        {
            nt = HResourceIdentifier(arg.mid(i+2));
        }
        catch(HException&)
        {
            // intentional
        }

        // if there is a "::", then both components should be valid
        if (!udn.isValid() || nt.type() == HResourceIdentifier::Undefined)
        {
            return;
        }

        m_udn      = udn;
        m_resource = nt;
    }

    HUsnPrivate(const HUdn& udn) :
        m_udn(), m_resource()
    {
        if (udn.isValid())
        {
            m_udn      = udn;
            m_resource = HResourceIdentifier(udn);
        }
    }

    HUsnPrivate(const HUdn& udn, const HResourceIdentifier& res) :
        m_udn(), m_resource()
    {
        if (udn.isValid())
        {
            m_udn = udn;
            m_resource = res;
        }
    }
};

/*******************************************************************************
 * HUsn
 ******************************************************************************/
HUsn::HUsn() :
    h_ptr(new HUsnPrivate())
{
}

HUsn::HUsn(const QString& arg) :
    h_ptr(new HUsnPrivate(arg))
{
}

HUsn::HUsn(const HUdn& udn) :
    h_ptr(new HUsnPrivate(udn))
{
}

HUsn::HUsn(const HUdn& udn, const HResourceIdentifier& nt) :
    h_ptr(new HUsnPrivate(udn, nt))
{
}

HUsn::HUsn(const HUsn& other) :
    h_ptr(new HUsnPrivate(*other.h_ptr))
{
}

HUsn::~HUsn()
{
    delete h_ptr;
}

HUsn& HUsn::operator=(const HUsn& other)
{
    HUsnPrivate* newHptr = new HUsnPrivate(*other.h_ptr);

    delete h_ptr;
    h_ptr = newHptr;

    return *this;
}

void HUsn::setResource(const HResourceIdentifier& resource)
{
    h_ptr->m_resource = resource;
}

HUdn HUsn::udn() const
{
    return h_ptr->m_udn;
}

HResourceIdentifier HUsn::resource() const
{
    return h_ptr->m_resource;
}

bool HUsn::isValid() const
{
    return h_ptr->m_udn.isValid();
}

QString HUsn::toString() const
{
    if (!isValid())
    {
        return QString();
    }

    if (h_ptr->m_resource.type() == HResourceIdentifier::SpecificDevice ||
        h_ptr->m_resource.type() == HResourceIdentifier::Undefined)
    {
        return h_ptr->m_udn.toString();
    }

    return QString("%1::%2").arg(h_ptr->m_udn.toString(), h_ptr->m_resource.toString());
}

bool operator==(const HUsn& usn1, const HUsn& usn2)
{
    return usn1.udn()      == usn2.udn() &&
           usn1.resource() == usn2.resource();
}

bool operator!=(const HUsn& usn1, const HUsn& usn2)
{
    return !(usn1 == usn2);
}

}
}
