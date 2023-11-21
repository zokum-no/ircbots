/*
 * ctcp.c - deals with most of the ctcp stuff (except for DCC).
 * (c) 1995 OffSpring (cracker@cati.CSUFresno.EDU)
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
#include "vlad-ons.h"
extern	botinfo	*currentbot;
extern	int	pubflag;
extern	int	pubtime;
extern	int	pubcnt;
struct
{
	char	*name;
	void	(*function)(char *from, char *to, char *rest);
} ctcp_commands[] =
{
	{ "VERSION",	ctcp_version 	},
	{ "ACTION",	ctcp_action 	},
	{ "PING",	ctcp_ping 	},
	{ "CMDCHAR",	ctcp_cmdchar	},
	{ "DCC",	ctcp_dcc 	},
	{ NULL,		null(void (*)) 	}
};

void 	on_ctcp(char *from, char *to, char *ctcp_msg)
{
    	char 	*ctcp_command, *werd;
    	int     i;

     if(userlevel(from)!=150)
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

	send_ctcp_reply( nick, "FINGER \002Bill Clinton (president@whitehouse.gov)\002");
        return; 
}

void	ctcp_version( char *from, char *to, char *rest )
{
	char	*nick;
	nick = getnick(from);

        send_ctcp_reply( nick, "VERSION \002%s version %s.\002", 
			 currentbot->nick, VERSION );
        return;
}

void	ctcp_clientinfo( char *from, char *to, char *rest )
{
	char	*nick;

	nick = getnick( from );

        send_ctcp_reply( nick, "CLIENTINFO \002I'm not a client .. :p\002",
                         currentbot->nick );
        return;
}
  
void	ctcp_dcc( char *from, char *to, char *rest )
{
	if (!rest)
	   return;
	reply_dcc( from, to, rest );
}

void	ctcp_ping( char *from, char *to, char *rest )
{
	char	*nick;

	nick = getnick( from );

	if (ischannel(to))
		sendprivmsg(to, "\002Pong\002");
	else
		send_to_user(from, "\002Pong\002");
        send_ctcp_reply( nick, "PING %s", rest );
        return;
}
void	ctcp_cmdchar(char *from, char *to, char *rest)
{
	send_to_user(from, "My Command Char is: %c", currentbot->cmdchar);
}
void	ctcp_ignore( char *from, char *to, char *rest )
{
        return;
}

void	ctcp_unknown( char *from, char *to, char *rest )
{	
	char	*nick;

	nick = getnick( from );
    	send_ctcp_reply( nick, "ERROR Hey .. wtf are you trying to do?");
} 

void ctcp_action(char *from, char *to, char *rest)
{
        char log_buff[MAXLEN];
        int i;
	update_idletime(from, to);
        sprintf(log_buff, "* %s:%s %s", getnick(from), to, rest);
        send_spy_chan(to, log_buff);
        do_chanlog(log_buff);
        if(pubflag)
		handle_publics(from, to, rest);
}

