/*
 * debug.c - debugging functions
 * (c) 1995 CoMSTuD (cbehrens@slavery.com)
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

#include <stdio.h>
#include <stdarg.h>

#include "global.h"
#include "config.h"
#include "defines.h"
#include "structs.h"
#include "h.h"

void debug(int lvl, char *format, ...)
{
#ifdef DBUG
	va_list msg;
	char buf[HUGE];

	if (lvl > dbuglevel)
		return;
	va_start(msg, format);
	vsprintf(buf, format, msg);
	printf("[%s] %s\n", current->nick, buf);
	va_end(msg);
#endif
}

void global_dbg(int lvl, char *format, ...)
{
#ifdef DBUG
	va_list msg;
	char buf[HUGE];

	if (lvl > dbuglevel)
		return;
	va_start(msg, format);
	vsprintf(buf, format, msg);
	printf("[-GLOBAL-] %s\n", buf);
	va_end(msg);
#endif
}

int set_debuglvl(newlvl)
int newlvl;
{
#ifdef DBUG
	if ((newlvl < QUIET) || (newlvl > NOTICE))
		return 0;
	dbuglevel = newlvl;
	return 1;
#else
	return 0;
#endif
}








