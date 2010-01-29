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

#ifndef EXCEPTIONS_H_
#define EXCEPTIONS_H_

#include "hglobal.h"
#include <exception>

class QString;

namespace Herqq
{

class HExceptionPrivate;

/*!
 * A base class for all exception classes used in the Herqq libraries.
 *
 * \ingroup exceptions
 */
class H_CORE_EXPORT HException :
    public std::exception
{
H_DECLARE_PRIVATE(HException)

protected:

    HExceptionPrivate* h_ptr;
    HException(HExceptionPrivate& dd);

public:

    /*!
     * Creates a new, empty instance with no description.
     */
    HException();

    /*!
     * Copy constructor.
     */
    HException(const HException&);

    /*!
     * Creates a new instance with the specified error string.
     *
     * \param reason specifies the reason why the exception was thrown.
     *
     * \remark The reason is a free description and it is meant for humans only.
     */
    explicit HException(const QString& reason);

    /*!
     * Creates a new instance specifying another exception that was caught
     * and a reason detailing why this instance was thrown.
     *
     * This is useful in situations where you have caught an exception, but want
     * to throw another exception of different type of description, yet without
     * losing the information stored in the caught exception.
     *
     * \param inner specifies the exception caught.
     * \param reason specifies the reason why the exception was thrown.
     *
     * \sa reason() \sa inner()
     */
    HException(const HException& inner, const QString& reason);

    /*!
     * Destroys the instance.
     */
    virtual ~HException() throw ();

    /*!
     * Assignment operator.
     */
    virtual HException& operator=(const HException&);

    /*!
     * Returns full description of the exception.
     *
     * \return full description of the exception. This is an override of
     * std::exception::what() and it is equal to call reason(true).
     */
    virtual const char* what() const throw();

    /*!
     * Returns a clone of the exception.
     *
     * \return a clone of the exception.
     *
     * \remark the ownership of the object is transferred to the caller.
     */
    virtual HException* clone() const = 0;

    /*!
     * Returns the inner exception stored, if any.
     *
     * \return the inner exception stored, if any. In case the exception has no
     * inner exception chained, null is returned.
     */
    const HException* inner() const;

    /*!
     * Returns the reason -if specified- detailing why the exception was thrown.
     *
     * The reason is a free description and it is meant for humans.
     *
     * \return the reason -if specified- detailing why the exception was thrown.
     */
    QString reason(bool includeInner = true) const;
};

class HInitializationExceptionPrivate;

/*!
 * An exception class used when some type of initialization,
 * such as object construction, fails.
 *
 * \ingroup exceptions
 */
class H_CORE_EXPORT HInitializationException :
    public HException
{
H_DECLARE_PRIVATE(HInitializationException)

protected:

    HInitializationException(HInitializationExceptionPrivate& dd);

public:

    /*!
     * Creates a new, empty instance with no description.
     */
    HInitializationException();

    /*!
     * Creates a new instance with the specified error string.
     *
     * \param reason specifies the reason why the exception was thrown.
     *
     * \remark The reason is a free description and it is meant for humans only.
     */
    explicit HInitializationException(const QString& reason);

    /*!
     * Creates a new instance specifying another exception that was caught
     * and a reason detailing why this instance was thrown.
     *
     * This is useful in situations where you have caught an exception, but want
     * to throw another exception of different type of description, yet without
     * losing the information stored in the caught exception.
     *
     * \sa reason() \sa inner()
     *
     * \param inner specifies the exception caught.
     * \param reason specifies the reason why the exception was thrown.
     */
    HInitializationException(const HException& inner, const QString& reason);

    /*!
     * Destroys the instance.
     */
    virtual ~HInitializationException() throw();
    virtual HInitializationException* clone() const;
};


class HArgumentExceptionPrivate;

/*!
 * An exception class used to indicate some type of an error relating to
 * an argument of any type.
 *
 * \ingroup exceptions
 */
class H_CORE_EXPORT HArgumentException :
    public HException
{
H_DECLARE_PRIVATE(HArgumentException)

protected:

    HArgumentException(HArgumentExceptionPrivate& dd);

public:

    /*!
     * Creates a new, empty instance with no description.
     */
    HArgumentException();

    //HArgumentException(const HArgumentException&);

    /*!
     * Creates a new instance with the specified error string.
     * \param reason specifies the reason why the exception was thrown.
     *
     * \remark The reason is a free description and it is meant for humans only.
     */
    explicit HArgumentException(const QString& reason);

    /*!
     * Creates a new instance specifying another exception that was caught
     * and a reason detailing why this instance was thrown.
     *
     * This is useful in situations where you have caught an exception, but want
     * to throw another exception of different type of description, yet without
     * losing the information stored in the caught exception.
     *
     * \sa reason() \sa inner()
     *
     * \param inner specifies the exception caught.
     * \param reason specifies the reason why the exception was thrown.
     */
    HArgumentException(const HException& inner, const QString& reason);

    /*!
     * Destroys the instance.
     */
    virtual ~HArgumentException() throw();
    virtual HArgumentException* clone() const;
};

class HIllegalArgumentExceptionPrivate;

/*!
 * An exception class used in situations where an invalid argument has been
 * provided to a method.
 *
 * \ingroup exceptions
 */
class H_CORE_EXPORT HIllegalArgumentException :
    public HArgumentException
{
H_DECLARE_PRIVATE(HIllegalArgumentException)

protected:

    HIllegalArgumentException(HIllegalArgumentExceptionPrivate& dd);

public:

    /*!
     * Creates a new, empty instance with no description.
     */
    HIllegalArgumentException();
    //HIllegalArgumentException(const HIllegalArgumentException&);

    /*!
     * Creates a new instance with the specified error string.
     * \param reason specifies the reason why the exception was thrown.
     *
     * \remark The reason is a free description and it is meant for humans only.
     */
    explicit HIllegalArgumentException(const QString& reason);

    /*!
     * Creates a new instance specifying another exception that was caught
     * and a reason detailing why this instance was thrown.
     *
     * This is useful in situations where you have caught an exception, but want
     * to throw another exception of different type of description, yet without
     * losing the information stored in the caught exception.
     *
     * \sa reason() \sa inner()
     *
     * \param inner specifies the exception caught.
     * \param reason specifies the reason why the exception was thrown.
     */
    HIllegalArgumentException(const HException& inner, const QString& reason);

    /*!
     * Destroys the instance.
     */
    virtual ~HIllegalArgumentException() throw();
    virtual HIllegalArgumentException* clone() const;
};

class HMissingArgumentExceptionPrivate;

/*!
 * An exception class used in situations where processing cannot continue
 * due to a missing argument.
 *
 * \ingroup exceptions
 */
class H_CORE_EXPORT HMissingArgumentException :
    public HArgumentException
{
H_DECLARE_PRIVATE(HMissingArgumentException)

protected:

    HMissingArgumentException(HMissingArgumentExceptionPrivate& dd);

public:

    /*!
     * Creates a new, empty instance with no description.
     */
    HMissingArgumentException();
    //HMissingArgumentException(const HMissingArgumentException&);

    /*!
     * Creates a new instance with the specified error string.
     * \param reason specifies the reason why the exception was thrown.
     *
     * \remark The reason is a free description and it is meant for humans only.
     */
    explicit HMissingArgumentException(const QString& reason);

    /*!
     * Creates a new instance specifying another exception that was caught
     * and a reason detailing why this instance was thrown.
     *
     * This is useful in situations where you have caught an exception, but want
     * to throw another exception of different type of description, yet without
     * losing the information stored in the caught exception.
     *
     * \sa reason() \sa inner()
     *
     * \param inner specifies the exception caught.
     * \param reason specifies the reason why the exception was thrown.
     */
    HMissingArgumentException(const HException& inner, const QString& reason);

    /*!
     * Destroys the instance.
     */
    virtual ~HMissingArgumentException() throw();
    virtual HMissingArgumentException* clone() const;
};

class HOperationFailedExceptionPrivate;

/*!
 * An exception class used in situations where an operation could not
 * be successfully completed for some reason.
 *
 * \ingroup exceptions
 */
class H_CORE_EXPORT HOperationFailedException :
    public HException
{
H_DECLARE_PRIVATE(HOperationFailedException)

protected:

    HOperationFailedException(HOperationFailedExceptionPrivate& dd);

public:

    /*!
     * Creates a new, empty instance with no description.
     */
    HOperationFailedException();
    //HOperationFailedException(const HOperationFailedException&);

    /*!
     * Creates a new instance with the specified error string.
     * \param reason specifies the reason why the exception was thrown.
     *
     * \remark The reason is a free description and it is meant for humans only.
     */
    explicit HOperationFailedException(const QString& reason);

    /*!
     * Creates a new instance specifying another exception that was caught
     * and a reason detailing why this instance was thrown.
     *
     * This is useful in situations where you have caught an exception, but want
     * to throw another exception of different type of description, yet without
     * losing the information stored in the caught exception.
     *
     * \sa reason() \sa inner()
     *
     * \param inner specifies the exception caught.
     * \param reason specifies the reason why the exception was thrown.
     */
    HOperationFailedException(const HException& inner, const QString& reason);

    /*!
     * Destroys the instance.
     */
    virtual ~HOperationFailedException() throw();
    virtual HOperationFailedException* clone() const;
};

class HTimeoutExceptionPrivate;

/*!
 * An exception class used in situations where an operation could not
 * be successfully completed due to timeout.
 *
 * \ingroup exceptions
 */
class H_CORE_EXPORT HTimeoutException :
    public HOperationFailedException
{
H_DECLARE_PRIVATE(HTimeoutException)

protected:

    HTimeoutException(HTimeoutExceptionPrivate& dd);

public:

    /*!
     * Creates a new, empty instance with no description.
     */
    HTimeoutException();
    //HTimeoutException(const HTimeoutException&);

    /*!
     * Creates a new instance with the specified error string.
     * \param reason specifies the reason why the exception was thrown.
     *
     * \remark The reason is a free description and it is meant for humans only.
     */
    explicit HTimeoutException(const QString& reason);

    /*!
     * Creates a new instance specifying another exception that was caught
     * and a reason detailing why this instance was thrown.
     *
     * This is useful in situations where you have caught an exception, but want
     * to throw another exception of different type of description, yet without
     * losing the information stored in the caught exception.
     *
     * \sa reason() \sa inner()
     *
     * \param inner specifies the exception caught.
     * \param reason specifies the reason why the exception was thrown.
     */
    HTimeoutException(const HException& inner, const QString& reason);

    /*!
     * Destroys the instance.
     */
    virtual ~HTimeoutException() throw();
    virtual HTimeoutException* clone() const;
};

class HIoExceptionPrivate;

/*!
 * An exception class used in situations where an I/O operation could not
 * be successfully completed.
 *
 * \ingroup exceptions
 */
class H_CORE_EXPORT HIoException :
    public HOperationFailedException
{
H_DECLARE_PRIVATE(HIoException)

protected:

    HIoException(HIoExceptionPrivate& dd);

public:

    /*!
     * Creates a new, empty instance with no description.
     */
    HIoException();

    /*!
     * Creates a new instance with the specified error string.
     * \param reason specifies the reason why the exception was thrown.
     *
     * \remark The reason is a free description and it is meant for humans only.
     */
    explicit HIoException(const QString& reason);

    /*!
     * Creates a new instance specifying another exception that was caught
     * and a reason detailing why this instance was thrown.
     *
     * This is useful in situations where you have caught an exception, but want
     * to throw another exception of different type of description, yet without
     * losing the information stored in the caught exception.
     *
     * \sa reason() \sa inner()
     *
     * \param inner specifies the exception caught.
     * \param reason specifies the reason why the exception was thrown.
     */
    HIoException(const HException& inner, const QString& reason);

    /*!
     * Destroys the instance.
     */
    virtual ~HIoException() throw();
    virtual HIoException* clone() const;
};

class HShutdownInProgressExceptionPrivate;

/*!
 * An exception class used in situations where an operation was aborted due
 * to initiated shutdown of an entity crucial to completing the operation.
 *
 * \ingroup exceptions
 */
class H_CORE_EXPORT HShutdownInProgressException :
    public HOperationFailedException
{
H_DECLARE_PRIVATE(HShutdownInProgressException)

protected:

    HShutdownInProgressException(HShutdownInProgressExceptionPrivate& dd);

public:

    /*!
     * Creates a new, empty instance with no description.
     */
    HShutdownInProgressException();
    //HShutdownInProgressException(const HShutdownInProgressException&);

    /*!
     * Creates a new instance with the specified error string.
     * \param reason specifies the reason why the exception was thrown.
     *
     * \remark The reason is a free description and it is meant for humans only.
     */
    explicit HShutdownInProgressException(const QString& reason);

    /*!
     * Creates a new instance specifying another exception that was caught
     * and a reason detailing why this instance was thrown.
     *
     * This is useful in situations where you have caught an exception, but want
     * to throw another exception of different type of description, yet without
     * losing the information stored in the caught exception.
     *
     * \sa reason() \sa inner()
     *
     * \param inner specifies the exception caught.
     * \param reason specifies the reason why the exception was thrown.
     */
    HShutdownInProgressException(const HException& inner, const QString& reason);

    /*!
     * Destroys the instance.
     */
    virtual ~HShutdownInProgressException() throw();
    virtual HShutdownInProgressException* clone() const;
};

class HSocketExceptionPrivate;

/*!
 * An exception class used when an operation failed due to a socket issue.
 *
 * \ingroup exceptions
 */
class H_CORE_EXPORT HSocketException :
    public HException
{
H_DECLARE_PRIVATE(HSocketException)

protected:

    HSocketException(HSocketExceptionPrivate& dd);

public:

    /*!
     * Creates a new, empty instance with no description.
     */
    HSocketException();
    //HSocketException(const HSocketException&);

    /*!
     * Creates a new instance with the specified error string.
     * \param reason specifies the reason why the exception was thrown.
     *
     * \remark The reason is a free description and it is meant for humans only.
     */
    explicit HSocketException(const QString& reason);

    /*!
     * Creates a new instance specifying another exception that was caught
     * and a reason detailing why this instance was thrown.
     *
     * This is useful in situations where you have caught an exception, but want
     * to throw another exception of different type of description, yet without
     * losing the information stored in the caught exception.
     *
     * \sa reason() \sa inner()
     *
     * \param inner specifies the exception caught.
     * \param reason specifies the reason why the exception was thrown.
     */
    HSocketException(const HException& inner, const QString& reason);

    /*!
     * Destroys the instance.
     */
    virtual ~HSocketException() throw();
    virtual HSocketException* clone() const;
};

class HParseExceptionPrivate;

/*!
 * An exception class used when a parse operation of any type of document or
 * data failed due to the document or data not following a specified format.
 *
 * \ingroup exceptions
 */
class H_CORE_EXPORT HParseException :
    public HException
{
H_DECLARE_PRIVATE(HParseException)

protected:

    HParseException(HParseExceptionPrivate& dd);

public:

    /*!
     * Creates a new, empty instance with no description.
     */
    HParseException();
    //HParseException(const HParseException&);

    /*!
     * Creates a new instance with the specified error string.
     *
     * \param reason specifies the reason why the exception was thrown.
     *
     * \remark The reason is a free description and it is meant for humans only.
     */
    explicit HParseException(const QString& reason);

    /*!
     * Creates a new instance specifying another exception that was caught
     * and a reason detailing why this instance was thrown.
     *
     * This is useful in situations where you have caught an exception, but want
     * to throw another exception of different type of description, yet without
     * losing the information stored in the caught exception.
     *
     * \sa reason() \sa inner()
     *
     * \param inner specifies the exception caught.
     * \param reason specifies the reason why the exception was thrown.
     */
    HParseException(const HException& inner, const QString& reason);

    /*!
     * Destroys the instance.
     */
    virtual ~HParseException() throw ();
    virtual HParseException* clone() const;
};

}

#endif /* EXCEPTIONS_H_ */
