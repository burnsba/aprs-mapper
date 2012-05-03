#ifndef APRS_PARSER_H
#define APRS_PARSER_H

/*
 * qaprstools - Qt based APRS tools (based on Ham::APRS::Fap 1.17)
 * Copyright (C) 2010  Holger Schurig, DH3HS, Germany, Nieder-Wöllstadt
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QStringList>
#include <QVariant>
#include <QHash>



enum result {
        OK,
        E_PACKET_NONE,
        E_PACKET_SHORT,
        E_PACKET_NO_BODY,
        E_SRCCALL_BADCHARS = 10,
        E_SRCCALL_NO_AX25,
        E_DSTPATH_TOOMANY = 20,
        E_DSTPATH_NONE,
        E_DSTCALL_NO_AX25 = 30,
        E_DIGICALL_BADCHARS = 40,
        E_DIGICALL_NO_AX25,
        E_TIMESTAMP_INV = 50,
        E_LOC_SHORT = 60,
        E_LOC_INV,
        E_LOC_LARGE,
        E_LOC_AMB_INV,
        E_SYM_INV_TABLE = 70,
        E_TLM_INV = 80,
        E_PACKET_INV = 90,
        E_MICE_SHORT = 100,
        E_MICE_INV,
        E_MICE_INV_INFO,
        E_MICE_AMB_INV,
        E_MICE_AMB_LARGE,
        E_MICE_AMB_ODD,
        E_TIMESTAMP_INV_STA = 110,
        E_OBJ_SHORT = 120,
        E_OBJ_INV,
        E_OBJ_TIMESTAMP_INV,
        E_OBJ_DEC_ERR,
        E_COMP_INV = 130,
        E_NMEA_INV = 140,
        E_NMEA_INV_CHECKSUM,
        E_NMEA_WX,
        E_NMEA_INV_CVAL,
        E_NMEA_INV_SIGN,
        E_NMEA_LARGE_NS,
        E_NMEA_LARGE_EW,
        E_GPRMC_FEW_FIELDS = 150,
        E_GPRMC_NO_FIX,
        E_GPRMC_INV_TIME,
        E_GPRMC_INV_DATE,
        E_GPRMC_DATE_OUT,
        E_GPRMC_INV_SPEED,
        E_MSG_INV = 160,
        E_ITEM_INV = 170,
        E_ITEM_DEC_ERR,
        E_CAP_INV = 180,
        E_WX_UNSUPP = 190,
        E_TLM_LARGE = 200,

        E_TODO_WX = 900,
        E_TODO_DAO,
        E_TODO_DX,
        E_TODO_NMEA,
        E_TODO_AX25,
        E_TODO_LOCATION,
        E_EXP_UNSUPP = 999,
};

enum format {
        F_UNKNOWN,
        F_UNCOMPRESSED,
        F_COMPRESSED,
        F_MICE,
        F_NMEA,
};

enum aprstype {
        T_UNKNOWN,       //                           0
        T_CAPABILITIES,  // 2616                      1
        T_DX,            // 2635                      2
        T_ITEM,          // 2600                      3
        T_LOCATION,      // 2491, 2509, 2590, 2650    4
        T_MESSAGE,       // 2608                      5
        T_OBJECT,        // 2580                      6
        T_STATUS,        // 2624                      7
        T_TELEMETRY,     // 2630                      8
        T_TELEMETRY_MSG, // 1382                      9
        T_WX,            // 2556, 2570, 2593          10
};



class APRSParser
{
public:
	APRSParser();
        enum result parse(QString orig, bool isAX25=false);

	enum aprstype type;
	QString pktheader;
	QString pktbody;
	QString comment;
	QString data; // objectname, itemname, status, message destination
	QString srccallsign;
	QString dstcallsign;
	QStringList digipeaters;
	QList<int> wasdigied;

	void set(const QString &key, const QVariant &data);
	QVariant get(const QString &key, const char *format=NULL) const;
	bool has(const QString &key) const;
	QHash<QString,QVariant> hash;

	double nmea_get_latlon(const QString &value, const QString &sign);

	enum result parse_capabilities(const QString &body);
	bool dao_parse(const QString &candidate);
	enum result parse_wx(QString body);
	enum result parse_wx_u2000(QString body);
	float temperature;
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

	bool parse_timestamp(const QString &time);
        QString stamptype;
	uint day;
	uint month;
	uint year;
	uint hour;
	uint minute;
	uint second;

	enum result parse_status(QString rest);

	enum result parse_message(QString rest);
	bool messaging;
	QString message;
	QString messageid;
        QString messageType;

	enum result normalpos_to_decimal(const QString &packet, int &len);
	enum result compressed_to_decimal(const QString &packet);
	enum result nmea_to_decimal(QString packet);
	enum format format;
	float latitude;
	float longitude;
	float altitude;
	float speed;
	int course;
	int posambiguity;
	double posresolution;
	char symbolcode;
	char symboltable;

	enum result comments_to_decimal(QString rest);
	QString phg;

	enum result mice_to_decimal(QString rest);

	enum result parse_obj_item(const QString &rest, int locationoffset, enum result error);
	enum result object_to_decimal(const QString &rest);
	enum result item_to_decimal(const QString &rest);
	bool alive;


// Error handling
private:
	enum result setError(enum result errnum, const char *desc=NULL);
	enum result resultcode;
public:
	//TODO QString getError();
	int getErrNum();
};


#endif
