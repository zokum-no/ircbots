/*
 * parse.c - server input parsing
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
#include <stdio.h>
#include <ctype.h>
#include <sys/time.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

#include "global.h"
#include "config.h"
#include "defines.h"
#include "structs.h"
#include "h.h"


char lastcommand[1024]="";

#ifdef USE_CCC

struct
{
	char	*name;
	void	(*function)(char *lin);
} ccc_funcs[] =
{
	{ "DELALL",	ccc_del_all   },
	{ "RSHITALL",	ccc_rshit_all },
        { "ADD",        ccc_add_user  },
        { "DEL",        ccc_del_user  },
	{ "SHIT",	ccc_add_shit  },
        { "RSHIT",      ccc_del_shit  },
        { "AUTH",       ccc_add_auth  },
        { "DAUTH",      ccc_del_auth  },
        { "NICK",       ccc_do_nick   },
	{ "BOT",	ccc_add_bot   },
	{ "QUIT",	ccc_del_bot   },
	{ "DIE",	ccc_die       },
	{ "CUSER",	ccc_cuser     },
	{ "CSHIT",	ccc_cshit     },	
        { NULL,         (void(*)())(NULL) }
};

#endif


struct
{
	char    *name;
	void    (*function)(char *from, char *rest);
} parse_funcs[] =
{
	{ "TOPIC",	parse_topic	},
	{ "PRIVMSG",    parse_privmsg   },
	{ "NOTICE",     parse_notice    },
	{ "INVITE",     parse_invite    },
	{ "JOIN",       parse_join      },
	{ "PART",       parse_part      },
	{ "MODE",       parse_mode      },      
	{ "NICK",       parse_nick      },
	{ "KICK",       parse_kick      },
	{ "PING",       parse_ping      },
	{ "PONG",       parse_pong      },
	{ "QUIT",       parse_quit      },
	{ "ERROR",      parse_error     },
	{ "001",        parse_001       },
	{ "324",        parse_324       },
	{ "352",        parse_352       },
	{ "364",	parse_364	},
	{ "365",	parse_365	},
	{ "367",        parse_367       },
	{ "368",	parse_368	},
	{ "433",        parse_433       },
	{ "471",        parse_471       },
	{ "473",        parse_471       },
	{ "474",        parse_471       },
	{ "475",        parse_471       },
	{ "476",        parse_471       },
	{ "",         (void(*)())(NULL) }
};

#ifdef USE_CCC

void ccc_cshit(lin)
char *lin;
{
        char *whochanged, *channel, *userhost, *level, *thetime, *expire, *reason;
        register aUser *tmp;
        int done = 0;

        if (!(whochanged = get_token(&lin, " ")) ||
                !(channel = get_token(&lin, " ")) ||
                !(userhost = get_token(&lin, " ")) ||
                !(level = get_token(&lin, " ")) ||
                !(thetime = get_token(&lin, " ")) ||
                !(expire = get_token(&lin, " ")))
                return;
        reason = lin;
        if (!(tmp = find_shit(&Userlist, userhost, channel)))
		return;
        if (atoi(level) != -1)
                tmp->access = atoi(level);
        if (atol(thetime) != -1)
                tmp->time = atoi(thetime);
        if (atol(expire) != -1)
                tmp->expire = atoi(expire);
        if (reason && *reason)
        {
                MyFree(&tmp->reason);
                mstrcpy(&tmp->reason, reason);
        }
}

void ccc_cuser(lin)
char *lin;
{
	char *whochanged, *channel, *userhost, *level, *aop, *prot, *passwd;
        register aUser *tmp;
        int done = 0;

        if (!(whochanged=get_token(&lin, " ")) ||
                !(channel=get_token(&lin, " ")) ||
                !(userhost=get_token(&lin, " ")) ||
                !(level=get_token(&lin, " ")) ||
                !(aop=get_token(&lin, " ")) ||
                !(prot=get_token(&lin, " ")))
                return;
	passwd = get_token(&lin, " ");
        if (strcmp(channel, "*"))
        {
                if (!(tmp = find_user(&Userlist, userhost, channel)))
                        return;
                done = 1;
                if (atoi(level) != -1)
                        tmp->access = atoi(level);
                if (atoi(aop) != -1)
                        tmp->aop = atoi(aop);
                if (atoi(prot) != -1)
                        tmp->prot = atoi(prot);
        }
        if (!done || (passwd && *passwd))
        {
                for (tmp=Userlist;tmp;tmp=tmp->next)
                        if (IsUser(tmp) && !matches(tmp->userhost, userhost))
                        {
                                if (!done)
                                {
                                        if (atoi(level) != -1)
                                                tmp->access = atoi(level);
                                        if (atoi(aop) != -1)
                                                tmp->aop = atoi(aop);
                                        if (atoi(prot) != -1)
                                                tmp->prot = atoi(prot);
                                }
                                if (passwd && *passwd)
                                {
                                        if (tmp->password)
                                                MyFree(&tmp->password);
                                        mstrcpy(&tmp->password, passwd);
                                }
                        }
        }
}
 
void ccc_die(lin)
char *lin;
{
	char *from, *reason;

	from = get_token(&lin, " ");
	reason = get_token(&lin, " ");
	if (!reason || !*reason)
		reason = "Killed from telnet connection";
	if (from)
	        log_it(COMMANDS_LOG_VAR, 0, "Killed from telnet by %s", from);
	signoff(NULL, reason);
        if (number_of_bots == 0)
	{
		sleep(2);
                FinishExit(0);
	}
}

void ccc_del_all(lin)
char *lin;
{
	delete_userlist(&Userlist);
}

void ccc_rshit_all(lin)
char *lin;
{
	delete_shitlist(&Userlist);
}

void ccc_add_bot(lin)
char *lin;
{
	char *nuh;

	if (!(nuh = get_token(&lin, " ")))
		return;
	make_list(&Bots, nuh);	
	op_bot(nuh);
}

void ccc_del_bot(lin)
char *lin;
{
	char *nuh;
	aList *tmp;

	if (!(nuh = get_token(&lin, " ")))
		return;
	if (!strcmp(nuh, "ccc"))
	{
		close(ccc_sock);
		ccc_sock = -1;
		return;
	}	
	if ((tmp=find_list(&Bots, nuh)) != NULL)
		remove_list(&Bots, tmp);
}

void ccc_add_user(lin)
char *lin;
{
	char *channel, *userhost, *level, *aop, *prot, *passwd;

	if (!(channel = get_token(&lin, " ")) ||
		!(userhost = get_token(&lin, " ")) ||
		!(level = get_token(&lin, " ")) ||
		!(aop = get_token(&lin, " ")) ||
		!(prot = get_token(&lin, " ")))
		return;
	passwd = get_token(&lin, " ");
	add_to_userlist(&Userlist, userhost, atoi(level),
		atoi(aop), atoi(prot), channel, passwd);
}

void ccc_del_user(lin)
char *lin;
{
	char *channel, *userhost;
	aUser *tmp;
	
	if (!(channel = get_token(&lin, " ")) ||
		!(userhost = get_token(&lin, " ")))
		return;
	if ((tmp=find_user(&Userlist, userhost, channel)) != NULL)
		remove_user(&Userlist, tmp);
}

void ccc_add_shit(lin)
char *lin;
{
	char *whoshit, *channel, *userhost, *level, *thetime,
		*expire, *reason;

	if (!(whoshit = get_token(&lin, " ")) ||
		!(channel = get_token(&lin, " ")) ||
		!(userhost = get_token(&lin, " ")) ||
		!(level = get_token(&lin, " ")) ||
		!(thetime = get_token(&lin, " ")) ||
		!(expire = get_token(&lin, " ")) ||
		!(reason = lin))
		return;
	if (atol(expire) > getthetime())
	add_to_shitlist(&Userlist, userhost, atoi(level), channel,
		whoshit, reason, atol(thetime), atol(expire)); 
}

void ccc_del_shit(lin)
char *lin;
{
	char *channel, *userhost;
	aUser *tmp;
	if (!(channel = get_token(&lin, " ")) ||
		!(userhost = get_token(&lin, "")))
		return;
	if ((tmp=find_user(&Userlist, userhost, channel)) != NULL)
		remove_user(&Userlist, tmp);
}

void ccc_add_auth(lin)
char *lin;
{
	char *nuh, *last;
	aTime *tmp;

	if (!(nuh = get_token(&lin, " ")) ||
		!(last = get_token(&lin, " ")))
		return;
	if ((tmp = make_time(&Auths, nuh)) != NULL)
		tmp->time = atol(last); 
	op_all_chan(nuh);
}

void ccc_del_auth(lin)
char *lin;
{
	char *nuh, *last;
	aTime *tmp;

	if (!(nuh = get_token(&lin, " ")) ||
		!(last = get_token(&lin, " ")))
		return;
	if ((tmp = find_time(&Auths, nuh)) != NULL)
	{
		if (tmp->time != atol(last))
		{
			if (ccc_sock != -1)
				send_to_socket(ccc_sock, "1 AUTH %s %li",
					tmp->name, tmp->time);
		}
		else
			remove_time(&Auths, tmp);
	}
}

void ccc_do_nick(lin)
char *lin;
{
	char *old, *new;
	aList *tmp;
	aTime *tmp2;

	if (!(old = get_token(&lin, " ")) ||
		!(new = get_token(&lin, " ")))
		return;
	if ((tmp = find_list(&Bots, old)) != NULL)
	{
		MyFree((char **)&tmp->name);
		mstrcpy(&tmp->name, new);
	}
	if ((tmp2 = find_time(&Auths, old)) != NULL)
	{
		MyFree ((char **)&tmp2->name);
		mstrcpy(&tmp2->name, new);
	}
}

void parsecccline(lin)
char *lin;
{
	char *typ, *command;
	int i;

	terminate(lin, "\n\r");
	if (!(typ = get_token(&lin, " ")) ||
		!(command = get_token(&lin, " ")))
		return;
	switch(atoi(typ))
	{
		case 1:
			for(i=0;ccc_funcs[i].name;i++)
				if (!strcmp(ccc_funcs[i].name, command))
				{
					ccc_funcs[i].function(lin);
					return;
				}
			break;
		case 2:
			/* DCC Channels */
			break;
		default:
			break; /* dunno :-/ */
	}
}

