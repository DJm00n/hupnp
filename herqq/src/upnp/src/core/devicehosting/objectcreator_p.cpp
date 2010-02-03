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

#include "objectcreator_p.h"
#include "defaultdevice.h"
#include "exceptions_p.h"

#include "../dataelements/udn.h"
#include "../dataelements/deviceinfo.h"

#include "../upnp_global_p.h"
#include "../datatype_mappings_p.h"

#include "../devicemodel/device_p.h"
#include "../devicemodel/action_p.h"
#include "../devicemodel/service_p.h"
#include "../devicemodel/actionarguments.h"

#include "../../../../utils/src/logger_p.h"

#include <QImage>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HObjectCreationParameters
 ******************************************************************************/
HObjectCreationParameters::HObjectCreationParameters() :
    m_deviceDescription(), m_deviceLocations(), m_deviceCreator(),
    m_actionInvokeCreator(), m_createDefaultObjects(false),
    m_serviceDescriptionFetcher(), m_deviceTimeoutInSecs(0),
    m_appendUdnToDeviceLocation(false), m_sharedActionInvokers(),
    m_iconFetcher(), m_strictParsing(true), m_stateVariablesAreImmutable(false)
{
}

/*******************************************************************************
 * HObjectCreator
 ******************************************************************************/
HObjectCreator::HObjectCreator(
    const HObjectCreationParameters& creationParameters) :
        m_creationParameters(creationParameters)
{
    Q_ASSERT(creationParameters.m_serviceDescriptionFetcher);
    Q_ASSERT(creationParameters.m_deviceLocations.size() > 0);
    Q_ASSERT(creationParameters.m_sharedActionInvokers);
    Q_ASSERT(creationParameters.m_iconFetcher);
}

void HObjectCreator::init(
    HService* service, const QDomElement& serviceDefinition)
{
    HLOG(H_AT, H_FUN);
    Q_ASSERT(service);
    Q_ASSERT(!serviceDefinition.isNull());

    service->h_ptr->q_ptr = service;

    bool wasDefined = false;

    service->h_ptr->m_serviceId =
        readElementValue("serviceId"  , serviceDefinition, &wasDefined);

    if (!wasDefined)
    {
        throw HParseException(QObject::tr(
            "Missing mandatory <serviceId> element: %1").arg(
                toString(serviceDefinition)));
    }

    if (!service->h_ptr->m_serviceId.isValid())
    {
        throw HParseException(QObject::tr(
            "The service ID is invalid: %1").arg(toString(serviceDefinition)));
    }

    service->h_ptr->m_serviceType =
        readElementValue("serviceType", serviceDefinition, &wasDefined);

    if (!wasDefined)
    {
        throw HParseException(QObject::tr(
            "Missing mandatory <serviceType> element: %1").arg(
                toString(serviceDefinition)));
    }

    if (!service->h_ptr->m_serviceType.isValid())
    {
        throw HParseException(QObject::tr(
            "The service type is invalid: %1").arg(toString(serviceDefinition)));
    }

    QUrl tmp = readElementValue("SCPDURL", serviceDefinition, &wasDefined);
    if (!wasDefined)
    {
        throw HParseException(QObject::tr(
            "Missing mandatory <SCPDURL> element: %1").arg(
                toString(serviceDefinition)));
    }

    //if (!tmp.startsWith('/')) { tmp.insert(0, '/'); }
    if (tmp.isEmpty() || !tmp.isValid())
    {
        throw HParseException(QObject::tr(
            "The SCPDURL is invalid: %1").arg(toString(serviceDefinition)));
    }

    service->h_ptr->m_scpdUrl = tmp;

    tmp = readElementValue("controlURL" , serviceDefinition, &wasDefined);
    if (!wasDefined)
    {
        throw HParseException(QObject::tr(
            "Missing mandatory <controlURL> element: %1").arg(
                toString(serviceDefinition)));
    }

    //if (!tmp.startsWith('/')) { tmp.insert(0, '/'); }
    if (tmp.isEmpty() || !tmp.isValid())
    {
        throw HParseException(QObject::tr(
            "The controlURL is invalid: %1").arg(toString(serviceDefinition)));
    }
    service->h_ptr->m_controlUrl = tmp;

    tmp = readElementValue("eventSubURL", serviceDefinition, &wasDefined);
    if (!wasDefined)
    {
        throw HParseException(QObject::tr(
            "Missing mandatory <eventSubURL> element: %1").arg(
                toString(serviceDefinition)));
    }

    //if (!tmp.startsWith('/')) { tmp.insert(0, '/'); }
    if (tmp.isEmpty() || !tmp.isValid())
    {
        throw HParseException(QObject::tr(
            "The eventSubURL is invalid: %1").arg(toString(serviceDefinition)));
    }
    service->h_ptr->m_eventSubUrl = tmp;

    service->h_ptr->m_serviceDescriptor =
        m_creationParameters.m_serviceDescriptionFetcher(
            extractBaseUrl(m_creationParameters.m_deviceLocations[0]),
            service->h_ptr->m_scpdUrl); // TODO

    parseServiceDescription(service);
}

