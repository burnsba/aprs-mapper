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


#include "aprsmain.h"
#include "connectionmenu.h"
#include "ui_aprsmain.h"
#include "point.h"
#include "map.h"
#include "parserthread.h"
#include "profile.h"
#include "profilemenu.h"
#include "createnew.h"
#include "loadprofile.h"
#include "overlay.h"
#include "about.h"
#include <QDebug>
#include <QMessageBox>
#include <QTableView>
#include <QTimer>
#include <QListWidgetItem>
#include <QStandardItemModel>

#define MAX_ZOOM_LEVEL 15
#define MIN_ZOOM_LEVEL 3

APRSmain::APRSmain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::APRSmain)
{
    ui->setupUi(this);

    //ui->lblCallsignFind->hide();
    //ui->leCallsignFind->hide();
    ui->dateTimeEdit->hide();
    ui->dateTimeEdit_2->hide();
    ui->horizontalSlider->hide();

    //ui->listWidget->hide();

    QSettings settings("Team 05", "APRS Mapper");
    restoreGeometry(settings.value("geometry").toByteArray());

    ui->actionCallsigns_list->setChecked(settings.value("callsign_menu_checked", true).toBool());
    on_actionCallsigns_list_triggered();
    ui->actionShow_connections->setChecked(settings.value("connections_menu_checked", true).toBool());
    on_actionShow_connections_triggered();
    ui->actionShow_weather_list->setChecked(settings.value("overlays_menu_checked", true).toBool());
    on_actionShow_weather_list_triggered();
    ui->actionShow_status_bar->setChecked(settings.value("status_bar_menu_checked", true).toBool());
    on_actionShow_status_bar_triggered();

    qDebug() << settings.value("lastprofile").toString();

    //Profile::loadProfile(settings.value("lastprofile").toString()); // <- loads the default profile settings
    Profile::loadProfile(settings.value("lastprofile", QString("default")).toString());

    this->setWindowTitle("Team 05soft - APRS Mapper v2 :: " + Profile::getCurrentProfile());

    lastSliderPosition = ui->mapZoomSlider->value();

    ErrorHandler::Start();
    ErrorHandler::Log("Starting application.\n");

    // Database connection
    db = new Database(); //Database thread
    db->Start();

    // need to set up the scene before the view can be accessed
    mScene = new MapGraphicsScene(ui->mainMap);
    ui->mainMap->setScene(mScene);
    mMap = new Map(ui->mainMap, db);

    connect(ui->actionProfile, SIGNAL(triggered()), this, SLOT(openProfile()));
    connect(ui->actionConnections, SIGNAL(triggered()), this, SLOT(openConnections()));
    connect(ui->actionDatabase, SIGNAL(triggered()), this, SLOT(openDatabase()));
    connect(ui->actionCreate_New_Profile, SIGNAL(triggered()), this, SLOT(openCreateNew()));
    connect(ui->actionLoad_Profile, SIGNAL(triggered()), this, SLOT(openLoadProfile()));


    // status bar item for distance/bearing to current point
    ui->statusBar->addPermanentWidget(&statusBar3Label);
    // status bar item for showing last received packet
    ui->statusBar->addPermanentWidget(&statusBar2Label);
    statusBar2Label.setMinimumWidth(100);
    statusBar2Label.setMaximumWidth(100);
    // since applying statusBar2Label.setFrameShape(QFrame::NoFrame) and
    // statusBar2Label.setFrameShadow(QFrame::Plain)
    // don't actually remove the frames on the added widgets, the
    // workaround is to apply a style sheet to the application
    qApp->setStyleSheet("QStatusBar::item { border: 0px solid black }; ");


    // double click for zoom
    //connect(mScene, SIGNAL(mgsDoubleClickEvent(QGraphicsSceneMouseEvent*)), mMap, SLOT(ZoomIn()));
    connect(mScene, SIGNAL(mgsDoubleClickEvent(QGraphicsSceneMouseEvent*)), this, SLOT(on_map_double_click(QGraphicsSceneMouseEvent*)));
    // moving the mouse will update current coords
    connect(mScene, SIGNAL(mgsMouseEvent(QGraphicsSceneMouseEvent*)), this, SLOT(on_mouse_move(QGraphicsSceneMouseEvent*)));
    // should be some keystrokes to move the map around
    connect(mScene, SIGNAL(mgsKeyPressEvent(QKeyEvent*)), mMap, SLOT(keyPressEvent(QKeyEvent*))); // pan
    connect(mScene, SIGNAL(mgsKeyPressEvent(QKeyEvent*)), this, SLOT(on_map_key_press(QKeyEvent*))); // capture zoom to adjust slider
    // it also needs to tell the map to drag the tiles, etc
    connect(mScene, SIGNAL(mgsMouseEvent(QGraphicsSceneMouseEvent*)), mMap, SLOT(sceneMouseMoveEvent(QGraphicsSceneMouseEvent*)));
    connect(mScene, SIGNAL(mgsPressEvent(QGraphicsSceneMouseEvent*)), mMap, SLOT(sceneMousePressEvent(QGraphicsSceneMouseEvent*)));
    connect(mScene, SIGNAL(mgsReleaseEvent(QGraphicsSceneMouseEvent*)), mMap, SLOT(sceneMouseReleaseEvent(QGraphicsSceneMouseEvent*)));
    //connect(mScene, SIGNAL(mgsSceneRectChanged(QRectF)), mMap, SLOT(sceneRectChanged(QRectF)));
    connect(mScene, SIGNAL(mgsMouseWheelEvent(QGraphicsSceneWheelEvent*)), this, SLOT(on_mouse_scroll(QGraphicsSceneWheelEvent*)));
    // show the bearing/distance to the current mouse over item
    connect(mMap, SIGNAL(showMouseOverStats(QString)), this, SLOT(setStatusBar3Text(QString)));

    //connections for the about box
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(aboutMenu_clicked()));



    // when the map pans and things, it needs to add points that are now going to be in the view
    connect(mMap, SIGNAL(RequestPointsInRange(QRectF)), db, SLOT(GetPacketsArea(QRectF)), Qt::QueuedConnection);
    connect(mMap, SIGNAL(PointRemoved(QString)), this, SLOT(removePointFromList(QString)));
    connect(db, SIGNAL(PlotThis(Point*)), mMap, SLOT(Add(Point*)), Qt::QueuedConnection);
    connect(mMap, SIGNAL(PointPlotted(Point*)), this, SLOT(pointAdded(Point*)));
    // if a zoom happens while the database is dumping packets to the map, that needs to be interrupted
    connect(mMap, SIGNAL(tellDatabaseToStopSendingPoints()), db, SLOT(StopGettingPacketsArea()));

    // UI elements for locating stations
    connect(ui->leCallsignFind, SIGNAL(returnPressed()), this, SLOT(callCenter()));
    connect(ui->listWidget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(callCenter(QModelIndex)));

    // the timer will fire an event; the slots will check and remove
    // items based on the times in the profile settings
    clearAndPurgeTimer.setInterval(30000); // fire event every 30 seconds
    connect(&clearAndPurgeTimer, SIGNAL(timeout()), mMap, SLOT(ClearOldItems()));
    connect(&clearAndPurgeTimer, SIGNAL(timeout()), db, SLOT(Purge()));
    clearAndPurgeTimer.start();

    // timer for getting rid of a callsign after it hasn't been updated in a while
    connect(&clearStatus2, SIGNAL(timeout()), this, SLOT(clearStatus2Event()));

    // make sure the slider thinger is at the correct location saved in the profile
    ui->mapZoomSlider->setValue(mMap->getZoom());


    // setup the weather overlay things
    addWeatherOverlayItems();
    connect(&m_net_world_file, SIGNAL(finished(QNetworkReply*)), this, SLOT(processWorldFile(QNetworkReply*)), Qt::QueuedConnection);
    connect(&m_net_overlay, SIGNAL(finished(QNetworkReply*)), this, SLOT(processOverlay(QNetworkReply*)), Qt::QueuedConnection);


    discIcon.addFile("resource/red-light.png");
    connIcon.addFile("resource/green-light.png");

    parserThread = new ParserThread();
    foreach(Connection* conn, parserThread->connections)
    {
        connect(conn, SIGNAL(AddItem(Point*)), mMap, SLOT(Add(Point*)), Qt::QueuedConnection);
        //connect(conn, SIGNAL(AddItem(Point*)), this, SLOT(pointAdded(Point*)), Qt::QueuedConnection);
        connect(conn, SIGNAL(addedPoint(QString)), this, SLOT(setStatusBar2Text(QString)));
        connect(conn, SIGNAL(QueueFull(QQueue<Packet>*)), db, SLOT(DatabaseCommit(QQueue<Packet>*)));
        connect(conn, SIGNAL(InvalidPacket(Packet)), db, SLOT(CommitInvalid(Packet)));
        connect(conn, SIGNAL(ConnectionChanged(QString)), this, SLOT(updateConnections(QString)));

        QAction* action = new QAction(ui->menuConnection);
        action->setText("Toggle " + conn->getAlias());
        action->setData(conn->getAlias());

        if(Profile::getConnectStart(conn->getAlias()))
        {
            conn->connectToIS();
            new QListWidgetItem(connIcon, conn->getAlias(), ui->connectionList);
            action->setIcon(connIcon);
        }
        else
        {
            new QListWidgetItem(discIcon, conn->getAlias(), ui->connectionList);
            action->setIcon(discIcon);
        }

        connect(action, SIGNAL(triggered()), this, SLOT(toggleConnection()));
        ui->menuConnection->addAction(action);
    }
    connect(db, SIGNAL(DatabaseChanging()), parserThread, SLOT(DisconnectAll()));

    srand(QDateTime::currentDateTime().toTime_t());
    // add items for black/white listing
    setupFilters();
    // connect the filters to the map
    connect(this, SIGNAL(applyFilterBlackSymbol(QVector<int>)), mMap, SLOT(setBlackListSymbol(QVector<int>)));
    connect(this, SIGNAL(applyFilterWhiteSymbol(QVector<int>)), mMap, SLOT(setWhiteListSymbol(QVector<int>)));
    connect(this, SIGNAL(applyFilterBlackType(QVector<aprstype>)), mMap, SLOT(setBlackListType(QVector<aprstype>)));
    connect(this, SIGNAL(applyFilterWhiteType(QVector<aprstype>)), mMap, SLOT(setWhiteListType(QVector<aprstype>)));
    // setup the filters (whitelist) for the first time
    sendNewFilters();


    // add the home point
    replotHome();

}

