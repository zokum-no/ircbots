/*
 * debug.c - debugging functions for DweeB0t
 */

#include <stdio.h>
#include <stdarg.h>
#include "dweeb.h"
#include "config.h"
#include "debug.h"

extern botinfo *current;
int    dbuglevel = QUIET;

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






