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

#include "hobjectcreator_p.h"

#include "hdevicehosting_exceptions_p.h"

#include "../dataelements/hudn.h"
#include "../dataelements/hdeviceinfo.h"

#include "../general/hupnp_global_p.h"
#include "../datatypes/hdatatype_mappings_p.h"

#include "../devicemodel/hservice.h"
#include "../devicemodel/hdevice_p.h"
#include "../devicemodel/haction_p.h"
#include "../devicemodel/haction_p.h"
#include "../devicemodel/hservice_p.h"
#include "../devicemodel/hactionarguments.h"
#include "../devicemodel/hactions_setupdata.h"
#include "../devicemodel/hdevices_setupdata.h"
#include "../devicemodel/hservices_setupdata.h"
#include "../devicemodel/hwritable_statevariable.h"
#include "../devicemodel/hreadable_statevariable.h"
#include "../devicemodel/hstatevariables_setupdata.h"

#include "../../utils/hlogger_p.h"

#include <QImage>
#include <QDomElement>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HObjectCreationParameters
 ******************************************************************************/
HObjectCreationParameters::HObjectCreationParameters() :
    m_deviceDescription(),
    m_deviceLocations(),
    m_actionInvokeProxyCreator(),
    m_serviceDescriptionFetcher(),
    m_deviceTimeoutInSecs(0),
    m_appendUdnToDeviceLocation(false),
    m_iconFetcher(),
    m_strictness(StrictChecks),
    m_stateVariablesAreImmutable(false),
    m_threadPool(0),
    m_loggingIdentifier()
{
}

HObjectCreationParameters::~HObjectCreationParameters()
{
}

HObjectCreationParameters* HObjectCreationParameters::clone() const
{
    HObjectCreationParameters* newObj = createType();

    newObj->m_deviceDescription = m_deviceDescription;
    newObj->m_deviceLocations = m_deviceLocations;
    newObj->m_actionInvokeProxyCreator = m_actionInvokeProxyCreator;
    newObj->m_serviceDescriptionFetcher = m_serviceDescriptionFetcher;
    newObj->m_deviceTimeoutInSecs = m_deviceTimeoutInSecs;
    newObj->m_appendUdnToDeviceLocation = m_appendUdnToDeviceLocation;
    newObj->m_iconFetcher = m_iconFetcher;
    newObj->m_strictness = m_strictness;
    newObj->m_stateVariablesAreImmutable = m_stateVariablesAreImmutable;
    newObj->m_threadPool = m_threadPool;
    newObj->m_loggingIdentifier = m_loggingIdentifier;

    return newObj;
}

HDevice* HObjectCreationParameters::createDefaultDevice(
    const HDeviceInfo&)
{
    return 0;
}

HService* HObjectCreationParameters::createDefaultService(
    const HResourceType&)
{
    return 0;
}

/*******************************************************************************
 * HDeviceHostObjectCreationParameters
 ******************************************************************************/
HDeviceHostObjectCreationParameters::HDeviceHostObjectCreationParameters() :
    m_deviceCreator()
{
}

HDeviceHostObjectCreationParameters*
    HDeviceHostObjectCreationParameters::clone() const
{
    HDeviceHostObjectCreationParameters* newObj =
        static_cast<HDeviceHostObjectCreationParameters*>(
            HObjectCreationParameters::clone());

    newObj->m_deviceCreator = m_deviceCreator;

    return newObj;
}

HDeviceHostObjectCreationParameters*
    HDeviceHostObjectCreationParameters::createType() const
{
    return new HDeviceHostObjectCreationParameters();
}

HDevice* HDeviceHostObjectCreationParameters::createDevice(
    const HDeviceInfo& arg)
{
    return m_deviceCreator(arg);
}

HDevice* HDeviceHostObjectCreationParameters::createDefaultDevice(
    const HDeviceInfo&)
{
    return 0;
}

HService* HDeviceHostObjectCreationParameters::createDefaultService(
    const HResourceType&)
{
    return 0;
}

/*******************************************************************************
 * HControlPointObjectCreationParameters
 ******************************************************************************/
HControlPointObjectCreationParameters::HControlPointObjectCreationParameters() :
    m_deviceCreator(), m_defaultDeviceCreator(), m_defaultServiceCreator()
{
}

HControlPointObjectCreationParameters*
    HControlPointObjectCreationParameters::clone() const
{
    HControlPointObjectCreationParameters* newObj =
        static_cast<HControlPointObjectCreationParameters*>(
            HObjectCreationParameters::clone());

    newObj->m_deviceCreator = m_deviceCreator;
    newObj->m_defaultDeviceCreator = m_defaultDeviceCreator;
    newObj->m_defaultServiceCreator = m_defaultServiceCreator;

    return newObj;
}

