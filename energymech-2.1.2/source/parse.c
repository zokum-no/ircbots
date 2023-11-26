/*
 * parse.c - server input parsing
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
	{ "DELALL",		ccc_del_all   },
	{ "RSHITALL",	ccc_rshit_all },
    { "ADD",        ccc_add_user  },
    { "DEL",        ccc_del_user  },
	{ "SHIT",		ccc_add_shit  },
    { "RSHIT",      ccc_del_shit  },
    { "AUTH",       ccc_add_auth  },
    { "DAUTH",      ccc_del_auth  },
    { "NICK",       ccc_do_nick   },
	{ "BOT",		ccc_add_bot   },
	{ "QUIT",		ccc_del_bot   },
	{ "DIE",		ccc_die       },
	{ "CUSER",		ccc_cuser     },
	{ "CSHIT",		ccc_cshit     },	
    { NULL,         (void(*)())(NULL) }
};

#endif


struct
{
	char    *name;
	void    (*function)(char *from, char *rest);
} parse_funcs[] =
{
	{ "TOPIC",		parse_topic		},
	{ "PRIVMSG",    parse_privmsg   },
	{ "NOTICE",     parse_notice    },
	{ "WALLOPS",    parse_wallops   },
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
	{ "200",        parse_200       },
	{ "201",        parse_200       },
	{ "202",        parse_200       },
	{ "203",        parse_200       },
	{ "204",        parse_204       },
	{ "205",        parse_205       },
	{ "206",        parse_200       },
	{ "207",        parse_200       },
	{ "208",        parse_200       },
	{ "209",        parse_200       },
	{ "211",        parse_213       },
	{ "213",        parse_213       },
	{ "214",        parse_213       },
	{ "215",        parse_213       },
	{ "216",        parse_213       },
	{ "217",        parse_213       },
	{ "218",        parse_213       },
	{ "219",        parse_219       },
	{ "222",        parse_213       },
	{ "223",        parse_213       },
	{ "241",        parse_213       },
	{ "242",        parse_213       },
	{ "243",        parse_243       },
	{ "244",        parse_213       },
	{ "245",        parse_213       },
	{ "251",        parse_251       },
	{ "252",        parse_252       },
	{ "253",        parse_253       },
	{ "254",        parse_254       },
	{ "255",        parse_255       },
	{ "262",        parse_262       },
	{ "301",        parse_301       },
	{ "311",        parse_311       },
	{ "312",        parse_312       },
	{ "313",        parse_313       },
	{ "317",        parse_317       },
	{ "318",        parse_318       },
	{ "319",        parse_319       },
	{ "324",        parse_324       },
	{ "352",        parse_352       },
	{ "361",		parse_361		},
	{ "364",		parse_364		},
	{ "365",		parse_365		},
	{ "367",        parse_367       },
	{ "368",		parse_368		},
	{ "381",        parse_381       },
	{ "433",        parse_433       },
	{ "451",        parse_451       },
	{ "471",        parse_471       },
	{ "473",        parse_471       },
	{ "474",        parse_471       },
	{ "475",        parse_471       },
	{ "476",        parse_471       },
	{ "481",        parse_481       },
	{ "491",        parse_491       },
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
        if (!(tmp = find_shit(&(current->Userlist), userhost, channel)))
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
                if (!(tmp = find_user(&(current->Userlist), userhost, channel)))
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
                for (tmp=current->Userlist;tmp;tmp=tmp->next)
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
	delete_userlist(&(current->Userlist));
}

void ccc_rshit_all(lin)
char *lin;
{
	delete_shitlist(&(current->Userlist));
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
	add_to_userlist(&(current->Userlist), userhost, atoi(level),
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
	if ((tmp=find_user(&(current->Userlist), userhost, channel)) != NULL)
		remove_user(&(current->Userlist), tmp);
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
	add_to_shitlist(&(current->Userlist), userhost, atoi(level), channel,
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
	if ((tmp=find_user(&(current->Userlist), userhost, channel)) != NULL)
		remove_user(&(current->Userlist), tmp);
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
#ifdef RANDQUOTE
	char	tbuf[MAXLEN];
#endif

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
		CurrentUser = find_user(&(current->Userlist), from, to);
		CurrentShit = find_shit(&(current->Userlist), from, to);
	}
	else
	{
		CurrentUser = find_user(&(current->Userlist), from, "*");
		CurrentShit = find_shit(&(current->Userlist), from, "*");
	}
	strcpy(currentnick, getnick(from));
	if (*text == '\001' && *(text + strlen(text) - 1) == '\001')
	{
		*(text++) = '\0';
		terminate(text, "\001");
		on_ctcp(from, to, text);
	}
	else {
		on_msg(from, to, text);
	}
}

void parse_notice(char *from, char *rest)
{
	char    *text;
	char    *to;
	char	*aptr, *temp, *temp2, *aptr2;
	char	tbuf[MAXLEN], tbuf2[MAXLEN];
	long	ping1, ping2;
	int		varlen;

	if (!from) {
		return;
	}
	if (!strchr(from, '@')) {
		get_token(&rest, " ");
		if (rest[0] == ':') {
			rest++;
		}
		aptr = strcpy(tbuf, rest);
#ifdef SERVER_MONITOR
		parse_server_notice(from, rest);
#endif
		temp = strcpy(tbuf2, tbuf);
		temp2 = get_token(&temp, " ");
		if (!strcmp(temp2, "***")) {
			get_token(&aptr, " ");
			get_token(&aptr, " ");
			get_token(&aptr, " ");
		}
		send_monitormsg("[\002- %s\002] %s", from, aptr);
		return;
	}
    if ((text = strchr(rest, ' ')) != NULL) {
    	*( text ) = '\0';
    }
	text += 2;
	to = rest;
	if (*text == ':') {
    	*(text++) = '\0';
    }
    if (*text == '\001' && *( text + strlen( text) - 1 ) == '\001' ) {
    	char abuf[MAXLEN];
		send_statmsg("[CTCP Reply From: \002%s\002] %s", from, text);
		aptr = strcpy(abuf, text);
		varlen = strlen(aptr);
		aptr[varlen - 1] = '\0';
		aptr++;
		aptr2 = get_token(&aptr, " ");
		if (!my_stricmp(aptr2, "PING")) {
			if (aptr2 = get_token(&aptr, " ")) {
				if (isdigit(*aptr2)) {
					ping1 = atol(aptr2);
				}
				ping2 = getthetime();
				send_statmsg("[\002LAG\002] %li second(s) lag from %s", (ping2 - ping1), getnick(from));
			}
		}
	}
	else {
		if (ischannel(to)) {
			CurrentUser = find_user(&(current->Userlist), from, to);
			CurrentShit = find_shit(&(current->Userlist), from, to);
		}
		else {
			CurrentUser = find_user(&(current->Userlist), from, "*");
			CurrentShit = find_shit(&(current->Userlist), from, "*");
		}
		strcpy(currentnick, getnick(from));
		on_msg( from, to, text );
	}
}

void parse_wallops(char *from, char *rest)
{
	char    *text;
	char    *to;

	if (!from) {
		return;
	}
/*	get_token(&rest, " "); */
	if (rest[0] == ':') {
		rest++;
	}
	send_statmsg("[\002+ %s\002] %s", from, rest);
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
			if (current->isIRCop == TRUE) {
				change_usermode( rest, current->nick, MODE_OPER );
			}
		/* get channelinfo */
		if (get_str_var(rest, SETENFM_VAR)) {
			send_to_server( "MODE %s %s", rest, get_str_var(rest, SETENFM_VAR) );
		}
		send_to_server( "WHO %s", rest );
		send_to_server( "MODE %s", rest );
		send_to_server( "MODE %s b", rest );
		add_user_to_channel(rest, n, u, h, current->serverlist[current->current_server].realname);
	}
	else {
		add_user_to_channel(rest, n, u, h, "?");
	}
	check_limit(rest);
	CurrentUser = find_user(&(current->Userlist), from, rest);
	CurrentShit = find_shit(&(current->Userlist), from, rest);
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
	}
	else
	{
		addseen(getnick(from), 1, rest);
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
		CurrentUser = find_user(&(current->Userlist), from, to);
		CurrentShit = find_shit(&(current->Userlist), from, to);
		strcpy(currentnick, getnick(from));
		on_mode(from, to, rest);
		return;
	}
	if (!my_stricmp(current->nick, to)) {
		char tbuf[32], tbuf2[32], tbuf3[32];
		char *themodes, *aptr, *aptr2, sign;
		int modelen, i, j;
		if (current->mymodes) {
			strcpy(tbuf, current->mymodes);
		}
		else {
			strcpy(tbuf, "");
		}
		themodes = get_token(&rest, " ");
		if (themodes[0] == ':') {
			themodes++;
		}
		modelen = strlen(themodes);
		for (i = 0; i < modelen; i++) {
			switch (themodes[i]) {
			  case '+':
			  case '-':
			  	sign = themodes[i];
			  	break;
			  default:
			  	if (sign == '+') {
			  		sprintf(tbuf, "%s%c", tbuf, themodes[i]);
			  		break;
			  	}
			  	else if (sign == '-') {
			  		sprintf(tbuf3, "%c", themodes[i]);
			  		aptr = strcpy(tbuf2, tbuf);
			  		aptr2 = get_token(&aptr, tbuf3);
			  		sprintf(tbuf, "%s%s", aptr2, aptr);
			  		break;
			  	}
			}
		}
		strcpy(current->mymodes, tbuf);
	}
}

