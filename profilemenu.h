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

#ifndef PROFILEMENU_H
#define PROFILEMENU_H

#include <QDialog>
#include "profile.h"
#include "connectionmenu.h"
#include <QAbstractButton>
#include <QDoubleValidator>

namespace Ui {
    class ProfileMenu;
}

class ProfileMenu : public QDialog
{
    Q_OBJECT

public:
    explicit ProfileMenu(int currTab = 0, QWidget *parent = 0);
    ~ProfileMenu();
    void refreshConnections();

private:
    Ui::ProfileMenu *ui;

    void redrawIcon(int id);
    void updateOverlayBox(int id);

    qint32 temp_icon_id; // temp variable to track icon changes (in case user hits cancel)

private slots:
    void on_pushButtonCache_clicked();
    void on_dbFilepathButton_clicked();
    void on_buttonBox_accepted();
    void editClicked();
    void removeClicked();
    void addClicked();
    void databaseToggle(bool);
    void unitsToggle(bool);
    void on_icon_click(QGraphicsSceneMouseEvent *e);
    void on_select_icon(QGraphicsSceneMouseEvent *e);
    void purgeTimeChanged(QString);
    void iconImageChanged();
    void iconCharsChanged(QString);
    void overlayChanged(QString);

signals:
    void ConnectionEdited(QString alias);
    void ConnectionRemoved(QString alias);
    void ConnectionAdded();
    void DatabaseChanged(QString type);
    void refreshHome();

};

#endif // PROFILEMENU_H
