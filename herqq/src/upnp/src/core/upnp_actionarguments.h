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

#ifndef UPNP_ACTIONARGUMENTS_H_
#define UPNP_ACTIONARGUMENTS_H_

#include "defs_p.h"
#include "upnp_datatypes.h"

template<typename T, typename U>
class QHash;

class QString;
class QVariant;

#include <QList>

namespace Herqq
{

namespace Upnp
{

class HStateVariable;
class HActionArgumentPrivate;

/*!
 * A class that represents an UPnP input action argument.
 *
 * \remark the class is not thread-safe.
 *
 * \headerfile upnp_actionarguments.h HActionInputArgument
 *
 * \ingroup devicemodel
 *
 * \sa HActionInputArguments, HActionOutputArgument, HActionOutputArguments
 */
class H_UPNP_CORE_EXPORT HActionInputArgument
{
friend class HObjectCreator;

private:

    HActionArgumentPrivate* h_ptr;

    //
    // \internal
    //
    // Constructs a new, empty instance.
    //
    // \remark object constructed using this method is always considered
    // invalid, in which case isValid() always returns false.
    //
    // \sa isValid()
    //
    HActionInputArgument();

    //
    // \internal
    //
    // Constructs a new instance with the specified name and related state variable.
    //
    // \param name specifies the name of the argument
    // \param stateVariable specifies the related state variable.
    //
    // \remark in case the name parameter fails the criteria specified for
    // UPnP action arguments in UPnP Device Architecture 1.1 specification
    // or the stateVariable is null, the object is constructed as "invalid";
    // isValid() always returns false.
    //
    // \sa isValid()
    //
    HActionInputArgument(const QString& name, HStateVariable* stateVariable);

public:

    /*!
     * Copy constructor.
     */
    HActionInputArgument(const HActionInputArgument&);

    /*!
     * Destroys the instance.
     */
    ~HActionInputArgument();

    /*!
     * Assignment operator.
     */
    HActionInputArgument& operator=(const HActionInputArgument&);

    /*!
     * Returns the name of the argument.
     *
     * \return the name of the argument.
     */
    QString name() const;

    /*!
     * Returns the state variable that is associated with this action argument.
     *
     * \return the state variable that is associated with this action argument,
     * or a null pointer in case there is no state variable associated with this action
     * argument. This is the case only when isValid() returns false.
     *
     * \sa isValid()
     */
    HStateVariable* relatedStateVariable() const;

    /*!
     * Helper method for accessing the data type of the related state variable
     * directly.
     *
     * \return the data type of the state variable.
     */
    HUpnpDataTypes::DataType dataType() const;

    /*!
     * Returns the value of the argument.
     *
     * \return the value of the argument.
     */
    QVariant value() const;

    /*!
     * Sets the value of the argument if the object is valid and the value is
     * of right type.
     *
     * \param value specifies the new value of the argument.
     *
     * \return \e true in case the value was successfully set.
     */
    bool setValue(const QVariant& value);

    /*!
     * Indicates if the object is constructed with a proper name and a state
     * variable.
     *
     * \return \e true in case the object has a proper name and the object refers
     * to a valid state variable.
     */
    bool isValid() const;

    /*!
     * Indicates whether or not the object is considered as invalid.
     *
     * \return \e true in case the object is invalid.
     */
    bool operator!() const;

    /*!
     * Returns a string representation of the object.
     *
     * \return a string representation of the object.
     */
    QString toString() const;
};

/*!
 * A class that represents an UPnP output action argument.
 *
 * \remark the class is not thread-safe.
 *
 * \headerfile upnp_actionarguments.h HActionOutputArgument
 *
 * \ingroup devicemodel
 *
 * \sa HActionOutputArguments, HActionInputArgument, HActionInputArguments
 */
class H_UPNP_CORE_EXPORT HActionOutputArgument
{
friend class HObjectCreator;

private:

    HActionArgumentPrivate* h_ptr;

    //
    // \internal
    //
    // Constructs a new, empty instance.
    //
    // \remark object constructed using this method is always considered
    // invalid, in which case isValid() always returns false.
    //
    // \sa isValid()
    //
    HActionOutputArgument();

    //
    // \internal
    //
    // Constructs a new instance with the specified name and related state variable.
    //
    // \param name specifies the name of the argument
    // \param stateVariable specifies the related state variable.
    //
    // \remark in case the name parameter fails the criteria specified for
    // UPnP action arguments in UPnP Device Architecture 1.1 specification
    // or the stateVariable is null, the object is constructed as "invalid";
    // isValid() always returns false.
    //
    // \sa isValid()
    //
    HActionOutputArgument(const QString& name, HStateVariable* stateVariable);

public:

    /*!
     * Destroys the instance.
     */
    ~HActionOutputArgument();

    /*!
     * Copy constructor.
     */
    HActionOutputArgument(const HActionOutputArgument&);

    /*!
     * Assignment operator.
     */
    HActionOutputArgument& operator=(const HActionOutputArgument&);

