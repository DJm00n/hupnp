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

#ifndef HSTATEVARIABLEINFO_H_
#define HSTATEVARIABLEINFO_H_

#include "../general/hupnp_global.h"
#include "../datatypes/hupnp_datatypes.h"

class QString;
class QVariant;

namespace Herqq
{

namespace Upnp
{

class HStateVariableInfoPrivate;

/*!
 * \brief This class is used to contain information of a UPnP state variable
 * found in a UPnP service description document.
 *
 * UPnP service description documents specify the actions and state variables
 * of the service. An instance of this class contain the information of a
 * state variable found in a service description document, such as the
 * name() and the dataType().
 *
 * In addition to the information found in the service description document,
 * the HService containing the HStateVariable that is depicted by the
 * HStateVariableInfo object may have specified additional information
 * about the state variable:
 *
 * - inclusionRequirement() details whether the state variable is considered as
 * mandatory or optional.
 * - maxEventRate() specifies the maximum rate at which an evented
 * state variable may send events.
 *
 * Further, the class contains a few helper methods:
 * - isConstrained() indicates if the state variable is restricted either by
 * a value range or a value list.
 * - isValidValue() checks if a specified \c QVariant contains a value that could be
 * inserted into the state variable taking into consideration the data types
 * of the state variable and the specified value as well as any possible
 * constraint set to the state variable.
 *
 * \headerfile hstatevariableinfo.h HStateVariableInfo
 *
 * \ingroup dataelements
 *
 * \remarks this class is not thread-safe.
 *
 * \sa HStateVariable, HDeviceInfo, HServiceInfo and HActionInfo.
 */
class H_UPNP_CORE_EXPORT HStateVariableInfo
{
friend H_UPNP_CORE_EXPORT bool operator==(
    const HStateVariableInfo&, const HStateVariableInfo&);

public:

    /*!
     * Specifies different types of eventing.
     *
     * \sa devicehosting
     */
    enum EventingType
    {
        /*!
         * The state variable is not evented and it will never emit
         * valueChanged() signal.
         */
        NoEvents = 0,

        /*!
         * The state variable is evented, valueChanged() signal is emitted upon
         * value change and the HUPnP will propagate events over network
         * to registered listeners through unicast only.
         */
        UnicastOnly = 1,

        /*!
         * The state variable is evented, valueChanged() signal is emitted upon
         * value change and the HUPnP will propagate events over network
         * using uni- and multicast.
         */
        UnicastAndMulticast = 2
    };

private:

    HStateVariableInfoPrivate* h_ptr;

public:

    /*!
     * Creates a new, empty instance.
     *
     * \sa isValid()
     */
    HStateVariableInfo();

    /*!
     * Creates a new instance.
     *
     * \param name specifies the name of the state variable.
     *
     * \param dataType specifies the UPnP data type of the state variable.
     *
     * \param eventingType specifies the type of eventing used with the
     * state variable. This is optional.
     *
     * \param incReq specifies whether the service is required or optional.
     * This parameter is optional.
     *
     * \param err specifies a pointer to a \c QString that will contain
     * an error description in case the construction failed. This is optional.
     *
     * \sa isValid()
     */
    HStateVariableInfo(
        const QString& name,
        HUpnpDataTypes::DataType dataType,
        EventingType eventingType = NoEvents,
        HInclusionRequirement incReq = InclusionMandatory,
        QString* err = 0);

    /*!
     * Creates a new instance.
     *
     * \param name specifies the name of the state variable.
     *
     * \param dataType specifies the UPnP data type of the state variable.
     *
     * \param defaultValue specifies the default value.
     *
     * \param eventingType specifies the type of eventing used with the
     * state variable. This is optional.
     *
     * \param incReq specifies whether the service is required or optional.
     * This parameter is optional.
     *
     * \param err specifies a pointer to a \c QString that will contain
     * an error description in case the construction failed. This is optional.
     *
     * \sa isValid()
     */
    HStateVariableInfo(
        const QString& name,
        HUpnpDataTypes::DataType dataType,
        const QVariant& defaultValue,
        EventingType eventingType = NoEvents,
        HInclusionRequirement incReq = InclusionMandatory,
        QString* err = 0);

