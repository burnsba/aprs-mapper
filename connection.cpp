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

#include "connection.h"
#include <QTcpSocket>
#include <QApplication>
#include "errorhandler.h"
#include "profile.h"
#include "coordinate.h"
#include <QStringList>
#include <QDebug>
#include <QtSql>
#include "aprs_parser.h"

#define MAXQUEUE 24

Connection::Connection(QObject *parent) :
    QObject(parent)
{
    try
    {
        //test
        dataParser = new parser(this);
        connectionSocket = new QTcpSocket(this);
        connect(connectionSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(failedConnection(QAbstractSocket::SocketError)));
        connect(connectionSocket, SIGNAL(connected()), this, SLOT(sendConnection()));
        packetQueue = new QQueue<Packet>();
        latestPoint = NULL;
        isConnected = false;
    }
    catch(...)
    {
        ErrorHandler::AlertAndLog("Could not create a new connection");
    }
}

Connection::Connection(QString alias, QObject *parent) :
    QObject(parent)
{
    try
    {
        dataParser = new parser(this);
        connectionSocket = new QTcpSocket(this);
        connect(connectionSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(failedConnection(QAbstractSocket::SocketError)));
        connect(connectionSocket, SIGNAL(connected()), this, SLOT(sendConnection()));
        packetQueue = new QQueue<Packet>();
        latestPoint = NULL;
        isConnected = false;
        mAlias = alias;
        isSendEnabled = Profile::getSendReport(mAlias);

    }
    catch(...)
    {
        ErrorHandler::AlertAndLog("Could not create a new connection");
    }
}

void Connection::connectToIS(QString ipAddress, int portNum)
{
    connect(connectionSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(authSent()));
    connectionSocket->connectToHost(ipAddress, portNum);
    isConnected = false;

}

void Connection::connectToIS()
{
    connect(connectionSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(authSent()));
    connectionSocket->connectToHost(Profile::getServerAddress(mAlias), Profile::getServerPort(mAlias));
    isConnected = false;
}

void Connection::failedConnection(QAbstractSocket::SocketError error)
{
    QApplication::beep();
    // put in some more error handling logic here
    ErrorHandler::AlertAndLog("Connection Failed");
    isConnected = false;
    emit ConnectionChanged(mAlias);
    ErrorHandler::Alert(QString("Socket Error: " + QString::number(error) + " caused connection to drop."));
}

void Connection::sendConnection()
{
    QTextStream connection(connectionSocket);

    QString connectString;
    if(Profile::getAuthConn(mAlias))
    {
        connectString = QString("user %1 pass %3 vers APRS-Mapper v1 %4").arg(Profile::getCallSign()).arg(Profile::getConnectionPass()).arg(Profile::getConnectStr(mAlias));
    }
    else
    {
        connectString = QString("user %1 pass %3 vers APRS-Mapper v1 %4").arg(Profile::getCallSign()).arg("-1").arg(Profile::getConnectStr(mAlias));
    }
    //connectString = QString("user %1-%2 pass %3 vers APRS-Mapper v1 %4").arg("UAH05").arg("1").arg("-1").arg("");
    // pass 23443
    qDebug() << connectString;

    connectString += "\n";

    connection << connectString;
}

void Connection::authSent()
{
    disconnect(connectionSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(authSent()));
    connect(connectionSocket, SIGNAL(readyRead()), this, SLOT(readMessages()));
    isConnected = true;
    emit ConnectionChanged(mAlias);
    //sendPositionReport();
}


