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

#ifndef HSERVICE_H_
#define HSERVICE_H_

#include <HUpnpCore/HAsyncOp>

#include <QtCore/QList>
#include <QtCore/QObject>

class QUrl;
class QString;

namespace Herqq
{

namespace Upnp
{

class HObjectCreator;
class HServicePrivate;
class HServiceController;

/*!
 * \brief An abstract base class that represents a UPnP service hosted by an
 * HDevice.
 *
 * \c %HService is a core component of the HUPnP \ref hupnp_devicemodel
 * and it models a UPnP service. The UPnP Device Architecture specifies a
 * UPnP service as <em>"Logical functional unit. Smallest units of control. Exposes
 * actions and models the state of a physical device with state variables"</em>.
 * In other words, a UPnP service is the entry point for accessing
 * certain type of functionality and state of the containing device.
 *
 * <h2>Using the class</h2>
 *
 * You can retrieve the containing device, the \em parent \em device, using parentDevice().
 * You can retrieve all the actions the service contains by calling actions(), or
 * if you know the name of the action you can call the actionByName(). Similarly,
 * you can retrieve all the state variables the service contains by calling
 * stateVariables(), or if you know the name of the state variable you can call
 * stateVariableByName().
 *
 * The class exposes all the details in the device description concerning a
 * service through info(), which returns a const reference to a HServiceInfo
 * instance. From this class you can retrieve the \e serviceId and \e serviceType
 * along with various URLs found in the device description, such as the:
 * \li \e scpdUrl, which returns the URL for fetching the service description,
 * \li \e controlUrl, which returns the URL to be used in action invocation and
 * \li \e eventSubUrl, which returns the URL used in event (un)subscriptions.
 *
 * However, the above URLs usually provide informational value only, since
 * HUPnP provides a simpler interface for everything those URLs expose:
 * \li You can retrieve the service description of a service using description().
 * \li Action invocation is abstracted into the HAction class.
 * \li You can receive all the event notifications from a UPnP service by connecting
 * to the stateChanged() signal. You do not need to worry about UPnP eventing at all,
 * since HUPnP handles that for you.
 *
 * <h2>Sub-classing</h2>
 *
 * In case you have written your own server-side HDevice that exposes UPnP
 * services, you need to write corresponding \c %HService classes,
 * which you instantiate in your HDevice::createServices() method.
 *
 * Writing a custom \c %HService is simple, since usually you are required
 * to override createActions() only. That is, if your service defines actions.
 * This is the place where you plug-in the functionality
 * of your UPnP device by providing the implementations of the UPnP actions
 * defined in your service description documents.
 *
 * Consider an example:
 *
 * \code
 *
 * #include "myswitchpower.h" // your code
 *
 * Herqq::Upnp::HActionsSetupData MySwitchPower::createActions()
 * {
 *     Herqq::Upnp::HActionsSetupData retVal;
 *
 *     retVal.insert(
 *         HActionSetup(
 *             "SetTarget",
 *             Herqq::Upnp::HActionInvoke(this, &MySwitchPower::setTarget)));
 *
 *     retVal.insert(
 *         HActionSetup(
 *             "GetTarget",
 *             Herqq::Upnp::HActionInvoke(this, &MySwitchPower::getTarget)));
 *
 *     retVal.insert(
 *         HActionSetup(
 *             "GetStatus",
 *             Herqq::Upnp::HActionInvoke(this, &MySwitchPower::getStatus)));
 *
 *     // The above binds member functions to the specified action names.
 *     // However, you could also use normal functions and functors.
 *
 *     return retVal;
 * }
 *
 * \endcode
 *
 * In the above example three member functions of a fictional class named
 * \c MySwitchPower are bound to actions named \c SetTarget,
 * \c GetTarget and \c GetStatus. In this particular case, these action names
 * have to be defined in the service's description document, but that is not
 * necessarily always the case (more on that later).
 * On the other hand, every action defined in the description document is
 * \b always required to have an implementation bound and returned by the
 * createActions(). The above example uses member functions as arguments to
 * Herqq::Upnp::HActionInvoke, but you can use other
 * <em>callable entities</em> as well, such as normal functions and functors.
 * Once set up properly, it is these callable entities that are called whenever
 * the corresponding actions are invoked.
 *
 * \headerfile hservice.h HService
 *
 * \ingroup hupnp_devicemodel
 *
 * \sa hupnp_devicemodel
 *
 * \remarks The methods introduced in this class are thread-safe, but the \c QObject
 * base class is largely not. However, the signal stateChanged() has thread affinity
 * and any connections to it \b must be done in the thread where
 * the instance of HService resides.
 */
class H_UPNP_CORE_EXPORT HService :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HService)
H_DECLARE_PRIVATE(HService)
friend class HObjectCreator;
friend class HServiceController;

protected:

    /*!
     * Creates and returns the actions the service exposes.
     *
     * It is very important to note that every descendant that specifies
     * actions has to override this. In addition, the override of this method
     * should always call the implementation of the super class too. For instance,
     *
     * \code
     *
     * void HService::HActionsSetupData MyServiceType::createActions()
     * {
     *     HActionsSetupData retVal = SuperClass::createActions();
     *
     *     // create and add the actions of this class to the "retVal" variable
     *
     *     return retVal;
     * }
     *
     * \endcode
     *
     * Most commonly this method is called only once when the instance
     * is being initialized by the managing host (HDeviceHost or HControlPoint).
     *
     * \return the actions that this \c %HService exposes.
     */
    virtual HActionsSetupData createActions();

