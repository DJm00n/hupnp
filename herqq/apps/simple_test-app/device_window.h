/*
 *  Copyright (C) 2010 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of an application named HUpnpSimpleTestApp
 *  used for demonstrating how to use the Herqq UPnP (HUPnP) library.
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

#ifndef DEVICE_WINDOW_H
#define DEVICE_WINDOW_H

#include <HUpnp>
#include <HDevice>
#include <HService>

#include <QtGui/QMainWindow>

namespace Ui {
    class DeviceWindow;
}

class DeviceWindow;

//
// Test HService created by the HTestDevice. This is the only service
// exposed by the HTestDevice
//
class HTestService :
    public Herqq::Upnp::HService
{
Q_OBJECT
Q_DISABLE_COPY(HTestService)

private:

    virtual Herqq::Upnp::HService::HActionMapT createActions();

public:

    HTestService ();
    virtual ~HTestService();

    //
    // The following methods could very well be private. In fact, they
    // could be regular functions or functors defined elsewhere as well.
    // It is a design decision whether you wish your service type to be used directly,
    // in which case exposing its UPnP actions as methods of the service
    // like in the following case ease the use of the class.
    qint32 echoAction(
        const Herqq::Upnp::HActionInputArguments& inArgs,
        Herqq::Upnp::HActionOutputArguments* outArgs = 0);

    qint32 registerAction(
        const Herqq::Upnp::HActionInputArguments& inArgs,
        Herqq::Upnp::HActionOutputArguments* outArgs = 0);

    qint32 chargenAction(
        const Herqq::Upnp::HActionInputArguments& inArgs,
        Herqq::Upnp::HActionOutputArguments* outArgs = 0);

Q_SIGNALS:

    void actionInvoked(const QString& actionName, const QString& text);
};

//
// A class created internally to represent our UPnP test device.
//
class HTestDevice :
    public Herqq::Upnp::HDevice
{
Q_OBJECT
Q_DISABLE_COPY(HTestDevice)

private:

    virtual Herqq::Upnp::HDevice::HServiceMapT createServices();

public:

    explicit HTestDevice();
    virtual ~HTestDevice();
};

//
//
//
class DeviceWindow :
    public QMainWindow
{
Q_OBJECT
Q_DISABLE_COPY(DeviceWindow)

private:

    Ui::DeviceWindow* m_ui;

    Herqq::Upnp::HDeviceHost* m_deviceHost;
    // ^^ This is needed to host the HTestDevice

    Herqq::Upnp::HRootDevicePtrT m_testDevice;
    // The hosted root HDevice is stored as a member so that there is no way
    // the root HDevice could be deleted while we may be using it. The use of the
    // typedef HRootDevicePtrT ensures that.
    //
    // In more detail, a root HDevice hosted by a HDeviceHost will not be deleted
    // until the device host is shutdown or deleted. The root device is stored
    // here just for this example to demonstrate that:
    // 1) You may use the hosted HDevice directly while control points may be
    // accessing it through the network.
    // 2) If you plan on using any device hosted by a HAbstractHost (base class
    // of HDeviceHost), it is a good practice to ensure that those devices
    // _cannot_ be deleted while you may using them.

protected:

    virtual void changeEvent(QEvent*);
    virtual void closeEvent(QCloseEvent*);

public:

    explicit DeviceWindow(QWidget *parent = 0);
    virtual ~DeviceWindow();

private slots:

    void actionInvoked(const QString&, const QString&);

Q_SIGNALS:

    void closed();
};

#endif // DEVICE_WINDOW_H
