/*
 *  Copyright (C) 2010 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of an application named HUpnpSimpleTestApp
 *  used for demonstrating how to use the Herqq UPnP (HUPnP) library.
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

#include "dataitem_display.h"

#include <HDevice>
#include <HAction>
#include <HService>
#include <HServiceId>
#include <HDeviceInfo>
#include <HResourceType>
#include <HStateVariable>
#include <HActionArguments>

#include <QUrl>
#include <QStringList>

using namespace Herqq::Upnp;

DataItemDisplay::NavItemVisitor::NavItemVisitor(DataItemDisplay* owner) :
    m_owner(owner)
{
    Q_ASSERT(m_owner);
}

DataItemDisplay::NavItemVisitor::~NavItemVisitor()
{
}

void DataItemDisplay::NavItemVisitor::visit(ActionItem* item)
{
    Q_ASSERT(item);
    m_owner->m_modelData.clear();

    HAction* action = item->action();

    m_owner->m_rootDeviceUdn = action->parentService()->parentDevice()->
        rootDevice()->deviceInfo().udn();

    m_owner->m_modelData.clear();
    m_owner->reset();

    m_owner->m_modelData.push_back(qMakePair(
        QString("Name"), action->name()));

    m_owner->m_modelData.push_back(qMakePair(
        QString("Number of input arguments"),
        QString::number(action->inputArguments().size())));

    m_owner->m_modelData.push_back(qMakePair(
        QString("Number of output arguments"),
        QString::number(action->outputArguments().size())));

    m_owner->m_modelData.push_back(qMakePair(
        QString("Return argument name"),
        action->returnArgumentName()));

    m_owner->reset();
}

void DataItemDisplay::NavItemVisitor::visit(ServiceItem* item)
{
    Q_ASSERT(item);
    m_owner->m_modelData.clear();

    HService* service = item->service();

    m_owner->m_rootDeviceUdn = service->parentDevice()->
        rootDevice()->deviceInfo().udn();

    m_owner->m_modelData.push_back(qMakePair(
        QString("Service ID"), service->serviceId().toString()));

    m_owner->m_modelData.push_back(qMakePair(
        QString("Service type"), service->serviceType().toString()));

    m_owner->m_modelData.push_back(qMakePair(
        QString("SCPD URL"), service->scpdUrl().toString()));

    m_owner->m_modelData.push_back(qMakePair(
        QString("Event Sub URL"), service->eventSubUrl().toString()));

    m_owner->m_modelData.push_back(qMakePair(
        QString("Control URL"), service->controlUrl().toString()));

    m_owner->reset();
}

void DataItemDisplay::NavItemVisitor::visit(DeviceItem* item)
{
    Q_ASSERT(item);

    m_owner->m_modelData.clear();

    HDevice* device = item->device();
    HDeviceInfo deviceInfo = device->deviceInfo();

    m_owner->m_rootDeviceUdn = device->rootDevice()->deviceInfo().udn();

    m_owner->m_modelData.push_back(
        qMakePair(QString("Friendly name"), deviceInfo.friendlyName()));

    m_owner->m_modelData.push_back(
        qMakePair(QString("Device type"), deviceInfo.deviceType().toString()));

    m_owner->m_modelData.push_back(
        qMakePair(QString("Model name"), deviceInfo.modelName()));

    m_owner->m_modelData.push_back(
        qMakePair(QString("Manufacturer"), deviceInfo.manufacturer()));

    m_owner->m_modelData.push_back(
        qMakePair(QString("UDN"), deviceInfo.udn().toString()));

    QList<QUrl> locations = device->locations(true);
    for (qint32 i = 0; i < locations.size(); ++i)
    {
        m_owner->m_modelData.push_back(
            qMakePair(
                QString("Device description URL"),
                locations.at(i).toString()));
    }

    m_owner->reset();
}

void DataItemDisplay::NavItemVisitor::visit(StateVariableItem* item)
{
    Q_ASSERT(item);

    m_owner->m_modelData.clear();

    HStateVariable* stateVar = item->stateVariable();

    m_owner->m_rootDeviceUdn = stateVar->parentService()->parentDevice()->
        rootDevice()->deviceInfo().udn();

    m_owner->m_modelData.push_back(
        qMakePair(QString("Name"), stateVar->name()));

    m_owner->m_modelData.push_back(
        qMakePair(QString("Minimum value"), stateVar->minimumValue().toString()));

    m_owner->m_modelData.push_back(
        qMakePair(QString("Maximum value"), stateVar->maximumValue().toString()));

    m_owner->m_modelData.push_back(
        qMakePair(QString("Step value"), stateVar->stepValue().toString()));

    m_owner->m_modelData.push_back(
        qMakePair(QString("Default value"), stateVar->defaultValue().toString()));

    QString boolStr =
        stateVar->eventingType() != HStateVariable::NoEvents ? "Yes" : "No";

    m_owner->m_modelData.push_back(
        qMakePair(QString("Is evented"), boolStr));

    m_owner->m_modelData.push_back(
        qMakePair(QString("Allowed values"), stateVar->allowedValueList().join(";")));

    m_owner->reset();
}

DataItemDisplay::DataItemDisplay(QObject* parent) :
    QAbstractTableModel(parent)
{
}

DataItemDisplay::~DataItemDisplay()
{
}

void DataItemDisplay::setData(ControlPointNavigatorItem* navItem)
{
    NavItemVisitor visitor(this);
    navItem->accept(&visitor);
}

void DataItemDisplay::deviceRemoved(const Herqq::Upnp::HUdn& udn)
{
    if (udn == m_rootDeviceUdn)
    {
        m_modelData.clear();
        reset();
    }
}

Qt::ItemFlags DataItemDisplay::flags(const QModelIndex& /*index*/) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant DataItemDisplay::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant::Invalid;
    }

    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
        case 0:
            return m_modelData[index.row()].first;
        case 1:
            return m_modelData[index.row()].second;
        }
    }

    return QVariant::Invalid;
}

QVariant DataItemDisplay::headerData (
    int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            switch(section)
            {
            case 0:
                return QString("Name");
            case 1:
                return QString("Value");
            }
        }
    }

    return QVariant::Invalid;
}

int DataItemDisplay::rowCount(const QModelIndex& /*parent*/) const
{
    return m_modelData.size();
}

int DataItemDisplay::columnCount (const QModelIndex& /*parent*/) const
{
    return 2;
}
