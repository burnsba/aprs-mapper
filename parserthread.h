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

#ifndef PARSERTHREAD_H
#define PARSERTHREAD_H

#include "connection.h"
#include "point.h"

#include <QThread>
#include <QMutex>
#include <QVector>
#include <QTimer>
//#include <QCloseEvent>


class ParserThread : public QThread
{
    Q_OBJECT

public:
    ParserThread();
    ~ParserThread();

    void Pause();
    void Start();
    void Stop();

    QVector<Connection*> connections;
    bool isConnected(QString alias);
    Connection* getConnection(QString alias);
protected:
    void run(); // this is like main(), for threads (don't change the case)

private:
    QThread mParserThread;
    bool TimeToStop;
    QMutex mMutex;
    QTimer posReportTimer;
signals:
    void AddItem(Point *p);

public slots:
    void EditConnection(QString alias);
    void RemoveConnection(QString alias);
    void AddConnection(QString alias);

    void DisconnectAll();
    void ConnectAll();

    void SendPositionReports();
private slots:


};

#endif // PARSERTHREAD_H
