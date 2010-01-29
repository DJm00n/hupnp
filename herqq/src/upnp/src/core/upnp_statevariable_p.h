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

#ifndef UPNP_STATEVARIABLE_P_H_
#define UPNP_STATEVARIABLE_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "upnp_datatypes.h"
#include "upnp_statevariable.h"
#include "utils/valuerange_p.h"

#include <QUrl>
#include <QMutex>
#include <QString>
#include <QStringList>

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

    HStateVariable* m_eventSource;
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

    bool isValidValue(const QVariant& value, QVariant* convertedValue = 0) const;
    bool setValue(const QVariant& newValue);
};

//
// Implementation details of HStateVariable
//
class HStateVariablePrivate
{
H_DISABLE_COPY(HStateVariablePrivate)

public:

    QString                  m_name;
    HUpnpDataTypes::DataType m_dataType;
    QVariant::Type           m_variantDataType;
    QVariant                 m_defaultValue;
    HStateVariable::EventingType m_eventingType;
    QStringList              m_allowedValueList;
    HValueRange              m_allowedValueRange;
    QVariant                 m_value;
    QMutex                   m_valueMutex;

    HService*                m_parentService;

public:

    HStateVariablePrivate ();
    ~HStateVariablePrivate();

    void setName             (const QString&);
    void setDataType         (HUpnpDataTypes::DataType);
    QVariant checkValue      (const QVariant&);
    void setDefaultValue     (const QVariant&);
    void setEventingType     (HStateVariable::EventingType);
    void setAllowedValueList (const QStringList&);
    void setAllowedValueRange(HValueRange);
    bool setValue            (const QVariant& value);
};

}
}

#endif /* UPNP_ACTION_P_H_ */