HControlPointObjectCreationParameters*
    HControlPointObjectCreationParameters::createType() const
{
    return new HControlPointObjectCreationParameters();
}

HDevice* HControlPointObjectCreationParameters::createDevice(
    const HDeviceInfo& arg)
{
    return m_deviceCreator(arg);
}

HDevice* HControlPointObjectCreationParameters::createDefaultDevice(
    const HDeviceInfo& arg)
{
    return m_defaultDeviceCreator(arg);
}

HService* HControlPointObjectCreationParameters::createDefaultService(
    const HResourceType& arg)
{
    return m_defaultServiceCreator(arg);
}

/*******************************************************************************
 * HObjectCreator
 ******************************************************************************/
HObjectCreator::HObjectCreator(
    const HObjectCreationParameters& creationParameters) :
        m_creationParameters(creationParameters.clone())
{
    Q_ASSERT(creationParameters.m_serviceDescriptionFetcher);
    Q_ASSERT(creationParameters.m_deviceLocations.size() > 0);
    Q_ASSERT(creationParameters.m_iconFetcher);
    Q_ASSERT(!creationParameters.m_loggingIdentifier.isEmpty());
}

void HObjectCreator::initService(
    HService* service, const QDomElement& serviceDefinition,
    HServiceSetup* setupInfo)
{
    HLOG2(H_AT, H_FUN, m_creationParameters->m_loggingIdentifier);
    Q_ASSERT(service);
    Q_ASSERT(!serviceDefinition.isNull());

    service->h_ptr->q_ptr = service;

    bool wasDefined = false;

    HServiceId serviceId =
        readElementValue("serviceId", serviceDefinition, &wasDefined);

    if (!wasDefined)
    {
        throw HParseException(QString(
            "Missing mandatory <serviceId> element:\n%1").arg(
                toString(serviceDefinition)));
    }

    service->h_ptr->m_loggingIdentifier =
        m_creationParameters->m_loggingIdentifier.append(
            serviceId.toString()).append(": ");

    HResourceType resourceType =
        readElementValue("serviceType", serviceDefinition, &wasDefined);

    if (!wasDefined)
    {
        throw HParseException(QString(
            "Missing mandatory <serviceType> element:\n%1").arg(
                toString(serviceDefinition)));
    }

    QUrl scpdUrl = readElementValue("SCPDURL", serviceDefinition, &wasDefined);
    if (!wasDefined)
    {
        throw HParseException(QString(
            "Missing mandatory <SCPDURL> element:\n%1").arg(
                toString(serviceDefinition)));
    }

    QUrl controlUrl =
        readElementValue("controlURL" , serviceDefinition, &wasDefined);
    if (!wasDefined)
    {
        throw HParseException(QString(
            "Missing mandatory <controlURL> element:\n%1").arg(
                toString(serviceDefinition)));
    }

    QUrl eventSubUrl =
        readElementValue("eventSubURL", serviceDefinition, &wasDefined);
    if (!wasDefined)
    {
        throw HParseException(QString(
            "Missing mandatory <eventSubURL> element:\n%1").arg(
                toString(serviceDefinition)));
    }

    QString err;
    HServiceInfo serviceInfo(
        serviceId, resourceType, controlUrl, eventSubUrl, scpdUrl,
        setupInfo ?
            setupInfo->inclusionRequirement() : InclusionRequirementUnknown,
        m_creationParameters->m_strictness,
        &err);

    if (!serviceInfo.isValid(m_creationParameters->m_strictness))
    {
        throw HParseException(QString("%1:\n%2").arg(
            err, toString(serviceDefinition)));
    }

    service->h_ptr->m_serviceInfo = serviceInfo;

    service->h_ptr->m_serviceDescription =
        m_creationParameters->m_serviceDescriptionFetcher(
            extractBaseUrl(m_creationParameters->m_deviceLocations[0]), scpdUrl);

    parseServiceDescription(service);
}

