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

#ifndef HDEVICES_SETUPDATA_H_
#define HDEVICES_SETUPDATA_H_

#include <HUpnpCore/HUpnp>

#include <QtCore/QHash>
#include <QtCore/QString>

namespace Herqq
{

namespace Upnp
{

class HDeviceSetupPrivate;

/*!
 * This class is used to specify information that is required to setup an
 * HDevice.
 *
 * \headerfile hdevices_setupdata.h HDeviceSetup
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa HDevicesSetupData, HDevice
 *
 * \remarks this class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HDeviceSetup
{
H_DISABLE_COPY(HDeviceSetup)

private:

    HDeviceSetupPrivate* h_ptr;

public:

    /*!
     * Creates a new, invalid instance.
     *
     * \sa isValid()
     */
    HDeviceSetup();

    /*!
     * Creates a new instance.
     *
     * \param type specifies the device type.
     *
     * \param incReq specifies <em>inclusion requirement</em> of the device.
     *
     * \sa isValid()
     *
     * \remarks the version() is set to 1.
     */
    HDeviceSetup(
        const HResourceType& type,
        HInclusionRequirement incReq = InclusionMandatory);

    /*!
     * Creates a new instance.
     *
     * \param type specifies the device type.
     *
     * \param version specifies the version of the UPnP device,
     * which first specified the embedded device.
     *
     * \param incReq specifies <em>inclusion requirement</em> of the device.
     *
     * \sa isValid()
     */
    HDeviceSetup(
        const HResourceType& type,
        qint32 version,
        HInclusionRequirement incReq = InclusionMandatory);

    /*!
     * Creates a new instance.
     *
     * \param type specifies the device type.
     *
     * \param device specifies a pointer to a heap-allocated HDevice.
     * This instance takes the ownership of the device.
     *
     * \param incReq specifies <em>inclusion requirement</em> of the device.
     *
     * \sa isValid()
     *
     * \remarks the version() is set to 1.
     */
    HDeviceSetup(
        const HResourceType& type,
        HDevice* device,
        HInclusionRequirement incReq = InclusionMandatory);

    /*!
     * Creates a new instance.
     *
     * \param type specifies the device type.
     *
     * \param device specifies a pointer to a heap-allocated HDevice.
     * This instance takes the ownership of the device.
     *
     * \param version specifies the version of the UPnP device,
     * which first specified the embedded device.
     *
     * \param incReq specifies <em>inclusion requirement</em> of the device.
     *
     * \sa isValid()
     */
    HDeviceSetup(
        const HResourceType& type,
        HDevice* device,
        qint32 version,
        HInclusionRequirement incReq = InclusionMandatory);

    /*!
     * Destroys the instance.
     *
     * Destroys the instance.
     */
    ~HDeviceSetup();

    /*!
     * Returns the device type.
     *
     * \return the device type.
     *
     * \sa setDeviceType()
     */
    const HResourceType& deviceType() const;

    /*!
     * Returns the HDevice pointer associated with the instance.
     *
     * \return the HDevice pointer associated with the instance. The ownership
     * of the HDevice is not transferred to the caller.
     *
     * \sa takeDevice()
     */
    HDevice* device() const;

    /*!
     * Returns the <em>inclusion requirement</em>.
     *
     * \return the <em>inclusion requirement</em>.
     *
     * \sa setInclusionRequirement()
     */
    HInclusionRequirement inclusionRequirement() const;

    /*!
     * Indicates if the object is valid.
     *
     * \return \e true in case the object is valid, that is, the device type,
     * version and inclusion requirement are properly defined.
     *
     * \sa version(), deviceType(), inclusionRequirement()
     */
    bool isValid() const;

    /*!
     * Returns the version of the UPnP device, which first specified the
     * embedded device.
     *
     * \return the version of the UPnP device, which first specified the
     * embedded device.
     *
     * \sa setVersion()
     */
    qint32 version() const;

    /*!
     * Sets the the <em>inclusion requirement</em>.
     *
     * \param arg specifies the <em>inclusion requirement</em>.
     *
     * \sa inclusionRequirement()
     */
    void setInclusionRequirement(HInclusionRequirement arg);

    /*!
     * Sets the device type.
     *
     * \param arg specifies the device type.
     *
     * \sa deviceType()
     */
    void setDeviceType(const HResourceType& arg);

