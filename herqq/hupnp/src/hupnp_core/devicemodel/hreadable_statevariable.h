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

#ifndef HREADABLE_STATEVARIABLE_H_
#define HREADABLE_STATEVARIABLE_H_

#include <HUpnpCore/HStateVariable>

namespace Herqq
{

namespace Upnp
{

/*!
 * Class that provides \e read-only access to a state variable.
 *
 * \c %HReadableStateVariable is a core component of the HUPnP \ref devicemodel
 * and it models a UPnP state variable, which allows \e read-only access. Typically,
 * instances of this class are available only on client-side.
 *
 * \headerfile hreadable_statevariable.h HReadableStateVariable
 *
 * \ingroup devicemodel
 *
 * \sa devicehosting, \ref statevariables,
 * HStateVariable, HWritableStateVariable, HService
 *
 * \remark
 * \li the value of the state variable can be changed internally by the class
 * that is hosting the state variable.
 * \li the methods introduced in this class are thread-safe, but the \c QObject
 * base class is largely not.
 */
class HReadableStateVariable :
    public HStateVariable
{
H_DISABLE_COPY(HReadableStateVariable)
friend class HObjectCreator;
friend class HStateVariableController;

private:

    //
    // \internal
    //
    // Constructs a new instance.
    //
    HReadableStateVariable(HService* parent);

public:

    /*!
     * Destroys the instance.
     *
     * An \c %HReadableStateVariable is always destroyed by the containing HService when it
     * is being deleted. You should never destroy an \c %HReadableStateVariable.
     */
    virtual ~HReadableStateVariable();
};

}
}

#endif /* HREADABLE_STATEVARIABLE_H_ */
