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

#ifndef APRSMAIN_H
#define APRSMAIN_H

#include <QMainWindow>
#include <QtGui>

#include "map.h"
#include "mapgraphicsscene.h"
#include "profile.h"
#include "point.h"
#include "parserthread.h"
#include "coordinate.h"
#include "overlay.h"
#include <QtSql>
#include <database.h>
#include <QModelIndex>




namespace Ui {
    class APRSmain;
}


class APRSmain : public QMainWindow
{
    Q_OBJECT

public:
    explicit APRSmain(QWidget *parent = 0);
    ~APRSmain();
    Database* db;
    ParserThread* parserThread;

public slots:
    void setStatusBar2Text(QString text);
    void setStatusBar3Text(QString text);
    void replotHome();

signals:

    void applyFilterWhiteType(QVector<aprstype>);
    void applyFilterBlackType(QVector<aprstype>);
    void applyFilterWhiteSymbol(QVector<int>);
    void applyFilterBlackSymbol(QVector<int>);

private slots:
    void on_buttonWhitelistNone_pressed();
    void on_buttonWhitelistAll_pressed();
    void on_buttonBlacklistNone_pressed();
    void on_buttonBlacklistAll_pressed();
    void on_buttonApplyFilters_pressed();
    void on_buttonClearAllFilters_pressed();
    void on_actionE_xit_triggered();
    //void on_actionRemove_Traces_triggered();
    void on_actionShow_connections_triggered();
    void on_actionShow_status_bar_triggered();
    void on_actionShow_weather_list_triggered();
    void on_radarTree_itemPressed(QTreeWidgetItem* item, int column);
    void on_radarTree_itemActivated(QTreeWidgetItem* item, int column);
    void on_actionView_Invalid_Packets_triggered();
    void on_actionDisconnect_All_triggered();
    void on_connectionList_doubleClicked(QModelIndex index);
    void on_actionCallsigns_list_triggered();
    void on_actionSend_Position_Report_triggered();
    void on_actionPurge_All_Packets_triggered();
    void on_actionEmpty_Packets_Table_triggered();
    void on_actionRemove_All_triggered();
    void on_mapZoomSlider_valueChanged(int value);
    void on_actionConnection_Connect_triggered();
    //void on_actionTerminate_Connection_triggered();
    void on_actionView_Packets_Table_triggered();
    void aboutMenu_clicked();
    void clearPointList();
    void removePointFromList(QString);

    void on_mouse_move(QGraphicsSceneMouseEvent *event);
    void on_map_double_click(QGraphicsSceneMouseEvent* event);
    void on_map_key_press(QKeyEvent* event);
    void on_mouse_scroll(QGraphicsSceneWheelEvent *event);

    void openProfile();
    void openConnections();
    void openDatabase();
    void openCreateNew();
    void openLoadProfile();

    void pointAdded(Point* point);
    void callCenter();
    void callCenter(QModelIndex index);

    void processWorldFile(QNetworkReply *reply);
    void processOverlay(QNetworkReply *reply);

    void updateConnections(QString alias);
    void toggleConnection();

    void EditConnection(QString);
    void AddConnection();
    void RemoveConnection(QString);

    void ChangeProfile(QString);
    void clearStatus2Event();

private:
    Ui::APRSmain *ui;
    Map* mMap;
    MapGraphicsScene* mScene;
    //Profile* currProfile;
    //Database* db;
    qint32 lastSliderPosition;
    void closeEvent(QCloseEvent *);
    ParserThread* parse;
    void resizeEvent(QResizeEvent *);
    //void changeEvent(QEvent *);
    void ZoomMap(int zoomLevel);
    void addWeatherOverlayItems();

    void processRadarTreeEvent(QTreeWidgetItem* item, int column);

    void setupFilters();
    void sendNewFilters();

    QTimer clearAndPurgeTimer;
    QTimer clearStatus2;
    QPointF* temp_p;

    QNetworkAccessManager m_net_world_file; // to retrieve world files
    QNetworkAccessManager m_net_overlay; // to grab the .gif once the world file is downloaded

    QIcon discIcon;
    QIcon connIcon;

    QLabel statusBar2Label;
    QLabel statusBar3Label;
};

#endif // APRSMAIN_H