    /*!
     * Creates a new instance with the data type set to \c HUpnpDataTypes::string.
     *
     * \param name specifies the name of the state variable.
     *
     * \param defaultValue specifies the default value.
     *
     * \param allowedValueList specifies the values the state variable
     * accepts.
     *
     * \param eventingType specifies the type of eventing used with the
     * state variable. This is optional.
     *
     * \param incReq specifies whether the service is required or optional.
     * This parameter is optional.
     *
     * \param err specifies a pointer to a \c QString that will contain
     * an error description in case the construction failed. This is optional.
     *
     * \sa isValid()
     */
    HStateVariableInfo(
        const QString& name,
        const QVariant& defaultValue,
        const QStringList& allowedValueList,
        EventingType eventingType = NoEvents,
        HInclusionRequirement incReq = InclusionMandatory,
        QString* err = 0);

    /*!
     * \param name specifies the name of the state variable.
     *
     * \param dataType specifies the UPnP data type of the state variable.
     *
     * \param defaultValue specifies the default value.
     *
     * \param minimumValue specifies the inclusive lower bound of an
     * acceptable value. This cannot be larger than the \c maximumValue.
     *
     * \param maximumValue specifies the inclusive upper bound of an
     * acceptable value. This cannot be smaller than the \c minimumValue.
     *
     * \param stepValue specifies the step value. This value cannot be
     * larger than the subtraction of the maximum and minimum values.
     *
     * \param eventingType specifies the type of eventing used with the
     * state variable. This is optional.
     *
     * \param incReq specifies whether the service is required or optional.
     * This parameter is optional.
     *
     * \param err specifies a pointer to a \c QString that will contain
     * an error description in case the construction failed. This is optional.
     *
     */
    HStateVariableInfo(
        const QString& name,
        HUpnpDataTypes::DataType dataType,
        const QVariant& defaultValue,
        const QVariant& minimumValue,
        const QVariant& maximumValue,
        const QVariant& stepValue,
        EventingType eventingType = NoEvents,
        HInclusionRequirement incReq = InclusionMandatory,
        QString* err = 0);

    /*!
     * Copy constructor.
     *
     * Creates a new instance identical to the \c other object.
     */
    HStateVariableInfo(const HStateVariableInfo&);

    /*!
     * Assignment operator.
     *
     * Assigns the contents of the \c other to this.
     */
    HStateVariableInfo& operator=(const HStateVariableInfo&);

    /*!
     * Destroys the instance.
     *
     * Destroys the instance.
     */
    ~HStateVariableInfo();

    /*!
     * Returns the name of the action.
     *
     * This is the name specified in the corresponding service description file.
     *
     * \return the name of the action.
     */
    QString name() const;

    /*!
     * Returns the type of the action, i.e. is it required or optional.
     *
     * This is the name specified in the corresponding service description file.
     *
     * \return the type of the action.
     */
    HInclusionRequirement inclusionRequirement() const;

    /*!
     * Specifies whether the depicted state variable is required or optional.
     *
     * \param arg specifies whether the service is required or optional.
     */
    void setInclusionRequirement(HInclusionRequirement arg);

    /*!
     * Returns the maximum rate at which an evented state variable may send
     * events.
     *
     * \return the maximum rate at which an evented state variable may send
     * events. The returned value is -1 if the state variable is not evented.
     *
     * \sa setMaxEventRate(), eventingType()
     */
    qint32 maxEventRate() const;

    /*!
     * Sets the maximum rate at which an evented state variable may send
     * events.
     *
     * \param arg specifies the maximum rate at which an evented state
     * variable may send events. The rate is not set if the state variable is
     * not evented.
     *
     * \sa maxEventRate(), eventingType()
     */
    void setMaxEventRate(qint32 arg);

    /*!
     * Returns the data type of the state variable.
     *
     * \return the data type of the state variable.
     */
    HUpnpDataTypes::DataType dataType() const;

    /*!
     * Returns the type of eventing the state variable supports, if any.
     *
     * \return the type of eventing the state variable supports, if any.
     */
    EventingType eventingType() const;

    /*!
     * Sets the type of eventing the state variable supports, if any.
     *
     * \param arg specifies the type of eventing the state variable supports, if any.
     */
    void setEventingType(EventingType arg);

    /*!
     * Returns the list of allowed values.
     *
     * \return the list of allowed values if the contained data type is string
     * or empty list otherwise.
     *
     * \remarks this is only applicable on state variables, which data type is
     * HUpnpDataTypes::string.
     *
     * \sa setAllowedValueList(), dataType()
     */
    QStringList allowedValueList() const;

    /*!
     * Specifies the values the state variable accepts.
     *
     * \param arg specifies the values the state variable accepts.
     *
     * \remarks this is only applicable on state variables, which data type is
     * HUpnpDataTypes::string.
     *
     * \sa allowedValueList(), dataType()
     */
    bool setAllowedValueList(const QStringList& arg);

