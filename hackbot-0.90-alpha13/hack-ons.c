/*
 * hack-ons.c - kinda like /on ^.. in ircII
 * (c) 1994 CoMSTuD (cbehrens@iastate.edu)
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
/*
#include "ftext.h"
*/
#include "function.h"
#include "memory.h"
#include "userlist.h"
#include "hackbot.h"
#include "hack-ons.h"
#include "commands.h"

extern  botinfo *current;

/* external parseroutines ("prefix commands")   */
extern void parse_global(char *from, char *to, char *rest, int cmdlevel);

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
	{ "GLOBAL",     parse_global,     0,    FALSE,     TRUE,  TRUE },
	{ "ACCESS",     do_access,       -1,    FALSE,     TRUE, FALSE },
	{ "CHAT",       do_chat,          0,    FALSE,     TRUE, FALSE },
	{ "DOWN",       do_deopme,        0,    FALSE,    FALSE, FALSE },
	{ "FILES",      do_flist,         0,     TRUE,     TRUE, FALSE },
	{ "FLIST",      do_flist,         0,     TRUE,     TRUE, FALSE },
	{ "PING",       do_ping,          1,    FALSE,     TRUE, FALSE },
	{ "SEND",       do_send,          0,    FALSE,     TRUE, FALSE },
	{ "STATS",      do_stats,        -1,     TRUE,     TRUE, FALSE },
	{ "INFO",       do_info,          0,     TRUE,     TRUE, FALSE },
	{ "VERSION",    do_version,       0,    FALSE,     TRUE, FALSE },
	{ "PASSWD",     do_passwd,        1,    FALSE,     TRUE, FALSE },
	{ "UP",         do_opme,         20,    FALSE,    FALSE,  TRUE },
	{ "CHANNELS",   do_channels,     20,     TRUE,     TRUE,  TRUE },
	{ "GETCH",      do_getch,        20,    FALSE,     TRUE,  TRUE },
	{ "CHANSTATS",  do_chanstats,    20,    FALSE,     TRUE,  TRUE },
	{ "B",          do_ban,          30,    FALSE,     TRUE,  TRUE },
	{ "UB",         do_unban,        30,    FALSE,     TRUE,  TRUE },
	{ "OP",         do_op,           30,    FALSE,     TRUE,  TRUE },
	{ "DEOP",       do_deop,         30,    FALSE,     TRUE,  TRUE },
	{ "K",          do_kick,         30,    FALSE,     TRUE,  TRUE },
	{ "KB",         do_kickban,      40,    FALSE,     TRUE,  TRUE },
	{ "INVITE",     do_invite,       40,    FALSE,     TRUE,  TRUE },
	{ "TOPIC",      do_topic,        40,    FALSE,     TRUE,  TRUE },
	{ "SB",         do_siteban,      40,    FALSE,     TRUE,  TRUE },
	{ "SKB",        do_sitekickban,  40,    FALSE,     TRUE,  TRUE },
	{ "MOPU",       do_massopu,      50,    FALSE,     TRUE,  TRUE },
	{ "MDEOPNU",    do_massdeopnu,   50,    FALSE,     TRUE,  TRUE },
	{ "BANLIST",    do_banlist,      50,     TRUE,     TRUE,  TRUE },
	{ "MUB",        do_massunban,    50,    FALSE,     TRUE,  TRUE },
	{ "MUBF",       do_massunbanfrom,50,    FALSE,     TRUE,  TRUE },
	{ "SPYMSG",     do_spymsg,       70,    FALSE,     TRUE,  TRUE },
	{ "RSPYMSG",    do_rspymsg,      70,    FALSE,     TRUE,  TRUE },
	{ "SPY",        do_spy,          70,     TRUE,     TRUE,  TRUE },
	{ "RSPY",       do_rspy,         70,    FALSE,     TRUE,  TRUE },
	{ "TOGTOP",     do_togtop,       70,    FALSE,     TRUE,  TRUE },
	{ "TOGBK",      do_togbk,        70,    FALSE,     TRUE,  TRUE },
	{ "TOGCK",      do_togck,        70,    FALSE,     TRUE,  TRUE },
	{ "TOGAOP",     do_togaop,       70,    FALSE,     TRUE,  TRUE },
	{ "TOGSHIT",    do_togshit,      70,    FALSE,     TRUE,  TRUE },
	{ "TOGPUB",     do_togpub,       70,    FALSE,     TRUE,  TRUE },
	{ "AOP",        do_aop,          70,    FALSE,     TRUE,  TRUE },
	{ "RAOP",       do_raop,         70,    FALSE,     TRUE,  TRUE },
	{ "DCCLIST",    do_listdcc,      70,     TRUE,     TRUE,  TRUE },
	{ "SETBKL",     do_setbkl,       70,    FALSE,     TRUE,  TRUE },
		  { "SETCKL",     do_setckl,       70,    FALSE,     TRUE,  TRUE },
	{ "TOGPROT",    do_togprot,      80,    FALSE,     TRUE,  TRUE },
	{ "SAY",        do_say,          80,    FALSE,     TRUE,  TRUE },
	{ "ME",         do_me,           80,    FALSE,     TRUE,  TRUE },
	{ "CLVL",       do_clvl,         80,    FALSE,     TRUE,  TRUE },
	{ "MSG",        do_msg,          80,    FALSE,     TRUE,  TRUE },
	{ "JOIN",       do_join,         80,    FALSE,     TRUE,  TRUE },
	{ "CYCLE",      do_cycle,        80,    FALSE,     TRUE,  TRUE },
	{ "LEAVE",      do_leave,        80,    FALSE,     TRUE,  TRUE },
	{ "NICK",       do_nick,         80,    FALSE,     TRUE,  TRUE },
	{ "ADD",        do_add,          80,    FALSE,     TRUE,  TRUE },
	{ "DEL",        do_del,          80,    FALSE,     TRUE,  TRUE },
	{ "SHIT",       do_shit,         80,    FALSE,     TRUE,  TRUE },
	{ "RESTRICT",   do_restrict,     80,    FALSE,     TRUE,  TRUE },
	{ "LIMIT",      do_limit,        80,    FALSE,     TRUE,  TRUE },
	{ "RSHIT",      do_rshit,        80,    FALSE,     TRUE,  TRUE },
	{ "PROT",       do_prot,         80,    FALSE,     TRUE,  TRUE },
	{ "RPROT",      do_rprot,        80,    FALSE,     TRUE,  TRUE },
	{ "SHOWUSERS",  do_showusers,    80,     TRUE,     TRUE,  TRUE },
	{ "SHOWIDLE",   do_showidle,     80,     TRUE,     TRUE,  TRUE },
	{ "MKNU",       do_masskicknu,   80,    FALSE,     TRUE,  TRUE },
	{ "USERLIST",   do_userlist,     80,     TRUE,     TRUE,  TRUE },
	{ "SHITLIST",   do_shitlist,     80,     TRUE,     TRUE,  TRUE },
	{ "SETMDL",     do_setmdl,       80,    FALSE,     TRUE,  TRUE },
	{ "SETMBL",     do_setmbl,       80,    FALSE,     TRUE,  TRUE },
	{ "SETMKL",     do_setmkl,       80,    FALSE,     TRUE,  TRUE },
	{ "SETMPL",     do_setmpl,       80,    FALSE,     TRUE,  TRUE },
	{ "SETFL",      do_setfl,        80,    FALSE,     TRUE,  TRUE },
	{ "SETFPL",     do_setfpl,       80,    FALSE,     TRUE,  TRUE },
	{ "SETNCL",     do_setncl,       80,    FALSE,     TRUE,  TRUE },
	{ "SETIKT",     do_setikt,       80,    FALSE,     TRUE,  TRUE },
	{ "TOGIK",      do_togik,        80,    FALSE,     TRUE,  TRUE },
	{ "MOP",        do_massop,       85,    FALSE,     TRUE,  TRUE },
	{ "MDEOP",      do_massdeop,     85,    FALSE,     TRUE,  TRUE },
	{ "MK",         do_masskick,     85,    FALSE,     TRUE,  TRUE },
	{ "MKB",        do_masskickban,  85,    FALSE,     TRUE,  TRUE },
	{ "SETMAL",     do_setmal,       90,    FALSE,     TRUE,  TRUE },
	{ "USERHOST",   do_userhost,     90,    FALSE,     TRUE,  TRUE },
	{ "SPYLIST",    do_spylist,      90,    FALSE,     TRUE,  TRUE },
	{ "TOGSD",      do_togsd,        90,    FALSE,     TRUE,  TRUE },
	{ "SETMAL",     do_setmal,       90,    FALSE,     TRUE,  TRUE },
	{ "STEAL",      do_steal,        90,    FALSE,     TRUE,  TRUE },
	{ "RSTEAL",     do_rsteal,       90,    FALSE,     TRUE,  TRUE },
	{ "TOGCC",      do_togcc,        90,    FALSE,     TRUE,  TRUE },
	{ "TOGENFM",    do_togenfm,      90,    FALSE,     TRUE,  TRUE },
	{ "ENFMODES",   do_enfmodes,     90,    FALSE,     TRUE,  TRUE },
	{ "CMDCHAR",    do_cmdchar,      90,    FALSE,     TRUE,  TRUE },
	{ "SERVER",     do_server,       90,    FALSE,     TRUE,  TRUE },
	{ "NEXTSERVER", do_nextserver,   90,    FALSE,     TRUE,  TRUE },
	{ "BOMB",       do_bomb,         90,    FALSE,     TRUE,  TRUE },
