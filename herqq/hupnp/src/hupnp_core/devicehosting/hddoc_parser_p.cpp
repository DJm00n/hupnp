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

#include "hddoc_parser_p.h"

#include "../dataelements/hudn.h"
#include "../dataelements/hdeviceinfo.h"
#include "../dataelements/hresourcetype.h"

#include "../general/hupnp_global_p.h"
#include "../datatypes/hupnp_datatypes.h"
#include "../datatypes/hdatatype_mappings_p.h"

#include "../../utils/hlogger_p.h"

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HDocParser
 ******************************************************************************/
HDocParser::HDocParser(
    const QByteArray& loggingIdentifier, HValidityCheckLevel clevel) :
        m_loggingIdentifier(loggingIdentifier), m_cLevel(clevel),
        m_lastErrorDescription(), m_lastError(NoError)
{
}

HStateVariableInfo HDocParser::parseStateVariableInfo_str(
    const QString& name, const QVariant& defValue, const QDomElement& svElement,
    HStateVariableInfo::EventingType evType, HInclusionRequirement incReq)
{
    QStringList allowedValues;

    QDomElement allowedValueListElement =
        svElement.firstChildElement("allowedValueList");

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

    return HStateVariableInfo(
        name, defValue, allowedValues, evType, incReq, &m_lastErrorDescription);
}

HStateVariableInfo HDocParser::parseStateVariableInfo_numeric(
    const QString& name, const QVariant& defValue, const QDomElement& svElement,
    HStateVariableInfo::EventingType evType, HInclusionRequirement incReq,
    HUpnpDataTypes::DataType dataTypeEnumValue)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QDomElement allowedValueRangeElement =
        svElement.firstChildElement("allowedValueRange");

    if (allowedValueRangeElement.isNull())
    {
        return HStateVariableInfo(
            name, dataTypeEnumValue, defValue, evType, incReq, &m_lastErrorDescription);
    }

    QString minimumStr = readElementValue("minimum", allowedValueRangeElement);

    if (minimumStr.isEmpty())
    {
        QString descr = QString("State variable [%1] is missing a "
            "mandatory <minimum> element within <allowedValueRange>.").arg(
                name);

        if (m_cLevel == StrictChecks)
        {
            m_lastError = InvalidServiceDescriptionError;
            m_lastErrorDescription = descr;
            return HStateVariableInfo();
        }
        else
        {
            HLOG_WARN_NONSTD(descr);
            minimumStr = QString::number(INT_MIN);
        }
    }

    QString maximumStr = readElementValue("maximum", allowedValueRangeElement);

    if (maximumStr.isEmpty())
    {
        QString descr = QString("State variable [%1] is missing a "
            "mandatory <maximum> element within <allowedValueRange>.").arg(
                name);

        if (m_cLevel == StrictChecks)
        {
            m_lastError = InvalidServiceDescriptionError;
            m_lastErrorDescription = descr;
            return HStateVariableInfo();
        }
        else
        {
            HLOG_WARN_NONSTD(descr);
            maximumStr = QString::number(INT_MAX);
        }
    }

    QString stepStr = readElementValue("step", allowedValueRangeElement);

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

    return HStateVariableInfo(
        name, dataTypeEnumValue, defValue, minimumStr, maximumStr, stepStr,
        evType, incReq);
}

