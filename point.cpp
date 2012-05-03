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

#include <QDebug>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QPen>
#include <QImage>
#include "profile.h"
#include "point.h"
#include "errorhandler.h"
#include "coordinate.h"

#define SOURCE_TEXT_PADDING 6
#define SOURCE_ICON_GAP 5
#define ICON_SIZE 20
#define ICON_HALF_SIZE 10
#define ROUNDED_RADIUS 2
#define ROUNDED_HALF_RADIUS 1

#define GRAB_BUFFER 3


// checks if an overlay is allowed
bool overlayAllowed(int id)
{
    if (id == 98 || id == 101 || id == 111 || id == 125 || id == 128 || id == 150 ||
            id == 157 || id == 158 || id == 160 || id == 162 || id == 168 ||
            id == 173 || id == 178 || id == 180 || id == 181)
        return true;
    return false;
}

Point::Point()
{
    int col1 = rand() % 180;
    int col2 = rand() % 180;
    int col3 = rand() % 180;
    m_traceColor = QColor::fromRgb(col1,col2,col3);
    m_traceHoverColor = QColor::fromRgb(col1+60,col2+60,col3+60);

    m_packetType = T_UNKNOWN;
    m_timeStamp = QDateTime();
    m_point = QPointF(0,0);
    m_bounds = QRectF(0,0,0,0);
    m_sourceFont = QFont("Times New Roman", 14, 1, false);
    m_iconImage = new QPixmap(ICON_SIZE, ICON_SIZE);
    setSource("Callsign-0");
    setSymbolId(0);
    m_currentlyActive = false;

    //this->setCacheMode(); <- may change performance.
    //this->setCacheMode(QGraphicsItem::ItemCoordinateCache);

}

Point::Point(QString callSign)
{
    int col1 = rand() % 180;
    int col2 = rand() % 180;
    int col3 = rand() % 180;
    m_traceColor = QColor::fromRgb(col1,col2,col3);
    m_traceHoverColor = QColor::fromRgb(col1+60,col2+60,col3+60);

    m_packetType = T_UNKNOWN;
    m_timeStamp = QDateTime();
    m_point = QPointF(0,0);
    m_bounds = QRectF(0,0,0,0);
    m_sourceFont = QFont("Times New Roman", 14, 1, false);
    m_iconImage = new QPixmap(ICON_SIZE, ICON_SIZE);
    setSymbolId(0);
    if (callSign != "")
        setSource(callSign);
    else
    {
        ErrorHandler::Log("ERROR: empty callsign");
        setSource("ERROR");
    }
    m_currentlyActive = false;

    //this->setCacheMode(QGraphicsItem::ItemCoordinateCache);

}

Point::Point(QString callSign, QPointF point)
{
    int col1 = rand() % 180;
    int col2 = rand() % 180;
    int col3 = rand() % 180;
    m_traceColor = QColor::fromRgb(col1,col2,col3);
    m_traceHoverColor = QColor::fromRgb(col1+60,col2+60,col3+60);

    m_packetType = T_UNKNOWN;
    m_timeStamp = QDateTime();
    m_point = QPointF(0,0);
    m_bounds = QRectF(0,0,0,0);
    m_sourceFont = QFont("Times New Roman", 14, 1, false);
    m_iconImage = new QPixmap(ICON_SIZE, ICON_SIZE);
    setSymbolId(0);
    if (callSign != "")
        setSource(callSign);
    else
    {
        ErrorHandler::Log("ERROR: empty callsign");
        setSource("ERROR");
    }
    m_currentlyActive = false;

    setLocation(point.y(), point.x());

    //this->setCacheMode(QGraphicsItem::ItemCoordinateCache);

}

Point::~Point()
{
    delete m_iconImage;
}

void Point::setLocation(qreal Latitude, qreal Longitude)
{
    try
    {
        if (qAbs(Longitude) < 180)
            m_point.setX(Longitude);
        if (qAbs(Latitude < 90))
            m_point.setY(Latitude);
    }
    catch(...)
    {

    }
}

