#ifndef DEBUGLVL
#define DEBUGLVL 0
#endif
#include "mydebug.h"
#include "stringify.h"

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


#include "aprs_parser.h"
#include "packettypes.h"
#include <math.h>


const char symbol_weather = '_';
const char symbol_object  = ';';
const char symbol_nmea    = '$';
const char symbol_item    = ')';
const char symbol_message = ':';
const char symbol_capab   = '<';
const char symbol_reports = '>';
const char symbol_mice_2b = '\'';
const char symbol_mice_5b = '`';
const char symbol_experimental = '{'; // actually {{

const double knot_to_kmh = 1.852;
const double mph_to_kmh = 1.609344;
const double kmh_to_ms = 10.0 / 36;
const double mph_to_ms = mph_to_kmh * kmh_to_ms;
const double hinch_to_mm = 0.254;


#define QREGEXP(name, string) static QRegExp *name = 0; if (!name) name = new QRegExp(string)
#define QREGEXP2(name, string) static QRegExp *name = 0; if (!name) name = new QRegExp(string, Qt::CaseInsensitive)

#if DEBUGLVL
#define SET(var, val) do { qDebug("%s", qPrintable(QString("%1 => %2").arg(__stringify(var)).arg(val))); var = val; } while (0)
#else
#define SET(var, val) var = val
#endif

static QString cleanup_comment(QString s)
{
	QREGEXP(rx, "[^\\x20-\\x7e\\x80-\\xfe]");
	s = s.remove(*rx).trimmed();
	return s;
}


static QString check_ax25_call(const QString &s)
{
	MYTRACE("check_ax25_call(%s)", qPrintable(s));

	QREGEXP(rx, "^([A-Z0-9]{1,6})(-\\d{1,2}|)$");
	if (rx->indexIn(s) == -1)
		return QString::null;
	if (rx->cap(2).isEmpty())
		return rx->cap(1);
	int n = -rx->cap(2).toInt();
	if (n >= 16 || n < 0)
		return QString::null;
	return QString("%1-%2").arg(rx->cap(1)).arg(n);
}

static double fahrenheit_to_celsius(int f)
{
	return (f - 32) / 1.8;
}


static double get_posresolution(double f)
{
	return knot_to_kmh * 1000 * pow(10, -f);
}



APRSParser::APRSParser()
{
	MYTRACE("APRSParser::APRSParser");

	type = T_UNKNOWN;
	format = F_UNKNOWN;
	resultcode = OK;

	latitude = NAN;
	longitude = NAN;
	altitude = NAN;
	speed = 0;
	course = 0;
	posambiguity = -1;
	posresolution = NAN;

	temperature = NAN;
	pressure = NAN;
	wind_speed = NAN;
	wind_direction = 0;
	wind_gust = NAN;
	rain_1h = NAN;
	rain_24h = NAN;
	rain_midnight = NAN;
	snow_24h = NAN;
	raincount = -1;
	humidity = -1;
	luminosity = -1;

	messaging = false;
	alive = false;
}


enum result APRSParser::setError(enum result errnum, const char *desc)
{
	MYTRACE("APRSParser::setError(%d, %s)", (int) errnum, desc);

	// Don't clear any error
	if (errnum == OK)
		return resultcode;

	resultcode = errnum;
	return errnum;
}


enum result APRSParser::parse_capabilities(const QString &body)
{
	MYTRACE("APRSParser::parse_capabilities(%s)", qPrintable(body));
	// line 1312

	QREGEXP(rx_cap2, "^\\s*([^=]+)\\s*=\\s*(.*)\\s*$");
	QREGEXP(rx_cap1, "^\\s*([^=]+)\\s*$");

	foreach(QString cap, body.split(',')) {
		if (rx_cap2->indexIn(cap) != -1) {
			set(QString("cap_%1").arg(rx_cap2->cap(1)), rx_cap2->cap(2));
		} else
		if (rx_cap1->indexIn(cap) != -1) {
			set(QString("cap_%1").arg(rx_cap1->cap(1)), true);
		}
		else
			return setError(E_CAP_INV);
	}
	return OK;
}


enum result APRSParser::parse_wx(QString body)
{
	MYTRACE("APRSParser::parse_wx(%s)", qPrintable(body));
	// line 2041

	QREGEXP(rx_wx1, "^_{0,1}([\\d \\.\\-]{3})/([\\d \\.]{3})g([\\d \\.]+)t(-{0,1}[\\d \\.]+)");
	QREGEXP(rx_wx2, "^_{0,1}c([\\d \\.\\-]{3})s([\\d \\.]{3})g([\\d \\.]+)t(-{0,1}[\\d \\.]+)");
	QREGEXP(rx_wx3, "^_{0,1}([\\d \\.\\-]{3})/([\\d \\.]{3})t(-{0,1}[\\d \\.]+)");
	QREGEXP(rx_wx4, "^_{0,1}([\\d \\.\\-]{3})/([\\d \\.]{3})g([\\d \\.]+)");
	QREGEXP(rx_wx5, "^g(\\d+)t(-{0,1}[\\d \\.]+)");
	QString wnd_dir, wnd_spd, wnd_gust;
	QString tmp;
	if (rx_wx1->indexIn(body) != -1) {
		MYVERBOSE("  wx format 1");
		wnd_dir  = rx_wx1->cap(1);
		wnd_spd  = rx_wx1->cap(2);
		wnd_gust = rx_wx1->cap(3);
		tmp       = rx_wx1->cap(4);
		MYVERBOSE("  wnd_dir:   '%s'  (1.1)", qPrintable(wnd_dir) );
		MYVERBOSE("  wnd_speed: '%s'", qPrintable(wnd_spd) );
		MYVERBOSE("  wnd_gust:  '%s'", qPrintable(wnd_gust) );
		MYVERBOSE("  tmp:        '%s'", qPrintable(tmp) );
		body.remove(*rx_wx1);
	} else
	if (rx_wx2->indexIn(body) != -1) {
		MYVERBOSE("  wx format 2");
		wnd_dir  = rx_wx2->cap(1);
		wnd_spd  = rx_wx2->cap(2);
		wnd_gust = rx_wx2->cap(3);
		tmp       = rx_wx2->cap(4);
		MYVERBOSE("  wnd_dir:   '%s' (1.2)", qPrintable(wnd_dir) );
		MYVERBOSE("  wnd_speed: '%s'", qPrintable(wnd_spd) );
		MYVERBOSE("  wnd_gust:  '%s'", qPrintable(wnd_gust) );
		MYVERBOSE("  tmp:        '%s'", qPrintable(tmp) );
		body.remove(*rx_wx2);
	} else
	if (rx_wx3->indexIn(body) != -1) {
		MYVERBOSE("  wx format 3");
		wnd_dir  = rx_wx3->cap(1);
		wnd_spd  = rx_wx3->cap(2);
		tmp       = rx_wx3->cap(3);
		MYVERBOSE("  wnd_dir:   '%s' (2)", qPrintable(wnd_dir) );
		MYVERBOSE("  wnd_speed: '%s'", qPrintable(wnd_spd) );
		MYVERBOSE("  tmp:        '%s'", qPrintable(tmp) );
		body.remove(*rx_wx3);
	} else
	if (rx_wx4->indexIn(body) != -1) {
		MYVERBOSE("  wx format 4");
		wnd_dir  = rx_wx4->cap(1);
		wnd_spd  = rx_wx4->cap(2);
		wnd_gust = rx_wx4->cap(3);
		MYVERBOSE("  wnd_dir:   '%s' (3)", qPrintable(wnd_dir) );
		MYVERBOSE("  wnd_speed: '%s'", qPrintable(wnd_spd) );
		MYVERBOSE("  wnd_gust:  '%s'", qPrintable(wnd_gust) );
		body.remove(*rx_wx4);
	} else
	if (rx_wx5->indexIn(body) != -1) {
		MYVERBOSE("  wx format 5");
		wnd_gust = rx_wx5->cap(1);
		tmp       = rx_wx5->cap(2);
		MYVERBOSE("  wnd_gust:  '%s'", qPrintable(wnd_gust) );
		MYVERBOSE("  tmp:        '%s'", qPrintable(tmp) );
		body.remove(*rx_wx5);
	} else {
		MYVERBOSE("  no know wx format");
		return OK;
	}

