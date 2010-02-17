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

#include "invokeactiondialog.h"
#include "ui_invokeactiondialog.h"

#include "genericinput.h"
#include "allowedvaluelist_input.h"

#include <HAction>
#include <HDevice>
#include <HService>
#include <HStateVariable>
#include <HActionInputArguments>
#include <HActionOutputArguments>

#include <QUuid>
#include <QMessageBox>
#include <QIntValidator>
#include <QDoubleValidator>

#include <limits>

using namespace Herqq::Upnp;

InvokeActionDialog::InvokeActionDialog(
    HAction* action, QWidget* parent) :
        QDialog(parent),
            m_ui(new Ui::InvokeActionDialog), m_action(action), m_inputWidgets()
{
    Q_ASSERT(action);

    m_ui->setupUi(this);
    setupArgumentWidgets();

    bool ok = connect(
        action, SIGNAL(invokeComplete(QUuid)),
        this, SLOT(invokeComplete(QUuid)));

    Q_ASSERT(ok);

    HDevice* device = action->parentService()->parentDevice();

    ok = connect(
        device, SIGNAL(disposed()), this, SLOT(contentSourceDisposed()));

    Q_ASSERT(ok);
}

void InvokeActionDialog::invokeComplete(const QUuid& invokeId)
{
    qint32 rc = 0;
    HActionOutputArguments outArgs;
    m_action->waitForInvoke(invokeId, &rc, &outArgs);

    if (rc == HAction::Success())
    {
        for(qint32 i = 0; i < outArgs.size(); ++i)
        {
            const HActionOutputArgument* outputArg = outArgs[i];
            m_ui->outputArguments->item(i, 2)->setText(
                outputArg->value().toString());
        }
    }
    else
    {
        QMessageBox msgBox;

        msgBox.setText(QString("Action invocation [id: %1] failed: %1").arg(
            invokeId.toString(), HAction::errorCodeToString(rc)));

        msgBox.exec();
    }

    m_ui->invokeButton->setEnabled(true);
}

void InvokeActionDialog::setupArgumentWidgets()
{
    HActionInputArguments inputArgs = m_action->inputArguments();

    m_ui->inputArguments->setRowCount(inputArgs.size());

    for(qint32 i = 0; i < inputArgs.size(); ++i)
    {
        HActionInputArgument* inputArg = inputArgs[i];
        HStateVariable* stateVar = inputArg->relatedStateVariable();

        QTableWidgetItem* item =
            new QTableWidgetItem(HUpnpDataTypes::toString(stateVar->dataType()));

        item->setFlags(Qt::NoItemFlags);

        m_ui->inputArguments->setItem(i, 0, item);

        item = new QTableWidgetItem(stateVar->name());
        item->setFlags(Qt::NoItemFlags);

        m_ui->inputArguments->setItem(i, 1, item);

        IDataHolder* dh = createDataHolder(stateVar);
        m_inputWidgets[inputArg->name()] = dh;

        m_ui->inputArguments->setCellWidget(i, 2, dh);
        //m_ui->inputArguments->resizeColumnsToContents();
    }

    HActionOutputArguments outputArgs = m_action->outputArguments();

    m_ui->outputArguments->setRowCount(outputArgs.size());

    for(qint32 i = 0; i < outputArgs.size(); ++i)
    {
        HActionOutputArgument* outputArg = outputArgs[i];
        HStateVariable* stateVar = outputArg->relatedStateVariable();

        QTableWidgetItem* item =
            new QTableWidgetItem(HUpnpDataTypes::toString(stateVar->dataType()));

        item->setFlags(Qt::NoItemFlags);

        m_ui->outputArguments->setItem(i, 0, item);

        item = new QTableWidgetItem(stateVar->name());
        item->setFlags(Qt::NoItemFlags);

        m_ui->outputArguments->setItem(i, 1, item);

        item = new QTableWidgetItem();
        item->setFlags(Qt::NoItemFlags);

        m_ui->outputArguments->setItem(i, 2, item);
    }
}

void InvokeActionDialog::contentSourceDisposed()
{
    done(0);
}

