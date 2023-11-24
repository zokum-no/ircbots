/**/
/*
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
#include "autoconf.h"
#include "ctcp.h"
#include "dcc.h"
#include "debug.h"
#include "ftext.h"
#include "ftp_dcc.h"
#include "misc.h"
#include "note.h"
#include "session.h"
#include "userlist.h"
#include "frite_bot.h"
#include "frite_add.h"
#include "frite_add2.h"
#include "bonk.h"
#include "greeting.h"
#include "publics.h"
#include "function.h"
#include "email.h"
#include "war.h"

extern	botinfo	*currentbot;
extern	int	number_of_bots;
extern int kslevel;
extern	int	rehash;
extern	long	uptime;
extern	int	malevel;		/* for Channel Steal -pb */
extern	char	lastcmds[10][100];	/* for Last10 -pb */
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
	int	forcedcc;		/* If true, command requires DCC */
	char	*comment;		/* Used for command lister.. :) */
	int	show;			/* Show on command lister */
} on_msg_commands[] =

/* Any command which produces more than 2 lines output can be 
   used to make the bot "excess flood". Make it "forcedcc"     */

/* Command / function / userlvl / forcedcc / description / show */
{
	{ "HELP",	show_help,	0,	TRUE,
	  "Help with commands",			TRUE }, 
	{ "CREDITS",	do_credits,	0,	TRUE,
	  "Credits on who wrote this bot.",	TRUE },
	{ "RULES",	do_rules,	0,	TRUE,
	  "Rules of the channel.",		TRUE },
	{ "CTOF",	do_ctof,	0,	FALSE,
	  "Centigrade to Farinhiet.",		TRUE },
	{ "FTOC",	do_ftoc,	0, 	FALSE,
	  "Farinhiet to Centigrade.",		TRUE },
	{ "CTOK", 	do_ctok,	0,	FALSE,
	   "Celsius to Kelvin",			TRUE },
	{ "KTOC",	do_ktoc,	0,	FALSE,
	   "Kelvin to Celsius",			TRUE },
	{ "LIST",	do_command_list,0,	TRUE,
	  "Lists all commands available to you",TRUE },
	{ "DICE",	do_dice,	0,	TRUE,
	  "A Simple dice game",			TRUE },
	{ "8BALL",	do_8ball,	0,	FALSE,
	  "The magic 8 ball",			TRUE },
	{ "BULLETIN",	do_bulletin,	0,	TRUE,
	  "Bulletins .. :)",			TRUE },
	{ "ASCII",	do_ascii,	0,	TRUE,
	  "Ascii Pic viewer",			TRUE },
	{ "QUOTE",	do_discquote,	0,	FALSE,
	  "Discordian Quotes",			TRUE },
	{ "SING",	do_sing,	0,	TRUE,
	  "Sings your favorite group",		TRUE },
	{ "RAP",	do_sing,	0,	FALSE,
	  "Rap! :)",				FALSE },
	{ "DANCE",	do_dance,	0,	FALSE,
	  "Does a random dance",		TRUE },
	{ "DTOH",	do_dtoh,	0,	FALSE,
	  "Converts DECIMAL to HEX",		FALSE },
	{ "INFO",	show_info,	0,	TRUE,
	  "Some information on the bot",	TRUE },
	{ "TIME",	show_time,	0,	FALSE,
	  "Display the current time",		TRUE },	
	{ "NEWS",	show_news,	0,	TRUE,
	  "Latest bot news",			TRUE },
	{ "LISTDCC",	do_listdcc,	0,	TRUE,
	  "List your DCC connections",		TRUE },
 	{ "CHAT",	do_chat,	0,	FALSE, 
	  "Create a DCC chat connection",	TRUE },
	{ "MGET",	do_send,	0,	FALSE,
	  "Send you a file",			FALSE },
	{ "GET",	do_send,	0,	FALSE,
	  "Send you a file",			TRUE },
	{ "SEND",	do_send,	0,	FALSE,
	  "Send you a file",			FALSE },
	{ "LS",		show_dir,	0,	TRUE,
	  "File list",				TRUE },
	{ "PWD",	show_cwd,	0,	FALSE,
	  "Print working directory",		TRUE },
	{ "CD",		do_cd,		0,	FALSE,
	  "Change directory",			TRUE },
	{ "QUEUE",	show_queue,	0,	TRUE,
	  "Show file queue",			TRUE },
	{ "FILES",	do_flist,	0,	TRUE,
	  "Show file index list",		TRUE },
	{ "GLOBAL",	parse_global,	0,	FALSE,
	  "Global bot functions",		TRUE },
	{ "CC",		do_getch,	0,	FALSE,
	  "Show you my current channel",	TRUE },
	{ "PING",	do_ping,	0,	FALSE,
	  "Send you a ping",			TRUE },
	{ "NOTE",	parse_note,	0,	TRUE,
	  "Send/Recieve notes",			FALSE },
	{ "LYRICS",	do_lyrics,	50,	FALSE,
	  "Add some lyrics to the bot.",	TRUE },
        { "USERLIST",   show_userlist,  50,     TRUE,
          "Shows you the userlist",             TRUE },
        { "SHITLIST",   show_shitlist,  50,     TRUE,
          "Shows you the shitlist",             TRUE },
        { "PROTLIST",   show_protlist,  50,     TRUE,
          "Shows you the protlist",             TRUE },
        { "POET",       do_poet,        50,     FALSE,
          "Do a random bit of poetry.",         TRUE },
	{ "OPHELP",	show_ophelp,	50,	TRUE,
	  "Displays ChanOp Help",		TRUE },
	{ "APOET", 	do_apoet,	50,	FALSE,
	  "Add a bit of poetry..",		TRUE },
       	{ "OPME",	do_op, 		50,	FALSE,
	  "Op yourself",			TRUE },
	{ "BONK",	do_bonk,	50,	FALSE,
	  "Bonk somone",			TRUE },
	{ "INVITE",	do_invite,	50,	FALSE,
	  "Invite you to a channel",		TRUE },
	{ "ACCESS",	show_nwhois,	50,	FALSE,
	  "Get access of you/nick/host",	TRUE },
	{ "ATOPIC",	do_atopic,	50,	FALSE,
	  "Add a topic to the random topic list",TRUE },
	{ "RTOPIC",	do_disctopic,	50,	FALSE,
	  "Random Topic",			TRUE },
	{ "TOPIC",	do_topic,	50,	FALSE, 
	  "Change topic",			TRUE },
	{ "SHOWIDLE",	do_showidle,	50,	TRUE,
	  "Show idle times of users",		TRUE },
	{ "USERHOST",	do_userhost,	50,	FALSE,
	  "Find the user@host of a nick",	TRUE },
	{ "STATS",	do_stats,	50,	TRUE,
	  "Shows the stats for a nick",		TRUE },
	{ "STATUS",	do_chanstats,	50,	FALSE,
	  "Show channel status",		TRUE },
        { "LKS",      	do_listks,    	50,     TRUE,
	  "List KickSays",			TRUE },
        { "LKSC",       do_listksc,     50,     TRUE,
	  "List KickSay Channels",		TRUE },
        { "CKS",        do_clrks,       100,    FALSE,
	  "Clear KickSays",			FALSE },
        { "CKSC",       do_clrksc,      100,    FALSE,
	  "Clear KickSay Channels",		FALSE },
        { "KSC",        do_addksc,      100,    FALSE,
	  "Add KickSay Channel",		TRUE },
        { "RKSC",       do_delksc,      100,    FALSE,
	  "Remove KickSay Channel",		TRUE },
        { "KS",         do_addks,       100,    FALSE,
	  "Add KickSay",			TRUE },
        { "RKS",        do_delks,       100,    FALSE,
	  "Remove KickSay",			TRUE },
	{ "LIMIT",	do_limit,	100,	FALSE,
	  "Set a channel limit.",		TRUE },
	{ "INSULT",	do_insult,	100,	FALSE,
 	  "Insult someone",			TRUE },
	{ "MODE",	do_mode,	100,	FALSE,
	  "Change mode on channel",		TRUE },
	{ "AWAY",	do_away,	100,	FALSE,
	  "Set me away/unaway ..",		TRUE },
	{ "LAST10",	do_last10,	100,	TRUE,
	  "Last 10 Commands",			TRUE },
	{ "FLIP",	do_flip,	100,	FALSE,
	  "Flip off a user",			TRUE },
	{ "HEART",	do_heart,	100,	FALSE,
	  "Give a heart to a user",		TRUE },
	{ "LASTLOG",	do_fakemsg,	100,	FALSE,
	  "Fake lastlog",			TRUE },
	{ "K",		do_kick,	100,	FALSE,
	  "Kick a user",			TRUE },
	{ "KB",		do_fuck,	100,	FALSE,
	  "KickBan a user",			TRUE },
	{ "OPEN",	do_open,	100,	FALSE,
	  "Open Channel so you can get it",	TRUE },
	{ "B",		do_banuser,	100,	FALSE,
	  "Ban a user",				TRUE },
	{ "UB",		do_unban,	100,	FALSE,
	  "Unban yourself on a channel",	TRUE },
	{ "MOP",	do_massop,	150,	FALSE,
	  "MassOp pattern",			TRUE },
	{ "MDEOP",	do_massdeop,	100,	FALSE,
	  "MassDeop pattern",			TRUE },
	{ "MK",		do_masskick,	100,	FALSE,
	  "MassKick pattern",			TRUE },
	{ "MUB",	do_massunban,	100,	FALSE,
	  "MassUnban pattern",			TRUE },
	{ "JDEOP", 	do_jokedeop,	100,	FALSE,
	  "Joke mass",				TRUE },
	{ "CHANNELS",	show_channels,	100,	TRUE,
	  "Shows Channels",			TRUE },
	{ "DEOP",	do_deop,	100,	FALSE,
	  "Deop someone",			TRUE },
	{ "OP",		do_giveop,	150,	FALSE, 
 	  "Op someone else",			TRUE },
	{ "ADDRULE",	do_addrule,	100,	FALSE,
	  "Add a rule to the rules list.",	TRUE },
	{ "WALL",	do_wall,	125,	FALSE,
	  "Send wallop to all chops on channel", TRUE },
	{ "JOIN",	do_join,	125,	FALSE,
	  "Join a channel",			TRUE },
	{ "LEAVE",	do_leave,	125,	FALSE,
	  "Leave a channel",			TRUE },
	{ "CYCLE",	do_cycle,	125,	FALSE,
	  "Cycle a channel",			TRUE },
	{ "NICK",	do_nick,	125,	FALSE,
	  "Change nick",			TRUE },
	{ "SHOWUSERS",	do_showusers,	125,	TRUE,
	  "Show Users on Channel",		TRUE },
	{ "SETMPL",     do_setmpl,      125,    FALSE,
     	  "Set mass protection level",		TRUE },
	{ "SETFPL",     do_setfpl,      125,    FALSE,
     	  "Set flood protection level",		TRUE },
	{ "SETNCL",     do_setncl,      125,    FALSE,
     	  "Set nick change level",		TRUE },
	{ "SETIKT",     do_setikt,      125,    FALSE,
     	  "Set idle kick time",			TRUE },
	{ "SETMAL",     do_setmal,      125,    FALSE,
    	  "Set mass action level", 		TRUE },
	{ "TOGBONK",    do_bonktog,     125,    FALSE,
	  "Toggle bonking",			TRUE },
	{ "TOGGREET",   do_greettog,    125,    FALSE,
	  "Toggle greets on a channel",		TRUE },
	{ "TOGSING",	do_singtog,	125,	FALSE,
	  "Toggle sining on a channel",		TRUE },
	{ "TOGPUB",	do_pubtog,	125,	FALSE,
	  "Toggle publics on a channel",	TRUE },
	{ "TOGPROT",	do_prottog,	125,	FALSE,
	  "Toggle protection.",			TRUE },
	{ "TOGMASS",	do_masstog,	125,	FALSE,
	  "Toggle mass protection",		TRUE },
	{ "TOGAOP",	do_aoptog,	125,	FALSE,
	  "Toggles auto oping on a channel",	TRUE },
	{ "TOGSHIT",	do_shittog,	125,	FALSE,
	  "Toggles shitlist enforcement",	TRUE },
	{ "TOGWALL",	do_walltog,	125,	FALSE,
	  "Toggles the use of wallops",		TRUE },
        { "TOGIK",	do_iktog,	125,	FALSE,
	  "Toggles idle kicking on channel",	TRUE },
	{ "TOGSO",	do_sotog,	125,	FALSE,
	  "Toggle Strict Oping",		TRUE },
	{ "TOGCK",	do_cktog,	125,	FALSE,
	  "Toggle Caps Kick",			TRUE },
	{ "TOGBK",	do_bktog,	125,	FALSE,
	  "Toggle Beep Kicks",			TRUE },
	{ "TOGSD",	do_sdtog,	125,	FALSE,
	  "Toggle Server Deoping",		TRUE },
	{ "CHACCESS",	do_chaccess,	125,	FALSE,
	  "Change access level of any command",	TRUE },
	{ "RESTRICT",	do_restrict,	125,	FALSE,
	  "Change the restrict level",		TRUE },
	{ "INFORM",	do_inform,	125,	FALSE,
	  "Inform you if deoped/kicked",	TRUE },
	{ "RINFORM",	do_noinform,	125,	FALSE,
	  "Take yourself off the informlist",	TRUE },
	{ "INFORMLIST", do_informlist,	125,	TRUE,
	  "List of informed users",		TRUE },
	{ "STEAL",	do_steal,	125,	FALSE,
	  "Attempt to steal a channel",		TRUE },
	{ "RSTEAL",	do_nosteal,	125,	FALSE,
	  "Stop stealing a channel",		TRUE },
	{ "SPY",	do_spy,		125,	TRUE,
	  "Spy on a channel",			TRUE },
	{ "RSPY",	do_rspy,	125,	FALSE,
	  "Stop saying on a channel",		TRUE },
	{ "SPYLIST",	do_spylist,	125,	TRUE,
	  "List People spying on a channel",	TRUE },
	{ "SPYMSG",	do_spymsg,	125,	FALSE,
	  "Spy on messages sent to the bot",	TRUE },
	{ "RSPYMSG",	do_nospymsg,	125,	FALSE,
	  "Stop spying on messages",		TRUE },
	{ "CMDCHAR",	do_cmdchar,	150,	FALSE,
	  "Display/Change the cmdchar",		TRUE },
	{ "SAY",	do_say,		150,	FALSE,
 	  "Say something",			TRUE },
	{ "ME",		do_action,      150,    FALSE,
	  "Do an action",			TRUE },
	{ "MSG",	do_msg,		150,	FALSE,
	  "Send a message to someone",		TRUE },
	{ "IGNORE",	do_ignore, 	150,   	FALSE,
	  "Ignore a user",			TRUE }, 
	{ "RIGNORE",	do_unignore,	150,	FALSE,
	  "UnIgnore a user",			TRUE },
	{ "USERADD",	do_nuseradd,	150,	FALSE,
	  "Add user to userlist",		TRUE },
	{ "USERWRITE",	do_userwrite,	150,	FALSE,
	  "Save userlist",			TRUE },
	{ "USERDEL",	do_userdel,	150,	FALSE,
	  "Delete userhot from userlist",	TRUE },
	{ "SHITADD",	do_nshitadd,	150,	FALSE,
	  "Shitadd user",			TRUE },
	{ "SHITWRITE",	do_shitwrite,	150,	FALSE,
	  "Save the shitlist",			TRUE },
	{ "SHITDEL",	do_shitdel,	150,	FALSE,
	  "Delete userhost on shtilist",	TRUE },
        { "PROTADD",  	do_nprotadd,    150,    FALSE,
	  "Add user to protlist",		TRUE },
        { "PROTWRITE", 	do_protwrite,   150,    FALSE,
	  "Save protlist",			TRUE },
        { "PROTDEL",   	do_protdel,     150,    FALSE,
	  "Delete userhost from protlist",	TRUE },
/*
	{ "VALUES",	do_show_values,	150,	TRUE,
	  "Show values of variables",		TRUE },
*/
/* Problem with these. Just rehash the bot
	{ "RRBONK", 	reinit_bonk,	150,	FALSE,
	  "Re Init the Bonk list",		TRUE },
	{ "RRPUB",	re_read_publics,150,	FALSE,
	  "Re Init the publics list",		TRUE },
 */
	{ "SERVER",	do_server,	150,	FALSE,
	  "Change servers.",			TRUE },
	{ "REHASH",	do_rehash,	150,	FALSE,
	  "Reload the bot",			TRUE },
        { "FORK",       do_fork,        175,    FALSE,
          "Spawn another bot",                  TRUE },
        { "CTCP",       do_ctcp,        175,    FALSE,
          "Ctcp someone",                       TRUE },
        { "REPLY",      do_reply,       175,    FALSE,
          "Send a ctcp reply",                  TRUE },
        { "TOGLOG",     do_logtog,      175,    FALSE,
          "Toggle channel logging",             TRUE },
	{ "REBOOT",	do_reboot,	175,	FALSE,
	  "Reload the bot with a new binary",	TRUE },
	{ "DO",		do_do,		175,	FALSE,
	  "Send a command to the server",	TRUE },
	{ "DIE",	do_die,		175,	FALSE,
	  "Kill Bot",				TRUE },
	{ "DIEDIE",	do_die,		175,	FALSE,
	  "Kill Bot",				FALSE },
	{ "QUIT",	do_quit,	175,	FALSE,
	  "Kill all bots",			TRUE },
	/*
	 :
	 :
	 */
	{ NULL,		null(void(*)()), 0,	0,	NULL, FALSE }
};
void	on_topic(char *from, char *channel, char *topic)
{
	char log_buff[MAXLEN];

	sprintf(log_buff, "*** %s has changed the topic on channel %s to %s",
		getnick(from), channel, topic);
	send_spy_chan(channel, log_buff);
	do_chanlog(channel, log_buff);
}

