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

#ifndef CONNECTION_H
#define CONNECTION_H

#include "errorhandler.h"
#include "point.h"
#include "parser.h"
#include <packettypes.h>
#include <QQueue>
#include <QTcpSocket>


#include <QObject>
class QTcpSocket;

class Connection : public QObject
{
    Q_OBJECT

public:
    explicit Connection(QObject *parent = 0);
    Connection(QString alias, QObject *parent = 0);
    ~Connection();
    void connectToIS(QString ipAddress, int portNum);
    void connectToIS();
    Point* latestPoint;

    QTcpSocket* connectionSocket;
    bool GetConnectionStatus();
    bool isSendEnabled;
    parser* dataParser;

    void setAlias(QString alias){mAlias = alias;}
    QString getAlias(){return mAlias;}
signals:

    void AddItem(Point *p);
    void addedPoint(QString p); // emits a signal containing the new packet as a string, when plotted
    void QueueFull(QQueue<Packet>* packet);
    void ConnectionChanged(QString alias);
    void InvalidPacket(Packet invalidPacket);

public slots:
    void failedConnection(QAbstractSocket::SocketError);
    void sendConnection();
    void authSent();
    void readMessages();
    void disconnectIS();
    void sendPositionReport();

private:
    bool isConnected;
    QQueue<Packet>* packetQueue;
    QString mAlias;

};

#endif // CONNECTION_H
