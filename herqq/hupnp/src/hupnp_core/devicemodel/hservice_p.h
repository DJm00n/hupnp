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

#ifndef HSERVICE_P_H_
#define HSERVICE_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include <HUpnpCore/HServiceInfo>

#include <QtCore/QUrl>
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QMutex>
#include <QtCore/QString>

class QByteArray;

namespace Herqq
{

namespace Upnp
{

class HActionController;
class HStateVariableController;

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

    union
    {
        HService* m_service;
        HServiceProxy* m_serviceProxy;
    };

    HServiceController(HService* service);
    virtual ~HServiceController();

    bool updateVariables(
        const QList<QPair<QString, QString> >& variables, bool sendEvent);

    HActionController* actionByName(const QString& name);

    QList<HActionController*> actions() const;
};

//
// Implementation details of HService
//
class H_UPNP_CORE_EXPORT HServicePrivate
{
H_DECLARE_PUBLIC(HService)
H_DISABLE_COPY(HServicePrivate)

public: // attributes

    HServiceInfo m_serviceInfo;
    QString m_serviceDescription;

    QList<HActionController*> m_actions;
    QHash<QString, HActionController*> m_actionsAsMap;

    QHash<QString, HStateVariableController*> m_stateVariables;

    HService*     q_ptr;
    volatile bool m_eventsEnabled;

    HDevice* m_parentDevice;

    bool m_evented;

    QMutex m_updateMutex;

    QByteArray m_loggingIdentifier;

public: // methods

    HServicePrivate();

    virtual ~HServicePrivate();

    bool addStateVariable(HStateVariableController* stateVariable);

    bool updateVariable(
        const QString& stateVarName, const QVariant& value);

    bool updateVariables(
        const QList<QPair<QString, QString> >& variables, bool sendEvent);
};

}
}

#endif /* HSERVICE_P_H_ */
