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

#ifndef CONNECTIONMENU_H
#define CONNECTIONMENU_H

#include <QDialog>

namespace Ui {
    class ConnectionMenu;
}

class ConnectionMenu : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionMenu(QString alias = "", QWidget *parent = 0);
    ~ConnectionMenu();

private:
    Ui::ConnectionMenu *ui;

private slots:
    void on_buttonBox_accepted();
    void preventSave(QString);
};

#endif // CONNECTIONMENU_H
