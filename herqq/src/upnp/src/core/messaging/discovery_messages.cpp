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

#include "discovery_messages.h"

#include "usn.h"
#include "endpoint.h"
#include "product_tokens.h"
#include "resource_identifier.h"

#include "../utils/xml_utils_p.h"
#include "../../../../utils/src/logger_p.h"
#include "../../../../core/include/HExceptions"

#include <QHostAddress>
#include <QTextStream>
#include <QAtomicInt>
#include <QMetaType>
#include <QDateTime>
#include <QObject>
#include <QString>
#include <QUrl>

static bool registerMetaTypes()
{
    static QAtomicInt tester(0);

    if (tester.testAndSetAcquire(0, 1))
    {
        qRegisterMetaType<Herqq::Upnp::HResourceAvailable>("Herqq::Upnp::HResourceAvailable");
        qRegisterMetaType<Herqq::Upnp::HResourceUnavailable>("Herqq::Upnp::HResourceUnavailable");
        qRegisterMetaType<Herqq::Upnp::HDiscoveryRequest>("Herqq::Upnp::HDiscoveryRequest");
        qRegisterMetaType<Herqq::Upnp::HResourceUpdate>("Herqq::Upnp::HResourceUpdate");
        qRegisterMetaType<Herqq::Upnp::HDiscoveryResponse>("Herqq::Upnp::HDiscoveryResponse");
    }

    return true;
}

static bool test = registerMetaTypes();

namespace Herqq
{

namespace Upnp
{

namespace
{
HEndpoint multicastEndpoint()
{
    static HEndpoint retVal(QHostAddress("239.255.255.250"), 1900);
    return retVal;
}
}

/*******************************************************************************
 * HResourceAvailablePrivate
 ******************************************************************************/
class HResourceAvailablePrivate
{
public: // attributes

    HProductTokens m_serverTokens;
    HUsn    m_usn;
    QUrl    m_location;
    qint32  m_cacheControlMaxAge;
    qint32  m_bootId;
    qint32  m_configId;
    qint32  m_searchPort;

public: // methods

