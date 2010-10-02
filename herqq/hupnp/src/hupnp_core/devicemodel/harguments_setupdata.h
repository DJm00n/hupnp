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

#ifndef HARGUMENTS_SETUPDATA_H_
#define HARGUMENTS_SETUPDATA_H_

#include <HUpnpCore/HStateVariableSetup>

#include <QtCore/QHash>
#include <QtCore/QString>

template<typename T>
class QSet;

namespace Herqq
{

namespace Upnp
{

/*!
 * This class is used to specify information that is required to setup an
 * HActionArgument.
 *
 * \headerfile harguments_setupdata.h HArgumentSetup
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HArgumentsSetupData, HActionArgument
 *
 * \remarks this class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HArgumentSetup
{
public:

    /*!
     * This enumeration specifies the different action argument types.
     */
    enum Type
    {
        /*!
         * The argument is provided to the action by the caller.
         *
         * The argument is input only.
         */
        Input = 0,

        /*!
         * The value of the argument is set by the action during its invocation.
         *
         * The argument is output only.
         */
        Output
    };

private:

    QString m_name;
    Type m_type;
    HStateVariableSetup m_relatedStateVariable;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HArgumentSetup();

    /*!
     * Creates a new instance.
     *
     * \param name specifies the name of the action.
     *
     * \param svSetup specifies setup information about the related state variable.
     *
     * \param type specifies the type of the action argument.
     *
     * \sa isValid()
     *
     * \remarks the version() is set to 1.
     */
    HArgumentSetup(
        const QString& name, const HStateVariableSetup& svSetup, Type type = Input);

    /*!
     * Indicates if the object is valid.
     *
     * \return \e true in case the object is valid, that is,
     * the name() and relatedStateVariable() are properly defined.
     */
    inline bool isValid() const
    {
        return !m_name.isEmpty() && m_relatedStateVariable.isValid();
    }

    /*!
     * Returns the name of the action argument.
     *
     * \return the name of the action argument.
     *
     * \sa setName()
     */
    inline QString name() const
    {
        return m_name;
    }

    /*!
     * Returns the type of the action argument.
     *
     * \return the type of the action argument.
     *
     * \sa setType()
     */
    inline Type type() const
    {
        return m_type;
    }

    /*!
     * Returns setup information about the related state variable.
     *
     * \return setup information about the related state variable.
     *
     * \sa setRelatedStateVariable()
     */
    inline const HStateVariableSetup& relatedStateVariable() const
    {
        return m_relatedStateVariable;
    }

    /*!
     * Sets the name of the action argument.
     *
     * \param name specifies the name of the action argument.
     *
     * \param err is a pointer to a \c QString that contains an error description
     * in case the name could not be set. This is an optional parameter.
     *
     * \return \e true in case the specified name was successfully set.
     *
     * \sa name()
     */
    bool setName(const QString& name, QString* err = 0);

    /*!
     * Associates setup information of a state variable with this instance.
     *
     * \param arg specifies the state variable setup information.
     *
     * \sa relatedStateVariable()
     */
    inline void setRelatedStateVariable(const HStateVariableSetup& relatedSv)
    {
        m_relatedStateVariable = relatedSv;
    }

    /*!
     * Specifies the type of the action argument.
     *
     * \param type specifies the type of the action argument.
     *
     * \sa type()
     */
    inline void setType(Type type)
    {
        m_type = type;
    }
};

/*!
 * This class is used to specify information that is required to setup the
 * \c HActionArguments of an HAction.
 *
 * \headerfile harguments_setupdata.h HArgumentsSetupData
 *
 * \ingroup hupnp_devicemodel
 *
 * \remarks this class is not thread-safe.
 *
 * \sa HArgumentSetup
 */
class H_UPNP_CORE_EXPORT HArgumentsSetupData
{

private:

    QHash<QString, HArgumentSetup> m_argumentSetupData;

public:

    /*!
     * Creates a new, empty instance.
     *
     * \sa isEmpty()
     */
    HArgumentsSetupData();

    /*!
     * Inserts a new item.
     *
     * \param newItem specifies the item to be added.
     *
     * \return \e true in case the item was added. The item will not be added
     * if the instance already contains an item with the
     * same name as \c newItem.
     *
     * \sa remove()
     */
    bool insert(const HArgumentSetup& newItem);

    /*!
     * Removes an existing item.
     *
     * \param name specifies the name of the item to be removed.
     *
     * \return \e true in case the item was found and removed.
     *
     * \sa insert()
     */
    bool remove(const QString& name);

    /*!
     * Retrieves an argument setup object.
     *
     * \param name specifies the name of the item to be retrieved.
     *
     * \return the item with the specified name. Note that the returned item
     * is invalid, i.e. HArgumentSetup::isValid() returns \e false in case no item
     * with the specified name was found.
     *
     * \sa contains()
     */
    HArgumentSetup get(const QString& name) const;

    /*!
     * Indicates if the instance contains an item with the specified name.
     *
     * \param name specifies the name of the item.
     *
     * \return \e true when the instance contains an item with the specified name.
     *
     * \sa get()
     */
    inline bool contains(const QString& name) const
    {
        return m_argumentSetupData.contains(name);
    }

    /*!
     * Returns the names of the contained items.
     *
     * \return the names of the contained items.
     */
    QSet<QString> names() const;

    /*!
     * Returns the number of contained items.
     *
     * \return the number of contained items.
     */
    inline qint32 size() const
    {
        return m_argumentSetupData.size();
    }

    /*!
     * Indicates if the object is empty.
     *
     * \return \e true in case the instance has no items.
     */
    inline bool isEmpty() const
    {
        return m_argumentSetupData.isEmpty();
    }

    /*!
     * Removes every contained object.
     */
    inline void clear()
    {
        m_argumentSetupData.clear();
    }
};

}
}

#endif /* HARGUMENTS_SETUPDATA_H_ */
