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

#include "profile.h"
#include "createnew.h"
#include <QDebug>


QString Profile::loadProf;
QFile   Profile::profileFile;
QString Profile::profName;
QString Profile::errorMsg;
QString Profile::result;
qint32  Profile::errorLine;
qint32  Profile::errorColumn;
QString Profile::callSign;
QString Profile::SSID;
qint32  Profile::mIconId;
QString Profile::mOverlayChar;
QString Profile::posComment;
qreal   Profile::mLatitude;
qreal   Profile::mLongitude;
QString Profile::mErrorFile;
QString Profile::cServerAddress;
qint32  Profile::cServerPort;
QString Profile::connectStr;
bool    Profile::cSendReport;
qint32  Profile::mMapZoom;
qreal   Profile::mMapLat;
qreal   Profile::mMapLong;
QString Profile::connectionPass;
int  Profile::purgeTime;
int  Profile::clearTime;
qint32  Profile::posIntervalTime;
qint32  Profile::mOnlineMode;
QString Profile::mTileCacheDir;
QString Profile::mTileServerUrl;
QString Profile::currDB;
QString Profile::mysqlUser;
QString Profile::mysqlPass;
QString Profile::mysqlDBName;
QString Profile::mysqlIPAddr;
qint32 Profile::mysqlPort;
QString Profile::sqliteFilepath;
QString Profile::pUnits;



void Profile::readXML( QString profileName, QDomDocument profileDoc )
{

    QDomElement settings = profileDoc.namedItem( "settings" ).toElement();
    if ( settings.isNull() )
    {
        qDebug() << "No <settings> element found at the top-level "
                << "of the XML file!";
    }

    if ( profileName == "last" )
    {
        QDomElement lastUsedName = settings.namedItem( "lastused" ).toElement();
        if ( lastUsedName.isNull() )
            qDebug() << "Cannot retrieve previous settings.";
        profileName = lastUsedName.text();
    }

    QDomElement profile = settings.firstChildElement( "profile" );

    if(profile.isNull())
    {
        createnew* n = new createnew();
        n->show();
    }

    for ( ; !profile.isNull(); profile = profile.nextSiblingElement( "profile" ) )
    {
        QDomElement v = profile.namedItem( "profName" ).toElement();
        if ( v.text() == profileName )
        {
            profName = v.text();
            v = profile.namedItem( "callSign" ).toElement();
            callSign = v.text();
            v = profile.namedItem( "SSID" ).toElement();
            SSID = v.text();
            v = profile.namedItem( "mIconId" ).toElement();
            mIconId = v.text().toInt();
            v = profile.namedItem( "mOverlayChar" ).toElement();
            mOverlayChar = v.text();
            v = profile.namedItem( "posComment" ).toElement();
            posComment = v.text();
            v = profile.namedItem( "pUnits" ).toElement();
            pUnits = v.text();
            v = profile.namedItem( "mLat" ).toElement();
            mLatitude = v.text().toFloat();
            v = profile.namedItem( "mLong" ).toElement();
            mLongitude = v.text().toFloat();
            v = profile.namedItem( "connectionPass" ).toElement();
            connectionPass = v.text();
            v = profile.namedItem( "currDB" ).toElement();
            currDB = v.text();
            v = profile.namedItem( "mysqlUser" ).toElement();
            mysqlUser = v.text();
            v = profile.namedItem( "mysqlPass" ).toElement();
            mysqlPass = v.text();
            v = profile.namedItem( "mysqlDBName" ).toElement();
            mysqlDBName = v.text();
            v = profile.namedItem( "mysqlIPAddr" ).toElement();
            mysqlIPAddr = v.text();
            v = profile.namedItem( "mysqlPort" ).toElement();
            mysqlPort = v.text().toInt();
            v = profile.namedItem( "sqliteFilepath" ).toElement();
            sqliteFilepath = v.text();
            v = profile.namedItem( "purgeTime" ).toElement();
            purgeTime = v.text().toInt(NULL);
            v = profile.namedItem( "clearTime" ).toElement();
            clearTime = v.text().toInt(NULL);
            v = profile.namedItem( "posIntervalTime" ).toElement();
            posIntervalTime = v.text().toInt();
            v = profile.namedItem( "mMapZoom" ).toElement();
            mMapZoom = v.text().toInt();
            v = profile.namedItem( "mMapLat" ).toElement();
            mMapLat = v.text().toFloat();
            v = profile.namedItem( "mMapLong" ).toElement();
            mMapLong = v.text().toFloat();
            v = profile.namedItem( "mOnlineMode").toElement();
            mOnlineMode = v.text().toInt();
            v = profile.namedItem( "mTileCacheDir" ).toElement();
            mTileCacheDir = v.text();
            v = profile.namedItem( "mTileServerUrl").toElement();
            mTileServerUrl = v.text();
            break;
        }
    }
}


