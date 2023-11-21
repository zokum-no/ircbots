/*
 * vlad-ons.c - kinda like /on ^.. in ircII
 * (c) 1993 VladDrac (irvdwijk@cs.vu.nl)
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>

#include "channel.h"
#include "config.h"
#include "dcc.h"
#include "debug.h"
#include "ftext.h"
#include "ftp_dcc.h"
#include "misc.h"
#include "parsing.h"
#include "send.h"
#include "session.h"
#include "userlist.h"
#include "vladbot.h"
#include "vlad-ons.h"

extern	botinfo	*currentbot;
extern	int	number_of_bots;
extern	int	rehash;
extern	long	uptime;
extern	char	*botmaintainer;

/* external parseroutines ("prefix commands")	*/
extern	void	parse_global(char *from, char *to, char *rest);

command_tbl on_msg_commands[] =

/* Any command which produces more than 2 lines output can be 
   used to make the bot "excess flood". Make it "forcedcc"     */

/*	Command		function   userlvl   shitlvl  forcedcc */
{
	{ "HELP",	show_help,	0,	100,	TRUE  }, 
	{ "WHOAMI",	show_whoami,  	0,	100,	TRUE  },
	{ "INFO",	show_info,	0,	100,	TRUE  },
	{ "TIME",	show_time,	0,	100,	FALSE },	
	{ "USERLIST",	show_userlist,	0,	100,	TRUE  },	
	{ "SHITLIST",	show_shitlist,	0,	100,	TRUE  },
	{ "PROTLIST",	show_protlist,	0,	100,	TRUE  },
	{ "LISTDCC",	do_listdcc,	0,	100,	TRUE  },
	{ "CHAT",	do_chat,	0,	100,	FALSE },
	{ "SEND",	do_send,	0,	100,	FALSE },
	/* These two are just aliases for ftp-dcc		*/
	{ "GET",	do_send,	0,	100,	FALSE },
	{ "MGET",	do_send,	0,	100,	FALSE },
	/* Some more ftp-dcc functions				*/
	{ "LS",		show_dir,	0,	100,	TRUE  },
	{ "PWD",	show_cwd,	0,	100,	FALSE },
	{ "CD",		do_cd,		0,	100,	FALSE },
	{ "QUEUE",	show_queue,	0,	100,	TRUE  },
	{ "FILES",	do_flist,       0,      100,    TRUE  },
	{ "FLIST",	do_flist,	0,	100,	TRUE  },
	/* userlevel and dcc are dealt with within NOTE 	*/
	/* userlevel and dcc are dealt with within GLOBAL 	*/
	{ "GLOBAL",	parse_global,	0,	100,	FALSE },
	{ "OP",		do_op, 		50,	0,	FALSE },
	{ "GIVEOP",	do_giveop,	50,	0,	FALSE },
	{ "DEOP",	do_deop,	50,	0,	FALSE },
	{ "INVITE",	do_invite,	50,	0,	FALSE },
	{ "OPEN",	do_open,	100,	0,	FALSE },
	{ "SAY",	do_say,		100,	0,	FALSE },
	{ "WHOIS",	show_whois,	100,	0,	TRUE  },
	{ "NWHOIS",	show_nwhois,	100,	0,	TRUE  },
	{ "NUSERADD",	do_nuseradd,	100,	0,	FALSE },
	{ "USERWRITE",	do_userwrite,	100,	0,	FALSE },
	{ "USERDEL",	do_userdel,	100,	0,	FALSE },
	{ "NSHITADD",	do_nshitadd,	100,	0,	FALSE },
	{ "SHITWRITE",	do_shitwrite,	100,	0,	FALSE },
	{ "SHITDEL",	do_shitdel,	100,	0,	FALSE },
        { "PROTWRITE", 	do_protwrite,   100,    0,	FALSE },
        { "PROTDEL",   	do_protdel,     100,    0,	FALSE },
	{ "BANUSER",	do_banuser,	100,	0,	FALSE },
	{ "UNBAN",	do_unban,	100,	0,	FALSE },
	{ "KICK",	do_kick,	100,	0,	FALSE },
	{ "JOIN",	do_join,	125,	0,	FALSE },
	{ "LEAVE",	do_leave,	125,	0,	FALSE },
	{ "NICK",	do_nick,	125,	0,	FALSE },
	{ "USERADD",	do_useradd,	125,	0,	FALSE },
	{ "SHITADD",	do_shitadd,	125,	0,	FALSE },
        { "PROTADD",   	do_protadd,     125,    0,	FALSE },
        { "NPROTADD",  	do_nprotadd,    125,    0,	FALSE },
	{ "CHANNELS",	show_channels,	125,	0,	TRUE  },
	{ "SHOWUSERS",	do_showusers,	125,	0,	TRUE  },
	{ "MASSOP",	do_massop,	125,	0,	FALSE },
	{ "MASSDEOP",	do_massdeop,	125,	0,	FALSE },
	{ "MASSKICK",	do_masskick,	125,	0,	FALSE },
	{ "MASSUNBAN",	do_massunban,	125,	0,	FALSE },
	{ "SERVER",	do_server,	125,	0,	FALSE },
	/* Priviliged commands					*/	
	{ "FORK",	do_fork,	150,	0,	FALSE },
	{ "REHASH",	do_rehash,	150,	0,	FALSE },
	{ "DO",		do_do,		150,	0,	FALSE },
	{ "DIE",	do_die,		150,	0,	FALSE },
	{ "DIEDIE",	do_die,		150,	0,	FALSE },
	{ "QUIT",	do_quit,	150,	0,	FALSE },
	/*
	 :
	 :
	 */
	{ NULL,		null(void(*)()), 0,	0,	FALSE }
};

