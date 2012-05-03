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

#include "database.h"
#include "point.h"
#include "coordinate.h"
#include "aprs_parser.h"
#include <errorhandler.h>
#include "time.h"


Database::Database()
{
    //dbType = Profile::getDBType();
    qDebug() << "MySQL? > " + Profile::isMySQL();
    if(Profile::isMySQL())
    {
        dbType = MySQL;
    }
    else
    {
        dbType = SQLite;
    }
    //dbType = MySQL;
}

Database::~Database()
{
    db.close();
    this->Stop();
    this->quit();
}

void Database::Start()
{
    try
    {
        this->start(QThread::NormalPriority);
        qDebug() << "start database thread";
        this->Init();
    }
    catch(...)
    {
        qDebug() << "Error starting database thread";
    }

}

void Database::Init()
{
    switch(dbType)
    {
        case SQLite:
            if(!InitSQLite())
            {
                ErrorHandler::Alert("SQLite Database Initialization Failed");
            }
            break;
        case MySQL:
            if(!InitMySQL())
            {
                ErrorHandler::Alert("MySQL Database Initialization Failed");
            }
            break;
        default:
            ErrorHandler::Alert("WAT DID YOU DO?");
        break;
    }
}

bool Database::InitSQLite()
{
    qDebug() << Profile::getSQLiteFilepath();
    db = QSqlDatabase::addDatabase("QSQLITE");
    if(Profile::getSQLiteFilepath() == "")
    {
        db.setDatabaseName("DATABASE.DB");
    }
    else
    {
        db.setDatabaseName(Profile::getSQLiteFilepath());
    }

    if(!db.open())
    {
        return false;
    }

    return FirstTimeSQLite();
}

bool Database::InitMySQL()
{
    db = QSqlDatabase::addDatabase("QMYSQL");

    db.setHostName(Profile::getMySQLIPAddr());
    db.setPort(Profile::getMySQLPort());
    db.setUserName(Profile::getMySQLUser());
    db.setPassword(Profile::getMySQLPass());
    db.setDatabaseName(Profile::getMySQLDBName());
    /*
    db.setHostName("localhost");
    db.setPort(3306);
    db.setUserName("aprs");
    db.setPassword("aprs");
    db.setDatabaseName("aprs");
    */
    if(!db.open())
    {
        return false;
    }

    return FirstTimeMySQL();
}

#warning TODO:  modify the compound unique key.  possible to get duplicates that shouldnt be, esp. with weather stations

bool Database::FirstTimeSQLite()
{
    QSqlQuery query;
    QString makeTables = "CREATE TABLE IF NOT EXISTS packets (" \
                         "source TEXT, " \
                         "data TEXT, " \
                         "destination TEXT, "
                         "rcvdtime TEXT, " \
                         "timestamp TEXT, " \
                         "type INT, " \
                         "symbolID INT, " \
                         "overlay TEXT, " \
                         "latitude DOUBLE, " \
                         "longitude DOUBLE, " \
                         "altitude DOUBLE, " \
                         "path TEXT, " \
                         "payload TEXT, " \
                         "message TEXT, " \
                         "course INT, " \
                         "speed DOUBLE, " \
                         "temp DOUBLE, " \
                         "pressure DOUBLE, " \
                         "windspeed DOUBLE, " \
                         "winddirection INT, " \
                         "windgust DOUBLE, " \
                         "rainh DOUBLE, " \
                         "raind DOUBLE, " \
                         "rainm DOUBLE, " \
                         "snowd DOUBLE, " \
                         "raincount INT, " \
                         "humidity INT, " \
                         "luminosity INT, " \
                         "UNIQUE (source, latitude, longitude, payload) ON CONFLICT REPLACE)";
    if(!query.exec(makeTables))
    {
        return false;
    }

    makeTables = "CREATE TABLE IF NOT EXISTS invalidpackets (" \
                 "timestamp TEXT, " \
                 "errorcode INT, " \
                 "rawpacket TEXT)";

    return query.exec(makeTables);
}