	if (tmp.isEmpty()) {
		QREGEXP(rx_wx6, "t(-{0,1}\\d{1,3})");
		if (rx_wx6->indexIn(body) != -1) {
			tmp = rx_wx6->cap(1);
			MYVERBOSE("  tmp:        '%s'", qPrintable(tmp) );
			body.remove(*rx_wx6);
		}
	}
	bool ok;
	int t = wnd_gust.toInt(&ok);
	if (ok)
		SET(wind_gust, t * mph_to_ms);
	t = wnd_dir.toInt(&ok);
	if (ok)
		SET(wind_direction, t);
	t = wnd_spd.toInt(&ok);
	if (ok)
		SET(wind_speed, t * mph_to_ms);
	t = tmp.toInt(&ok);
	if (ok)
		SET(temperature, fahrenheit_to_celsius(t));

	QREGEXP(rx_r, "r(\\d{1,3})");
	if (rx_r->indexIn(body) != -1) {
		SET(rain_1h, rx_r->cap(1).toInt() * hinch_to_mm);
		body.remove(*rx_r);
	}
	QREGEXP(rx_p, "p(\\d{1,3})");
	if (rx_p->indexIn(body) != -1) {
		SET(rain_24h, rx_p->cap(1).toInt() * hinch_to_mm);
		body.remove(*rx_p);
	}
	QREGEXP(rx_P, "P(\\d{1,3})");
	if (rx_P->indexIn(body) != -1) {
		SET(rain_midnight, rx_P->cap(1).toInt() * hinch_to_mm);
		body.remove(*rx_P);
	}
	QREGEXP(rx_h, "h(\\d{1,3})");
	if (rx_h->indexIn(body) != -1) {
		int hum = rx_h->cap(1).toInt();
		if (hum == 0)
			hum = 100;
		if (hum > 0 && humidity <= 100)
			SET(humidity, hum);
		body.remove(*rx_h);
	}
	QREGEXP(rx_b, "b(\\d{4,5})");
	if (rx_b->indexIn(body) != -1) {
		SET(pressure, rx_b->cap(1).toDouble() / 10);
		body.remove(*rx_b);
	}
	QREGEXP(rx_l, "([lL])(\\d{1,3})");
	if (rx_l->indexIn(body) != -1) {
		uint lum = rx_l->cap(2).toUInt();
		if (rx_l->cap(1) == "l")
			lum += 1000;
		SET(luminosity, lum);
		body.remove(*rx_l);
	}
	QREGEXP(rx_v, "v([\\-\\+]{0,1}\\d+)");
	if (rx_v->indexIn(body) != -1) {
		MYDEBUG("  TODO v:     '%s'", qPrintable(rx_v->cap(1)) );
		body.remove(*rx_v);
	}
	QREGEXP(rx_s, "s(\\d{1,3})");
	if (rx_s->indexIn(body) != -1) {
		SET(snow_24h, rx_s->cap(1).toInt() * hinch_to_mm);
		body.remove(*rx_s);
	}
	QREGEXP(rx_H, "#(\\d+)");
	if (rx_H->indexIn(body) != -1) {
		SET(raincount, rx_H->cap(1).toInt());
		body.remove(*rx_H);
	}
	MYVERBOSE("  body now '%s'", qPrintable(body) );

	body.remove(QRegExp("^([rPphblLs#][\\. ]{1,5})+"));
	body = body.trimmed();

	QREGEXP(rx_soft, "^[a-zA-Z0-9\\-_]{3,5}$");
	if (rx_soft->indexIn(body) != -1) {
		set("soft", body.left(16));
	} else {
		SET(comment, cleanup_comment(body));
	}

	return OK;
}


enum result APRSParser::parse_wx_u2000(QString body)
{
	MYTRACE("APRSParser::parse_wx_u2000(%s)", qPrintable(body));

	qWarning("TODO %s:%d", __func__, __LINE__);
	return E_TODO_WX;
}


bool APRSParser::parse_timestamp(const QString &time)
{
	MYTRACE("APRSParser::parse_timestamp(%s)", qPrintable(time));

	QREGEXP(rx_ts, "^(\\d{2})(\\d{2})(\\d{2})(z|h|/)$");
	if (rx_ts->indexIn(time) == -1)
		return false;

        stamptype = rx_ts->cap(4);

	if (stamptype == "h") {
		day = 0;
		hour = rx_ts->cap(1).toUInt();
		minute = rx_ts->cap(2).toUInt();
		second = rx_ts->cap(3).toUInt();
		if (hour > 23 || minute > 59 || second > 59)
			return false;
		MYDEBUG("  h,m,s: %02d:%02d:%02d", hour, minute, second);
		//qWarning("TODO %s:%d add lots of cleanups", __func__, __LINE__);
	} else
	if (stamptype == "z" || stamptype == "/") {
		day = rx_ts->cap(1).toUInt();
		hour = rx_ts->cap(2).toUInt();
		minute = rx_ts->cap(3).toUInt();
		second = 0;
		if (day < 1 || day > 31 || hour > 23 || minute > 59)
			return false;
		MYDEBUG("  d h,m: %02d, %02d:%02d", day, hour, minute);
		//qWarning("TODO %s:%d add lots of cleanups", __func__, __LINE__);
	}

	return true;
}


enum result APRSParser::parse_status(QString packet)
{
	MYTRACE("APRSParser::parse_status(%s)", qPrintable(packet));
	// line 1289
	enum result res = OK;

	packet = packet.trimmed();

	// Check for a timestamp
	QREGEXP(rx_t, "^(\\d{6}z)");
	if (rx_t->indexIn(packet) != -1) {
		if (!parse_timestamp(rx_t->cap(1)))
			res = setError(E_TIMESTAMP_INV_STA);
		packet = packet.mid(7);
	}
	SET(data, packet);
	return res;
}


