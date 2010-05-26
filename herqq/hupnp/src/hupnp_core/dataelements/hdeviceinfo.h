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

#ifndef HDEVICEINFO_H_
#define HDEVICEINFO_H_

#include "../general/hdefs_p.h"
#include "../general/hupnp_fwd.h"

#include <QPair>

class QUrl;
class QImage;
class QString;

namespace Herqq
{

namespace Upnp
{

class HDeviceInfoPrivate;

/*!
 * \brief Class that represents device information found in a UPnP device description file.
 *
 * A device description details a UPnP device. A device description specifies
 * the services of a device, the embedded devices of a device and other information,
 * such as the manufacturer, model name, serial number and the Unique Device Name
 * that uniquely identifies a device.
 *
 * In HUPnP the UPnP services and embedded devices are part of the
 * HUPnP's \ref devicemodel and because of that they are represented
 * as objects, which ownership is always held by HUPnP.
 * All the other information found in a device description is contained by
 * an instance of this class.
 *
 * \headerfile hdeviceinfo.h HDeviceInfo
 *
 * \ingroup dataelements
 *
 * \remarks this class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HDeviceInfo
{
H_DECLARE_PRIVATE(HDeviceInfo)
friend H_UPNP_CORE_EXPORT bool operator==(
    const HDeviceInfo& obj1, const HDeviceInfo& obj2);

private:

    HDeviceInfoPrivate* h_ptr;

public:

    /*!
     * Creates a new, empty instance.
     *
     * \sa isValid()
     */
    HDeviceInfo();

    /*!
     * Constructs a new instance from the specified parameters that the UDA
     * specification mandates for a device. These are the arguments found in the
     * device description file and they are all mandatory for a valid UPnP device.
     *
     * \param deviceType specifies the device type.
     *
     * \param friendlyName specifies a short description for the end-user. This
     * cannot be empty and should be less than 64 characters.
     *
     * \param manufacturer specifies the name of the manufacturer. This cannot
     * be empty and should be less than 64 characters.
     *
     * \param modelName specifies the model name. This cannot be empty and
     * should be less than 32 characters.
     *
     * \param udn specifies the unique device name. This is a universally
     * unique identifier for the device, regardless if the device is root
     * or embedded. The specified UDN has to be valid.
     *
     * \param err specifies a pointer to a \c QString that will contain
     * an error description in case the construction failed. This is optional.
     *
     * \remarks in case any of the provided arguments does not meet the
     * specified requirements, the created object is \e invalid.
     *
     * \sa isValid()
     */
    HDeviceInfo(
        const HResourceType& deviceType,
        const QString& friendlyName,
        const QString& manufacturer,
        const QString& modelName,
        const HUdn&    udn,
        QString* err = 0);

    /*!
     * Constructs a new instance from the specified parameters. These are
     * all the arguments found in the device description file.
     *
     * \param deviceType specifies the device type.
     *
     * \param friendlyName specifies a short description for the end-user. This
     * cannot be empty and should be less than 64 characters.
     *
     * \param manufacturer specifies the name of the manufacturer. This cannot
     * be empty and should be less than 64 characters.
     *
     * \param manufacturerUrl specifies the web site for the manufacturer.
     *
     * \param modelDescription specifies the long description for the end user.
     * This can be empty and should be less than 128 characters.
     *
     * \param modelName specifies the model name. This cannot be empty and
     * should be less than 32 characters.
     *
     * \param modelNumber specifies the model number of the device. There is
     * no format specified. This should be less than 32 characters.
     *
     * \param modelUrl specifies the web site for the device model.
     *
     * \param serialNumber specifies the serial number of the device. No
     * format specified. This should be less than 64 characters.
     *
     * \param udn specifies the unique device name. This is a universally
     * unique identifier for the device, regardless if the device is root
     * or embedded. The specified UDN has to be valid.
     *
     * \param upc specifies the Universal Product Code, which is 12-digit,
     * all-numeric code that identifies the consumer package.
     * Managed by the Uniform Code Council.
     *
     * \param icons specifies the icons of the device, if any.
     *
     * \param presentationUrl specifies the URL for HTML-based user interface
     * for controlling and/or viewing device status.
     *
     * \param err specifies a pointer to a \c QString that will contain
     * an error description in case the construction failed. This is optional.
     *
     * \remarks in case any of the provided arguments does not meet the
     * specified requirements, the created object is \e invalid.
     *
     * \sa isValid()
     */
    HDeviceInfo(
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
        QString* err = 0);

    /*!
     * Destroys the instance.
     */
    ~HDeviceInfo();

    /*!
     * Copies the contents of the other to this.
     *
     * \param other specifies the object to be copied.
     */
    HDeviceInfo(const HDeviceInfo& other);

