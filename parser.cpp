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

#include "parser.h"
#include "packettypes.h"
#include <QtDebug>
#include <QRegExp>
#include <QStringList>
#include <QVariant>
#include <QtCore/qmath.h>
#include "errorhandler.h"
#include "aprs_parser.h"
#include "stringify.h"
#include "point.h"

parser::parser(QObject *parent) :
    QObject(parent)
{
    //if(CURRENT_ERROR_LEVEL == ErrorLevel::P)
    //{
        //qDebug() << "Attempted to parse the packet: 9A7S-10>AP4R10,TCPIP*,qAC,T2AUSTRIA:!4520.34N/1620.20E#9A7S-10 APRS4R IGATE";

        //Packet test = ParseAlmostEveryPacket("9A7S-10>AP4R10,TCPIP*,qAC,T2AUSTRIA:!4520.34N/1620.20E#9A7S-10 APRS4R IGATE");

        //DumpPacketInfo(test);
        //DumpPacketInfo(ParseCompressedPacket("DC4ZZ-3>APU25N,TCPIP*,qAC,T2FINLAND:=/5/K{Pew'#  B", 33));
        //Packet test = ParsePacket("VA3SHL-9>TS3R9V,WIDE1-1,WIDE2-2,qAR,VE3YAP:'O2!^->/]\"6+}Barnaby-a3shl@rac.ca  ARES");
        //DumpPacketInfo(test);
    //}
    // SV4VQX: +3646.18
    // change


}

QString parser::ConvertToBase91(qint32 input)
{
    QString returnVal = "";
    int result;

    /*
PSUEDO CODE:
1       String Conversion(int M, int N)	// return string, accept two integers
2	{
3	    Stack stack = new Stack();	// create a stack
4	    while (M >= N)	// now the repetitive loop is clearly seen
5	    {
6	        stack.push(M mod N);	// store a digit
7	        M = M/N;	// find new M
8	    }
9	    // now it's time to collect the digits together
10	    String str = new String(""+M);	// create a string with a single digit M
11	    while (stack.NotEmpty())
12	        str = str+stack.pop()	// get from the stack next digit
13	    return str;
14	}

     */

    result = input;

    while(result >= 91)
    {
        returnVal.prepend(char((result % 91) + 33));
        result = result / 91;
    }
    returnVal.prepend(char((result % 91) + 33));

    return returnVal;
}

qint32 parser::ConvertFromBase91(QString input)
{
    qint32 size = input.length();
    qint32 resultVal = 0;

    for(int i = 0; i < size; i++)
    {
        resultVal += ((input[i].toAscii() - 33) * pow(91, (size - (i + 1))));
    }

    return resultVal;
}

