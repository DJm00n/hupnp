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

#ifndef UPNP_GLOBAL_P_H_
#define UPNP_GLOBAL_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

class QUrl;
class QString;
class QTcpSocket;
class QDomElement;
class QHostAddress;

namespace Herqq
{

namespace Upnp
{

class HProductTokens;

//
//
//
QString readElementValue(
    const QString elementTagToSearch,
    const QDomElement& parentElement,
    bool* wasDefined = 0);

//
//
//
QString toString(const QDomElement&);

//
//
//
void verifySpecVersion(const QDomElement& rootElement);

//
//
//
qint32 readConfigId(const QDomElement& rootElement);

//
//
//
QString verifyName(const QString& name);

//
//
//
HProductTokens herqqProductTokens();

//
//
//
QString peerAsStr(const QTcpSocket& sock);

//
//
//
QUrl extractBaseUrl(const QUrl& url);

//
// Returns the provided URLs as a string following format "#N URL\n",
// where N = 0..., and URL is the N'th URL in the list.
//
QString urlsAsStr(const QList<QUrl>&);

//
//
//
QString extractBaseUrl(const QString& url);

//
//
//
QUrl appendUrls(const QUrl& baseUrl, const QUrl& relativeUrl);

//
// Returns the part + query + fragment (== request in entirety) sections of the url
//
QString extractRequestPart(const QUrl&);

}
}

#endif /* UPNP_GLOBAL_P_H_ */