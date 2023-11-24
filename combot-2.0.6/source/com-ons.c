/*
 * com-ons.c - kinda like /on ^.. in ircII
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>

#include "global.h"
#include "config.h"
#include "defines.h"
#include "structs.h"
#include "h.h"
#include "commands.h"

struct
{
	char    *name;                  /* name of command      */
	void    (*function)(char *, char *, char *, int);
	int     userlevel;              /* required userlvl     */
	int     forcedcc;               /* If true, command requires DCC */
	int     needcc;                 /* If true, command requires cmdchar */
	int     needpass;
} on_msg_commands[] =

/* Any command which produces more than 2 lines output can be 
	used to make the bot "excess flood". Make it "forcedcc"     */

/*   Command        function   userlvl   forcedcc  needcmdchar */
{
	{ "AUTH",       do_auth,         -1,    FALSE,    FALSE, FALSE },
	{ "GLOBAL",     parse_global,    -1,    FALSE,     TRUE, FALSE },
	{ "ACCESS",     do_access,       -1,    FALSE,     TRUE, FALSE },
	{ "HELP",       do_help,         -1,     TRUE,     TRUE, FALSE },
	{ "CHAT",       do_chat,          0,    FALSE,     TRUE, FALSE },
	{ "IDLE",	do_idle,	  1,    FALSE,     TRUE, FALSE },
	{ "DOWN",       do_deopme,        0,    FALSE,    FALSE, FALSE },
	{ "FILES",      do_flist,         0,     TRUE,     TRUE, FALSE },
	{ "FLIST",      do_flist,         0,     TRUE,     TRUE, FALSE },
	{ "PING",       do_ping,          1,    FALSE,     TRUE, FALSE },
	{ "SEND",       do_send,          0,    FALSE,     TRUE, FALSE },
	{ "STATS",      do_stats,        -1,     TRUE,     TRUE, FALSE },
	{ "INFO",       do_info,          0,     TRUE,     TRUE, FALSE },
	{ "VERSION",    do_version,       0,    FALSE,     TRUE, FALSE },
	{ "PASSWD",     do_passwd,        1,    FALSE,     TRUE, FALSE },
	{ "KICKS",	do_kicks,	  1,	FALSE,	   TRUE, FALSE },
	{ "TOPKICKS",   do_topkicks,      1,     TRUE,     TRUE, FALSE },
	{ "ROLLCALL",   do_rollcall,     20,    FALSE,    FALSE, FALSE },
	{ "UP",         do_opme,         50,    FALSE,    FALSE,  TRUE },
	{ "CHANNELS",   do_channels,     20,     TRUE,     TRUE,  TRUE },
	{ "GETCH",      do_getch,        20,    FALSE,     TRUE,  TRUE },
	{ "CHANSTATS",  do_chanstats,    20,    FALSE,     TRUE,  TRUE },
	{ "B",          do_ban,          30,    FALSE,     TRUE,  TRUE },
	{ "UB",         do_unban,        30,    FALSE,     TRUE,  TRUE },
	{ "OP",         do_op,           90,    FALSE,     TRUE,  TRUE },
	{ "DEOP",       do_deop,         90,    FALSE,     TRUE,  TRUE },
	{ "K",          do_kick,         30,    FALSE,     TRUE,  TRUE },
	{ "KB",         do_kickban,      30,    FALSE,     TRUE,  TRUE },
	{ "INVITE",     do_invite,       40,    FALSE,     TRUE,  TRUE },
	{ "TOPIC",      do_topic,        40,    FALSE,     TRUE,  TRUE },
	{ "SB",         do_siteban,      40,    FALSE,     TRUE,  TRUE },
	{ "SKB",        do_sitekickban,  40,    FALSE,     TRUE,  TRUE },
	{ "MOPU",       do_massopu,      50,    FALSE,     TRUE,  TRUE },
	{ "MDEOPNU",    do_massdeopnu,   50,    FALSE,     TRUE,  TRUE },
	{ "BANLIST",    do_banlist,      50,     TRUE,     TRUE,  TRUE },
	{ "MUB",        do_massunban,    50,    FALSE,     TRUE,  TRUE },
	{ "MUBF",       do_massunbanfrom,50,    FALSE,     TRUE,  TRUE },
	{ "FUCK",	do_fuck,	 50,	FALSE,	  FALSE,  TRUE },
	{ "SPYMSG",     do_spymsg,       70,    FALSE,     TRUE,  TRUE },
	{ "RSPYMSG",    do_rspymsg,      70,    FALSE,     TRUE,  TRUE },
	{ "SPY",        do_spy,          70,     TRUE,     TRUE,  TRUE },
	{ "RSPY",       do_rspy,         70,    FALSE,     TRUE,  TRUE },
	{ "REPORT",	do_report,	 70,	 TRUE,     TRUE,  TRUE },
	{ "AOP",        do_aop,          70,    FALSE,     TRUE,  TRUE },
	{ "RAOP",       do_raop,         70,    FALSE,     TRUE,  TRUE },
	{ "DCCLIST",    do_listdcc,      70,     TRUE,     TRUE,  TRUE },
	{ "SAY",        do_say,          80,    FALSE,     TRUE,  TRUE },
	{ "ME",         do_me,           80,    FALSE,     TRUE,  TRUE },
	{ "CLVL",       do_clvl,         80,    FALSE,     TRUE,  TRUE },
	{ "MSG",        do_msg,          80,    FALSE,     TRUE,  TRUE },
	{ "JOIN",       do_join,         80,    FALSE,     TRUE,  TRUE },
	{ "CYCLE",      do_cycle,        80,    FALSE,     TRUE,  TRUE },
	{ "LEAVE",      do_leave,        80,    FALSE,     TRUE,  TRUE },
	{ "PART",	do_leave,	 80,	FALSE,	   TRUE,  TRUE },
	{ "FORGET",	do_forget,	 80,    FALSE,     TRUE,  TRUE },
	{ "NICK",       do_nick,         80,    FALSE,     TRUE,  TRUE },
	{ "ADD",        do_add,          80,    FALSE,     TRUE,  TRUE },
	{ "DEL",        do_del,          80,    FALSE,     TRUE,  TRUE },
	{ "SHIT",       do_shit,         50,    FALSE,     TRUE,  TRUE },
	{ "CSLVL",	do_cslvl,	 50,	FALSE,	   TRUE,  TRUE },
	{ "RESTRICT",   do_restrict,     80,    FALSE,     TRUE,  TRUE },
	{ "LIMIT",      do_limit,        80,    FALSE,     TRUE,  TRUE },
	{ "RSHIT",      do_rshit,        10,    FALSE,     TRUE,  TRUE },
	{ "COLLECT",	do_collect,	 80,    FALSE,     TRUE,  TRUE },
	{ "PROT",       do_prot,         80,    FALSE,     TRUE,  TRUE },
	{ "RPROT",      do_rprot,        80,    FALSE,     TRUE,  TRUE },
	{ "SHOWUSERS",  do_showusers,    80,     TRUE,     TRUE,  TRUE },
	{ "WHO",	do_showusers,    80,     TRUE,     TRUE,  TRUE },
	{ "NAMES",	do_names,	 80,    FALSE,     TRUE,  TRUE },
	{ "SHOWIDLE",   do_showidle,     80,     TRUE,     TRUE,  TRUE },
	{ "MKNU",       do_masskicknu,   80,    FALSE,     TRUE,  TRUE },
	{ "USERLIST",   do_userlist,     80,     TRUE,     TRUE,  TRUE },
	{ "NICKLIST",   do_nicklist,     80,     TRUE,     TRUE,  TRUE },
	{ "SHITLIST",   do_shitlist,     80,     TRUE,     TRUE,  TRUE },
	{ "GREETLIST",	do_greetlist,	 80,	 TRUE,     TRUE,  TRUE },
	{ "KSLIST",	do_kicksaylist,	 80,	 TRUE,	   TRUE,  TRUE },
	{ "SSLIST",	do_saysaylist,	 80,	 TRUE,	   TRUE,  TRUE },
	{ "SET",        do_set,          80,    FALSE,     TRUE,  TRUE },
	{ "TOG",	do_toggle,	 80,	FALSE,	   TRUE,  TRUE },
	{ "GREET",	do_greet,	 80,    FALSE,     TRUE,  TRUE },
	{ "RGREET",	do_rgreet,	 80,	FALSE,	   TRUE,  TRUE },
	{ "KS",		do_kicksay,	 80,	FALSE,	   TRUE,  TRUE },
	{ "RKS",	do_rkicksay,	 80,	FALSE,	   TRUE,  TRUE },
	{ "SS",		do_saysay,	 80,	FALSE,	   TRUE,  TRUE },
	{ "RSS",	do_rsaysay,	 80,	FALSE,	   TRUE,  TRUE },
	{ "SETSHITREASON", do_setshitreason, 10,FALSE,	   TRUE,  TRUE },
	{ "MOP",        do_massop,       85,    FALSE,     TRUE,  TRUE },
	{ "MDEOP",      do_massdeop,     85,    FALSE,     TRUE,  TRUE },
	{ "MK",         do_masskick,     85,    FALSE,     TRUE,  TRUE },
	{ "MKB",        do_masskickban,  85,    FALSE,     TRUE,  TRUE },
	{ "USERHOST",   do_userhost,     90,    FALSE,     TRUE,  TRUE },
	{ "SPYLIST",    do_spylist,      90,    FALSE,     TRUE,  TRUE },
	{ "STEAL",      do_steal,        90,    FALSE,     TRUE,  TRUE },
	{ "RSTEAL",     do_rsteal,       90,    FALSE,     TRUE,  TRUE },
	{ "CMDCHAR",    do_cmdchar,      90,    FALSE,     TRUE,  TRUE },
	{ "SERVER",     do_server,       90,    FALSE,     TRUE,  TRUE },
	{ "NEXTSERVER", do_nextserver,   90,    FALSE,     TRUE,  TRUE },
	{ "BOMB",       do_bomb,         90,    FALSE,     TRUE,  TRUE },
/*
	{ "LAST10",     do_last10,       95,     TRUE,     TRUE,  TRUE },
*/
	{ "SPLITS",	do_splits,	 95,	 TRUE,     TRUE,  TRUE },
	{ "LINKS",	do_links,	 95,     TRUE,     TRUE,  TRUE },
	{ "TOPSPLITS",  do_topsplits,    95,     TRUE,     TRUE,  TRUE },
	{ "CHACCESS",   do_chaccess,     95,    FALSE,     TRUE,  TRUE },
	{ "PROTNICK",   do_protnick,     95,    FALSE,     TRUE,  TRUE },
	{ "RPROTNICK",  do_rprotnick,    95,    FALSE,     TRUE,  TRUE },
	{ "SAVEKICKS",  do_savekicks,    95,    FALSE,     TRUE,  TRUE },
	{ "LOADKICKS",  do_loadkicks,    95,    FALSE,     TRUE,  TRUE },
	{ "LOADALL",    do_loadall,      95,    FALSE,     TRUE,  TRUE },
	{ "SAVEALL",    do_saveall,      95,    FALSE,     TRUE,  TRUE },
	{ "EXEC",	do_exec,	 90,    FALSE,     TRUE,  TRUE },
	{ "LOADLISTS",  do_loadlists,    99,    FALSE,     TRUE,  TRUE },
	{ "SAVELEVELS", do_savelevels,	 90,	FALSE,	   TRUE,  TRUE },
	{ "LOADLEVELS", do_loadlevels,	 99,	FALSE,	   TRUE,  TRUE },
	{ "SAVELISTS",  do_savelists,    90,    FALSE,     TRUE,  TRUE },
	{ "DO",         do_do,           99,    FALSE,     TRUE,  TRUE },
	{ "RELOGIN",    do_relogin,      99,    FALSE,     TRUE,  TRUE },
	{ "SPAWN",      do_spawn,        95,    FALSE,     TRUE,  TRUE },
	{ "SETPASS",	do_setpass,	 99,	FALSE,	   TRUE,  TRUE },
	{ "DIE",        do_die,          99,    FALSE,     TRUE,  TRUE },
	/*
	 :
	 :
	 */
	{ NULL,         (void(*)(char *, char *, char *, int))(NULL), 0, FALSE, FALSE }
};