APRSmain::~APRSmain()
{
    ErrorHandler::End();
    db->Stop();
    delete ui;
}

void APRSmain::openProfile()
{
    ProfileMenu* p = new ProfileMenu(0);
    connect(p, SIGNAL(ConnectionEdited(QString)), this, SLOT(EditConnection(QString)));
    connect(p, SIGNAL(ConnectionEdited(QString)), parserThread, SLOT(EditConnection(QString)));
    connect(p, SIGNAL(ConnectionRemoved(QString)), parserThread, SLOT(RemoveConnection(QString)));
    connect(p, SIGNAL(ConnectionRemoved(QString)), this, SLOT(RemoveConnection(QString)));
    connect(p, SIGNAL(ConnectionAdded()), this, SLOT(AddConnection()));
    connect(p, SIGNAL(DatabaseChanged(QString)), db, SLOT(ChangeDatabase(QString)));
    // if the profile changes the location of the home station, redraw that
    connect(p, SIGNAL(refreshHome()), this, SLOT(replotHome()));
    p->show();
}
void APRSmain::openConnections()
{
    ProfileMenu* p = new ProfileMenu(1);
    connect(p, SIGNAL(ConnectionEdited(QString)), this, SLOT(EditConnection(QString)));
    connect(p, SIGNAL(ConnectionEdited(QString)), parserThread, SLOT(EditConnection(QString)));
    connect(p, SIGNAL(ConnectionRemoved(QString)), parserThread, SLOT(RemoveConnection(QString)));
    connect(p, SIGNAL(ConnectionRemoved(QString)), this, SLOT(RemoveConnection(QString)));
    connect(p, SIGNAL(ConnectionAdded()), this, SLOT(AddConnection()));
    connect(p, SIGNAL(DatabaseChanged(QString)), db, SLOT(ChangeDatabase(QString)));
    p->show();
}
void APRSmain::openDatabase()
{
    ProfileMenu* p = new ProfileMenu(3);
    connect(p, SIGNAL(ConnectionEdited(QString)), this, SLOT(EditConnection(QString)));
    connect(p, SIGNAL(ConnectionEdited(QString)), parserThread, SLOT(EditConnection(QString)));
    connect(p, SIGNAL(ConnectionRemoved(QString)), parserThread, SLOT(RemoveConnection(QString)));
    connect(p, SIGNAL(ConnectionRemoved(QString)), this, SLOT(RemoveConnection(QString)));
    connect(p, SIGNAL(ConnectionAdded()), this, SLOT(AddConnection()));
    connect(p, SIGNAL(DatabaseChanged(QString)), db, SLOT(ChangeDatabase(QString)));
    p->show();
}

