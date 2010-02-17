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

#include "hdeviceinfo.h"
#include "hdeviceinfo_p.h"

#include <QUrl>
#include <QList>
#include <QImage>
#include <QString>
#include <QMetaType>

static bool registerMetaTypes()
{
    qRegisterMetaType<Herqq::Upnp::HDeviceInfo>("Herqq::Upnp::HDeviceInfo");
    return true;
}

static bool regMetaT = registerMetaTypes();

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HDeviceInfoPrivate
 ******************************************************************************/
HDeviceInfoPrivate::HDeviceInfoPrivate() :
    m_deviceType      (), m_friendlyName(), m_manufacturer(), m_manufacturerUrl(),
    m_modelDescription(), m_modelName   (), m_modelNumber (), m_modelUrl       (),
    m_serialNumber    (), m_udn         (), m_upc         (), m_presentationUrl(),
    m_icons()
{
    Q_UNUSED(regMetaT)
}

HDeviceInfoPrivate::~HDeviceInfoPrivate()
{
}

/*******************************************************************************
 * HDeviceInfo
 ******************************************************************************/
HDeviceInfo::HDeviceInfo() :
    h_ptr(new HDeviceInfoPrivate())
{
}

HDeviceInfo::HDeviceInfo(const HDeviceInfo& other) :
    h_ptr(new HDeviceInfoPrivate(*other.h_ptr))
{
}

HDeviceInfo& HDeviceInfo::operator=(const HDeviceInfo& other)
{
    HDeviceInfoPrivate* newHptr = new HDeviceInfoPrivate(*other.h_ptr);

    delete h_ptr;
    h_ptr = newHptr;

    return *this;
}

HDeviceInfo::HDeviceInfo(
    const HResourceType& deviceType,
    const QString& friendlyName,
    const QString& manufacturer,
    const QString& modelName,
    const HUdn&    udn,
    QString* err) :
        h_ptr(new HDeviceInfoPrivate())
{
    HDeviceInfoPrivate tmp;

    QString errTmp;
    if (!tmp.setDeviceType(deviceType))
    {
        errTmp = QObject::tr("Invalid device type: [%1]").arg(deviceType.toString());
    }
    else if (!tmp.setFriendlyName(friendlyName))
    {
        errTmp = QObject::tr("Invalid friendly name: [%1]").arg(friendlyName);
    }
    else if (!tmp.setManufacturer(manufacturer))
    {
        errTmp = QObject::tr("Invalid manufacturer: [%1]").arg(manufacturer);
    }
    else if (!tmp.setModelName(modelName))
    {
        errTmp = QObject::tr("Invalid model name: [%1]").arg(modelName);
    }
    else if (!tmp.setUdn(udn))
    {
        errTmp = QObject::tr("Invalid UDN: [%1]").arg(udn.toString());
    }

    if (!errTmp.isEmpty())
    {
        if (err)
        {
            *err = errTmp;
        }

        return;
    }

    *h_ptr = tmp;
}

HDeviceInfo::HDeviceInfo(
    const HResourceType& deviceType,
    const QString& friendlyName,
    const QString& manufacturer,
    const QUrl&    manufacturerUrl,
    const QString& modelDescription,
    const QString& modelName,
    const QString& modelNumber,
    const QUrl&    modelUrl,
    const QString& serialNumber,
    const HUdn&    udn,
    const QString& upc,
    const QList<QPair<QUrl, QImage> >& icons,
    const QUrl&    presentationUrl,
    QString* err) :
        h_ptr(new HDeviceInfoPrivate())
{
    HDeviceInfoPrivate tmp;

    QString errTmp;
    if (!tmp.setDeviceType(deviceType))
    {
        errTmp = QObject::tr("Invalid device type: [%1]").arg(deviceType.toString());
    }
    else if (!tmp.setFriendlyName(friendlyName))
    {
        errTmp = QObject::tr("Invalid friendly name: [%1]").arg(friendlyName);
    }
    else if (!tmp.setManufacturer(manufacturer))
    {
        errTmp = QObject::tr("Invalid manufacturer: [%1]").arg(manufacturer);
    }
    else if (!tmp.setModelName(modelName))
    {
        errTmp = QObject::tr("Invalid model name: [%1]").arg(modelName);
    }
    else if (!tmp.setUdn(udn))
    {
        errTmp = QObject::tr("Invalid UDN: [%1]").arg(udn.toString());
    }

    if (!errTmp.isEmpty())
    {
        if (err)
        {
            *err = errTmp;
        }

        return;
    }

    *h_ptr = tmp;

    // these are optional ==> no need to be strict
    h_ptr->setManufacturerUrl (manufacturerUrl.toString());
    h_ptr->setModelDescription(modelDescription);

    h_ptr->setModelNumber     (modelNumber);
    h_ptr->setModelUrl        (modelUrl.toString());
    h_ptr->setSerialNumber    (serialNumber);
    h_ptr->setUpc             (upc);
    h_ptr->setIcons           (icons);
    h_ptr->setPresentationUrl (presentationUrl.toString());
}