enum result APRSParser::parse_message(QString packet)
{
	MYTRACE("APRSParser::parse_message(%s)", qPrintable(packet));
	// line 1350

	QREGEXP(rx_m, "^:([A-Za-z0-9_ -]{9}):([\\x20-\\x7e\\x80-\\xfe]+)$");
	if (rx_m->indexIn(packet) == -1)
		return setError(E_MSG_INV);

	SET(data, rx_m->cap(1).trimmed());

	QString msg = rx_m->cap(2);
	QREGEXP(rx_a, "^ack([A-Za-z0-9}]{1,5})\\s*$");
	if (rx_a->indexIn(msg) != -1) {
		set("messageack", rx_a->cap(1));
		return OK;
	}
	QREGEXP(rx_r, "^rej([A-Za-z0-9}]{1,5})\\s*$");
	if (rx_r->indexIn(msg) != -1) {
		set("messagerej", rx_r->cap(1));
		return OK;
	}
	QREGEXP(rx_ib, "^([^{]*)\\{([A-Za-z0-9}]{1,5})\\s*$");
	if (rx_ib->indexIn(msg) == -1)
		SET(message, msg);
	else {
		SET(message, rx_ib->cap(1));
		SET(messageid, rx_ib->cap(2));
	}

	// catch telemetry
	if (msg.indexOf("BITS",0, Qt::CaseInsensitive) == 0 ||
	    msg.indexOf("PARM",0, Qt::CaseInsensitive) == 0 ||
	    msg.indexOf("UNIT",0, Qt::CaseInsensitive) == 0 ||
	    msg.indexOf("EQNS",0, Qt::CaseInsensitive) == 0)
	{
		type = T_TELEMETRY;
	}

	return OK;
}


enum result APRSParser::normalpos_to_decimal(const QString &packet, int &len)
{
	MYTRACE("APRSParser::normalpos_to_decimal(%s)", qPrintable(packet));

	if (packet.length() < 19)
		return setError(E_LOC_SHORT);
	SET(format, F_UNCOMPRESSED);

#ifdef LAX_PARSING
	QREGEXP(rx, "^(\\d{2,3})([0-7 ][0-9 ]\\.[0-9 ]{2,4})([NnSs])(.)(\\d{1,3})([0-7 ][0-9 ]\\.[0-9 ]{2,4})([EeWw])([\\x21-\\x7b\\x7d])");
#else
	QREGEXP(rx, "^(\\d{2})([0-7 ][0-9 ]\\.[0-9 ]{2})([NnSs])(.)(\\d{3})([0-7 ][0-9 ]\\.[0-9 ]{2})([EeWw])([\\x21-\\x7b\\x7d])");
#endif
	//            1       2                           3       4  5         6                           7       8
	if (rx->indexIn(packet) == -1)
		return setError(E_LOC_INV);
	len = rx->matchedLength();

	// Extract symbol code and table and check the latter
	QREGEXP(rx2, "^[/\\\\A-Z0-9]$");
	symbolcode = rx->cap(8).at(0).toAscii();
	if (rx2->indexIn(rx->cap(4)) == -1)
		return setError(E_SYM_INV_TABLE);
	symboltable = rx->cap(4).at(0).toAscii();
	MYDEBUG("  symbol code, table: '%c', '%c' (%d)", symbolcode, symboltable, __LINE__);

	// Check whole degrees
	bool ok;
	uint lat_deg = rx->cap(1).toUInt(&ok);
	if (lat_deg > 89 || !ok)
		return setError(E_LOC_LARGE);
#ifdef LAX_PARSING
	// In strict parsing mode, lon_deg can't be higher than 99.
	// Therefore don't compile this check at all.
	uint lon_deg = rx->cap(1).toUInt(&ok);
	if (lon_deg > 179 || !ok) {
		return setError(E_LOC_LARGE);
	}
#endif

	// "Count" position ambiguation
	SET(posambiguity, rx->cap(2).count(" "));
	double lati = 0;
	double longi = 0;
	switch (posambiguity) {
	case 0:
		if (rx->cap(6).count(" ") != 0)
			return setError(E_LOC_AMB_INV, "longitude 0");
		lati  = rx->cap(1).toFloat() + rx->cap(2).toFloat()/60.0;
		longi = rx->cap(5).toFloat() + rx->cap(6).toFloat()/60.0;
		break;
	case 1: {
		// last minute digit is unused
		QString lat = rx->cap(2).mid(0,4);
		QString lon = rx->cap(6).mid(0,4);
		if (lon.count(" ") || lat.count(" "))
			return setError(E_LOC_AMB_INV, "lat/lon 1");
		lati  = rx->cap(1).toFloat() + (lat.toFloat()+0.05)/60.0;
		longi = rx->cap(5).toFloat() + (lon.toFloat()+0.05)/60.0;
		break;
		}
	case 2: {
		// last two minute digits are unused
		QString lat = rx->cap(2).mid(0,2);
		QString lon = rx->cap(6).mid(0,2);
		if (lon.count(" ") || lat.count(" "))
			return setError(E_LOC_AMB_INV, "lat/lon 2");
		lati  = rx->cap(1).toFloat() + (lat.toFloat()+0.5)/60.0;
		longi = rx->cap(5).toFloat() + (lon.toFloat()+0.5)/60.0;
		break;
		}
	case 3: {
		// the single minute is unused
		QString lat = rx->cap(2).mid(0,1) + "5";
		QString lon = rx->cap(6).mid(0,1) + "5";
		if (lon.count(" ") || lat.count(" "))
			return setError(E_LOC_AMB_INV, "lat/lon 3");
		lati  = rx->cap(1).toFloat() + lat.toFloat()/60.0;
		longi = rx->cap(5).toFloat() + lon.toFloat()/60.0;
		break;
		}
	case 4:
		// no minutes used at all
		lati  = rx->cap(1).toFloat() + 0.5;
		longi = rx->cap(5).toFloat() + 0.5;
	default:
		return setError(E_LOC_AMB_INV);
	}

	// apply north/south east/west
	if (rx->cap(3).toUpper()=="S")
		lati = -lati;
	if (rx->cap(7).toUpper()=="W") {
		longi = -longi;
	}
	SET(latitude, lati);
	SET(longitude, longi);

	SET(posresolution, get_posresolution(2-posambiguity));

	return OK;
}


enum result APRSParser::compressed_to_decimal(const QString &packet)
{
	MYTRACE("APRSParser::compressed_to_decimal(%s)", qPrintable(packet));
	// line 1820

	QREGEXP(rx_comp, "^[/\\\\A-Za-j]{1}[\\x21-\\x7b]{8}[\\x21-\\x7b\\x7d]{1}[\\x20-\\x7b]{3}");
	if (rx_comp->indexIn(packet) == -1)
		return setError(E_COMP_INV);
	SET(format, F_COMPRESSED);