bool HDocParser::parseActionArguments(
    const QDomElement& argListElement,
    const QHash<QString, HStateVariableInfo>& stateVars,
    QVector<HActionArgument*>* inArgs,
    QVector<HActionArgument*>* outArgs,
    bool* hasRetVal)
{
    bool firstOutArgFound  = false;

    QDomElement argumentElement = argListElement.firstChildElement("argument");
    while(!argumentElement.isNull())
    {
        QString name = readElementValue("name", argumentElement);
        QString dirStr = readElementValue("direction", argumentElement);

        bool retValWasDefined = false;
        readElementValue("retval", argumentElement, &retValWasDefined);

        QString relatedSvStr =
            readElementValue("relatedStateVariable", argumentElement);

        if (!stateVars.contains(relatedSvStr))
        {
            m_lastError = InvalidServiceDescriptionError;
            m_lastErrorDescription = QString(
                "The specified <relatedStateVariable> [%1] is undefined").arg(
                    relatedSvStr);

            return false;
        }

        HActionArgument* createdArg = 0;
        if (dirStr.compare("out", Qt::CaseInsensitive) == 0)
        {
            if (retValWasDefined)
            {
                if (firstOutArgFound)
                {
                    m_lastError = InvalidServiceDescriptionError;
                    m_lastErrorDescription = "[retval] must be the first [out] argument.";
                    return false;
                }

                *hasRetVal = true;
            }

            firstOutArgFound = true;

            createdArg = new HActionArgument(name, stateVars[relatedSvStr]);
            outArgs->push_back(createdArg);
        }
        else if (dirStr.compare("in", Qt::CaseInsensitive) == 0)
        {
            if (firstOutArgFound)
            {
                m_lastError = InvalidServiceDescriptionError;
                m_lastErrorDescription =
                    "Invalid argument order. Input arguments must all come "
                    "before output arguments.";

                return false;
            }

            createdArg = new HActionArgument(name, stateVars[relatedSvStr]);
            inArgs->push_back(createdArg);
        }
        else
        {
            m_lastError = InvalidServiceDescriptionError;
            m_lastErrorDescription = QString(
                "Invalid [direction] value: [%1].").arg(dirStr);

            return false;
        }

        argumentElement = argumentElement.nextSiblingElement("argument");
    }

    return true;
}

QList<QUrl> HDocParser::parseIconList(
    const QDomElement& iconListElement)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    QList<QUrl> retVal;

    QDomElement iconElement = iconListElement.firstChildElement("icon");
    while(!iconElement.isNull())
    {
        QUrl iconUrl = readElementValue("url", iconElement);

        QString iconUrlAsStr = iconUrl.toString();
        retVal.append(QUrl(iconUrlAsStr));

        iconElement = iconElement.nextSiblingElement("icon");
    }

    return retVal;
}

bool HDocParser::parseRoot(
    const QString& docStr, QDomDocument* doc, QDomElement* rootEl)
{
    Q_ASSERT(doc);
    Q_ASSERT(rootEl);

    QString errMsg; qint32 errLine = 0;
    if (!doc->setContent(docStr, false, &errMsg, &errLine))
    {
        m_lastError = InvalidDeviceDescriptionError;
        m_lastErrorDescription = QString(
            "Could not parse the device description file: "
            "[%1] @ line [%2]:\n[%3]").arg(
                errMsg, QString::number(errLine), docStr);

        return false;
    }

    QDomElement rootElement = doc->firstChildElement("root");

    // "urn:schemas-upnp-org:device-1-0",

    if (rootElement.isNull())
    {
        m_lastError = InvalidDeviceDescriptionError;
        m_lastErrorDescription = "Invalid device description: no <root> element defined";
        return false;
    }

    if (!verifySpecVersion(rootElement))
    {
        m_lastError = InvalidDeviceDescriptionError;
        return false;
    }

    QDomElement rootDeviceElement = rootElement.firstChildElement("device");
    if (rootDeviceElement.isNull())
    {
        m_lastError = InvalidDeviceDescriptionError;
        m_lastErrorDescription = "The specified file does not contain a valid root device definition";
        return false;
    }

    *rootEl = rootDeviceElement;
    return true;
}

qint32 HDocParser::readConfigId(const QDomElement& rootElement)
{
    bool ok = false;

    QString cid = readElementValue("configId", rootElement);
    qint32 retVal = cid.toInt(&ok);
    if (!ok || retVal < 0 || retVal > ((1 << 24)-1))
    {
        return 0;
    }

    return retVal;
}