void	on_kick(char *from, char *channel, char *nick, char *reason)
{
	/*
	 * If user A kicks user B then kick user A if B was protected
	 * and A has lower protlevel
	 */
	/*
	 * Perhaps vladbot is getting to much warbot with this piece
	 * of code. Maybe add an #ifdef...
	 */
	if(shitlevel(username(nick)) == 0 &&
	   protlevel(username(nick)) >= 100 &&
	   protlevel(from) < 100 &&
	   /* I should never revenge myself :) */
	   !STRCASEEQUAL(currentbot->nick, getnick(from)))
		sendkick(channel, getnick(from), "That wasn't very smart!");
}

void 	on_join(char *who, char *channel)
{
    	if(shitlevel(who) == 100)
    	{
		ban_user(who, channel);
        	sendkick(channel, getnick(who), "You're not wanted here");
        	return;
    	}
    	if(userlevel(who) >= AUTO_OPLVL && shitlevel(who) == 0)
        	giveop(channel, getnick(who));
}

void	on_mode(char *from, char *rest)
/*
 * from = who did the modechange
 * rest = modestring, usually of form +mode-mode param1 .. paramN
 */
#define REMOVEMODE(chr, prm) do{ strcat(unminmode, chr); \
			         strcat(unminparams, prm); \
			         strcat(unminparams, " "); \
			     } while(0)

#define ADDMODE(chr, prm)    do{ strcat(unplusmode, chr); \
			         strcat(unplusparams, prm); \
			         strcat(unplusparams, " "); \
			     } while(0)
{
	int	did_change=FALSE;	/* If we have to change anything */
	char	*channel;

	char	*chanchars;	/* i.e. +oobli */
	char	*params;
	char	sign = '+';	/* +/-, i.e. +o, -b etc */

	char	unminmode[MAXLEN] = "";		/* used to undo modechanges */
	char	unminparams[MAXLEN] = "";
	char	unplusmode[MAXLEN] = "";	/* used to undo modechanges */
	char	unplusparams[MAXLEN] = "";

	/* these are here merely to make things clearer */
	char	*nick;
	char	*banstring;
	char	*key;
	char	*limit;

#ifdef DBUG
	debug(NOTICE, "on_mode(\"%s\", \"%s\")", from, rest);
#endif

	channel = get_token(&rest, " ");
	if(STRCASEEQUAL(channel, currentbot->nick))	/* if target is me... */
		return;			/* mode NederServ +i etc */

	chanchars = get_token(&rest, " ");
	params = rest;

	while( *chanchars )
	{
		switch( *chanchars )
		{
		case '+':
		case '-':
			sign = *chanchars;
			break;
		case 'o':
			nick = get_token(&params, " ");
			if(sign == '+')
			{
				add_channelmode(channel, CHFL_CHANOP, nick);
#ifdef ONLY_OPS_FOR_REGISTRED
				if(shitlevel(username(nick)) > 0 ||
				   userlevel(username(nick)) == 0)
#else
				if(shitlevel(username(nick))>0)
#endif
				{
					REMOVEMODE("o", nick);
					send_to_user(from, 
#ifdef ONLY_OPS_FOR_REGISTRED
						"Sorry, I can't allow this :)");
#else
						"%s is shitted! :P", nick);
#endif
					did_change=TRUE;
				}
			}
			else
			{
				del_channelmode(channel, CHFL_CHANOP, nick);
				if((protlevel(username(nick))>=100)&&
				   (shitlevel(username(nick))==0))
				{
					ADDMODE("o", nick);
					send_to_user(from, 
						"%s is protected!",
						     nick);
					did_change=TRUE;
				}	
			}
			break;
		case 'v':
			nick = get_token(&params, " ");
			if(sign == '+')
				add_channelmode(channel, CHFL_VOICE, nick);
			else
				del_channelmode(channel, CHFL_VOICE, nick);
			break;
		case 'b':
			banstring = get_token(&params, " ");
                        if(sign == '+')
			{
                                add_channelmode(channel, MODE_BAN, banstring);
				if(find_highest(channel, banstring) >= 100)
				{
					REMOVEMODE("b", banstring);
					did_change = TRUE;
				}
				
			}
                        else
                                del_channelmode(channel, MODE_BAN, banstring);
                        break;
		case 'p':
			if(sign == '+')
				add_channelmode(channel, MODE_PRIVATE, "");
			else
				del_channelmode(channel, MODE_PRIVATE, "");
			break;
		case 's':
			if(sign == '+')
				add_channelmode(channel, MODE_SECRET, "");
			else
				del_channelmode(channel, MODE_SECRET, "");
			break;
		case 'm':
			if(sign == '+')
				add_channelmode(channel, MODE_MODERATED, "");
			else
				del_channelmode(channel, MODE_MODERATED, "");
			break;
		case 't':
			if(sign == '+')
				add_channelmode(channel, MODE_TOPICLIMIT, "");
			else
				del_channelmode(channel, MODE_TOPICLIMIT, "");
			break;
		case 'i':
			if(sign == '+')
				add_channelmode(channel, MODE_INVITEONLY, "");
			else
				del_channelmode(channel, MODE_INVITEONLY, "");
			break;
		case 'n':
			if(sign == '+')
				add_channelmode(channel, MODE_NOPRIVMSGS, "");
			else
				del_channelmode(channel, MODE_NOPRIVMSGS, "");
			break;
		case 'k':
                       	key = get_token(&params, " ");
			if(sign == '+')
			{
				char	*s;

				add_channelmode(channel, MODE_KEY, 
						key?key:"???");
				/* try to unset bogus keys */
				for(s = key; key && *s; s++)
					if(*s < ' ')
					{
						REMOVEMODE("k", key);
						did_change = TRUE;
						send_to_user(from,
						"No bogus keys pls");
						break;
					}
			}
			else
				del_channelmode(channel, MODE_KEY, "");
			break;
		case 'l':
			if(sign == '+')
			{
                        	limit = get_token(&params, " ");
				add_channelmode(channel, MODE_LIMIT, 
						limit?limit:"0");
			}
			else
				del_channelmode(channel, MODE_LIMIT, "");
			break;
		default:
#ifdef DBUG
			debug(ERROR, "on_mode(): unknown mode %c", *chanchars);
#endif
			break;
		}
		chanchars++;
	}
	/* restore unwanted modechanges */
	if(did_change)
		sendmode( channel, "+%s-%s %s %s", unplusmode, 
			  unminmode, unplusparams, unminparams);
}

