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

#ifndef HSTATEVARIABLES_SETUPDATA_H_
#define HSTATEVARIABLES_SETUPDATA_H_

#include "../general/hupnp_global.h"

#include <QHash>
#include <QString>

template<typename T>
class QSet;

namespace Herqq
{

namespace Upnp
{

/*!
 * This class is used to specify information that can be used to setup an
 * HStateVariable.
 *
 * \headerfile hstatevariables_setupdata.h HStateVariableSetup
 *
 * \ingroup devicemodel
 *
 * \sa HStateVariablesSetupData, HStateVariable
 *
 * \remarks this class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HStateVariableSetup
{

private:

    QString m_name;
    HInclusionRequirement m_inclusionRequirement;
    qint32 m_maxRate;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid().
     */
    HStateVariableSetup();

    /*!
     * Creates a new instance.
     *
     * \param name specifies the name of the state variable.
     *
     * \param incReq specifies the \e inclusion \e requirement of the
     * state variable.
     *
     * \sa isValid()
     */
    HStateVariableSetup(
        const QString& name, HInclusionRequirement incReq = InclusionMandatory);

    /*!
     * Creates a new instance.
     *
     * \param name specifies the name of the state variable.
     *
     * \param maxRate specifies the maximum rate at which an
     * evented state variable may send events.
     *
     * \param incReq specifies the \e inclusion \e requirement of the
     * state variable.
     *
     * \sa isValid()
     */
    HStateVariableSetup(
        const QString& name,
        qint32 maxRate,
        HInclusionRequirement incReq = InclusionMandatory);

    /*!
     * Destroys the instance.
     */
    ~HStateVariableSetup();

    /*!
     * Returns the name of the state variable.
     *
     * \return the name of the state variable.
     */
    inline const QString& name() const
    {
        return m_name;
    }

    /*!
     * Sets the name of the state variable.
     *
     * \param name specifies the name of the state variable.
     *
     * \param err is a pointer to a \c QString that contains an error description
     * in case the name could not be set. This is an optional parameter.
     *
     * \return \e true in case the specified name was successfully set.
     */
    bool setName(const QString& name, QString* err = 0);

    /*!
     * Returns the maximum rate at which an evented state variable may send events.
     *
     * \return the maximum rate at which an evented state variable may send events.
     */
    inline qint32 maxEventRate() const
    {
        return m_maxRate;
    }

    /*!
     * Sets the maximum rate at which an evented state variable may send events.
     *
     * \param arg specifies the maximum rate at which an evented
     * state variable may send events.
     */
    inline void setMaxEventRate(qint32 arg)
    {
        m_maxRate = arg < 0 ? -1 : arg;
    }

    /*!
     * Returns the <em>inclusion requirement</em> of the state variable.
     *
     * \return the <em>inclusion requirement</em> of the state variable.
     */
    inline HInclusionRequirement inclusionRequirement() const
    {
        return m_inclusionRequirement;
    }

    /*!
     * Sets the <em>inclusion requirement</em> of the state variable.
     *
     * \param arg specifies the <em>inclusion requirement</em> of the state variable.
     */
    inline void setInclusionRequirement(HInclusionRequirement arg)
    {
        m_inclusionRequirement = arg;
    }

    /*!
     * Indicates if the object is valid.
     *
     * \return \e true in case the object is valid, that is,
     * both the name and the inclusion requirement are properly defined.
     */
    inline bool isValid() const
    {
        return !m_name.isEmpty() &&
                m_inclusionRequirement != InclusionRequirementUnknown;
    }
};

/*!
 * This class is used to specify information that is required to setup the
 * \c HStateVariables of an HService.
 *
 * \headerfile hstatevariables_setupdata.h HStateVariablesSetupData
 *
 * \ingroup devicemodel
 *
 * \remarks this class is not thread-safe.
 *
 * \sa HActionSetup
 */
class H_UPNP_CORE_EXPORT HStateVariablesSetupData
{

public:

    /*!
     * This enumeration specifies the actions the HUPnP device model builder
     * should take when it encounters an unknown state variable definition in a
     * service description file.
     */
    enum DefaultInclusionPolicy
    {
        /*!
         * The unknown state variable should be accepted.
         */
        Accept,

        /*!
         * The unknown state variable should be rejected.
         */
        Deny
    };

private:

    QHash<QString, HStateVariableSetup> m_setupData;
    DefaultInclusionPolicy m_defaultInclusionPolicy;

public:

    /*!
     * Creates a new, empty instance.
     *
     * \param defIncPol specifies the default inclusion policy for state variables
     * that are \b not contained in this instance.
     *
     * \sa isEmpty(), defaultInclusionPolicy()
     */
    HStateVariablesSetupData(DefaultInclusionPolicy defIncPol = Accept);

    /*!
     * Creates and inserts a new item based on the provided arguments.
     *
     * \param name specifies the name of the item.
     * \param incReq specifies the inclusion requirement of the new item.
     *
     * \return \e true in case a new item was created and added.
     * No item is created if the instance already contains an item
     * that has name equal to \c name.
     */
    bool insert(
        const QString& name,
        HInclusionRequirement incReq = InclusionMandatory);

    /*!
     * Inserts a new item.
     *
     * \param newItem specifies the item to be added.
     *
     * \return \e true in case the item was added. The item will not be added
     * if the instance already contains an item that has the
     * same name as the \c newItem.
     */
    bool insert(const HStateVariableSetup& newItem);

    /*!
     * Removes an existing item.
     *
     * \param name specifies the name of the item to be removed.
     *
     * \return \e true in case the item was found and removed.
     */
    bool remove(const QString& name);

    /*!
     * Returns the default inclusion policy.
     *
     * The default inclusion policy specifies the action to take when a
     * state variable definition in a service description file does not map
     * to any HStateVariableSetup object contained within this instance.
     *
     * \return the default inclusion policy.
     */
    DefaultInclusionPolicy defaultInclusionPolicy() const;

    /*!
     * Sets the inclusion requirement element of an item.
     *
     * \param name specifies the name of the item.
     *
     * \param incReq specifies the inclusion requirement value.
     *
     * \return \e true when the item was found and the inclusion requirement
     * element was set.
     */
    bool setInclusionRequirement(
        const QString& name, HInclusionRequirement incReq);

    /*!
     * Retrieves an item.
     *
     * \param name specifies the name of the item to be retrieved.
     *
     * \return the item with the specified name. Note that the returned item
     * is invalid, i.e. HStateVariableSetup::isValid() returns false in case no item
     * with the specified name was found.
     */
    HStateVariableSetup get(const QString& name) const;

    /*!
     * Indicates if the instance contains an item that has the
     * specified name.
     *
     * \param name specifies the name of the item.
     *
     * \return \e true when the instance contains an item that
     * has the specified name.
     */
    bool contains(const QString& name) const;

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
    qint32 size() const;

    /*!
     * Indicates if the object is empty.
     *
     * \return \e true in case the instance has no items.
     */
    bool isEmpty() const;
};

}
}

#endif /* HSTATEVARIABLES_SETUPDATA_H_ */
