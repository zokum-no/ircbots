/*
 * ctcp.c - deals with most of the ctcp stuff (except for DCC).
 * (c) 1993-94 VladDrac (irvdwijk@cs.vu.nl)
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

#include <string.h>
#ifndef SYSV
#include <strings.h>
#endif
#include <stdlib.h>
#include <time.h>

#include "config.h"
#include "ctcp.h"
#include "debug.h"
#include "misc.h"
#include "send.h"
#include "vladbot.h"
#include "vlad-ons.h"

extern	botinfo	*currentbot;

struct
{
	char	*name;
	void	(*function)(char *from, char *to, char *rest);
} ctcp_commands[] =
{
	{ "FINGER",	ctcp_finger 	},
	{ "VERSION",	ctcp_version	},
	{ "CLIENTINFO",	ctcp_ignore	},
	 { "ACTION",	ctcp_ignore 	},
	{ "ZIRCON",	ctcp_ignore 	},
	{ "PING",	ctcp_ping   	},
	{ "SOURCE",	ctcp_ignore 	},
	{ "DCC",	ctcp_dcc 	},
	{ "SED",	ctcp_ignore	},
	{ NULL,		null(void (*)) 	}
};

void 	on_ctcp(char *from, char *to, char *ctcp_msg)
{
	char			*ctcp_command;
	int     		i;
	static long	last_ctcp = 0;

	if((ctcp_msg[0] == 'A') && (ctcp_msg[1] == 'C') && (ctcp_msg[2] == 'T') &&
		(ctcp_msg[3] == 'I') && (ctcp_msg[4] == 'O') && (ctcp_msg[5] == 'N'))
	{
		if (currentbot->floodprot_public &&	check_channel_flood(from,to,1))
			return;
	}
	else
		if (currentbot->floodprot_ctcp && check_channel_flood(from,to,3))
			return;

	if(check_session(from,to) == IS_FLOODING)
		return;

	if ((time(NULL)-last_ctcp) < REPLY_CTCP_INTERVAL)
		return;

	if (currentbot->spylog && !STRCASEEQUAL(currentbot->nick,getnick(from)))
		do_spylog(from,to,ctcp_msg);

	 if((ctcp_command = get_token(&ctcp_msg, " ")) == NULL)
		return;

	last_ctcp = time(NULL);

	for(i = 0; ctcp_commands[i].name != NULL; i++)
			if(STRCASEEQUAL(ctcp_commands[i].name, ctcp_command))
			{
							ctcp_commands[i].function( from, to, ctcp_msg);
							return;
			}

/*	ctcp_unknown(from, to, ctcp_command);*/
}

void	ctcp_finger( char *from, char *to, char *rest )

{
	char	*nick;

	if (currentbot->reply_finger == 0)
		return;
	else if (currentbot->reply_finger == 1)
	{
		if (userlevel(from)<MIN_USERLEVEL_FOR_OPS)
			return;
	}

	nick = getnick( from );

        send_ctcp_reply( nick, "FINGER Sorry, no fingerinfo available" );
        return; 
}

void	ctcp_version( char *from, char *to, char *rest )
{
	char	*nick;

        if (currentbot->reply_version == 0)
                return;
        else if (currentbot->reply_version == 1)
        {
                if (userlevel(from)<MIN_USERLEVEL_FOR_OPS)
                        return;
        }

	nick = getnick(from);

        send_ctcp_reply( nick, "VERSION %s", "Running ircII 2.8.2 Linux with BuRRiTo mods by TaCo" );
        return;
}

void	ctcp_clientinfo( char *from, char *to, char *rest )
{
	char	*nick;

	nick = getnick( from );

        send_ctcp_reply( nick, "CLIENTINFO I understand these CTCP-commands:" );
        send_ctcp_reply( nick, "CLIENTINFO VERSION, FINGER, ACTION, CLIENTINFO PING SOURCE DCC" );
        send_ctcp_reply( nick, "CLIENTINFO (btw, I'm %s, not a client :)",
                         currentbot->nick );
        return;
}
  
void	ctcp_dcc( char *from, char *to, char *rest )
{
	if (userlevel(from)>=100)
		reply_dcc( from, to, rest );
}

void	ctcp_ping( char *from, char *to, char *rest )
{
	char	*nick;

        if (currentbot->reply_ping == 0)
                return;
        else if (currentbot->reply_ping == 1)
        {
                if (userlevel(from)<MIN_USERLEVEL_FOR_OPS)
                        return;
        }
 
	nick = getnick( from );

        send_to_server( "NOTICE %s :\001PING %s\001", nick, rest);
        return;
}

void	ctcp_source( char *from, char *to, char *rest )
{
	char	*nick;

	nick = getnick( from );

        send_ctcp_reply( nick, "SOURCE The latest version of %s can be obtained from",
			 currentbot->nick );
	send_ctcp_reply( nick, "SOURCE NederServ (when it's up :)" );
	send_ctcp_reply( nick, "SOURCE %s is based on VladBot, written by VladDrac (irvdwijk@cs.vu.nl)", currentbot->nick );
        return; 
}

void	ctcp_ignore( char *from, char *to, char *rest )
{
        return;
}

void	ctcp_unknown( char *from, char *to, char *rest )
{	
	char	*nick;

	nick = getnick( from );
    	send_ctcp_reply( nick, "ERROR Unknown ctcp-command %s", rest );
} 
