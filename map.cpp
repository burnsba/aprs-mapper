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

#include <QtGui>
#include <QDebug>
#include "map.h"
#include "overlay.h"
#include "errorhandler.h"
#include "profile.h"
#include "coordinate.h"
#include <QtNetwork>
#include <QFuture>
#include "packetslist.h"

#include "time.h"

Map::Map(QGraphicsView *view, Database *d)
    :mView(view)

{
    try
    {        

        // reset mouse stats to nothing
        oX = 0; oY = 0;
        sX = 0; sY = 0;
        dX = 0; dY = 0;

        view->scene()->setSceneRect(0,0,1024,1024);
        view->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
        view->setResizeAnchor(QGraphicsView::AnchorViewCenter);
        view->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        view->scene()->setBackgroundBrush(Qt::lightGray);

        // load profile settings.
        zoom = 15;
        latitude = Profile::getLatitude();
        longitude = Profile::getLongitude();

        // setup empty tiles
        m_emptyTile = QPixmap(TILE_SIZE, TILE_SIZE);
        m_emptyTile.fill(Qt::lightGray);

        connect(&m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(handleNetworkData(QNetworkReply*)), Qt::QueuedConnection);

        // draw the map for the first time (or it will be completely blank on startup)
        // wait to finish starting, then redraw...
        old_center = QPointF(Profile::getMapLong(), Profile::getMapLat());

        zoom = Profile::getMapZoom();
        last_zoom = zoom;
        CenterOn(QPointF(0.0, 0.0));
        resizeTimer.start(200, this);

        emptyPoint = new Point();
        lastActiveItem = emptyPoint;

        packetsListOpen = false;

        db = d; // store a reference to the database

    }
    catch (...)
    {
        ErrorHandler::AlertAndLog("Map::Map() -- Problem initializing the map.");
    }

}

void Map::Add(Point *p)
{
    try
    {
        if (p->scene() == mView->scene())
            qDebug() << "HEY! item already added";

        if (!doesPointPassFilters(p))
        {
            delete p;
            return;
        }

        // don't plot items that aren't in the current map view
        QPointF pointLocation = p->getLocation();
        if (!getLatLonBound().contains(pointLocation.x(), pointLocation.y()) && p->getSource() != Profile::getCallSign())
        {
            delete p;
            return;
        }

        foreach(Point* item, Items)
        {
            if(p->getSource() == item->getSource())
            {
                // this means the point already exists in the scene
                // now figure out if this is an actual duplicate, or if this is a point that
                // has moved since the last time it was added.
                if (p->getTimestamp() > item->getTimestamp() && p->getLocation() != item->getLocation())
                {
                    //qDebug() << "duplicate! " + p->getSource();
                    if (item->addTrace(p->getTimestamp(), p->getLocation()))
                    {
                        //qDebug() << p->getSource() + ": added location " + QString::number(p->getLocation().x()) + ", " + QString::number(p->getLocation().y());
                        item->setLocation(p->getLocation().y(), p->getLocation().x());
                        Coordinate c = Coordinate(p->getLocation());
                        QPointF sc = c.ToSceneCoord(getMapOrigin(), zoom);
                        item->setPos(sc.x(), sc.y());

//                        if (sc.y() > mView->height() || sc.x() > mView->width() || sc.x() < 0 || sc.y() < 0)
//                        {
//                            qDebug() << "moving point off screen";
//                        }
                    }

                }
                delete p;
                return;
            }
        }

        Coordinate c = Coordinate(p->getLocation()); // getLocation returns a pointer...
        QPointF sc = c.ToSceneCoord(getMapOrigin(), zoom);
        p->setPos(sc.x(), sc.y());
        Items.append(p);
        mView->scene()->addItem(p);
        emit PointPlotted(p);

    }
    catch(...)
    {
        ErrorHandler::AlertAndLog("Map::Add(" + p->getSource() + ") -- problem adding point at [" + QString::number(p->getLocation().y()) + ", " + QString::number(p->getLocation().x()) + "]");
    }
}

void Map::Add(Overlay *v)
{
    try
    {
        // make sure the overlay has not already been added.
        // Note that overlays must have the same RadarStation id and the
        // same RadarImg/Warning type to be a duplicate
        foreach(Overlay* over_i, Overlays)
        {
            if((v != NULL && over_i != NULL) &&
               (v->getId() == over_i->getId()) &&
               (v->getType() == over_i->getType()))
            {
                return;
            }
        }

        // so it was not a duplicate. Add to the list
        this->Overlays.append(v);
        // If the scene is null when Draw is called, segfault will be the result
        v->setScene(this->mView->scene());
        v->Draw();

        // draw sets the scene position to 1,1, so figure out the
        // correct location in pixel coordinates
        Coordinate c = Coordinate(v->getLocation()); // getlocation does not return a pointer
        QPointF sc = c.ToSceneCoord(getMapOrigin(), zoom);
        v->setSceneLocation(sc.x(), sc.y());
    }
    catch(...) { /* rawr! */ }
}


void Map::hideAllPoints()
{
    foreach(Point* item, this->Items)
    {
        item->hide();;
    }
}

void Map::showAllPoints()
{
    foreach(Point* item, this->Items)
    {
        item->show();
    }
}

void Map::hideAllOverlays()
{
    foreach(Overlay* v, this->Overlays)
    {
        v->Hide();
    }
}

void Map::showAllOverlays()
{
    foreach(Overlay* v, this->Overlays)
    {
        v->Show();
    }
}

