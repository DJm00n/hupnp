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

#ifndef HACTION_H_
#define HACTION_H_

#include "hasyncop.h"
#include "hactioninvoke_callback.h"

#include "../general/hdefs_p.h"
#include "../general/hupnp_fwd.h"

class QString;

#include <QObject>

namespace Herqq
{

namespace Upnp
{

class HObjectCreator;
class HActionPrivate;
class HActionController;

/*!
 * \brief A class that represents a UPnP action found in a UPnP service.
 *
 * \c %HAction is a core component of the HUPnP \ref devicemodel
 * and it models a UPnP action. The UPnP Device Architecture specifies a UPnP
 * action as command, which takes one or more input or output arguments and that
 * may have a return value. In a way, a UPnP action is an abstraction
 * to a method or to a remote procedure call.
 *
 * You can call inputArguments() and outputArguments() to find out what the
 * defined input and output arguments for the action are. In essence,
 * when you execute an action using invoke() or beginInvoke(), you have to provide the set
 * of input arguments the inputArguments() method returns.
 * Otherwise, the invocation fails. Obviously the values of the arguments can be changed.
 * On the other hand, HUPnP will fill the user provided output arguments to match
 * the return value of outputArguments(), if necessary.
 *
 * You can invoke an \c %HAction synchronously and asynchronously.
 * To perform an asynchronous invocation, you have to call beginInvoke().
 * To perform a synchronous invocation, you can call beginInvoke()
 * followed by waitForInvoke(), or you can call invoke(), which is a
 * helper method that calls beginInvoke() and waitForInvoke().
 * It is important to note that although \c %HAction
 * is derived from \c QObject, neither the invoke() nor beginInvoke() has thread affinity.
 * Because of this, you can perform invocation from any thread.
 *
 * \headerfile haction.h HAction
 *
 * \ingroup devicemodel
 *
 * \sa HService
 *
 * \warning you should \b never perform a synchronous invocation from a thread
 * in which the \c %HAction to be invoked resides, since this may cause a dead-lock.
 *
 * \remarks
 * \li the methods introduced in this class are thread-safe, although the
 * base class is largely not.
 */
class H_UPNP_CORE_EXPORT HAction :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HAction)
H_DECLARE_PRIVATE(HAction)
friend class HObjectCreator;
friend class HActionController;

private:

    HActionPrivate* h_ptr;

    //
    // \internal
    //
    // Initializes the instance with the specified name.
    // \param name specifies the name of the action.
    //
    // \throw Herqq::Utils::HIllegalArgumentException in case the the name
    // argument is invalid.
    //
    HAction(const QString& name, HService* parent);

public:

    /*!
     * \brief Action invocation succeeded.
     *
     * Action invocation succeeded.
     */
    inline static qint32 Success() { return 0; }

    /*!
     * \brief Action invocation failed due to the action lacking an implementation.
     *
     * Action invocation failed due to the action lacking an implementation.
     */
    inline static qint32 NotImplemented() { return 0xffffffff; }

    /*!
     * Action invocation failed due to:
     * \li not enough arguments,
     * \li arguments in wrong order,
     * \li one or more arguments have wrong data type
     */
    inline static qint32 InvalidArgs() { return 402; }

    /*!
     * \brief Action invocation failed due to an invalid argument value.
     *
     * Action invocation failed due to an invalid argument value.
     */
    inline static qint32 ArgumentValueInvalid() { return 600; }

    /*!
     * Action invocation failed due to:
     * \li an argument value is less than the minimum of the allowed value range,
     * \li an argument value is more than the maximum of the allowed value range,
     * \li an argument value is not in the allowed value list
     */
    inline static qint32 ArgumentValueOutOfRange() { return 601; }

    /*!
     * Action invocation failed due to the requested action being optional
     * and not implemented by the device.
     */
    inline static qint32 OptionalActionNotImplemented() { return 602; }

    /*!
     * Action invocation failed due to insufficient memory.
     *
     * The device does not have sufficient memory available to complete the action.
     * This MAY be a temporary condition; the control point MAY choose to retry the
     * unmodified request again later and it MAY succeed if memory is available.
     */
    inline static qint32 OutOfMemory() { return 603; }

    /*!
     * The device has encountered an error condition which it cannot resolve itself
     * and required human intervention such as a reset or power cycle. See the device
     * display or documentation for further guidance.
     */
    inline static qint32 HumanInterventionRequired() { return 604; }

    /*!
     * Action invocation failed due to a string argument being
     * too long for the device to handle properly.
     */
    inline static qint32 StringArgumentTooLong() { return 605; }

    /*!
     * \brief The current state of the service prevents the action invocation.
     *
     * The current state of the service prevents the action invocation.
     */
    inline static qint32 ActionFailed() { return 501; }

