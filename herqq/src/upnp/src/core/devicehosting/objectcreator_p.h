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

#ifndef UPNP_OBJECTCREATOR_P_H_
#define UPNP_OBJECTCREATOR_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "devicecreator.h"

#include "../upnp_fwd.h"
#include "../devicemodel/service.h"
#include "../../../../core/include/HGlobal"
#include "../../../../core/include/HFunctor"

#include "../devicemodel/actionarguments.h"

#include <QUrl>
#include <QImage>
#include <QList>
#include <QString>
#include <QDomElement>
#include <QDomDocument>
#include <QSharedPointer>

namespace Herqq
{

namespace Upnp
{

class HActionController;
class HDeviceController;
class HServiceController;
class HSharedActionInvoker;
class HStateVariableController;

typedef HActionInvoke (*ActionInvokeCreatorFunctionType)(
    HService* service, const QString& actionName,
    const HActionInputArguments& inArgs,
    const HActionOutputArguments& outArgs);

//
//
//
typedef Functor<
    Herqq::Upnp::HActionInvoke,
    H_TYPELIST_4(
        Herqq::Upnp::HService*,
        const QString&,
        const Herqq::Upnp::HActionInputArguments&,
        const Herqq::Upnp::HActionOutputArguments&)> ActionInvokeCreator;

//
//
//
typedef Functor<QDomDocument, H_TYPELIST_2(const QUrl&, const QUrl&)> ServiceDescriptionFetcher;

//
//
//
typedef Functor<QImage, H_TYPELIST_2(const QUrl&, const QUrl&)> IconFetcher;

//
//
//
class HObjectCreationParameters
{
public:

    HObjectCreationParameters();

    QDomDocument  m_deviceDescription;
    QList<QUrl>   m_deviceLocations;
    HDeviceCreator m_deviceCreator;

    ActionInvokeCreator m_actionInvokeCreator;
    // provides the possibility to intercept (and override) the user defined
    // action invocations

    bool m_createDefaultObjects;
    // provides the possibility to create a UPnP device tree of default
    // device and service types (HDefaultDevice & HDefaultService).
    // This is especially useful with control points that cannot rely on user's
    // knowledge regarding the encountered device and service types.

    ServiceDescriptionFetcher m_serviceDescriptionFetcher;
    // provides the possibility of defining how the service description is
    // retrieved

    quint32 m_deviceTimeoutInSecs;

    bool m_appendUdnToDeviceLocation;

    QHash<HUdn, HSharedActionInvoker*>* m_sharedActionInvokers;

    IconFetcher m_iconFetcher;

    bool m_strictParsing;

    bool m_stateVariablesAreImmutable;
};

//
//
//
class HObjectCreator
{
H_DISABLE_COPY(HObjectCreator)

private:

    HObjectCreationParameters m_creationParameters;

private:

    void init(
        HService* service, const QDomElement& serviceDefinition);

    QList<QPair<QUrl, QImage> > parseIconList(const QDomElement& iconListElement);
    HDeviceInfo* parseDeviceInfo(const QDomElement& deviceElement);
    void parseServiceDescription(HService* service);

    HAction* parseAction(
        HService* parentService,
        const QDomElement& actionElement,
        const HService::HActionMapT& definedActions);

    HStateVariableController* parseStateVariable(
        const QDomElement& stateVariableElement);

    QList<HServiceController*> parseServiceList(
        const QDomElement& serviceListElement, HDevice* device);

    HDeviceController* parseDevice(const QDomElement& deviceElement);

public:

    HObjectCreator(const HObjectCreationParameters& creationParameters);
    HDeviceController* createRootDevice();
};

}
}

#endif /* UPNP_OBJECTCREATOR_P_H_ */