bool Database::FirstTimeMySQL()
{

    #warning TODO:  put in real mysql data types

    QSqlQuery query;
    QString makeTables = "CREATE TABLE IF NOT EXISTS packets (" \
                         "source VARCHAR(16), " \
                         "data VARCHAR(16), " \
                         "destination VARCHAR(16), " \
                         "rcvdtime VARCHAR(100), " \
                         "timestamp VARCHAR(100), " \
                         "type INT, " \
                         "symbolID INT, " \
                         "overlay VARCHAR(1), " \
                         "latitude REAL, " \
                         "longitude REAL, " \
                         "altitude REAL, " \
                         "path VARCHAR(100), " \
                         "payload VARCHAR(400), " \
                         "message VARCHAR(400), " \
                         "course INT, " \
                         "speed REAL, " \
                         "temp REAL, " \
                         "pressure REAL, " \
                         "windspeed REAL, " \
                         "winddirection INT, " \
                         "windgust REAL, " \
                         "rainh REAL, " \
                         "raind REAL, " \
                         "rainm REAL, " \
                         "snowd REAL, " \
                         "raincount INT, " \
                         "humidity INT, " \
                         "luminosity INT, " \
                         "UNIQUE (source, latitude, longitude, payload))";

    if(!query.exec(makeTables))
    {
        qDebug() << query.lastError();
        return false;
    }
    makeTables = "CREATE TABLE IF NOT EXISTS invalidpackets (" \
                 "timestamp TEXT, " \
                 "errorcode INT, " \
                 "rawpacket TEXT)";

    return query.exec(makeTables);
}

//Method to commit large amounts of packet data
void Database::DatabaseCommit(QQueue<Packet> *packetQueue)
{
    try
    {
        int count = 0; //used to check the database inserts
        qDebug() << "COMMITTING TO DATABASE";
        QSqlQuery query;
        //Loop through packetQueue
        query.exec("BEGIN TRANSACTION");
        while (!packetQueue->isEmpty())
        {
            Packet parsedPacket = packetQueue->dequeue();
            //qDebug() << "\t\t\t" << "inserting into database" << count;
            switch(dbType)
            {
            case SQLite:
                query.prepare("INSERT INTO packets (source, data, destination, rcvdtime, timestamp, type, symbolID, " \
                              "overlay, latitude, longitude, altitude, path, payload, message, course, speed, temp, " \
                              "pressure, windspeed, winddirection, windgust, rainh, raind, rainm, snowd, raincount, " \
                              "humidity, luminosity) " \
                              "VALUES (:source, :data, :destination, :rcvdtime, :timestamp, :type, :symbolID, :overlay, "\
                              ":latitude, :longitude, :altitude, :path, :payload, :message, :course, :speed, :temp, " \
                              ":pressure, :windspeed, :winddirection, :windgust, :rainh, :raind, :rainm, :snowd, :raincount, " \
                              ":humidity, :luminosity)");
                break;
            case MySQL:
                query.prepare("REPLACE INTO packets (source, data, destination, rcvdtime, timestamp, type, symbolID, " \
                              "overlay, latitude, longitude, altitude, path, payload, message, course, speed, temp, " \
                              "pressure, windspeed, winddirection, windgust, rainh, raind, rainm, snowd, raincount, " \
                              "humidity, luminosity) " \
                              "VALUES (:source, :data, :destination, :rcvdtime, :timestamp, :type, :symbolID, :overlay, "\
                              ":latitude, :longitude, :altitude, :path, :payload, :message, :course, :speed, :temp, " \
                              ":pressure, :windspeed, :winddirection, :windgust, :rainh, :raind, :rainm, :snowd, :raincount, " \
                              ":humidity, :luminosity)");
                break;
            }
                query.bindValue(":source", parsedPacket.source);
                query.bindValue(":data", parsedPacket.data);
                query.bindValue(":destination", parsedPacket.destination);
                query.bindValue(":rcvdtime", parsedPacket.rcvdTime);
                query.bindValue(":timestamp", parsedPacket.timestamp);
                query.bindValue(":type", parsedPacket.packetType);
                query.bindValue(":symbolID", parsedPacket.symbolID);
                query.bindValue(":overlay", parsedPacket.symbolOverlay);
                query.bindValue(":latitude", parsedPacket.latitude);
                query.bindValue(":longitude", parsedPacket.longitude);
                query.bindValue(":altitude", parsedPacket.altitude);
                query.bindValue(":path", parsedPacket.path);
                query.bindValue(":payload", parsedPacket.payload);
                query.bindValue(":message", parsedPacket.message);
                query.bindValue(":course", parsedPacket.course);
                query.bindValue(":speed", parsedPacket.speed);
                query.bindValue(":temp", parsedPacket.temp);
                query.bindValue(":pressure", parsedPacket.pressure);
                query.bindValue(":windspeed", parsedPacket.wind_speed);
                query.bindValue(":winddirection", parsedPacket.wind_direction);
                query.bindValue(":windgust", parsedPacket.wind_gust);
                query.bindValue(":rainh", parsedPacket.rain_1h);
                query.bindValue(":raind", parsedPacket.rain_24h);
                query.bindValue(":rainm", parsedPacket.rain_midnight);
                query.bindValue(":snowd", parsedPacket.snow_24h);
                query.bindValue(":raincount", parsedPacket.raincount);
                query.bindValue(":humidity", parsedPacket.humidity);
                query.bindValue(":luminosity", parsedPacket.luminosity);

                query.exec();

                count++;

        }
        query.exec("COMMIT");

        count = 0;
    }
    catch(...)
    {
        ErrorHandler::AlertAndLog("Problem committing parsed packets to database.");
    }

}

