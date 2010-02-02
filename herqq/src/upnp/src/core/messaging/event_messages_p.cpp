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

#include "event_messages_p.h"

#include "../../../../utils/src/logger_p.h"

#include <QRegExp>
#include <QStringList>
#include <QHostAddress>
#include <QDomDocument>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * SubscribeRequest
 *******************************************************************************/
namespace
{
bool isValidCallback(const QUrl& callback)
{
    return callback.isValid() && !callback.isEmpty() &&
           callback.scheme() == "http" && !(QHostAddress(callback.host()).isNull());
}

bool isValidEventUrl(const QUrl& eventUrl)
{
    return eventUrl.isValid() && !eventUrl.isEmpty() &&
        !(QHostAddress(eventUrl.host()).isNull());
}
}

SubscribeRequest::SubscribeRequest() :
    m_callbacks(), m_timeout(), m_sid(), m_eventUrl(), m_userAgent()
{
}

SubscribeRequest::SubscribeRequest(
    const QUrl& eventUrl, const HSid& sid, const HTimeout& timeout) :
        m_callbacks(), m_timeout(), m_sid(), m_eventUrl(), m_userAgent()
{
    HLOG(H_AT, H_FUN);

    if (!isValidEventUrl(eventUrl))
    {
        HLOG_WARN(QObject::tr("Invalid eventURL: [%1]").arg(eventUrl.toString()));
        return;
    }
    else if (sid.isNull())
    {
        HLOG_WARN(QObject::tr("Null SID"));
        return;
    }

    m_timeout  = timeout;
    m_eventUrl = eventUrl;
    m_sid      = sid;
}

SubscribeRequest::SubscribeRequest(
    const QUrl& eventUrl, const HProductTokens& userAgent, const QUrl& callback,
    const HTimeout& timeout) :
        m_callbacks (), m_timeout(), m_sid(), m_eventUrl(), m_userAgent()
{
    HLOG(H_AT, H_FUN);
    if (!isValidEventUrl(eventUrl))
    {
        HLOG_WARN(QObject::tr("Invalid eventURL: [%1]").arg(eventUrl.toString()));
        return;
    }
    else if (!isValidCallback(callback))
    {
        HLOG_WARN(QObject::tr("Invalid callback: [%1]").arg(callback.toString()));
        return;
    }

    m_callbacks.push_back(callback);

    m_timeout   = timeout;
    m_eventUrl  = eventUrl;
    m_userAgent = userAgent;
}

SubscribeRequest::SubscribeRequest(
    const QUrl& eventUrl, const HProductTokens& userAgent,
    const QList<QUrl>& callbacks, const HTimeout& timeout) :
        m_callbacks (callbacks), m_timeout(), m_sid(), m_eventUrl(), m_userAgent()
{
    HLOG(H_AT, H_FUN);
    if (!isValidEventUrl(eventUrl))
    {
        HLOG_WARN(QObject::tr("Invalid eventURL: [%1]").arg(eventUrl.toString()));
        return;
    }

    Q_ASSERT(!callbacks.isEmpty());

    foreach(QUrl callback, callbacks)
    {
        if (isValidCallback(callback))
        {
            return;
        }
    }

    m_timeout   = timeout;
    m_eventUrl  = eventUrl;
    m_userAgent = userAgent;
    m_callbacks = callbacks;
}

SubscribeRequest::~SubscribeRequest()
{
}

namespace
{
QList<QUrl> parseCallbacks(const QString& arg)
{
    HLOG(H_AT, H_FUN);
    QList<QUrl> retVal;

    QStringList callbacks =
        arg.split(QRegExp("<[.]*>"), QString::SkipEmptyParts);

    foreach(QString callbackStr, callbacks)
    {
        QUrl callback(callbackStr.remove('<').remove('>'));
        if (!callback.isValid() || callback.isEmpty() || callback.scheme() != "http")
        {
            return QList<QUrl>();
        }

        retVal.push_back(callback);
    }

    return retVal;
}
}

