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
#ifndef SYSV
#include <strings.h>
#endif
#include <ctype.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>

#include "channel.h"
#include "config.h"
#include "dcc.h"
#include "log.h"
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
#include "crc-ons.h"
#ifdef BOTNET
#include "botnet.h"
#endif
#include "tb-ons.h"
#include "incu-ons.h"
#include "fnmatch.h"
#ifdef BackUp
#include "mega-ons.h"
extern int Abackup;
#endif
#ifdef BackedUp
extern int Abackedup;
#endif

extern	botinfo	*currentbot;
extern	int		number_of_bots;
extern	int		rehash;
extern	int		sent_ban;
extern	long	uptime;
extern	char	*botmaintainer;
/* external parseroutines ("prefix commands")	*/
extern	void	parse_global(char *from, char *to, char *rest);

int	onchannel;
int   addressed_to_me;
int   public_command;
int	NICK=0;
char	kicker[70];

command_tbl on_msg_commands[] =

/* Any command which produces more than 2 lines output can be
	used to make the bot "excess flood". Make it "forcedcc"     */

/*	Command				function   		  userlvl shitlvl  forcedcc  forceauth */
{
/**************** User utilities **************/
	{ "SEEN",			do_seen,					  0,	  0,	FALSE,		FALSE },
	{ "SHOWSEEN",		do_showseen,			 20,	  0,	TRUE,			FALSE },
	{ "CMDHISTORY",	show_cmdhistory,		 50,	  0,	TRUE,			FALSE },
	{ "ONE",				do_one,					 75,	  0,	FALSE,		FALSE },
	{ "SHOWACCESS",	do_showaccess,			 20,	  0,	TRUE,			FALSE },
	{ "CMDACCESS",		do_showcmdaccess,		 20,	  0,	FALSE,		FALSE },
#ifdef TB_MSG
	{ "ADDMSG",			do_addmsg,				 50,	  0,	FALSE,		FALSE },
	{ "SHOWMSGS",		do_showmsgs,			 50,	  0,	TRUE,			FALSE },
	{ "SHOWALLMSGS",	do_showallmsgs,		 50,	  0,	TRUE,			FALSE },
	{ "CHECKMSGS",		do_checkmsgs,			 50,	  0,	FALSE,		FALSE },
#endif
	{ "MYEMAIL",		do_myemail,				 50,	  0,	FALSE,		TRUE  },
	{ "SHOWEMAIL",		do_showemail,			 50,	  0,	FALSE,		TRUE  },
	{ "EMAILLIST",		show_email_list,		 50,	  0,	TRUE,			TRUE  },
	{ "MYNICK",			do_mynick,				 50,	  0,	FALSE,		TRUE  },
	{ "USERINFO",		show_userinfo,			 50,	  0,	FALSE,		TRUE  },
	{ "NUSERINFO",		show_nuserinfo,		 50,	  0,	FALSE,		TRUE  },
	{ "BUY",				do_buy,					 50,	  0,	FALSE,		FALSE },
	{ "HELP",			show_help,				  0,	  0,	FALSE,		FALSE },
	{ "WHOAMI",			show_whoami,			 50,	  0,	FALSE,		FALSE },
	{ "INFO",			show_info,				 50,	  0,	FALSE,		FALSE },
	{ "TIME",			show_time,				 50,	  0,	FALSE,		FALSE },
	{ "LISTDCC",		do_listdcc,				 50,	  0,	TRUE,			FALSE },
/* Some more ftp-dcc functions				*/
	{ "CD",				do_cd,					  0,	  0,	FALSE,		FALSE },
	{ "CHAT",			do_chat,					 50,	  0,	FALSE,		FALSE },
	{ "FILES",			do_flist,				  0,	  0,	TRUE,			FALSE },
	{ "FLIST",			do_flist,				  0,	  0,	TRUE,			FALSE },
	{ "GET",				do_send,					  0,	  0,	FALSE,		FALSE },
	{ "LS",				show_dir,				  0,	  0,	TRUE,			FALSE },
	{ "MGET",			do_send,					  0,	  0,	FALSE,		FALSE },
	{ "PWD",				show_cwd,				  0,	  0,	FALSE ,		FALSE },
	{ "QUEUE",			show_queue,				  0,	  0,	TRUE,			FALSE },
	{ "SEND",			do_send,					  0,	  0,	FALSE,		FALSE },
	{ "SENDBOTLIST",	do_send_list,			150,	  0,	FALSE,		TRUE	},
/* userlevel and dcc are dealt with within GLOBAL 	*/
	{ "GLOBAL",			parse_global,			  0,	100,	FALSE,		TRUE  },
	{ "OPEN",			do_open,					100,	  0,	FALSE,		TRUE	},
	{ "WHOIS",			show_whois,				 50,	  0,	FALSE,		FALSE },
	{ "NWHOIS",			show_nwhois,			 50,	  0,	FALSE,		FALSE },
	{ "SHOWUSERS",		do_showusers,			125,	  0,	TRUE,			FALSE },
	{ "NSHOWAUTH",		show_authed,			 50,	  0,	FALSE,		FALSE },
/******************** List Commands *****************/
/* Prot list commands */
	{ "NPROTADD",		do_nprotadd,			100,	  0,	FALSE,		TRUE  },
	{ "NPROTDEL",		do_nprotdel,			100,	  0,	FALSE,		TRUE  },
	{ "PROTADD",		do_protadd,				125,	  0,	FALSE,		TRUE  },
	{ "PROTDEL",		do_protdel,				100,	  0,	FALSE,		TRUE  },
	{ "PROTLIST",		show_protlist,			 50,	  0,	TRUE,			FALSE },
	{ "PROTWRITE",		do_protwrite,			100,	  0,	FALSE,		FALSE },
/* User list commands */
	{ "NUSERADD",		do_nuseradd,			100,	  0,	FALSE,		TRUE  },
	{ "NHUSERADD",		do_nhuseradd,			100,	  0,	FALSE,		TRUE  },
	{ "NUSERDEL",		do_nuserdel,			100,	  0,	FALSE,		TRUE  },
	{ "USERADD",		do_useradd,				125,	  0,	FALSE,		TRUE  },
	{ "USERDEL",		do_userdel,				100,	  0,	FALSE,		TRUE  },
	{ "CHUSERLEVEL",	do_chuserlevel,		100,	  0,	FALSE,		TRUE  },
	{ "NCHUSERLEVEL",	do_nchuserlevel,		100,	  0,	FALSE,		TRUE  },
	{ "CHUSERMASK",	do_chusermask, 		125,	  0,	FALSE,		TRUE  },
	{ "USERLIST",		show_userlist,			 50,	  0,	TRUE,			FALSE },
	{ "USERWRITE",		do_userwrite,			100,	  0,	FALSE,		FALSE },
/* Exclude list commands */
	{ "EXADD",			do_excludeadd,			100,	  0,	FALSE,		TRUE  },
	{ "EXDEL",			do_excludedel,			100,	  0,	FALSE,		TRUE  },
	{ "EXLIST",			show_exclude_list,	 50,	  0,	TRUE,			FALSE },
	{ "EXWHOIS",		show_whois_excluded,	 50,	  0,	FALSE,		FALSE },
	{ "EXWRITE",		do_excludewrite,		100,	  0,	FALSE,		FALSE },
	{ "EXUNBAN",		do_exunban,				  0,	150,	FALSE,		FALSE },
/* Expire list commands */
	{ "EXPIREADD",		do_expireadd,			100,	  0,	FALSE,		TRUE  },
	{ "EXPIREDEL",		do_expiredel,			100,	  0,	FALSE,		TRUE  },
	{ "EXPIREWRITE",	do_expirewrite,		100,	  0,	FALSE,		FALSE },
	{ "EXPIRELIST",	show_expire_list,	    50,	  0,	TRUE,			FALSE },
/* Shit list commands */
	{ "NPEWPADD",		do_nshitadd,			100,	  0,	FALSE,		TRUE  },
	{ "NSHITADD",		do_nshitadd,			100,	  0,	FALSE,		TRUE  },
	{ "SHITWRITE",		do_shitwrite,			100,	  0,	FALSE,		FALSE },
	{ "PEWPADD",		do_shitadd,				125,	  0,	FALSE,		TRUE  },
	{ "PEWPDEL",		do_shitdel,				100,	  0,	FALSE,		TRUE  },
	{ "SHITADD",		do_shitadd,				125,	  0,	FALSE,		TRUE  },
	{ "SHITDEL",		do_shitdel,				100,	  0,	FALSE,		TRUE  },
	{ "SHITLIST",		show_shitlist,		 	 50,	  0,	TRUE,			FALSE },
	{ "WHYSHITTED",	do_whyshitted,		 	 75,	  0,	FALSE,		TRUE  },
	{ "WHOSHITTED",	do_whoshitted,		 	 75,	  0,	FALSE,		TRUE  },
	{ "WHENSHITTED",	do_whenshitted,		 75,	  0,	FALSE,		TRUE  },
	{ "CHSHITLEVEL",	do_chshitlevel,		125,	  0,	FALSE,		TRUE  },
	{ "CHWHOSHITTED",	do_chwhoshitted,		125,	  0,	FALSE,		TRUE  },
	{ "CHWHENSHITTED",do_chwhenshitted,		125,	  0,	FALSE,		TRUE  },
	{ "CHSHITREASON",	do_chshitreason,		125,	  0,	FALSE,		TRUE  },
	{ "CHSHITMASK",	do_chshitmask,		 	125,	  0,	FALSE,		TRUE  },
/* Password list commands */
	{ "CHPASSWD",		do_chpasswd,		 	 20,	  0,	FALSE,		FALSE },
	{ "MAILPASSWD",	do_mailpasswd,			125,	  0,	FALSE,		TRUE  },
	{ "NLOCK",			do_nlock,				125,	  0,	FALSE,		TRUE  },
	{ "NSHOWPASSWD",	do_nshowpasswd,		125,	  0,	FALSE,		TRUE  },
	{ "PASSWD",			do_passwd,		 		 50,	  0,	FALSE,		FALSE },
	{ "PASSWDADD",		do_passwdadd,			125,	  0,	FALSE,		TRUE  },
	{ "PASSWDDEL",		do_passwddel,			125,	  0,	FALSE,		TRUE  },
	{ "PASSWDLIST",	show_password_list,	125,	  0,	TRUE,			FALSE },
	{ "PASSWDOP",		do_passwdop,		 	 50,	  0,	FALSE,		FALSE },
	{ "PASSWDWRITE",	do_passwdwrite,		100,	  0,	FALSE,		FALSE },
	{ "SHOWLOCKED",	do_showlocked,			125,	  0,	TRUE,			TRUE  },
	{ "SHOWPASSWD",	do_showpasswd,			125,	  0,	FALSE,		TRUE  },
	{ "UNLOCK",			do_unlock,				125,	  0,	FALSE,		TRUE  },
/* Priviliged commands */
	{ "ACCESSWRITE",	do_accesswrite,		150,	  0,	FALSE,		TRUE  },
	{ "FPWRITE",		do_fpwrite,				150,	  0,	FALSE,		TRUE  },
	{ "BWALLOP",		do_bwallop,				150,	  0,	FALSE,		FALSE },
	{ "CHACCESS",		do_chaccess,			125,	  0,	FALSE,		TRUE  },
	{ "EMAILADD",		do_emailadd,			125,	  0,	FALSE,		TRUE  },
	{ "FP",				do_floodprot,			125,	  0,	FALSE,		TRUE  },
	{ "MAILEVENTS",	do_mail_list_events,	150,	  0,	FALSE,		TRUE  },
	{ "NICKADD",		do_nickadd,			 	125,	  0,	FALSE,		TRUE  },
	{ "LOGLEVEL",		do_loglevel,		 	150,	  0,	FALSE,		TRUE  },
	{ "READACCESS",	do_readaccess,		 	125,	  0,	FALSE,		FALSE },
	{ "READFP",			do_readfp,			 	125,	  0,	FALSE,		FALSE },
	{ "SHOWHACKS",		show_nethacks,		 	100,	  0,	TRUE,			TRUE  },
	{ "SPY",				do_cspy,					150,	  0,	TRUE,			TRUE  },
	{ "SPYLOG",			do_cspylog,			 	150,	  0,	FALSE,		TRUE  },
	{ "SECURE",			do_secure,			 	150,	  0,	FALSE,		TRUE  },
	{ "LISTWRITE",		write_all_lists,		100,	  0,	FALSE,		TRUE  },
/******************* Bot Control ******************/
	{ "CYCLE",			do_cycle,			 	125,	  0,	FALSE,		TRUE  },
	{ "DO",				do_do,				 	150,	  0,	FALSE,		TRUE  },
	{ "DIE",				do_die,				 	150,	  0,	FALSE,		TRUE  },
	{ "DIEDIE",			do_die,				 	150,	  0,	FALSE,		TRUE  },
	{ "FORK",			do_fork,				 	150,	  0,	FALSE,		TRUE  },
	{ "JOIN",			do_join,			 	 	125,	  0,	FALSE,		TRUE  },
	{ "LEAVE",			do_leave,			 	125,	  0,	FALSE,		TRUE  },
	{ "NICK",			do_nick,				 	125,	  0,	FALSE,		FALSE },
	{ "NICKCH",			do_nickch,			 	100,	  0,	FALSE,		FALSE },
	{ "QUIT",			do_quit,				 	150,	  0,	FALSE,		TRUE  },
	{ "REHASH",			do_rehash,			 	150,	  0,	FALSE,		TRUE  },
	{ "SERVER",			do_server,			 	125,	  0,	FALSE,		TRUE  },
	{ "SERVERLIST",	show_serverlist,	 	125,	  0,	TRUE,			TRUE  },
/* Bot Personality */
	{ "DESC",			do_describe,		 	100,	  0,	FALSE,		FALSE },
	{ "ME",				do_action,			 	100,	  0,	FALSE,		FALSE },
	{ "SAY",				do_say,				 	100,	  0,	FALSE,		FALSE },
	{ "TALK",			do_ctalk,			 	125,	  0,	FALSE,		FALSE },
/* Bot channel commands */
	{ "AWP",				do_op,			 	 	 50,	  0,	FALSE,		TRUE  },
	{ "BANUSER",		do_banuser,			 	100,	  0,	FALSE,		TRUE  },
	{ "BEWT",			do_bk,				 	100,	  0,	FALSE,		TRUE  },
	{ "BK",				do_bk,				 	100,	  0,	FALSE,		TRUE  },
	{ "TBK",				do_timer_bk,		 	100,	  0,	FALSE,		TRUE  },
	{ "SHOWBANS",		do_showbans,		 	100,	  0,	TRUE,			FALSE },
	{ "CHANNELS",		show_channels,		 	125,	  0,	TRUE,			FALSE },
	{ "DEOP",			do_deop,		 		 	 50,	  0,	FALSE,		TRUE  },
	{ "GIVEOP",			do_giveop,			 	100,	  0,	FALSE,		TRUE  },
	{ "INVITE",			do_invite,		 	  	 50,	  0,	FALSE,		FALSE },
	{ "KICK",			do_kick,				 	100,	  0,	FALSE,		TRUE  },
	{ "MASSDEOP",		do_massdeop,		 	125,	  0,	FALSE,		TRUE  },
	{ "MASSKICK",		do_masskick,		 	 75,	  0,	FALSE,		TRUE  },
	{ "MASSNK",			do_massnk,		 	 	 75,	  0,	FALSE,		TRUE  },
	{ "MASSOP",			do_massop,			 	125,	  0,	FALSE,		TRUE  },
	{ "MASSUNBAN",		do_massunban,		 	 50,	  0,	FALSE,		FALSE },
	{ "OP",				do_op,			 	 	 50,	  0,	FALSE,		TRUE  },
	{ "UNBAN",			do_unban,			 	100,	  0,	FALSE,		TRUE  },
	{ "WALL",			do_wallop,			 	100,	  0,	FALSE,		TRUE  },
	{ "CHANST",			do_chanst,			 	 50,	  0,	FALSE,		FALSE },
	{ "VOICE",			do_voice,			 	 20,	  0,	FALSE,		FALSE },
	{ "DEVOICE",		do_devoice,			 	 20,	  0,	FALSE,		FALSE },
#ifdef BackUp
	{ "COMM",			do_hotlink,		  		  0,	  0,	FALSE,		FALSE },
#endif
	/*
	 :
	 :
	 */
	{ NULL,				null(void(*)()), 		  0,	  0,	FALSE,		FALSE }
};