    /*!
     * \brief Action invocation failed, but the exact cause could not be determined.
     *
     * Action invocation failed, but the exact cause could not be determined.
     */
    inline static qint32 UndefinedFailure() { return 0xf0000000; }

public:

    /*!
     * Destroys the instance.
     *
     * An HAction is always destroyed by the containing HService when it
     * is being deleted. You should never destroy an \c %HAction.
     */
    virtual ~HAction();

    /*!
     * Returns the parent service of the action.
     *
     * \return the parent service of the action.
     *
     * \warning the pointer is guaranteed to point to a valid object as long
     * as the \c %HAction exists, which ultimately is as long as the
     * containing Herqq::Upnp::HDevice exists.
     *
     * \sa HDevice
     */
    HService* parentService() const;

    /*!
     * Returns the name of the action.
     *
     * This is the name specified in the corresponding service description file.
     *
     * \return the name of the action.
     */
    QString name() const;

    /*!
     * Returns a copy of the input arguments the action expects.
     *
     * Most often action invocation is preceded by calling this and
     * setting the values for the desired input arguments. For example,
     *
     * \code
     *
     * Herqq::Upnp::HActionArguments inArgs = action->inputArguments();
     * inArgs["DesiredArgumentName"]->setValue("ValueThatHappensToBeStringInThisExample");
     *
     * Herqq::Upnp::HActionArguments outArgs;
     * qint32 retVal = action->invoke(inArgs, &outArgs);
     *
     * \endcode
     *
     * \return a copy of the input arguments the action expects. The returned
     * input arguments are set to their default state. The values are always
     * the same.
     *
     * \sa outputArguments(), invoke(), beginInvoke()
     */
    HActionArguments inputArguments() const;

    /*!
     * Returns a copy of the output arguments of the action.
     *
     * \return a copy of the output arguments of the action. The returned output
     * arguments are set to their default state. The values are always the same.
     *
     * \remark \e action \e invocation fills a copy of the output arguments reflecting
     * the result of the action invocation. The output arguments of an \e action
     * are never changed.
     *
     * \sa inputArguments(), invoke(), beginInvoke(), waitForInvoke()
     */
    HActionArguments outputArguments() const;

    /*!
     * Returns the name of the output argument that is marked as the
     * action's return value.
     *
     * \return the name of the output argument that is marked as the action's
     * return value, or an empty string, if no output argument has been marked as
     * the action's return value.
     */
    QString returnArgumentName() const;

    /*!
     * Schedules the action to be invoked.
     *
     * The method performs an asynchronous action invocation. The invocation
     * is placed in queue and will be executed on a random thread pool thread
     * as soon as possible. Therefore, the following issues are important to be
     * noted:
     *
     * \li the method is thread-safe
     * \li the order of invocations is the order in which beginInvoke() methods
     * are invoked
     * \li the executing thread is arbitrary
     * \li the method returns immediately
     *
     * When the action invocation is done, the signal invokeComplete()
     * is emitted. After that, you have to call waitForInvoke() with the proper
     * <em>action invocation id</em> to retrieve the result of the invocation.
     *
     * \param inArgs specifies the input arguments for the action invocation.
     *
     * \return the ID used to identify the asynchronous operation. Once the
     * invokeComplete() signal is emitted, you have to call waitForInvoke()
     * providing this ID to retrieve the result of the action invocation.
     *
     * \remark
     * \li The invokeComplete() signal is always emitted,
     * even if you have called waitForInvoke() before that.
     * \li waitForInvoke() with a proper ID will complete before
     * the invokeComplete() signal is sent.
     *
     * \sa waitForInvoke(), invoke(), inputArguments()
     */
    HAsyncOp beginInvoke(const HActionArguments& inArgs);