    HResourceAvailablePrivate ();
    ~HResourceAvailablePrivate();
};

HResourceAvailablePrivate::HResourceAvailablePrivate() :
    m_serverTokens(), m_usn(), m_location(), m_cacheControlMaxAge(0),
    m_bootId(0), m_configId(0), m_searchPort(0)
{
}

HResourceAvailablePrivate::~HResourceAvailablePrivate()
{
}

/*******************************************************************************
 * HResourceAvailable
 ******************************************************************************/
HResourceAvailable::HResourceAvailable() :
    h_ptr(new HResourceAvailablePrivate())
{
}

HResourceAvailable::HResourceAvailable(
    quint32 cacheControlMaxAge, const QUrl& location,
    const HProductTokens& serverTokens, const HUsn& usn,
    qint32 bootId, qint32 configId, qint32 searchPort) :
        h_ptr(new HResourceAvailablePrivate())
{
    HLOG(H_AT, H_FUN);

    if (cacheControlMaxAge < 60)
    {
        cacheControlMaxAge = 60;
    }
    else if (cacheControlMaxAge > 60 * 60 * 24)
    {
        cacheControlMaxAge = 60* 60 * 24;
    }

    if (!usn.isValid())
    {
        HLOG_WARN(QObject::tr("Invalid USN."));
        return;
    }

    if (!location.isValid() || location.isEmpty())
    {
        HLOG_WARN(QObject::tr("Invalid LOCATION header field: %1.").arg(
            location.toString()));
        return;
    }

    if (!serverTokens.upnpToken().isValid() || serverTokens.upnpToken().majorVersion() < 1)
    {
        HLOG_WARN(QObject::tr("Invalid server tokens"));
        return;
    }

    if (serverTokens.upnpToken().minorVersion() > 0)
    {
        if (bootId < 0 || configId < 0)
        {
            HLOG_WARN(QObject::tr("bootId and configId must both be >= 0."));
            return;
        }
        if (searchPort < 49152 || searchPort > 65535)
        {
            searchPort = -1;
        }
    }
    else
    {
        searchPort = -1;
    }

    h_ptr->m_serverTokens       = serverTokens;
    h_ptr->m_usn                = usn;
    h_ptr->m_location           = location;
    h_ptr->m_cacheControlMaxAge = cacheControlMaxAge;
    h_ptr->m_configId           = configId;
    h_ptr->m_bootId             = bootId;
    h_ptr->m_searchPort         = searchPort;
}

HResourceAvailable::HResourceAvailable(const HResourceAvailable& other) :
    h_ptr(new HResourceAvailablePrivate(*other.h_ptr))
{
}

HResourceAvailable& HResourceAvailable::operator=(const HResourceAvailable& other)
{
    HResourceAvailablePrivate* newDptr = new HResourceAvailablePrivate(*other.h_ptr);

    delete h_ptr;
    h_ptr = newDptr;
    return *this;
}

HResourceAvailable::~HResourceAvailable()
{
    delete h_ptr;
}

bool HResourceAvailable::isValid() const
{
    return h_ptr->m_usn.isValid();
    // if the object is valid, the USN is valid
}

HProductTokens HResourceAvailable::serverTokens() const
{
    return h_ptr->m_serverTokens;
}

HUsn HResourceAvailable::usn() const
{
    return h_ptr->m_usn;
}

QUrl HResourceAvailable::location() const
{
    return h_ptr->m_location;
}

qint32 HResourceAvailable::cacheControlMaxAge() const
{
    return h_ptr->m_cacheControlMaxAge;
}

qint32 HResourceAvailable::bootId() const
{
    return h_ptr->m_bootId;
}

qint32 HResourceAvailable::configId() const
{
    return h_ptr->m_configId;
}

qint32 HResourceAvailable::searchPort() const
{
    return h_ptr->m_searchPort;
}

QString HResourceAvailable::toString() const
{
    if (!isValid())
    {
        return "";
    }

    QString retVal;
    QTextStream ts(&retVal);

    ts << "NOTIFY * HTTP/1.1\r\n"
       << "HOST: "                  << multicastEndpoint().toString() << "\r\n"
       << "CACHE-CONTROL: max-age=" << cacheControlMaxAge  () << "\r\n"
       << "LOCATION: "              << location().toString () << "\r\n"
       << "NT: "                    << usn().resource().toString() << "\r\n"
       << "NTS: "                   << "ssdp:alive\r\n"
       << "SERVER: "                << serverTokens().toString()   << "\r\n"
       << "USN: "                   << usn().toString()       << "\r\n";

    if (serverTokens().upnpToken().minorVersion() > 0)
    {
        ts << "BOOTID.UPNP.ORG: "   << bootId()   << "\r\n"
           << "CONFIGID.UPNP.ORG: " << configId() << "\r\n";

        if (h_ptr->m_searchPort >= 0)
        {
            ts << "SEARCHPORT.UPNP.ORG: " << searchPort() << "\r\n";
        }
    }

    ts << "\r\n";

    return retVal;
}

bool operator==(const HResourceAvailable& obj1, const HResourceAvailable& obj2)
{
    return obj1.toString() == obj2.toString();
}

bool operator!=(const HResourceAvailable& obj1, const HResourceAvailable& obj2)
{
    return !(obj1 == obj2);
}

/*******************************************************************************
 * HResourceUnavailablePrivate
 ******************************************************************************/
class HResourceUnavailablePrivate
{
public: // attributes

    HUsn    m_usn;
    qint32  m_bootId;
    qint32  m_configId;
    HEndpoint m_sourceLocation;

public: // methods

