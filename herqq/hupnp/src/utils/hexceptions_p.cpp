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

#include "hexceptions_p.h"
#include "hlogger_p.h"

namespace Herqq
{

/*******************************************************************************
 * HException
 *******************************************************************************/
HException::HException() :
    std::exception(), m_reason(), m_inner(0)
{
    HLOG(H_AT, H_FUN);
}

HException::HException(const HException& other) :
    std::exception(),
        m_reason(other.m_reason),
        m_inner(other.m_inner.isNull() ? 0 : other.m_inner->clone())
{
    HLOG(H_AT, H_FUN);
}

HException::HException(const QString& reason) :
    m_reason(reason), m_inner(0)
{
    HLOG(H_AT, H_FUN);
}

HException::HException(const HException& inner, const QString& reason) :
    m_reason(reason), m_inner(inner.clone())
{
    HLOG(H_AT, H_FUN);
}

HException& HException::operator=(const HException& other)
{
    HLOG(H_AT, H_FUN);
    m_reason = other.m_reason;
    m_inner.reset(other.m_inner.isNull() ? 0 : other.m_inner->clone());

    return *this;
}

HException::~HException() throw()
{
    HLOG(H_AT, H_FUN);
}

const HException* HException::inner() const
{
    HLOG(H_AT, H_FUN);
    return m_inner.data();
}

const char* HException::what() const throw()
{
    HLOG(H_AT, H_FUN);
    return reason(true).toAscii().data();
}

QString HException::reason(bool includeInner) const
{
    HLOG(H_AT, H_FUN);
    if (!m_inner || !includeInner)
    {
        return m_reason;
    }

    return QString(m_reason).append(":\n").append(m_inner->reason(includeInner));
}

/*******************************************************************************
 * HInitializationException
 *******************************************************************************/
HInitializationException::HInitializationException()
{
    HLOG(H_AT, H_FUN);
}

HInitializationException::HInitializationException(const QString& reason) :
    HException(reason)
{
    HLOG(H_AT, H_FUN);
}

HInitializationException::HInitializationException(
    const HException& inner, const QString& reason) :
        HException(inner, reason)
{
    HLOG(H_AT, H_FUN);
}

HInitializationException::~HInitializationException() throw()
{
    HLOG(H_AT, H_FUN);
}

HInitializationException* HInitializationException::clone() const
{
    HLOG(H_AT, H_FUN);
    return new HInitializationException(*this);
}

/*******************************************************************************
 * HArgumentException
 *******************************************************************************/
HArgumentException::HArgumentException()
{
    HLOG(H_AT, H_FUN);
}

HArgumentException::HArgumentException(const QString& reason) :
    HException(reason)
{
    HLOG(H_AT, H_FUN);
}

HArgumentException::HArgumentException(
    const HException& inner, const QString& reason) :
        HException(inner, reason)
{
    HLOG(H_AT, H_FUN);
}

HArgumentException::~HArgumentException() throw()
{
    HLOG(H_AT, H_FUN);
}

HArgumentException* HArgumentException::clone() const
{
    HLOG(H_AT, H_FUN);
    return new HArgumentException(*this);
}

/*******************************************************************************
 * HIllegalArgumentException
 *******************************************************************************/
HIllegalArgumentException::HIllegalArgumentException()
{
    HLOG(H_AT, H_FUN);
}

HIllegalArgumentException::HIllegalArgumentException(const QString& reason) :
    HArgumentException(reason)
{
    HLOG(H_AT, H_FUN);
}

HIllegalArgumentException::HIllegalArgumentException(
    const HException& inner, const QString& reason) :
        HArgumentException(inner, reason)
{
    HLOG(H_AT, H_FUN);
}

HIllegalArgumentException::~HIllegalArgumentException() throw()
{
    HLOG(H_AT, H_FUN);
}

HIllegalArgumentException* HIllegalArgumentException::clone() const
{
    HLOG(H_AT, H_FUN);
    return new HIllegalArgumentException(*this);
}

/*******************************************************************************
 * HMissingArgumentException
 *******************************************************************************/
HMissingArgumentException::HMissingArgumentException()
{
    HLOG(H_AT, H_FUN);
}

HMissingArgumentException::HMissingArgumentException(const QString& reason) :
    HArgumentException(reason)
{
    HLOG(H_AT, H_FUN);
}

HMissingArgumentException::HMissingArgumentException(
    const HException& inner, const QString& reason) :
        HArgumentException(inner, reason)
{
    HLOG(H_AT, H_FUN);
}

HMissingArgumentException::~HMissingArgumentException() throw()
{
    HLOG(H_AT, H_FUN);
}

HMissingArgumentException* HMissingArgumentException::clone() const
{
    HLOG(H_AT, H_FUN);
    return new HMissingArgumentException(*this);
}

/*******************************************************************************
 * HOperationFailedException
 *******************************************************************************/
HOperationFailedException::HOperationFailedException()
{
    HLOG(H_AT, H_FUN);
}

HOperationFailedException::HOperationFailedException(const QString& reason) :
    HException(reason)
{
    HLOG(H_AT, H_FUN);
}

HOperationFailedException::HOperationFailedException(
    const HException& inner, const QString& reason) :
        HException(inner, reason)
{
    HLOG(H_AT, H_FUN);
}

HOperationFailedException::~HOperationFailedException() throw()
{
    HLOG(H_AT, H_FUN);
}

HOperationFailedException* HOperationFailedException::clone() const
{
    HLOG(H_AT, H_FUN);
    return new HOperationFailedException(*this);
}

/*******************************************************************************
 * HTimeoutException
 *******************************************************************************/
HTimeoutException::HTimeoutException()
{
    HLOG(H_AT, H_FUN);
}

HTimeoutException::HTimeoutException(const QString& reason) :
    HOperationFailedException(reason)
{
    HLOG(H_AT, H_FUN);
}

HTimeoutException::HTimeoutException(
    const HException& inner, const QString& reason) :
        HOperationFailedException(inner, reason)
{
    HLOG(H_AT, H_FUN);
}

HTimeoutException::~HTimeoutException() throw()
{
    HLOG(H_AT, H_FUN);
}

HTimeoutException* HTimeoutException::clone() const
{
    HLOG(H_AT, H_FUN);
    return new HTimeoutException(*this);
}

/*******************************************************************************
 * HIoException
 *******************************************************************************/
HIoException::HIoException()
{
    HLOG(H_AT, H_FUN);
}

HIoException::HIoException(const QString& reason) :
    HOperationFailedException(reason)
{
    HLOG(H_AT, H_FUN);
}

HIoException::HIoException(
    const HException& inner, const QString& reason) :
        HOperationFailedException(inner, reason)
{
    HLOG(H_AT, H_FUN);
}

HIoException::~HIoException() throw()
{
    HLOG(H_AT, H_FUN);
}

HIoException* HIoException::clone() const
{
    HLOG(H_AT, H_FUN);
    return new HIoException(*this);
}

/*******************************************************************************
 * HShutdownInProgressException
 *******************************************************************************/
HShutdownInProgressException::HShutdownInProgressException()
{
    HLOG(H_AT, H_FUN);
}

HShutdownInProgressException::HShutdownInProgressException(const QString& reason) :
    HOperationFailedException(reason)
{
    HLOG(H_AT, H_FUN);
}

HShutdownInProgressException::HShutdownInProgressException(
    const HException& inner, const QString& reason) :
        HOperationFailedException(inner, reason)
{
    HLOG(H_AT, H_FUN);
}

HShutdownInProgressException::~HShutdownInProgressException() throw()
{
    HLOG(H_AT, H_FUN);
}

HShutdownInProgressException* HShutdownInProgressException::clone() const
{
    HLOG(H_AT, H_FUN);
    return new HShutdownInProgressException(*this);
}

/*******************************************************************************
 * HSocketException
 *******************************************************************************/
HSocketException::HSocketException()
{
    HLOG(H_AT, H_FUN);
}

HSocketException::HSocketException(const QString& reason) :
    HException(reason)
{
    HLOG(H_AT, H_FUN);
}

HSocketException::HSocketException(
    const HException& inner, const QString& reason) :
        HException(inner, reason)
{
    HLOG(H_AT, H_FUN);
}

HSocketException::~HSocketException() throw()
{
    HLOG(H_AT, H_FUN);
}

HSocketException* HSocketException::clone() const
{
    HLOG(H_AT, H_FUN);
    return new HSocketException(*this);
}

/*******************************************************************************
 * HParseException
 *******************************************************************************/
HParseException::HParseException()
{
    HLOG(H_AT, H_FUN);
}

HParseException::HParseException(const QString& reason) :
    HException(reason)
{
    HLOG(H_AT, H_FUN);
}

HParseException::HParseException(
    const HException& inner, const QString& reason) :
        HException(inner, reason)
{
    HLOG(H_AT, H_FUN);
}

HParseException::~HParseException() throw()
{
    HLOG(H_AT, H_FUN);
}

HParseException* HParseException::clone() const
{
    HLOG(H_AT, H_FUN);
    return new HParseException(*this);
}

}
