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

#ifndef HUPNP_GLOBAL_H_
#define HUPNP_GLOBAL_H_

#include "hdefs_p.h"
#include "hupnp_fwd.h"

namespace Herqq
{

namespace Upnp
{

/*!
 * \file
 * This file contains public functions and enumerations.
 */

 /*!
 * Specifies the logging levels that can be used with the device host.
 */
enum LogLevel
{
    /*!
     * No logs are generated.
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
     * All, but debug level messages are logged. An informational message is used
     * to log status information of control flow. A good example of an informational
     * message is when a sizable component logs the start of an initialization procedure.
     */
    Information = 4,

    /*!
     * All up to the debug messages are output. This excludes only the function
     * enter and exit messages.
     *
     * \remark Enabling this level of logging has notable effect on performance.
     * This generally should be used only on debugging purposes.
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
void H_UPNP_CORE_EXPORT SetLoggingLevel(LogLevel level);

/*!
 * Enables / disables warnings that relate to non-standard behavior discovered
 * in other UPnP software.
 *
 * Most often if non-standard behavior in other UPnP software is discovered, it
 * isn't fatal or critical and it may be possible to inter-operate with the software.
 * However, deviations from the specifications and standards are unfortunate and such
 * \b errors should be fixed.
 *
 * Regardless, you may not want to be notified about these warnings, in which
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
