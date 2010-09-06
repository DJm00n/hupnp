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

#ifndef HSTATEVARIABLE_H_
#define HSTATEVARIABLE_H_

#include <HUpnpCore/HUpnpDatatypes>

#include <QtCore/QObject>
#include <QtCore/QVariant>

class QStringList;

namespace Herqq
{

namespace Upnp
{

class HObjectCreator;
class HStateVariablePrivate;
class HStateVariableController;

/*!
 * Class that represents a UPnP state variable.
 *
 * \c %HStateVariable is a core component of the HUPnP \ref devicemodel
 * and it models a UPnP state variable. The UPnP Device Architecture specifies a
 * UPnP state variable as an item or aspect that models state in a service.
 * In a way, a UPnP state variable is an abstraction to a member variable, since it is
 * always contained within a UPnP service.
 *
 * A state variable can be \em evented in which case it notifies interested listeners
 * of changes in its value. You can see if a state variable is evented by checking
 * the HStateVariableInfo object using info() and you can connect to the signal
 * valueChanged() to be notified when the value of the state variable changes.
 * Note, only evented state variables emit the valueChanged() signal.
 *
 * \headerfile hstatevariable.h HStateVariable
 *
 * \ingroup devicemodel
 *
 * \sa HReadableStateVariable, HWritableStateVariable, HService
 *
 * \remarks The methods introduced in this class are thread-safe, but the \c QObject
 * base class is largely not. However, the signal valueChanged() has thread affinity
 * and any connections to it \b must be done in the thread where
 * the instance of HStateVariable resides.
 */
class H_UPNP_CORE_EXPORT HStateVariable :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HStateVariable)
H_DECLARE_PRIVATE(HStateVariable)
friend class HObjectCreator;
friend class HStateVariableController;

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
    bool init(const HStateVariableInfo& arg);

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
    // \remarks the new value will be set if the value:
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
     * containing root HDevice exists.
     *
     * \sa HDevice
     */
    HService* parentService() const;

    /*!
     * Returns the value of the state variable.
     *
     * \return the value of the state variable.
     */
    QVariant value() const;

    /*!
     * Returns information about the state variable that is read from the
     * service description.
     *
     * \return information about the state variable that is read from the
     * service description.
     */
    const HStateVariableInfo& info() const;

    /*!
     * Attempts to cast the instance to HWritableStateVariable.
     *
     * This is a helper method for performing a dynamic cast.
     *
     * \return this instance as HWritableStateVariable when the dynamic type
     * of the instance is HWritableStateVariable. Otherwise zero is returned.
     */
    HWritableStateVariable* writable();

    /*!
     * Attempts to cast the instance to HReadableStateVariable.
     *
     * This is a helper method for performing a dynamic cast.
     *
     * \return this instance as HReadableStateVariable when the dynamic type
     * of the instance is HReadableStateVariable. Otherwise zero is returned.
     */
    HReadableStateVariable* readable();

Q_SIGNALS:

    /*!
     * This signal is emitted when the value of the state variable has changed.
     *
     * \param event specifies information about the event that occurred.
     *
     * \remarks This signal has thread affinity to the thread where the object
     * resides. Do not connect to this signal from other threads.
     */
    void valueChanged(const Herqq::Upnp::HStateVariableEvent& event);
};

class HStateVariableEventPrivate;

/*!
 * A class used to transfer HStateVariable event information.
 *
 * \headerfile hstatevariable.h HStateVariableEvent
 *
 * \sa HStateVariable::valueChanged()
 *
 * \remarks this class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HStateVariableEvent
{
private:

    HStateVariableEventPrivate* h_ptr;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * Creates a new, invalid instance.
     *
     * \sa isValid()
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
     * \remarks in case the previousValue and newValue contains a different data
     * types, the values are ignored and the object will be invalid. Similarly, if
     * the eventSource is not defined, the object will be invalid.
     *
     * \sa isValid()
     */
    HStateVariableEvent(
        HStateVariable* eventSource,
        const QVariant& previousValue,
        const QVariant& newValue);

    /*!
     * Copy constructor.
     *
     * Copies the contents of the \c other to this.
     */
    HStateVariableEvent(const HStateVariableEvent&);

    /*!
     * Destroys the instance.
     *
     * Destroys the instance.
     */
    virtual ~HStateVariableEvent();

    /*!
     * Assigns the contents of the other object to this.
     * \return reference to this object.
     */
    HStateVariableEvent& operator=(const HStateVariableEvent&);

    /*!
     * Indicates whether the object is valid.
     *
     * \return \e true in case previousValue() and newValue() return a valid
     * \c QVariant and eventSource() is defined.
     */
    bool isValid() const;

    /*!
     * Returns the source state variable that generated the event.
     *
     * \return the source state variable that generated the event.
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
