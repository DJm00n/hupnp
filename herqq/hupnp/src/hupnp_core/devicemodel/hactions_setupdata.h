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

#ifndef HACTIONS_SETUPDATA_H_
#define HACTIONS_SETUPDATA_H_

#include "hactioninvoke.h"
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
 * This class is used to specify information that is required to setup an
 * HAction.
 *
 * \headerfile hactions_setupdata.h HActionSetup
 *
 * \ingroup devicemodel
 *
 * \sa HActionsSetupData, HAction
 *
 * \remarks this class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HActionSetup
{
private:

    QString m_name;
    HInclusionRequirement m_inclusionRequirement;
    HActionInvoke m_actionInvoke;

public:

    /*!
     * Creates a new, empty instance.
     *
     * \sa isValid()
     */
    HActionSetup();

    /*!
     * Creates a new instance.
     *
     * \param name specifies the name of the action.
     *
     * \param incReq specifies the \e inclusion \e requirement of the action.
     *
     * \sa isValid()
     */
    HActionSetup(
        const QString& name, HInclusionRequirement incReq = InclusionMandatory);

    /*!
     * Creates a new instance.
     *
     * \param name specifies the name of the action.
     *
     * \param invoke specifies the callable entity that is called when the
     * action is invoked. This is used only at server side.
     *
     * \param incReq specifies the \e inclusion \e requirement of the action.
     *
     * \sa isValid()
     */
    HActionSetup(
        const QString& name, const HActionInvoke& invoke,
        HInclusionRequirement incReq = InclusionMandatory);

    /*!
     * Returns the name of the action.
     *
     * \return the name of the action.
     */
    inline const QString& name() const
    {
        return m_name;
    }

    /*!
     * Sets the name of the action.
     *
     * \param name specifies the name of the action.
     *
     * \param err is a pointer to a \c QString that contains an error description
     * in case the name could not be set. This is an optional parameter.
     *
     * \return \e true in case the specified name was successfully set.
     */
    bool setName(const QString& name, QString* err = 0);

    /*!
     * Returns the <em>inclusion requirement</em> of the action.
     *
     * \return the <em>inclusion requirement</em> of the action.
     */
    inline HInclusionRequirement inclusionRequirement() const
    {
        return m_inclusionRequirement;
    }

    /*!
     * Sets the <em>inclusion requirement</em> of the action.
     *
     * \param arg specifies the <em>inclusion requirement</em> of the action.
     */
    inline void setInclusionRequirement(HInclusionRequirement arg)
    {
        m_inclusionRequirement = arg;
    }

    /*!
     * Returns the callable entity that is called when the
     * action is invoked.
     *
     * \remarks This is used only at server side.
     */
    inline const HActionInvoke& actionInvoke() const
    {
        return m_actionInvoke;
    }

    /*!
     * Specifies the callable entity that is called when the
     * action is invoked.
     *
     * \param arg specifies the callable entity that is called when the
     * action is invoked.
     *
     * \remarks This is used only at server side.
     */
    inline void setActionInvoke(const HActionInvoke& arg)
    {
        m_actionInvoke = arg;
    }

    /*!
     * Indicates if the object is valid.
     *
     * \return \e true in case the object is valid, that is,
     * the name and the inclusion requirement are properly defined.
     */
    inline bool isValid() const
    {
        return !m_name.isEmpty() &&
                m_inclusionRequirement != InclusionRequirementUnknown;
    }
};

/*!
 * This class is used to specify information that is required to setup the
 * \c HActions of an HService.
 *
 * \headerfile hactions_setupdata.h HActionsSetupData
 *
 * \ingroup devicemodel
 *
 * \remarks this class is not thread-safe.
 *
 * \sa HActionSetup
 */
class H_UPNP_CORE_EXPORT HActionsSetupData
{

private:

    QHash<QString, HActionSetup> m_actionSetupInfos;

public:

    /*!
     * Creates a new, empty instance.
     *
     * \sa isEmpty()
     */
    HActionsSetupData();

    /*!
     * Inserts a new item.
     *
     * \param newItem specifies the item to be added.
     *
     * \return \e true in case the item was added. The item will not be added
     * if the instance already contains an item with the
     * same name as \c newItem.
     */
    bool insert(const HActionSetup& newItem);

    /*!
     * Creates and inserts a new item based on the provided arguments.
     *
     * \param name specifies the name of the new item.
     *
     * \param incReq specifies whether the action is required or optional.
     *
     * \return \e true in case a new item was created was added.
     * No item is created if the instance already contains an item
     * with the same name as the new item.
     */
    bool insert(
        const QString& name, HInclusionRequirement incReq = InclusionMandatory);

    /*!
     * Creates and inserts a new item based on the provided arguments.
     *
     * \param name specifies the name of the new item.
     *
     * \param invoke specifies the callable entity that is called when the
     * action is invoked. This is used only at server side.
     *
     * \param incReq specifies whether the action is required or optional.
     *
     * \return \e true in case a new item was created was added.
     * No item is created if the instance already contains an action setup
     * object that has the same name as the new item.
     */
    bool insert(const QString& name, const HActionInvoke& invoke,
        HInclusionRequirement incReq = InclusionMandatory);

    /*!
     * Removes an existing item.
     *
     * \param name specifies the name of the item to be removed.
     *
     * \return \e true in case the item was found and removed.
     */
    bool remove(const QString& name);

    /*!
     * Retrieves an action setup object.
     *
     * \param name specifies the name of the item to be retrieved.
     *
     * \return the item with the specified name. Note that the returned item
     * is invalid, i.e. HActionSetup::isValid() returns false in case no item
     * with the specified name was found.
     */
    HActionSetup get(const QString& name) const;

    /*!
     * Sets the callable entity that is called when the
     * action is invoked. This is used only at server side.
     *
     * \param name specifies the name of the item.
     *
     * \param actionInvoke specifies the callable entity.
     *
     * \return \e true in case the item was found and its callable entity was set.
     */
    bool setInvoke(
        const QString& name, const HActionInvoke& actionInvoke);

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
     * Indicates if the instance contains an item with the specified name.
     *
     * \param name specifies the name of the item.
     *
     * \return \e true when the instance contains an item with the specified name.
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

#endif /* HACTIONS_SETUPDATA_H_ */
