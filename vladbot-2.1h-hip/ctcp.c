/*
 * ctcp.c - deals with most of the ctcp stuff (except for DCC).
 * (c) 1993-94 VladDrac (irvdwijk@cs.vu.nl)
 */

#include <strings.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"
#include "ctcp.h"
#include "debug.h"
#include "misc.h"
#include "send.h"
#include "vladbot.h"

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
        { "ACTION",	ctcp_ignore 	},
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

        send_ctcp_reply( nick, "FINGER Sorry, no fingerinfo available" );
        return; 
}

void	ctcp_version( char *from, char *to, char *rest )
{
	char	*nick;

	nick = getnick(from);

        send_ctcp_reply( nick, "VERSION Hello, I'm %s version %s.", 
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

        send_ctcp_reply( nick, "PING" );
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
/* brage 
	char	*nick;

	nick = getnick( from );
    	send_ctcp_reply( nick, "ERROR Unknown ctcp-command %s", rest );
    	
   */
} 