void parse_nick(char *from, char *rest)
{
	CurrentUser = find_user(&(current->Userlist), from, "*");
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
	char	tbuf[MAXLEN];

	channel = get_token(&rest, " ");
	nick = get_token(&rest, " ");

	if (rest && (*rest == ':'))
	  rest++;
	CurrentUser = find_user(&(current->Userlist), from, channel);
	CurrentShit = find_shit(&(current->Userlist), from, channel);
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
	else {
		sprintf(tbuf, "%s %s", from, rest);
		addseen(nick, 4, tbuf);
		remove_user_from_channel(channel, nick);
	}
	check_limit(channel);
}

void parse_ping(char *from, char *rest)
{
	sendping(rest);
	sendpong(rest);
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
        char *temp, *temp2, *nick, buf[MAXLEN];
        static  int split = FALSE;
        static  long splitime;
        
        strcpy(buf, rest);

	CurrentUser = NULL;
	CurrentShit = NULL;
/*
	CurrentUser = find_user(&(current->Userlist), from, "*");
	CurrentShit = find_shit(&(current->Userlist), from, "*");
*/
	strcpy(currentnick, getnick(from));
	on_quit(from, rest);
	remove_user_from_all_channels(getnick(from), rest);
	check_all_steal();
	
	nick = getnick(from);
	
	temp = get_token(&rest, " ");
	
	if (getthetime() - splitime > 60) {
		split = FALSE;
	}
	
	if (rest && *rest) {
		while (temp) {
			if (!matches("*Local*Kill*", temp) || !matches("*Killed*", temp)) {
				if (get_token(&rest, " ")) {
					log_it(KILL_LOG_VAR, 0, "KILL from QUIT: [%s] %s", nick, buf);
				}
				return;
			}
			if (!split) {
				if (!matches("irc.*.*", temp) || !matches("*.*.??", temp) || !matches("*.*.???", temp)) {
        			if (matches("*..*", temp)) {
        				temp2 = get_token(&rest, " ");
        				if (temp2) {
        					log_it(SPLIT_LOG_VAR, 0, "POSSIBLE SPLIT: %s %s", temp, temp2);
        					split = TRUE;
                			splitime = getthetime();
        					return;
        				}
        				else {
        					return;
        				}
        			}
        		}
        	}
        	temp = get_token(&rest, " ");
		}
	}
}

