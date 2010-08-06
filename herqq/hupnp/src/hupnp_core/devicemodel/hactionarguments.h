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

#ifndef HACTIONARGUMENTS_H_
#define HACTIONARGUMENTS_H_

#include "../general/hdefs_p.h"
#include "../datatypes/hupnp_datatypes.h"
#include "../dataelements/hstatevariableinfo.h"

template<typename T, typename U>
class QHash;

template<typename T>
class QList;

template<typename T>
class QVector;

#include <QString>
#include <QVariant>

namespace Herqq
{

namespace Upnp
{

/*!
 * A class that represents an argument for a UPnP action invocation.
 *
 * A UPnP argument is defined in the UPnP service description within
 * an action. If you picture a UPnP action as a function, then an
 * action argument is a parameter to the function. In that sense, a UPnP
 * \e input \e argument is a single \b constant parameter that provides
 * input for the function. An input argument is never modified during action
 * invocation. On the other hand, a UPnP \e output \e argument relays information
 * back from the callee to the caller and thus it is modified during action invocation.
 *
 * A UPnP argument has an unique name() within the definition
 * of the action that contains it. A UPnP argument contains a value, which you
 * can retrieve using value() and which you can set using setValue(). Note, the
 * value of a UPnP argument is bound by its dataType().
 *
 * A somewhat unusual aspect of a UPnP argument is the concept of a
 * <em>related state variable</em>. According to the UDA specification, a
 * UPnP argument is \b always associated with a HStateVariable, even if the
 * state variable does not serve any other purpose besides that.
 * This type of a state variable
 * is used to describe the data type of a UPnP argument and thus the value of a
 * UPnP argument is bound by the data type of its related state variable.
 * The dataType() method introduced in this class is equivalent for calling
 * \verbatim relatedStateVariable()->dataType() \endverbatim
 *
 * \note
 * relatedStateVariable() returns a const reference to an
 * HStateVariableInfo object, rather than a reference or
 * a pointer to an HStateVariable. HStateVariableInfo is an object with value semantics
 * that describes an HStateVariable.
 *
 * Since it is common for actions
 * to use both input and output arguments that are defined only for the duration of
 * the action invocation, there are bound to be numerous state variables that
 * exist only for UPnP action invocation. It is defined in the UDA specification
 * that these types of state variables have to have a name that includes the
 * prefix \b A_ARG_TYPE.
 *
 * Due to the strict typing of UPnP arguments, HUPnP attempts to make sure that
 * invalid values are not entered into a UPnP argument. Because of this, you can
 * call isValidValue() to check if a value you wish to set using setValue()
 * will be accepted. In addition, the setValue() returns false in case the value
 * was not accepted. It is advised that you make sure your values are properly
 * set before attempting action invocation, since the invocation is likely to
 * fail in case any of the provided arguments is invalid.
 *
 * Finally, you can use isValid() to check if the object itself is valid, which
 * is true if the object was constructed with a proper name and a related state
 * variable.
 *
 * \remarks the class is not thread-safe.
 *
 * \headerfile hactionarguments.h HActionArgument
 *
 * \ingroup devicemodel
 *
 * \sa HActionArguments, HAction
 */
class H_UPNP_CORE_EXPORT HActionArgument
{
friend H_UPNP_CORE_EXPORT bool operator==(
    const HActionArgument&, const HActionArgument&);

private:

    QString m_name;
    HStateVariableInfo m_stateVariableInfo;
    QVariant m_value;

public:

    /*!
     * Constructs a new, empty instance.
     *
     * \remarks Object constructed using this method is always invalid.
     *
     * \sa isValid()
     */
    HActionArgument();

    /*!
     *
     * Initializes a new instance with the specified name and related state variable.
     *
     * \param name specifies the name of the argument
     * \param stateVariableInfo specifies the related state variable.
     *
     * \remarks in case the name parameter fails the criteria specified for
     * UPnP action arguments in UPnP Device Architecture 1.1 specification
     * or the stateVariable is null, the object is constructed as "invalid";
     * isValid() always returns false.
     *
     * \sa isValid()
     */
    HActionArgument(
        const QString& name, const HStateVariableInfo& stateVariableInfo);

    /*!
     * Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HActionArgument(const HActionArgument&);

    /*!
     * Assignment operator.
     *
     * Copies the contents of \c other to this.
     */
    HActionArgument& operator=(const HActionArgument&);

    /*!
     * Destroys the instance.
     *
     * Destroys the instance.
     */
    ~HActionArgument();

    /*!
     * Returns the name of the argument.
     *
     * \return the name of the argument. The return value is an empty string in
     * case the object is invalid.
     *
     * \sa isValid()
     */
    QString name() const;

