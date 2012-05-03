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

#ifndef DATABASE_H
#define DATABASE_H

#include "packettypes.h"
#include "point.h"
#include <QQueue>
#include <QObject>
#include <QString>
#include <QtSql>
#include <QDebug>
#include <QQueue>
#include <QPointF>


class Database : public QThread
{
    Q_OBJECT

public:
    enum DatabaseType{SQLite, MySQL};
    Database();
    ~Database();
    QQueue<Packet> PlotPackets();
    QSqlDatabase db;
    QThread databaseThread;
    QMutex dMutex;
    void Pause();
    void Start();
    void Stop();
    void Purge(QDateTime time);
    void PurgeAll();
    QQueue<Packet> GetPacketInfo(QString callsign);
    void setDatabaseType(DatabaseType type);
    DatabaseType getDatabaseType();
    QPointF getPointLocation(QString callsign);

public slots:
    void Purge();
    void ChangeDatabase(QString newType);
    void GetPacketsArea(QRectF area);
    void StopGettingPacketsArea();

signals:
    void ReadyToPlot();
    void DatabaseChanging();
    void PlotRectangle(QQueue<Packet>);
    void PlotThis(Point*);

private:
    bool FirstTimeSQLite();
    bool FirstTimeMySQL();
    bool TimeToStop;
    void Init();
    DatabaseType dbType;
    bool InitSQLite();
    bool InitMySQL();
    bool timeToStopGettingPacketsArea;
private slots:
    void DatabaseCommit(QQueue<Packet> *packetQueue);
    void CommitInvalid(Packet invalidPacket);
protected:
    void run(); // this is like main(), for threads (don't change the case)
};

#endif // DATABASE_H
