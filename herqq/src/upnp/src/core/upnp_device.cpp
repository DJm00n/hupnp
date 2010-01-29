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

#include "upnp_device.h"
#include "upnp_global_p.h"
#include "upnp_device_p.h"
#include "upnp_service_p.h"
#include "upnp_deviceinfo.h"

#include "../../../utils/src/logger_p.h"

#include <QString>

/*! \mainpage Herqq UPnP (HUPnP) Reference Documentation
 *
 * \section introduction Introduction
 *
 * %Herqq UPnP Library (thereafter HUPnP) is a collection of reusable classes that
 * provide UPnP connectivity conforming to the
 * <a href="http://www.upnp.org/specs/arch/UPnP-arch-DeviceArchitecture-v1.1.pdf">
 * UPnP Device Architecture version 1.1</a>.
 *
 * Above everything else, HUPnP is designed to be simple to use and robust in operation.
 * HUPnP does its best to enable you, the client, to focus on the business logic
 * of your domain, rather than to the details of UPnP. However, not everything
 * can be hidden and some UPnP knowledge is required to fully understand
 * the system mechanics and the terms used in this documentation. To fill such
 * gaps, you can check the aforementioned UDA specification and other documents
 * available at the <a href="http://www.upnp.org/resources/documents.asp">UPnP Forum</a>.
 *
 * HUPnP is tightly integrated into the <a href="http://qt.nokia.com/">Qt Framework</a> and
 * follows closely the very same design principles and programming practices Qt follows. You will
 * get the most out of HUPnP by using it alongside with Qt. You can use HUPnP
 * from other environments as well, assuming the appropriate Qt headers and libraries are available.
 *
 * \section settingup Setting Up
 *
 * First, it is important to point out that at the moment HUPnP is
 * officially distributed in source code only. If you
 * come across a binary of HUPnP, it is not made by the author of HUPnP.
 * Second, HUPnP uses the
 * <a href="http://qt.nokia.com/products/appdev/add-on-products/catalog/4/Utilities/qtsoap">QtSoap library</a>
 * under the <a href="http://www.gnu.org/licenses/gpl.html">GPLv3</a> license.
 * The QtSoap library is distributed along the HUPnP in
 * source code and the library is built into a shared library during the compilation of HUPnP.
 * \attention At the moment, HUPnP uses a modified version of QtSoap version 2.7.
 * This is because the original version 2.7 contains few severe bugs in regard
 * to thread-safety. Until the errors are fixed in the official QtSoap release, the users
 * of HUPnP \b must use the modified version distributed with HUPnP.
 *
 * In order to use HUPnP, you need to build it first. By far the simplest way to do
 * this is to download the <a href="http://qt.nokia.com/downloads">Qt SDK</a>,
 * install it, start QtCreator and open the HUPnP project file \b herqq.pro
 * located in the root of the HUPnP package.
 *
 * The build produces three shared libraries to which you need to link in order to use
 * HUPnP. Currently, static linking is not an option. The created libraries are
 * placed in \c bin directory and they are named \c HCore.x, \c HUPnP.x and
 * \c QtSolutions_SOAP-2.7.x, where \c ".x" is the platform dependent suffix
 * for shared libraries. In addition, your compiler
 * must be aware of the HUPnP includes, which can be found in the \c include
 * directory. It is very important that you do \b not directly include anything that
 * is not found in the \c include directory. In any case, once your compiler finds the HUPnP includes and your
 * linker finds the HUPnP shared library, you are good to go.
 * \attention You do \b not need to include the QtSoap includes. HUPnP does not expose
 * the types declared in QtSoap.
 *
 * \section importantnotes Important notes
 *
 * Before starting to use HUPnP, there are a few things you have to know.
 *
 * \subsection include \#include
 *
 * HUPnP follows similar include pattern to that of Qt. When you want to use a
 * a class in HUPnP, you have to use \verbatim #include <HClassName> \endverbatim where <c>HClassName</c>
 * matches the name of the class you wish to use exactly.
 *
 * \subsection memorymanagement Memory management
 *
 * Some of the key classes in HUPnP are always instantiated by HUPnP and the
 * instances are always destroyed by HUPnP. You should never delete these.
 * The API documentation of HUPnP is clear about object ownership and
 * these classes are identified in documentation.
 *
 * \subsection herqqheader The <Herqq> include file
 *
 * %Herqq libraries introduce a number of types, functions, enums and
 * type definitions under the root namespace \c Herqq. For instance, all the
 * HUPnP core types can be found under the namespace Herqq::Upnp.
 *
 * In several occasions, you do not need to include the full %Herqq type definitions for your
 * code to work. More specifically, if the compiler doesn't need to see the layout
 * of a %Herqq type to compile your code, you should only forward-declare such %Herqq
 * types that are seen <em>in name</em> only. In such a case, you can include the
 * <c><Herqq></c> file, which provides forward-declarations to every public
 * %Herqq type and function.
 *
 * \subsection logging Logging
 *
 * In many situations it is useful to get some log output to clarify what is
 * going on under the hood. This is especially true in case something appears
 * to be malfunctioning. You can enable logging in HUPnP by calling the
 * function Herqq::Upnp::SetLoggingLevel() with a desired \e level argument.
 * Include <c><Herqq></c> to use the Herqq::Upnp::SetLoggingLevel().
 *
 * \section gettingstarted Getting Started
 *
 * Often the best explanation is demonstration.
 * So without further ado, the following links should get you started.
 *
 * \li \ref builddevice_tutorial shows how to build your own UPnP device using the
 * HUPnP.
 * \li The API documentation of Herqq::Upnp::HDeviceHost shows how to host a Herqq::Upnp::HDevice.
 * \li The API documentation of Herqq::Upnp::HControlPoint shows how to discover and use UPnP devices
 * on the network.
 *
 * For more detailed information, you can check
 *
 * \li \ref devicemodel for the details of the HUPnP device model and
 * \li \ref devicehosting for the details of hosting a device.
 *
 * From there, the API reference is the way to go.
 */

