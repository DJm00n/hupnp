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

#include "upnp_fwd.h"
#include "upnp_service.h"
#include "upnp_devicecreator.h"
#include "../../../core/include/HGlobal"
#include "../../../core/include/HFunctor"
#include "../../../core/include/HExceptions"

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

//
//
//
class ServiceDescriptionFetcherImpl
{
public:

    ServiceDescriptionFetcherImpl(){}
    virtual ~ServiceDescriptionFetcherImpl() = 0;
    virtual QDomDocument operator ()(
        const QUrl& deviceLocation, const QUrl& scpdUrl) = 0;

    virtual ServiceDescriptionFetcherImpl* clone() const = 0;
};

typedef QDomDocument (*ServiceDescriptionFetcherFuntionType)(
    const QUrl& deviceLocation, const QUrl& scpdUrl);

//
//
//
template<typename Fun>
class ServiceDescriptionFetcherHandler :
    public ServiceDescriptionFetcherImpl
{
private:

    Fun m_fun;

public:

    ServiceDescriptionFetcherHandler(const Fun& fun) :
        m_fun(fun)
    {
    }

    virtual ~ServiceDescriptionFetcherHandler(){}

    virtual QDomDocument operator()(const QUrl& deviceLocation, const QUrl& scpdUrl)
    {
        return m_fun(deviceLocation, scpdUrl);
    }

    virtual ServiceDescriptionFetcherHandler* clone() const
    {
        return new ServiceDescriptionFetcherHandler(m_fun);
    }
};

//
//
//
template<typename PointerToObject, typename PointerToMemFun>
class ServiceDescriptionFetcherMemFunHandler :
    public ServiceDescriptionFetcherImpl
{
private:

    PointerToObject m_pobj;
    PointerToMemFun m_pmemf;

public:

    ServiceDescriptionFetcherMemFunHandler(
        const PointerToObject& pobj, PointerToMemFun pmemf) :
            m_pobj(pobj), m_pmemf(pmemf)
    {
    }

    virtual ~ServiceDescriptionFetcherMemFunHandler(){}

    virtual QDomDocument operator()(const QUrl& deviceLocation, const QUrl& scpdUrl)
    {
        return ((*m_pobj).*m_pmemf)(deviceLocation, scpdUrl);
    }

    virtual ServiceDescriptionFetcherMemFunHandler* clone() const
    {
        return new ServiceDescriptionFetcherMemFunHandler(m_pobj, m_pmemf);
    }
};

//
//
//
class ServiceDescriptionFetcher
{
private:

    ServiceDescriptionFetcherImpl* m_impl;

public:

    ServiceDescriptionFetcher() : m_impl(0) {}

    template<typename Fun>
    ServiceDescriptionFetcher(const Fun& fun) :
        m_impl(new ServiceDescriptionFetcherHandler<Fun>(fun))
    {
    }

    ServiceDescriptionFetcher(ServiceDescriptionFetcherFuntionType fun) :
        m_impl(new ServiceDescriptionFetcherHandler<ServiceDescriptionFetcherFuntionType>(fun))
    {
    }

    template<class PointerToObject, typename PointerToMemFun>
    ServiceDescriptionFetcher(const PointerToObject& pobj, PointerToMemFun pmemfun) :
        m_impl(new ServiceDescriptionFetcherMemFunHandler<PointerToObject, PointerToMemFun>(pobj, pmemfun))
    {
    }

    ~ServiceDescriptionFetcher() { delete m_impl; }

    ServiceDescriptionFetcher& operator=(const ServiceDescriptionFetcher& other)
    {
        ServiceDescriptionFetcherImpl* newImpl = other.m_impl ? other.m_impl->clone() : 0;
        delete m_impl;
        m_impl = newImpl;

        return *this;
    }

    ServiceDescriptionFetcher(const ServiceDescriptionFetcher& other) :
        m_impl(other.m_impl ? other.m_impl->clone() : 0) {}

    QDomDocument operator()(const QUrl& deviceLocation, const QUrl& scpdUrl)
    {
        return m_impl ? (*m_impl)(deviceLocation, scpdUrl) : QDomDocument();
    }

    bool operator!() const
    {
        return !m_impl;
    }

private:

    // Helper for enabling 'if (sp)'
    struct Tester
    {
        Tester(int) {}
        void dummy() {}
    };

    typedef void (Tester::*unspecified_boolean_type_)();

public:

    // enable 'if (sp)'
    operator unspecified_boolean_type_() const
    {
        return !m_impl ? 0 : &Tester::dummy;
    }
};

typedef HActionInvoke (*ActionInvokeCreatorFunctionType)(
    HService* service, const QString& actionName,
    const HActionInputArguments& inArgs,
    const HActionOutputArguments& outArgs);

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

    ActionInvokeCreatorFunctionType m_actionInvokeCreator;
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

    qint32 m_deviceTimeoutInSecs;

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


//
//
//
class InvalidDeviceDescription :
    public Herqq::HParseException
{
public:
    InvalidDeviceDescription();
    InvalidDeviceDescription(const QString& reason);
    virtual ~InvalidDeviceDescription() throw ();
};

//
//
//
class InvalidServiceDescription :
    public Herqq::HParseException
{
public:
    InvalidServiceDescription();
    InvalidServiceDescription(const QString& reason);
    virtual ~InvalidServiceDescription() throw ();
};

}
}

#endif /* UPNP_OBJECTCREATOR_P_H_ */
