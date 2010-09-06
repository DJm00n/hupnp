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

#include "haction.h"
#include "haction_p.h"

#include "../../utils/hlogger_p.h"

#include <QtCore/QMutexLocker>

#include <QtCore/QMetaType>
#include <QtCore/QAtomicInt>

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
 * HInvocation
********************************************************************************/
HInvocation::HInvocation(
    HActionPrivate* action, const HActionArguments& iArgs) :
        m_action(action), m_inArgs(iArgs), m_invokeId(), m_waitCond(),
        m_outArgs(m_action->m_info->outputArguments()), m_hasListener(0),
        m_completed(false)
{
    Q_UNUSED(test)
}

HInvocation::~HInvocation()
{
}

/*******************************************************************************
 * HActionInvoker
********************************************************************************/
HActionInvoker::HActionInvoker(HActionPrivate* action) :
    m_action(action)
{
    Q_ASSERT(m_action);
}

HActionInvoker::~HActionInvoker()
{
}

/*******************************************************************************
 * HAsyncInvocation
********************************************************************************/
HAsyncInvocation::HAsyncInvocation(
    HActionPrivate* action, const HActionArguments& iArgs) :
        HInvocation(action, iArgs)
{
}

HAsyncInvocation::~HAsyncInvocation()
{
}

/*******************************************************************************
 * HAsyncActionInvoker
********************************************************************************/
HAsyncActionInvoker::HAsyncActionInvoker(HActionPrivate* action) :
    HActionInvoker(action)
{
    Q_ASSERT(action->m_actionInvokeProxy);
    action->m_actionInvokeProxy->setCallback(
        HActionInvokeCallback(this, &HAsyncActionInvoker::invokeComplete));
}

HAsyncActionInvoker::~HAsyncActionInvoker()
{
    delete m_action->m_actionInvokeProxy;
}

bool HAsyncActionInvoker::invokeComplete(HAsyncOp op)
{
    Q_ASSERT(!op.isNull());
    m_action->onActionInvocationComplete(op);
    return true;
}

HAsyncInvocation* HAsyncActionInvoker::runAction(const HActionArguments& iArgs)
{
    HAsyncInvocation* inv = new HAsyncInvocation(m_action, iArgs);
    m_action->m_actionInvokeProxy->beginInvoke(inv);
    return inv;
}

/*******************************************************************************
 * HSyncInvocation
********************************************************************************/
HSyncInvocation::HSyncInvocation(
    HActionPrivate* action, const HActionArguments& iArgs) :
        HRunnable(),
        HInvocation(action, iArgs)
{
}

HSyncInvocation::~HSyncInvocation()
{
}

void HSyncInvocation::run()
{
    Q_ASSERT(m_action->m_actionInvoke);
    m_invokeId.setReturnValue((*m_action->m_actionInvoke)(m_inArgs, &m_outArgs));
    m_action->onActionInvocationComplete(m_invokeId);
}

/*******************************************************************************
 * HSyncActionInvoker
********************************************************************************/
HSyncActionInvoker::HSyncActionInvoker(HActionPrivate* action, HThreadPool* tp) :
    HActionInvoker(action),
        m_threadPool(tp)
{
    Q_ASSERT(m_threadPool);
    Q_ASSERT(m_action->m_actionInvoke);
}

HSyncActionInvoker::~HSyncActionInvoker()
{
    delete m_action->m_actionInvoke;
}

HSyncInvocation* HSyncActionInvoker::runAction(const HActionArguments& iArgs)
{
    HSyncInvocation* invocation = new HSyncInvocation(m_action, iArgs);
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
    return (*m_action->h_ptr->m_actionInvoke)(iargs, oargs);
}

/*******************************************************************************
 * HActionInvokeProxy
 ******************************************************************************/
HActionInvokeProxy::HActionInvokeProxy() :
    m_callback()
{
}

HActionInvokeProxy::~HActionInvokeProxy()
{
}

/*******************************************************************************
 * HActionPrivate
 ******************************************************************************/
HActionPrivate::HActionPrivate() :
    q_ptr(0), m_info(), m_parentService(0), m_actionInvoke(),
    m_actionInvoker(0), m_invocations(), m_invocationsMutex()
{
}

HActionPrivate::~HActionPrivate()
{
    delete m_actionInvoker;
}

void HActionPrivate::onActionInvocationComplete(const HAsyncOp& id)
{
    QMutexLocker lock(&m_invocationsMutex);

    InvocationInfo invocationInfo = m_invocations.value(id.id());

    Q_ASSERT(invocationInfo.invocation);

    invocationInfo.invocation->m_completed = true;
    invocationInfo.invocation->m_waitCond.wakeAll();

    lock.unlock();

    if (invocationInfo.execArgs &&
        invocationInfo.execArgs->execType() == HExecArgs::FireAndForget)
    {
        qint32 count = m_invocations.remove(
            invocationInfo.invocation->m_invokeId.id());
        Q_ASSERT(count == 1); Q_UNUSED(count)
        return;
    }

    bool sendEvent = true;
    if (invocationInfo.callback)
    {
        sendEvent = invocationInfo.callback(id);
    }

    if (sendEvent)
    {
        emit q_ptr->invokeComplete(invocationInfo.invocation->m_invokeId);
    }
}