void HObjectCreator::parseServiceDescription(HService* service)
{
    HLOG2(H_AT, H_FUN, m_creationParameters->m_loggingIdentifier);
    Q_ASSERT(service);

    qint32 errLine;
    QString errMsg;
    QDomDocument srvDescr;
    if (!srvDescr.setContent(
            service->h_ptr->m_serviceDescription, false, &errMsg, &errLine))
    {
        throw InvalidServiceDescription(QString(
            "Could not parse the service description document: [%1] @ line [%2]:\n[%3]").arg(
                errMsg, QString::number(errLine),
                service->h_ptr->m_serviceDescription));
    }
    //QDomNodeList scpdElementNodeList =
        //tmp.elementsByTagNameNS("urn:schemas-upnp-org:service-1-0","scpd");

    QDomElement scpdElement = srvDescr.firstChildElement("scpd");
    if (scpdElement.isNull())
    {
        throw HParseException(
            "Invalid service description: missing <scpd> element");
    }

    QString err;
    if (!verifySpecVersion(scpdElement, &err))
    {
        throw InvalidServiceDescription(err);
    }

    QDomElement serviceStateTableElement =
        scpdElement.firstChildElement("serviceStateTable");

    if (serviceStateTableElement.isNull())
    {
       throw InvalidServiceDescription(QString(
           "Service [%1] is missing mandatory <serviceStateTable> element.").arg(
                service->info().serviceId().toString()));
    }

    QDomElement stateVariableElement =
        serviceStateTableElement.firstChildElement("stateVariable");

    if (stateVariableElement.isNull())
    {
        QString err = QString(
            "Service [%1] does not have a single <stateVariable> element. "
            "Each service MUST have at least one state variable").arg(
                service->info().serviceId().toString());

        if (m_creationParameters->m_strictness == StrictChecks)
        {
            throw HParseException(err);
        }
        else
        {
            HLOG_WARN_NONSTD(err);
        }
    }

    HStateVariablesSetupData stateVariablesSetup =
        service->stateVariablesSetupData();

    while(!stateVariableElement.isNull())
    {
        HStateVariableController* stateVariable =
            parseStateVariable(service, stateVariableElement, stateVariablesSetup);

        Q_ASSERT(stateVariable);

        service->h_ptr->addStateVariable(stateVariable);

        bool ok = QObject::connect(
            stateVariable->m_stateVariable,
            SIGNAL(valueChanged(const Herqq::Upnp::HStateVariableEvent&)),
            service,
            SLOT(notifyListeners()));

        Q_ASSERT(ok); Q_UNUSED(ok)

        stateVariableElement =
            stateVariableElement.nextSiblingElement("stateVariable");

        stateVariablesSetup.remove(
            stateVariable->m_stateVariable->info().name());
    }

    if (!stateVariablesSetup.isEmpty())
    {
        foreach(const QString& name, stateVariablesSetup.names())
        {
            HStateVariableSetup svSetup = stateVariablesSetup.get(name);
            if (svSetup.inclusionRequirement() == InclusionMandatory &&
                svSetup.version() <= service->info().serviceType().version())
            {
                throw HParseException(QString(
                    "Service description is missing a mandatory state variable "
                    "[%1]").arg(name));
            }
        }
    }

    QDomElement actionListElement = scpdElement.firstChildElement("actionList");

    if (actionListElement.isNull())
    {
        return;
    }

    QDomElement actionElement = actionListElement.firstChildElement("action");

    if (actionElement.isNull())
    {
        QString err = QString(
            "Service [%1] has <actionList> element that has no <action> "
            "elements.").arg(service->info().serviceId().toString());
        if (m_creationParameters->m_strictness == StrictChecks)
        {
            throw HParseException(err);
        }
        else
        {
            HLOG_WARN_NONSTD(err);
        }
    }

    HActionsSetupData actions = service->createActions();

    while(!actionElement.isNull())
    {
        HActionController* action =
            parseAction(service, actionElement, actions);

        if (!action)
        {
            continue;
        }

        QString name = action->m_action->info().name();

        service->h_ptr->m_actions.push_back(action);
        service->h_ptr->m_actionsAsMap[name] = action;

        actions.remove(name);

        actionElement = actionElement.nextSiblingElement("action");
    }

    if (!actions.isEmpty())
    {
        foreach(const QString& name, actions.names())
        {
            HActionSetup setupInfo = actions.get(name);
            if (setupInfo.inclusionRequirement() == InclusionMandatory &&
                setupInfo.version() <= service->info().serviceType().version())
            {
                throw HParseException(QString(
                    "Service description for [%1] is missing a mandatory action "
                    "[%2]").arg(service->info().serviceId().toString(), name));
            }
        }
    }
}