void HObjectCreator::parseServiceDescription(HService* service)
{
    HLOG(H_AT, H_FUN);
    Q_ASSERT(service);

    QDomDocument tmp(service->h_ptr->m_serviceDescriptor);
    //QDomNodeList scpdElementNodeList =
        //tmp.elementsByTagNameNS("urn:schemas-upnp-org:service-1-0","scpd");

    QDomElement scpdElement = tmp.firstChildElement("scpd");
    if (scpdElement.isNull())
    {
        throw HParseException(
            QObject::tr("Invalid service description: missing <scpd> element"));
    }

    verifySpecVersion(scpdElement);

    QDomElement serviceStateTableElement =
        scpdElement.firstChildElement("serviceStateTable");

    if (serviceStateTableElement.isNull())
    {
       throw HParseException(QObject::tr(
           "Service [%1] is missing mandatory <serviceStateTable> element.").arg(
                service->serviceId().toString()));
    }

    QDomElement stateVariableElement =
        serviceStateTableElement.firstChildElement("stateVariable");

    if (stateVariableElement.isNull())
    {
       throw HParseException(QObject::tr(
           "Service [%1] does not have a single <stateVariable>. Each service MUST "
           "have at least 1 state variable").arg(service->serviceId().toString()));
    }

    while(!stateVariableElement.isNull())
    {
        HStateVariableController* stateVariable =
            parseStateVariable(stateVariableElement);

        Q_ASSERT(stateVariable);

        stateVariable->m_stateVariable->setParentService(service);

        service->h_ptr->addStateVariable(stateVariable);

        bool ok = QObject::connect(
            stateVariable->m_stateVariable,
            SIGNAL(valueChanged(const Herqq::Upnp::HStateVariableEvent&)),
            service,
            SLOT(notifyListeners()),
            Qt::DirectConnection);

        Q_ASSERT(ok); Q_UNUSED(ok)

        stateVariableElement =
            stateVariableElement.nextSiblingElement("stateVariable");
    }

    QDomElement actionListElement = scpdElement.firstChildElement("actionList");

    if (actionListElement.isNull())
    {
        return;
    }

    QDomElement actionElement = actionListElement.firstChildElement("action");

    if (actionElement.isNull())
    {
       throw HParseException(QObject::tr(
           "Service [%1] has <actionList> element that has no <action> elements."
           "If your service has no actions, do NOT define <actionList>.").arg(
               service->serviceId().toString()));
    }

    HService::HActionMapT actions = service->createActions();

    while(!actionElement.isNull())
    {
        HAction* action = parseAction(service, actionElement, actions);
        service->h_ptr->m_actions.push_back(action);
        service->h_ptr->m_actionsAsMap[action->name()] = action;

        actionElement = actionElement.nextSiblingElement("action");
    }
}