    /*!
     * Assigns the contents of the other to this.
     *
     * \param other specifies the object to be copied.
     */
    HDeviceInfo& operator=(const HDeviceInfo& other);

    /*!
     * Indicates if the object is valid.
     *
     * A valid object contains the mandatory data of a device description.
     *
     * \return \e true in case the object is valid.
     */
    bool isValid() const;

    /*!
     * Sets the URL for the web site of the manufacturer.
     *
     * \param arg specifies the URL for the web site of the manufacturer.
     *
     * \sa manufacturerUrl()
     */
    void setManufacturerUrl(const QUrl& arg);

    /*!
     * Sets the model description.
     *
     * A model description is used to display a long description for end user.
     * Should be < 128 characters.
     *
     * \param arg specifies the model description.
     *
     * \sa modelDescription()
     */
    void setModelDescription(const QString& arg);

    /*!
     * Sets the model number.
     *
     * There is no format specified for the model number,
     * other than it should be < 32 characters.
     *
     * \param arg specifies the model number.
     *
     * \sa modelNumber()
     */
    void setModelNumber(const QString& arg);

    /*!
     * Sets the URL for the web site of the model.
     *
     * \param arg specifies the model URL.
     *
     * \sa modelUrl()
     */
    void setModelUrl(const QUrl& arg);

    /*!
     * Sets the serial number of the device.
     *
     * There is no format specified for the serial number,
     * other than it should be < 64 characters.
     *
     * \param arg specifies the serial number.
     *
     * \sa serialNumber()
     */
    void setSerialNumber(const QString& arg);

    /*!
     * Sets the Universal Product Code.
     *
     * UPC is a 12-digit, all-numeric code that
     * identifies the consumer package. Managed by the Uniform Code Council.
     *
     * \param arg specifies the UPC.
     *
     * \sa upc()
     */
    void setUpc(const QString& arg);

    /*!
     * Sets the icons of the device.
     *
     * \param arg specifies the icons of the device.
     *
     * \sa icons()
     */
    void setIcons(const QList<QPair<QUrl, QImage> >& arg);

    /*!
     * Sets the presentation URL.
     *
     * Presentation URL specifies the URL for HTML-based user interface
     * for controlling and/or viewing device status.
     *
     * \param arg specifies the presentation URL.
     *
     * \sa presentationUrl()
     */
    void setPresentationUrl(const QUrl& arg);

    /*!
     * Returns the type of the device found in the device description file.
     *
     * \return the type of the device found in the device description file.
     */
    HResourceType deviceType() const;

    /*!
     * Returns short description for end user.
     *
     * \return short description for end user.
     */
    QString friendlyName() const;

    /*!
     * Returns manufacturer's name.
     *
     * \return manufacturer's name.
     */
    QString manufacturer() const;

    /*!
     * Returns the manufacturer's web site.
     *
     * \return the manufacturer's web site.
     *
     * \sa setManufacturerUrl()
     */
    QUrl manufacturerUrl() const;

    /*!
     * Returns long description for end user.
     *
     * \return long description for end user.
     *
     * \sa setModelDescription()
     */
    QString modelDescription() const;

    /*!
     * Returns the model name.
     *
     * \return the model name.
     */
    QString modelName() const;

    /*!
     * Returns the model number.
     *
     * \return the model number.
     *
     * \sa setModelNumber()
     */
    QString modelNumber() const;

    /*!
     * Returns the web site for the device model.
     *
     * \return the web site for the device model.
     *
     * \sa setModelUrl()
     */
    QUrl modelUrl() const;

    /*!
     * Returns the serial number.
     *
     * \return the serial number.
     *
     * \sa setSerialNumber()
     */
    QString serialNumber() const;

    /*!
     * Returns the Unique Device Name.
     *
     * \return Universally-unique identifier for the device, whether root or embedded.
     *
     * \remarks the UDN is same over time for a specific device instance.
     */
    HUdn udn() const;

    /*!
     * Returns the Universal Product Code.
     *
     * \return the Universal Product Code.
     *
     * \sa setUpc()
     */
    QString upc() const;

    /*!
     * Returns the icons of the device, if any.
     *
     * \return the icons of the device.
     *
     * \sa setIcons()
     */
    QList<QPair<QUrl, QImage> > icons() const;

    /*!
     * Returns the location of the device's presentation page.
     *
     * \return the location of the device's presentation page.
     *
     * \sa setPresentationUrl()
     */
    QUrl presentationUrl() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the object are logically equivalent.
 *
 * \relates HDeviceInfo
 */
H_UPNP_CORE_EXPORT bool operator==(const HDeviceInfo&, const HDeviceInfo&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the object are not logically equivalent.
 *
 * \relates HDeviceInfo
 */
H_UPNP_CORE_EXPORT bool operator!=(const HDeviceInfo&, const HDeviceInfo&);

}
}

#endif /* HDEVICEINFO_H_ */