/*!
 * \defgroup devicemodel Device Model
 *
 * \brief This page explains the concept of HUPnP Device Model, which is the
 * logical object hierarchy of HUPnP representing the UPnP Device Architecture.
 *
 * \section notesaboutdesign A few notes about the design
 *
 * The main four components of the UPnP device model are
 * <em>a device</em> (Herqq::Upnp::HDevice), <em>a service</em> (Herqq::Upnp::HService),
 * <em>a state variable</em> (Herqq::Upnp::HStateVariable) <em>and an action</em> (Herqq::Upnp::HAction).
 * These four components form a type of a tree, in which devices and services are contained by devices,
 * and state variables and actions are contained by services.
 *
 * \subsection lifetime_and_ownership The lifetime and ownership of objects
 *
 * The root component is always a device, which is labeled as the <em>root device</em>.
 * In HUPnP, there is a type definition for such a device: Herqq::Upnp::HRootDevicePtrT.
 * Although an \c HDevice, a <em>root device</em> is a special device that owns and controls
 * any <em>embedded device</em> or \em service it contains, recursively.
 * Perhaps the most important outcome of this is that the lifetime of every other
 * object underneath the root device depends of the lifetime of the root device.
 *
 * A Herqq::Upnp::HRootDevicePtrT is a type definition for an Herqq::Upnp::HDevice wrapped
 * in a \c QSharedPointer. This implies that you never have the ownership of a
 * root device. Rather, once you are done with it, you should just discard the
 * pointer. The root device will be deleted once every user of the object
 * has done the same. Note, however, that when the root device is deleted, every
 * device, service, state variable and action underneath it are deleted as well.
 * In essence, it is safe to store and use pointers to these objects as long as you have
 * at least one valid \c HRootDevicePtrT in scope. If that is not the case, you should
 * never access any stored pointer to any of these objects, since it may result
 * in memory access violation.
 *
 * The components of the device model (devices, services, state variables and actions)
 * are always owned by the HUPnP and the ownership is never transferred. You should
 * never delete any of these objects, as it would eventually cause <em>double deletion</em>,
 * which generally means <em>undefined behavior</em>.
 *
 * \subsection devicedisposal Device disposal
 *
 * As stated previously, you never have the ownership of the objects that build the
 * HUPnP Device Model. These objects are created, updated and destroyed by the HUPnP.
 * This eases memory management significantly in many ways. However, since any client
 * at any time can hold a pointer to a root device, HUPnP cannot delete the
 * root device or any of the objects it contains until every client of the root device
 * have dropped their references to it. Regardless, there are situations where the
 * root device should be deleted promptly.
 *
 * For instance, it may be desirable that when
 * a UPnP device is removed from the network, the control points update their
 * internal structures accordingly, usually removing the object structures that
 * represent the device.
 *
 * It is because of situations like these the HUPnP
 * can mark a \c HDevice as \em disposed. A disposed device can be used, it's methods
 * can be called and so on, but it's state will no longer be updated and it's
 * action invocations will fail. Furthermore, when a device enters the disposed state,
 * it will never be usable again. This should be a clear sign for the users to
 * drop their references to the device, which will eventually allow the HUPnP to delete the
 * device tree altogether.
 *
 * \section usage Usage
 *
 * The device model is <em>location independent</em>, which in essence means that the
 * device model is \b always used the same way. That is, if you have a pointer to
 * any of the components of the device model, you use it the same way regardless
 * of whether you got the pointer directly or indirectly from a
 * Herqq::Upnp::HDeviceHost or a Herqq::Upnp::HControlPoint.
 *
 * Basic use is about
 * interacting with already created objects that comprise the device model.
 * To get started, you need to initialize a <em>device host</em> and retrieve a
 * list of <em>root devices</em> from it. See \ref devicehosting for more information about the
 * device hosts. Once you have a root device, you can interact with any of its
 * embedded devices, services, state variables and actions until the device gets disposed.
 * See the corresponding classes for more information concerning their use.
 *
 * \note It is very important to point out that HUPnP keeps the state of the
 * state variables always up-to-date. That is, an \c HControlPoint always
 * subscribes to events the UPnP services expose. When you have a \c HStateVariable
 * indirectly retrieved from an \c HControlPoint, it's state is always synchronized
 * with the UPnP device as long as the UPnP device keeps sending events.
 *
 * However, if you wish to implement and host your own UPnP device, things get
 * more involved. See \ref builddevice_tutorial to get you started on building your
 * own UPnP devices.
 *
 * \sa devicehosting
 */

