#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include "dcc.h"
#include "hofbot.h"
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




