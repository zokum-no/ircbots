/*
   * toggles.c - all the toggles...
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <strings.h>
#include <time.h>
#include <signal.h>
#include "config.h"
#include "fnmatch.h"
#include "dweeb.h"
#include "channel.h"
#include "userlist.h"
#include "dcc.h"
#include "function.h"
#include "commands.h"

extern botinfo *current;
extern char owneruserhost[MAXLEN];
extern int number_of_bots;
extern int togreet = 0;



void do_togenfm(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	int temp;
	CHAN_list *Chan;

	chan = find_channel(to, &rest);

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!(Chan = search_chan(chan)))
	{
		not_on(from, chan);
		return;
	}
	toggle(from, &Chan->enfmtoggle, "Enforced modes on %s", Chan->name);
}

void do_togtop(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	int temp;
	CHAN_list *Chan;

	chan = find_channel(to, &rest);

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!(Chan = search_chan(chan)))
	{
		not_on(from, chan);
		return;
	}
	toggle(from, &Chan->toptoggle, "Enforced topics on %s", Chan->name);
}

void do_togpub(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	int temp;
	CHAN_list *Chan;

	chan = find_channel(to, &rest);

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!(Chan = search_chan(chan)))
	{
		not_on(from, chan);
		return;
	}
	toggle(from, &Chan->pubtoggle, "Public commands on %s", Chan->name);
}

void do_togshit(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	int temp;
	CHAN_list *Chan;

	chan = find_channel(to, &rest);

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!(Chan = search_chan(chan)))
	{
		not_on(from, chan);
		return;
	}
	toggle(from, &Chan->shittoggle, "Shit-kicking on %s", Chan->name);
}

void do_togaop(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	int temp;
	CHAN_list *Chan;

	chan = find_channel(to, &rest);

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!(Chan = search_chan(chan)))
	{
		not_on(from, chan);
		return;
	}
	toggle(from, &Chan->aoptoggle, "Auto-opping on %s", Chan->name);
}

void do_togprot(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	int temp;
	CHAN_list *Chan;

	chan = find_channel(to, &rest);

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!(Chan = search_chan(chan)))
	{
		not_on(from, chan);
		return;
	}
	toggle(from, &Chan->sdtoggle, "Protection on %s", Chan->name);
}

void do_togmass(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	int temp;
	CHAN_list *Chan;

	chan = find_channel(to, &rest);

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!(Chan = search_chan(chan)))
	{
		not_on(from, chan);
		return;
	}
	toggle(from, &Chan->masstoggle, "Mass protection on %s", Chan->name);
}

void do_togsd(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	int temp;
	CHAN_list *Chan;

	chan = find_channel(to, &rest);

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!(Chan = search_chan(chan)))
	{
		not_on(from, chan);
		return;
	}
	toggle(from, &Chan->sdtoggle, "Server-deop on %s", Chan->name);
}

void do_togso(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	int temp;
	CHAN_list *Chan;

	chan = find_channel(to, &rest);

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!(Chan = search_chan(chan)))
	{
		not_on(from, chan);
		return;
	}
	toggle(from, &Chan->sotoggle, "Strict-opping on %s", Chan->name);
}

void do_togcc(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	toggle(from, &current->cctoggle, "Command char. required");
}

void do_togik(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	int temp;
	CHAN_list *Chan;

	chan = find_channel(to, &rest);

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!(Chan = search_chan(chan)))
	{
		not_on(from, chan);
		return;
	}
	toggle(from, &Chan->iktoggle, "Idle-kick on %s", Chan->name);
}

void do_togbk(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	int temp;
	CHAN_list *Chan;

	chan = find_channel(to, &rest);

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!(Chan = search_chan(chan)))
	{
		not_on(from, chan);
		return;
	}
	toggle(from, &Chan->bktoggle, "Beep-kick on %s", Chan->name);
}

void do_togck(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	int temp;
	CHAN_list *Chan;

	chan = find_channel(to, &rest);

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!(Chan = search_chan(chan)))
	{
		not_on(from, chan);
		return;
	}
	toggle(from, &Chan->cktoggle, "Caps-kick on %s", Chan->name);
}

void do_togreet(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{

	char *chan;

	chan = find_channel(to, &rest);

	if (get_userlevel(from, chan) < cmdlevel) 
	{
		no_access(from, chan);
		return;
	}

	if (togreet != 0)
	{
		togreet = 0;
		send_to_user(from, "Value of Greets is OFF");
	}
	
	else {
		togreet = 1;
		send_to_user(from, "Value of Greets is ON");
	}

}




