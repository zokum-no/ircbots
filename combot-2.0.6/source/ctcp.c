/*
 * ctcp.c - deals with most of the ctcp stuff (except for DCC).
 * (c) 1995 CoMSTuD (cbehrens@slavery.com)
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
	{ "ACTION",	ctcp_action },
	{ "PING",       ctcp_ping },
	{ "DCC",        ctcp_dcc },
	{ "CMDCHAR",    ctcp_cmdchar },
	{ NULL,         (void(*)())(NULL) }
};

void    on_ctcp(char *from, char *to, char *ctcp_msg)
{
	char    *ctcp_command, *blah;
	int     i;
 
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
	if (get_int_varc(NULL, TOGIC_VAR))
		return;
	send_ctcp_reply(currentnick,
		"CMDCHAR \002My command character is: %c\002",
		current->cmdchar);
}

void ctcp_version( char *from, char *to, char *rest )
{
	char    *nick;

	if (get_int_varc(NULL, TOGIC_VAR))
		return;
	nick = currentnick;

	send_ctcp_reply( nick, "VERSION \002ComBot Version %s\002", VERSION); 
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

	nick = currentnick;
	send_ctcp_reply( nick, "PING %s", rest ? rest : "");
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
	nick = currentnick;

	send_ctcp_reply( nick, "ERROR Unknown ctcp-command");
}