void Database::CommitInvalid(Packet invalidPacket)
{
    QSqlQuery query;
    query.prepare("INSERT INTO invalidpackets (timestamp, errorcode, rawpacket) VALUES (:timestamp, :errorcode, :rawpacket)");
    query.bindValue(":timestamp", invalidPacket.timestamp);
    query.bindValue(":errorcode", invalidPacket.error);
    query.bindValue(":rawpacket", invalidPacket.rawPacket);
    query.exec();
    qDebug() << "bad packet: " << invalidPacket.rawPacket;
}

//Method to retrieve this data
QQueue<Packet> Database::PlotPackets()
{
    // THIS FUNCTION NOT IN USE
    //ErrorHandler::Debug(ErrorLevel::N, "PlotPackets()");
    QQueue<Packet> packetsToPlot;
    Packet packets;
    QSqlQuery query;
    QString packetsToRetrieve = "SELECT * FROM packets";
    query.exec(packetsToRetrieve);



    while(query.next())
    {
        packets.source = query.value(1).toString();
        packets.symbolID = query.value(3).toInt();
        packets.latitude = query.value(4).toDouble();
        packets.longitude = query.value(5).toDouble();
        packetsToPlot.enqueue(packets);

    }
    return packetsToPlot;
}

void Database::Purge(QDateTime time)
{
    QString queryString;
    switch(dbType)
    {
    case MySQL:
        queryString = QString("DELETE FROM packets WHERE rcvdtime < '%1'").arg(time.toString("yyyy-MM-dd HH:mm:ss"));
        break;
    case SQLite:
        queryString = QString("DELETE FROM packets WHERE rcvdtime < '%1'").arg(time.toString("yyyy-MM-ddTHH:mm:ss"));
        break;
    }

    QSqlQuery* query = new QSqlQuery(db);

    bool successful = query->exec(queryString);

    if(!successful)
    {
        qDebug() << queryString;
        qDebug() << query->lastError();
    }
}

void Database::Purge()
{
    QDateTime time = QDateTime::currentDateTime().addSecs(-3600 * Profile::getPurgeTime());
    QString queryString;
    switch(dbType)
    {
    case MySQL:
        queryString = QString("DELETE FROM packets WHERE rcvdtime < '%1'").arg(time.toString("yyyy-MM-dd HH:mm:ss"));
        break;
    case SQLite:
        queryString = QString("DELETE FROM packets WHERE rcvdtime < '%1'").arg(time.toString("yyyy-MM-ddTHH:mm:ss"));
        break;
    }

    QSqlQuery* query = new QSqlQuery(db);

    bool successful = query->exec(queryString);

    if(!successful)
    {
        qDebug() << queryString;
        qDebug() << query->lastError();
    }
}

void Database::PurgeAll()
{
    QSqlQuery* query = new QSqlQuery("DELETE FROM PACKETS");
    query->exec();
}