void parse_error(char *from, char *rest)
{	
	log_it(SERV_LOG_VAR, 0, "SIGNING OFF [%s]: %s", current->serverlist[current->current_server].name, rest);
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
	
	/* Reset Oper Attributes */

	current->hasOLine = FALSE;
	current->isIRCop = FALSE;
	
	strcpy(current->mymodes, "");
	
#ifdef OPER_ON_CON
	sendoper(OPER_NAME, OPER_PASS);
#endif
	
	if (current->tempvar) {
		MyFree((char **)&current->tempvar);
	}
}

void parse_200(char *from, char *rest)
{
/* Trace Parse */
/* Can be used for ALL "Generic" trace commands */

	aTime *aVarX, *aVarX2;

#ifdef SERVER_MONITOR	
	if (current->trace_tog || current->trace_kill_tog) {
		return;
	}
#endif	
	
	get_token(&rest, " ");

/* Welcome to Land of the Loops */
	
	aVarX = current->ParseList;
	while (aVarX) {
		if ((getthetime() - 60 > aVarX->time) && (aVarX->num == 9)) {
			aVarX2 = aVarX->next;
			remove_time(&current->ParseList, aVarX);
			aVarX = aVarX2;
		}
		else if (aVarX->num == 10 || 9) {
			send_to_user(aVarX->name, "[\002%s\002] %s", from, rest);
			aVarX->num = 9;
			aVarX = aVarX->next;
		}
		else {
			aVarX = aVarX->next;
		}
	}
}