QDomElement Profile::addElement( QDomDocument &profileDoc, QDomNode &node,
                        const QString tag,
                        const QString value)
{
    QDomElement newElement = profileDoc.createElement( tag );
    node.appendChild( newElement );
    if ( !value.isNull() )
    {
        QDomText txt = profileDoc.createTextNode( value );
        newElement.appendChild( txt );
    }
    return newElement;
}

void Profile::saveElement( QDomDocument &profileDoc, QDomElement &elemToChange,
                           const QString tag, const QString value)
{

    QDomElement oldElement = elemToChange.namedItem( tag ).toElement();
    QDomElement newElement = profileDoc.createElement( tag );
    if ( !value.isNull() )
    {
        QDomText txt = profileDoc.createTextNode( value );
        newElement.appendChild( txt );
    }
    elemToChange.replaceChild(newElement, oldElement);

}

void Profile::loadProfile()
{
    QDomDocument profileDoc;
    profileFile.setFileName("resource/profile.xml");
    if ( profileDoc.setContent( &profileFile, &errorMsg, &errorLine, &errorColumn ) )
    {
        readXML( "last", profileDoc );
    }
    profileFile.close();
}

void Profile::loadProfile(QString profileName)
{
    QDomDocument profileDoc;
    profileFile.setFileName("resource/profile.xml");
    if ( profileDoc.setContent( &profileFile, &errorMsg, &errorLine, &errorColumn ) )
    {
        readXML( profileName, profileDoc );
    }
    profileFile.close();
}

void Profile::saveProfile()
{
    QDomDocument profileDoc;
    profileFile.setFileName("resource/profile.xml");
    if ( profileDoc.setContent( &profileFile, &errorMsg, &errorLine, &errorColumn ) )
    {
        QDomElement settings = profileDoc.namedItem( "settings" ).toElement();
        QDomElement profile = settings.firstChildElement( "profile" );
        for ( ; !profile.isNull(); profile = profile.nextSiblingElement( "profile" ) )
        {
            QDomElement v = profile.namedItem( "profName" ).toElement();
            if ( v.text() == profName )
            {
                saveElement( profileDoc, profile, "callSign", callSign );
                saveElement( profileDoc, profile, "SSID", SSID );
                saveElement( profileDoc, profile, "mOverlayChar", mOverlayChar);
                saveElement( profileDoc, profile, "mIconId", QString::number(mIconId));
                saveElement( profileDoc, profile, "mLat", QString::number(mLatitude) );
                saveElement( profileDoc, profile, "mLong", QString::number(mLongitude) );
                saveElement( profileDoc, profile, "connectionPass", connectionPass );
                saveElement( profileDoc, profile, "posComment", posComment );
                saveElement( profileDoc, profile, "pUnits", pUnits );
                saveElement( profileDoc, profile, "currDB", currDB );
                saveElement( profileDoc, profile, "mysqlUser", mysqlUser );
                saveElement( profileDoc, profile, "mysqlPass", mysqlPass );
                saveElement( profileDoc, profile, "mysqlDBName", mysqlDBName );
                saveElement( profileDoc, profile, "mysqlIPAddr", mysqlIPAddr );
                saveElement( profileDoc, profile, "mysqlPort", QString::number(mysqlPort) );
                saveElement( profileDoc, profile, "sqliteFilepath", sqliteFilepath );
                saveElement( profileDoc, profile, "purgeTime", QString::number(purgeTime) );
                saveElement( profileDoc, profile, "clearTime", QString::number(clearTime) );
                saveElement( profileDoc, profile, "posIntervalTime", QString::number(posIntervalTime) );
                saveElement( profileDoc, profile, "mMapZoom", QString::number(mMapZoom) );
                saveElement( profileDoc, profile, "mMapLat", QString::number(mMapLat) );
                saveElement( profileDoc, profile, "mMapLong", QString::number(mMapLong) );
                saveElement( profileDoc, profile, "mOnlineMode", QString::number(mOnlineMode)) ;
                saveElement( profileDoc, profile, "mTileServerUrl", mTileServerUrl);
                saveElement( profileDoc, profile, "mTileCacheDir", mTileCacheDir);

                profileFile.close();
                profileFile.open(QIODevice::Truncate | QIODevice::Text | QIODevice::ReadWrite);
                QTextStream out(&profileFile);

                out << profileDoc.toString();
                break;
            }
        }
    }

    profileFile.close();

}