void APRSmain::openCreateNew()
{
    createnew* n = new createnew();
    n->show();
}

void APRSmain::openLoadProfile()
{
    LoadProfile* l = new LoadProfile();
    connect(l, SIGNAL(newProfile(QString)), this, SLOT(ChangeProfile(QString)));
    qDebug() << "Loading profile";
    l->show();
}

void APRSmain::EditConnection(QString alias)
{
    QString newServerName = "";
    if(!Profile::getConnectionList().contains(alias))
    {
        foreach(QAction* action, ui->menuConnection->actions())
        {
            if(action->data() == alias)
            {
                foreach(QString server, Profile::getConnectionList())
                {
                    if(parserThread->getConnection(server) == 0)
                    {
                        newServerName = server;
                        action->setText("Toggle " + server);
                        action->setData(server);
                    }
                }
            }
        }
        QList<QListWidgetItem*> item = ui->connectionList->findItems(alias, Qt::MatchExactly);
        if(!item.empty() && newServerName != "")
        {
            item.at(0)->setText(newServerName);
        }
    }
}

void APRSmain::AddConnection()
{
    foreach(QString server, Profile::getConnectionList())
    {
        if(parserThread->getConnection(server) == 0)
        {
            parserThread->AddConnection(server);
            Connection* conn = parserThread->getConnection(server);

            connect(conn, SIGNAL(AddItem(Point*)), mMap, SLOT(Add(Point*)), Qt::QueuedConnection);
            connect(conn, SIGNAL(AddItem(Point*)), this, SLOT(pointAdded(Point*)));
            connect(conn, SIGNAL(addedPoint(QString)), this, SLOT(setStatusBar2Text(QString)));
            connect(conn, SIGNAL(QueueFull(QQueue<Packet>*)), db, SLOT(DatabaseCommit(QQueue<Packet>*)));
            connect(conn, SIGNAL(InvalidPacket(Packet)), db, SLOT(CommitInvalid(Packet)));
            connect(conn, SIGNAL(ConnectionChanged(QString)), this, SLOT(updateConnections(QString)));

            new QListWidgetItem(discIcon, conn->getAlias(), ui->connectionList);
            QAction* action = new QAction(ui->menuConnection);
            action->setText("Toggle " + conn->getAlias());
            action->setData(conn->getAlias());
            action->setIcon(discIcon);
            connect(action, SIGNAL(triggered()), this, SLOT(toggleConnection()));
            ui->menuConnection->addAction(action);
        }
    }
}