HStateVariableController* HObjectCreator::parseStateVariable(
    const QDomElement& stateVariableElement)
{
    HLOG(H_AT, H_FUN);

    QString strSendEvents = stateVariableElement.attribute("sendEvents", "no");
    bool bSendEvents      = false;
    if (strSendEvents.compare("yes", Qt::CaseInsensitive) == 0)
    {
        bSendEvents = true;
    }
    else if (strSendEvents.compare("no", Qt::CaseInsensitive) != 0)
    {
        throw HParseException(QObject::tr(
            "Invalid value for [sendEvents] attribute: %1.").arg(
                toString(stateVariableElement)));
    }

    QString strMulticast  = stateVariableElement.attribute("multicast", "no");
    bool bMulticast       = false;
    if (strMulticast.compare("yes", Qt::CaseInsensitive) == 0)
    {
        bMulticast = true;
    }
    else if (strMulticast.compare("no", Qt::CaseInsensitive) != 0)
    {
        throw HParseException(QObject::tr(
            "Invalid value for [multicast]: %1.").arg(
                toString(stateVariableElement)));
    }

    HStateVariable::EventingType evType = HStateVariable::NoEvents;
    if (bSendEvents)
    {
        evType = bMulticast ?
            HStateVariable::UnicastAndMulticast : HStateVariable::UnicastOnly;
    }

    QString name     = readElementValue("name", stateVariableElement);
    QString dataType = readElementValue("dataType", stateVariableElement);

    bool wasDefined = false;
    QString defaultValue =
         readElementValue("defaultValue", stateVariableElement, &wasDefined);

    try
    {
        if (dataType.compare(HUpnpDataTypes::string_str()) == 0)
        {
            QStringList allowedValues;

            QDomElement allowedValueListElement =
                stateVariableElement.firstChildElement("allowedValueList");

            if (!allowedValueListElement.isNull())
            {
                QDomElement allowedValueElement =
                    allowedValueListElement.firstChildElement("allowedValue");

                while(!allowedValueElement.isNull())
                {
                    allowedValues.push_back(allowedValueElement.text());

                    allowedValueElement =
                        allowedValueElement.nextSiblingElement("allowedValue");
                }
            }

            return new HStateVariableController(
                new HStateVariable(
                    name, wasDefined ? defaultValue : QVariant(),
                    allowedValues, evType));
        }

        HUpnpDataTypes::DataType dataTypeEnumValue =
            HUpnpDataTypes::dataType(dataType);

        if (HUpnpDataTypes::isNumeric(dataTypeEnumValue))
        {
            QDomElement allowedValueRangeElement =
                stateVariableElement.firstChildElement("allowedValueRange");

            if (!allowedValueRangeElement.isNull())
            {
                QString minimumStr =
                    readElementValue("minimum", allowedValueRangeElement);

               QString maximumStr =
                    readElementValue("maximum", allowedValueRangeElement);

               QString stepStr =
                   readElementValue("step", allowedValueRangeElement);

               if (stepStr.isEmpty())
               {
                   if (HUpnpDataTypes::isRational(dataTypeEnumValue))
                   {
                       bool ok = false;
                       double maxTmp = maximumStr.toDouble(&ok);
                       if (ok && maxTmp < 1)
                       {
                           stepStr = QString::number(maxTmp / 10);
                       }
                       else
                       {
                           stepStr = "1.0";
                       }
                   }
                   else
                   {
                       stepStr = "1";
                   }
               }

               return new HStateVariableController(
                   new HStateVariable(
                       name, dataTypeEnumValue,
                       wasDefined ?
                           convertToRightVariantType(defaultValue, dataTypeEnumValue) :
                           QVariant(),
                       minimumStr, maximumStr, stepStr, evType));
            }
        }

        return new HStateVariableController(
            new HStateVariable(
                name, dataTypeEnumValue,
                wasDefined ?
                     convertToRightVariantType(defaultValue, dataTypeEnumValue) :
                     QVariant(),
                 evType));

    }
    catch(HException& ex)
    {
        throw HParseException(
            QObject::tr("Failed to parse stateVariable [%1]: %2").arg(
                name, ex.reason()));
    }
}

