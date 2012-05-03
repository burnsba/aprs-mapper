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

#ifndef PACKETTYPES_H
#define PACKETTYPES_H

#include <QString>
#include <QDateTime>
#include "aprs_parser.h"

struct Packet{
    Packet():source(""),destination(""),path(""),data(""),packetType(T_UNKNOWN),payload(""),symbolID(0),symbolOverlay(""),latitude(0),longitude(0), \
        rcvdTime(QDateTime::currentDateTime()),timestamp(QDateTime::currentDateTime()),timestampType('z'),isValid(false),message(""), \
        altitude(0.0),course(0),speed(0.0),temp(0.0),pressure(0.0),wind_speed(0.0),wind_direction(0),wind_gust(0.0),rain_1h(0.0),rain_24h(0.0), \
        rain_midnight(0.0),snow_24h(0.0),raincount(0),humidity(0),luminosity(0),alive(true),error(OK),rawPacket(""){}
    QString source;
    QString destination;
    QString path;
    QString data;
    aprstype packetType;
    QString payload;
    qint32 symbolID;
    QString symbolOverlay;
    qreal latitude;
    qreal longitude;
    QDateTime rcvdTime;
    QDateTime timestamp;
    QString timestampType;
    bool isValid;
    QString message;
    float altitude;
    int course;
    float speed;
    float temp;
    float pressure;
    float wind_speed;
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
};



//functions to convert from APRS lat/lon to deg/min/sec
//clarification example: position = -4726.23
inline int ToDegrees(qreal position) { return (int)(position/100); } //move the decimal to make it -47.2623 convert it to an int to make it -47

inline int ToMinutes(qreal position) {
                                        int minutes = (int)position % 100; //take the modulus of 100 to make it -26
                                        if((minutes >= 0) ? minutes : -(minutes)){} //negatives not allowed- take the absolute value
                                        return minutes; //return 26
                                     }

inline int ToSeconds(qreal position) {
                                        int seconds = (int)(position*100) % 100; //multiply by 100 to make it -472623 then take modulus 100 to make it -23
                                        if((seconds >= 0) ? seconds : -(seconds)){} //negatives are not allowed- take the absolute value
                                        return seconds; //return 23
                                     }
//clarification example success: result is -47 degrees, 26 minutes, 23 seconds




#endif // PACKETTYPES_H
