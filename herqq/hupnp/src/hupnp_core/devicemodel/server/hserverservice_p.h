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

#ifndef HSERVERSERVICE_P_H_
#define HSERVERSERVICE_P_H_

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
#include <QtCore/QString>

class QByteArray;

namespace Herqq
{

namespace Upnp
{

class HServerStateVariable;

//
// Implementation details of HServerService
//
class H_UPNP_CORE_EXPORT HServerServicePrivate
{
H_DECLARE_PUBLIC(HServerService)
H_DISABLE_COPY(HServerServicePrivate)

public: // attributes

    HServiceInfo m_serviceInfo;
    QString m_serviceDescription;

    QHash<QString, HServerAction*> m_actions;
    QHash<QString, HServerStateVariable*> m_stateVariables;

    HServerService* q_ptr;

    QByteArray m_loggingIdentifier;

    bool m_eventsEnabled;
    bool m_evented;

public: // methods

    HServerServicePrivate();

    virtual ~HServerServicePrivate();

    bool addStateVariable(HServerStateVariable* stateVariable);

    bool updateVariable(
        const QString& stateVarName, const QVariant& value);

    bool updateVariables(
        const QList<QPair<QString, QString> >& variables, bool sendEvent);
};

}
}

#endif /* HSERVERSERVICE_P_H_ */