bool HDocParser::parseDeviceInfo(
    const QDomElement& deviceElement, HDeviceInfo* info)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);
    Q_ASSERT(info);

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
    QList<QUrl> icons;
    if (!iconListElement.isNull())
    {
        icons = parseIconList(iconListElement);
    }

    bool wasDefined = false;

    QString tmp = readElementValue("presentationURL", deviceElement, &wasDefined);

    if (m_cLevel == StrictChecks && wasDefined && tmp.isEmpty())
    {
        m_lastError = InvalidDeviceDescriptionError;
        m_lastErrorDescription = "Presentation URL has to be defined, if the "
                                 "corresponding element is used.";

        return false;
    }

    QUrl presentationUrl(tmp);

    *info = HDeviceInfo(
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
        m_cLevel,
        &m_lastErrorDescription);

    if (!info->isValid(m_cLevel))
    {
        m_lastError = InvalidDeviceDescriptionError;
        m_lastErrorDescription =
            QString("Invalid device description: %1").arg(
                m_lastErrorDescription);

        return false;
    }

    return true;
}

bool HDocParser::parseServiceInfo(
    const QDomElement& serviceDefinition, HServiceInfo* serviceInfo)
{
    HLOG2(H_AT, H_FUN, m_loggingIdentifier);

    Q_ASSERT(serviceInfo);
    Q_ASSERT(!serviceDefinition.isNull());

    bool wasDefined = false;

    HServiceId serviceId =
        readElementValue("serviceId", serviceDefinition, &wasDefined);

    if (!wasDefined)
    {
        m_lastError = InvalidDeviceDescriptionError;
        m_lastErrorDescription = QString(
            "Missing mandatory <serviceId> element:\n%1").arg(
                toString(serviceDefinition));

        return false;
    }

    HResourceType resourceType =
        readElementValue("serviceType", serviceDefinition, &wasDefined);

    if (!wasDefined)
    {
        m_lastError = InvalidDeviceDescriptionError;
        m_lastErrorDescription = QString(
            "Missing mandatory <serviceType> element:\n%1").arg(
                toString(serviceDefinition));

        return false;
    }

    QUrl scpdUrl = readElementValue("SCPDURL", serviceDefinition, &wasDefined);
    if (!wasDefined)
    {
        m_lastError = InvalidDeviceDescriptionError;
        m_lastErrorDescription = QString(
            "Missing mandatory <SCPDURL> element:\n%1").arg(
                toString(serviceDefinition));

        return false;
    }

    QUrl controlUrl =
        readElementValue("controlURL" , serviceDefinition, &wasDefined);

    if (!wasDefined)
    {
        m_lastError = InvalidDeviceDescriptionError;
        m_lastErrorDescription = QString(
            "Missing mandatory <controlURL> element:\n%1").arg(
                toString(serviceDefinition));

        return false;
    }

    QUrl eventSubUrl =
        readElementValue("eventSubURL", serviceDefinition, &wasDefined);

    if (!wasDefined)
    {
        m_lastError = InvalidDeviceDescriptionError;
        m_lastErrorDescription = QString(
            "Missing mandatory <eventSubURL> element:\n%1").arg(
                toString(serviceDefinition));

        return false;
    }

    HServiceInfo tmpServiceInfo(
        serviceId, resourceType, controlUrl, eventSubUrl, scpdUrl,
        InclusionMandatory, m_cLevel, &m_lastErrorDescription);

    if (!tmpServiceInfo.isValid(m_cLevel))
    {
        m_lastError = InvalidDeviceDescriptionError;
        m_lastErrorDescription =
            QString("%1:\n%2").arg(
                m_lastErrorDescription, toString(serviceDefinition));

        return false;
    }

    *serviceInfo = tmpServiceInfo;
    return true;
}