QQueue<Packet> Database::GetPacketInfo(QString callsign)
{
   QSqlQuery query;
   query.prepare("SELECT * FROM packets WHERE source=:source ORDER BY timestamp");
   query.bindValue(":source", callsign);
   query.exec();

   Packet dbPacket;

   QQueue<Packet> dbPacketQueue;

   while(query.next())
   {

       dbPacket.source = query.value(0).toString();
       dbPacket.destination = query.value(2).toString();
       dbPacket.data = query.value(1).toString();
       dbPacket.rcvdTime = query.value(3).toDateTime();
       dbPacket.timestamp = query.value(4).toDateTime();
       switch(query.value(5).toInt())
       {
       case 0:
           dbPacket.packetType = T_UNKNOWN;
           break;
       case 1:
           dbPacket.packetType = T_CAPABILITIES;
           break;
       case 2:
           dbPacket.packetType = T_DX;
           break;
       case 3:
           dbPacket.packetType = T_ITEM;
           break;
       case 4:
           dbPacket.packetType = T_LOCATION;
           break;
       case 5:
           dbPacket.packetType = T_MESSAGE;
           break;
       case 6:
           dbPacket.packetType = T_OBJECT;
           break;
       case 7:
           dbPacket.packetType = T_STATUS;
           break;
       case 8:
           dbPacket.packetType = T_TELEMETRY;
           break;
       case 9:
           dbPacket.packetType = T_TELEMETRY_MSG;
           break;
       case 10:
           dbPacket.packetType = T_WX;
           break;
       default:
           dbPacket.packetType = T_UNKNOWN;
           break;
       }

       dbPacket.symbolID = query.value(6).toInt();
       dbPacket.symbolOverlay = query.value(7).toString();
       dbPacket.latitude = query.value(8).toDouble();
       dbPacket.longitude = query.value(9).toDouble();
       dbPacket.altitude = query.value(10).toDouble();
       dbPacket.path = query.value(11).toString();
       dbPacket.payload = query.value(12).toString();
       dbPacket.message = query.value(13).toString();
       dbPacket.course = query.value(14).toInt();
       dbPacket.speed = query.value(15).toDouble();
       dbPacket.temp = query.value(16).toDouble();
       dbPacket.pressure = query.value(17).toDouble();
       dbPacket.wind_speed = query.value(18).toDouble();
       dbPacket.wind_direction = query.value(19).toInt();
       dbPacket.wind_gust = query.value(20).toDouble();
       dbPacket.rain_1h = query.value(21).toDouble();
       dbPacket.rain_24h = query.value(22).toDouble();
       dbPacket.rain_midnight = query.value(23).toDouble();
       dbPacket.snow_24h = query.value(24).toDouble();
       dbPacket.raincount = query.value(25).toDouble();
       dbPacket.humidity = query.value(26).toInt();
       dbPacket.luminosity = query.value(27).toInt();

       dbPacketQueue.enqueue(dbPacket);

       qApp->processEvents();
   }

    if (dbPacketQueue.count() > 0)
        return dbPacketQueue;

    // else it was probable an object

    query;
    query.prepare("SELECT * FROM packets WHERE data=:source ORDER BY timestamp");
    query.bindValue(":source", callsign);
    query.exec();

    while(query.next())
    {

        dbPacket.source = query.value(0).toString();
        dbPacket.destination = query.value(2).toString();
        dbPacket.data = query.value(1).toString();
        dbPacket.rcvdTime = query.value(3).toDateTime();
        dbPacket.timestamp = query.value(4).toDateTime();
        switch(query.value(5).toInt())
        {
        case 0:
            dbPacket.packetType = T_UNKNOWN;
            break;
        case 1:
            dbPacket.packetType = T_CAPABILITIES;
            break;
        case 2:
            dbPacket.packetType = T_DX;
            break;
        case 3:
            dbPacket.packetType = T_ITEM;
            break;
        case 4:
            dbPacket.packetType = T_LOCATION;
            break;
        case 5:
            dbPacket.packetType = T_MESSAGE;
            break;
        case 6:
            dbPacket.packetType = T_OBJECT;
            break;
        case 7:
            dbPacket.packetType = T_STATUS;
            break;
        case 8:
            dbPacket.packetType = T_TELEMETRY;
            break;
        case 9:
            dbPacket.packetType = T_TELEMETRY_MSG;
            break;
        case 10:
            dbPacket.packetType = T_WX;
            break;
        default:
            dbPacket.packetType = T_UNKNOWN;
            break;
        }

        dbPacket.symbolID = query.value(6).toInt();
        dbPacket.symbolOverlay = query.value(7).toString();
        dbPacket.latitude = query.value(8).toDouble();
        dbPacket.longitude = query.value(9).toDouble();
        dbPacket.altitude = query.value(10).toDouble();
        dbPacket.path = query.value(11).toString();
        dbPacket.payload = query.value(12).toString();
        dbPacket.message = query.value(13).toString();
        dbPacket.course = query.value(14).toInt();
        dbPacket.speed = query.value(15).toDouble();
        dbPacket.temp = query.value(16).toDouble();
        dbPacket.pressure = query.value(17).toDouble();
        dbPacket.wind_speed = query.value(18).toDouble();
        dbPacket.wind_direction = query.value(19).toInt();
        dbPacket.wind_gust = query.value(20).toDouble();
        dbPacket.rain_1h = query.value(21).toDouble();
        dbPacket.rain_24h = query.value(22).toDouble();
        dbPacket.rain_midnight = query.value(23).toDouble();
        dbPacket.snow_24h = query.value(24).toDouble();
        dbPacket.raincount = query.value(25).toDouble();
        dbPacket.humidity = query.value(26).toInt();
        dbPacket.luminosity = query.value(27).toInt();

        dbPacketQueue.enqueue(dbPacket);

        qApp->processEvents();
    }

    return dbPacketQueue;
}