void	on_msg(char *from, char *to, char *msg)
{
	int	i;
	char	msg_copy[MAXLEN];	/* for session */
	char	*command;
	DCC_list	*userclient;

	strcpy(msg_copy, msg);

	if((command = get_token(&msg, ",: "))== NULL)
		return;			/* NULL-command */

	if(STRCASEEQUAL(currentbot->nick, command))
	{
		if((command = get_token(&msg, ",: "))==NULL)
			return;		/* NULL-command */
	}
	else if((*command != PREFIX_CHAR) && !STRCASEEQUAL(to, currentbot->nick))
		return; /* The command should start with PREFIX_CHAR if public */

	if(*command == PREFIX_CHAR)
		command++;

	/*
	 * Now we got
	 * - A public command.
	 * - A private command/message. 
	 * Time to do a flood check :).
	 */
	if(check_session(from) == IS_FLOODING)
		return;
	
	for(i = 0; on_msg_commands[i].name != NULL; i++)
		if(STRCASEEQUAL(on_msg_commands[i].name, command))
		{
			if(userlevel(from) < on_msg_commands[i].userlevel)
			{
				send_to_user(from, "Userlevel too low");
				return;
			}
		        if(shitlevel(from) > on_msg_commands[i].shitlevel)
			{
				send_to_user(from, "Shitlevel too high");
				return;
			}
			userclient = search_list("chat", from, DCC_CHAT);
			if(on_msg_commands[i].forcedcc && 
		    	  (!userclient || (userclient->flags&DCC_WAIT)))
			{
#ifdef AUTO_DCC
				dcc_chat(from, msg);
				nodcc_session(from, to, msg_copy);
				sendnotice(getnick(from), "Please type: /dcc chat %s", 
					currentbot->nick);
#else
				sendnotice( getnick(from), 
					"Sorry, %s is only available through DCC",
					command );
                		sendnotice( getnick(from), 
					"Please start a dcc chat connection first" );
#endif /* AUTO_DCC */
                			return;
			}
			/* if we're left with a null-string (""), give NULL
			   as msg */
			on_msg_commands[i].function(from, to, *msg?msg:NULL);
			return;
		}
	/* If the command was private, let the user know how stupid (s)he is */
	if(STRCASEEQUAL(to, currentbot->nick))
		send_to_user(from, "%s %s?!? What's that?",
			     command, msg ? msg : "");
}

void	show_help(char *from, char *to, char *rest)
{
	FILE	*f;
	char	*s;

	if((f=fopen(currentbot->helpfile, "r"))==NULL)
	{
		send_to_user(from, "Helpfile missing");
		return;
	}
	
	if(rest == NULL)
	{
		find_topic(f, "standard");
		while((s=get_ftext(f)))
			send_to_user(from, s);
	}
	else
		if(!find_topic( f, rest ))
			send_to_user(from, "No help available for \"%s\"", rest);
		else
			while((s=get_ftext(f)))
				send_to_user(from, s);
	fclose(f);
}

