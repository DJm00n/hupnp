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
#ifndef UPNP_DEVICEINFO_P_H_
#define UPNP_DEVICEINFO_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "upnp_udn.h"
#include "upnp_resourcetype.h"

#include "../../../utils/src/logger_p.h"
#include "../../../core/include/HExceptions"

#include <QUrl>
#include <QList>
#include <QImage>
#include <QString>

namespace Herqq
{

namespace Upnp
{

//
// Implementation details of HDeviceInfo
//
class HDeviceInfoPrivate
{

public: // attributes

    HResourceType m_deviceType;
    QString m_friendlyName;
    QString m_manufacturer;
    QUrl    m_manufacturerUrl;
    QString m_modelDescription;
    QString m_modelName;
    QString m_modelNumber;
    QUrl    m_modelUrl;
    QString m_serialNumber;
    HUdn    m_udn;
    QString m_upc;
    QUrl    m_presentationUrl;
    QList<QPair<QUrl, QImage> > m_icons;

public: // methods

    HDeviceInfoPrivate();
    virtual ~HDeviceInfoPrivate();

    virtual HDeviceInfoPrivate* clone() const;

    inline void setDeviceType(const HResourceType& deviceType)
    {
        if (!deviceType.isValid())
        {
            throw Herqq::HIllegalArgumentException(QObject::tr("deviceType"));
        }

        if (deviceType.type() != "device")
        {
            throw Herqq::HIllegalArgumentException(QObject::tr("deviceType"));
        }

        m_deviceType = deviceType;
    }

    inline void setFriendlyName(const QString& friendlyName)
    {
        HLOG(H_AT, H_FUN);

        m_friendlyName = friendlyName;
        if (m_friendlyName.isEmpty())
        {
            throw Herqq::HIllegalArgumentException(QObject::tr("friendlyName"));
        }
        if (m_friendlyName.size() > 64)
        {
            HLOG_WARN(QString(
                "friendlyName longer than 64 characters").arg(friendlyName));
        }
    }

    inline void setManufacturer(const QString& manufacturer)
    {
        HLOG(H_AT, H_FUN);

        m_manufacturer = manufacturer;
        if (m_manufacturer.isEmpty())
        {
            throw Herqq::HIllegalArgumentException(QObject::tr("manufacturer"));
        }
        if (m_manufacturer.size() > 64)
        {
            HLOG_WARN(QString(
                "manufacturer longer than 64 characters").arg(manufacturer));
        }
    }

    inline void setManufacturerUrl(const QUrl& manufacturerUrl)
    {
        m_manufacturerUrl = manufacturerUrl;
    }

    inline void setModelDescription(const QString& modelDescription)
    {
        HLOG(H_AT, H_FUN);

        m_modelDescription = modelDescription;
        if (m_modelDescription.size() > 128)
        {
            HLOG_WARN(QString(
                "modelDescription longer than 64 characters").arg(modelDescription));
        }
    }

    inline void setModelName(const QString& modelName)
    {
        HLOG(H_AT, H_FUN);

        m_modelName = modelName;
        if (m_modelName.isEmpty())
        {
            throw Herqq::HIllegalArgumentException(QObject::tr("modelName"));
        }
        if (m_modelName.size() > 32)
        {
            HLOG_WARN(QString(
                "modelName longer than 32 characters: [%1]").arg(modelName));
        }
    }

    inline void setModelNumber(const QString& modelNumber)
    {
        HLOG(H_AT, H_FUN);

        m_modelNumber = modelNumber;
        if (m_modelNumber.size() > 32)
        {
            HLOG_WARN(QString(
                "modelNumber longer than 32 characters: [%1]").arg(modelNumber));
        }
    }

    inline void setModelUrl(const QUrl& modelUrl)
    {
        m_modelUrl = modelUrl;
    }

    inline void setSerialNumber(const QString& serialNumber)
    {
        HLOG(H_AT, H_FUN);

        m_serialNumber = serialNumber;
        if (m_serialNumber.size() > 32)
        {
            HLOG_WARN(QString(
                "serialNumber longer than 64 characters: [%1]").arg(serialNumber));
        }
    }

    inline void setUdn(const HUdn& udn)
    {
        if (!udn.isValid())
        {
            throw HIllegalArgumentException(QObject::tr("Invalid UDN"));
        }

        m_udn = udn;
    }

    inline void setUpc(const QString& upc)
    {
        HLOG(H_AT, H_FUN);

        if (upc.isEmpty())
        {
            // UPC is optional, so if it is not provided at all, that is okay.
            return;
        }

        // even if something is provided, we only warn the user of possible error.
        // (since upc is optional)

        if (upc.size() > 13 || upc.size() < 12) // a white-space in the middle is acceptable
        {
            HLOG_WARN(QString(
                "UPC should be 12-digit, all-numeric code. Ignoring invalid value [%1].").arg(upc));

            return;
        }

        for(qint32 i = 0; i < upc.size(); ++i)
        {
            QChar ch = upc[i];
            if ((!ch.isDigit() && !ch.isSpace()) || (ch.isSpace() && i != 6))
            {
                HLOG_WARN(QString(
                    "UPC should be 12-digit, all-numeric code. Ignoring invalid value [%1].").arg(upc));

                return;
            }
        }

        m_upc = upc;
    }

    inline void setIcons(const QList<QPair<QUrl, QImage> >& icons)
    {
        m_icons = icons;
    }

    inline void setPresentationUrl(const QUrl& presentationUrl)
    {
        Q_ASSERT(presentationUrl.isValid() || presentationUrl.isEmpty());
        m_presentationUrl = presentationUrl;
    }
};

}
}

#endif /* UPNP_DEVICEINFO_P_H_ */