	symboltable = packet.at(0).toAscii();
	if (symboltable >= 'a' && symboltable <= 'j')
		symboltable = symboltable - 'a' + '0';
	int lat1 = packet.at(1).toAscii() - 33;
	int lat2 = packet.at(2).toAscii() - 33;
	int lat3 = packet.at(3).toAscii() - 33;
	int lat4 = packet.at(4).toAscii() - 33;
	int long1 = packet.at(5).toAscii() - 33;
	int long2 = packet.at(6).toAscii() - 33;
	int long3 = packet.at(7).toAscii() - 33;
	int long4 = packet.at(8).toAscii() - 33;
	symbolcode = packet.at(9).toAscii();
	int c1 = packet.at(10).toAscii() - 33;
	int s1 = packet.at(11).toAscii() - 33;
	int comptype = packet.at(12).toAscii() - 33;
	MYVERBOSE("  c1_raw %d ", packet.at(10).toAscii());
	MYVERBOSE("  s1_raw %d ", packet.at(11).toAscii());
	MYVERBOSE("  combtype 0x%x", comptype);
	MYDEBUG("  symbol code, table: '%c', '%c'", symbolcode, symboltable);

	double lati = 90.0 -
		(lat1 * 91.0 * 91.0 * 91.0 +
		 lat2 * 91.0 * 91.0 +
		 lat3 * 91.0 +
		 lat4) / 380926.0;
	double longi = -180.0 +
		(long1 * 91.0 * 91.0 * 91.0 +
		 long2 * 91.0 * 91.0 +
		 long3 * 91.0 +
		 long4) / 190463.0;
	SET(posresolution, 0.291);
	SET(latitude, lati);
	SET(longitude, longi);

	if (c1 != -1)
		set("gpsfixstatus", !!(comptype & 0x20));

	if (c1 == -1 && s1 == -1) {
		// no more data
	} else
	if ((comptype & 0x18) == 0x10) {
		SET(altitude, pow(1.002, c1 * 91.0 + s1) * 0.3048);
	} else
	if (c1 >= 0 && c1 <= 89) {
		if (c1 == 0) {
			SET(course, 360);
		} else {
			SET(course, c1 * 4);
		}
		SET(speed, (pow(1.08, s1) - 1) * knot_to_kmh);
	} else
	if (c1 == 90) {
		set("range", pow(2*1.08, s1) * mph_to_kmh);
	}

	return OK;
}


double APRSParser::nmea_get_latlon(const QString &value, const QString &sign)
{
	MYTRACE("APRSParser::nmea_get_latlon(%s, %s)", qPrintable(value), qPrintable(sign));
	// 761

	double val = 0;
	QREGEXP(rx_ll, "^\\s*(\\d{1,3})([0-5][0-9])\\.(\\d+)\\s*$");
	if (rx_ll->indexIn(value) == -1) {
		setError(E_NMEA_INV_CVAL);
		return 0;
	}
	QString minutes = QString("%1.%2")
		.arg(rx_ll->cap(2))
		.arg(rx_ll->cap(3));
	val = rx_ll->cap(1).toDouble() + (minutes.toDouble()/60.0);
	SET(posresolution, get_posresolution(rx_ll->cap(3).length()));

	bool e = sign.indexOf('E', 0, Qt::CaseInsensitive);
	bool w = sign.indexOf('W', 0, Qt::CaseInsensitive);
	bool n = sign.indexOf('N', 0, Qt::CaseInsensitive);
	bool s = sign.indexOf('S', 0, Qt::CaseInsensitive);
	if (e || w) {
		if (val > 179.999999) {
			setError(E_NMEA_LARGE_EW);
			return 0;
		}
		if (!w)
			val = -val;
	} else
	if (n || s) {
		if (val > 89.999999) {
			setError(E_NMEA_LARGE_NS);
			return 0;
		}
		if (!s)
			val = -val;
	} else {
		setError(E_NMEA_INV_SIGN);
		return 0;
	}

	return val;
}

enum result APRSParser::nmea_to_decimal(QString body)
{
	MYTRACE("APRSParser::nmea_to_decimal(%s)", qPrintable(body));
	// line 881

	// check checksum if provided
	body = body.trimmed();
	QREGEXP2(rx_cs, "^([\\x20-\\x7e]+)\\*([0-9A-F]{2})$");
	if (rx_cs->indexIn(body) != -1) {
		int calc = 0;
		foreach(QChar c, rx_cs->cap(1))
			calc ^= c.toAscii();
		QString c;
		c = c.sprintf("%2X", calc);
		if (c.compare(rx_cs->cap(2), Qt::CaseInsensitive) != 0)
			return setError(E_NMEA_INV_CHECKSUM);
		set("checksumok", true);
	}

	SET(format, F_NMEA);

	// TOOD _get_symbol_fromdst
	symbolcode = '/';
	symboltable = '/';
	MYDEBUG("  symbol code, table: '%c', '%c'", symbolcode, symboltable);

	QStringList nmea = body.split(",");
	if (nmea.at(0) == "GPRMC") {
		if (nmea.length() < 10)
			return setError(E_GPRMC_FEW_FIELDS);
		if (nmea.at(2) != "A")
			return setError(E_GPRMC_NO_FIX);

		// Time
		QREGEXP(rx_hms, "^\\s*(\\d{2})(\\d{2})(\\d{2})(|\\.\\d+)\\s*$");
		if (rx_hms->indexIn(nmea.at(1)) == -1)
			return setError(E_GPRMC_INV_TIME);
		uint h = rx_hms->cap(1).toUInt();
		uint m = rx_hms->cap(2).toUInt();
		uint s = rx_hms->cap(3).toUInt();
		if (h > 23 || m > 59 || s > 59)
			return setError(E_GPRMC_INV_TIME);
		hour = h;
		minute = m;
		second = s;
		MYDEBUG("  h,m,s: %02d:%02d:%02d", hour, minute, second);

		// Date
		QREGEXP(rx_ymd, "^\\s*(\\d{2})(\\d{2})(\\d{2})\\s*$");
		if (rx_ymd->indexIn(nmea.at(9)) == -1)
			return setError(E_GPRMC_INV_DATE);
		uint y = rx_ymd->cap(3).toUInt();
		if (y > 70)
			y += 1900;
		else
			y += 2000;
		m = rx_ymd->cap(2).toUInt();
		uint d = rx_ymd->cap(2).toUInt();
		// TODO if (!check_date(y,m,d) return
		// setError(E_GPRMC_INV_DATE);
		if (y < 1970 || y > 2070)
			return setError(E_GPRMC_DATE_OUT);
		year = y;
		month = m;
		day = d;
		MYDEBUG("  y-m-d: %04d-%02d-%02d", year, month, day);

		// Speed
		QREGEXP(rx_s, "^\\s*(\\d+(|\\.\\d+))\\s*$");
		if (rx_s->indexIn(nmea.at(7)) == -1)
			setError(E_GPRMC_INV_SPEED);
		SET(speed, rx_s->cap(1).toDouble() * knot_to_kmh);

		// Course
		QREGEXP(rx_c, "^\\s*(\\d+(|\\.\\d+))\\s*$");
		if (rx_c->indexIn(nmea.at(8)) == -1)
			SET(course, 0);
		else {
			double cou = rx_c->cap(1).toDouble() + 0.5;
			if (cou == 0) {
				cou = 360;
			} else
			if (cou > 360) {
				cou = 0;
			}
			SET(course, cou);
		}

		// Latitude, longitude
		double lati = nmea_get_latlon(nmea.at(3), nmea.at(4));
		if (lati==0) {
			return resultcode;
		}
		double longi = nmea_get_latlon(nmea.at(5), nmea.at(6));
		if (longi==0) {
			return resultcode;
		}
		SET(latitude, lati);
		SET(longitude, longi);
	} else {
		// line 1031
		qWarning("TODO NMEA %s:%d", __func__, __LINE__);
		return E_TODO_NMEA;
	}

