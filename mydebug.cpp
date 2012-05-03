#undef DEBUGLVL
#define DEBUGLVL 3
#include "mydebug.h"
#include "packettypes.h"
/**
 * @file
 * @author Holger Schurig
 *
 * @section DESCRIPTION
 *
 * Simple debugging aid for Qt/C++ programs.
 *
 * This file contains code to
 * a) send Qt's debug output to syslog
 * b) dump arbitrary memory regions as a hex dump
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <qglobal.h>
#include <stdio.h>		// for snprintf
#include <stdlib.h>		// for abort
#include <stdarg.h>


/*!
 * Re-implemented MessageHandler for Qt
 *
 * This message-handler will be used by qDebug(), qWarning() etc, but also
 * by myDebug() --- and this via MYDEBUG(), MYTRACE() and MYVERBOSE().
 *
 * All text will be sent to STDOUT (not STDERR, as the default
 * message-handler from Qt!). If USE_SYSLOG is defined, the text will
 * additionally be sent to the syslog facility.
 */
static void MessageHandler(QtMsgType type, const char *msg)
{
	const char *t = "Unknown: ";

	switch (type) {
	case QtDebugMsg:
		t = "";
		break;
	case QtWarningMsg:
                t = "Warning: ";
		break;
	case QtCriticalMsg:
                t = "Critical: ";
		break;
	case QtFatalMsg:
		t = "Fatal: ";
		break;
	}

	// Send to stdout
	fputs(t, stdout);
	puts(msg);
	fflush(stdout);

	if (type == QtFatalMsg)
		abort();
}


void myDebug(const char *fmt, ...)
{
	char buf[256];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	MessageHandler(QtDebugMsg, buf);
	va_end(ap);
}


void dump(const void *data, int len, bool with_addr)
{
	int i, thisline;
	int offset = 0;
	const unsigned char *p = (const unsigned char *)data;

	while (offset < len) {
		if (with_addr)
			printf("%04x ", offset);
		thisline = len - offset;
		if (thisline > 16) {
			thisline = 16;
		}
		for (i = 0; i < thisline; i++) {
			printf("%02x ", p[i]);
		}
		for (; i < 16; i++) {
			printf("   ");
		}
		for (i = 0; i < thisline; i++) {
			printf("%c", (p[i] >= 0x20 && p[i] < 0x7f) ? p[i] : '.');
		}
		printf("\n");
		offset += thisline;
		p += thisline;
	}
	fflush(stdout);
}


/*! Install MessageHandler() as Qt's message-handler */
static void init_debug(void) __attribute__((__constructor__));

static void init_debug(void)
{
	qInstallMsgHandler(MessageHandler);
}
