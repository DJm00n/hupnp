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

#include "haction.h"
#include "hservice.h"
#include "haction_p.h"
#include "../general/hupnp_global_p.h"

#include "../../utils/hlogger_p.h"
#include "../../utils/hexceptions_p.h"

#include <QThreadPool>
#include <QMutexLocker>

#include <QMetaType>
#include <QAtomicInt>

static bool registerMetaTypes()
{
    static QAtomicInt tester(0);

    if (tester.testAndSetAcquire(0, 1))
    {
        qRegisterMetaType<Herqq::Upnp::HAsyncOp>("Herqq::Upnp::HAsyncOp");
    }

    return true;
}

static bool test = registerMetaTypes();

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HSharedActionInvoker::Invocation
********************************************************************************/
HSharedActionInvoker::Invocation::Invocation(
    Herqq::Upnp::HActionPrivate* action, const HActionArguments& iArgs) :
        m_action(action), m_iargs(iArgs), m_invokeId(), m_waitCond(),
        m_outArgs(*m_action->m_outputArguments), m_hasListener(0),
        m_completed(false)
{
    Q_UNUSED(test)
}

void HSharedActionInvoker::Invocation::run()
{
    m_invokeId.setReturnValue(m_action->m_actionInvoke(m_iargs, &m_outArgs));
    m_action->onActionInvocationComplete(m_invokeId);
}

/*******************************************************************************
 * HSharedActionInvoker
********************************************************************************/
HSharedActionInvoker::HSharedActionInvoker(QThreadPool* tp) :
    m_threadPool(tp)
{
}

HSharedActionInvoker::~HSharedActionInvoker()
{
}

HSharedActionInvoker::Invocation* HSharedActionInvoker::runAction(
    HActionPrivate* action, const HActionArguments& iArgs)
{
    Invocation* invocation = new Invocation(action, iArgs);
    invocation->setAutoDelete(false);

    m_threadPool->start(invocation);

    return invocation;
}

/*******************************************************************************
 * HActionController
********************************************************************************/
HActionController::HActionController(HAction* action) :
    m_action(action)
{
    Q_ASSERT(m_action);
}

HActionController::~HActionController()
{
    delete m_action;
}

qint32 HActionController::invoke(
    const HActionArguments& iargs, HActionArguments* oargs)
{
    return m_action->h_ptr->m_actionInvoke(iargs, oargs);
}

/*******************************************************************************
 * HActionPrivate
 ******************************************************************************/
HActionPrivate::HActionPrivate() :
    q_ptr(0), m_name(), m_inputArguments(0), m_outputArguments(0),
    m_hasRetValArg(false), m_parentService(0), m_actionInvoke(),
    m_sharedActionInvoker(0), m_invocations(), m_invocationsMutex()
{
}

HActionPrivate::~HActionPrivate()
{
}

void HActionPrivate::onActionInvocationComplete(const HAsyncOp& id)
{
    QMutexLocker lock(&m_invocationsMutex);

    QPair<InvocationPtrT, HActionInvokeCallback> invocation =
        m_invocations.value(id.id());

    Q_ASSERT(invocation.first);

    invocation.first->m_completed = true;
    invocation.first->m_waitCond.wakeAll();

    lock.unlock();

    bool sendEvent = true;
    if (invocation.second)
    {
        sendEvent = invocation.second(id);
    }

    if (sendEvent)
    {
        emit q_ptr->invokeComplete(invocation.first->m_invokeId);
    }
}

bool HActionPrivate::waitForInvocation(
    HAsyncOp* waitResult, HActionArguments* outArgs)
{
    Q_ASSERT(waitResult);

    QMutexLocker lock(&m_invocationsMutex);

    QPair<InvocationPtrT, HActionInvokeCallback> invocation =
        m_invocations.value(waitResult->id());

    if (!invocation.first)
    {
        // no invocation matches the specified ID
        waitResult->setWaitCode(HAsyncOp::WaitInvalidId);
        return false;
    }

    if (!invocation.first->m_hasListener.testAndSetAcquire(0, 1))
    {
        waitResult->setWaitCode(HAsyncOp::WaitListenerRegisteredAlready);
        return false;
    }

    if (!invocation.first->isCompleted())
    {
        bool b =
            invocation.first->m_waitCond.wait(
                &m_invocationsMutex,
                waitResult->waitTimeout() < 0 ?
                    ULONG_MAX : waitResult->waitTimeout());

        if (!b)
        {
            waitResult->setWaitCode(HAsyncOp::WaitTimeout);
            return false;
        }
    }

    if (outArgs && invocation.first->m_invokeId.returnValue() == HAction::Success())
    {
        *outArgs = invocation.first->m_outArgs;
    }

    waitResult->setReturnValue(invocation.first->m_invokeId.returnValue());

    qint32 count = m_invocations.remove(invocation.first->m_invokeId.id());
    Q_ASSERT(count == 1); Q_UNUSED(count)

    waitResult->setWaitCode(HAsyncOp::WaitSuccess);
    return waitResult->returnValue() == HAction::Success();
}

