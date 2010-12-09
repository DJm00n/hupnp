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

#ifndef HCLIENTSERVICE_H_
#define HCLIENTSERVICE_H_

#include <HUpnpCore/HAsyncOp>

#include <QtCore/QObject>

class QUrl;

namespace Herqq
{

namespace Upnp
{

class HClientServicePrivate;

/*!
 * \brief A client-side class that represents a server-side UPnP service.
 *
 * \c %HClientService is a core component of the HUPnP's client-side \ref hupnp_devicemodel
 * and it models a UPnP service. The UPnP Device Architecture specifies a
 * UPnP service as <em>"Logical functional unit. Smallest units of control. Exposes
 * actions and models the state of a physical device with state variables"</em>.
 * In other words, a UPnP service is the entry point for accessing
 * certain type of functionality and state of the containing device.
 *
 * <h2>Using the class</h2>
 *
 * You can retrieve the containing device, the \e parent \e device, using parentDevice().
 * You can retrieve all the actions the service contains by calling actions(),
 * Similarly, you can retrieve all the state variables of the service by calling
 * stateVariables().
 *
 * The class exposes all the details in the device description concerning a
 * service through info(). From the returned HServiceInfo object you can
 * retrieve the \e serviceId and \e serviceType along with various URLs found
 * in the device description, such as the:
 * \li \e scpdUrl, which returns the URL for fetching the service description,
 * \li \e controlUrl, which returns the URL to be used in action invocation and
 * \li \e eventSubUrl, which returns the URL used in event (un)subscriptions.
 *
 * However, the above URLs usually provide informational value only, since
 * HUPnP provides a simpler interface for everything those URLs expose:
 * \li You can retrieve the service description of a service using description().
 * \li Action invocation is abstracted into the HClientAction class.
 * \li You can receive all the event notifications from a UPnP service by connecting
 * to the stateChanged() signal. You do not need to worry about UPnP eventing at all,
 * since HUPnP handles that for you.
 *
 * \headerfile hclientservice.h HClientService
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa hupnp_devicemodel
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HClientService :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HClientService)
H_DECLARE_PRIVATE(HClientService)

protected:

    HClientServicePrivate* h_ptr;

    /*!
     * Creates a new instance.
     *
     * \param info specifies information of the service.
     *
     * \param parentDevice specifies the device instance that contains this
     * service.
     *
     * Default constructor for derived classes.
     */
    HClientService(const HServiceInfo& info, HClientDevice* parentDevice);

public:

    /*!
     * Destroys the instance.
     *
     * Destroys the instance.
     */
    virtual ~HClientService() = 0;

    /*!
     * Returns the parent HClientDevice that contains this service instance.
     *
     * \return the parent HClientDevice that contains this service instance.
     */
    HClientDevice* parentDevice() const;

    /*!
     * Returns information about the service.
     *
     * \return information about the service. This information is usually read
     * from a device description document.
     */
    const HServiceInfo& info() const;

    /*!
     * Returns the full service description.
     *
     * \return the full service description.
     */
    QString description() const;

    /*!
     * Returns the actions the service supports.
     *
     * \return the actions the service supports.
     *
     * \remarks The ownership of the returned objects is not transferred.
     * Do \b not delete the returned objects.
     */
    const HClientActions& actions() const;

    /*!
     * Returns the state variables of the service.
     *
     * \return the state variables of the service.
     *
     * \remarks The ownership of the returned objects is not transferred.
     * Do \b not delete the returned objects.
     */
    const HClientStateVariables& stateVariables() const;

    /*!
     * Indicates whether or not the service contains state variables that
     * are evented.
     *
     * \return \e true in case the service contains one or more state variables
     * that are evented.
     *
     * \remarks In case the service is not evented, the stateChanged() signal
     * will never be emitted and the notifyListeners() method does nothing.
     */
    bool isEvented() const;

public Q_SLOTS:

    /*!
     * Explicitly forces stateChanged() event to be emitted if the service is
     * evented. Otherwise this method does nothing.
     */
    void notifyListeners();

Q_SIGNALS:

    /*!
     * This signal is emitted when the state of one or more state variables
     * has changed.
     *
     * \param source specifies the source of the event.
     *
     * \remarks This signal has thread affinity to the thread where the object
     * resides. Do not connect to this signal from other threads.
     */
    void stateChanged(const Herqq::Upnp::HClientService* source);
};

}
}

#endif /* HCLIENTSERVICE_H_ */