void	show_whoami(char *from, char *to, char *rest)
{
        send_to_user(from, "You are %s. Your levels are:", 
		      from);
	send_to_user(from, "-- User -+- Shit -+- Protect --");
	send_to_user(from, "    %3d  |   %3d  |      %3d", userlevel(from),
		      shitlevel(from), protlevel(from));
        return;
}

void	show_info(char *from, char *to, char *rest)
{
	sendreply("I am VladBot version %s (%s)", VERSION, currentbot->botname);
	sendreply("Started: %-20.20s", time2str(currentbot->uptime));
	sendreply("Up: %s", idle2str(time(NULL)-currentbot->uptime));
	if(botmaintainer)
		sendreply("This bot is maintained by %s", botmaintainer);
        return;
}

void	show_time(char *from, char *to, char *rest)
{
	sendreply("Current time: %s", time2str(time(NULL)));
}

void	show_userlist(char *from, char *to, char *rest)
{
	send_to_user(from, " userlist: %30s   %s", "nick!user@host", "level");
        send_to_user(from, " -----------------------------------------+------");
	show_lvllist(currentbot->lists->opperlist, from, rest?rest:"");
}

void	show_shitlist(char *from, char *to, char *rest)
{
	send_to_user(from, " shitlist: %30s   %s", "nick!user@host", "level");
        send_to_user(from, " -----------------------------------------+------");
	show_lvllist(currentbot->lists->shitlist, from, rest?rest:"");
}

void    show_protlist(char *from, char *to, char *rest)
{
        send_to_user(from, " protlist: %30s   %s", "nick!user@host", "level");
        send_to_user(from, " -----------------------------------------+------");
	show_lvllist(currentbot->lists->protlist, from, rest?rest:"");
}

void	do_op(char *from, char *to, char *rest)
{
	if(usermode(!STRCASEEQUAL(to, currentbot->nick) ? to : currentchannel(),  
	   getnick(from))&MODE_CHANOP)
		send_to_user(from, "You're already channel operator!");
	else
        	giveop(!STRCASEEQUAL(to, currentbot->nick) ? to : currentchannel(),  
		       getnick(from));
}

void	do_giveop(char *from, char *to, char *rest)
{
	int	i = 0;
	char	nickname[MAXNICKLEN];
	char	op[MAXLEN];

	strcpy(op, "");
	if(not(rest))
	{
		sendreply("Who do you want me to op?");
		return;
	}
	while(readnick(&rest, nickname))
		if(username(nickname) != NULL && 
		   shitlevel(username(nickname)) == 0)
		{
			i++;
			strcat(op, " ");
			strcat(op, nickname);
			if(i==3)
			{
				sendmode(ischannel(to) ?to :currentchannel(), "+ooo %s", op);
				i = 0;
				strcpy(op, "");
			}
		}
	if(i != 0)
		sendmode(ischannel(to) ?to :currentchannel(), "+ooo %s", op);
}

void	do_deop(char *from, char *to, char *rest)
{
	int	i = 0;
	char	nickname[MAXNICKLEN];
	char	deop[MAXLEN];

	strcpy(deop, "");
	if(not(rest))
	{
		sendreply("Who do you want me to deop?");
		return;
	}
	while(readnick(&rest, nickname))
		if(username(nickname) != NULL &&
		  (protlevel(username(nickname)) < 100 ||
		   shitlevel(username(nickname)) > 0))
		{
		i++;
			strcat(deop, " ");
			strcat(deop, nickname);
			if(i==3)
			{
				sendmode(ischannel(to) ?to :currentchannel(), "-ooo %s", deop);
				i = 0;
				strcpy(deop, "");
			}
		}
	if(i != 0)
		sendmode(ischannel(to) ?to :currentchannel(), "-ooo %s", deop);
}

void	do_invite(char *from, char *to, char *rest)
{
        if(rest)
	{
		if(!invite_to_channel(getnick(from), rest))	
			send_to_user(from, "I'm not on channel %s", rest);
	}
	else
		invite_to_channel(from, currentchannel());
}
			
void	do_open(char *from, char *to, char *rest)
{
        if(rest)
        {
		if(!open_channel(rest))
			send_to_user(from, "I could not open %s!", rest);
		else
			channel_unban(rest, from);
	}
	else if(ischannel(to))
	{
		if(!open_channel(to))
			send_to_user(from, "I could not open %s!", to);
		else
			channel_unban(to, from);
	}
	else 
	{
		open_channel(currentchannel());
		channel_unban(currentchannel(), from);
	}
}

void    do_chat(char *from, char *to, char *rest)
{
	dcc_chat(from, rest);
	sendnotice(getnick(from), "Please type: /dcc chat %s", currentbot->nick);
}

void    do_send(char *from, char *to, char *rest)
{
	char	*pattern;
	
	if(rest)
		while((pattern = get_token(&rest, " ")))
			send_file(from, pattern);
	else
		send_to_user(from, "Please specify a filename (use !files)");
}