void	on_kick(char *from, char *channel, char *nick, char *reason)
{
	USER_list  *User;
	CHAN_list  *Channel;
	char log_buff[MAXLEN];
	char i_buff[MAXLEN];
	char *dude;

        if (!(Channel = search_chan(channel)))
           return;
        if (!(User = search_user(&(Channel->users), getnick(from))))
           return;

	User->kicks++;
	sprintf(log_buff, "*** %s kicked from channel %s by %s (%s)", nick, channel, from, reason);
	send_spy_chan(channel, log_buff);
	do_chanlog(channel, log_buff);
	dude = nick2uh(NULL, nick, 0);

	if(protlevel(dude))
	{
		prot_action(from, channel, dude);
                send_to_server("INVITE %s %s", channel, getnick(dude));
	}
	if(STRCASEEQUAL(currentbot->nick, nick))
	{
		sprintf(i_buff, "I was kicked off channel %s by %s", channel, getnick(from));
		send_inform(i_buff);
	    if(!protlevel(from))
                set_kickedby(channel, from);
                join_channel(channel, "", "", TRUE);
	}
}
void	on_part(char *who, char *channel)
{
	char log_buff[MAXLEN];
	sprintf(log_buff, "*** %s left channel %s", getnick(who), channel);
	send_spy_chan(channel, log_buff);
	do_chanlog(channel, log_buff);
}
void	on_quit(char *from, char *rest)
{
        char log_buff[MAXLEN];
        sprintf(log_buff, "*** Signoff: %s (%s)", from, rest);
        send_spy_chan(currentchannel(), log_buff);
        do_chanlog(currentchannel(), log_buff);
}
void 	on_join(char *who, char *channel)
{
        char log_buff[MAXLEN];
	sprintf(log_buff, "*** %s joined channel %s", who, channel);
	send_spy_chan(channel, log_buff);
	do_chanlog(channel, log_buff); 
	if(!STRCASEEQUAL(currentbot->nick, getnick(who)))
	{
    	if(shitlevel(who) >= 100)
		shit_action(who, channel);
    	if(userlevel(who) >= AUTO_OPLVL && userlevel(who) >= currentbot->restrict &&
	     !shitlevel(who) && get_aoptog(channel))
	        giveop(channel, getnick(who));
	if(userlevel(who) >= ASSTLVL && get_greettog(channel))
		do_greeting(getnick(who), channel);
	check_note(who); /* My note checker -pb */
	}
}
void	on_mode(char *from, char *rest)
/* Neat stuff i got from VladBot */
#define REMOVEMODE(chr, prm) do{ strcat(unminmode, chr); \
                                 strcat(unminparams, prm); \
                                 strcat(unminparams, " "); \
                             } while(0)