void	on_kick (char *from, char *channel, char *nick, char *reason)
{
	/*
	 * If user A kicks user B then kick user A if B was protected
	 * and A has lower protlevel
	 */
	/*
	 * Perhaps vladbot is getting to much warbot with this piece
	 * of code. Maybe add an #ifdef...
	 */

	strcpy(kicker,from);

	/* I should never revenge myself :) */
	if (STRCASEEQUAL(currentbot->nick, getnick(from)) ||
		userlevel(from)==OTHER_BOT_LEVEL)
		return;

	if(shitlevel(username(nick)) == 0 &&
		protlevel(username(nick)) > 0 &&
		lvlcheck(from,username(nick)))
		if(userlevel(from) < userlevel(username(nick)) ||
			(checkauth(nick) || usermode(channel,nick)&MODE_CHANOP))
			sendkick(channel, getnick(from), "What a freaking moron!");
}

void 	on_join(char *who, char *channel)
{
	int	excludelvl;
	int	shitlvl;
	int	userlvl;
	char	joinnick[MAXLEN];
#ifdef BOOTLAMENICKS
	char	blah[256];
#endif /* BOOTLAMENICKS */

	excludelvl = excludelevel(who);
	shitlvl = shitlevel(who);
	userlvl = userlevel(who);
	strcpy(joinnick, getnick(who));


	if(excludelvl==0)
	{
		if(shitlvl == 101)
		{
			switch (rand() % 7)
			{
				case 0:  send_ctcp(channel,"ACTION spots %s sneaking around like his usual llama self", joinnick); break;
				case 1:  send_ctcp(channel,"ACTION tears up %s with a nail gun and launches his carcass outta %s",joinnick,channel); break;
				case 2:  send_ctcp(channel,"ACTION summons his Ogre pals to join him in an old fashioned %s-beating",joinnick); break;
				case 3:  send_ctcp(channel,"ACTION and his pals beat the living shit out of %s and throw his bloody carcass outta %s",joinnick,channel); break;
				case 4:  send_ctcp(channel,"ACTION notices that %s slipped through the %s llama-filter(tm)",joinnick,channel); break;
				case 5:  send_ctcp(channel,"ACTION grabs %s by the yossarians and launches him in writhing pain outta %s",joinnick,channel); break;
				default: send_ctcp(channel,"ACTION urinates all over %s and throws him out of %s like a piece of trash",joinnick,channel); break;
			}
		}
		if(shitlvl >= 100)
		{
			if (shitmask(who)==NULL)
				ban_user(who, channel);
			else
				ban(shitmask(who),channel);

			if (usershitreason(who)!=NULL)
				sendkick(channel, joinnick, usershitreason(who));
			else
				sendkick(channel, joinnick, "You're not wanted here");

			globallog(3, DONELOG, "%s was kicked because he was shitlisted", who);
			return;

		}
	}

	if (currentbot->floodprot_clone && userlvl < MIN_USERLEVEL_FOR_OPS)
		check_channel_clones(who,channel);

#ifdef BOOTLAMENICKS
	if (STRCASEEQUAL(joinnick,"mode") ||
		STRCASEEQUAL(joinnick,"irc") ||
		STRCASEEQUAL(joinnick,"["))
	{
		sprintf(blah, "%s!*@*", joinnick);
		addban(blah, channel, joinnick);
		sendkick(channel,joinnick,"That nick is too leet [llama-kick]");
	}
#endif
#ifdef TB_MSG
	if (currentbot->talk>3)
		Check_Msgs(who);
#endif

	if (STRCASEEQUAL(currentbot->nick,joinnick))
	{
		if (currentbot->talk>2)
		{
			switch(rand()%6)
			{
				case 0: sendprivmsg(channel,"sup people"); break;
				case 1: sendprivmsg(channel,"yo people"); break;
				case 2: sendprivmsg(channel,"whattup homies"); break;
				case 3: sendprivmsg(channel,"holas"); break;
				case 4: sendprivmsg(channel,"re all"); break;
				default: sendprivmsg(channel,"your mastah hath returned"); break;
			}
		}
	}

	if(currentbot->talk>3 && userlvl == 150)
		sendprivmsg(channel, "Greetings master %s",joinnick);

	if((shitlvl == 0 || excludelvl>0) &&
		( (currentbot->secure==0 && userlvl >= AUTO_OPLVL) ||
		(currentbot->secure==3 && userlvl==OTHER_BOT_LEVEL) ) )
	{
		if (!(usermode(channel,joinnick)&MODE_CHANOP))
			giveop(channel, joinnick);
	}

	globallog(4, JOINLOG, "%s joined %s", who, channel);
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
	int	userlvl;
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
			userlvl = userlevel(username(nick));
			if(sign == '+')
			{
			    if (strstr(from,"!")==NULL)
			    {
				if (userlvl >= MIN_USERLEVEL_FOR_OPS)
				    return;

				if (userlvl==25 && currentbot->l_25_flags&FL_CANHAVEOPS)
				    return;

				if (currentbot->secure==1)
				    return;

				if (STRCASEEQUAL(nick,currentbot->nick))
				    return;

				sendmode(channel, "-o %s",nick);
				REMOVEMODE("o", nick);

				if (currentbot->talk>1)
				    sendprivmsg(channel, "damn nethacks!!");

				if (usermode(channel,currentbot->nick)&MODE_CHANOP)
				    globallog(3, DONELOG, 
				    "%s nethacked ops from %s and was deopped", username(nick), from);
				else
				    globallog(3, 
				    DONELOG, "%s nethacked ops from %s and I'm not opped", username(nick), from);
			    }
			    else
			    {
				/* if the dude's already an op, lets bail */
				if (usermode(channel,nick)&MODE_CHANOP)
					break;

				add_channelmode(channel, CHFL_CHANOP, nick);

				if (STRCASEEQUAL(getnick(from),currentbot->nick))
				    return;
				if (userlevel(from)==OTHER_BOT_LEVEL && currentbot->secure>3)
				    return;

				/* why is this shit never indented right ??  
				   hello! McFly! use set sw=4  or else move to emacs
				   now to fix this punknuggeting  expression 
				   ldw - 5/9/1999
				*/
				/*	(userlvl == 25 && currentbot->l_25_flags&FL_CANHAVEOPS != 0))) || */

				if 
				(
				    (shitlevel(username(nick)) > 0 && excludelevel(username(nick))==0) ||
				    (
					(currentbot->secure != 1) && 
					!(
					    userlvl >= MIN_USERLEVEL_FOR_OPS ||
					    (userlvl == 25 && (currentbot->l_25_flags&FL_CANHAVEOPS) != 0)
					)
				    ) ||
				    (
					(currentbot->secure > 3) && 
					! (checkauth(username(nick))) 
				    )
				)
				{
				    REMOVEMODE("o", nick);
				    send_to_user(from,"What a moron!! Don't op peasants!!");
				    globallog(2, DONELOG, "illegal ops for %s by %s", username(nick), from);
				    did_change=TRUE;
				}
			    }
			}
			else
			{
			    if(!STRCASEEQUAL(getnick(from), currentbot->nick) &&
				userlevel(from)!=OTHER_BOT_LEVEL && (protlevel(username(nick))>0) &&
				(shitlevel(username(nick))==0 || excludelevel(username(nick))>0) &&
				lvlcheck(from,username(nick)))
				if(usermode(channel,nick)&MODE_CHANOP)
				{
				    ADDMODE("o", nick);
				    send_to_user(from, "%s is protected!", nick);
				    globallog(3, DONELOG, "%s reopped after %s deopped a protected user", nick, from);
				    did_change=TRUE;
				}

			    del_channelmode(channel, CHFL_CHANOP, nick);

			    /* if it was me that was deopped, reop from other bots */
			    if (STRCASEEQUAL(nick,currentbot->nick))
			    {
				reop_after_deop(channel);
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
			    if(STRCASEEQUAL(getnick(from),currentbot->nick))
				    sent_ban=0;

			    add_channelmode(channel, MODE_BAN, banstring);
			    if(currentbot->secure && userlevel(from) < MIN_USERLEVEL_FOR_BANS)
			    {
				REMOVEMODE("b", banstring);
				did_change = TRUE;
				send_to_user(from,"You are not permitted to do bans here!");
				globallog(3, DONELOG, "illegal ban %s by %s removed", banstring, from);
			    }
			    else
				if(
				  (find_highest(channel, banstring, 1) >= 100 && 
				   userlevel(from) != 105 &&
				   !STRCASEEQUAL(getnick(from),currentbot->nick)) ||
				  (find_highest(channel, banstring, 2) >= 100)
				)
				{
				    REMOVEMODE("b", banstring);
				    did_change = TRUE;
				}
				else
				    check_channel_bans(channel);
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
			{
			    add_channelmode(channel, MODE_MODERATED, "");
			    if (!strstr(from,"!") && strcmp(from,currentbot->serverlist[currentbot->current_server].realname))
			    {
				REMOVEMODE("m", "");
				did_change = TRUE;
			    }
			}
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
			{
			    add_channelmode(channel, MODE_INVITEONLY, "");
			    if (!strstr(from,"!") && strcmp(from,currentbot->serverlist[currentbot->current_server].realname))
			    {
				REMOVEMODE("i", "");
				did_change = TRUE;
			    }
			}
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

			    add_channelmode(channel, MODE_KEY,key?key:"???");

			    if (!strstr(from,"!") && strcmp(from,currentbot->serverlist[currentbot->current_server].realname))
			    {
				REMOVEMODE("k", key);
				did_change = TRUE;
			    }

			    /* try to unset bogus keys */
			    for(s = key; key && *s; s++)
				if(*s < ' ')
				{
				    REMOVEMODE("k", key);
				    did_change = TRUE;
				    send_to_user(from,"No bogus keys pls");
				    globallog(3, DONELOG, "boguskey %s from %s removed", key, from);
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
			    add_channelmode(channel, MODE_LIMIT,limit?limit:"0");

			    if (!strstr(from,"!") && strcmp(from,currentbot->serverlist[currentbot->current_server].realname))
			    {
				REMOVEMODE("l", limit?limit:"0");
				did_change = TRUE;
			    }
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
	    sendmode( channel, "+%s-%s %s %s", unplusmode,unminmode, unplusparams, unminparams);
}

void	on_msg(char *from, char *to, char *msg)
{
	int	i;
	int	userlvl;
	char	msg_copy[MAXLEN],msg2[MAXLEN];	/* for session */
	char	*command,*ptr;
	char	*nick;
	char	From[80];
	DCC_list	*userclient;

	if (currentbot->Spyuser_list != NULL)
		do_spy(from,to,msg);

	onchannel=FALSE;
	strcpy(msg_copy, msg);
	strcpy(msg2, msg);
	for (i=0;i<strlen(msg2);i++) msg2[i]=tolower(msg2[i]);
	strcpy(From,from);
	ptr=strstr(From,"!");
	*ptr = '\0';

	if(STRCASEEQUAL(currentbot->nick, to))
		public_command = FALSE;
	else
		public_command = TRUE;

	if((command = get_token(&msg, ",: "))== NULL)
		return;			/* NULL-command */

	if(STRCASEEQUAL(currentbot->nick, command))
	{
		if((command = get_token(&msg, ",: "))==NULL)
			return;		/* NULL-command */

		addressed_to_me = TRUE;
	}
	else
		addressed_to_me = FALSE;


	if((*command != PREFIX_CHAR) && !STRCASEEQUAL(to, currentbot->nick))
	{
		if (currentbot->floodprot_public &&
			check_channel_flood(from,to,1))
				return;

		nick = strdup(currentbot->nick);

		for (i=0;i<strlen(nick);i++)
			nick[i]=tolower(nick[i]);

		if (currentbot->talk==2 && strstr(msg2,nick))
			do_talk(From,msg2,0);

		if (currentbot->talk>2)
			do_crctalk(from,to,msg2,nick);

		free(nick);

		return; /* The command should start with PREFIX_CHAR if public */
	}

	if(*command == PREFIX_CHAR)
		command++;

	userlvl = userlevel(from);

	/*
	 * Now we got
	 * - A public command.
	 * - A private command/message.
	 * Time to do a flood check :).
	 */
	if(userlvl!=OTHER_BOT_LEVEL &&
		check_session(from,to) == IS_FLOODING)
	{
		return;
	}

	if (currentbot->spylog)
		do_spylog(from,to,msg_copy);

	if (strcmp(getnick(from),currentbot->nick)!=0)
	{
#ifdef BOTNET
	if (*command == BOTNET_CHAR)
	{
		command++;

		if(on_botnet_command(command,from,to,msg,msg_copy))
			return;
	}
#endif
	for(i = 0; on_msg_commands[i].name != NULL; i++)
		if(STRCASEEQUAL(on_msg_commands[i].name, command))
		{
			if(userlvl < currentbot->commandlvl[i])
			{
				if ((strcasecmp(command,"op")==0 && currentbot->floodprot_op!=0) ||
					(strcasecmp(command,"awp")==0 && currentbot->floodprot_awp!=0))
				{
					if(userchannel(getnick(from)) != NULL && ischannel(to) &&
						(userlvl < MIN_USERLEVEL_FOR_OPS &&
						!(userlvl == 25 && currentbot->l_25_flags&FL_NOOPKICK)))
						floodevent(to,from,currentbot->floodprot_op,"Op begging bastard!");
					else
						if(userlvl >= MIN_USERLEVEL_FOR_OPS)
							send_to_user(from, "Userlevel too low");
				}
				return;
			}

			if(excludelevel(from)==0 && shitlevel(from) > on_msg_commands[i].shitlevel)
			{
				/*send_to_user(from, "Shitlevel too high");*/
				return;
			}

			if (!checkauth(from) && on_msg_commands[i].forceauth == TRUE)
			{
				send_to_user(from,"You're not authenticated!");
				return;
			}

			userclient = search_list("chat", from, DCC_CHAT);

			if(on_msg_commands[i].forcedcc &&
				  (!userclient || (userclient->flags&DCC_WAIT)))
			{
#ifdef AUTO_DCC
				dcc_chat(from, msg);
				nodcc_session(from, to, msg_copy);

/*				if (userclient && !(userclient->flags&DCC_WAIT))
					sendnotice(getnick(from), "Please type: /dcc chat %s",
						currentbot->nick);
*/
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
#ifdef BOTNET
		if(on_botnet_command(command,from,to,msg,msg_copy))
			return;
#endif
	}

	/* If the command was private, let the user know how stupid (s)he is */
	if(currentbot->talk>1 && STRCASEEQUAL(to, currentbot->nick))
		do_talk(From,msg2,1);
	else
		onchannel=TRUE;
/*		send_to_user(from, "%s %s?!? What's that?",
				  command, msg ? msg : "");*/
}


void	show_help(char *from, char *to, char *rest)
{
	FILE	*f;
	char	*s;
	char	org_msg[MAXLEN];
	DCC_list	*userclient;

	if(public_command == TRUE && addressed_to_me == FALSE)
	{
		sendreply("That command must be addressed to me if used in public.");
		return;
	}

	userclient = search_list("chat", from, DCC_CHAT);

	if(!userclient || (userclient->flags&DCC_WAIT))
	{
#ifdef AUTO_DCC
		dcc_chat(from, rest);
		sprintf(org_msg, "%s !help %s", currentbot->nick, rest?rest:"");
		nodcc_session(from, to, org_msg);

#else
		sendnotice( getnick(from),
			"Please start a dcc chat connection first" );
#endif /* AUTO_DCC */
		return;
	}

	if((f=fopen(currentbot->helpfile, "r"))==NULL)
	{
		send_to_user(from, "Helpfile missing");
		globallog(0, ERRFILE, "couldn't find helpfile to help %s", from);
		return;
	}

	if(rest == NULL)
	{
		find_topic(f, "standard");
		while((s = get_ftext(f, "standard")))
			send_to_user(from, s);
	}
	else
		if(!find_topic( f, rest ))
			send_to_user(from, "No help available for \"%s\"", rest);
		else
			while((s = get_ftext(f, rest)))
				send_to_user(from, s);
	fclose(f);
}

void	show_whoami(char *from, char *to, char *rest)
{
	if (userlevel(from)==0)
		send_to_user(from, "USERLIST: You're not a user");
	else
		show_lvllist( "USERLIST: ", currentbot->lists->opperlist, from, from, 5, checkauth(from));

	if (protlevel(from)==0)
		send_to_user(from, "PROTLIST: You're not protected");
	else
		show_lvllist( "PROTLIST: ", currentbot->lists->protlist, from, from, 2, 0);

	show_lvllist( "PASSLIST: ", currentbot->lists->passwdlist, from, from, 4, 0);
	show_lvllist( "SHITLIST: ", currentbot->lists->shitlist, from, from, 2, 0);
	show_excludelist( "EXCLUDED: ", currentbot->lists->excludelist, from, from);
	return;
}

void	show_info(char *from, char *to, char *rest)
{
	sendreply("%s (%s)", VERSION, currentbot->botname);
	sendreply("Started: %-20.20s", time2str(currentbot->uptime));
	sendreply("Up: %s", idle2str(time(NULL)-currentbot->uptime));
	if(botmaintainer)
		sendreply("%s is my owner", botmaintainer);
        return;

	globallog(3, DONELOG, "showed info to %s", from);
}

void	show_time(char *from, char *to, char *rest)
{
	if(rest)
	{
		if(STRCASEEQUAL(rest, "-RAW"))
			sendreply("Current time: %ld", time(NULL));
		else
			send_to_user(from,"Nice try dickhead");
	}
	else
		sendreply("Current time: %s", time2str(time(NULL)));

}

void	show_userlist(char *from, char *to, char *rest)
{
	if(!rest)
	{
		send_to_user(from, "%s User List", currentbot->nick);
		send_to_user(from, "-------+-----------------------------------------------+--------------");
		send_to_user(from, " Level | %-45s | Nickname", "nick!user@host");
		send_to_user(from, "-------+-----------------------------------------------+--------------");
		show_lvllist("   ", currentbot->lists->opperlist, from, rest?rest:"",1, 0);
		send_to_user(from, "End of User List");
	}
	else
		show_userlist2(currentbot->lists->opperlist, from, to, rest);
}

void	show_shitlist(char *from, char *to, char *rest)
{
	if(!rest)
	{
		send_to_user(from, "%s Shit List", currentbot->nick);
		send_to_user(from, "-------+--------------------------------------------------------------");
		send_to_user(from, " Level | %s", "nick!user@host");
		send_to_user(from, "-------+--------------------------------------------------------------");
		show_lvllist("   ", currentbot->lists->shitlist, from, rest?rest:"",2, 0);
		send_to_user(from, "End of Shit List");
	}
	else
		show_shitlist2(currentbot->lists->shitlist, currentbot->lists->shitlist2, from, to, rest);
}

void    show_protlist(char *from, char *to, char *rest)
{
	send_to_user(from, "%s Prot List", currentbot->nick);
	send_to_user(from, "-------+--------------------------------------------------------------");
	send_to_user(from, " Level | %s", "nick!user@host");
	send_to_user(from, "-------+--------------------------------------------------------------");
	show_lvllist("   ", currentbot->lists->protlist, from, rest?rest:"",2, 0);
	send_to_user(from, "End of Prot List");
}

void	do_op(char *from, char *to, char *rest)
{
	char	*opchan;
	int   allocated = 0;

	if (!userchannel(getnick(from)))
	{
		send_to_user(from, "You're not on channel fool!");
		return;
	}
	if ((opchan=get_token(&rest," "))==NULL)
	{
		if (STRCASEEQUAL(to, currentbot->nick))
			opchan = userchannel(getnick(from));
		else
		{
			opchan = strdup(to);
			allocated = 1;
		}
	}
	if (search_chan(opchan)==NULL)
	{
		send_to_user(from, "I'm not in that channel freako!");
		if(allocated)
			free(opchan);
		return;
	}

	if(!(usermode(opchan, currentbot->nick)&MODE_CHANOP))
		send_to_user(from, "I'm not opped on %s!", opchan);
	else
		if(!(usermode(opchan, getnick(from))&MODE_CHANOP))
			giveop(opchan, getnick(from));

	if(allocated)
		free(opchan);
}

void	do_giveop(char *from, char *to, char *rest)
{
	int	i = 0, numfailednicks = 0;
	char	nickname[WAYTOBIG];
	char	op[WAYTOBIG];
	char	orgrest[WAYTOBIG];
	char	failednicks[WAYTOBIG];

	strcpy(op, "");
	strcpy(failednicks, "");

	if(not(rest))
	{
		sendreply("Who do you want me to op?");
		return;
	}
	strcpy(orgrest,rest);
	while(readnick(&rest, nickname))
		if(username(nickname) != NULL &&	(shitlevel(username(nickname))==0 ||
			excludelevel(username(nickname))>0)	&& (currentbot->secure==1
			|| (userlevel(username(nickname))>=MIN_USERLEVEL_FOR_OPS ||
			(userlevel(username(nickname)) == 25 && currentbot->l_25_flags&FL_CANHAVEOPS))))
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
			globallog(3, DONELOG, "%s made me op %s", from, nickname);
		}
		else
		{
			numfailednicks++;
			strcat(failednicks, " ");
			strcat(failednicks, nickname);
		}

	if(i != 0)
		sendmode(ischannel(to) ?to :currentchannel(), "+ooo %s", op);

	if(numfailednicks)
		send_to_user(from, "Could not op:%s", failednicks);

	add_to_cmdhist(from, ischannel(to)?to:currentchannel(), "!giveop %s", orgrest);
}

void	do_deop(char *from, char *to, char *rest)
{
	int	i = 0, numfailednicks = 0;
	char	nickname[WAYTOBIG];
	char	deop[WAYTOBIG];
	char	orgrest[WAYTOBIG];
	char	failednicks[WAYTOBIG];

	strcpy(deop, "");
	strcpy(failednicks, "");

	if(not(rest))
	{
		sendreply("Who do you want me to deop?");
		return;
	}
	strcpy(orgrest,rest);
	while(readnick(&rest, nickname))
		if(username(nickname) != NULL &&
		  (userlevel(username(nickname)) < userlevel(from) ||
			shitlevel(username(nickname)) > 0))
		{
			i++;
			strcat(deop, " ");
			strcat(deop, nickname);
			if(i==3)
			{
				sendmode(ischannel(to) ?to :currentchannel(), "-ooo %s", deop);
				i = 0;
				globallog(3, DONELOG, "%s made me deop %s", from, nickname);
				strcpy(deop, "");
			}
		}
		else
		{
			numfailednicks++;
			strcat(failednicks, " ");
			strcat(failednicks, nickname);
		}

	if(i != 0)
		sendmode(ischannel(to) ?to :currentchannel(), "-ooo %s", deop);

	if(numfailednicks)
		send_to_user(from, "Could not deop:%s", failednicks);

	add_to_cmdhist(from, ischannel(to)?to:currentchannel(), "!deop %s", orgrest);
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
			send_to_user(from, "Channel [%s] could not be opened!", rest);
		else
		{
			channel_unban(rest, from);
			add_to_cmdhist(from, rest, "!open");
		}
	}
	else if(ischannel(to))
	{
		if(!open_channel(to))
			send_to_user(from, "Channel [%s] could not be opened!", to);
		else
		{
			channel_unban(to, from);
			add_to_cmdhist(from, to, "!open");
		}
	}
	else
	{
		open_channel(currentchannel());
		channel_unban(currentchannel(), from);
		add_to_cmdhist(from, currentchannel(), "!open");
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

	if (rest && strstr(rest,"* * ")!=NULL)
	{
		botlog(SYSTEMLOG,"%s tried to flood me off with send",from);

		if (search_list("chat",from,DCC_CHAT))
			send_to_user(from, "Lame attempt");
		return;
	}

	if(rest)
		while((pattern = get_token(&rest, " ")))
			send_file(from, pattern);
	else if (search_list("chat",from,DCC_CHAT))
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

	while((s=get_ftext(ls_file, "")))
		send_to_user(from, s);
	fclose(ls_file);
}	

void	do_say(char *from, char *to, char *rest)
{
	char *blah, *saychan, temp[2];
	int i;

	if (userlevel(from)==OTHER_BOT_LEVEL)
		return;

	if (!rest)
	{
					send_to_user(from, "I don't know what to say");
		return;
	}

	if (rest[0]==1 || rest[0]=='!')
	{
		send_to_user(from,"Screw off man");
		send_ctcp(userchannel(getnick(from)),"ACTION grabs %s and stares in his eyes and says don\'t even think about it",getnick(from));
		return;
	}

	temp[0] = 3;
	temp[1] = 0;
	if(strstr(rest, temp))
	{
		send_ctcp(userchannel(getnick(from)),"ACTION shoves some mIRC colors up %s\'s ass",getnick(from));
		return;
	}

	for (i=0;i<(strlen(rest)-1);i++)
	{
		if (rest[i]==' ' && (rest[i+1]=='!' || rest[i+1]==1))
		{
			send_to_user(from,"Screw off man");
			send_ctcp(userchannel(getnick(from)),"ACTION grabs %s and stares in his eyes and says don\'t even think about it",getnick(from));
			return;
		}
	}

	blah = get_token(&rest," ");

	if (ischannel(blah))
	{
		if (search_chan(blah)==NULL)
		{
			send_to_user(from, "I'm not in that channel freako!");
			return;
		}

		sendprivmsg(blah, "%s", rest);
	}
	else
	{
		if (STRCASEEQUAL(to, currentbot->nick))
			saychan = strdup(currentchannel());
		else
			saychan = strdup(to);

		sendprivmsg(saychan, "%s %s", blah, rest);
		free(saychan);
	}

}

void	do_do(char *from, char *to, char *rest)
{
	if(rest)
	{
		send_to_server("%s", rest);
		globallog(0, SYSTEMLOG, "did %s for %s", rest, from);
	}
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
	CHAN_list	*dummy;
	char			*chan;

	if((chan = get_token(&rest,", ")) != NULL)
	{
		do
		{
			if(isvalidchan(chan))
			{
				if((dummy = search_chan(chan)) == NULL)
				{
					join_channel(chan, "", "", TRUE);
					globallog(0, SYSTEMLOG, "%s made me join %s", from, chan);
				}
				else
				{
					if(dummy->active == FALSE || dummy->joined == FALSE)
					{
						sendjoin(dummy->name);
					}
					else
					{
						send_to_user(from, "I'm already on that channel");
					}
				}
			}
			else
				send_to_user(from, "%s is an invalid channel", chan);
		} while ((chan = get_token(&rest,", ")) != NULL);
	}
	else
		send_to_user(from, "What channel do you want me to join?");
	return;
}

void	do_leave(char *from, char *to, char *rest)
{
	if(public_command == TRUE && addressed_to_me == FALSE)
	{
		sendreply("That command must be addressed to me if used in public.");
		return;
	}
	if(rest)
	{
		leave_channel(rest); 
		globallog(0, SYSTEMLOG, "%s made me leave %s", from, rest);
	}
	else if(STRCASEEQUAL(currentbot->nick, to))
	{
		leave_channel(currentchannel());
		globallog(0, SYSTEMLOG, "%s made me leave %s", from, currentchannel());
	}
	else
	{
		leave_channel(to);
		globallog(0, SYSTEMLOG, "%s made me leave %s", from, to);
	}

	return;
}

void	do_nick(char *from, char *to, char *rest)
{
	if(rest)
	{
		if(public_command && !addressed_to_me)
		{
			sendreply("That command must be addressed to me if used in public.");
			return;
		}

		if(!isnick(rest))
		{
			sendreply("Illegal nickname %s", rest);
			return;
		}

		strncpy(currentbot->nick, rest, NICKLEN);
		strncpy(currentbot->realnick, rest, NICKLEN);
		sendnick(currentbot->nick);
		globallog(0, SYSTEMLOG, "%s made me change my nick to %s", from, rest);
	}
	else
		send_to_user(from, "You need to tell me what nick to use");

	return;
}

void	do_die(char *from, char *to, char *rest)
{
	if(public_command == TRUE && addressed_to_me == FALSE)
	{
		sendreply("That command must be addressed to me if used in public.");
		return;
	}

	if(rest != NULL)
	{
		signoff(from, rest);
	}
	else
		signoff( from, "Leaving" );

	globallog(0, SYSTEMLOG, "%s made me die", from);
	if(number_of_bots == 0)
		exit(0);
}

void	do_quit(char *from, char *to, char *rest)
{
	if(public_command == TRUE && addressed_to_me == FALSE)
	{
		sendreply("That command must be addressed to me if used in public.");
		return;
	}

	quit_all_bots(from, "Quiting all bots.");
	globallog(0, SYSTEMLOG, "%s made me quit", from);
	exit(0);
}

void    show_whois(char *from, char *to, char *rest)
{
	char	org_msg[MAXLEN];
	DCC_list	*userclient;
	int num_matches = 0;

	if(rest == NULL)
	{
		send_to_user(from, "Please specify a user mask");
		return;
	}

	userclient = search_list("chat", from, DCC_CHAT);

	if(!userclient || (userclient->flags&DCC_WAIT))
	{
		num_matches += num_userlist_matches(currentbot->lists->opperlist, rest, 2);
		num_matches += num_userlist_matches(currentbot->lists->protlist, rest, 2);
		num_matches += num_userlist_matches(currentbot->lists->passwdlist, rest, 2);
		num_matches += num_userlist_matches(currentbot->lists->shitlist, rest, 2);
		num_matches += num_excludelist_matches (currentbot->lists->excludelist, rest);
		num_matches += num_expirelist_matches (currentbot->lists->expirelist, rest);

		if(num_matches > 6)
		{
#ifdef AUTO_DCC
			dcc_chat(from, rest);
			sprintf(org_msg, "!whois %s", rest);
			nodcc_session(from, to, org_msg);

#else
			sendnotice( getnick(from),
				"Sorry, \"whois %s\" is only available through DCC",	rest );
			sendnotice( getnick(from),
				"Please start a dcc chat connection first" );
#endif /* AUTO_DCC */
			return;
		}
	}

	if (!show_lvllist_pattern( "USERLIST: ", currentbot->lists->opperlist, from, rest, 0))
		send_to_user(from, "USERLIST: %s is not a user", rest);

	if (!show_lvllist_pattern( "PROTLIST: ", currentbot->lists->protlist, from, rest, 2))
		send_to_user(from, "PROTLIST: %s is not protected", rest);

	if (!show_lvllist_pattern( "PASSLIST: ", currentbot->lists->passwdlist, from, rest, 4))
		send_to_user(from, "PASSLIST: %s has no password", rest);

	show_lvllist_pattern( "SHITLIST: ", currentbot->lists->shitlist, from, rest, 2);
	show_expirelist_pattern( "EXPIRELIST: ", currentbot->lists->expirelist, from, rest);
	show_excludelist_pattern( "EXCLUDED:", currentbot->lists->excludelist, from, rest);
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

	if (userlevel(nuh)==0)
		send_to_user(from, "USERLIST: %s is not a user", rest);
	else
	{
		show_lvllist( "USERLIST: ", currentbot->lists->opperlist, from, nuh, 5, checkauth(rest));
	}

	if (protlevel(nuh)==0)
		send_to_user(from, "PROTLIST: %s is not protected", rest);
	else
		show_lvllist( "PROTLIST: ", currentbot->lists->protlist, from, nuh,2, 0);

	show_lvllist("PASSLIST: ", currentbot->lists->passwdlist, from, nuh, 4, 0);
	show_lvllist( "SHITLIST: ", currentbot->lists->shitlist, from, nuh, 2, 0);
	show_expirelist_pattern( "EXPIRELIST: ", currentbot->lists->expirelist, from, rest);
	show_excludelist( "EXCLUDED: ", currentbot->lists->excludelist, from, nuh);
}

void	do_nuseradd(char *from, char *to, char *rest)
{
	char	*newuser;
	char	*newlevel;
	char	*mailaddr;
	char	*nick;
	char	*nuh;		/* nick!@user@host */
	char	passwd[6];
	char	userstr[MAXLEN];
	int	already;
	int	i;
	int	hosttype = 0;

	if (userlevel(from)==OTHER_BOT_LEVEL)
		return;

	if((newuser = get_token(&rest, " ")) == NULL)
	{
		sendreply("Who do you want me to add?" );
		return;
	}

	if(STRCASEEQUAL(newuser, "-fullhost"))
		hosttype = 1;

	if(STRCASEEQUAL(newuser, "-minhost"))
		hosttype = 2;

	if(hosttype)
		if((newuser = get_token(&rest, " ")) == NULL)
		{
			sendreply("Who do you want me to add?" );
			return;
		}

	if((nuh=username(newuser))==NULL)
	{
		sendreply("%s is not on this channel!", newuser);
		return;
	}

	if((newlevel = get_token(&rest, " ")) == NULL)
	{
		sendreply("What level should %s have?", newuser );
		return;
	}

	if(!checklevel(atoi(newlevel)))
	{
		sendreply("Invalid level: %s", newlevel);
		return;
	}
	if(atoi(newlevel) > userlevel(from) && userlevel(from) < 150)
	{
		sendreply("Sorry bro, can't do that!" );
		return;
	}
	if(userlevel(from) < userlevel(nuh) && userlevel(from) < 150)
	{
		sendreply("Sorry, %s has a higher level", newuser);
		return;
	}

	if ((mailaddr = get_token(&rest, " ")) == NULL)
		mailaddr = "None";
	else if (!check_email(mailaddr) && !STRCASEEQUAL(mailaddr, "None"))
	{
		send_to_user(from,"Invalid email address: %s", mailaddr);
		return;
	}

	if ((nick = get_token(&rest, " ")) == NULL)
		nick = newuser;
	else if (!check_nick(nick))
	{
		send_to_user(from,"Invalid nickname");
		return;
	}

	strcpy(userstr,get_add_mask(nuh,hosttype));
	already=userlevel(userstr);

	if (already==atoi(newlevel))
	{
		sendreply("%s is already a level %s user fool!",newuser,newlevel);
		return;
	}

	if (!add_to_levellist( currentbot->lists->opperlist, userstr,
		atoi(newlevel), mailaddr, nick))
	{
		sendreply("There is already a record that matches %s", userstr);
		return;
	}

#ifdef TB_MSG
	AddUser(nuh,atoi(newlevel));
#endif
	sendreply("User %s added with access %d as %s with email %s nick %s",
		newuser, atoi(newlevel), userstr, mailaddr, nick);

#ifdef BackedUp
	if (Abackedup)
		send_to_user(currentbot->backupnick,
			"!comm %s useradd %s %d %s %s",
			currentbot->backupbotpasswd,userstr,
			atoi(newlevel), mailaddr, nick);
#endif

	if(atoi(newlevel) >= 50)
	{
		for (i=0;i<5;i++)
			passwd[i]=rand()%26+65;
	
		passwd[5]='\0';
	
		if (already==0)
		{
			send_to_user(newuser,
				"Your password is: %s, to use it type /msg %s passwd %s",
				passwd, currentbot->nick,passwd);
			send_to_user(newuser,
				"To change your password type /msg %s chpasswd %s <new password>",
				currentbot->nick,passwd);
	
			add_to_passwdlist(currentbot->lists->passwdlist,userstr, passwd);


#ifdef BackedUp
		if (Abackedup)
			send_to_user(currentbot->backupnick, "!comm %s passwdadd %s %s",
				currentbot->backupbotpasswd,userstr,passwd);
#endif
		}
	}
	
		notify_useradd(newuser, userstr, newlevel, from);
		globallog(2, DONELOG, "%s added as %s with level %d email %s nick %s by %s",
			newuser, userstr, atoi(newlevel), mailaddr, nick, from);
}                   

void	do_useradd(char *from, char *to, char *rest)
{
	char	*newuser;
	char	*newlevel;
	char	*mailaddr;
	char	*nick;
	char	passwd[6];
	int	already;
	int 	i;

	if (userlevel(from)==OTHER_BOT_LEVEL)
		return;

	if((newuser = get_token(&rest, " ")) == NULL)
	{
             	send_to_user( from, "Who do you want me to add?" );
		return;
	}

	if((newlevel = get_token(&rest, " ")) == NULL)
	{
		send_to_user( from, "What level should %s have?", newuser );
		return;
	}

	if(!checklevel(atoi(newlevel)))
	{
		sendreply("Invalid level: %s", newlevel);
		return;
	}

	if(atoi(newlevel) > userlevel(from) && userlevel(from) < 150)
	{
		send_to_user(from, "Sorry bro, can't do that!");
		return;
	}

	if(userlevel(from) < userlevel(newuser) && userlevel(from) < 150)
	{
		send_to_user(from, "Sorry, %s has a higher level", newuser);
		return;
	}

	already=userlevel(newuser);

	if (already==atoi(newlevel))
	{
		sendreply("%s is already a level %s user fool!",newuser,newlevel);
		return;
	}

	if ((mailaddr = get_token(&rest, " ")) == NULL)
		mailaddr = "None";
	else if (!check_email(mailaddr) && !STRCASEEQUAL(mailaddr, "None"))
	{
		send_to_user(from,"Invalid email address");
		return;
	}

	if ((nick = get_token(&rest, " ")) == NULL)
		nick = "None";
	else if (!check_nick(nick))
	{
		send_to_user(from,"Invalid nickname");
		return;
	}

	if (!add_to_levellist(currentbot->lists->opperlist, newuser, atoi(newlevel), mailaddr, nick))
	{
		sendreply("There is already a record that matches %s", newuser);
		return;
	}

	send_to_user( from, "User %s added with access %d email %s nick %s",newuser, atoi(newlevel), mailaddr, nick);
#ifdef TB_MSG
	AddUser2(newuser,atoi(newlevel));
#endif

#ifdef BackedUp
	if (Abackedup)
		send_to_user(currentbot->backupnick,"!comm %s useradd %s %d %s %s",
			currentbot->backupbotpasswd,newuser,atoi(newlevel),mailaddr,nick);
#endif

	if(atoi(newlevel) >= 50)
	{
		for (i=0;i<5;i++)
			passwd[i]=rand()%26+65;

		passwd[5]='\0';

		if (already==0)
		{
			send_to_user(from,"Password is set to: %s",passwd);
			add_to_passwdlist(currentbot->lists->passwdlist, newuser, passwd);

#ifdef BackedUp
			if (Abackedup)
				send_to_user(currentbot->backupnick,
					"!comm %s passwdadd %s %s",
					currentbot->backupbotpasswd,newuser,passwd);
#endif
		}
	}

	globallog(2, DONELOG, "%s added with level %d email %s nick %s by %s",
		newuser, atoi(newlevel), mailaddr, nick, from);
	notify_useradd(newuser, newuser, newlevel, from);
}                   

void    do_userwrite(char *from, char *to, char *rest)
{
	if(!write_lvllist(currentbot->lists->opperlist, currentbot->lists->opperfile))
	{
		send_to_user(from, "Userlist could not be written to file %s",
			currentbot->lists->opperfile);

#ifdef BackedUp
		if (Abackedup)
			send_to_user(currentbot->backupnick,"!comm %s userwrite",
				currentbot->backupbotpasswd);
#endif
	}
	else
		send_to_user(from, "Userlist written to file %s", currentbot->lists->opperfile);

	do_passwdwrite(from,to,rest);

#ifdef BackedUp
       		if (Abackedup)
	         send_to_user(currentbot->backupnick,"!comm %s passwdwrite",
                        currentbot->backupbotpasswd);
#endif
}

void	do_userdel(char *from, char *to, char *rest)
{
	int	inlist=0;

	if (userlevel(from)==OTHER_BOT_LEVEL)
		return;

	if(!rest)
	{
		send_to_user(from, "Who do you want me to delete?");
		return;
	}

	if (userlevel(from) < userlevel(rest) && userlevel(from) < 150)
	{
		sendreply("%s has a higer level.. sorry", rest);
		return;
	}

	if (remove_from_levellist(currentbot->lists->passwdlist,rest))
		inlist++;
	else
		sendreply("%s has no password!", rest);

	if (remove_from_levellist(currentbot->lists->opperlist,rest))
	{
		sendreply("User %s has been deleted", rest);
		inlist++;
	}
	else
		sendreply("%s has no level!",rest);

	if (remove_from_levellist(currentbot->lists->protlist,rest))
	{
		sendreply("User %s has been unprotected", rest);
		inlist++;
	}

	globallog(2, DONELOG, "%s deleted by %s",rest, from);

#ifdef BackedUp
	if (Abackedup)
		send_to_user(currentbot->backupnick,"!comm %s userdel %s",
			currentbot->backupbotpasswd,rest);
#endif
#ifdef TB_MSG
	DelUser(from);
#endif

	if (inlist)
		notify_userdel(rest,from);
}

void	do_nshitadd(char *from, char *to, char *rest)
{
	char	*newuser;
	char	*newlevel;
	char	*nuh;		/* nick!@user@host */
	char	userstr[MAXLEN];
	char  m[300],m2[100];
	char	*expirestr;
	char	temp[2];
	int	level;
	int	doexpire = 0;
	long	expiretime;

	if (userlevel(from)==OTHER_BOT_LEVEL) return;

	if((newuser = get_token(&rest, " ")) == NULL)
	{
		sendreply("Who do you want me to add?");
		return;
	}

	if(STRCASEEQUAL(newuser, "-expire"))
	{
		doexpire = 1;

		if((expirestr = get_token(&rest, " ")) == NULL)
		{
			sendreply("How long should they be shitted?");
			return;
		}

		if(!isnumeric(expirestr))
		{
			send_to_user(from, "%s is an invalid expire time", expirestr);
			return;
		}

		sscanf(expirestr, "%ld", &expiretime);

		if(!expiretime)
		{
			sendreply("Expire time should be > 0" );
			return;
		}

		expiretime *= 86400;
		expiretime += time(NULL);

		if((newuser = get_token(&rest, " ")) == NULL)
		{
			sendreply("Who do you want me to add?");
			return;
		}
	}

	if((nuh=username(newuser))==NULL)
	{
		sendreply("%s is not on this channel!", newuser);
		return;
	}

	if((newlevel = get_token(&rest, " ")) == NULL )
	{
		sendreply("What level should %s have?", newuser);
		return;
	}

	if(!isnumeric(newlevel))
	{
		send_to_user(from, "%s is an invalid shitlevel", newlevel);
		return;
	}

	sscanf(newlevel, "%d", &level);

	if(level < 0)
	{
		sendreply("level should be >= 0!");
		return;
	}

	if(userlevel(from) < userlevel(nuh) && userlevel(from) < 150)
	{
		/* This way, someone with level 100 can't shit someone with level 150 */
		sendreply("Sorry, %s has a higher level", newuser);
		return;
	}


	strcpy(userstr,get_add_mask(nuh,0));

	if (strlen(rest)==0)
		strcpy(m2,"Blow Us");
	else
	{
		strncpy(m2,rest,99);
		m2[99] = '\0';
	}

	temp[0] = 3;
	temp[1] = 0;
	if(strstr(m2, temp))
	{
		send_ctcp(userchannel(getnick(from)),"ACTION shoves some mIRC colors up %s\'s ass",getnick(from));
		return;
	}

	if (!add_to_levellist(currentbot->lists->shitlist, userstr, level, from, get_gmtime()))
	{
		sendreply("There is already a record that matches %s",userstr);
		return;
	}

	sendreply("User %s shitted with access %d as %s", newuser, level, userstr);

	if(doexpire)
	{
		if (!add_to_expirelist(currentbot->lists->expirelist, userstr, expiretime))
		{
			sendreply("There is already an expirelist record that matches %s",userstr);
			return;
		}
		else
			if(!write_expirelist(currentbot->lists->expirelist, currentbot->lists->expirefile))
			{
				send_to_user(from, "Expirelist could not be written to file %s",
					currentbot->lists->expirefile);
			}
			else
				send_to_user(from, "Expirelist written to file %s", currentbot->lists->expirefile);
	}

	notify_shitadd(newuser,userstr,newlevel,from,m2);
	sprintf(m,"%s shitted as %s with level %d by %s for %s",newuser,
		userstr, level, from,m2);

	if (currentbot->talk>0)
		do_bwallop(currentbot->nick,to,m);

	globallog(2, DONELOG, "%s shitted as %s with level %d by %s",
		newuser, userstr, level, from);

	sprintf(m,"Shitted: %s",m2);
	add_to_passwdlist(currentbot->lists->shitlist2, userstr, m);

#ifdef BackedUp
	if (Abackedup)
		send_to_user(currentbot->backupnick,"!comm %s shitadd %s %d %s",
			currentbot->backupbotpasswd,userstr, level, m);
#endif
}

void	do_shitadd(char *from, char *to, char *rest)
{
	char	m[300],m2[100];
	char	*newuser;
	char	*newlevel;
	char	*expirestr;
	char	temp[2];
	int	level;
	int	doexpire = 0;
	long	expiretime;

	if (userlevel(from)==OTHER_BOT_LEVEL)
		return;

	if((newuser = get_token(&rest, " ")) == NULL)
	{
		send_to_user(from, "Who do you want me to add?");
		return;
	}

	if(STRCASEEQUAL(newuser, "-expire"))
	{
		doexpire = 1;

		if((expirestr = get_token(&rest, " ")) == NULL)
		{
			sendreply("How long should they be shitted?");
			return;
		}

		if(!isnumeric(expirestr))
		{
			send_to_user(from, "%s is an invalid expire time", expirestr);
			return;
		}

		sscanf(expirestr, "%ld", &expiretime);

		if(!expiretime)
		{
			sendreply("Expire time should be > 0" );
			return;
		}

		expiretime *= 86400;
		expiretime += time(NULL);

		if((newuser = get_token(&rest, " ")) == NULL)
		{
			sendreply("Who do you want me to add?");
			return;
		}
	}

	if(mymatch("*?!*?@*?",newuser, FNM_CASEFOLD))
	{
		send_to_user(from, "%s is an invalid mask.", newuser);
		return;
	}

	if((newlevel = get_token(&rest, " ")) == NULL )
	{
		send_to_user(from, "What level should %s have?", newuser);
		return;
	}

	if(!isnumeric(newlevel))
	{
		send_to_user(from, "%s is an invalid shitlevel", newlevel);
		return;
	}

	sscanf(newlevel, "%d", &level);

	if(level < 0)
	{
		send_to_user(from, "level should be >= 0!");
		return;
	}

	if(userlevel(from) < userlevel(newuser) && userlevel(from) < 150)
	{
		/* This way, someone with level 100 can't shit someone with level 150 */
		sendreply("Sorry, %s has a higher level", newuser);
		return;
	}

	temp[0] = 3;
	temp[1] = 0;
	if(strstr(rest, temp))
	{
		send_ctcp(userchannel(getnick(from)),"ACTION shoves some mIRC colors up %s\'s ass",getnick(from));
		return;
	}

	if (!add_to_levellist(currentbot->lists->shitlist, newuser, level, from, get_gmtime()))
	{
		sendreply("There is already a record that matches %s",newuser);
		return;
	}

	send_to_user(from, "User %s shitted with access %d",
		newuser, level);

	if(doexpire)
	{
		if (!add_to_expirelist(currentbot->lists->expirelist, newuser, expiretime))
		{
			sendreply("There is already an expirelist record that matches %s",newuser);
			return;
		}
		else
			if(!write_expirelist(currentbot->lists->expirelist, currentbot->lists->expirefile))
			{
				send_to_user(from, "Expirelist could not be written to file %s",
					currentbot->lists->expirefile);
			}
			else
				send_to_user(from, "Expirelist written to file %s", currentbot->lists->expirefile);
	}

	if (strlen(rest)==0)
		strcpy(m2,"Blow Us");
	else
	{
		strncpy(m2,rest,99);
		m2[99] = '\0';
	}

	notify_shitadd(newuser,newuser,newlevel,from,m2);
	sprintf(m,"%s shitted with level %d by %s for %s",
		newuser, level, from,m2);

	if (currentbot->talk>0)
		do_bwallop(currentbot->nick,to,m);

	globallog(2, DONELOG, "%s shitted with level %d by %s",
		newuser, level, from);
	sprintf(m,"Shitted: %s",m2);
	add_to_passwdlist(currentbot->lists->shitlist2, newuser, m);
#ifdef BackedUp
	if (Abackedup)
		send_to_user(currentbot->backupnick,"!comm %s shitadd %s %d %s",
			currentbot->backupbotpasswd,newuser, level, m);
#endif
}

void	do_shitwrite(char *from, char *to, char *rest)
{
	if(!write_lvllist(currentbot->lists->shitlist,
								  currentbot->lists->shitfile))
		send_to_user(from, "Shitlist could not be written to file %s",
									  currentbot->lists->shitfile);
	else
	{
		send_to_user(from, "Shitlist written to file %s",
									  currentbot->lists->shitfile);
#ifdef BackedUp
				if (Abackedup)
				send_to_user(currentbot->backupnick,"!comm %s shitwrite",
								currentbot->backupbotpasswd);
#endif
	}
		  if(!write_passwdlist(currentbot->lists->shitlist2,
				currentbot->lists->shitfile2))
					 send_to_user(from, "Shitlist2 could not be written to file %s",
										currentbot->lists->shitfile2);
		  else
	{
					 send_to_user(from, "Shitlist2 written to file %s",
										currentbot->lists->shitfile2);
#ifdef BackedUp
				if (Abackedup)
				send_to_user(currentbot->backupnick,"!comm %s shitwrite2",
								currentbot->backupbotpasswd);
#endif
	}
 }

void	do_shitdel(char *from, char *to, char *rest)
{
	int exlist_matches;

	if(!rest)
	{
		send_to_user(from, "Who do you want me to delete?");
		return;
	}

	if(!remove_from_levellist(currentbot->lists->shitlist, rest))
	{
		send_to_user(from, "%s is not shitted!", rest);
		return;
	}

	remove_from_expirelist(currentbot->lists->expirelist, rest);

	if(!write_expirelist(currentbot->lists->expirelist,currentbot->lists->expirefile))
		send_to_user(from, "Could not save expirelist");

	send_to_user(from, "User %s has been deleted", rest);
	if((exlist_matches=num_excludelist_matches(currentbot->lists->excludelist, rest)) > 0)
		send_to_user(from, "There %d exlist entries that match %s", exlist_matches, rest);

	globallog(2, DONELOG, "%s unshitted by %s", rest, from);
	notify_shitdel(rest,from);

#ifdef BackedUp
	if (Abackedup)
		send_to_user(currentbot->backupnick,"!comm %s shitdel %s",
		currentbot->backupbotpasswd,rest);
#endif
	remove_from_levellist(currentbot->lists->shitlist2, rest);
}

void	do_nprotadd(char *from, char *to, char *rest)
{
	char	*newuser;
	char	*newlevel;
	char	*nuh;		/* nick!@user@host */
	char	*userstr;

	if(userlevel(from) == OTHER_BOT_LEVEL)
	{
		return;
	}

	if((newuser = get_token(&rest, " ")) == NULL)
	{
		send_to_user(from, "Who do you want me to protect?");
		return;
	}

	if((nuh=username(newuser))==NULL)
	{
		send_to_user(from, "%s is not on this channel!", newuser);
		return;
	}

	if(userlevel(nuh) > userlevel(from) && userlevel(from) < 125)
	{
		send_to_user(from, "Sorry. %s has a higher level", newuser);
		return;
	}

	if((newlevel = get_token(&rest, " ")) == NULL)
	{
		send_to_user(from, "What level should %s have?", newuser);
		return;
	}

	if(atoi(newlevel)!=50 && atoi(newlevel)!=100)
	{
		send_to_user(from, "That's an invalid prot level moron!");
		return;
	}

	if((userstr = get_mask(currentbot->lists->opperlist, nuh)) == NULL)
	{
		send_to_user(from, "%s is not a user.",newuser);
		return;
	}

	if (!add_to_levellist(currentbot->lists->protlist, userstr, atoi(newlevel), NULL, NULL))
	{
		sendreply("There is already a record that matches %s",userstr);
		return;
	}

	send_to_user(from, "User %s protected with access %d as %s", 
		newuser, atoi(newlevel), userstr);

	globallog(2, DONELOG, "%s protected as %s with level %d by %s", 
		newuser, userstr, atoi(newlevel), from);

#ifdef BackedUp
	if (Abackedup)
		send_to_user(currentbot->backupnick,"!comm %s protadd %s %d",
		currentbot->backupbotpasswd,userstr,atoi(newlevel));
#endif
}

void    do_protadd(char *from, char *to, char *rest)
{
	char    *newuser;
	char    *newlevel;

	if(userlevel(from) == OTHER_BOT_LEVEL)
	{
		return;
	}

	if((newuser = get_token(&rest, " ")) == NULL)
	{
		send_to_user(from, "Who do you want me to protect?");
		return;
	}

	if(userlevel(newuser) > userlevel(from) && userlevel(from) < 125)
	{
		send_to_user(from, "Sorry. %s has a higher level", newuser);
		return;
	}

	if((newlevel = rest) == NULL )
	{
		send_to_user(from, "What level should %s have?", newuser);
		return;
	}

	if(atoi(newlevel)!=50 && atoi(newlevel)!=100)
	{
		send_to_user(from, "That's an invalid prot level moron!");
		return;
	}

	if (!add_to_levellist(currentbot->lists->protlist, newuser, atoi(newlevel), NULL, NULL))
	{
		sendreply("There is already a record that matches %s",newuser);
		return;
	}

	send_to_user(from, "User %s protected with access %d",
		newuser, atoi(newlevel));
	globallog(2, DONELOG, "%s protected with level %d by %s",
		newuser, atoi(newlevel), from);
#ifdef BackedUp
			if (Abackedup)
		send_to_user(currentbot->backupnick,"!comm %s protadd %s %d",
		currentbot->backupbotpasswd,newuser,atoi(newlevel));
#endif

}

void    do_protwrite(char *from, char *to, char *rest)
{
	if(!write_lvllist(currentbot->lists->protlist,
		currentbot->lists->protfile))
		send_to_user(from, "Protlist could not be written to file %s",
			currentbot->lists->protfile);
	else
	{
		send_to_user(from, "Protlist written to file %s",
			currentbot->lists->protfile);
#ifdef BackedUp
		if (Abackedup)
			send_to_user(currentbot->backupnick,"!comm %s protwrite",
				currentbot->backupbotpasswd);
#endif
	}
}

void    do_protdel(char *from, char *to, char *rest)
{
	if(userlevel(from) == OTHER_BOT_LEVEL)
	{
		return;
	}

	if(!rest)
	{
		send_to_user(from, "Who do you want me to unprotect?");
		return;
	}

	if(userlevel(rest) < userlevel(from) && userlevel(from) < 125)
	{
		send_to_user(from, "Sorry. %s has a higher level", rest);
		return;
	}

	if(!remove_from_levellist(currentbot->lists->protlist, rest))
	{
		send_to_user(from, "%s is not protected!", rest);
		return;
	}

	send_to_user(from, "User %s has been unprotected", rest);
	globallog(2, DONELOG, "%s unprotected by %s", rest, from);

#ifdef BackedUp
 	if (Abackedup)
		send_to_user(currentbot->backupnick,"!comm %s protdel %s",
			currentbot->backupbotpasswd,rest);
#endif
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
		{
			ban_user(user_2b_banned, channel);
			globallog(2, DONELOG,"%s banned user %s", from, rest);
			add_to_cmdhist(from, channel, "!banuser %s", user_2b_banned);
		}
		else
			send_to_user(from, "%s not on this channel", rest);
	else
		send_to_user( from, "No." );
}

void	do_showusers(char *from, char *to, char *rest)
{
        char    *channel;

	if (rest == NULL)
		show_users_on_channel( from, currentchannel());
	else
	  {
	        channel = strtok( rest, " " );
		show_users_on_channel( from, channel );
	  }
}

void	do_massop(char *from, char *to, char *rest)
{
	char	*op_pattern;

	if( ( op_pattern = strtok( rest, " " ) ) == NULL )
		send_to_user( from, "Please specify a pattern" );
	else
		channel_massop( ischannel( to ) ? to : currentchannel(), op_pattern );
		add_to_cmdhist(from, ischannel(to)?to:currentchannel(), "!massop %s", op_pattern);
		globallog(2, DONELOG, "%s made me massop %s", from, op_pattern);

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
		add_to_cmdhist(from, ischannel(to)?to:currentchannel(), "!massdeop %s", op_pattern);
		globallog(2, DONELOG, "%s made me massdeop %s", from, op_pattern);
	return;
}

void	do_masskick(char *from, char *to, char *rest)
{
	char	*chan;
	char	*op_pattern;

	if( (op_pattern = get_token(&rest," ")) == NULL )
	{
		send_to_user( from, "Please specify a pattern" );
		return;
	}

	if ((chan = get_token(&rest," "))==NULL || !ischannel(chan) || search_chan(chan)==NULL)
		chan = (ischannel(to) ? to : currentchannel());

	channel_masskick(from, chan, op_pattern);

	globallog(2, DONELOG, "%s made me masskick %s in %s", from, op_pattern, chan);
	add_to_cmdhist(from, chan, "!masskick %s", op_pattern);
	add_list_event("%s made me mass kick %s in %s", from, op_pattern, chan);
	return;
}

void	do_massunban(char *from, char *to, char *rest)
{
	char	*channel;

	if((channel = get_token(&rest, " "))==NULL)
	{
		channel_massunban(currentchannel());
		add_to_cmdhist(from, currentchannel(), "!massunban");
	}
	else
	{
		channel_massunban(channel);
		add_to_cmdhist(from, channel, "!massunban");
	}

	globallog(2, DONELOG, "%s made me massunban %s", from, rest);
}
	
void	do_server(char *from, char *to, char *rest)
{
	char	*server;
	int	serv;
	int	port;

	if(public_command == TRUE && addressed_to_me == FALSE)
	{
		sendreply("That command must be addressed to me if used in public.");
		return;
	}
	if((server=get_token(&rest," "))==NULL)
	{
		sendreply("Change to which server?");
		return;
	}

	if(*rest && !readint(&rest,&port))
	{
		sendreply("Illegal port number!");
		return;
	}
	else port = 6667;

	if (strstr(server,".")!=NULL)
	{
		serv = find_server(currentbot,server,port);

		if (!serv)
		{
			if (!add_server_to_bot(currentbot,server,port))
			{
				sendreply("Can't add more servers to list");
				return;
			}
			serv = currentbot->num_of_servers - 1;
		}
		else serv--;
	}
	else if(!readint(&server, &serv))
	{
		sendreply("Illegal servernumber!");
		return;
	}

	if(not(change_server(serv)))
		sendreply("Invalid server number!");
	else
	{
		send_to_user(from,"Hold on, I'm trying..");
		globallog(0, SYSTEMLOG, "%s  told me to connect to server %d", from,serv);
	}
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
	if (search_list("chat",from,DCC_CHAT))
		pwd(from);
}

void	do_cd(char *from, char *to, char *rest)
{
	if (search_list("chat",from,DCC_CHAT))
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

	if(public_command == TRUE && addressed_to_me == FALSE)
	{
		sendreply("That command must be addressed to me if used in public.");
		return;
	}
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
		else
		        globallog(0, SYSTEMLOG, "%s made me fork a new bot called %s", from, nick);
	}	
	else
		send_to_user(from, "Pls specify nick");
}

void	do_unban(char *from, char *to, char *rest)
{
	char    *channel;
	CHAN_list *c;

	if(rest)
	{
		channel = strtok(rest, " ");

		if (ischannel(channel) && (c=search_chan(channel))==NULL)
		{
			send_to_user(from,"I'm not in that channel freako!");
			return;
		}
	}
	else
		channel = currentchannel();


	channel_unban(channel, from);
	add_to_cmdhist(from, channel, "!unban");
}

void	do_kick(char *from, char *to, char *rest)
{
	char	*nuh,*nick;
	
	if(rest && (nick=get_token(&rest, " ")) && ((nuh = username(nick)) != NULL))
		if((protlevel(username(nick))>=100)&&
		   (shitlevel(username(nick))==0) && lvlcheck(from,username(nick)))
		{
			send_to_user(from, "%s is protected!", nick);
		}
		else if(ischannel(to))
		{
			if (strlen(rest)>0)
			  sendkick(to, nick,rest);
			else
			  sendkick(to, nick, "User requested llama-kick");

			globallog(2, DONELOG, "%s made me kick %s", from, nick);
			add_to_cmdhist(from, to, "!kick %s", nick);
		}
		else
		{
			if (strlen(rest)>0)
			  sendkick(currentchannel(), nick, rest);
			else
			  sendkick(currentchannel(), nick, "User requested llama-kick");

			globallog(2, DONELOG, "%s made me kick %s", from, nick);
			add_to_cmdhist(from, currentchannel(), "!kick %s", nick);
		}
	else
		send_to_user(from, "Who!?!");
}


void	do_listdcc(char *from, char *to, char *rest)
{
	show_dcclist( from );
}

void	do_rehash(char *from, char *to, char *rest)
{
	if(public_command == TRUE && addressed_to_me == FALSE)
	{
		sendreply("That command must be addressed to me if used in public.");
		return;
	}
	rehash = TRUE;
	globallog(0, SYSTEMLOG, "%s made me rehash", from);
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
char 	*shitmask(char *from)
{
	if(from)
		return(get_mask(currentbot->lists->shitlist, from));
	else
		return NULL;
}

int     protlevel(char *from)
{
	if(from)
		return(get_level(currentbot->lists->protlist, from));
	else
		return 0;
}

/*
int     protlevel_wmatch(char *from)
{
	if(from)
		return(get_level_wmatch(currentbot->lists->protlist, from));
	else
		return 0;
}
*/

void	ban_user(char *who, char *channel)
/*
 * Ban the user as nick and as user (two bans)
 */
{
	char	buf[MAXLEN];	/* make a copy, save original */
	char	*usr = buf;
	char	*nick;
	char	*user;
	char	*host;
	char	*domain;
	char	*ip2;
	char	*ip3;
	char	banmask[256];

	strcpy(buf, who);
	nick = get_token(&usr, "!");
	user = get_token(&usr, "@");

	if(*user == '~' || *user == '#')
		user++;

	host=get_token(&usr, ".");
 
	if (check_ip(host))
	{
		ip2 = get_token(&usr, ".");
		ip3 = get_token(&usr, ".");

		if (not(*user) || (*user =='*') || shitlevel(who) == 150)
			sprintf(banmask, "*!*@%s.%s.%s.*", host, ip2, ip3);
		else
			sprintf(banmask, "*!*%s@%s.%s.%s.*", user, host, ip2, ip3);
	}
	else
	{
		domain=get_token(&usr,"");
 
		if(not(*user) || (*user == '*') || shitlevel(who) == 150)/* faking login			*/
		{  /* we can't ban this user on his login, */
		   /* and banning the nick isn't 'nuff, so */
		   /* ban the entire site!                 */
			if (domain && strstr(domain,".")==NULL)
				sprintf(banmask, "*!*@*%s.%s", host, domain);
			else
				sprintf(banmask, "*!*@*%s", domain?domain:host);

			addban(banmask, channel, nick);
			return;
		}

		if (domain && strstr(domain,".")==NULL)
			sprintf(banmask, "*!*%s@*%s.%s", user, host, domain);
		else
			sprintf(banmask, "*!*%s@*%s", user, domain?domain:host);
	}
	addban(banmask, channel, nick);
}

void ban(char *banstring, char *channel)
{
	addban(banstring, channel, NULL);
}

void	signoff(char *from, char *reason)
{
	char	*fromcpy;

	mstrcpy(&fromcpy, from);	/* something hoses, dunno what */

	if(number_of_bots == 1)
	{
		send_to_user(fromcpy, "No bots left... Terminating");
	}

	if (strstr(reason,"Received")==NULL)
	{
		write_all_lists(fromcpy, NULL, NULL);
	}
	send_to_user(fromcpy, "Bye...");
	free(fromcpy);
	killbot(reason);
}
void write_all_lists(char *from, char *to, char *rest)
{
	send_to_user(from, "Saving lists...");
	if(!write_lvllist(currentbot->lists->opperlist,currentbot->lists->opperfile))
		send_to_user(from, "Could not save opperlist");
	if(!write_lvllist(currentbot->lists->shitlist,currentbot->lists->shitfile))
		send_to_user(from, "Could not save shitlist");
	if(!write_lvllist(currentbot->lists->protlist,currentbot->lists->protfile))
		send_to_user(from, "Could not save protlist");
	if(!write_passwdlist(currentbot->lists->shitlist2,currentbot->lists->shitfile2))
		send_to_user(from, "Could not save shitlist2");
	if(!write_passwdlist(currentbot->lists->passwdlist,currentbot->lists->passwdfile))
		send_to_user(from, "Could not save passwdlist");
	if(!write_expirelist(currentbot->lists->expirelist,currentbot->lists->expirefile))
		send_to_user(from, "Could not save expirelist");
	if(!write_excludelist(currentbot->lists->excludelist, currentbot->lists->excludefile))
		send_to_user(from, "Could not save excludelist");
	do_fpwrite(from, NULL, NULL);
	do_accesswrite(from, NULL, NULL);
}