void on_kick(from, channel, nick, reason)
char *from;
char *channel;
char *nick;
char *reason;
{
	char	*temp;
	aTime	*Kicks;
	aChannel *Dummy;

	send_spy(channel, "%s has been kicked off channel %s by %s (%s)",
		nick, channel, getnick(from), reason);
	if (my_stricmp(currentnick, current->nick))
		if (check_masskick(from, channel))
			if (!CurrentUser || CurrentUser->access < ASSTLEVEL)
				mass_action(from, channel);
	if (!my_stricmp(current->nick, nick))
	{
		set_kickedby(channel, from);
/*      strcpy(current->nick, random_str(3, 9));
		sendnick(current->nick); */
		join_channel(channel, NULL, TRUE);
		lastrejoin = getthetime();
		if ((Dummy = find_channel(channel)) != NULL)
			Dummy->flags |= REJOIN_TYPE;
	}
	if (((temp=username(nick)) != NULL) &&
		get_protlevel(temp, channel))
	{
		log_it(DANGER_LOG_VAR, 0, "%s kicked %s from %s",
			from, username(nick), channel);
		prot_action(from, channel, username(nick));
		if (!is_me(nick))
			send_to_server("INVITE %s %s", nick, channel);
	}
	temp = format_nuh(from);
	Kicks = find_time(&current->KickList, temp);
	if (Kicks)
	{
		Kicks->num++;
		Kicks->time = getthetime();
	}
	else
		make_time(&current->KickList, temp);
}