SubscribeRequest::RetVal SubscribeRequest::setContents(
    const QString& nt, const QUrl& eventUrl, const QString& sid,
    const QString& callback, const QString& timeout, const QString& userAgent)
{
    HLOG(H_AT, H_FUN);
    // these have to be properly defined no matter what
    if (!isValidEventUrl(eventUrl))
    {
        HLOG_WARN(QObject::tr("Invalid eventURL: [%1]").arg(eventUrl.toString()));
        return BadRequest;
    }

    SubscribeRequest tmp;

    // these fields are the same regardless of message type
    tmp.m_eventUrl = eventUrl;
    tmp.m_timeout  = timeout;

    if (!HSid(sid).isNull())
    {
        // this appears to be a renewal, confirm.
        if (!callback.isEmpty() || !nt.isEmpty())
        {
            return IncompatibleHeaders;
        }

        tmp.m_sid = sid;

        *this = tmp;
        return Success;
    }

    // this appears to be an initial subscription

    if (nt.compare("upnp:event", Qt::CaseInsensitive) != 0)
    {
        return PreConditionFailed;
    }

    tmp.m_callbacks = parseCallbacks(callback);
    if (tmp.m_callbacks.isEmpty())
    {
        return PreConditionFailed;
    }

    tmp.m_userAgent = userAgent;

    *this = tmp;
    return Success;
}

HNt SubscribeRequest::nt() const
{
    return HNt(HNt::Type_UpnpEvent);
}

QList<QUrl> SubscribeRequest::callbacks() const
{
    return m_callbacks;
}

HTimeout SubscribeRequest::timeout() const
{
    return m_timeout;
}

HSid SubscribeRequest::sid() const
{
    return m_sid;
}

QUrl SubscribeRequest::eventUrl() const
{
    return m_eventUrl;
}

bool SubscribeRequest::isRenewal() const
{
    return !m_sid.isNull();
}

HProductTokens SubscribeRequest::userAgent() const
{
    return m_userAgent;
}

bool SubscribeRequest::hasUserAgent() const
{
    return m_userAgent.isValid();
}

bool SubscribeRequest::isValid() const
{
    return !m_callbacks.isEmpty() || !m_sid.isNull();
}

/*******************************************************************************
 * SubscribeResponse
 *******************************************************************************/
SubscribeResponse::SubscribeResponse() :
    m_sid(), m_timeout(), m_server(), m_responseGenerated()
{
}

SubscribeResponse::SubscribeResponse(
    const HSid& sid, const HProductTokens& server, const HTimeout& timeout,
    const QDateTime& responseGenerated) :
        m_sid(sid), m_timeout(timeout), m_server(server),
        m_responseGenerated(responseGenerated)
{
    HLOG(H_AT, H_FUN);
    if (m_sid.isNull())
    {
        *this = SubscribeResponse();
    }
}

SubscribeResponse::~SubscribeResponse()
{
}

HTimeout SubscribeResponse::timeout() const
{
    return m_timeout;
}

HSid SubscribeResponse::sid() const
{
    return m_sid;
}

bool SubscribeResponse::isValid() const
{
    return !m_sid.isNull();
}

HProductTokens SubscribeResponse::server() const
{
    return m_server;
}

QDateTime SubscribeResponse::responseGenerated() const
{
    return m_responseGenerated;
}

/*******************************************************************************
 * UnsubscribeRequest
 *******************************************************************************/
UnsubscribeRequest::UnsubscribeRequest() :
    m_eventUrl(), m_sid()
{
}

UnsubscribeRequest::UnsubscribeRequest(const QUrl& eventUrl, const HSid& sid) :
    m_eventUrl(), m_sid()
{
    HLOG(H_AT, H_FUN);
    if (sid.isNull() || !isValidEventUrl(eventUrl))
    {
        return;
    }

    m_eventUrl = eventUrl;
    m_sid      = sid;
}

UnsubscribeRequest::~UnsubscribeRequest()
{
}

UnsubscribeRequest::RetVal UnsubscribeRequest::setContents(
    const QUrl& eventUrl, const QString& sid)
{
    HLOG(H_AT, H_FUN);
    UnsubscribeRequest tmp;

    tmp.m_sid = sid;
    tmp.m_eventUrl = eventUrl;

    if (tmp.m_sid.isNull())
    {
        return PreConditionFailed;
    }
    else if (!isValidEventUrl(tmp.m_eventUrl))
    {
        return BadRequest;
    }

    *this = tmp;
    return Success;
}

bool UnsubscribeRequest::isValid() const
{
    return !m_sid.isNull();
}

HSid UnsubscribeRequest::sid() const
{
    return m_sid;
}

QUrl UnsubscribeRequest::eventUrl() const
{
    return m_eventUrl;
}

/*******************************************************************************
 * NotifyRequest
 *******************************************************************************/
