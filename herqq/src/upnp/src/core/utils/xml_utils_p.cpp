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

#include "xml_utils_p.h"

#include <QString>
#include <QDomElement>

namespace Herqq
{

namespace Upnp
{

XmlUtils::XmlUtils()
{
}

XmlUtils::~XmlUtils()
{
}

QString XmlUtils::readElementValue(
    const QString elementTagToSearch,
    const QDomElement& parentElementrootDevice,
    bool* wasDefined)
{
    QDomNodeList nodeList =
        parentElementrootDevice.elementsByTagName(elementTagToSearch);

    if (nodeList.size() <= 0)
    {
        if (wasDefined)
        {
            *wasDefined = false;
        }

        return "";
    }

    if (wasDefined)
    {
        *wasDefined = true;
    }

    return nodeList.at(0).toElement().text();
}

}
}