void Profile::saveProfile(QString profileName)
{
    profName = profileName;
    QDomDocument profileDoc;
    profileFile.setFileName("resource/profile.xml");
    if ( profileDoc.setContent( &profileFile, &errorMsg, &errorLine, &errorColumn ) )
    {
        QDomElement settings = profileDoc.namedItem( "settings" ).toElement();
        QDomElement newProfile = addElement( profileDoc, settings, "profile", QString::null );
        addElement( profileDoc, newProfile, "profName", profName );
        addElement( profileDoc, newProfile, "callSign", callSign );
        addElement( profileDoc, newProfile, "SSID", SSID );
        addElement( profileDoc, newProfile, "mIconId", QString::number(mIconId) );
        addElement( profileDoc, newProfile, "mOverlayChar", mOverlayChar );
        addElement( profileDoc, newProfile, "mLat", QString::number(mLatitude) );
        addElement( profileDoc, newProfile, "mLong", QString::number(mLongitude) );
        addElement( profileDoc, newProfile, "connectionPass", connectionPass );
        addElement( profileDoc, newProfile, "posComment", posComment );
        addElement( profileDoc, newProfile, "pUnits", pUnits );
        addElement( profileDoc, newProfile, "currDB", currDB );
        addElement( profileDoc, newProfile, "mysqlUser", mysqlUser );
        addElement( profileDoc, newProfile, "mysqlPass", mysqlPass );
        addElement( profileDoc, newProfile, "mysqlDBName", mysqlDBName );
        addElement( profileDoc, newProfile, "mysqlIPAddr", mysqlIPAddr );
        addElement( profileDoc, newProfile, "mysqlPort", QString::number(mysqlPort) );
        addElement( profileDoc, newProfile, "sqliteFilepath", sqliteFilepath );
        addElement( profileDoc, newProfile, "purgeTime", QString::number(purgeTime) );
        addElement( profileDoc, newProfile, "clearTime", QString::number(clearTime) );
        addElement( profileDoc, newProfile, "posIntervalTime", QString::number(posIntervalTime) );
        addElement( profileDoc, newProfile, "mMapZoom", QString::number(mMapZoom) );
        addElement( profileDoc, newProfile, "mMapLat", QString::number(mMapLat) );
        addElement( profileDoc, newProfile, "mMapLong", QString::number(mMapLong) );
        addElement( profileDoc, newProfile, "mOnlineMode", QString::number(mOnlineMode)) ;
        addElement( profileDoc, newProfile, "mTileServerUrl", mTileServerUrl);
        addElement( profileDoc, newProfile, "mTileCacheDir", mTileCacheDir);

        profileFile.close();
        profileFile.open(QIODevice::Truncate | QIODevice::Text | QIODevice::ReadWrite);
        QTextStream out(&profileFile);

        out << profileDoc.toString();
    }

    profileFile.close();
}

void Profile::removeProfile(QString profileName)
{
    QDomDocument profileDoc;
    profileFile.setFileName("resource/profile.xml");
    if ( profileDoc.setContent( &profileFile, &errorMsg, &errorLine, &errorColumn ) )
    {
        QDomElement settings = profileDoc.namedItem( "settings" ).toElement();
        QDomElement profile = settings.firstChildElement( "profile" );
        for ( ; !profile.isNull(); profile = profile.nextSiblingElement( "profile" ) )
        {
            QDomElement v = profile.namedItem( "profName" ).toElement();
            if ( v.text() == profileName )
            {
                settings.removeChild(profile);

                profileFile.close();
                profileFile.open(QIODevice::Truncate | QIODevice::Text | QIODevice::ReadWrite);
                QTextStream out(&profileFile);

                out << profileDoc.toString();
                break;
            }
        }
    }

    profileFile.close();

}