void Map::redrawPoints()
{
    try
    {
        hideAllPoints();
        QPointF map_origin = getMapOrigin();
        foreach(Point* item, this->Items)
        {
            Coordinate c = Coordinate(item->getLocation());
            QPointF sc = c.ToSceneCoord(map_origin, zoom);
            item->setPos(sc.x(), sc.y());
        }
        showAllPoints();
    }
    catch(...)
    {
        ErrorHandler::AlertAndLog("Problem redrawing the points.");
    }
}

void Map::redrawOverlays()
{
    hideAllOverlays();
    QPointF map_origin = getMapOrigin();
    foreach(Overlay* v, this->Overlays)
    {
        v->setZoom(zoom);
        v->Draw();
        Coordinate c = Coordinate(v->getLocation());
        QPointF sc = c.ToSceneCoord(map_origin, zoom);
        v->setSceneLocation(sc.x(), sc.y());
    }

    showAllOverlays();
}

void Map::ZoomIn()
{
    zoom = zoom > 14 ? 15 : zoom + 1;
    last_zoom = zoom - 1;
    ZoomTo(zoom);
}

void Map::ZoomOut()
{
    zoom = zoom < 3 ? 2 : zoom - 1;
    last_zoom = zoom + 1;
    ZoomTo(zoom);
}

void Map::ZoomTo(int zoomLevel)
{
    //qDebug() << "inside ZoomTo";

    if (zoomLevel > 15 || zoomLevel < 3)
        return;

    // tell the database to hold up on those points
    emit tellDatabaseToStopSendingPoints();

    last_zoom = zoom;

    // stop all pending download requests
    clearPendingDownloads();
    //removeAllTraces();

    QPointF pre_zoom_center = QPointF(longitude, latitude);

    zoom = zoomLevel;
    hideAllPoints();
    zoomAllPoints();
    RemoveAllTiles();
    CenterOn(pre_zoom_center); // pans, refreshes lat/lon boundaries, and removes non-visible points
    redrawOverlays();
    last_zoom = zoom; // this is not redundant

}

void Map::RemoveItem(Point *p)
{
    try
    {
        if (p == NULL)
            return;
        if (p->getSource() == Profile::getCallSign())
            return;
        if (lastActiveItem == p)
            lastActiveItem = emptyPoint;
        qint32 index = Items.indexOf(p, 0);
        if (index >= 0)
            Items.remove(index);
        QString callsign = p->getSource();
        if (callsign != "")
            emit PointRemoved(callsign);
        mView->scene()->removeItem(p);
        delete p;
    }
    catch(...)
    {
        ErrorHandler::AlertAndLog("Could not remove point " + p->getSource());
    }
}

void Map::RemoveOverlay(Overlay *v)
{
    v->Hide();
    int index = Overlays.indexOf(v, 0);
    Overlays.remove(index);
    v->RemoveFromMap();
}


void Map::CenterOn(QString callsign)
{
    foreach(Point* item, this->Items)
    {
        if(item->getSource() == callsign)
        {
            //mView->centerOn(item->getInternalItem()->x(), item->getInternalItem()->y());
            CenterOn(item->getLocation());
            break;
        }
    }
}

// centers on a long/lat
// calls pan, which refreshes the lat/lon boundaries
// also removes non-visible points
void Map::CenterOn(QPointF coordinates)
{
    //qDebug() << "inside centerOn()";

    QPointF pan_to_tile = tileForCoordinate(coordinates.y(), coordinates.x(), zoom);
    QPointF current_tile = tileForCoordinate(latitude, longitude, zoom);

    //qDebug() << "calling CenterOn: " + QString::number(coordinates.y()) + ", " + QString::number(coordinates.x());

    //latitude = coordinates.y();
    //longitude = coordinates.x();

    // just pan there
    QPointF delta = current_tile - pan_to_tile; // I know this is backwards, but this is how it works...
    delta.setX(delta.x() * TILE_SIZE); // convert tiles to pixels
    delta.setY(delta.y() * TILE_SIZE);

    pan(delta);
    redrawPoints();

}

// centers the map on a x,y coord (in pixels)
void Map::CenterOn(int px, int py)
{
    //qDebug() << "CenterOn(" + QString::number(px) + ", " + QString::number(py) + ")";

    // find center of map
    QPointF center = tileForCoordinate(latitude, longitude, zoom);

    // find tile difference between center and 0,0
    qreal tile_offset_x = (static_cast<qreal>(mView->width())/static_cast<qreal>(2))/static_cast<qreal>(TILE_SIZE);
    qreal tile_offset_y = (static_cast<qreal>(mView->height())/static_cast<qreal>(2))/static_cast<qreal>(TILE_SIZE);

    // adjust for offset to px,py
    tile_offset_x -= static_cast<qreal>(px)/static_cast<qreal>(TILE_SIZE);
    tile_offset_y -= static_cast<qreal>(py)/static_cast<qreal>(TILE_SIZE);

    // center
    CenterOn(QPointF(longitudeFromTile(center.x() - tile_offset_x, zoom),
                   latitudeFromTile(center.y() - tile_offset_y, zoom)));
}

void Map::RemoveAllItems()
{
    foreach(Point* item, this->Items)
    {
        RemoveItem(item);
    }
}

void Map::RemoveAllTiles()
{
    try
    {
        if (!m_tilePixmaps.isEmpty())
        {
            QHash<QPoint, QPixmap>::iterator i = m_tilePixmaps.begin();
            while (i != m_tilePixmaps.end())
            {
                i = m_tilePixmaps.erase(i);
            }
        }

        if (!m_drawnTiles.isEmpty())
        {
            QHash<QPoint, QGraphicsItem*>::iterator j = m_drawnTiles.begin();
            while (j != m_drawnTiles.end())
            {
                mView->scene()->removeItem(m_drawnTiles[j.key()]);
                QGraphicsItem* item = m_drawnTiles[j.key()];
                delete item;
                j = m_drawnTiles.erase(j);
            }
        }
        if (!m_emptyTiles.isEmpty())
        {
            QHash<QPoint, QGraphicsItem*>::iterator k = m_emptyTiles.begin();
            while (k != m_emptyTiles.end())
            {
                mView->scene()->removeItem(m_emptyTiles[k.key()]);
                QGraphicsItem* item = m_emptyTiles[k.key()];
                delete item;
                k = m_emptyTiles.erase(k);
            }
        }
    }
    catch(...)
    {

    }

}