#endif

void parse_topic(char *from, char *rest)
{
	char *chan;

	if (!from)
		return;
	chan = get_token(&rest, " ");
	if (chan && (*chan == ':'))
	  chan++;
	if (rest && (*rest == ':'))
		rest++;
	CurrentUser = NULL;
	CurrentShit = NULL;
	strcpy(currentnick, getnick(from));
	on_topic(from, chan, rest);
}

void parse_invite(char *from, char *rest)
{
	char *to;
	char *chan;

	to = get_token(&rest, " ");
	chan = get_token(&rest, " ");
	if (chan && (*chan == ':'))
	  chan++;
	if (get_userlevel(from, chan) >= ASSTLEVEL)
	{
		join_channel(chan, NULL, 1);
		lastrejoin = getthetime();
	}
}

void parse_privmsg(char *from, char *rest)
{
	char    *text;
	char    *to;

	if (!from)
		return;
	if (!strchr(from, '@'))
		return;
	if ((text = strchr(rest, ' ')) != NULL)
		*(text) = '\0';
	text += 2;
	to = rest;
	if(*text == ':')
		*(text++) = '\0';
	if (ischannel(to))
	{
		CurrentUser = find_user(&Userlist, from, to);
		CurrentShit = find_shit(&Userlist, from, to);
	}
	else
	{
		CurrentUser = find_user(&Userlist, from, "*");
		CurrentShit = find_shit(&Userlist, from, "*");
	}
	strcpy(currentnick, getnick(from));
	if (*text == '\001' && *(text + strlen(text) - 1) == '\001')
	{
		*(text++) = '\0';
		terminate(text, "\001");
		on_ctcp(from, to, text);
	}
	else
		on_msg(from, to, text);
}

