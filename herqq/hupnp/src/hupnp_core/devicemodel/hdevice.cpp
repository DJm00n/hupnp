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

#include "hdevice.h"
#include "hdevice_p.h"
#include "hservice_p.h"

#include "../../utils/hlogger_p.h"
#include "../general/hupnp_global_p.h"

#include "../dataelements/hserviceid.h"
#include "../dataelements/hdeviceinfo.h"

#include <QtCore/QTimer>
#include <QtCore/QString>

/*! \mainpage %Herqq UPnP (HUPnP) Reference Documentation
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
 * gaps you can check the aforementioned UDA specification and other documents
 * available at the <a href="http://www.upnp.org/resources/documents.asp">UPnP Forum</a>.
 *
 * HUPnP is tightly integrated into the <a href="http://qt.nokia.com/">Qt Framework</a> and
 * follows closely the very same design principles and programming practices Qt
 * follows. You will get the most out of HUPnP by using it alongside with Qt.
 * You can use HUPnP from other environments as well, assuming the appropriate
 * Qt headers and libraries are available.
 *
 * \section settingup Setting Up
 *
 * First, it is important to point out that at the moment HUPnP is
 * officially distributed in source code only. If you
 * come across a binary of HUPnP, it is not made by the author of HUPnP.
 * Second, HUPnP uses the
 * <a href="http://qt.nokia.com/products/appdev/add-on-products/catalog/4/Utilities/qtsoap">QtSoap library</a>
 * under the <a href="http://www.gnu.org/licenses/lgpl-2.1.html">LGPLv2.1</a> license.
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
 * The build produces two shared libraries to which you need to link in order to use
 * HUPnP. Currently, static linking is not an option. The created libraries are
 * placed in \c bin directory and they are named \c HUPnP.x and
 * \c QtSolutions_SOAP-2.7.x, where \c ".x" is the platform dependent suffix
 * for shared libraries. In addition, your compiler
 * must be aware of the HUPnP includes, which can be found in the \c include
 * directory. It is very important that you do \b not directly include anything that
 * is not found in the \c include directory. In any case, once your compiler
 * finds the HUPnP includes and your linker finds the HUPnP shared library,
 * you are good to go. \attention You do \b not need to include the QtSoap includes.
 * HUPnP does not expose the types declared in QtSoap.
 *
 * \section importantnotes Important notes
 *
 * Before starting to use HUPnP, there are a few things you have to know.
 *
 * \subsection include \#include
 *
 * HUPnP follows similar include pattern to that of Qt. When you want to use a
 * a class in HUPnP, you have to use \verbatim #include <HClassName> \endverbatim
 * where <c>HClassName</c> matches the name of the class you wish to use exactly.
 *
 * \subsection memorymanagement Memory management
 *
 * Some of the key classes in HUPnP are always instantiated by HUPnP and the
 * instances are always destroyed by HUPnP. You should never delete these.
 * The API documentation of HUPnP is clear about object ownership and
 * these classes are identified in documentation.
 *
 * \subsection herqqheader The <HUpnp> include file
 *
 * %HUpnp introduce a number of types, functions, enums and
 * type definitions under the root namespace \c Herqq. For instance, all the
 * HUPnP core types can be found under the namespace Herqq::Upnp.
 *
 * In several occasions, you do not need to include the full %HUPnP type definitions for your
 * code to work. More specifically, if the compiler doesn't need to see the layout
 * of a %HUPnP type to compile your code, you should only forward-declare such %HUPnP
 * types. In that case, you can include the
 * <c><HUpnp></c> file, which provides forward-declarations to every public
 * %HUPnP type and function.
 *
 * \subsection logging Logging
 *
 * In many situations it is useful to see some log output to have some idea what is
 * going on under the hood, especially when something appears
 * to be malfunctioning. You can enable logging in HUPnP by calling the
 * function Herqq::Upnp::SetLoggingLevel() with a desired \e level argument.
 * Include <c><HUpnp></c> to use the Herqq::Upnp::SetLoggingLevel().
 *
 * \subsection deployment Deployment
 *
 * You can run \c make \c install after compiling the project to copy the
 * public headers and created dynamic libraries into \c hupnp/deploy folder. More
 * specifically, \c hupnp/deploy/include will contain all the public headers and
 * \c hupnp/deploy/bin/ will contain the dynamic libraries. This may be useful
 * to you in case you do not  want to include the full HUPnP source tree with
 * your software.
 *
 * \section gettingstarted Getting Started
 *
 * Often the best explanation is demonstration.
 * So without further ado, the following links should get you started.
 *
 * \li \ref builddevice_tutorial shows how to build your own UPnP device using
 * HUPnP.
 * \li The API documentation of Herqq::Upnp::HControlPoint shows how to discover
 * and use UPnP devices on the network.
 * \li The API documentation of Herqq::Upnp::HDeviceHost shows how to host a Herqq::Upnp::HDevice.
 * This is how you setup a UPnP device to be found and used by UPnP control points.
 *
 * For more detailed information, you can check
 *
 * \li \ref hupnp_devicemodel for the details of the HUPnP device model and
 * \li \ref hupnp_devicehosting for the details of hosting a device.
 *
 * From there, the API reference is the way to go.
 */

