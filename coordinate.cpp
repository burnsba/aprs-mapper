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

#include "coordinate.h"
#include "errorhandler.h"



uint qHash(const QPoint& p)
{
    return p.x() * 17 ^ p.y();
}

QPointF tileForCoordinate(qreal lat, qreal lng, int zoom)
{
    try
    {
//        while (lng > 180) lng -= 180;
//        while (lat > 90) lat -= 90;
//        while (lng < -180) lng += 180;
//        while (lat < -90) lat += 90;
        qreal zn = static_cast<qreal>(1 << zoom);
        qreal tx = (lng + 180.0) / 360.0;
        qreal ty = (1.0 - log(tan(lat * M_PI / 180.0) +
                              1.0 / cos(lat * M_PI / 180.0)) / M_PI) / 2.0;
        return QPointF(tx * zn, ty * zn);
    }
    catch(...)
    {
        ErrorHandler::Log("static tileForCoordinate(" + QString::number(lat) + ", " + QString::number(lng) + ", " + QString::number(zoom) + ") -- error?");
    }
}

qreal longitudeFromTile(qreal tx, int zoom)
{
    try
    {
        qreal zn = static_cast<qreal>(1 << zoom);
        qreal lat = tx / zn * 360.0 - 180.0;
        return lat;
    }
    catch(...)
    {
        ErrorHandler::Log("static longitudeFromTile(" + QString::number(tx) + ", " + QString::number(zoom) + ") -- error?");
    }
}

qreal latitudeFromTile(qreal ty, int zoom)
{
    try
    {
        qreal zn = static_cast<qreal>(1 << zoom);
        qreal n = M_PI - 2 * M_PI * ty / zn;
        qreal lng = 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
        return lng;
    }
    catch(...)
    {
        ErrorHandler::Log("static latitudeFromTile(" + QString::number(ty) + ", " + QString::number(zoom) + ") -- error?");
    }
}

// returns the distance in kilometers between two lat/long points
qreal distanceBetweenTwoPoints(QPointF p1, QPointF p2)
{
    if (p1.y() > 90 || p1.y() < -90)
        return NAN;
    if (p1.x() > 180 || p1.x() < -180)
        return NAN;
    if (p2.y() > 90 || p2.y() < -90)
        return NAN;
    if (p2.x() > 180 || p2.x() < -180)
        return NAN;

    qreal lat1 = p1.y() * M_PI / 180;
    qreal lon1 = p1.x() * M_PI / 180;
    qreal lat2 = p2.y() * M_PI / 180;
    qreal lon2 = p2.x() * M_PI / 180;

    return acos(sin(lat1) * sin(lat2) + cos(lat1) * cos(lat2) * cos(lon2 - lon1)) * 6371; // 6371 ~ radius of earth
}

// returns the bearing (0-360 degree) from source to dest
qreal bearingBetweenTwoPoints(QPointF dest, QPointF source)
{
    if (source.y() > 90 || source.y() < -90)
        return NAN;
    if (source.x() > 180 || source.x() < -180)
        return NAN;
    if (dest.y() > 90 || dest.y() < -90)
        return NAN;
    if (dest.x() > 180 || dest.x() < -180)
        return NAN;

    qreal lat1 = source.y() * M_PI / 180;
    qreal lon1 = source.x() * M_PI / 180;
    qreal lat2 = dest.y() * M_PI / 180;
    qreal lon2 = dest.x() * M_PI / 180;

    qreal y = sin(lon2 - lon1) * cos(lat2);
    qreal x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(lon2 - lon1);
    qreal res = (atan2(y,x) * 180 / M_PI) + 360;
    res = res > 360 ? res - 360 : res;
    return res;
}

QString bearingToCompassPoint(qreal bearing)
{
    if (bearing < 0 || bearing > 360)
        return "";
    if (bearing > 348.75 || bearing <= 11.25)
        return "N";
    else if (bearing > 11.25 && bearing <= 33.75)
        return "NNE";
    else if (bearing > 33.75 && bearing <= 56.25)
        return "NE";
    else if (bearing > 56.25 && bearing <= 78.75)
        return "ENE";
    else if (bearing > 78.75 && bearing <= 101.25)
        return "E";
    else if (bearing > 101.25 && bearing <= 123.75)
        return "ESE";
    else if (bearing > 123.75 && bearing <= 146.25)
        return "SE";
    else if (bearing > 146.25 && bearing <= 168.75)
        return "SSE";
    else if (bearing > 168.75 && bearing <= 191.25)
        return "S";
    else if (bearing > 191.25 && bearing <= 213.75)
        return "SSW";
    else if (bearing > 213.75 && bearing <= 236.25)
        return "SW";
    else if (bearing > 236.25 && bearing <= 258.75)
        return "WSW";
    else if (bearing > 258.75 && bearing <= 281.25)
        return "W";
    else if (bearing > 281.25 && bearing <= 303.75)
        return "WNW";
    else if (bearing > 303.75 && bearing <= 326.25)
        return "NW";
    else if (bearing > 326.25 && bearing <= 348.75)
        return "NNW";
    else
        return "";
}

