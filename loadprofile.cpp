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

#include "loadprofile.h"
#include "profile.h"
#include "ui_loadprofile.h"
#include "errorhandler.h"

LoadProfile::LoadProfile(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoadProfile)
{
    ui->setupUi(this);
    QVector<QString> list = Profile::getProfileList();
    int size = list.size();
    for(int count = 0; count < size; count++)
    {
        ui->profileCombo->addItem(list.at(count));

        if(list.at(count) == Profile::getCurrentProfile())
        {
            ui->profileCombo->setCurrentIndex(count);
        }
    }
    this->setWindowTitle("Load Profile - Current Profile: " + Profile::getCurrentProfile());
    connect( ui->removeButton, SIGNAL(clicked()), this, SLOT(removeProfile()) );

}

LoadProfile::~LoadProfile()
{
    delete ui;
}

void LoadProfile::on_buttonBox_accepted()
{
    Profile::loadProfile(ui->profileCombo->currentText());
    emit newProfile(ui->profileCombo->currentText());
}

void LoadProfile::removeProfile()
{
    QString alias = ui->profileCombo->currentText();
    if(alias != Profile::getCurrentProfile())
    {
        Profile::removeProfile(alias);
        ui->profileCombo->removeItem(ui->profileCombo->currentIndex());
    }
    else
        ErrorHandler::AlertAndLog("Could not remove profile: " + alias + ", is this profile currently in use?");
}