HAction* HObjectCreator::parseAction(
    HService* parentService, const QDomElement& actionElement,
    const HService::HActionMapT& definedActions)
{
    HLOG(H_AT, H_FUN);

    QString name = readElementValue("name", actionElement);

    HAction* action = new HAction(name, parentService);
    // throws if creation fails

    QDomElement argumentListElement  =
        actionElement.firstChildElement("argumentList");

    QList<HActionInputArgument> inputArguments;
    QList<HActionOutputArgument> outputArguments;
    bool hasRetvalArgument = false;

    if (!argumentListElement.isNull())
    {
        QDomElement argumentElement =
            argumentListElement.firstChildElement("argument");

        bool firstOutArgFound = false;

        while(!argumentElement.isNull())
        {
            QString name   =
                readElementValue("name", argumentElement);

            QString dirStr =
                readElementValue("direction", argumentElement);

            bool retValWasDefined = false;
            readElementValue("retval", argumentElement, &retValWasDefined);

            QString relatedStateVar =
                readElementValue("relatedStateVariable", argumentElement);

            if (!parentService->h_ptr->m_stateVariables.contains(relatedStateVar))
            {
                QString err(QObject::tr("No state variable named "));
                err.append(relatedStateVar);

                throw HParseException(err);
            }

            if (dirStr.compare("out", Qt::CaseInsensitive) == 0)
            {
                if (retValWasDefined)
                {
                    if (firstOutArgFound)
                    {
                        throw HParseException(QObject::tr(
                            "[retval] must be the first [out] argument."));
                    }

                    hasRetvalArgument = true;
                }

                firstOutArgFound = true;

                HActionOutputArgument arg(
                    name, parentService->h_ptr->m_stateVariables[relatedStateVar]->m_stateVariable);

                outputArguments.push_back(arg);
            }
            else if (dirStr.compare("in", Qt::CaseInsensitive) == 0)
            {
                if (firstOutArgFound)
                {
                    throw HParseException(QObject::tr(
                        "Invalid argument order. Input arguments must all come "
                        "before output arguments."));
                }

                HActionInputArgument arg(
                    name, parentService->h_ptr->m_stateVariables[relatedStateVar]->m_stateVariable);

                inputArguments.push_back(arg);
            }
            else
            {
                throw HParseException(QObject::tr("Invalid [direction] value."));
            }

            argumentElement = argumentElement.nextSiblingElement("argument");
        }
    }

    HActionInputArguments inArgs(inputArguments);
    HActionOutputArguments outArgs(outputArguments);

    HActionInvoke actionInvoke =
        m_creationParameters.m_actionInvokeCreator ?
            m_creationParameters.m_actionInvokeCreator(
                parentService, name, inArgs, outArgs) :
            definedActions.value(name);

    try
    {
        action->h_ptr->init(
            inArgs, outArgs, hasRetvalArgument, actionInvoke,
            m_creationParameters.m_sharedActionInvokers->value(
                parentService->parentDevice()->deviceInfo().udn()));
    }
    catch(HException& ex)
    {
        delete action;
        throw HParseException(QObject::tr("Failed to initialize action [%1]: %2").arg(
            name, ex.reason()));
    }

    return action;
}