void parse_notice(char *from, char *rest)
{
	char    *text;
	char    *to;

	if (!from)
		return;
	if (!strchr(from, '@'))
		return;
        if ((text = strchr(rest, ' ')) != NULL)
                *( text ) = '\0';
        text += 2;
        to = rest;
        if (*text == ':')
                *(text++) = '\0';
        if (*text == '\001' && *( text + strlen( text) - 1 ) == '\001' )
        {
        	/* ignore ctcp replies */
	}
        else
	{
		if (ischannel(to))
		{
			CurrentUser = find_user(&Userlist, from, to);
			CurrentShit = find_shit(&Userlist, from, to);
		}
		else
		{
			CurrentUser = find_user(&Userlist, from, "*");
			CurrentShit = find_shit(&Userlist, from, "*");
		}
		strcpy(currentnick, getnick(from));
		on_msg( from, to, text );
	}
}

void parse_join(char *from, char *rest)
{
	char    from_copy[MAXLEN];
	char    *f = from_copy;         /* blah */
	char    *n, *u, *h;
			
	strcpy(from_copy, from);
	n=get_token(&f, "!");
	u=get_token(&f, "@");
	h=get_token(&f, "");
	/* if it's me I succesfully joined a channel! */
	if (!my_stricmp(current->nick, n))
	{
		aChannel *Huh;

		if (!(Huh = find_channel(rest)))
		{
			Huh = find_oldchannel(rest);
			if (Huh)
				oldchannel_to_newchannel(Huh);
		}
		if (Huh)
		{
			delete_chanusers(&(Huh->users));
			delete_bans(&(Huh->banned));
			Huh->mode = 0;
			current->Current_chan = Huh;
		}	
		if (Huh->flags & REJOIN_TYPE)
			Huh->flags &= ~REJOIN_TYPE;
		/* get channelinfo */
		send_to_server( "WHO %s", rest );
		send_to_server( "MODE %s", rest );
		send_to_server( "MODE %s b", rest );
	}
	add_user_to_channel(rest, n, u, h);
	check_limit(rest);
	CurrentUser = find_user(&Userlist, from, rest);
	CurrentShit = find_shit(&Userlist, from, rest);
	strcpy(currentnick, getnick(from));
	on_join(from, rest);
}