/*
	{ "LAST10",     do_last10,       95,     TRUE,     TRUE,  TRUE },
*/
	{ "CHACCESS",   do_chaccess,     95,    FALSE,     TRUE,  TRUE },
	{ "TOGSO",      do_togso,        95,    FALSE,     TRUE,  TRUE },
	{ "EXEC",       do_exec,         99,   FALSE,      TRUE,  TRUE },
	{ "LOADUSERS",  do_loadusers,    99,    FALSE,     TRUE,  TRUE },
	{ "LOADSHIT",   do_loadshit,     99,    FALSE,     TRUE,  TRUE },
	{ "SAVEUSERS",  do_saveusers,    99,    FALSE,     TRUE,  TRUE },
	{ "SAVESHIT",   do_saveshit,     99,    FALSE,     TRUE,  TRUE },
	{ "DO",         do_do,           99,    FALSE,     TRUE,  TRUE },
	{ "SPAWN",      do_spawn,        95,    FALSE,     TRUE,  TRUE },
	{ "DIE",        do_die,          99,    FALSE,     TRUE,  TRUE },
	/*
	 :
	 :
	 */
	{ NULL,         (void(*)())(NULL), 0,   FALSE,      FALSE }
};

void on_kick(char *from, char *channel, char *nick, char *reason)
{
	send_spy(channel, "%s has been kicked off channel \002%s\002 by %s (%s)",
		nick, channel, getnick(from), reason);
	if (my_stricmp(getnick(from), current->nick))
	 if (check_masskick(from, channel))
		if (get_userlevel(from,channel) < ASSTLEVEL)
	mass_action(from, channel);
  if (!my_stricmp(current->nick, nick))
	 {
		set_kickedby(channel, from);
/*      strcpy(current->nick, random_str(3, 9));
		sendnick(current->nick); */
		join_channel(channel, "", TRUE);
	 }
  if (get_protlevel(username(nick), channel))
	 {
		prot_action(from, channel, username(nick));
		send_to_server("INVITE %s %s", nick, channel);
	 }
}

