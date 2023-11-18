
/* public code for Vladbot */
/* snagged from Holo and Vale */
/* implemented by TheFuture

here's whatcha do .. grab this code .. #include "publics.h" in
your vlad-ons.c file .. go down to on_msg .. add this in before
the flood/command parser:

	do_publics(from, to, rest);

Recompile, and it should work cwel .. -TheFuture

*/

#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include "config.h"
#include "publics.h"

/*

Here .. below is the publics table .. the first field is the keyword
to match, the second is an optional keyword to match against ..
The third field is what to say, followed by the next .. and the fourth
is the appendage of that ... the first flag is userflag, which tells
the code wether or not to put 'there nick' in there, next to the first
and last part to say ..  the second flag denotes wether to make your bot
say it .. or action it ..

-TheFuture

*/

struct
{
	char	*keyword1, *keyword2, *response1, *response2;
        int     userflag, actionflag;
} publics[] =
{
{ "THANK",   PUBLIC_NICK, "Don't mention it, ", " ..", 1, 0},
{ "LOVE Y",  PUBLIC_NICK, "blushes .. aww shucks, ", ".. :)", 1, 1},
{ "HELLO",   PUBLIC_NICK, "Hello ", "!", 1, 0},
{ "HEY",     PUBLIC_NICK, "Hey ", ", how ya doin'?!", 1, 0},
{ "HI",      PUBLIC_NICK, "Hiya ", " ..", 1, 0},
{ "COFFEE",  PUBLIC_NICK, "gives ", " a cup of coffee ..", 1, 1},
{ "BEER",    PUBLIC_NICK, "tosses ", " a nice cold beer ..", 1, 1},
{ "HOW ARE", PUBLIC_NICK, "feels tired ..", "", 0, 1},
	{ NULL, NULL,		NULL, NULL, 0, 0}
};


void do_publics(from, to, rest)
char *from;
char *to;
char *rest;
{
#ifdef PUBLICS
	int i;

	for(i = 0; publics[i].keyword1 != NULL; i++)
	{
		if((strcasestr(rest, publics[i].keyword1)) && (strcasestr(rest, publics[i].keyword2)))
			if((!publics[i].userflag) && (!publics[i].actionflag))
				sendprivmsg(to, "%s", publics[i].response1);
			else if((publics[i].userflag) && (publics[i].actionflag))
				send_ctcp(to, "ACTION %s%s%s", publics[i].response1, getnick(from), publics[i].response2);
			else if((!publics[i].userflag) && (publics[i].actionflag))
				send_ctcp(to, "ACTION %s", publics[i].response1);
			else
				sendprivmsg(to, "%s%s%s", publics[i].response1, getnick(from), publics[i].response2);
	}
#endif
        return;
}
