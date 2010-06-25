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

#ifndef HDEVICEPROXY_CREATOR_H_
#define HDEVICEPROXY_CREATOR_H_

#include "../../general/hupnp_fwd.h"
#include "../../../utils/hfunctor.h"

/*!
 * \file
 */

namespace Herqq
{

namespace Upnp
{

/*!
 * This is a type definition for a <em>callable entity</em> that is used
 * to create HDeviceProxy instances.
 *
 * You can create \c %HDeviceProxyCreator objects using normal functions, functors and
 * member functions that follow the signature of
 *
 * <tt>
 *
 * Herqq::Upnp::HDeviceProxy* function(const Herqq::Upnp::HDeviceInfo&);
 *
 * </tt>
 *
 * The following example demonstrates how you can instantiate the \c %HDeviceProxyCreator
 * for a normal function, functor and a member function.
 *
 * \code
 *
 * #include <HDeviceProxyCreator>
 *
 * #include "myclass.h"         // your code that contains the declaration for MyClass
 * #include "my_hdeviceproxy.h" // your code that contains the declaration of MyHDeviceProxy
 *
 * namespace
 * {
 * Herqq::Upnp::HDeviceProxy* freefun(const Herqq::Upnp::HDeviceInfo&)
 * {
 *     return new MyHDeviceProxy();
 * }
 *
 * class MyFunctor
 * {
 * public:
 *     Herqq::Upnp::HDeviceProxy* operator(const Herqq::Upnp::HDeviceInfo&)
 *     {
 *         return new MyHDeviceProxy();
 *     }
 * };
 * }
 *
 * Herqq::Upnp::HDeviceProxy* MyClass::memfun(const Herqq::Upnp::HDeviceInfo&)
 * {
 *     return new MyHDeviceProxy();
 * }
 *
 * void MyClass::example()
 * {
 *     Herqq::Upnp::HDeviceProxyCreator usingFreeFunction(freefun);
 *
 *     MyFunctor myfunc;
 *     Herqq::Upnp::HDeviceProxyCreator usingFunctor(myfunc);
 *
 *     Herqq::Upnp::HDeviceProxyCreator usingMemberFunction(this, &MyClass::memfun);
 * }
 *
 * \endcode
 *
 * You can test if the object can be invoked simply by issuing
 * <tt>if (deviceCreatorObject) { ... } </tt>
 *
 * \headerfile hdeviceproxy_creator.h HDeviceProxyCreator
 *
 * \ingroup devicehosting
 */
typedef Functor<Herqq::Upnp::HDeviceProxy*, H_TYPELIST_1(
    const Herqq::Upnp::HDeviceInfo&)> HDeviceProxyCreator;

}
}

#endif /* HDEVICEPROXY_CREATOR_H_ */