Coordinate::Coordinate()
{
    latitudeDecimal = 0;
    longitudeDecimal = 0;
}

Coordinate::Coordinate(qreal latDecimal, qreal lonDecimal)
{
    if (latDecimal >= -90 && latDecimal <= 90)
        latitudeDecimal = latDecimal;
    else
        latitudeDecimal = 0;
    if (lonDecimal >= -180 && lonDecimal <= 180)
        longitudeDecimal = lonDecimal;
    else
        longitudeDecimal = 0;
}

Coordinate::Coordinate(QPointF lon_then_lat_as_x_y)
{
    if (lon_then_lat_as_x_y.y() >= -90 && lon_then_lat_as_x_y.y() <= 90)
        latitudeDecimal = lon_then_lat_as_x_y.y();
    else
        latitudeDecimal = 0;
    if (lon_then_lat_as_x_y.x() >= -180 && lon_then_lat_as_x_y.x() <= 180)
        longitudeDecimal = lon_then_lat_as_x_y.x();
    else
        longitudeDecimal = 0;
}

Coordinate::Coordinate(qreal latDeg, qreal latMin, qreal latSec, qreal lonDeg, qreal lonMin, qreal lonSec)
{
    try
    {
        QPointF result = Coordinate::ToDecimalLatLon(latDeg,latMin,latSec,lonDeg,lonMin,lonSec);
        latitudeDecimal = result.y();
        longitudeDecimal = result.x();
    }
    catch(...)
    {
        ErrorHandler::Log("Coordinate::Coordinate(" + QString::number(latDeg) + ", "  + QString::number(latMin) +
                          ", "  + QString::number(latSec) + ", "  + QString::number(lonDeg) + ", "  + QString::number(lonMin) +
                          ", "  + QString::number(lonSec) + ") -- error?");
    }
}

Coordinate::Coordinate(qreal locationPixelX, qreal locationPixelY, qreal sceneWidth, qreal sceneHeight)
{
    try
    {
        // takes a location on an image (say, mouse position) and saves it as lat/long
        // assume the width and height of the image are given and
        // that the image runs from -180 to +180 and -90 to +90

        // assume that the top left corner is 0,0
        // this corresponds to -180 longitude, +90 latitude
        // also, take into account scaling of image
        qreal temp_x = locationPixelX; // longitudeDecimal;
        qreal temp_y = locationPixelY; //latitudeDecimal;

        // scale pixels to degrees
        temp_x = temp_x * (360/sceneWidth); // 360 degrees in long.
        if (temp_x <= 0)
            temp_x = 1;
        if (temp_x > sceneWidth)
            temp_x = sceneWidth - 1;
        temp_y = temp_y * (180/sceneHeight); // 180 degrees in lat.
        if (temp_y <= 0)
            temp_y = 1;
        if (temp_y > sceneHeight)
            temp_y = sceneHeight - 1;

        // transform to correct lat/lon
        temp_x -= 180; // far left = 0 pixels; bring to -180 degrees
        temp_y -= 90; // upper edge = 0 pixels; bring to +90 degrees
        temp_y *= -1; // need to change the range from 0 thru -90 to 0 thru 90

        latitudeDecimal = temp_y;
        longitudeDecimal = temp_x;
    }
    catch(...)
    {
        latitudeDecimal = 0;
        longitudeDecimal = 0;
        ErrorHandler::Log("Coordinate::Coordinate(" + QString::number(locationPixelX) + ", "  + QString::number(locationPixelY) +
                          ", "  + QString::number(sceneWidth) + ", "  + QString::number(sceneHeight) + ") -- error?");
    }

}

Coordinate::Coordinate(QPointF sceneOriginLatLon, int zoom, qreal locationPixelX, qreal locationPixelY)
{
    try
    {
        QPointF scene_origin_tile = tileForCoordinate(sceneOriginLatLon.y(), sceneOriginLatLon.x(), zoom);

        qreal tile_offset_x = locationPixelX / TILE_SIZE;
        qreal tile_offset_y = locationPixelY / TILE_SIZE;

        latitudeDecimal = latitudeFromTile(scene_origin_tile.y() + tile_offset_y, zoom);
        longitudeDecimal = longitudeFromTile(scene_origin_tile.x() + tile_offset_x, zoom);
    }
    catch(...)
    {
        latitudeDecimal = 0;
        longitudeDecimal = 0;
        ErrorHandler::Log("Coordinate::Coordinate( QPointF(lat=" + QString::number(sceneOriginLatLon.y()) + ", lon="  +
                          QString::number(sceneOriginLatLon.x()) + "), " + QString::number(zoom) +
                          ", "  + QString::number(locationPixelX) + ", "  + QString::number(locationPixelY) + ") -- error?");
    }

}

QPointF Coordinate::ToDecimalLatLon()
{
    return QPointF(longitudeDecimal, latitudeDecimal);
}

