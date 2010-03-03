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

#include "husn.h"

#include <QString>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HUsn
 ******************************************************************************/
HUsn::HUsn() :
    m_udn(), m_resource()
{
}

HUsn::HUsn(const QString& arg) :
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

    HResourceIdentifier nt(arg.mid(i+2));

    // if there is a "::", then both components should be valid
    if (!udn.isValid() || nt.type() == HResourceIdentifier::Undefined)
    {
        return;
    }

    m_udn      = udn;
    m_resource = nt;
}

HUsn::HUsn(const HUdn& udn) :
    m_udn(), m_resource()
{
    if (udn.isValid())
    {
        m_udn      = udn;
        m_resource = HResourceIdentifier(udn);
    }
}

HUsn::HUsn(const HUdn& udn, const HResourceIdentifier& res) :
    m_udn(), m_resource()
{
    if (udn.isValid())
    {
        m_udn = udn;
        m_resource = res;
    }
}

HUsn::~HUsn()
{
}

QString HUsn::toString() const
{
    if (!isValid())
    {
        return QString();
    }

    if (m_resource.type() == HResourceIdentifier::SpecificDevice ||
        m_resource.type() == HResourceIdentifier::Undefined)
    {
        return m_udn.toString();
    }

    return QString("%1::%2").arg(m_udn.toString(), m_resource.toString());
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
