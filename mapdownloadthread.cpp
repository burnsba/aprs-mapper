#include "mapdownloadthread.h"


MapDownloadThread::MapDownloadThread()
{
//    m_parent = parent;

//    // setup the downloader thinger
//    connect(&m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(handleNetworkData(QNetworkReply*)), Qt::QueuedConnection);
}


//void MapDownloadThread::Start()
//{
//    //this->start(QThread::NormalPriority);
//}



//void MapDownloadThread::clearPendingDownloads()
//{
//    while(!m_pendingTileDownloads.isEmpty())
//    {
//        m_pendingTileDownloads.dequeue();
//    }
//    while(!m_tilesToDownload.isEmpty())
//    {
//        m_tilesToDownload.dequeue();
//    }
//}


////void MapDownloadThread::purgeNonVisibleTiles()
////{

////    if (!m_parent->m_tilePixmaps.isEmpty())
////    {
////        QHashIterator<QPoint, QPixmap> i (m_parent->m_tilePixmaps);
////        while (i.hasNext())
////        {
////            i.next();
////            if (!m_parent->m_tilesInView.contains(i.key()))
////                m_parent->m_tilePixmaps.remove(i.key());
////        }
////    }

////    if (!m_parent->m_drawnTiles.isEmpty())
////    {
////        QHashIterator<QPoint, QGraphicsItem*> j (m_parent->m_drawnTiles);
////        while (j.hasNext())
////        {
////            j.next();
////            if (!m_parent->m_tilesInView.contains(j.key()))
////            {
////                m_parent->mView->scene()->removeItem(m_parent->m_drawnTiles[j.key()]);
////                QGraphicsItem* item = m_parent->m_drawnTiles[j.key()];
////                delete item;
////                m_parent->m_drawnTiles.remove(j.key());
////            }
////        }
////    }
////    if (!m_parent->m_emptyTiles.isEmpty())
////    {
////        QHashIterator<QPoint, QGraphicsItem*> k (m_parent->m_emptyTiles);
////        while (k.hasNext())
////        {
////            k.next();
////            if (!m_parent->m_tilesInView.contains(k.key()))
////            {
////                m_parent->mView->scene()->removeItem(m_parent->m_emptyTiles[k.key()]);
////                QGraphicsItem* item = m_parent->m_emptyTiles[k.key()];
////                delete item;
////                m_parent->m_emptyTiles.remove(k.key());
////            }
////        }
////    }

////}

////void MapDownloadThread::purgeNonVisiblePoints()
////{

////    try
////    {
////        foreach(Point* item, m_parent->Items)
////        {
////            Coordinate* c = new Coordinate(*(item->getLocation()));
////            if (m_parent->getLatLonBound().contains(c->GetLon(), c->GetLat()))
////            {
////                // don't do anything
////            }
////            else
////            {
////                m_parent->RemoveItem(item);
////            }
////        }
////    }
////    catch(...)
////    {
////        ErrorHandler::AlertAndLog("Map::purgeNonVisiblePoints() -- exception removing items... ");
////    }
////}


////void MapDownloadThread::backgroundAddPoint(Point *p)
////{
////    time_t start, end;
////    start = clock();
////    try
////    {
////        // don't plot items that aren't in the current map view
////        QPointF *pointLocation = p->getLocation();
////        if (!m_parent->getLatLonBound().contains(pointLocation->x(), pointLocation->y()))
////        {
////            return;
////        }

////        foreach(Point* item, m_parent->Items)
////        {
////            if(p != NULL && item != NULL && p->getCallSign() == item->getCallSign())
////            {
////                // this means the point already exists in the scene
////                // now figure out if this is an actual duplicate, or if this is a point that
////                // has moved since the last time it was added.

////                if ((qAbs(p->getLocation()->x() - item->getLocation()->x()) < 0.0000001) &&
////                    (qAbs(p->getLocation()->y() - item->getLocation()->y()) < 0.0000001))
////                   {
////                   }
////                else
////                {
////                    Coordinate* c = new Coordinate(*(p->getLocation())); // getLocation returns a pointer...
////                    //qDebug() << "adding point (" + QString::number(c->GetLat()) + ", " + QString::number(c->GetLon()) + ")";
////                    QPointF* sc = c->ToSceneCoord(m_parent->getMapOrigin(), m_parent->getZoom());
////                    //qDebug() << "which is scene: " + QString::number(sc->y()) + ", " + QString::number(sc->x());

////                    //m_parent->mView->scene()->addLine(sc->x(), sc->y(), item->getScenePos().x(), item->getScenePos().y(), QPen(QBrush(Qt::black), 5));
////                    QGraphicsLineItem* line = new QGraphicsLineItem(sc->x(), sc->y(), item->getScenePos().x(), item->getScenePos().y(), NULL, m_parent->mView->scene());
////                    line->setPen(QPen(QBrush(Qt::black), 5));
////                    m_parent->Traces.append(line);

////                    m_parent->mView->scene()->addItem(line);

////                    //qDebug() << "\n\ngot duplicate, not in the same position. callsign: " + p->getCallSign() + "\n\n";
////                }

////                ///qDebug() << "item->getScene(): " << item->getScene();
////                //qDebug() << "item: " << item;
////                item->Hide();

////                m_parent->Items.remove(m_parent->Items.indexOf(item));
////                item->RemoveFromMap(); // sets scene to 0x0 (removes from scene)

////                //emit(RemoveFromList(item->getCallSign()));
////                //qDebug() << "Duplicate: " << p->getCallSign();
////                // QGraphicsScene::removeItem: item 0x88ab7f8's scene (0x0) is different from this scene (0x87cb520)
////                //ErrorHandler::Log("Map::Add(" + p->getCallSign() + ") -- duplicate point at [" + QString::number(p->getLocation()->y()) + ", " + QString::number(p->getLocation()->x()) + "]");
////                break;
////            }
////        }

////        p->setScene(m_parent->mView->scene());
////        //p->setScale(this->getCurrentScale());
////        m_parent->Items.append(p);
////        p->Draw();

////        // I have no idea why this doesn't use the VIEW_OFFSET
////        Coordinate* c = new Coordinate(*(p->getLocation())); // getLocation returns a pointer...
////        //qDebug() << "adding point (" + QString::number(c->GetLat()) + ", " + QString::number(c->GetLon()) + ")";
////        QPointF* sc = c->ToSceneCoord(m_parent->getMapOrigin(), m_parent->getZoom());
////        //qDebug() << "which is scene: " + QString::number(sc->y()) + ", " + QString::number(sc->x());
////        p->setSceneLocation(sc->x(), sc->y());
////    }
////    catch(...)
////    {
////        ErrorHandler::AlertAndLog("Map::Add(" + p->getCallSign() + ") -- problem adding point at [" + QString::number(p->getLocation()->y()) + ", " + QString::number(p->getLocation()->x()) + "]");
////    }

////    end = clock();
////    if (end - start > 0)
////        qDebug() << "mapDownloadThread::add took " + QString::number(end - start);

////    //QEventLoop::processEvents();

////}