// changes the icon shown.
void Point::setSymbolId(qint32 id)
{
    if (id > 192 || id < 0)
    {
        id = 0;
        ErrorHandler::Log("received invalid symbold id.");
    }
    try
        {
        // all of the icons are in one .png file, in rows of 16 icons
        int h_start = 0;
        int v_start = 0;
        int side_length = ICON_SIZE;
        int border = 1;

        int start_col = id % 16; // 16 columns per row
        int start_row = id / 16;

        h_start = ((side_length * start_col) + (border * (start_col + 1)));
        v_start = ((side_length * start_row) + (border * (start_row + 1)));

        m_symbolId = id;
        delete m_iconImage;
        m_iconImage = new QPixmap(ICON_SIZE, ICON_SIZE); // empty space for working

        QImage icons = QImage(":/icons/aprs_icons.png", 0); // going to use this file
        m_iconImage->fill(Qt::transparent);    // make sure the background is empty
        QPainter painter(m_iconImage); // set a painter
        painter.setBackgroundMode(Qt::TransparentMode); // make double sure the background is empty
        // so now grab the correct icon from the big file
        painter.drawImage(0, 0, icons, h_start, v_start, side_length, side_length);
    }
    catch(...)
    {

    }

}

void Point::setSource(QString source)
{

    foreach (QChar c, source)
    {
        if (!c.isPrint())
            return;
    }
    prepareGeometryChange();
    m_source = source;
    updateBounds();
    if (m_bounds.width() < 1 || m_bounds.height() < 1)
        qDebug() << "crazy bounding box.";
}

void Point::setOverlay(QString s)
{
    foreach (QChar c, s)
    {
        if (!c.isPrint())
            return;
    }
    m_overlay = s;
}

// this is a virtual function that desides collision detection and things
QPainterPath Point::shape() const
{
    QPainterPath path;
    QFontMetrics fm(m_sourceFont);
    QRect temp = fm.boundingRect(m_source);
    QRect text_area = QRect(m_bounds.left() + ROUNDED_HALF_RADIUS,
                            m_bounds.top() + ROUNDED_HALF_RADIUS,
                            m_bounds.width() - ROUNDED_RADIUS,
                            temp.height() + 2 - ROUNDED_RADIUS);

    if (m_traces.count() > 0)
    {
        QPointF lastTilePos;
        QPointF nextTilePos = tileForCoordinate(m_point.y(), m_point.x(), m_zoom);
        QPointF lastPixelPos = QPointF(0,0);
        QPointF nextPixelPos;

        path.moveTo(0,0);
        QMapIterator<QDateTime, QPointF> i(m_traces);
        i.toBack();
        while (i.hasPrevious()) {
            i.previous();
            lastTilePos = tileForCoordinate(i.value().y(), i.value().x(), m_zoom);
            nextPixelPos.setX((nextTilePos.x() - lastTilePos.x())*TILE_SIZE);
            nextPixelPos.setY((nextTilePos.y() - lastTilePos.y())*TILE_SIZE);
            nextPixelPos = lastPixelPos - nextPixelPos;

            path.addPolygon(enclosePath(lastPixelPos, nextPixelPos));
            path.closeSubpath();

            lastPixelPos = nextPixelPos;
            nextTilePos = lastTilePos;
        }
    }

    path.addRect(-ICON_HALF_SIZE, -ICON_HALF_SIZE, ICON_SIZE, ICON_SIZE);
    path.closeSubpath();
    path.moveTo(0,0);
    path.addRect(text_area);
    path.closeSubpath();

    return path;
}

QRectF Point::boundingRect() const
{
    return m_bounds;
}

