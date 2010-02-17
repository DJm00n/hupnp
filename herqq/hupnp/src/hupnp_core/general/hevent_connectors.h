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

#ifndef UPNP_EVENT_CONNECTORS_H_
#define UPNP_EVENT_CONNECTORS_H_

#include "hupnp_fwd.h"
#include "./../../utils/hglobal.h"
#include "./../../utils/hfunctor.h"

#include <QObject>

namespace Herqq
{

namespace Upnp
{

class HEventConnector;

class HEventListenerPrivate;

/*!
 *
 * \remark this class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HEventListener
{
H_DISABLE_COPY(HEventListener)
friend class HEventConnector;

public:

    typedef Functor<void, H_TYPELIST_1(const HDeviceInfo&)> AbstractHostEventCallback;
    typedef Functor<void, H_TYPELIST_1(const HService*)> ServiceEventCallback;
    typedef Functor<void, H_TYPELIST_1(const HStateVariableEvent&)> StateVariableEventCallback;

private:

    HEventListenerPrivate* h_ptr;

    void rootDeviceAdded(const HDeviceInfo& deviceInfo);
    void rootDeviceRemoved(const HDeviceInfo& deviceInfo);
    void stateChanged(const HService* source);
    void valueChanged(const Herqq::Upnp::HStateVariableEvent&);

public:

    HEventListener();
    void setRootDeviceAddedListener(AbstractHostEventCallback);
    void setRootDeviceRemovedListener(AbstractHostEventCallback);
    void setServiceStateChangedListener(ServiceEventCallback);
    void setServiceStateChangedListener(StateVariableEventCallback);

    ~HEventListener();
};

class HEventConnectorPrivate;

/*!
 *
 * \remark this class is not thread safe.
 */
class H_UPNP_CORE_EXPORT HEventConnector :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HEventConnector)

private:

    HEventConnectorPrivate* h_ptr;

private Q_SLOTS:

    void rootDeviceAdded(const Herqq::Upnp::HDeviceInfo&);
    void rootDeviceRemoved(const Herqq::Upnp::HDeviceInfo&);
    void stateChanged(const HService*);
    void valueChanged(const Herqq::Upnp::HStateVariableEvent&);

public:

    HEventConnector();
    virtual ~HEventConnector();

    void setConnection(HAbstractHost*, HEventListener*);
    void setConnection(HService*, HEventListener*);
    void setConnection(HStateVariable*, HEventListener*);
    void setConnection(HAction*, HEventListener*);
};

}
}

#endif /* UPNP_EVENT_CONNECTORS_H_ */