namespace
{
void validateRootDevice(HDeviceController* device)
{
    HLOG(H_AT, H_FUN);

    class DeviceValidator
    {
    private:

        QSet<QString> eventUrls;
        QSet<QString> controlUrls;
        QSet<QString> scpdUrls;
        QSet<QString> iconUrls;

    public:

        void validateDevice(HDeviceController* device)
        {
            QList<QPair<QUrl, QImage> > icons =
                device->m_device->deviceInfo().icons();

            for (qint32 i = 0; i < icons.size(); ++i)
            {
                QString iconUrl = icons.at(i).first.toString();

                if (iconUrls.contains(iconUrl))
                {
                    throw InvalidDeviceDescription(QObject::tr(
                        "Multiple icons have the same URL [%1] within a device tree. "
                        "Icon URLs MUST be unique within a device tree.").arg(
                            iconUrl));
                }
                else
                {
                    iconUrls.insert(iconUrl);
                }
            }

            QList<HServiceController*> services = device->services();
            for(qint32 i = 0; i < services.size(); ++i)
            {
                HServiceController* service = services[i];

                QString eventUrl = service->m_service->eventSubUrl().toString();
                if (!eventUrl.isEmpty())
                {
                    if (eventUrls.contains(eventUrl))
                    {
                        throw InvalidDeviceDescription(QObject::tr(
                            "EventSubUrl [%1] encountered more than once."
                            "EventSubUrls MUST be unique within a device tree.").arg(eventUrl));
                    }
                    else
                    {
                        eventUrls.insert(eventUrl);
                    }
                }

                QString scpdUrl = service->m_service->scpdUrl().toString();
                if (scpdUrls.contains(scpdUrl))
                {
                    throw InvalidDeviceDescription(QObject::tr(
                        "ScpdUrl [%1] encountered more than once."
                        "ScpdUrls MUST be unique within a device tree.").arg(eventUrl));
                }
                else
                {
                    scpdUrls.insert(eventUrl);
                }

                QString controlUrl = service->m_service->controlUrl().toString();
                if (controlUrls.contains(controlUrl))
                {
                    throw InvalidDeviceDescription(QObject::tr(
                        "ControlUrl [%1] encountered more than once. "
                        "ControlUrls MUST be unique within a device tree.").arg(eventUrl));
                }
                else
                {
                    controlUrls.insert(eventUrl);
                }
            }

            QList<HDeviceController*> devices = device->embeddedDevices();
            for(qint32 i = 0; i < devices.size(); ++i)
            {
                validateDevice(devices[i]);
            }
        }
    };

    DeviceValidator validator;
    validator.validateDevice(device);
}
}

QList<QPair<QUrl, QImage> > HObjectCreator::parseIconList(
    const QDomElement& iconListElement)
{
    HLOG(H_AT, H_FUN);

    QList<QPair<QUrl, QImage> > retVal;

    QDomElement iconElement = iconListElement.firstChildElement("icon");
    while(!iconElement.isNull())
    {
        QUrl iconUrl = readElementValue("url", iconElement);

        try
        {
            QImage icon = m_creationParameters.m_iconFetcher(
                extractBaseUrl(m_creationParameters.m_deviceLocations[0]),
                iconUrl); // TODO

            if (icon.isNull())
            {
                if (m_creationParameters.m_strictParsing)
                {
                    throw HParseException(
                        QObject::tr("Could not create icon from [%1]").arg(
                            iconUrl.toString()));
                }
                else
                {
                    HLOG_WARN(QObject::tr(
                        "Failed to create an icon [%1] specified in the device description. "
                        "Ignoring, since strict parsing is not enabled.").arg(iconUrl.toString()));
                }
            }
            else
            {
                QString iconUrlAsStr = iconUrl.toString();
                //if (!iconUrlAsStr.startsWith('/')) { iconUrlAsStr.insert(0, '/'); }

                retVal.append(qMakePair(QUrl(iconUrlAsStr), icon));
            }
        }
        catch(HException& ex)
        {
            if (m_creationParameters.m_strictParsing)
            {
                throw;
            }

            HLOG_WARN(QObject::tr(
                "Failed to create an icon [%1] specified in the device description."
                "Ignoring, since strict parsing is not enabled.").arg(iconUrl.toString()));
        }

        iconElement = iconElement.nextSiblingElement("icon");
    }

    return retVal;
}

