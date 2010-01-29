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

#include "product_tokens.h"

#include "../../../../utils/src/logger_p.h"

#include <QString>
#include <QStringList>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HVersionTokenPrivate
 ******************************************************************************/
class HVersionTokenPrivate
{
public:

    qint32 m_majorVersion;
    qint32 m_minorVersion;

public:

    HVersionTokenPrivate() :
        m_majorVersion(0), m_minorVersion(0)
    {
    }

    bool init(const QString& token)
    {
        if (!token.startsWith("UPnP/"))
        {
            return false;
        }

        QString tokenTmp = token.section("UPnP/", 1);

        qint32 separatorIndex = tokenTmp.indexOf('.');
        if (separatorIndex < 0)
        {
            return false;
        }

        bool ok = false;
        qint32 majTmp = tokenTmp.left(separatorIndex).toInt(&ok);
        if (!ok)
        {
            return false;
        }

        qint32 minTmp = tokenTmp.mid(separatorIndex+1).toInt(&ok);
        if (!ok)
        {
            return false;
        }

        m_majorVersion = majTmp;
        m_minorVersion = minTmp;

        return true;
    }
};


/*******************************************************************************
 * HVersionToken
 ******************************************************************************/
HVersionToken::HVersionToken() :
    h_ptr(new HVersionTokenPrivate())
{
}

HVersionToken::HVersionToken(const QString& token) :
    h_ptr(new HVersionTokenPrivate())
{
    HLOG(H_AT, H_FUN);

    if (!h_ptr->init(token))
    {
        HLOG_WARN(QObject::tr("Invalid token: %1").arg(token));
    }
}

HVersionToken::HVersionToken(const HVersionToken& other) :
    h_ptr(new HVersionTokenPrivate(*other.h_ptr))
{
}

HVersionToken& HVersionToken::operator=(const HVersionToken& other)
{
    HVersionTokenPrivate* newHptr = new HVersionTokenPrivate(*other.h_ptr);

    delete h_ptr;
    h_ptr = newHptr;

    return *this;
}

HVersionToken::~HVersionToken()
{
    delete h_ptr;
}

bool HVersionToken::isValid() const
{
    return h_ptr->m_majorVersion >= 0;
}

qint32 HVersionToken::majorVersion() const
{
    return h_ptr->m_majorVersion;
}

qint32 HVersionToken::minorVersion() const
{
    return h_ptr->m_minorVersion;
}

QString HVersionToken::toString() const
{
    if (!isValid())
    {
        return QString();
    }

    return QString("UPnP/%1.%2").arg(
        QString::number(h_ptr->m_majorVersion),
        QString::number(h_ptr->m_minorVersion));
}

bool operator==(const HVersionToken& obj1, const HVersionToken& obj2)
{
    return obj1.h_ptr->m_majorVersion == obj2.h_ptr->m_majorVersion &&
           obj1.h_ptr->m_minorVersion == obj2.h_ptr->m_minorVersion;
}

bool operator!=(const HVersionToken& obj1, const HVersionToken& obj2)
{
    return !(obj1 == obj2);
}

/*******************************************************************************
 * HProductTokensPrivate
 ******************************************************************************/
class HProductTokensPrivate
{
public:

    QStringList m_tokens;
    HVersionToken m_versionToken;

public:

    HProductTokensPrivate() :
        m_tokens(), m_versionToken()
    {
    }

    HProductTokensPrivate(const QString& tokens) :
        m_tokens(), m_versionToken()
    {
        HLOG(H_AT, H_FUN);

        QStringList tmp(tokens.simplified().split(' '));

        if (tmp.size() != 3)
        {
            // it seems that the token string does not follow the UDA specification
            // since it is known that some UPnP software uses comma as the delimiter,
            // check it next:
            tmp = tokens.simplified().split(',');
            if (tmp.size() != 3)
            {
                return;
            }

            HLOG_WARN(QObject::tr(
                "The specified token string uses invalid delimiter [,], but accepting it."));
        }

        m_versionToken = tmp[1].simplified();

        for(qint32 i = 0; i < tmp.size(); ++i)
        {
            m_tokens.append(tmp[i].simplified());
        }
    }
};


/*******************************************************************************
 * HProductTokens
 ******************************************************************************/
HProductTokens::HProductTokens() :
    h_ptr(new HProductTokensPrivate())
{
}

HProductTokens::HProductTokens(const QString& tokens) :
    h_ptr(new HProductTokensPrivate(tokens))
{
}

HProductTokens::HProductTokens(const HProductTokens& other) :
    h_ptr(new HProductTokensPrivate(*other.h_ptr))
{
}

HProductTokens& HProductTokens::operator=(const HProductTokens& other)
{
    HProductTokensPrivate* newHptr = new HProductTokensPrivate(*other.h_ptr);

    delete h_ptr;
    h_ptr = newHptr;

    return *this;
}

HProductTokens::~HProductTokens()
{
    delete h_ptr;
}

bool HProductTokens::isValid() const
{
    return h_ptr->m_tokens.size() > 0;
}

QString HProductTokens::osToken() const
{
    if (!isValid())
    {
        return QString();
    }

    return h_ptr->m_tokens[0];
}

HVersionToken HProductTokens::upnpToken() const
{
    return h_ptr->m_versionToken;
}

QString HProductTokens::productToken() const
{
    if (!isValid())
    {
        return QString();
    }

    return h_ptr->m_tokens[2];
}

QString HProductTokens::toString() const
{
    if (!isValid())
    {
        return QString();
    }

    return QString("%1 %2 %3").arg(osToken(), upnpToken().toString(), productToken());
}

bool operator==(const HProductTokens& ht1, const HProductTokens& ht2)
{
    return ht1.toString() == ht2.toString();
}

bool operator!=(const HProductTokens& ht1, const HProductTokens& ht2)
{
    return !(ht1 == ht2);
}

}
}