void on_join(who, channel)
char *who;
char *channel;
{
	aUser *Lame;

	send_spy(channel, "%s has joined channel %s",
		who, channel);
	if (get_int_var(channel, TOGABK_VAR) && is_banned(who, channel))
	{
		sendmode(channel, "-o %s", currentnick);
		sendkick(channel, currentnick, "\002%s\002",
			"You are banned on this server");
	} 
	if (!is_a_bot(who) && (!CurrentUser || CurrentUser->access < 50))
		check_clonebots(who, channel);

	if (CurrentShit && CurrentShit->access && !CurrentUser &&
		!is_a_bot(who))
	{
		shit_action(who, channel);
		return;
	}
	Lame = find_protnick(&Userlist, who);
	if (Lame && !is_a_bot(who))
		if (!CurrentUser || CurrentUser->access < Lame->access)
		{
			sendmode(channel, "-o+b %s %s", currentnick,
				format_uh(who, 1));
			sendkick(channel, currentnick, "\002%s\002",
				Lame->reason);
		}
	if (get_int_var(channel, TOGGREET_VAR))
	if (!is_a_bot(who) && (!CurrentUser || CurrentUser->access < 100))
		if ((Lame=find_greet(&Userlist, who)) != NULL)
			sendprivmsg(channel, "%s",
				formatgreet(Lame->reason, currentnick));
	if (!i_am_op(channel))
		return;
	if (is_a_bot(who))
		giveop(channel, currentnick);
	else if (is_aop(who, channel) && get_int_var(channel, TOGAOP_VAR))
	  if (!password_needed(who) || verified(who))
	    if (CurrentUser && CurrentUser->access >= current->restrict)
		giveop(channel, currentnick);
}