    HResourceUnavailablePrivate ();
    ~HResourceUnavailablePrivate();
};

HResourceUnavailablePrivate::HResourceUnavailablePrivate() :
    m_usn(), m_bootId(0), m_configId(0), m_sourceLocation(0)
{
}

HResourceUnavailablePrivate::~HResourceUnavailablePrivate()
{
}

/*******************************************************************************
 * HResourceUnavailable
 ******************************************************************************/
HResourceUnavailable::HResourceUnavailable() :
    h_ptr(new HResourceUnavailablePrivate())
{
}

HResourceUnavailable::HResourceUnavailable(
    const HUsn& usn, const HEndpoint& sourceLocation,
    qint32 bootId, qint32 configId) :
        h_ptr(new HResourceUnavailablePrivate())
{
    HLOG(H_AT, H_FUN);

    if (!usn.isValid())
    {
        HLOG_WARN(QObject::tr("Invalid USN."));
        return;
    }

    if ((bootId < 0 && configId >= 0) || (configId < 0 && bootId >= 0))
    {
        HLOG_WARN(QObject::tr(
            "If either bootId or configId is specified, they both must be >= 0."));
        return;
    }

    if (bootId < 0)
    {
        bootId = -1; configId = -1;
    }

    h_ptr->m_usn            = usn;
    h_ptr->m_configId       = configId;
    h_ptr->m_bootId         = bootId;
    h_ptr->m_sourceLocation = sourceLocation;
}

HResourceUnavailable::HResourceUnavailable(const HResourceUnavailable& other) :
    h_ptr(new HResourceUnavailablePrivate(*other.h_ptr))
{
}

HResourceUnavailable& HResourceUnavailable::operator=(
    const HResourceUnavailable& other)
{
    HResourceUnavailablePrivate* newDptr = new HResourceUnavailablePrivate(*other.h_ptr);

    delete h_ptr;
    h_ptr = newDptr;

    return *this;
}

HResourceUnavailable::~HResourceUnavailable()
{
    delete h_ptr;
}

HEndpoint HResourceUnavailable::location() const
{
    return h_ptr->m_sourceLocation;
}

bool HResourceUnavailable::isValid() const
{
    return h_ptr->m_usn.isValid();
    // if the object is valid, the USN is valid
}

HUsn HResourceUnavailable::usn() const
{
    return h_ptr->m_usn;
}

qint32 HResourceUnavailable::bootId() const
{
    return h_ptr->m_bootId;
}

qint32 HResourceUnavailable::configId() const
{
    return h_ptr->m_configId;
}

QString HResourceUnavailable::toString() const
{
    if (!isValid())
    {
        return "";
    }

    QString retVal;
    QTextStream ts(&retVal);

    ts << "NOTIFY * HTTP/1.1\r\n"
       << "HOST: "     << multicastEndpoint().toString()<< "\r\n"
       << "NT: "       << usn().resource().toString()   << "\r\n"
       << "NTS: "      << "ssdp:byebye\r\n"
       << "USN: "      << usn().toString() << "\r\n";

    if (bootId() >= 0)
    {
        ts << "BOOTID.UPNP.ORG: "   << bootId  ()  << "\r\n"
           << "CONFIGID.UPNP.ORG: " << configId()  << "\r\n";
    }

    ts << "\r\n";

    return retVal;
}

bool operator==(const HResourceUnavailable& obj1, const HResourceUnavailable& obj2)
{
    return obj1.toString() == obj2.toString();
}

bool operator!=(const HResourceUnavailable& obj1, const HResourceUnavailable& obj2)
{
    return !(obj1 == obj2);
}

/*******************************************************************************
 * HResourceUpdatePrivate
 ******************************************************************************/
class HResourceUpdatePrivate
{
public: // attributes

    HUsn    m_usn;
    QUrl    m_location;
    qint32  m_bootId;
    qint32  m_configId;
    qint32  m_nextBootId;
    qint32  m_searchPort;

public: // methods

