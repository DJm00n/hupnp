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

#ifndef HSTATEVARIABLE_H_
#define HSTATEVARIABLE_H_

#include "./../general/hdefs_p.h"
#include "./../datatypes/hupnp_datatypes.h"

#include <QList>
#include <QObject>
#include <QVariant>

class QStringList;

namespace Herqq
{

namespace Upnp
{

class HService;
class HObjectCreator;
class HStateVariableEvent;
class HStateVariablePrivate;
class HStateVariableController;
class HWritableStateVariable;
class HReadableStateVariable;

/*!
 * Class that represents a UPnP state variable.
 *
 * \c %HStateVariable is a core component of the HUPnP \ref devicemodel
 * and it models a UPnP state variable. The UPnP Device Architecture specifies a
 * UPnP state variable as an item or aspect that models state in a service.
 * In a way, a UPnP state variable is an abstraction to a member variable, since it is
 * always contained within a UPnP service.
 *
 * A state variable can be \em evented, in which case it notifies interested listeners
 * of changes in its value. You can see if a state variable is evented by calling
 * eventingType() and you can connect to the signal valueChanged() to be notified
 * when the value of the state variable changes. Note, however, that only evented
 * state variables emit the valueChanged signal.
 *
 * \headerfile hstatevariable.h HStateVariable
 *
 * \ingroup devicemodel
 *
 * \sa HReadableStateVariable, HWritableStateVariable, HService
 *
 * \remark the methods introduced in this class are thread-safe, but the \c QObject
 * base class is largely not.
 */
class H_UPNP_CORE_EXPORT HStateVariable :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HStateVariable)
H_DECLARE_PRIVATE(HStateVariable)
friend class HObjectCreator;
friend class HStateVariableController;

public:

    /*!
     * Specifies different types of eventing.
     *
     * \sa devicehosting
     */
    enum EventingType
    {
        /*!
         * The state variable is not evented and it will never emit
         * valueChanged() signal.
         */
        NoEvents = 0,

        /*!
         * The state variable is evented, valueChanged() signal is emitted upon
         * value change and the HUPnP will propagate events over network
         * to registered listeners through unicast only.
         */
        UnicastOnly = 1,

        /*!
         * The state variable is evented, valueChanged() signal is emitted upon
         * value change and the HUPnP will propagate events over network
         * using uni- and multicast.
         */
        UnicastAndMulticast = 2
    };

protected:

    HStateVariablePrivate* h_ptr;

    //
    // \internal
    //
    HStateVariable(HService* parent);
    HStateVariable(HStateVariablePrivate& dd, HService* parent);

    //
    // \internal
    //
    void init(
        const QString& name, HUpnpDataTypes::DataType datatype,
        const QVariant& defaultValue, EventingType eventingType = NoEvents);

    //
    // \internal
    //
    void init(
        const QString& name, const QVariant& defaultValue,
        const QStringList& allowedValueList, EventingType eventingType = NoEvents);

    //
    // \internal
    //
    void init(
        const QString& name, HUpnpDataTypes::DataType datatype,
        const QVariant& defaultValue, const QVariant& minimumValue,
        const QVariant& maximumValue, const QVariant& stepValue,
        EventingType eventingType = NoEvents);

    //
    // \internal
    //
    // Changes the value of the state variable. If the instance is evented
    // (sendsEvents() returns true), after the value has been changed,
    // valueChanged() signal is emitted.
    //
    // \param newValue specifies the new value of the state variable. The new value
    // must have the same underlying data type as the previous value
    // (and the default value). If the new value has different data type, the value
    // is not changed, no event is sent and false is returned.
    //
    // \retval true in case the new value was successfully set.
    // \retval false in case the new value could not be set.
    //
    // \remark the new value will be set if the value:
    //  - does not violate the defined constraints
    //  - has the same variant type or the type of the new value can be converted
    //  to the same variant type
    //  - is not QVariant::Invalid
    //
    bool setValue(const QVariant& newValue);

public:

    /*!
     * Destroys the instance.
     *
     * An \c %HStateVariable is always destroyed by the containing HService when it
     * is being deleted. You should never destroy an \c %HStateVariable.
     */
    virtual ~HStateVariable() = 0;

    /*!
     * Returns the HService that contains this state variable.
     *
     * \return the HService that contains this state variable.
     *
     * \warning the pointer is guaranteed to point to a valid object as long
     * as the \c %HStateVariable exists, which ultimately is as long as the
     * containing HRootDevicePtrT exists.
     *
     * \sa HRootDevicePtrT, HDevice
     */
    HService* parentService() const;

    /*!
     * Returns the data type of the state variable.
     *
     * \return the data type of the state variable.
     */
    HUpnpDataTypes::DataType dataType() const;

    /*!
     * Returns the name of the state variable.
     *
     * \return the name of the state variable.
     */
    QString name() const;

    /*!
     * Indicates the type of eventing this state variable supports, if any.
     *
     * \return the type of eventing this state variable supports, if any.
     */
    EventingType eventingType() const;

    /*!
     * Returns the list of allowed values.
     *
     * \return the list of allowed values if the contained data type is string
     * or empty list otherwise.
     *
     * \remark this is only applicable on state variables, which data type is
     * \e string.
     *
     * \sa dataType()
     */
    QStringList allowedValueList() const;

