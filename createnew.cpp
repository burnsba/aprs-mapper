/* <APRS-Mapper . This program is used for recording and mapping APRS(Automatic Packet Reporting System) packets>
    Copyright (C) 2011  Ben Burns, Nikolas Boatright, Patrick Gilbert, Jason Schansman

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "createnew.h"
#include "ui_createnew.h"
#include "profile.h"
#include "profilemenu.h"
#include "connectionmenu.h"

createnew::createnew(bool connection, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::createnew)
{
    newconnect = connection;
    ui->setupUi(this);

    ui->label->adjustSize();
    connect(ui->newProfileEdit, SIGNAL(textChanged(QString)), this, SLOT(acceptToggle()));
}

createnew::~createnew()
{
    delete ui;
}

void createnew::acceptToggle()
{
    if(ui->newProfileEdit->text() != "")
        ui->buttonBox->setEnabled(true);
    else
        ui->buttonBox->setEnabled(false);
}

void createnew::on_buttonBox_accepted()
{
    if(newconnect)
    {
        Profile::newConnection(ui->newProfileEdit->text(), "default", "1111", "", false, false, false);
    }
    else
    {
        Profile::saveProfile(ui->newProfileEdit->text());
        ProfileMenu* p = new ProfileMenu();
        p->show();
    }
}
