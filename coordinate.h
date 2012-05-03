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

#ifndef COORDINATE_H
#define COORDINATE_H

#include <QGraphicsPixmapItem>
#include "math.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


// tile size in pixels
#define TILE_SIZE 256

uint qHash(const QPoint& p);
QPointF tileForCoordinate(qreal lat, qreal lng, int zoom);
qreal longitudeFromTile(qreal tx, int zoom);
qreal latitudeFromTile(qreal ty, int zoom);

qreal distanceBetweenTwoPoints(QPointF p1, QPointF p2);
qreal bearingBetweenTwoPoints(QPointF source, QPointF dest);
QString bearingToCompassPoint(qreal bearing);

class Coordinate
{
public:
    Coordinate();
    Coordinate(qreal latDecimal, qreal lonDecimal);
    Coordinate(QPointF lon_then_lat_as_x_y);
    Coordinate(qreal latDeg, qreal latMin, qreal latSec,
               qreal lonDeg, qreal lonMin, qreal lonSec);
    Coordinate(qreal locationPixelX, qreal locationPixelY, qreal sceneWidth, qreal sceneHeight);
    Coordinate(QPointF sceneOriginLatLon, int zoom, qreal locationPixelX, qreal locationPixelY);

    QPointF ToDecimalLatLon();
    QPointF ToDecimalLatLon(qreal latDeg, qreal latMin, qreal latSec,
                             qreal lonDeg, qreal lonMin, qreal lonSec);
    //QPointF* ToSceneCoord(qreal width, qreal height);
    //QPointF* ToSceneCoord(qreal width, qreal height, qreal xOffsetToZero, qreal yOffsetToZero);
    QPointF ToSceneCoord(QPointF sceneOriginLatLon, int zoom); // returns pixel offset between origin lat/lon and this->lat/lon

    QString ToString();

    qreal GetLat();
    qreal GetLon();

private:
    qreal latitudeDecimal;
    qreal longitudeDecimal;



};

#endif // COORDINATE_H