Point* Map::getPointFromCallsign(QString callsign)
{
    foreach(Point* item, this->Items)
    {
        if(item->getSource() == callsign)
        {
            return item;
        }
    }

    return 0;
}

Overlay* Map::getOverlay(RadarStation r, int type)
{
    foreach(Overlay* v, Overlays)
    {
        //qDebug() << "comparing " + QString::number((int)r) + ", " + QString::number(type) + " to " + QString::number((int)v->getId()) + ", " + QString::number(v->getType());
        if(v->getId() == r && v->getType() == type)
        {
            return v;
        }
    }

    return 0;
}

// start slippy map code....


void Map::updateTileBounds()
{
    // this function updates the class variables that hold the current view size.
    // tilesInView and tileToViewOffset

    // get size of the current view in tiles (convert pixels to tiles)
    QRectF exactTileSizeOfView = QRectF(0, 0,
           static_cast<qreal>(mView->width())/static_cast<qreal>(TILE_SIZE),
           static_cast<qreal>(mView->height())/static_cast<qreal>(TILE_SIZE));

    // take the location the map is centered on...
    QPointF centerTile = tileForCoordinate(latitude, longitude, zoom);

    // The tile for 0,0 of the view is used a lot, so I'm precalculating it here.
    QPointF viewOriginAsTile = QPointF(centerTile.x() - (exactTileSizeOfView.width() / 2),
                                       centerTile.y() - (exactTileSizeOfView.height() / 2));

    // ... and find all the tiles that should be displayed based on the current
    // viewing area. This will build a QRect (not QRectF) containing a list of all the
    // tiles that should be rendered.

    m_tilesInView = QRect(
            /* find the left edge of the far left tile */
            QPoint(static_cast<int>(floor(viewOriginAsTile.x())),
                   /* find the top edge of the very top tile */
                   static_cast<int>(floor(viewOriginAsTile.y()))),
            /* find the left edge of the far right tile */
            QPoint(static_cast<int>(ceil(viewOriginAsTile.x() + exactTileSizeOfView.width())),
                   /* find the top edge of the very bottom tile */
                   static_cast<int>(ceil(viewOriginAsTile.y() + exactTileSizeOfView.height())))
            );

    // now there's some crazy rectanlge the size of all the tiles in view. However,
    // because of the floor/ceil, this is not the location of 0,0.
    // The last thing to do is calculate the offset between tilesInView.upperLeft()
    // and the 0,0 location of the view, in pixels
    m_tileToViewOffset = QPointF(
            (viewOriginAsTile.x() - floor(viewOriginAsTile.x())) * TILE_SIZE,
            (viewOriginAsTile.y() - floor(viewOriginAsTile.y())) * TILE_SIZE
            );
}

void Map::render(QPoint tp)
{
//    time_t start, end;
//    start = clock();

    // get the tile origin. This could potentially be a very large number,
    // but I actually want to put the first tile at 0,0 (and adjust for
    // offset)
    QPointF tileOrigin = QPointF(m_tilesInView.x(), m_tilesInView.y());

    QGraphicsItem* item = NULL;
    if (m_tilePixmaps.contains(tp))
    {
        // check to see if this was a blank tile that is now being updated
        if (m_emptyTiles.contains(tp))
        {
            mView->scene()->removeItem(m_emptyTiles[tp]);
            QGraphicsItem *item = m_emptyTiles[tp];
            delete item;
            m_emptyTiles.remove(tp);
        }
        // check to see if this is an existing tile that's being updated (shouldn't be
        // here very often)
        if (m_drawnTiles.contains(tp))
        {
            mView->scene()->removeItem(m_drawnTiles[tp]);
            QGraphicsItem* item = m_drawnTiles[tp];
            delete item;
            m_drawnTiles.remove(tp);
        }
        item = new QGraphicsPixmapItem(m_tilePixmaps.value(tp), NULL, NULL);
        //qDebug() << "drawing image: " + QString::number(tp.x()) + ", " + QString::number(tp.y());
        item->setZValue(-1);
        m_drawnTiles[tp] = item;
        // x is tile position. figure out the offset to view/tile corner,
        // then move it over by so many tiles, and adjust for view offset.
        ((QGraphicsPixmapItem*)item)->setOffset(
                ((tp.x() - tileOrigin.x()) * TILE_SIZE) - m_tileToViewOffset.x(),
                ((tp.y() - tileOrigin.y()) * TILE_SIZE) - m_tileToViewOffset.y());
        mView->scene()->addItem(item);

    }
    else
    {
        // make sure this hasn't already been drawn
        if (!m_emptyTiles.contains(tp))
        {
            item = new QGraphicsPixmapItem(m_emptyTile, NULL, NULL);
            //qDebug() << "drawing blank: " + QString::number(tp.x()) + ", " + QString::number(tp.y());
            item->setZValue(-1000);
            m_emptyTiles[tp] = item;
            ((QGraphicsPixmapItem*)item)->setOffset(
                    ((tp.x() - tileOrigin.x()) * TILE_SIZE) - m_tileToViewOffset.x(),
                    ((tp.y() - tileOrigin.y()) * TILE_SIZE) - m_tileToViewOffset.y());
            mView->scene()->addItem(item);
        }
    }

//    end = clock();
//    if (end - start > 0)
//    qDebug() << "render(tp) took " + QString::number(end - start);

}