namespace
{
NotifyRequest::RetVal parseData(
    const QByteArray& data, QList<QPair<QString, QString> >& parsedData)
{
    HLOG(H_AT, H_FUN);
    QDomDocument dd;
    if (!dd.setContent(data, true))
    {
        return NotifyRequest::InvalidContents;
    }

    //QDomNodeList propertySetNodes =
      //  dd.elementsByTagNameNS("urn:schemas-upnp.org:event-1-0", "propertyset");

    QDomNodeList propertySetNodes = dd.elementsByTagName("propertyset");

    if (propertySetNodes.size() != 1)
    {
        return NotifyRequest::InvalidContents;
    }

    QDomNodeList propertyNodeList =
        propertySetNodes.at(0).toElement().elementsByTagName("property");
        //propertySetNodes.at(0).toElement().elementsByTagNameNS(
          //  "urn:schemas-upnp.org:event-1-0", "property");

    if (!propertyNodeList.size())
    {
        return NotifyRequest::InvalidContents;
    }

    QList<QPair<QString, QString> > tmp;

    for (int i = 0; i < propertyNodeList.size(); ++i)
    {
        QDomElement propertyElement   = propertyNodeList.at(i).toElement();
        QDomNodeList variableElements = propertyElement.childNodes();
        if (variableElements.size() <= 0)
        {
            return NotifyRequest::InvalidContents;
        }

        QDomElement variableElement = variableElements.at(0).toElement();
        QDomText variableValue = variableElement.firstChild().toText();
        tmp.push_back(
            qMakePair(variableElement.localName(), variableValue.data()));
    }

    parsedData = tmp;

    return NotifyRequest::Success;
}
}

NotifyRequest::NotifyRequest() :
    m_callback(), m_sid(), m_seq(0), m_dataAsVariables(), m_data()
{
}

NotifyRequest::NotifyRequest(
    const QUrl& callback, const HSid& sid,
    quint32 seq, const QByteArray& contents) :
        m_callback(), m_sid(), m_seq(0), m_dataAsVariables(),
        m_data    ()
{
    HLOG(H_AT, H_FUN);

    if (!isValidCallback(callback) ||
        sid.isNull() || contents.isEmpty())
    {
        return;
    }

    if (parseData(contents, m_dataAsVariables) != Success)
    {
        return;
    }

    m_callback = callback;
    m_sid      = sid;
    m_seq      = seq;
    m_data     = contents;
}

NotifyRequest::~NotifyRequest()
{
}

NotifyRequest::RetVal NotifyRequest::setContents(
    const QUrl& callback,
    const QString& nt, const QString& nts, const QString& sid,
    const QString& seq, const QString& contents)
{
    HLOG(H_AT, H_FUN);

    HNt tmpNt(nt, nts);
    if (tmpNt.type   () != HNt::Type_UpnpEvent ||
        tmpNt.subType() != HNt::SubType_UpnpPropChange)
    {
        return PreConditionFailed;
    }

    NotifyRequest tmp;

    tmp.m_callback = callback;
    if (!isValidCallback(tmp.m_callback))
    {
        return BadRequest;
    }

    tmp.m_sid = sid;
    if (tmp.m_sid.isNull())
    {
        return PreConditionFailed;
    }

    QString tmpSeq = seq.trimmed();

    bool ok = false;
    tmp.m_seq = tmpSeq.toUInt(&ok);
    if (!ok)
    {
        return InvalidSequenceNr;
    }

    tmp.m_data = contents.toUtf8();

    RetVal rv = parseData(tmp.m_data, tmp.m_dataAsVariables);
    if (rv != Success)
    {
        return rv;
    }

    *this = tmp;
    return Success;
}

HNt NotifyRequest::nt() const
{
    return HNt(HNt::Type_UpnpEvent, HNt::SubType_UpnpPropChange);
}

bool NotifyRequest::isValid() const
{
    return !m_sid.isNull();
    // if this is defined then everything else is defined as well
}

HSid NotifyRequest::sid() const
{
    return m_sid;
}

quint32 NotifyRequest::seq() const
{
    return m_seq;
}

QUrl NotifyRequest::callback() const
{
    return m_callback;
}

QList<QPair<QString, QString> > NotifyRequest::variables() const
{
    return m_dataAsVariables;
}

QByteArray NotifyRequest::data() const
{
    return m_data;
}

}
}
