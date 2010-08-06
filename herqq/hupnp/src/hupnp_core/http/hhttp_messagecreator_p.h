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

#ifndef HHTTP_MESSAGECREATOR_H_
#define HHTTP_MESSAGECREATOR_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "hhttp_p.h"
#include "../devicehosting/messages/hevent_messages_p.h"

class QString;
class QByteArray;
class QHttpHeader;
class QHttpRequestHeader;
class QHttpResponseHeader;

namespace Herqq
{

namespace Upnp
{

class MessagingInfo;

//
//
//
class HHttpMessageCreator
{
private:

    HHttpMessageCreator();
    ~HHttpMessageCreator();

    static QByteArray setupData(
        MessagingInfo&, qint32 statusCode,
        const QString& reasonPhrase, const QString& body,
        ContentType ct = Undefined);

public:

    static QByteArray setupData(QHttpHeader& hdr, MessagingInfo&);
    static QByteArray setupData(
        QHttpHeader& hdr, const QByteArray& body, MessagingInfo&,
        ContentType = Undefined);

    inline static QByteArray createResponse(StatusCode, MessagingInfo&)
    {
        return createResponse(sc, mi, QByteArray());
    }

    static QByteArray createResponse(
        StatusCode, MessagingInfo&, const QByteArray& body,
        ContentType = Undefined);

    static QByteArray createResponse(
        MessagingInfo&, qint32 actionErrCode, const QString& msg="");

    static QByteArray create(const NotifyRequest&     , MessagingInfo&);
    static QByteArray create(const SubscribeRequest&  , MessagingInfo&);
    static QByteArray create(const UnsubscribeRequest&, MessagingInfo&);
    static QByteArray create(const SubscribeResponse& , MessagingInfo&);

    static NotifyRequest::RetVal create(
        const QHttpRequestHeader& reqHdr, const QByteArray& body,
        NotifyRequest& req);

    static SubscribeRequest::RetVal create(
        const QHttpRequestHeader& reqHdr,
        SubscribeRequest& req);

    static UnsubscribeRequest::RetVal create(
        const QHttpRequestHeader& reqHdr,
        UnsubscribeRequest& req);

    static bool create(
        const QHttpResponseHeader& respHdr,
        SubscribeResponse& resp);
};

}
}

#endif /* HHTTP_MESSAGECREATOR_H_ */