bool HActionPrivate::waitForInvocation(
    HAsyncOp* waitResult, HActionArguments* outArgs)
{
    Q_ASSERT(waitResult);

    QMutexLocker lock(&m_invocationsMutex);

    InvocationInfo invocationInfo = m_invocations.value(waitResult->id());
    if (!invocationInfo.invocation)
    {
        // no invocation matches the specified ID
        waitResult->setWaitCode(HAsyncOp::WaitInvalidId);
        return false;
    }
    else if (invocationInfo.execArgs &&
             invocationInfo.execArgs->execType() == HExecArgs::FireAndForget)
    {
        waitResult->setWaitCode(HAsyncOp::WaitInvalidOperation);
        return false;
    }

    if (!invocationInfo.invocation->m_hasListener.testAndSetAcquire(0, 1))
    {
        waitResult->setWaitCode(HAsyncOp::WaitListenerRegisteredAlready);
        return false;
    }

    if (!invocationInfo.invocation->isCompleted())
    {
        bool b =
            invocationInfo.invocation->m_waitCond.wait(
                &m_invocationsMutex,
                waitResult->waitTimeout() < 0 ?
                    ULONG_MAX : waitResult->waitTimeout());

        if (!b)
        {
            waitResult->setWaitCode(HAsyncOp::WaitTimeout);
            return false;
        }
    }

    if (outArgs && invocationInfo.invocation->m_invokeId.returnValue() == HAction::Success)
    {
        *outArgs = invocationInfo.invocation->m_outArgs;
    }

    waitResult->setReturnValue(invocationInfo.invocation->m_invokeId.returnValue());

    qint32 count = m_invocations.remove(invocationInfo.invocation->m_invokeId.id());
    Q_ASSERT(count == 1); Q_UNUSED(count)

    waitResult->setWaitCode(HAsyncOp::WaitSuccess);
    return waitResult->returnValue() == HAction::Success;
}

HAsyncOp HActionPrivate::invoke(
    const HActionArguments& inArgs, HExecArgs* execArgs)
{
    return invoke(inArgs, HActionInvokeCallback(), execArgs);
}

HAsyncOp HActionPrivate::invoke(
    const HActionArguments& inArgs, const HActionInvokeCallback& cb,
    HExecArgs* execArgs)
{
    QMutexLocker lock(&m_invocationsMutex);

    HInvocation* invocation = m_actionInvoker->runAction(inArgs);

    m_invocations.insert(
        invocation->m_invokeId.id(),
        InvocationInfo(HInvocationPtrT(invocation), cb, execArgs));

    return invocation->m_invokeId;
}

bool HActionPrivate::setActionInvoke(const HActionInvoke& actionInvoke)
{
    if (!actionInvoke)
    {
        return false;
    }

    if (m_actionInvoke) { delete m_actionInvoke; }
    m_actionInvoke = new HActionInvoke(actionInvoke);

    return true;
}

bool HActionPrivate::setInvoker(HActionInvoker* arg)
{
    if (!arg)
    {
        return false;
    }

    m_actionInvoker = arg;

    return true;
}

bool HActionPrivate::setInfo(const HActionInfo& info)
{
    if (!info.isValid())
    {
        return false;
    }

    m_info.reset(new HActionInfo(info));
    return true;
}

/*******************************************************************************
 * HAction
 ******************************************************************************/
HAction::HAction(const HActionInfo& info, HService* parent) :
    QObject(reinterpret_cast<QObject*>(parent)),
        h_ptr(new HActionPrivate())
{
    Q_ASSERT_X(parent, H_AT, "Parent service must be defined.");
    Q_ASSERT_X(info.isValid(), H_AT, "Action information must be defined.");
    h_ptr->m_parentService = parent;

    h_ptr->m_info.reset(new HActionInfo(info));
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

const HActionInfo& HAction::info() const
{
    return *h_ptr->m_info;
}

HAsyncOp HAction::beginInvoke(
    const HActionArguments& inArgs, HExecArgs* execArgs)
{
    return h_ptr->invoke(inArgs, execArgs);
}

HAsyncOp HAction::beginInvoke(
    const HActionArguments& inArgs,
    const HActionInvokeCallback& completionCallback,
    HExecArgs* execArgs)
{
    return h_ptr->invoke(inArgs, completionCallback, execArgs);
}

bool HAction::waitForInvoke(HAsyncOp* waitResult, HActionArguments* outArgs)
{
    Q_ASSERT_X(waitResult, H_AT, "A valid pointer to waitResult variable has to be provided");
    return h_ptr->waitForInvocation(waitResult, outArgs);
}

qint32 HAction::invoke(
    const HActionArguments& inArgs, HActionArguments* outArgs)
{
    HAsyncOp id = beginInvoke(inArgs);
    waitForInvoke(&id, outArgs);

    return id.returnValue();
}

QString HAction::errorCodeToString(qint32 errCode)
{
    if (!errCode)
    {
        return "Success";
    }
    else if (errCode == InvalidArgs)
    {
        return "InvalidArgs";
    }
    else if (errCode == ArgumentValueInvalid)
    {
        return "ArgumentValueInvalid";
    }
    else if (errCode == ArgumentValueOutOfRange)
    {
        return "ArgumentValueOutOfRange";
    }
    else if (errCode == OptionalActionNotImplemented)
    {
        return "OptionalActionNotImplemented";
    }
    else if (errCode == OutOfMemory)
    {
        return "OutOfMemory";
    }
    else if (errCode == HumanInterventionRequired)
    {
        return "HumanInterventionRequired";
    }
    else if (errCode == StringArgumentTooLong)
    {
        return "StringArgumentTooLong";
    }
    else if (errCode == ActionFailed)
    {
        return "ActionFailed";
    }
    else if (errCode == UndefinedFailure)
    {
        return "UndefinedFailure";
    }

    return "";
}

}
}