void APRSmain::RemoveConnection(QString alias)
{
    // connection list
    QList<QListWidgetItem*> item = ui->connectionList->findItems(alias, Qt::MatchExactly);
    delete item.at(0);

    // Menu items
    foreach(QAction* action, ui->menuConnection->actions())
    {
        if(action->data() == alias)
        {
            ui->menuConnection->removeAction(action);
            delete action;
        }
    }
}


void APRSmain::on_actionView_Packets_Table_triggered()
{
    QSqlQueryModel *packetModel = new QSqlQueryModel;
    packetModel->setQuery("SELECT * FROM packets");

    QTableView *packetsView = new QTableView();
    packetsView->setModel(packetModel);
    packetsView->setWindowTitle("Captured Packets");
    packetsView->show();
}

void APRSmain::on_actionConnection_Connect_triggered()
{
    //if status is connected
    //loop through connections and stop them
    //else
    //loop through connections and start them

    parserThread->ConnectAll();
}

void APRSmain::aboutMenu_clicked()
{
    About* about = new About();
    about->show();
}

void APRSmain::callCenter()
{
    //mMap->CenterOn(ui->leCallsignFind->text());
    QPointF point = db->getPointLocation(ui->leCallsignFind->text());
    if(point.y() <= 90.0 && point.x() <= 180.0)
    {
        mMap->CenterOn(point);
        qDebug() << "centering";
    }
}

void APRSmain::callCenter(QModelIndex index)
{
    //mMap->CenterOn(ui->listWidget->item(index.row())->text());
    mMap->CenterOn(ui->listWidget->currentItem()->text());
}

void APRSmain::pointAdded(Point* point)
{
    if(ui->listWidget->findItems(point->getSource(), Qt::MatchFixedString).isEmpty())
    {
        ui->listWidget->addItem(point->getSource());
    }
    //ui->listWidget->sortItems();
}

void APRSmain::on_actionRemove_All_triggered()
{
    mMap->RemoveAllItems();
    ui->listWidget->clear();
}

void APRSmain::clearPointList()
{
    ui->listWidget->clear();
}

void APRSmain::on_actionEmpty_Packets_Table_triggered()
{
    db->Purge(QDateTime::currentDateTime());
}

void APRSmain::on_actionPurge_All_Packets_triggered()
{
    db->PurgeAll();
    qDebug() << "Purge All Packets";
}

void APRSmain::closeEvent(QCloseEvent *event)
{
    //emit tellDatabaseToStopSendingPoints();

    QSettings settings("Team 05", "APRS Mapper");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("lastprofile", Profile::getCurrentProfile());
    settings.setValue("callsign_menu_checked", ui->actionCallsigns_list->isChecked());
    settings.setValue("connections_menu_checked", ui->actionShow_connections->isChecked());
    settings.setValue("overlays_menu_checked", ui->actionShow_weather_list->isChecked());
    settings.setValue("status_bar_menu_checked", ui->actionShow_status_bar->isChecked());


    QPointF center = mMap->getViewCenterLatLon();
    Profile::setMapLat(center.y());
    Profile::setMapLong(center.x());
    Profile::setMapZoom(mMap->getZoom());

    Profile::saveProfile();

    event->accept();
}

void APRSmain::on_actionSend_Position_Report_triggered()
{
    parserThread->SendPositionReports();
}


void APRSmain::resizeEvent(QResizeEvent *e)
{
    //qDebug() << "APRSmain::resizeEvent";
    QRectF new_size = QRectF(0, 0, ui->mainMap->size().width(), ui->mainMap->size().height());
    mMap->sceneRectChanged(new_size);
    //qDebug() << "scene rect: " + QString::number(mMap->mView->scene()->sceneRect().x()) + ", " + QString::number(mMap->mView->scene()->sceneRect().y());

}


//void APRSmain::changeEvent(QEvent *e)
//{
//    if (e->type() == QEvent::WindowStateChange)
//    {
//        QRectF new_size = QRectF(0, 0, ui->mainMap->size().width(), ui->mainMap->size().height());
//        mMap->sceneRectChanged(new_size);
//    }
//}

void APRSmain::on_mouse_scroll(QGraphicsSceneWheelEvent *event)
{
    if (event->delta() > 0)
    {
        if (ui->mapZoomSlider->value() < MAX_ZOOM_LEVEL)
        {
            ui->mapZoomSlider->setValue(ui->mapZoomSlider->value() + 1);
        }
    }
    else if (event->delta() < 0)
    {
        if (ui->mapZoomSlider->value() > MIN_ZOOM_LEVEL)
        {
            ui->mapZoomSlider->setValue(ui->mapZoomSlider->value() - 1);
        }
    }
    event->accept();

}

void APRSmain::on_mouse_move(QGraphicsSceneMouseEvent *event)
{
    try
    {
        // create the point
        Coordinate c = Coordinate(mMap->getMapOrigin(), mMap->getZoom(), event->scenePos().x(), event->scenePos().y());
        this->statusBar()->showMessage(c.ToString());

    }
    catch(...)
    {
        ErrorHandler::Log("Could not get cursor coordinates.");
    }
}