void	do_flist(char *from, char *to, char *rest)
{
	FILE	*ls_file;
	char	indexfile[MAXLEN];
	char	*s;
	char	*p;

	strcpy(indexfile, currentbot->indexfile );
	if(rest)
	{
		for(p=rest; *p; p++) *p=tolower(*p);
		sprintf(indexfile, "%s.%s", currentbot->indexfile, rest);
		if((ls_file = openindex(from, indexfile)) == NULL)
		{
			send_to_user(from, "No indexfile for %s", rest);
			return;
		}
	}
	else
		if((ls_file = openindex(from, currentbot->indexfile)) == NULL)
		{
			send_to_user(from, "No indexfile");
			return;
		}

	while((s=get_ftext(ls_file)))
		send_to_user(from, s);
	fclose(ls_file);
}	

void	do_say(char *from, char *to, char *rest)
{
	if(rest)
            	if(ischannel(to))
                	sendprivmsg(to, "%s", rest);
            	else
                	sendprivmsg(currentchannel(), "%s", rest);
        else
            	send_to_user(from, "I don't know what to say");
        return;
}

void	do_do(char *from, char *to, char *rest)
{
	if(rest)
            send_to_server(rest);
        else
            send_to_user(from, "What do you want me to do?");
        return;
}

void	show_channels(char *from, char *to, char *rest)
{
        show_channellist(from);
        return;
}


void	do_join(char *from, char *to, char *rest)
{
	if(rest)
            join_channel(rest, "", "", TRUE);
        else
            send_to_user(from, "What channel do you want me to join?");
        return;
}

void	do_leave(char *from, char *to, char *rest)
{
	if(rest)
            	leave_channel(rest); 
        else
		if(STRCASEEQUAL(currentbot->nick, to))
			leave_channel(currentchannel());
		else
			leave_channel(to);
        return;
}
 
void	do_nick(char *from, char *to, char *rest)
{
	if(rest)
        {
		if(!isnick(rest))
		{
			send_to_user(from, "Illegal nickname %s", rest);
			return;
		}	
            	strncpy(currentbot->nick, rest, NICKLEN);
            	strncpy(currentbot->realnick, rest, NICKLEN);
            	sendnick(currentbot->nick); 
        }
        else
            	send_to_user(from, "You need to tell me what nick to use");
        return;
}

void	do_die(char *from, char *to, char *rest)
{
	if( rest != NULL )
            	signoff( from, rest );
        else
            	signoff( from, "Bye bye!" );
	if(number_of_bots == 0)
		exit(0);
}

void	do_quit(char *from, char *to, char *rest)
{
	quit_all_bots(from, "Quiting all bots.");
	exit(0);
}

void    show_whois(char *from, char *to, char *rest)
{
	if(rest == NULL)
	{
		send_to_user(from, "Please specify a user");
		return;
	} 
 
        send_to_user(from, "%s's levels are:",
                     rest);
        send_to_user(from, "-- User -+- Shit -+- Protect --" );
        send_to_user(from, "    %3d  |   %3d  |      %3d", userlevel(rest),
                     shitlevel(rest), protlevel(rest));
        return;
}

void	show_nwhois(char *from, char *to, char *rest)
{
	char	*nuh;

        if( rest == NULL )
        {
                send_to_user( from, "Please specify a nickname" );
                return;
        }
        if((nuh=username(rest))==NULL)
        {
                send_to_user(from, "%s is not on this channel!", rest);
                return;
        }

        send_to_user(from, "USERLIST:-------------Matching pattern(s) + level");
	show_lvllist( currentbot->lists->opperlist, from, nuh);
	send_to_user(from, "SHITLIST:---------------------------------+");
        show_lvllist( currentbot->lists->shitlist, from, nuh);
	send_to_user(from, "PROTLIST:---------------------------------+");
        show_lvllist( currentbot->lists->protlist, from, nuh);
	send_to_user(from, "End of nwhois-----------------------------+");
}

void	do_nuseradd(char *from, char *to, char *rest)
{
	char	*newuser;
	char	*newlevel;
	char	*nuh;		/* nick!@user@host */

        if((newuser = get_token(&rest, " ")) == NULL)
	{
             	send_to_user( from, "Who do you want me to add?" );
		return;
	}
	if((nuh=username(newuser))==NULL)
	{
		send_to_user(from, "%s is not on this channel!", newuser);
		return;
	}
        if((newlevel = get_token(&rest, " ")) == NULL)
	{
                send_to_user( from, "What level should %s have?", newuser );
		return;
	}


        if(atoi(newlevel) < 0)
		send_to_user(from, "level should be >= 0!");
	else if(atoi(newlevel) > userlevel(from))
		send_to_user( from, "Sorry bro, can't do that!" );
	else if(userlevel(from) < userlevel(nuh))
		send_to_user(from, "Sorry, %s has a higher level", newuser);
	else
	{
		char	*nick;
		char	*user;
		char	*host;
		char	*domain;
		char	userstr[MAXLEN];

		nick=get_token(&nuh, "!");
		user=get_token(&nuh,"@");
		if(*user == '~' || *user == '#') user++;
		host=get_token(&nuh, ".");
		domain=get_token(&nuh,"");

		sprintf(userstr, "*!*%s@*%s", user, domain?domain:host);
               	send_to_user( from, "User %s added with access %d as %s", 
			      newuser, atoi( newlevel ), userstr );
               	add_to_levellist( currentbot->lists->opperlist, 
                                  userstr, atoi( newlevel ) );
	}
        return;
}                   


