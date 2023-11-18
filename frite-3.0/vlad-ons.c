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
#include "ctcp.h"
#include "dcc.h"
#include "debug.h"
#include "ftext.h"
#include "ftp_dcc.h"
#include "misc.h"
#include "note.h"
#include "session.h"
#include "userlist.h"
#include "vladbot.h"
#include "vlad-ons.h"
#include "bonk.h"
#include "greeting.h"
#include "publics.h"
#include "news.h"
#include "chanlog.h"
#include "discquote.h"
#include "sing.h"
#include "spy.h"
#include "ascii_lib.h"
#include "function.h"
#include "dance.h"
#include "8ball.h"
#include "inform.h"
#include "kicksay.h"
#include "splatk.h"

#ifdef NO_OPS
int	no_ops_flag=1;
#endif
	int	top_enf;
	char *top_chan, *top_top;
extern	botinfo	*currentbot;
extern	int	number_of_bots;
extern int kslevel;
extern	int	rehash;
extern	long	uptime;
extern	int	malevel;		/* for Channel Steal -pb */
extern	char	lastcmds[10][100];	/* for Last10 -pb */
extern  char    lasttop[10][100];	/* for last 5 topics.. :) */
extern  char	lasttime[10][100];	/* for last 5 topics.. :) */
extern	char	*ownernick;
extern	char	*ownerhost;
extern ListStruct *KickSays, *KickSayChan; /* kick say -pb */
extern	ListStruct *StealList;		/* for Channel Steal -pb */
extern  ListStruct *IgnoreList;		/* ignore -pb/tf */
extern	ListStruct *SpyMsgList;		/* SpyMsg.. :) -pb */
extern	ListStruct *InformList;		/* Inform -pb */
/* external parseroutines ("prefix commands")	*/
extern	void	parse_note(char *from, char *to, char *s);
extern	void	parse_global(char *from, char *to, char *rest);
struct
{
	char	*name;			/* name of command 	*/
	void	(*function)(char *, char*, char*);	/* pointer to function 	*/
	int	userlevel;		/* required userlvl 	*/
	int	shitlevel;		/* max. allowed shitlvl */
	int	forcedcc;		/* If true, command requires DCC */
	char	*comment;		/* Used for command lister.. :) */
} on_msg_commands[] =

/* Any command which produces more than 2 lines output can be 
   used to make the bot "excess flood". Make it "forcedcc"     */

