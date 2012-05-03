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

// This file is supposed to be used to log error messages

#include <QtCore/QCoreApplication>
#include <QDebug>
#include <QMessageBox>
#include <iostream>

#include "profile.h"
#include "errorhandler.h"

QFile ErrorHandler::mFile;

void ErrorHandler::Start()
{
    try
    {
        //setLogFile("error_log_default.txt");  // this can be changed later, from the user profile
        setLogFile(Profile::getErrorFile());
    }
    catch(...)
    {
        ErrorHandler::msgBox("There was an error starting the error logger.");
    }
}

void ErrorHandler::End()
{
    try
    {
        mFile.close();
    }
    catch(...)
    {
        ErrorHandler::msgBox("There was an error shutting down the error logger.");
    }
}

QString ErrorHandler::getLogFile()
{
    return mFile.fileName();
}

void ErrorHandler::setLogFile(QString fileName)
{
    try
    {
        if (mFile.isOpen())
        {
            mFile.close();
        }
        mFile.setFileName(fileName);
        Profile::setErrorFile(fileName);
        mFile.open((QFile::WriteOnly | QFile::Text));
    }
    catch(...)
    {
        ErrorHandler::msgBox("There was an error setting the error log file: " + Profile::getErrorFile());
    }
}

void ErrorHandler::Log(QString value)
{
    try
    {
        QString toWrite = "";
        toWrite += QDateTime::currentDateTime().toString();
        toWrite += ": ";
        toWrite += value;
        toWrite += "\n";

        mFile.write(toWrite.toLocal8Bit().constData());
        qDebug() << "ErrorHandler: " + toWrite;
    }
    catch(...)
    {
        ErrorHandler::msgBox("There was an error writing to the error log file: " + Profile::getErrorFile());
    }
}


void ErrorHandler::AlertAndLog(QString value)
{
    Alert(value);
    Log(value);
}

void ErrorHandler::Alert(QString text)
{
    try
    {
        QMessageBox m_box;
        m_box.setTextFormat(Qt::RichText);
        m_box.setText(text);
        m_box.exec();
    }
    catch(...)
    {
        // not much to do here ...
    }
}

void ErrorHandler::msgBox(QString text)
{
    try
    {
        QMessageBox m_box;
        m_box.setTextFormat(Qt::RichText);
        m_box.setText(text);
        m_box.exec();
    }
    catch(...)
    {
        // not much to do here ...
    }
}
