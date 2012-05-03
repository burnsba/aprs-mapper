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

#ifndef PROFILE_H
#define PROFILE_H

#include <QObject>
#include <QString>
#include <QImage>
#include <qdom.h>

#include "errorhandler.h"

#include "coordinate.h"

class Profile : public QObject
{
    Q_OBJECT

public:

    static void loadProfile();  // load default/blank profile
    static void loadProfile(QString profileName);
    static void saveProfile();
    static void saveProfile(QString profileName);
    static void removeProfile(QString profileName);
    static void saveConnection(QString alias, QString newalias, QString address, QString port, QString cstring, bool startup, bool sendreport, bool authconn);
    static void newConnection(QString alias, QString address, QString port, QString cstring, bool startup, bool sendreport, bool authconn);
    static void removeConnection(QString connectionName);

    // getters
    static QString getCallSign();
    static QString getSSID();
    static qint32  getIconId();
    static QString getOverlayChar();
    static QString getPosComment();
    static qreal   getLatitude();
    static qreal   getLongitude();
    static QString getErrorFile();
    static QString getServerAddress(QString);
    static qint32  getServerPort(QString);
    static QString getConnectStr(QString);
    static bool    getConnectStart(QString);
    static bool    getSendReport(QString);
    static bool    getAuthConn(QString);
    static qint32  getMapZoom();
    static qreal   getMapLat();
    static qreal   getMapLong();
    static QVector<QString> getProfileList();
    static QVector<QString> getConnectionList();
    static QString getConnectionPass();
    static QString getMySQLUser();
    static QString getMySQLPass();
    static QString getMySQLDBName();
    static QString getMySQLIPAddr();
    static qint32  getMySQLPort();
    static QString getSQLiteFilepath();
    static int  getPurgeTime();
    static int  getClearTime();
    static qint32  getPosIntervalTime();
    static QString getCurrentProfile();
    static bool    getOnlineMode();
    static QString getTileServerUrl();
    static QString getTileCacheDir();

    static bool    isMySQL();
    static bool    isUnitsEnglish();

    // setters
    static void setCallSign(QString);
    static void setSSID(QString);
    static void setIconId(qint32);
    static void setOverlayChar(QString);
    static void setPosComment(QString);
    static void setLatitude(qreal);
    static void setLongitude(qreal);
    static void setErrorFile(QString);
    static void setMapZoom(qint32);
    static void setMapLat(qreal);
    static void setMapLong(qreal);
    static void setConnectionPass(QString);
    static void setMySQLUser(QString);
    static void setMySQLPass(QString);
    static void setMySQLDBName(QString);
    static void setMySQLIPAddr(QString);
    static void setMySQLPort(qint32);
    static void setSQLiteFilepath(QString);
    static void setPurgeTime(int);
    static void setClearTime(int);
    static void setPosIntervalTime(qint32);
    static void setOnlineMode(bool);
    static void setTileServerUrl(QString);
    static void setTileCacheDir(QString);

    static void setMySQL(bool);
    static void setUnitsEnglish(bool);



private:

    static QString profName;
    static QString callSign;
    static QString SSID;
    static qint32  mIconId;
    static QString mOverlayChar;
    static QString posComment;
    static QString pUnits;
    static qreal   mLatitude;
    static qreal   mLongitude;
    static QString mErrorFile;
    static QString cServerAddress;
    static qint32  cServerPort;
    static QString connectStr;
    static bool    cSendReport;
    static qint32  mMapZoom;    // for last location
    static qreal   mMapLat;
    static qreal   mMapLong;
    static QString connectionPass;
    static int  purgeTime;
    static int  clearTime;
    static qint32  posIntervalTime;
    static bool    connectStart;

    static QString currDB;
    static QString mysqlUser;
    static QString mysqlPass;
    static QString mysqlDBName;
    static QString mysqlIPAddr;
    static qint32  mysqlPort;
    static QString sqliteFilepath;

    static QString loadProf;
    static QFile   profileFile;
    static QString errorMsg;
    static QString result;
    static qint32  errorLine, errorColumn;

    static void readXML(QString loadName, QDomDocument profileDoc);
    static QDomElement addElement( QDomDocument &profileDoc, QDomNode &node,
                            const QString tag, const QString value = QString::null );
    static void saveElement( QDomDocument &profileDoc, QDomElement &elemToChange,
                             const QString tag, const QString value);

    static qint32 mOnlineMode;
    static QString mTileServerUrl;
    static QString mTileCacheDir;

signals:

public slots:

};

#endif // PROFILE_H