void parse_204(char *from, char *rest)
{
/* Oper Trace Parse */
	char *aptr, *aptr2, *aptr3;
	char tbuf[MAXLEN], abuf[MAXLEN];
	int varlen, varlen2;
	aTime *aTimeVar;
	
	aTime *aVarX, *aVarX2;
	
/* Get rid of my nick */
	get_token(&rest, " ");
	
#ifdef SERVER_MONITOR
	if (current->trace_tog) {
/* If the server doesn't match the one were even on return */
		if (my_stricmp(from, current->serverlist[current->current_server].realname)) {
			return;
		}
	
		current->total_connections++;
		current->today_connections++;
	
		log_it(MON_CON_LOG_VAR, 0, "[From TRACE] %s", rest);
	
/* Because nicks could have the '[' char in them and there is no space between them :( */

		aptr = get_token(&rest, " ");
		while (aptr) {
			if (isdigit(*aptr)) {
				break;
			}
			aptr = get_token(&rest, " ");
		}
		
		aptr = get_token(&rest, " ");
		
		if (aptr2 = strrchr(aptr, '[')) {
			varlen = strlen(aptr);
			varlen2 = strlen(aptr2);
			aptr2 = strcpy(abuf, aptr2);
			aptr2[varlen2 - 1] = '\0';
			aptr2++;
			aptr[varlen - varlen2] = '\0';
		}
		else {
			return;
		}
		

	
		sprintf(tbuf, "%s!%s", aptr, aptr2);
		
		aptr3 = format_uh(tbuf, 0);
		
		if (current->trace_kill_tog == 2) {
			for (aTimeVar = current->botList; aTimeVar; aTimeVar = aTimeVar->next) {
				if (!matches(aptr3, aTimeVar->name)) {
					sprintf(abuf, "%s No Bots Please", aptr);
					do_kill(current->nick, current->nick, abuf, OWNERLEVEL);
					return;
				}
			}
		}
		else if (current->trace_kill_tog == 1) {
			for (aTimeVar = current->conList; aTimeVar; aTimeVar = aTimeVar->next) {
				if (!matches(aptr3, aTimeVar->name) && (aTimeVar->num > 1)) {
					sprintf(abuf, "%s No Clones Please", aptr);
					do_kill(current->nick, current->nick, abuf, OWNERLEVEL);
					return;
				}
			}
		}
		
		if (aTimeVar = find_time(&current->conList, aptr3)) {
			aTimeVar->num++;
		}
		else {
			aTimeVar = make_time(&current->conList, aptr3);
		}
	
		if (aTimeVar->num > 1) {
/* Ignore Oper Clones */
/*		send_monitormsg("[\002Clone\002] \002%s\002 connecting with \002%i\002 other clone(s)", tbuf, (aTimeVar->num - 1)); */
		}
	}
	else {
#endif
/* Welcome to Land of the Loops */
	
		aVarX = current->ParseList;
		while (aVarX) {
			if ((getthetime() - 60 > aVarX->time) && (aVarX->num == 9)) {
				aVarX2 = aVarX->next;
				remove_time(&current->ParseList, aVarX);
				aVarX = aVarX2;
			}
			else if (aVarX->num == 10 || 9) {
				send_to_user(aVarX->name, "[\002%s\002] %s", from, rest);
				aVarX->num = 9;
				aVarX = aVarX->next;
			}
			else {
				aVarX = aVarX->next;
			}
		}
#ifdef SERVER_MONITOR
	}
#endif
}