void parse_part(char *from, char *rest)
{
	char *resttmp;
	aChannel *Chan;
	int old = 0;

	mstrcpy(&resttmp, rest);
	strcpy(currentnick, getnick(from));
	on_part(from, rest);

	if (!(Chan = find_channel(resttmp)))
	{
		if (!(Chan = find_oldchannel(resttmp)))
			return;
		old = 1;
	}
	if (!my_stricmp(currentnick, current->nick))
	{
		if (!old)
			newchannel_to_oldchannel(Chan);
/* Not used...
		if (Chan->flags & CYCLE_TYPE)
		{
			Chan->flags &= ~CYCLE_TYPE;
			join_channel(Chan->name, NULL, 1);
		}
*/
	}
	else
	{
		remove_user_from_channel(resttmp, currentnick);
		check_all_steal();
	}
	check_limit(resttmp);
	MyFree((char **)&resttmp);
}

void parse_mode(char *from, char *rest)
{
	char *to;

	to = get_token(&rest, " ");
	if (!to)
		return;
	if (*to == ':')
		to++;
	if (ischannel(to))
	{
		CurrentUser = find_user(&Userlist, from, to);
		CurrentShit = find_shit(&Userlist, from, to);
		strcpy(currentnick, getnick(from));
		on_mode(from, to, rest);
	} /* disregard usermodes */
}

void parse_nick(char *from, char *rest)
{
	CurrentUser = find_user(&Userlist, from, "*");
	CurrentShit = NULL;
/*
	CurrentShit = find_shit(&Shitlist, from, "*");
*/
	strcpy(currentnick, getnick(from));
	on_nick(from, rest);
	change_nick(getnick(from), rest);
	check_shit();
	check_protnicks();
}

void parse_kick(char *from, char *rest)
{
	char    *channel;
	char    *nick;

	channel = get_token(&rest, " ");
	nick = get_token(&rest, " ");

	if (rest && (*rest == ':'))
	  rest++;
	CurrentUser = find_user(&Userlist, from, channel);
	CurrentShit = find_shit(&Userlist, from, channel);
	strcpy(currentnick, getnick(from));
	on_kick(from, channel, nick, rest);

	if (!my_stricmp(current->nick, nick))
	{
		aChannel *Dummy;

		if ((Dummy = find_channel(channel)) != NULL)
		{
			newchannel_to_oldchannel(Dummy);
			if (Dummy == current->Current_chan)
				current->Current_chan = current->Channel_list;
		}
	}
	else
		remove_user_from_channel(channel, nick);
	check_limit(channel);
}