void Map::render()
{
//    clock_t start, end;
//    start = clock();
    for (int x = m_tilesInView.x(); x <= m_tilesInView.x() + m_tilesInView.width(); x++)
    {
        for (int y = m_tilesInView.y(); y <= m_tilesInView.y() + m_tilesInView.height(); y++)
        {
            QPoint tp = QPoint(x,y);
            render(tp);
        }
    }
//    end = clock();
//    qDebug() << "render took " + QString::number(end - start);
}



void Map::sceneMouseMoveEvent(QGraphicsSceneMouseEvent *e)
{
    //qDebug () << "entering mouse move.";
    if (e->scenePos().x() == 0 && e->scenePos().y() == 0)
        return;
    try
    {
        QGraphicsItem *item = 0;
        QList<QGraphicsItem *> items_list;
        //item = mView->scene()->itemAt(e->scenePos());
        items_list = mView->scene()->items(e->scenePos());
        if (items_list.count() > 0)
            item = items_list.first();

        if (item != NULL)
        {
            Point* p = dynamic_cast<Point*>(item);

            if (p != NULL)
            {
                if (p != lastActiveItem)
                {
                    p->m_currentlyActive = true;
                    lastActiveItem->m_currentlyActive = false;
                    lastActiveItem->update();
                    p->update();
                    lastActiveItem = p;

                    qreal d = distanceBetweenTwoPoints(p->getLocation(), QPointF(Profile::getLongitude(), Profile::getLatitude()));
                    qreal b = bearingBetweenTwoPoints(p->getLocation(), QPointF(Profile::getLongitude(), Profile::getLatitude()));

                    emit showMouseOverStats(p->getSource() + ": (" + p->getTimestamp().toString("yyyy-MM-dd, HH:mm:ss") + ") " +
                                            (Profile::isUnitsEnglish() == true ? QString::number(d*0.621371192) + " mi away" : QString::number(d) + " km away @ ") +
                                            QString::number(b) + "° (" + bearingToCompassPoint(b) + ")");
                }
                return;
            }
            else
            {
                if (lastActiveItem != emptyPoint)
                {
                    lastActiveItem->m_currentlyActive = false;
                    lastActiveItem->update();
                    lastActiveItem = emptyPoint;
                    emit showMouseOverStats("");
                }
            }
        }
    }
    catch(...)
    {
        qDebug() << "mousemove: Could not get items at " + QString::number(e->scenePos().x()) + ", " + QString::number(e->scenePos().y());
    }


    //qDebug() << "leaving mouse move";
}

void Map::sceneMousePressEvent(QGraphicsSceneMouseEvent *e)
{
    dX = oX;
    dY = oY;
//  starting offset recorded
    sX = static_cast<qint64>(e->scenePos().x());
    sY = static_cast<qint64>(e->scenePos().y());

    if (packetsListOpen)
        return;

    if (e->button() != Qt::RightButton)
        return;

    QQueue<QString> items_under_mouse;

    if (e->scenePos().x() == 0 && e->scenePos().y() == 0)
        return;
    int num_items_found = 0;
    QList<QGraphicsItem *> items_list;
    items_list = mView->scene()->items(e->scenePos().x() - 5, e->scenePos().y() - 5, 10, 10, Qt::IntersectsItemShape, Qt::AscendingOrder);

    foreach(QGraphicsItem *item, items_list)
    {
        if (item != NULL)
        {
            Point* p = dynamic_cast<Point*>(item);
            if (p != NULL)
            {
                qDebug() << "found: " + p->getSource();
                //if (p->getPacketType() == T_ITEM || p->getPacketType() == T_OBJECT)
                    items_under_mouse.append(p->getSource());
                //else
                    //items_under_mouse.append(p->getData());
                num_items_found++;
            }
            else
            {

            }
        }
    }

    if (num_items_found == 0)
        return;

    packetsList *p = new packetsList(items_under_mouse, db);
    packetsListOpen = true;
    connect(p, SIGNAL(closingNow()), this, SLOT(packetsListIsClosed()));
    p->show();

}

void Map::sceneMouseReleaseEvent(QGraphicsSceneMouseEvent *e)
{
    oX = dX;
    oY = dY;

    dX = static_cast<qint64>(e->scenePos().x()) - sX;
    dY = static_cast<qint64>(e->scenePos().y()) - sY;

    if (dX != 0 || dY != 0)
        pan(QPointF(dX, dY));
}