namespace
{
void minMaxValues(HUpnpDataTypes::DataType dt, qint32* max, qint32* min)
{
    switch(dt)
    {
        case HUpnpDataTypes::ui1:
            *max = std::numeric_limits<unsigned char>::max();
            *min = std::numeric_limits<unsigned char>::min();
            break;
        case HUpnpDataTypes::ui2:
            *max = std::numeric_limits<unsigned short>::max();
            *min = std::numeric_limits<unsigned short>::min();
            break;
        case HUpnpDataTypes::ui4:
            *max = std::numeric_limits<int>::max();
            *min = std::numeric_limits<int>::min();
            // for this example, the signed int range is acceptable.
            break;

        case HUpnpDataTypes::i1:
            *max = std::numeric_limits<char>::max();
            *min = std::numeric_limits<char>::min();
            break;
        case HUpnpDataTypes::i2:
            *max = std::numeric_limits<short>::max();
            *min = std::numeric_limits<short>::min();
            break;
        case HUpnpDataTypes::i4:
            *max = std::numeric_limits<int>::max();
            *min = std::numeric_limits<int>::min();
            break;

        default:
            Q_ASSERT(false);
    }
}

void minMaxValues(HUpnpDataTypes::DataType dt, qreal* max, qreal* min)
{
    switch(dt)
    {
        case HUpnpDataTypes::r4:
        case HUpnpDataTypes::fp:
            *max = std::numeric_limits<float>::max();
            *min = std::numeric_limits<float>::min();
            break;
        case HUpnpDataTypes::r8:
        case HUpnpDataTypes::number:
        case HUpnpDataTypes::fixed_14_4:
            *max = std::numeric_limits<double>::max();
            *min = std::numeric_limits<double>::min();
            break;
        default:
            Q_ASSERT(false);
    }
}
}

IDataHolder* InvokeActionDialog::createDataHolder(
    HStateVariable* stateVar)
{
    IDataHolder* content = 0;

    if (HUpnpDataTypes::isInteger(stateVar->dataType()))
    {
        if (stateVar->isConstrained())
        {
            content = new GenericInput(
                new QIntValidator(
                    stateVar->minimumValue().toInt(),
                    stateVar->maximumValue().toInt(),
                    0));
        }
        else
        {
            qint32 max = 0, min = 0;
            minMaxValues(stateVar->dataType(), &max, &min);
            content = new GenericInput(new QIntValidator(min, max, 0));
        }
    }
    else if (HUpnpDataTypes::isRational(HUpnpDataTypes::string))
    {
        if (stateVar->isConstrained())
        {
            content = new GenericInput(
                new QDoubleValidator(
                    stateVar->minimumValue().toDouble(),
                    stateVar->maximumValue().toDouble(),
                    0,
                    0));
        }
        else
        {
            qreal max = 0, min = 0;
            minMaxValues(stateVar->dataType(), &max, &min);
            content = new GenericInput(new QDoubleValidator(min, max, 0, 0));
        }
    }
    else if (stateVar->dataType() == HUpnpDataTypes::string)
    {
        if (stateVar->isConstrained())
        {
            content = new AllowedValueListInput(stateVar->allowedValueList());
        }
        else
        {
            content = new GenericInput();
        }
    }
    else if (stateVar->dataType() == HUpnpDataTypes::boolean)
    {
        QStringList allowedValues;
        allowedValues.append("True");
        allowedValues.append("False");

        content = new AllowedValueListInput(allowedValues);
    }

    return content;
}

InvokeActionDialog::~InvokeActionDialog()
{
    delete m_ui;
}

void InvokeActionDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void InvokeActionDialog::on_invokeButton_clicked()
{
    HActionInputArguments inputArgs = m_action->inputArguments();

    for(qint32 i = 0; i < inputArgs.size(); ++i)
    {
        HActionInputArgument* inputArg = inputArgs[i];

        IDataHolder* dataHolder = m_inputWidgets[inputArg->name()];

        QVariant data = dataHolder->data();
        if (inputArg->isValidValue(data))
        {
            bool ok = inputArg->setValue(data);
            Q_ASSERT(ok); Q_UNUSED(ok)
        }
        else
        {
            QMessageBox mbox;
            mbox.setText(QObject::tr("Check your arguments!"));
            mbox.setWindowTitle(QObject::tr("Error"));

            mbox.exec();
            return;
        }
    }

    m_action->beginInvoke(inputArgs);
    m_ui->invokeButton->setEnabled(false);
}