void parse_205(char *from, char *rest)
{
/* User Trace Parse */
	char *aptr, *aptr2, *aptr3;
	char tbuf[MAXLEN], abuf[MAXLEN];
	int varlen, varlen2;
	aTime *aTimeVar;
	
	aTime *aVarX, *aVarX2;
	
/* Get rid of my nick */
	get_token(&rest, " ");
	
#ifdef SERVER_MONITOR
	if (current->trace_tog) {
/* If the server doesn't match the one were even on return */
		if (my_stricmp(from, current->serverlist[current->current_server].realname)) {
			return;
		}
	
		current->total_connections++;
		current->today_connections++;
	
		log_it(MON_CON_LOG_VAR, 0, "[From TRACE] %s", rest);
	
/* Nicks could have the '[' or ']' char in them and there is no space between them :( */

		aptr = get_token(&rest, " ");
		while (aptr) {
			if (isdigit(*aptr)) {
				break;
			}
			aptr = get_token(&rest, " ");
		}
		
		aptr = get_token(&rest, " ");
		
		if (aptr2 = strrchr(aptr, '[')) {
			varlen = strlen(aptr);
			varlen2 = strlen(aptr2);
			aptr2 = strcpy(abuf, aptr2);
			aptr2[varlen2 - 1] = '\0';
			aptr2++;
			aptr[varlen - varlen2] = '\0';
		}
		else {
			return;
		}
	
		sprintf(tbuf, "%s!%s", aptr, aptr2);
	
		aptr3 = format_uh(tbuf, 0);
		
		if (current->trace_kill_tog == 2) {
			for (aTimeVar = current->botList; aTimeVar; aTimeVar = aTimeVar->next) {
				if (!matches(aptr3, aTimeVar->name)) {
					sprintf(abuf, "%s No Bots Please", aptr);
					do_kill(current->nick, current->nick, abuf, OWNERLEVEL);
					return;
				}
			}
		}
		else if (current->trace_kill_tog == 1) {
			for (aTimeVar = current->conList; aTimeVar; aTimeVar = aTimeVar->next) {
				if (!matches(aptr3, aTimeVar->name) && (aTimeVar->num > 1)) {
					sprintf(abuf, "%s No Clones Please", aptr);
					do_kill(current->nick, current->nick, abuf, OWNERLEVEL);
					return;
				}
			}
		}
	
		if (aTimeVar = find_time(&current->conList, aptr3)) {
			aTimeVar->num++;
		}
		else {
			aTimeVar = make_time(&current->conList, aptr3);
		}
	
		if (aTimeVar->num > 1) {
			send_monitormsg("[\002Clone\002] \002%s\002 detected with \002%i\002 other clone(s)", tbuf, (aTimeVar->num - 1));
		}
	}
	else {
#endif
/* Welcome to Land of the Loops */
	
		aVarX = current->ParseList;
		while (aVarX) {
			if ((getthetime() - 60 > aVarX->time) && (aVarX->num == 9)) {
				aVarX2 = aVarX->next;
				remove_time(&current->ParseList, aVarX);
				aVarX = aVarX2;
			}
			else if (aVarX->num == 10 || 9) {
				send_to_user(aVarX->name, "[\002%s\002] %s", from, rest);
				aVarX->num = 9;
				aVarX = aVarX->next;
			}
			else {
				aVarX = aVarX->next;
			}
		}
#ifdef SERVER_MONITOR
	}
#endif
}

void parse_213(char *from, char *rest)
{
/* Stats C Parse */
/* Can be used for ALL stats commands */
	aTime *aVarX;
	
	get_token(&rest, " ");
	
	if (rest[0] == ':') {
		rest++;
	}
	
	for (aVarX = current->ParseList; aVarX; aVarX = aVarX->next) {
		if (aVarX->num == 20) {
			send_to_user(aVarX->name, "[\002%s\002] %s", from, rest);
		}
	}
}

void parse_219(char *from, char *rest)
{
/* End of Stats */

	aTime *aVarX, *aVarX2;
	
	aVarX = current->ParseList;
	while (aVarX) {
		if (aVarX->num == 20) {
			aVarX2 = aVarX->next;
			remove_time(&current->ParseList, aVarX);
			aVarX = aVarX2;
		}
		else {
			aVarX = aVarX->next;
		}
	}
}

void parse_243(char *from, char *rest)
{
/* O-Line */	
	char *line, *host, *myhost;
	char tbuf[MAXLEN], tbuf2[MAXLEN];
	aTime *aVarX, *aVarD;
	
	get_token(&rest, " ");
	
	strcpy(tbuf2, rest);
	
	line = get_token(&rest, " ");
	host = get_token(&rest, " ");
	
	sprintf(tbuf, "%s!%s@%s", current->nick, current->login, mylocalhostname);
	myhost = format_uh(tbuf, 1);
	
	if (!matches(host, myhost)) {
		current->hasOLine = TRUE;
	}
	
	for (aVarX = current->ParseList; aVarX; aVarX = aVarX->next) {
		if (aVarX->num == 20) {
			send_to_user(aVarX->name, "[\002%s\002] %s", from, tbuf2);
		}
	}
}

