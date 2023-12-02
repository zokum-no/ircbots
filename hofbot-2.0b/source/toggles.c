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
#include "hofbot.h"
#include "channel.h"
#include "userlist.h"
#include "dcc.h"
#include "function.h"
#include "commands.h"

extern botinfo *current;
extern char owneruserhost[MAXLEN];
extern int number_of_bots;
/* extern int togreet;
extern int togtopic;
extern int tognick; */

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

void do_togdelayop(from, to, rest, cmdlevel)
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
	toggle(from, &Chan->delayoptoggle, "Delayop on %s is", Chan->name);
}

void do_togranquote(from, to, rest, cmdlevel)
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
	toggle(from, &Chan->ranquotetoggle, "Random quote on %s is", Chan->name);
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
void do_togwk(from, to, rest, cmdlevel)
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
	toggle(from, &Chan->wktoggle, "Word-kick on %s", Chan->name);
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

       	toggle(from, &current->greetoggle, "Greet is");

}
void do_togtopic(from, to, rest, cmdlevel)
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

        toggle(from, &current->rantopictoggle, "Random topic is");

}
void do_tognick(from, to, rest, cmdlevel)
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
	toggle(from, &current->nicktoggle, "Nick is");
}

void do_tognews(from, to, rest, cmdlevel)
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
	toggle(from, &current->newstoggle, "News is");
}

void do_togwaron(from, to, rest, cmdlevel)
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
	toggle(from, &current->warontoggle, "War on is");
}

void do_togranick(from, to, rest, cmdlevel)
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
	toggle(from, &current->ranicktoggle, "Random nick is");
}