void APRSmain::on_map_double_click(QGraphicsSceneMouseEvent* event)
{
    if (ui->mapZoomSlider->value() < MAX_ZOOM_LEVEL)
    {
        mMap->CenterOn(static_cast<int>(event->scenePos().x()), static_cast<int>(event->scenePos().y()));
        ui->mapZoomSlider->setValue(ui->mapZoomSlider->value() + 1);
    }
}

void APRSmain::on_mapZoomSlider_valueChanged(int value)
{
    //qDebug() << "changing to position: " + QString::number(value,10) + " last: " + QString::number(lastSliderPosition, 10);
    if (value != lastSliderPosition)
    {
        ZoomMap(value);
        lastSliderPosition = value;
    }
    else
    {
        // do nothing
    }
}

void APRSmain::on_map_key_press(QKeyEvent *event)
{
    // this traps zooming to adjust slider
    if (event->key() == Qt::Key_PageDown)
    {
        if (ui->mapZoomSlider->value() < MAX_ZOOM_LEVEL)
        {
            // triggers zoom event
            ui->mapZoomSlider->setValue(ui->mapZoomSlider->value() + 1);
        }
    }
    if (event->key() == Qt::Key_PageUp)
    {
        if (ui->mapZoomSlider->value() > MIN_ZOOM_LEVEL)
        {
            // triggers zoom event
            ui->mapZoomSlider->setValue(ui->mapZoomSlider->value() - 1);
        }
    }

}

void APRSmain::ZoomMap(int zoomLevel)
{
    mMap->ZoomTo(zoomLevel);
}


void APRSmain::removePointFromList(QString callsign)
{
    if(!ui->listWidget->findItems(callsign, Qt::MatchExactly).empty())
    {
        delete ui->listWidget->findItems(callsign, Qt::MatchExactly).at(0);
    }
}


// adds theh list of available radar stations to the ui
void APRSmain::addWeatherOverlayItems()
{
    RadarStation i = (RadarStation)0;
    QString last_state = "AL";
    QString next_state = RadarStationToState(i);
    int outer_count = 0;

    // do this for every station...
    while (i != NULL_STATION)
    {
        // create the parent state element
        QTreeWidgetItem *topper = new QTreeWidgetItem();
        topper->setText(0, RadarStationToState(i));
        while (next_state == last_state)
        {
            // each state will have some radar stations
            QTreeWidgetItem *tree_item = new QTreeWidgetItem(topper);
            tree_item->setExpanded(false);
            tree_item->setText(0, RadarStationLongName(i) + " (" + RadarStationShortName(i) +")");

            // each radar station has radar, warnings, etc
                QTreeWidgetItem *radar_item = new QTreeWidgetItem(tree_item);
                radar_item->setText(0, tr("radar"));
                radar_item->setCheckState(0, Qt::Unchecked);
                radar_item->setText(1, tr("warning"));
                radar_item->setCheckState(1, Qt::Unchecked);

            i = (RadarStation)((int)i + 1);
            next_state = RadarStationToState(i);
        }
        last_state = next_state;
        ui->radarTree->insertTopLevelItem(outer_count, topper);
        outer_count++;
    }

}

void APRSmain::on_radarTree_itemActivated(QTreeWidgetItem* item, int column)
{
    processRadarTreeEvent(item, column);
}

void APRSmain::on_radarTree_itemPressed(QTreeWidgetItem* item, int column)
{
    processRadarTreeEvent(item, column);
}

void APRSmain::processRadarTreeEvent(QTreeWidgetItem *item, int column)
{

    int go = 0;
    if (item)
        if (item->parent())
            if (item->parent()->parent())
                go = 1;
    if (go != 1)
        return;

    // need to figure out the station ID of this no matter what.

    int state_index = 0;
    // get the station index of the state
    int station_index = item->parent()->parent()->indexOfChild(item->parent());
    QTreeWidgetItem * state_item = item->parent()->parent();
    // if this is after the first state, need to count backwards
    // and get all the states + children to find the radar station index
    if (ui->radarTree->indexOfTopLevelItem(state_item) > 0)
    {
        do
        {
            state_item = ui->radarTree->itemAbove(state_item);
            state_index = ui->radarTree->indexOfTopLevelItem(state_item);
            station_index += state_item->childCount();
        } while (state_index != 0);
    }

    RadarStation r = (RadarStation)station_index;

    // item is currently unchecked, and processing the event to become checked
    // (so, add it)
    if (item->checkState(column) == Qt::Unchecked)
    {
        QUrl url = QUrl("");
        url = QUrl("http://radar.weather.gov/ridge/RadarImg/N0R/" + RadarStationShortName(r) + "_N0R_0.gfw");

        if (url.path() != "")
        {
            QNetworkRequest request;
            request.setUrl(url);
            request.setRawHeader("User-Agent", "Nokia (Qt) Graphics Dojo 1.0");
            request.setAttribute(QNetworkRequest::User, (int)r);    // pass the RadarStation id
            request.setAttribute(QNetworkRequest::UserMax, column); // pass the column (radar or warning)
            m_net_world_file.get(request);
        }
    }
    // else the item is currently checked, and this is processing the event
    // to become unchecked (so, remove it)
    else
    {
        Overlay* v = mMap->getOverlay(r, (column + 1)); // column starts at 0
        if (v != NULL)
        {
            mMap->RemoveOverlay(v);
        }
    }

}