HStateVariableController* HObjectCreator::parseStateVariable(
    HService* parentService,
    const QDomElement& stateVariableElement,
    const HStateVariablesSetupData& stateVariablesSetupData)
{
    HLOG2(H_AT, H_FUN, m_creationParameters->m_loggingIdentifier);

    QString strSendEvents = stateVariableElement.attribute("sendEvents", "no");
    bool bSendEvents      = false;
    if (strSendEvents.compare("yes", Qt::CaseInsensitive) == 0)
    {
        bSendEvents = true;
    }
    else if (strSendEvents.compare("no", Qt::CaseInsensitive) != 0)
    {
        throw HParseException(QString(
            "Invalid value for [sendEvents] attribute:\n%1.").arg(
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
        throw HParseException(QString(
            "Invalid value for [multicast]: %1.").arg(
                toString(stateVariableElement)));
    }

    HStateVariableInfo::EventingType evType = HStateVariableInfo::NoEvents;
    if (bSendEvents)
    {
        evType = bMulticast ?
            HStateVariableInfo::UnicastAndMulticast : HStateVariableInfo::UnicastOnly;
    }

    QString name     = readElementValue("name", stateVariableElement);
    QString dataType = readElementValue("dataType", stateVariableElement);

    bool wasDefined = false;
    QString defaultValue =
         readElementValue("defaultValue", stateVariableElement, &wasDefined);

    HStateVariableSetup setupData = stateVariablesSetupData.get(name);
    if (!setupData.isValid() &&
        stateVariablesSetupData.defaultInclusionPolicy() ==
        HStateVariablesSetupData::Deny)
    {
        throw InvalidServiceDescription(QString(
            "Implementation of service [%1] does not support state variable"
            "[%2]").arg(parentService->info().serviceId().toString(), name));
    }

    QString err;
    QScopedPointer<HStateVariable> stateVar;
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

            stateVar.reset(m_creationParameters->m_stateVariablesAreImmutable ?
                (HStateVariable*) new HReadableStateVariable(parentService) :
                (HStateVariable*) new HWritableStateVariable(parentService));

            HStateVariableInfo info(
                name, wasDefined ? defaultValue : QVariant(),
                allowedValues, evType, setupData.inclusionRequirement(), &err);

            if (!info.isValid())
            {
                throw HParseException(err);
            }

            bool ok = stateVar->init(info);
            Q_ASSERT(ok); Q_UNUSED(ok)
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

                if (minimumStr.isEmpty())
                {
                    QString err = QString("State variable [%1] is missing a "
                        "mandatory <minimum> element within <allowedValueRange>.").arg(name);

                    if (m_creationParameters->m_strictness == StrictChecks)
                    {
                        throw InvalidServiceDescription(err);
                    }
                    else
                    {
                        HLOG_WARN_NONSTD(err);
                        minimumStr = QString::number(INT_MIN);
                    }
                }

                QString maximumStr =
                    readElementValue("maximum", allowedValueRangeElement);

                if (maximumStr.isEmpty())
                {
                    QString err = QString("State variable [%1] is missing a "
                        "mandatory <maximum> element within <allowedValueRange>.").arg(name);

                    if (m_creationParameters->m_strictness == StrictChecks)
                    {
                        throw InvalidServiceDescription(err);
                    }
                    else
                    {
                        HLOG_WARN_NONSTD(err);
                        maximumStr = QString::number(INT_MAX);
                    }
                }

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

                stateVar.reset(m_creationParameters->m_stateVariablesAreImmutable ?
                    (HStateVariable*) new HReadableStateVariable(parentService) :
                    (HStateVariable*) new HWritableStateVariable(parentService));

                HStateVariableInfo info(
                    name,
                    dataTypeEnumValue,
                    wasDefined ?
                        convertToRightVariantType(defaultValue, dataTypeEnumValue) :
                        QVariant(),
                    minimumStr, maximumStr, stepStr,
                    evType,
                    setupData.inclusionRequirement(), &err);

                if (!info.isValid())
                {
                    throw HParseException(err);
                }

                bool ok = stateVar->init(info);
                Q_ASSERT(ok); Q_UNUSED(ok)
            }
        }

        stateVar.reset(m_creationParameters->m_stateVariablesAreImmutable ?
           (HStateVariable*) new HReadableStateVariable(parentService) :
           (HStateVariable*) new HWritableStateVariable(parentService));

        HStateVariableInfo info(
            name,
            dataTypeEnumValue,
            wasDefined ?
                convertToRightVariantType(defaultValue, dataTypeEnumValue) :
                QVariant(),
            evType,
            setupData.inclusionRequirement(),
            &err);

        if (!info.isValid())
        {
            throw HParseException(err);
        }

        bool ok = stateVar->init(info);
        Q_ASSERT(ok); Q_UNUSED(ok)
    }
    catch(HException& ex)
    {
        throw HParseException(
            QString("Failed to parse <stateVariable> [%1]: %2").arg(
                name, ex.reason()));
    }

    return new HStateVariableController(stateVar.take());
}