    /*!
     * Returns the minimum value of the specified value range.
     *
     * \return the minimum value of the specified value range.
     *
     * \remarks this is only applicable on state variables, which data type is
     * numeric. In addition, it is optional and it may not be defined.
     *
     * \sa dataType()
     */
    QVariant minimumValue() const;

    /*!
     * Returns the maximum value of the specified value range.
     *
     * \return the maximum value of the specified value range.
     *
     * \remarks this is only applicable on state variables, which data type is
     * numeric. In addition, it is optional and it may not be defined.
     *
     * \sa dataType()
     */
    QVariant maximumValue() const;

    /*!
     * Returns the step value of the specified value range.
     *
     * \return the step value of the specified value range.
     *
     * \remarks this is only applicable on state variables, which data type is
     * numeric. In addition, it is optional and it may not be defined.
     *
     * \sa dataType()
     */
    QVariant stepValue() const;

    /*!
     * Sets the allowed value range.
     *
     * \param minimumValue specifies the inclusive lower bound of an
     * acceptable value. This cannot be larger than the \c maximumValue.
     *
     * \param maximumValue specifies the inclusive upper bound of an
     * acceptable value. This cannot be smaller than the \c minimumValue.
     *
     * \param stepValue specifies the step value. This value cannot be
     * larger than the subtraction of the maximum and minimum values.
     *
     * \param err specifies a pointer to a \c QString, which contains an
     * error description in case the any of the provided values is invalid. This
     * parameter is optional.
     *
     * \remarks this is only applicable on state variables, which data type is
     * numeric. In addition, it is optional and it may not be defined.
     *
     * \return \e true in case the values were successfully set.
     */
    bool setAllowedValueRange(
        const QVariant& minimumValue, const QVariant& maximumValue,
        const QVariant& stepValue, QString* err = 0);

    /*!
     * Returns the default value of the state variable.
     *
     * \return the default value of the state variable. If no default has been
     * specified, QVariant::Invalid is returned.
     */
    QVariant defaultValue() const;

    /*!
     * Sets the default value.
     *
     * \param arg specifies the default value. If the value range has been
     * specified the value has to be within the specified range.
     *
     * \param err specifies a pointer to a \c QString, which contains an
     * error description in case the value is invalid. This
     * parameter is optional.
     *
     * \return \e true in case the default value was successfully set.
     */
    bool setDefaultValue(const QVariant& arg, QString* err = 0);

    /*!
     * Indicates if the state variable's value is constrained either by minimum,
     * maximum or by a list of allowed values.
     *
     * \return true in case the state variable's value is constrained either by minimum,
     * maximum or by a list of allowed values.
     *
     * \sa minimumValue(), maximumValue(), allowedValueList()
     */
    bool isConstrained() const;

    /*!
     * Indicates whether or not the value is valid in terms of this particular
     * state variable.
     *
     * \param value specifies the value to be checked.
     *
     * \param convertedValue specifies a pointer to a \c QVariant that contains
     * the value as a variant of the correct type. This is optional.
     * Further, it will not be set if the value is invalid.
     *
     * \param err specifies a pointer to a \c QString, which contains an
     * error description in case the value is invalid. This
     * parameter is optional.
     *
     * \retval \e true in case the specified value is valid in terms of the
     * state variable this info object depicts. In other words, setValue() will
     * succeed with the value.
     *
     * \retval false otherwise.
     */
    bool isValidValue(
        const QVariant& value, QVariant* convertedValue = 0, QString* err = 0) const;

    /*!
     * Indicates if the object is valid.
     *
     * \return \e true in case the object is valid.
     */
    bool isValid() const;
};

/*!
 * Compares the two objects for equality.
 *
 * \return \e true in case the object are logically equivalent.
 *
 * \relates HStateVariableInfo
 */
H_UPNP_CORE_EXPORT bool operator==(
    const HStateVariableInfo&, const HStateVariableInfo&);

/*!
 * Compares the two objects for inequality.
 *
 * \return \e true in case the object are not logically equivalent.
 *
 * \relates HStateVariableInfo
 */
H_UPNP_CORE_EXPORT bool operator!=(
    const HStateVariableInfo&, const HStateVariableInfo&);

/*!
 * Returns a value that can be used as a unique key in a hash-map identifying
 * the object.
 *
 * \param key specifies the HStateVariableInfo object from which the hash value
 * is created.
 *
 * \return a value that can be used as a unique key in a hash-map identifying
 * the object.
 *
 * \relates HStateVariableInfo
 */
H_UPNP_CORE_EXPORT quint32 qHash(const HStateVariableInfo& key);

}
}

#endif /* HSTATEVARIABLEINFO_H_ */
