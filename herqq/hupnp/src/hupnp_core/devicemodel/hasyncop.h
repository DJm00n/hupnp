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

#ifndef HASYNCOP_H_
#define HASYNCOP_H_

#include <HUpnpCore/HUpnp>

#include <QtCore/QUuid>

namespace Herqq
{

namespace Upnp
{

class HAsyncOpPrivate;

/*!
 * This class is used to identify an asynchronous operation and detail information
 * of it.
 *
 * Some HUPnP components provide an asynchronous interface for running possible
 * long-standing operations. A most notable example of this is the client-side
 * action invocation initiated with HClientAction::beginInvoke(). In cases
 * like this, instances of this class are used to identify and describe the
 * operations.
 *
 * \section Usage
 *
 * The component that runs an asynchronous operation provides an instance
 * of this class when the operation is started. A copy of this instance is
 * provided also when the component signals the operation is complete.
 * The provided instance uniquely identifies the operation, carries information
 * whether the operation eventually succeeded or not, may contain an error
 * description in case of an error and can be used to pass user-defined data from
 * instance to another.
 *
 * For example:
 *
 * \code
 *
 * HAsyncOp op = someObject->beginSomeAsyncOp();
 *
 * //
 * // The operation completes, after which you can:
 * //
 *
 * int retVal = op.returnValue();
 * // retrieve a return value indicating whether the operation succeeded.
 *
 * QString errDescr = op.errorDescription();
 * // retrieve an error description if the operation failed.
 *
 * \endcode
 *
 * In some scenarios it is useful to pass custom data within an HAsyncOp.
 * For example,
 *
 * \code
 *
 * void MyQObject::slotToBeCalledWhenAsyncOpCompletes(HAsyncOp op)
 * {
 *     SomeClass* someObject = reinterpret_cast<SomeClass*>(op.userData());
 *     someObject->waitForSomeAsyncOp(&op);
 * }
 *
 * void MyQObject::someMethod()
 * {
 *     HAsyncOp op = someObject->beginSomeAsyncOp();
 *     op.setUserData(reinterpret_cast<void*>(someObject));
 *     // call executes and the above slot gets called once the operation completes
 *     // (or fails)
 * }
 * \endcode
 *
 * Note, the contents of the instance are retrievable from \b any copy of the
 * object. That is, the copy constructor and assignment operator make shallow
 * copies of the contents. So for example if an \c %HAsyncOp instance is created
 * by the runner of an asynchronous operation, setting the userData of that
 * instance will associate the userData with all the copies the runner uses too.
 * From this follows that when the runner informs the user an operation is
 * finished, the provided HAsyncOp object contains the previously set userData
 * and you can read the returnValue() from the originally received instance.
 *
 * Note also that the user data is never referenced by the runner of an
 * asynchronous operation. This also means that the ownership of the data is
 * never transferred and you have to ensure the memory is handled correctly in
 * that regard.
 *
 * \headerfile hasyncop.h HAsyncOp
 *
 * \ingroup hupnp_devicemodel
 *
 * \remarks this class is thread-safe.
 */
class H_UPNP_CORE_EXPORT HAsyncOp
{
friend H_UPNP_CORE_EXPORT bool operator==(const HAsyncOp&, const HAsyncOp&);

private:

    HAsyncOpPrivate* h_ptr;
    HAsyncOp(qint32 returnCode, const QString& errorDescription);

public:

    /*!
     * \brief Creates a new valid instance.
     *
     * Creates a new valid instance, i.e isNull() always returns \e false.
     *
     * \sa isNull(), createInvalid()
     */
    HAsyncOp();

    /*!
     * \brief Destroys the instance.
     *
     * Decreases the reference count or destroys the instance once the reference
     * count drops to zero.
     */
    ~HAsyncOp();

    /*!
     * Copy constructor.
     *
     * Creates a shallow copy of \a other increasing the reference count of
     * \a other.
     */
    HAsyncOp(const HAsyncOp&);

    /*!
     * Assignment operator.
     *
     * Switches this instance to refer to the contents of \a other increasing the
     * reference count of \a other.
     */
    HAsyncOp& operator=(const HAsyncOp&);

    /*!
     * Returns a human readable error description.
     *
     * \return a human readable error description, if any.
     *
     * \sa setErrorDescription()
     */
    QString errorDescription() const;

    /*!
     * Sets a human readable error description.
     *
     * \param arg specifies the human readable error description.
     *
     * \sa errorDescription()
     */
    void setErrorDescription(const QString& arg);

    /*!
     * Returns the return value of the asynchronous operation.
     *
     * \sa setReturnValue()
     */
    int returnValue() const;

    /*!
     * Sets the return value of the asynchronous operation.
     *
     * \param returnValue specifies the return value of the asynchronous operation.
     *
     * \sa returnValue()
     */
    void setReturnValue(int returnValue);

    /*!
     * Associates arbitrary user provided data with the asynchronous operation.
     *
     * \param userData is the pointer to arbitrary user data.
     *
     * \remarks The instance never references the provided data.
     *
     * \sa userData()
     */
    void setUserData(void* userData);

    /*!
     * Returns the user provided data if set.
     *
     * \return a pointer to user provided data or null if the user data isn't set.
     *
     * \sa setUserData()
     */
    void* userData() const;

    /*!
     * Returns an identifier of the asynchronous operation.
     *
     * \return an identifier of the asynchronous operation. The identifier
     * is "unique" within the process where the library is loaded. More specifically,
     * the ID is monotonically incremented and it is allowed to overflow.
     */
    unsigned int id() const;

    /*!
     * Indicates whether the object identifies an asynchronous operation.
     *
     * \return \e true in case the object identifies an asynchronous operation.
     */
    bool isNull() const;

    /*!
     * Creates a new invalid instance.
     *
     * An invalid HAsyncOp represents an asynchronous operation that failed
     * to begin. Note, isNull() returns \e true always.
     *
     * \param returnCode specifies the return code.
     *
     * \param errorDescr specifies the human readable error description.
     *
     * \sa returnCode(), errorDescription(), isNull()
     */
    static HAsyncOp createInvalid(int returnCode, const QString& errorDescr);
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the object are logically equivalent.
 *
 * \relates HAsyncOp
 */
H_UPNP_CORE_EXPORT bool operator==(const HAsyncOp&, const HAsyncOp&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the object are not logically equivalent.
 *
 * \relates HAsyncOp
 */
H_UPNP_CORE_EXPORT bool operator!=(const HAsyncOp&, const HAsyncOp&);

/*!
 * Returns a value that can be used as a unique key in a hash-map identifying
 * the object.
 *
 * \param key specifies the HAsyncOp object from which the hash value is created.
 *
 * \return a value that can be used as a unique key in a hash-map identifying
 * the object.
 *
 * \relates HAsyncOp
 */
inline quint32 qHash(const HAsyncOp& key) { return key.id(); }

}
}

#endif /* HASYNCOP_H_ */
