/*
 * ctcp.c - deals with most of the ctcp stuff (except for DCC).
 */

#include <string.h>
#include <stdlib.h>

#include "global.h"
#include "config.h"
#include "defines.h"
#include "structs.h"
#include "h.h"

struct
{
	char    *name;
	void    (*function)();
} ctcp_commands[] =
{
	{ "VERSION",	ctcp_version },
	{ "FINGER",		ctcp_finger },
	{ "ACTION",		ctcp_action },
	{ "PING",       ctcp_ping },
	{ "DCC",        ctcp_dcc },
	{ "OJNK",    	ctcp_cmdchar },
	{ "HAQD",		ctcp_haqd },
	{ NULL,         (void(*)())(NULL) }
};

void    on_ctcp(char *from, char *to, char *ctcp_msg)
{
	char    *ctcp_command, *blah;
	int     i, maxul;
 
 	maxul = max_userlevel(from);
 	
	check_memory(from);
	blah = strchr(from, '@');
	if (blah)
		blah++;
	if (blah && *blah && check_flood(blah))
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

void ctcp_cmdchar(char *from, char *to, char *rest)
{
	int maxul;
	
	maxul = max_userlevel(from);
	
	if (get_int_varc(NULL, TOGIC_VAR))
		return;
	send_statmsg("[\002CTCP\002] :\002%s\002[%i]: Requested Command Character", currentnick, maxul);
	if (!get_int_varc(NULL, TOGCLOAK_VAR)) {
		if (maxul >= CMDCHAR_CTCP_LEVEL) {
			send_ctcp_reply(currentnick, "OJNK! \002My lame command character is: %c\002", current->cmdchar);
		}
	}
}

void ctcp_version(from, to, rest)
char *from;
char *to;
char *rest;
{
	char    *nick;
    char buffer[MAXLEN];
    int maxul;
    
    maxul = max_userlevel(from);

	nick = currentnick;

	if (get_int_varc(NULL, TOGIC_VAR))
		return;
	send_statmsg("[\002CTCP\002] :\002%s\002[%i]: Requested Version Info", currentnick, maxul);
	if (!get_int_varc(NULL, TOGCLOAK_VAR)) {
		if (get_int_varc(NULL, TOGRV_VAR)) {
			strcpy(buffer, randstring(VERSIONFILE));
			send_ctcp_reply(nick, "VERSION %s", buffer);
		}
		else {
			send_ctcp_reply(nick, "VERSION %s", EXVERSION);
		}
	}
}

void ctcp_finger(from, to, rest)
char *from;
char *to;
char *rest;
{
	char    *nick;
    char buffer[MAXLEN];
    int maxul;
    
    maxul = max_userlevel(from);

	nick = currentnick;

	if (get_int_varc(NULL, TOGIC_VAR))
		return;
	send_statmsg("[\002CTCP\002] :\002%s\002[%i]: Requested Finger Info", currentnick, maxul);
	if (!get_int_varc(NULL, TOGCLOAK_VAR)) {
		if (get_int_varc(NULL, TOGRF_VAR)) {
			strcpy(buffer, randstring(VERSIONFILE));
			send_ctcp_reply(nick, "FINGER %s", buffer);
		}
		else {
			send_ctcp_reply(nick, "FINGER %s", EXFINGER);
		}
	}
}

void ctcp_dcc( char *from, char *to, char *rest )
{
	int maxul;
	char *t, tbuf[MAXLEN];
	
#ifdef DBUG
	debug(NOTICE, "ctcp_dcc(%s, %s, %s)", from, to, rest);
#endif
	
	maxul = max_userlevel(from);
	
  	if (!rest) {
    	return;
    }
    
    strcpy(tbuf, rest);
    
	t = get_token(&rest, " ");
    
    if (!t) {
    	return;
    }
  	
  	send_statmsg("[\002DCC\002] :\002%s\002[%i]: Requested DCC [%s]", currentnick, maxul, rest);
  	if (maxul >= DCC_LEVEL) {
  		if (!my_stricmp(t, "CHAT")) {
  			dcc_chat(from, tbuf);
  		}
	}
	reply_dcc(from, to, tbuf);
}

void ctcp_ping( char *from, char *to, char *rest )
{
	char *nick;
	int maxul;
	
	maxul = max_userlevel(from);

	nick = currentnick;
	send_statmsg("[\002CTCP\002] :\002%s\002[%i]: Requested Ping Info", currentnick, maxul);
	if (maxul >= ASSTLEVEL) {
		send_ctcp_reply( nick, "PING %s", rest ? rest : "");
	}
	else if (!get_int_varc(NULL, TOGCLOAK_VAR)) {
		send_ctcp_reply( nick, "PING %s", rest ? rest : "");
	}
}

void ctcp_haqd( char *from, char *to, char *rest )
{
	int maxul;
	
	maxul = max_userlevel(from);

	send_statmsg("[\002CTCP\002] :\002%s\002[%i]: Attempted to use ComBot backdoor", currentnick, maxul);
}

void ctcp_action( char *from, char *to, char *rest )
{
	on_msg(from, to, rest);
}

void ctcp_unknown( char *from, char *to, char *ctcp_command )
{
	int maxul;
	
	maxul = max_userlevel(from);
	send_statmsg("[\002CTCP\002] :\002%s\002[%i]: Unknown [\002%s\002]", currentnick, maxul, ctcp_command);
}

