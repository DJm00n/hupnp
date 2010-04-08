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

#ifndef HEVENT_MESSAGES_H_
#define HEVENT_MESSAGES_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "hnt_p.h"
#include "hsid_p.h"
#include "htimeout_p.h"

#include "./../../dataelements/hproduct_tokens.h"

#include <QUrl>
#include <QList>
#include <QPair>
#include <QDateTime>
#include <QByteArray>

class QString;

namespace Herqq
{

namespace Upnp
{

//
// Class that represents the UPnP eventing subscription request.
//
class SubscribeRequest
{
private:

    QList<QUrl>    m_callbacks;
    HTimeout       m_timeout;
    HSid           m_sid;
    QUrl           m_eventUrl;
    HProductTokens m_userAgent;

public:

    enum RetVal
    {
        Success = 0,
        PreConditionFailed = -1,
        IncompatibleHeaders = -2,
        BadRequest          = -3
    };

public:

    // creates an empty, invalid object.
    SubscribeRequest();

    // creates a renew subscription object (HSid present)
    SubscribeRequest(
        const QUrl& eventUrl, const HSid& sid, const HTimeout& timeout);

    // creates a normal subscription request with a single callback
    SubscribeRequest(
        const QUrl& eventUrl, const HProductTokens& userAgent,
        const QUrl& callback, const HTimeout& timeout);

    // creates a normal subscription request with multiple callbacks
    SubscribeRequest(
        const QUrl& eventUrl, const HProductTokens& userAgent,
        const QList<QUrl>& callbacks, const HTimeout& timeout);

    //
    RetVal setContents(
        const QString& nt, const QUrl& eventUrl, const QString& sid,
        const QString& callback, const QString& timeout,
        const QString& userAgent);

    ~SubscribeRequest();

    inline HNt nt() const
    {
        return HNt(HNt::Type_UpnpEvent);
    }

    inline QList<QUrl> callbacks() const
    {
        return m_callbacks;
    }

    inline bool isValid(bool strict) const
    {
        return !m_callbacks.isEmpty() ||
              (strict ? m_sid.isValid() : !m_sid.isEmpty());
    }

    inline HTimeout timeout() const
    {
        return m_timeout;
    }

    inline HSid sid() const
    {
        return m_sid;
    }

    inline QUrl eventUrl() const
    {
        return m_eventUrl;
    }

    inline bool isRenewal() const
    {
        return !m_sid.isEmpty();
    }

    inline HProductTokens userAgent() const
    {
        return m_userAgent;
    }

    inline bool hasUserAgent() const
    {
        return m_userAgent.isValid();
    }
};

//
//
//
class SubscribeResponse
{
private:

    HSid           m_sid;
    HTimeout       m_timeout;
    HProductTokens m_server;
    QDateTime      m_responseGenerated;

public:

    SubscribeResponse();
    SubscribeResponse(
        const HSid& sid, const HProductTokens& server, const HTimeout& timeout,
        const QDateTime& responseGenerated = QDateTime::currentDateTime());

    ~SubscribeResponse();

    inline bool isValid(bool strict) const
    {
        return strict ? m_sid.isValid() : !m_sid.isEmpty();
    }

    inline HTimeout timeout() const
    {
        return m_timeout;
    }

    inline HSid sid() const
    {
        return m_sid;
    }

    inline HProductTokens server() const
    {
        return m_server;
    }

    inline QDateTime responseGenerated() const
    {
        return m_responseGenerated;
    }
};

//
//
//
class UnsubscribeRequest
{
private:

    QUrl m_eventUrl;
    HSid m_sid;

public:

    enum RetVal
    {
        Success = 0,
        PreConditionFailed = -1,
        BadRequest = -2,
        IncompatibleHeaders = -3
    };

public:

    UnsubscribeRequest();
    UnsubscribeRequest(const QUrl& eventUrl, const HSid& sid);

    ~UnsubscribeRequest();

    RetVal setContents(const QUrl& eventUrl, const QString& sid);

    inline bool isValid(bool strict)const
    {
        return strict ? m_sid.isValid() : !m_sid.isEmpty();
    }

    inline HSid sid() const
    {
        return m_sid;
    }

    inline QUrl eventUrl() const
    {
        return m_eventUrl;
    }
};

//
//
//
class NotifyRequest
{
public:

    enum RetVal
    {
        Success = 0,
        PreConditionFailed = -1,
        InvalidContents = -2,
        InvalidSequenceNr = -3,
        BadRequest = -4
    };

    typedef QList<QPair<QString, QString> > Variables;

private:

    QUrl       m_callback;
    HSid       m_sid;
    quint32    m_seq;
    Variables  m_dataAsVariables;
    QByteArray m_data;

public:

    NotifyRequest();

    NotifyRequest(
        const QUrl& callback, const HSid& sid,
        quint32 seq, const QByteArray& contents);

    ~NotifyRequest();

    RetVal setContents(
        const QUrl& callback,
        const QString& nt, const QString& nts, const QString& sid,
        const QString& seq, const QString& contents);

    inline bool isValid(bool strict) const
    {
        return strict ? m_sid.isValid() : !m_sid.isEmpty();
        // if this is defined then everything else is defined as well
    }

    inline QUrl callback() const { return m_callback; }

    inline HNt nt() const
    {
        return HNt(HNt::Type_UpnpEvent, HNt::SubType_UpnpPropChange);
    }

    inline HSid       sid      () const { return m_sid            ; }
    inline quint32    seq      () const { return m_seq            ; }
    inline QByteArray data     () const { return m_data           ; }
    inline Variables  variables() const { return m_dataAsVariables; }
};

}
}

#endif /* HEVENT_MESSAGES_H_ */