    /*!
     * Returns the name of the argument.
     *
     * \return the name of the argument.
     */
    QString name() const;

    /*!
     * Returns the state variable that is associated with this action argument.
     *
     * \retval the state variable that is associated with this action argument.
     *
     * \retval null in case there is no state variable associated with this action
     * argument. This is the case only when isValid() returns false.
     */
    HStateVariable* relatedStateVariable() const;

    /*!
     * Helper method for accessing the data type of the related state variable
     * directly.
     *
     * \return the data type of the state variable.
     */
    HUpnpDataTypes::DataType dataType() const;

    /*!
     * Returns the value of the argument.
     *
     * \return the value of the argument.
     */
    QVariant value() const;

    /*!
     * Sets the value of the argument if the object is valid and the value is
     * of right type.
     *
     * \param value specifies the new value of the argument.
     *
     * \retval true in case the value was successfully set.
     * \retval false otherwise.
     */
    bool setValue(const QVariant& value);

    /*!
     * Indicates if the object is constructed with a proper name and a state
     * variable.
     *
     * \retval true in case the object has a proper name and the object refers
     * to a valid state variable.
     *
     * \retval false otherwise.
     */
    bool isValid() const;

    /*!
     * Indicates whether or not the object is considered as invalid.
     *
     * \retval true in case the object is invalid.
     * \retval false otherwise.
     */
    bool operator!() const;

    /*!
     * Returns a string representation of the object.
     *
     * \return a string representation of the object.
     */
    QString toString() const;
};

template<class T>
class HActionArgumentsPrivate;

/*!
 * A storage class for HActionInputArgument instances.
 *
 * Provides iterative and keyed access to the stored HActionInputArgument instances.
 *
 * \headerfile upnp_actionarguments.h HActionInputArguments
 *
 * \remark this class is not thread-safe.
 *
 * \ingroup devicemodel
 */
class H_UPNP_CORE_EXPORT HActionInputArguments
{
friend class HObjectCreator;

private:

    HActionArgumentsPrivate<HActionInputArgument>* h_ptr;

    //
    // \internal
    //
    // Creates a new, empty instance.
    //
    HActionInputArguments();

    //
    // \internal
    //
    // Creates a new instance from the specified input arguments.
    //
    explicit HActionInputArguments(const QList<HActionInputArgument>& args);

    //
    // \internal
    //
    // Creates a new instance from the specified input arguments.
    //
    // \param args specifies the input arguments, where the key of the hash table
    // is the name of the argument.
    //
    explicit HActionInputArguments(
        const QHash<QString, HActionInputArgument>& args);

public:

    typedef QList<HActionInputArgument* >::const_iterator const_iterator;
    typedef QList<HActionInputArgument* >::iterator iterator;

    /*!
     * Swaps the contents of the two containers.
     */
    friend void swap(HActionInputArguments& a, HActionInputArguments& b);

    /*!
     * Destroys the instance.
     */
    ~HActionInputArguments();

    /*!
     * Copy constructor.
     */
    HActionInputArguments(const HActionInputArguments&);

    /*!
     * Assignment operator.
     */
    HActionInputArguments& operator=(const HActionInputArguments&);

    /*!
     * Indicates if the object contains an argument with the specified name.
     *
     * \param argumentName specifies the name of the action argument.
     *
     * \retval true in case the object contains an argument with the specified name.
     * \retval false otherwise.
     */
    bool contains(const QString& argumentName) const;

    /*!
     * \overload
     */
    const HActionInputArgument* get(qint32 index) const;

    /*!
     * \overload
     */
    HActionInputArgument* get(qint32 index);

    /*!
     * \overload
     */
    const HActionInputArgument* get(const QString& argumentName) const;

    /*!
     * Attempts to retrieve an action argument with the specified name.
     *
     * \retval a pointer to the action argument with the specified name, if such is found.
     * \retval a null pointer if no such action argument is stored in this instance.
     *
     * \warning do not delete the return value.
     */
    HActionInputArgument* get(const QString& argumentName);

    /*!
     * Returns a const STL-style iterator pointing to the first item.
     */
    HActionInputArguments::const_iterator constBegin() const;

    /*!
     * Returns a const STL-style iterator pointing to the
     * imaginary item after the last item.
     */
    HActionInputArguments::const_iterator constEnd() const;

    /*!
     * Returns an STL-style iterator pointing to the first item.
     */
    HActionInputArguments::iterator begin();

    /*!
     * Returns an STL-style iterator pointing to the imaginary item after the last item.
     */
    HActionInputArguments::iterator end();

    /*!
     * \overload
     */
    HActionInputArguments::const_iterator begin() const;

    /*!
     * \overload
     */
    HActionInputArguments::const_iterator end() const;

    /*!
     * Returns the number of items.
     */
    qint32 size() const;

    /*!
     * Returns the action argument matching the specified index.
     *
     * \retval a pointer to the action argument, which name matches the provided argument.
     * \retval a null pointer, in case the specified argument name did not match
     * any stored argument.
     *
     * \sa HActionArgument<T>::isValid()
     */
    HActionInputArgument* operator[](qint32 index);

