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

#ifndef PARSER_H
#define PARSER_H

#include <QObject>
#include <QDateTime>
#include "packettypes.h"
#include "aprs_parser.h"

class parser : public QObject
{
    Q_OBJECT
public:
    explicit parser(QObject *parent = 0);
    Packet ParsePacket(QString packet);
    static void DumpPacketInfo(Packet dumpMe);
    static QString GetLookupCharsFromID(int symbolID);
    static qint32 GetSymbolID(QString tableIdentifier, QString symbolCode);
private:
    QString ConvertToBase91(qint32 input);
    qint32 ConvertFromBase91(QString input);
    Packet ParseAlmostEveryPacket(QString packet);
    qreal ParseLatitude(QString packet);
    qreal ParseLongitude(QString packet);
    Packet ParseMicEPacket(QString packet);
    QDateTime ParseTimestamp(QString packet);
    QString ParseWeatherData(QString packet);


    Packet ParseCompressedPacket(QString packet, int dataTypeID);

signals:

public slots:

};

#endif // PARSER_H