void Map::pan(const QPointF &delta)
{
//    time_t start, end;
//    start = clock();

    QPointF p_delta = QPointF(delta) / qreal(TILE_SIZE);
    QPointF center = tileForCoordinate(latitude, longitude, zoom) - p_delta;

    qreal lat = latitudeFromTile(center.y(), zoom);
    qreal lat_delta = lat - latitude;
    qreal lon = longitudeFromTile(center.x(), zoom);
    qreal lon_delta = lon - longitude;

    // don't pan off the edge of the world
    if (p_delta.y() > 0)
        if (m_mapLatLonBound.top() + lat_delta > 85)
            return;
    if (p_delta.y() < 0)
        if (m_mapLatLonBound.bottom() + lat_delta < -85)
            return;

    if (p_delta.x() > 0)
        if (m_mapLatLonBound.left() + lon_delta < -180)
            return;
    if (p_delta.x() < 0)
        if (m_mapLatLonBound.right() + lon_delta > 180)
            return;

    latitude = lat;
    longitude = lon;
    QRectF oldLatLonBound = m_mapLatLonBound;
    refreshLatLonBound();



    //qDebug() << "panning to: " + QString::number(latitude) + ", " + QString::number(longitude);

    purgeNonVisiblePoints();
    purgeNonVisibleTiles();

//    end = clock();
//    if (end - start > 0)
//    qDebug() << "pan1 took " + QString::number(end - start);
//    start - clock();

    // query the database and add points that are about to be in the current view.

    //
    // NOTE: Y position is negative (passing negative height values to fix this)
    //
    if (zoom == last_zoom)
    {
        // this is a pan event, not a zoom.
        // figure out what lat/lon rectangles are now in the view.
        // going to go through a million cases now...

        QRectF request_rect;

        // moved north and east
        if (m_mapLatLonBound.right() > oldLatLonBound.right() && m_mapLatLonBound.top() > oldLatLonBound.top())
        {
            // get the top part
            request_rect = QRectF(m_mapLatLonBound.left(), m_mapLatLonBound.top(), m_mapLatLonBound.width(), -m_mapLatLonBound.top() + oldLatLonBound.top());
            emit RequestPointsInRange(request_rect);
            // get the right part
            request_rect = QRectF(oldLatLonBound.right(), oldLatLonBound.top(), m_mapLatLonBound.right() - oldLatLonBound.right(), -oldLatLonBound.top() + m_mapLatLonBound.bottom());
            emit RequestPointsInRange(request_rect);

            //qDebug() << " panning north east";

        }
        // moved south and east
        else if (m_mapLatLonBound.right() > oldLatLonBound.right() && m_mapLatLonBound.top() < oldLatLonBound.top())
        {
            // get the bottom part
            request_rect = QRectF(m_mapLatLonBound.left(), oldLatLonBound.bottom(), m_mapLatLonBound.width(), -oldLatLonBound.bottom() + m_mapLatLonBound.bottom());
            emit RequestPointsInRange(request_rect);
            // get the right part
            request_rect = QRectF(oldLatLonBound.right(), m_mapLatLonBound.top(), m_mapLatLonBound.right() - oldLatLonBound.right(), -m_mapLatLonBound.top() + oldLatLonBound.bottom());
            emit RequestPointsInRange(request_rect);

            //qDebug() << " panning south east";
        }
        // moved north and west
        else if (m_mapLatLonBound.right() < oldLatLonBound.right() && m_mapLatLonBound.top() > oldLatLonBound.top())
        {
            // get the top part
            request_rect = QRectF(m_mapLatLonBound.left(), m_mapLatLonBound.top(), m_mapLatLonBound.width(), -m_mapLatLonBound.top() + oldLatLonBound.top());
            emit RequestPointsInRange(request_rect);
            // get the left part
            request_rect = QRectF(m_mapLatLonBound.left(), oldLatLonBound.top(), oldLatLonBound.left() - m_mapLatLonBound.left(), -oldLatLonBound.top() + m_mapLatLonBound.bottom());
            emit RequestPointsInRange(request_rect);

            //qDebug() << " panning north west";
        }
        // moved south and west
        else if (m_mapLatLonBound.right() < oldLatLonBound.right() && m_mapLatLonBound.top() < oldLatLonBound.top())
        {
            // get the bottom part
            request_rect = QRectF(m_mapLatLonBound.left(), oldLatLonBound.bottom(), m_mapLatLonBound.width(), -m_mapLatLonBound.top() + m_mapLatLonBound.bottom());
            emit RequestPointsInRange(request_rect);
            // get the left part
            request_rect = QRectF(m_mapLatLonBound.left(), m_mapLatLonBound.top(), oldLatLonBound.left() - m_mapLatLonBound.left(), -m_mapLatLonBound.top() + m_mapLatLonBound.bottom());
            emit RequestPointsInRange(request_rect);

            //qDebug() << " panning south west";
        }
        // moved north
        else if (m_mapLatLonBound.top() > oldLatLonBound.top())
        {
            // get the bottom part
            request_rect = QRectF(m_mapLatLonBound.left(), m_mapLatLonBound.top(), m_mapLatLonBound.width(), -m_mapLatLonBound.top() + oldLatLonBound.top());
            emit RequestPointsInRange(request_rect);
        }
        // moved south
        else if (m_mapLatLonBound.top() < oldLatLonBound.top())
        {
            // get the bottom part
            request_rect = QRectF(m_mapLatLonBound.left(), oldLatLonBound.bottom(), m_mapLatLonBound.width(), -oldLatLonBound.bottom() + m_mapLatLonBound.bottom());
            emit RequestPointsInRange(request_rect);
        }
        // moved east
        else if (m_mapLatLonBound.right() > oldLatLonBound.right())
        {
            // get the right part
            request_rect = QRectF(oldLatLonBound.right(), m_mapLatLonBound.top(), m_mapLatLonBound.right() - oldLatLonBound.right(), m_mapLatLonBound.height());
            emit RequestPointsInRange(request_rect);
        }
        // moved west
        else if (m_mapLatLonBound.right() < oldLatLonBound.right())
        {
            // get the left part
            request_rect = QRectF(m_mapLatLonBound.left(), m_mapLatLonBound.top(), oldLatLonBound.left() - m_mapLatLonBound.left(), m_mapLatLonBound.height());
            emit RequestPointsInRange(request_rect);

        }
        else
        {

        }

    }
    else
    {
        // a zoom-in happened
        // this is triggered because if the database was in the process of sending points when the user zoomed-in
        // then that request would be cancelled. In that case, we would need to re-request those points.
        emit RequestPointsInRange(m_mapLatLonBound);
    }

//    end = clock();
//    if (end - start > 0)
//    qDebug() << "pan2 took " + QString::number(end - start);
//    start - clock();

    // move all the items in the view
    foreach(Point* i, this->Items)
    {
        i->setPos(i->pos().x()+delta.x(), i->pos().y()+delta.y());
    }

//    foreach(QGraphicsLineItem *l, this->Traces)
//    {
//        l->setPos(l->pos().x() + delta.x(), l->pos().y() + delta.y());
//    }

    if (!m_drawnTiles.isEmpty())
    {
        QHashIterator<QPoint, QGraphicsItem*> i (m_drawnTiles);
        while (i.hasNext())
        {
            i.next();
            QGraphicsItem* item = m_drawnTiles[i.key()];
            item->setPos(item->pos().x() + delta.x(), item->pos().y() + delta.y());
        }
    }
    if (!m_emptyTiles.isEmpty())
    {
        QHashIterator<QPoint, QGraphicsItem*> i (m_emptyTiles);
        while (i.hasNext())
        {
            i.next();
            QGraphicsItem* item = m_emptyTiles[i.key()];
            item->setPos(item->pos().x() + delta.x(), item->pos().y() + delta.y());
        }
    }
    foreach(Overlay* v, this->Overlays)
    {
        v->setSceneLocation(v->getSceneLocation().x() + delta.x(), v->getSceneLocation().y() + delta.y());
    }
    //qDebug() << "panning to: " + QString::number(latitude) + ", " + QString::number(longitude);

//    end = clock();
//    if (end - start > 0)
//    qDebug() << "pan3 took " + QString::number(end - start);
//    start - clock();

    // refresh -- add tiles that are now in the view that weren't before

    updateTileBounds();

    //QtConcurrent::run(this, &Map::download, m_tilesInView);
    download(m_tilesInView);

//    end = clock();
//    if (end - start > 0)
//    qDebug() << "pan4 took " + QString::number(end - start);
}