bool HDocParser::parseServiceDescription(
    const QString& docStr, QDomDocument* doc,
    QDomElement* stateVarElement, QDomElement* retVal)
{
    Q_ASSERT(stateVarElement);
    Q_ASSERT(retVal);

    qint32 errLine;
    QString errMsg;
    if (!doc->setContent(docStr, false, &errMsg, &errLine))
    {
        m_lastError = InvalidServiceDescriptionError;
        m_lastErrorDescription = QString(
            "Could not parse the service description document: [%1] @ line [%2]:\n[%3]").arg(
                errMsg, QString::number(errLine), docStr);

        return false;
    }
    //QDomNodeList scpdElementNodeList =
        //tmp.elementsByTagNameNS("urn:schemas-upnp-org:service-1-0","scpd");

    QDomElement scpdElement = doc->firstChildElement("scpd");
    if (scpdElement.isNull())
    {
        m_lastError = InvalidServiceDescriptionError;
        m_lastErrorDescription = "Invalid service description: missing <scpd> element";
        return false;
    }

    if (!verifySpecVersion(scpdElement))
    {
        m_lastError = InvalidServiceDescriptionError;
        return false;
    }

    QDomElement serviceStateTableElement =
        scpdElement.firstChildElement("serviceStateTable");

    if (serviceStateTableElement.isNull())
    {
        m_lastError = InvalidServiceDescriptionError;
       m_lastErrorDescription = "Missing mandatory <serviceStateTable> element.";
       return false;
    }

    QDomElement stateVariableElement =
        serviceStateTableElement.firstChildElement("stateVariable");

    if (stateVariableElement.isNull())
    {
        QString err = "Service description document does not have a "
                      "single <stateVariable> element. "
                      "Each service MUST have at least one state variable";

        if (m_cLevel == StrictChecks)
        {
            m_lastError = InvalidServiceDescriptionError;
            m_lastErrorDescription = err;
            return false;
        }
    }

    QDomElement actionListElement = scpdElement.firstChildElement("actionList");
    if (actionListElement.isNull())
    {
        return true;
    }

    QDomElement actionElement = actionListElement.firstChildElement("action");
    if (actionElement.isNull())
    {
        QString err = "Service description document has <actionList> "
                      "element that has no <action> elements.";

        if (m_cLevel == StrictChecks)
        {
            m_lastError = InvalidServiceDescriptionError;
            m_lastErrorDescription = err;
            return false;
        }
    }

    *stateVarElement = stateVariableElement;
    *retVal = actionElement;
    return true;
}

