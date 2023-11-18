/* action code for Vladbot */

/* just rewrote the public parser for actions :) */
/* -thefuture */

#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include "config.h"
#include "actions.h"
#include "chanlog.h"
#include "spy.h"
       char log_buff[MAXLEN];
struct
{
	char	*keyword1, *keyword2, *response1, *response2;
        int     userflag, actionflag;
} actions[] =
{
{ "HUG",  PUBLIC_NICK, "feels warm and fuzzy all over..", "", 0, 1},
{ "KISS", PUBLIC_NICK, "turns a bright shade of pink..", "", 0, 1},
{ "LOVE", PUBLIC_NICK, "blushes .. aww shucks, ", ".. :)", 1, 1},
{ "HIT",  PUBLIC_NICK, "cries his heart out ..", "", 0, 1},
{ "KICK", PUBLIC_NICK, "beats the crap out of ", " ..", 1, 1},
	{ NULL, NULL,		NULL, NULL, 0, 0}
};


void do_ctcps(from, to, rest)
char *from;
char *to;
char *rest;
{
	int i;
        sprintf(log_buff, "* %s (%s) %s", getnick(from), to, rest);
	send_spy_chan(to, log_buff);
        do_chanlog(log_buff);
#ifdef PUBLICS
	for(i = 0; actions[i].keyword1 != NULL; i++)
	{
		if((strcasestr(rest, actions[i].keyword1)) && (strcasestr(rest, actions[i].keyword2)))
			if((!actions[i].userflag) && (!actions[i].actionflag))
				sendprivmsg(to, "%s", actions[i].response1);
			else if((actions[i].userflag) && (actions[i].actionflag))
				send_ctcp(to, "ACTION %s%s%s", actions[i].response1, getnick(from), actions[i].response2);
			else if((!actions[i].userflag) && (actions[i].actionflag))
				send_ctcp(to, "ACTION %s", actions[i].response1);
			else
				sendprivmsg(to, "%s%s%s", actions[i].response1, getnick(from), actions[i].response2);
	}
#endif
        return;
}
