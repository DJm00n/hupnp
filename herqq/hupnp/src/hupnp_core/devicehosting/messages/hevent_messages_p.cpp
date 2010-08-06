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

#include "hevent_messages_p.h"

#include "../../../utils/hlogger_p.h"

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
inline bool isValidCallback(const QUrl& callback)
{
    return callback.isValid() && !callback.isEmpty() &&
           callback.scheme() == "http" && !(QHostAddress(callback.host()).isNull());
}

inline bool isValidEventUrl(const QUrl& eventUrl)
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
        HLOG_WARN(QString("Invalid eventURL: [%1]").arg(eventUrl.toString()));
        return;
    }
    else if (sid.isEmpty())
    {
        HLOG_WARN("Empty SID");
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
        HLOG_WARN(QString("Invalid eventURL: [%1]").arg(eventUrl.toString()));
        return;
    }
    else if (!isValidCallback(callback))
    {
        HLOG_WARN(QString("Invalid callback: [%1]").arg(callback.toString()));
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
        m_callbacks(), m_timeout(), m_sid(), m_eventUrl(), m_userAgent()
{
    HLOG(H_AT, H_FUN);

    if (!isValidEventUrl(eventUrl))
    {
        HLOG_WARN(QString("Invalid eventURL: [%1]").arg(eventUrl.toString()));
        return;
    }

    foreach(const QUrl& callback, callbacks)
    {
        if (!isValidCallback(callback))
        {
            HLOG_WARN(QString("Invalid callback: [%1]").arg(callback.toString()));
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

    // this has to be properly defined no matter what
    if (!isValidEventUrl(eventUrl))
    {
        HLOG_WARN(QString("Invalid eventURL: [%1]").arg(eventUrl.toString()));
        return BadRequest;
    }

    SubscribeRequest tmp;

    // these fields are the same regardless of message type
    tmp.m_eventUrl = eventUrl;
    tmp.m_timeout  = timeout;

    if (!HSid(sid).isEmpty())
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

    if (nt.simplified().compare("upnp:event", Qt::CaseInsensitive) != 0)
    {
        return PreConditionFailed;
    }

    tmp.m_callbacks = parseCallbacks(callback);
    if (tmp.m_callbacks.isEmpty())
    {
        return PreConditionFailed;
    }

    tmp.m_userAgent = HProductTokens(userAgent);

    *this = tmp;
    return Success;
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
    if (m_sid.isEmpty())
    {
        *this = SubscribeResponse();
    }
}

SubscribeResponse::~SubscribeResponse()
{
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
    if (sid.isEmpty() || !isValidEventUrl(eventUrl))
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
    UnsubscribeRequest tmp;

    tmp.m_sid = sid;
    tmp.m_eventUrl = eventUrl;

    if (tmp.m_sid.isEmpty())
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

    QDomElement propertySetElement = dd.firstChildElement("propertyset");

    if (propertySetElement.isNull())
    {
        return NotifyRequest::InvalidContents;
    }

    QDomElement propertyElement =
        propertySetElement.firstChildElement("property");
        //propertySetNodes.at(0).toElement().elementsByTagNameNS(
          //  "urn:schemas-upnp.org:event-1-0", "property");

    QList<QPair<QString, QString> > tmp;
    while(!propertyElement.isNull())
    {
        QDomElement variableElement = propertyElement.firstChildElement();
        if (variableElement.isNull())
        {
            return NotifyRequest::InvalidContents;
        }

        QDomText variableValue = variableElement.firstChild().toText();
        tmp.push_back(
            qMakePair(variableElement.localName(), variableValue.data()));

        propertyElement = propertyElement.nextSiblingElement("property");
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

    if (!isValidCallback(callback) || sid.isEmpty() || contents.isEmpty())
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
    if (tmp.m_sid.isEmpty())
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

}
}