Packet parser::ParsePacket(QString packet)
{ 
    try
    {
        APRSParser aprsparser;
        result res = aprsparser.parse(packet);
        Packet parsedPacket;

        parsedPacket.source = aprsparser.srccallsign;
        parsedPacket.destination = aprsparser.dstcallsign;
        parsedPacket.path = aprsparser.digipeaters.join(",");
        parsedPacket.packetType = aprsparser.type;
        parsedPacket.altitude = aprsparser.altitude;
        parsedPacket.data = aprsparser.data;
        parsedPacket.latitude = aprsparser.latitude;
        parsedPacket.longitude = aprsparser.longitude;
        parsedPacket.symbolID = GetSymbolID(QString(aprsparser.symboltable), QString(aprsparser.symbolcode));
        if((aprsparser.symboltable != '\\') && (aprsparser.symboltable != '/'))
        {
            int symbolTableAscii = QChar(aprsparser.symbolcode).toAscii();
            if((symbolTableAscii >= 32) && (symbolTableAscii <= 126))
            {
                parsedPacket.symbolOverlay = aprsparser.symboltable;
            }
        }
        parsedPacket.payload = aprsparser.comment;
        parsedPacket.course = aprsparser.course;
        parsedPacket.speed = aprsparser.speed;
        parsedPacket.message = aprsparser.message;

        parsedPacket.rcvdTime = QDateTime::currentDateTime();

        QDateTime timestamp = QDateTime::currentDateTime();
        QString time;

        if(aprsparser.stamptype == "h")
        {
            time = QString("%1-%2-%3-%4:%5:%6").arg(QString::number(QDate::currentDate().year())) \
                   .arg(QString::number(QDate::currentDate().month())).arg(QString::number(QDate::currentDate().day())) \
                   .arg(QString::number(aprsparser.hour)).arg(QString::number(aprsparser.minute)).arg(QString::number(aprsparser.second));
            timestamp = timestamp.fromString(time, "yyyy-MM-dd-HH:mm:ss");
        }
        else if((aprsparser.stamptype == "z") || (aprsparser.stamptype == "/"))
        {
            time = QString("%1-%2-%3-%4:%5").arg(QString::number(QDate::currentDate().year())) \
                   .arg(QString::number(QDate::currentDate().month())).arg(aprsparser.day) \
                   .arg(QString::number(aprsparser.hour)).arg(QString::number(aprsparser.minute));
            timestamp = timestamp.fromString(time, "yyyy-MM-dd-HH:mm");
        }
        if(timestamp.isNull() || !timestamp.isValid())
        {
            timestamp = QDateTime::currentDateTime();
        }
        parsedPacket.timestamp = timestamp;
        parsedPacket.timestampType = aprsparser.stamptype;
        parsedPacket.speed = aprsparser.speed;
        parsedPacket.temp = aprsparser.temperature;
        parsedPacket.pressure = aprsparser.pressure;
        parsedPacket.wind_speed = aprsparser.wind_speed;
        parsedPacket.wind_direction = aprsparser.wind_direction;
        parsedPacket.wind_gust = aprsparser.wind_gust;
        parsedPacket.rain_1h = aprsparser.rain_1h;
        parsedPacket.rain_24h = aprsparser.rain_24h;
        parsedPacket.rain_midnight = aprsparser.rain_midnight;
        parsedPacket.snow_24h = aprsparser.snow_24h;
        parsedPacket.raincount = aprsparser.raincount;
        parsedPacket.humidity = aprsparser.raincount;
        parsedPacket.luminosity = aprsparser.luminosity;
        parsedPacket.alive = aprsparser.alive;
        /*
        parsedPacket.messaging = aprsparser.messaging;
        parsedPacket.message = aprsparser.message;
        parsedPacket.messageid = aprsparser.messageid;
        parsedPacket.PHG = aprsparser.phg;
        */
        parsedPacket.isValid = true;
        parsedPacket.error = res;
        if(aprsparser.format == F_MICE)
        {
            parsedPacket.message = aprsparser.messageType;
        }
        if(res != OK)
        {
            parsedPacket.isValid = false;
            parsedPacket.rawPacket = packet;
        }

        //qDebug() << res << " - " << packet;
        //DumpPacketInfo(parsedPacket);

        return parsedPacket;
        /*
        QRegExp regex;
        regex.setPattern("^([A-Z0-9-]{1,10})>([A-Z0-9-]{1,10}),([^:]*):(.)(.*)$");

        QStringList list;

        Packet parsedPacket;

        qDebug() << packet;

        if(packet.contains(regex))
        {
            list = regex.capturedTexts();

            parsedPacket.source = list[1];
            parsedPacket.destination = list[2];
            parsedPacket.path = list[3];
            parsedPacket.payload = list[5];

            QChar dataTypeChar = list[4][0];
            int dataTypeID = dataTypeChar.toAscii();

            switch(dataTypeID){
            case 123:
            case 125:
                // user defined and 3rd party
                break;
            case 39:
            case 96:
                parsedPacket = ParseMicEPacket(packet);
                break;
            case 36:
                // Raw GPS information
                break;
            case 58:  // message
            case 62:  // status
            case 63:  // query
            case 95:  // weather report without position
                // do something else with these.
                // they dont contain position information
                break;
            case 84:
                // Telemetry data
                break;
            default:
                parsedPacket = ParseAlmostEveryPacket(packet);
                if((fabs(parsedPacket.latitude) < 0.00001) && (fabs(parsedPacket.longitude) < 0.00001))
                {
                    parsedPacket = ParseCompressedPacket(packet, dataTypeID);
                }
                break;
            }
            if(CURRENT_ERROR_LEVEL == ErrorLevel::P)
            {
                DumpPacketInfo(parsedPacket);
            }

            switch(dataTypeID){
            // Projected Flow: ones with timestamp + position > ones with just position > ones without position > mic-e
            case 33:    // !
                qDebug() << "Position without timestamp (no APRS messaging), or Ultimeter 2000 WX Station";
                parsedPacket.latitude = ParseLatitude(packet);
                parsedPacket.longitude = ParseLongitude(packet);
                parsedPacket = ParseAlmostEveryPacket(packet);
                break;
            case 35:    // #
                qDebug() << "Peet Bros U-II Weather Station";
                break;
            case 36:    // $
                qDebug() << "Raw GPS data or Ultimeter 2000";
                break;
            case 37:    // %
                qDebug() << "Agrelo DFJr / MicroFinder";
                break;
            case 38:    // &
                qDebug() << "[Reserved - Map Feature]";
                break;
            case 39:    // '
                qDebug() << "Old Mic-E Data (but Current data for TM-D700)";
                parsedPacket = ParseMicEPacket(packet);
                break;
            case 41:    // )
                qDebug() << "Item";
                break;
            case 42:    // *
                qDebug() << "Peet Bros U-II Weather Station";
                break;
            case 43:    // +
                qDebug() << "[Reserved - Shelter data with time]";
                break;
            case 44:    // ,
                qDebug() << "Invalid data or test data";
                break;
            case 46:    // .
                qDebug() << "[Reserved - Space weather]";
                break;
            case 47:    // /
                qDebug() << "Position with timestamp (no APRS messaging)";
                parsedPacket.timestamp = ParseTimestamp(packet);
                break;
            case 58:    // :
                qDebug() << "Message";
                break;
            case 59:    // ;
                qDebug() << "Object";
                break;
            case 60:    // <
                qDebug() << "Station Capabilities";
                break;
            case 61:    // =
                qDebug() << "Position without timestamp (with APRS messaging)";
                break;
            case 62:    // >
                qDebug() << "Status";
                break;
            case 63:    // ?
                qDebug() << "Query";
                break;
            case 64:    // @
                qDebug() << "Position with timestamp (with APRS messaging)";
                parsedPacket.timestamp = ParseTimestamp(packet);
                break;
            case 84:    // T
                qDebug() << "Telemetry data";
                break;
            case 91:    // [
                qDebug() << "Maidenhead grid locator beacon (obsolete)";
                break;
            case 95:    // _
                qDebug() << "Weather Report (without position)";
                break;
            case 96:    // `
                qDebug() << "Current Mic-E Data (not used in TM-D700)";
                parsedPacket = ParseMicEPacket(packet);
                break;
            case 123:   // {
                qDebug() << "User-Defined APRS packet format";
                break;
            case 125:   // }
                qDebug() << "Third-party traffic";
                break;
            //cases 34, 40, 45, 48-57, 65-83, 85-90, 92-94, 97-122, 124 are unused.  ignore them
            // 25 types in use.
            default:
                qDebug() << "Invalid Packet!";
                break;
            }
    */
    }
    catch(...)
    {
        ErrorHandler::AlertAndLog("(parser.cpp) Error parsing raw packets");

    }
    Packet p;
    return p;
}

