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

#ifndef MAP_H
#define MAP_H

#include <QObject>
#include <QGraphicsView>
#include <QPixmap>
#include <QVector>

#include "errorhandler.h"
#include "point.h"
#include "overlay.h"
#include "profile.h"
#include "packettypes.h"
#include "database.h"


#include <QtGui>
#include <QtNetwork>



// This clas is the bastard child of at least two different projects
// and are own existing solution.
// the following two websites have been invaluable in getting a
// working product; I'm sure you can recognize some of the code that
// I'm using from there, but I implemented many features very differently.
// http://labs.qt.nokia.com/2009/08/04/openstreetmap-and-qt-and-s60/
// http://code.google.com/p/zee-qt/source/browse/trunk/dev/maptest/?r=122
// -- Ben Burns
class Map : public QObject
{
    Q_OBJECT

public:
    // constructors
    Map(QGraphicsView* view, Database *d);

    Point* getPointFromCallsign(QString callsign);
    Overlay* getOverlay(RadarStation r, int type);

    int getZoom() { return zoom; }  // OSM zoom level

    QRectF getLatLonBound();
    QPointF getMapOrigin(); // returns upper left corner of map in lat/long
    QVector<Point *> Items; // list of plotted points (why is this public?)
    QVector<Overlay *> Overlays; // list of added overlays
    //QVector<QGraphicsLineItem *> Traces;
    QPointF getViewCenterLatLon();

    // these are public to be accessed by the download thread
    QPixmap m_emptyTile;    // default image to show when a tile is unavailable
    QHash<QPoint, QPixmap> m_tilePixmaps;   // internal reference of available tiles
    QHash<QPoint, QGraphicsItem*> m_drawnTiles; // list of correctly rendered tiles
    QHash<QPoint, QGraphicsItem*> m_emptyTiles; // list of empty tiles
    QGraphicsView* mView;   // the map view
    QRect m_tilesInView; // box containg all the tiles that should be rendered

    void purgeNonVisiblePoints();
    void purgeNonVisibleTiles();




private:
    void drawGridLines();   // not implemented
    void pan(const QPointF &delta); // moves the view by delta px
    void RemoveAllTiles();  // empties (background) tiles
    void redrawPoints();    // replots everything in Items
    void redrawOverlays();
    void hideAllOverlays();
    void showAllOverlays();
    void hideAllPoints();
    void showAllPoints();
    void refreshLatLonBound();
    void zoomAllPoints();


    void timerEvent(QTimerEvent *); // resize event timer

    QPoint m_offset;    // ?
    QRect m_tilesRect;  // bounding box of drawn tiles, in px
    QRectF m_mapLatLonBound;

    //int width;  // width of view
    //int height; // height of view
    int zoom;   // OSM zoom
    int last_zoom;
    qreal latitude; // center of view
    qreal longitude; // center of view

    QNetworkAccessManager m_manager;
    QUrl m_url;
    QQueue<QPoint> m_tilesToDownload;
    QQueue<QPoint> m_pendingTileDownloads;
    void updateTileBounds(); // updates the bounding box containing the tiles to be rendered
    void render(); // redraws the entire map

    void clearPendingDownloads();

    QPointF m_tileToViewOffset; // offset from m_tilesInView origin to mView origin

    // these are for tracking mouse events to pan correctly
    // don't use setDraggable
    qint64 oX, oY;
    qint64 sX, sY;
    qint64 dX, dY;

    QBasicTimer resizeTimer; // don't refresh a million times while resizing
    QRectF resizeRect;
    QPointF old_center;

    Point* lastActiveItem;
    Point* emptyPoint;

    QVector<aprstype> blackListType;
    QVector<aprstype> whiteListType;
    QVector<int> blackListSymbol;
    QVector<int> whiteListSymbol;
    bool doesPointPassFilters(Point *);

    bool packetsListOpen;

    Database *db;


protected:
    QRect tileRect(const QPoint &tp);   // ?

public slots:
    void Add(Point* p);
    void Add(Overlay* v);
    void ZoomIn();
    void ZoomOut();
    void ZoomTo(int);
    void RemoveItem(Point* p);
    void RemoveOverlay(Overlay* v);
    void CenterOn(QString callsign);
    void CenterOn(QPointF coordinates);
    void CenterOn (int px, int py);
    void RemoveAllItems();
    //void removeAllTraces();

    void sceneMouseMoveEvent(QGraphicsSceneMouseEvent *);
    void sceneMousePressEvent(QGraphicsSceneMouseEvent *);
    void sceneMouseReleaseEvent(QGraphicsSceneMouseEvent *);
    void sceneRectChanged(const QRectF &rect);  // forces update of view. Or not.

    void keyPressEvent(QKeyEvent *event);
    void ClearOldItems();

    void render(QPoint tp); // redraws the tile at tile location tp

    void setBlackListType(QVector<aprstype>);
    void setWhiteListType(QVector<aprstype>);
    void setBlackListSymbol(QVector<int>);
    void setWhiteListSymbol(QVector<int>);

    void packetsListIsOpen();
    void packetsListIsClosed();

signals:
    void RequestPointsInRange(QRectF area);

    void tellDatabaseToStopSendingPoints();

    void PointPlotted(Point*);
    void PointRemoved(QString);
    void showMouseOverStats(QString);

private slots:
    void handleNetworkData(QNetworkReply*);
    void download(QRect tilesInView);
};

#endif // MAP_H