void Profile::saveConnection(QString alias, QString newalias, QString address, QString port, QString cstring, bool startup, bool sendreport, bool authconn)
{
    QDomDocument profileDoc;
    profileFile.setFileName("resource/profile.xml");
    if ( profileDoc.setContent( &profileFile, &errorMsg, &errorLine, &errorColumn ) )
    {
        QDomElement settings = profileDoc.namedItem( "settings" ).toElement();
        QDomElement profile = settings.firstChildElement( "profile" );
        for ( ; !profile.isNull(); profile = profile.nextSiblingElement( "profile" ) )
        {
            QDomElement v = profile.namedItem( "profName" ).toElement();
            if ( v.text() == profName )
            {
                QDomElement connection = profile.firstChildElement("connection");
                for ( ; !connection.isNull(); connection = connection.nextSiblingElement( "connection" ) )
                {
                    v = connection.namedItem("cAlias").toElement();
                    if ( v.text() == alias )
                    {
                        saveElement( profileDoc, connection, "cAlias", newalias );
                        saveElement( profileDoc, connection, "cServerAddress", address );
                        saveElement( profileDoc, connection, "cServerPort", port );
                        saveElement( profileDoc, connection, "connectStr", cstring );
                        QString check;
                        if(startup)
                            check = "True";
                        else
                            check = "False";
                        saveElement( profileDoc, connection, "connectStart", check );
                        if(sendreport)
                            check = "True";
                        else
                            check = "False";
                        saveElement( profileDoc, connection, "cSendReport", check );
                        if(authconn)
                            check = "True";
                        else
                            check = "False";
                        saveElement( profileDoc, connection, "cAuthConn", check );

                        profileFile.close();
                        profileFile.open(QIODevice::Truncate | QIODevice::Text | QIODevice::ReadWrite);
                        QTextStream out(&profileFile);

                        out << profileDoc.toString();
                        break;
                    }
                }
            }
        }
    }

    profileFile.close();

}

void Profile::newConnection(QString alias, QString address, QString port, QString cstring, bool startup, bool sendreport, bool authconn)
{
    QDomDocument profileDoc;
    profileFile.setFileName("resource/profile.xml");
    if ( profileDoc.setContent( &profileFile, &errorMsg, &errorLine, &errorColumn ) )
    {
        QDomElement settings = profileDoc.namedItem( "settings" ).toElement();
        QDomElement profile = settings.firstChildElement( "profile" );
        for ( ; !profile.isNull(); profile = profile.nextSiblingElement( "profile" ) )
        {
            QDomElement v = profile.namedItem( "profName" ).toElement();
            if ( v.text() == profName )
            {

                QDomElement newConnection = addElement( profileDoc, profile, "connection", QString::null );
                addElement( profileDoc, newConnection, "cAlias", alias );
                addElement( profileDoc, newConnection, "cServerAddress", address );
                addElement( profileDoc, newConnection, "cServerPort", port );
                addElement( profileDoc, newConnection, "connectStr", cstring );

                QString check;
                if(startup)
                    check = "True";
                else
                    check = "False";
                addElement( profileDoc, newConnection, "connectStart", check );
                if(sendreport)
                    check = "True";
                else
                    check = "False";
                addElement( profileDoc, newConnection, "cSendReport", check );
                if(authconn)
                    check = "True";
                else
                    check = "False";
                addElement( profileDoc, newConnection, "cAuthConn", check );

                profileFile.close();
                profileFile.open(QIODevice::Truncate | QIODevice::Text | QIODevice::ReadWrite);
                QTextStream out(&profileFile);

                out << profileDoc.toString();
                break;
            }
        }
    }

    profileFile.close();

}

void Profile::removeConnection(QString alias)
{

    QDomDocument profileDoc;
    profileFile.setFileName("resource/profile.xml");
    if ( profileDoc.setContent( &profileFile, &errorMsg, &errorLine, &errorColumn ) )
    {
        QDomElement settings = profileDoc.namedItem( "settings" ).toElement();
        QDomElement profile = settings.firstChildElement( "profile" );
        for ( ; !profile.isNull(); profile = profile.nextSiblingElement( "profile" ) )
        {
            QDomElement v = profile.namedItem( "profName" ).toElement();
            if ( v.text() == profName )
            {
                QDomElement connection = profile.firstChildElement("connection");
                for ( ; !connection.isNull(); connection = connection.nextSiblingElement( "connection" ) )
                {
                    v = connection.namedItem("cAlias").toElement();
                    if ( v.text() == alias )
                    {
                        profile.removeChild(connection);

                        profileFile.close();
                        profileFile.open(QIODevice::Truncate | QIODevice::Text | QIODevice::ReadWrite);
                        QTextStream out(&profileFile);

                        out << profileDoc.toString();
                        break;
                    }
                }
            }
        }
    }

    profileFile.close();

}