// call prepareGeometryChange before calling this function! (and before changing
// anything used here such as m_traces, m_source, etc)
void Point::updateBounds()
{
    QRect temp;
    QFontMetrics fm(m_sourceFont);
    temp = fm.boundingRect(m_source);
    m_tBounds = QRectF(-(temp.width()/2)-(SOURCE_TEXT_PADDING) - ROUNDED_HALF_RADIUS - 1,
                        -temp.height() - 1 - ICON_HALF_SIZE - ROUNDED_HALF_RADIUS - 1,
                        temp.width()+(SOURCE_TEXT_PADDING*2) + ROUNDED_RADIUS + 2,
                        temp.height()+ICON_SIZE+SOURCE_ICON_GAP + ROUNDED_RADIUS + 2);

    if (m_traces.count() > 0)
    {
        QPointF lastTilePos;
        QPointF nextTilePos = tileForCoordinate(m_point.y(), m_point.x(), m_zoom);
        QPointF lastPixelPos = QPointF(0,0);
        QPointF nextPixelPos;

        qreal minx = 1000;
        qreal maxx = 0;
        qreal miny = 1000;
        qreal maxy = 0;

        //path.moveTo(0,0);

        QMapIterator<QDateTime, QPointF> i(m_traces);
        //int j = 0;
        i.toBack();
        while (i.hasPrevious()) {
            i.previous();
            lastTilePos = tileForCoordinate(i.value().y(), i.value().x(), m_zoom);
            nextPixelPos.setX((nextTilePos.x() - lastTilePos.x())*TILE_SIZE);
            nextPixelPos.setY((nextTilePos.y() - lastTilePos.y())*TILE_SIZE);
            nextPixelPos = lastPixelPos - nextPixelPos;

            minx = qMin(minx, nextPixelPos.x());
            miny = qMin(miny, nextPixelPos.y());
            maxx = qMax(maxx, nextPixelPos.x());
            maxy = qMax(maxy, nextPixelPos.y());

            lastPixelPos = nextPixelPos;
            nextTilePos = lastTilePos;
        }

        m_bounds = QRectF(QPointF(qMin(minx, m_bounds.left()) - 1, qMin(miny, m_bounds.top()) - 1),
                          QPointF(qMax(maxx, m_bounds.right()) + 2, qMax(maxy, m_bounds.bottom()) + 2));
    }
    else
    {
        m_bounds = m_tBounds;
    }
}

