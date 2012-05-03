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

#ifndef POINT_H
#define POINT_H

#include <QGraphicsPixmapItem> /* for QPointF */
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsItem>

#include "profile.h"
#include "aprs_parser.h"

bool overlayAllowed(int id);
QPolygonF enclosePath(QPointF lineStart, QPointF lineEnd);

class Point : public QGraphicsItem
{

public:

    Point();
    Point(QString callSign);
    Point(QString callSign, QPointF point);
    ~Point();

    // setters
    void setLocation(qreal Latitude, qreal Longitude);
    void setSource(QString s);
    void setTimestamp(QDateTime timestamp) { m_timeStamp = timestamp; }
    void setOverlay(QString s);
    void setSymbolId(qint32 id);
    void setPacketType(aprstype a) { m_packetType = a; }
    void setZoom(int z) { m_zoom = z; }
    void setData(QString s) { m_data = s; }

    // getters
    QPointF getLocation()       { return m_point; }
    QString getSource()         { return m_source; }
    QDateTime getTimestamp()    { return m_timeStamp; }
    QString getOverlay()        { return m_overlay; }
    qint32 getSymbolId()        { return m_symbolId; }
    aprstype getPacketType()    { return m_packetType; }
    int getZoom()               { return m_zoom; }
    QString getData()           { return m_data; }

    // other stuff
    bool m_currentlyActive;
    QRectF getBounds() { return m_bounds; }

    QMap<QDateTime, QPointF> m_traces;
    bool addTrace(QDateTime, QPointF);
    void clearTraceBefore(QDateTime);
    void preDestruct();

private:
    QPointF m_point;           // location
    aprstype m_packetType;      // from the parser (objecct, etc)
    QString m_source;           // callsign
    QString m_overlay;          // overlay character
    qint32 m_symbolId;          // icon id
    QDateTime m_timeStamp;      // received time
    QPixmap* m_iconImage;       // the icon shown
    QFont m_sourceFont;         // default font for callsign text
    QRectF m_bounds;            // bounding box, used by graphicsview thingers
    QRectF m_tBounds;           // bounding box for callsign stuff
    int m_zoom;                 // zoom level, for figure out lat/lon to pixel transforms
    QString m_data;             // ...

    QColor m_traceColor;
    QColor m_traceHoverColor;

    void updateBounds();

protected:
    virtual QRectF boundingRect() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QPainterPath shape() const;

};

#endif // POINT_H