#define ADDMODE(chr, prm)    do{ strcat(unplusmode, chr); \
                                 strcat(unplusparams, prm); \
                                 strcat(unplusparams, " "); \
                             } while(0)
#define REM(chr) do{ strcat(unminmode, chr); } while(0)
#define ADD(chr) do{ strcat(unplusmode, chr); } while(0)
{
	int	did_change=FALSE;	/* If we have to change anything */
	char	*channel;
/*	char	*mode; */

	char	*chanchars;	/* i.e. +oobli */
	char	*params;
	char	sign;		/* +/-, i.e. +o, -b etc */

	char	unminmode[MAXLEN] = "";		/* used to undo modechanges */
	char	unminparams[MAXLEN] = "";
	char	unplusmode[MAXLEN] = "";	/* used to undo modechanges */
     	char	unplusparams[MAXLEN] = "";
	char    *tempy;
        char log_buff[MAXLEN];
	char i_buff[MAXLEN];
	CHAN_list	*Channel;
	USER_list	*User;
	char	*nick;
	char	*banstring;
	char	*key;
	char	*limit;
 /*       ShitStruct *temp;*/

#ifdef DBUG
	debug(NOTICE, "on_mode(\"%s\", \"%s\")", from, rest);
#endif
	channel = get_token(&rest, " ");
	if(STRCASEEQUAL(channel, currentbot->nick))   /* if target is me... */
		return;			  	      /* mode fRitE +i etc */
        if (!(Channel = search_chan(channel)))
           return;
        if (!(User = search_user(&(Channel->users), getnick(from))))
           return;
	chanchars = get_token(&rest, " ");
	params = rest;
	sprintf(log_buff, "*** Mode change %s %s on channel %s by %s", chanchars,
		params, channel,  getnick(from));
	send_spy_chan(channel, log_buff);
	do_chanlog(channel, log_buff);
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
			User->ops++;
			nick = get_token(&params, " ");
			if(sign == '+')
			{
				if(STRCASEEQUAL(nick, currentbot->nick))
				{
				   if (is_in_list(StealList, channel))
                                   { 
				     int temp = malevel;
                                     malevel = 99;
                                     channel_massdeop(channel, "*!*@*");
                                     malevel = temp;
                                     remove_from_list(&StealList, channel);
				   }                            
				    check_shit();
				    tempy = get_kickedby(channel);
                                    if (tempy)
                                    {
                                          sendkick(channel, getnick(tempy), "This is revenge, FUCKER!");
                                          set_kickedby(channel, NULL);
                                    }
                                }
				add_channelmode( from, channel, CHFL_CHANOP, nick, getthetime() );
				if(shitlevel(username(nick))>0)
				{
                                        REMOVEMODE("o", nick);
					send_to_user(from, "Sorry That user is on the SHITLIST!");
					did_change=TRUE;
				}
                                if (!strchr(from, '@') && !userlevel(username(nick)) &&
			            get_sdtog(channel))
                                {
			                REMOVEMODE("o", nick);
			                sendnotice(nick, "Net-split Ops Detected By You.");
                                        did_change=TRUE;
                                }
				if((!userlevel(username(nick))) && (userlevel(from)<ASSTLVL) &&
				   get_sotog(channel))
                	        {
			                REMOVEMODE("o", nick);
			                send_to_user(from, "Sorry OPs for people on the OPLIST only!");
			                did_change=TRUE;
				}
			}
			else
			{
			  User->deops++;
			  if (!STRCASEEQUAL(currentbot->nick, getnick(from)))
			     if (check_massdeop(from, channel) && get_masstog(channel))
				 if (userlevel(from) < ASSTLVL)
					mass_action(from, channel);
                          if(STRCASEEQUAL(currentbot->nick, nick))
                          {
		             sprintf(i_buff,
                                 "I was deoped by %s on channel %s", getnick(from), channel);
		             send_inform(i_buff);
	                  }
	                  else
	                  {	
				del_channelmode(channel, CHFL_CHANOP, nick);
				if(protlevel(username(nick))>=50 &&
				   !shitlevel(username(nick)))
				{
					ADDMODE("o", nick);
					send_to_user(from, "HEY ! That user is protected!");
				        prot_action(from, channel, nick);
					did_change=TRUE;
				}	
	                  }
			}
			break;
		case 'v':
			nick = get_token(&params, " ");
			if(sign == '+')
				add_channelmode(from, channel, CHFL_VOICE, nick, getthetime());
			else
				del_channelmode(channel, CHFL_VOICE, nick);
			break;
		case 'b':
			banstring = get_token(&params, " ");
                        if(sign == '+')
			{
                                add_channelmode(from, channel, MODE_BAN, banstring, getthetime());
				check_bans();  /* Kick people matchin ban >:) */
                                if(find_highest(channel, banstring) >= 125)
                                {
					REMOVEMODE("b", banstring);
                                        did_change = TRUE;
				        prot_action(from, channel, banstring);
 		                }
			}
                        else
                                del_channelmode(channel, MODE_BAN, banstring);
                        break;
		case 'p':
                        if(sign == '+')
			{
                                add_channelmode(from, channel, MODE_PRIVATE, "", getthetime());
                                if (mycheckmode(from)) {
				  REM("p");
				  did_change=TRUE;
				}
			}
                        else
                                del_channelmode(channel, MODE_PRIVATE, "");
                        break;
		case 's':
			if(sign == '+')
			{
		                add_channelmode(from, channel, MODE_SECRET, "", getthetime());
                                if (mycheckmode(from)) {
				  REM("s");
				  did_change=TRUE;
				}
			}
			else
				del_channelmode(channel, MODE_SECRET, "");
			break;
		case 'm':
			if(sign == '+')
			{
				add_channelmode(from,channel, MODE_MODERATED, "", getthetime());
				if (mycheckmode(from)) {
				  REM("m");
				  did_change=TRUE;
			 	}
			}
			else
				del_channelmode(channel, MODE_MODERATED, "");
			break;
		case 't':
			if(sign == '+')
				add_channelmode(from,channel, MODE_TOPICLIMIT, "", getthetime());
			else
			{
				del_channelmode(channel, MODE_TOPICLIMIT, "");
				if (mycheckmode(from)) {
				  strcat(unplusmode, "t");
				  did_change=TRUE;
			 	}
			}
			break;
		case 'i':
			if(sign == '+')
			{
 				add_channelmode(from,channel, MODE_INVITEONLY, "",getthetime());
				if (mycheckmode(from)) {
				  strcat(unminmode, "i");
				  did_change=TRUE;
			 	}
			}
			else
				del_channelmode(channel, MODE_INVITEONLY, "");
			break;
		case 'n':
			if(sign == '+')
				add_channelmode(from,channel, MODE_NOPRIVMSGS, "", getthetime());
			else
			{
				del_channelmode(channel, MODE_NOPRIVMSGS, "");
				if (mycheckmode(from)) {
				  strcat(unplusmode, "n");
				  did_change=TRUE;
			 	}
			}
			break;
		case 'k':
                        key = get_token(&params, " ");
                        if(sign == '+')
                        {
                                char    *s;

                                add_channelmode(from,channel, MODE_KEY,
                                                key?key:"???",getthetime());
                                /* try to unset bogus keys */
                                for(s = key; key && *s; s++)
                                        if(*s < ' ')
                                        {
                                                REMOVEMODE("k", key);
                                                did_change = TRUE;
                                                send_to_user(from,
                                                "No bogus keys please ..");
                                                break;
                                        }
				if (mycheckmode(from)) {
				  REMOVEMODE("k", key);
				  did_change=TRUE;
			 	}
                        }
                        else
                                del_channelmode(channel, MODE_KEY, "");
                        break;

		case 'l':
                        if(sign == '+')
                        {
                                limit = get_token(&params, " ");
                                add_channelmode(from,channel, MODE_LIMIT,
                                                limit?limit:"0", getthetime());
				if (mycheckmode(from)) {
				  strcat(unminmode, "l");
				  did_change=TRUE;
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
	if(did_change)
		sendmode( channel, "+%s-%s %s %s", unplusmode, 
			  unminmode, unplusparams, unminparams);
}
void	on_public(char *from, char *to, char *msg)
{
	ListStruct *Dummy;
	CHAN_list *Channel;
	char 	log_buff[MAXLEN];	
	int 	level;
	char 	thehost[100];
	TS 	*Temp;

	Channel = currentbot->Current_chan; 
	Temp = is_in_timelist(currentbot->OffendersList, gethost(from)?"":"");
	if (!Temp)
        {
		add_to_timelist(currentbot->OffendersList, thehost);
		Temp = is_in_timelist(currentbot->OffendersList, thehost);
		if (Temp->num)
		  Temp->num = 0;
        }

        update_idletime(from, to);
	if (get_pubtog(to))
	  handle_publics(from, to, msg);


        if (is_in_list(KickSays, msg) && is_in_list(KickSayChan, to))
        {
                Dummy = find_list(KickSays, msg);
                if (userlevel(from) <= kslevel)
                {
                        sprintf(log_buff, "ACTION me was patrolling the channel when I heard %s say %s...", getnick(from),
                                Dummy->name);
                        send_ctcp( to, log_buff);
                          sprintf(log_buff, "ACTION Don't know about you, but where I was raised, that language woudda got yer butt whooped..", getnick(from));
			send_ctcp(to, log_buff);
                        sendkick( to, getnick( from ),
                                "*WhOoMp*");
                        sendprivmsg( to, "%s shouldn't have said that!",
                                getnick(from) );
                        send_to_user( from, "Careful, %s is banned!",
                                Dummy->name);
			sprintf(log_buff, "Kick say activated by %s", getnick(from));
			        send_spy_chan(to, log_buff);
				do_chanlog(to, log_buff);
                }
                else
                {
			sprintf(log_buff, "ACTION overhears %s asking somebody why it hurts when (s)he says \"%s\"", getnick(from), Dummy->name);
			send_ctcp( to, log_buff);
                        sprintf(log_buff, "ACTION leaps up into the air, running towards %s with a huge club!",
                         getnick(from) );
                        send_ctcp( to, log_buff);
			sprintf(log_buff, "ACTION hits %s!  *pOk*  Then don't say that!!", getnick(from));
			send_ctcp( to, log_buff);
				
                }
        }
 if (Channel->bk) {
  level = numchar(msg, '\007');  
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
		        /*	channel_massunban(to, temp, 0); */
				sendmode(to, "+b %s", temp);
			}
		}
		sendkick(to, getnick(from), "Stop beeping, BITCH!\007");
	}
}
if (Channel->ck) {
  level = percent_caps(msg);
  if (level >= CAPP && (strlen(msg) >= CAPL))
  {
	if(!userlevel(from))
	{
		if (Temp)
		{
			Temp->num++;
			if (Temp->num == 4)
				sendmode(to, "+b %s", format_uh(from, 1));
			else if (Temp->num > 4)
			{
				char *temp;
				temp = format_uh(from, 2);
				sendmode(to, "+b %s", temp);
			}
		}
		if (Temp->num == 1) sendprivmsg(from, "Turn off your caps lock !!");
		sendkick(to, getnick(from), "Hit your friendly CAPS LOCK key please!");
       }
  }
}

  if ((level = check_pubflooding(from, to)) && get_masstog(to))
  {
		if (level >= 1)
		  sendkick(to, getnick(from), "Get out you flooder!"); 
  }

}
void	on_msg(char *from, char *to, char *msg)
{

	int	i, ii;
	char	msg_copy[MAXLEN];	/* for session */
	char	*command;
	DCC_list	*userclient;
	char log_buff[MAXLEN];
	char yuck[MAXLEN];
	strcpy(msg_copy, msg);
	strcpy(yuck, msg);

	if(is_in_list(IgnoreList, from))
		return;

	if(shitlevel(from) && !userlevel(from))
		return;
	if (ischannel(to))
	    on_public(from, to, msg);

	if(check_session(from) == IS_FLOODING)
		return;

	sprintf(log_buff, "<%s> %s", getnick(from), msg);
	send_spy_chan(to, log_buff);
	if (ischannel(to))
	  do_chanlog(to, log_buff);

	if((command = get_token(&msg, ",: "))== NULL)
		return;			/* NULL-command */

	if(STRCASEEQUAL(currentbot->nick, command) ||
	   STRCASEEQUAL(currentbot->initials, command))
	{
		if((command = get_token(&msg, ",: "))==NULL)
			return;		/* NULL-command */
	}
	else if((*command != currentbot->cmdchar) && !STRCASEEQUAL(to, currentbot->nick))
		return;

	if(*command == currentbot->cmdchar)
		command++;

	if(STRCASEEQUAL(currentbot->nick, to))
		showspymsg(from, yuck);

	for(i = 0; on_msg_commands[i].name != NULL; i++)
		if(STRCASEEQUAL(on_msg_commands[i].name, command))
		{
			if(userlevel(from) < on_msg_commands[i].userlevel)
			{
				send_to_user(from, "Sorry your access is too low !");
				return;
			}
                        if ((userlevel(from) < currentbot->restrict) &&
                                 (on_msg_commands[i].userlevel != -1))
                        {
                                sendreply("Sorry, I'm being restricted to at least level %i",
					currentbot->restrict);
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
   		        for (ii=8;ii>=0;ii--)
			     strcpy(lastcmds[ii+1], lastcmds[ii]);
                        strcpy(lastcmds[0], on_msg_commands[i].name);
                        strcat(lastcmds[0], " done by ");
                        strcat(lastcmds[0], left(from,50));
			command_log(from, to, on_msg_commands[i].name);
			/* if we're left with a null-string (""), give NULL
			   as msg */
			on_msg_commands[i].function(from, to, *msg?msg:NULL);
			     return;
		}
	/* If the command was private, let the user know how stupid (s)he is */
	if(STRCASEEQUAL(to, currentbot->nick))
	{
		send_to_user(from, "Huh??? Try LIST for a list of commands");
	}
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

void    show_ophelp(char *from, char *to, char *rest)
{
        FILE    *f;
        char    *s;

        if((f=fopen(currentbot->ophelpfile, "r"))==NULL)
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

void	show_info(char *from, char *to, char *rest)
{
  sendreply("I am %s, version %s (%s)", currentbot->nick, VERSION,
			currentbot->botname);
  sendreply("For programming info, use \"%ccredits\"", currentbot->cmdchar);
  count_lvllist(from, currentbot->lists->opperlist);
  sendreply("Started: %-20.20s", time2str(currentbot->uptime));
  sendreply("Up: %s", idle2str(time(NULL)-currentbot->uptime));
  if(ownernick && ownerhost)
   sendreply("This Bot belongs to %s (%s).", ownernick, ownerhost);
}

void	show_time(char *from, char *to, char *rest)
{
	if(rest)
	   if(ischannel(rest))
		sendprivmsg(rest, "Current time: %s", time2str(time(NULL)));
	   else
		sendnotice(rest, "Current time: %s", time2str(time(NULL)));
	else
		sendreply("Current time: %s", time2str(time(NULL)));
}

void	show_userlist(char *from, char *to, char *rest)
{
	sendreply(" userlist: %30s   %s", "nick!user@host", "level");
        sendreply(" -----------------------------------------o------");
	show_lvllist(currentbot->lists->opperlist, from, rest?rest:"");
}

void	show_shitlist(char *from, char *to, char *rest)
{
	sendreply(" shitlist: %30s   %s", "nick!user@host", "level");
        sendreply(" -----------------------------------------o------");
	show_lvllist(currentbot->lists->shitlist, from, rest?rest:"");
}

void    show_protlist(char *from, char *to, char *rest)
{
        sendreply(" protlist: %30s   %s", "nick!user@host", "level");
        sendreply(" -----------------------------------------o------");
	show_lvllist(currentbot->lists->protlist, from, rest?rest:"");
}
void do_last10(char *from, char *to, char *rest)
{
        int i;

        sendreply("Last 10 commands:");
        sendreply("------------------------");
        for(i=0;i<10;i++)
           sendreply("%2i: %s", i+1, lastcmds[i]);
        sendreply("------ end of list ------");
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
      sendreply("  Usage: heart where nick");
      sendreply("Example: heart #Chat OffSpring");
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

 if(!rest)
  {
    send_to_user(from, "Usage: \002<nick> <fake last message>\002");
    return;
  }

	nick = get_token(&rest, " ");
	msg = get_token(&rest, "\n");
   sendprivmsg(currentchannel(), "*** Lastlog:");
   sendprivmsg(currentchannel(), "\002*%s*\002 %s", nick, msg);
   sendprivmsg(currentchannel(), "*** End of Lastlog");
}


void	do_op(char *from, char *to, char *rest)
{
        char *chan;

	chan = findchannel(to, &rest);
	if(usermode(chan, getnick(from))&MODE_CHANOP)
		send_to_user(from, "You're already channel operator!");
	else
        	giveop(chan, getnick(from));
}

void	do_giveop(char *from, char *to, char *rest)
{
	char *channel;

	channel = findchannel(to, &rest);
	if(rest)
            	sendmode(channel, "+ooo %s", rest);
        else
                giveop(channel, getnick(from));
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
void	do_topic(char *from, char *to, char *rest)
{
	char *chan;
	chan = findchannel(to, &rest);

	if(rest)
	{
		send_to_server("TOPIC %s :%s", chan, rest);
		sendreply("Topic on %s now set to \"%s\"", to, rest);
	}
	else
		sendreply("What should the topic be?");
	return;
}
void	do_fuck(char *from, char *to, char *rest)
{
        char    *nuh;
	char	*nick;
	char	*channel;

	channel = findchannel(to, &rest);
	if ((nick = get_token(&rest, " "))!=NULL)
        if((nuh = username(nick)) != NULL)
                if((protlevel(username(rest))>=100)&&
                   (shitlevel(username(rest))==0))
                        send_to_user(from, "%s is protected!", rest);
                else
		{
			deop_ban(channel, nick, nuh);
		    if (!rest || !*rest)
                        splatter(channel, nick);
		    else
			sendkick(channel, nick, rest);
		}
        else
		send_to_user(from, "I need a nick to kick ..");
	return;
}
void	do_deop(char *from, char *to, char *rest)
{
        char *chan;

	chan = findchannel(to, &rest);

	if(rest)
            	sendmode(chan, "-ooo %s", rest);
        else
            	send_to_user(from, "I need a nick to deop ..");
        return;
}

void	do_invite(char *from, char *to, char *rest)
{
	char *chan, *nick;

	chan = findchannel(to, &rest);
	nick = getnick(from);

	if (rest && *rest)
	  nick = get_token(&rest, " ");

	if (!invite_to_channel(nick, chan))
	  sendreply("I'm not on channel %s..", chan);
	else
	  sendreply("%s invited to %s", nick, chan);
/*
        if(rest)
	{
		if(!invite_to_channel(getnick(from), rest))	
			send_to_user(from, "I'm not on channel %s", rest);
	}
	else
		invite_to_channel(from, currentchannel());
*/
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

/*	strcpy(indexfile, currentbot->indexfile ); */
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
	send_to_user(from, "I need a channel name ..");
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
	char *chan;

	chan = findchannel(to, &rest);
	if (!rest)
	{
		sendreply("What am I suppposed to say?");
		return;
	}
                           /* yes!! */
	sendprivmsg(chan, "\002\002%s", rest);

/*
	if(rest)
            	if(ischannel(to))
                	sendprivmsg(to, "%s", rest);
            	else
                	sendprivmsg(currentchannel(), "%s", rest);
        else
            	send_to_user(from, "I don't know what to say");
*/
        return;
}
void do_msg(from, to, rest)
char *from;
char *to;
char *rest;
{
	char *nick;

	if (!rest)
	{
		sendreply("No nick given");
		return;
	}
	nick = get_token(&rest, " ");

	if (!rest)
	{
		 sendreply("What's the message?");
		 return;
	}

	if (ischannel(nick))
	{
		sendreply("Use \"say\" instead");
		return;
	}

	if (!my_stricmp(nick,currentbot->nick))
	{
		 sendreply("Nice try, but I don't think so");
		 return;
	}

	sendprivmsg(nick, "\002\002%s", rest);
	sendreply("Message sent to %s", nick);
}
void do_ctcp(from, to, rest)
char *from;
char *to;
char *rest;
{
	char *nick;

	if (!rest)
	{
		sendreply("No nick/#channel given");
		return;
	}
	nick = get_token(&rest, " ");

	if (!rest)
	{
		 sendreply("What's the CTCP?");
		 return;
	}

	if (!my_stricmp(nick,currentbot->nick))
	{
		 sendreply("Nice try, but I don't think so");
		 return;
	}

	send_ctcp(nick, "\002\002%s", rest);
	sendreply("CTCP sent to %s", nick);
}
void do_reply(from, to, rest)
char *from;
char *to;
char *rest;
{
	char *nick;

	if (!rest)
	{
		sendreply("No nick/#channel given");
		return;
	}
	nick = get_token(&rest, " ");

	if (!rest)
	{
		 sendreply("What's the reply?");
		 return;
	}

	if (!my_stricmp(nick,currentbot->nick))
	{
		 sendreply("Nice try, but I don't think so");
		 return;
	}

	send_ctcp_reply(nick, "\002\002%s", rest);
	sendreply("CTCP reply sent to %s", nick);
}

void	do_do(char *from, char *to, char *rest)
{
  char *temp;

  if (rest)
  {
	 temp = strchr(rest, ':');
	 if (temp)
		*temp++ = '\0';
	 if (temp && *temp)
		send_to_server( "%s:\002\002%s", rest, temp);
	 else
		send_to_server( "%s", rest);
  }
  else
	 sendreply("What do you want me to do?");
/*	if(rest)
            send_to_server(rest);
        else
            send_to_user(from, "What do you want me to do?");
*/
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
		send_to_user(from, "I need a channel name ..");
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
void    do_cycle( char *from, char *to, char *rest )
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
        {
        send_to_user(from,"Cycling channel %s", channel);  
        cycle_channel(channel);
        }
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
		signoff( from, "Same bot time .. Same bot channel ..");
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
		send_to_user(from, "Please specify a user");
 	else if(!matches(rest, "@"))
		send_to_user(from, "Invalid userhost ..");
	else
	{
        send_to_user(from, "%s's levels are:",
                     rest);
        send_to_user(from, "-- User -+- Shit -+- Protect --" );
        send_to_user(from, "    %3d  |   %3d  |      %3d", userlevel(rest),
                     shitlevel(rest), protlevel(rest));
	}
        return;
}

void	show_nwhois(char *from, char *to, char *rest)
{

	char *nuh;

	if ((nuh = get_token(&rest, " ")))
	  nuh = nick2uh(from, nuh, 0);
	else
	  nuh = from;
	if (!nuh)
	  return;
        sendreply("%s's levels are:", getnick(nuh));
        sendreply("-- User -+- Shit -+- Protect --" );
        sendreply("    %3d  |   %3d  |      %3d", userlevel(nuh),
                     shitlevel(nuh), protlevel(nuh));

}
void	do_nuseradd(char *from, char *to, char *rest)
{
	char	*newuser;
	char	*newlevel;
	char	*uh;

        if((newuser = get_token(&rest, " ")) == NULL)
	{
             	send_to_user( from, "Who do you want me to add?" );
		return;
	}
	if (!(uh = nick2uh(from, newuser, 1)))
	  return;
        if((newlevel = get_token(&rest, " ")) == NULL)
	{
                send_to_user( from, "What level should %s have?", newuser );
		return;
	}
        if(atoi(newlevel) < 0)
		send_to_user(from, "level should be >= 0!");
	else if(atoi(newlevel) > userlevel(from))
		send_to_user( from, "Sorry bro, can't do that!" );
	else if(atoi(newlevel) > 155)
		send_to_user( from, "Can't modify level 175 users." );
	else
	    if(userlevel(from) < userlevel(uh))
		send_to_user(from, "Sorry, %s has a higher level", newuser);
	else
	{
	 char	userstr[MAXLEN];

	 strcpy(userstr, uh);
         sendreply("User added with access %i as %s", atoi(newlevel),
		userstr);
         add_to_levellist(currentbot->lists->opperlist, userstr,
		atoi(newlevel));
	 sendnotice(newuser, "%s has added you to my userlist at level %d ..",
			getnick(from), atoi( newlevel));
         if(atoi( newlevel ) >= AUTO_OPLVL)
	      sendnotice(newuser, "You are being auto oped ..");
	 sendnotice(newuser, "My command character is %c", currentbot->cmdchar);
	 check_shit();
	}
         return;
}                   

/*
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
*/
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
	char *nuh;
	int master = 175;
        if(rest)
        {
		if (!(nuh = nick2uh(from, rest, 1)))
                   return; 
           	if(!remove_from_levellist(currentbot->lists->opperlist, nuh))
                	sendreply("User has no level!");
            	else if(userlevel(from) < userlevel(nuh))
			sendreply("User has a higer level");
		else if(master = userlevel(nuh))
			sendreply("User is level 175.  You cannot delete this user.");
	    	else
                	sendreply("User %s has been deleted", 
				     nuh);
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
		send_to_user(from, "I need a nick ..");
		return;
	}
	if (!(nuh = nick2uh(from, newuser, 1)))
                return;
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
             add_to_levellist(currentbot->lists->shitlist, nuh,
		atoi(newlevel));
             sendreply("User shitted with access %i as %s", 
			     atoi(newlevel), nuh);
	     check_shit();
	}
        return;
}                   
/*
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
*/
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
	char *nuh;
	if(rest)
        {
		if (!(nuh = nick2uh(from, rest, 1)))
                  return;
            	if(!remove_from_levellist(currentbot->lists->shitlist, nuh))
	      		sendreply("User is not shitted!");
            	else
                	sendreply("User %s has been deleted", nuh);
        }
	else
        	send_to_user(from, "Who do you want me to delete?");
        return;
}

void	do_nprotadd(char *from, char *to, char *rest)
{
	char	*newuser;
	char	*newlevel;
	char	*nuh;	/* nick!@user@host */

        if((newuser = get_token(&rest, " ")) == NULL)
	{
		send_to_user(from, "I need a nick ..");
		return;
	}

	if (!(nuh = nick2uh(from, newuser, 1)))
                return;
        if((newlevel = get_token(&rest, "")) == NULL)
	{
                send_to_user(from, "What level should %s have?", newuser);
		return;
	}
        if(atoi(newlevel) < 0)
		send_to_user(from, "level should be >= 0!");
	else if(atoi(newlevel) > 125)
		send_to_user(from, "level should be =< 125!");
	else
	{
           add_to_levellist(currentbot->lists->protlist,
		nuh, atoi(newlevel));
           sendreply("User protected with access %i as %s", 
			     atoi(newlevel), nuh);
	   sendnotice(newuser, "%s has added you to my protection list at level %i",
		getnick(from), atoi(newlevel));
	}
        return;
}                   
/*
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
*/
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
	char *nuh;
        if(rest)
        {
	        if (!(nuh = nick2uh(from, rest, 1)))
                    return;
            	if(!remove_from_levellist(currentbot->lists->protlist, nuh))
           		send_to_user(from, "User is not protected!");
		else
                	send_to_user(from, "User %s has been deleted", nuh);
        }
	else
        	send_to_user(from, "Who do you want me to delete?");
        return;
}

