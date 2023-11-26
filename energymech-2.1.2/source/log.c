/*
 * log.c - implements logging to file
 */

#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "global.h"
#include "config.h"
#include "defines.h"
#include "structs.h"
#include "h.h"

char *get_logname(which)
int which;
{
	return Logs[which].value;
}

int set_logname(name, value)
char *name;
char *value;
{
	int i;
	char *temp;

	for (i=0;*Logs[i].name;i++)
		if (!my_stricmp(name, Logs[i].name))
		{
			if (Logs[i].value)
				MyFree((char **)&Logs[i].value);
			mstrcpy(&temp, value);
			Logs[i].value = temp;
			return TRUE;
		}
	return FALSE;
}

void free_logs(void)
{
	int i;

	for (i=0;*Logs[i].name;i++)
		if (Logs[i].value)
			MyFree((char **)&Logs[i].value);
}

void log_it(int which, int global, char *fmt, ...)
{
	char *filename;
	char    buf[HUGE], buf2[HUGE];
	va_list msg;
	FILE    *out;

	if (!(filename = get_logname(which)))
		return;
	strcpy(buf, LOGDIR);
	if (*right(buf, 1) != '/')
		strcat(buf, "/");
	strcat(buf, filename);
	if (!(out = fopen(buf, "at")))
		return;
	va_start(msg, fmt);
	vsprintf(buf2, fmt, msg);
	fprintf(out, "%s [%10s]: %s\n", time2str(getthetime()),
		global ? "==GLOBAL==" : current->nick, buf2);
	va_end(msg);
	fclose(out);
}