//
// Returns 0 if the defined actions map does not contain an OPTIONAL
// action encountered in the service description. Otherwise returns a valid
// object or throws an exception.
//
HActionController* HObjectCreator::parseAction(
    HService* parentService, const QDomElement& actionElement,
    const HActionsSetupData& definedActions)
{
    HLOG2(H_AT, H_FUN, m_creationParameters->m_loggingIdentifier);

    QString name = readElementValue("name", actionElement);

    //
    // First, check if the service type identifies the action name
    HActionSetup actionSetup = definedActions.get(name);
    if (!definedActions.isEmpty() && !actionSetup.isValid())
    {
        QString txt = QString(
            "The implementation of service [%1] does not support "
            "the action named [%2].").arg(
                parentService->info().serviceId().toString(), name);

        throw HParseException(txt);
    }

    QDomElement argumentListElement  =
        actionElement.firstChildElement("argumentList");

    bool hasRetvalArgument = false;
    QVector<HActionArgument*> inputArguments;
    QVector<HActionArgument*> outputArguments;
    try
    {
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

                QString relatedSvStr =
                    readElementValue("relatedStateVariable", argumentElement);

                HStateVariableController* relatedSv =
                    parentService->h_ptr->m_stateVariables.value(relatedSvStr);

                if (!relatedSv)
                {
                    throw HParseException(QString(
                        "No state variable named %1").arg(relatedSvStr));
                }

                if (dirStr.compare("out", Qt::CaseInsensitive) == 0)
                {
                    if (retValWasDefined)
                    {
                        if (firstOutArgFound)
                        {
                            throw HParseException(
                                "[retval] must be the first [out] argument.");
                        }

                        hasRetvalArgument = true;
                    }

                    firstOutArgFound = true;

                    HActionArgument* arg = new HActionArgument(
                        name, relatedSv->m_stateVariable->info());

                    outputArguments.push_back(arg);
                }
                else if (dirStr.compare("in", Qt::CaseInsensitive) == 0)
                {
                    if (firstOutArgFound)
                    {
                        throw HParseException(
                            "Invalid argument order. Input arguments must all come "
                            "before output arguments.");
                    }

                    HActionArgument* arg = new HActionArgument(
                        name, relatedSv->m_stateVariable->info());

                    inputArguments.push_back(arg);
                }
                else
                {
                    throw HParseException("Invalid [direction] value.");
                }

                argumentElement = argumentElement.nextSiblingElement("argument");
            }
        }
    }
    catch(HException&)
    {
        qDeleteAll(inputArguments);
        qDeleteAll(outputArguments);
        throw;
    }

    QString err;
    HActionInfo actionInfo(
        name,
        HActionArguments(inputArguments),
        HActionArguments(outputArguments),
        hasRetvalArgument,
        actionSetup.inclusionRequirement(),
        &err);

    if (!actionInfo.isValid())
    {
        throw HParseException(QString(
            "Service [%1]: failed to initialize action [%2]: %3").arg(
                parentService->info().serviceId().toString(), name, err));
    }

    QScopedPointer<HAction> action(new HAction(actionInfo, parentService));
    if (m_creationParameters->m_actionInvokeProxyCreator)
    {
        QScopedPointer<HActionInvokeProxy> proxy(
            m_creationParameters->m_actionInvokeProxyCreator(action.data()));
        if (!proxy.data())
        {
            throw HParseException(
                "Failed to create a client side proxy object for action invocation");
        }
        action->h_ptr->m_actionInvokeProxy = proxy.take();
        action->h_ptr->m_actionInvoker = new HAsyncActionInvoker(action->h_ptr);
    }
    else
    {
        HActionInvoke actionInvoke = actionSetup.actionInvoke();
        if (!actionInvoke)
        {
            throw HParseException(QString(
                "Service [%1]: action [%2] lacks an implementation").arg(
                    parentService->info().serviceId().toString(), name));
        }
        action->h_ptr->m_actionInvoke = new HActionInvoke(actionInvoke);
        action->h_ptr->m_actionInvoker =
            new HSyncActionInvoker(
                action->h_ptr, m_creationParameters->m_threadPool);
    }

    return new HActionController(action.take());
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
                device->m_device->info().icons();

            for (qint32 i = 0; i < icons.size(); ++i)
            {
                QString iconUrl = icons.at(i).first.toString();

                if (iconUrls.contains(iconUrl))
                {
                    throw InvalidDeviceDescription(QString(
                        "Multiple icons have the same URL [%1] within a device tree. "
                        "Icon URLs MUST be unique within a device tree.").arg(
                            iconUrl));
                }
                else
                {
                    iconUrls.insert(iconUrl);
                }
            }

            const QList<HServiceController*>* services = device->services();
            for(qint32 i = 0; i < services->size(); ++i)
            {
                HServiceController* service = (*services)[i];

                QString eventUrl =
                    service->m_service->info().eventSubUrl().toString();

                if (!eventUrl.isEmpty())
                {
                    if (eventUrls.contains(eventUrl))
                    {
                        throw InvalidDeviceDescription(QString(
                            "EventSubUrl [%1] encountered more than once."
                            "EventSubUrls MUST be unique within a device tree.").arg(eventUrl));
                    }
                    else
                    {
                        eventUrls.insert(eventUrl);
                    }
                }

                QString scpdUrl =
                    service->m_service->info().scpdUrl().toString();

                if (scpdUrls.contains(scpdUrl))
                {
                    throw InvalidDeviceDescription(QString(
                        "ScpdUrl [%1] encountered more than once."
                        "ScpdUrls MUST be unique within a device tree.").arg(eventUrl));
                }
                else
                {
                    scpdUrls.insert(eventUrl);
                }

                QString controlUrl =
                    service->m_service->info().controlUrl().toString();

                if (controlUrls.contains(controlUrl))
                {
                    throw InvalidDeviceDescription(QString(
                        "ControlUrl [%1] encountered more than once. "
                        "ControlUrls MUST be unique within a device tree.").arg(eventUrl));
                }
                else
                {
                    controlUrls.insert(eventUrl);
                }
            }

            const QList<HDeviceController*>* devices = device->embeddedDevices();
            for(qint32 i = 0; i < devices->size(); ++i)
            {
                validateDevice((*devices)[i]);
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
    HLOG2(H_AT, H_FUN, m_creationParameters->m_loggingIdentifier);

    QList<QPair<QUrl, QImage> > retVal;

    QDomElement iconElement = iconListElement.firstChildElement("icon");
    while(!iconElement.isNull())
    {
        QUrl iconUrl = readElementValue("url", iconElement);

        try
        {
            QImage icon = m_creationParameters->m_iconFetcher(
                extractBaseUrl(m_creationParameters->m_deviceLocations[0]),
                iconUrl);

            if (icon.isNull())
            {
                if (m_creationParameters->m_strictness == StrictChecks)
                {
                    throw HParseException(
                        QString("Could not create icon from [%1]").arg(
                            iconUrl.toString()));
                }
                else
                {
                    HLOG_WARN(QString(
                        "Failed to create an icon [%1] specified in the device description. "
                        "Ignoring, since strict parsing is not enabled.").arg(iconUrl.toString()));
                }
            }
            else
            {
                QString iconUrlAsStr = iconUrl.toString();
                retVal.append(qMakePair(QUrl(iconUrlAsStr), icon));
            }
        }
        catch(HException& /*ex*/)
        {
            if (m_creationParameters->m_strictness == StrictChecks)
            {
                throw;
            }

            HLOG_WARN(QString(
                "Failed to create an icon [%1] specified in the device description. "
                "Ignoring, since strict parsing is not enabled.").arg(
                    iconUrl.toString()));
        }

        iconElement = iconElement.nextSiblingElement("icon");
    }

    return retVal;
}

