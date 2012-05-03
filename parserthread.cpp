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

#include "parserthread.h"
#include "errorhandler.h"
#include "profile.h"
#include <QDebug>

ParserThread::ParserThread()
{
    foreach(QString server, Profile::getConnectionList())
    {
        Connection* connection = new Connection(server);

        connections.append(connection);
    }
    posReportTimer.setInterval(Profile::getPosIntervalTime() * 60 * 1000);
    connect(&posReportTimer, SIGNAL(timeout()), this, SLOT(SendPositionReports()));
    posReportTimer.start();
}

ParserThread::~ParserThread()
{
    this->Stop();
    this->quit();
}

void ParserThread::Start()
{
    try
    {
        this->start(QThread::NormalPriority);

        //
    }
    catch(...)
    {

    }
}

void ParserThread::Stop()
{
    TimeToStop = true;
}

void ParserThread::Pause()
{
    try
    {
        if (mParserThread.isRunning())
        {
            mParserThread.terminate();
        }
    }
    catch(...)
    {
        ErrorHandler::AlertAndLog("The parsing thread failed to pause.");
    }
}

void ParserThread::run()
{
    TimeToStop = false;
    int i = 0;
    while (TimeToStop == false)
    {
        try
        {

            if(!TimeToStop && this->mMutex.tryLock(500) == true)
            {
                // parse or something
                //plot a few parsed packets
                if(i<2)
                {

                }
                else
                {

                    this->mMutex.unlock();
                }
                this->msleep(2000);
            }
        }
        catch(...)
        {

        }
         this->mMutex.lock();

    }
}

bool ParserThread::isConnected(QString alias)
{
    foreach(Connection* conn, this->connections)
    {
        if(conn->getAlias() == alias)
        {
            return conn->GetConnectionStatus();
        }
    }
    return false;
}

void ParserThread::EditConnection(QString alias)
{
    if(!Profile::getConnectionList().contains(alias))
    {
        foreach(QString server, Profile::getConnectionList())
        {
            if(getConnection(server) == 0)
            {
                getConnection(alias)->setAlias(server);
                break;
            }
        }
    }
}

void ParserThread::RemoveConnection(QString alias)
{
    foreach(Connection* conn, this->connections)
    {
        if(conn->getAlias() == alias)
        {
            connections.remove(connections.indexOf(conn));
            conn->disconnectIS();
            delete conn;
            break;
        }
    }
}

void ParserThread::AddConnection(QString alias)
{
    Connection* conn = new Connection(alias);
    connections.append(conn);
}

Connection* ParserThread::getConnection(QString alias)
{
    foreach(Connection* conn, this->connections)
    {
        if(conn->getAlias() == alias)
        {
            return conn;
        }
    }
    return 0;
}

void ParserThread::ConnectAll()
{
    foreach(Connection* conn, this->connections)
    {
        if(!conn->GetConnectionStatus())
        {
            conn->connectToIS();
        }
    }
}

void ParserThread::DisconnectAll()
{
    foreach(Connection* conn, this->connections)
    {
        if(conn->GetConnectionStatus())
        {
            conn->disconnectIS();
        }
    }
}

void ParserThread::SendPositionReports()
{
    qDebug() << "sending position reports";
    foreach(Connection* conn, this->connections)
    {
        if(Profile::getSendReport(conn->getAlias()))
        {
            conn->sendPositionReport();
        }
    }
}
