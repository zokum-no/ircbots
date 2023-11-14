/*
 * d-ons.c - kinda like /on ^.. in ircII
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
#include "dweeb.h"
#include "d-ons.h"
#include "commands.h"

extern  botinfo *current;
extern  char owneruserhost[MAXLEN];

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
	{ "AUTH",       do_auth,         10,    FALSE,    FALSE, FALSE },
	{ "GLOBAL",     parse_global,    10,    FALSE,     TRUE, FALSE },
	{ "ACCESS",     do_access,       10,    FALSE,     TRUE, FALSE },
	{ "HELP",       do_help,         10,     TRUE,     TRUE, FALSE },
	{ "CHAT",       do_chat,         10,    FALSE,     TRUE, FALSE },
	{ "PING",       do_ping,         10,    FALSE,     TRUE, FALSE },
	{ "STATS",      do_stats,        10,     TRUE,     TRUE, FALSE },
	{ "INFO",       do_info,         10,     TRUE,     TRUE, FALSE },
	{ "VERSION",    do_version,      10,    FALSE,     TRUE, FALSE },
	{ "PASSWD",     do_passwd,       10,    FALSE,     TRUE, FALSE },
	{ "ROLLCALL", 	do_roll,	 10,	FALSE,	  FALSE, FALSE },
	{ "FUCK",	do_fuck,	 75,	FALSE,	   TRUE,  TRUE },
	{ "REVIEW",	do_review,	 90,	 TRUE,	   TRUE,  TRUE },
	{ "UP",         do_opme,         50,    FALSE,    FALSE,  TRUE },
	{ "CHANNELS",   do_channels,     50,     TRUE,     TRUE,  TRUE },
	{ "GETCH",      do_getch,        50,    FALSE,     TRUE,  TRUE },
	{ "CHANSTATS",  do_chanstats,    50,    FALSE,     TRUE,  TRUE },
	{ "B",          do_ban,          25,    FALSE,     TRUE,  TRUE },
	{ "UB",         do_unban,        25,    FALSE,     TRUE,  TRUE },
	{ "OP",         do_op,           90,    FALSE,     TRUE,  TRUE },
	{ "DEOP",       do_deop,         90,    FALSE,     TRUE,  TRUE },
	{ "K",          do_kick,         25,    FALSE,     TRUE,  TRUE },
	{ "KB",         do_kickban,      25,    FALSE,     TRUE,  TRUE },
	{ "INVITE",     do_invite,       25,    FALSE,     TRUE,  TRUE },
	{ "TOPIC",      do_topic,        90,    FALSE,     TRUE,  TRUE },
	{ "SB",         do_siteban,      75,    FALSE,     TRUE,  TRUE },
	{ "SKB",        do_sitekickban,  75,    FALSE,     TRUE,  TRUE },
	{ "MOPU",       do_massopu,      75,    FALSE,     TRUE,  TRUE },
	{ "MDEOPNU",    do_massdeopnu,   75,    FALSE,     TRUE,  TRUE },
	{ "BANLIST",    do_banlist,      50,     TRUE,     TRUE,  TRUE },
	{ "MUB",        do_massunban,    50,    FALSE,     TRUE,  TRUE },
	{ "MUBF",       do_massunbanfrom,50,    FALSE,     TRUE,  TRUE },
	{ "TOGTOP",     do_togtop,       75,    FALSE,     TRUE,  TRUE },
	{ "TOGBK",      do_togbk,        75,    FALSE,     TRUE,  TRUE },
	{ "TOGCK",      do_togck,        75,    FALSE,     TRUE,  TRUE },
	{ "TOGAOP",     do_togaop,       75,    FALSE,     TRUE,  TRUE },
	{ "TOGSHIT",    do_togshit,      75,    FALSE,     TRUE,  TRUE },
	{ "TOGPUB",     do_togpub,       75,    FALSE,     TRUE,  TRUE },
	{ "AOP",        do_aop,          90,    FALSE,     TRUE,  TRUE },
	{ "RAOP",       do_raop,         90,    FALSE,     TRUE,  TRUE },
	{ "SETBKL",     do_setbkl,       80,    FALSE,     TRUE,  TRUE },
	{ "SETCKL",     do_setckl,       80,    FALSE,     TRUE,  TRUE },
	{ "TOGPROT",    do_togprot,      90,    FALSE,     TRUE,  TRUE },
	{ "SAY",        do_say,          90,    FALSE,     TRUE,  TRUE },
	{ "ME",         do_me,           90,    FALSE,     TRUE,  TRUE },
	{ "CLVL",       do_clvl,         90,    FALSE,     TRUE,  TRUE },
	{ "MSG",        do_msg,          90,    FALSE,     TRUE,  TRUE },
	{ "JOIN",       do_join,         99,    FALSE,     TRUE,  TRUE },
	{ "CYCLE",      do_cycle,        50,    FALSE,     TRUE,  TRUE },
	{ "LEAVE",      do_leave,        75,    FALSE,     TRUE,  TRUE },
	{ "NICK",       do_nick,         99,    FALSE,     TRUE,  TRUE },
	{ "ADD",        do_add,          99,    FALSE,     TRUE,  TRUE },
	{ "DEL",        do_del,          99,    FALSE,     TRUE,  TRUE },
	{ "SHIT",       do_shit,         50,    FALSE,     TRUE,  TRUE },
	{ "RESTRICT",   do_restrict,     99,    FALSE,     TRUE,  TRUE },
	{ "LIMIT",      do_limit,        75,    FALSE,     TRUE,  TRUE },
	{ "RSHIT",      do_rshit,        50,    FALSE,     TRUE,  TRUE },
	{ "PROT",       do_prot,         90,    FALSE,     TRUE,  TRUE },
	{ "RPROT",      do_rprot,        90,    FALSE,     TRUE,  TRUE },
	{ "SHOWUSERS",  do_showusers,    75,     TRUE,     TRUE,  TRUE },
	{ "SHOWIDLE",   do_showidle,     75,     TRUE,     TRUE,  TRUE },
	{ "MKNU",       do_masskicknu,   75,    FALSE,     TRUE,  TRUE },
	{ "USERLIST",   do_userlist,     75,     TRUE,     TRUE,  TRUE },
	{ "SHITLIST",   do_shitlist,     75,     TRUE,     TRUE,  TRUE },
	{ "SETMDL",     do_setmdl,       75,    FALSE,     TRUE,  TRUE },
	{ "SETMBL",     do_setmbl,       75,    FALSE,     TRUE,  TRUE },
	{ "SETMKL",     do_setmkl,       75,    FALSE,     TRUE,  TRUE },
	{ "SETMPL",     do_setmpl,       75,    FALSE,     TRUE,  TRUE },
	{ "SETFL",      do_setfl,        75,    FALSE,     TRUE,  TRUE },
	{ "SETFPL",     do_setfpl,       75,    FALSE,     TRUE,  TRUE },
	{ "SETNCL",     do_setncl,       75,    FALSE,     TRUE,  TRUE },
	{ "SETIKT",     do_setikt,       75,    FALSE,     TRUE,  TRUE },
	{ "TOGIK",      do_togik,        75,    FALSE,     TRUE,  TRUE },
	{ "MOP",        do_massop,       99,    FALSE,     TRUE,  TRUE },
	{ "MDEOP",      do_massdeop,     99,    FALSE,     TRUE,  TRUE },
	{ "MK",         do_masskick,     75,    FALSE,     TRUE,  TRUE },
	{ "MKB",        do_masskickban,  99,    FALSE,     TRUE,  TRUE },
	{ "SETMAL",     do_setmal,       90,    FALSE,     TRUE,  TRUE },
	{ "TOGSD",      do_togsd,        90,    FALSE,     TRUE,  TRUE },
	{ "SETMAL",     do_setmal,       90,    FALSE,     TRUE,  TRUE },
	{ "TOGCC",      do_togcc,        90,    FALSE,     TRUE,  TRUE },
	{ "TOGENFM",    do_togenfm,      90,    FALSE,     TRUE,  TRUE },
	{ "TOGREET",	do_togreet,	 90,    FALSE,	   TRUE,  TRUE },
	{ "ENFMODES",   do_enfmodes,     75,    FALSE,     TRUE,  TRUE },
	{ "CMDCHAR",    do_cmdchar,     100,    FALSE,     TRUE,  TRUE },
	{ "SERVER",     do_server,       99,    FALSE,     TRUE,  TRUE },
	{ "NEXTSERVER", do_nextserver,   99,    FALSE,     TRUE,  TRUE },
	{ "BOMB",       do_bomb,        100,    FALSE,     TRUE,  TRUE },
	{ "TOGSO",      do_togso,        99,    FALSE,     TRUE,  TRUE },
	{ "LOAD",       do_load,         99,    FALSE,     TRUE,  TRUE },
	{ "SAVE",       do_save,         50,    FALSE,     TRUE,  TRUE },
	{ "RELOGIN",    do_relogin,      99,    FALSE,     TRUE,  TRUE },
	{ "SPAWN",      do_spawn,        99,    FALSE,     TRUE,  TRUE },
	{ "SETPASS",	do_setpass,	 99,	FALSE,	   TRUE,  TRUE },
	{ "DIE",        do_die,          99,    FALSE,     TRUE,  TRUE },
	/*
	 :
	 :
	 */
	{ NULL,         (void(*)())(NULL), 0,   FALSE,      FALSE }
};