void parse_251(char *from, char *rest)
{
/* This used to be all fancy and do something else but
 * I took it out and just left the fancy code... */
/* Parse Lusers */
	char *users, *invusers, *servers;
	long c_users, c_invusers, tot_users, c_servers;
	int temp = 1;
	aTime *aVarX;
	
	users = get_token(&rest, " ");
	while (temp && users && *users) {
		if (isdigit(*users)) {
			c_users = atol(users);
			temp = 0;
		}
		else {
			users = get_token(&rest, " ");
		}
	}
	
	temp = 1;
	invusers = get_token(&rest, " ");
	while (temp && invusers && *invusers) {
		if (isdigit(*invusers)) {
			c_invusers = atol(invusers);
			tot_users = c_users + c_invusers;
			temp = 0;
		}
		else {
			invusers = get_token(&rest, " ");
		}
	}
	
	temp = 1;
	servers = get_token(&rest, " ");
	while (temp && servers && *servers) {
		if (isdigit(*servers)) {
			c_servers = atol(servers);
			temp = 0;
		}
		else {
			servers = get_token(&rest, " ");
		}
	}

	for (aVarX = current->ParseList; aVarX; aVarX = aVarX->next) {
		if (aVarX->num == 15) {
			send_to_user(aVarX->name, "[\002%s\002] Users: \002%li\002 Invisible: \002%li\002 Servers: \002%li\002", from, tot_users, c_invusers, c_servers);
		}
	}
}

void parse_252(char *from, char *rest)
{
/* Parse Lusers */
	char *opers;
	long c_opers;
	int temp = 1;
	aTime *aVarX;
	
	opers = get_token(&rest, " ");
	while (temp && opers && *opers) {
		if (isdigit(*opers)) {
			c_opers = atol(opers);
			temp = 0;
		}
		else {
			opers = get_token(&rest, " ");
		}
	}
	
	for (aVarX = current->ParseList; aVarX; aVarX = aVarX->next) {
		if (aVarX->num == 15) {
			send_to_user(aVarX->name, "[\002%s\002] IRC Operators \002%li\002", from, c_opers);
		}
	}
}

void parse_253(char *from, char *rest)
{
/* Parse Lusers */
	char *ucons;
	long c_ucons;
	int temp = 1;
	aTime *aVarX;
	
	ucons = get_token(&rest, " ");
	while (temp && ucons && *ucons) {
		if (isdigit(*ucons)) {
			c_ucons = atol(ucons);
			temp = 0;
		}
		else {
			ucons = get_token(&rest, " ");
		}
	}
	
	for (aVarX = current->ParseList; aVarX; aVarX = aVarX->next) {
		if (aVarX->num == 15) {
			send_to_user(aVarX->name, "[\002%s\002] Unknown Connections: \002%li\002", from, c_ucons);
		}
	}
}

void parse_254(char *from, char *rest)
{
/* Parse Lusers */
	char *chans;
	long c_chans;
	int temp = 1;
	aTime *aVarX;
	
	chans = get_token(&rest, " ");
	while (temp && chans && *chans) {
		if (isdigit(*chans)) {
			c_chans = atol(chans);
			temp = 0;
		}
		else {
			chans = get_token(&rest, " ");
		}
	}
	
	for (aVarX = current->ParseList; aVarX; aVarX = aVarX->next) {
		if (aVarX->num == 15) {
			send_to_user(aVarX->name, "[\002%s\002] Channels: \002%li\002", from, c_chans);
		}
	}
}

void parse_255(char *from, char *rest)
{
/* Parse Lusers */
	char *clients, *servers;
	long c_clients, c_servers;
	int temp = 1;
	aTime *aVarX, *aVarX2;
	
	clients = get_token(&rest, " ");
	while (temp && clients && *clients) {
		if (isdigit(*clients)) {
			c_clients = atol(clients);
			temp = 0;
		}
		else {
			clients = get_token(&rest, " ");
		}
	}
	
	temp = 1;
	servers = get_token(&rest, " ");
	
	while (temp && servers && *servers) {
		if (isdigit(*servers)) {
			c_servers = atol(servers);
			temp = 0;
		}
		else {
			servers = get_token(&rest, " ");
		}
	}
	
	for (aVarX = current->ParseList; aVarX; aVarX = aVarX->next) {
		if (aVarX->num == 15) {
			send_to_user(aVarX->name, "[\002%s\002] Clients: \002%li\002 Servers: \002%li\002", from, c_clients, c_servers);
		}
	}
	
	aVarX = current->ParseList;
	while (aVarX) {
		if (aVarX->num == 15) {
			aVarX2 = aVarX->next;
			remove_time(&current->ParseList, aVarX);
			aVarX = aVarX2;
		}
		else {
			aVarX = aVarX->next;
		}
	}
}

void parse_262(char *from, char *rest)
{
/* End of Trace */
/* This doesn't occur on CSr servers :( we'll just have to wait for an update */

	aTime *aVarX, *aVarX2;
	
	aVarX = current->ParseList;
	while (aVarX) {
		if (aVarX->num == 10) {
			aVarX2 = aVarX->next;
			remove_time(&current->ParseList, aVarX);
			aVarX = aVarX2;
		}
		else {
			aVarX = aVarX->next;
		}
	}
}

