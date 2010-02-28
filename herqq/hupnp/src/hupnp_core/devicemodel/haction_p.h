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

#ifndef HUPNP_ACTION_P_H_
#define HUPNP_ACTION_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "haction.h"
#include "hactioninvoke.h"
#include "hactionarguments.h"

#include <QHash>
#include <QUuid>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QRunnable>
#include <QWaitCondition>
#include <QScopedPointer>
#include <QSharedPointer>

class QThreadPool;

namespace Herqq
{

namespace Upnp
{

//
// This class enables asynchronous action invocation amongst actions within
// a device.
//
class HSharedActionInvoker
{
H_DISABLE_COPY(HSharedActionInvoker)
friend class HActionPrivate;

    //
    //
    //
    class Invocation :
        public QRunnable
    {
    H_DISABLE_COPY(Invocation)

    public:

        HActionPrivate* m_action;
        HActionInputArguments m_iargs;
        QUuid m_invokeId;
        QWaitCondition m_waitCond;
        HActionOutputArguments m_outArgs;
        qint32 m_rc;

        QAtomicInt m_hasListener;

    public:

        Invocation(HActionPrivate*, const HActionInputArguments&, const QUuid&);
        virtual void run();

        inline bool isCompleted() { return m_rc != 0x0fffffff; }
    };

private:

    QThreadPool* m_threadPool;
    qint32 m_actionCount;

public:

    HSharedActionInvoker(QThreadPool*);
    ~HSharedActionInvoker();

    Invocation* runAction(HActionPrivate*, const HActionInputArguments&);
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

    qint32 invoke(
        const Herqq::Upnp::HActionInputArguments&,
        Herqq::Upnp::HActionOutputArguments*);
};

//
// Implementation details of HAction
//
class HActionPrivate
{
H_DECLARE_PUBLIC(HAction)
H_DISABLE_COPY(HActionPrivate)
friend class HSharedActionInvoker::Invocation;

private:

    void onActionInvocationComplete(const QUuid& invokeId, qint32 rc);

    HAction::InvocationWaitReturnValue waitForInvocation(
        const QUuid& invokeId, qint32* rc, qint32 timeout, HActionOutputArguments*);

    QUuid invoke(const HActionInputArguments& inArgs);
    QUuid invoke(const HActionInputArguments&, const HActionInvokeCallback&);

public:

    HAction* q_ptr;
    QString m_name;
    QScopedPointer<HActionInputArguments>  m_inputArguments;
    QScopedPointer<HActionOutputArguments> m_outputArguments;
    bool m_hasRetValArg;

    HService* m_parentService;
    HActionInvoke m_actionInvoke;

    HSharedActionInvoker* m_sharedActionInvoker;

    typedef QSharedPointer<HSharedActionInvoker::Invocation> InvocationPtrT;

    QHash<QUuid, QPair<InvocationPtrT, HActionInvokeCallback> > m_invocations;
    QMutex m_invocationsMutex;

public:

    HActionPrivate();
    ~HActionPrivate();

    bool setName      (const QString& name);
    bool setInputArgs (const HActionInputArguments& inputArguments);
    bool setOutputArgs(
        const HActionOutputArguments& outputArguments, bool hasRetValArg);

    bool setActionInvoke(const HActionInvoke& ai);
    bool setSharedInvoker(HSharedActionInvoker*);
};

}
}

#endif /* HUPNP_ACTION_P_H_ */