/*! \page builddevice_tutorial Tutorial for Building a UPnP Device
 *
 * \section settingup_descriptions Setting up the device and service descriptions
 *
 * Generally, building a UPnP device with HUPnP involves two main steps in your part.
 * First, you have to define a \em UPnP \em device \em description document following
 * the specifications set by the UPnP forum. Depending of your UPnP Device Description
 * document, you may need to define one or more \em UPnP \em service \em description documents
 * as well. Second, you have to implement a class for your device and most often
 * one or more classes for each service of your device.
 *
 * For example, if you want to implement a standard UPnP device named
 * \b BinaryLight:1, your device description could look something like this:
 *
 * \code
 * <?xml version="1.0"?>
 * <root xmlns="urn:schemas-upnp-org:device-1-0">
 *     <specVersion>
 *         <major>1</major>
 *         <minor>0</minor>
 *     </specVersion>
 *     <device>
 *         <deviceType>urn:schemas-upnp-org:device:BinaryLight:1</deviceType>
 *         <friendlyName>UPnP Binary Light</friendlyName>
 *         <manufacturer>MyCompany</manufacturer>
 *         <manufacturerURL>www.mywebsite.org</manufacturerURL>
 *         <modelDescription>New brilliant BinaryLight</modelDescription>
 *         <modelName>SuperWhiteLight 4000</modelName>
 *         <modelNumber>1</modelNumber>
 *         <UDN>uuid:138d3934-4202-45d7-bf35-8b50b0208139</UDN>
 *         <serviceList>
 *             <service>
 *                 <serviceType>urn:schemas-upnp-org:service:SwitchPower:1</serviceType>
 *                 <serviceId>urn:upnp-org:serviceId:SwitchPower:1</serviceId>
 *                 <SCPDURL>switchpower_scpd.xml</SCPDURL>
 *                 <controlURL>/control</controlURL>
 *                 <eventSubURL>/eventing</eventSubURL>
 *             </service>
 *         </serviceList>
 *     </device>
 * </root>
 * \endcode
 *
 * Note that the above is the standard device template for UPnP \b BinaryLight:1 filled
 * with imaginary information.
 *
 * Since the \b BinaryLight:1 defines a service, \b SwitchPower:1, you have to provide a
 * service description document that could look like this:
 *
 * \code
 * <?xml version="1.0"?>
 * <scpd xmlns="urn:schemas-upnp-org:service-1-0">
 *     <specVersion>
 *         <major>1</major>
 *         <minor>0</minor>
 *     </specVersion>
 *     <actionList>
 *         <action>
 *             <name>SetTarget</name>
 *             <argumentList>
 *                 <argument>
 *                     <name>newTargetValue</name>
 *                     <relatedStateVariable>Target</relatedStateVariable>
 *                     <direction>in</direction>
 *                 </argument>
 *             </argumentList>
 *          </action>
 *          <action>
 *              <name>GetTarget</name>
 *              <argumentList>
 *                  <argument>
 *                      <name>RetTargetValue</name>
 *                      <relatedStateVariable>Target</relatedStateVariable>
 *                      <direction>out</direction>
 *                  </argument>
 *              </argumentList>
 *          </action>
 *          <action>
 *              <name>GetStatus</name>
 *              <argumentList>
 *                  <argument>
 *                      <name>ResultStatus</name>
 *                      <relatedStateVariable>Status</relatedStateVariable>
 *                      <direction>out</direction>
 *                  </argument>
 *              </argumentList>
 *          </action>
 *      </actionList>
 *      <serviceStateTable>
 *          <stateVariable sendEvents="no">
 *              <name>Target</name>
 *              <dataType>boolean</dataType>
 *              <defaultValue>0</defaultValue>
 *          </stateVariable>
 *          <stateVariable sendEvents="yes">
 *              <name>Status</name>
 *              <dataType>boolean</dataType>
 *              <defaultValue>0</defaultValue>
 *          </stateVariable>
 *      </serviceStateTable>
 * </scpd>
 * \endcode
 *
 * The above description is the standard service description for the
 * \b SwitchPower:1 without any vendor specific declarations. For more information
 * about description documents, see UDA specification, sections 2.3 and 2.5.
 *
 * \section creatingclasses Creating the necessary HUPnP classes
 *
 * With HUPnP, each description document has to be accompanied by a class.
 * So if we continue with the example, we have to derive a class
 * from Herqq::Upnp::HDevice for the \b BinaryLight:1 device description and
 * we have to derive a class from the Herqq::Upnp::HService for the
 * \b SwitchPower:1 service description.
 *
 * To create a concrete class from Herqq::Upnp::HDevice you have to implement its single abstract method
 * Herqq::Upnp::HServiceMapT Herqq::Upnp::HDevice::createServices(). As the name implies,
 * the purpose of this method is to create objects at run-time that reflect the services defined
 * in the device description. If your device has no services, this method has to
 * return an empty list.
 *
 * To create a concrete class from Herqq::Upnp::HService, you have to implement its single abstract method
 * Herqq::Upnp::HActionMapT Herqq::Upnp::HService::createActions(). The purpose of the method is to create
 * "callable entities" that reflect the "actions" of the service defined
 * in the service description. If your service has no actions, this method has
 * to return an empty list.
 *
 * To continue with the example, we have to create two classes and for this example
 * we put their declarations into the same header file:
 *
 * <c>mybinarylight.h</c>
 *
 * \code
 *
 * #include <HDevice>
 * #include <HService>
 *
 * class MyBinaryLightDevice :
 *    public Herqq::Upnp::HDevice
 * {
 * private:
 *
 *     virtual Herqq::Upnp::HServiceMapT createServices();
 *
 * public:
 *
 *    explicit MyBinaryLightDevice(QObject* parent = 0);
 *    virtual ~MyBinaryLightDevice();
 * };
 *
 * class MySwitchPowerService :
 *    public Herqq::Upnp::HService
 * {
 * private:
 *
 *     virtual Herqq::Upnp::HActionMapT createActions();
 *
 * public:
 *
 *     MySwitchPowerService();
 *     virtual ~MySwitchPowerService();
 * };
 *
 * \endcode
 *
 * In turn, the implementation could look something like this:
 *
 * <c>mybinarylight.cpp</c>
 *
 * \code
 *
 * #include "mybinarylight.h"
 *
 * using namespace Herqq::Upnp;
 *
 * MyBinaryLightDevice::MyBinaryLightDevice()
 * {
 * }
 *
 * MyBinaryLightDevice::~MyBinaryLightDevice()
 * {
 * }
 *
 * HDevice::HServiceMapT MyBinaryLightDevice::createServices()
 * {
 *   HServiceMapT retVal;
 *
 *   retVal[HResourceType("urn:schemas-upnp-org:service:SwitchPower:1")] =
 *       new MySwitchPower();
 *
 *   return retVal;
 * }
 *
 * MySwitchPowerService::MySwitchPowerService()
 * {
 * }
 *
 * MySwitchPowerService::~MySwitchPowerService()
 * {
 * }
 *
 * HService::HActionMapT MySwitchPowerService::createActions()
 * {
 *   HActionMapT retVal;
 *   return retVal;
 * }
 *
 * \endcode
 *
 * Those who know UPnP and paid close attention to the above example might have
 * noticed that something was off. Where are the actions?
 *
 * According to the UPnP Device Architecture (UDA), a service may have zero or
 * more actions defined. For a device type that has no services, or for a
 * device type that has services that in turn have no actions,
 * the above class declarations and definitions are enough.
 *
 * However, the standard \b BinaryLight device type specifies the \b SwitchPower service
 * that has three actions defined (look back in the service description document).
 * Namely these are \b SetTarget, \b GetTarget and \b GetStatus. To make the example complete,
 * the <c>MySwitchPowerService</c> class requires some additional work. Note, however, that the
 * next example shows only one way of making the service complete. There are
 * a few other ways, which will be discussed later in depth.
 *
 * The complete declaration for <c>MySwitchPowerService</c>:
 *
 * <c>mybinarylight.h</c>
 *
 * \code
 *
 * #include <HService>
 *
 * class MySwitchPowerService :
 *    public Herqq::Upnp::HService
 * {
 * private:
 *
 *     virtual Herqq::Upnp::HActionMapT createActions();
 *
 * public:
 *
 *     MySwitchPowerService();
 *     virtual ~MySwitchPowerService();
 *
 *     qint32 setTarget(
 *         const Herqq::Upnp::HActionInputArguments& inArgs,
 *         Herqq::Upnp::HActionOutputArguments* outArgs);
 *
 *     qint32 getTarget(
 *         const Herqq::Upnp::HActionInputArguments& inArgs,
 *         Herqq::Upnp::HActionOutputArguments* outArgs);
 *
 *     qint32 getStatus(
 *         const Herqq::Upnp::HActionInputArguments& inArgs,
 *         Herqq::Upnp::HActionOutputArguments* outArgs);
 * };
 *
 * \endcode
 *
 * The complete definition for <c>MySwitchPowerService</c>:
 *
 * <c>mybinarylight.cpp</c>
 *
 * \code
 *
 * #include "mybinarylight.h"
 *
 * #include <HStateVariable>
 * #include <HActionInputArguments>
 * #include <HActionOutputArguments>
 *
 * using namespace Herqq::Upnp;
 *
 * MySwitchPowerService::MySwitchPowerService()
 * {
 * }
 *
 * MySwitchPowerService::~MySwitchPowerService()
 * {
 * }
 *
 * qint32 MySwitchPowerService::setTarget(
 *     const HActionInputArguments& inArgs,
 *     HActionOutputArguments* outArgs)
 * {
 *     const HActionInputArgument* iarg = inArgs["newTargetValue"];
 *     if (!iarg)
 *     {
 *         return HAction::InvalidArgs();
 *     }
 *
 *     bool newTargetValue = iarg->value().toBool();
 *     setStateVariableValue("Target", newTargetValue);
 *
 *     //
 *     // Do here whatever that is required to turn on / off the light
 *     //
 *
 *     //
 *     // If it succeeded, we better modify the Status state variable to reflect
 *     // the new state of the light.
 *     //
 *
 *     setStateVariableValue("Status", newTargetValue);
 *
 *     return HAction::Success();
 * }
 *
 * qint32 MySwitchPowerService::getTarget(
 *     const HActionInputArguments& inArgs,
 *     HActionOutputArguments* outArgs)
 * {
 *     bool b = stateVariableByName("Target")->value().toBool();
 *     (*outArgs)["RetTargetValue"]->setValue(b);
 *
 *     return HAction::Success();
 * }
 *
 * qint32 MySwitchPowerService::getStatus(
 *     const HActionInputArguments& inArgs,
 *     HActionOutputArguments* outArgs)
 * {
 *     bool b = stateVariableByName("Status")->value().toBool();
 *     (*outArgs)["ResultStatus"]->setValue(b);
 *
 *     return HAction::Success();
 * }
 *
 * HService::HActionMapT MySwitchPowerService::createActions()
 * {
 *     HActionMapT retVal;
 *
 *     retVal["SetTarget"] =
 *         HActionInvoke(h, &MySwitchPowerService::setTarget);
 *
 *     retVal["GetTarget"] =
 *         HActionInvoke(h, &MySwitchPowerService::getTarget);
 *
 *     retVal["GetStatus"] =
 *         HActionInvoke(h, &MySwitchPowerService::getStatus);
 *
 *     return retVal;
 * }
 *
 * \endcode
 *
 * And there you have it. Fully standard-compliant implementation
 * of BinaryLight:1. One final note however. Before implementing your own device
 * and service types directly from Herqq::Upnp::HDevice and Herqq::Upnp::HService,
 * you should always check if HUPnP provides more refined classes to suit your
 * requirements. For instance, HUPnP provides a base class
 * Herqq::Upnp::Lighting::HAbstractSwitchPower for simplifying the implementation
 * and use of \b SwitchPower:1.
 *
 * To publish your \c HDevice in the network for UPnP control points to discover,
 * see Herqq::Upnp::HDeviceHost.
 *
 * If, on the other hand, you wish to use the created \c HDevice type when a
 * control point discovers a device on the network matching the \em device \em type
 * of the created device, see Herqq::Upnp::HControlPoint.
 */

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HDeviceStatus
 ******************************************************************************/