QString Profile::getCallSign()
{
    return callSign;
}
QString Profile::getSSID()
{
    return SSID;
}
qreal   Profile::getLatitude()
{
    return mLatitude;
}
qreal   Profile::getLongitude()
{
    return mLongitude;
}
QString Profile::getConnectionPass()
{
    return connectionPass;
}
QString Profile::getPosComment()
{
    return posComment;
}
bool Profile::isUnitsEnglish()
{
    if( pUnits == "English")
        return true;
    else
        return false;
}
bool Profile::isMySQL()
{
    if( currDB == "MySQL")
        return true;
    else
        return false;
}
QString Profile::getMySQLUser()
{
    return mysqlUser;
}
QString Profile::getMySQLPass()
{
    return mysqlPass;
}
QString Profile::getMySQLDBName()
{
    return mysqlDBName;
}
QString Profile::getMySQLIPAddr()
{
    return mysqlIPAddr;
}
qint32 Profile::getMySQLPort()
{
    return mysqlPort;
}
QString Profile::getSQLiteFilepath()
{
    return sqliteFilepath;
}
int  Profile::getPurgeTime()
{
    return purgeTime;
}
int  Profile::getClearTime()
{
    return clearTime;
}
qint32  Profile::getPosIntervalTime()
{
    return posIntervalTime;
}
QString Profile::getErrorFile()
{
    return mErrorFile;
}
QString Profile::getCurrentProfile()
{
    return profName;
}
QString Profile::getServerAddress(QString alias)
{
    QString address;
    QDomDocument profileDoc;
    profileFile.setFileName("resource/profile.xml");
    if ( profileDoc.setContent( &profileFile, &errorMsg, &errorLine, &errorColumn ) )
    {
        QDomElement settings = profileDoc.namedItem( "settings" ).toElement();
        QDomElement profile = settings.firstChildElement( "profile" );
        for ( ; !profile.isNull(); profile = profile.nextSiblingElement( "profile" ) )
        {
            QDomElement v = profile.namedItem( "profName" ).toElement();
            if ( v.text() == profName )
            {
                QDomElement connection = profile.firstChildElement("connection");
                for ( ; !connection.isNull(); connection = connection.nextSiblingElement( "connection" ) )
                {
                    QDomElement aliasElement = connection.namedItem("cAlias").toElement();
                    if ( aliasElement.text() == alias )
                    {
                        address = connection.namedItem("cServerAddress").toElement().text();
                        break;
                    }
                }
            }
        }
    }

    profileFile.close();
    return address;
}
qint32 Profile::getServerPort(QString alias)
{
    qint32 port;
    QDomDocument profileDoc;
    profileFile.setFileName("resource/profile.xml");
    if ( profileDoc.setContent( &profileFile, &errorMsg, &errorLine, &errorColumn ) )
    {
        QDomElement settings = profileDoc.namedItem( "settings" ).toElement();
        QDomElement profile = settings.firstChildElement( "profile" );
        for ( ; !profile.isNull(); profile = profile.nextSiblingElement( "profile" ) )
        {
            QDomElement v = profile.namedItem( "profName" ).toElement();
            if ( v.text() == profName )
            {
                QDomElement connection = profile.firstChildElement("connection");
                for ( ; !connection.isNull(); connection = connection.nextSiblingElement( "connection" ) )
                {
                    QDomElement aliasElement = connection.namedItem("cAlias").toElement();
                    if ( aliasElement.text() == alias )
                    {
                        port = connection.namedItem("cServerPort").toElement().text().toInt();
                        break;
                    }
                }
            }
        }
    }

    profileFile.close();
    return port;
}
QString Profile::getConnectStr(QString alias)
{
    QString address;
    QDomDocument profileDoc;
    profileFile.setFileName("resource/profile.xml");
    if ( profileDoc.setContent( &profileFile, &errorMsg, &errorLine, &errorColumn ) )
    {
        QDomElement settings = profileDoc.namedItem( "settings" ).toElement();
        QDomElement profile = settings.firstChildElement( "profile" );
        for ( ; !profile.isNull(); profile = profile.nextSiblingElement( "profile" ) )
        {
            QDomElement v = profile.namedItem( "profName" ).toElement();
            if ( v.text() == profName )
            {
                QDomElement connection = profile.firstChildElement("connection");
                for ( ; !connection.isNull(); connection = connection.nextSiblingElement( "connection" ) )
                {
                    QDomElement aliasElement = connection.namedItem("cAlias").toElement();
                    if ( aliasElement.text() == alias )
                    {
                        address = connection.namedItem("connectStr").toElement().text();
                        break;
                    }
                }
            }
        }
    }

    profileFile.close();
    return address;
}
bool Profile::getConnectStart(QString alias)
{
    QString text;
    QDomDocument profileDoc;
    profileFile.setFileName("resource/profile.xml");
    if ( profileDoc.setContent( &profileFile, &errorMsg, &errorLine, &errorColumn ) )
    {
        QDomElement settings = profileDoc.namedItem( "settings" ).toElement();
        QDomElement profile = settings.firstChildElement( "profile" );
        for ( ; !profile.isNull(); profile = profile.nextSiblingElement( "profile" ) )
        {
            QDomElement v = profile.namedItem( "profName" ).toElement();
            if ( v.text() == profName )
            {
                QDomElement connection = profile.firstChildElement("connection");
                for ( ; !connection.isNull(); connection = connection.nextSiblingElement( "connection" ) )
                {
                    QDomElement aliasElement = connection.namedItem("cAlias").toElement();
                    if ( aliasElement.text() == alias )
                    {
                        text = connection.namedItem("connectStart").toElement().text();
                        break;
                    }
                }
            }
        }
    }

    profileFile.close();
    if ( text == "True" )
        return true;
    else
        return false;
}