void APRSmain::processWorldFile(QNetworkReply *reply)
{
    //QImage img;
    QUrl url = reply->url();
    RadarStation r = (RadarStation)reply->request().attribute(QNetworkRequest::User).toInt();
    int type = reply->request().attribute(QNetworkRequest::UserMax).toInt();
    char buf[128];
    QRectF world_file_infos;
    qreal x_scale;
    qreal y_scale;
    qreal lat;
    qreal lon;
    int bytes_read;
    if (!reply->error())
    {
        //ErrorHandler::msgBox(url.path());
        qDebug() << "downloaded " + url.path();
        qDebug() << "stationn is " + RadarStationShortName(r);
    }
    else
    {
        ErrorHandler::Log("Could not download " + url.path());
        reply->deleteLater();
        return;
    }

    // line 1, x scale
    bytes_read = reply->readLine(buf, 128);
    x_scale = 0;
    if (bytes_read > 0)
        x_scale = atof(buf);
    // line 2, rotation (ignore)
    bytes_read = reply->readLine(buf, 128);
    // line 3, rotation (ignore)
    bytes_read = reply->readLine(buf, 128);
    // line 4, y scale (negative)
    bytes_read = reply->readLine(buf, 128);
    y_scale = 0;
    if (bytes_read > 0)
        y_scale = atof(buf);
    // line 5, x coord
    bytes_read = reply->readLine(buf, 128);
    lon = 0;
    if (bytes_read > 0)
        lon = atof(buf);
    // line 6, y coord
    bytes_read = reply->readLine(buf, 128);
    lat = 0;
    if (bytes_read > 0)
        lat = atof(buf);

    // for some reason this rect doesn't want me to
    // set the scale before settings the x,y
    world_file_infos.setRect(lon, lat, x_scale, y_scale);

    reply->deleteLater(); // done with the reply now

    QUrl overlay_url = QUrl("");

    if (type == 0) // radar
    {
        overlay_url = QUrl("http://radar.weather.gov/ridge/RadarImg/N0R/" + RadarStationShortName(r) + "_N0R_0.gif");
    }
    else if (type == 1) // warnings
    {
        overlay_url = QUrl("http://radar.weather.gov/ridge/Warnings/Short/" + RadarStationShortName(r) + "_Warnings_0.gif");
    }

    if (url.path() != "")
    {
        QNetworkRequest request;
        request.setUrl(overlay_url);
        request.setRawHeader("User-Agent", "Nokia (Qt) Graphics Dojo 1.0");
        request.setAttribute(QNetworkRequest::User, (int)r);
        request.setAttribute(QNetworkRequest::UserMax, world_file_infos);
        m_net_overlay.get(request);
    }

}

void APRSmain::processOverlay(QNetworkReply *reply)
{
    QImage img;
    //QUrl url = reply->url();
    QRectF overlay_infos = reply->request().attribute(QNetworkRequest::UserMax).toRectF();
    RadarStation r = (RadarStation)reply->request().attribute(QNetworkRequest::User).toInt();
    if (!reply->error())
    {
        if (!img.load(reply, 0))
        {
            img = QImage();
            reply->deleteLater();
            return;
        }
    }
    reply->deleteLater();

    int type;
    if (reply->url().toString().contains("RadarImg", Qt::CaseSensitive))
        type = 1;
    else if (reply->url().toString().contains("Warnings", Qt::CaseSensitive))
        type = 2;
    else
        type = 0;

    qreal overlay_lon_width = overlay_infos.width() * img.width(); // overlay_infos.width is x scale
    qreal overlay_lat_height = overlay_infos.height() * img.height(); // overlay_infos.height is y scale

    Overlay* v = new Overlay(img, overlay_infos.x(), overlay_infos.y(),
                             overlay_lon_width, overlay_lat_height, mMap->getZoom(),
                             r, type);
    mMap->Add(v);

}

void APRSmain::on_connectionList_doubleClicked(QModelIndex index)
{
    QListWidgetItem* item = ui->connectionList->currentItem();
    QString alias = item->text();

    if(parserThread->getConnection(alias)->GetConnectionStatus())
    {
        parserThread->getConnection(alias)->disconnectIS();
    }
    else
    {
        parserThread->getConnection(alias)->connectToIS();
    }
    /*
    foreach(Connection* conn, parserThread->connections)
    {
        if(conn->getAlias() == alias)
        {
            if(conn->GetConnectionStatus())
            {
                conn->disconnectIS();
                //item->setIcon(discIcon);
            }
            else
            {
                conn->connectToIS();
                //item->setIcon(connIcon);
            }
        }
    }
    */
}

void APRSmain::updateConnections(QString alias)
{
    // Connection List
    QList<QListWidgetItem*> item = ui->connectionList->findItems(alias, Qt::MatchExactly);
    if(parserThread->isConnected(alias))
    {
        item.at(0)->setIcon(connIcon);
    }
    else
    {
        item.at(0)->setIcon(discIcon);
    }

    // Connection Menu
    foreach(QAction* action, ui->menuConnection->actions())
    {
        if(action->data() == alias)
        {
            if(parserThread->isConnected(alias))
            {
                action->setIcon(connIcon);
            }
            else
            {
                action->setIcon(discIcon);
            }
        }
    }
}