    /*!
     * Creates and returns setup information about the state variables
     * the service exposes.
     *
     * The purpose of this method is to enable the custom HService to pass
     * information of its state variables to HUPnP, which uses that information
     * -if available- to verify that service descriptions are properly setup
     * in respect to the custom HService class. The benefit of this is that
     * your HService class can rest assured that once it is up and running all
     * the required state variables are properly defined in the service description.
     * This is important for two reasons:
     * - at server side the service description is the mechanism used to marshal
     * device model information to clients. If the service description does not
     * accurately reflect the back-end classes, the client side may not be able
     * to correctly invoke the server-side.
     * - at client side HUPnP can use this information to verify
     * that the server-side is publishing a device in a way the client-side
     * understands.
     *
     * In any case, overriding this method is always optional, but if you
     * override it, remember to call the implementation of the super class too.
     * For instance,
     *
     * \code
     *
     * void HStateVariablesSetupData MyServiceType::createActions()
     * {
     *     HStateVariablesSetupData retVal = SuperClass::stateVariablesSetupData();
     *
     *     // modify the "retVal" as desired.
     *
     *     return retVal;
     * }
     *
     * \endcode
     *
     * Most commonly this method is called only once when the instance
     * is being initialized by the managing host (HDeviceHost or HControlPoint).
     *
     * \return information about the state variables this \c %HService exposes.
     */
    virtual HStateVariablesSetupData stateVariablesSetupData() const;

    /*!
     * Provides an opportunity to do post-construction initialization routines
     * in derived classes.
     *
     * As \c %HService is part of the HUPnP's \ref hupnp_devicemodel
     * the object creation process is driven by HUPnP. At the time
     * of instantiation of a descendant \c %HService the base \c %HService
     * sub-object is not yet fully set up. In other words, at that time
     * it is not guaranteed that every private or protected member of a
     * \c %HService is set to its "final" value that is used once the object
     * is fully initialized and ready to be used.
     *
     * Because of the above, descendants of
     * \c %HService should not reference or rely on values of \c %HService at
     * the time of construction. If the initialization of a \c %HService
     * descendant needs to do things that rely on \c %HService being fully
     * set up, you can override this method. This method is called \b once
     * right after the base \c %HService is fully initialized.
     *
     * \param errDescription
     *
     * \return \e true in case the initialization succeeded.
     *
     * \note It is advisable to keep the constructors of the descendants of
     * \c %HService small and fast, and do more involved initialization routines
     * here.
     */
    virtual bool finalizeInit(QString* errDescription);

protected:

    HServicePrivate* h_ptr;

    /*!
     * Creates a new instance.
     *
     * Default constructor for derived classes.
     */
    HService();

    //
    // \internal
    //
    // Creates a new instance and re-uses the h-ptr.
    //
    HService(HServicePrivate& dd);

public:

    /*!
     * Destroys the instance.
     *
     * Destroys the instance.
     */
    virtual ~HService() = 0;

    /*!
     * Returns the parent HDevice that contains this service instance.
     *
     * \return the parent HDevice that contains this service instance.
     */
    HDevice* parentDevice() const;

    /*!
     * Returns information about the service that is read from the
     * device description.
     *
     * \return information about the service that is read from the
     * device description.
     */
    const HServiceInfo& info() const;

    /*!
     * Returns the full service description.
     *
     * \return the full service description.
     */
    const QString& description() const;

    /*!
     * Returns the actions the service supports.
     *
     * \return the actions the service supports.
     *
     * \remarks the ownership of the returned objects is not transferred.
     * Do \b not delete the returned objects.
     */
    HActions actions() const;

    /*!
     * Attempts to retrieve an action by name.
     *
     * \param name specifies the name of the action to be returned.
     * This is the name of the action specified in the \e service \e description.
     *
     * \return a pointer to an action supported by this service if the provided
     * name is valid or a null pointer otherwise.
     *
     * \remarks the ownership of the object is not transferred. Do \b not delete
     * the returned object.
     */
    HAction* actionByName(const QString& name) const;

    /*!
     * Returns the state variables of the service.
     *
     * \return the state variables of the service.
     *
     * \remarks the ownership of the returned objects is not transferred.
     * Do \b not delete the returned objects.
     */
    HStateVariables stateVariables() const;

    /*!
     * Attempts to retrieve a state variable by name.
     *
     * \param name specifies the name of the state variable to be returned.
     * This is the name of the state variable specified in the \e service \e description.
     *
     * \return a pointer to a state variable found in this service if the provided
     * name is valid or a null pointer otherwise.
     *
     * \remarks the ownership of the object is not transferred. Do \b not delete
     * the returned object.
     */
    HStateVariable* stateVariableByName(const QString& name) const;

    /*!
     * Indicates whether or not the service contains state variables that
     * are evented.
     *
     * \return \e true in case the service contains one or more state variables
     * that are evented.
     *
     * \remarks in case the service is not evented, the stateChanged() signal
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
    void stateChanged(const Herqq::Upnp::HService* source);
};

}
}

#endif /* HSERVICE_H_ */
