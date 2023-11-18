
/*
Channel Log Code
Written from scratch, by TheFuture .. 'moded' by PosterBoy :)

This log code has evolved from a simple logger into more ..
Check out getnick(from) if you want a full UID report ..

PB added some log on/off switches and a defineable log file.

Ok .. Sorry tf.. I trashed your code.. Now it logs pretty much everything .. :)
If there are bugs, too bad .. you can fix 'em.. -OffSpring (PB)
*/

#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <strings.h>
#include <string.h>
#include <stddef.h>
#include "config.h"
#include "chanlog.h"

int number, i, logflag=1;

void do_chanlog(rest)
char *rest;
{
#ifdef LOG
	if(logflag) {
    FILE    *flog;

		if((flog = fopen( LOGFILE, "a")) == NULL)
			return;   
		fprintf(flog, "%s\n", rest);
		fclose(flog);
	}
#endif
}


/* switches by PB :) */

void	do_log_on(char *from, char *to, char *rest)
{
logflag=1;
send_to_user(from, "Channel logging on.");
return;
}
void	do_log_off(char *from, char *to, char *rest)
{
logflag=0;
send_to_user(from, "Channel logging off.");
return;
}

