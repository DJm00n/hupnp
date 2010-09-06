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

#ifndef HABSTRACTHOST_P_H_
#define HABSTRACTHOST_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "hdevicestorage_p.h"

#include "../general/hupnp_fwd.h"
#include "../http/hhttp_handler_p.h"

#include "../../utils/hthreadpool_p.h"

#include <QtCore/QObject>
#include <QtCore/QAtomicInt>

class QString;

namespace Herqq
{

namespace Upnp
{

class HUdn;
class HSharedActionInvoker;

//
//
//
class H_UPNP_CORE_EXPORT HAbstractHostPrivate :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HAbstractHostPrivate)

private:

    virtual void doClear() = 0;

public: // attributes

    const QByteArray m_loggingIdentifier;
    // the prefix shown before the actual log output

    QScopedPointer<HHttpHandler> m_http;
    // the helper object used in http messaging

    QScopedPointer<DeviceStorage> m_deviceStorage;
    // the storage for device model.

    HThreadPool* m_threadPool;
    // thread pool for worker threads

    QAtomicInt m_initializationStatus;
    // -1 exiting, can change to state 0
    // 0 uninitialized / closed, can change to state 1
    // 1 initializing, can change to state 2, or -1
    // 2 initialized, can change to state -1

    QString m_lastErrorDescription;
    // description of the error that occurred last

    enum InitState
    {
        Exiting = -1,
        Uninitialized = 0,
        Initializing = 1,
        Initialized = 2
    };

    inline InitState state() const
    {
        return static_cast<InitState>(static_cast<qint32>(m_initializationStatus));
    }

    inline void setState(InitState arg)
    {
        switch (arg)
        {
        case Exiting:
            m_initializationStatus = -1;
            break;

        case Uninitialized:
            m_initializationStatus = 0;
            break;

        case Initializing:
            m_initializationStatus = 1;
            break;

        case Initialized:
            m_initializationStatus = 2;
            break;
        }
    }

public: // methods

    HAbstractHostPrivate(const QString& loggingIdentfier = "");
    virtual ~HAbstractHostPrivate();

    void clear();
    // clears the state of the host. purges everything and shuts down every
    // running task
};

}
}

#endif /* HABSTRACTHOST_P_H_ */