    /*!
     * Returns information about the state variable that is associated
     * with this action argument.
     *
     * \return information about the state variable that is associated
     * with this action argument or a null pointer in case the object is invalid.
     *
     * \sa isValid()
     */
    const HStateVariableInfo& relatedStateVariable() const;

    /*!
     * Helper method for accessing the data type of the related state variable
     * info object directly.
     *
     * \return the data type of the state variable. The data type is
     * HUpnpDataTypes::Undefined in case the object is invalid.
     *
     * \sa isValid()
     */
    HUpnpDataTypes::DataType dataType() const;

    /*!
     * Returns the value of the argument.
     *
     * \return the value of the argument. The returned \c QVariant has a type of
     * \c QVariant::Invalid in case the object is invalid.
     *
     * \sa isValid()
     */
    QVariant value() const;

    /*!
     * Sets the value of the argument if the object is valid and the new value is
     * of right type.
     *
     * \param value specifies the new value of the argument.
     *
     * \return \e true in case the new value was successfully set.
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
     * This is the opposite for calling isValid().
     *
     * \return \e true in case the object is invalid.
     *
     * \sa isValid()
     */
    bool operator!() const;

    /*!
     * Returns a string representation of the object.
     *
     * The format of the return value is \c "name: theValue".
     *
     * \return a string representation of the object.
     */
    QString toString() const;