void on_join(char *who, char *channel)
{
	send_spy(channel, "%s has joined channel \002%s\002",
		who, channel);

	check_clonebots(who, channel);

	if (is_shitted(who, channel))
	{
		shit_action(who, channel);
		return;
	}

	if (is_aop(who, channel) && get_aoptoggle(channel))
	  if (!password_needed(who) || verified(who))
		giveop(channel, getnick(who));
}

void on_part(char *who, char *channel)
{
	send_spy(channel, "%s has left channel \002%s\002",
		who, channel);
}

void on_nick(char *who, char *newnick)
{
  check_nickflood(who);  /* kicking taken care of in that function */
}

void on_quit(char *who, char *reason)
{
}

void on_msg(char *from, char *to, char *msg)
{
	int      i;
	char     from_copy[MAXLEN];
	char     to_copy[MAXLEN];
	char     msg_copy[HUGE];
	char     *command, *tempcommand;
	DCC_list *userclient;


	msg[254] = '\0';
	if (from)
	  strcpy(from_copy, from);
	else
	  strcpy(from_copy, "");
	strcpy(to_copy, to);
	strcpy(msg_copy, msg);

	if (is_shitted(from, "*"))
	  return;

	if (check_memory(from) == IS_FLOODING)
		return;
	
	if ((command = get_token(&msg, " "))== NULL)
		return;                 /* NULL-command */

	userclient = search_list("chat", from, DCC_CHAT);

	for (i = 0; on_msg_commands[i].name; i++)
	  {
		 tempcommand = command;
		 if (current->cctoggle && on_msg_commands[i].needcc &&
				(*tempcommand != current->cmdchar))
			continue;
		 if (*tempcommand == current->cmdchar)
			tempcommand++;
		if (!my_stricmp(on_msg_commands[i].name, tempcommand))
		{
			if (max_userlevel(from) < on_msg_commands[i].userlevel)
			{
				send_to_user(from, "\002%s\002", "Incorrect levels");
				return;
			}
			if ((max_userlevel(from) < current->restrict) &&
				 (on_msg_commands[i].userlevel != -1))
			{
				send_to_user(from, "\002Sorry, I'm being restricted to at least level %i\002", current->restrict);
				return;
			}
			if (password_needed(from) &&
				 on_msg_commands[i].needpass && !verified(from))
			  {
				 send_to_user(from, "\002Use \"auth\" to get verified first\002");
				 return;
			  }
			if (on_msg_commands[i].forcedcc &&
			  (!userclient || (userclient->flags&DCC_WAIT)))
			{
				nodcc_session(from, to, msg_copy);
#ifdef AUTO_DCC
				dcc_chat(from, msg);
				sendnotice(getnick(from), "\002Please type: /dcc chat %s\002",
					current->nick);
#else
				sendnotice(getnick(from),
					"\002Sorry, %s is only available through DCC Chat\002",
					command);
				sendnotice(getnick(from),
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
	if (!ischannel(to))
	  {
		 if (*command == current->cmdchar)
			send_to_user(from, "\002%s\002", "Uh...huh huh...what???");
		 send_spymsg("\002*%s*\002 %s", getnick(from), msg_copy);
	  }
	else
	  {
		on_public(from_copy, to_copy, msg_copy);
	  }
}

void on_mode( from, rest )
char *from;
char *rest;
{
	int     did_change, enfm, doh;
	char    *channel, *mode, *chanchars, *params, sign;

	char    unminmode[MAXLEN];      /* used to undo modechanges */
	char    unminparams[MAXLEN];
	char    unplusmode[MAXLEN];     /* used to undo modechanges */
	char    unplusparams[MAXLEN];
	SLS     *dummy; 
	char    *nick;
	char    *banstring;
	char    *key;
	char    *limit;

#ifdef DBUG
		  debug(NOTICE, "Entered mode, from = %s, rest = %s\n", from, rest);
#endif

	did_change = FALSE;
	enfm = 0;
	strcpy(unminmode, "");          /* min-modes, i.e. -bi */
	strcpy(unminparams, "");        /* it's parameters */
	strcpy(unplusmode, "");         /* plus-modes, i.e. +oo */
	strcpy(unplusparams, "");       /* parameters */


	channel = get_token(&rest, " ");
	if (!my_stricmp(channel, current->nick))
		 return;                 /* mode NederServ +i etc */
	if (!mychannel(channel))
	  return;
	chanchars = get_token(&rest, " ");
	params = rest;
	update_idletime(from, channel);
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
			 if (!username(nick))
				{
			send_to_server("WHO %s", channel);
			break;
				}
			 if( sign == '+' )
				{
			add_channelmode(from, channel, CHFL_CHANOP, nick, getthetime() );
			if (!my_stricmp(nick, current->nick))
			  {
				 char *tempy;
				 strcpy(current->nick,current->realnick);
				 sendnick(current->nick);
				 if (is_in_list(current->StealList, channel))
					{
				int temp = current->malevel;
				current->malevel = 99;
				channel_massdeop(channel, "*!*@*");
				current->malevel = temp;
				remove_from_list(current->StealList, channel);
					}
				 check_shit(); /* kick shitters */
				tempy = get_kickedby(channel);             
				if (tempy && (get_userlevel(channel, tempy) < ASSTLEVEL)) 
					if (username(getnick(tempy)) &&
						(get_userlevel(channel, username(getnick(tempy))) < ASSTLEVEL))
					{
					  sendkick(channel, getnick(tempy), "\002%s\002", "Doh! This is revenge fucker!");
					  set_kickedby(channel, NULL);
					}
			  }
			if (get_shitlevel(username(nick), channel))
			  {
				 strcat(unminmode, "o");
				 strcat(unminparams, nick);
				 strcat(unminparams, " ");
				 did_change=TRUE;
			  }
			else if (!strchr(from, '@') &&
				 !is_user(username(nick), channel) &&
				 get_sdtoggle(channel))
			  {
				 strcat(unminmode, "o");
				 strcat(unminparams, nick);   /* Net-split op */
				 strcat(unminparams, " ");
				 did_change=TRUE;
			  }
			else if (get_sotoggle(channel) &&
				 !is_user(username(nick), channel) &&
				 (get_userlevel(from, channel) < ASSTLEVEL) &&
				 (get_userlevel(username(nick), channel) < 30))
			  {
				 strcat(unminmode, "o");
				 strcat(unminparams, nick);
				 strcat(unminparams, " ");
				 did_change = TRUE;
			  }
				}
			 else
				{
			del_channelmode( channel, CHFL_CHANOP, nick );
			if (my_stricmp(current->nick, getnick(from)))
			  if (check_massdeop(from, channel))
				 if (get_userlevel(from, channel) < ASSTLEVEL)
					mass_action(from, channel);
			if (get_protlevel(username(nick), channel) &&
				 !get_shitlevel(username(nick), channel))
			  {
				 if (my_stricmp(getnick(from), current->nick) &&
				my_stricmp(getnick(from), nick))
					{
				strcat(unplusmode, "o");
				strcat(unplusparams, nick);
				strcat(unplusparams, " ");
				prot_action(from, channel, nick);
				did_change=TRUE;
					}
			  }
				}
			 break;
		  case 'v':
			 nick = get_token(&params, " ");
			 if (sign == '+')
				add_channelmode(from, channel, CHFL_VOICE, nick, getthetime() );
			 else
				del_channelmode( channel, CHFL_VOICE, nick );
			 break;
		  case 'b':
			 banstring = get_token(&params, " ");
			 if (sign == '+')
				{
			add_channelmode(from, channel, MODE_BAN, banstring,
					getthetime());
			
			if (my_stricmp(getnick(from), current->nick))
				 {
					if (check_massban(from, channel))
				if (get_userlevel(from, channel) < ASSTLEVEL)
				  mass_action(from, channel);
				  if (get_protlevel2(banstring, channel) &&
						!get_shitlevel2(banstring, channel))
				  {
					 strcat(unminmode, "b");
					 strcat(unminparams, banstring);
					 strcat(unminparams, " ");
					 prot_action(from, channel, banstring);
					 did_change=TRUE;
				  }
				 }
			  }
			  else
			  {
				 del_channelmode(channel, MODE_BAN, banstring);
				 
				 dummy = NULL;
				 if (!get_shittoggle(channel))
					continue;
				 if (((doh=get_shitlevel2(banstring, channel)) >= 3)
				|| (dummy = is_in_shitlist(
					current->ShitList, banstring,
								channel)))
					{
				char *temp;

				temp = NULL;
				if (doh >= 3)
				  temp = get_shituh2(banstring, channel);
				if (dummy && (dummy->access >= 3))
				  temp = dummy->userhost;
				if (temp)
				  {
					 strcat(unplusmode, "b");
					 strcat(unplusparams, temp);
					 strcat(unplusparams, " ");
					 did_change=TRUE;
				  }
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
			  {
				  add_channelmode(from, channel, MODE_KEY, key ? key : "???", getthetime());
			  }
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
  TS *Temp;

  strcpy(thehost, "");
  if (gethost(from)) strcpy(thehost, gethost(from));
  Temp = is_in_timelist(current->OffendersList, thehost);
  if (!Temp)
  {
	add_to_timelist(current->OffendersList, thehost);
	Temp = is_in_timelist(current->OffendersList, thehost);
	if (Temp->num)
	  Temp->num = 0;
  }
  update_idletime(from, to);
  level = numchar(rest, '\007');  
  if (level)
	if (check_beepkick(from, to, level))
	{
		if (Temp)
		{
			Temp->num++;
			if (Temp->num == 2)
				sendmode(to, "+b %s", format_uh(from, 1));
			else if (Temp->num > 2)
			{
				char *temp;
				temp = format_uh(from, 2);
				channel_massunban(to, temp, 0);
				sendmode(to, "+b %s", temp);
			}
		}
		sendkick(to, getnick(from), "\002%s\002", "Umm...I think we can hear you");
	}
  level = percent_caps(rest);
  if (level >= 50)
	if (check_capskick(from, to, 1))
	{
		if (Temp)
		{
			Temp->num++;
			if (Temp->num == 2)
				sendmode(to, "+b %s", format_uh(from, 1));
			else if (Temp->num > 2)
			{
				char *temp;
				temp = format_uh(from, 2);
				channel_massunban(to, temp, 0);
				sendmode(to, "+b %s", temp);
			}
		}
		sendkick(to, getnick(from), "\002%s\002", "I CAN SEE YOU D00D, SO STOP!!!");
	}
  send_spy(to, "\002<%s:%s>\002 %s", getnick(from), to, rest);
  if ((level = check_pubflooding(from, to)) && (!get_protlevel(from, to)))
	 {
		if (Temp)
		{
			Temp->num++;
			if (Temp->num >= 4)
			{
				add_to_shitlist(current->ShitList, format_uh(from,1), 2,
						to, "Auto-Shit", 
					 "Bah...flooding and or beeping/CAPSing", getthetime());
				/* auto shitlist */
			}
		}
		if (level >= 2)
		  deop_ban(to, format_uh(from, 1));
		if (level >= 1)
		  sendkick(to, getnick(from), "\002%s\002", "Get out you flooder!"); 
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
			




