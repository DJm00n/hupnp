/*
 *  Copyright (C) 2010 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq Core library.
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

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#ifndef EXCEPTIONS_P_H_
#define EXCEPTIONS_P_H_

#include <QString>
#include <QScopedPointer>

namespace Herqq
{

//
//
//
class HExceptionPrivate
{
public: // attributes

    QString m_reason;
    QScopedPointer<HException> m_inner;

public: // methods
    HExceptionPrivate ();
    explicit HExceptionPrivate (const QString& reason);
    explicit HExceptionPrivate (const HException& inner, const QString& reason);
    virtual ~HExceptionPrivate();

    virtual HExceptionPrivate* clone() const;
};

//
//
//
class HInitializationExceptionPrivate :
    public HExceptionPrivate
{
public:

    HInitializationExceptionPrivate();
    explicit HInitializationExceptionPrivate(const QString& reason);
    virtual ~HInitializationExceptionPrivate();

    virtual HInitializationExceptionPrivate* clone() const;
};

//
//
//
class HArgumentExceptionPrivate :
    public HExceptionPrivate
{
public:

    HArgumentExceptionPrivate();
    explicit HArgumentExceptionPrivate(const QString& reason);
    virtual ~HArgumentExceptionPrivate();

    virtual HArgumentExceptionPrivate* clone() const;
};

//
//
//
class HIllegalArgumentExceptionPrivate :
    public HArgumentExceptionPrivate
{
public:

    HIllegalArgumentExceptionPrivate();
    explicit HIllegalArgumentExceptionPrivate(const QString& reason);
    virtual ~HIllegalArgumentExceptionPrivate();

    virtual HIllegalArgumentExceptionPrivate* clone() const;
};

//
//
//
class HMissingArgumentExceptionPrivate :
    public HArgumentExceptionPrivate
{
public:

    HMissingArgumentExceptionPrivate();
    explicit HMissingArgumentExceptionPrivate(const QString& reason);
    virtual ~HMissingArgumentExceptionPrivate();

    virtual HMissingArgumentExceptionPrivate* clone() const;
};

//
//
//
class HOperationFailedExceptionPrivate :
    public HExceptionPrivate
{
public:

    HOperationFailedExceptionPrivate();
    explicit HOperationFailedExceptionPrivate(const QString& reason);

    virtual ~HOperationFailedExceptionPrivate();

    virtual HOperationFailedExceptionPrivate* clone() const;
};

//
//
//
class HTimeoutExceptionPrivate :
    public HOperationFailedExceptionPrivate
{
public:

    HTimeoutExceptionPrivate();
    explicit HTimeoutExceptionPrivate(const QString& reason);

    virtual ~HTimeoutExceptionPrivate();

    virtual HTimeoutExceptionPrivate* clone() const;
};

//
//
//
class HIoExceptionPrivate :
    public HOperationFailedExceptionPrivate
{
public:

    HIoExceptionPrivate();
    explicit HIoExceptionPrivate(const QString& reason);

    virtual ~HIoExceptionPrivate();

    virtual HIoExceptionPrivate* clone() const;
};

//
//
//
class HShutdownInProgressExceptionPrivate :
    public HOperationFailedExceptionPrivate
{
public:

    HShutdownInProgressExceptionPrivate();
    explicit HShutdownInProgressExceptionPrivate(const QString& reason);

    virtual ~HShutdownInProgressExceptionPrivate();

    virtual HShutdownInProgressExceptionPrivate* clone() const;
};

//
//
//
class HSocketExceptionPrivate :
    public HExceptionPrivate
{
public:

    HSocketExceptionPrivate();
    HSocketExceptionPrivate(const QString& reason);

    virtual ~HSocketExceptionPrivate();

    virtual HSocketExceptionPrivate* clone() const;
};

//
//
//
class HParseExceptionPrivate :
    public HExceptionPrivate
{
public:

    HParseExceptionPrivate();
    HParseExceptionPrivate(const QString& reason);

    virtual ~HParseExceptionPrivate();

    virtual HParseExceptionPrivate* clone() const;
};

}

#endif /* EXCEPTIONS_P_H_ */
