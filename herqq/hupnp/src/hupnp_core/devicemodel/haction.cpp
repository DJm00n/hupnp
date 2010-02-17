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
#include "./../general/hupnp_global_p.h"

#include "./../../utils/hlogger_p.h"
#include "./../../utils/hexceptions_p.h"

#include <QThreadPool>
#include <QMutexLocker>

#include <QMetaType>
#include <QAtomicInt>

static bool registerMetaTypes()
{
    static QAtomicInt tester(0);

    if (tester.testAndSetAcquire(0, 1))
    {
        qRegisterMetaType<QUuid>("QUuid");
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
    Herqq::Upnp::HActionPrivate* action, const HActionInputArguments& iArgs,
    const QUuid& invokeId) :
        m_action(action), m_iargs(iArgs), m_invokeId(invokeId), m_waitCond(),
        m_outArgs(*m_action->m_outputArguments), m_rc(0x0fffffff),
        m_hasListener(0)
{
    Q_UNUSED(test)
}

void HSharedActionInvoker::Invocation::run()
{
    HLOG(H_AT, H_FUN);

    qint32 rc = m_action->m_actionInvoke(m_iargs, &m_outArgs);
    m_action->onActionInvocationComplete(m_invokeId, rc);
}

/*******************************************************************************
 * HSharedActionInvoker
********************************************************************************/
HSharedActionInvoker::HSharedActionInvoker(QThreadPool* tp) :
    m_threadPool(tp), m_actionCount(0)
{
}

HSharedActionInvoker::~HSharedActionInvoker()
{
    HLOG(H_AT, H_FUN);
}

HSharedActionInvoker::Invocation* HSharedActionInvoker::runAction(
    HActionPrivate* action, const HActionInputArguments& iArgs)
{
    HLOG(H_AT, H_FUN);

    Invocation* invocation = new Invocation(action, iArgs, QUuid::createUuid());

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
    //registerMetaTypes();
}

HActionController::~HActionController()
{
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
    HLOG(H_AT, H_FUN);
}

void HActionPrivate::onActionInvocationComplete(const QUuid& id, qint32 rc)
{
    HLOG(H_AT, H_FUN);

    QMutexLocker lock(&m_invocationsMutex);
    QPair<InvocationPtrT, HActionInvokeCallback> invocation =
        m_invocations.value(id);

    Q_ASSERT(invocation.first);

    invocation.first->m_rc = rc;
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

HAction::InvocationWaitReturnValue HActionPrivate::waitForInvocation(
    const QUuid& invokeId, qint32* rc, qint32 timeout,
    HActionOutputArguments* oArgs)
{
    HLOG(H_AT, H_FUN);

    Q_ASSERT(oArgs);

    QMutexLocker lock(&m_invocationsMutex);

    QPair<InvocationPtrT, HActionInvokeCallback> invocation =
        m_invocations.value(invokeId);

    if (!invocation.first)
    {
        // no invocation matches the specified ID
        return HAction::WaitInvalidInvocationId;
    }

    if (!invocation.first->m_hasListener.testAndSetAcquire(0, 1))
    {
        return HAction::WaitListenerRegisteredAlready;
    }

    if (!invocation.first->isCompleted())
    {
        bool b = invocation.first->m_waitCond.wait(
            &m_invocationsMutex, timeout < 0 ? ULONG_MAX : timeout);

        if (!b)
        {
            return HAction::WaitTimeout;
        }
    }

    if (oArgs && invocation.first->m_rc == HAction::Success())
    {
        *oArgs = invocation.first->m_outArgs;
    }

    *rc = invocation.first->m_rc;

    m_invocations.remove(invocation.first->m_invokeId);

    return HAction::WaitSuccess;
}

QUuid HActionPrivate::invoke(const HActionInputArguments& inArgs)
{
    HLOG(H_AT, H_FUN);

    QMutexLocker lock(&m_invocationsMutex);

    HSharedActionInvoker::Invocation* invocation =
        m_sharedActionInvoker->runAction(this, inArgs);

    m_invocations.insert(
        invocation->m_invokeId,
        qMakePair(InvocationPtrT(invocation), HActionInvokeCallback()));

    return invocation->m_invokeId;
}

QUuid HActionPrivate::invoke(
    const HActionInputArguments& inArgs, const HActionInvokeCallback& cb)
{
    HLOG(H_AT, H_FUN);

    QMutexLocker lock(&m_invocationsMutex);

    HSharedActionInvoker::Invocation* invocation =
        m_sharedActionInvoker->runAction(this, inArgs);

    m_invocations.insert(
        invocation->m_invokeId, qMakePair(InvocationPtrT(invocation), cb));

    return invocation->m_invokeId;
}

bool HActionPrivate::setActionInvoke(const HActionInvoke& actionInvoke)
{
    HLOG(H_AT, H_FUN);

    if (!actionInvoke)
    {
        return false;
    }

    m_actionInvoke = actionInvoke;

    return true;
}

bool HActionPrivate::setSharedInvoker(HSharedActionInvoker* sharedInvoker)
{
    HLOG(H_AT, H_FUN);

    if (!sharedInvoker)
    {
        return false;
    }

    m_sharedActionInvoker = sharedInvoker;

    return true;
}

bool HActionPrivate::setName(const QString& name)
{
    HLOG(H_AT, H_FUN);

    m_name = verifyName(name);
    return true;
}

bool HActionPrivate::setInputArgs(const HActionInputArguments& inputArguments)
{
    HLOG(H_AT, H_FUN);

    m_inputArguments.reset(new HActionInputArguments(inputArguments));
    return true;
}

bool HActionPrivate::setOutputArgs(
    const HActionOutputArguments& outputArguments, bool hasRetValArg)
{
    HLOG(H_AT, H_FUN);

    m_outputArguments.reset(new HActionOutputArguments(outputArguments));

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
HAction::HAction(
    HActionPrivate& dd, const QString& name, HService* parent) :
        QObject(parent),
            h_ptr(&dd)
{
    HLOG(H_AT, H_FUN);

    Q_ASSERT_X(parent, H_AT, "Parent service must be defined.");
    h_ptr->m_parentService = parent;

    if (!h_ptr->setName(name))
    {
        throw HIllegalArgumentException("name");
    }

    h_ptr->q_ptr = this;
}

HAction::HAction(const QString& name, HService* parent) :
    QObject(parent),
        h_ptr(new HActionPrivate())
{
    HLOG(H_AT, H_FUN);

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
    HLOG(H_AT, H_FUN);
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

HActionInputArguments HAction::inputArguments() const
{
    return *h_ptr->m_inputArguments;
}

HActionOutputArguments HAction::outputArguments() const
{
    return *h_ptr->m_outputArguments;
}

QString HAction::returnArgumentName() const
{
    return h_ptr->m_hasRetValArg ? h_ptr->m_outputArguments->get(0)->name() : "";
}

QUuid HAction::beginInvoke(
    const HActionInputArguments& inArgs)
{
    HLOG(H_AT, H_FUN);
    return h_ptr->invoke(inArgs);
}

QUuid HAction::beginInvoke(
    const HActionInputArguments& inArgs,
    const HActionInvokeCallback& completionCallback)
{
    HLOG(H_AT, H_FUN);
    return h_ptr->invoke(inArgs, completionCallback);
}

HAction::InvocationWaitReturnValue HAction::waitForInvoke(
    QUuid invokeId, qint32* rc, HActionOutputArguments* outArgs, qint32 timeout)
{
    Q_ASSERT_X(rc, H_AT, "A valid pointer to return code variable has to be provided");
    return h_ptr->waitForInvocation(invokeId, rc, timeout, outArgs);
}

qint32 HAction::invoke(
    const HActionInputArguments& inArgs, HActionOutputArguments* outArgs)
{
    HLOG(H_AT, H_FUN);

    qint32 rc = 0;

    QUuid id = beginInvoke(inArgs);
    waitForInvoke(id, &rc, outArgs);

    return rc;
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