void APRSmain::on_actionDisconnect_All_triggered()
{
    parserThread->DisconnectAll();
}

void APRSmain::toggleConnection()
{
    QAction* action = (QAction*)sender();
    if(action == 0)
    {
       return;
    }
    QString alias = action->data().toString();

    foreach(Connection* conn, parserThread->connections)
    {
        if(conn->getAlias() == alias)
        {
            if(conn->GetConnectionStatus())
            {
                conn->disconnectIS();
                //action->setIcon(discIcon);
            }
            else
            {
                conn->connectToIS();
                //action->setIcon(connIcon);
            }
        }
    }
}

void APRSmain::on_actionView_Invalid_Packets_triggered()
{
    QSqlQueryModel *packetModel = new QSqlQueryModel;
    packetModel->setQuery("SELECT * FROM invalidpackets");

    QTableView *packetsView = new QTableView();
    packetsView->setModel(packetModel);
    packetsView->setWindowTitle("Captured Packets");
    packetsView->show();
}

void APRSmain::on_actionShow_weather_list_triggered()
{
    // checked state changes, then this event is processed
    if (ui->actionShow_weather_list->isChecked() == false)
    {
        ui->weatherOverlayFrame->hide();
    }
    else
    {
        ui->weatherOverlayFrame->show();
    }
}

void APRSmain::on_actionShow_status_bar_triggered()
{
    // checked state changes, then this event is processed
    if (ui->actionShow_status_bar->isChecked() == false)
    {
        ui->statusBar->hide();
    }
    else
    {
        ui->statusBar->show();
    }
}

void APRSmain::on_actionShow_connections_triggered()
{
    // checked state changes, then this event is processed
    if (ui->actionShow_connections->isChecked() == false)
    {
        ui->connectionListFrame->hide();
    }
    else
    {
        ui->connectionListFrame->show();
    }
}

void APRSmain::on_actionCallsigns_list_triggered()
{
    // checked state changes, then this event is processed
    if (ui->actionCallsigns_list->isChecked() == false)
    {
        ui->callsignsFrame->hide();
    }
    else
    {
        ui->callsignsFrame->show();
    }
}

//void APRSmain::on_actionRemove_Traces_triggered()
//{
//    mMap->removeAllTraces();
//}

void APRSmain::on_actionE_xit_triggered()
{
    this->close();
}

void APRSmain::ChangeProfile(QString newProfile)
{
    this->setWindowTitle("Team 05soft - APRS Mapper v2 :: " + Profile::getCurrentProfile());
}

void APRSmain::setStatusBar2Text(QString text)
{
    this->statusBar2Label.setText(text);

    if (clearStatus2.isActive())
        clearStatus2.stop();

    clearStatus2.start(20000);
}

void APRSmain::setStatusBar3Text(QString text)
{
    this->statusBar3Label.setText(text);
}

void APRSmain::clearStatus2Event()
{
    clearStatus2.stop();
    this->statusBar2Label.setText("");
}


void APRSmain::replotHome()
{
    // Plot home point
    Coordinate homeCoord = Coordinate(Profile::getLatitude(), Profile::getLongitude());
    //qDebug() << homeCoord->ToDecimalLatLon()->x() << "  " << homeCoord->ToDecimalLatLon()->y();
    Point* home = new Point(Profile::getCallSign(), homeCoord.ToDecimalLatLon());
    home->setSymbolId(Profile::getIconId());
    home->setOverlay(Profile::getOverlayChar());
    home->setTimestamp(QDateTime::currentDateTime());
    home->setPacketType(T_LOCATION);
    mMap->Add(home);
}


void APRSmain::setupFilters()
{
    QListWidgetItem *i;

    for (int id=0; id<192; id++)
    {
        // all of the icons are in one .png file, in rows of 16 icons
        int h_start = 0;
        int v_start = 0;
        int side_length = 20;
        int border = 1;

        int start_col = id % 16; // 16 columns per row
        int start_row = id / 16;

        h_start = ((side_length * start_col) + (border * (start_col + 1)));
        v_start = ((side_length * start_row) + (border * (start_row + 1)));

        //m_symbolId = id;
        //delete m_iconImage;
        QPixmap* m_iconImage = new QPixmap(20, 20); // empty space for working

        QImage icons = QImage(":/icons/aprs_icons.png", 0); // going to use this file
        m_iconImage->fill(Qt::transparent);    // make sure the background is empty
        QPainter painter(m_iconImage); // set a painter
        painter.setBackgroundMode(Qt::TransparentMode); // make double sure the background is empty
        // so now grab the correct icon from the big file
        painter.drawImage(0, 0, icons, h_start, v_start, side_length, side_length);

        i = new QListWidgetItem();
        i->setIcon(QIcon(*m_iconImage));
        i->setCheckState(Qt::Checked);
        ui->listFilterWhiteSymbol->addItem(i);

        i = new QListWidgetItem();
        i->setIcon(QIcon(*m_iconImage));
        i->setCheckState(Qt::Unchecked);
        ui->listFilterBlackSymbol->addItem(i);
    }

    i = new QListWidgetItem();
    i->setCheckState(Qt::Checked);
    i->setText(tr("ITEM"));
    ui->listFilterWhiteType->addItem(i);
    i = new QListWidgetItem();
    i->setCheckState(Qt::Checked);
    i->setText(tr("OBJECT"));
    ui->listFilterWhiteType->addItem(i);
    i = new QListWidgetItem();
    i->setCheckState(Qt::Checked);
    i->setText(tr("LOCATION"));
    ui->listFilterWhiteType->addItem(i);

    i = new QListWidgetItem();
    i->setCheckState(Qt::Unchecked);
    i->setText(tr("ITEM"));
    ui->listFilterBlackType->addItem(i);
    i = new QListWidgetItem();
    i->setCheckState(Qt::Unchecked);
    i->setText(tr("OBJECT"));
    ui->listFilterBlackType->addItem(i);
    i = new QListWidgetItem();
    i->setCheckState(Qt::Unchecked);
    i->setText(tr("LOCATION"));
    ui->listFilterBlackType->addItem(i);
}