void	do_banuser(char *from, char *to, char *rest)
{

        char *chan, *nick, *nuh;
        chan = findchannel(to, &rest);
        
	if (!(nick = get_token(&rest, " ")))
        {
                sendreply("No nick specified");
                return;
        }
	if (!(nuh = nick2uh(from, nick, 0)))
          return;
        if (userlevel(nuh) > userlevel(from))
        { 
          sendkick(chan, getnick(from), "Ban attempt of %s", nuh);
          return;
        }
        deop_ban(chan, nick, nuh);
        sendreply("%s banned on %s", nick, chan);

}

void	do_showusers(char *from, char *to, char *rest)
{
        char    *chan;

        chan = findchannel(to, &rest);
	show_users_on_channel( from, chan);
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
void	do_jokedeop(char *from, char *to, char *rest)
{
	char *nick;
	char *chan = ischannel( to ) ? to : currentchannel();

	if((nick = get_token(&rest, " ")) == NULL)
		send_to_user( from, "Whats the nick for the fake deop?");
	else if (!rest)
		send_to_user( from, "Whats the nick to be deoped?");
	else
	send_ctcp( chan, "ACTION change \"-o %s\" on channel %s by %s",
		rest, chan, nick );
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
	char *chan;
	chan = findchannel(to, &rest);


        sendreply("Mass Unbanning on %s ..", chan);
        channel_massunban(chan);
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

        char *chan, *nick, *nuh;
        chan = findchannel(to, &rest);
	nick = get_token(&rest, " ");
        if (!i_am_op(chan))
        {
                not_opped(from, chan);
                return;
        }
	if (!nick)
        {
                channel_unban(chan, from);
                sendreply("You have been unbanned on %s", chan);
		return;
	}        
	if (!(nuh = nick2uh(from, nick, 0)))
                return;
        channel_unban(chan, nuh);
        sendreply("%s unbanned on %s", nuh, chan);
 
/*       
if(rest)
        {
		if(!mychannel(rest))
		{
			send_to_user(from, "I'm not on channel %s", rest);
			return;
		}
		else
	                send_to_user(from, "Unbanning you on %s ..", rest);
                        channel_unban(rest, from);
        }
        else if(ischannel(to))
        {
                        send_to_user(from, "Unbanning you on %s ..", to);
                        channel_unban(to, from);
        }
        else
        {
                	send_to_user(from, "Unbanning you on %s ..", currentchannel());
                	open_channel(currentchannel());
	}
*/
}
void	do_kick(char *from, char *to, char *rest)
{
	char	*nuh;
	char 	*nick;
	char	*channel;

  channel = findchannel(to, &rest);

  if((nick=get_token(&rest, " "))==NULL)
    sendreply("Usage: kick <nick> [comment]");
  else if ((nuh=username(nick))==NULL)
    sendreply("%s isnt on this channel.", nick);
  else if (protlevel(nuh) >= 100)
    sendreply("%s is protected!", nuh);
  else
    if (!rest || !*rest)
       splatter(channel, nick);
    else
       sendkick(channel, nick, rest);
}

void	do_listdcc(char *from, char *to, char *rest)
{
	show_dcclist( from );
}
void	global_dccstats(char *from, char *rest)
{
	global_stats( from );
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
	char *chan;
	chan = findchannel(to, &rest);

	if(rest)
                  send_ctcp(chan, "ACTION \002\002%s", rest);
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
}

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
	if(!is_in_list(IgnoreList, rest))
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
		send_to_user(from, "I need a userhost to unignore ..");
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
/* Old Command List */
void    do_command_list(char *from, char *to, char *rest)
{
 
	char buf[MAXLEN];
        char *ltmp;
	int i, lvl, ii;

        ltmp = get_token(&rest, " ");

	if(!ltmp)
	{
	   lvl=userlevel(from);
	   send_to_user(from, "\002 Lvl   Command   Description\002");
	   send_to_user(from, "-----------------------------");

	for(i=0;on_msg_commands[i].userlevel <= lvl;i++)
	{
	   char *command=on_msg_commands[i].name;
	   int level=on_msg_commands[i].userlevel;
	   char *comment=on_msg_commands[i].comment;
	   if(command != NULL && on_msg_commands[i].show == TRUE)
	   {
	    sprintf(buf, " %3i   \002%10s\002 %s", level, command, comment);
	    send_to_user(from, buf);
	   }
	}
	send_to_user(from, "-----------------------------");
	return;
	}
	
	lvl = atoi(ltmp);
	ii = userlevel(from);

	  send_to_user(from, "\002 Lvl   Command   Description\002");
	  send_to_user(from, "-----------------------------");
	  send_to_user(from, "No command list indicates that you do not have");
	  send_to_user(from, "enough access for those commands.");
	  send_to_user(from, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");

	for(i=0;on_msg_commands[i].userlevel <= ii;i++)
	{
	  char *command=on_msg_commands[i].name;
	  int level=on_msg_commands[i].userlevel;
	  char *comment=on_msg_commands[i].comment;
	 if(command != NULL && on_msg_commands[i].show == TRUE &&
		on_msg_commands[i].userlevel == lvl)
	 {
	  sprintf(buf, " %3i   \002%10s\002 %s", level, command, comment);
	  send_to_user(from, buf);
	 }
	}
	
}

void do_discquote(char *from, char *to, char *rest)
{
  FILE *infile;
  int number=0, i;
  char buffer[255];

  infile=fopen(QUOTEFILE, "r");
  while(!feof(infile)) {
    readline(infile, buffer);
    number++;
  }
  number=rand()%number;
  fclose(infile);
  infile=fopen(QUOTEFILE, "r");
  for(i=0;i<number;i++) readline(infile, buffer);
  if(ischannel(to))
  sendprivmsg(to, "%s", buffer);
  else
  sendprivmsg(currentchannel(), "%s", buffer);
}
void do_atopic(char *from , char *to, char *rest)
{
    FILE *infile;

    if(!rest)
       sendreply("I need a topic.");
    else
    {
        sendreply("Added topic: %s",rest);
        if(!(infile=fopen(TOPICFILE, "a")))
        {
            sendreply("Problem saving topic ..");
            return;
        }
        fprintf(infile, "%s\n", rest);
        fclose(infile);
   }
}

void do_disctopic(char *from, char *to, char *rest)
{
  FILE *infile;
  int number=0, i;
  char buffer[255];

  infile=fopen(TOPICFILE, "r");
  while(!feof(infile)) {
    readline(infile, buffer);
    number++;
  }
  number=rand()%number;
  fclose(infile);
  infile=fopen(TOPICFILE, "r");
  for(i=0;i<number;i++) readline(infile, buffer);
  if(ischannel(to))
    send_to_server("TOPIC %s :%s", to, buffer);
  else
    send_to_server("TOPIC %s :%s", currentchannel(), buffer);
}

void do_insult(char *from, char *to, char *rest)
{
  FILE *infile;
  int number=0, i;
  char buffer[255];

  if(!rest)
  {
        send_to_user(from, "I need a nick to insult ..");
        return;
  }
  infile=fopen(INSULTFILE, "r");
  while(!feof(infile)) {
    readline(infile, buffer);
    number++;
  }
  number=rand()%number;
  fclose(infile);
  infile=fopen(INSULTFILE, "r");
  for(i=0;i<number;i++) readline(infile, buffer);
  if(ischannel(to))
    sendprivmsg(to, "%s: %s", rest, buffer);
  else
    sendprivmsg(currentchannel(), "%s: %s", rest, buffer);

}


void    do_8ball(char *from, char *to, char *rest)
{
        char buffer[MAXLEN];

        if(!rest)
        {
                send_to_user(from, "You need to ask me a yes or no question.");
                return;
        }
        switch (rand() % 5) {
        case 0:
                sprintf(buffer, "Of Course!");
                break;
        case 1:
                sprintf(buffer, "Ahh .. No ..");
                break;
        case 2:
                sprintf(buffer, "Ask again later ..");
                break;
        case 3:
                sprintf(buffer, "It is certain ..");
                break;
        case 4:
                sprintf(buffer, "Not a chance.");
                break;
        }
        if(ischannel(to))
                sendprivmsg(to, buffer);
        else
                send_to_user(from, buffer);
}
void    do_dice(char *from, char *to, char *rest)
{
        int i,ii;
        switch (rand() % 6) {
        case 0:
          i=1;
          break;
        case 1:
          i=2;
          break;
        case 2:
          i=3;
          break;
        case 3:
          i=4;
          break;
        case 4:
          i=5;
          break;
        case 5:
          i=6;
          break;
        }
        send_to_user(from, "I rolled a %i", i);
        switch (rand() % 6) {
        case 0:
          ii=1;
          break;
        case 1:
          ii=2;
          break;
        case 2:
          ii=3;
          break;
        case 3:
          ii=4;
          break;
        case 4:
          ii=5;
          break;
        case 5:
          ii=6;
          break;
        }
        send_to_user(from, "You rolled a %i", ii);
        if(i==ii)
                send_to_user(from, "Its a tie!");
	else if(i>ii)
                send_to_user(from, "I Win!");
        else
                send_to_user(from, "You Win!");
}
 
void do_dance(char *from, char *to, char *rest)
{
FILE *infile;
int number=0, i;
char buffer[255];

  infile=fopen(DANCEFILE, "r");
  while(!feof(infile)) {
    readline(infile, buffer);
    number++;
  }
  number=rand()%number;
  fclose(infile);
  infile=fopen(DANCEFILE, "r");
  for(i=0;i<number;i++) readline(infile, buffer);
  if(ischannel(to))
    send_ctcp(to, "ACTION %s", buffer);
  else
    send_ctcp(currentchannel(), "ACTION %s", buffer);
}

void show_inform_list(char *from)
{
   ListStruct *temp;

     for (temp=InformList;temp;temp=temp->next)
       send_to_user(from, temp->name);
}
void send_inform(char *text)
{
   ListStruct *temp;
     for (temp=InformList;temp;temp=temp->next)
       sendnotice(temp->name, text);
}

void do_inform(char *from, char *to, char *rest)
{
        add_to_list(&InformList, getnick(from));
        send_to_user(from, "Now informing of kicks and deops.");
}

void do_noinform(char *from, char *to, char *rest)
{
        remove_from_list(&InformList, getnick(from));
        send_to_user(from, "No longer informing you.");
}

void do_informlist(char *from, char *to, char *rest)
{
        send_to_user(from, "--- Inform List ---", rest);
        show_inform_list(from);
        send_to_user(from, "--- end of list ---");
}

void    do_ascii(char *from, char *to, char *rest)
{
    FILE *infile;
    char *out;
    char *tmp;

        if(!rest)
                infile=fopen(LIBFILE, "r");
        else
	{
	if (strchr(rest, '/')) send_to_user(from, "Your a Lamer Hack Something Else!");
	else
	{
tmp=(char *)malloc((strlen(PICDIR)+strlen(rest))*sizeof(char));
		sprintf(tmp, "%s%s", PICDIR, rest);
		infile=fopen(tmp, "r");
	}
    if(infile)
    {
          while((out=get_ftext(infile)))
          send_to_user(from, out);
          fclose(infile);
          return;
    }
        send_to_user(from, "Invalid Pic. For a list use \"%cascii\" ..",
              currentbot->cmdchar);
   }
}


void do_lyrics(char *from, char *to, char *rest)
{
FILE *infile;
char *tmp, *band, *p, *sec_chk, *sec_tmp;
	if(!rest || (band = get_token(&rest, ":"))==NULL)
		send_to_user(from, "Usage: %clyrics Band Name: \".. Lyrics they Sing ..\"",
			currentbot->cmdchar);
	else
	if(strchr(band, '/'))
	{
	send_to_user(from, "\002Band Names containing /'s are not allowed\002");
	send_to_user(from, "Please try a different way to hack LaMeR...");
	}
	else
	{
	for(p=band; *p; p++) *p=tolower(*p);
	send_to_user(from, "Name: %s", band);
	send_to_user(from, "Lyrics: %s", rest);
	send_to_user(from, "Thanks!");
	tmp=(char *)malloc((strlen(SONGDIR)+strlen(rest))*sizeof(char));
	sprintf(tmp, "%s%s", SONGDIR, band);
	if((infile=fopen(tmp, "a"))==NULL || !rest)
        {
            send_to_user(from, "Problem saving lyrics .. :/");
            return;
        }
	fprintf(infile, "%s\n", rest);
	fclose(infile);
	}
}


void do_sing(char *from, char *to, char *rest)
{
 FILE *infile;
 int number=0, i;
 char *tmp, *p;
 char buffer[255];
 char *chan;
 CHAN_list *Chan;

 if (!ischannel(to))
 {
      sendreply("You must issue this command to a channel.");
      return;
 }
 if (get_singtog(to))
 {
   if (!rest)
   {
  	infile=fopen(LYRICFILE, "r");
         while((p=get_ftext(infile)))
         send_to_user(from, p);
         fclose(infile);
         return;
   }
    if (!strchr(rest, '/')) {
     for(p=rest; *p; p++) *p=tolower(*p);
      tmp=(char *)malloc((strlen(SONGDIR)+strlen(rest))*sizeof(char));
      sprintf(tmp, "%s%s", SONGDIR, rest);
      infile=fopen(tmp, "r");
      if(!infile)
    {
     sendreply("I don't know that group. Use \"sing\" to get a list of bands I know.");
     return;
    }
    while(!feof(infile)) {
      readline(infile, buffer);
      number++;
    }
    number=rand()%number;
    fclose(infile);
    infile=fopen(tmp, "r");
    for(i=0;i<number;i++) readline(infile, buffer);
       sendprivmsg(to, "\002\002%s", buffer);
   }
   else
    sendreply("Your a Lamer.. Hack Something else !");
  }
  else
   sendreply("Sorry, my throat is sore ..");
}


void do_addks(char *from, char *to, char *rest)
{
        char ks[MAXLEN];
        if (!rest)
        {
                sendreply("Specify a string of text to kick upon when said");
                return;
        }
        strcpy(ks, "*");
        strcat(ks, rest);
        strcat(ks, "*");
        if (is_in_list(KickSays, ks))
        {
                sendreply("Already kicking on the say of that");
                if (KickSayChan == NULL)
                    sendreply("WARNING: No channels selected for auto-kick");
                return;
        }
        if (add_to_list(&KickSays, ks))
        {
                send_to_user( from, "Now kicking on the say of %s", ks);
                if (KickSayChan == NULL)
		{
		  if (ischannel(to))
		        if (add_to_list(&KickSayChan, to))
			  sendreply("Setting kicksay channel to %s", to);
		  else
			if (add_to_list(&KickSayChan, currentchannel()))
			  sendreply("Setting kicksay channel to %s", currentchannel());
/*         sendreply("WARNING: No channels selected for auto-kick");   */
		}
        }
        else
                send_to_user( from, "Problem adding the kicksay");
        return;
}


void do_addksc(char *from, char *to, char *rest)
{
        if (!rest || !ischannel(rest))
        {
	  send_to_user(from, "I need a channel name ..");
          return;
        }
        if (!mychannel(rest) && (*rest != '*'))
          {
	    send_to_user(from, "I'm not on that channel ..");
            return;
          }
        if (is_in_list(KickSayChan, rest))
        {
                send_to_user( from, "Already auto-kicking on that channel");
                return;
        }
        if (add_to_list(&KickSayChan, rest))
                send_to_user( from, "Now auto-kicking on %s", rest);
        else
                send_to_user( from, "Problem adding the channel");
        return;
}


void do_clrks(char *from, char *to, char *rest)
{
        if (KickSays == NULL)
        {
                send_to_user( from, "There are no kicksays");
                return;
        }
        remove_all_from_list( &KickSays);
        send_to_user( from, "All kicksays have been removed");
        return;
}

void do_clrksc(char *from, char *to, char *rest)
{
        if (KickSayChan == NULL)
        {
                send_to_user( from, "There are no kicksay channels");
                return;
        }
        remove_all_from_list( &KickSayChan);
        send_to_user( from, "All channels have been removed");
        return;
}
void do_delks(char *from, char *to, char *rest)
{
        char ks[MAXLEN];
        if (!rest)
        {
                send_to_user( from, "Specify the string of text to delete");
                return;
        }
        strcpy(ks, rest);
        if (!is_in_list(KickSayChan, ks))
        {
                send_to_user( from, "I'm not auto-kicking for that");
                return;
        }
        if (remove_from_list(&KickSayChan, ks))
                send_to_user( from, "The kicksay has been deleted");
        else
                send_to_user( from, "Problem deleting the kick say");
        return;
}
void do_listks(char *from, char *to, char *rest)
{
        ListStruct *temp;
        int i = 0;
        if (KickSays == NULL)
        {
                send_to_user( from, "There are currently no kicksays");
                return;
        }
        send_to_user( from, "--- List of Kicksays: ---" );
        for (temp = KickSays; temp; temp = temp->next)
        {
                i++;
                send_to_user( from, "%2i -- %s", i, temp->name);
        }
        send_to_user( from, "--- End of list ---");
        return;
}
void do_listksc(char *from, char *to, char *rest)
{
        ListStruct *temp;
        int i = 0;
        if (KickSayChan == NULL)
        {
                send_to_user( from, "There are currently no kicksay channels");
                return;
        }
        send_to_user( from, "--- List of Kicksay channels: ---" );
        for (temp = KickSayChan; temp; temp = temp->next)
        {
                i++;
                send_to_user( from, "%2i -- %s", i, temp->name);
        }
        send_to_user( from, "--- End of list ---");
        return;
}

void do_delksc(char *from, char *to, char *rest)
{
        char ks[MAXLEN];
        if (!rest)
        {
	  send_to_user(from, "I need a channel name ..");
          return;
        }
        strcpy(ks, rest);
        if (!is_in_list(KickSayChan, ks))
        {
                send_to_user( from, "I'm not auto-kicking on that channel");
                return;
        }
        if (remove_from_list(&KickSayChan, ks))
                send_to_user( from, "The channel has been deleted");
        else
                send_to_user( from, "Problem deleting the channel");
        return;
}



void    show_news(char *from, char *to, char *rest)
{
    FILE    *f;
    char    *s;

          f=fopen(NEWSFILE, "r");
            while((s=get_ftext(f)))
                send_to_user(from, s);
          fclose(f);
}

int add_spy_chan(char *name, char *nick)
{
        CHAN_list *Channel;
        if ( (Channel = search_chan( name )) != NULL)
                return add_to_list(&(Channel->spylist), nick);
        return FALSE;
}
int del_spy_chan(char *name, char *nick)
{
        CHAN_list *Channel;
        if ( (Channel = search_chan( name )) != NULL)
                return remove_from_list(&(Channel->spylist), nick);
        return FALSE;
}
void show_spy_list(char *from, char *name)
{
   ListStruct *temp;
 
   CHAN_list *Channel;
   if ((Channel = search_chan(name)) != NULL)
     for (temp=Channel->spylist;temp;temp=temp->next)
       send_to_user(from, temp->name);
}
void send_spy_chan(char *name, char *text)
{
        ListStruct *temp;
        CHAN_list *Channel;
        if ((Channel = search_chan( name )) != NULL)
                for (temp=Channel->spylist;temp;temp=temp->next)
                        sendnotice(temp->name, text);
}
int is_spy_chan(char *name, char *nick)
{
        CHAN_list *Channel;
        if ( (Channel = search_chan( name )) != NULL)
                return is_in_list(Channel->spylist, nick);
        return FALSE;
}

void do_spy(char *from, char *to, char *rest)
{
        if (!rest)
        {
                send_to_user(from,
                         "The least you could do is give me a channel name");
                return;
        }        
        if (!mychannel(rest))
        {
                send_to_user(from, "I'm not on that channel");
                return;
        }        
        if (add_spy_chan(rest, getnick(from)))
                send_to_user(from, "Now spying on %s for you", rest);
        else    
                send_to_user(from, "You are already spying on %s", rest);
        return;
}
 
void do_rspy(char *from, char *to, char *rest)
{
        if (!rest)
        {
                send_to_user(from,
                         "The least you could do is give me a channel name");
                return;
        }
        if (!mychannel(rest))
        {
                send_to_user(from, "I'm not on that channel");
                return;
        }
        if (del_spy_chan(rest, getnick(from)))
                send_to_user(from, "No longer spying on %s for you", rest);
        else    
                send_to_user(from, "You are not spying on %s", rest);
        return;
}
 
void do_spylist(char *from, char *to, char *rest)
{
        if(!rest)
        {
	  send_to_user(from, "I need a channel name ..");
          return;
        }
 
        send_to_user(from, "--- Spylist for %s", rest);
        show_spy_list(from, rest);
        send_to_user(from, "--- end of list ---");
}
 
void do_spymsg(char *from, char *to, char *rest)
{
        if (is_in_list(SpyMsgList, getnick(from)))
        {
                send_to_user( from, "Already redirecting messages to you");
                return;
        }
        if (add_to_list(&SpyMsgList, getnick(from)))
                send_to_user( from, "Now redirecting messages to you");
        else    
                send_to_user( from, "Error: Can't redirect you the messages");
        return;
}
void do_nospymsg(char *from, char *to, char *rest)
{
        if (!is_in_list(SpyMsgList, getnick(from)))
        {
                send_to_user( from, "I'm not redirecting messages to you");
                return;
        }
        if (remove_from_list(&SpyMsgList, getnick(from)))
                send_to_user( from, "No longer redirecting messages to you");
        else    
                send_to_user( from, "Error: Can't stop redirecting you the messages");
        return;
}


void do_chanlog(char *chan, char *rest)
{
	CHAN_list *Channel;
        FILE	*lfile;
	char	file[MAXLEN];
	char	*p;

	if ((Channel = search_chan(chan)) != NULL)
        {
		for(p=chan; *p; p++) *p=tolower(*p);
	 	sprintf(file, "%s.%s", LOGFILE, chan);
                if((lfile = fopen( file, "a")) == NULL)
                        return;
                fprintf(lfile, "%s\n", rest);
                fclose(lfile);
        }
}
void	command_log(char *from, char *to, char *command)
{
	FILE *lfile;
	char file[MAXLEN];
	char blah[MAXLEN];

	sprintf(file, "%s.command", LOGFILE);
	if((lfile = fopen( file, "a")) == NULL)
	      return;
	sprintf(blah, "%s %s :%s", from, to, command);
	fprintf(lfile, "%s\n", blah);
	fclose(lfile);
}
void splatter(char *channel, char *nick)
{
  FILE *infile;
  int i=0, ii;
  char buffer[MAXLEN];


  infile=fopen(SPLATFILE, "r");
  while(!feof(infile)) {
    readline(infile, buffer);
    i++;
  }
  i=rand()%i;
  fclose(infile);
  infile=fopen(SPLATFILE, "r");
  for(ii=0;ii<i;ii++)
    readline(infile, buffer);
  sendkick(channel, nick, buffer);
}

void    do_server(char *from, char *to, char *rest)
{
	char *server;
	char buffer[MAXLEN];
	int port;

	if ((server = get_token(&rest, " ")))
	{
          if (rest)
                        port = atoi(rest);
          else
          	        port = 6667;

	  sprintf(buffer, "Changing to server %s", server);
          sendquit(buffer);
          if (!addserver(server, port) || !set_server(server))
          {
                	sendreply("Problem switching servers");
                	return;
          }
	  connect_to_server();
          return;
        }
        else
	{
		sendreply("Usage: %cserver <server> [port]", currentbot->cmdchar);
		return;
	}
}

void do_chaccess(char *from, char *to, char *rest)
{

  int i, oldlevel, newlevel;
  char *name, *level;

  name = get_token(&rest, " ");
  level = get_token(&rest, " ");

  if (!level && !name)
  {
     send_to_user(from, "Usage: chaccess <commandname> [level]");
     return;
  }
  
  if (level)
  {
     newlevel = atoi(level);
     if ((newlevel < 0) || (newlevel > 150))
     {
         send_to_user(from, "Level must be between 0 and 150");
         return;
     }
  }
  else
     newlevel = -1;
  if (newlevel > userlevel(from))
  {
     send_to_user(from, "Can't change level to one higher than yours");
     return;
  }
  for (i = 0; on_msg_commands[i].name; i++ )
  {
     if (STRCASEEQUAL(on_msg_commands[i].name, name))
     {
        oldlevel = on_msg_commands[i].userlevel;
        if (newlevel == -1)
        {
          send_to_user(from, "The access level needed for that command is %i",
                             oldlevel);
          send_to_user(from, "To change it, specify a level");
          return;
        }
        if ((oldlevel > userlevel(from)) || (oldlevel == 175))
        {
          send_to_user(from, "Can't change a level that is higher than yours.");
	  send_to_user(from, "Also, 175 level commands cannot be adjusted.");
          return;
        }
        if (oldlevel == newlevel)
             send_to_user(from, "The level was not changed");
        else
             send_to_user(from, "Level changed from %i to %i",
                oldlevel, newlevel);
        on_msg_commands[i].userlevel = newlevel;
        return;
        }
  }
  send_to_user(from, "That command is not known");
  return;
}

void do_restrict(char *from, char *to, char *rest)
{
     int newlevel;

     if (!rest)
     {
          if (currentbot->restrict)
             sendreply("I'm restricted to level %i",
               currentbot->restrict);
          else
             sendreply("I'm not being restricted to any level");
          return;
     }
     newlevel = atoi(rest);
     if (newlevel < 0)
         sendreply("Restrict level must be >= 0");
     else if (newlevel > 175)
        sendreply("Restrict level must be <= 175");
     else if (newlevel > userlevel(from))
        sendreply("I can't restrict to a higher level than your user level");
     else
     {
        if (newlevel)
           sendreply("Now restricted at level %i", newlevel);
        else
           sendreply("Not restricting access");
        currentbot->restrict = newlevel;
     }
     return;
}