HDeviceInfo* HObjectCreator::parseDeviceInfo(const QDomElement& deviceElement)
{
    HLOG(H_AT, H_FUN);

    QString deviceType       =
        readElementValue("deviceType"      , deviceElement);

    QString friendlyName     =
        readElementValue("friendlyName"    , deviceElement);

    QString manufacturer     =
        readElementValue("manufacturer"    , deviceElement);

    QString manufacturerURL  =
        readElementValue("manufacturerURL" , deviceElement);

    QString modelDescription =
        readElementValue("modelDescription", deviceElement);

    QString modelName        =
        readElementValue("modelName"       , deviceElement);

    QString modelNumber      =
        readElementValue("modelNumber"     , deviceElement);

    QUrl    modelUrl         =
        readElementValue("modelURL"        , deviceElement);

    QString serialNumber     =
        readElementValue("serialNumber"    , deviceElement);

    HUdn udn(readElementValue("UDN"        , deviceElement));

    QString upc              =
        readElementValue("UPC"             , deviceElement);

    QDomElement iconListElement = deviceElement.firstChildElement("iconList");
    QList<QPair<QUrl, QImage> > icons;
    if (!iconListElement.isNull())
    {
        icons = parseIconList(iconListElement);
    }

    bool wasDefined = false;

    QString tmp =
        readElementValue("presentationURL", deviceElement, &wasDefined);

    if (m_creationParameters.m_strictParsing && wasDefined && tmp.isEmpty())
    {
        throw InvalidDeviceDescription(QObject::tr(
            "Presentation URL has to be defined, if the corresponding element is used."));
    }

    QUrl presentationUrl(tmp);

    return new HDeviceInfo(
            HResourceType(deviceType), friendlyName, manufacturer, manufacturerURL,
            modelDescription, modelName, modelNumber, modelUrl, serialNumber,
            udn, upc, icons, presentationUrl);
}

QList<HServiceController*> HObjectCreator::parseServiceList(
    const QDomElement& serviceListElement, HDevice* device)
{
    HLOG(H_AT, H_FUN);

    Q_ASSERT(device);
    Q_ASSERT(!serviceListElement.isNull());

    HDevice::HServiceMapT services = device->createServices();

    QList<HServiceController*> retVal;

    try
    {
        QDomElement serviceElement = serviceListElement.firstChildElement("service");
        while(!serviceElement.isNull())
        {
            HServiceId serviceId   =
                readElementValue("serviceId", serviceElement);

            HResourceType serviceType =
                readElementValue("serviceType", serviceElement);

            if (!serviceId.isValid())
            {
                throw InvalidServiceDescription(
                    QObject::tr("Service ID is invalid: %1.").arg(
                        toString(serviceElement)));
            }
            if (!serviceType.isValid())
            {
                throw InvalidServiceDescription(
                    QObject::tr("Service Type is invalid: %1.").arg(
                        toString(serviceElement)));
            }

            HService* service = services.value(serviceType);

            if (!service)
            {
                if (m_creationParameters.m_createDefaultObjects)
                {
                    service = new HDefaultService();
                }
                else
                {
                    QString err(QObject::tr(
                        "No object created for service of type [%1] with ID %2").arg(
                            serviceType.toString(), serviceId.toString()));

                    throw InvalidServiceDescription(err);
                }
            }

            service->h_ptr->m_parentDevice = device;
            init(service, serviceElement); // may throw

            retVal.push_back(new HServiceController(service));

            serviceElement = serviceElement.nextSiblingElement("service");
        }
    }
    catch(HException& ex)
    {
        qDeleteAll(services);
        throw InvalidServiceDescription(ex.reason());
    }

    return retVal;
}

namespace
{
QList<QUrl> generateLocations(const HUdn& udn, const QList<QUrl>& locations)
{
    HLOG(H_AT, H_FUN);

    QList<QUrl> retVal;

    foreach(QUrl location, locations)
    {
        QString locStr = location.toString();
        if (!locStr.endsWith('/'))
        {
            locStr.append(QString("/%1/%2").arg(
                udn.toSimpleUuid(), HDevicePrivate::deviceDescriptionPostFix()));
        }

        retVal.push_back(locStr);
    }

    return retVal;
}
}