HDeviceStatus::HDeviceStatus() :
    m_bootId(0), m_configId(0), m_searchPort(0)
{
}

HDeviceStatus::~HDeviceStatus()
{
}

qint32 HDeviceStatus::bootId() const
{
    return m_bootId;
}

qint32 HDeviceStatus::configId() const
{
    return m_configId;
}

quint32 HDeviceStatus::searchPort() const
{
    return m_searchPort;
}

/*******************************************************************************
 * HDeviceController
 ******************************************************************************/
HDeviceController::HDeviceController(
    HDevice* device, qint32 deviceTimeoutInSecs, QObject* parent) :
        QObject(parent),
            m_statusNotifier(new QTimer(this)),
            m_deviceStatus(new HDeviceStatus()),
            m_device(device, &QObject::deleteLater)
{
    HLOG(H_AT, H_FUN);

    m_statusNotifier->setInterval(deviceTimeoutInSecs * 1000);
    bool ok = connect(
        m_statusNotifier.data(), SIGNAL(timeout()), this, SLOT(timeout_()));

    Q_ASSERT(ok); Q_UNUSED(ok)
}

HDeviceController::~HDeviceController()
{
    HLOG(H_AT, H_FUN);
    dispose();
}

void HDeviceController::timeout_()
{
    HLOG(H_AT, H_FUN);

    m_timedout = true;
    stopStatusNotifier(HDeviceController::ThisOnly);

    emit statusTimeout(this);
}