Packet parser::ParseAlmostEveryPacket(QString packet)
{
    Packet parsedPacket;

    // major problem with combining everything into one regex is that some things
    // are not necessarily always in the same order.  sometimes course comes first
    // sometimes altitude. this only applies to information kept after lat/long
    // however, splitting into multiple regexs makes it hard to catch info such as payload.

    QString pattern = "^([A-Z0-9-]{1,10})>([A-Z0-9-]{1,10}),([^:]*):(.)([^*]{9}[*_])?(\\d{6,8})?([zh/])?([0-9 ]{4}\\.[0-9 ]{2})?([NS])?(.)([0-9 ]{5}\\.[0-9 ]{2})?([EW])?(.)(\\d{3}/\\d{3})?(/A=\\d{6})?(.*)$";
    QRegExp regex;
    regex.setPattern(pattern);
    QStringList list;

    if(packet.contains(regex))
    {
        list = regex.capturedTexts();

        parsedPacket.source = list[1];
        parsedPacket.destination = list[2];
        parsedPacket.path = list[3];
        //parsedPacket.packetType = list[4];
        // TYPE ; INCLUDES TRACKED OBJECT ID:
        //parsedPacket.objectID = list[5];
        // TYPES @ AND / INCLUDE TIMESTAMP:
        parsedPacket.timestamp = ParseTimestamp(packet);
        parsedPacket.timestampType = list[7][0];
        // TYPES !, =, @, / INCLUDE POSITON INFO AND SYMBOL LOOKUP:
        parsedPacket.latitude = ParseLatitude(packet);
        parsedPacket.longitude = ParseLongitude(packet);
        if(list[14] != "")
        {
            QString courseAndSpeed = list[14];
            QStringList courseSpeed = courseAndSpeed.split('/');
            QString course = courseSpeed[0];
            QString speed = courseSpeed[1];
            parsedPacket.course = course.toInt();
            parsedPacket.speed = speed.toInt();
        }
        if(list[15] != "")
        {
            QString altitude = list[15];
            altitude.remove(0, 3);
            parsedPacket.altitude = altitude.toInt();
        }
        parsedPacket.payload = list[16];
        parsedPacket.symbolID = GetSymbolID(list[10], list[13]);
        if((list[10][0] != '\\') && (list[10][0] != '/'))
        {
            parsedPacket.symbolOverlay = list[10][0];
        }
    }

    return parsedPacket;
}

