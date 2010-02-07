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
 * HProductTokenPrivate
 ******************************************************************************/
class HProductTokenPrivate
{
public:

    QString m_token;
    QString m_productVersion;

public:

    HProductTokenPrivate() :
        m_token(), m_productVersion()
    {
    }

    HProductTokenPrivate(const QString& token, const QString& productVersion) :
        m_token(), m_productVersion()
    {
        QString tokenTmp(token.simplified());
        QString productVersionTmp(productVersion.simplified());
        if (tokenTmp.isEmpty() || productVersionTmp.isEmpty())
        {
            return;
        }

        m_token = tokenTmp;
        m_productVersion = productVersionTmp;
    }
};


/*******************************************************************************
 * HProductToken
 ******************************************************************************/
HProductToken::HProductToken() :
    h_ptr(new HProductTokenPrivate())
{
}

HProductToken::HProductToken(const QString& token, const QString& productVersion) :
    h_ptr(new HProductTokenPrivate(token, productVersion))
{
    HLOG(H_AT, H_FUN);

    if (h_ptr->m_token.isEmpty())
    {
        HLOG_WARN(QObject::tr(
            "Invalid product token. Token: %1, Product Version: %2").arg(
                token, productVersion));
    }
}

HProductToken::HProductToken(const HProductToken& other) :
    h_ptr(new HProductTokenPrivate(*other.h_ptr))
{
}

HProductToken& HProductToken::operator=(const HProductToken& other)
{
    HProductTokenPrivate* newHptr = new HProductTokenPrivate(*other.h_ptr);

    delete h_ptr;
    h_ptr = newHptr;

    return *this;
}

HProductToken::~HProductToken()
{
    delete h_ptr;
}

bool HProductToken::isValid() const
{
    return !h_ptr->m_token.isEmpty();
}

QString HProductToken::toString() const
{
    if (!isValid())
    {
        return QString();
    }

    return QString("%1/%2").arg(h_ptr->m_token, h_ptr->m_productVersion);
}

QString HProductToken::token() const
{
    return h_ptr->m_token;
}

QString HProductToken::version() const
{
    return h_ptr->m_productVersion;
}

bool HProductToken::isValidUpnpToken(const HProductToken& token)
{
    if (token.token() != "UPnP")
    {
        return false;
    }

    QString version = token.version();

    return (version.size() == 3    &&
           (version[0]     == '1') && 
            version[1]     == '.'  && 
           (version[2] == '0' || version[2] == '1'));
}

qint32 HProductToken::minorVersion(const HProductToken& token)
{
    QString tokenVersion = token.version();

    qint32 separatorIndex = tokenVersion.indexOf('.');
    if (separatorIndex < 0)
    {
        return false;
    }

    bool ok = false;

    qint32 minTmp = tokenVersion.mid(separatorIndex+1).toInt(&ok);
    if (ok)
    {
        return minTmp;
    }

    return -1;
}

qint32 HProductToken::majorVersion(const HProductToken& token)
{
    QString tokenVersion = token.version();

    qint32 separatorIndex = tokenVersion.indexOf('.');
    if (separatorIndex < 0)
    {
        return false;
    }

    bool ok = false;
    qint32 majTmp = tokenVersion.left(separatorIndex).toInt(&ok);
    if (ok)
    {
        return majTmp;
    }

    return -1;
}

bool operator==(const HProductToken& obj1, const HProductToken& obj2)
{
    return obj1.toString() == obj2.toString();
}

bool operator!=(const HProductToken& obj1, const HProductToken& obj2)
{
    return !(obj1 == obj2);
}

/*******************************************************************************
 * HProductTokensPrivate
 ******************************************************************************/
class HProductTokensPrivate
{
private:

    bool parseCommaDelimited(const QString& tokens)
    {
        HLOG(H_AT, H_FUN);

        QStringList tmp(tokens.split(','));

        if (tmp.size() != 3)
        {
            return false;
        }

        for (qint32 i = 0; i < 3; ++i)
        {
            qint32 index = tmp[i].indexOf('/');
            if (index < 0)
            {
                m_productTokens.clear();
                return false;
            }

            m_productTokens.append(
                HProductToken(tmp[i].left(index), tmp[i].right(index)));
        }

        return true;
    }

    bool parseNormal(const QString& tokens)
    {
        HLOG(H_AT, H_FUN);

        QList<HProductToken> productTokens;

        QString token, version, buf;
        qint32 i = tokens.indexOf('/'), j = 0, lastSpace = 0;
        if (i < 0)
        {
            return false;
        }

        // the first special case "token/version token/version token/version"
        //                         ^^^^^
        token = tokens.left(i);

        for(i = i + 1; i < tokens.size(); ++i, ++j)
        {
            if (tokens[i] == '/')
            {
                if (lastSpace <= 0)
                {
                    // there must have been at least one space between the previous '/'
                    // and this one. it is an error otherwise.
                    return false;
                }

                HProductToken newToken(token, buf.left(lastSpace));
                productTokens.append(newToken);

                token = buf.mid(lastSpace+1);

                version.clear(); buf.clear(); j = -1;
                continue;
            }
            else if (tokens[i] == ' ')
            {
                lastSpace = j;
            }

            buf.append(tokens[i]);
        }

        HProductToken newToken(token, buf);
        productTokens.append(newToken);

        if (productTokens.size() != 3 ||
           !HProductToken::isValidUpnpToken(productTokens[1]))
        {
            return false;
        }
        else
        {
            m_productTokens = productTokens;
        }

        return true;
    }

public:

    QList<HProductToken> m_productTokens;

public:

    HProductTokensPrivate() :
        m_productTokens()
    {
    }

    HProductTokensPrivate(const QString& tokens) :
        m_productTokens()
    {
        HLOG(H_AT, H_FUN);

        QString tokensTmp(tokens.simplified());

        if (!parseNormal(tokensTmp))
        {
            // it seems that the token string does not follow the UDA specification.
            // since it is known that some UPnP software uses comma as the delimiter,
            // check it next:

            if (parseCommaDelimited(tokensTmp))
            {
                HLOG_WARN_NONSTD(QObject::tr(
                    "The specified token string [%1] uses invalid delimiter [,],"
                    " but accepting it.").arg(tokens));
            }
            else
            {
                HLOG_WARN(QObject::tr("Invalid Product Tokens: %1").arg(tokens));
            }
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
    return h_ptr->m_productTokens.size() > 0;
}

HProductToken HProductTokens::osToken() const
{
    if (!isValid())
    {
        return HProductToken();
    }

    return h_ptr->m_productTokens[0];
}

HProductToken HProductTokens::upnpToken() const
{
    if (!isValid())
    {
        return HProductToken();
    }

    return h_ptr->m_productTokens[1];
}

HProductToken HProductTokens::productToken() const
{
    if (!isValid())
    {
        return HProductToken();
    }

    return h_ptr->m_productTokens[2];
}

QString HProductTokens::toString() const
{
    if (!isValid())
    {
        return QString();
    }

    return QString("%1 %2 %3").arg(
        osToken().toString(), upnpToken().toString(), productToken().toString());
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