void Connection::readMessages()
{

    try
    {
        QTextStream* incomingMessage = new QTextStream(connectionSocket);

        Coordinate c;
        QString msg = incomingMessage->readLine();
        static Packet parsedPacket;

        if((msg != "") && (msg[0] != '#'))
        {
            // occassionally fails at the next line
            parsedPacket = dataParser->ParsePacket(msg);
            if(parsedPacket.isValid == true)
            {
                packetQueue->enqueue(parsedPacket);
            }
            else
            {
                emit InvalidPacket(parsedPacket);
            }

            if (packetQueue->count() == MAXQUEUE)
            {
                emit QueueFull(packetQueue);
                packetQueue->empty();
            }

                // comparing to itself checks for NaN's.  NaN is the default assigned value, and comparing two NaN's will always fail.
            if (parsedPacket.isValid == true && parsedPacket.latitude == parsedPacket.latitude && parsedPacket.longitude == parsedPacket.longitude)
            {
                c = Coordinate(parsedPacket.latitude, parsedPacket.longitude);

                Point* item;
                if((parsedPacket.packetType == T_ITEM) || (parsedPacket.packetType == T_OBJECT))
                {
                    item = new Point(parsedPacket.data, c.ToDecimalLatLon());
                }
                else
                {
                    item = new Point(parsedPacket.source, c.ToDecimalLatLon());
                }
                item->setSymbolId(parsedPacket.symbolID);
                //item->setTimestamp(parsedPacket.timestamp);
                item->setTimestamp(parsedPacket.rcvdTime);
    //            item->setAltitude(parsedPacket.altitude);
    //            item->setPayload(parsedPacket.payload);
    //            item->setSpeed(parsedPacket.speed);
    //            item->setPath(parsedPacket.path);
    //            item->setSource(parsedPacket.source);
                item->setOverlay(parsedPacket.symbolOverlay);
    //            item->setDestination(parsedPacket.destination);
    //            item->setPath(parsedPacket.path);
                item->setData(parsedPacket.data);
    //            item->setIsValid(parsedPacket.isValid);
    //            item->setMessage(parsedPacket.message);
    //            item->setCourse(parsedPacket.course);
    //            item->setTemp(parsedPacket.temp);
    //            item->setPressure(parsedPacket.pressure);
    //            item->setWindSpeed(parsedPacket.wind_speed);
    //            item->setWindDirection(parsedPacket.wind_direction);
    //            item->setWindGust(parsedPacket.wind_gust);
    //            item->setRain1hr(parsedPacket.rain_1h);
    //            item->setRain24hr(parsedPacket.rain_24h);
    //            item->setRainMidnight(parsedPacket.rain_midnight);
    //            item->setSnow24hr(parsedPacket.snow_24h);
    //            item->setRaincount(parsedPacket.raincount);
    //            item->setHumidity(parsedPacket.humidity);
    //            item->setLuminosity(parsedPacket.luminosity);
    //            item->setAlive(parsedPacket.alive);
                item->setPacketType(parsedPacket.packetType);

                emit AddItem(item);
                emit addedPoint(item->getSource());
                qApp->processEvents();
            }
        }
        delete incomingMessage;

    }
    catch(...)
    {
        ErrorHandler::AlertAndLog("Problem reading raw message packets");
    }

}

void Connection::disconnectIS()
{
    connectionSocket->disconnectFromHost();
    isConnected = false;
    emit ConnectionChanged(mAlias);
}

bool Connection::GetConnectionStatus()
{
    return isConnected;
}

Connection::~Connection()
{
    emit(QueueFull(packetQueue));
}

void Connection::sendPositionReport()
{
    if(isConnected && isSendEnabled)
    {
        QString latitude;
        QString longitude;
        QStringList split;

        if(Profile::getLatitude() < 0)
        {
            latitude = QString::number(-1 * Profile::getLatitude()) + "S";
        }
        else
        {
            latitude = QString::number(Profile::getLatitude()) + "N";
        }

        split = latitude.split('.');
        QString beforedecimal = split[0];
        while(beforedecimal.length() < 2)
        {
            beforedecimal.prepend("0");
        }
        QString afterdecimal = split[1];
        while(afterdecimal.length() < 4)
        {
            afterdecimal.append("0");
        }
        latitude = beforedecimal + "." + afterdecimal;

        if(Profile::getLongitude() < 0)
        {
            longitude = QString::number(-1 * Profile::getLongitude()) + "W";
        }
        else
        {
            longitude = QString::number(Profile::getLongitude()) + "E";
        }

        split = longitude.split('.');
        beforedecimal = split[0];
        while(beforedecimal.length() < 3)
        {
            beforedecimal.prepend("0");
        }
        afterdecimal = split[1];
        while(afterdecimal.length() < 4)
        {
            afterdecimal.append("0");
        }
        longitude = beforedecimal + "." + afterdecimal;

        QString symbolChars = parser::GetLookupCharsFromID(Profile::getIconId());
        QString positionReport = QString("%1-%2>%3:=%4%5%6%7%8\n").arg(Profile::getCallSign()).arg(Profile::getSSID()).arg(QString("UAH")).arg(latitude).arg(symbolChars[0]).arg(longitude).arg(symbolChars[1]).arg("comment");
        qDebug() << positionReport;

        QTextStream connection(connectionSocket);
        connection << positionReport;
    }
}