void	do_useradd(char *from, char *to, char *rest)
{
	char	*newuser;
	char	*newlevel;

        if((newuser = get_token(&rest, " ")) == NULL)
	{
             	send_to_user( from, "Who do you want me to add?" );
		return;
	}
        if((newlevel = rest) == NULL)
	{
                send_to_user( from, "What level should %s have?", newuser );
		return;
	}
        if(atoi(newlevel) < 0)
		send_to_user(from, "level should be >= 0!");
	else if(atoi( newlevel) > userlevel(from))
		send_to_user(from, "Sorry bro, can't do that!");
	else if(userlevel(from) < userlevel(newuser))
		send_to_user(from, "Sorry, %s has a higher level", newuser);
	else
	{
               	send_to_user( from, "User %s added with access %d", 
			      newuser, atoi(newlevel));
               	add_to_levellist(currentbot->lists->opperlist, newuser, atoi(newlevel));
	}
        return;
}                   

void    do_userwrite(char *from, char *to, char *rest)
{
        if(!write_lvllist(currentbot->lists->opperlist, 
            currentbot->lists->opperfile))
                send_to_user(from, "Userlist could not be written to file %s", 
                              currentbot->lists->opperfile);
        else
                send_to_user(from, "Userlist written to file %s", 
                              currentbot->lists->opperfile);
}

void	do_userdel(char *from, char *to, char *rest)
{
        if(rest)
        {
            	if (userlevel(from) < userlevel(rest))
			send_to_user(from, "%s has a higer level.. sorry",
				     rest);
            	else if (!remove_from_levellist(currentbot->lists->opperlist,
						rest))
                	send_to_user(from, "%s has no level!", rest);
	    	else
                	send_to_user(from, "User %s has been deleted", rest);
        }
	else
        	send_to_user(from, "Who do you want me to delete?");
        return;
}

void	do_nshitadd(char *from, char *to, char *rest)
{
	char	*newuser;
	char	*newlevel;
	char	*nuh;		/* nick!@user@host */

        if((newuser = get_token(&rest, " ")) == NULL)
	{
             	send_to_user(from, "Who do you want me to add?");
		return;
	}
	if((nuh=username(newuser))==NULL)
	{
		send_to_user(from, "%s is not on this channel!", newuser);
		return;
	}
        if((newlevel = get_token(&rest, " ")) == NULL )
	{
                send_to_user(from, "What level should %s have?", newuser);
		return;
	}


        if(atoi(newlevel) < 0)
		send_to_user(from, "level should be >= 0!");
	else if(atoi(newlevel) > userlevel(from))
		send_to_user(from, "Sorry bro, can't do that!");
	else if(userlevel(from) < userlevel(nuh))
		/* This way, someone with level 100 can't shit someone with level 150 */
		send_to_user(from, "Sorry, %s has a higher level", newuser);
	else
	{
		char	*nick;
		char	*user;
		char	*host;
		char	*domain;
		char	userstr[MAXLEN];

		nick=get_token(&nuh, "!");
		user=get_token(&nuh,"@");
		if(*user == '~' || *user == '#') user++;
		host=get_token(&nuh, ".");
		domain=get_token(&nuh,"");

		sprintf(userstr, "*!*%s@*%s", user, domain?domain:host);
               	send_to_user(from, "User %s shitted with access %d as %s", 
			     newuser, atoi(newlevel), userstr);
               	add_to_levellist(currentbot->lists->shitlist, userstr, atoi(newlevel));
	}
        return;
}                   

void	do_shitadd(char *from, char *to, char *rest)
{
	char	*newuser;
	char	*newlevel;

        if((newuser = get_token(&rest, " ")) == NULL)
	{
             	send_to_user(from, "Who do you want me to add?");
		return;
	}
        if((newlevel = rest) == NULL)
	{
                send_to_user(from, "What level should %s have?", newuser);
		return;
	}
        if(atoi(newlevel) < 0)
                send_to_user(from, "level should be >= 0!");
        else  
        {
                send_to_user(from, "User %s shitted with access %d", newuser, 
		             atoi(newlevel));
                add_to_levellist(currentbot->lists->shitlist, newuser, atoi(newlevel));
        } 
        return;
}                   

void	do_shitwrite(char *from, char *to, char *rest)
{
	if(!write_lvllist(currentbot->lists->shitlist, 
                          currentbot->lists->shitfile))
		send_to_user(from, "Shitlist could not be written to file %s", 
                             currentbot->lists->shitfile);
	else
		send_to_user(from, "Shitlist written to file %s", 
                             currentbot->lists->shitfile);
}