    HResourceUpdatePrivate ();
    ~HResourceUpdatePrivate();
};

HResourceUpdatePrivate::HResourceUpdatePrivate() :
    m_usn(), m_location(), m_bootId(0), m_configId(0), m_nextBootId(0),
    m_searchPort(0)
{
}

HResourceUpdatePrivate::~HResourceUpdatePrivate()
{
}

/*******************************************************************************
 * HResourceUpdate
 ******************************************************************************/
HResourceUpdate::HResourceUpdate() :
    h_ptr(new HResourceUpdatePrivate())
{
}

HResourceUpdate::HResourceUpdate(
    const QUrl& location, const HUsn& usn,
    qint32 bootId, qint32 configId, qint32 nextBootId, qint32 searchPort) :
        h_ptr(new HResourceUpdatePrivate())
{
    HLOG(H_AT, H_FUN);

    if (!usn.isValid())
    {
        HLOG_WARN(QObject::tr("Invalid USN."));
        return;
    }

    if (!location.isValid())
    {
        HLOG_WARN(QObject::tr("Invalid LOCATION header field."));
        return;
    }

    if ((bootId     < 0 && (configId >= 0 || nextBootId >= 0)) ||
        (configId   < 0 && (bootId   >= 0 || nextBootId >= 0)) ||
        (nextBootId < 0 && (bootId   >= 0 || configId   >= 0)))
    {
        HLOG_WARN(
            QObject::tr("If bootId, configId or nextBootId is specified, " \
                        "they all must be >= 0."));
        return;
    }

    if (bootId < 0)
    {
        bootId = -1; configId = -1; nextBootId = -1; searchPort = -1;
    }
    else if (searchPort < 49152 || searchPort > 65535)
    {
        searchPort = -1;
    }

    h_ptr->m_usn         = usn;
    h_ptr->m_location    = location;
    h_ptr->m_configId    = configId;
    h_ptr->m_bootId      = bootId;
    h_ptr->m_nextBootId  = nextBootId;
    h_ptr->m_searchPort  = searchPort;
}

HResourceUpdate::HResourceUpdate(const HResourceUpdate& other) :
    h_ptr(new HResourceUpdatePrivate(*other.h_ptr))
{
}

HResourceUpdate& HResourceUpdate::operator=(const HResourceUpdate& other)
{
    HResourceUpdatePrivate* newDptr = new HResourceUpdatePrivate(*other.h_ptr);

    delete h_ptr;
    h_ptr = newDptr;
    return *this;
}

HResourceUpdate::~HResourceUpdate()
{
    delete h_ptr;
}

bool HResourceUpdate::isValid() const
{
    return h_ptr->m_usn.isValid();
    // if the object is valid, the USN is valid
}

HUsn HResourceUpdate::usn() const
{
    return h_ptr->m_usn;
}

QUrl HResourceUpdate::location() const
{
    return h_ptr->m_location;
}

qint32 HResourceUpdate::bootId() const
{
    return h_ptr->m_bootId;
}

qint32 HResourceUpdate::configId() const
{
    return h_ptr->m_configId;
}

qint32 HResourceUpdate::nextBootId() const
{
    return h_ptr->m_nextBootId;
}

qint32 HResourceUpdate::searchPort() const
{
    return h_ptr->m_searchPort;
}

QString HResourceUpdate::toString() const
{
    if (!isValid())
    {
        return "";
    }

    QString retVal;
    QTextStream ts(&retVal);

    ts << "NOTIFY * HTTP/1.1\r\n"
       << "HOST: "                  << multicastEndpoint().toString() << "\r\n"
       << "LOCATION: "              << location().toString ()      << "\r\n"
       << "NT: "                    << usn().resource().toString() << "\r\n"
       << "NTS: "                   << "ssdp:update\r\n"
       << "USN: "                   << usn().toString() << "\r\n";

    if (bootId() >= 0)
    {
        ts << "BOOTID.UPNP.ORG: "       << bootId()     << "\r\n"
           << "CONFIGID.UPNP.ORG: "     << configId()   << "\r\n"
           << "NEXTBOOTID.UPNP.ORG: "   << nextBootId() << "\r\n";

        if (h_ptr->m_searchPort >= 0)
        {
            ts << "SEARCHPORT.UPNP.ORG: " << searchPort() << "\r\n";
        }
    }

    ts << "\r\n";

    return retVal;
}

bool operator==(const HResourceUpdate& obj1, const HResourceUpdate& obj2)
{
    return obj1.toString() == obj2.toString();
}

bool operator!=(const HResourceUpdate& obj1, const HResourceUpdate& obj2)
{
    return !(obj1 == obj2);
}

/*******************************************************************************
 * HDiscoveryRequestPrivate
 ******************************************************************************/
class HDiscoveryRequestPrivate
{
public: // attributes

    HResourceIdentifier m_st;
    qint32              m_mx;
    HProductTokens      m_userAgent;

public: // methods

    HDiscoveryRequestPrivate() : m_st(), m_mx(0), m_userAgent() {}

