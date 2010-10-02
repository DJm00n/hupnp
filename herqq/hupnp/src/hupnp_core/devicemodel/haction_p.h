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

#ifndef HACTION_P_H_
#define HACTION_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "haction.h"
#include "hexecargs.h"
#include "hactioninvoke.h"
#include "hactionarguments.h"

#include "../../utils/hthreadpool_p.h"
#include "../dataelements/hactioninfo.h"

#include <QtCore/QHash>
#include <QtCore/QUuid>
#include <QtCore/QMutex>
#include <QtCore/QString>
#include <QtCore/QWaitCondition>
#include <QtCore/QSharedPointer>
#include <QtCore/QScopedPointer>

namespace Herqq
{

namespace Upnp
{

//
// The base class for synchronous and asynchronous invocation objects.
// The purpose of this class is to contain information about the action invocation
// required to run the operation to completion.
//
class HInvocation
{
H_DISABLE_COPY(HInvocation)

public:

    HActionPrivate* const m_action;
    const HActionArguments m_inArgs;
    HAsyncOp m_invokeId;
    QWaitCondition m_waitCond;
    HActionArguments m_outArgs;

    QAtomicInt m_hasListener;

    enum Status
    {
        NotStarted = 0,
        Running = 1,
        Finished = 2
    };

    volatile Status m_status;

public:

    HInvocation(HActionPrivate*, const HActionArguments& inArgs);
    virtual ~HInvocation() = 0;

    inline bool isCompleted() { return m_status == Finished; }
};

//
// The base class for synchronous and asynchronous action invocation runners.
// These classes start the action invocation operations.
//
class HActionInvoker
{
H_DISABLE_COPY(HActionInvoker)
friend class HActionPrivate;

protected:

    HActionPrivate* m_action;

public:

    HActionInvoker(HActionPrivate*);
    virtual ~HActionInvoker();

    virtual HInvocation* runAction(const HActionArguments&) = 0;
};

//
// This class contains the data needed to run an asynchronous action invocation.
//
class HAsyncInvocation :
    public HInvocation
{
H_DISABLE_COPY(HAsyncInvocation)

public:

    HAsyncInvocation(HActionPrivate*, const HActionArguments&);
    virtual ~HAsyncInvocation();
};

//
// This class runs the asynchronous action invocations.
//
class HAsyncActionInvoker :
    public HActionInvoker
{
H_DISABLE_COPY(HAsyncActionInvoker)
friend class HActionPrivate;

private:

    bool invokeComplete(HAsyncOp);

public:

    HAsyncActionInvoker(HActionPrivate*);
    virtual ~HAsyncActionInvoker();

    virtual HAsyncInvocation* runAction(const HActionArguments&);
};

//
// This class contains the data needed to run a synchronous action invocation.
//
class HSyncInvocation :
    public HRunnable,
    public HInvocation
{
H_DISABLE_COPY(HSyncInvocation)
public:

    HSyncInvocation(HActionPrivate*, const HActionArguments&);
    virtual ~HSyncInvocation();
    virtual void run();
};

//
// This class runs the synchronous action invocations.
//
class HSyncActionInvoker :
    public HActionInvoker
{
H_DISABLE_COPY(HSyncActionInvoker)
friend class HActionPrivate;

private:

    HThreadPool* m_threadPool;

public:

    HSyncActionInvoker(HActionPrivate*, HThreadPool*);
    virtual ~HSyncActionInvoker();

    virtual HSyncInvocation* runAction(const HActionArguments&);
};

//
//
//
class HActionInvokeProxy
{
H_DISABLE_COPY(HActionInvokeProxy)

protected:

    HActionInvokeCallback m_callback;

public:

    HActionInvokeProxy();
    virtual ~HActionInvokeProxy();
    virtual bool beginInvoke(HAsyncInvocation*) = 0;

    inline void setCallback(const HActionInvokeCallback& cb)
    {
        m_callback = cb;
    }
};

//
// This is an internal class that provides more powerful interface for interacting
// with HAction than what the HAction's public interface offers.
//
// These features are required so that the HService
// can appropriately manage the HAction instances they own.
//
class HActionController
{
H_DISABLE_COPY(HActionController)

public:

    HAction* m_action;

    HActionController(HAction* action);
    virtual ~HActionController();

    qint32 invoke(const HActionArguments&, HActionArguments*);
};

//
// Implementation details of HAction
//
class HActionPrivate
{
H_DECLARE_PUBLIC(HAction)
H_DISABLE_COPY(HActionPrivate)
friend class HSyncInvocation;
friend class HAsyncActionInvoker;

private:

    void onActionInvocationComplete(const HAsyncOp& invokeId);

    bool waitForInvocation(
        HAsyncOp* waitResult, HActionArguments* outArgs);

    HAsyncOp invoke(
        const HActionArguments& inArgs, HExecArgs* execArgs);

    HAsyncOp invoke(
        const HActionArguments& inArgs, const HActionInvokeCallback&,
        HExecArgs* execArgs);

public:

    HAction* q_ptr;
    QScopedPointer<HActionInfo> m_info;

    HService* m_parentService;

    union
    {
        HActionInvoke* m_actionInvoke;
        HActionInvokeProxy* m_actionInvokeProxy;
    };

    HActionInvoker* m_actionInvoker;

    typedef QSharedPointer<HInvocation> HInvocationPtrT;

    struct InvocationInfo
    {
        HInvocationPtrT invocation;
        HActionInvokeCallback callback;
        HExecArgs* execArgs;

        InvocationInfo() :
            invocation(0), callback(), execArgs(0)
        {
        }

        ~InvocationInfo()
        {
            delete execArgs;
        }

        InvocationInfo(const InvocationInfo& other) :
            invocation(other.invocation), callback(other.callback),
            execArgs(other.execArgs ? new HExecArgs(*other.execArgs) : 0)
        {
            Q_ASSERT(&other != this);
        }

        InvocationInfo& operator=(const InvocationInfo& other)
        {
            Q_ASSERT(&other != this);

            invocation = other.invocation;
            callback = other.callback;
            if (execArgs) { delete execArgs; }
            execArgs = other.execArgs ? new HExecArgs(*other.execArgs) : 0;
            return *this;
        }

        InvocationInfo(
            const HInvocationPtrT& invocation, const HActionInvokeCallback& cb,
            HExecArgs* eargs) :
                invocation(invocation), callback(cb),
                execArgs(eargs ? new HExecArgs(*eargs) : 0)
        {
        }
    };

    QHash<QUuid, InvocationInfo> m_invocations;
    QMutex m_invocationsMutex;

public:

    HActionPrivate();
    ~HActionPrivate();

    bool setInfo(const HActionInfo&);
    bool setActionInvoke(const HActionInvoke&);
    bool setInvoker(HActionInvoker*);
};

}
}

#endif /* HACTION_P_H_ */
