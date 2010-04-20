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

#ifndef HDEVICECREATOR_H_
#define HDEVICECREATOR_H_

#include "./../general/hupnp_fwd.h"
#include "./../../utils/hfunctor.h"

/*!
 * \file
 */

namespace Herqq
{

namespace Upnp
{

/*!
 * This is a type definition for a <em>callable entity</em> that is used
 * to create HDevice instances.
 *
 * You can create \c %HDeviceCreator objects using normal functions, functors and
 * member functions that follow the signature of
 *
 * <tt>
 *
 * Herqq::Upnp::HDevice* function(const Herqq::Upnp::HDeviceInfo&);
 *
 * </tt>
 *
 * The following example demonstrates how you can instantiate the \c %HDeviceCreator
 * for a normal function, functor and a member function.
 *
 * \code
 *
 * #include <HDeviceCreator>
 *
 * #include "myclass.h"    // your code that contains the declaration for MyClass
 * #include "my_hdevice.h" // your code that contains the declaration of MyHDevice
 *
 * namespace
 * {
 * Herqq::Upnp::HDevice* freefun(const Herqq::Upnp::HDeviceInfo&)
 * {
 *     return new MyHDevice();
 * }
 *
 * class MyFunctor
 * {
 * public:
 *     Herqq::Upnp::HDevice* operator(const Herqq::Upnp::HDeviceInfo&)
 *     {
 *         return new MyHDevice();
 *     }
 * };
 * }
 *
 * Herqq::Upnp::HDevice* MyClass::memfun(const Herqq::Upnp::HDeviceInfo&)
 * {
 *     return new MyHDevice();
 * }
 *
 * void MyClass::example()
 * {
 *     Herqq::Upnp::HDeviceCreator usingFreeFunction(freefun);
 *
 *     MyFunctor myfunc;
 *     Herqq::Upnp::HDeviceCreator usingFunctor(myfunc);
 *
 *     Herqq::Upnp::HDeviceCreator usingMemberFunction(this, &MyClass::memfun);
 * }
 *
 * \endcode
 *
 * You can test if the object can be invoked simply by issuing
 * <tt>if (deviceCreatorObject) { ... } </tt>
 *
 * \headerfile hdevicecreator.h HDeviceCreator
 *
 * \ingroup devicehosting
 */
typedef Functor<Herqq::Upnp::HDevice*, H_TYPELIST_1(
    const Herqq::Upnp::HDeviceInfo&)> HDeviceCreator;

}
}

#endif /* HDEVICECREATOR_H_ */