void APRSmain::on_buttonClearAllFilters_pressed()
{
    for(int i = 0; i < ui->listFilterBlackSymbol->count(); i++)
    {
        ui->listFilterBlackSymbol->item(i)->setCheckState(Qt::Unchecked);
    }
    for(int i = 0; i < ui->listFilterWhiteSymbol->count(); i++)
    {
        ui->listFilterWhiteSymbol->item(i)->setCheckState(Qt::Checked);
    }
    for(int i = 0; i < ui->listFilterBlackType->count(); i++)
    {
        ui->listFilterBlackType->item(i)->setCheckState(Qt::Unchecked);
    }
    for(int i = 0; i < ui->listFilterWhiteType->count(); i++)
    {
        ui->listFilterWhiteType->item(i)->setCheckState(Qt::Checked);
    }
    sendNewFilters();
}

void APRSmain::on_buttonApplyFilters_pressed()
{
    sendNewFilters();
}

void APRSmain::sendNewFilters()
{
    QVector<int> bs;
    for(int i = 0; i < ui->listFilterBlackSymbol->count(); i++)
    {
        if (ui->listFilterBlackSymbol->item(i)->checkState() == Qt::Checked)
            bs.append(i);
    }
    emit applyFilterBlackSymbol(bs);

    QVector<int> ws;
    for(int i = 0; i < ui->listFilterWhiteSymbol->count(); i++)
    {
        if (ui->listFilterWhiteSymbol->item(i)->checkState() == Qt::Checked)
            ws.append(i);
    }
    emit applyFilterWhiteSymbol(ws);

    QVector<aprstype> bt;
    if (ui->listFilterBlackType->item(0)->checkState() == Qt::Checked)
        bt.append(T_ITEM);
    if (ui->listFilterBlackType->item(1)->checkState() == Qt::Checked)
        bt.append(T_OBJECT);
    if (ui->listFilterBlackType->item(2)->checkState() == Qt::Checked)
        bt.append(T_LOCATION);
    emit applyFilterBlackType(bt);

    QVector<aprstype> wt;
    if (ui->listFilterWhiteType->item(0)->checkState() == Qt::Checked)
        wt.append(T_ITEM);
    if (ui->listFilterWhiteType->item(1)->checkState() == Qt::Checked)
        wt.append(T_OBJECT);
    if (ui->listFilterWhiteType->item(2)->checkState() == Qt::Checked)
        wt.append(T_LOCATION);
    emit applyFilterWhiteType(wt);
}

void APRSmain::on_buttonBlacklistAll_pressed()
{
    for(int i = 0; i < ui->listFilterBlackSymbol->count(); i++)
    {
        ui->listFilterBlackSymbol->item(i)->setCheckState(Qt::Checked);
    }
    for(int i = 0; i < ui->listFilterBlackType->count(); i++)
    {
        ui->listFilterBlackType->item(i)->setCheckState(Qt::Checked);
    }
}

void APRSmain::on_buttonBlacklistNone_pressed()
{
    for(int i = 0; i < ui->listFilterBlackSymbol->count(); i++)
    {
        ui->listFilterBlackSymbol->item(i)->setCheckState(Qt::Unchecked);
    }
    for(int i = 0; i < ui->listFilterBlackType->count(); i++)
    {
        ui->listFilterBlackType->item(i)->setCheckState(Qt::Unchecked);
    }
}

void APRSmain::on_buttonWhitelistAll_pressed()
{
    for(int i = 0; i < ui->listFilterWhiteSymbol->count(); i++)
    {
        ui->listFilterWhiteSymbol->item(i)->setCheckState(Qt::Checked);
    }
    for(int i = 0; i < ui->listFilterWhiteType->count(); i++)
    {
        ui->listFilterWhiteType->item(i)->setCheckState(Qt::Checked);
    }
}

void APRSmain::on_buttonWhitelistNone_pressed()
{
    for(int i = 0; i < ui->listFilterWhiteSymbol->count(); i++)
    {
        ui->listFilterWhiteSymbol->item(i)->setCheckState(Qt::Unchecked);
    }
    for(int i = 0; i < ui->listFilterWhiteType->count(); i++)
    {
        ui->listFilterWhiteType->item(i)->setCheckState(Qt::Unchecked);
    }
}