void Map::sceneRectChanged(const QRectF &rect)
{
    //qDebug() << "Inside sceneRectChanged.";

    // might want to check that the width and height are larger than the view
    resizeRect = rect; // to pass to the timer
    if (resizeTimer.isActive())
        resizeTimer.stop();
    else
        old_center = getViewCenterLatLon(); // only save the "old_center" the first time an event triggers
    resizeTimer.start(400, this);
}

// use a timer for resizing so things aren't recalculated a million times while
// the view changes
void Map::timerEvent(QTimerEvent *)
{
    //qDebug() << "inside timerEvent()";
    resizeTimer.stop();

    RemoveAllTiles();
    CenterOn(old_center); // CenterOn will call pan, which will update the lat/lon bound

    //qDebug() << QString::number(mView->sceneRect().left()) + " -- " + QString::number(mView->sceneRect().top()) + " -- " + QString::number(mView->sceneRect().width()) + " -- " + QString::number(mView->sceneRect().height());
}

QPointF Map::getMapOrigin()
{
    // the lat/long are actually the center of the map
    // need to adjust to get the upper left corner
    // of course, this varies based on the current lat/long

    // get center tile (with fractions)
    QPointF center = tileForCoordinate(latitude, longitude, zoom);

    // take the scene position and convert to tiles
    // lat/lon are in the center of the map
    qreal tile_offset_x = (static_cast<qreal>(mView->width())/static_cast<qreal>(2))/static_cast<qreal>(TILE_SIZE);
    qreal tile_offset_y = (static_cast<qreal>(mView->height())/static_cast<qreal>(2))/static_cast<qreal>(TILE_SIZE);

    // so now move over slightly and convert that partial tile coordinate
    // into a lat/lon
    return QPointF(longitudeFromTile(center.x() - tile_offset_x, zoom),
                   latitudeFromTile(center.y() - tile_offset_y, zoom));
}

void Map::keyPressEvent(QKeyEvent *e)
{
//    time_t start, end;
//    start = clock();
    // page up/down are tracked in the main UI to capture
    // zoom events and adjust the scroll bar accordingly
    if (e->key() == Qt::Key_Left)
        pan(QPoint(40, 0));
    if (e->key() == Qt::Key_Right)
        pan(QPoint(-40, 0));
    if (e->key() == Qt::Key_Up)
        pan(QPoint(0, 40));
    if (e->key() == Qt::Key_Down)
        pan(QPoint(0, -40));
    if (e->key() == Qt::Key_H)
        CenterOn(QPointF(Profile::getLongitude(), Profile::getLatitude()));
//    end = clock();
//    if (end - start > 0)
//        qDebug() << "keyPressEvent took " + QString::number(end - start);
}

QRectF Map::getLatLonBound()
{
    return m_mapLatLonBound;
}

// takes the current view of the map and converts that to a
// lat/lon bounding box
void Map::refreshLatLonBound()
{
    QPointF center_tile = getViewCenterLatLon();
    center_tile = tileForCoordinate(center_tile.y(), center_tile.x(), zoom);

    QPointF top_left_tile;
    top_left_tile.setX(center_tile.x() - (static_cast<qreal>(static_cast<qreal>(mView->width()) / static_cast<qreal>(2)) / static_cast<qreal>(TILE_SIZE)));
    top_left_tile.setY(center_tile.y() - (static_cast<qreal>(static_cast<qreal>(mView->height()) / static_cast<qreal>(2)) / static_cast<qreal>(TILE_SIZE)));

    QPointF bottom_right_tile;
    bottom_right_tile.setX(center_tile.x() + (static_cast<qreal>(static_cast<qreal>(mView->width()) / static_cast<qreal>(2)) / static_cast<qreal>(TILE_SIZE)));
    bottom_right_tile.setY(center_tile.y() + (static_cast<qreal>(static_cast<qreal>(mView->height()) / static_cast<qreal>(2)) / static_cast<qreal>(TILE_SIZE)));

    QPointF top_left_coord;
    top_left_coord.setY(latitudeFromTile(top_left_tile.y(), zoom));
    top_left_coord.setX(longitudeFromTile(top_left_tile.x(), zoom));

    QPointF bottom_right_coord;
    bottom_right_coord.setY(latitudeFromTile(bottom_right_tile.y(), zoom));
    bottom_right_coord.setX(longitudeFromTile(bottom_right_tile.x(), zoom));

    m_mapLatLonBound = QRectF(top_left_coord, bottom_right_coord);

    //Coordinate* c1 = new Coordinate(top_left_coord);
    //Coordinate* c2 = new Coordinate(bottom_right_coord);
    //qDebug() << "setting map boundaries to: " + c1->ToString() + " <-> " + c2->ToString();

}