HAsyncOp HActionPrivate::invoke(const HActionArguments& inArgs)
{
    QMutexLocker lock(&m_invocationsMutex);

    HSharedActionInvoker::Invocation* invocation =
        m_sharedActionInvoker->runAction(this, inArgs);

    Q_ASSERT(invocation);

    m_invocations.insert(
        invocation->m_invokeId.id(),
        qMakePair(InvocationPtrT(invocation), HActionInvokeCallback()));

    return invocation->m_invokeId;
}

HAsyncOp HActionPrivate::invoke(
    const HActionArguments& inArgs, const HActionInvokeCallback& cb)
{
    QMutexLocker lock(&m_invocationsMutex);

    HSharedActionInvoker::Invocation* invocation =
        m_sharedActionInvoker->runAction(this, inArgs);

    m_invocations.insert(
        invocation->m_invokeId.id(), qMakePair(InvocationPtrT(invocation), cb));

    return invocation->m_invokeId;
}

bool HActionPrivate::setActionInvoke(const HActionInvoke& actionInvoke)
{
    if (!actionInvoke)
    {
        return false;
    }

    m_actionInvoke = actionInvoke;

    return true;
}

bool HActionPrivate::setSharedInvoker(HSharedActionInvoker* sharedInvoker)
{
    if (!sharedInvoker)
    {
        return false;
    }

    m_sharedActionInvoker = sharedInvoker;

    return true;
}

bool HActionPrivate::setName(const QString& name)
{
    m_name = verifyName(name);
    return true;
}

bool HActionPrivate::setInputArgs(const HActionArguments& inputArguments)
{
    m_inputArguments.reset(new HActionArguments(inputArguments));
    return true;
}

bool HActionPrivate::setOutputArgs(
    const HActionArguments& outputArguments, bool hasRetValArg)
{
    m_outputArguments.reset(new HActionArguments(outputArguments));

    if (!m_outputArguments->size() && hasRetValArg)
    {
        return false;
    }

    m_hasRetValArg = hasRetValArg;
    return true;
}

/*******************************************************************************
 * HAction
 ******************************************************************************/
HAction::HAction(const QString& name, HService* parent) :
    QObject(parent),
        h_ptr(new HActionPrivate())
{
    Q_ASSERT_X(parent, H_AT, "Parent service must be defined.");
    h_ptr->m_parentService = parent;

    if (!h_ptr->setName(name))
    {
        throw HIllegalArgumentException("name");
    }

    h_ptr->q_ptr = this;
}

HAction::~HAction()
{
    delete h_ptr;
}

HService* HAction::parentService() const
{
    return h_ptr->m_parentService;
}

QString HAction::name() const
{
    return h_ptr->m_name;
}

HActionArguments HAction::inputArguments() const
{
    return *h_ptr->m_inputArguments;
}

HActionArguments HAction::outputArguments() const
{
    return *h_ptr->m_outputArguments;
}

QString HAction::returnArgumentName() const
{
    return h_ptr->m_hasRetValArg ? h_ptr->m_outputArguments->get(0)->name() : "";
}

HAsyncOp HAction::beginInvoke(
    const HActionArguments& inArgs)
{
    return h_ptr->invoke(inArgs);
}

HAsyncOp HAction::beginInvoke(
    const HActionArguments& inArgs,
    const HActionInvokeCallback& completionCallback)
{
    return h_ptr->invoke(inArgs, completionCallback);
}

bool HAction::waitForInvoke(
    HAsyncOp* waitResult, HActionArguments* outArgs)
{
    Q_ASSERT_X(waitResult, H_AT, "A valid pointer to waitResult variable has to be provided");
    return h_ptr->waitForInvocation(waitResult, outArgs);
}

qint32 HAction::invoke(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HAsyncOp id = beginInvoke(inArgs);
    bool b = waitForInvoke(&id, outArgs);
    H_ASSERT(b);

    return id.returnValue();
}

QString HAction::errorCodeToString(qint32 errCode)
{
    if (!errCode)
    {
        return "Success";
    }
    else if (errCode == InvalidArgs())
    {
        return "InvalidArgs";
    }
    else if (errCode == ArgumentValueInvalid())
    {
        return "ArgumentValueInvalid";
    }
    else if (errCode == ArgumentValueOutOfRange())
    {
        return "ArgumentValueOutOfRange";
    }
    else if (errCode == OptionalActionNotImplemented())
    {
        return "OptionalActionNotImplemented";
    }
    else if (errCode == OutOfMemory())
    {
        return "OutOfMemory";
    }
    else if (errCode == HumanInterventionRequired())
    {
        return "HumanInterventionRequired";
    }
    else if (errCode == StringArgumentTooLong())
    {
        return "StringArgumentTooLong";
    }
    else if (errCode == ActionFailed())
    {
        return "ActionFailed";
    }
    else if (errCode == UndefinedFailure())
    {
        return "UndefinedFailure";
    }

    return "";
}

}
}