/*!
 * \defgroup hupnp_devicemodel Device Model
 * \ingroup hupnp_core
 *
 * \brief This page explains the concept of HUPnP Device Model, which is the
 * logical object hierarchy of HUPnP representing the UPnP Device Architecture.
 *
 * \section notesaboutdesign A few notes about the design
 *
 * The main four components of the UPnP device model are
 * <em>a device</em> (Herqq::Upnp::HDevice), <em>a service</em> (Herqq::Upnp::HService),
 * <em>a state variable</em> (Herqq::Upnp::HStateVariable) <em>and an action</em>
 * (Herqq::Upnp::HAction). These four components form a type of a tree in which
 * devices and services are contained by devices, and state variables
 * and actions are contained by services. This is called the <em>device tree</em>.
 * A device tree has a \e root \e device,
 * which is a UPnP device that has no parent, but may contain other UPnP devices.
 * These contained devices are called <em>embedded devices</em>.
 *
 * The purpose of the other classes part of the device model is to support the
 * initialization and use of the four core classes.
 *
 * \subsection The API differences between client and server sides
 *
 * The HUPnP device model is largely the same at the server and client sides.
 * That is, whether you are writing a custom UPnP device or
 * trying to interact with a UPnP device found in the network, you will be
 * interacting with the HUPnP device model similarly. The most notable difference
 * is that whereas at server side you will be interacting with classes derived
 * directly from the main four components (\c HDevice, \c HService, \c HAction and
 * \c HStateVariable), at client side you will be interacting with devices and
 * services derived from Herqq::Upnp::HDeviceProxy and Herqq::Upnp::HServiceProxy
 * respectively.
 *
 * The \c HDeviceProxy is an \c HDevice and the \c HServiceProxy is an \c HService.
 * They both provide some additional methods not found in their base classes, but
 * otherwise their usage is identical. The reason the client side introduces the
 * proxy classes is tied to the very nature of client-server architecture;
 * the server side usually contains the business logic, whereas the client side
 * only invokes it. Put in HUPnP terms, a server side \c HDevice often contains
 * code that is undesired at client-side and vice versa. Furthermore, the model is
 * clearer when the design makes the difference explicit and as such the model
 * clearly separates the different code bases of server-side and client-side.
 *
 * \subsection lifetime_and_ownership The lifetime and ownership of objects
 *
 * Every \c HDevice has the ownership of all of its embedded devices,
 * services, actions and state variables and the ownership is never released.
 * This means that every \c HDevice always manages the memory used by the
 * objects it owns. Hence, the owner of a root \c HDevice ultimately has the ownership
 * of an entire device tree. This is very important to notice;
 * the lifetime of every object contained by the root device depends of the
 * lifetime of the root device. Or in other words, when a root device is deleted, every
 * embedded device, service, state variable and action underneath it are deleted as well.
 * Furthermore, every root \c HDevice is always owned by HUPnP and the
 * ownership is never released. Because of this you should \b never call \c delete
 * to \b any of the components of the device model that is setup by HUPnP.
 * This \b will result in \b undefined \b behavior.
 *
 * \note There are situations where you may want to instruct HUPnP to \c delete
 * an \c HDevice. For instance, when a UPnP device is removed from the network
 * you may want your \c HControlPoint instance to remove the device that is no
 * longer available. This can be done through the \c HControlPoint interface.
 * But note, HUPnP never deletes an \c HDevice without an explicit request from
 * a user.
 *
 * \section usage Usage
 *
 * The device model is <em>location independent</em>, which in essence means that the
 * device model is almost always used the same way. That is, if you have a pointer to
 * any of the core components of the device model, you use the object the same way
 * regardless of whether you got the pointer directly or indirectly from a
 * Herqq::Upnp::HDeviceHost or a Herqq::Upnp::HControlPoint. There is one exception
 * to the rule and it will be discussed in the section concerning the state
 * variables.
 *
 * Basic use is about interacting with already created objects that comprise the device model.
 * To get started you need to initialize either a device host or a control point
 * and retrieve a list of root devices from it. See \ref hupnp_devicehosting for more
 * information about device hosts and control points. Once you have a
 * root device you can interact with any of its
 * embedded devices, services, state variables and actions until:
 * - you explicitly request an \c HDevice be deleted,
 * - shut down the owner of a device tree, such as \c HControlPoint or
 * \c HDeviceHost.
 *
 * See the corresponding classes for more information concerning their use.
 *
 * \note By default, \c HControlPoint keeps the state of the
 * state variables up-to-date. That is, using default configuration
 * an \c HControlPoint automatically subscribes to events the UPnP services expose.
 * In such a case the state of a device tree at control point side reflects
 * the state of the corresponding device tree at device host side as accurately
 * as the device host keeps sending events.
 *
 * However, if you wish to implement and host your own UPnP device, things get
 * more involved. See \ref builddevice_tutorial to get you started on building your
 * own UPnP devices.
 *
 * \section statevariables About State Variables
 *
 * UPnP Device Architecture does not specify a mechanism for changing the value
 * of a state variable from a control point. Certainly the value of a state variable
 * may be changeable, but that and the method how it is done
 * depends of the service type in which the state variable is defined.
 *
 * As described previously, HUPnP uses the same device model everywhere.
 * This means that the same fundamental core classes are used both at the server and
 * client sides. Perhaps the most significant benefit
 * of this is that it provides <em>uniform API</em> regardless of the type of use.
 * In turn, uniform API calls for simplicity and re-usability, since there is only
 * one class structure to be learned and used on both server and client side programming.
 *
 * However, the lack of a standardized method for manipulating the values of
 * state variables means that the client and server-side cannot
 * use an exactly symmetrical API. This is because on server-side you have
 * to have \e read-write access to the state variables, whereas on client-side
 * you have to have \e read-only access to the state variables. In HUPnP
 * this is abstracted to  Herqq::Upnp::HWritableStateVariable and
 * Herqq::Upnp::HReadableStateVariable classes.
 * On device host side the dynamic type of every Herqq::Upnp::HStateVariable
 * is \c HWritableStateVariable and on control point side the type is
 * \c HReadableStateVariable.
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
 * about description documents, see the UDA specification, sections 2.3 and 2.5.
 *
 * \section creatingclasses Creating the necessary HUPnP classes
 *
 * HUPnP requires device and service descriptions to be accompanied by
 * corresponding classes. In our example we have to derive a class
 * from Herqq::Upnp::HDevice for the \b BinaryLight:1 device description and
 * we have to derive a class from Herqq::Upnp::HService for the
 * \b SwitchPower:1 service description. Note that if your device has no
 * services you do not need to create your own \c HDevice type. Similarly, if your
 * service has no actions you do not need to create your own \c HService type.
 *
 * To create a concrete class from Herqq::Upnp::HDevice that exposes services
 * you have to override its protected method Herqq::Upnp::HDevice::createServices().
 * As the name implies, the purpose of this method is to create \c HService objects
 * at run-time that reflect the services defined in the device description.
 *
 * To create a concrete class from Herqq::Upnp::HService that exposes actions
 * you have to override its protected method Herqq::Upnp::HService::createActions().
 * The purpose of this method is to create <em>callable entities</em> that will
 * be called when the corresponding UPnP actions are invoked. Note, the
 * UPnP actions of a particular UPnP service are defined
 * in the service's description file and your service implementation has to
 * implement all of them. As a side note, these callable entities
 * are used internally by HUPnP. HUPnP does not expose them directly
 * in the public API.
 *
 * To continue with the example we have to create two classes, one for the
 * \b BinaryLight:1 and one for the \b SwitchPowerService:1. For this example
 * the class declarations are put into the same header file, although in real
 * code you might want to separate them.
 *
 * <c>mybinarylight.h</c>
 *
 * \code
 *
 * #include <HUpnpCore/HDevice>
 * #include <HUpnpCore/HService>
 *
 * class MyBinaryLightDevice :
 *    public Herqq::Upnp::HDevice
 * {
 * protected:
 *
 *     virtual Herqq::Upnp::HServicesSetupData* createServices();
 *
 * public:
 *
 *    MyBinaryLightDevice();
 *    virtual ~MyBinaryLightDevice();
 * };
 *
 * class MySwitchPowerService :
 *    public Herqq::Upnp::HService
 * {
 * protected:
 *
 *     virtual Herqq::Upnp::HActionsSetupData createActions();
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
 * #include <HUpnpCore/HActionsSetupData>
 * #include <HUpnpCore/HServicesSetupData>
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
 * HServicesSetupData* MyBinaryLightDevice::createServices()
 * {
 *     HServicesSetupData* retVal = new HServicesSetupData();
 *
 *     retVal->insert(
 *         HServiceId("urn:schemas-upnp-org:serviceId:SwitchPower"),
 *         HResourceType("urn:schemas-upnp-org:service:SwitchPower:1"),
 *         new MySwitchPowerService());
 *
 *  // This maps the UPnP service type "urn:schemas-upnp-org:service:SwitchPower:1"
 *  // with the standard service ID "urn:schemas-upnp-org:serviceId:SwitchPower"
 *  // defined in the device description file to our custom C++ type.
 *  // You have to map each UPnP service found in your device description file
 *  // to a C++ type derived from Herqq::Upnp::HService.
 *
 *     return retVal;
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
 * HActionsSetupData MySwitchPowerService::createActions()
 * {
 *     HActionsSetupData retVal;
 *     return retVal;
 * }
 *
 * \endcode
 *
 * Those who know UPnP and paid close attention to the above example might have
 * noticed that something was off. Where are the actions?
 *
 * According to the UPnP Device Architecture (UDA), a service may have zero or
 * more actions. For a device type that has no services or for a
 * device type that has services that in turn have no actions,
 * the above class declarations and definitions are enough.
 *
 * However, the standard \b BinaryLight:1 device type specifies the
 * \b SwitchPower:1 service type that has three actions defined
 * (look back in the service description document).
 * Namely these are \b SetTarget, \b GetTarget and \b GetStatus.
 * To make the example complete the <c>MySwitchPowerService</c> class
 * requires some additional work. Note that next example shows only one way
 * of making the service complete. There are a few other ways,
 * which will be discussed later in depth.
 *
 * The complete declaration for <c>MySwitchPowerService</c>:
 *
 * <c>mybinarylight.h</c>
 *
 * \code
 *
 * #include <HUpnpCore/HService>
 *
 * class MySwitchPowerService :
 *    public Herqq::Upnp::HService
 * {
 * protected:
 *
 *     virtual Herqq::Upnp::HActionsSetupData createActions();
 *
 * public:
 *
 *     MySwitchPowerService();
 *     virtual ~MySwitchPowerService();
 *
 *     qint32 setTarget(
 *         const Herqq::Upnp::HActionArguments& inArgs,
 *         Herqq::Upnp::HActionArguments* outArgs);
 *
 *     qint32 getTarget(
 *         const Herqq::Upnp::HActionArguments& inArgs,
 *         Herqq::Upnp::HActionArguments* outArgs);
 *
 *     qint32 getStatus(
 *         const Herqq::Upnp::HActionArguments& inArgs,
 *         Herqq::Upnp::HActionArguments* outArgs);
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
 * #include <HUpnpCore/HAction>
 * #include <HUpnpCore/HStateVariable>
 * #include <HUpnpCore/HActionArguments>
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
 * HService::HActionsSetupData MySwitchPowerService::createActions()
 * {
 *     HActionsSetupData retVal;
 *
 *     retVal.insert(
 *         "SetTarget",
 *         HActionInvoke(this, &MySwitchPowerService::setTarget));
 *
 *     // The above lines map the MySwitchPowerService::setTarget() method to
 *     // the action that has the name SetTarget. In essence, this mapping instructs
 *     // HUPnP to call this method when the SetTarget action is invoked.
 *     // However, note that HActionInvoke accepts any "callable entity",
 *     // such as a normal function or a functor. Furthermore, if you use a
 *     // method the method does not have to be public.
 *
 *     retVal.insert(
 *         "GetTarget",
 *         HActionInvoke(this, &MySwitchPowerService::getTarget));
 *
 *     retVal.insert(
 *         "GetStatus",
 *         HActionInvoke(this, &MySwitchPowerService::getStatus));
 *
 *     return retVal;
 * }
 *
 * qint32 MySwitchPowerService::setTarget(
 *     const HActionArguments& inArgs, HActionArguments* outArgs)
 * {
 *     const HActionArgument* newTargetValueArg = inArgs.get("newTargetValue");
 *     if (!newTargetValueArg)
 *     {
 *         // If MySwitchPowerService class is not made for direct public use
 *         // this check is redundant, since in that case this method is called only by
 *         // HUPnP and HUPnP always ensures that the action arguments defined in the
 *         // service description are present when an action is invoked.
 *
 *         return HAction::InvalidArgs;
 *     }
 *
 *     bool newTargetValue = newTargetValueArg->value().toBool();
 *     stateVariableByName("Target")->writable()->setValue(newTargetValue);
 *
 *     // The above line modifies the state variable "Target", which reflects the
 *     // "target state" of a light device, i.e. if a user wants to turn off a light, the
 *     // "target state" is the light turned off whether the light can be turned
 *     // off or not.
 *
 *     //
 *     // Do here whatever that is required to turn on / off the light
 *     // (set it to the target state)
 *     //
 *
 *     //
 *     // If it succeeded, we should modify the Status state variable to reflect
 *     // the new state of the light.
 *     //
 *
 *     stateVariableByName("Status")->writable()->setValue(newTargetValue);
 *
 *     return HAction::Success;
 * }
 *
 * qint32 MySwitchPowerService::getTarget(
 *     const HActionArguments& inArgs, HActionArguments* outArgs)
 * {
 *     if (!outArgs)
 *     {
 *         // See the comments in MySwitchPowerService::setTarget why this
 *         // check is here. Basically, this check is redundant if this method
 *         // is called only by HUPnP, as HUPnP ensures proper arguments
 *         // are always provided when an action is invoked.
 *
 *         return HAction::InvalidArgs;
 *     }
 *
 *     HActionArgument* retTargetValue = outArgs->get("RetTargetValue");
 *     if (!retTargetValue)
 *     {
 *         // See the comments above. The same thing applies here as well.
 *         return HAction::InvalidArgs;
 *     }
 *
 *     bool b = stateVariableByName("Target")->value().toBool();
 *     retTargetValue->setValue(b);
 *
 *     return HAction::Success;
 * }
 *
 * qint32 MySwitchPowerService::getStatus(
 *     const HActionArguments& inArgs, HActionArguments* outArgs)
 * {
 *     if (!outArgs)
 *     {
 *         // See the comments in MySwitchPowerService::getTarget();
 *         return HAction::InvalidArgs;
 *     }
 *
 *     HActionArgument* resultStatus = outArgs->get("ResultStatus");
 *     if (!resultStatus)
 *     {
 *         // See the comments above. The same thing applies here as well.
 *         return HAction::InvalidArgs;
 *     }
 *
 *     bool b = stateVariableByName("Status")->value().toBool();
 *     resultStatus->setValue(b);
 *
 *     return HAction::Success;
 * }
 *
 * \endcode
 *
 * \subsection some_notes_about_switchpower_example Some closing notes
 *
 * First of all, you may want to skim the discussion in
 * \ref hupnp_devicemodel and \ref hupnp_devicehosting to fully understand
 * the comments in the example above. That being said, perhaps the
 * most important issues of building a custom UPnP device using HUPnP
 * can be summarized to:
 *
 * - Every device description has to have a corresponding Herqq::Upnp::HDevice
 * and every service description has to have a corresponding Herqq::Upnp::HService.
 * Remember, you need to derive from \c HDevice if your device has one or
 * more services and you need to derice from \c HService if your service has
 * one or more actions. This is the most common scenario.
 *
 * - It is perfectly fine to create custom \c HDevice and \c HService classes
 * just to be hosted in a Herqq::Upnp::HDeviceHost. Such classes exist only to
 * run your code when UPnP control points interact with them over the network.
 * These types of classes are to be used directly only by \c HDeviceHost.
 *
 * - You can create more advanced \c HDevice and \c HService classes perhaps
 * to build a higher level public API or just to provide yourself a nicer interface for
 * doing something. This was the case with \c MySwitchPowerService class, which
 * extended the \c HService interface by providing the possibility of invoking
 * the actions of the service through the \c setTarget(), \c getTarget() and \c getStatus()
 * methods.
 *
 * - HUPnP allows direct (in-process) access to the hosted
 * \c HDevice and \c HService classes, which means you can interact with
 * your classes \b while they are being hosted and possibly used from external
 * processes. Custom \c HDevice and \c HService interfaces may be beneficial
 * in such a case.
 *
 * - The type behind an Herqq::Upnp::HActionInvoke can hold any
 * <em>callable entity</em>, such as a pointer to a normal function,
 * functor or a pointer to a member function.
 *
 * - A public callable entity should always strictly verify the input and
 * respond to illegal input accordingly. A "private" callable entity that
 * is called only by HUPnP can rest assured that HUPnP never passes a null input
 * argument or an argument that has an incorrect name or data type.
 *
 * - Before implementing your own device
 * and service types directly from Herqq::Upnp::HDevice and Herqq::Upnp::HService,
 * you should check if HUPnP provides more refined classes to suit your
 * requirements. For instance, HUPnP provides a base class
 * Herqq::Upnp::Lighting::HAbstractSwitchPower for simplifying the implementation
 * and use of \b SwitchPower:1.
 *
 * In any case, the above example demonstrates a fully standard-compliant implementation
 * of \b BinaryLight:1. The next step is to publish your \c HDevice in the network
 * for UPnP control points to discover. You can find the instructions for that
 * in Herqq::Upnp::HDeviceHost and \ref hupnp_devicehosting.
 */

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HDeviceStatus
 ******************************************************************************/