void on_topic(from, channel, topic)
char *from;
char *channel;
char *topic;
{
	if (!is_a_bot(from))
		reverse_topic(from, channel, topic);
}

void on_part(who, channel)
char *who;
char *channel;
{
	send_spy(channel, "%s has left channel %s",
		who, channel);
}

void on_nick(who, newnick)
char *who;
char *newnick;
{
	register aUser *Lame;
	register aList *tmp;
	register aTime *tmp2;
	static char newnuh[MAXLEN];
	int changed;

	changed = 0;
	strcpy(newnuh, newnick);
	strcat(newnuh, strchr(who, '!'));

	check_nickflood(who);  /* kicking taken care of in that function */
	send_common_spy(who, "%s is now known as %s", who, newnick);
	change_memnick(who, newnuh);
        change_dccnick(who, newnuh);
	if ((tmp=find_list(&Bots, who)) != NULL)
	{
		MyFree((char **)&tmp->name);
		mstrcpy(&tmp->name, newnuh);
		changed = 1;
	}
	if ((tmp2=find_time(&Auths, who)) != NULL)
	{
		MyFree((char **)&tmp2->name);
		mstrcpy(&tmp2->name, newnuh);
		changed = 1;
	}
	if (!my_stricmp(current->nick, newnick))
		changed = 1;
        Lame = find_protnick(&Userlist, newnuh);
        if (Lame && !is_a_bot(newnuh))
                if (!CurrentUser || CurrentUser->access < Lame->access)
			kickban_all_channels(newnick,
				format_uh(newnuh, 1),
				Lame->reason);
#ifdef USE_CCC
	if (changed)
		send_to_ccc("1 NICK %s %s", who, newnuh);
#endif
}

void on_quit(who, reason)
char *who;
char *reason;
{
	send_common_spy(who, "Signoff %s (%s)", who, reason);
}

void on_msg(from, to, msg)
char *from;
char *to;
char *msg;
{
	static char from_copy[MAXLEN];
	static char to_copy[MAXLEN];
	static char msg_copy[HUGE];
	char *command, *tempcommand;
	int needcc, pubcommand, i;
	aDCC *userclient;
	aTime *tmp;
	int ulevel, maxul;

	if (!from || !msg || !*msg)
		return;    /* bug fix from 99A2 and below */
	msg[254] = '\0';
	strcpy(from_copy, from);
	strcpy(to_copy, to);
	strcpy(msg_copy, msg);

	check_memory(from);
	ulevel = CurrentUser ? CurrentUser->access : 0;
	if (ulevel)
		if (CurrentShit && CurrentShit->access)
			return;
	maxul = max_userlevel(from);
	if ((tmp = find_time(&Auths, from)) != NULL)
		tmp->time = getthetime();
	if (ulevel < 75)
		if (check_flood(gethost(from)))
			return;
	update_flood();  /* Frees old crap from memory */
	if ((command = get_token(&msg, " "))== NULL)
		return;
	if ((pubcommand=ischannel(to)) && !get_int_var(to, TOGPUB_VAR))
	{
		on_public(from_copy, to_copy, msg_copy);
		return;
	}
	userclient = search_list("chat", from, DCC_CHAT);
	needcc = get_int_varc(NULL, TOGCC_VAR);
	if (!my_stricmp(left(command, strlen(current->nick)), current->nick) &&
		((command[strlen(current->nick)] == '\0') ||
		(strchr(":-", command[strlen(current->nick)]) &&
		(command[strlen(current->nick)+1] == '\0'))))
	{
		if (!(command = get_token(&msg, " ")))
			return;
		needcc = 0;
	}
	if (from_shell)
		needcc = 0;
	for (i = 0; on_msg_commands[i].name; i++)
	{
		 tempcommand = command;
		 if (needcc && on_msg_commands[i].needcc &&
				(*tempcommand != current->cmdchar))
			continue;
		 if (*tempcommand == current->cmdchar)
			tempcommand++;
		if (!my_stricmp(on_msg_commands[i].name, tempcommand))
		{
			if (!strcmp(on_msg_commands[i].name, "AUTH") &&
				pubcommand)
			{
				send_to_user(from, "\002Public authing defeats the purpose, don't you think???\002");
				return;
			}
			if (maxul < on_msg_commands[i].userlevel)
			{
				if (pubcommand && !on_msg_commands[i].needcc)				
					on_public(from, to, msg_copy);
				else {
				log_it(FAILED_LOG_VAR, 0, "%s tried: %s",
					from, msg_copy);
				if (!pubcommand)
					send_to_user(from, "\002Sorry, you need level %i to be able to do that command\002", on_msg_commands[i].userlevel);
				}
				return;
			}
			if ((maxul < current->restrict) &&
				 (on_msg_commands[i].userlevel != -1))
			{
				log_it(FAILED_LOG_VAR, 0, "%s tried: %s",
                                        from, msg_copy);
				send_to_user(from, "\002Sorry, I'm being restricted to at least level %i\002", current->restrict);
				return;
			}
			if (!from_shell && password_needed(from) &&
				 on_msg_commands[i].needpass && !verified(from))
			  {
			    if (on_msg_commands[i].needcc || !pubcommand)
			      send_to_user(from, "\002Use \"auth\" to get verified first\002");
				 return;
			  }
			log_it(COMMANDS_LOG_VAR, 0, "%s did: %s", from,
				msg_copy);
			need_dcc = on_msg_commands[i].forcedcc;
			if (on_msg_commands[i].forcedcc &&
			  (!userclient || (userclient->flags&DCC_WAIT)))
			{
				nodcc_session(from, to, msg_copy);
				need_dcc = 1; /* don't ask */
#ifdef AUTO_DCC
				dcc_chat(from, msg);
#else
				sendnotice(currentnick,
					"\002Sorry, %s is only available through DCC Chat\002",
					command);
				sendnotice(currentnick,
					"\002%s\002", "Please start a dcc chat connection first");
#endif /* AUTO_DCC */
				return;
			}
			on_msg_commands[i].function(from, to, (msg && !*msg) ? NULL : msg,
					on_msg_commands[i].userlevel);

/*
			for (ii=8;ii>=0;ii--)
				strcpy(lastcmds[ii+1],lastcmds[ii]);
			strcpy(lastcmds[0], on_msg_commands[i].name);
			strcat(lastcmds[0], " done by ");
			strcat(lastcmds[0], left(from,50));
*/
			return;
		}
			}
	if (*command == current->cmdchar)
		log_it(FAILED_LOG_VAR, 0, "%s tried: %s", from, msg_copy);
	if (!pubcommand)
	  {
		 if (*command == current->cmdchar)
			send_to_user(from, "\002%s\002", "Uh...huh huh...what???");
		else
			send_spymsg("\002*%s*\002 %s", currentnick, msg_copy);
	  }
	else
	  {
		on_public(from_copy, to_copy, msg_copy);
	  }
}

