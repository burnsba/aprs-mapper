#include "about.h"
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

#include "ui_about.h"

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);
    QString license = getLicense();
    ui->aboutLabel->setText(license);
}

QString About::getLicense()
{
    QString license = "APRS-Mapper . This program is used for recording and mapping APRS(Automatic Packet Reporting System) packets\n";
            license += "Copyright (C) 2011  Ben Burns, Nikolas Boatright, Patrick Gilbert, Jason Schansman\n";
            license += "\nMap provided by OpenStreeMaps\n";
            license += "License provided by Creative Commons Attribution Share-Alike 3.0\n";
            license += "You can read this license at http://creativecommons.org/licenses/by-sa/3.0/";
            license += "\n\nCore APRS Parser by qaprstools\n";
            license += "qaprstools - Qt based APRS tools (based on Ham::APRS::Fap 1.17)\n";
            license += "Copyright (C) 2010  Holger Schurig, DH3HS, Germany, Nieder-Wöllstadt\n\n";
            license += "This program is free software: you can redistribute it and/or modify ";
            license += "it under the terms of the GNU General Public License as published by ";
            license += "the Free Software Foundation, either version 3 of the License, or ";
            license += "(at your option) any later version.\n";
            license += "This program is distributed in the hope that it will be useful, ";
            license += "but WITHOUT ANY WARRANTY; without even the implied warranty of ";
            license += "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the ";
            license += "GNU General Public License for more details.\n";
            license += "You should have received a copy of the GNU General Public License ";
            license += "along with this program.  If not, see http://www.gnu.org/licenses/.\n";

    return license;
}

About::~About()
{
    delete ui;
}

void About::on_okayButton_clicked()
{
    this->accept();
}