	return OK;
}


bool APRSParser::dao_parse(const QString &candidate)
{
	MYTRACE("APRSParser::dao_parse(%s)", qPrintable(candidate));
	// line 1912

	double latoff, lonoff;

	QREGEXP(rx_dao1, "^([A-Z])(\\d)(\\d)$");
	QREGEXP(rx_dao2, "^([a-z])([\\x21-\\x7b])([\\x21-\\x7b])$");
	QREGEXP(rx_dao3, "^([\\x21-\\x7b])  $");
	if (rx_dao1->indexIn(candidate) != -1) {
		set("daodatumbyte", rx_dao1->cap(1));
		SET(posresolution, get_posresolution(3));
		latoff = rx_dao1->cap(2).toDouble() * 0.001 / 60.0;
		lonoff = rx_dao1->cap(3).toDouble() * 0.001 / 60.0;
	} else
	if (rx_dao2->indexIn(candidate) != -1) {
		// base 91 datum
		set("daodatumbyte", rx_dao2->cap(1).toUpper());
		SET(posresolution, get_posresolution(4));
		latoff = (rx_dao2->cap(2).at(0).toAscii() - 33.0) / 91.0 * 0.01 / 60.0;
		lonoff = (rx_dao2->cap(3).at(0).toAscii() - 33.0) / 91.0 * 0.01 / 60.0;
	} else
	if (rx_dao3->indexIn(candidate) != -1) {
		// only datum information, no lat/lon
		set("daodatumbyte", rx_dao3->cap(1).toUpper());
		return true;
	} else {
		return false;
	}

	if (latitude < 0)
		SET(latitude, latitude - latoff);
	else
		SET(latitude, latitude + latoff);
	if (longitude < 0)
		SET(longitude, longitude - lonoff);
	else
		SET(longitude, longitude + lonoff);

	return true;
}


enum result APRSParser::comments_to_decimal(QString rest)
{
	MYTRACE("APRSParser::comments_to_decimal(%s)", qPrintable(rest));

	if (rest.length() >= 7) {
		QREGEXP(rx_cs, "^([0-9. ]{3})/([0-9. ]{3})");
		QREGEXP(rx2, "^PHG(\\d[\\x30-\\x7e]\\d\\d[0-9A-Z])/");
		QREGEXP(rx3, "^PHG(\\d[\\x30-\\x7e]\\d\\d)");
		QREGEXP(rx4, "^RNG(\\d{4})");
		if (rx_cs->indexIn(rest) != -1) {
			// line 1146
			QREGEXP(rx_d3, "^\\d{3}$");
			uint c = rx_cs->cap(1).toUInt();
			if (rx_d3->indexIn(rx_cs->cap(1)) != -1 &&
			    c <= 360 &&
			    c >= 1)
			{
				SET(course, c);
			}
			if (rx_d3->indexIn(rx_cs->cap(2)) != -1) {
				SET(speed, rx_cs->cap(2).toDouble() * knot_to_kmh);
			}
			rest = rest.mid(7);
			MYVERBOSE("  rest now: '%s' (%d)", qPrintable(rest), __LINE__);
		} else
		if (rx2->indexIn(rest) != -1) {
			SET(phg, rx2->cap(1));
			rest = rest.mid(8);
			MYVERBOSE("  rest now: '%s' (%d)", qPrintable(rest), __LINE__);
		} else
		if (rx3->indexIn(rest) != -1) {
			SET(phg, rx3->cap(1));
			rest = rest.mid(7);
			MYVERBOSE("  rest now: '%s' (%d)", qPrintable(rest), __LINE__);
		} else
		if (rx4->indexIn(rest) != -1) {
			set("radiorange", rx4->cap(1).toDouble() * mph_to_kmh);
			rest = rest.mid(7);
			MYVERBOSE("  rest now: '%s' (%d)", qPrintable(rest), __LINE__);
		}
	}

	// Check for optional altitude anywhere in the comment
	// TODO: the perl code took the first occurrence
	QREGEXP(rx5, "^(.*)/A=(-\\d{5}|\\d{6})(.*)$");
	if (rx5->indexIn(rest) != -1) {
		SET(altitude, rx5->cap(2).toDouble() * 0.3048);
		rest = rx5->cap(1) + rx5->cap(3);
	}

	// Check for !DAO!, take the last occurrence (per recommendation)
	QREGEXP(rx_dao, "^(.*)\\!([\\x21-\\x7b][\\x20-\\x7b]{2})\\!(.*)$");
	if (rx_dao->indexIn(rest) != -1) {
		if (dao_parse(rx_dao->cap(2)))
			rest = rx_dao->cap(1) + rx_dao->cap(3);
	}

	// Strip a / or a ' ' from the beginning of a comment
	QREGEXP(rx7, "^[/\\s]+");
	SET(comment, cleanup_comment(rest.remove(*rx7)));

	return OK;
}


enum result APRSParser::mice_to_decimal(QString packet)
{
	MYTRACE("APRSParser::mice_to_decimal(%s)", qPrintable(packet));
	// line 1587

	// We only want the base callsign
	int i = dstcallsign.indexOf('-');
	if (i != -1) {
		dstcallsign = dstcallsign.mid(i-1);
		SET(dstcallsign, dstcallsign);
	}

	SET(format, F_MICE);

	if (packet.length() < 8 || dstcallsign.length() != 6)
		return setError(E_MICE_SHORT);

	QREGEXP2(rx1, "^[0-9A-LP-Z]{3}[0-9LP-Z]{3}$");
	if (rx1->indexIn(packet) != -1) {
		// A-K characters are not used in the last 3 characters
		// and MNO are never used
		return setError(E_MICE_INV);
	}