void HDeviceController::startStatusNotifier(SearchCriteria searchCriteria)
{
    HLOG(H_AT, H_FUN);

    if (m_device->h_ptr->m_disposed)
    {
        Q_ASSERT_X(false, "", "The object is disposed");
        return;
    }

    m_statusNotifier->start();
    if (searchCriteria & Services)
    {
        /*foreach(HServiceController* sc, services())
        {
            // TODO
            // sc->startStatusNotifier();
        }*/
    }
    if (searchCriteria & EmbeddedDevices)
    {
        foreach(HDeviceController* dc, m_device->h_ptr->m_embeddedDevices)
        {
            dc->startStatusNotifier(searchCriteria);
        }
    }

    m_timedout = false;
}

void HDeviceController::stopStatusNotifier(SearchCriteria searchCriteria)
{
    HLOG(H_AT, H_FUN);

    if (m_device->h_ptr->m_disposed)
    {
        Q_ASSERT_X(false, "", "The object is disposed");
        return;
    }

    m_statusNotifier->stop();
    if (searchCriteria & Services)
    {
        /*foreach(HServiceController* sc, services())
        {
            // TODO
            // sc->stopStatusNotifier();
        }*/
    }
    if (searchCriteria & EmbeddedDevices)
    {
        foreach(HDeviceController* dc, m_device->h_ptr->m_embeddedDevices)
        {
            dc->stopStatusNotifier(searchCriteria);
        }
    }
}