void	do_shitdel(char *from, char *to, char *rest)
{
	if(rest)
        {
            	if(!remove_from_levellist(currentbot->lists->shitlist, rest))
	      		send_to_user(from, "%s is not shitted!", rest);
            	else
                	send_to_user(from, "User %s has been deleted", rest);
        }
	else
        	send_to_user(from, "Who do you want me to delete?");
        return;
}

void	do_nprotadd(char *from, char *to, char *rest)
{
	char	*newuser;
	char	*newlevel;
	char	*nuh;		/* nick!@user@host */

        if((newuser = get_token(&rest, " ")) == NULL)
	{
             	send_to_user(from, "Who do you want me to add?");
		return;
	}
	if((nuh=username(newuser))==NULL)
	{
		send_to_user(from, "%s is not on this channel!", newuser);
		return;
	}
        if((newlevel = get_token(&rest, " ")) == NULL)
	{
                send_to_user(from, "What level should %s have?", newuser);
		return;
	}


        if(atoi(newlevel) < 0)
		send_to_user(from, "level should be >= 0!");
	else if(atoi(newlevel) > 100)
		send_to_user(from, "yeah, right! You're shitted.");
	else
	{
		char	*nick;
		char	*user;
		char	*host;
		char	*domain;
		char	userstr[MAXLEN];

		nick=get_token(&nuh, "!");
		user=get_token(&nuh,"@");
		if(*user == '~' || *user == '#') user++;
		host=get_token(&nuh, ".");
		domain=get_token(&nuh,"");

		sprintf(userstr, "*!*%s@*%s", user, domain?domain:host);
               	send_to_user(from, "User %s protected with access %d as %s", 
			     newuser, atoi(newlevel), userstr);
               	add_to_levellist(currentbot->lists->protlist, userstr, atoi(newlevel));
	}
        return;
}                   

void    do_protadd(char *from, char *to, char *rest)
{
        char    *newuser;
        char    *newlevel;

        if((newuser = get_token(&rest, " ")) == NULL)
	{
             	send_to_user(from, "Who do you want me to add?");
		return;
	}
        if((newlevel = rest) == NULL )
	{
                send_to_user(from, "What level should %s have?", newuser);
		return;
	}
        if(atoi(newlevel) < 0)
                send_to_user(from, "level should be >= 0!");
        else  
        {
                send_to_user(from, "User %s protected with access %d", newuser,
                             atoi(newlevel));
                add_to_levellist(currentbot->lists->protlist, newuser, atoi(newlevel));
        }
        return;
}

void    do_protwrite(char *from, char *to, char *rest)
{
        if(!write_lvllist(currentbot->lists->protlist, 
                          currentbot->lists->protfile))
                send_to_user(from, "Protlist could not be written to file %s", 
                             currentbot->lists->protfile);
        else
                send_to_user(from, "Protlist written to file %s", 
                             currentbot->lists->protfile);
}

void    do_protdel(char *from, char *to, char *rest)
{
        if(rest)
        {
            	if(!remove_from_levellist(currentbot->lists->protlist, rest))
           		send_to_user(from, "%s is not protected!", rest);
            	else
                	send_to_user(from, "User %s has been deleted", rest);
        }
	else
        	send_to_user(from, "Who do you want me to delete?");
        return;
}

void	do_banuser(char *from, char *to, char *rest)
{
	char	*user_2b_banned;
	char	*channel;

	if(ischannel(to))
		channel = to;
	else
		channel = currentchannel();
		
	if(rest)
		if((user_2b_banned = username(rest)))
			ban_user(user_2b_banned, channel);
		else
			send_to_user(from, "%s not on this channel", rest);
	else
		send_to_user( from, "No." );
}

void	do_showusers(char *from, char *to, char *rest)
{
        char    *channel;

	if( (channel = strtok( rest, " " )) == NULL )
		show_users_on_channel( from, currentchannel());
	else
		show_users_on_channel( from, channel );
}

void	do_massop(char *from, char *to, char *rest)
{
	char	*op_pattern;

        if( ( op_pattern = strtok( rest, " " ) ) == NULL )
             send_to_user( from, "Please specify a pattern" );
        else
            channel_massop( ischannel( to ) ? to : currentchannel(), 
			    op_pattern );    
        return;
}

void    do_massdeop(char *from, char *to, char *rest)
{
        char    *op_pattern;
 
        if( ( op_pattern = strtok( rest, " " ) ) == NULL )
            send_to_user( from, "Please specify a pattern" );
        else
            channel_massdeop( ischannel( to ) ? to : currentchannel(), 
			      op_pattern );
        return;
}

void	do_masskick(char *from, char *to, char *rest)
{
        char    *op_pattern;

        if( ( op_pattern = strtok( rest, " " ) ) == NULL )
            send_to_user( from, "Please specify a pattern" );
        else
            channel_masskick( ischannel( to ) ? to : currentchannel(), 
			      op_pattern );
        return;
}

