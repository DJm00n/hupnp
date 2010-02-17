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

#ifndef HUPNP_SERVICE_H_
#define HUPNP_SERVICE_H_

#include "hactioninvoke.h"
#include "./../general/hdefs_p.h"
#include "./../general/hupnp_fwd.h"

#include <QList>
#include <QHash>
#include <QObject>

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
 * \c %HService is a core component of the HUPnP \ref devicemodel
 * and it models a UPnP service. The UPnP Device Architecture specifies a
 * UPnP service as <em>"Logical functional unit. Smallest units of control. Exposes
 * actions and models the state of a physical device with state variables"</em>.
 * In other words, a UPnP service is the entry point for accessing
 * certain type of functionality and state of the containing device.
 *
 * <h2>Using the class</h2>
 *
 * You can retrieve the containing device, the \em parent \em device, using parentDevice().
 * You can retrieve all the actions the service contains by calling actions() or
 * if you know the name of the action, you can call the actionByName(). Similarly,
 * you can retrieve all the state variables the service contains by calling
 * stateVariables() or if you know the name of the state variable, you can call
 * stateVariableByName().
 *
 * The class exposes all the details in the device description concerning a
 * service, such as the serviceId() and serviceType(). You can also get the
 * various URLs found in the device description:
 * \li scpdUrl() returns the URL for fetching the service description,
 * \li controlUrl() returns the URL to be used in action invocation and
 * \li eventSubUrl() returns the URL used in event (un)subscriptions.
 *
 * However, the above URLs usually provide informational value only, since
 * HUPnP provides a simpler interface for everything those URLs expose:
 * \li You can retrieve a service description of a service simply
 * by calling serviceDescription().
 * \li Action invocation is abstracted into HAction.
 * \li You can receive all the event notifications from a UPnP service by connecting
 * to the stateChanged() signal. You do not need to worry about UPnP eventing at all,
 * since HUPnP handles that for you.
 *
 * <h2>Sub-classing</h2>
 *
 * In case you have written your own HDevice that exposes UPnP services, you need
 * to write corresponding \c %HService classes, which you instantiate in your
 * HDevice::createServices() method.
 *
 * However, writing a custom \c %HService is simple, since you are required to implement
 * createActions() only. This is the place where you plug-in the functionality
 * of your UPnP device and it is done as UPnP actions found in your UPnP services.
 *
 * Consider an example:
 *
 * \code
 *
 * #include "myswitchpower.h" // your code
 *
 * Herqq::Upnp::HService::HActionMapT MySwitchPower::createActions()
 * {
 *     Herqq::Upnp::HService::HActionMapT retVal;
 *
 *     retVal["SetTarget"] =
 *         Herqq::Upnp::HActionInvoke(this, &MySwitchPower::setTarget);
 *
 *     retVal["GetTarget"] =
 *         Herqq::Upnp::HActionInvoke(this, &MySwitchPower::getTarget);
 *
 *     retVal["GetStatus"] =
 *         Herqq::Upnp::HActionInvoke(this, &MySwitchPower::getStatus);
 *
 *     // the above binds member functions to the specified action names.
 *     // however, you could also use free functions and functors
 *
 *     return retVal;
 * }
 *
 * \endcode
 *
 * In the above example three member functions of a fictional class named \c MySwitchPower
 * are bound to actions named \c SetTarget, \c GetTarget and \c GetStatus.
 * These action names have to be defined in the service's description file and similarly,
 * every action name found in the service's description file has to have an implementation
 * bound and returned by the createActions(). The above example uses member functions
 * as arguments to Herqq::Upnp::HActionInvoke, but you can use other <em>callable entities</em> as well,
 * such as free functions and functors.
 *
 * Once set up properly, it is these callable entities that are called whenever
 * the corresponding actions are invoked.
 *
 * \headerfile hservice.h HService
 *
 * \ingroup devicemodel
 *
 * \sa devicemodel
 *
 * \remark the methods introduced in this class are thread-safe, but the \c QObject
 * base class is largely not.
 */