QList<HServiceController*> HDeviceController::services() const
{
    HLOG(H_AT, H_FUN);

    if (m_device->h_ptr->m_disposed)
    {
        Q_ASSERT_X(false, "", "The object is disposed");
        return QList<HServiceController*>();
    }

    return m_device->h_ptr->m_services;
}

QList<HDeviceController*> HDeviceController::embeddedDevices() const
{
    HLOG(H_AT, H_FUN);

    if (m_device->h_ptr->m_disposed)
    {
        Q_ASSERT_X(false, "", "The object is disposed");
        return QList<HDeviceController*>();
    }

    return m_device->h_ptr->m_embeddedDevices;
}

HDeviceController* HDeviceController::parentDevice() const
{
    HLOG(H_AT, H_FUN);

    if (m_device->h_ptr->m_disposed)
    {
        Q_ASSERT_X(false, "", "The object is disposed");
        return 0;
    }

    return m_device->h_ptr->m_parent;
}

HDeviceController* HDeviceController::rootDevice()
{
    HLOG(H_AT, H_FUN);

    if (m_device->h_ptr->m_disposed)
    {
        Q_ASSERT_X(false, "", "The object is disposed");
        return 0;
    }

    HDeviceController* root = this;
    while (root->parentDevice()) { root = root->parentDevice(); }
    return root;
}

