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

#include "../general/hupnp_global.h"

#include <QUuid>
#include <QSharedPointer>

namespace Herqq
{

namespace Upnp
{

/*!
 * This class is used to identify an asynchronous operation and detail information
 * of it.
 *
 * Some HUPnP components provide an asynchronous interface for running possible
 * long-standing operations. A most notable example of this is the action
 * invocation (HAction::beginInvoke()). In these cases this class is used
 * to identify, describe and control some aspects of the execution and
 * the wait for the completion of the operation.
 *
 * \section Usage
 *
 * The component that runs an asynchronous operation always provides an instance
 * of this class when the operation is started and when it signals the operation
 * is complete. The provided instance identifies the operation and it, or any
 * copy of it is provided to the runner of the asynchronous operation when the
 * result of the operation is retrieved or waited upon.
 *
 * For example:
 *
 * \code
 *
 * HAsyncOp op = someObject->beginSomeAsyncOp();
 *
 * // if you do not know the operation is complete, the following wait could be
 * // indefinite, unless you specify it not to be:
 * op.setWaitTimeout(5000); // 5 seconds
 * someObject->waitForSomeAsyncOp(&op);
 *
 * // after the wait you can check what happened with the wait by calling
 * HAsyncOp::WaitCode wcode = op.waitCode();
 *
 * // and if the operation uses an integer as a return value, you can query it
 * // by calling:
 * qint32 retVal = op.returnValue();
 *
 * \endcode
 *
 * The above example highlights two different return codes that have two different
 * purposes, the <em>wait code</em> and the <em>return value of the
 * asynchronous operation</em>. The wait code specifies the result of the
 * \e wait operation; it tells whether the wait operation succeeded. A wait can
 * be successful even if the asynchronous operation failed and vice versa;
 * a wait can fail even if the asynchronous operation eventually succeeds.
 *
 * All the HUPnP's waitFor() methods used to retrieve the results of
 * asynchronous operations use \c bool as a return value to indicate if both
 * the wait and the asynchronous operation succeeded. For instance,
 *
 * \code
 *
 * HAsyncOp op = someObject->beginSomeAsyncOp();
 * if (!someObject->waitForSomeAsyncOp(&op))
 * {
 *     // Either the wait or the asynchronous operation failed. You can check
 *     // the wait code to see if it was the wait that failed:
 *     if (op.waitCode() != HAsyncOp::WaitSuccess)
 *     {
 *         // It was the wait that failed. This means that the operation is
 *         // still running and it may still succeed normally.
 *     }
 *     else if (op.returnValue() != SomeErrorCodeThatIndicatesSuccess)
 *     {
 *        // It was the asynchronous operation that failed.
 *     }
 * }
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
 * Note, the user data is retrievable from \b any copy of the object that was
 * used to set the data. If an instance is created by the runner of an
 * asynchronous operation, setting the userData of that instance will associate
 * the userData with all the copies the runner uses too. From this follows that
 * when the runner informs the user an operation is finished, the provided
 * HAsyncOp object contains the previously set userData.
 *
 * Note also that the user data is never referenced by the runner of an
 * asynchronous operation. This also means that the ownership of the data is
 * never transferred.
 *
 * \headerfile hasyncop.h HAsyncOp
 *
 * \ingroup devicemodel
 *
 * \remarks this class is thread-safe.
 */
class H_UPNP_CORE_EXPORT HAsyncOp
{
friend H_UPNP_CORE_EXPORT bool operator==(const HAsyncOp&, const HAsyncOp&);

public:

    /*!
     * This enumeration specifies the values the waiting for the completion of an
     * asynchronous operation can return.
     */
    enum AsyncWaitCode
    {
        /*!
         * The asynchronous operation was successfully completed.
         */
        WaitSuccess = 0,

        /*!
         * A timeout elapsed before the asynchronous operation was completed.
         */
        WaitTimeout,

