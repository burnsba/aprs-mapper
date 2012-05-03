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

#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <QString>
#include <QFile>
#include <QDateTime>

class ErrorHandler
{

public:
    static void Start();
    static void End();
    static void Alert(QString);
    static void AlertAndLog(QString value);
    static void Log(QString value);
    static QString getLogFile();
    static void setLogFile(QString);
    static void msgBox(QString);

private:
    static QFile mFile;

};

#endif // ERRORHANDLER_H