void Point::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPen drawingPen;
    QBrush background;

    // don't repaint anything that's hidden
    painter->setClipRect(option->exposedRect);

    // draw traces (on the bottom)
    if (m_traces.count() > 0)
    {
        if (m_currentlyActive == true)
        {
            drawingPen.setColor(m_traceHoverColor);
            drawingPen.setWidth(6);
        }
        else
        {
            drawingPen.setColor(m_traceColor);
            drawingPen.setWidth(4);
        }

        background = QBrush(m_traceColor, Qt::SolidPattern);
        painter->setBrush(background);
        painter->setPen(drawingPen);

        QPointF lastTilePos;
        QPointF nextTilePos = tileForCoordinate(m_point.y(), m_point.x(), m_zoom);
        QPointF lastPixelPos = QPointF(0,0);
        QPointF nextPixelPos;

        QMapIterator<QDateTime, QPointF> i(m_traces);
        i.toBack();
        while (i.hasPrevious()) {
            i.previous();
            lastTilePos = tileForCoordinate(i.value().y(), i.value().x(), m_zoom);
            nextPixelPos.setX((nextTilePos.x() - lastTilePos.x())*TILE_SIZE);
            nextPixelPos.setY((nextTilePos.y() - lastTilePos.y())*TILE_SIZE);
            nextPixelPos = lastPixelPos - nextPixelPos;
            painter->drawLine(lastPixelPos, nextPixelPos);
            //painter->drawPolygon(enclosePath(lastPixelPos, nextPixelPos));

            lastPixelPos = nextPixelPos;
            nextTilePos = lastTilePos;
        }

        if (m_currentlyActive == true)
        {
            background = QBrush(m_traceColor, Qt::NoBrush);
            painter->setBrush(background);
            drawingPen.setColor(QColor::fromRgb(0,0,0));
            drawingPen.setWidth(2);
            drawingPen.setStyle(Qt::DashLine);
            painter->setPen(drawingPen);

            // do some dead reckoning stuff
            i.toBack();
            do {
                i.previous();
            }
            while (i.hasPrevious() && i.value() == m_point);
            if (i.value() == i.value() && i.value() != m_point)
            {
                // get the most recent position, before current
                lastTilePos = tileForCoordinate(i.value().y(), i.value().x(), m_zoom);
                // get the current position
                nextTilePos = tileForCoordinate(m_point.y(), m_point.x(), m_zoom);
                // so now, estimate the location of the next position. This is the central location estimate
                nextPixelPos.setX((nextTilePos.x() - lastTilePos.x())*TILE_SIZE);
                nextPixelPos.setY((nextTilePos.y() - lastTilePos.y())*TILE_SIZE);
                // now get the radius for this stuff (half the distance to the next point)
                qreal rad = sqrt(pow(nextPixelPos.x(),2) + pow(nextPixelPos.y(), 2)) / 2;
                // get the slope of the intercepting line
                qreal slope = 1;
                if (nextPixelPos.y() != 0)
                     slope = (nextPixelPos.x() / nextPixelPos.y()) * -1;
                // find y intercept b ( y = mx + b)
                qreal b = nextPixelPos.y() - (slope * nextPixelPos.x());

                QPointF p1;
                QPointF p2;

                if (nextPixelPos.x() != 0)
                {
                    qreal theta_1 = atan2(nextPixelPos.y(), nextPixelPos.x());
                    p1.setX(nextPixelPos.x() + (sin(theta_1) * rad));
                    p1.setY(nextPixelPos.y() - (cos(theta_1) * rad));

                    p2.setX(nextPixelPos.x() - (sin(theta_1) * rad));
                    p2.setY(nextPixelPos.y() + (cos(theta_1) * rad));
                }
                else
                {
                    p1.setX(nextPixelPos.x() - rad);
                    p1.setY(nextPixelPos.y());

                    p2.setX(nextPixelPos.x() + rad);
                    p2.setY(nextPixelPos.y());
                }



                // now draw a circle around the estimate, and two lines to the edge of the circle from the origin
                painter->drawEllipse(nextPixelPos, rad, rad);
                painter->drawLine(QPointF(0,0), p1);
                painter->drawLine(p1,p2);
                painter->drawLine(p2, QPointF(0,0));
            }
        }
    }

    if (m_currentlyActive == false)
        drawingPen.setColor(Qt::black);
    else
        drawingPen.setColor(Qt::darkGreen);
    drawingPen.setWidth(1);
    drawingPen.setStyle(Qt::SolidLine);
    painter->setPen(drawingPen);

    if (m_currentlyActive == false)
        background = QBrush(QColor::fromRgb(230,230,230,255), Qt::SolidPattern);
    else
        background = QBrush(QColor::fromRgb(255,255,255,255), Qt::SolidPattern);
    painter->setBackgroundMode(Qt::OpaqueMode);
    painter->setBrush(background);

    // figure out what font is being used
    painter->setFont(m_sourceFont);

    // get the area for the background of the text
    QRectF rect = QRectF(m_tBounds.left()+ROUNDED_RADIUS, m_tBounds.top()+ROUNDED_RADIUS, m_tBounds.width()-(ROUNDED_RADIUS*2), m_tBounds.height()-(ICON_SIZE+SOURCE_ICON_GAP)-(ROUNDED_RADIUS*2));
    // draw the background for the text
    painter->drawRoundedRect(rect, ROUNDED_RADIUS, ROUNDED_RADIUS);

    // get the area for the actual text
    rect = QRectF(m_tBounds.left()+SOURCE_TEXT_PADDING, m_tBounds.top(), m_tBounds.width(), m_tBounds.height()-(ICON_SIZE+SOURCE_ICON_GAP));
    // draw the text of the callsign/object
    painter->setBackgroundMode(Qt::TransparentMode);
    painter->drawText(rect, m_source);

    // draw the icon (centered at 0,0)
    painter->drawPixmap(-ICON_HALF_SIZE, -ICON_HALF_SIZE, *m_iconImage);

    // draw the overlay character
    if (m_overlay != "")
    {
        QPainterPath path;
        path.addText(-5, 5, QFont("Courier New", 12, -1), m_overlay);

        // draw an outline for the overlay
        drawingPen.setColor(Qt::white);
        drawingPen.setWidth(3);
        painter->setPen(drawingPen);
        painter->drawPath(path);

        // draw the inside of the overlay
        drawingPen.setColor(Qt::black);
        drawingPen.setWidth(1);
        painter->setPen(drawingPen);
        painter->drawPath(path);

    }
}

