/*
 * log.c - implements logging to file
 * (c) 1994 CoMSTuD (cbehrens@iastate.edu)
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
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include "dcc.h"
#include "hackbot.h"
#include "config.h"
#include "log.h"
#include "function.h"

extern  botinfo *current;

void botlog( char *logfile, char *logfmt, ...)

{
	char    buf[HUGE];
	va_list msg;
	FILE    *flog;
	time_t  T;
	char    *time_string;


	if((T = time((time_t *)NULL)) != -1)
	{
		time_string = ctime(&T);        
		*(time_string + strlen(time_string) - 1) = '\0';
	}       
	if((flog = fopen( logfile, "a")) == NULL)
		return;
	va_start(msg, logfmt);
	vsprintf(buf, logfmt, msg);
	fprintf(flog, "[%s] %s: %s\n", current->nick, time_string, buf);
	va_end(msg);
	fclose(flog);
}

void    globallog( char *logfile, char *logfmt, ...)

{
	char    buf[HUGE];
	va_list msg;
	FILE    *flog;
	time_t  T;
	char    *time_string;


	if((T = time((time_t *)NULL)) != -1)
	{
		time_string = ctime(&T);        
		*(time_string + strlen(time_string) - 1) = '\0';
	}       
	if((flog = fopen( logfile, "a")) == NULL)
		return;
	va_start(msg, logfmt);
	vsprintf(buf, logfmt, msg);
	fprintf(flog, "[-GLOBAL-] %s: %s\n", time_string, buf);
	va_end(msg);
	fclose(flog);
      }