	symbolcode  = packet.at(6).toAscii();
	symboltable = packet.at(7).toAscii();
	MYVERBOSE("  symbol code, table: '%c', '%c'", symbolcode, symboltable);
	QREGEXP(rx2, "^[\\x26-\\x7f][\\x26-\\x61][\\x1c-\\x7f]{2}[\\x1c-\\x7d][\\x1c-\\x7f][\\x21-\\x7b\\x7d][/\\\\A-Z0-9]");
	if (rx2->indexIn(packet) == -1) {
		// TODO broken mic e", line 1612
		QREGEXP(rx_st, "^[/\\\\A-Z0-9]$");
		if (rx_st->indexIn(QString(symboltable)) == -1)
			return setError(E_SYM_INV_TABLE);
		else
			return setError(E_MICE_INV_INFO);
	}

	QString s;
	char c;
	QString micemsg;
	int amount = 6;
	for (i=0; i<6; i++) {
		c = dstcallsign.at(i).toAscii();

		if (i<3) {
			if ((c >= '0' && c <= '9') || c == 'L')
				micemsg += '0';
			else
			if (c >= 'A' && c <= 'K')
				micemsg += '2';
			else
			if (c >= 'P' && c <= 'Z')
				micemsg += '1';
		}

		if (c >= '0' && c <= '9')
			amount--;
		else
		if (c >= 'A' && c <= 'J') {
			c = c-'A'+'0';
			amount --;
		} else
		if (c >= 'P' && c <= 'Y') {
			c = c-'P'+'0';
			amount --;
		} else
		if (c == 'K' || c == 'L' || c == 'Z')
			c = ' ';
		else
			return setError(E_MICE_INV_INFO);
		s += c;
	}
        int messageNum = micemsg.toInt();
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
	set("mbits", micemsg);
	if (amount > 4)
		setError(E_MICE_AMB_LARGE);
	SET(posambiguity, amount);
	SET(posresolution, get_posresolution(2 - amount));

	QREGEXP(rx3, "^(\\d+)( *)$");
	if (rx3->indexIn(s) == -1)
		return setError(E_MICE_AMB_INV);

	// line 1660    TODO: this has been set above
	int posamb = 6 - rx3->cap(1).length();
	if (posamb > 4)
		return setError(E_MICE_AMB_LARGE);
	SET(posambiguity, posamb);

	// convert latitude
	int n = s.indexOf(" ");
	if (n != -1) {
		if (posambiguity > 4) {
			s[n] = '3';
		} else {
			s[n] = '5';
		}
	}
	s = s.replace(' ', '0');

	float lati = s.left(2).toFloat();
	s = s.mid(2,2) + '.' + s.right(2);
	lati += s.toDouble() / 60;
	// check north-south
	if (dstcallsign.at(3) <= 'L')
		lati = -lati;
	SET(latitude, lati);

	// convert longitude
	float longi = (packet.at(0).toAscii() - 28);
	if (dstcallsign.at(4) >= 'P')
		longi += 100;
	if (longi >= 180 && longi <= 189)
		longi -= 80;
	else
	if (longi >= 190 && longi <= 199)
		longi -= 190;
	// decode longitude minutes
	double longmin = packet.at(1).toAscii() - 28;
	if (longmin >= 60)
		longmin -= 60;
	// decode longitude minute decimals
	int longdec = packet.at(2).toAscii() - 28;
	QString lm = QString("%1.%2").arg(longmin).arg(longdec);
	// apply position ambiguity
	switch (posambiguity) {
	case 4:
		longi += .5;
		break;
	case 3:
		lm = lm.mid(0, 1) + "5";
		longi += lm.toDouble()/60;
		break;
	case 2:
		lm = lm.mid(0, 2) + "5";
		longi += lm.toDouble()/60;
		break;
	case 1:
		lm = lm.mid(0, 4) + "5";
		longi += lm.toDouble()/60;
		break;
	case 0:
		longi += lm.toDouble()/60;
		break;
	default:
		return setError(E_MICE_AMB_ODD);
	}
	// check east-west
	if (dstcallsign.at(5) >= 'P')
		longi = -longi;
	SET(longitude, longi);

	// position and course
	double spd = (packet.at(3).toAscii() - 28) * 10;
	double coursespeed = packet.at(4).toAscii() - 28;
	int tmp = coursespeed / 10.0;
	spd += tmp;
	coursespeed -= tmp * 10;
	double cou = 100 * coursespeed;
	cou += packet.at(5).toAscii() - 28;
	if (spd >= 800)
		spd -= 800;
	if (cou >= 400)
		cou -= 400;
	SET(speed, spd * knot_to_kmh);
	SET(course, cou);

	// TODO: mic-e telemetry data is not yet handled, but neither is it
	// in Ham::APRS::FAP. Example:
	//  LA5HL-9>VY5X54,WIDE1-1,WIDE2-2,qAR,LA5A:`3-Enh#>/>"4h}
	//                                          ^          ^^
	//                               Indicator for       Telemetry
	//                             2 byte Telemetry        data
	//                                   data

	// check for possibly altitude and comment data
	if (packet.length() > 8) {
		packet = packet.mid(8);

		// check for altitude
		QREGEXP(rx_alt, "^(.*)([\\x21-\\x7b])([\\x21-\\x7b])([\\x21-\\x7b])\\}(.*)$");
		if (rx_alt->indexIn(packet) != -1) {
			SET(altitude, (
				(rx_alt->cap(2).at(0).toAscii() - 33) * 91 * 91 +
				(rx_alt->cap(3).at(0).toAscii() - 33) * 91 +
				(rx_alt->cap(4).at(0).toAscii() - 33)) - 10000);
			packet = rx_alt->cap(1) + rx_alt->cap(5);
		}

		// Check for !DAO!, take the last occurrence (per recommendation)
		QREGEXP(rx_dao, "^(.*)\\!([\\x21-\\x7b][\\x20-\\x7b]{2})\\!(.*)$");
		if (rx_dao->indexIn(packet) != -1) {
			if (dao_parse(rx_dao->cap(2)))
				packet = rx_dao->cap(1) + rx_dao->cap(3);
		}

		if (!packet.isEmpty()) {
			SET(comment, cleanup_comment(packet));
		}
	}

	return OK;
}

enum result APRSParser::parse_obj_item(const QString &packet, int locationoffset, enum result error)
{
	MYTRACE("APRSParser::parse_obj_item(%s, %d, %d)",
	        qPrintable(packet), locationoffset, (int) error);
	// line 1255
	enum result ret = OK;

	QString locationchar = packet.mid(locationoffset, 1);
	QREGEXP(rx_lc, "^[/\\\\A-Za-j]$");
	if (rx_lc->indexIn(locationchar) != -1) {
		// 1259
		ret = compressed_to_decimal(packet.mid(locationoffset));
		locationoffset += 13;
	} else
	if (locationchar.at(0).digitValue() != -1) {
		// line 1263
		int len = 19;
		ret = normalpos_to_decimal(packet.mid(locationoffset), len);
		locationoffset += len;
	} else {
		ret = setError(error);
	}
	if (ret != 0)
		return ret;

	if (symbolcode != '_') {
		ret = comments_to_decimal(packet.mid(locationoffset));
	} else {
		// possibly a weather packet
		ret = parse_wx(packet);
	}
	return ret;
}


