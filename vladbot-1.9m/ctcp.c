/*
 * ctcp.c - deals with most of the ctcp stuff (except for DCC).
 *
 *          (c) 1993 VladDrac (irvdwijk@cs.vu.nl)
 */

#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include "misc.h"
#include "ctcp.h"
#include "config.h"

extern	char	current_nick[MAXLEN];

struct
{
	char	*name;
	void	(*function)();
} ctcp_commands[] =
{
	{ "FINGER",	ctcp_finger },
	{ "VERSION",	ctcp_version },
	{ "CLIENTINFO",	ctcp_clientinfo },
        { "ACTION",	ctcp_action },
	{ "PING",	ctcp_ping },
	{ "SOURCE",	ctcp_source },
	{ "DCC",	ctcp_dcc },
	{ NULL,		null(void (*)) }
};

void 	on_ctcp( from, to, ctcp_msg )
char 	*from;
char	*to;
char 	*ctcp_msg;

{
    	char 	*ctcp_command;
        char	*rest;
    	int     i;

#ifdef DBUG
	printf( "ENTERED on_ctcp( %s, %s, %s );\n",
	from, to, ctcp_msg );
#endif
        rest = strchr( ctcp_msg, ' ' ) + 1;
    	if( ( ctcp_command = strtok( ctcp_msg, " " ) ) == NULL )
            return;
        
	for( i = 0; ctcp_commands[i].name != NULL; i++ )
        	if( !strcasecmp( ctcp_commands[i].name, ctcp_command ) )
        	{
                    ctcp_commands[i].function( from, to, rest );
                    return;
    		}
	ctcp_unknown( from, to, ctcp_command );
#ifdef DBUG
	printf( "exiting on_ctcp() correctly\n" );
#endif
}

void	ctcp_finger( from, to, rest )
char 	*from;
char	*to;
char	*rest;

{	
	char	*nick;

	nick = getnick( from );

        send_ctcp_reply( nick, "FINGER Sorry, no fingerinfo available" );
        return; 
}

void	ctcp_version( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
	char	*nick;

	nick = getnick(from);

        send_ctcp_reply( nick, "VERSION Hello, I'm %s version %s.", 
			 current_nick, VERSION );
        return;
}

void	ctcp_clientinfo( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
	char	*nick;

	nick = getnick( from );

        send_ctcp_reply( nick, "CLIENTINFO I understand these CTCP-commands:" );
        send_ctcp_reply( nick, "CLIENTINFO VERSION, FINGER, ACTION, CLIENTINFO PING SOURCE DCC" );
        send_ctcp_reply( nick, "CLIENTINFO (btw, I'm %s, not a client :)",
                         current_nick );
        return;
}
  
void	ctcp_dcc( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
	reply_dcc( from, to, rest );
}

void	ctcp_ping( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
	char	*nick;

	nick = getnick( from );

        send_ctcp_reply( nick, "PING" );
        return;
}

void	ctcp_source( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
	char	*nick;

	nick = getnick( from );

        send_ctcp_reply( nick, "The latest version of %s can be obtained from",
			 current_nick );
	send_ctcp_reply( nick, "NederServ (when it's up :)" );
	send_ctcp_reply( nick, "%s is based on VladBot, written by VladDrac (irvdwijk@cs.vu.nl)", current_nick );
        return; 
}

void	ctcp_action( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
        return;
}

void	ctcp_unknown( from, to, ctcp_command )
char	*from;
char	*to;
char	*ctcp_command;

{	
	char	*nick;

	nick = getnick( from );

    	send_ctcp_reply( nick, "ERROR Unknown ctcp-command %s", ctcp_command );
} 
