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

#include "exceptions.h"
#include "exceptions_p.h"

using std::exception;

namespace Herqq
{

/*!
 * \defgroup exceptions Exceptions
 *
 */

/*******************************************************************************
 * HExceptionPrivate
 *******************************************************************************/
HExceptionPrivate::HExceptionPrivate() :
    m_reason(), m_inner(0)
{
}

HExceptionPrivate::HExceptionPrivate(const QString& reason) :
    m_reason(reason), m_inner(0)
{
}

HExceptionPrivate::HExceptionPrivate(
    const HException& inner, const QString& reason) :
        m_reason(reason), m_inner(inner.clone())
{
}

HExceptionPrivate::~HExceptionPrivate()
{
}

HExceptionPrivate* HExceptionPrivate::clone() const
{
    return new HExceptionPrivate(m_reason);
}

/*******************************************************************************
 * HInitializationExceptionPrivate
 *******************************************************************************/
HInitializationExceptionPrivate::HInitializationExceptionPrivate()
{
}

HInitializationExceptionPrivate::HInitializationExceptionPrivate(
    const QString& reason) :
        HExceptionPrivate(reason)
{
}

HInitializationExceptionPrivate::~HInitializationExceptionPrivate()
{
}

HInitializationExceptionPrivate* HInitializationExceptionPrivate::clone() const
{
    return new HInitializationExceptionPrivate(m_reason);
}

/*******************************************************************************
 * HArgumentExceptionPrivate
 *******************************************************************************/
HArgumentExceptionPrivate::HArgumentExceptionPrivate()
{
}

HArgumentExceptionPrivate::HArgumentExceptionPrivate(
    const QString& reason) :
        HExceptionPrivate(reason)
{
}

HArgumentExceptionPrivate::~HArgumentExceptionPrivate()
{
}

HArgumentExceptionPrivate* HArgumentExceptionPrivate::clone() const
{
    return new HArgumentExceptionPrivate(m_reason);
}

/*******************************************************************************
 * HIllegalArgumentExceptionPrivate
 *******************************************************************************/
HIllegalArgumentExceptionPrivate::HIllegalArgumentExceptionPrivate()
{
}

HIllegalArgumentExceptionPrivate::HIllegalArgumentExceptionPrivate(
    const QString& reason) :
        HArgumentExceptionPrivate(reason)
{
}

HIllegalArgumentExceptionPrivate::~HIllegalArgumentExceptionPrivate()
{
}

HIllegalArgumentExceptionPrivate* HIllegalArgumentExceptionPrivate::clone() const
{
    return new HIllegalArgumentExceptionPrivate(m_reason);
}

/*******************************************************************************
 * HMissingArgumentExceptionPrivate
 *******************************************************************************/
HMissingArgumentExceptionPrivate::HMissingArgumentExceptionPrivate()
{
}

HMissingArgumentExceptionPrivate::HMissingArgumentExceptionPrivate(
    const QString& reason) :
        HArgumentExceptionPrivate(reason)
{
}

HMissingArgumentExceptionPrivate::~HMissingArgumentExceptionPrivate()
{
}

HMissingArgumentExceptionPrivate* HMissingArgumentExceptionPrivate::clone() const
{
    return new HMissingArgumentExceptionPrivate(m_reason);
}

HOperationFailedExceptionPrivate::HOperationFailedExceptionPrivate()
{
}

/*******************************************************************************
 * HOperationFailedExceptionPrivate
 *******************************************************************************/
HOperationFailedExceptionPrivate::HOperationFailedExceptionPrivate(const QString& reason) :
    HExceptionPrivate(reason)
{
}

HOperationFailedExceptionPrivate::~HOperationFailedExceptionPrivate()
{
}

HOperationFailedExceptionPrivate* HOperationFailedExceptionPrivate::clone() const
{
    return new HOperationFailedExceptionPrivate(m_reason);
}

/*******************************************************************************
 * HShutdownInProgressExceptionPrivate
 *******************************************************************************/
HShutdownInProgressExceptionPrivate::HShutdownInProgressExceptionPrivate()
{
}

HShutdownInProgressExceptionPrivate::HShutdownInProgressExceptionPrivate(
    const QString& reason) :
        HOperationFailedExceptionPrivate(reason)
{
}

HShutdownInProgressExceptionPrivate::~HShutdownInProgressExceptionPrivate()
{
}

HShutdownInProgressExceptionPrivate* HShutdownInProgressExceptionPrivate::clone() const
{
    return new HShutdownInProgressExceptionPrivate(m_reason);
}

/*******************************************************************************
 * HTimeoutExceptionPrivate
 *******************************************************************************/
HTimeoutExceptionPrivate::HTimeoutExceptionPrivate()
{
}

HTimeoutExceptionPrivate::HTimeoutExceptionPrivate(
    const QString& reason) :
        HOperationFailedExceptionPrivate(reason)
{
}

HTimeoutExceptionPrivate::~HTimeoutExceptionPrivate()
{
}

HTimeoutExceptionPrivate* HTimeoutExceptionPrivate::clone() const
{
    return new HTimeoutExceptionPrivate(m_reason);
}

/*******************************************************************************
 * HIoExceptionPrivate
 *******************************************************************************/
HIoExceptionPrivate::HIoExceptionPrivate()
{
}

HIoExceptionPrivate::HIoExceptionPrivate(
    const QString& reason) :
        HOperationFailedExceptionPrivate(reason)
{
}

HIoExceptionPrivate::~HIoExceptionPrivate()
{
}

HIoExceptionPrivate* HIoExceptionPrivate::clone() const
{
    return new HIoExceptionPrivate(m_reason);
}

/*******************************************************************************
 * HExceptionPrivate
 *******************************************************************************/
HSocketExceptionPrivate::HSocketExceptionPrivate()
{
}

HSocketExceptionPrivate::HSocketExceptionPrivate(const QString& reason) :
    HExceptionPrivate(reason)
{
}

HSocketExceptionPrivate::~HSocketExceptionPrivate()
{
}

HSocketExceptionPrivate* HSocketExceptionPrivate::clone() const
{
    return new HSocketExceptionPrivate(m_reason);
}

/*******************************************************************************
 * HExceptionPrivate
 *******************************************************************************/
HParseExceptionPrivate::HParseExceptionPrivate()
{
}

HParseExceptionPrivate::HParseExceptionPrivate(const QString& reason) :
    HExceptionPrivate(reason)
{
}

HParseExceptionPrivate::~HParseExceptionPrivate()
{
}

HParseExceptionPrivate* HParseExceptionPrivate::clone() const
{
    return new HParseExceptionPrivate(m_reason);
}

/*******************************************************************************
 * HException
 *******************************************************************************/
HException::HException() :
    h_ptr(new HExceptionPrivate())
{
}

HException::HException(const HException& other) :
    std::exception(), h_ptr(other.h_ptr->clone())
{
}

HException::HException(HExceptionPrivate& dd) :
    h_ptr(&dd)
{
}

HException::HException(const QString& reason) :
    h_ptr(new HExceptionPrivate(reason))
{
}

HException::HException(const HException& inner, const QString& reason) :
    h_ptr(new HExceptionPrivate(inner, reason))
{
}

HException& HException::operator=(const HException& other)
{
    HExceptionPrivate* newHptr = other.h_ptr->clone();

    delete h_ptr;
    h_ptr = newHptr;

    return *this;
}

HException::~HException() throw()
{
    delete h_ptr;
}

const HException* HException::inner() const
{
    return h_ptr->m_inner.data();
}

const char* HException::what() const throw()
{
    return reason(true).toAscii().data();
}

QString HException::reason(bool includeInner) const
{
    if (!h_ptr->m_inner || !includeInner)
    {
        return h_ptr->m_reason;
    }

    return h_ptr->m_reason.append(
        ":\n").append(h_ptr->m_inner->reason(includeInner));
}

/*******************************************************************************
 * HInitializationException
 *******************************************************************************/
HInitializationException::HInitializationException() :
    HException(*new HInitializationExceptionPrivate())
{
}

HInitializationException::HInitializationException(
    HInitializationExceptionPrivate& dd) :
        HException(dd)
{
}

HInitializationException::HInitializationException(const QString& reason) :
    HException(reason)
{
}

HInitializationException::HInitializationException(
    const HException& inner, const QString& reason) :
        HException(inner, reason)
{
}

HInitializationException::~HInitializationException() throw()
{
}

HInitializationException* HInitializationException::clone() const
{
    const H_D(HInitializationException);
    return new HInitializationException(*h->clone());
}

/*******************************************************************************
 * HArgumentException
 *******************************************************************************/
HArgumentException::HArgumentException() :
    HException(*new HArgumentExceptionPrivate())
{
}

HArgumentException::HArgumentException(HArgumentExceptionPrivate& dd) :
    HException(dd)
{
}

HArgumentException::HArgumentException(const QString& reason) :
    HException(reason)
{
}

HArgumentException::HArgumentException(
    const HException& inner, const QString& reason) :
        HException(inner, reason)
{
}

HArgumentException::~HArgumentException() throw()
{
}

HArgumentException* HArgumentException::clone() const
{
    const H_D(HArgumentException);
    return new HArgumentException(*h->clone());
}

/*******************************************************************************
 * HIllegalArgumentException
 *******************************************************************************/
HIllegalArgumentException::HIllegalArgumentException() :
    HArgumentException(*new HIllegalArgumentExceptionPrivate())
{
}

HIllegalArgumentException::HIllegalArgumentException(
    HIllegalArgumentExceptionPrivate& dd) :
        HArgumentException(dd)
{
}

HIllegalArgumentException::HIllegalArgumentException(const QString& reason) :
    HArgumentException(reason)
{
}

HIllegalArgumentException::HIllegalArgumentException(
    const HException& inner, const QString& reason) :
        HArgumentException(inner, reason)
{
}

HIllegalArgumentException::~HIllegalArgumentException() throw()
{
}

HIllegalArgumentException* HIllegalArgumentException::clone() const
{
    const H_D(HIllegalArgumentException);
    return new HIllegalArgumentException(*h->clone());
}

/*******************************************************************************
 * HMissingArgumentException
 *******************************************************************************/
HMissingArgumentException::HMissingArgumentException() :
    HArgumentException(*new HMissingArgumentExceptionPrivate())
{
}

HMissingArgumentException::HMissingArgumentException(
    HMissingArgumentExceptionPrivate& dd) :
        HArgumentException(dd)
{
}

HMissingArgumentException::HMissingArgumentException(const QString& reason) :
    HArgumentException(reason)
{
}

HMissingArgumentException::HMissingArgumentException(
    const HException& inner, const QString& reason) :
        HArgumentException(inner, reason)
{
}

HMissingArgumentException::~HMissingArgumentException() throw()
{
}

HMissingArgumentException* HMissingArgumentException::clone() const
{
    const H_D(HMissingArgumentException);
    return new HMissingArgumentException(*h->clone());
}

/*******************************************************************************
 * HOperationFailedException
 *******************************************************************************/
HOperationFailedException::HOperationFailedException() :
    HException(*new HOperationFailedExceptionPrivate())
{
}

HOperationFailedException::HOperationFailedException(
    HOperationFailedExceptionPrivate& dd) :
        HException(dd)
{
}

HOperationFailedException::HOperationFailedException(const QString& reason) :
    HException(reason)
{
}

HOperationFailedException::HOperationFailedException(
    const HException& inner, const QString& reason) :
        HException(inner, reason)
{
}

HOperationFailedException::~HOperationFailedException() throw()
{
}

HOperationFailedException* HOperationFailedException::clone() const
{
    const H_D(HOperationFailedException);
    return new HOperationFailedException(*h->clone());
}

/*******************************************************************************
 * HTimeoutException
 *******************************************************************************/
HTimeoutException::HTimeoutException() :
    HOperationFailedException(*new HTimeoutExceptionPrivate())
{
}

HTimeoutException::HTimeoutException(HTimeoutExceptionPrivate& dd) :
    HOperationFailedException(dd)
{
}

HTimeoutException::HTimeoutException(const QString& reason) :
    HOperationFailedException(reason)
{
}

HTimeoutException::HTimeoutException(
    const HException& inner, const QString& reason) :
        HOperationFailedException(inner, reason)
{
}

HTimeoutException::~HTimeoutException() throw()
{
}

HTimeoutException* HTimeoutException::clone() const
{
    const H_D(HTimeoutException);
    return new HTimeoutException(*h->clone());
}

/*******************************************************************************
 * HIoException
 *******************************************************************************/
HIoException::HIoException() :
    HOperationFailedException(*new HIoExceptionPrivate())
{
}

HIoException::HIoException(HIoExceptionPrivate& dd) :
    HOperationFailedException(dd)
{
}

HIoException::HIoException(const QString& reason) :
    HOperationFailedException(reason)
{
}

HIoException::HIoException(
    const HException& inner, const QString& reason) :
        HOperationFailedException(inner, reason)
{
}

HIoException::~HIoException() throw()
{
}

HIoException* HIoException::clone() const
{
    const H_D(HIoException);
    return new HIoException(*h->clone());
}

/*******************************************************************************
 * HShutdownInProgressException
 *******************************************************************************/
HShutdownInProgressException::HShutdownInProgressException() :
    HOperationFailedException(*new HShutdownInProgressExceptionPrivate())
{
}

HShutdownInProgressException::HShutdownInProgressException(
    HShutdownInProgressExceptionPrivate& dd) :
        HOperationFailedException(dd)
{
}

HShutdownInProgressException::HShutdownInProgressException(const QString& reason) :
    HOperationFailedException(reason)
{
}

HShutdownInProgressException::HShutdownInProgressException(
    const HException& inner, const QString& reason) :
        HOperationFailedException(inner, reason)
{
}

HShutdownInProgressException::~HShutdownInProgressException() throw()
{
}

HShutdownInProgressException* HShutdownInProgressException::clone() const
{
    const H_D(HShutdownInProgressException);
    return new HShutdownInProgressException(*h->clone());
}

/*******************************************************************************
 * HSocketException
 *******************************************************************************/
HSocketException::HSocketException() :
    HException(*new HSocketExceptionPrivate())
{
}

HSocketException::HSocketException(HSocketExceptionPrivate& dd) :
    HException(dd)
{
}

HSocketException::HSocketException(const QString& reason) :
    HException(reason)
{
}

HSocketException::HSocketException(
    const HException& inner, const QString& reason) :
        HException(inner, reason)
{
}

HSocketException::~HSocketException() throw()
{
}

HSocketException* HSocketException::clone() const
{
    const H_D(HSocketException);
    return new HSocketException(*h->clone());
}

/*******************************************************************************
 * HParseException
 *******************************************************************************/
HParseException::HParseException() :
    HException(*new HParseExceptionPrivate())
{
}

HParseException::HParseException(HParseExceptionPrivate& dd) :
    HException(dd)
{
}

HParseException::HParseException(const QString& reason) :
    HException(reason)
{
}

HParseException::HParseException(
    const HException& inner, const QString& reason) :
        HException(inner, reason)
{
}

HParseException::~HParseException() throw()
{
}

HParseException* HParseException::clone() const
{
    const H_D(HParseException);
    return new HParseException(*h->clone());
}

}
