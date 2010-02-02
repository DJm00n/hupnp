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

#ifndef UPNP_ACTIONARGUMENTS_P_H_
#define UPNP_ACTIONARGUMENTS_P_H_

#include "../upnp_datatypes.h"
#include "statevariable.h"

#include <QUrl>
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
class HActionArgumentPrivate
{
private:

    QString         m_name;
    HStateVariable* m_stateVariable;
    QVariant        m_value;

public:

    HActionArgumentPrivate();
    HActionArgumentPrivate(const QString& name, HStateVariable* stateVariable);

    inline QString name() const
    {
        return m_name;
    }

    inline HStateVariable* relatedStateVariable() const
    {
        return m_stateVariable;
    }

    inline HUpnpDataTypes::DataType dataType() const
    {
        if (m_stateVariable)
        {
            return m_stateVariable->dataType();
        }
        else
        {
            return HUpnpDataTypes::Undefined;
        }
    }

    inline QVariant value() const
    {
        return m_value;
    }

    inline bool setValue(const QVariant& value)
    {
        QVariant convertedValue;
        if (isValid() && relatedStateVariable()->isValidValue(value, &convertedValue))
        {
            m_value = convertedValue;
            return true;
        }

        return false;
    }

    inline bool isValid() const
    {
        return !m_name.isEmpty();
    }

    inline bool operator!() const
    {
        return !isValid();
    }

    inline QString toString() const
    {
        return QString("%1: %2").arg(
            name(),
            dataType() == HUpnpDataTypes::uri ? value().toUrl().toString() :
            value().toString());
    }
};

//
//
//
template<class T>
class HActionArgumentsPrivate
{
public:

    QList<T*> m_argumentsOrdered;
    // UDA 1.1 mandates that action arguments are always transmitted in the order
    // they were specified in the service description.

    QHash<QString, T*> m_arguments;
    // For the user's convenience

public:

    HActionArgumentsPrivate()
    {
    }

    explicit HActionArgumentsPrivate(const QList<T>& args)
    {
        typename QList<T>::const_iterator ci = args.constBegin();

        for (; ci != args.constEnd(); ++ci)
        {
            T* arg = new T(*ci);
            m_argumentsOrdered.push_back(arg);
            m_arguments[arg->name()] = arg;
        }
    }

    explicit HActionArgumentsPrivate(const QHash<QString, T>& args)
    {
        typename QHash<QString, T>::const_iterator ci = args.constBegin();

        for(; ci != args.end(); ++ci)
        {
            T* arg = new T(ci.value());
            m_argumentsOrdered.push_back(arg);
            m_arguments[arg->name()] = arg;
        }
    }

public:

    typedef typename QList<T*>::const_iterator const_iterator;
    typedef typename QList<T*>::iterator iterator;

    ~HActionArgumentsPrivate()
    {
        qDeleteAll(m_argumentsOrdered);
    }

    HActionArgumentsPrivate(const HActionArgumentsPrivate<T>& other)
    {
        typename HActionArgumentsPrivate<T>::const_iterator ci = other.constBegin();
        for (; ci != other.constEnd(); ++ci)
        {
            T* arg = new T(**ci);
            m_argumentsOrdered.push_back(arg);
            m_arguments[arg->name()] = arg;
        }
    }

    HActionArgumentsPrivate<T>& operator=(const HActionArgumentsPrivate<T>& other)
    {
        qDeleteAll(m_argumentsOrdered);
        m_arguments.clear(); m_argumentsOrdered.clear();

        typename HActionArgumentsPrivate<T>::const_iterator ci = other.constBegin();
        for (; ci != other.constEnd(); ++ci)
        {
            T* arg = new T(**ci);
            m_argumentsOrdered.push_back(arg);
            m_arguments[arg->name()] = arg;
        }

        return *this;
    }

    inline bool contains(const QString& argumentName) const
    {
        return m_arguments.contains(argumentName);
    }

    inline const T* get(qint32 index) const
    {
        return m_argumentsOrdered.at(index);
    }

    inline T* get(qint32 index)
    {
        return m_argumentsOrdered.at(index);
    }

    inline const T* get(const QString& argumentName) const
    {
        return m_arguments.value(argumentName);
    }

    inline T* get(const QString& argumentName)
    {
        return m_arguments.value(argumentName);
    }

    inline typename HActionArgumentsPrivate<T>::const_iterator constBegin() const
    {
        return m_argumentsOrdered.constBegin();
    }

    inline typename HActionArgumentsPrivate<T>::const_iterator constEnd() const
    {
        return m_argumentsOrdered.constEnd();
    }

    inline typename HActionArgumentsPrivate<T>::iterator begin()
    {
        return m_argumentsOrdered.begin();
    }

    inline typename HActionArgumentsPrivate<T>::iterator end()
    {
        return m_argumentsOrdered.end();
    }

    inline typename HActionArgumentsPrivate<T>::const_iterator begin() const
    {
        return m_argumentsOrdered.begin();
    }

    inline typename HActionArgumentsPrivate<T>::const_iterator end() const
    {
        return m_argumentsOrdered.end();
    }

    inline qint32 size() const
    {
        return m_argumentsOrdered.size();
    }

    inline T* operator[](qint32 index)
    {
        return m_argumentsOrdered.at(index);
    }

    inline const T* operator[](qint32 index) const
    {
        return m_argumentsOrdered.at(index);
    }

    inline T* operator[](const QString& argName)
    {
        return contains(argName) ? m_arguments[argName] : 0;
    }

    inline const T* operator[](const QString& argName) const
    {
        return contains(argName) ? m_arguments[argName] : 0;
    }

    QList<QString> names() const
    {
        return m_arguments.keys();
    }

    QString toString() const
    {
        QString retVal;

        typename HActionArgumentsPrivate<T>::const_iterator ci = constBegin();
        for (; ci != constEnd(); ++ci)
        {
            retVal.append((*ci)->toString()).append("\n");
        }

        return retVal;
    }
};

}
}

#endif /* UPNP_ACTIONARGUMENTS_P_H_ */