HDeviceStatus::HDeviceStatus() :
    m_bootId(0), m_configId(0), m_searchPort(0), m_online(true)
{
}

/*******************************************************************************
 * HDeviceController
 ******************************************************************************/
HDeviceController::HDeviceController(
    HDevice* device, qint32 deviceTimeoutInSecs, QObject* parent) :
        QObject(parent),
            m_statusNotifier(new QTimer(this)),
            m_deviceStatus(new HDeviceStatus()),
            m_device(device)
{
    Q_ASSERT(m_device);
    m_device->setParent(this);

    m_statusNotifier->setInterval(deviceTimeoutInSecs * 1000);
    bool ok = connect(
        m_statusNotifier.data(), SIGNAL(timeout()), this, SLOT(timeout_()));

    Q_ASSERT(ok); Q_UNUSED(ok)
}

HDeviceController::~HDeviceController()
{
    HLOG(H_AT, H_FUN);
}

qint32 HDeviceController::deviceTimeoutInSecs() const
{
    return m_statusNotifier->interval() / 1000;
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

bool HDeviceController::isTimedout(SearchCriteria searchCriteria) const
{
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

namespace
{
bool shouldAdd(const HDevice* device, const QUrl& location)
{
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

bool HDeviceController::addLocation(const QUrl& location)
{
    Q_ASSERT(!m_device->parentDevice());
    // embedded devices always query the parent device for locations

    QMutexLocker lock(&m_device->h_ptr->m_locationsMutex);
    if (shouldAdd(m_device, location))
    {
        m_device->h_ptr->m_locations.push_back(location);
        return true;
    }

    return false;
}

void HDeviceController::addLocations(const QList<QUrl>& locations)
{
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
    q_ptr(0), m_locations(), m_deviceDescription(),
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
HDevice::HDevice() :
    QObject(), h_ptr(new HDevicePrivate())
{
}

HDevice::HDevice(HDevicePrivate& dd) :
    QObject(), h_ptr(&dd)
{
}

HDevice::~HDevice()
{
    delete h_ptr;
}

HServicesSetupData* HDevice::createServices()
{
    return 0;
}

HDevicesSetupData* HDevice::createEmbeddedDevices()
{
    return 0;
}

bool HDevice::finalizeInit(QString*)
{
    // intentionally empty
    Q_ASSERT(h_ptr->q_ptr);
    return true;
}

HDevice* HDevice::parentDevice() const
{
    return h_ptr->m_parent ? h_ptr->m_parent->m_device : 0;
}

HDevice* HDevice::rootDevice() const
{
    HDevice* root = const_cast<HDevice*>(this);
    while(root->h_ptr->m_parent)
    {
        root = root->h_ptr->m_parent->m_device;
    }

    return root;
}

HService* HDevice::serviceById(const HServiceId& serviceId) const
{
    foreach(HServiceController* sc, h_ptr->m_services)
    {
        if (sc->m_service->info().serviceId() == serviceId)
        {
            return sc->m_service;
        }
    }

    return 0;
}

HServices HDevice::services() const
{
    HServices retVal;
    foreach(HServiceController* sc, h_ptr->m_services)
    {
        retVal.push_back(sc->m_service);
    }

    return retVal;
}

HServices HDevice::servicesByType(
    const HResourceType& type, HResourceType::VersionMatch versionMatch) const
{
    if (!type.isValid())
    {
        return HServices();
    }

    HServices retVal;
    foreach(HServiceController* sc, h_ptr->m_services)
    {
        if (sc->m_service->info().serviceType().compare(type, versionMatch))
        {
            retVal.push_back(sc->m_service);
        }
    }

    return retVal;
}

HDevices HDevice::embeddedDevices() const
{
    HDevices retVal;
    foreach(HDeviceController* dc, h_ptr->m_embeddedDevices)
    {
        retVal.push_back(dc->m_device);
    }

    return retVal;
}

const HDeviceInfo& HDevice::info() const
{
    return *h_ptr->m_upnpDeviceInfo;
}

QString HDevice::description() const
{
    return h_ptr->m_deviceDescription;
}

QList<QUrl> HDevice::locations(LocationUrlType urlType) const
{
    if (h_ptr->m_parent)
    {
        // the root device "defines" the locations and they are the same for each
        // embedded device.
        return h_ptr->m_parent->m_device->locations(urlType);
    }

    QMutexLocker lock(&h_ptr->m_locationsMutex);

    QList<QUrl> retVal;
    QList<QUrl>::const_iterator ci;
    for(ci = h_ptr->m_locations.begin(); ci != h_ptr->m_locations.end(); ++ci)
    {
        retVal.push_back(urlType == AbsoluteUrl ? *ci : extractBaseUrl(*ci));
    }

    return retVal;
}

}
}