        /*!
         * The specified asynchronous operation ID is invalid.
         */
        WaitInvalidId,

        /*!
         * The result of an asynchronous operation can be waited by a single listener
         * and the operation in question already has a listener.
         */
        WaitListenerRegisteredAlready,

        /*!
         * The wait for the completion of an asynchronous operation was aborted.
         */
        WaitAborted,

        /*!
         * The object in question cannot execute the specified asynchronous
         * operation at the moment.
         */
        WaitInvalidObjectState,

        /*!
         * The asynchronous operation cannot be waited upon. For instance,
         * this is the case when the operation is launched with
         * <em>fire and forget</em> semantics (HExecArgs::FireAndForget).
         */
        WaitInvalidOperation
    };

private:

   const QUuid m_id;
   volatile qint32 m_waitTimeout;
   volatile AsyncWaitCode m_waitCode;
   volatile qint32 m_returnValue;
   QSharedPointer<volatile void*> m_userData;

   HAsyncOp& operator=(const HAsyncOp&);

public:

   /*!
    * \brief Creates a new instance.
    *
    * Creates a new instance.
    *
    * \param waitCode specifies the initial wait code for the operation.
    * The default is HAsyncOp::WaitSuccess.
    *
    * \sa isNull()
    */
   explicit HAsyncOp(AsyncWaitCode waitCode = WaitSuccess);

   /*!
    * \brief Destroys the instance.
    *
    * Destroys the instance.
    */
   ~HAsyncOp();

   /*!
    * \brief Copy constructor.
    *
    * Copy constructor.
    */
   HAsyncOp(const HAsyncOp&);

   /*!
    * Returns the wait timeout in milliseconds -if any- associated with the operation.
    *
    * \return the wait timeout in milliseconds -if any- associated with the operation.
    *
    * \sa setWaitTimeout()
    */
   inline qint32 waitTimeout() const { return m_waitTimeout; }

   /*!
    * Sets the wait timeout in milliseconds for the operation.
    *
    * \param timeout specifies the wait timeout in milliseconds.
    * A negative value means that the timeout isn't set.
    *
    * \sa waitTimeout()
    */
   inline void setWaitTimeout(qint32 timeout) { m_waitTimeout = timeout; }

   /*!
    * Returns the return value of the wait of operation completion.
    *
    * \return the return value of the wait of operation completion.
    *
    * \sa setWaitCode()
    */
   inline AsyncWaitCode waitCode() const { return m_waitCode; }

   /*!
    * Sets the return value of the wait of operation completion.
    *
    * \param waitCode specifies the return value of the wait of
    * operation completion.
    *
    * \sa waitCode()
    */
   inline void setWaitCode(AsyncWaitCode waitCode) { m_waitCode = waitCode; }

   /*!
    * Returns the return value of the asynchronous operation.
    *
    * \sa setReturnValue()
    */
   inline qint32 returnValue() const { return m_returnValue; }

   /*!
    * Sets the return value of the asynchronous operation.
    *
    * \param returnValue specifies the return value of the asynchronous operation.
    *
    * \sa returnValue()
    */
   inline void setReturnValue(qint32 returnValue)
   {
       m_returnValue = returnValue;
   }

   /*!
    * Associates arbitrary user provided data with the asynchronous operation.
    *
    * \param userData is the pointer to arbitrary user data.
    *
    * \remarks the instance never references the provided data.
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
    * Returns universally unique identifier of the asynchronous operation.
    *
    * \return universally unique identifier of the asynchronous operation.
    */
   inline QUuid id() const { return m_id; }

   /*!
    * Indicates whether the object identifies an asynchronous operation.
    *
    * \return \e true in case the object identifies an asynchronous operation.
    */
   inline bool isNull() const { return m_id.isNull(); }
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
H_UPNP_CORE_EXPORT quint32 qHash(const HAsyncOp& key);

}
}

#endif /* HASYNCOP_H_ */