void	do_massunban(char *from, char *to, char *rest)
{
	char	*channel;

	if((channel = strtok(rest, " "))==NULL)
		channel_massunban(currentchannel());
	else
		channel_massunban(channel);
}
	
void	do_server(char *from, char *to, char *rest)
{
	int	serv;

	if(rest)
		if(readint(&rest, &serv))
			if(not(change_server(serv)))
				sendreply("Incorrent servernumber!");
			else
				sendreply("Hold on, I'm trying..");
		else
			sendreply("Illegal servernumber!");
	else
		sendreply("Change to which server?");
}

void	show_dir(char *from, char *to, char *rest)
{
	char	*pattern;

	if(rest)
		while((pattern = get_token(&rest, " ")))
		{
			if(*rest != '-')
				do_ls(from, pattern);
		}
	else
		do_ls(from, "");
}

void	show_cwd(char *from, char *to, char *rest)
{
	pwd(from);
}

void	do_cd(char *from, char *to, char *rest)
{
	do_chdir(from, rest?rest:"/");
}

void    show_queue( char *from, char *rest, char *to )
{
	do_showqueue();
}

void	do_fork(char *from, char *to, char *rest)
{
	char	*nick;
	char	*login;

	if(rest && (nick=get_token(&rest, " ")))
	{
		if(!isnick(nick))
		{
			send_to_user(from, "Illegal nick %s", nick);
			return;
		}
		login= get_token(&rest, " ");
		rest = get_token(&rest, "");
		if(!forkbot(nick, login, rest))
			send_to_user(from, "Sorry, no more bots free");
	}	
	else
		send_to_user(from, "Pls specify nick");
}

void	do_unban(char *from, char *to, char *rest)
{
	        char    *channel;

        if((channel = strtok(rest, " "))==NULL)
                channel_unban(currentchannel(), from);
	else
		channel_unban(channel, from);
}

void	do_kick(char *from, char *to, char *rest)
{
	char	*nuh;
	
	if(rest && ((nuh = username(rest)) != NULL))
		if((protlevel(username(rest))>=100)&&
		   (shitlevel(username(rest))==0))
			send_to_user(from, "%s is protected!", rest);
		else if(ischannel(to))
			sendkick(to, rest, "User requested kick");
		else
			sendkick(currentchannel(), rest, "User requested kick");
	else
		send_to_user(from, "Who!?!");
}

void	do_listdcc(char *from, char *to, char *rest)
{
	show_dcclist( from );
}

void	do_rehash(char *from, char *to, char *rest)
{
	rehash = TRUE;
}

void 	giveop(char *channel, char *nicks )
{
    	sendmode(channel, "+ooo %s", nicks);
}

int 	userlevel(char *from)
{
	if(from)
		return(get_level(currentbot->lists->opperlist, from));
	else
		return 0;
}

int 	shitlevel(char *from)
{
	if(from)
		return(get_level(currentbot->lists->shitlist, from));
	else
		return 0;
}

int     protlevel(char *from)
{
	if(from)
		return(get_level(currentbot->lists->protlist, from));
	else
		return 0;
}

void	ban_user(char *who, char *channel)
/*
 * Ban the user as nick and as user (two bans)
 */
{
	char	buf[MAXLEN];	/* make a copy, save original */
	char	*usr = buf;
	char	*nick;
	char	*user;

	strcpy(buf, who);
	nick = get_token(&usr, "!");
	user = get_token(&usr, "@");

	if(*user == '#' || *user == '~')
		user++;
	if(not(*user) || (*user == '*'))/* faking login			*/
	{			/* we can't ban this user on his login,	*/
		             	/* and banning the nick isn't 'nuff, so	*/
				/* ban the entire site!			*/
		if(not(strchr(usr, '.')))
			sendmode(channel, "+bb %s!*@* *!*@*%s", nick, usr);
		else
			sendmode(channel, "+bb %s!*@* *!*@*%s", 
				 strchr(usr, '.'));
		return;
	}
	sendmode(channel, "+bb %s!*@* *!*%s@*", nick, user);
}

void	signoff(char *from, char *reason)
{
	char	*fromcpy;

	mstrcpy(&fromcpy, from);	/* something hoses, dunno what */
	if(number_of_bots == 1)
	{
		send_to_user(fromcpy, "No bots left... Terminating");
	}
	send_to_user(fromcpy, "Saving lists...");
       	if(!write_lvllist(currentbot->lists->opperlist, 
           currentbot->lists->opperfile))
		send_to_user(fromcpy, "Could not save opperlist");
       	if(!write_lvllist(currentbot->lists->shitlist, 
	   currentbot->lists->shitfile))
		send_to_user(fromcpy, "Could not save shitlist");
       	if(!write_lvllist(currentbot->lists->protlist, 
           currentbot->lists->protfile))
		send_to_user( fromcpy, "Could not save protlist");
	send_to_user(fromcpy, "Bye...");
	free(fromcpy);
	killbot(reason);
}
