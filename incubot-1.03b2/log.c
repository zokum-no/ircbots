/*
 * log.c - implements logging to file
 * (c) 1993 VladDrac (irvdwijk@cs.vu.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include "dcc.h"
#include "vladbot.h"
#include "config.h"
#include "crc-ons.h"
#include "log.h"

extern	botinfo	*currentbot;
int loglevel;

void 	botlog( char *logfile, char *logfmt, ...)

{
    	char 	buf[WAYTOBIG];
	va_list	msg;
	FILE	*flog;

	if((flog = fopen( logfile, "a")) == NULL)
		return;
	va_start(msg, logfmt);
	vsprintf(buf, logfmt, msg);
	fprintf(flog, "[%s] %s: %s\n", currentbot->botname, get_ctime(0), buf);
	va_end(msg);
	fclose(flog);
}

int     set_loglevel( int level )
{
        if((level >= 0) && (level <= 3))
	{
	         loglevel = level;
                 return(1);
	}
        else
                 return(0);
}

int     get_loglevel( void )
{
        return(loglevel);
}

void 	globallog( int level, char *logfile, char *logfmt, ...)
{
   	char 	buf[WAYTOBIG];
	va_list	msg;
	FILE	*flog;

	if ( level > loglevel )
		return;

	if((flog = fopen( logfile, "a")) == NULL)
		return;

	va_start(msg, logfmt);
	vsprintf(buf, logfmt, msg);
	fprintf(flog, "[-GLOBAL-] %s: %s\n", get_ctime(0), buf);
	va_end(msg);
	fclose(flog);
}
