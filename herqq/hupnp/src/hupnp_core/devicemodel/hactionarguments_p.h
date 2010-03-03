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

#ifndef H_ACTIONARGUMENTS_P_H_
#define H_ACTIONARGUMENTS_P_H_

#include "hactionarguments.h"
#include "./../datatypes/hupnp_datatypes.h"

#include <QUrl>
#include <QVector>
#include <QString>
#include <QVariant>

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
    // lookup using argument names regardless of the number of arguments. then again,
    // if the number of arguments is usually (nearly always?) very low, this
    // provides little to no benefit.

public: // functions

    inline HActionArgumentsPrivate()
    {
    }

    inline explicit HActionArgumentsPrivate(const QVector<HActionArgument*>& args)
    {
        QVector<HActionArgument*>::const_iterator ci = args.constBegin();

        for (; ci != args.constEnd(); ++ci)
        {
            m_argumentsOrdered.push_back(*ci);
            m_arguments[(*ci)->name()] = *ci;
        }
    }

    inline explicit HActionArgumentsPrivate(const QHash<QString, HActionArgument*>& args)
    {
        QHash<QString, HActionArgument*>::const_iterator ci = args.constBegin();

        for(; ci != args.end(); ++ci)
        {
            m_argumentsOrdered.push_back(*ci);
            m_arguments[(*ci)->name()] = *ci;
        }
    }

    inline ~HActionArgumentsPrivate()
    {
        qDeleteAll(m_argumentsOrdered);
    }

    inline HActionArgumentsPrivate(const HActionArgumentsPrivate& other)
    {
        QVector<HActionArgument*>::const_iterator ci =
            other.m_argumentsOrdered.constBegin();

        for (; ci != other.m_argumentsOrdered.constEnd(); ++ci)
        {
            HActionArgument* arg = new HActionArgument(**ci);
            m_argumentsOrdered.push_back(arg);
            m_arguments[arg->name()] = arg;
        }
    }

    inline HActionArgumentsPrivate& operator=(const HActionArgumentsPrivate& other)
    {
        qDeleteAll(m_argumentsOrdered);
        m_arguments.clear(); m_argumentsOrdered.clear();

        QVector<HActionArgument*>::const_iterator ci =
            other.m_argumentsOrdered.constBegin();

        for (; ci != other.m_argumentsOrdered.constEnd(); ++ci)
        {
            HActionArgument* arg = new HActionArgument(**ci);
            m_argumentsOrdered.push_back(arg);
            m_arguments[arg->name()] = arg;
        }

        return *this;
    }
};

}
}

#endif /* H_ACTIONARGUMENTS_P_H_ */