QPointF Map::getViewCenterLatLon()
{
    //qDebug() << "map center is: " + QString::number(latitude, 'g', 10) + ", " + QString::number(longitude, 'g', 10);
    return QPointF(longitude, latitude);
}


void Map::ClearOldItems()
{
    QDateTime clearBefore = QDateTime::currentDateTime().addSecs(-1 * static_cast<int>(Profile::getClearTime() * 3600));
    //qDebug() << "getting points since " + clearBefore.toString("yyyy-MM-ddTHH:mm:ss");

//    for (int i=pointsToDeleteLater.count()-1; i>=0; i--)
//    {
//        Point *p = pointsToDeleteLater.at(i);
//        delete p;
//        pointsToDeleteLater.remove(i);
//    }

    foreach(Point* p, this->Items)
    {
        if(p->getTimestamp() <= clearBefore && p->getSource() != Profile::getCallSign())
        {
            RemoveItem(p);
        }
    }
}



//void Map::removeAllTraces()
//{
//    foreach(QGraphicsLineItem *l, this->Traces)
//    {
//        mView->scene()->removeItem(l);
//        Traces.remove(Traces.indexOf(l));
//        delete l;
//    }
//}


void Map::purgeNonVisiblePoints()
{
    try
    {
        foreach(Point* item, Items)
        {
            Coordinate c = Coordinate(item->getLocation());
            if (getLatLonBound().contains(c.GetLon(), c.GetLat()))
            {
                // don't do anything
            }
            else
            {
                RemoveItem(item);
            }
        }
    }
    catch(...)
    {
        ErrorHandler::AlertAndLog("Map::purgeNonVisiblePoints() -- exception removing items... ");
    }
}


void Map::purgeNonVisibleTiles()
{
    if (!m_tilePixmaps.isEmpty())
    {
        QHashIterator<QPoint, QPixmap> i (m_tilePixmaps);
        while (i.hasNext())
        {
            i.next();
            if (!m_tilesInView.contains(i.key()))
                m_tilePixmaps.remove(i.key());
        }
    }

    if (!m_drawnTiles.isEmpty())
    {
        QHashIterator<QPoint, QGraphicsItem*> j (m_drawnTiles);
        while (j.hasNext())
        {
            j.next();
            if (!m_tilesInView.contains(j.key()))
            {
                mView->scene()->removeItem(m_drawnTiles[j.key()]);
                QGraphicsItem* item = m_drawnTiles[j.key()];
                delete item;
                m_drawnTiles.remove(j.key());
            }
        }
    }
    if (!m_emptyTiles.isEmpty())
    {
        QHashIterator<QPoint, QGraphicsItem*> k (m_emptyTiles);
        while (k.hasNext())
        {
            k.next();
            if (!m_tilesInView.contains(k.key()))
            {
                mView->scene()->removeItem(m_emptyTiles[k.key()]);
                QGraphicsItem* item = m_emptyTiles[k.key()];
                delete item;
                m_emptyTiles.remove(k.key());
            }
        }
    }
}

void Map::download(QRect tilesInView)
{
//    time_t start, end;
//    start = clock();

    try
        {

        QPoint grab;
        for (int x = tilesInView.x(); x <= tilesInView.x() + tilesInView.width(); x++)
        {
            for (int y = tilesInView.y(); y <= tilesInView.y() + tilesInView.height(); y++)
            {
                QPoint tp = QPoint(x,y);
                if (!m_drawnTiles.contains(tp))
                {
                    if (!m_tilesToDownload.contains(tp) && !m_pendingTileDownloads.contains(tp))
                    {
                        //qDebug() << "adding to queue: " + QString::number(tp.x()) + ", " + QString::number(tp.y());
                        m_tilesToDownload.enqueue(tp);
                    }
                }
            }
        }

        while(!m_tilesToDownload.isEmpty())
        {
            grab = m_tilesToDownload.dequeue();
            m_pendingTileDownloads.enqueue(grab);

            // the map can operate in online or offline mode.
            // online mode will check the tile directory and then download
            // the tile if it is not found. Offline mode will never
            // download new tiles.

            //QString path = QCoreApplication::applicationDirPath() + "/tiles" + "/%1/%2/%3.png";
            QString path;
            QFile file;

            if (Profile::getOnlineMode())
            {
                path = Profile::getTileCacheDir() + "/tiles/%1/%2/%3.png";
                file.setFileName(path.arg(zoom).arg(grab.x()).arg(grab.y()));
                if(!file.exists())
                {
                    path = Profile::getTileServerUrl();
                }
            }
            else
            {
                path = Profile::getTileCacheDir() + "/tiles/%1/%2/%3.png";
            }

            m_url = QUrl(path.arg(zoom).arg(grab.x()).arg(grab.y()));

            //qDebug() << "downloading: " + m_url.toString();

            QNetworkRequest request;

            request.setUrl(m_url);
            request.setRawHeader("User-Agent", "aprs_mapper_v2");
            request.setAttribute(QNetworkRequest::User, QVariant(grab));
            request.setAttribute(QNetworkRequest::UserMax, QVariant(zoom)); // pass along the current zoom level...

            //qDebug() << "sending request " + QString::number(request.attribute(QNetworkRequest::User).toPoint().x()) + ", " + QString::number(request.attribute(QNetworkRequest::User).toPoint().y());
            m_manager.get(request);

            //qApp->processEvents();

        } // end while download queue is not empty

    //    end = clock();
    //    if (end - start > 0)
    //    qDebug() << "download took " + QString::number(end - start);
    }
    catch(...)
    {
        qDebug() << "error downloading tile.";
    }

}

