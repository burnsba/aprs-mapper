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

#include "connectionmenu.h"
#include "ui_connectionmenu.h"
#include "profile.h"
#include "profilemenu.h"
#include <QPushButton>

ConnectionMenu::ConnectionMenu(QString alias, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionMenu)
{
    ui->setupUi(this);

    //beepvalidator* portValidator = new beepvalidator(1, 65536, this);
    //ui->portEdit->setValidator(portValidator);
    //ui->portEdit->setValidator(new BeepIntValidator(1, 65536, this));
    ui->portEdit->setValidator(new QIntValidator(1, 65536, this));

    ui->addressLabel->adjustSize();
    ui->aliasLabel->adjustSize();
    ui->portLabel->adjustSize();
    ui->connectStartupCheck->adjustSize();
    ui->connectStrLabel->adjustSize();
    ui->sendReportCheck->adjustSize();
    ui->authConnCheck->adjustSize();

    setWindowTitle(alias);

    ui->aliasEdit->setText(alias);
    ui->addressEdit->setText(Profile::getServerAddress(alias));
    ui->portEdit->setText(QString::number(Profile::getServerPort(alias)));
    ui->connectStrEdit->setText(Profile::getConnectStr(alias));
    ui->connectStartupCheck->setChecked(Profile::getConnectStart(alias));
    ui->sendReportCheck->setChecked(Profile::getSendReport(alias));
    ui->authConnCheck->setChecked(Profile::getAuthConn(alias));

    connect(ui->addressEdit, SIGNAL(textChanged(QString)), this, SLOT(preventSave(QString)));
    connect(ui->portEdit, SIGNAL(textChanged(QString)), this, SLOT(preventSave(QString)));
    connect(ui->aliasEdit, SIGNAL(textChanged(QString)), this, SLOT(preventSave(QString)));
}

ConnectionMenu::~ConnectionMenu()
{
    delete ui;
}

void ConnectionMenu::on_buttonBox_accepted()
{
    Profile::saveConnection(windowTitle(), ui->aliasEdit->text(), ui->addressEdit->text(), ui->portEdit->text(),
                            ui->connectStrEdit->text(), ui->connectStartupCheck->isChecked(), ui->sendReportCheck->isChecked(),
                            ui->authConnCheck->isChecked());
}

void ConnectionMenu::preventSave(QString changedText)
{
    QPushButton* okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    if(changedText == "")
    {
        okButton->setEnabled(false);
    }
    else
    {
        okButton->setEnabled(true);
    }
}
