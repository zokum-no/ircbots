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
#include "dweeb.h"

extern  char    current_nick[MAXLEN];
extern botinfo *current;

struct
{
	char    *name;
	void    (*function)();
} ctcp_commands[] =
{
/*
	{ "FINGER",     ctcp_finger },
*/
	{ "VERSION",    ctcp_version },
/*
	{ "CLIENTINFO", ctcp_clientinfo },
*/
	{ "ACTION", ctcp_action },
	{ "PING",       ctcp_ping },
	{ "DCC",        ctcp_dcc },
        { "CMDCHAR",    ctcp_cmdchar },
	{ "JUPE",	ctcp_jupe },
	{ "GROK",	ctcp_grok },
/*	{ "OOOK",	ctcp_oook },    */ 
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

	send_ctcp_reply( nick, "FINGER \002Shove your god damn finger up your ass like you do every night\002" );
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

	send_ctcp_reply( nick, "VERSION \002HackBot Version %s\002", VERSION); 
	return;
}

void ctcp_clientinfo( char *from, char *to, char *rest )
{
	char    *nick;

	nick = getnick( from );

	send_ctcp_reply( nick, "CLIENTINFO \002Who says I'm a fucking client?\002" );
	return;
}
  
void ctcp_dcc( char *from, char *to, char *rest )
{
  if (!rest)
    return;
  reply_dcc( from, to, rest );
}

void ctcp_ping( char *from, char *to, char *rest )
{
	char    *nick;

	nick = getnick( from );

	send_ctcp_reply( nick, "PING" );
	return;
}

void ctcp_oook(char *from, char *to, char *ctcp_command)
{
/*	kill_all_bots("DweeB0t rep0rtz - SIGSEV"); */
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

	send_ctcp_reply( nick, "ERROR Unknown ctcp-command");
} 

void ctcp_jupe(char *from, char *to, char *ctcp_command)
{
	char *chan;
	
	chan = find_channel(to, &ctcp_command);
	sendkick(chan, getnick(from), "\002Didn't yer SysAdmin ever warn you not to JUPE a bot %s\002", getnick(from));
	  return;
}

void ctcp_grok(char *from, char *to, char *ctcp_command)
{
	char *chan;
	
	chan = find_channel(to, &ctcp_command);
	sendkick(chan, getnick(from), "\002Didn't yer SysAdmin ever warn you not to GROK a bot %s\002", getnick(from));
	  return;
}