//// when the network request is done, this function gets called
//// which adds the tile to the list of available images, and saves the
//// tile to disk

void Map::handleNetworkData(QNetworkReply *reply)
{
    QImage img;
    QPoint tp = reply->request().attribute(QNetworkRequest::User).toPoint();
    int request_zoom = reply->request().attribute(QNetworkRequest::UserMax).toInt();
    //QUrl url = reply->url();
    if (!reply->error())
    {
        if (request_zoom != zoom)
        {
            // ignore downloaded tiles that come from other zoom levels
            reply->deleteLater();
            //qDebug() << "ignoring invalid tile download event";
            return;
        }
        if (!img.load(reply, 0))
            img = QImage();
    }
    else
    {
        //qDebug() << "\n\n\n NETWORK DOWNLOAD ERROR\n\n\n";
    }
    reply->deleteLater();

    if (!m_pendingTileDownloads.isEmpty())
    {
        //qDebug() << "removing from queue: " + QString::number(tp.x()) + ", " + QString::number(tp.y());
        int i = m_pendingTileDownloads.indexOf(tp);
        m_pendingTileDownloads.removeAt(i);
    }

    if (img.isNull())
    {
        m_tilePixmaps[tp] = m_emptyTile;

        // redownload failures

        if (Profile::getOnlineMode())
        {
            QNetworkRequest request;
            request.setUrl(reply->url());
            request.setRawHeader("User-Agent", "Nokia (Qt) Graphics Dojo 1.0");
            request.setAttribute(QNetworkRequest::User, QVariant(reply->attribute(QNetworkRequest::User).toPoint()));
            request.setAttribute(QNetworkRequest::UserMax, QVariant(reply->attribute(QNetworkRequest::UserMax).toInt())); // pass along the current zoom level...

            //qDebug() << "redownloading request from handleNetworkData" + QString::number(request.attribute(QNetworkRequest::User).toPoint().x()) + ", " + QString::number(request.attribute(QNetworkRequest::User).toPoint().y());
            m_manager.get(request);
        }

    }
    else
    {
        if (m_tilePixmaps.contains(tp))
        {
            m_tilePixmaps.remove(tp);
        }
        m_tilePixmaps[tp] = QPixmap::fromImage(img);

    }

    QString path = Profile::getTileCacheDir() + "/tiles" + "/%1/%2/%3.png";
    path = path.arg(request_zoom).arg(tp.x()).arg(tp.y());
    if (!QFile::exists(path))
    {
        QDir working_dir;
        // check whether the tiles directory exists
        QString dir_path = Profile::getTileCacheDir() + "/tiles/";
        working_dir.setPath(dir_path);
        if (!working_dir.exists(dir_path))
        {
            working_dir.mkdir(dir_path);
        }

        // check whether the zoom directory exists
        dir_path = Profile::getTileCacheDir() + "/tiles" + "/%1/";
        dir_path = dir_path.arg(request_zoom);
        working_dir.setPath(dir_path);
        if (!working_dir.exists(dir_path))
        {
            working_dir.mkdir(dir_path);
        }

        // check whether the next directory exists
        dir_path = Profile::getTileCacheDir() + "/tiles" + "/%1/%2";
        dir_path = dir_path.arg(request_zoom).arg(tp.x());
        working_dir.setPath(dir_path);
        if (!working_dir.exists(dir_path))
        {
            working_dir.mkdir(dir_path);
        }

        // save the tile to path, as png, max compression
        //qDebug() << "tile path: " + path;
        /*bool result = */ img.save(path, "png", 0);
        //qDebug() << "tile saved: " + QString(result?"true":"false");
    }

    render(tp);
}


void Map::clearPendingDownloads()
{
    while(!m_pendingTileDownloads.isEmpty())
    {
        m_pendingTileDownloads.dequeue();
    }
    while(!m_tilesToDownload.isEmpty())
    {
        m_tilesToDownload.dequeue();
    }
}


void Map::zoomAllPoints()
{
    foreach(Point *p, this->Items)
    {
        p->setZoom(zoom);
    }
}


bool Map::doesPointPassFilters(Point *p)
{
    if (p->getSource() == Profile::getCallSign())
        return true;

    if (!whiteListType.contains(p->getPacketType()))
        return false;
    if (!whiteListSymbol.contains(p->getSymbolId()))
        return false;
    if (blackListType.contains(p->getPacketType()))
        return false;
    if (blackListSymbol.contains(p->getSymbolId()))
        return false;

    return true;
}

void Map::setBlackListSymbol(QVector<int> b)
{
    blackListSymbol = b;
}
void Map::setBlackListType(QVector<aprstype> b)
{
    blackListType = b;
}
void Map::setWhiteListSymbol(QVector<int> w)
{
    whiteListSymbol = w;
}
void Map::setWhiteListType(QVector<aprstype> w)
{
    whiteListType = w;
}

void Map::packetsListIsClosed()
{
    packetsListOpen = false;
}

void Map::packetsListIsOpen()
{
    packetsListOpen = true;
}
