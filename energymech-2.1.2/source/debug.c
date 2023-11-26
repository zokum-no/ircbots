/*
 * debug.c - debugging functions
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
	if (dbuglevel == LOG) {
		log_it(DEBUG_LOG_VAR, 0, "%s", buf);
	}
	else {
		printf("[%s] %s\n", current->nick, buf);
	}
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
	if (dbuglevel == LOG) {
		log_it(DEBUG_LOG_VAR, 0, "[-GLOBAL-] %s", buf);
	}
	else {
		printf("[-GLOBAL-] %s\n", buf);
	}
	va_end(msg);
#endif
}

int set_debuglvl(newlvl)
int newlvl;
{
#ifdef DBUG
	if ((newlvl < QUIET) || (newlvl > LOG))
		return 0;
	dbuglevel = newlvl;
	return 1;
#else
	return 0;
#endif
}