qreal parser::ParseLatitude(QString packet)
{
    QString pattern = "([0-9 ]{4}\\.[0-9 ]{2})([NnSs])";
    QRegExp regex;
    regex.setPattern(pattern);
    QStringList list;
    qreal latitude;
    if(packet.contains(regex))
    {
        list = regex.capturedTexts();

        QVariant blah = list[1];
        latitude = blah.toDouble();

        if((list[2][0] == 'S') || (list[2][0] == 's'))
        {
            latitude = latitude * (-1);
        }
    }

    return latitude;
}

qreal parser::ParseLongitude(QString packet)
{
    QString pattern = "([0-9 ]{4,5}\\.[0-9 ]{2})([EeWw])";
    QRegExp regex;
    regex.setPattern(pattern);
    QStringList list;
    qreal longitude;
    if(packet.contains(regex))
    {
        list = regex.capturedTexts();

        QVariant blah = list[1];
        longitude = blah.toDouble();

        if((list[2][0] == 'W') || (list[2][0] == 'w'))
        {
            longitude = longitude * (-1);
        }
    }

    return longitude;
}

Packet parser::ParseCompressedPacket(QString packet, int dataTypeID)
{
    Packet parsedPacket;

    QString pattern;
    QRegExp regex;
    QStringList list;

    // Sample:        "LX9SES>APU25N,WIDE3-3,qAR,DB0XIT:=/5B{7P)XI`  BClubstation S.E.S. Betzder"
    if(dataTypeID == 59)
    {
        //([^*_]{9}[*_])?
        pattern = "^([A-Z0-9-]{1,10})>([A-Z0-9-]{1,10}),([^:]*):(.)(^[*_]{9}[*_])";
        regex.setPattern(pattern);

        QString objectTest = packet;

        if(objectTest.contains(regex))
        {
            list = regex.capturedTexts();

            parsedPacket.source = list[1];
            parsedPacket.destination = list[2];
            parsedPacket.path = list[3];
            //parsedPacket.packetType = list[4];
            //parsedPacket.objectID = list[5];

            objectTest.remove(regex);

            pattern = "(\\d{6,8})?([zh])?(.)(.{4})(.{4})(.)(.{2})(.)(.*)$";
            regex.setPattern(pattern);

            if(objectTest.contains(regex))
            {
                parsedPacket.timestamp = ParseTimestamp(packet);
                parsedPacket.timestampType = list[2][0];
                parsedPacket.symbolID = GetSymbolID(list[3], list[6]);
                parsedPacket.latitude =  (((ConvertFromBase91(list[4]) / 380926.0) - 90) * -100);
                parsedPacket.longitude = (((ConvertFromBase91(list[5]) / 190463.0) - 180) * 100);
                parsedPacket.payload = list[9];
            }
        }
    }
    else
    {
        pattern = "^([A-Z0-9-]{1,10})>([A-Z0-9-]{1,10}),([^:]*):(.)(\\d{6,8})?([zh])?(.)(.{4})(.{4})(.)(.{2})(.)(.*)$";
        regex.setPattern(pattern);

        if(packet.contains(regex))
        {
            list = regex.capturedTexts();

            parsedPacket.source = list[1];
            parsedPacket.destination = list[2];
            parsedPacket.path = list[3];
            //parsedPacket.packetType = list[4];
            parsedPacket.timestamp = ParseTimestamp(packet);
            parsedPacket.timestampType = list[5][0];
            parsedPacket.symbolID = GetSymbolID(list[7], list[10]);
            parsedPacket.latitude =  (((ConvertFromBase91(list[8]) / 380926.0) - 90) * -100);
            parsedPacket.longitude = (((ConvertFromBase91(list[9]) / 190463.0) - 180) * 100);
            parsedPacket.payload = list[13];
            qDebug() << "Compressed Lat:    " << list[9];
            qDebug() << "Decompressed Lat:  " << parsedPacket.latitude;
            qDebug() << "Compressed Long:   " << list[10];
            qDebug() << "Decompressed Long: " << parsedPacket.longitude;
        }
    }

    return parsedPacket;
}

