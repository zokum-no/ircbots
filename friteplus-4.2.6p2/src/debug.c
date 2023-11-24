/*
 * debug.c - debugging functions for fRitE
 * Copyright (C) 1995 OffSpring (cracker@cati.CSUFresno.EDU)
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
#include <stdio.h>
#include <stdarg.h>
#include "frite_bot.h"
#include "config.h"
#include "autoconf.h"
#include "debug.h"
#include "fnmatch.h"

extern	botinfo *currentbot;
int	dbuglevel = QUIET;

void	debug(int lvl, char *format, ...)
{
#ifdef DBUG
	va_list	msg;
	char	buf[WAYTOBIG];
    time_t  T;
    char    *time_string;
 
    if((T = time((time_t *)NULL)) != -1)
     {
     time_string = ctime(&T);
     *(time_string + strlen(time_string) - 1) = '\0';
     }

	if(lvl > dbuglevel)
		return;

	va_start(msg, format);
	vsprintf(buf, format, msg);

	if(fnmatch("PRIVMSG", buf, FNM_CASEFOLD)) {
		printf("[%s] %s\n", time_string, buf);
	}

/* old-	printf("[%s] %s\n", currentbot->botname, buf); */

	va_end(msg);
#endif
}	

void	global_dbg(int lvl, char *format, ...)
{
#ifdef DBUG
	va_list	msg;
	char	buf[WAYTOBIG];

	if(lvl > dbuglevel)
		return;
	va_start(msg, format);
	vsprintf(buf, format, msg);
	printf("[-GLOBAL-] %s\n", buf);
	va_end(msg);
#endif
}	

int	set_debuglvl(int newlvl)
{
#ifdef DBUG
	if((newlvl < QUIET) || (newlvl > NOTICE))
		return 0;
	dbuglevel = newlvl;
	return 1;
#else
	return 0;
#endif
}