HDeviceController* HObjectCreator::parseDevice(const QDomElement& deviceElement)
{
    HLOG(H_AT, H_FUN);

    QScopedPointer<HDeviceInfo> deviceInfo;
    try
    {
        deviceInfo.reset(parseDeviceInfo(deviceElement));
    }
    catch(HException& ex)
    {
        throw InvalidDeviceDescription(ex.reason());
    }

    HDevice* device = 0;
    if (m_creationParameters.m_deviceCreator)
    {
        device = m_creationParameters.m_deviceCreator(*deviceInfo);
    }

    if (!device)
    {
        if (m_creationParameters.m_createDefaultObjects)
        {
            device = new HDefaultDevice();
        }
        else
        {
            throw HOperationFailedException(QObject::tr(
                "No object created for UPnP device type [%1], with UDN: [%2]").arg(
                    deviceInfo->deviceType().toString(), deviceInfo->udn().toString()));
        }
    }

    device->h_ptr->m_upnpDeviceInfo.swap(deviceInfo);

    m_creationParameters.m_sharedActionInvokers->insert(
        device->deviceInfo().udn(), new HSharedActionInvoker());

    QDomElement serviceListElement = deviceElement.firstChildElement("serviceList");
    if (!serviceListElement.isNull())
    {
        device->h_ptr->m_services = parseServiceList(serviceListElement, device);
    }

    // the device controller takes ownership of the created device. it will become
    // its parent as well.
    HDeviceController* retVal =
        new HDeviceController(
            device, m_creationParameters.m_deviceTimeoutInSecs);

    QDomElement deviceListElement = deviceElement.firstChildElement("deviceList");
    if (!deviceListElement.isNull())
    {
        QList<HDeviceController*> embeddedDevices;

        QDomElement embeddedDeviceElement =
            deviceListElement.firstChildElement("device");

        while(!embeddedDeviceElement.isNull())
        {
            HDeviceController* embeddedDevice =
                parseDevice(embeddedDeviceElement);

            embeddedDevice->setParent(retVal);

            embeddedDevice->m_device->h_ptr->m_parent = retVal;
            embeddedDevice->m_device->setParent(retVal->m_device.data());

            embeddedDevice->m_device->h_ptr->m_deviceDescription =
                m_creationParameters.m_deviceDescription;

            embeddedDevices.push_back(embeddedDevice);

            embeddedDeviceElement =
                embeddedDeviceElement.nextSiblingElement("device");
        }

        device->h_ptr->m_embeddedDevices = embeddedDevices;
    }

    return retVal;
}

HDeviceController* HObjectCreator::createRootDevice()
{
    HLOG(H_AT, H_FUN);

    QDomElement rootElement =
        m_creationParameters.m_deviceDescription.firstChildElement("root");

    // "urn:schemas-upnp-org:device-1-0",

    if (rootElement.isNull())
    {
        throw InvalidDeviceDescription(QObject::tr(
            "Invalid device description: no valid root element defined"));
    }

    try
    {
        verifySpecVersion(rootElement);
    }
    catch(HException& ex)
    {
        throw InvalidDeviceDescription(ex.reason());
    }

    QDomElement rootDeviceElement = rootElement.firstChildElement("device");
    if (rootDeviceElement.isNull())
    {
        throw InvalidDeviceDescription(QObject::tr(
            "The specified file does not contain a valid root device definition"));
    }

    HDeviceController* createdDevice = parseDevice(rootDeviceElement);

    createdDevice->m_configId = readConfigId(rootElement);
    createdDevice->m_device->h_ptr->m_deviceDescription =
        m_creationParameters.m_deviceDescription;

    createdDevice->m_device->h_ptr->m_locations =
        m_creationParameters.m_appendUdnToDeviceLocation ?
            generateLocations(createdDevice->m_device->deviceInfo().udn(),
                              m_creationParameters.m_deviceLocations) :
            m_creationParameters.m_deviceLocations;

    validateRootDevice(createdDevice);

    return createdDevice;
}

}
}