QDateTime parser::ParseTimestamp(QString packet)
{
    QString pattern = "^([A-Z0-9-]{1,10})>([A-Z0-9-]{1,10}),([^:]*):(.)([^*]{9}[*_])?(\\d{6,8})([zh/])?";
    QRegExp regex;
    regex.setPattern(pattern);
    QStringList list;

    QDateTime timestamp = QDateTime::currentDateTime();

    if(packet.contains(regex))
    {
        list = regex.capturedTexts();
        QString packetTime = list[6];

        if((list[7] == "z") || (list[7] == "/"))
        {
            packetTime.prepend(QString::number(QDate::currentDate().month()));
            packetTime.prepend(QString::number(QDate::currentDate().year()));
            timestamp = timestamp.fromString(packetTime, "yyyyMMddHHmm");
        }
        else if(list[7] == "h")
        {
            packetTime.prepend(QString::number(QDate::currentDate().day()));
            packetTime.prepend(QString::number(QDate::currentDate().month()));
            packetTime.prepend(QString::number(QDate::currentDate().year()));
            timestamp = timestamp.fromString(packetTime, "yyyyMMddHHmmss");
        }
        else if(list[4] == "_")
        {
            packetTime.prepend(QString::number(QDate::currentDate().year()));
            timestamp = timestamp.fromString(packetTime, "yyyyMMddHHmm");
        }
    }

    return timestamp;
}

QString parser::ParseWeatherData(QString packet)
{
    QString pattern = "\\d{3}/\\d{3}g([0-9. ]{3})t([0-9. ]{3})r([0-9. ]{3})p([0-9. ]{3})P([0-9. ]{3})h([0-9. ]{2})b([0-9. ]{5})";
    QRegExp regex;
    regex.setPattern(pattern);
    QStringList list = (QStringList() << "" << "" << "" << "" << "" << "" << "");
    if(packet.contains(regex))
    {
        list = regex.capturedTexts();
    }

    return list[0];
}

qint32 parser::GetSymbolID(QString tableIdentifier, QString symbolCode)
{
    qint32 returnVal;

    if(tableIdentifier == "/")
    {
        returnVal = symbolCode[0].toAscii() - 33;
    }
    else
    {
        returnVal = symbolCode[0].toAscii() - 33 + 96;
    }

    return returnVal;
}