void parse_ping(char *from, char *rest)
{
	sendping(rest);
	current->lastping = getthetime();
}

void parse_pong(char *from, char *rest)
{
	char *nick, *server;

	server = get_token(&rest, " ");
	nick = get_token(&rest, "");
	if (*nick == ':')
		nick++;
	pong_received(nick, server);
}

void parse_quit(char *from, char *rest)
{
	CurrentUser = NULL;
	CurrentShit = NULL;
/*
	CurrentUser = find_user(&Userlist, from, "*");
	CurrentShit = find_shit(&Userlist, from, "*");
*/
	strcpy(currentnick, getnick(from));
	on_quit(from, rest);
	remove_user_from_all_channels(getnick(from));
	check_all_steal();
}

void parse_error(char *from, char *rest)
{
	log_it(ERR_LOG_VAR, 0, "SIGNING OFF: %s", rest);
	reconnect_to_server();
}

void parse_001(char *from, char *rest)
/*
 * Use the 001-reply (Welcome to the Internet Relay Network NICK)
 * to determine the server's name (this might not be the name in the
 * serverlist!). All we have to do is look at "from"
 */
{
	debug(NOTICE, "Current server calls himself %s", from);
	strcpy(current->serverlist[current->current_server].realname, from);  
	/* Successfull connect so we can do the join-stuff */
	rejoin_channels(HARDRESET);
}

void parse_368(from, rest)
char *from;
char *rest;
{
	char *to, *chan;
	aChannel *Chan;

	to = get_token(&rest, " ");
	if (!(chan = get_token(&rest, " ")))
		return;
	if (!(Chan = find_channel(chan)))
		return;
	if (Chan->flags & COLLECT_TYPE)
	{
		Chan->flags &= ~COLLECT_TYPE;
		leave_channel(chan);
		if (Chan->collectby)
		{
			need_dcc = 0;
			send_to_user(Chan->collectby,
				"\002Data has been collected for %s\002", chan);
			MyFree((char **)&Chan->collectby);
		}
	}
}

void parse_324(char *from, char *rest)
{
	char *chan;

	rest = strchr( rest, ' ' );
	rest++;
	chan = get_token(&rest, " ");
	if (chan && (*chan == ':'))
		chan++;
	on_mode(from, chan, rest);
}

void    parse_352( char *from, char *rest )
/* 352: who-reply */
{
	char    *channel;
	char    *nick;
	char    *user;
	char    *host;
	char    *server;  /* currently unused */
	char    *mode;
	aList	*tmp;	
	/* skip my own nick */
	get_token(&rest, " " );
	/* Is it a header? return.. */
	if (*rest == 'C')              /* NO NO NO! */
		return;
	channel = get_token(&rest, " ");
	user = get_token(&rest, " ");
	host = get_token(&rest, " ");
	server = get_token(&rest, " ");
	nick = get_token(&rest, " ");
	mode = get_token(&rest, " ");
	add_user_to_channel( channel, nick, user, host );
	if (username(nick) && i_am_op(channel))
	  if (is_shitted(username(nick), channel))
		 shit_action(username(nick), channel);
	while (*mode )
	{
		switch( *mode )
		{
		case 'H':
		case 'G':
		case '*':
			break;
		case '@':
			if ((tmp=find_list(&current->StealList,channel))!=NULL)
				remove_list(&current->StealList, tmp);
			change_usermode( channel, nick, MODE_CHANOP );
			break;
		case '+':
			change_usermode( channel, nick, MODE_VOICE );
			break;
		default:
#ifdef DBUG
			debug(ERROR, "parse_352: unknown mode %c", *mode);
#endif
			break;
		}
		mode++;
	}
}

void parse_364(char *from, char *rest)
{
	char	*serv1, *serv2;

	(void) get_token(&rest, " ");
	serv1 = get_token(&rest, " ");
	serv2 = get_token(&rest, " ");
	if (!serv1 || !serv2 || !*serv1 || !*serv2)
		return;
	add_link(&current->TempLinks, serv1, serv2);
}