    /*!
     * Associates an HDevice pointer with this instance.
     *
     * \param arg specifies the HDevice pointer to be associated with this
     * instance. The instance takes the ownership of the provided HDevice.
     *
     * \remarks if the instance already has an HDevice pointer associated with it,
     * the old HDevice is first deleted, even if the provided HDevice is null.
     *
     * \sa device(), takeDevice()
     */
    void setDevice(HDevice* arg);

    /*!
     * Specifies the version of the UPnP device, which first specified the
     * embedded device.
     *
     * \param version specifies the version of the UPnP device,
     * which first specified the embedded device.
     *
     * \sa version()
     */
    void setVersion(qint32 version);

    /*!
     * Returns the HDevice pointer associated with the instance and passes
     * the ownership of the object to the caller.
     *
     * \return the HDevice pointer associated with the instance and passes
     * the ownership of the object to the caller.
     *
     * \sa device(), setDevice()
     */
    HDevice* takeDevice();
};

/*!
 * This class is used to specify information that can be used to setup multiple
 * HDevice instances.
 *
 * \headerfile hdevices_setupdata.h HDevicesSetupData
 *
 * \ingroup hupnp_devicemodel
 *
 * \remarks this class is not thread-safe.
 *
 * \sa HDeviceSetup, HDevice
 */
class H_UPNP_CORE_EXPORT HDevicesSetupData
{
H_DISABLE_COPY(HDevicesSetupData)

private:

    QHash<HResourceType, HDeviceSetup*> m_deviceSetupInfos;

public:

    /*!
     * Creates a new, empty instance.
     *
     * \sa isEmpty()
     */
    HDevicesSetupData();

    /*!
     * Destroys the instance.
     *
     * Destroys the instance.
     */
    ~HDevicesSetupData();

    /*!
     * Indicates if the instance contains an item with the
     * specified device type.
     *
     * \param deviceType specifies the device type of the searched item.
     *
     * \return \e true when the instance contains an item with the specified
     * device type.
     *
     * \sa get()
     */
    bool contains(const HResourceType& deviceType) const;

    /*!
     * Returns the device types of the contained items.
     *
     * \return the device types of the contained items.
     */
    QSet<HResourceType> deviceTypes() const;

    /*!
     * Retrieves an item.
     *
     * \param type specifies the device type of the item.
     *
     * \return the item with the specified device type. A null pointer is returned
     * in case no item with the specified device type was found.
     *
     * \remarks the ownership of the object is \b not transferred.
     *
     * \sa take(), contains()
     */
    HDeviceSetup* get(const HResourceType& type) const;

    /*!
     * Indicates if the object is empty.
     *
     * \return \e true in case the instance has no items.
     */
    bool isEmpty() const;

    /*!
     * Returns the number of contained items.
     *
     * \return the number of contained items.
     */
    qint32 size() const;

    /*!
     * Inserts a new item.
     *
     * \param newItem specifies the item to be added.
     *
     * \return \e true in case the item was added. The \c newItem will not be added
     * if the instance already contains an item that has the
     * same HDeviceSetup::deviceType() as the \c newItem or the \c newItem is null.
     *
     * \remarks
     * \li The \c newItem has to be heap-allocated and
     * \li the instance takes the ownership of the \c newItem, even if it is not
     * added. If the item is not added the item is deleted.
     */
    bool insert(HDeviceSetup* newItem);

    /*!
     * Removes an existing item.
     *
     * \param type specifies the device type of the item to be removed.
     *
     * \return \e true in case the item was found and removed.
     */
    bool remove(const HResourceType& type);

    /*!
     * Associates an HDevice pointer with an item.
     *
     * \param type specifies the device type of the item.
     *
     * \param device specifies the device to be associated. This instance
     * takes the ownership of the HDevice.
     *
     * \return \e true when an item with the specified service ID was found
     * and the provided HDevice pointer was associated with it. Note also that
     * the pointer can be null.
     *
     * \remarks if an item with the specified device type exists and it already has
     * an HDevice pointer associated with it, the existing HDevice is deleted.
     */
    bool setDevice(const HResourceType& type, HDevice* device);

    /*!
     * Retrieves an item and removes it from the instance.
     *
     * \param type specifies the device type of the item.
     *
     * \return the item with the specified device type. A null pointer is returned
     * in case no item with the specified device type was found.
     *
     * \remarks the ownership of the object \b is transferred to the caller.
     *
     * \sa get()
     */
    HDeviceSetup* take(const HResourceType& type);
};

}
}

#endif /* HDEVICES_SETUPDATA_H_ */
