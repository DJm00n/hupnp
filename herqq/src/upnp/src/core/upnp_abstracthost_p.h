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

#ifndef UPNP_HOST_P_H_
#define UPNP_HOST_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "upnp_fwd.h"
#include "upnp_device_p.h"
#include "messaging/endpoint.h"
#include "messaging/http_server_p.h"

#include <QList>
#include <QMutex>
#include <QObject>
#include <QAtomicInt>
#include <QThreadPool>
#include <QSharedPointer>

class QUrl;
class QString;

namespace Herqq
{

namespace Upnp
{

class HUdn;
class HAbstractHost;
class HSharedActionInvoker;

//
// Implementation details of HAbstractHost
//
class HAbstractHostPrivate :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HAbstractHostPrivate)

private:

    void checkDeviceTreeForUdnConflicts(HDeviceController* device) const;
    HDeviceController* searchRootDevice(const QUrl& arg) const;

    virtual void doClear() = 0;

public: // attributes

    HHttpHandler m_http;
    QList<HDeviceController*> m_rootDevices;
    mutable QMutex m_rootDevicesMutex;
    HAbstractHost* q_ptr;
    QThreadPool* m_threadPool;
    const QByteArray m_loggingIdentifier;

    QAtomicInt m_initializationStatus;
    // -1 exiting, can change to state 0
    // 0 uninitialized / closed, can change to state 1
    // 1 initializing, can change to state 2, or -1
    // 2 initialized, can change to state -1

    QHash<HUdn, HSharedActionInvoker*> m_sharedActionInvokers;

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

    void addRootDevice(HDeviceController* root);
    void removeRootDevice(HDeviceController* root);

    HDeviceController*  searchDeviceByUdn(const HUdn& udn) const;
    HServiceController* searchServiceByScpdUrl(const QUrl& scpdUrl) const;
    HServiceController* searchServiceByControlUrl(
        const QUrl& controlUrl) const;

    QPair<QUrl, QImage> searchIcon(const QString& path) const;

    HServiceController* searchServiceByEventUrl(const QUrl& eventUrl) const;

    QList<HDeviceController*> searchDevicesByDeviceType(
        const HResourceType& deviceType, bool exactMatch = true)const;

    QList<HServiceController*> searchServicesByServiceType(
        const HResourceType& serviceType, bool exactMatch = true) const;

    static bool searchValidLocation(
        const HDevice* device, const HEndpoint& interface, QUrl* location);

    HRootDevicePtrListT rootDevices() const;
};

}
}

#endif /* UPNP_HOST_P_H_ */