void Database::GetPacketsArea(QRectF area)
{
//    time_t start, end;
//    start = clock();

    timeToStopGettingPacketsArea = false; // can be set to false by a signal

    QSqlQuery query;

    //query.prepare("SELECT * FROM packets JOIN (SELECT DISTINCT source, data FROM packets) calls ON calls.source = packets.source AND calls.data = packets.data WHERE latitude <= :topLeftLat AND latitude >= :botRightLat AND longitude >= :topLeftLong AND longitude <= :botRightLong AND rcvdtime >= :purgeCutOff");
    query.prepare("SELECT * FROM packets WHERE latitude <= :topLeftLat AND latitude >= :botRightLat AND longitude >= :topLeftLong AND longitude <= :botRightLong AND rcvdtime >= :purgeCutOff");
    query.bindValue(":topLeftLat", area.top());
    query.bindValue(":botRightLat", area.bottom());
    query.bindValue(":topLeftLong", area.left());
    query.bindValue(":botRightLong", area.right());
    QDateTime clearBefore = QDateTime::currentDateTime().addSecs(-1 * static_cast<int>(Profile::getClearTime() * 3600));
    switch(dbType)
    {
    case MySQL:
        query.bindValue(":purgeCutOff", clearBefore.toString("yyyy-MM-dd HH:mm:ss"));
        break;
    case SQLite:
        query.bindValue(":purgeCutOff", clearBefore.toString("yyyy-MM-ddTHH:mm:ss"));
        break;
    }


    //qDebug() << "getting points since " + clearBefore.toString("yyyy-MM-ddTHH:mm:ss");
    query.exec();

    Packet dbPacket;


    //QQueue<Packet> dbPacketQueue;

    while(query.next() && !timeToStopGettingPacketsArea)
    {

        dbPacket.source = query.value(0).toString();
        dbPacket.destination = query.value(2).toString();
        dbPacket.data = query.value(1).toString();
        dbPacket.rcvdTime = query.value(3).toDateTime();
        dbPacket.timestamp = query.value(4).toDateTime();
        switch(query.value(5).toInt())
        {
        case 0:
            dbPacket.packetType = T_UNKNOWN;
            break;
        case 1:
            dbPacket.packetType = T_CAPABILITIES;
            break;
        case 2:
            dbPacket.packetType = T_DX;
            break;
        case 3:
            dbPacket.packetType = T_ITEM;
            break;
        case 4:
            dbPacket.packetType = T_LOCATION;
            break;
        case 5:
            dbPacket.packetType = T_MESSAGE;
            break;
        case 6:
            dbPacket.packetType = T_OBJECT;
            break;
        case 7:
            dbPacket.packetType = T_STATUS;
            break;
        case 8:
            dbPacket.packetType = T_TELEMETRY;
            break;
        case 9:
            dbPacket.packetType = T_TELEMETRY_MSG;
            break;
        case 10:
            dbPacket.packetType = T_WX;
            break;
        default:
            dbPacket.packetType = T_UNKNOWN;
            break;
        }

        dbPacket.symbolID = query.value(6).toInt();
        dbPacket.symbolOverlay = query.value(7).toString();
        dbPacket.latitude = query.value(8).toDouble();
        dbPacket.longitude = query.value(9).toDouble();
//        dbPacket.altitude = query.value(10).toDouble();
//        dbPacket.path = query.value(11).toString();
//        dbPacket.payload = query.value(12).toString();
//        dbPacket.message = query.value(13).toString();
//        dbPacket.course = query.value(14).toInt();
//        dbPacket.speed = query.value(15).toDouble();
//        dbPacket.temp = query.value(16).toDouble();
//        dbPacket.pressure = query.value(17).toDouble();
//        dbPacket.wind_speed = query.value(18).toDouble();
//        dbPacket.wind_direction = query.value(19).toInt();
//        dbPacket.wind_gust = query.value(20).toDouble();
//        dbPacket.rain_1h = query.value(21).toDouble();
//        dbPacket.rain_24h = query.value(22).toDouble();
//        dbPacket.rain_midnight = query.value(23).toDouble();
//        dbPacket.snow_24h = query.value(24).toDouble();
//        dbPacket.raincount = query.value(25).toDouble();
//        dbPacket.humidity = query.value(26).toInt();
//        dbPacket.luminosity = query.value(27).toInt();

//        for(int i = 0; i < 28; i++)
//        {
//            qDebug() << i << " - " << query.value(i).toString();
//        }

        //dbPacketQueue.enqueue(dbPacket);
        Point *point = new Point();
        if(dbPacket.packetType == T_ITEM || dbPacket.packetType == T_OBJECT)
        {
            point->setSource(dbPacket.data);
        }
        else
        {
            point->setSource(dbPacket.source);
        }
        point->setLocation(dbPacket.latitude, dbPacket.longitude);
//        point->setAltitude(dbPacket.altitude);
//        point->setPayload(dbPacket.payload);
        point->setSymbolId(dbPacket.symbolID);
//        point->setSpeed(dbPacket.speed);
        point->setOverlay(dbPacket.symbolOverlay);
        point->setTimestamp(dbPacket.rcvdTime);
//        point->setPath(dbPacket.path);
//        point->setSource(dbPacket.source);
//        point->setDestination(dbPacket.destination);
//        point->setPath(dbPacket.path);
        point->setData(dbPacket.data);
//        point->setIsValid(dbPacket.isValid);
//        point->setMessage(dbPacket.message);
//        point->setCourse(dbPacket.course);
//        point->setTemp(dbPacket.temp);
//        point->setPressure(dbPacket.pressure);
//        point->setWindSpeed(dbPacket.wind_speed);
//        point->setWindDirection(dbPacket.wind_direction);
//        point->setWindGust(dbPacket.wind_gust);
//        point->setRain1hr(dbPacket.rain_1h);
//        point->setRain24hr(dbPacket.rain_24h);
//        point->setRainMidnight(dbPacket.rain_midnight);
//        point->setSnow24hr(dbPacket.snow_24h);
//        point->setRaincount(dbPacket.raincount);
//        point->setHumidity(dbPacket.humidity);
//        point->setLuminosity(dbPacket.luminosity);
//        point->setAlive(dbPacket.alive);
        point->setPacketType(dbPacket.packetType);

        qApp->processEvents();

        emit PlotThis(point);


    }

//    end = clock();
//    if (end - start > 0)
//        qDebug() << "db::GetPacketsArea took " + QString::number(end - start);


    //emit PlotRectangle(dbPacketQueue);
}