    /*!
     * Returns the minimum value of the specified value range.
     *
     * \return the minimum value of the specified value range.
     *
     * \remark this is only applicable on state variables, which data type is
     * numeric. In addition, it is optional and it may not be defined.
     *
     * \sa dataType()
     */
    QVariant minimumValue() const;

    /*!
     * Returns the maximum value of the specified value range.
     *
     * \return the maximum value of the specified value range.
     *
     * \remark this is only applicable on state variables, which data type is
     * numeric. In addition, it is optional and it may not be defined.
     *
     * \sa dataType()
     */
    QVariant maximumValue() const;

    /*!
     * Returns the step value of the specified value range.
     *
     * \return the step value of the specified value range.
     *
     * \remark this is only applicable on state variables, which data type is
     * numeric. In addition, it is optional and it may not be defined.
     *
     * \sa dataType()
     */
    QVariant stepValue() const;

    /*!
     * Returns the default value of the state variable.
     *
     * \return the default value of the state variable. If no default has been
     * specified, QVariant::Invalid is returned.
     */
    QVariant defaultValue() const;

    /*!
     * Returns the value of the state variable.
     *
     * \return the value of the state variable.
     */
    QVariant value() const;

    /*!
     * Indicates if the state variable's value is constrained either by minimum,
     * maximum or by a list of allowed values.
     *
     * \return true in case the state variable's value is constrained either by minimum,
     * maximum or by a list of allowed values.
     *
     * \sa minimumValue(), maximumValue(), allowedValueList()
     */
    bool isConstrained() const;

    /*!
     * Indicates whether or not the value is valid in terms of this particular
     * state variable.
     *
     * \param value specifies the value to be checked.
     * \param convertedValue specifies the exact value that would be used if
     * the specified value is considered valid. Since two different variant types can contain
     * the same value, it is sometimes useful to know the variant type that is
     * used to hold the value as well.
     *
     * \retval true in case the specified value is valid to this state variable.
     * In other words, setValue() will succeed when called with this argument.
     *
     * \retval false otherwise.
     */
    bool isValidValue(const QVariant& value, QVariant* convertedValue = 0) const;

    /*!
     * Attempts to cast the instance to HWritableStateVariable.
     *
     * This is a helper method for performing a dynamic cast.
     *
     * \return this instance as HWritableStateVariable when the dynamic type
     * of the instance is HWritableStateVariable. Otherwise zero is returned.
     */
    HWritableStateVariable* toWritable();

    /*!
     * Attempts to cast the instance to HReadableStateVariable.
     *
     * This is a helper method for performing a dynamic cast.
     *
     * \return this instance as HReadableStateVariable when the dynamic type
     * of the instance is HReadableStateVariable. Otherwise zero is returned.
     */
    HReadableStateVariable* toReadable();

Q_SIGNALS:

    /*!
     * This signal is emitted when the value of the state variable has changed.
     *
     * \param event specifies information about the event that occurred.
     */
    void valueChanged(const Herqq::Upnp::HStateVariableEvent& event);
};

class HStateVariableEventPrivate;

/*!
 * A class used to transfer HStateVariable event information.
 *
 * \headerfile hstatevariable.h HStateVariable
 *
 * \sa HStateVariable::valueChanged()
 *
 * \remark this class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HStateVariableEvent
{
private:

    HStateVariableEventPrivate* h_ptr;

public:

    /*!
     * Creates a new, empty instance.
     */
    HStateVariableEvent();

    /*!
     * Creates a new instance based on the provided values.
     *
     * \param eventSource specifies the state variable that generated the event.
     *
     * \param previousValue specifies the value before the value changed.
     *
     * \param newValue specifies the newly set value.
     *
     * \remark in case the previousValue and newValue contains a different data
     * types, the values are ignored and the object is set to empty. Similarly, if
     * the eventSource is not defined, the object is constructed empty.
     *
     * \sa isEmpty()
     */
    HStateVariableEvent(
        HStateVariable* eventSource, const QVariant& previousValue,
        const QVariant& newValue);

    /*!
     * Copies the contents of the other object to this.
     */
    HStateVariableEvent(const HStateVariableEvent&);

    /*!
     * Destroys the instance.
     */
    virtual ~HStateVariableEvent();

    /*!
     * Assigns the contents of the other object to this.
     * \return reference to this object.
     */
    HStateVariableEvent& operator=(const HStateVariableEvent&);

    /*!
     * Indicates whether or not the object contains any information.
     *
     * \return true in case previousValue() and newValue() return a null QVariant and
     * eventSource() returns null.
     */
    bool isEmpty() const;

    /*!
     * Returns the source state variable that generated the event.
     *
     * \return the source state variable of the event.
     */
    HStateVariable* eventSource() const;

    /*!
     * Returns the previous value of the state variable.
     *
     * \return the previous value of the state variable.
     */
    QVariant previousValue() const;

    /*!
     * Returns the new, changed value of the state variable.
     *
     * \return the new, changed value of the state variable.
     */
    QVariant newValue() const;
};

}
}

#endif /* HSTATEVARIABLE_H_ */
