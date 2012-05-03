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

#include "mapgraphicsscene.h"
#include "errorhandler.h"

MapGraphicsScene::MapGraphicsScene(QGraphicsView* view)
{
    // nothing to do.
    view = view;
}

void MapGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    emit mgsMouseEvent(event);
}

void MapGraphicsScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    emit mgsDoubleClickEvent(event);
}

void MapGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    emit mgsPressEvent(event);
}

void MapGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    emit mgsReleaseEvent(event);
}

void MapGraphicsScene::keyPressEvent(QKeyEvent *event)
{
    emit mgsKeyPressEvent(event);
}

void MapGraphicsScene::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    emit mgsMouseWheelEvent(event);
}

//void MapGraphicsScene::sceneRectChanged(const QRectF &rect)
//{
//    emit mgsSceneRectChanged(rect);
//}