void parse_301(char *from, char *rest)
{
/* Whois User Away*/
	char tbuf[MAXLEN];
	aTime *aVarX;
	
	get_token(&rest, " "); /* Junk */
	get_token(&rest, ":");
	
	for (aVarX = current->ParseList; aVarX; aVarX = aVarX->next) {
		if (aVarX->num == 5) {
			send_to_user(aVarX->name, "Away            - \002%s\002", rest);
		}
	}
}

void parse_311(char *from, char *rest)
{
/* Whois User */
	char *nick, *userid, *host, *ircname;
	char tbuf[MAXLEN];
	aTime *aVarX;
	
	get_token(&rest, " "); /* Junk */
	nick = get_token(&rest, " ");
	userid = get_token(&rest, " ");
	host = get_token(&rest, " ");
	get_token(&rest, " "); /* Star */
	ircname = get_token(&rest, ":");
	
	strcpy(tbuf, ircname);
	while (ircname = get_token(&rest, " ")) {
		sprintf(tbuf, "%s %s", tbuf, ircname);
	}
	
	for (aVarX = current->ParseList; aVarX; aVarX = aVarX->next) {
		if (aVarX->num == 5) {
			send_to_user(aVarX->name, "Whois Info for  - \002%s\002", nick);
			send_to_user(aVarX->name, "Address         - \002%s@%s\002", userid, host);
			send_to_user(aVarX->name, "IRCName         - \002%s\002", tbuf);
		}
	}
}

void parse_312(char *from, char *rest)
{
/* Whois Server */
	char *nick, *server, *servinfo;
	char tbuf[MAXLEN];
	aTime *aVarX;
	
	get_token(&rest, " ");
	nick = get_token(&rest, " ");
	server = get_token(&rest, " ");
	servinfo = get_token(&rest, ":");
	
	strcpy(tbuf, servinfo);
	while (servinfo = get_token(&rest, " ")) {
		sprintf(tbuf, "%s %s", tbuf, servinfo);
	}
	
	for (aVarX = current->ParseList; aVarX; aVarX = aVarX->next) {
		if (aVarX->num == 5) {
			send_to_user(aVarX->name, "Server          - \002%s (%s)\002", server, tbuf);
		}
	}
}

void parse_313(char *from, char *rest)
{
/* Whois Operator */
	char *nick, *operinfo;
	char tbuf[MAXLEN];
	aTime *aVarX;
	
	get_token(&rest, " ");
	nick = get_token(&rest, " ");
	operinfo = get_token(&rest, ":");
	
	strcpy(tbuf, operinfo);
	while (operinfo = get_token(&rest, " ")) {
		sprintf(tbuf, "%s %s", tbuf, operinfo);
	}
	
	
	for (aVarX = current->ParseList; aVarX; aVarX = aVarX->next) {
		if (aVarX->num == 5) {
			send_to_user(aVarX->name, "IRCop           - \002%s %s\002", nick, tbuf);
		}
	}
}

void parse_317(char *from, char *rest)
{
/* Whois Idle */
	char *nick, *idle, *signon;
	long c_idle, c_signon;
	int min_idle;
	aTime *aVarX;

	get_token(&rest, " ");
	nick = get_token(&rest, " ");
	idle = get_token(&rest, " ");
	signon = get_token(&rest, " ");
	
	c_idle = atol(idle);
	if (signon && *signon) {
		c_signon = atol(signon);
	}
	else {
		c_signon = 0;
	}

	min_idle = (c_idle / 60);
	
	for (aVarX = current->ParseList; aVarX; aVarX = aVarX->next) {
		if (aVarX->num == 5) {
			send_to_user(aVarX->name, "Idle            - \002%i minutes (%li seconds)\002", min_idle, c_idle);
			if (c_signon != 0) {
				send_to_user(aVarX->name, "Signed On       - \002%s\002", time2away(c_signon));
			}
		}
	}
}

void parse_318(char *from, char *rest)
{
/* End of Whois */
	aTime *aVarX, *aVarX2;
	
	aVarX = current->ParseList;
	while (aVarX) {
		if (aVarX->num == 5) {
			aVarX2 = aVarX->next;
			remove_time(&current->ParseList, aVarX);
			aVarX = aVarX2;
		}
		else {
			aVarX = aVarX->next;
		}
	}
}