void on_kick(from, channel, nick, reason)
char *from;
char *channel;
char *nick;
char *reason;
{
	send_spy(channel, "%s has been kicked off channel %s by %s (%s)",
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

void on_join(who, channel)
char *who;
char *channel;
{
	char *nick;
	
	send_spy(channel, "%s has joined channel %s",
		who, channel);
	check_bans();
	check_clonebots(who, channel);

	if (is_shitted(who, channel) && !is_user(who, channel))
	{
		shit_action(who, channel);
		return;
	}

	if (is_aop(who, channel) && get_aoptoggle(channel))
	  if (!password_needed(who) || verified(who))
	    if (get_userlevel(who, channel) >= current->restrict)
		giveop(channel, getnick(who));
}

void on_topic(from, channel, topic)
char *from;
char *channel;
char *topic;
{
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
  check_nickflood(who);  /* kicking taken care of in that function */
  send_common_spy(who, "%s is now known as %s", who, newnick);
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

	if (is_shitted(from, "*") && !max_userlevel(from))
	  return;

	if (check_memory(from) == IS_FLOODING)
		return;
	
	if ((command = get_token(&msg, " "))== NULL)
		return;                 /* NULL-command */

	if (ischannel(to) && !get_pubtoggle(to))
	  {
	    on_public(from_copy, to_copy, msg_copy);
	    return;
	  }

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
/*
				send_to_user(from, "\002%s\002", "Incorrect levels");
*/
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
			    if (on_msg_commands[i].needcc || !ischannel(to))
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

void on_mode(from, rest)
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
	send_spy(channel, "Mode change \"%s\" on channel %s by %s",
		 rest, channel, from);
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
				if (tempy && (get_userlevel(tempy, channel) < ASSTLEVEL)) 
				  if (username(getnick(tempy)) &&
				      (get_userlevel(username(getnick(tempy)),channel) < ASSTLEVEL))
					{
					 /* used to be those stupid revenge kicks */
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
	/*		check_bans(); /* kick users matching the ban */	
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