void Database::Stop()
{
    TimeToStop = true;
}

void Database::Pause()
{
    try
    {
        if (databaseThread.isRunning())
        {
            databaseThread.terminate();
        }
    }
    catch(...)
    {
        ErrorHandler::AlertAndLog("The database thread failed to pause.");
    }
}
void Database::run()
{
    TimeToStop = false;

    while (TimeToStop == false)
    {
        try
        {

            if(!TimeToStop && this->dMutex.tryLock(1000) == true)
            {
                this->dMutex.unlock();
            }
            this->msleep(2000);
        }
        catch(...)
        {

        }
        this->dMutex.lock();
    }
}

void Database::setDatabaseType(DatabaseType type)
{
    dbType = type;
}

Database::DatabaseType Database::getDatabaseType()
{
    return dbType;
}

void Database::ChangeDatabase(QString newType)
{
    if(newType == "MySQL")
    {
        setDatabaseType(MySQL);
    }
    else
    {
        setDatabaseType(SQLite);
    }

    // signals connections to disconnect
    emit DatabaseChanging();
    // waits for them to finish disconnecting
    wait(500);
    db.close();

    Init();
}

QPointF Database::getPointLocation(QString callsign)
{
    QPointF result;
    QSqlQuery query;
    query.prepare("SELECT latitude, longitude FROM packets WHERE source='" + callsign + "' OR data='" + callsign + "' LIMIT 1");
    //query.bindValue(":callsign", callsign);
    query.exec();
    while(query.next())
    {
        result.setX(query.value(1).toReal());
        result.setY(query.value(0).toReal());

        return result;
    }
    result.setX(360.0);
    result.setY(360.0);
    return result;
}

void Database::StopGettingPacketsArea()
{
    timeToStopGettingPacketsArea = true;
}
