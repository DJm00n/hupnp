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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>

namespace Ui {
    class MainWindow;
}

//
// Main window for the test application.
//
class MainWindow :
    public QMainWindow
{
Q_OBJECT
Q_DISABLE_COPY(MainWindow)

public:

    explicit MainWindow(QWidget* parent = 0);
    virtual ~MainWindow();

protected:

    virtual void changeEvent(QEvent*);

private:

    Ui::MainWindow* m_ui;

private slots:

    void on_startControlPointButton_clicked();
    void on_hostDeviceButton_clicked();
    void deviceWindowClosed();
};

#endif // MAINWINDOW_H