    /*!
     * Indicates if the provided value can be set into this input argument
     * successfully.
     *
     * A value is considered \e valid, when:
     * \li the argument object is valid, i.e. isValid() returns true and
     * \li the data type of the provided value matches the data type of the argument or
     * \li the data type of the provided value can be converted to the data type
     * of the argument.
     *
     * \param value specifies the value to be checked.
     *
     * \return \e true in case the provided value can be set into this input argument
     * successfully.
     */
    bool isValidValue(const QVariant& value);
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the object are logically equivalent.
 *
 * \relates HActionArgument
 */
H_UPNP_CORE_EXPORT bool operator==(
    const HActionArgument&, const HActionArgument&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the object are not logically equivalent.
 *
 * \relates HActionArgument
 */
H_UPNP_CORE_EXPORT bool operator!=(
    const HActionArgument&, const HActionArgument&);

class HActionArgumentsPrivate;

/*!
 * A storage class for HActionArgument instances.
 *
 * Instances of this class are used to contain the input and output arguments
 * for an action invocation.
 *
 * \note
 * The class provides iterative and keyed access to the stored HActionArgument instances.
 * The order of action arguments during iteration is the order in which the
 * HActionArgument objects are provided to the instance.
 * If the class is instantiated by HUPnP, the order of the contained arguments
 * during iteration is the order in which they are defined in the service
 * description document.
 *
 * \headerfile hactionarguments.h HActionArguments
 *
 * \ingroup devicemodel
 *
 * \sa HActionArgument, HAction
 *
 * \remarks this class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HActionArguments
{
friend H_UPNP_CORE_EXPORT bool operator==(
    const HActionArguments&, const HActionArguments&);

private:

    HActionArgumentsPrivate* h_ptr;

public:

    typedef HActionArgument** iterator;
    typedef const HActionArgument* const* const_iterator;

    /*!
     * Swaps the contents of the two containers.
     *
     * Swaps the contents of the two containers.
     *
     * \relates HActionArguments
     */
    friend H_UPNP_CORE_EXPORT void swap(HActionArguments&, HActionArguments&);

    /*!
     * Creates a new, empty instance.
     *
     * \sa isEmpty()
     */
    HActionArguments();

    /*!
     * Creates a new instance from the specified input arguments and takes the
     * ownership of the provided arguments.
     *
     * \sa isEmpty()
     */
    HActionArguments(const QVector<HActionArgument*>& args);

    /*!
     * Copy constructor.
     *
     * Creates a copy of \c other.
     */
    HActionArguments(const HActionArguments&);

    /*!
     * Destroys the instance.
     *
     * Destroys the instance.
     */
    ~HActionArguments();

    /*!
     * Assignment operator.
     *
     * Copies the contents of \c other to this.
     *
     * \return a reference to this object.
     */
    HActionArguments& operator=(const HActionArguments&);

    /*!
     * Indicates if the object contains an argument with the specified name.
     *
     * \param argumentName specifies the name of the action argument.
     *
     * \return \e true in case the object contains an argument with the specified name.
     *
     * \remarks this is a \e constant-time operation.
     */
    bool contains(const QString& argumentName) const;

    /*!
     * Retrieves an action argument.
     *
     * Retrieves an action argument with the specified name.
     *
     * \param argumentName specifies the name of the argument to be retrieved.
     *
     * \return a pointer to the action argument with the specified name
     * or a null pointer in case no argument has the specified name.
     *
     * \warning
     * \li Do not delete the return value. The ownership of the object is
     * not transferred.
     * \li The returned object is deleted when this container is being deleted.
     *
     * \remarks this is a \e constant-time operation.
     */
    HActionArgument* get(const QString& argumentName);

    /*!
     * \overload
     *
     * \param argumentName specifies the name of the argument to be retrieved.
     *
     * \return a pointer to the action argument with the specified name
     * or a null pointer in case no argument has the specified name.
     */
    const HActionArgument* get(const QString& argumentName) const;

    /*!
     * Retrieves an action argument.
     *
     * Retrieves an action argument from the specified \e index.
     *
     * \param index specifies the index of the action argument to return. The
     * index has to be valid position in the container, i.e. it must be
     * 0 <= i < size().
     *
     * \return a pointer to the action argument that can be found at the specified
     * index.
     *
     * \warning
     * \li Do not delete the return value. The ownership of the object is
     * not transferred.
     * \li The returned object is deleted when this container is being deleted.
     *
     * \remarks this is a \e constant-time operation.
     */
    HActionArgument* get(qint32 index);

    /*!
     * \overload
     *
     * \param index specifies the index of the action argument to return. The
     * index has to be valid position in the container, i.e. it must be
     * 0 <= i < size().
     *
     * \return a pointer to the action argument that can be found at the specified
     * index.
     */
    const HActionArgument* get(qint32 index) const;

    /*!
     * Returns a const STL-style iterator pointing to the first item.
     *
     * \return a const STL-style iterator pointing to the first item.
     */
    HActionArguments::const_iterator constBegin() const;

    /*!
     * Returns a const STL-style iterator pointing to the
     * imaginary item after the last item.
     *
     * \return a const STL-style iterator pointing to the
     * imaginary item after the last item.
     */
    HActionArguments::const_iterator constEnd() const;

    /*!
     * Returns an STL-style iterator pointing to the first item.
     *
     * \return an STL-style iterator pointing to the first item.
     */
    HActionArguments::iterator begin();

    /*!
     * \overload
     *
     * \return an STL-style iterator pointing to the first item.
     */
    HActionArguments::const_iterator begin() const;

    /*!
     * Returns an STL-style iterator pointing to the imaginary item
     * after the last item.
     *
     * \return an STL-style iterator pointing to the imaginary item
     * after the last item.
     */
    HActionArguments::iterator end();

    /*!
     * \overload
     *
     * \return an STL-style iterator pointing to the imaginary item
     * after the last item.
     */
    HActionArguments::const_iterator end() const;

    /*!
     * Returns the number of arguments.
     *
     * \return the number of arguments.
     */
    qint32 size() const;

    /*!
     * Returns the action argument matching the specified index.
     *
     * This is the same as calling get() with the specified index. This method is
     * provided for convenience.
     *
     * \param index specifies the index of the action argument to return. The
     * index has to be valid position in the container, i.e. it must be
     * 0 <= i < size().
     *
     * \return the action argument matching the specified index.
     */
    HActionArgument* operator[](qint32 index);

    /*!
     * \overload
     *
     * \param index specifies the index of the action argument to return. The
     * index has to be valid position in the container, i.e. it must be
     * 0 <= i < size().
     *
     * \return the action argument matching the specified index.
     */
    const HActionArgument* operator[](qint32 index) const;

    /*!
     * Returns the action argument matching the specified name, if any.
     *
     * This is the same as calling get() with the specified argument name.
     * This method is provided for convenience.
     *
     * \param argName specifies the name of the argument to be retrieved.
     *
     * \return the action argument matching the specified name, if any.
     */
    HActionArgument* operator[](const QString& argName);

    /*!
     * \overload
     *
     * \param argName specifies the name of the argument to be retrieved.
     *
     * \return the action argument matching the specified name, if any.
     */
    const HActionArgument* operator[](const QString& argName) const;

    /*!
     * The names of all the arguments.
     *
     * \return names of all the arguments.
     */
    QList<QString> names() const;

    /*!
     * Indicates if the object is empty, i.e. it has no arguments.
     *
     * \return \e true when the object has no arguments.
     */
    bool isEmpty() const;

    /*!
     * Returns a string representation of the object.
     *
     * \return a string representation of the object. The
     * returned string contains all the arguments represented as strings and
     * separated from each other by a new-line. The string representation of
     * an argument is retrieved using HActionArgument::toString().
     */
    QString toString() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the object are logically equivalent.
 *
 * \relates HActionArguments
 */
H_UPNP_CORE_EXPORT bool operator==(
    const HActionArguments&, const HActionArguments&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the object are not logically equivalent.
 *
 * \relates HActionArguments
 */
H_UPNP_CORE_EXPORT bool operator!=(
    const HActionArguments&, const HActionArguments&);

}
}
#endif /* HACTIONARGUMENTS_H_ */
