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

#include "upnp_action.h"
#include "upnp_service.h"
#include "upnp_action_p.h"
#include "upnp_global_p.h"

#include "../../../utils/src/logger_p.h"
#include "../../../core/include/HExceptions"

#include <QThreadPool>
#include <QMutexLocker>

#include <QMetaType>
#include <QAtomicInt>

static bool registerMetaTypes()
{
    static QAtomicInt tester(0);

    if (tester.testAndSetAcquire(0, 1))
    {
        //qRegisterMetaType<Herqq::Upnp::HActionInputArgument>("Herqq::Upnp::HActionInputArgument");
        //qRegisterMetaType<Herqq::Upnp::HActionInputArguments>("Herqq::Upnp::HActionInputArguments");
        //qRegisterMetaType<Herqq::Upnp::HActionOutputArgument>("Herqq::Upnp::HActionOutputArgument");
        qRegisterMetaType<Herqq::Upnp::HActionOutputArguments>("Herqq::Upnp::HActionOutputArguments");
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
        m_outArgs(*m_action->m_outputArguments)
{
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
HSharedActionInvoker::HSharedActionInvoker() :
    m_threadPool(new QThreadPool()), m_actionCount(0)
{
}

HSharedActionInvoker::~HSharedActionInvoker()
{
    HLOG(H_AT, H_FUN);

    delete m_threadPool;
}

void HSharedActionInvoker::setActionCount(qint32 actionCount)
{
    HLOG(H_AT, H_FUN);

    m_actionCount = actionCount;
    m_threadPool->setMaxThreadCount(qMax(actionCount / 4, 1));
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

void HActionPrivate::onActionInvocationComplete(const QUuid& id, qint32 retCode)
{
    HLOG(H_AT, H_FUN);

    QMutexLocker lock(&m_invocationsMutex);
    QPair<InvocationPtrT, HActionInvokeCallback> invocation =
        m_invocations.value(id);

    Q_ASSERT(invocation.first);

    bool sendEvent = true;
    if (invocation.second)
    {
        sendEvent = invocation.second(id, retCode, invocation.first->m_outArgs);
    }

    if (sendEvent)
    {
        if (retCode == HAction::Success())
        {
            emit q_ptr->invokeComplete(
                invocation.first->m_invokeId, invocation.first->m_outArgs);
        }
        else
        {
            emit q_ptr->invokeFailed(invocation.first->m_invokeId, retCode);
        }
    }

    invocation.first->m_waitCond.wakeAll();

    m_invocations.remove(invocation.first->m_invokeId);
}

HAction::InvocationWaitReturnValue HActionPrivate::waitForInvocation(
    const QUuid& invokeId, qint32 timeout, HActionOutputArguments* oArgs)
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

    bool b = invocation.first->m_waitCond.wait(
        &m_invocationsMutex, timeout < 0 ? ULONG_MAX : timeout);

    if (!b)
    {
        return HAction::WaitTimeout;
    }

    if (oArgs)
    {
        *oArgs = invocation.first->m_outArgs;
    }

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

void HActionPrivate::init(
    const HActionInputArguments& inputArguments,
    const HActionOutputArguments& outputArguments,
    bool hasRetvalArgument, HActionInvoke actionInvoke,
    HSharedActionInvoker* sharedActionInvoker)
{
    HLOG(H_AT, H_FUN);

    bool b = setInputArgs(inputArguments);
    if (!b)
    {
        throw HIllegalArgumentException("inputArguments");
    }

    b = setOutputArgs(outputArguments, hasRetvalArgument);
    if (!b)
    {
        throw HIllegalArgumentException("outputArguments");
    }

    if (!setActionInvoke(actionInvoke))
    {
        throw HIllegalArgumentException("actionInvoke");
    }

    Q_ASSERT(sharedActionInvoker);
    m_sharedActionInvoker = sharedActionInvoker;
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

QUuid HAction::beginInvoke(const HActionInputArguments& inArgs)
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
    QUuid invokeId, HActionOutputArguments* outArgs, qint32 timeout)
{
    return h_ptr->waitForInvocation(invokeId, timeout, outArgs);
}

qint32 HAction::invoke(
    const HActionInputArguments& inArgs, HActionOutputArguments* outArgs)
{
    HLOG(H_AT, H_FUN);
    return h_ptr->m_actionInvoke(inArgs, outArgs);
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
