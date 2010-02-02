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

#ifndef UPNP_DEVICE_P_H_
#define UPNP_DEVICE_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../upnp_fwd.h"
#include "../../../../core/include/HGlobal"

#include <QUrl>
#include <QList>
#include <QMutex>
#include <QTimer>
#include <QAtomicInt>
#include <QDomDocument>
#include <QScopedPointer>

namespace Herqq
{

namespace Upnp
{

//
//
//
class HDeviceStatus
{
H_DISABLE_COPY(HDeviceStatus)

private:

    qint32 m_bootId;
    qint32 m_configId;
    qint32 m_searchPort;

public:

    HDeviceStatus ();
    ~HDeviceStatus();

    qint32  bootId    () const;
    qint32  configId  () const;
    quint32 searchPort() const;
};

class HServiceController;

//
// This is an internal class that provides more powerful interface for interacting
// with HDevices than what the HDevice's public interface offers.
//
// These features are required so that the HUpnpContolPoint and HDeviceHost
// can appropriately manage the HDevice instances they own.
//
class H_UPNP_CORE_EXPORT HDeviceController :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HDeviceController)

private:

    volatile bool m_timedout;
    QScopedPointer<QTimer> m_statusNotifier;
    QScopedPointer<HDeviceStatus> m_deviceStatus;

private Q_SLOTS:

    void timeout_();

public:

    QSharedPointer<HDevice> m_device;
    qint32 m_configId;

public:

    enum SearchCriteria
    {
        ThisOnly = 0,
        EmbeddedDevices = 1,
        Services = 2,
        All = 3
    };

    HDeviceController(
        HDevice* device, qint32 deviceTimeoutInSecs, QObject* parent = 0);

    virtual ~HDeviceController();

    QList<HServiceController*> services() const;
    QList<HDeviceController*>  embeddedDevices() const;
    HDeviceController* parentDevice() const;
    HDeviceController* rootDevice();
    HDeviceStatus* deviceStatus() const;
    qint32 deviceTimeoutInSecs() const;

    void startStatusNotifier(SearchCriteria searchCriteria);
    void stopStatusNotifier(SearchCriteria searchCriteria);

    void addLocation(const QUrl& location);
    void addLocations(const QList<QUrl>& locations);
    bool isTimedout(SearchCriteria searchCriteria) const;

public Q_SLOTS:

    void dispose();

Q_SIGNALS:

    void statusTimeout(HDeviceController* source);
};

class HDeviceInfo;
class HServiceController;

//
// Base class for the implementation details of HDevice
//
class H_UPNP_CORE_EXPORT HDevicePrivate
{
H_DISABLE_COPY(HDevicePrivate)

public: // attributes

    QScopedPointer<HDeviceInfo> m_upnpDeviceInfo;
    QList<HDeviceController*>  m_embeddedDevices;
    QList<HServiceController*> m_services;
    HDeviceController*         m_parent;
    HDevice*                   q_ptr;
    QList<QUrl>                m_locations;
    QDomDocument               m_deviceDescription;
    QAtomicInt                 m_disposed;

    mutable QMutex m_locationsMutex;

public: // methods

    HDevicePrivate();
    virtual ~HDevicePrivate();

    inline static QString deviceDescriptionPostFix()
    {
        static QString retVal = "device_description.xml";
        return retVal;
    }
};

}
}

#endif /* UPNP_DEVICE_P_H_ */
