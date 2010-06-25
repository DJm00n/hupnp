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

#ifndef EXCEPTIONS_P_H_
#define EXCEPTIONS_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "hglobal.h"
#include <exception>

#include <QString>
#include <QScopedPointer>

namespace Herqq
{

//
// A base class for all exception classes used in the Herqq libraries.
//
class HException :
    public std::exception
{
protected:

    QString m_reason;
    QScopedPointer<HException> m_inner;

public:

    //
    // Creates a new, empty instance with no description.
    //
    HException();

    //
    // Copy constructor.
    //
    HException(const HException&);

    //
    // Creates a new instance with the specified error string.
    //
    // \param reason specifies the reason why the exception was thrown.
    //
    // \remark The reason is a free description and it is meant for humans only.
    //
    explicit HException(const QString& reason);

    //
    // Creates a new instance specifying another exception that was caught
    // and a reason detailing why this instance was thrown.
    //
    // This is useful in situations where you have caught an exception, but want
    // to throw another exception of different type of description, yet without
    // losing the information stored in the caught exception.
    //
    // \param inner specifies the exception caught.
    // \param reason specifies the reason why the exception was thrown.
    //
    // \sa reason() \sa inner()
    //
    HException(const HException& inner, const QString& reason);

    //
    // Destroys the instance.
    //
    virtual ~HException() throw ();

    //
    // Assignment operator.
    //
    virtual HException& operator=(const HException&);

    //
    // Returns full description of the exception.
    //
    // \return full description of the exception. This is an override of
    // std::exception::what() and it is equal to call reason(true).
    //
    virtual const char* what() const throw();

    //
    // Returns a clone of the exception.
    //
    // \return a clone of the exception.
    //
    // \remark the ownership of the object is transferred to the caller.
    //
    virtual HException* clone() const = 0;

    //
    // Returns the inner exception stored, if any.
    //
    // \return the inner exception stored, if any. In case the exception has no
    // inner exception chained, null is returned.
    //
    const HException* inner() const;

    //
    // Returns the reason -if specified- detailing why the exception was thrown.
    //
    // The reason is a free description and it is meant for humans.
    //
    // \return the reason -if specified- detailing why the exception was thrown.
    //
    QString reason(bool includeInner = true) const;
};

//
// An exception class used when some type of initialization,
// such as object construction, fails.
//
//
class HInitializationException :
    public HException
{
public:

    //
    // Creates a new, empty instance with no description.
    //
    HInitializationException();

    //
    // Creates a new instance with the specified error string.
    //
    // \param reason specifies the reason why the exception was thrown.
    //
    // \remark The reason is a free description and it is meant for humans only.
    //
    explicit HInitializationException(const QString& reason);

    //
    // Creates a new instance specifying another exception that was caught
    // and a reason detailing why this instance was thrown.
    //
    // This is useful in situations where you have caught an exception, but want
    // to throw another exception of different type of description, yet without
    // losing the information stored in the caught exception.
    //
    // \sa reason() \sa inner()
    //
    // \param inner specifies the exception caught.
    // \param reason specifies the reason why the exception was thrown.
    //
    HInitializationException(const HException& inner, const QString& reason);

    //
    // Destroys the instance.
    //
    virtual ~HInitializationException() throw();
    virtual HInitializationException* clone() const;
};


class HArgumentExceptionPrivate;

//
// An exception class used to indicate some type of an error relating to
// an argument of any type.
//
class HArgumentException :
    public HException
{
public:

    //
    // Creates a new, empty instance with no description.
    //
    HArgumentException();

    //
    // Creates a new instance with the specified error string.
    // \param reason specifies the reason why the exception was thrown.
    //
    // \remark The reason is a free description and it is meant for humans only.
    //
    explicit HArgumentException(const QString& reason);

    //
    // Creates a new instance specifying another exception that was caught
    // and a reason detailing why this instance was thrown.
    //
    // This is useful in situations where you have caught an exception, but want
    // to throw another exception of different type of description, yet without
    // losing the information stored in the caught exception.
    //
    // \sa reason() \sa inner()
    //
    // \param inner specifies the exception caught.
    // \param reason specifies the reason why the exception was thrown.
    //
    HArgumentException(const HException& inner, const QString& reason);

    //
    // Destroys the instance.
    //
    virtual ~HArgumentException() throw();
    virtual HArgumentException* clone() const;
};

//
// An exception class used in situations where an invalid argument has been
// provided to a method.
//
class HIllegalArgumentException :
    public HArgumentException
{
public:

    //
    // Creates a new, empty instance with no description.
    //
    HIllegalArgumentException();

    //
    // Creates a new instance with the specified error string.
    // \param reason specifies the reason why the exception was thrown.
    //
    // \remark The reason is a free description and it is meant for humans only.
    //
    explicit HIllegalArgumentException(const QString& reason);

    //
    // Creates a new instance specifying another exception that was caught
    // and a reason detailing why this instance was thrown.
    //
    // This is useful in situations where you have caught an exception, but want
    // to throw another exception of different type of description, yet without
    // losing the information stored in the caught exception.
    //
    // \sa reason() \sa inner()
    //
    // \param inner specifies the exception caught.
    // \param reason specifies the reason why the exception was thrown.
    //
    HIllegalArgumentException(const HException& inner, const QString& reason);

    //
    // Destroys the instance.
    //
    virtual ~HIllegalArgumentException() throw();
    virtual HIllegalArgumentException* clone() const;
};

//
// An exception class used in situations where processing cannot continue
// due to a missing argument.
//
class HMissingArgumentException :
    public HArgumentException
{
public:

    //
    // Creates a new, empty instance with no description.
    //
    HMissingArgumentException();

    //
    // Creates a new instance with the specified error string.
    // \param reason specifies the reason why the exception was thrown.
    //
    // \remark The reason is a free description and it is meant for humans only.
    //
    explicit HMissingArgumentException(const QString& reason);

    //
    // Creates a new instance specifying another exception that was caught
    // and a reason detailing why this instance was thrown.
    //
    // This is useful in situations where you have caught an exception, but want
    // to throw another exception of different type of description, yet without
    // losing the information stored in the caught exception.
    //
    // \sa reason() \sa inner()
    //
    // \param inner specifies the exception caught.
    // \param reason specifies the reason why the exception was thrown.
    //
    HMissingArgumentException(const HException& inner, const QString& reason);

    //
    // Destroys the instance.
    //
    virtual ~HMissingArgumentException() throw();
    virtual HMissingArgumentException* clone() const;
};

//
// An exception class used in situations where an operation could not
// be successfully completed for some reason.
//
class HOperationFailedException :
    public HException
{
public:

    //
    // Creates a new, empty instance with no description.
    //
    HOperationFailedException();

    //
    // Creates a new instance with the specified error string.
    // \param reason specifies the reason why the exception was thrown.
    //
    // \remark The reason is a free description and it is meant for humans only.
    //
    explicit HOperationFailedException(const QString& reason);

    //
    // Creates a new instance specifying another exception that was caught
    // and a reason detailing why this instance was thrown.
    //
    // This is useful in situations where you have caught an exception, but want
    // to throw another exception of different type of description, yet without
    // losing the information stored in the caught exception.
    //
    // \sa reason() \sa inner()
    //
    // \param inner specifies the exception caught.
    // \param reason specifies the reason why the exception was thrown.
    //
    HOperationFailedException(const HException& inner, const QString& reason);

    //
    // Destroys the instance.
    //
    virtual ~HOperationFailedException() throw();
    virtual HOperationFailedException* clone() const;
};

//
// An exception class used in situations where an operation could not
// be successfully completed due to timeout.
//
class HTimeoutException :
    public HOperationFailedException
{
public:

    //
    // Creates a new, empty instance with no description.
    //
    HTimeoutException();

    //
    // Creates a new instance with the specified error string.
    // \param reason specifies the reason why the exception was thrown.
    //
    // \remark The reason is a free description and it is meant for humans only.
    //
    explicit HTimeoutException(const QString& reason);

    //
    // Creates a new instance specifying another exception that was caught
    // and a reason detailing why this instance was thrown.
    //
    // This is useful in situations where you have caught an exception, but want
    // to throw another exception of different type of description, yet without
    // losing the information stored in the caught exception.
    //
    // \sa reason() \sa inner()
    //
    // \param inner specifies the exception caught.
    // \param reason specifies the reason why the exception was thrown.
    //
    HTimeoutException(const HException& inner, const QString& reason);

    //
    // Destroys the instance.
    //
    virtual ~HTimeoutException() throw();
    virtual HTimeoutException* clone() const;
};

//
// An exception class used in situations where an I/O operation could not
// be successfully completed.
//
class HIoException :
    public HOperationFailedException
{
public:

    //
    // Creates a new, empty instance with no description.
    //
    HIoException();

    //
    // Creates a new instance with the specified error string.
    // \param reason specifies the reason why the exception was thrown.
    //
    // \remark The reason is a free description and it is meant for humans only.
    //
    explicit HIoException(const QString& reason);

    //
    // Creates a new instance specifying another exception that was caught
    // and a reason detailing why this instance was thrown.
    //
    // This is useful in situations where you have caught an exception, but want
    // to throw another exception of different type of description, yet without
    // losing the information stored in the caught exception.
    //
    // \sa reason() \sa inner()
    //
    // \param inner specifies the exception caught.
    // \param reason specifies the reason why the exception was thrown.
    //
    HIoException(const HException& inner, const QString& reason);

    //
    // Destroys the instance.
    //
    virtual ~HIoException() throw();
    virtual HIoException* clone() const;
};

//
// An exception class used in situations where an operation was aborted due
// to initiated shutdown of an entity crucial to completing the operation.
//
class HShutdownInProgressException :
    public HOperationFailedException
{
public:

    //
    // Creates a new, empty instance with no description.
    //
    HShutdownInProgressException();

    //
    // Creates a new instance with the specified error string.
    // \param reason specifies the reason why the exception was thrown.
    //
    // \remark The reason is a free description and it is meant for humans only.
    //
    explicit HShutdownInProgressException(const QString& reason);

    //
    // Creates a new instance specifying another exception that was caught
    // and a reason detailing why this instance was thrown.
    //
    // This is useful in situations where you have caught an exception, but want
    // to throw another exception of different type of description, yet without
    // losing the information stored in the caught exception.
    //
    // \sa reason() \sa inner()
    //
    // \param inner specifies the exception caught.
    // \param reason specifies the reason why the exception was thrown.
    //
    HShutdownInProgressException(const HException& inner, const QString& reason);

    //
    // Destroys the instance.
    //
    virtual ~HShutdownInProgressException() throw();
    virtual HShutdownInProgressException* clone() const;
};

class HSocketExceptionPrivate;

//
// An exception class used when an operation failed due to a socket issue.
//
class HSocketException :
    public HException
{
public:

    //
    // Creates a new, empty instance with no description.
    //
    HSocketException();

    //
    // Creates a new instance with the specified error string.
    // \param reason specifies the reason why the exception was thrown.
    //
    // \remark The reason is a free description and it is meant for humans only.
    //
    explicit HSocketException(const QString& reason);

    //
    // Creates a new instance specifying another exception that was caught
    // and a reason detailing why this instance was thrown.
    //
    // This is useful in situations where you have caught an exception, but want
    // to throw another exception of different type of description, yet without
    // losing the information stored in the caught exception.
    //
    // \sa reason() \sa inner()
    //
    // \param inner specifies the exception caught.
    // \param reason specifies the reason why the exception was thrown.
    //
    HSocketException(const HException& inner, const QString& reason);

    //
    // Destroys the instance.
    //
    virtual ~HSocketException() throw();
    virtual HSocketException* clone() const;
};

//
// An exception class used when a parse operation of any type of document or
// data failed due to the document or data not following a specified format.
//
class HParseException :
    public HException
{
public:

    //
    // Creates a new, empty instance with no description.
    //
    HParseException();

    //
    // Creates a new instance with the specified error string.
    //
    // \param reason specifies the reason why the exception was thrown.
    //
    // \remark The reason is a free description and it is meant for humans only.
    //
    explicit HParseException(const QString& reason);

    //
    // Creates a new instance specifying another exception that was caught
    // and a reason detailing why this instance was thrown.
    //
    // This is useful in situations where you have caught an exception, but want
    // to throw another exception of different type of description, yet without
    // losing the information stored in the caught exception.
    //
    // \sa reason() \sa inner()
    //
    // \param inner specifies the exception caught.
    // \param reason specifies the reason why the exception was thrown.
    //
    HParseException(const HException& inner, const QString& reason);

    //
    // Destroys the instance.
    //
    virtual ~HParseException() throw ();
    virtual HParseException* clone() const;
};

}

#endif // EXCEPTIONS_P_H_