QPointF Coordinate::ToDecimalLatLon(qreal latDeg, qreal latMin, qreal latSec, qreal lonDeg, qreal lonMin, qreal lonSec)
{
    qreal temp_lat = latSec >= 0 ? latSec : latSec*(-1);
    qreal temp_lon = lonSec >= 0 ? lonSec : lonSec*(-1);
    temp_lat /= 60; // convert to minutes
    temp_lon /= 60;
    temp_lat += latMin >= 0 ? latMin : latMin*(-1);
    temp_lon += lonMin >= 0 ? lonMin : lonMin*(-1);
    temp_lat /= 60; // convert to degrees
    temp_lon /= 60;
    // if degrees are negative, it should stay that way, but make sure to
    // correctly add the fractional part
    temp_lat = latDeg >= 0 ? temp_lat + latDeg : latDeg - temp_lat;
    temp_lon = lonDeg >= 0 ? temp_lon + lonDeg : lonDeg - temp_lon;

    QPointF result = QPointF(temp_lon, temp_lat);
    return result;
}

//QPointF* Coordinate::ToSceneCoord(qreal width, qreal height)
//{
//    // assume that the top left corner is 0,0
//    // this corresponds to -180 longitude, +90 latitude
//    // also, take into account scaling of image
//    qreal temp_x = longitudeDecimal;
//    qreal temp_y = latitudeDecimal;

//    // translate from lat/lon to pixel coords
//    temp_x += 180; // far left = -180; bring to zero
//    temp_y -= 90; // upper edge = +90, bring to zero
//    temp_y *= -1; // need to change the range from 0 thru -90 to 0 thru 90

//    // scale coords to image size
//    temp_x = temp_x * (width/360); // 360 degrees in long.
//    if (temp_x <= 0)
//        temp_x = 1;
//    if (temp_x > width)
//        temp_x = width - 1;
//    temp_y = temp_y * (height/180); // 180 degrees in lat.
//    if (temp_y <= 0)
//        temp_y = 1;
//    if (temp_y > height)
//        temp_y = height - 1;

//    QPointF* result = new QPointF(temp_x,temp_y);
//    return result;

//}

//QPointF* Coordinate::ToSceneCoord(qreal width, qreal height, qreal xOffsetToZero, qreal yOffsetToZero)
//{
//    // assume that the top left corner is 0,0
//    // this corresponds to -180 longitude, +90 latitude
//    // also, take into account scaling of image
//    qreal temp_x = longitudeDecimal;
//    qreal temp_y = latitudeDecimal;

//    // translate from lat/lon to pixel coords
//    temp_x += 180; // far left = -180; bring to zero
//    temp_y -= 90; // upper edge = +90, bring to zero
//    temp_y *= -1; // need to change the range from 0 thru -90 to 0 thru 90

//    // scale coords to image size
//    temp_x = temp_x * (width/360); // 360 degrees in long.
//    if (temp_x <= 0)
//        temp_x = 1;
//    if (temp_x > width)
//        temp_x = width - 1;
//    temp_x += (width - xOffsetToZero); // adjust for offset
//    if (temp_x > width)
//        temp_x -= width;
//    temp_y = temp_y * (height/180); // 180 degrees in lat.
//    if (temp_y <= 0)
//        temp_y = 1;
//    if (temp_y > height)
//        temp_y = height - 1;
//    temp_y += (height - yOffsetToZero); // adjust for offset
//    if (temp_y > height)
//        temp_y -= height;

//    QPointF* result = new QPointF(temp_x,temp_y);
//    return result;

//}

// takes the coordinates internal lat/lon and calculates a pixel
// offset based on the upperleft corner (in lat/lon) of some item
QPointF Coordinate::ToSceneCoord(QPointF sceneOriginLatLon, int zoom)
{
    QPointF this_tile = tileForCoordinate(latitudeDecimal, longitudeDecimal, zoom);
    qreal this_tile_x = this_tile.x();
    qreal this_tile_y = this_tile.y();

    QPointF scene_origin_tile = tileForCoordinate(sceneOriginLatLon.y(), sceneOriginLatLon.x(), zoom);
    qreal tile_difference_x = this_tile_x - scene_origin_tile.x();
    qreal tile_difference_y = this_tile_y - scene_origin_tile.y();

    QPointF result = QPointF(tile_difference_x * TILE_SIZE, tile_difference_y * TILE_SIZE);

    return result;
}

QString Coordinate::ToString()
{
    QString result;
    if (latitudeDecimal >= 0)
    {
        result += QString::number(latitudeDecimal,'g',8);
        result += "N";
    }
    else
    {
        qreal temp = latitudeDecimal * -1;
        result += QString::number(temp,'g',8);
        result += "S";
    }
    result += ",";
    if (longitudeDecimal >= 0)
    {
        result += QString::number(longitudeDecimal,'g',8);
        result += "E";
    }
    else
    {
        qreal temp = longitudeDecimal * -1;
        result += QString::number(temp,'g',8);
        result += "W";
    }
    return result;
}

qreal Coordinate::GetLat()
{
    return latitudeDecimal;
}

qreal Coordinate::GetLon()
{
    return longitudeDecimal;
}
