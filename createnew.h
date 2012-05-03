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

#ifndef CREATENEW_H
#define CREATENEW_H

#include <QDialog>

namespace Ui {
    class createnew;
}

class createnew : public QDialog
{
    Q_OBJECT

public:
    explicit createnew(bool connection = false, QWidget *parent = 0);
    ~createnew();

private:
    Ui::createnew *ui;
    bool newconnect;

private slots:
    void on_buttonBox_accepted();
    void acceptToggle();
};

#endif // CREATENEW_H