HDeviceStatus* HDeviceController::deviceStatus() const
{
    HLOG(H_AT, H_FUN);

    if (m_device->h_ptr->m_disposed)
    {
        Q_ASSERT_X(false, "", "The object is disposed");
        return 0;
    }

    return m_deviceStatus.data();
}

qint32 HDeviceController::deviceTimeoutInSecs() const
{
    HLOG(H_AT, H_FUN);

    if (m_device->h_ptr->m_disposed)
    {
        Q_ASSERT_X(false, "", "The object is disposed");
        return -1;
    }

    return m_statusNotifier->interval() / 1000;
}

bool HDeviceController::isTimedout(SearchCriteria searchCriteria) const
{
    HLOG(H_AT, H_FUN);

    if (m_device->h_ptr->m_disposed)
    {
        Q_ASSERT_X(false, "", "The object is disposed");
        return true;
    }

    if (m_timedout)
    {
        return true;
    }

    if (searchCriteria & Services)
    {
        /*foreach(HServiceController* sc, services())
        {
            // TODO
        }*/
    }

    if (searchCriteria & EmbeddedDevices)
    {
        foreach(HDeviceController* dc, m_device->h_ptr->m_embeddedDevices)
        {
            if (dc->isTimedout(searchCriteria))
            {
                return true;
            }
        }
    }

    return false;
}

void HDeviceController::dispose()
{
    HLOG(H_AT, H_FUN);

    if (m_device->h_ptr->m_disposed.testAndSetAcquire(0, 1))
    {
        emit m_device->disposed();
        return;
    }

    m_statusNotifier->stop();

    foreach(HDeviceController* embeddedDevice, m_device->h_ptr->m_embeddedDevices)
    {
        embeddedDevice->dispose();
    }
}

namespace
{
bool shouldAdd(
    const HDevice* device, const QUrl& location)
{
    HLOG(H_AT, H_FUN);
    Q_ASSERT(!device->parentDevice());

    QList<QUrl> locations = device->locations();
    QList<QUrl>::const_iterator ci = locations.constBegin();

    for(; ci != locations.constEnd(); ++ci)
    {
        if ((*ci).host() == location.host())
        {
            return false;
        }
    }

    return true;
}
}

void HDeviceController::addLocation(const QUrl& location)
{
    HLOG(H_AT, H_FUN);

    Q_ASSERT(!m_device->parentDevice());
    // embedded devices always query the parent device for locations

    if (m_device->h_ptr->m_disposed)
    {
        Q_ASSERT_X(false, "", "The object is disposed");
        return;
    }

    QMutexLocker lock(&m_device->h_ptr->m_locationsMutex);
    if (shouldAdd(m_device.data(), location))
    {
        m_device->h_ptr->m_locations.push_back(location);
    }
    lock.unlock();
}

