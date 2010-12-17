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

#ifndef HSERVERSERVICE_H_
#define HSERVERSERVICE_H_

#include <HUpnpCore/HUpnp>
#include <HUpnpCore/HActionInvoke>

#include <QtCore/QList>
#include <QtCore/QObject>

class QUrl;
class QString;

namespace Herqq
{

namespace Upnp
{

class HServerServicePrivate;

/*!
 * \brief This is an abstract base class for server-side UPnP services.
 *
 * \c %HServerService is a core component of the HUPnP's server-side \ref hupnp_devicemodel
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
 * \li Action invocation is abstracted into the HServerAction class.
 * \li You can receive all the event notifications from a UPnP service by connecting
 * to the stateChanged() signal.
 *
 * <h2>Sub-classing</h2>
 *
 * Writing a custom \c %HServerService is simple, because you only have to
 * override createActionInvokes(), in which you provide the implementations of the
 * actions of the service. See \ref builddevice_tutorial and \ref setting_up_the_devicemodel
 * for more information about creating your own HServerServices with custom
 * functionality.
 *
 * \headerfile hserverservice.h HServerService
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa hupnp_devicemodel
 *
 * \remarks This class is not thread-safe.
 */
class H_UPNP_CORE_EXPORT HServerService :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HServerService)
H_DECLARE_PRIVATE(HServerService)
friend class HServerModelCreator;

protected:

    /*!
     * This is a type definition for a hash table containing HActionInvoke objects
     * keyed with strings representing the names of the actions.
     *
     * \ingroup hupnp_devicemodel
     *
     * \sa HActionInvoke
     */
    typedef QHash<QString, HActionInvoke> HActionInvokes;

    /*!
     * Creates and returns the "action implementations" of the actions the
     * service exposes.
     *
     * It is very important to note that the overrides of this method
     * should always call the implementation of the super class too. For instance,
     *
     * \code
     *
     * void HServerService::HActionInvokes MyServiceType::createActionInvokes()
     * {
     *     HActionInvokes retVal = SuperClass::createActionInvokes();
     *
     *     // create and add the actions of this class to the "retVal" variable
     *
     *     return retVal;
     * }
     *
     * \endcode
     *
     * \return the implementations of the actions this \c %HServerService exposes.
     */
    virtual HActionInvokes createActionInvokes() = 0;

    /*!
     * Provides an opportunity to do post-construction initialization routines
     * in derived classes.
     *
     * As \c %HServerService is part of the HUPnP's server-side \ref hupnp_devicemodel,
     * its initialization process is usually driven by HUPnP. If this is the case,
     * at the time of instantiation of a descendant \c %HServerService the base
     * \c %HServerService sub-object is not yet fully set up. In other words,
     * at that time it is not guaranteed that every private or protected member
     * of a \c %HServerService are set to their "final" values that are used
     * once the object is fully initialized and ready to be used.
     *
     * Because of the above, descendants of \c %HServerService should not
     * reference or rely on values of \c %HServerService at
     * the time of construction. If the initialization of a \c %HServerService
     * descendant needs to do something that rely on \c %HServerService being fully
     * set up, you can override this method. This method is called \b once
     * right after the base \c %HServerService is fully initialized.
     *
     * \param errDescription
     *
     * \return \e true in case the initialization succeeded.
     *
     * \note It is advisable to keep the constructors of the descendants of
     * \c %HServerService small and fast, and do more involved initialization routines
     * here.
     */
    virtual bool finalizeInit(QString* errDescription);

protected:

    HServerServicePrivate* h_ptr;

    /*!
     * Creates a new instance.
     *
     * You have to call init() to fully initialize the instance.
     *
     * \sa init()
     */
    HServerService();

    //
    // \internal
    //
    // Creates a new instance and re-uses the h_ptr.
    //
    HServerService(HServerServicePrivate& dd);

    /*!
     * Initializes the instance.
     *
     * This method will succeed only once after construction. Subsequent
     * calls will do nothing.
     *
     * \param info specifies information of the service. This is usually read
     * from a device description document.
     *
     * \param parentDevice specifies the UPnP device instance that contains
     * this service.
     */
    bool init(const HServiceInfo& info, HServerDevice* parentDevice);

public:

    /*!
     * Destroys the instance.
     *
     * Destroys the instance.
     */
    virtual ~HServerService() = 0;

    /*!
     * Returns the parent HServerDevice that contains this service instance.
     *
     * \return the parent HServerDevice that contains this service instance.
     */
    HServerDevice* parentDevice() const;

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
    const QString& description() const;

    /*!
     * Returns the actions the service contains.
     *
     * \return the actions the service contains.
     *
     * \remarks The ownership of the returned objects is not transferred.
     * Do \b not delete the returned objects.
     */
    const HServerActions& actions() const;

    /*!
     * Returns the state variables of the service.
     *
     * \return the state variables of the service.
     *
     * \remarks The ownership of the returned objects is not transferred.
     * Do \b not delete the returned objects.
     */
    const HServerStateVariables& stateVariables() const;

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
    void stateChanged(const Herqq::Upnp::HServerService* source);
};

}
}

#endif /* HSERVERSERVICE_H_ */