void on_mode(from, channel, rest)
char *from;
char *channel;
char *rest;
{
	int     did_change, enfm, doh;
	char    *chanchars, *params, sign;

	char    unminmode[MAXLEN];      /* used to undo modechanges */
	char    unminparams[MAXLEN];
	char    unplusmode[MAXLEN];     /* used to undo modechanges */
	char    unplusparams[MAXLEN];
	aUser   *dummy; 
	char    *nick;
	char    *banstring;
	char    *key;
	char    *limit;
	char	*nickuh;

	did_change = FALSE;
	enfm = 0;
	strcpy(unminmode, "");          /* min-modes, i.e. -bi */
	strcpy(unminparams, "");        /* it's parameters */
	strcpy(unplusmode, "");         /* plus-modes, i.e. +oo */
	strcpy(unplusparams, "");       /* parameters */

	if (!mychannel(channel))
	  return;
	send_spy(channel, "Mode change \"%s\" on channel %s by %s",
		 rest, channel, from);
	chanchars = get_token(&rest, " ");
	params = rest;
/*	update_idletime(from, channel);  don't update on modes */
	while( *chanchars )
	{
	switch( *chanchars )
	{
		case '+':
		case '-':
		sign = *chanchars;
		break;
/*o*/		case 'o':
		nick = get_token(&params, " ");
		nickuh = username(nick);
		if (!nickuh)
		{
			send_to_server("WHO %s", channel);
			break;
		}
/* +o */	if( sign == '+' )
		{
		add_channelmode(from, channel, CHFL_CHANOP, nick, getthetime());
		if (is_me(nick))
		{
			aList *tmp;
			char *tempy;

			if (!is_me(current->realnick))
			{
				strcpy(current->nick,current->realnick);
				sendnick(current->nick);
			}
			if ((tmp=find_list(&current->StealList, channel)) != NULL)
			{	
				int temp = get_int_var(channel, SETMAL_VAR);
				set_int_var(channel, SETMAL_VAR, 99);
				channel_massdeop(channel, "*!*@*");
				set_int_var(channel, SETMAL_VAR, temp);
				remove_list(&current->StealList, tmp);
			}
			check_shit(); /* kick shitters */
			tempy = get_kickedby(channel);             
			if (get_int_var(channel, TOGRK_VAR) && tempy &&
				(get_userlevel(tempy, channel) < ASSTLEVEL) && 
				username(getnick(tempy)) &&
				(get_userlevel(username(getnick(tempy)),channel) < ASSTLEVEL))
				sendkick(channel, getnick(tempy), "\002%s\002", "Doh! This is revenge fucker!");
			set_kickedby(channel, NULL);
		}
		if (!is_user(nickuh,channel) && get_shitlevel(nickuh, channel))
		{
			strcat(unminmode, "o");
			strcat(unminparams, nick);
			strcat(unminparams, " ");
			did_change=TRUE;
		}
		else if (get_int_var(channel, TOGSD_VAR) &&
			!strchr(from, '@') &&
			!is_user(nickuh, channel))
		{
			log_it(DANGER_LOG_VAR, 0, "Serverops: %s from %s on %s",
				nickuh, from, channel);
			strcat(unminmode, "o");
			strcat(unminparams, nick);
			strcat(unminparams, " ");
			did_change=TRUE;
		}
		else if (get_int_var(channel, TOGSO_VAR) &&
			 !is_user(nickuh, channel) &&
			 (!CurrentUser || CurrentUser->access < 95) &&
			 (get_userlevel(nickuh, channel) < 30))
		{
			log_it(DANGER_LOG_VAR, 0, "Strictops: %s opped %s on %s",
				from, nickuh, channel);
			strcat(unminmode, "o");
			strcat(unminparams, nick);
			strcat(unminparams, " ");
			did_change = TRUE;
		}
		}
/* -o */	else
		{
			del_channelmode(channel, CHFL_CHANOP, nick);
			if (!(is_a_bot(from) ||
				!my_stricmp(currentnick, nick)))
			{
			if (check_massdeop(from, channel) &&
				(!CurrentUser ||
					CurrentUser->access < ASSTLEVEL))
				mass_action(from, channel);
			if (get_int_var(channel, TOGPROT_VAR) &&
				get_protlevel(nickuh, channel) &&
				!get_shitlevel(nickuh, channel))
			{
				log_it(DANGER_LOG_VAR, 0, "%s deopped %s on %s",
						from, nickuh, channel);
				if (!password_needed(nickuh) ||
					verified(nickuh) ||
					is_a_bot(nickuh))
				{ 
					strcat(unplusmode, "o");
					strcat(unplusparams, nick);
					strcat(unplusparams, " ");
					prot_action(from, channel, nick);
					did_change=TRUE;
				}
			}
		      }
		}
		break;
/* v */		case 'v':
		nick = get_token(&params, " ");
		if (sign == '+')
			add_channelmode(from, channel, CHFL_VOICE, nick, getthetime() );
		else
			del_channelmode( channel, CHFL_VOICE, nick );
		break;
		case 'b':
		banstring = get_token(&params, " ");
/* +b */	if (sign == '+')
		{
		add_channelmode(from, channel, MODE_BAN, banstring,
				getthetime());
		if (get_int_var(channel, SETBT_VAR) && (num_banned(channel) == 20))
			channel_massunbanfrom(channel, "*!*@*",
				60*get_int_var(channel, SETBT_VAR));
		else if (get_int_var(channel, SETAUB_VAR))
			auto_unban(get_int_var(channel, SETAUB_VAR));
		if (is_a_bot(from))
			break;
		if (check_massban(from, channel) &&
			(!CurrentUser || CurrentUser->access < ASSTLEVEL))
			mass_action(from, channel);
		if (get_int_var(channel, TOGPROT_VAR) &&
			get_protlevel_matches(banstring, channel))
		{
			dummy = get_shitrec_matches(banstring, channel);
			if (!dummy || !dummy->access)
			{
				log_it(DANGER_LOG_VAR, 0, "%s banned %s on %s",
						from, banstring, channel);
				strcat(unminmode, "b");
				strcat(unminparams, banstring);
				strcat(unminparams, " ");
				prot_action(from, channel, banstring);
				did_change=TRUE;
			}
		}
		}
/* -b */	else
		{
		del_channelmode(channel, MODE_BAN, banstring);
		dummy = NULL;
		if (!get_int_var(channel, TOGSHIT_VAR))
			break;
		dummy = get_shitrec_matches(banstring, channel);
		doh = dummy ? dummy->access : 0;
		if (doh < 3)
		{
			dummy = find_shit(&Userlist,
				banstring, channel);
			doh = dummy ? dummy->access : 0;
		}
		if ((doh >= 3) && dummy->userhost)
		{
			strcat(unplusmode, "b");
			strcat(unplusparams, dummy->userhost);
			strcat(unplusparams, " ");
		 	did_change=TRUE;
		}
		}
		break;
		case 'p':
		did_change = enfm = reverse_mode(from, channel, 'p', sign);
		if (enfm && (sign == '+'))
			strcat(unminmode, "p");
		else if (enfm)
			strcat(unplusmode, "p");
		if (sign == '+')
			add_channelmode(from, channel, MODE_PRIVATE, "", getthetime());
		else
			del_channelmode(channel, MODE_PRIVATE, "");
		break;
		case 's':
		did_change = enfm = reverse_mode(from, channel, 's', sign);
		if (enfm && (sign == '+'))
			strcat(unminmode, "s");
		else if (enfm)
			strcat(unplusmode, "s");
		if (sign == '+')
			add_channelmode(from, channel, MODE_SECRET, "", getthetime());
		else
			del_channelmode(channel, MODE_SECRET, "");
		break;
		case 'm':
		did_change = enfm = reverse_mode(from, channel, 'm', sign);
		if (enfm && (sign == '+'))
			strcat(unminmode, "m");
		else if (enfm)
			strcat(unplusmode, "m");
		if (sign == '+')
			add_channelmode(from, channel, MODE_MODERATED, "", getthetime());
		else
			del_channelmode(channel, MODE_MODERATED, "");
		break;
		case 't':
		did_change = enfm = reverse_mode(from, channel, 't', sign);
		if (enfm && (sign == '+'))
			strcat(unminmode, "t");
		else if (enfm)
			strcat(unplusmode, "t");
		if (sign == '+')
			add_channelmode(from, channel, MODE_TOPICLIMIT, "", getthetime());
		else
			del_channelmode(channel, MODE_TOPICLIMIT, "");
		break;
		case 'i':
		did_change = enfm = reverse_mode(from, channel, 'i', sign);
		if (enfm && (sign == '+'))
			strcat(unminmode, "i");
		else if (enfm)
			strcat(unplusmode, "i");
		if (sign == '+')
			add_channelmode(from, channel, MODE_INVITEONLY, "", getthetime());
		else
			del_channelmode(channel, MODE_INVITEONLY, "");
		break;
		case 'n':
		did_change = enfm = reverse_mode(from, channel, 'n', sign);
		if (enfm && (sign == '+'))
			strcat(unminmode, "n");
		else if (enfm)
			strcat(unplusmode, "n");
		if (sign == '+')
			add_channelmode(from, channel, MODE_NOPRIVMSGS, "", getthetime());
		else
			del_channelmode(channel, MODE_NOPRIVMSGS, "");
		break;
		case 'k':
		key = get_token(&params, " ");
		did_change = enfm = reverse_mode(from, channel, 'k', sign);
		if (!key)
			did_change = FALSE;
		if (enfm && (sign == '+'))
		{
			if (key)
			{
				strcat(unminmode, "k");
				strcat(unminparams, key);
				strcat(unminparams, " ");
			}
		}
		else if (enfm)
		{
			if (key)
			{
				strcat(unplusmode, "k");
				strcat(unplusparams, key);
				strcat(unplusparams, " ");
			}
		}
		if (sign == '+')
			add_channelmode(from, channel, MODE_KEY, key ? key : "???", getthetime());
		else
			del_channelmode(channel, MODE_KEY, "");
		break;
		case 'l':
		if (sign == '+')
		{
			limit = get_token(&params, " ");
			add_channelmode(from, channel, MODE_LIMIT, limit ? limit : "0", getthetime());
		}
		else
			del_channelmode(channel, MODE_LIMIT, "");
		did_change = enfm = reverse_mode(from, channel, 'l', sign);
		if (enfm && (sign == '+'))
			strcat(unminmode, "l");
		else if (enfm)
		{
			strcat(unplusmode, "l");
			strcat(unplusparams, "69 ");
		}
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
	if (did_change && i_am_op(channel))
		sendmode( channel, "+%s-%s %s %s", unplusmode,
			  unminmode, unplusparams, unminparams);
}

void on_public(from, to, rest)
char *from;
char *to;
char *rest;
{
	int level;
	char thehost[100];
	aTime *Temp;
	aUser *triggers;

	strcpy(thehost, gethost(from));	
	if ((Temp = find_time(&current->OffendersList, thehost)) == NULL)
		if ((Temp=make_time(&current->OffendersList, thehost)) != NULL)
			Temp->num = 0;
	update_idletime(from, to);
	level = numchar(rest, '\007');  
	if (level)
	if (check_beepkick(from, to, level))
	{
		if (Temp)
		{
			Temp->num++;
			if (Temp->num == 3)
				sendmode(to, "+b %s", format_uh(from, 1));
			else if (Temp->num > 3)
			{
				char *temp;
				temp = format_uh(from, 2);
				channel_massunban(to, temp, 0);
				sendmode(to, "+b %s", temp);
			}
		}
		sendkick(to,currentnick,"\002Please chill on the beeping\002");
	}
	level = percent_caps(rest);
	if (level >= 50)
	if (check_capskick(from, to, 1))
	{
		if (Temp)
		{
			Temp->num++;
			if (Temp->num == 3)
				sendmode(to, "+b %s", format_uh(from, 1));
			else if (Temp->num > 3)
			{
				char *temp;
				temp = format_uh(from, 2);
				channel_massunban(to, temp, 0);
				sendmode(to, "+b %s", temp);
			}
		}
		sendkick(to, currentnick, "\002STOP YELLING ALREADY!!!\002");
	}
	send_spy(to, "\002<%s:%s>\002 %s", currentnick, to, rest);
	if ((level=check_pubflooding(from, to)) && (!CurrentUser || !CurrentUser->prot))
	{
		if (Temp)
		{
			Temp->num++;
			if (Temp->num >= 4)
			{
                           if ((!CurrentUser || !CurrentUser->access) && get_int_var(to, TOGAS_VAR))
				add_to_shitlist(&Userlist, format_uh(from,1), 2,
						to, "Auto-Shit", 
					 "Bah...flooding and or beeping/CAPSing", getthetime(), getthetime() + (long) 86400);
				/* auto shitlist */
			}
		}
		if (level >= 2)
		  deop_ban(to, currentnick, from);
		if (level >= 1)
		  sendkick(to, currentnick, "\002Get out you flooder!\002"); 
	 }
	if (!(*rest == '\002' && *(rest+1) == '\002'))
	{
	if (get_int_var(to, TOGSS_VAR))
		if ((triggers = check_saysay(rest, to)) != NULL)
			if (!CurrentUser || CurrentUser->access < ASSTLEVEL)
			{
				char *temp;
				temp = formatgreet(triggers->reason,
					currentnick);
				sendprivmsg(to, "\002\002%s", temp);
			}
	if (get_int_var(to, TOGKS_VAR))	
		if ((triggers = check_kicksay(rest, to)) != NULL)
			if (!CurrentUser || !CurrentUser->access)
		sendkick(to, currentnick, "\002Watch the banned word(s)!\002");
	}
}

void do_chaccess(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  
  int i, oldlevel, newlevel;
  char *name, *level;
  
  name = get_token(&rest, " ");
  level = get_token(&rest, " ");
  
  if (!level && !name)
	 {
		send_to_user(from, "\002Usage: chaccess <commandname> [level]\002");
		return;
	 }
  if (level)
	 {
		newlevel = atoi(level);
		if ((newlevel < 0) || (newlevel > OWNERLEVEL))
	{
	  send_to_user(from, "\002Level must be between 0 and %i\002", OWNERLEVEL);
	  return;
	}
	 }
  else
	 newlevel = -1;
  if (newlevel > get_userlevel(from, "*"))
	 {
		send_to_user(from, "\002%s\002", "Can't change level to one higher than yours");
		return;
	 }
  for (i = 0; on_msg_commands[i].name; i++ )
	 {
		if (!my_stricmp(on_msg_commands[i].name, name))
	{
	  oldlevel = on_msg_commands[i].userlevel;
	  if (newlevel == -1)
		 {
			send_to_user(from, "\002The access level needed for that command is %i\002",
				oldlevel);
			send_to_user(from, "\002%s\002", "To change it, specify a level");
			return;
		 }
	  if (oldlevel > get_userlevel(from, "*"))
		 {
			send_to_user(from, "\002%s\002", "Can't change a level that is higher than yours");
			return;
		 }
	  if (oldlevel == newlevel)
		 send_to_user(from, "\002%s\002", "The level was not changed");
	  else
		 send_to_user(from, "\002Level changed from %i to %i\002",
			 oldlevel, newlevel);
	  on_msg_commands[i].userlevel = newlevel;
	  return;
	}
	 }
  send_to_user(from, "\002%s\002", "That command is not known");
  return;
}
			
int level_needed(name)
char *name;
{
  int i;
  for (i = 0; on_msg_commands[i].name; i++ )
     if (!my_stricmp(on_msg_commands[i].name, name))
       return on_msg_commands[i].userlevel > 0 ? on_msg_commands[i].userlevel : 0;
   return -1;
}

int set_access(name, level)
char *name;
int level;
{
	int i;
	for (i = 0; on_msg_commands[i].name; i++)
		if (!my_stricmp(on_msg_commands[i].name, name))
		{
			on_msg_commands[i].userlevel = level;
			return TRUE;
		}
	return FALSE;
}

int write_levelfile(filename)
char *filename;
{
        time_t    T;
        FILE   *list_file;
	int	i;

        if (!(list_file = fopen(filename, "w")))
                return FALSE;

        T = getthetime();

        fprintf( list_file, "###################################################
#########################\n" );
        fprintf( list_file, "## %s\n", filename);
        fprintf( list_file, "## Created: %s", ctime(&T) );
        fprintf( list_file, "## (c) 1995 CoMSTuD (cbehrens@slavery.com)\n" );
        fprintf( list_file, "###################################################
#########################\n" );

	for (i = 0; on_msg_commands[i].name; i++)
                fprintf( list_file, "%20s %3d\n", on_msg_commands[i].name,
			on_msg_commands[i].userlevel);
        fprintf(list_file, "## End of %s\n", filename);
        fclose(list_file);
        return TRUE;
}

int read_levelfile(filename)
char *filename;
{
  FILE *in;
  char lin[MAXLEN];
  char *ptr, *name, *level;

  if (!(in=fopen(filename, "rt")))
         return FALSE;
  while (freadln(in, lin))
  {
         ptr = lin;
         name = get_token(&ptr, ", ");
         level = get_token(&ptr, ", ");
  	 if (name && level)
		set_access(name, atoi(level));
  }
  fclose(in);
  return TRUE;
}