    /*!
     * \overload
     */
    const HActionInputArgument* operator[](qint32 index) const;

    /*!
     * Returns the action argument matching the specified name, if any.
     *
     * \retval a pointer to the action argument, which name matches the provided argument.
     * \retval a null pointer, in case the specified argument name did not match
     * any stored argument.
     *
     * \sa HActionArgument<T>::isValid()
     */
    HActionInputArgument* operator[](const QString& argName);

    /*!
     * \overload
     */
    const HActionInputArgument* operator[](const QString& argName) const;

    /*!
     * Returns a list of argument names that this instance contains.
     */
    QList<QString> names() const;

    /*!
     * Returns a string representation of the object.
     *
     * \return a string representation of the object.
     */
    QString toString() const;
};

/*!
 * A storage class for HActionOutputArgument instances.
 *
 * Provides iterative and keyed access to the stored HActionOutputArgument instances.
 *
 * \headerfile upnp_actionarguments.h HActionOutputArguments
 *
 * \remark this class is not thread-safe.
 *
 * \ingroup devicemodel
 */
class H_UPNP_CORE_EXPORT HActionOutputArguments
{
friend class HObjectCreator;

private:

    HActionArgumentsPrivate<HActionOutputArgument>* h_ptr;

    //
    // \internal
    //
    explicit HActionOutputArguments(const QList<HActionOutputArgument>& args);

    //
    // \internal
    //
    explicit HActionOutputArguments(
        const QHash<QString, HActionOutputArgument>& args);

public:

    typedef QList<HActionOutputArgument*>::const_iterator const_iterator;
    typedef QList<HActionOutputArgument*>::iterator iterator;

    /*!
     * Creates a new, empty instance.
     */
    HActionOutputArguments();

    /*!
     * Swaps the contents of the two containers.
     */
    friend void swap(HActionOutputArguments& a, HActionOutputArguments& b);

    /*!
     * Destroys the instance.
     */
    ~HActionOutputArguments();

    /*!
     * Copy constructor.
     */
    HActionOutputArguments(const HActionOutputArguments&);

    /*!
     * Assignment operator.
     */
    HActionOutputArguments& operator=(const HActionOutputArguments&);

    /*!
     * Indicates if the object contains an argument with the specified name.
     *
     * \param argumentName specifies the name of the action argument.
     *
     * \retval true in case the object contains an argument with the specified name.
     * \retval false otherwise.
     */
    bool contains(const QString& argumentName) const;

    /*!
     * \overload
     */
    const HActionOutputArgument* get(qint32 index) const;

    /*!
     * \overload
     */
    HActionOutputArgument* get(qint32 index);

    /*!
     * \overload
     */
    const HActionOutputArgument* get(const QString& argumentName) const;

    /*!
     * Attempts to retrieve an action argument with the specified name.
     *
     * \retval a pointer to the action argument with the specified name, if such is found.
     * \retval a null pointer if no such action argument is stored in this instance.
     *
     * \warning do not delete the return value.
     */
    HActionOutputArgument* get(const QString& argumentName);

    /*!
     * Returns a const STL-style iterator pointing to the first item.
     */
    HActionOutputArguments::const_iterator constBegin() const;

    /*!
     * Returns a const STL-style iterator pointing to the
     * imaginary item after the last item.
     */
    HActionOutputArguments::const_iterator constEnd() const;

    /*!
     * Returns an STL-style iterator pointing to the first item.
     */
    HActionOutputArguments::iterator begin();

    /*!
     * Returns an STL-style iterator pointing to the imaginary item after the last item.
     */
    HActionOutputArguments::iterator end();

    /*!
     * \overload
     */
    HActionOutputArguments::const_iterator begin() const;

    /*!
     * \overload
     */
    HActionOutputArguments::const_iterator end() const;

    /*!
     * Returns the number of items.
     */
    qint32 size() const;

    /*!
     * Returns the action argument matching the specified index.
     *
     * \retval a pointer to the action argument, which name matches the provided argument.
     * \retval a null pointer, in case the specified argument name did not match
     * any stored argument.
     *
     * \sa HActionArgument<T>::isValid()
     */
    HActionOutputArgument* operator[](qint32 index);

    /*!
     * \overload
     */
    const HActionOutputArgument* operator[](qint32 index) const;

    /*!
     * Returns the action argument matching the specified name, if any.
     *
     * \retval a pointer to the action argument, which name matches the provided argument.
     * \retval a null pointer, in case the specified argument name did not match
     * any stored argument.
     *
     * \sa HActionArgument<T>::isValid()
     */
    HActionOutputArgument* operator[](const QString& argName);

    /*!
     * \overload
     */
    const HActionOutputArgument* operator[](const QString& argName) const;

    /*!
     * Returns a list of argument names that this instance contains.
     */
    QList<QString> names() const;

    /*!
     * Returns a string representation of the object.
     *
     * \return a string representation of the object.
     */
    QString toString() const;
};

}
}
#endif /* UPNP_ACTIONARGUMENTS_H_ */
