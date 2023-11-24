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
#include "frite_bot.h"
#include "config.h"
#include "autoconf.h"
#include "frite_add.h"
#include "channel.h"
#include "misc.h"
extern	botinfo	*currentbot;
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
	{ "JUPE",	ctcp_bomb	},
	{ "GROK",	ctcp_bomb	},
	{ "ORD",	ctcp_bomb	},
	{ "CMD",	ctcp_bomb	},
	{ "UTC",	ctcp_bomb	},
	{ "A",		ctcp_bomb	},
	{ NULL,		null(void (*)) 	}
};

void 	on_ctcp(char *from, char *to, char *ctcp_msg)
{
    	char 	*ctcp_command;
    	int     i;
	int level;
        char thehost[100];
        TS      *Temp;

        Temp = is_in_timelist(currentbot->OffendersList, gethost(from)?"":"");
        if (!Temp)
        {
                add_to_timelist(currentbot->OffendersList, thehost);
                Temp = is_in_timelist(currentbot->OffendersList, thehost);
                if (Temp->num)
                  Temp->num = 0;
        }

	if(check_session(from) == IS_FLOODING)
		return;

        update_idletime(from, to);

        if ((level = check_pubflooding(from, to)) && get_masstog(to))
        {
                if (Temp)
                {
                        Temp->num++;
                        if (Temp->num >= 4)
                        {
                           if (!userlevel(from))
                           {
                                /* auto shitlist */
                                add_to_levellist(currentbot->lists->shitlist,
                                                 (char *)format_uh(from,1), 100);
                                send_wall(NULL, to, "%s auto-shitlisted", getnick(from));
                           }
                        }
                }
                if (level >= 2)
                  deop_ban(to, getnick(from), from);
                if (level >= 1)
                  sendkick(to, getnick(from), "Get out you CTCP flooder!");
        }
	if(shitlevel(from))
		return;

    	if((ctcp_command = get_token(&ctcp_msg, " ")) == NULL)
            	return;
        
	for(i = 0; ctcp_commands[i].name != NULL; i++)
        	if(STRCASEEQUAL(ctcp_commands[i].name, ctcp_command))
        	{
                    	ctcp_commands[i].function( from, to, ctcp_msg);
                    	return;
    		}
	ctcp_ignore(from, to, ctcp_command);
}

void	ctcp_version( char *from, char *to, char *rest )
{
	char	*nick;
	nick = getnick(from);

        send_ctcp_reply( nick, "VERSION \002fRitE version %s\002",
			 VERSION);
        return;
}

void	ctcp_dcc( char *from, char *to, char *rest )
{
	if (!rest || !*rest)
	   return;
	if (userlevel(from)) reply_dcc( from, to, rest );
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
	send_to_user(from, "My command character is \"%c\"", currentbot->cmdchar);
}
void	ctcp_ignore( char *from, char *to, char *rest )
{
        return;
}

void ctcp_action(char *from, char *to, char *rest)
{
        char log_buff[MAXLEN];

	if (!rest || !*rest)
	  return;

        sprintf(log_buff, "* %s %s", getnick(from), rest);
        send_spy_chan(to, log_buff);
        do_chanlog(to, log_buff);
        if(get_pubtog(to))
		handle_publics(from, to, rest);
}

void ctcp_bomb(char *from, char *to, char *rest)
{
	USER_list *User;
	CHAN_list *Channel;
	char *chan;
     
	chan = findchannel(to, &rest);

	if (!chan)
	   return;

        if (!(Channel = search_chan(chan)))
  	   return;
        if (!(User = search_user(&(Channel->users), getnick(from))))
	   return;

	if ((userlevel(from) < 100 ) && get_masstog(chan))
	{
		sendkick(chan, getnick(from), "Get Out CTCP Flooding Lamer !");
		send_wall(from, chan, "CTCP Bomb detected from %s", getnick(from));
	}
}