QString parser::GetLookupCharsFromID(int symbolID)
{
    QString returnVal = "";
    if(symbolID < 96)
    {
        returnVal.prepend("/");
        returnVal.append(char(symbolID + 33));
    }
    else
    {
        if(overlayAllowed(symbolID))
        {
            returnVal.prepend(Profile::getOverlayChar());
        }
        else
        {
            returnVal.prepend("\\");
        }
        returnVal.append(char(symbolID + 33 - 96));
    }
    return returnVal;
}

Packet parser::ParseMicEPacket(QString packet)
{
    // Mic-E packet:  "W6TUI>SV4VQX,WA6YLB-7*,WIDE2-2,qAR,KC6YRU:`/JWl >k/]"4g}"
    // Mic-E packets encode the latitude and a message type in the destination.
    // In this case, SV4VQX is not a callsign, but Mic-E encoded data.
    // Latitude and payload are encoded in the /JWl >k/]"4g} part of the packet.
    // For more info on how Mic-E encoding works, see pae 52 of the APRS101 PDF

    Packet parsedPacket;

    QString pattern = "^([A-Z0-9-]{1,10})>([A-Z0-9-]{1,10}),([^:]*):(.)(.*)$";
    QRegExp regex;
    regex.setPattern(pattern);
    QStringList list;
    QString latitude = "";
    QString longitude = "";
    QString speed = "";
    QString course = "";
    QString messageType = "";
    //bool isCustom;
    bool error;
    bool isLatNeg;
    bool isLongNeg;
    qint32 longOffset;
    qint32 digit;
    qint32 speedNumber;
    qint32 courseNumber;
    qreal latNumber;
    qreal longNumber;

    if(packet.contains(regex))
    {
        list = regex.capturedTexts();

        QString firstPart = list[2]; // destination/latitude
        QString secondPart = list[5]; // long/payload

        // LATITUDE CALCULATIONS FOLLOW:
        for(int i = 0; i < 6; i++)
        {
            if(i == 4)
            {
                latitude.append('.');
            }
            if(((firstPart[i] >= '0') && (firstPart[i] <= '9')) || (firstPart[i] == 'L'))
            {
                if(i < 3)
                {
                   messageType.append("0");
                }
                else if(i == 3)
                {
                    isLatNeg = true;
                }
                else if(i == 4)
                {
                    longOffset = 0;
                }
                else if(i == 5)
                {
                    isLongNeg = false;
                }

                if(firstPart[i] != 'L')
                {
                    //qDebug() << firstPart[i];
                    latitude.append(firstPart[i]);
                }
                else
                {
                    //qDebug() << '0';
                    latitude.append('0'); // L is " ", an ambiguous location
                }
            }
            else if((firstPart[i] >= 'A') && (firstPart[i] <= 'K'))
            {
                if(i < 3)
                {
                    messageType.append("2"); // custom message
                }
                else
                {
                    error = true;
                }
                digit = (firstPart[i].toAscii() - 65) % 10;
                latitude.append(QString::number(digit));  // K is " ", an ambiguous location
            }
            else if((firstPart[i] >= 'P') && (firstPart[i] <= 'Z'))
            {
                if(i < 3)
                {
                    messageType.append("1"); // standard message
                }
                else if(i == 3)
                {
                    isLatNeg = false;
                }
                else if(i == 4)
                {
                    longOffset = 100;
                }
                else if(i == 5)
                {
                    isLongNeg = true;
                }

                digit = (firstPart[i].toAscii() - 80) % 10;
                latitude.append(QString::number(digit));  // Z is " ", an ambiguous location
            }
        }

        latNumber = latitude.toDouble();
        if(isLatNeg)
        {
            latNumber *= (-1);
        }

        int messageNum = messageType.toInt();
        switch(messageNum)
        {
        case 001:
            messageType = "M6: Priority";
            break;
        case 010:
            messageType = "M5: Special";
            break;
        case 011:
            messageType = "M4: Committed";
            break;
        case 100:
            messageType = "M3: Returning";
            break;
        case 101:
            messageType = "M2: In Service";
            break;
        case 110:
            messageType = "M1: En Route";
            break;
        case 111:
            messageType = "M0: Off-Duty";
            break;
        case 002:
            messageType = "C6: Custom-6";
            break;
        case 020:
            messageType = "C5: Custom-5";
            break;
        case 022:
            messageType = "C4: Custom-4";
            break;
        case 200:
            messageType = "C3: Custom-3";
            break;
        case 202:
            messageType = "C2: Custom-2";
            break;
        case 220:
            messageType = "C1: Custom-1";
            break;
        case 222:
            messageType = "C0: Custom-0";
            break;
        case 000:
            messageType = "Emergency";
            break;
        default:
            messageType = "Invalid Message Format";
            break;
        }

        // LONGITUDE CALCULATIONS FOLLOW:
        for(int j = 0; j < 3; j++)
        {
            digit = secondPart[j].toAscii() - 28;
            if(j == 0)
            {
                digit += longOffset;
                if((180 <= digit) && (digit <= 189))
                {
                    digit -= 80;
                }
                else if((190 <= digit) && (digit < 199))
                {
                    digit -= 190;
                }
            }
            else if(j == 1)
            {
                if(digit >= 60)
                {
                    digit -= 60;
                }
            }
            else
            {
                longitude.append('.');
            }

            if(digit < 10)
            {
                longitude.append('0');
            }

            longitude.append(QString::number(digit));
        }

        longNumber = longitude.toDouble();
        if(isLongNeg)
        {
            longNumber *= (-1);
        }

        // SPEED AND COURSE CALCULATIONS FOLLOW:
        for(int k = 3; k < 6; k++)
        {
            if(k == 3)
            {
                digit = secondPart[k].toAscii() - 28;

                speed.append(QString::number(digit));
            }
            else if(k == 4)
            {
                digit = secondPart[k].toAscii() - 28;
                int otherdigit = digit / 10;
                speed.append(QString::number(otherdigit));
                otherdigit = digit % 10;
                course.append(QString::number(otherdigit));
            }
            else
            {
                digit = secondPart[k].toAscii() - 28;
                course.append(QString::number(digit));
            }
            //qDebug() << "course/speed digit" << k << ": " << secondPart[k];
        }

        speedNumber = speed.toDouble();
        if(speedNumber >= 800)
        {
            speedNumber -= 800;
        }

        courseNumber = course.toDouble();
        if(courseNumber >= 400)
        {
            courseNumber -= 400;
        }

        parsedPacket.symbolID = GetSymbolID(QString(secondPart[7]), QString(secondPart[6]));
        if((secondPart[7] != '/') && (secondPart[7] != '\\'))
        {
            parsedPacket.symbolOverlay = secondPart[7];
        }

        // if secondPart[8] == '`' or '\'' then 5 chars of telemetry data (in hex)
        // else if secondPart[8] == '>' or ']' then "status" (for Kenwood radios)
        // presumably the status message isnt encoded and is just plain text
        secondPart.remove(0, 8);

        QRegExp altitudeRegex;
        QString altitudePattern = "(.{3})\\}";
        altitudeRegex.setPattern(altitudePattern);
        if(secondPart.contains(altitudeRegex))
        {
            QStringList altitudeList = altitudeRegex.capturedTexts();

            parsedPacket.altitude = (ConvertFromBase91(altitudeList[1]) - 10000);
            secondPart.remove(altitudeRegex);
        }

        parsedPacket.payload = secondPart;
    }

    parsedPacket.source = list[1];
    parsedPacket.path = list[3];
    //parsedPacket.packetType = "Mic-E Data";
    //parsedPacket.isMicE = true;
    parsedPacket.latitude = latNumber;
    parsedPacket.longitude = longNumber;
    parsedPacket.speed = speedNumber;
    parsedPacket.course = courseNumber;
    parsedPacket.message = messageType;

    return parsedPacket;
}

