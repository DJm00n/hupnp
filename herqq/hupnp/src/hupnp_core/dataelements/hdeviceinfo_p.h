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

#ifndef HDEVICEINFO_P_H_
#define HDEVICEINFO_P_H_

//
// !! Warning !!
//
// This file is not part of public API and it should
// never be included in client code. The contents of this file may
// change or the file may be removed without of notice.
//

#include "../dataelements/hudn.h"
#include "../dataelements/hresourcetype.h"

#include "../../utils/hlogger_p.h"

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
    ~HDeviceInfoPrivate();

    inline bool setDeviceType(const HResourceType& deviceType)
    {
        if (!deviceType.isValid())
        {
            return false;
        }

        if (deviceType.type() != HResourceType::StandardDeviceType &&
            deviceType.type() != HResourceType::VendorSpecifiedDeviceType)
        {
            return false;
        }

        m_deviceType = deviceType;
        return true;
    }

    inline bool setFriendlyName(const QString& friendlyName)
    {
        HLOG(H_AT, H_FUN);

        if (friendlyName.isEmpty())
        {
            return false;
        }

        if (friendlyName.size() > 64)
        {
            HLOG_WARN(QString(
                "friendlyName longer than 64 characters").arg(friendlyName));
        }

        m_friendlyName = friendlyName;
        return true;
    }

    inline bool setManufacturer(const QString& manufacturer)
    {
        HLOG(H_AT, H_FUN);

        if (manufacturer.isEmpty())
        {
            return false;
        }

        if (manufacturer.size() > 64)
        {
            HLOG_WARN(QString(
                "manufacturer longer than 64 characters").arg(manufacturer));
        }

        m_manufacturer = manufacturer;
        return true;
    }

    inline bool setManufacturerUrl(const QUrl& manufacturerUrl)
    {
        m_manufacturerUrl = manufacturerUrl;
        return true;
    }

    inline bool setModelDescription(const QString& modelDescription)
    {
        HLOG(H_AT, H_FUN);

        if (modelDescription.size() > 128)
        {
            HLOG_WARN(QString(
                "modelDescription longer than 64 characters").arg(modelDescription));
        }

        m_modelDescription = modelDescription;
        return true;
    }

    inline bool setModelName(const QString& modelName)
    {
        HLOG(H_AT, H_FUN);

        if (modelName.isEmpty())
        {
            return false;
        }

        if (modelName.size() > 32)
        {
            HLOG_WARN(QString(
                "modelName longer than 32 characters: [%1]").arg(modelName));
        }

        m_modelName = modelName;
        return true;
    }

    inline bool setModelNumber(const QString& modelNumber)
    {
        HLOG(H_AT, H_FUN);

        if (modelNumber.size() > 32)
        {
            HLOG_WARN(QString(
                "modelNumber longer than 32 characters: [%1]").arg(modelNumber));
        }

        m_modelNumber = modelNumber;
        return true;
    }

    inline bool setModelUrl(const QUrl& modelUrl)
    {
        m_modelUrl = modelUrl;
        return true;
    }

    inline bool setSerialNumber(const QString& serialNumber)
    {
        HLOG(H_AT, H_FUN);

        if (serialNumber.size() > 64)
        {
            HLOG_WARN(QString(
                "serialNumber longer than 64 characters: [%1]").arg(serialNumber));
        }

        m_serialNumber = serialNumber;
        return true;
    }

    inline bool setUdn(const HUdn& udn)
    {
        if (!udn.isValid())
        {
            return false;
        }

        m_udn = udn;
        return true;
    }

    inline bool setUpc(const QString& upc)
    {
        HLOG(H_AT, H_FUN);

        if (upc.isEmpty())
        {
            // UPC is optional, so if it is not provided at all, that is okay.
            return false;
        }

        // even if something is provided, we only warn the user of possible error.
        // (since upc is optional)

        if (upc.size() > 13 || upc.size() < 12)
        {
            // a white-space and a hyphen in the middle are acceptable
            HLOG_WARN_NONSTD(QString(
                "UPC should be 12-digit, all-numeric code. Encountered: [%1].").arg(
                    upc));
        }
        else
        {
            for(qint32 i = 0; i < upc.size(); ++i)
            {
                QChar ch = upc[i];

                if ((i == 6 && !ch.isSpace() && ch != '-' && upc.size() == 13) ||
                    !ch.isDigit())
                {
                    HLOG_WARN_NONSTD(QString(
                        "UPC should be 12-digit, all-numeric code. "
                        "Ignoring invalid value [%1].").arg(upc));

                    break;
                }
            }
        }

        m_upc = upc;
        return true;
    }

    inline bool setIcons(const QList<QPair<QUrl, QImage> >& icons)
    {
        m_icons = icons;
        return true;
    }

    inline bool setPresentationUrl(const QUrl& presentationUrl)
    {
        Q_ASSERT(presentationUrl.isValid() || presentationUrl.isEmpty());
        m_presentationUrl = presentationUrl;
        return true;
    }
};

}
}

#endif /* HDEVICEINFO_P_H_ */
