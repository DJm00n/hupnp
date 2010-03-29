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

#ifndef HOBJECTCREATOR_P_H_
#define HOBJECTCREATOR_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "hdevicecreator.h"

#include "./../general/hdefs_p.h"
#include "./../general/hupnp_fwd.h"
#include "./../../utils/hfunctor.h"
#include "./../devicemodel/hservice.h"
#include "./../devicemodel/hactionarguments.h"

#include <QUrl>
#include <QList>
#include <QImage>
#include <QString>
#include <QDomElement>
#include <QDomDocument>
#include <QSharedPointer>

class QThreadPool;

namespace Herqq
{

namespace Upnp
{

class HActionController;
class HDeviceController;
class HServiceController;
class HSharedActionInvoker;
class HStateVariableController;

//
//
//
typedef Functor<
    Herqq::Upnp::HActionInvoke, H_TYPELIST_1(HAction*)> ActionInvokeCreator;

//
//
//
typedef Functor<QDomDocument, H_TYPELIST_2(const QUrl&, const QUrl&)>
    ServiceDescriptionFetcher;

//
//
//
typedef Functor<QImage, H_TYPELIST_2(const QUrl&, const QUrl&)> IconFetcher;

//
// A class that contains information for the creation of HUPnP's device model
// This information is set by the HDeviceHost and HControlPoint according
// to their needs
//
class HObjectCreationParameters
{
public:

    HObjectCreationParameters();

    QDomDocument   m_deviceDescription;
    QList<QUrl>    m_deviceLocations;
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

    QThreadPool* m_threadPool;

    QByteArray m_loggingIdentifier;
};

//
// The class that creates the HUPnP's device model from description files
//
class HObjectCreator
{
H_DISABLE_COPY(HObjectCreator)

private:

    HObjectCreationParameters m_creationParameters;

private:

    void initService(HService* service, const QDomElement& serviceDefinition);

    QList<QPair<QUrl, QImage> > parseIconList(const QDomElement& iconListElement);
    HDeviceInfo* parseDeviceInfo(const QDomElement& deviceElement);
    void parseServiceDescription(HService* service);

    HActionController* parseAction(
        HService* parentService,
        const QDomElement& actionElement,
        const HService::HActionMap& definedActions);

    HStateVariableController* parseStateVariable(
        HService* parentService, const QDomElement& stateVariableElement);

    QList<HServiceController*> parseServiceList(
        const QDomElement& serviceListElement, HDevice* device);

    HDeviceController* parseDevice(const QDomElement& deviceElement);

public:

    HObjectCreator(const HObjectCreationParameters& creationParameters);

    HDeviceController* createRootDevice();
    // Initializes the creation of the HUPnP's device model for a particular
    // UPnP root device
};

}
}

#endif /* HOBJECTCREATOR_P_H_ */
