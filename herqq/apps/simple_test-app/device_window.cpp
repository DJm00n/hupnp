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

#include "device_window.h"
#include "ui_device_window.h"

#include <HUpnp>
#include <HAction>
#include <HServiceId>
#include <HDeviceHost>
#include <HStateVariable>
#include <HActionArguments>
#include <HActionArguments>
#include <HWritableStateVariable>
#include <HDeviceHostConfiguration>

#include <QtDebug>
#include <QDateTime>

using namespace Herqq::Upnp;

/*******************************************************************************
 * HTestService
 *******************************************************************************/
HTestService::HTestService()
{
}

HTestService::~HTestService()
{
}

HService::HActionMapT HTestService::createActions()
{
    HActionMapT retVal;

    //
    // This is where it is defined what are to be called when the actions
    // identified by their names are invoked.

    // In this example, public member functions are used.
    retVal["Echo"]     = HActionInvoke(this, &HTestService::echoAction);
    retVal["Register"] = HActionInvoke(this, &HTestService::registerAction);
    retVal["Chargen"]  = HActionInvoke(this, &HTestService::chargenAction);

    return retVal;
}

qint32 HTestService::echoAction(
    const HActionArguments& inArgs,
    HActionArguments* outArgs)
{
    // Simple implementation of the echo service:
    // merely echos the received message back to the invoker.

    QString echoMsg = inArgs["MessageIn"]->value().toString();
    (*outArgs)["MessageOut"]->setValue(echoMsg);

    emit actionInvoked(
        "Echo", QString("Argument was set to [%1].").arg(echoMsg));

    // This signal is sent so that the user interface can react to this
    // particular invocation somehow.

    return HAction::Success();
}

qint32 HTestService::registerAction(
    const HActionArguments& /*inArgs*/,
    HActionArguments* /*outArgs*/)
{
    // Simple implementation of the Register service:
    // modifies an evented state variable, which causes events to be sent to
    // all registered listeners.

    bool ok = false;
    HWritableStateVariable* sv =
        stateVariableByName("RegisteredClientCount")->toWritable();

    Q_ASSERT(sv);
    // fetch the state variable we want to modify

    HStateVariableLocker svLocker(sv);
    // lock the state variable to ensure
    // that the value of the state variable is not modified by another thread
    // while we are incrementing it

    quint32 count = sv->value().toUInt(&ok);
    Q_ASSERT(ok);
    // check its current value

    ok = sv->setValue(++count);
    Q_ASSERT(ok);
    // and increment it

    svLocker.unlock();
    // explicitly unlock the state variable. this is not necessary, but it is a
    // good practice to use, since it ensures that no matter what happens during
    // the event processing, the lock we don't need anymore isn't held.

    emit actionInvoked(
        "Register",
        QString("Register invoked %1 times.").arg(QString::number(count)));

    // This signal is sent so that the user interface can react to this
    // particular invocation somehow.

    return HAction::Success();
}

qint32 HTestService::chargenAction(
    const HActionArguments& inArgs,
    HActionArguments* outArgs)
{
    qint32 charCount = inArgs["Count"]->value().toInt();
    (*outArgs)["Characters"]->setValue(QString(charCount, 'z'));

    emit actionInvoked(
        "Chargen",
        QString("Character count set to %1.").arg(
            QString::number(charCount)));

    // This signal is sent so that the user interface can react to this
    // particular invocation somehow.

    return HAction::Success();
}

/*******************************************************************************
 * HTestDevice
 ******************************************************************************/
HTestDevice::HTestDevice() :
    HDevice()
{
}

HTestDevice::~HTestDevice()
{
}

HDevice::HServiceMapT HTestDevice::createServices()
{
    HDevice::HServiceMapT retVal;

    retVal[HResourceType("urn:herqq-org:service:HTestService:1")] =
        new HTestService();

    // This UPnP device has a single service identified by serviceId
    // "urn:herqq-org:service:HTestService:1", which is mapped to our
    // type HTestService. The services are defined in the device description.

    return retVal;
}
/*******************************************************************************
 * DeviceWindow
 *******************************************************************************/
namespace
{

//
// This simple functor is used to create our HTestDevice type.
//
class Creator
{
public:

    HDevice* operator()(const HDeviceInfo& /*deviceInfo*/)
    {
        // If we wanted to use this functor for creating other types as well,
        // we would quite likely read the received "deviceInfo" and base the
        // object creation on that.
        return new HTestDevice();
    }
};
}

DeviceWindow::DeviceWindow(QWidget *parent) :
    QMainWindow(parent),
        m_ui(new Ui::DeviceWindow), m_deviceHost(0), m_testDevice()
{
    m_ui->setupUi(this);

    HDeviceConfiguration initParams;
    initParams.setPathToDeviceDescription(
        "./descriptions/hupnp_testdevice.xml");
    // the path to the device description file we want to be instantiated

    initParams.setDeviceCreator(Creator());
    // the functor that is used for creating the HDevice types.

    initParams.setCacheControlMaxAge(30);

    m_deviceHost = new HDeviceHost();

    QString err;
    if (m_deviceHost->init(initParams, &err) != HDeviceHost::Success)
    {
        qWarning() << err;
        Q_ASSERT(false);
        return;
    }

    m_testDevice = m_deviceHost->rootDevices().at(0);
    // since we know there is at least one device if the initialization succeeded...

    HService* service =
        m_testDevice->serviceById(HServiceId("urn:upnp-org:serviceId:HTestService"));

    // our user interface is supposed to react when our actions are invoked, so
    // let's connect the signal introduced in HTestService to this class.
    // (note that the connection works although the static type of our "service"
    // is not HTestService during the connect() call)

    bool ok = connect(
        service, SIGNAL(actionInvoked(QString, QString)),
        this, SLOT(actionInvoked(QString, QString)));

    Q_ASSERT(ok); Q_UNUSED(ok)
}

DeviceWindow::~DeviceWindow()
{
    delete m_ui;

    //
    // **THIS IS IMPORTANT**
    // The HRootDevicePtrT is cleared before deleting the HDeviceHost, as instructed
    // in the documentation. Otherwise double deletion would occur.
    m_testDevice.clear();

    delete m_deviceHost;
}

void DeviceWindow::actionInvoked(const QString& actionName, const QString& text)
{
    //
    // okay, one of our actions was invoked, let's display something.

    QString textToDisplay = QString("%1 Action [%2] invoked: %3").arg(
        QDateTime::currentDateTime().toString(), actionName, text);

    m_ui->statusDisplay->append(textToDisplay);
}

void DeviceWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);

    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void DeviceWindow::closeEvent(QCloseEvent*)
{
    emit closed();
}