bool HDocParser::parseStateVariable(
    const QDomElement& stateVariableElement, HStateVariableInfo* svInfo)
{
    Q_ASSERT(svInfo);

    QString strSendEvents = stateVariableElement.attribute("sendEvents", "no");
    bool bSendEvents      = false;
    if (strSendEvents.compare("yes", Qt::CaseInsensitive) == 0)
    {
        bSendEvents = true;
    }
    else if (strSendEvents.compare("no", Qt::CaseInsensitive) != 0)
    {
        m_lastError = InvalidServiceDescriptionError;
        m_lastErrorDescription = QString(
            "Invalid value for [sendEvents] attribute:\n%1.").arg(
                toString(stateVariableElement));

        return false;
    }

    QString strMulticast  = stateVariableElement.attribute("multicast", "no");
    bool bMulticast       = false;
    if (strMulticast.compare("yes", Qt::CaseInsensitive) == 0)
    {
        bMulticast = true;
    }
    else if (strMulticast.compare("no", Qt::CaseInsensitive) != 0)
    {
        m_lastError = InvalidServiceDescriptionError;
        m_lastErrorDescription = QString(
            "Invalid value for [multicast]: %1.").arg(
                toString(stateVariableElement));

        return false;
    }

    HStateVariableInfo::EventingType evType = HStateVariableInfo::NoEvents;
    if (bSendEvents)
    {
        evType = bMulticast ?
            HStateVariableInfo::UnicastAndMulticast : HStateVariableInfo::UnicastOnly;
    }

    QString name = readElementValue("name", stateVariableElement);
    QString dataType = readElementValue("dataType", stateVariableElement);

    HUpnpDataTypes::DataType dataTypeEnumValue =
        HUpnpDataTypes::dataType(dataType);

    bool defValueWasDefined = false;
    QString defaultValueStr = readElementValue(
        "defaultValue", stateVariableElement, &defValueWasDefined);

    QVariant defaultValue = defValueWasDefined ?
        convertToRightVariantType(defaultValueStr, dataTypeEnumValue) : QVariant();

    HStateVariableInfo parsedInfo;

    if (dataTypeEnumValue == HUpnpDataTypes::string)
    {
        parsedInfo = parseStateVariableInfo_str(
            name,
            defValueWasDefined ? defaultValueStr : QVariant(),
            stateVariableElement,
            evType,
            InclusionMandatory);
    }
    else if (HUpnpDataTypes::isNumeric(dataTypeEnumValue))
    {
        parsedInfo = parseStateVariableInfo_numeric(
            name,
            defaultValue,
            stateVariableElement,
            evType,
            InclusionMandatory,
            dataTypeEnumValue);
    }
    else
    {
        parsedInfo = HStateVariableInfo(
            name,
            dataTypeEnumValue,
            defaultValue,
            evType,
            InclusionMandatory,
            &m_lastErrorDescription);
    }

    if (!parsedInfo.isValid())
    {
        m_lastError = InvalidServiceDescriptionError;
        m_lastErrorDescription =
            QString("Failed to parse <stateVariable> [%1]: %2").arg(
                name, m_lastErrorDescription);

        return false;
    }

    *svInfo = parsedInfo;
    return true;
}

bool HDocParser::parseActionInfo(
    const QDomElement& actionElement,
    const QHash<QString, HStateVariableInfo>& stateVars,
    HActionInfo* ai)
{
    QString name = readElementValue("name", actionElement);

    bool hasRetVal = false;
    QVector<HActionArgument*> inputArguments;
    QVector<HActionArgument*> outputArguments;

    QDomElement argumentListElement =
        actionElement.firstChildElement("argumentList");

    if (!argumentListElement.isNull())
    {
        if (!parseActionArguments(
                argumentListElement,
                stateVars,
                &inputArguments,
                &outputArguments,
                &hasRetVal))
        {
            qDeleteAll(inputArguments);
            qDeleteAll(outputArguments);
            return false;
        }
    }

    HActionArguments inArgs(inputArguments);
    HActionArguments outArgs(outputArguments);

    HActionInfo actionInfo(
        name, inArgs, outArgs, hasRetVal, InclusionMandatory, &m_lastErrorDescription);

    if (!actionInfo.isValid())
    {
        m_lastError = InvalidServiceDescriptionError;
        m_lastErrorDescription = QString(
            "Failed to parse action [%1]: %2").arg(name, m_lastErrorDescription);

        return false;
    }

    *ai = actionInfo;
    return true;
}

bool HDocParser::verifySpecVersion(const QDomElement& rootElement)
{
    QDomElement specVersionElement = rootElement.firstChildElement("specVersion");
    if (specVersionElement.isNull())
    {
        m_lastErrorDescription = "Missing mandatory <specVersion> element";
        return false;
    }

    QString minorVersion = readElementValue("minor", specVersionElement);
    QString majorVersion = readElementValue("major", specVersionElement);

    bool ok;
    qint32 major = majorVersion.toInt(&ok);
    if (!ok || major != 1)
    {
        m_lastErrorDescription = "Major element of <specVersion> is not 1";
        return false;
    }

    qint32 minor = minorVersion.toInt(&ok);
    if (!ok || (minor != 1 && minor != 0))
    {
        m_lastErrorDescription = "minor element of <specVersion> is not 0 or 1";
        return false;
    }

    return true;
}


}
}
