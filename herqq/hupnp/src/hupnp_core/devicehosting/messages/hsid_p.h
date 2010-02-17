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

#ifndef HSID_H_
#define HSID_H_

#include <QUuid>

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

namespace Herqq
{

namespace Upnp
{

//
//
//
class HSid
{
friend quint32 qHash(const HSid &key);

private:

    QUuid m_value;

public:

    HSid();
    explicit HSid (const QUuid&  sid);
    explicit HSid (const QString& sid);
    HSid          (const HSid& other);

    ~HSid();

    HSid& operator=(const HSid& other);
    HSid& operator=(const QUuid& other);
    HSid& operator=(const QString& other);

    QUuid   value   () const;
    QString toString() const;
    bool    isNull  () const;
};

bool operator==(const HSid& sid1, const HSid& sid2);
bool operator!=(const HSid& sid1, const HSid& sid2);

quint32 qHash(const HSid& key);

}
}

#endif /* HSID_H_ */