    bool init(const HResourceIdentifier& st, qint32 mx, const HProductTokens& userAgent)
    {
        HLOG(H_AT, H_FUN);

        if (st.type() == HResourceIdentifier::Undefined)
        {
            HLOG_WARN(QObject::tr("Invalid Search Target."));
            return false;
        }

        if (mx < 1)
        {
            HLOG_WARN(QObject::tr("MX cannot be smaller than 1."));
            return false;
        }
        else if (mx > 5)
        {
            HLOG_WARN(QObject::tr("MX is larger than 5, setting it to 5."));
            mx = 5; // UDA instructs to treat MX values larger than 5 as 5
        }

        m_st          = st;
        m_mx          = mx;
        m_userAgent   = userAgent;

        return true;
    }
};

/*******************************************************************************
 * HDiscoveryRequest
 ******************************************************************************/
HDiscoveryRequest::HDiscoveryRequest() :
    h_ptr(new HDiscoveryRequestPrivate())
{
}

HDiscoveryRequest::HDiscoveryRequest(
    qint32 mx, const HResourceIdentifier& st, const HProductTokens& userAgent) :
        h_ptr(new HDiscoveryRequestPrivate())
{
    h_ptr->init(st, mx, userAgent);
}

HDiscoveryRequest::HDiscoveryRequest(const HDiscoveryRequest& other) :
    h_ptr(new HDiscoveryRequestPrivate(*other.h_ptr))
{
}

HDiscoveryRequest& HDiscoveryRequest::operator=(const HDiscoveryRequest& other)
{
    HDiscoveryRequestPrivate* newDptr =
        new HDiscoveryRequestPrivate(*other.h_ptr);

    delete h_ptr;
    h_ptr = newDptr;

    return *this;
}

HDiscoveryRequest::~HDiscoveryRequest()
{
    delete h_ptr;
}

bool HDiscoveryRequest::isValid() const
{
    return h_ptr->m_st.type() != HResourceIdentifier::Undefined;
    // if the object is valid, the ResourceIdentifier is defined ==> this is a good enough
    // test for validity
}

HResourceIdentifier HDiscoveryRequest::searchTarget() const
{
    return h_ptr->m_st;
}

qint32 HDiscoveryRequest::mx() const
{
    return h_ptr->m_mx;
}

QString HDiscoveryRequest::toString() const
{
    if (!isValid())
    {
        return "";
    }

    QString retVal;
    QTextStream out(&retVal);

    out << "M-SEARCH * HTTP/1.1\r\n"
        << "HOST: "       << multicastEndpoint().toString() << "\r\n"
        << "MAN: "        << "\"ssdp:discover\"\r\n"
        << "MX: "         << mx()                      << "\r\n"
        << "ST: "         << searchTarget().toString() << "\r\n"
        << "USER-AGENT: " << userAgent().toString()    << "\r\n\r\n";

    return retVal;
}

HProductTokens HDiscoveryRequest::userAgent() const
{
    return h_ptr->m_userAgent;
}

bool operator==(const HDiscoveryRequest& obj1, const HDiscoveryRequest& obj2)
{
    return obj1.toString() == obj2.toString();
}

bool operator!=(const HDiscoveryRequest& obj1, const HDiscoveryRequest& obj2)
{
    return !(obj1 == obj2);
}

/*******************************************************************************
 * HDiscoveryResponsePrivate
 ******************************************************************************/
class HDiscoveryResponsePrivate
{
public: // attributes

    HProductTokens m_serverTokens;
    HUsn      m_usn;
    QUrl      m_location;
    QDateTime m_date;
    qint32    m_cacheControlMaxAge;
    qint32    m_bootId;
    qint32    m_configId;
    qint32    m_searchPort;

public: // methods

