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

#ifndef HUPNP_GLOBAL_H_
#define HUPNP_GLOBAL_H_

#include <HUpnpCore/public/hupnp_fwd.h>
#include <HUpnpCore/public/hupnp_defs.h>

/*!
 * \file
 * This file contains public functions and enumerations.
 */

namespace Herqq
{

namespace Upnp
{

/*!
 * This enumeration is used to specify the strictness of argument validation.
 */
enum HValidityCheckLevel
{
    /*!
     * The arguments are validated strictly according to the UDA
     * v1.0 and v1.1 specifications.
     */
    StrictChecks,

    /*!
     * The validation allows slight deviations from the UDA specifications
     * in an attempt to improve interoperability. The accepted exceptions
     * have been encountered in other UPnP software that are popular enough
     * to warrant the exceptional behavior.
     */
    LooseChecks
};

/*!
 * This enumeration specifies whether a component of the \ref devicemodel is
 * mandatory within a specific UPnP device.
 *
 * In more detail, any component of the device model
 * (a device, a service, a state variable or an action) may be specified as
 * a mandatory or an optional part of a UPnP device; for example,
 * a UPnP device may have two mandatory embedded devices and one
 * optional embedded device. The same applies to the other components as well.
 *
 * When HUPnP builds an object model of a UPnP device, this information can be
 * used in validating a description document, or verifying that the provided
 * implementation (HDevice, HDeviceProxy, HService and HServiceProxy derivatives)
 * accurately depict a description document.
 *
 * For instance, if the author of a subclass of a server-side HService has
 * specified that a particular action is mandatory, the user of the class,
 * who is the one that provides the description document, has to make sure that
 * the description document also contains the definition of the action.
 *
 * These types of mappings are optional, but they are highly useful in case
 * the component is to be used as a public part of a library.
 * They help to ensure that the implementation back-end reflects the used
 * description documents appropriately. This is important, as it is the
 * description documents that are transferred from servers to clients and it is
 * these documents that advertise what a particular UPnP device supports and
 * is capable of doing.
 *
 * From the client's perspective they are also useful in defining requirements
 * for a particular device type. For instance, if you are an author of a
 * HDeviceProxy or HServiceProxy derivative class and you have written
 * the component to work using a specific configuration that requires the
 * presence of certain actions and state variables on the server side, HUPnP
 * can use these requirements to filter devices that are suitable in terms
 * of advertised capabilities.
 */
enum HInclusionRequirement
{
    /*!
     * This value indicates that the inclusion requirement for the component
     * is not specified.
     *
     * This value is used only in error situations.
     */
    InclusionRequirementUnknown = 0,

    /*!
     * This value indicates that the component has to be appropriately specified.
     * It is a critical error if the component is missing.
     */
    InclusionMandatory,

    /*!
     * This value indicates that the component is optional and may or may not be
     * specified.
     */
    InclusionOptional
};

 /*!
 * This enumeration specifies the logging levels that can be used with the device host.
 */
enum HLogLevel
{
    /*!
     * No logs are generated.
     *
     * \remark by default, HUPnP uses this logging level.
     */
    None = 0,

    /*!
     * Only fatal messages are logged. Most often a fatal message is
     * followed by termination of the application.
     */
    Fatal = 1,

    /*!
     * Only critical and fatal messages are logged. Most often a critical message
     * signals a severe runtime error.
     */
    Critical = 2,

    /*!
     * Messages with level set to warning, critical and fatal are logged.
     * A warning message usually signifies an error or exceptional situation
     * that should be noted. Most often the system stability is not at stake
     * when warning messages appear, but they may still indicate that some
     * component, internal or external, is not functioning correctly.
     * Usually the source of warnings should be investigated.
     */
    Warning = 3,

    /*!
     * All but debug level messages are logged. An informational message is used
     * to log status information of control flow. A good example of an informational
     * message is when a sizable component logs the start of an initialization procedure.
     */
    Information = 4,

    /*!
     * All up to the debug messages are output. This excludes only the function
     * enter and exit messages.
     *
     * \remark Enabling this level of logging has notable effect on performance.
     * This generally should be used only for debugging purposes.
     */
    Debug = 5,

    /*!
     * Every log message is output. This includes even the function enters
     * and exits.
     *
     * \remark Enabling this level of logging has severe effect on performance.
     * This is very rarely needed and usually the debug level is far more helpful.
     */
    All = 6
};

/*!
 * Sets the logging level the HUPnP should use.
 *
 * \param level specifies the desired logging level.
 *
 * \remark
 * \li The new logging level will take effect immediately.
 * \li The function is thread-safe.
 */
void H_UPNP_CORE_EXPORT SetLoggingLevel(HLogLevel level);

/*!
 * Enables / disables warnings that relate to non-standard behavior discovered
 * in other UPnP software.
 *
 * Most often if non-standard behavior in other UPnP software is discovered, it
 * isn't fatal or critical and it may be possible to inter-operate with the software.
 * However, deviations from the specifications and standards are unfortunate and such
 * \b errors should be fixed.
 *
 * Regardless, you may not want to be notified about these warnings in which
 * case you can specifically disable all the warnings that relate to non-standard
 * behavior.
 *
 * \param arg specifies whether to output warnings that are about non-standard
 * behavior in other UPnP software.
 *
 * \remark by default, the non standard behavior warnings are on.
 */
void H_UPNP_CORE_EXPORT EnableNonStdBehaviourWarnings(bool arg);

}
}

#endif /* HUPNP_GLOBAL_H_ */