void HDeviceController::addLocations(const QList<QUrl>& locations)
{
    HLOG(H_AT, H_FUN);

    QMutexLocker lock(&m_device->h_ptr->m_locationsMutex);
    QList<QUrl>::const_iterator ci = locations.constBegin();
    for(; ci != locations.constEnd(); ++ci)
    {
        addLocation(*ci);
    }
}

/*******************************************************************************
 * HDevicePrivate
 ******************************************************************************/
HDevicePrivate::HDevicePrivate() :
    m_upnpDeviceInfo(0), m_embeddedDevices(), m_services(), m_parent(0),
    q_ptr(0), m_locations(), m_deviceDescription(), m_disposed(0),
    m_locationsMutex(QMutex::Recursive)
{
}

HDevicePrivate::~HDevicePrivate()
{
    HLOG(H_AT, H_FUN);

    // all device controllers & devices and service controllers & services
    // are organized in qobject trees, which ensures that every qobject parent
    // will delete its children when it is being deleted.
    // because of this, there is no need to explicitly delete embedded device
    // controllers or services in here.
}

/*******************************************************************************
 * HDevice
 ******************************************************************************/
HDevice::HDevice(QObject* parent) :
    QObject(parent), h_ptr(new HDevicePrivate())
{
}

HDevice::HDevice(HDevicePrivate& dd, QObject* parent) :
    QObject(parent), h_ptr(&dd)
{
}

HDevice::~HDevice()
{
    HLOG(H_AT, H_FUN);

    delete h_ptr;
}

const HDevice* HDevice::parentDevice() const
{
    HLOG(H_AT, H_FUN);

    return !isDisposed() && h_ptr->m_parent ? h_ptr->m_parent->m_device.data() : 0;
}

QString HDevice::deviceDescription() const
{
    HLOG(H_AT, H_FUN);

    return h_ptr->m_deviceDescription.toString();
}

HDeviceInfo HDevice::deviceInfo() const
{
    HLOG(H_AT, H_FUN);
    return *h_ptr->m_upnpDeviceInfo;
}

HDevicePtrListT HDevice::embeddedDevices() const
{
    HLOG(H_AT, H_FUN);

    if (isDisposed())
    {
        return HDevicePtrListT();
    }

    HDevicePtrListT retVal;
    foreach(HDeviceController* dc, h_ptr->m_embeddedDevices)
    {
        retVal.push_back(dc->m_device.data());
    }

    return retVal;
}

HServicePtrListT HDevice::services() const
{
    HLOG(H_AT, H_FUN);

    if (isDisposed())
    {
        return HServicePtrListT();
    }

    HServicePtrListT retVal;
    foreach(HServiceController* sc, h_ptr->m_services)
    {
        retVal.push_back(sc->m_service);
    }

    return retVal;
}

HService* HDevice::serviceById(const HServiceId& serviceId) const
{
    HLOG(H_AT, H_FUN);

    if (isDisposed())
    {
        return 0;
    }

    foreach(HServiceController* sc, h_ptr->m_services)
    {
        if (sc->m_service->serviceId() == serviceId)
        {
            return sc->m_service;
        }
    }

    return 0;
}

QList<QUrl> HDevice::locations(bool includeDeviceDescriptionPostfix) const
{
    HLOG(H_AT, H_FUN);

    if (isDisposed())
    {
        return QList<QUrl>();
    }
    else if (h_ptr->m_parent)
    {
        // the root device "defines" the locations and they are the same for each
        // embedded device.
        return h_ptr->m_parent->m_device->locations(includeDeviceDescriptionPostfix);
    }

    QMutexLocker lock(&h_ptr->m_locationsMutex);
    if (includeDeviceDescriptionPostfix)
    {
        return h_ptr->m_locations;
    }

    QList<QUrl> retVal;
    foreach(QUrl location, h_ptr->m_locations)
    {
        retVal.push_back(extractBaseUrl(location));
    }

    return retVal;
}

bool HDevice::isDisposed() const
{
    return h_ptr->m_disposed;
}

}
}
