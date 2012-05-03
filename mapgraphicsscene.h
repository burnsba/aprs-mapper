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

#ifndef MAPGRAPHICSSCENE_H
#define MAPGRAPHICSSCENE_H

#include <QGraphicsScene>

// this class is used to capture events for the map

class MapGraphicsScene : public QGraphicsScene
{
    Q_OBJECT
public:
    MapGraphicsScene(QGraphicsView* view);

signals:

    void mgsMouseEvent(QGraphicsSceneMouseEvent *event);
    void mgsDoubleClickEvent(QGraphicsSceneMouseEvent *event);

    void mgsPressEvent(QGraphicsSceneMouseEvent *event);
    void mgsReleaseEvent(QGraphicsSceneMouseEvent *event);

    void mgsMouseWheelEvent(QGraphicsSceneWheelEvent *event);

    void mgsKeyPressEvent(QKeyEvent *event);

    //void mgsSceneRectChanged(const QRectF &rect);

public slots:

protected:

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    void wheelEvent(QGraphicsSceneWheelEvent *event);

    void keyPressEvent(QKeyEvent *event);

    //void sceneRectChanged(const QRectF &rect);

};

#endif // MAPGRAPHICSSCENE_H
