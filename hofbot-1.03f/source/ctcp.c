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
/*	{ "FINGER",     ctcp_finger }, */
	{ "VERSION",    ctcp_version }, 
/* Please don't delete TRUEVER if you really appreciate my hard work and
   want to get HOFbotv2.0 or greater in the future.  Thank you! */
	{ "TRUEVER",    ctcp_trueversion }, 
	{ "CLIENTINFO", ctcp_clientinfo },
/*	{ "ACTION", ctcp_action }, */
	{ "PING",       ctcp_ping }, 
/*	{ "DCC",        ctcp_dcc }, */
        { "GETCMDCH",     ctcp_cmdchar }, 
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
  ctcp_unknown(from, to, ctcp_command);
}

void ctcp_finger( char *from, char *to, char *rest )
{       
	char    *nick;

	nick = getnick( from );

	send_ctcp_reply( nick, "FINGER \002Fingering is prohibited\002" );
	return; 
}

void ctcp_cmdchar(char *from, char *to, char *rest)
{
  send_ctcp_reply(getnick(from), "CMDCHAR \002My command character is: %c\002", current->cmdchar);
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
	send_ctcp_reply( nick, "VERSION \002HOFBotv1.03F (c) 1995-96 by HOF (jpoon@mathlab.sunysb.edu)\002");
	return;
}

void ctcp_clientinfo( char *from, char *to, char *rest )
{
	char    *nick;

	nick = getnick( from );

	send_ctcp_reply( nick, "CLIENTINFO ircII 2.2.9 *IX: Are we there yet?!?");
	return;
}
  
void ctcp_dcc( char *from, char *to, char *rest )
{
  if (!rest)
    return;
/*  reply_dcc( from, to, rest ); */
        dcc_chat(from, rest);
}

void ctcp_ping( char *from, char *to, char *rest )
{
	char    *nick;

	nick = getnick( from );

	send_ctcp_reply( nick, "PING" );
	return;
}

void ctcp_action( char *from, char *to, char *rest )
{
	on_msg(from, to, rest);
}

void ctcp_unknown( char *from, char *to, char *ctcp_command )
{       
	char    *nick;
	return;
	nick = getnick( from );

	send_ctcp_reply( nick, "ERROR Unknown ctcp-command : %s", ctcp_command);
} 