HDeviceInfo* HObjectCreator::parseDeviceInfo(const QDomElement& deviceElement)
{
    HLOG2(H_AT, H_FUN, m_creationParameters->m_loggingIdentifier);

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

    if (m_creationParameters->m_strictness == StrictChecks &&
        wasDefined && tmp.isEmpty())
    {
        throw InvalidDeviceDescription(
            "Presentation URL has to be defined, if the corresponding element is used.");
    }

    QUrl presentationUrl(tmp);

    QString err;
    QScopedPointer<HDeviceInfo> deviceInfo(
        new HDeviceInfo(
            HResourceType(deviceType),
            friendlyName,
            manufacturer,
            manufacturerURL,
            modelDescription,
            modelName,
            modelNumber,
            modelUrl,
            serialNumber,
            udn,
            upc,
            icons,
            presentationUrl,
            m_creationParameters->m_strictness,
            &err));

    if (!deviceInfo->isValid(m_creationParameters->m_strictness))
    {
        throw InvalidDeviceDescription(
            QString("Invalid device description: %1").arg(err));
    }

    return deviceInfo.take();
}

QList<HServiceController*> HObjectCreator::parseServiceList(
    const QDomElement& serviceListElement, HDevice* device)
{
    HLOG2(H_AT, H_FUN, m_creationParameters->m_loggingIdentifier);

    Q_ASSERT(device);
    Q_ASSERT(!serviceListElement.isNull());

    QScopedPointer<HServicesSetupData> services(device->createServices());
    if (!services.data())
    {
        services.reset(new HServicesSetupData());
    }

    QList<HServiceController*> retVal;

    try
    {
        QDomElement serviceElement =
            serviceListElement.firstChildElement("service");

        while(!serviceElement.isNull())
        {
            HServiceId serviceId =
                readElementValue("serviceId", serviceElement);

            HResourceType serviceType =
                HResourceType(readElementValue("serviceType", serviceElement));

            if (!serviceId.isValid(m_creationParameters->m_strictness))
            {
                throw InvalidDeviceDescription(
                    QString("Service ID is invalid:\n%1.").arg(
                        toString(serviceElement)));
            }
            else if (!serviceType.isValid())
            {
                throw InvalidDeviceDescription(
                    QString("Service Type is invalid:\n%1.").arg(
                        toString(serviceElement)));
            }

            HService* service = 0;
            QScopedPointer<HServiceSetup> setupInfo(services->take(serviceId));
            if (!setupInfo.data())
            {
                service = m_creationParameters->createDefaultService(serviceType);
                if (!service)
                {
                    QString err(QString(
                        "No object created for service of type [%1] with ID %2.").arg(
                            serviceType.toString(), serviceId.toString()));

                    throw InvalidDeviceDescription(err);
                }
            }
            else
            {
                service = setupInfo->takeService();
            }

            if (service)
            {
                service->h_ptr->m_parentDevice = device;

                retVal.push_back(new HServiceController(service));

                initService(service, serviceElement, setupInfo.data()); // may throw

                QString errDescr;
                bool ok = service->finalizeInit(&errDescr);
                if (!ok)
                {
                    throw HInitializationException(errDescr);
                }
            }

            serviceElement = serviceElement.nextSiblingElement("service");
        }

        if (!services->isEmpty())
        {
            // In this case the HDevice descendant has defined more UPnP services
            // than the provided UPnP service description. If the HDevice has marked
            // any of the services as mandatory the service description is invalid.
            foreach(const HServiceId& serviceId, services->serviceIds())
            {
                HServiceSetup* setupInfo = services->get(serviceId);
                if (setupInfo->inclusionRequirement() == InclusionMandatory &&
                    setupInfo->version() <= device->info().deviceType().version())
                {
                    throw InvalidDeviceDescription(QString(
                        "Device description is missing a mandatory service: [%1]").arg(
                            serviceId.toString()));
                }
            }
        }
    }
    catch(InvalidDeviceDescription&)
    {
        qDeleteAll(retVal);
        throw;
    }
    catch(HParseException& ex)
    {
        qDeleteAll(retVal);
        throw InvalidServiceDescription(ex.reason());
    }
    catch(HException&)
    {
        qDeleteAll(retVal);
        throw;
    }

    return retVal;
}