class H_UPNP_CORE_EXPORT HService :
    public QObject
{
Q_OBJECT
H_DISABLE_COPY(HService)
H_DECLARE_PRIVATE(HService)
friend class HObjectCreator;
friend class HServiceController;

public: // typedefs

    /*!
     * Type definition for a map holding HAction names as keys and
     * callable entities named Herqq::Upnp::HActionInvoke as values.
     *
     * \sa HActionInvoke, createActions()
     */
    typedef QHash<QString, HActionInvoke> HActionMapT;

private:

    /*!
     * Creates and returns the actions the service exposes.
     *
     * Every descendant has to override this.
     *
     * This method is called once when the device is being initialized by the managing device host.
     *
     * \return the actions that this \c %HService exposes.
     *
     * \remark The base class takes the ownership of the created actions and will
     * delete them upon destruction. Because of that, you can store the
     * addresses of the created actions and use them safely throughout the lifetime
     * of the containing service object. However, you cannot delete them.
     */
    virtual HActionMapT createActions() = 0;

protected:

    HServicePrivate* h_ptr;

    /*!
     * Creates a new instance.
     */
    HService();

    //
    // \internal
    //
    // Creates a new instance and re-uses the h-ptr.
    //
    HService(HServicePrivate& dd);

    /*!
     * Attempts to change the value of the specified state variable.
     *
     * \param stateVarName specifies the state variable to be changed.
     * \param value specifies the new value.
     *
     * \retval true in case the specified state variable was successfully modified.
     * \retval false in case the state variable wasn't modified.
     *
     * \attention the state of a state variable cannot be modified directly
     * from a control point. If a call to this method is made from a \c %HService
     * that is hosted in a control point, this method returns false. This is because
     * the UPnP Device Architecture does not support such a procedure.
     */
    bool setStateVariableValue(
        const QString& stateVarName, const QVariant& value);

    /*!
     * Attempts to change the values of the specified state variables.
     *
     * \param values specifies a map, in which keys identify the state variables
     * by name and corresponding values represent the new state variable values.
     *
     * \retval true in case all the specified state variables were successfully modified.
     *
     * \retval false otherwise. In this case, none of the state variables were
     * changed.
     *
     * \attention the state of a state variable cannot be modified directly
     * from a control point. If a call to this method is made from a \c %HService
     * that is hosted in a control point, this method returns false. This is because
     * the UPnP Device Architecture does not support such a procedure.
     */
    bool setStateVariableValues(const QHash<QString, QVariant>& values);

public:

    /*!
     * Destroys the instance.
     *
     * An \c %HService is always destroyed by the containing HDevice when it
     * is being deleted. You should never destroy an \c %HService.
     */
    virtual ~HService() = 0;

    /*!
     * Returns the parent HDevice that contains this
     * service instance.
     *
     * \return the parent HDevice that contains this
     * service instance.
     */
    HDevice* parentDevice() const;

    /*!
     * Returns the service identifier found in the device description file.
     *
     * \return the service identifier found in the device description file.
     */
    HServiceId serviceId() const;

    /*!
     * Returns the type of the service found in the device description file.
     *
     * \return the type of the service found in the device description file.
     */
    HResourceType serviceType() const;

    /*!
     * Returns the URL for service description.
     *
     * This is the URL where the service description can be retrieved.
     * This is defined in the device description.
     *
     * \return the URL for service description.
     */
    QUrl scpdUrl() const;

    /*!
     * Returns the URL for control.
     *
     * This is the URL to which the action invocations must be sent.
     * This is defined in the device description.
     *
     * \return the URL for control.
     */
    QUrl controlUrl() const;

    /*!
     * Returns the URL for eventing.
     *
     * This is the URL to which subscriptions and un-subscriptions are sent.
     * This is defined in the device description.
     *
     * \return the URL for eventing.
     */
    QUrl eventSubUrl() const;

    /*!
     * Returns the full service description.
     *
     * \return full service description.
     */
    QString serviceDescription() const;

    /*!
     * Returns the actions the service supports.
     *
     * \return the actions the service supports.
     */
    QList<HAction*> actions() const;

    /*!
     * Attempts to retrieve an action by name.
     *
     * \param name specifies the name of the action to be returned.
     * This is the name of the action specified in the \e service \e description.
     *
     * \return a pointer to an action supported by this service if the provided
     * name is valid or a null pointer otherwise.
     */
    HAction* actionByName(const QString& name) const;

    /*!
     * Returns the state variables of the service.
     *
     * \return the state variables of the service.
     */
    QList<HStateVariable*> stateVariables() const;

    /*!
     * Attempts to retrieve a state variable by name.
     *
     * \param name specifies the name of the state variable to be returned.
     * This is the name of the state variable specified in the \e service \e description.
     *
     * \return a pointer to a state variable found in this service if the provided
     * name is valid or a null pointer otherwise.
     */
    HStateVariable* stateVariableByName(const QString& name) const;

    /*!
     * Indicates whether or not the service contains state variables that
     * are evented.
     *
     * \return true in case the service contains one or more state variables that
     * are evented.
     *
     * \remark in case the service is not evented, the stateChanged() signal
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
     * This signal is emitted when the state of one or more state variables has changed.
     *
     * \param source specifies the source of the event.
     */
    void stateChanged(const Herqq::Upnp::HService* source);
};

}
}

#endif /* HUPNP_SERVICE_H_ */
