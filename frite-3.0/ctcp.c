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

#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include "misc.h"
#include "ctcp.h"
#include "debug.h"
#include "vladbot.h"
#include "config.h"
#include "actions.h"
#include "vlad-ons.h"

extern	botinfo	*currentbot;

struct
{
	char	*name;
	void	(*function)(char *from, char *to, char *rest);
} ctcp_commands[] =
{
	{ "FINGER",	ctcp_finger 	},
	{ "VERSION",	ctcp_version 	},
	{ "CLIENTINFO",	ctcp_clientinfo },
	{ "ACTION",	do_ctcps 	},
	{ "ZIRCON",	ctcp_ignore 	},
	{ "PING",	ctcp_ping 	},
	{ "SOURCE",	ctcp_source 	},
	{ "DCC",	ctcp_dcc 	},
	{ NULL,		null(void (*)) 	}
};

void 	on_ctcp(char *from, char *to, char *ctcp_msg)
{
    	char 	*ctcp_command;
    	int     i;

	if(check_session(from) == IS_FLOODING)
		return;
	
    	if((ctcp_command = get_token(&ctcp_msg, " ")) == NULL)
            	return;
        
	for(i = 0; ctcp_commands[i].name != NULL; i++)
        	if(STRCASEEQUAL(ctcp_commands[i].name, ctcp_command))
        	{
                    	ctcp_commands[i].function( from, to, ctcp_msg);
                    	return;
    		}
	ctcp_unknown(from, to, ctcp_command);
}

void	ctcp_finger( char *from, char *to, char *rest )

{	
	char	*nick;

	nick = getnick( from );

	send_ctcp_reply( nick, "FINGER %s version %s.",
			 currentbot->nick, VERSION );
        return; 
}

void	ctcp_version( char *from, char *to, char *rest )
{
	char	*nick;
	nick = getnick(from);

        send_ctcp_reply( nick, "VERSION Hiya, I'm %s version %s.", 
			 currentbot->nick, VERSION );
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
	reply_dcc( from, to, rest );
}

void	ctcp_ping( char *from, char *to, char *rest )
{
	char	*nick;

	nick = getnick( from );

	bot_reply(from, 9);
        send_ctcp_reply( nick, "PING %s", rest );
        return;
}
void	do_ping( char *from, char *to, char *rest )
{
  if (ischannel(to))
    sendprivmsg(to, "\x2Pong\x2");
  else
    send_to_user(from, "\x2Pong\x2");
  return;
}
void	ctcp_source( char *from, char *to, char *rest )
{
	char	*nick;

	nick = getnick( from );

        send_ctcp_reply( nick, "SOURCE The latest version of %s can be obtained from",
			 currentbot->nick );
	send_ctcp_reply( nick, "SOURCE NederServ (when it's up :) or fRitE (EfNet & UnderNet)" );
	send_ctcp_reply( nick, "SOURCE %s is based on VladBot, this code written by VladDrac.");
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
    	send_ctcp_reply( nick, "ERROR Hey .. what are you trying to do?");
} 