// if the location/time already exists, this will return false
bool Point::addTrace(QDateTime d, QPointF p)
{
    if (!m_traces.contains(d))
    {
        prepareGeometryChange();
        m_traces.insert(d, p);
        updateBounds();
        return true;
    }
    return false;
}

void Point::clearTraceBefore(QDateTime d)
{
    prepareGeometryChange();
    QMapIterator<QDateTime, QPointF> i(m_traces);
    while (i.hasNext()) {
        i.next();
        if (i.key() < d)
            m_traces.remove(i.key());
    }
}



QPolygonF enclosePath(QPointF lineStart, QPointF lineEnd)
{

    //http://davidwdrell.net/graphicslesson4.htm

    /* to draw the select region around the line,
        define a large box(rect p1,p2,p3,p4) around each end of the line.
        then define a box(rect p5,p6,p7,p8) that connects the two end boxes.
        now no matter the angle of the line, we can be sure that its
        surrounded by a symetrical region that will pick up the mouse.


        .    .             .    .
           X-----------------X
        .    .             .    .

        the squares have to connect at the two pairs of square corners that are closest.

       */

    QList<QPointF> pointsStart;
    QList<QPointF> pointsEnd;

    pointsStart.append( QPointF(lineStart.x()  - GRAB_BUFFER, lineStart.y() - GRAB_BUFFER));
    pointsStart.append(  QPointF(lineStart.x() + GRAB_BUFFER, lineStart.y() - GRAB_BUFFER));
    pointsStart.append(  QPointF(lineStart.x() + GRAB_BUFFER, lineStart.y() + GRAB_BUFFER));
    pointsStart.append(  QPointF(lineStart.x() - GRAB_BUFFER, lineStart.y() + GRAB_BUFFER));

    pointsEnd.append(  QPointF(lineEnd.x() - GRAB_BUFFER, lineEnd.y() - GRAB_BUFFER));
    pointsEnd.append(  QPointF(lineEnd.x() + GRAB_BUFFER, lineEnd.y() - GRAB_BUFFER));
    pointsEnd.append(  QPointF(lineEnd.x() + GRAB_BUFFER, lineEnd.y() + GRAB_BUFFER));
    pointsEnd.append(  QPointF(lineEnd.x() - GRAB_BUFFER, lineEnd.y() + GRAB_BUFFER));

    qreal minDistance = 0 ;
    qreal secondMinDistance = 0;
    QPointF p1 ;
    QPointF p2 ;
    QPointF p3 ;
    QPointF p4 ;
    int i1=0;
    int i2=0;

    for (int i =0 ; i < 4; i++)
    {
        for (int j = 0; j < 4 ; j++)
        {
            qreal d1 = pow(pointsStart[i].x() - pointsEnd[j].x(), 2.0)
                       + pow( pointsStart[i].y() - pointsEnd[j].y(), 2.0);
            if ( d1 > minDistance)
            {
                minDistance = d1;
                p1 = pointsStart[i];
                p2 = pointsEnd[j];
                i1 = i;
                i2 = j;
            }
        }
    }



    for (int i =0 ; i < 4; i++)
    {
        if ( i == i1) continue;

        for (int j = 0; j < 4 ; j++)
        {
            if ( j == i2 ) continue;

            qreal d1 = pow(pointsStart[i].x() - pointsEnd[j].x(), 2.0)
                       + pow( pointsStart[i].y() - pointsEnd[j].y(), 2.0);
            if ( d1 > secondMinDistance)
            {
                secondMinDistance = d1;
                p3 = pointsStart[i];
                p4 = pointsEnd[j];
            }
        }
    }

    //_selectRegion << mapFromScene(p1) << mapFromScene(p3)  << mapFromScene(p4) << mapFromScene(p2) << mapFromScene(p1) ;

    QPolygonF res;
    res << p1 << p3 << p4 << p2 << p1;
    return res;
}


void Point::preDestruct()
{
//    prepareGeometryChange();
//    m_bounds = QRectF(0,0,0,0);
//    m_traces.empty();
//    this->setVisible(false);
}