bool Profile::getSendReport(QString alias)
{
    QString text;
    QDomDocument profileDoc;
    profileFile.setFileName("resource/profile.xml");
    if ( profileDoc.setContent( &profileFile, &errorMsg, &errorLine, &errorColumn ) )
    {
        QDomElement settings = profileDoc.namedItem( "settings" ).toElement();
        QDomElement profile = settings.firstChildElement( "profile" );
        for ( ; !profile.isNull(); profile = profile.nextSiblingElement( "profile" ) )
        {
            QDomElement v = profile.namedItem( "profName" ).toElement();
            if ( v.text() == profName )
            {
                QDomElement connection = profile.firstChildElement("connection");
                for ( ; !connection.isNull(); connection = connection.nextSiblingElement( "connection" ) )
                {
                    QDomElement aliasElement = connection.namedItem("cAlias").toElement();
                    if ( aliasElement.text() == alias )
                    {
                        text = connection.namedItem("cSendReport").toElement().text();
                        break;
                    }
                }
            }
        }
    }

    profileFile.close();
    if ( text == "True" )
        return true;
    else
        return false;
}

bool Profile::getAuthConn(QString alias)
{
    QString text;
    QDomDocument profileDoc;
    profileFile.setFileName("resource/profile.xml");
    if ( profileDoc.setContent( &profileFile, &errorMsg, &errorLine, &errorColumn ) )
    {
        QDomElement settings = profileDoc.namedItem( "settings" ).toElement();
        QDomElement profile = settings.firstChildElement( "profile" );
        for ( ; !profile.isNull(); profile = profile.nextSiblingElement( "profile" ) )
        {
            QDomElement v = profile.namedItem( "profName" ).toElement();
            if ( v.text() == profName )
            {
                QDomElement connection = profile.firstChildElement("connection");
                for ( ; !connection.isNull(); connection = connection.nextSiblingElement( "connection" ) )
                {
                    QDomElement aliasElement = connection.namedItem("cAlias").toElement();
                    if ( aliasElement.text() == alias )
                    {
                        text = connection.namedItem("cAuthConn").toElement().text();
                        break;
                    }
                }
            }
        }
    }

    profileFile.close();
    if ( text == "True" )
        return true;
    else
        return false;
}