/*	Command		function   userlvl   shitlvl  forcedcc   description  */
{
	/* My new command lister .. */
	{ "LIST",	do_command_list,0,	100,	TRUE,
	  "Lists all commands available to you" },
	/* Dice command */
	{ "DICE",	do_dice,	0,	100,	TRUE,
	  "A Simple dice game" },
	/* PB's 8ball */
	{ "8BALL",	do_8ball,	0,	100,	FALSE,
	  "The magic 8 ball" },
        /* Ascii Library .. -pb */
	{ "ASCII",	do_ascii,	0,	100,	TRUE,
	  "Ascii Pic viewer"},
        /* A useless commands that needs some works .. -pb */
	{ "PING",	do_ping,	0,	100,	FALSE,
	  "Sends CTCP Ping to user" },
        /* Holocaine's wonderful Discordian Quote thingy -pb */
	{ "QUOTE",	do_discquote,	0,	100,	FALSE,
	  "Discordian Quotes" },
        /* LOOK WHAT I WROTE!!! -pb */
#ifdef SING
	{ "LYRICS",	do_lyrics,	0,	100,	FALSE,
	  "Give lyrics to me" },
	{ "SING",	do_sing,	0,	100,	FALSE,
	  "" },
	{ "RAP",	do_sing,	0,	100,	FALSE,
	  "" },
	{ "DANCE",	do_dance,	0,	100,	FALSE,
	  "Does a random dance" },
#endif
	{ "HELP",	show_help,	0,	100,	TRUE,
	  "Help with commands" }, 
	{ "WHOAMI",	show_whoami,  	0,	100,	TRUE,
	  "See what your levels are" },
	{ "INFO",	show_info,	0,	100,	TRUE,
	  "Some information on the bot" },
	{ "TIME",	show_time,	0,	100,	FALSE,
	  "Check the current time" },	
	{ "USERLIST",	show_userlist,	0,	100,	TRUE,
	  "Shows you the userlist (may be long)" },	
	{ "SHITLIST",	show_shitlist,	0,	100,	TRUE,
	  "Shows you the shitlist" },
	{ "PROTLIST",	show_protlist,	0,	100,	TRUE,
	  "Shows you the protlist" },
	{ "NEWS",	show_news,	0,	0,	TRUE,
	  "Latest bot news" },
	{ "LISTDCC",	do_listdcc,	0,	100,	TRUE,
	  "List your DCC connections" },
 	{ "CHAT",	do_chat,	0,	100,	FALSE, 
	  "Create a DCC chat connection with the bot" },
	{ "SEND",	do_send,	0,	100,	FALSE,
	  "Send you a file" },
	/* Some more ftp-dcc functions				*/
	{ "LS",		show_dir,	0,	100,	TRUE,
	  "File list" },
	{ "PWD",	show_cwd,	0,	100,	FALSE,
	  "Print working directory" },
	{ "CD",		do_cd,		0,	100,	FALSE,
	  "Change directory"},
	{ "QUEUE",	show_queue,	0,	100,	TRUE,
	  "Show file queue" },
	/* userlevel and dcc are dealt with within NOTE 	*/
	{ "NOTE",	parse_note,	0,	100,	TRUE,
	  "Send/Recieve notes" },
	/* userlevel and dcc are dealt with within GLOBAL 	*/
	{ "GLOBAL",	parse_global,	0,	100,	FALSE,
	  "Global bot functions"},
	{ "BONK",	do_bonk,	0,	0,	FALSE,
	  "Bonk somone" },
       	{ "OP",		do_op, 		50,	0,	FALSE,
	  "Op yourself" },
	{ "GIVEOP",	do_giveop,	50,	0,	FALSE, 
 	  "Op someone else" },
	{ "DEOP",	do_deop,	50,	0,	FALSE,
	  "Deop someone" },
	{ "INVITE",	do_invite,	50,	0,	FALSE,
	  "Invite you to a channel" },
	{ "WHOIS",	show_whois,	50,	0,	TRUE,
	  "Get userlevel on userhost" },
	{ "RANDTOPIC",	do_disctopic,	50,	0,	FALSE,
	  "Random Topic" },
	{ "TOPIC",	do_topic,	50,	0,	FALSE, 
	  "Change topic" },
	{ "MODE",	do_mode,	50,	0,	FALSE,
	  "Change mode on channel" },
	{ "NWHOIS",	show_nwhois,	50,	0,	TRUE,
	  "Get userlevels of nick" },
	{ "AWAY",	do_away,	50,	0,	FALSE,
	  "Set me away" },
	/* Kick say stuff.. -pb */
        { "LKS",      do_listks,     50,     100,    TRUE,
	  "List KickSays" },
        { "LKSC",     do_listksc,    50,     100,    TRUE,
	  "List KickSay Channels" },
        { "CKS",      do_clrks,      100,    100,    TRUE,
	  "Clear KickSays" },
        { "CKSC",     do_clrksc,     100,    100,    TRUE,
	  "Clear KickSay Channels" },
        { "KSC",      do_addksc,     100,    100,    TRUE,
	  "Add KickSay Channel" },
        { "RKSC",     do_delksc,     100,    100,    TRUE,
	  "Remove KickSay Channel" },
        { "KS",       do_addks,      100,    100,    TRUE,
	  "Add KickSay" },
        { "RKS",      do_delks,      100,    100,    TRUE,
	  "Remove KickSay" },
	/* end of kicksay -pb */
#ifdef NO_OPS
	{ "TOGOP",	do_togop,	100,	0,	FALSE,
	  "Toggle Strict Oping" },
#endif
	{ "LASTTOP",	do_lasttop,	100,	0,	TRUE,
	  "Last 10 Topics" },
	{ "LAST10",	do_last10,	100,	0,	TRUE,
	  "Last 10 Commands" },
	{ "HEART",	do_heart,	100,	0,	FALSE,
	  "Give a heart to a user" },
	{ "LASTLOG",	do_fakemsg,	100,	0,	FALSE,
	  "Fake lastlog" },
	/* My KickBan Code! -pb */
	{ "KB",		do_fuck,	100,	0,	FALSE,
	  "KickBan a user" },
	{ "OPEN",	do_open,	100,	0,	FALSE,
	  "Open Channel so you can get it" },
	{ "BANUSER",	do_banuser,	100,	0,	FALSE,
	  "Ban a user" },
	{ "UNBAN",	do_unban,	100,	0,	FALSE,
	  "Unban .. Doesnt Work" },
/*	{ "KICK",	do_kick,	100,	0,	FALSE },
 */
	{ "KICK",	do_splatk,	100,	0,	FALSE,
	  "Kicks a user with random kick comment" },
	{ "MASSOP",	do_massop,	100,	0,	FALSE,
	  "MassOp pattern" },
	{ "MASSDEOP",	do_massdeop,	100,	0,	FALSE,
	  "MassDeop pattern" },
	{ "MASSKICK",	do_masskick,	100,	0,	FALSE,
	  "MassKick pattern" },
	{ "MASSUNBAN",	do_massunban,	100,	0,	FALSE,
	  "MassUnban pattern" },
	{ "JOIN",	do_join,	125,	0,	FALSE,
	  "Join a channel" },
	{ "LEAVE",	do_leave,	125,	0,	FALSE,
	  "Leave a channel" },
	{ "NICK",	do_nick,	125,	0,	FALSE,
	  "Change nick" },
	{ "CHANNELS",	show_channels,	125,	0,	TRUE,
	  "Shows Channels" },
	{ "SHOWUSERS",	do_showusers,	125,	0,	TRUE,
	  "Show Users on Channel" },
	/* Holo's Bonk .. really Spiffy :) -pb */
	{ "BONKOFF",    do_bonk_off,    125,    0,      FALSE,
	  "Turn Bonking Off" },
	{ "BONKON",     do_bonk_on,     125,    0,      FALSE,
	  "Turn Bonking On" },
	/* Holo's Greet Code .. I like dis one .. -pb */
#ifdef GREET
	{ "GREETOFF",   do_greet_off,   125,    0,      FALSE,
	  "Turn Greeting Off" },
	{ "GREETON",    do_greet_on,    125,    0,      FALSE,
	  "Turn Greeting On" },
#endif
	/* Basic Inform code.. Needs some work .. -pb */
	{ "INFORM",	do_inform,	125,	0,	FALSE,
	  "Inform you if the bot is deoped/kicked" },
	{ "RINFORM",	do_noinform,	125,	0,	FALSE,
	  "Take yourself off the informlist" },
	{ "INFORMLIST", do_informlist,	125,	0,	TRUE,
	  "List of informed users" },
	/* Steal code :) -pb */
	{ "STEAL",	do_steal,	125,	0,	FALSE,
	  "Attempt to steal a channel" },
	{ "NOSTEAL",	do_nosteal,	125,	0,	FALSE,
	  "Stop stealing a channel" },
	{ "SAY",	do_say,		150,	0,	FALSE,
	 "Say something" },
	{ "ME",		do_action,      150,    0,      FALSE,
	  "Do an action" },
	/* My Spy Code. I am really proud of this. */
	{ "SPY",	do_spy,		125,	0,	TRUE,
	  "Spy on a channel" },
	{ "RSPY",	do_rspy,	125,	0,	FALSE,
	  "Stop saying on a channel" },
	{ "SPYLIST",	do_spylist,	125,	0,	TRUE,
	  "List People spying on a channel" },
	/* And of course, My 3l33+ Logging code .. :) */
#ifdef LOG
	{ "LOGOFF",	do_log_off,	150,	0,	FALSE,
	  "Turn Channel logging off" },
	{ "LOGON",	do_log_on,	150,	0,	FALSE,
	  "Turn Channel logging on" },
#endif
	/* hehehehe.. look Allen .. look what I wrote!! -tf */
	{ "IGNORE",	do_ignore, 	150,   	0, 	FALSE,
	  "Ignore a user" }, 
	/* thanks pat.. ill write the unignore.. :) -pb */
	{ "UNIGNORE",	do_unignore,	150,	0,	FALSE,
	  "UnIgnore a user" },
	{ "NUSERADD",	do_nuseradd,	150,	0,	FALSE,
	  "Add user by nick" },
	{ "USERWRITE",	do_userwrite,	150,	0,	FALSE,
	  "Save userlist" },
	{ "USERDEL",	do_userdel,	150,	0,	FALSE,
	  "Delete userhot from userlist" },
	{ "NSHITADD",	do_nshitadd,	150,	0,	FALSE,
	  "Shitadd user by nick" },
	{ "SHITWRITE",	do_shitwrite,	150,	0,	FALSE,
	  "Save the shitlist" },
	{ "SHITDEL",	do_shitdel,	150,	0,	FALSE,
	  "Delete userhost on shtilist" },
        { "NPROTADD",  	do_nprotadd,    150,    0,	FALSE,
	  "Add user to protlist by nick" },
        { "PROTWRITE", 	do_protwrite,   150,    0,	FALSE,
	  "Save protlist" },
        { "PROTDEL",   	do_protdel,     150,    0,	FALSE,
	  "Delete userhost from protlist" },
	{ "USERADD",	do_useradd,	150,	0,	FALSE,
	  "Add userhost to userlist" },
	{ "SHITADD",	do_shitadd,	150,	0,	FALSE,
	  "Add userhost to shitlist" },
        { "PROTADD",   	do_protadd,     150,    0,	FALSE,
	  "Add userhost to protlist" },
	{ "REREADBONK", reinit_bonk,	150,	0,	FALSE,
	  "Re Init the Bonk list" },
	{ "FORK",	do_fork,	150,	0,	FALSE,
	  "Spawn another bot" },
	{ "REHASH",	do_rehash,	150,	0,	FALSE,
	  "Reload the bot" },
	{ "DO",		do_do,		150,	0,	FALSE,
	  "Send a command to the server" },
	{ "DIE",	do_die,		150,	0,	FALSE,
	  "Kill Bot" },
	{ "DIEDIE",	do_die,		150,	0,	FALSE,
	  "Kill Bot" },
	{ "QUIT",	do_quit,	150,	0,	FALSE,
	  "Kill all bots" },
	/*
	 :
	 :
	 */
	{ NULL,		null(void(*)()), 0,	0,	FALSE, NULL }
};
/* im bored so ill steal tf's idea.. :) */
void	on_topic(char *from, char *channel, char *topic)
{
	char log_buff[MAXLEN];
	int ii;
	char *temp;

	sprintf(log_buff, "*** %s has changed the topic on channel %s to %s",
		getnick(from), channel, topic);
	send_spy_chan(channel, log_buff);
	do_chanlog(log_buff);
        for (ii=8;ii>=0;ii--)
	{
        	strcpy(lasttop[ii+1],lasttop[ii]);
	}
		sprintf(lasttop[0], "%s changed topic on channel %s to %s",
			getnick(from), channel, topic);
}
void	on_kick(char *from, char *channel, char *nick, char *reason)
{
	char log_buff[MAXLEN];
	char i_buff[MAXLEN];
	sprintf(log_buff, "*** %s kicked from channel %s by %s (%s)", nick, channel, from, reason);
	send_spy_chan(channel, log_buff);
	do_chanlog(log_buff);

	if(shitlevel(username(nick)) == 0 &&
	   protlevel(username(nick)) >= 100 &&
	   protlevel(from) < 100 &
	   !STRCASEEQUAL(currentbot->nick, getnick(from)))
		sendkick(channel, getnick(from), "Nice move, bung hole ..");
	if(STRCASEEQUAL(currentbot->nick, nick))
	{
		sprintf(i_buff, "I was kicked off channel %s by %s", channel, getnick(from));
		send_inform(i_buff);
	}
}
void	on_part(char *who, char *channel)
{
	char log_buff[MAXLEN];
	sprintf(log_buff, "*** %s left channel %s", who, channel);
	send_spy_chan(channel, log_buff);
	do_chanlog(log_buff);
}
void	on_quit(char *from, char *rest)
{
        char log_buff[MAXLEN];
        sprintf(log_buff, "*** Signoff: %s (%s)", from, rest);
        send_spy_chan(currentchannel(), log_buff);
        do_chanlog(log_buff);
}
void 	on_join(char *who, char *channel)
{
        char log_buff[MAXLEN];
	sprintf(log_buff, "*** %s joined channel %s", who, channel);
	send_spy_chan(channel, log_buff);
	do_chanlog(log_buff); 
	if(currentbot->nick == getnick(who))
		return;
    	if(shitlevel(who) >= 100)
		shitkick(who, channel);
    	if(userlevel(who) >= AUTO_OPLVL && shitlevel(who) == 0)
	        giveop(channel, getnick(who));
	if(userlevel(who) >= GREETLVL)
		do_greeting(getnick(who), channel);
	/* PB's note checker.. hey .. it works.. :) */
	check_note(who);
}
void	shitkick(char *who, char *channel)
{
        char log_buff[MAXLEN];

		ban_user(who, channel);
/* i changed this so it bans whatever the string was in the shitlist .. */
/*  sendmode(channel, "+b %s", shittype(currentbot->lists->shitlist, who)); */
/* too bad this shit doesnt work .. :) */
        	sendkick(channel, getnick(who), "Get the FUCK off my channel ..");
		sprintf(log_buff, "*** %s Shitkicked from channel %s", who, channel);
		send_spy_chan(channel, log_buff);
		do_chanlog(log_buff);
}
void	on_mode(char *from, char *rest)
/*
 * from = who did the modechange
 * rest = modestring, usually of form +mode-mode param1 .. paramN
 */
{
	int	did_change=FALSE;	/* If we have to change anything */
	char	*channel;
	char	*mode;

	char	*chanchars;	/* i.e. +oobli */
	char	*params;
	char	sign;		/* +/-, i.e. +o, -b etc */

	char	unminmode[MAXLEN] = "";		/* used to undo modechanges */
	char	unminparams[MAXLEN] = "";
	char	unplusmode[MAXLEN] = "";	/* used to undo modechanges */
     	char	unplusparams[MAXLEN] = "";
        char log_buff[MAXLEN];
	char i_buff[MAXLEN];
	/* these are here merely to make things clearer */
	char	*nick;
	char	*banstring;
	char	*key;
	char	*limit;
        ShitStruct *temp;

#ifdef DBUG
	debug(NOTICE, "on_mode(\"%s\", \"%s\")", from, rest);
#endif
	channel = get_token(&rest, " ");
	if(STRCASEEQUAL(channel, currentbot->nick))	/* if target is me... */
		return;			/* mode NederServ +i etc */

	chanchars = get_token(&rest, " ");
	params = rest;
	sprintf(log_buff, "*** Mode change %s %s on channel %s by %s", chanchars, params, channel,  getnick(from));
	send_spy_chan(channel, log_buff);
	do_chanlog(log_buff);

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
/* Code for Channel Steal -pb */
				if(!strcasecmp(nick, currentbot->nick))
				{
				   if (is_in_list(StealList, channel))
                                   { 
				     int temp = malevel;
                                     malevel = 99;
                                     channel_massdeop(channel, "*!*@*");
                                     malevel = temp;
                                     remove_from_list(&StealList, channel);
				   }                            
				   shit_check(channel);
				}
				add_channelmode( channel, CHFL_CHANOP, nick );
				if(shitlevel(username(nick))>0)
				{
					strcat(unminmode, "o");
					strcat(unminparams, nick);
					strcat(unminparams, " ");
					bot_reply(from, 6);
					did_change=TRUE;
				}
/* No ops if not on userlist ..*/
#ifdef NO_OPS
	if(no_ops_flag) {
		if((userlevel(username(nick))<1)&& (userlevel(from)<140)) {
					strcat(unminmode, "o");
					strcat(unminparams, nick);
					strcat(unminparams, " ");
					bot_reply(from, 7);
					did_change=TRUE;
				} 
	}
#endif
			}
			else
			{
        if(STRCASEEQUAL(currentbot->nick, nick))
        {
		sprintf(i_buff, "I was deoped by %s on channel %s", getnick(from), channel);
		send_inform(i_buff);
	}
	else
	{	
				del_channelmode(channel, CHFL_CHANOP, nick);
				if((protlevel(username(nick))>=100)&&
				   (shitlevel(username(nick))==0))
				{
					strcat(unplusmode, "o");
					strcat(unplusparams, nick);
					strcat(unplusparams, " ");
					bot_reply(from, 5);
					did_change=TRUE;
				}	
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
                                add_channelmode(channel, MODE_BAN, banstring);
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
				add_channelmode(channel, MODE_KEY, 
						key?key:"???");
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

	int	i, ii;
	char	msg_copy[MAXLEN];	/* for session */
	char	*command;
       ListStruct *Dummy;		/* kick say -pb */
	DCC_list	*userclient;
	char log_buff[MAXLEN];

	/* Ignore command :) -pb/tf */
	if(is_in_list(IgnoreList, getnick(from)))
		return;

	strcpy(msg_copy, msg);
	do_publics(from, to, msg);

	sprintf(log_buff, "<%s> (%s) %s", getnick(from), to, msg);
	send_spy_chan(to, log_buff);
	do_chanlog(log_buff);

        if (is_in_list(KickSays, msg) && is_in_list(KickSayChan, to))
        {
                Dummy = find_list(KickSays, msg);
                if (userlevel(from) <= kslevel)
                {
                        sprintf(log_buff, "ACTION overhears %s say %s", getnick(from),
                                Dummy->name);
                        send_ctcp( to, log_buff);
                          sprintf(log_buff, "ACTION sticks a sock in %s's mouth and...", getnick(from));
			send_ctcp(to, log_buff);
                        sendkick( to, getnick( from ),
                                "You said the magic word.. :)");
                        sendprivmsg( to, "%s shouldn't have said that!",
                                getnick(from) );
                        send_to_user( from, "Careful, %s is banned!",
                                Dummy->name);
			sprintf(log_buff, "Kick say activated by %s", getnick(from));
			        send_spy_chan(to, log_buff);
				do_chanlog(log_buff);
                }
                else
                {
                        sprintf(log_buff, "ACTION overhears %s say a banned word, but overlooks it",
                         getnick(from) );
                        send_ctcp( to, log_buff);
                }
        }
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
	{
		return;
	}
	for(i = 0; on_msg_commands[i].name != NULL; i++)
		if(STRCASEEQUAL(on_msg_commands[i].name, command))
		{
			if(userlevel(from) < on_msg_commands[i].userlevel)
			{
				bot_reply(from, 2);
				return;
			}
		        if(shitlevel(from) > on_msg_commands[i].shitlevel)
			{
				bot_reply(from, 3);
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
	/* Last 10 Commands.. Comes in handy :) */
                                for (ii=8;ii>=0;ii--)
                                        strcpy(lastcmds[ii+1],lastcmds[ii]);
                                strcpy(lastcmds[0], on_msg_commands[i].name);
                                strcat(lastcmds[0], " done by ");
                                strcat(lastcmds[0], left(from,50));


			/* if we're left with a null-string (""), give NULL
			   as msg */
			on_msg_commands[i].function(from, to, *msg?msg:NULL);
			return;
		}
	/* If the command was private, let the user know how stupid (s)he is */
	if(STRCASEEQUAL(to, currentbot->nick))
	{
		bot_reply(from, 1);
		sprintf(log_buff, "%s", msg);
		showspymsg(from, log_buff);
	}
}
void	show_help(char *from, char *to, char *rest)
{
	int 	i, j;
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
		while(s=get_ftext(f))
			send_to_user(from, s);
	}
	else
		if(!find_topic( f, rest ))
			send_to_user(from, "No help available for \"%s\"", rest);
		else
			while(s=get_ftext(f))
				send_to_user(from, s);
	fclose(f);
}

void	show_whoami(char *from, char *to, char *rest)
{
        send_to_user(from, "You are %s. Your levels are:", 
		      from);
	send_to_user(from, "-- User -o- Shit -o- Protect --");
	send_to_user(from, "    %3d  |   %3d  |      %3d", userlevel(from),
		      shitlevel(from), protlevel(from));
        return;
}

void	show_info(char *from, char *to, char *rest)
{
send_to_user(from, "I am %s, a modified Vladbot.", currentbot->nick);
send_to_user(from, "Version %s (%s)", VERSION, currentbot->botname);
send_to_user(from, "Originally by VladDrac, (fRitE) heavily modded by PosterBoy.");
send_to_user(from, "Code also included by TheFuture, and Holocaine.");
if(STRCASEEQUAL(currentbot->nick, "fRitE"))
{
	send_to_user(from, "Official Ascii Bot of the UnderNet");
	send_to_user(from, "For a list of pics: /msg %s ascii", currentbot->nick);
}
send_to_user(from, "Started: %-20.20s", time2str(currentbot->uptime));
send_to_user(from, "Up: %s", idle2str(time(NULL)-currentbot->uptime));
if(ownernick && ownerhost)
send_to_user(from, "This Bot belongs to %s (%s).", ownernick, ownerhost);
}


void	do_togop(char *from, char *to, char *rest)
{
	if(no_ops_flag)
	{
		send_to_user(from, "Strict oping off ..");
		no_ops_flag=0;
	}
	else
	{
		send_to_user(from, "Strict oping on ..");
		no_ops_flag=1;
	}
}
void	show_time(char *from, char *to, char *rest)
{
	if(rest)
		sendprivmsg(rest, "Current time: %s", time2str(time(NULL)));
	else
		send_to_user(from, "Current time: %s", time2str(time(NULL)));
}

void	show_userlist(char *from, char *to, char *rest)
{
	send_to_user(from, " userlist: %30s   %s", "nick!user@host", "level");
        send_to_user(from, " -----------------------------------------o------");
	show_lvllist(currentbot->lists->opperlist, from, rest?rest:"");
}

void	show_shitlist(char *from, char *to, char *rest)
{
	send_to_user(from, " shitlist: %30s   %s", "nick!user@host", "level");
        send_to_user(from, " -----------------------------------------o------");
	show_lvllist(currentbot->lists->shitlist, from, rest?rest:"");
}

void    show_protlist(char *from, char *to, char *rest)
{
        send_to_user(from, " protlist: %30s   %s", "nick!user@host", "level");
        send_to_user(from, " -----------------------------------------o------");
	show_lvllist(currentbot->lists->protlist, from, rest?rest:"");
}
void do_last10(char *from, char *to, char *rest)
{
        int i;

        send_to_user(from, "Last 10 commands:");
        send_to_user(from, "------------------------");
        for(i=0;i<10;i++)
                send_to_user(from, "%2i: %s", i+1, lastcmds[i]);
        send_to_user(from, "------ end of list ------");
        return;
} 
void do_lasttop(char *from, char *to, char *rest)
{
        int i;

        send_to_user(from, "Last 10 topics:");
        send_to_user(from, "------------------------");
        for(i=0;i<10;i++)
        {
	        send_to_user(from, "%2i: %s", i+1, lasttop[i]);
	}
        send_to_user(from, "------ end of list ------");
        return;
} 

void do_heart (char *from, char *to, char *rest)
{
  char *sendto;
  char *nick;
  int i;

  sendto=strtok(rest, " ");
  if (sendto)
    nick = strtok(NULL,  " ");
  if (!sendto || !nick)
    {
      send_to_user(from, "  Usage: heart where nick");
      send_to_user(from, "Example: heart #Chat OffSpring");
      return;
    }
  *(nick+10) = '\0';
  for(i=1;i<(10-strlen(nick))/2;i++)
      strcat(nick, " ");
  if (ischannel(sendto))
      {
        sendprivmsg(sendto,"\x2          ..     ..\x2");
        sendprivmsg(sendto,"\x2       .`    `.\'    \'.\x2");
        sendprivmsg(sendto,"\x2       .\x2%10s   \x2.\x2", nick);
        sendprivmsg(sendto,"\x2         .         .\x2");
        sendprivmsg(sendto,"\x2            .   .\x2");
        sendprivmsg(sendto,"\x2              .\x2");
    }
  else
      {
        sendnotice(sendto,"\x2          ..     ..\x2");
        sendnotice(sendto,"\x2       .`    `.\'    \'.\x2");
        sendnotice(sendto,"\x2       .\x2%10s\x2   .\x2", nick);
        sendnotice(sendto,"\x2         .         .\x2");
        sendnotice(sendto,"\x2            .   .\x2");
        sendnotice(sendto,"\x2              .\x2");
      }
  return;
}
void do_fakemsg(char *from, char *to, char *rest)
{
  char *nick, *msg;

  if (!*rest)
    {
      send_to_user(from, "\x2What's the nick for the fake lastlog?\x2");
      return;
    }
  nick = strtok(rest, " ");
  if (nick)
    msg = strtok(NULL, "\n");
  if (!msg)
    msg = "";
  sendprivmsg(currentchannel(), "*** Lastlog:");
  sendprivmsg(currentchannel(), "\x2*%s*\x2 %s", nick, msg);
  sendprivmsg(currentchannel(), "*** End of Lastlog");
  return;
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
	if(rest)
            	sendmode(to, "+ooo %s", rest);
        else
            	bot_reply(from, 4);
        return;
}
void	do_mode(char *from, char *to, char *rest)
{
	if(rest)
	{
		sendmode(currentchannel(), "%s", rest);
		send_to_user(from, "Mode changed on channel %s", currentchannel());
	}
	else
		send_to_user(from, "Give me a mode please ..");
}
void	do_away(char *from, char *to, char *rest)
{
	if(rest)
	{
		send_to_server("AWAY :%s", rest);
		send_to_user(from, "I am now away ..");
	}
	else
	{
		send_to_server("AWAY");
		send_to_user(from, "I am no longer away ..");
	}
}
void	do_holdtop(char *from, char *to, char *rest)
{
	if (rest)
	{
	  	top_top = rest;
	 	top_enf = 1;
	  if (!ischannel(to))
	  {
		send_to_server("TOPIC %s :%s", currentchannel(), rest);
		send_to_user(from, "Holding topic on %s ..",
			currentchannel());
		top_chan = currentchannel();
	  }
	  else
	  {
		send_to_server("TOPIC %s :%s", to, rest);
		send_to_user(from, "Holding topic on %s ..", to);
		top_chan = to;
	  }
	}
	else
	{
		send_to_user(from, "No longer holding topic ..");
		top_enf = 0;
	}
}
void	do_topic(char *from, char *to, char *rest)
{
	if(rest)
	{
	if (!ischannel(to))
	{
		send_to_server("TOPIC %s :%s", currentchannel(), rest);
		send_to_user(from, "Topic on %s now set to %s", currentchannel(), rest);
	}
	else
	{
		send_to_server("TOPIC %s :%s", to, rest);
		send_to_user(from, "Topic on %s now set to %s", to, rest);
	}
	}
	else
	{
		send_to_user(from, "What should the topic be?");
 	}
	return;
}
void	do_fuck(char *from, char *to, char *rest)
{
	if(rest)
	{
		do_banuser(from, to, rest);
		sendkick(to, rest, "Don't forget to write ..");
	}
	else
		bot_reply(from, 4);
	return;
}
void	do_deop(char *from, char *to, char *rest)
{
	if(rest)
            	sendmode(to, "-ooo %s", rest);
        else
            	bot_reply(from, 4);
        return;
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
void	do_voiceuser(char *from, char *to, char *rest)
{
	dcc_voiceuser(from, rest);
}
void    do_send(char *from, char *to, char *rest)
{
	char	*pattern;
	
	if(rest)
		while(pattern = get_token(&rest, " "))
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

	while(s=get_ftext(ls_file))
		send_to_user(from, s);
	fclose(ls_file);
}	
void do_steal(char *from, char *to, char *rest)
{ 
   ListStruct *temp;
  
   if (!rest || !ischannel(rest))
     { 
        send_to_user( from, "Trying to steal the following channels:");
        send_to_user( from, "---------------------------------------");
        for(temp=StealList;temp;temp=temp->next)
          send_to_user( from, "%s", temp->name);
        send_to_user( from, "-------------- end of list ------------");
          return;
     }
   if (is_in_list(StealList, rest))
     { 
        send_to_user( from, "Already stealing that channel");
        return;
     }
   if (add_to_list(&StealList, rest))
     { 
        send_to_user( from, "Now trying to steal %s", rest);
    	join_channel(rest, "", "", TRUE);
     }    
   else
        send_to_user( from, "Problem adding the channel");
   return;
}
void do_nosteal(char *from, char *to, char *rest)
{ 
  if (!*rest || !ischannel(rest))
    {
        bot_reply(from, 8);
        return;
    }
  if (!is_in_list(StealList, rest))
    { 
        send_to_user( from, "I'm not stealing that channel");
        return;
    }
  if (remove_from_list(&StealList, rest))
    send_to_user( from, "No longer stealing %s", rest);
  else
    send_to_user( from, "Problem deleting the channel");
    leave_channel(rest);
    return;
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
	if(rest || !ischannel(rest))
	{
		send_to_user(from, "Attempting join of channel %s", rest);
	    	join_channel(rest, "", "", TRUE);
	}
        else
		bot_reply(from, 8);
        return;
}
void	do_leave( char *from, char *to, char *rest )
{ 
        char channel[255];
  
        if (rest)
          strcpy(channel, (rest));
        else if (!strcasecmp( currentbot->nick, to))
          strcpy(channel, currentchannel());
        else
          strcpy(channel, to);
  
        if (is_in_list(StealList, channel))
          send_to_user(from, "Sorry, I'm trying to steal %s", channel);
        else
          leave_channel(channel);
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
		signoff( from, "Hey Beavis .. Your pretty cool .. :)");
	if(number_of_bots == 0)
		exit(0);
}

void	do_quit(char *from, char *to, char *rest)
{
	quit_all_bots(from, "Quiting all bots.");
	dump_notelist();
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
                bot_reply(from, 4);
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
	char	*nuh;		/* nick!user@host */

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
send_to_user(username(newuser), "%s has added you to my userlist at level %d ..",
			getnick(from), atoi( newlevel));
if(atoi( newlevel ) >= AUTO_OPLVL)
send_to_user(username(newuser), "You are being auto oped ..");
send_to_user(username(newuser), "My command character is %c", PREFIX_CHAR);
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
            	if(!remove_from_levellist(currentbot->lists->opperlist, rest))
                	send_to_user(from, "%s has no level!", rest);
            	else if(userlevel(from) < userlevel(rest))
			send_to_user(from, "%s has a higer level.. sorry", 
				     rest);
	    	else
                	send_to_user(from, "User %s has been deleted", 
				     rest);
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
		bot_reply(from, 4);
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
send_to_user(username(newuser), "You've been shitlisted by %s at level %d ..",
			getnick(from), atoi(newlevel));
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
		bot_reply(from, 4);
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
		bot_reply(from, 3);
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
            	else if (matches(ownerhost, rest))
			send_to_user(from, "Sorry, you can't delete the owners prot level ..");
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
		if(user_2b_banned = username(rest))
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
	
void	show_dir(char *from, char *to, char *rest)
{
	char	*pattern;

	if(rest)
		while(pattern = get_token(&rest, " "))
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
			sendkick(to, rest, "User requested kick.");
		else
			sendkick(currentchannel(), rest, "User requested kick.");
	else
		bot_reply(from, 4);
}

void	do_listdcc(char *from, char *to, char *rest)
{
	show_dcclist( from );
}
void	do_dccstats(char *from, char *to, char *rest)
{
	show_dcclist2( from );
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
 * this functions bans the user with nick 'nick'.
 * the user to be banned should be online, else
 * no userhost information is available 
 * USE: kick with shitlevel 100, possible kill etc.
 *
 * user nick!user@host.domain will be banned as:
 *      nick!*@*
 *      *!user@*
 *      nick!*user@host.domain
 */
{
	char	*userban, *bkup;
	char	*nick;
	char	*user;
	char	*host;
	char	*domain;
	
	bkup = mstrcpy(&userban, who);
	nick = get_token(&userban, "!");
	user = get_token(&userban, "@");
	if(*user == '~' || *user == '#') user++;
	host = get_token(&userban, ".");	
	domain = userban;

	sendmode(channel, "+b *!*%s@*%s", user, domain);
	free(bkup);
}

void	signoff(char *from, char *reason)
{
	char	*fromcpy;

	mstrcpy(&fromcpy, from);		/* something hoses */
	if(number_of_bots == 1)
	{
		send_to_user(fromcpy, "No bots left... Terminating");
		send_to_user(fromcpy, "Dumping notes");
		dump_notelist();
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

void	do_action(char *from, char *to, char *rest)
{
	if(rest)
            	if(ischannel(to))
                	send_ctcp(to, "ACTION %s", rest);
            	else
                	send_ctcp(currentchannel(), "ACTION %s", rest);
        else
            	send_to_user(from, "I don't know what to do");
        return;
}

void reinit_bonk(char *from, char *to, char *rest)
{
	if(rest)
		init_bonk(rest);
	else
		init_bonk(BONKFILE);
	send_to_user(from, "Bonk list reinitted");
};

void do_ignore(char *from, char *to, char *rest)
{
	ListStruct *temp;

	if(!rest)
	{
		send_to_user(from, " Currently ignoring these nicknames:");
		send_to_user(from, "---------------------------------------");
	        for(temp=IgnoreList;temp;temp=temp->next)
	          send_to_user( from, "%s", temp->name);
		send_to_user(from, "-------------- end of list ------------");
		return;
	}
	if(!is_in_list(IgnoreList, username(rest)))
	{
		send_to_user(from,"I will not take commands from %s for a while..",rest);
		add_to_list(&IgnoreList, rest);
	} 
	else
	{
		send_to_user(from, "Already ignoring %s.", rest);
	}

}

void do_unignore(char *from, char *to, char *rest)
{
	if(!rest)
	{
		bot_reply(from, 4);
		return;
	}
	if(!is_in_list(IgnoreList, rest))
	{
		send_to_user(from, "I'm not ignoring %s.", rest);
	}
	else
	{
		send_to_user(from, "Now accepting commands from %s.", rest);
		remove_from_list(&IgnoreList, rest);
	}
}

void    do_command_list(char *from, char *to, char *rest)
{
 
	char buf[MAXLEN];
        int from_lvl = userlevel(from);
        int i;
 
            send_to_user(from, " Lvl   Command   Description");
            send_to_user(from, "-----------------------------");
 
        for(i=0;on_msg_commands[i].userlevel <= from_lvl;i++)
        {
	  char *command=on_msg_commands[i].name;
	  char *level=on_msg_commands[i].userlevel;
	  char *comment=on_msg_commands[i].comment;
	  if(command != NULL)
	  {
            if(command > 99)
              sprintf(buf, " %i   %10s %s", level, command, comment);
            else
              sprintf(buf, "  %i  %10s %s", level, command, comment);
            send_to_user(from, buf);
	  }
        }
            send_to_user(from, "-----------------------------");
}