namespace
{
QList<QUrl> generateLocations(
    const HUdn& udn, const QList<QUrl>& locations)
{
    QList<QUrl> retVal;
    foreach(const QUrl& location, locations)
    {
        QString locStr = location.toString();
        if (!locStr.endsWith('/'))
        {
            locStr.append(QString("/%1/%2").arg(
                udn.toSimpleUuid(), HDevicePrivate::deviceDescriptionPostFix()));
        }

        retVal.append(locStr);
    }

    return retVal;
}
}

HDeviceController* HObjectCreator::parseDevice(
    const QDomElement& deviceElement, HDevicesSetupData* embeddedDevicesSetup)
{
    HLOG2(H_AT, H_FUN, m_creationParameters->m_loggingIdentifier);

    QScopedPointer<HDeviceInfo> deviceInfo;
    try
    {
        deviceInfo.reset(parseDeviceInfo(deviceElement));
    }
    catch(InvalidDeviceDescription&)
    {
        throw;
    }
    catch(HException& ex)
    {
        throw InvalidDeviceDescription(ex.reason());
    }

    QScopedPointer<HDevice> device;

    if (embeddedDevicesSetup &&
        embeddedDevicesSetup->contains(deviceInfo->deviceType()))
    {
        // this is a recursive call and the parent device has explicitly defined
        // embedded devices ==> take (and remove) the device from this list
        HDeviceSetup* devSetup =
            embeddedDevicesSetup->take(deviceInfo->deviceType());

        device.reset(devSetup->takeDevice());
        delete devSetup;
    }
    else
    {
        // either this is a root device or the device has not explicitly defined
        // embedded devices ==> use the default device creator.
        // (any device can have arbitrary embedded devices; there's no reason
        // to deny that)
        device.reset(m_creationParameters->createDevice(*deviceInfo));
    }

    if (!device)
    {
        // no explicitly defined embedded devices and the user provided
        // creator (if any, as in client-side) also failed. try to create a
        // "default device" (only client-side does this).
        device.reset(m_creationParameters->createDefaultDevice(*deviceInfo));
        if (!device)
        {
            throw HOperationFailedException(QString(
                "No object created for UPnP device type [%1], with UDN: [%2]").arg(
                    deviceInfo->deviceType().toString(), deviceInfo->udn().toString()));
        }
    }

    device->h_ptr->m_upnpDeviceInfo.swap(deviceInfo);
    device->h_ptr->q_ptr = device.data();

    QDomElement serviceListElement =
        deviceElement.firstChildElement("serviceList");

    if (!serviceListElement.isNull())
    {
        device->h_ptr->m_services =
            parseServiceList(serviceListElement, device.data());
    }

    QScopedPointer<HDeviceController> retVal(
        new HDeviceController(
            device.take(), m_creationParameters->m_deviceTimeoutInSecs));
    // the device controller takes ownership of the created device. it will become
    // its parent as well.

    QScopedPointer<HDevicesSetupData> embeddedDevicesSetupLocal(
        retVal->m_device->createEmbeddedDevices());
    // retrieve the explicitly defined embedded devices (if any) for this device.

    QDomElement deviceListElement = deviceElement.firstChildElement("deviceList");
    if (!deviceListElement.isNull())
    {
        QList<HDeviceController*> embeddedDevices;

        QDomElement embeddedDeviceElement =
            deviceListElement.firstChildElement("device");

        while(!embeddedDeviceElement.isNull())
        {
            HDeviceController* embeddedDevice = parseDevice(
                embeddedDeviceElement, embeddedDevicesSetupLocal.data());

            embeddedDevice->setParent(retVal.data());

            embeddedDevice->m_device->h_ptr->m_parent = retVal.data();

            embeddedDevice->m_device->h_ptr->m_deviceDescription =
                m_creationParameters->m_deviceDescription;

            embeddedDevices.push_back(embeddedDevice);

            embeddedDeviceElement =
                embeddedDeviceElement.nextSiblingElement("device");
        }

        retVal->m_device->h_ptr->m_embeddedDevices = embeddedDevices;
    }

    if (embeddedDevicesSetupLocal.data())
    {
        const HDeviceInfo& tmpDevInfo = retVal->m_device->info();

        // If a device has explicitly defined embedded devices, we have to check if
        // any of them has been marked as "mandatory" for the device version
        // in question. In such a case the device description doc has to have
        // a definition for it.
        // --------
        // Note, if every explicitly defined device was also present in the description,
        // the list will be empty at this point.
        foreach(const HResourceType& rt, embeddedDevicesSetupLocal->deviceTypes())
        {
            HDeviceSetup* setup =  embeddedDevicesSetupLocal->get(rt);
            if (setup->inclusionRequirement() == InclusionMandatory &&
                setup->version() <= tmpDevInfo.deviceType().version())
            {
                throw InvalidDeviceDescription(QString(
                    "Device description for device [%1] is missing a mandatory "
                    "embedded device [%2]").arg(
                        tmpDevInfo.deviceType().toString(), rt.toString()));
            }
        }
    }

    QString errDescr;
    if (!retVal->m_device->finalizeInit(&errDescr))
    {
        throw InvalidDeviceDescription(errDescr);
    }

    return retVal.take();
}

