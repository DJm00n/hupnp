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

#ifndef UPNP_SERVICE_P_H_
#define UPNP_SERVICE_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "upnp_fwd.h"
#include "upnp_action.h"
#include "upnp_service.h"
#include "upnp_serviceid.h"
#include "upnp_resourcetype.h"
#include "upnp_statevariable.h"
#include "upnp_statevariable_p.h"

#include <QUrl>
#include <QHash>
#include <QList>
#include <QPair>
#include <QMutex>
#include <QString>
#include <QDomDocument>

class QByteArray;

namespace Herqq
{

namespace Upnp
{

//
// This is an internal class that provides more powerful interface for interacting
// with HService than what the HServices's public interface offers.
//
// These features are required so that the HUpnpContolPoint and HDeviceHost
// can appropriately manage the HService instances they own.
//
class H_UPNP_CORE_EXPORT HServiceController :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HServiceController)

public:

    HService* m_service;

    HServiceController(HService* service);
    virtual ~HServiceController();

    bool updateVariables(
        const QList<QPair<QString, QString> >& variables, bool sendEvent);
};

//
// Implementation details of HService
//
class H_UPNP_CORE_EXPORT HServicePrivate
{
H_DISABLE_COPY(HServicePrivate)

public: // attributes

    HServiceId      m_serviceId;
    HResourceType   m_serviceType;
    QUrl            m_scpdUrl;
    QUrl            m_controlUrl;
    QUrl            m_eventSubUrl;
    QDomDocument    m_serviceDescriptor;

    QList<HAction*> m_actions;
    QHash<QString, HAction*> m_actionsAsMap;

    QHash<QString, HStateVariableController*> m_stateVariables;

    HService*     q_ptr;
    volatile bool m_eventsEnabled;

    HDevice* m_parentDevice;

    bool m_evented;

    QMutex m_updateMutex;

    const QByteArray m_loggingIdentifier;

    bool m_stateVariablesAreImmutable;

public: // methods

    HServicePrivate(
        const QString& loggingIdentifier = "");

    virtual ~HServicePrivate();

    bool addStateVariable(HStateVariableController* stateVariable);

    bool updateVariable(
        const QString& stateVarName, const QVariant& value);

    bool updateVariables(
        const QList<QPair<QString, QString> >& variables, bool sendEvent);
};

}
}

#endif /* UPNP_SERVICE_P_H_ */