    /*!
     * Schedules the action to be invoked.
     *
     * The method performs an asynchronous action invocation. The invocation
     * is placed in queue and will be executed on a random thread pool thread
     * as soon as possible. Therefore, the following issues are important to be
     * noted:
     *
     * \li the method is thread-safe
     * \li the order of invocations is the order in which beginInvoke() methods
     * are invoked
     * \li the executing thread is arbitrary
     * \li the method returns immediately
     *
     * When the action has completed or the invocation has failed,
     * the specified callback is called. No events are sent unless that is
     * explicitly wanted by \b returning \b true from the callback function.
     * You have to call waitForInvoke() with the proper <em>action invocation id</em>
     * to retrieve the result of the invocation.
     *
     * The different semantics compared to the other beginInvoke()
     * method are important to notice:
     *
     * \li If a <em> completion callback </em>is valid, no event is sent unless
     * the invoker explicitly requests so.
     * \li The callback is always invoked immediately after the invocation has
     * succeeded or failed <b>in the thread that executed the action</b>. Note,
     * that the time requirements to return from the callback are usually far less severe
     * than they are with \em slots executed in the eventloop thread. However,
     * the callback should still return as soon as possible.
     *
     * \param inArgs specifies the input arguments for the action invocation
     * \param completionCallback specifies the callable entity that is called
     * once the action invocation is completed or failed. If the specified callable
     * entity is not valid and it cannot be called, the callable entity is
     * ignored and events are sent instead.
     *
     * \return the ID used to identify the asynchronous operation. Once the callback
     * is called and possibly the invokeComplete() signal is emitted,
     * you have to call waitForInvoke() providing this ID to retrieve
     * the result of the action invocation.
     *
     * \remarks
     * \li the completion callback is always called, even if you have called
     * waitForInvoke()
     * \li waitForInvoke() with a proper ID will complete before
     * the callback is called and possibly the invokeComplete() signal is sent.
     *
     * \sa waitForInvoke(), invoke(), inputArguments()
     */
    HAsyncOp beginInvoke(
        const HActionArguments& inArgs,
        const HActionInvokeCallback& completionCallback);

    /*!
     * Waits for the completion of an asynchronous action invocation started
     * by beginInvoke().
     *
     * You have to call this method to retrieve the result of an action invocation.
     * In addition, you can use this method to block the current thread until the specified
     * action invocation is complete.
     *
     * If the action invocation corresponding to the specified invocationId
     * has completed and this is the first call to this method with the specified
     * invocationId, the call will return immediately. If the invocation has not
     * been completed at the time of this call, the calling thread will be blocked
     * until the action invocation is complete.
     *
     * \attention Calling this method to block the thread in which the
     * action object lives without \b may result in a dead-lock and the invocation
     * will \b always fail.
     *
     * Note also, that you can call this method only once with a particular
     * invocationId. The results of the action invocation are stored until a
     * call is made.
     *
     * \param op specifies the action invocation previously started by
     * beginInvoke(). If the parameter is invalid, the method returns immediately with
     * an error code.
     *
     * \param outArgs specifies a pointer to a HActionArguments
     * object that the user has created, or null, in case the output arguments
     * aren't wanted. In case the wait was successfully completed
     * and a valid pointer to object was provided by the user,
     * the object will contain the output arguments of the action invocation.
     * If the action has no output arguments, the parameter is ignored.
     *
     * \return value indicates whether the invocation successfully completed
     * before the timeout.
     *
     * \sa beginInvoke(), outputArguments()
     */
    bool waitForInvoke(HAsyncOp* op, HActionArguments* outArgs = 0);

    /*!
     * Invokes the action synchronously.
     *
     * This is a helper method for calling beginInvoke() and waitForInvoke().
     *
     * For example,
     *
     * \code
     *
     * Herqq::Upnp::HActionArguments inArgs = action->inputArguments();
     * inArgs["EchoInArgument"]->setValue("Ping");
     *
     * Herqq::Upnp::HActionArguments outArgs;
     *
     * qint32 retVal = action->invoke(inArgs, &outArgs);
     * if (retVal == Herqq::Upnp::HAction::Success())
     * {
     *     qDebug() << outArgs["EchoOutArgument"]->value().toString();
     * }
     *
     * \endcode
     *
     * \param inArgs specifies the input arguments for the action.
     * \param outArgs specifies a pointer to an object created by the user.
     * This can be null in which case the output arguments will not be set
     * even if the action has output arguments. If the parameter is specified
     * and the action has output arguments, the values of the arguments will be set accordingly.
     * If the action doesn't have output arguments, the parameter is ignored.
     *
     * \retval 0 indicates success. Any other value indicates that an error
     * occurred.
     *
     * \remarks You should never call this method from the thread in which the
     * \c %HAction lives, since this may result in a dead-lock.
     *
     * \sa beginInvoke(), waitForInvoke(), inputArguments(), outputArguments()
     */
    qint32 invoke(
        const HActionArguments& inArgs, HActionArguments* outArgs = 0);

    /*!
     * Returns a string representation of the specified error code.
     *
     * \param errCode specififes the error code.
     *
     * \return a string representation of the specified error code.
     */
    static QString errorCodeToString(qint32 errCode);

Q_SIGNALS:

    /*!
     * This signal is emitted when an asynchronous action invocation
     * has been successfully completed or the invocation failed.
     *
     * After the signal, you have to call waitForInvoke() to retrieve
     * the result of the action invocation.
     *
     * \param invocationId specifies the ID of the invocation that completed.
     */
    void invokeComplete(Herqq::Upnp::HAsyncOp invocationId);
};

}
}

#endif /* HACTION_H_ */