QVector<QString> Profile::getProfileList()
{
    QVector<QString> list;
    QDomDocument profileDoc;
    profileFile.setFileName("resource/profile.xml");
    if ( profileDoc.setContent( &profileFile, &errorMsg, &errorLine, &errorColumn ) )
    {
        QDomElement settings = profileDoc.namedItem( "settings" ).toElement();
        QDomElement profile = settings.firstChildElement( "profile" );
        for ( ; !profile.isNull(); profile = profile.nextSiblingElement( "profile" ) )
        {
            list.push_back( profile.namedItem( "profName" ).toElement().text() );
        }
    }
    profileFile.close();
    return list;

}

QVector<QString> Profile::getConnectionList()
{
    QVector<QString> list;
    QDomDocument profileDoc;
    profileFile.setFileName("resource/profile.xml");
    if ( profileDoc.setContent( &profileFile, &errorMsg, &errorLine, &errorColumn ) )
    {
        QDomElement settings = profileDoc.namedItem( "settings" ).toElement();
        QDomElement profile = settings.firstChildElement( "profile" );
        for ( ; !profile.isNull(); profile = profile.nextSiblingElement( "profile" ) )
        {
            QDomElement v = profile.namedItem( "profName" ).toElement();
            if ( v.text() == profName )
            {
                QDomElement connection = profile.firstChildElement( "connection" );
                for ( ; !connection.isNull(); connection = connection.nextSiblingElement( "connection" ) )
                {
                    list.push_back( connection.namedItem( "cAlias" ).toElement().text() );
                }
            }
        }
    }
    profileFile.close();
    return list;

}

// setters
void Profile::setCallSign(QString s)
{
    callSign = s;
}
void Profile::setSSID(QString s)
{
    SSID = s;
}
void Profile::setLatitude(qreal r)
{
    mLatitude = r;
}
void Profile::setLongitude(qreal r)
{
    mLongitude = r;
}
void Profile::setConnectionPass(QString s)
{
    connectionPass = s;
}
void Profile::setPosComment(QString s)
{
    posComment = s;
}
void Profile::setUnitsEnglish(bool u)
{
    if( u )
        pUnits = "English";
    else
        pUnits = "Metric";
}
void Profile::setMySQL(bool b)
{
    if( b )
        currDB = "MySQL";
    else
        currDB = "SQLite";
}
void Profile::setMySQLUser(QString s)
{
    mysqlUser = s;
}
void Profile::setMySQLPass(QString s)
{
    mysqlPass = s;
}
void Profile::setMySQLDBName(QString s)
{
    mysqlDBName = s;
}
void Profile::setMySQLIPAddr(QString s)
{
    mysqlIPAddr = s;
}
void Profile::setMySQLPort(qint32 i)
{
    mysqlPort = i;
}
void Profile::setSQLiteFilepath(QString s)
{
    sqliteFilepath = s;
}
void Profile::setErrorFile(QString s)
{
    mErrorFile = s;
}
void Profile::setMapZoom(qint32 i)
{
    mMapZoom = i;
}
qint32 Profile::getMapZoom()
{
    return mMapZoom;
}

void Profile::setMapLat(qreal r)
{
    mMapLat = r;
}
qreal Profile::getMapLat()
{
    return mMapLat;
}

void Profile::setMapLong(qreal r)
{
    mMapLong = r;
}
qreal Profile::getMapLong()
{
    return mMapLong;
}

void Profile::setPurgeTime(int i)
{
    purgeTime = i;
}
void Profile::setClearTime(int i)
{
    clearTime = i;
}
void Profile::setPosIntervalTime(qint32 i)
{
    posIntervalTime = i;
}

void Profile::setOnlineMode(bool b)
{
    mOnlineMode = b == true ? 1 : 0;
}

bool Profile::getOnlineMode()
{
    return mOnlineMode == 1 ? true : false;
}

void Profile::setTileCacheDir(QString s)
{
    mTileCacheDir = s;
}

QString Profile::getTileCacheDir()
{
    return mTileCacheDir;
}

void Profile::setTileServerUrl(QString s)
{
    mTileServerUrl = s;
}

QString Profile::getTileServerUrl()
{
    return mTileServerUrl;
}

QString Profile::getOverlayChar()
{
    return mOverlayChar;
}

void Profile::setOverlayChar(QString c)
{
    mOverlayChar = c.at(0);
}

qint32 Profile::getIconId()
{
    return mIconId;
}

void Profile::setIconId(qint32 i)
{
    mIconId = i;
}