void parser::DumpPacketInfo(Packet dumpMe)
{
    qDebug() << "Source:           " << dumpMe.source;
    qDebug() << "Destination:      " << dumpMe.destination;
    qDebug() << "Path:             " << dumpMe.path;
    qDebug() << "Data:             " << dumpMe.data;
    qDebug() << "Packet Type:      " << dumpMe.packetType;
    qDebug() << "Payload:          " << dumpMe.payload;
    qDebug() << "Symbol ID:        " << dumpMe.symbolID;
    qDebug() << "Symbol Overlay:   " << dumpMe.symbolOverlay;
    qDebug() << "Latitude:         " << dumpMe.latitude;
    qDebug() << "Longitude:        " << dumpMe.longitude;
    qDebug() << "Timestamp:        " << dumpMe.timestamp;
    qDebug() << "Timestamp Type:   " << dumpMe.timestampType;
    qDebug() << "Valid?            " << dumpMe.isValid;
    qDebug() << "Mic-E Msg Type:   " << dumpMe.message;
    qDebug() << "Altitude:         " << dumpMe.altitude;
    qDebug() << "Course:           " << dumpMe.course;
    qDebug() << "Speed:            " << dumpMe.speed;
    qDebug() << "Temp:             " << dumpMe.temp;
    qDebug() << "Pressure:         " << dumpMe.pressure;
    qDebug() << "Wind Speed:       " << dumpMe.wind_speed;
    qDebug() << "Wind Direction:   " << dumpMe.wind_direction;
    qDebug() << "Wind Gust:        " << dumpMe.wind_gust;
    qDebug() << "Rain 1h:          " << dumpMe.rain_1h;
    qDebug() << "Rain 24h:         " << dumpMe.rain_24h;
    qDebug() << "Rain Midnight:    " << dumpMe.rain_midnight;
    qDebug() << "Snow 24h:         " << dumpMe.snow_24h;
    qDebug() << "Rain Count:       " << dumpMe.raincount;
    qDebug() << "Humidity:         " << dumpMe.humidity;
    qDebug() << "Luminosity:       " << dumpMe.luminosity;
    qDebug() << "Alive:            " << dumpMe.alive;
    qDebug() << "Error:            " << dumpMe.error;
    qDebug() << "Raw Packet:       " << dumpMe.rawPacket;
}
/*
    int wind_direction;
    float wind_gust;
    float rain_1h;
    float rain_24h;
    float rain_midnight;
    float snow_24h;
    int raincount;
    int humidity;
    int luminosity;
    bool alive;
    result error;
    QString rawPacket;


This packet plots with no callsign

"IT9FKM>APU25N,TCPIP*,qAS,it9fkm:;IR9AX-R7a*221919z3809.22N/01321.20EmR7a Palermo - Monte Pellegrino - T.94.8"
59
Source:            "IT9FKM"
Destination:       "APU25N"
Path:              "TCPIP*,qAS,it9fkm"
Object ID:         "IR9AX-R7a*"
Packet Type:       ";"
Payload:           "R7a Palermo - Monte Pellegrino - T.94.8"
Symbol ID:         76
Symbol Overlay:    ""
Latitude:          3809.22
Longitude:         1321.2
Timestamp:         QDateTime("Tue Mar 22 19:19:00 2011")
Timestamp Type:    'z'
Mic-E?             false
Mic-E Msg Type:    ""
Altitude:          0
Course:            0
Speed:             0
Bearing:           0
NRG:               0

Why does it think this packet is compressed?

"CX4AE>APU25N,TCPIP*,qAC,iURUGUAY:=3453.36S/05607.85W-Montevideo, Uruguay {UIV32N}"
Compressed Lat:     "36S/"
Decompressed Lat:   5197.39
Compressed Long:    "0"
Decompressed Long:  -10784.6
Source:            "CX4AE"
Destination:       "APU25N"
Path:              "TCPIP*,qAC,iURUGUAY"
Object ID:         ""
Packet Type:       "="
Payload:           "7.85W-Montevideo, Uruguay {UIV32N}"
Symbol ID:         143
Symbol Overlay:    ""
Latitude:          5197.39
Longitude:         -10784.6
Timestamp:         QDateTime("Thu Mar 10 20:14:48 2011")
Timestamp Type:    '
Mic-E?             false
Mic-E Msg Type:    ""
Altitude:          0
Course:            0
Speed:             0
Bearing:           0
NRG:               0

*/