HDeviceInfo::~HDeviceInfo()
{
    delete h_ptr;
}

bool HDeviceInfo::isValid() const
{
    return h_ptr->m_deviceType.isValid();
}

void HDeviceInfo::setManufacturerUrl(const QUrl& arg)
{
    h_ptr->setManufacturerUrl(arg);
}

void HDeviceInfo::setModelDescription(const QString& arg)
{
    h_ptr->setModelDescription(arg);
}

void HDeviceInfo::setModelNumber(const QString& arg)
{
    h_ptr->setModelNumber(arg);
}

void HDeviceInfo::setModelUrl(const QUrl& arg)
{
    h_ptr->setModelUrl(arg);
}

void HDeviceInfo::setSerialNumber(const QString& arg)
{
    h_ptr->setSerialNumber(arg);
}

void HDeviceInfo::setUpc(const QString& arg)
{
    h_ptr->setUpc(arg);
}

void HDeviceInfo::setIcons(const QList<QPair<QUrl, QImage> >& arg)
{
    h_ptr->setIcons(arg);
}

void HDeviceInfo::setPresentationUrl(const QUrl& arg)
{
    h_ptr->setPresentationUrl(arg);
}

HResourceType HDeviceInfo::deviceType() const
{
    return h_ptr->m_deviceType;
}

QString HDeviceInfo::friendlyName () const
{
    return h_ptr->m_friendlyName;
}

QString HDeviceInfo::manufacturer() const
{
    return h_ptr->m_manufacturer;
}

QUrl HDeviceInfo::manufacturerUrl() const
{
    return h_ptr->m_manufacturerUrl;
}

QString HDeviceInfo::modelDescription() const
{
    return h_ptr->m_modelDescription;
}

QString HDeviceInfo::modelName () const
{
    return h_ptr->m_modelName;
}

QString HDeviceInfo::modelNumber() const
{
    return h_ptr->m_modelNumber;
}

QUrl HDeviceInfo::modelUrl() const
{
    return h_ptr->m_modelUrl;
}

QString HDeviceInfo::serialNumber() const
{
    return h_ptr->m_serialNumber;
}

HUdn HDeviceInfo::udn() const
{
    return h_ptr->m_udn;
}

QString HDeviceInfo::upc() const
{
    return h_ptr->m_upc;
}

QList<QPair<QUrl, QImage> > HDeviceInfo::icons() const
{
    return h_ptr->m_icons;
}

QUrl HDeviceInfo::presentationUrl() const
{
    return h_ptr->m_presentationUrl;
}

bool operator==(const HDeviceInfo& obj1, const HDeviceInfo& obj2)
{
    bool b = obj1.h_ptr->m_deviceType         == obj2.h_ptr->m_deviceType       &&
             obj1.h_ptr->m_friendlyName       == obj2.h_ptr->m_friendlyName     &&
             obj1.h_ptr->m_manufacturer       == obj2.h_ptr->m_manufacturer     &&
             obj1.h_ptr->m_manufacturerUrl    == obj2.h_ptr->m_manufacturerUrl  &&
             obj1.h_ptr->m_modelDescription   == obj2.h_ptr->m_modelDescription &&
             obj1.h_ptr->m_modelName          == obj2.h_ptr->m_modelName        &&
             obj1.h_ptr->m_modelNumber        == obj2.h_ptr->m_modelNumber      &&
             obj1.h_ptr->m_modelUrl           == obj2.h_ptr->m_modelUrl         &&
             obj1.h_ptr->m_serialNumber       == obj2.h_ptr->m_serialNumber     &&
             obj1.h_ptr->m_udn                == obj2.h_ptr->m_udn              &&
             obj1.h_ptr->m_upc                == obj2.h_ptr->m_upc              &&
             obj1.h_ptr->m_presentationUrl    == obj2.h_ptr->m_presentationUrl;

    if (!b)
    {
        return false;
    }

    QList<QPair<QUrl, QImage> > icons1 = obj1.icons();
    QList<QPair<QUrl, QImage> > icons2 = obj2.icons();

    if (icons1.size() != icons2.size())
    {
        return false;
    }

    for (qint32 i = 0; i < icons1.size(); ++i)
    {
        if (icons1[i] != icons2[i])
        {
            return false;
        }
    }

    return true;
}

bool operator!=(const HDeviceInfo& obj1, const HDeviceInfo& obj2)
{
    return !(obj1 == obj2);
}

}
}