enum result APRSParser::object_to_decimal(const QString &packet)
{
	MYTRACE("APRSParser::object_to_decimal(%s)", qPrintable(packet));
	// line 1218

	if (packet.length() < 31)
		return setError(E_OBJ_SHORT);

#ifdef LAX_PARSING
	QREGEXP(rx_obj, "^;([\\x20-\\x7e]{4,9})(\\*|_)(\\d{6})(z|h|/)");
	// 8 = 6 digits timestamp, 1 letter, and the * character itself
	int locationoffset = packet.indexOf("*") + 8;
#else
	QREGEXP(rx_obj, "^;([\\x20-\\x7e]{9})(\\*|_)(\\d{6})(z|h|/)");
	const int locationoffset = 18;
#endif
	if (rx_obj->indexIn(packet) == -1)
		return setError(E_OBJ_INV);
	SET(data, rx_obj->cap(1));
	if (rx_obj->cap(2) == "*")
		SET(alive, true);
	else
	if (rx_obj->cap(2) == "-") {
		SET(alive, false);
	} else
		return setError(E_OBJ_INV);

	if (!parse_timestamp(rx_obj->cap(3)+rx_obj->cap(4)))
		setError(E_OBJ_TIMESTAMP_INV);

	return parse_obj_item(packet, locationoffset, E_OBJ_DEC_ERR);
}


enum result APRSParser::item_to_decimal(const QString &packet)
{
	MYTRACE("APRSParser::item_to_decimal(%s)", qPrintable(packet));
	// line 1395

	// int ret = 0;

	if (packet.length() < 18)
		return setError(E_OBJ_SHORT);

	QREGEXP(rx_item, "^\\)([\\x20\\x22-\\x5e\\x60-\\x7e]{3,9})(!|_)");
	if (rx_item->indexIn(packet) == -1)
		return setError(E_ITEM_INV);
	SET(data, rx_item->cap(1));
	if (rx_item->cap(2) == "!") {
		SET(alive, true);
	} else
	if (rx_item->cap(2) == "_") {
		SET(alive, false);
	} else
		return setError(E_ITEM_INV);

	return parse_obj_item(packet, rx_item->cap(1).length() + 2, E_ITEM_DEC_ERR);
}


enum result APRSParser::parse(QString orig, bool isAX25)
{
	MYTRACE("APRSParser::parse(pkt, %d)", isAX25);
	MYDEBUG("\n%s", qPrintable(orig));
	enum result res = OK;

	if (orig.isEmpty())
		return setError(E_PACKET_NONE);
	if (orig.length() <= 2)
		return setError(E_PACKET_SHORT);

	int n = orig.indexOf(":");
	QString header = orig.left(n);
	QString body = orig.mid(n+1).trimmed();
	if (body.isEmpty())
		return setError(E_PACKET_NO_BODY);
	SET(pktheader, header);
	SET(pktbody, body);

	// Source calls sign
	QREGEXP2(rx1, "^([A-Z0-9-]{1,9})>(.*)$");
	if (rx1->indexIn(header) == -1)
		return setError(E_SRCCALL_BADCHARS);

	QString s;
	if (!isAX25) {
		s = rx1->cap(1);
	} else {
		s = check_ax25_call( rx1->cap(1).toUpper() );
		if (s.isEmpty())
			return setError(E_SRCCALL_NO_AX25);
	}
	SET(srccallsign, s);

	// Destination call sign
	QStringList pathcomponents = rx1->cap(2).split(',');
	if (pathcomponents.isEmpty())
		return setError(E_DSTPATH_NONE);
	if (isAX25 && pathcomponents.count() > 9)
		return setError(E_DSTPATH_TOOMANY);

	s = check_ax25_call(pathcomponents.takeFirst());
	if (s.isEmpty())
		return setError(E_DSTCALL_NO_AX25);
	SET(dstcallsign, s);

	// Digipeaters
	if (isAX25) {
		// Data comes directly from AX.25
		// line 2431

		QREGEXP2(rx_digi, "^([A-Z0-9-]+)(\\*|)$");
		foreach(QString digi, pathcomponents) {
			if (rx_digi->indexIn(digi) != -1) {
				QString tested = check_ax25_call(rx_digi->cap(1).toUpper());
				if (tested.isEmpty())
				    return setError(E_DIGICALL_NO_AX25);
				MYVERBOSE("  hop: %d %s",
				          rx_digi->cap(2) == "*",
				          qPrintable(tested) );
				digipeaters.append(tested);
				wasdigied.append(rx_digi->cap(2) == "*");
			}
		}
		return E_TODO_AX25;
	} else {
		// Data is from APRS-IS
		// line 2453

		bool seenQ = false;
		foreach(QString digi, pathcomponents) {
			MYVERBOSE("  check digicall '%s'", qPrintable(digi));
			QREGEXP(rx2, "^([A-Z0-9a-z-]{1,9})(\\*|)$");
			if (rx2->indexIn(digi) != -1) {
				// push call rx2->cap(1)
				MYVERBOSE("  hop: %d %s",
				          rx2->cap(2) == "*",
				          qPrintable(rx2->cap(1)) );
				digipeaters.append(rx2->cap(1));
				wasdigied.append(rx2->cap(2) == "*");
				if (rx2->cap(1).left(1)=="q")
					seenQ = true;
			} else {
				QREGEXP(rx3, "^[0-9A-F]{32}$");
				if (seenQ && rx3->indexIn(digi) != -1) {
					MYDEBUG("  call: '%s'",
					          qPrintable(rx3->cap(1)));
					digipeaters.append(digi);
					wasdigied.append(0);
				} else {
					return setError(E_DIGICALL_BADCHARS);
				}
			}
		}
	}

	// Now parse the body
	char packettype = body.at(0).toAscii();
	MYVERBOSE("  packettype: '%c'", packettype);

	// Mic-E encoded packet
	if (packettype == symbol_mice_2b ||
	    packettype == symbol_mice_5b) {
		if (body.length() >= 9) {
			type = T_LOCATION;
			return mice_to_decimal(body.mid(1));
		}
	} else

