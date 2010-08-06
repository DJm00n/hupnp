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

#ifndef HSTATEVARIABLE_P_H_
#define HSTATEVARIABLE_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../dataelements/hstatevariableinfo.h"

#include <QMutex>
#include <QString>
#include <QVariant>

namespace Herqq
{

namespace Upnp
{

class HService;

//
// Implementation details of HStateVariableEvent
//
class HStateVariableEventPrivate
{
public:

    HStateVariableInfo m_eventSource;
    QVariant m_previousValue;
    QVariant m_newValue;

public:

    HStateVariableEventPrivate ();
    ~HStateVariableEventPrivate();
};

//
// This is an internal class that provides more powerful interface for interacting
// with HService than what the HServices's public interface offers.
//
// These features are required so that the HUpnpContolPoint and HDeviceHost
// can appropriately manage the HService instances they own.
//
class HStateVariableController
{
H_DISABLE_COPY(HStateVariableController)

public:

    HStateVariable* m_stateVariable;

    HStateVariableController(HStateVariable* stateVar);
    virtual ~HStateVariableController();

    bool setValue(const QVariant& newValue);
};

//
// Implementation details of HStateVariable
//
class HStateVariablePrivate
{
H_DISABLE_COPY(HStateVariablePrivate)

public:

    HStateVariableInfo m_info;

    QVariant  m_value;
    QMutex    m_valueMutex;
    HService* m_parentService;

    const QByteArray m_loggingIdentifier;

public:

    HStateVariablePrivate();
    virtual ~HStateVariablePrivate();

    bool setValue(const QVariant& value, QString* err = 0);
};

}
}

#endif /* UPNP_ACTION_P_H_ */
