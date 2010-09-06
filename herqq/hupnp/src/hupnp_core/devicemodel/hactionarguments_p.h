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

#ifndef HACTIONARGUMENTS_P_H_
#define HACTIONARGUMENTS_P_H_

#include "hactionarguments.h"
#include "../datatypes/hupnp_datatypes.h"

#include <QtCore/QUrl>
#include <QtCore/QVector>
#include <QtCore/QString>
#include <QtCore/QVariant>

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
//
//
class HActionArgumentsPrivate
{
public: // attributes

    QVector<HActionArgument*> m_argumentsOrdered;
    // UDA 1.1 mandates that action arguments are always transmitted in the order
    // they were specified in the service description.

    QHash<QString, HActionArgument*> m_arguments;
    // QHash has a very low memory footprint and it provides us a constant-time
    // lookup using argument names regardless of the number of arguments.

public: // functions

    HActionArgumentsPrivate();
    explicit HActionArgumentsPrivate(const QVector<HActionArgument*>& args);
    ~HActionArgumentsPrivate();

    HActionArgumentsPrivate(const HActionArgumentsPrivate& other);
    HActionArgumentsPrivate& operator=(const HActionArgumentsPrivate& other);
};

}
}

#endif /* HACTIONARGUMENTS_P_H_ */