	// Normal or compressed location packet, with or without
	// timestamp, with or without messaging capability
	if (packettype == '!' ||
	    packettype == '=' ||
	    packettype == '/' ||
	    packettype == '@')
	{
		// line 2502
		SET(messaging, packettype != '!' && packettype != '/');

		if (body.length() < 14)
			return setError(E_PACKET_SHORT);
		type = T_LOCATION;

		// Check prepended timestamp
		if (packettype == '/' || packettype == '@') {
			if (!parse_timestamp(body.mid(1,7)))
				return setError(E_TIMESTAMP_INV);
			body = body.mid(7);
		}

		// Remove first character, e.g. the '!'
		body = body.mid(1);
		char poschar = body.at(0).toAscii();

		if (body.at(0).digitValue() != -1) {
			// normal uncompressed position
			int len = 19;
			res = normalpos_to_decimal(body, len);
			if (res)
				return setError(res);
			if (symbolcode != symbol_weather) {
				res = comments_to_decimal(body.mid(len));
			} else {
				// line 2533
				res = parse_wx(body.mid(len));
			}
		} else
		if (poschar == '/' ||
		    poschar == '\\' ||
		    (poschar >= 'A' && poschar <= 'Z') ||
		    (poschar >= 'a' && poschar <= 'z')) {
			// 2541
			if (body.length() >= 13) {
				res = compressed_to_decimal(body.left(13));
			}
			if (symbolcode != symbol_weather) {
				res = comments_to_decimal(body.mid(13));
			} else {
				// line 2533
				//TODO: the perl source code had here 19
				res = parse_wx(body.mid(17-4));
			}
		} else
		if (poschar == '!') {
			// line 2555
			type = T_WX;
			res = parse_wx_u2000(body);
		} else {
			res = setError(E_PACKET_INV);
		}
		return setError(res);
	}

	if (packettype == symbol_weather) {
		// line 2569
		QREGEXP(rx_wx, "_(\\d{8})c[\\- \\.\\d]{1,3}s[\\- \\.\\d]{1,3}");
		if (rx_wx->indexIn(body) == -1)
			return setError(E_WX_UNSUPP, "Positionless");
		type = T_WX;
		return parse_wx(body.mid(9));
	}

	if (packettype == symbol_object) {
		// line 2579
		if (body.length() >= 31) {
			type = T_OBJECT;
			res = object_to_decimal(body);
		}
		return setError(res);
	}

	if (packettype == symbol_nmea) {
		// line 2586
		if (body.left(3) == "$GP") {
			type = T_LOCATION;
			res = nmea_to_decimal(body.mid(1));
		} else
		if (body.left(5) == "$ULTW") {
			//type == wx;
			setError(E_NMEA_WX);
			qWarning("TODO ultimeter data %s:%d", __func__, __LINE__);
		} else {
			type = T_WX;
			res = parse_wx_u2000(body.mid(5));
		}
		return setError(res);
	}
	if (packettype == symbol_item) {
		// line 2599
		if (body.length() >= 18) {
			type = T_ITEM;
			res = item_to_decimal(body);
		}
		return setError(res);
	}

	if (packettype == symbol_message) {
		// line 2606
		if (body.length() >= 11) {
			type = T_MESSAGE;
			res = parse_message(body);
		}
	}

	if (packettype == symbol_capab) {
		// line 2614
		if (body.length() >= 2) {
			type = T_CAPABILITIES;
			res = parse_capabilities(body.mid(1).trimmed());
		}
		return setError(res);
	}

	if (packettype == symbol_reports) {
		if (body.length() >= 1) {
			type = T_STATUS;
			res = parse_status(body.mid(1));
		}
		return setError(res);
	}

	QREGEXP(rx_tm, "^T#(\\d+),(-|)(\\d{1,6}|\\d+\\.\\d+|\\.\\d+|),(-|)(\\d{1,6}|\\d+\\.\\d+|\\.\\d+|),(-|)(\\d{1,6}|\\d+\\.\\d+|\\.\\d+|),(-|)(\\d{1,6}|\\d+\\.\\d+|\\.\\d+|),(-|)(\\d{1,6}|\\d+\\.\\d+|\\.\\d+|),([01]{0,8})");
	if (rx_tm->indexIn(body) != -1) {
		// line 2280
		type = T_TELEMETRY;
		set("seq", rx_tm->cap(1));
		for (int i=2; i<12; i+=2) {
			QString s = rx_tm->cap(i) + rx_tm->cap(i+1);
			double n = 0;
			if (!s.isEmpty()) {
				n = s.toDouble();
				if (n >= 99999 || n <= -99999)
					return setError(E_TLM_LARGE);
			}
			set(QString("val_%1").arg(i/2-1), n);
		}
		// MYDEBUG("  vals: '%s' '%s'", qPrintable(rx_tm->cap(2)), qPrintable(rx_tm->cap(3)) );
		// MYDEBUG("  vals: '%s' '%s'", qPrintable(rx_tm->cap(4)), qPrintable(rx_tm->cap(5)) );
		// MYDEBUG("  vals: '%s' '%s'", qPrintable(rx_tm->cap(6)), qPrintable(rx_tm->cap(7)) );
		// MYDEBUG("  vals: '%s' '%s'", qPrintable(rx_tm->cap(8)), qPrintable(rx_tm->cap(9)) );
		// MYDEBUG("  vals: '%s' '%s'", qPrintable(rx_tm->cap(10)), qPrintable(rx_tm->cap(11)) );
		set("bits", (QString("0000000")+rx_tm->cap(12)).right(8));
		return setError(res);
	}

	QREGEXP2(rx_dx, "DX\\s+de\\s+(.*?)\\s*[:>]\\s*(.*)$");
	if (rx_dx->indexIn(body) != -1) {
		type = T_DX;
		qWarning("TODO DX %s:%d", __func__, __LINE__);
		return E_TODO_DX;
	}

	if (body.at(0) == symbol_experimental && body.at(1) == symbol_experimental)
		return setError(E_EXP_UNSUPP);

	int pos = body.indexOf('!');
	if (pos >= 0 && pos <= 39) {
		type = T_LOCATION;
		SET(messaging, false);
		QString pchar = body.mid(pos + 1, 1);
		MYVERBOSE("  pchar '%s'", qPrintable(pchar));
		QREGEXP(rx_lr1, "^[/\\\\A-Za-j]$");
                if (pchar.length() > 0 && rx_lr1->indexIn(pchar) != -1) {
			// compressed position
			if (body.length() >= pos + 1 + 13) {
				enum result ret = compressed_to_decimal(body.mid(pos+1, 13));
				if (ret)
					return ret;
				if (symbolcode != symbol_weather)
					return comments_to_decimal(body.mid(pos + 14));
			}
		} else
                if (pchar.length() > 0 && pchar.at(0).digitValue() != -1) {
			// normal, uncompressed position
			if (body.length() >= pos + 1 + 19) {
				int len = 1;
				enum result ret = normalpos_to_decimal(body.mid(pos+1), len);
				if (ret)
					return ret;
				if (symbolcode != symbol_weather)
					return comments_to_decimal(body.mid(pos + 20));
			}
		}

		// line 2648
		qWarning("TODO last-resort %s:%d", __func__, __LINE__);
		return E_TODO_LOCATION;
	}

	return setError(res);
}

void APRSParser::set(const QString &key, const QVariant &data)
{
	MYDEBUG("%s -> '%s'", qPrintable(key), qPrintable(data.toString()) );
	hash[key] = data;
}


QVariant APRSParser::get(const QString &key, const char *format) const
{
	QVariant v = hash.value(key);
	if (format) {
		bool ok;
		double f = v.toDouble(&ok);
		if (ok) {
			QString s;
			s.sprintf(format, f);
			return s;
		}
	}
	return v;
}


bool APRSParser::has(const QString &key) const
{
	return hash.contains(key);
}