void parse_319(char *from, char *rest)
{
/* Whois Channels */
	char *nick, *channel;
	char tbuf[MAXLEN];
	aTime *aVarX;
	
	get_token(&rest, " ");
	nick = get_token(&rest, " ");
	channel = get_token(&rest, ":");
	
	strcpy(tbuf, channel);
	while (channel = get_token(&rest, " ")) {
		sprintf(tbuf, "%s %s", tbuf, channel);
	}
	
	for (aVarX = current->ParseList; aVarX; aVarX = aVarX->next) {
		if (aVarX->num == 5) {
			send_to_user(aVarX->name, "Channels        - \002%s\002", tbuf);
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
	aChannel *Channel;
	aChanUser *User;
	
	char 	*temp;	
	char    *channel;
	char    *nick;
	char    *user;
	char    *host;
	char    *server;  /* now used */
	char    *mode;
	aList	*tmp;
	
	temp = getnick(from);
	
	for (Channel=current->Channel_list;Channel;Channel=Channel->next) {
		if ((User=find_chanuser(&Channel->users, temp)) != NULL) {
			if (get_opermode(User) & MODE_OPER) {
				del_oper(&(Channel->users), MODE_OPER, temp);
			}
		}
	}
	
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
	add_user_to_channel(channel, nick, user, host, server);
	/* SEEN */
	addseen(nick, 0, "no_info");   /* More intensive but more flexible also */
	/* END SEEN */

	if (username(nick) && i_am_op(channel))
	  if (is_shitted(username(nick), channel))
		 shit_action(username(nick), channel);
	while (*mode )
	{
		switch( *mode )
		{
		case 'H':
		case 'G':
			break;
		case '*':
			change_usermode( channel, nick, MODE_OPER );
			break;
		case '@':
			if ((tmp=find_list(&current->StealList,channel))!=NULL)
				remove_list(&current->StealList, tmp);
				change_usermode( channel, nick, MODE_CHANOP );
/* 1 */			del_channelmode( channel, CHFL_VOICE, nick );
			break;
		case '+':
			change_usermode( channel, nick, MODE_VOICE );
			del_channelmode( channel, MODE_CHANOP, nick );
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

void parse_361(char *from, char *rest)
{
/* Kill Done */
	if (current->tempvar) {
		need_dcc = 0;
		if (rest) {
			send_to_user(current->tempvar, "[\002%s\002] %s", from, rest);
		}
	}
}

void parse_364(char *from, char *rest)
{
	char	*serv1, *serv2;

	get_token(&rest, " ");
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

void parse_368(char *from, char *rest)
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

void parse_381(char *from, char *rest)
{
	aChannel *chan;
	
	get_token(&rest, " ");
	
	current->hasOLine = TRUE;
	current->isIRCop = TRUE;
	if (!current->Channel_list) {
		return;
	}
	else {
		for (chan = current->Channel_list; chan; chan = chan->next) {
			change_usermode(chan->name, current->nick, MODE_OPER);
		}
	}
	
	if (rest[0] == ':') {
		rest++;
	}
	
	if (current->tempvar) {
		need_dcc = 0;
		send_to_user(current->tempvar, "[\002%s\002] %s", from, rest);
	}

#ifdef SERVER_MONITOR
	current->trace_tog = 1;
	send_to_server("TRACE");
#endif
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

void parse_451(char *from, char *rest)
{
/* If we get a 451 (Not registered) Resend Registration... */
/* This will come about from the new undernet servers and there new random ping junk */
	
	sendregister(current->nick, current->login, current->ircname);
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

void parse_481(char *from, char *rest)
{
	get_token(&rest, " ");
	
	if (rest[0] == ':') {
		rest++;
	}
	
	current->isIRCop = FALSE;
	if (current->tempvar) {
		need_dcc = 0;
		send_to_user(current->tempvar, "[\002%s\002] %s", from, rest);
	}
}

void parse_491(char *from, char *rest)
{
	get_token(&rest, " ");
	
	if (rest[0] == ':') {
		rest++;
	}
	
	current->hasOLine = FALSE;
	if (current->tempvar) {
		need_dcc = 0;
		send_to_user(current->tempvar, "[\002%s\002] %s", from, rest);
	}
}

void parseline(char *line)
{
	char    *from; 
	char    *command;
	char    *rest;
	int     i;

	/* terminate the line where either \r or \n exist */
	/* The new servers put a \r before the \n */

	terminate(line, "\r\n");
	strcpy(lastcommand, line);
/*	check_idle();	*/

#ifdef DBUG
	debug(NOTICE, "parseline(\"%s\")", line);
#endif 

	if (get_int_varc(NULL, TOGSERVLOG_VAR)) {
		log_it(SERVER_LOG_VAR, 0, "%s", line);
	}
	

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