void parse_365(char *from, char *rest)
{
	register aLink *serv;
	
	for(serv=current->TheLinks;serv;serv=serv->next)
	{
		if (!find_link(current->TempLinks, serv->name))
		{
			if (!(serv->status & SPLIT))
			{
				serv->status |= SPLIT;
				serv->num++;
			}
		}
		else
			if (serv->status & SPLIT)
				serv->status &= ~SPLIT;
	}

	for(serv=current->TempLinks;serv;serv=serv->next)
	{
		if (!find_link(current->TheLinks,serv->name))
			add_link(&current->TheLinks, serv->name, serv->link);
	}
	free_links(&current->TempLinks);
}

void parse_367(char *from, char *rest )
{
	char    *nick;
	char    *channel;
	char    *banstring;
	char    *banfrom;
	char    *bantime;

	nick = get_token(&rest, " ");
	channel = get_token(&rest, " ");
	banstring = get_token(&rest, " ");
	banfrom = get_token(&rest, " ");
	bantime = get_token(&rest, " ");
	add_channelmode((banfrom && *banfrom) ? banfrom : from, channel, MODE_BAN, banstring, (bantime && *bantime) ? atol(bantime) : getthetime());
}

void parse_433(char *from, char *rest) /* 431..436! */
/*
 * How to determine the new nick: 
 * - If it's possible to add a '_' to the nick, do it (done)
 * - else loop through the nick from begin to end and replace
 *   the first non-'_' with a '_'. i.e. __derServ -> ___erServ
 * - If the nick is 9 '_''s, try the original nick with something random
 */
{
	char    *s;
	
	if (strlen(current->nick) == 9)
	{
		for(s=current->nick; *s && (*s == '_'); s++)
			;
		if(*s)
			*s = '_';
		else
		{
			char    randomstr[5];
			
			strncpy(current->nick, current->realnick, 5);
			current->nick[5] = '\0';
			sprintf(randomstr, "%d", (int)(rand() % 10000));
			strcat(current->nick, randomstr);
		}
	}
	else
		strcat(current->nick, "_");
	sendnick(current->nick);
#ifdef USE_CCC
	if (ccc_sock != -1)
		send_to_socket(ccc_sock, "1 NICK %s!%s@%s", current->nick,
                                        current->login,
                                        mylocalhostname);
#endif
}

void parse_471(char *from, char *rest)
{
	char    *channel;
	aChannel *Dummy;

	rest = strchr(rest, '#');
	channel = get_token(&rest, " ");
	if ((Dummy = find_channel(channel)) != NULL)
	{
		newchannel_to_oldchannel(Dummy);
		Dummy->flags |= REJOIN_TYPE;
	}
}

void parseline(char *line)
{
	char    *from; 
	char    *command;
	char    *rest;
	int     i;
	FILE *out;

	/* terminate the line where either \r or \n exist */
	/* The new servers put a \r before the \n */

	terminate(line, "\r\n");
	strcpy(lastcommand, line);
	if ((out = fopen("shit.log", "w")) != NULL)
	{
		fprintf(out, "%s\n", line);
		fclose(out);
	}
	check_idle();

#ifdef DBUG
	debug(NOTICE, "parseline(\"%s\")", line);
#endif 

	if (*line == ':' )
	{
		if (!(command = strchr(line, ' ')))
		  return;
		*(command++) = '\0';
		from = line + 1; 
	} 
	else 
	{
		command = line;
		from = NULL;
	}
	if(!(rest = strchr(command, ' ')))
	  return;
	*(rest++) = '\0';
	if (*rest == ':')
		*(rest++) = '\0';
	for (i=0; parse_funcs[i].name[0]; i++)
	  if (!my_stricmp(parse_funcs[i].name, command))
		 {
			parse_funcs[i].function(from, rest);
#ifdef DOUGH_MALLOC
			check_all_mallocs((char *)0);
#endif
			if (!current->created)
			{
/* FUCK...let's dump core...ok? */
				char *fuck = (char *) 0;
				*fuck = 'F';
			}
			return;
		 } 
}