HDeviceController* HObjectCreator::createRootDevice()
{
    HLOG2(H_AT, H_FUN, m_creationParameters->m_loggingIdentifier);

    QDomDocument dd;
    QString errMsg; qint32 errLine = 0;
    if (!dd.setContent(
        m_creationParameters->m_deviceDescription, false, &errMsg, &errLine))
    {
        throw InvalidDeviceDescription(QString(
            "Could not parse the device description file: [%1] @ line [%2]:\n[%3]").arg(
                errMsg, QString::number(errLine),
                m_creationParameters->m_deviceDescription));
    }

    QDomElement rootElement = dd.firstChildElement("root");

    // "urn:schemas-upnp-org:device-1-0",

    if (rootElement.isNull())
    {
        throw InvalidDeviceDescription(
            "Invalid device description: no <root> element defined");
    }

    QString err;
    if (!verifySpecVersion(rootElement, &err))
    {
        throw InvalidDeviceDescription(err);
    }

    QDomElement rootDeviceElement = rootElement.firstChildElement("device");
    if (rootDeviceElement.isNull())
    {
        throw InvalidDeviceDescription(
            "The specified file does not contain a valid root device definition");
    }

    HDeviceController* createdDevice = parseDevice(rootDeviceElement);

    createdDevice->m_configId = readConfigId(rootElement);

    createdDevice->m_device->h_ptr->m_deviceDescription =
        m_creationParameters->m_deviceDescription;

    if (m_creationParameters->m_appendUdnToDeviceLocation)
    {
        createdDevice->m_device->h_ptr->m_locations =
            generateLocations(
                createdDevice->m_device->info().udn(),
                m_creationParameters->m_deviceLocations);
    }
    else
    {
        createdDevice->m_device->h_ptr->m_locations =
            m_creationParameters->m_deviceLocations;
    }

    validateRootDevice(createdDevice);

    return createdDevice;
}

}
}