    HDiscoveryResponsePrivate() :
        m_serverTokens(), m_usn(), m_location(), m_date(),
        m_cacheControlMaxAge(0), m_bootId(0), m_configId(0), m_searchPort(0)
    {
    }

};

/*******************************************************************************
 * HDiscoveryResponse
 ******************************************************************************/
HDiscoveryResponse::HDiscoveryResponse() :
    h_ptr(new HDiscoveryResponsePrivate())
{
}

HDiscoveryResponse::HDiscoveryResponse(
    quint32 cacheControlMaxAge, const QDateTime& /*date*/, const QUrl& location,
    const HProductTokens& serverTokens, const HUsn& usn,
    qint32 bootId, qint32 configId, qint32 searchPort) :
        h_ptr(new HDiscoveryResponsePrivate())
{
    HLOG(H_AT, H_FUN);

    if (!usn.isValid())
    {
        HLOG_WARN(QObject::tr("Invalid USN."));
        return;
    }

    if (!location.isValid())
    {
        HLOG_WARN(QObject::tr("Invalid resource location."));
        return;
    }

    if (!serverTokens.upnpToken().isValid() || serverTokens.upnpToken().majorVersion() < 1)
    {
        HLOG_WARN(QObject::tr("Invalid server tokens."));
        return;
    }

    if (serverTokens.upnpToken().minorVersion() > 0)
    {
        if (bootId < 0 || configId < 0)
        {
            HLOG_WARN(QObject::tr("bootId and configId must both be positive."));
            return;
        }
    }

    h_ptr->m_serverTokens       = serverTokens;
    h_ptr->m_usn                = usn;
    h_ptr->m_location           = location;
    h_ptr->m_date               = QDateTime::currentDateTime();
    h_ptr->m_cacheControlMaxAge = cacheControlMaxAge;
    h_ptr->m_bootId             = bootId;
    h_ptr->m_configId           = configId;
    h_ptr->m_searchPort         = searchPort;
}

HDiscoveryResponse::HDiscoveryResponse(const HDiscoveryResponse& other) :
    h_ptr(new HDiscoveryResponsePrivate(*other.h_ptr))
{
}

HDiscoveryResponse& HDiscoveryResponse::operator=(const HDiscoveryResponse& other)
{
    HDiscoveryResponsePrivate* newDptr =
        new HDiscoveryResponsePrivate(*other.h_ptr);

    delete h_ptr;
    h_ptr = newDptr;

    return *this;
}

HDiscoveryResponse::~HDiscoveryResponse()
{
    delete h_ptr;
}

bool HDiscoveryResponse::isValid() const
{
    return h_ptr->m_usn.isValid();
    // if the object is valid, the USN is valid ==> this is a good enough
    // test for validity
}

HProductTokens HDiscoveryResponse::serverTokens() const
{
    return h_ptr->m_serverTokens;
}

QDateTime HDiscoveryResponse::date() const
{
    return h_ptr->m_date;
}

HUsn HDiscoveryResponse::usn() const
{
    return h_ptr->m_usn;
}

QUrl HDiscoveryResponse::location() const
{
    return h_ptr->m_location;
}

qint32 HDiscoveryResponse::cacheControlMaxAge() const
{
    return h_ptr->m_cacheControlMaxAge;
}

qint32 HDiscoveryResponse::bootId() const
{
    return h_ptr->m_bootId;
}

qint32 HDiscoveryResponse::configId() const
{
    return h_ptr->m_configId;
}

qint32 HDiscoveryResponse::searchPort() const
{
    return h_ptr->m_searchPort;
}

QString HDiscoveryResponse::toString() const
{
    if (!isValid())
    {
        return "";
    }

    QString retVal;
    QTextStream out(&retVal);

    out << "HTTP/1.1 200 OK\r\n"
        << "CACHE-CONTROL: max-age=" << cacheControlMaxAge()  << "\r\n"
        << "EXT:"                                             << "\r\n"
        << "LOCATION: "              << location().toString() << "\r\n"
        << "SERVER: "                << serverTokens().toString()   << "\r\n"
        << "ST: "                    << usn().resource().toString() << "\r\n"
        << "USN: "                   << usn().toString()      << "\r\n";

    if (bootId() >= 0)
    {
        out << "BOOTID.UPNP.ORG: "   << bootId()   << "\r\n"
            << "CONFIGID.UPNP.ORG: " << configId() <<"\r\n";

        if (h_ptr->m_searchPort >= 0)
        {
            out << "SEARCHPORT.UPNP.ORG: " << searchPort() << "\r\n";
        }
    }

    out << "\r\n";

    return retVal;
}

bool operator==(const HDiscoveryResponse& obj1, const HDiscoveryResponse& obj2)
{
    return obj1.toString() == obj2.toString();
}

bool operator!=(const HDiscoveryResponse& obj1, const HDiscoveryResponse& obj2)
{
    return !(obj1 == obj2);
}

}
}
