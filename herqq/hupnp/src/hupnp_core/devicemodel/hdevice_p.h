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

#ifndef HDEVICE_P_H_
#define HDEVICE_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "hdevice.h"
#include "../general/hdefs_p.h"
#include "../general/hupnp_fwd.h"
#include "../general/hupnp_global_p.h"

#include <QUrl>
#include <QList>
#include <QMutex>
#include <QTimer>
#include <QHostAddress>
#include <QDomDocument>
#include <QScopedPointer>

namespace Herqq
{

namespace Upnp
{

class HDeviceController;

//
//
//
class HDeviceStatus
{
friend class HDeviceController;

private:

    qint32 m_bootId;
    qint32 m_configId;
    qint32 m_searchPort;

    bool m_online;

public:

    inline HDeviceStatus() :
        m_bootId(0), m_configId(0), m_searchPort(0), m_online(true)
    {
    }

    inline qint32 bootId() const { return m_bootId; }
    inline qint32 configId() const { return m_configId; }
    inline qint32 searchPort() const { return m_searchPort; }

    inline bool online() const { return m_online; }
    inline void setOnline(bool arg) { m_online = arg; }
};

class HDeviceInfo;
class HServiceController;

//
// Base class for the implementation details of HDevice
//
class H_UPNP_CORE_EXPORT HDevicePrivate
{
H_DECLARE_PUBLIC(HDevice)
H_DISABLE_COPY(HDevicePrivate)

public: // attributes

    QScopedPointer<HDeviceInfo> m_upnpDeviceInfo;
    QList<HDeviceController*>   m_embeddedDevices;
    QList<HServiceController*>  m_services;
    HDeviceController*          m_parent;
    // ^^ this is not the "QObject" parent, but rather the parent in the
    // device tree. In other words, this device controller contains the HDevice
    // that is our "UPnP device parent".

    HDevice*                    q_ptr;
    QList<QUrl>                 m_locations;
    QDomDocument                m_deviceDescription;

    mutable QMutex m_locationsMutex;

public: // methods

    HDevicePrivate();
    virtual ~HDevicePrivate();

    inline static QString deviceDescriptionPostFix()
    {
        static QString retVal = "device_description.xml";
        return retVal;
    }

    inline bool isValid() const { return m_upnpDeviceInfo.data(); }
};

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

    union
    {
        HDevice* m_device;
        HDeviceProxy* m_deviceProxy;
    };

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

    inline const QList<HServiceController*>* services() const
    {
        return &m_device->h_ptr->m_services;
    }

    inline const QList<HDeviceController*>* embeddedDevices() const
    {
        return &m_device->h_ptr->m_embeddedDevices;
    }

    inline qint32 deviceTimeoutInSecs() const
    {
        return m_statusNotifier->interval() / 1000;
    }

    inline HDeviceController* parentDevice() const
    {
        return m_device->h_ptr->m_parent;
    }

    inline HDeviceController* rootDevice() const
    {
        HDeviceController* root = const_cast<HDeviceController*>(this);
        while (root->parentDevice()) { root = root->parentDevice(); }
        return root;
    }

    inline HDeviceStatus* deviceStatus() const
    {
        if (!parentDevice()) { return m_deviceStatus.data(); }
        return rootDevice()->deviceStatus();
    }

    void startStatusNotifier(SearchCriteria searchCriteria);
    void stopStatusNotifier(SearchCriteria searchCriteria);

    bool addLocation(const QUrl& location);
    void addLocations(const QList<QUrl>& locations);
    bool isTimedout(SearchCriteria searchCriteria) const;

Q_SIGNALS:

    void statusTimeout(HDeviceController* source);
};

}
}

#endif /* HDEVICE_P_H_ */
