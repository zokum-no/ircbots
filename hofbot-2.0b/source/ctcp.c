/*
 * ctcp.c - deals with most of the ctcp stuff (except for DCC).
 */

#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include "function.h"
#include "ctcp.h"
#include "config.h"
#include "debug.h"
#include "hofbot.h"

extern  char    current_nick[MAXLEN];
extern botinfo *current;

struct
{
	char    *name;
	void    (*function)();
} ctcp_commands[] =
{
	{ "VERSION",    ctcp_version }, 
	{ "TRUEVER",    ctcp_trueversion }, 
	{ "PING",       ctcp_ping }, 
	{ "DCC",        ctcp_dcc },
	{ NULL,         (void(*)())(NULL) }
};

void    on_ctcp(char *from, char *to, char *ctcp_msg)
{
  char    *ctcp_command;
  int     i;
  
  if (check_memory(from) == IS_FLOODING)
    return;
  
  if((ctcp_command = get_token(&ctcp_msg, " ")) == NULL)
    return;
  
  for(i = 0; ctcp_commands[i].name != NULL; i++)
    if (!my_stricmp(ctcp_commands[i].name, ctcp_command))
      {
	ctcp_commands[i].function( from, to, ctcp_msg);
	return;
      }
}

void ctcp_version( char *from, char *to, char *rest )
{
	char    *nick;

	nick = getnick(from);

	send_ctcp_reply( nick, "VERSION ircII 2.2.9 *IX: Are we there yet?!?");
	return;
}

void ctcp_trueversion( char *from, char *to, char *rest )
{
	char    *nick;

	nick = getnick(from);
	send_ctcp_reply( nick, "VERSION \002HOFBotv2.0B (c) 1995-96 by HOF (jpoon@mathlab.sunysb.edu)\002");
	return;
}

void ctcp_dcc( char *from, char *to, char *rest )
{
  if (!rest) 
    return;
  reply_dcc(from, to, rest); 
}

void ctcp_ping( char *from, char *to, char *rest )
{
	char    *nick;

	nick = getnick( from );

	send_ctcp_reply( nick, "PING" );
	return;
}

