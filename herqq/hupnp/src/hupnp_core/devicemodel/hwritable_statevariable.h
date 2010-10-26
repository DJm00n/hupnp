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

#ifndef HWRITABLE_STATEVARIABLE_H_
#define HWRITABLE_STATEVARIABLE_H_

#include <HUpnpCore/HStateVariable>

class QMutexLocker;

namespace Herqq
{

namespace Upnp
{

class HStateVariableLocker;
class HWritableStateVariablePrivate;

/*!
 * Class that provides \e write access to a state variable.
 *
 * \c %HWritableStateVariable is a core component of the HUPnP \ref hupnp_devicemodel
 * and it models a UPnP state variable, which allows \e read-write access. Typically,
 * instances of this class are available only on server-side.
 *
 * \attention if you need exclusive access to a writable state variable,
 * see HStateVariableLocker.
 *
 * \headerfile hwritable_statevariable.h HWritableStateVariable
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa hupnp_devicehosting, \ref statevariables,
 * HStateVariable, HReadableStateVariable, HService, HStateVariableLocker
 *
 * \remark
 * the methods introduced in this class are thread-safe, but the \c QObject
 * ancestor is largely not.
 */
class H_UPNP_CORE_EXPORT HWritableStateVariable :
    public HStateVariable
{
H_DISABLE_COPY(HWritableStateVariable)
H_DECLARE_PRIVATE(HWritableStateVariable)
friend class HObjectCreator;
friend class HStateVariableLocker;
friend class HStateVariableController;

private:

    //
    // \internal
    //
    // Constructs a new instance.
    //
    HWritableStateVariable(HService* parent);

public:

    /*!
     * Destroys the instance.
     *
     * An \c %HWritableStateVariable is always destroyed by the containing HService when it
     * is being deleted. You should never destroy an \c %HWritableStateVariable.
     */
    virtual ~HWritableStateVariable();

    /*!
     * Changes the value of the state variable.
     *
     * If the instance is evented the valueChanged() signal is emitted after
     * the value has been changed.
     *
     * \param newValue specifies the new value of the state variable. The new value
     * must have the same underlying data type as the previous value
     * (and the default value). If the new value has different data type, the value
     * is not changed, no event is sent and false is returned.
     *
     * \retval true in case the new value was successfully set.
     * \retval false in case the new value could not be set.
     *
     * \remarks the new value will be set if the value:
     *  - does not violate the defined constraints
     *  - has the same variant type or the type of the new value can be converted
     *  to the same variant type
     *  - is not QVariant::Invalid
     */
    bool setValue(const QVariant& newValue);
};

/*!
 * A class that enables \e locking an HWritableStateVariable for exclusive access.
 *
 * The HWritableStateVariable provides read-write access to its value. The methods
 * HWritableStateVariable::value() and HWritableStateVariable::setValue() are
 * thread-safe, but that is not enough when multiple \c value() and \c setValue()
 * calls have to be performed serially. In other words, the following code
 * is not thread-safe without additional measures:
 *
 * \code
 * void example()
 * {
 *     HWritableStateVariable* sv =
 *         stateVariableByName("MyIntegerVariable")->writable();
 *
 *     quint32 count = sv->value().toUInt(&ok);
 *     sv->setValue(++count);
 *     // WRONG! The state variable might have been modified by another thread before
 *     // the call to setValue() is done, effectively overwriting the value of
 *     // the state variable with stale data.
 * }
 * \endcode
 *
 * The correct and thread-safe way to do multiple consecutive operations that depend
 * on former operations with a state variable is demonstrated in the following:
 *
 * \code
 * void example()
 * {
 *     HWritableStateVariable* sv =
 *         stateVariableByName("MyIntegerVariable")->writable();
 *
 *     HStateVariableLocker svLocker(sv);
 *     // this will guarantee that we have exclusive access to the state variable
 *     // until the lock is destroyed or explicitly unlocked using
 *     // HStateVariableLocker::unlock()
 *
 *     quint32 count = sv->value().toUInt(&ok);
 *     sv->setValue(++count);
 * }
 * \endcode
 *
 * \headerfile hwritable_statevariable.h HStateVariableLocker
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HWritableStateVariable
 *
 * \remark
 * \li this class is thread-safe.
 * \li the lock is \e recursive, by which it is meant that a same thread
 * can lock the same state variable multiple times using one or more
 * \c %HStateVariableLockers. However, the state variable will not be unlocked
 * before the corresponding number of unlock() calls have been made.
 */
class H_UPNP_CORE_EXPORT HStateVariableLocker
{
H_DISABLE_COPY(HStateVariableLocker)

private:

    HWritableStateVariable* m_stateVariable;
    QMutexLocker* m_locker;

public:

    /*!
     * Creates a new instance and locks the state variable for exclusive access.
     *
     * \param stateVariable specifies the writable state variable that will be
     * locked for exclusive access.
     *
     * \remark the call will block the current thread until the state variable
     * can be locked for exclusive access for the thread.
     */
    explicit HStateVariableLocker(HWritableStateVariable* stateVariable);

    /*!
     * Calls unlock() and destroys the instance.
     *
     * Calls unlock() and destroys the instance.
     *
     * \remark the state variable will be unlocked only after it has been
     * unlocked as many times it has been locked by the current thread.
     */
    ~HStateVariableLocker();

    /*!
     * Unlocks the state variable from exclusive access.
     *
     * Unlocks the state variable from exclusive access if the state variable
     * was locked by this thread and the number of unlock() calls matches the
     * consecutive number of times the state variable has been locked by the current thread.
     */
    void unlock();

    /*!
     * Attempts to lock the state variable for exclusive access.
     *
     * Attempts to lock the state variable for exclusive access if the
     * state variable is not already locked by the instance, or block the
     * current thread until the state variable can be locked for exclusive access.
     *
     * \remark a thread can lock the state variable multiple times and the lock
     * will be unlocked only after the corresponding number of unlock() calls
     * have been made.
     */
    void relock();
};

}
}

#endif /* HWRITABLE_STATEVARIABLE_H_ */
