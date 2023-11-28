#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#ifndef MSDOS
	#include <strings.h>
#endif
#include <time.h>
#include <signal.h>
#include "config.h"

#include "stelbot.h"
#include "channel.h"
#include "userlist.h"
#include "dcc.h"
#include "ftext.h"
#include "misc.h"
#include "commands.h"

#define getrandom( min, max ) ((rand() % (int)(((max)+1) - (min))) + (min))

extern StelBotStruct bot;
extern char oldnick[MAXNICKLEN];
extern char lastcmd[1024];
extern char current_nick[MAXLEN];
extern char globsign;
extern long uptime;
extern int maxsameuser, malevel, klogtoggle, mutoggle;
extern UserStruct *UserList, *ShitList;
extern ListStruct *KickSays, *KickSayChan, *LogChatList;
extern ListStruct *SpyMsgList;
extern ListStruct *StealList;
extern UserHostStruct *UserHostList;
extern char cmdchar;
extern int beeptoggle;
extern int pubtoggle, idlelimit, idletoggle, sdtoggle, prottoggle, seentoggle;
extern int kslevel, weltoggle, soptoggle, cctoggle, logtoggle, pmlogtoggle;
extern int aoptoggle, shittoggle, enfmtoggle, floodtoggle, masstoggle;
extern int masskicklimit, massdeoplimit, massbanlimit, floodlimit;
extern int minoplevel, minbanlevel, mindeoplevel, minunbanlevel;
extern char *progname, ircserver[FNLEN], helpfile[FNLEN], killfile[FNLEN];
extern char filelistfile[FNLEN], bot_dir[FNLEN], userhostfile[FNLEN];
extern char filedispfile[FNLEN], settingsfile[FNLEN], pmlogfile[FNLEN];
extern char shitfile[FNLEN], usersfile[FNLEN], botlogfile[FNLEN];
extern char mailfile[FNLEN], serversfile[FNLEN], kinitfile[FNLEN];
extern int holdingnick, holdnick;
extern char download_dir[FNLEN];
extern char nhmsg[255], ownernick[MAXNICKLEN], owneruhost[255], botdate[15];
extern char lastcmds[10][100];
extern char welcomemess[10][50];
extern int numwelcome;

int pubseen = 0;

struct
{
	char  *name;
	void  (*function)();
	int   userlevel;
	int   shitlevel;
	int   forcedcc;
	int   cmdchar;
} on_msg_commands[] =

/* Any command which produces more than 2 lines output can be 
	used to make the bot "excess flood". Make it "forcedcc"     */

/*   Command        function   userlvl   shitlvl  forcedcc cmdchar_needed*/
{
	{ "ACCESS",   do_access,      0,   100,   FALSE,     TRUE },
	{ "GETMSG",   do_getmsg,      0,   100,   FALSE,     TRUE },
	{ "SCANMSG",  do_scanmsg,     0,   100,    TRUE,     TRUE },
	{ "HELP",     do_help,        0,   100,    TRUE,     TRUE },
        { "BOT",      do_bot,         0,   100,   FALSE,    FALSE },
        { "GOTCHA",   do_die,         0,   100,   FALSE,     TRUE },
        { "BOTINFO",  do_botinfo,     0,   100,    TRUE,    FALSE }, 
        { "STATS",    do_stats,       0,   100,    TRUE,     TRUE },
	{ "SEEN",     do_seen,        0,   100,   FALSE,     TRUE },
	{ "INFO",     do_info,        0,   100,    TRUE,     TRUE },
	{ "TIME",     do_time,        0,   100,   FALSE,     TRUE },
	{ "PING",     do_ping,        0,   100,   FALSE,     TRUE },
        { "LISTP",    do_listp,       0,   100,    TRUE,     TRUE },
        { "CHAT",     do_chat,        0,   100,   FALSE,     TRUE },
	{ "DOWN",     do_deopme,      0,   100,   FALSE,    FALSE },
	{ "USERHOST", do_getuserhost, 0,   100,   FALSE,     TRUE },
	{ "TOTSTATS", do_totstats,    0,   100,    TRUE,     TRUE },
	{ "TOPTEN",   do_top10kills,  0,   100,    TRUE,     TRUE },
	{ "DISPLAY",  do_display,     0,   100,    TRUE,     TRUE },
	{ "UP",       do_opme,       20,     0,   FALSE,    FALSE },
        { "LIST20",   do_list20,     20,     0,    TRUE,     TRUE }, 
        { "CHANNELS", show_channels, 20,     0,    TRUE,     TRUE },
	{ "LKS",      do_listks,     20,     0,    TRUE,     TRUE },
	{ "LKSC",     do_listksc,    20,     0,    TRUE,     TRUE },
	{ "GETCH",    do_getch,      20,   100,   FALSE,     TRUE },
	{ "B",        do_ban,        30,     0,   FALSE,     TRUE },
        { "LIST30",   do_list30,     30,     0,    TRUE,     TRUE },
        { "UB",       do_unban,      30,     0,   FALSE,     TRUE },
	{ "OP",       do_op,         30,     0,   FALSE,     TRUE },
	{ "DEOP",     do_deop,       30,     0,   FALSE,     TRUE },
	{ "K",        do_kick,       30,     0,   FALSE,     TRUE },
	{ "KB",       do_kickban,    40,     0,   FALSE,     TRUE },
	{ "INVITE",   do_invite,     40,     0,   FALSE,     TRUE },
        { "LIST40",   do_list40,     40,     0,    TRUE,     TRUE },
        { "UBM",      do_unbanme,    40,     0,   FALSE,     TRUE },
	{ "TOPIC",    do_topic,      40,     0,   FALSE,     TRUE },
        { "LIST50",   do_list50,     50,     0,    TRUE,     TRUE },
        { "SCREW",    do_screwban,   50,     0,   FALSE,     TRUE },
	{ "FUCK",     do_fuckban,    60,     0,   FALSE,     TRUE },
        { "LIST60",   do_list60,      60,     0,    TRUE,     TRUE },
        { "SB",       do_siteban,    60,     0,   FALSE,     TRUE },
	{ "SKB",      do_sitekb,     60,     0,   FALSE,     TRUE },
	{ "SM",       do_sendmsg,     0,     0,   FALSE,     TRUE },
	{ "SPYMSG",   do_spymsg,     70,     0,   FALSE,     TRUE },
	{ "RSPYMSG",  do_nospymsg,   70,   100,   FALSE,     TRUE },
	{ "SPY",      do_spy,        70,     0,   FALSE,     TRUE },
        { "LIST70",   do_list70,     70,     0,    TRUE,     TRUE },
        { "RSPY",     do_rspy,       70,   100,   FALSE,     TRUE },
        { "TOGLIST",  do_toglist,    70,     0,    TRUE,     TRUE },
        { "TOGAOP",   do_togaop,     70,   100,   FALSE,     TRUE },
        { "SETLIST",  do_setlist,    70,     0,    TRUE,     TRUE },
        { "TOGSHIT",  do_togshit,    70,     0,   FALSE,     TRUE },
	{ "TOGPUB",   do_togpub,     70,     0,   FALSE,     TRUE },
	{ "AOP",      do_aop,        70,   100,   FALSE,     TRUE },
	{ "RAOP",     do_naop,       70,   100,   FALSE,     TRUE },
	{ "USERLIST", do_userlist,   70,   100,    TRUE,     TRUE },
	{ "SHITLIST", do_shitlist,   70,   100,    TRUE,     TRUE },
	{ "PROTLIST", do_protlist,   70,   100,    TRUE,     TRUE },
	{ "DCCLIST",  do_listdcc,    70,   100,    TRUE,     TRUE },
	{ "SETSBT",   do_setsbt,     70,   100,   FALSE,     TRUE },
	{ "SAY",      do_say,        80,     0,   FALSE,     TRUE },
	{ "ME",       do_me,         80,     0,   FALSE,     TRUE },
	{ "CKS",      do_clrks,      80,     0,   FALSE,     TRUE },
	{ "CLVL",     do_clvl,       80,     0,   FALSE,     TRUE },
	{ "CKSC",     do_clrksc,     80,     0,   FALSE,     TRUE },
	{ "AKSC",     do_addksc,     80,     0,   FALSE,     TRUE },
	{ "RKSC",     do_delksc,     80,     0,   FALSE,     TRUE },
	{ "KS",       do_addks,      80,     0,   FALSE,     TRUE },
	{ "RKS",      do_delks,      80,     0,   FALSE,     TRUE },
	{ "MSG",      do_msg,        80,     0,   FALSE,     TRUE },
	{ "OPEN",     do_open,       80,     0,   FALSE,     TRUE },
	{ "JOIN",     do_join,       80,     0,   FALSE,     TRUE },
	{ "CYCLE",    do_cycle,      80,     0,   FALSE,     TRUE },
	{ "LEAVE",    do_leave,      80,     0,   FALSE,     TRUE },
	{ "NICK",     do_nick,       80,     0,   FALSE,     TRUE },
        { "LIST80",   do_list80,     80,     0,    TRUE,     TRUE },
        { "CNU",      do_add,        99,     0,   FALSE,     TRUE },
	{ "REM",      do_userdel,    99,     0,   FALSE,     TRUE },
	{ "SHIT",     do_shit,       80,     0,   FALSE,     TRUE },
	{ "RSHIT",    do_nshit,      80,     0,   FALSE,     TRUE },
	{ "PROT",     do_prot,       80,     0,   FALSE,     TRUE },
	{ "RPROT",    do_nprot,      80,     0,   FALSE,     TRUE },
	{ "SHOWUSERS",do_showusers,  80,     0,    TRUE,     TRUE },
	{ "SETFL",    do_setfl,      80,     0,   FALSE,     TRUE },
	{ "SETMOL",   do_setmol,     80,     0,   FALSE,     TRUE },
	{ "SETMDOL",  do_setmdol,    80,     0,   FALSE,     TRUE },
	{ "SETMBL",   do_setmbl,     80,     0,   FALSE,     TRUE },
	{ "SETMUBL",  do_setmubl,    80,     0,   FALSE,     TRUE },
	{ "TOGPROT",  do_togprot,    80,     0,   FALSE,     TRUE },
	{ "TOGWM",    do_togwm,      80,     0,   FALSE,     TRUE },
	{ "SPYLIST",  do_spylist,    90,     0,   FALSE,     TRUE },
	{ "TOGIK",    do_togik,      90,     0,   FALSE,     TRUE },
	{ "TOGSD",    do_togsd,      90,     0,   FALSE,     TRUE },
	{ "SETIKL",   do_setikl,     90,     0,   FALSE,     TRUE },
        { "LIST90",   do_list90,     90,     0,    TRUE,     TRUE },
        { "SETMAL",   do_setmal,     90,     0,   FALSE,     TRUE },
	{ "SETMPL",   do_setmpl,     90,     0,   FALSE,     TRUE },
	{ "STEAL",    do_steal,      90,     0,   FALSE,     TRUE },
	{ "RSTEAL",   do_nosteal,    90,     0,   FALSE,     TRUE },
	{ "TOGCC",    do_togcc,      90,     0,   FALSE,     TRUE },
	{ "MKNU",     do_masskicknu, 80,     0,   FALSE,     TRUE },
	{ "MOPU",     do_massopu,    80,     0,   FALSE,     TRUE },
	{ "MDEOPNU",  do_massdeopnu, 80,     0,   FALSE,     TRUE },
/*
	{ "EMAIL",    do_email,      85,     0,   FALSE,     TRUE },
*/
	{ "MOP",      do_massop,     85,     0,   FALSE,     TRUE },
	{ "MDEOP",    do_massdeop,   85,     0,   FALSE,     TRUE },
        { "LIST85",   do_list85,     85,     0,    TRUE,     TRUE },
        { "MK",       do_masskick,   85,     0,   FALSE,     TRUE },
	{ "MKB",      do_masskickban,85,     0,   FALSE,     TRUE },
	{ "UBA",      do_massunban,  85,     0,   FALSE,     TRUE },
	{ "SETMASS",  do_setmass,    85,     0,   FALSE,     TRUE },
	{ "SETKSL",   do_setksl,     85,     0,   FALSE,     TRUE },
	{ "TOGSEEN",  do_togseen,    90,     0,   FALSE,     TRUE },
	{ "TOGENFM",  do_togenfm,    90,     0,   FALSE,     TRUE },
	{ "TOGMASS",  do_togmass,    90,     0,   FALSE,     TRUE },
	{ "TOGFLOOD", do_togflood,   90,     0,   FALSE,     TRUE },
	{ "ENFMODES", do_enfmodes,   90,   100,   FALSE,     TRUE },
	{ "SEND",     do_send,        0,   100,   FALSE,     TRUE },
	{ "FILES",    do_flist,       0,   100,    TRUE,     TRUE },
	{ "FLIST",    do_flist,       0,   100,    TRUE,     TRUE },
	{ "CMDCHAR",  do_cmdchar,    90,     0,   FALSE,     TRUE },
	{ "SERVER",   do_server,     90,     0,   FALSE,     TRUE },
	{ "NEXTSERVER",do_nextserver,90,     0,   FALSE,     TRUE },
	{ "RUH",      do_removeuh,   90,     0,   FALSE,     TRUE },
	{ "SAVEUH",   do_saveuserhost, 99,   0,   FALSE,     TRUE },
	{ "LASTLOG",  do_fakemsg,    95,     0,   FALSE,     TRUE },
	{ "KINIT",    do_kinit,      95,   100,   FALSE,     TRUE },
	{ "HEART",    do_heart,      95,     0,   FALSE,     TRUE },
        { "LIST95",   do_list95,     95,     0,    TRUE,     TRUE },
        { "LIST99",   do_list99,     99,     0,    TRUE,     TRUE },
        { "LAST10",   do_last10,     99,     0,    TRUE,     TRUE },
	{ "CHACCESS", do_chaccess,   95,     0,   FALSE,     TRUE },
	{ "TOGSO",    do_togso,      95,     0,   FALSE,     TRUE },
	{ "TOGLOG",   do_toglog,     95,     0,   FALSE,     TRUE },
	{ "TOGPMLOG", do_toglogpm,   90,     0,   FALSE,     TRUE },
	{ "REREAD",   do_reread,     95,     0,   FALSE,     TRUE },
	{ "TOGKLOG",  do_toglogk,    99,     0,   FALSE,     TRUE },
	{ "TOGBK",    do_togbk,      90,     0,   FALSE,     TRUE },
	{ "TOGMUK",   do_togmuk,     90,     0,   FALSE,     TRUE },
	{ "SETMUL",   do_setmul,     90,     0,   FALSE,     TRUE },
	{ "LOGCHAT",  do_logchat,    99,     0,   FALSE,     TRUE },
	{ "RLOGCHAT", do_rlogchat,   99,     0,   FALSE,     TRUE },
	{ "HN",       do_holdnick,   99,   100,   FALSE,     TRUE },
	{ "RHN",      do_noholdnick, 99,   100,   FALSE,     TRUE },
	{ "RESETUH",  do_resetuhost, 99,   100,   FALSE,     TRUE },
	{ "LOADUSERS",do_loadusers,  99,     0,   FALSE,     TRUE },
	{ "LOADSHIT", do_loadshit,   99,     0,   FALSE,     TRUE },
	{ "KEEPU",    do_saveusers,  99,     0,   FALSE,     TRUE },
	{ "KEEPS",    do_saveshit,   99,     0,   FALSE,     TRUE },
	{ "DO",       do_do,         99,     0,   FALSE,     TRUE },
	{ "SPAWN",    do_spawn,      99,     0,   FALSE,     TRUE },
	{ "KILL",     do_die,        99,     0,   FALSE,     TRUE },
	/*
	 :
	 :
	 */
	{ NULL,     null(void(*)()), 0,  0, FALSE }
};

void on_join( char *who, char *channel )
{
	UserHostStruct *user;
	char buffer[MAXLEN];

 sprintf(buffer, "\x2%s has joined channel %s\x2", who, channel);
 send_spy_chan(channel, buffer);

	if (!userlevel(who) && mutoggle && (num_on_channel(channel, who) > maxsameuser))
	  {
	    char n[100], u[100], h[100], m[100];
	    sep_userhost(who, n, u, h, m);
	    sendprivmsg(channel, "\x2I smell something fishy...too many people with the user: %s\x2", fixuser(u));
	    sprintf(m, "*!*%s@*", fixuser(u));
	    channel_masskickban(channel, m);
	  }
			  
	if (is_in_list(LogChatList, channel))
	  {
	    char buffer[MAXLEN];
	    sprintf(buffer, "%s has joined channel %s", who, channel);
	    logchat(channel, buffer);
	  }
	update_userhostlist(who, channel, getthetime());
	user = find_userhost(who);
	if ((user) && (user->totmsg > user->msgnum))
	{
	    send_to_user(who, "\x2You have %i new message(s) waiting...\x2",
			user->totmsg - user->msgnum);
		 send_to_user(who, "\x2Use \"%cgetmsg\" to read them\x2",
			      cmdchar);
	}
	if( (shitlevel( who )) && shittoggle)
		shitkick(who, channel);
	if ( userlevel( who ) &&  !shitlevel( who ))
	{
		if (isaop( who ) && (aoptoggle))
			giveop ( channel, getnick (who) );
	}
	return;
}

void on_mode( char *from, char *rest )
{
	int   did_change=FALSE; /* If we have to change anything */
	int   netsplit = FALSE;
	char  *channel;
	char  *mode;

	char  *chanchars; /* i.e. +oobli */
	char  *params;
	char  sign;    /* +/-, i.e. +o, -b etc */

	char  unminmode[MAXLEN];   /* used to undo modechanges */
	char  unminparams[MAXLEN];
	char  unplusmode[MAXLEN];  /* used to undo modechanges */
	char  unplusparams[MAXLEN];

	/* these are here merely to make things clearer */
	char  *nick;
	char  *banstring;
	char  *key;
	char  *limit;
	UserHostStruct *user;
	ShitStruct *temp;
	static time_t currenttime;
	char buffer[MAXLEN];


	currenttime = getthetime();


#ifdef DBUG
	printf("Entered mode, from = %s, rest = %s\n", from, rest);
#endif

	strcpy(unminmode, "");     /* min-modes, i.e. -bi */
	strcpy(unminparams, "");   /* it's parameters */
	strcpy(unplusmode, "");    /* plus-modes, i.e. +oo */
	strcpy(unplusparams, "");  /* parameters */

	channel = strtok(rest, " ");
	if(!strcasecmp(channel, current_nick)) /* if target is me... */
		return;        /* mode NederServ +i etc */


	if (strchr(from, '@') != NULL)
	{
		update_userhostlist(from, channel, currenttime);
		user = find_userhost(from);
	}
	else
	  user = NULL;

	mode = strtok(NULL, "\n\0");

	params = strchr(mode, ' ')+1;
	chanchars = strtok(mode, " ");
	while( *chanchars )
	{
		switch( *chanchars )
		{
		case '+':
			sign = '+';
			globsign = '+';
			break;
		case '-':
			sign = '-';
			globsign = '-';
			break;
		case 'o':
			nick = strtok( params, " " );
			params = strtok(NULL, "\0\n");
			if (!nick)
				break;
			if( sign == '+' )
			{
			  if (!strcasecmp(nick, current_nick))
			    {
			      if (is_in_list(StealList, channel))
				{
				  int temp = malevel;
				  malevel = 99;
				  channel_massdeop(channel, "*!*@*");
				  malevel = temp;
				  remove_from_list(&StealList, channel);
				}
			    }
			  if (user)
				   user->totop++;
				add_channelmode( channel, CHFL_CHANOP, nick );
				if( sdtoggle && (strchr(from, '@') == NULL) && (userlevel(username(nick)) <= 0) )
				{
					strcat(unminmode, "o");
					strcat(unminparams, nick);
					strcat(unminparams, " ");
					netsplit = TRUE;
					did_change = TRUE;
				}
				else
				if (shitlevel(username(nick))>0)
				{
					strcat(unminmode, "o");
					strcat(unminparams, nick);
					strcat(unminparams, " ");
					send_to_user(from, 
						"Sorry, but %s is shitlisted!",
							  nick);
					did_change=TRUE;
				}
				else
				if ( (userlevel(from) < minoplevel) &&
					(!protlevel( username(nick)) ) )
				{
					strcat(unminmode, "o");
					strcat(unminparams, nick);
					strcat(unminparams, " ");
					send_to_user(from,
						 "\x2You don't have enough access to op\x2");
					send_to_user(from,
						 "\x2min op level is %i", '\x2',
						 minoplevel);
					did_change=TRUE;
				}
				else
				if (soptoggle && !userlevel(username(nick)))
				{
					strcat(unminmode, "o");
					strcat(unminparams, nick);
					strcat(unminparams, " ");
					did_change=TRUE;
				}
			}
			else
			{
			   if (user)
			     {
			       user->deopnum++;
			       user->totdeop++;
			     }

				del_channelmode( channel, CHFL_CHANOP, nick );
				if( protlevel(username(nick)) &&
					!shitlevel(username(nick)) &&
					strcasecmp(getnick(from), nick) &&
					strcasecmp(getnick(from), current_nick) )
				{
						strcat(unplusmode, "o");
						strcat(unplusparams, nick);
						strcat(unplusparams, " ");
						botlog(botlogfile, "%s deopped on %s by %s -- attempted reop",
							username(nick), channel, from);
						prot_action(from, username(nick), channel);
						did_change=TRUE;
				}
				else
				if (userlevel(from) < mindeoplevel)
				{
					strcat(unplusmode, "o");
					strcat(unplusparams, nick);
					strcat(unplusparams, " ");
					send_to_user(from,
						 "\x2You don't have enough access to deop\x2");
					send_to_user(from,
						 "%cCurrent min deop level is %i", '\x2',
						 mindeoplevel);
					did_change=TRUE;
				}

			}
			break;
		case 'v':
			nick = strtok( params, " " );
			params = strtok(NULL, "\0\n");
			if (!nick)
			   break;
			if( sign == '+' )
				add_channelmode( channel, CHFL_VOICE, nick );
			else
				del_channelmode( channel, CHFL_VOICE, nick );
			break;
		case 'b':
			banstring = strtok( params, " " );
			params = strtok(NULL, "\0\n");
			if (!banstring)
			    break;
			if( sign == '+' )
			{
			   if (user)
			     {
			       user->bannum++;
			       user->totban++;
			     }
				if (strcasecmp(getnick(from), current_nick))
				{
					if (strcasecmp(getnick(banstring), "*") &&
						 protlevel(username(getnick(banstring))))
					{
						botlog(botlogfile, "%s banned on %s by %s -- attempted unban",
							banstring, channel, from);
						prot_action(from, banstring, channel);
						strcat(unminmode, "b");
						strcat(unminparams, banstring);
						strcat(unminparams, " ");
						did_change = TRUE;
					}
					else if (!strcasecmp(getnick(banstring), "*") &&
						 (protlevel(banstring) || protlevelbm(banstring)))
					{
						botlog(botlogfile, "%s banned on %s by %s -- attempted unban",
							banstring, channel, from);
						prot_action(from, banstring, channel);
						strcat(unminmode, "b");
						strcat(unminparams, banstring);
						strcat(unminparams, " ");
						did_change = TRUE;
					}
					else
					{
						if (userlevel(from) < minbanlevel)
						{
							strcat(unminmode, "b");
							strcat(unminparams, banstring);
							strcat(unminparams, " ");
							did_change = TRUE;
							send_to_user(from,
							"\x2You don't have enough access to ban\x2");
							send_to_user(from,
								"%cCurrent min ban level is %i", '\x2',
								minbanlevel);
						}
					}
				}
				add_channelmode(channel, MODE_BAN, banstring);
			}
			else
			{
				del_channelmode(channel, MODE_BAN, banstring);
				if ( (userlevel(from) < minunbanlevel) &&
					(!userlevel(banstring) ) )
				{
					strcat(unplusmode, "b");
					strcat(unplusparams, banstring);
					strcat(unplusparams, " ");
					did_change = TRUE;
					send_to_user(from,
						 "\x2You don't have enough access to unban\x2");
						send_to_user(from,
						 "%cCurrent min unban level is %i", '\x2',
						 minunbanlevel);
				}
				else
				if (shittoggle && (shitlevelbm(banstring) >= 5))
				{
				     if (temp = find_shitbm(banstring))
				       {
					if (is_in_list(temp->channels, channel))
					{
					  strcat(unplusmode, "b");
					  strcat(unplusparams, temp->userhost);
					  strcat(unplusparams, " ");
					  did_change = TRUE;
					}
				      }
				   }
			}
			break;
		case 'p':
			if( sign == '+' )
			{
				add_channelmode(channel, MODE_PRIVATE, "");
				if (enfmtoggle && (strchr(get_enforced_modes(channel), 'p') == NULL))
				{
					strcat(unminmode, "p");
					did_change = TRUE;
				}
			}
			else
			{
				del_channelmode(channel, MODE_PRIVATE, "");
				if (enfmtoggle && (strchr(get_enforced_modes(channel), 'p') != NULL))
				{
					strcat(unplusmode, "p");
					did_change = TRUE;
				}
			}
			break;
		case 's':
			if( sign == '+' )
			{
				add_channelmode(channel, MODE_SECRET, "");
				if (enfmtoggle && (strchr(get_enforced_modes(channel), 's') == NULL))
				{
					strcat(unminmode, "s");
					did_change = TRUE;
				}
			}
			else
			{
				del_channelmode(channel, MODE_SECRET, "");
				if (enfmtoggle && (strchr(get_enforced_modes(channel), 's') != NULL))
				{
					strcat(unplusmode, "s");
					did_change = TRUE;
				}
			}
			break;
		case 'm':
			if( sign == '+' )
			{
				add_channelmode(channel, MODE_MODERATED, "");
				if (enfmtoggle && (strchr(get_enforced_modes(channel), 'm') == NULL))
				{
					strcat(unminmode, "m");
					did_change = TRUE;
				}
			}
			else
			{
				del_channelmode(channel, MODE_MODERATED, "");
				if ((enfmtoggle && strchr(get_enforced_modes(channel), 'm') != NULL))
				{
					strcat(unplusmode, "m");
					did_change = TRUE;
				}
			}
			break;
		case 't':
			if( sign == '+' )
			{
				add_channelmode(channel, MODE_TOPICLIMIT, "");
				if (enfmtoggle && (strchr(get_enforced_modes(channel), 't') == NULL))
				{
					strcat(unminmode, "t");
					did_change = TRUE;
				}
			}
			else
			{
				del_channelmode(channel, MODE_TOPICLIMIT, "");
				if (enfmtoggle && (strchr(get_enforced_modes(channel), 't') != NULL))
				{
					strcat(unplusmode, "t");
					did_change = TRUE;
				}
			}
			break;
		case 'i':
			if( sign == '+' )
			{
				add_channelmode(channel, MODE_INVITEONLY, "");
				if (enfmtoggle && (strchr(get_enforced_modes(channel), 'i') == NULL))
				{
					strcat(unminmode, "i");
					did_change = TRUE;
				}
			}
			else
			{
				del_channelmode(channel, MODE_INVITEONLY, "");
				if (enfmtoggle && (strchr(get_enforced_modes(channel), 'i') != NULL))
				{
					strcat(unplusmode, "i");
					did_change = TRUE;
				}
			}
			break;
		case 'n':
			if( sign == '+' )
			{
				add_channelmode(channel, MODE_NOPRIVMSGS, "");
				if (enfmtoggle && (strchr(get_enforced_modes(channel), 'n') == NULL))
				{
					strcat(unminmode, "n");
					did_change = TRUE;
				}
			}
			else
			{
				del_channelmode(channel, MODE_NOPRIVMSGS, "");
				if (enfmtoggle && (strchr(get_enforced_modes(channel), 'n') != NULL))
				{
					strcat(unplusmode, "n");
					did_change = TRUE;
				}
			}
			break;
		case 'k':
			key = strtok( params, " " );
			params = strtok(NULL, "\0\n");
			if( sign == '+' )
			{
				add_channelmode(channel, MODE_KEY, key?key:"???");
				if (enfmtoggle && (strchr(get_enforced_modes(channel), 'k') == NULL))
				{
					strcat(unminmode, "k");
					if (key)
						strcat(unminparams, key);
					else
						strcat(unminparams, " ");
					strcat(unminparams, " ");
					did_change = TRUE;
				}
			}
			else
			{
				del_channelmode(channel, MODE_KEY, key?key:"???");
				if (enfmtoggle && (strchr(get_enforced_modes(channel), 'k') != NULL))
				{
					strcat(unplusmode, "k");
					strcat(unplusparams, key);
					strcat(unplusparams, " ");
					did_change = TRUE;
				}
			}
			break;
		case 'l':
			if( sign == '+' )
			{
				limit = strtok( params, " " );
				if (limit && (atoi(limit) != 0))
				   params = strtok(NULL, "\0\n"); 
				add_channelmode(channel, MODE_LIMIT, limit);
				if (enfmtoggle && (strchr(get_enforced_modes(channel), 'l') == NULL))
				{
					strcat(unminmode, "l");
					did_change = TRUE;
				}
			}
			else
			{
				del_channelmode(channel, MODE_LIMIT, "");
				if (enfmtoggle && (strchr(get_enforced_modes(channel), 'l') != NULL))
				{
					strcat(unplusmode, "k");
					strcat(unplusparams, "69 ");
					did_change = TRUE;
				}
			}
			break;
		default:
#ifdef DBUG
			printf("MODE: unknown mode %c", *chanchars);
#endif
			break;
		}
		chanchars++;
	}
	/* restore unwanted modechanges */
	if (netsplit)
		botlog(botlogfile, "Op by net split detected");
	if (did_change)
	{
		botlog(botlogfile, "Enf.Modes: MODE %s +%s-%s %s %s",
			channel, unplusmode, unminmode, unplusparams, unminparams);
		sendmode( channel, "+%s-%s %s %s", unplusmode,
			  unminmode, unplusparams, unminparams);
	}
	if (netsplit)
	  bot_reply(channel, 3);

	if (user)
	  {
	    if (currenttime - user->bantime >= 10)
	      {
		  user->bantime = currenttime;
		  user->bannum = 0;
	      }
	    if (currenttime - user->deoptime >= 10)
	      {
		  user->deoptime = currenttime;
		  user->deopnum = 0;
	      }
	    sprintf(buffer, "\x2NO MASS DEOPS HERE FUCKER\x2");
	    if (user->deopnum >= massdeoplimit)
	      {
		 massprot_action(from, channel, buffer);
		 botlog(botlogfile, "Mass-deop detected by %s", from);
	      }
	    sprintf(buffer, "\x2TRY BANNING NOW, YOU FUCKER!!!\x2");
	    if (user->bannum >= massbanlimit)
	      {
		 massprot_action(from, channel, buffer);
		 botlog(botlogfile, "Mass-ban detected by %s", from);
	       }
	}
	return;
}


void on_serv_notice( char *from, char *to, char *msg)
{
	char *person = NULL, *temp = NULL, *nickname="", *nuh;
	char buffer[MAXLEN];

	person = strstr(msg, "Received KILL message for");
	if (!person)
	   return;
	person = strstr(msg, "for");
	if (!person)
	    nickname = "<UNKNOWN>";
	else
	  {
	    person = strchr(person, ' ');
	    if (person)
	      person++;
	    if (person)
	      {
		nickname = person;
		person = strchr(nickname, '.');
		if (person)
		  {
		    *person = '\0';
		    person++;
		    msg = person;
		  }
	      }
	    else
	      nickname = "<UNKNOWN>";
	  }

	if (person)
	  person = strstr(msg, "From");
	if (!person)
	   return;

	temp = strchr(person, '(');
	if (temp)
	  temp++;
	person = strchr(person, ' ');
	if (!person)
	  return;
	person++;
	if (!person)
	  return;
	person = strtok(person, " ");
	if (!person)
	  return;

	if (strchr(person, '.'))
	  return;
	if (temp && (strrchr(temp, ')')))
	  *strchr(temp, ')') = '\0';
	if (!temp)
	  temp = person;

	nuh = getuserhost(person);
	if (!*nuh)
	  nuh = getuserhost2(person);
	if (!*nuh)
	  return;
	sprintf(buffer, "%s!%s", person, nuh);
	on_kill(buffer, nickname, temp);
}

void on_kill(char *from, char *nick, char *reason)
{
  UserHostStruct *user;
  int temp;

  update_userhostlist( from, "", getthetime());
  user = find_userhost(from);
  if (user)
    {
      user->totkill++;
      user->killtime = getthetime();
    }
  if (klogtoggle)
    {
      temp = logtoggle;
      logtoggle = 1;
      botlog(killfile, "%s killed %s (%s)", from, nick, reason);
      logtoggle = temp;
    }
}

void on_msg( char *from, char *to, char *msg )
{
	int   i, ii, to_all, temp;
	static  time_t currenttime;
	char  *command;
	char bigbuf[255];
	char  *rest;
	UserHostStruct *user;

#ifndef MSDOS
	DCC_list *userclient;
#endif


	currenttime = getthetime();
#ifdef DBUG
	printf("Entered on_msg-\n");
	printf("from=%s\nto=%s\nmsg=%s\n", from, to, msg);
#endif

	for( ; *msg && *msg == ' '; msg++ );
	if( ( rest = strpbrk( msg, SEPERATORS ) ) != NULL )
		rest++;
	else
		rest = "\0";
	for( ; *rest && *rest == ' '; rest++ );
	strcpy(bigbuf, rest);
	rest = bigbuf;
	if( ( command = strtok( msg, SEPERATORS ) ) == NULL )
		return;        /* NULL-command */
	if (strstr( command, "\x2\x2\x2\x2\x2\x2"))
	  {
	    send_to_user(from, "\x2My command character is %c\x2", cmdchar);
	    return;
	  }
	else
	  to_all = FALSE;

	if (ischannel(to))
	{
		update_userhostlist(from, to, currenttime);
		update_idletime(to, from);
	}
	if (ischannel(to) && (*command == cmdchar) && !pubtoggle)
		return;
	for( i = 0; on_msg_commands[i].name != NULL; i++ )
	{
		if ((!strcasecmp( on_msg_commands[i].name, command+1) &&
			(*command == cmdchar )) ||
				(!strcasecmp( on_msg_commands[i].name, command) &&
					(!on_msg_commands[i].cmdchar || !cctoggle) ))
		{
			if( (userlevel(from) >= on_msg_commands[i].userlevel) &&
				(shitlevel(from) <= on_msg_commands[i].shitlevel) )
			{
#ifndef MSDOS
				userclient = search_list( "chat", from, DCC_CHAT, 1 );
				if( on_msg_commands[i].forcedcc &&
					(!userclient || (userclient->flags&DCC_WAIT)))
				{
#ifdef AUTO_DCC          
					dcc_chat( from, rest );
					sendnotice( getnick(from),
						"\x2Please type: /dcc chat %s\x2", current_nick );
					sendnotice( getnick(from),
						"%cAfter that, please retype: /msg %s %s %s\x2",
						'\x2', current_nick, msg, rest ? rest : "" );
#else
					sendnotice( getnick(from),
						"\x2Please start a dcc chat connection first\x2" );
#endif /* AUTO_DCC */
					return;
				}
#endif
				on_msg_commands[i].function( from, to, rest );
				for (ii=8;ii>=0;ii--)
					strcpy(lastcmds[ii+1],lastcmds[ii]);
				strcpy(lastcmds[0], on_msg_commands[i].name);
				strcat(lastcmds[0], " done by ");
				strcat(lastcmds[0], left(from,50));
				return;
			}
			else
			{
			 if (!to_all && cctoggle && on_msg_commands[i].cmdchar)
			   bot_reply(from, 2);
				return;
			}
		}
	}

	if (ischannel(to))
	{
	   user = find_userhost(from);
	   if (user)
	     update_flood(to, from);
	}
	else
	  {
	    if (holdingnick)
	      if (*nhmsg)
		send_to_user(from, "\x2%s is away:%s\x2", current_nick, nhmsg);
	      else
		send_to_user(from, "\x2%s is away...", current_nick);
	  }

	if (!to_all)
	    if( *command == cmdchar )
	      bot_reply( from, 1);

  if (*command != cmdchar )
    {
	if (*rest)
	  *(command+strlen(command)) = ' ';
	if (ischannel(to))
	  on_public(from, to, command);
	else
	  {
	    showspymsg(from, command);
	    if (pmlogtoggle || holdingnick)
	      botlog(pmlogfile, "[%s] %s", from, command);
	  }
      }
}

void on_public( char *from, char *to, char *rest )
{
	ListStruct *Dummy;
	int i;
	char buf[MAXLEN];

#ifdef DBUG
	printf("Entered Public: %s, %s, %s\n", from, to, rest);
#endif

  sprintf(buf, "\x2<%s:%s> %s\x2", getnick(from), to, rest);
	send_spy_chan(to, buf);
	if (beeptoggle && (numchar(rest, '\x7') > 1))
	  {
	    sprintf(buf, "%cAUTO-KICK --- Multiple Beeps\x2", '\x2');
	    sendkick(to, getnick(from), buf);
	  }
	    
	if (seentoggle && (strcasestr(rest, "seen ") &&
	    (strchr(rest, '?') || strcasestr(rest, "anyone") ||
	     strcasestr(rest, "someone"))))
	  {
	    char *temp;
	    temp = strcasestr(rest, "seen");
	    if (temp)
	      {
		temp = strchr(temp, ' ');
		if (temp)
		  {
		    temp++;
		    if (temp && *temp)
		      {
			pubseen = 1;
			do_seen(from, to, temp);
			pubseen = 0;
		      }
		  }
	      }
	  }
	if (is_in_list(LogChatList, to))
	  {
	    sprintf(buf, "<%s> ", getnick(from));
	    strcat(buf, rest);
	    logchat(to, buf);
	  }
	if (is_in_list(KickSays, rest) && is_in_list(KickSayChan, to) )
	{
		Dummy = find_list(KickSays, rest);
		if (userlevel(from) <= kslevel)
		{
			sprintf(buf, "ACTION overhears %s say %s", getnick(from),
				Dummy->name);
			send_ctcp( to, buf);
			  sprintf(buf, "ACTION sticks a sock in %s's mouth and...", getnick(from));
send_ctcp(to, buf);
			sendkick( to, getnick( from ),
				"\x2You're outta here\x2");
			sendprivmsg( to, "\x2%s shouldn't have said that!!!\x2",
				getnick(from) );
			send_to_user( from, "%cCareful, %s is banned!!!\x2", '\x2',
				Dummy->name);
		}
		else
		{
			sprintf(buf, "ACTION overhears %s say a banned word, but overlooks it",
				getnick(from) );
			send_ctcp( to, buf);
		}
	}
	if (!holdingnick && !userlevel(from) && weltoggle)
	{
		i = getrandom(0,numwelcome);
	      if (i == numwelcome)
		   i = 0;
		if (!matches("Re", rest) || !matches("Hello", rest) ||
			!matches("Hi", rest) || !matches("Hey", rest))
			sendprivmsg( to, "%s %s", welcomemess[i], getnick(from));
		sprintf(buf, "*Hey %s*", current_nick);
		if (!matches("*Hey All*", rest) || !matches(buf, rest) )
			sendprivmsg( to, "%s %s", welcomemess[i], getnick(from));
		sprintf(buf, "*Re %s*", current_nick);
		if (!matches("*Re All*", rest) || !matches(buf, rest) )
			sendprivmsg( to, "%s %s", welcomemess[i], getnick(from));
		sprintf(buf, "*Hi %s*", current_nick);
		if (!matches("*Hi All*", rest) || !matches(buf, rest) )
			sendprivmsg( to, "%s %s", welcomemess[i], getnick(from));
		sprintf(buf, "*Hello %s*", current_nick);
		if (!matches("*Hello All*", rest) || !matches(buf, rest) )
			sendprivmsg( to, "%s %s", welcomemess[i], getnick(from));

		sprintf(buf, "*Bye %s*", current_nick);
		if (!matches("*Bye All*", rest) || !matches(buf, rest) )
			sendprivmsg( to, "Bye %s", getnick(from));
	}
	else if (holdingnick)
	{
	  if (strcasestr(rest, current_nick))
	    if (*nhmsg)
	      sendprivmsg( to, "\x2%s is away:%s\x2", current_nick, nhmsg);
	    else
	      sendprivmsg( to, "\x2%s is away...\x2", current_nick);
	}
}

void on_part( char *from, char *channel)
{
    char buffer[MAXLEN];
    sprintf(buffer, "\x2%s has left channel %s\x2", from, channel);
    send_spy_chan(channel, buffer);
    sprintf(buffer, "%s has left channel %s", from, channel);
    if (is_in_list(LogChatList, channel))
      logchat(channel, buffer);
}

void on_nick( char *from, char *newnick)
{
;
}

void on_quit(char *from)
{
;
}

void on_kick( char *from, char *channel, char *nick, char *reason)
{
	char buffer[MAXLEN];

	UserHostStruct *user;
	static time_t currenttime;

	currenttime = getthetime();
	if (!reason)
		reason = "";

  sprintf(buffer, "\x2%s has been kicked off channel %s by %s (%s)\x2",
		nick, channel, getnick(from), reason);
     send_spy_chan(channel, buffer);
	if (is_in_list(LogChatList, channel))
	  {
		 sprintf(buffer, "%s has been kicked off channel %s by %s (%s)",
			 nick, channel, from, reason);
	    logchat(channel,buffer);
	  }

	user = find_userhost(from);

	if (user)
	  {
	    if (currenttime - user->kicktime > 10)
	      {
		user->kicktime = currenttime;
		user->kicknum = 0;
	      }
	    user->kicknum++;
	    user->totkick++;
	      if (user->kicknum >= masskicklimit)
		{
		      sprintf(buffer, "\x2MASS KICK THIS, YOU FUCK!!!\x2");
		      botlog(botlogfile, "Mass-kick detected by %s", from);
		      massprot_action(from, channel, buffer);
		      user->kicknum = 0;
		}
	}
	if( protlevel(username(nick)) &&
		!shitlevel(username(nick)) &&
		strcasecmp(getnick(from), nick) &&
		strcasecmp(getnick(from), current_nick) )
	{
		botlog(botlogfile, "%s kicked from %s by %s", username(nick),
		     channel, from);
		prot_action(from, username(nick), channel);
	}
}

void do_enfmodes( char *from, char *to, char *rest )
{
	char temp[10];
	char channel[20];
	char *newmodes;
	char *newptr = temp;
	*newptr = '\0';
	if (!*rest)
	{
	  bot_reply(from, 10);
	  return;
	}
	if (!ischannel(rest))
	{
	  bot_reply(from, 10);
	  return;
	}
	newmodes = strchr(rest, ' ');
	strncpy(channel, rest, 19);
	newmodes = strchr(rest, ' ');
	if (strchr(channel, ' ') != NULL)
	  *(strchr(channel, ' ')) = '\0';
	if (newmodes != NULL)
	{
	  for (; *newmodes != '\0'; newmodes++)
		 if (strchr(validenforced_modes, *newmodes) != NULL)
			*newptr++ = *newmodes;
	  *newptr = '\0';
	  if (!set_enforced_modes( channel, temp))
	    send_to_user( from, "\x2I'm not on %s\x2", channel);
	  else
	    send_to_user( from, "\x2New enforced modes on %s are %s\x2", channel, temp);
	}
	else
	{
	  if (!set_enforced_modes( channel, ""))
	    bot_reply(from, 11);
	  else
	    send_to_user( from, "\x2No modes enforced on %s\x2", channel);
	}
}

void do_access( char *from, char *to, char *rest )
{
  char *nuh = from;

  if (*rest)
  {
	 nuh = strtok(rest, " ");
	 nuh = find_person(from, nuh);
  }
  if (!*nuh)
	 return;

  send_to_user( from, "Access for %s: %i", nuh, userlevel(nuh) );
  send_to_user( from, "There is \x2NO FUCKING ERROR\x2, so don't e-mail me");
  return;
}

void do_stats( char *from, char *to, char *rest )
{
  char *nuh = from, *temp;

  if (*rest)
  {
	 nuh = strtok(rest, " ");
	 nuh = find_person(from, nuh);
  }
  if (!*nuh)
	 return;

  send_to_user( from, "Stats for %s", nuh);
  send_to_user( from, "--------------------------");
  send_to_user( from, "      User level: %i", userlevel(nuh));
  send_to_user( from, " Protected level: %i", protlevel(nuh));
  if (!protlevel(nuh) && !prottoggle)
	send_to_user( from,"                  \x2(Protection toggled off)\x2");
  send_to_user( from, "Shitlisted level: %i", shitlevel(nuh));
  if (shitlevel(nuh))
  {
	  temp = shitchan(nuh);
	  send_to_user(from, "Shitted channels: %s", temp);
  }
  send_to_user( from, "      Auto Opped: %s", isaop(nuh) ? "Yes" : "No");
  return;
}

void do_help( char *from, char *to, char *rest )
{
  FILE *in;
  char lin2[MAXLEN];
  char *cmd1, *cmd2 = lin2;
  char lin[MAXLEN];
  int num = 0, next = 0;

  if (!*rest)
  {
    send_to_user(from, "\x2Usage: \"%chelp\" <topic>\x2", cmdchar);
    send_to_user(from, "\x2If you would like a helpfile, try \"%cfiles\"",
		 cmdchar);
    return;
  }
  
  if ((in = fopen(helpfile, "r")) == NULL)
  {
    send_to_user(from, "\x2Sorry, %s not found\x2", helpfile);
    return;
  } 
  cmd1 = strtok(rest, " ");
  if (!cmd1)
  {
    send_to_user(from, "\x2Specify a valid command\x2");
    return;
  }
  while (!feof(in))
  {
    if (readln_from_a_file(in, lin))
      {
	if (next)
	  {
	    sscanf(lin, "%s", cmd2);
	    if (!strchr(cmd1, ','))
	      strcat(cmd1, ",");
	    if (!strcasecmp(cmd2, cmd1))
	      {
		next = 0;
		while (!next && !feof(in))
		  {
		    send_to_user(from, "\x2%s\x2", lin);
		    next = !readln_from_a_file(in, lin);
		    if (*lin == ' ')
		      strcpy(lin, strip(lin, ' '));
		    if (!*lin || ((int) (*lin) == 10))
		      next = 1;
		  }
		num++;
	      }
	    next = 0;
	  }
	if (lin && (*lin == ' '))
	  strcpy(lin, strip(lin, ' '));
	if (!*lin || ((int) (*lin) == 10))
	  next = 1;
      }
  }
  if (num == 0)
    send_to_user(from, "\x2That command not found...\x2");
  fclose(in);
  
}

void  do_info( char *from, char *to, char *rest )
{
  send_to_user( from, "\x2( StelBot )  /  Version  1.0\x2");
  send_to_user( from, "\x2Written by: Bryan8!bryan@primenet.com\x2");
#ifndef MSDOS
  send_to_user( from, "\x2Started: %-20.20s\x2", time2str(uptime) );
  send_to_user( from, "\x2Up: %s\x2", idle2str(time(NULL)-uptime));
#endif
  return;
}

void  do_getch( char *from, char *to, char *rest)
{
  send_to_user( from, "%cCurrent channel: %s\x2", '\x2',
	       currentchannel() );
  return;
}

void  do_time( char *from, char *to, char *rest )
{
  char buf[MAXLEN];

  strcpy(buf, "%cCurrent time: %s\x2");
  
#ifndef MSDOS
  if (*rest != '#')
    send_to_user( from, buf, '\x2', time2str(time(NULL)));
  else
    sendprivmsg( rest, buf, '\x2', time2str(time(NULL)));
#endif
}

void do_userlist( char *from, char *to, char *rest )
{
    send_to_user( from, " Userlist: %30s   %s", "nick!user@host", "level" );
    send_to_user( from, " -----------------------------------------+------" );
    show_userlist( from, rest?rest:"" );
    send_to_user( from, " -------------- End of Userlist ----------+------" );
}


void do_protlist( char *from, char *to, char *rest)
{
  send_to_user( from, " Protlist: %30s   %s", "nick!user@host", "level" );
  send_to_user( from, " -----------------------------------------+------" );
  show_protlist( from, rest ? rest : "");
  send_to_user( from, " ---------------- End of Protlist --------+------" );
}

void do_shitlist( char *from, char *to, char *rest)
{
   send_to_user( from, " Shitlist: %30s   %s     Channels",
		"nick!user@host", "level" );
   send_to_user( from, " -----------------------------------------+------+----------------" );
   show_shitlist( from, rest?rest:"");
   send_to_user( from, " --------------- End of Shitlist ---------+------+----------------" );
}

void do_opme( char *from, char *to, char *rest )
{
	char thechannel[MAXLEN];

	strcpy(thechannel, find_channel(to, &rest));

	if (usermode(thechannel, getnick(from)) & MODE_CHANOP)
		send_to_user(from, "\x2You're already channel operator on %s\x2",
			thechannel);
	else
		giveop(thechannel, getnick( from ) );
}

void do_deopme( char *from, char *to, char *rest )
{
	char thechannel[MAXLEN];
	strcpy(thechannel, find_channel(to, &rest));

	if (usermode(thechannel, getnick(from)) & MODE_CHANOP)
		takeop(thechannel, getnick( from ) );
	else
		send_to_user(from, "\x2You're already not a channel operator on %s\x2",
			thechannel);
}

void do_op( char *from, char *to, char *rest )
{
	char thechannel[MAXLEN];

	strcpy(thechannel, find_channel(to, &rest));
	if( rest != NULL )
	{
#ifdef DBUG
	printf("Opping:%s\n", rest);
#endif
		mode3( thechannel, "+ooo", rest );
	}
	 else
		send_to_user( from, "Who do you want me to op, you schmuck?" );
	return;
}

void do_deop( char *from, char *to, char *rest)
{
	char thechannel[MAXLEN];

	strcpy(thechannel, find_channel(to, &rest));
	if (*rest)
	{
#ifdef DBUG
		printf("DeOpping:%s\n", rest);
#endif
		mode3(thechannel, "-ooo", rest);
	}
	else
		send_to_user( from, "\x2Who am I supposed to deop???\x2");
	return;
}


void do_invite( char *from, char *to, char *rest )
{
	char thechannel[MAXLEN];
	char thenick[15];

	strcpy(thechannel, find_channel(to, &rest));
	strcpy(thenick, getnick(from));
	if (*rest)
	{
		strncpy(thenick, rest, 15);
		if (strchr(thenick, ' ') != NULL)
			*strchr(thenick, ' ') = '\0';
	}
	if (!*getuserhost(thenick))
	{
		send_to_user(from, "\x2No such nick, you dickhead\x2");
		return;
	}
	if( !invite_to_channel( getnick(thenick), thechannel) )
		send_to_user(from, "\x2I'm not on channel %s, you dickhead\x2", thechannel);
	else
		send_to_user(from, "\x2%s invited to %s\x2", getnick(thenick), thechannel);
}
			
void  do_open( char *from, char *to, char *rest )
{
		  if( rest != NULL )
		  {
		if(!open_channel(rest))
			send_to_user(from, "I could not open %s!", rest);
		else
			channel_unban(rest, from);
	}
	else
	{
		open_channel(currentchannel());
		channel_unban(currentchannel(), from);
	}
}

void do_chat( char *from, char *to, char *rest )
{
#ifndef MSDOS
	dcc_chat( from, rest );
	sendnotice( getnick(from), "\x2Please type: /dcc chat %s\x2", current_nick );
#endif
}

void do_send( char *from, char *to, char *rest )
{
  char  buf[MAXLEN];
  FILE *in;
  int ulevel, level;
  
  if (!*rest)
    {
      send_to_user(from, "\x2Please specify a filename or use \"%cfiles\"\x2",
		   cmdchar);
      return;
    }

  ulevel = userlevel(from);
#ifndef MSDOS
  if ((in = fopen(filelistfile, "r"))==NULL)
    {
      send_to_user( from, "\x2Problem opening %s\x2", filelistfile);
      return;
    }
  while (!feof(in))
    {
      fscanf(in, "%s %i\n", buf, &level);
      if (!strcmp(rest, buf))
	{
	  if (level > ulevel)
	    {
	      send_to_user(from, "\x2Sorry, you need access level %i to get this file\x2", level);
	      return;
	    }
	  fclose(in);
	  dcc_sendfile( from, rest );
	  return;
	}
    }
  send_to_user(from, "\x2That file does not exist...use \"%cfiles\"\x2",
	       cmdchar);
#endif
	fclose(in);
}

void  do_flist( char *from, char *to, char *rest )
{
	FILE  *ls_file;
	char buffer[255];
	int level;

	if( ( ls_file = fopen( filelistfile, "r" ) ) == NULL )
	{
		send_to_user(from, "\x2No files available\x2");
		return;
	}

	send_to_user(from, "\x2-------- Filename --------+- Level Needed -\x2");
	while (!feof(ls_file))
	  {
	    if (fscanf(ls_file, "%s %i\n", buffer, &level))
	      send_to_user(from, "%25s |    %3i", buffer, level);
	  }
	send_to_user(from, "\x2--------------------------+----------------\x2");
	
	fclose( ls_file );
}  

void do_say( char *from, char *to, char *rest )
{
	char thechannel[MAXLEN];
	char *text = rest;

	strcpy(thechannel, find_channel(to, &rest));
	if (!*rest)
	{
		send_to_user( from, "\x2What am I suppposed to say, dilweed?\x2");
		return;
	}
	sendprivmsg( thechannel, "%s", rest);
    
	return;
}

void do_me( char *from, char *to, char *rest )
{
	char thechannel[MAXLEN];
	char text[MAXLEN];
	strcpy(text, "ACTION ");
	strcpy(thechannel, find_channel(to, &rest));
	if (!*rest)
	{
		send_to_user( from, "\x2What am I suppposed to action, dilweed?\x2");
		return;
	}
	strcat(text, rest);
	send_ctcp( thechannel, "%s", text);
	return;
}

void do_msg( char *from, char *to, char *rest)
{
	char *theperson;
	char *themsg;
	if (!*rest)
	  {
	    bot_reply(from, 4);
	    return;
	  }
	if (!strchr(rest, ' '))
	  {
	    send_to_user( from, "\x2What's the message?\x2");
	    return;
	  }

	theperson = strtok(rest, " ");
	if (theperson && !strcasecmp(theperson,current_nick))
	  {
	    bot_reply(from, 6);
	    return;
	  }
	themsg = strtok(NULL, "\r\n\0");
	sendprivmsg( theperson, "%s", themsg);
	send_to_user( from, "\x2Message sent\x2");
}

   
void  do_do( char *from, char *to, char *rest )
{
  if ( *rest )
    send_to_server( "%s", rest );
  else
    send_to_user( from, "What do you want me to do?" );
  return;
}

void show_channels( char *from, char *to, char *rest )
{
  show_channellist( from );
  return;
}

void  do_botinfo( char *from, char *to, char *rest)
{
      send_to_user(from, "\x2 ( StelBot ) / Version 1.0 / By ~~Bryan8~~\x2");
      send_to_user(from, "\x2 Created :  November 15, 1994\x2");
      send_to_user(from, "\x2 Where To Get :  1. Ftp to  ftp.rahul.net\x2");
      send_to_user(from, "\x2 cd /pub/conquest/DeadelviS/bot/C\x2");   
      send_to_user(from, "\x2 Get Me From Bryan8 On Irc\x2");
}

void  do_bot( char *from, char *to, char *rest)
{
      send_to_user(from, "\x2My Owner Is  <---------->  Blank\x2");
      send_to_user(from, "\x2My Command Char. Is <--->  B\x2");
}

void  do_listp( char *from, char *to, char *rest)
{
      send_to_user(from, "\x2-=[ General Public Commands ]=-\x2");
      send_to_user(from, "\x2(ACCESS)  GETMSG  HELP  BOT  STATS\x2");
      send_to_user(from, "\x2SEEN  INFO  TIME  PING  CHAT  DOWN\x2");
      send_to_user(from, "\x2USERHOST  TOTSTATS  TOPTEN  DISLPAY\x2");
      send_to_user(from, "\x2SM  BOTINFO  SEND\x2");
}

void  do_list20( char *from, char *to, char *rest)
{
      send_to_user(from, "\x2-=[ Level 20 Commands ]=-\x2");
      send_to_user(from, "\x2UP  CHANNELS  LKS  LKSC  GETCH\x2");
}

void  do_list30( char *from, char *to, char *rest)
{
      send_to_user(from, "\x2-=[ Level 30 Commands ]=-\x2");
      send_to_user(from, "\x2(B)  UP  OP  DEOP  K\x2");
}

void  do_list40( char *from, char *to, char *rest)
{
      send_to_user(from, "\x2-=[ Level 40 Commands ]=-\x2");
      send_to_user(from, "\x2KB  INVITE  UBM  TOPIC\x2");
}

void  do_list50( char *from, char *to, char *rest)
{
      send_to_user(from, "\x2-=[ Level 50 Commands ]=-\x2");
      send_to_user(from, "\x2SCREW\x2");
}

void do_list60( char *from, char *to, char *rest)
{
     send_to_user(from, "\x2-=[ Level 60 Commands ]=-\x2");
     send_to_user(from, "\x2(FUCK)  SB  SKB\x2");
}

void do_list70( char *from, char *to, char *rest)
{
     send_to_user(from, "\x2-=[ Level 70 Commands ]=-\x2");
     send_to_user(from, "\x2SPYMSG  RSPYMSG  SPY  RSPY  AOP\x2");
     send_to_user(from, "\x2RAOP  USERLIST  SHITLIST\x2");
     send_to_user(from, "\x2PROTLIST  DCCLIST\x2");
}

void do_list80( char *from, char *to, char *rest)
{
     send_to_user(from, "\x2-=[ Level 80 Commands ]=-\x2");
     send_to_user(from, "\x2SAY  ME  CKS  CLVL  CKSC  AKSC\x2");
     send_to_user(from, "\x2KS  RKS  MSG  OPEN  JOIN  CYCLE\x2");
     send_to_user(from, "\x2LEAVE  NICK  RPROT  SHIT  RSHIT\x2");
     send_to_user(from, "\x2PROT  MDEOPMU  SHOWUSERS  MKNU  MOPU\x2");
}

void do_list85( char *from, char *to, char *rest)
{
     send_to_user(from, "\x2-=[ Level 85 Commands ]=-\x2");
     send_to_user(from, "\x2(EMAIL)  MOP  MDEOP  MK  MKB  UBA\x2");
}

void do_list90( char *from, char *to, char *rest)
{
     send_to_user(from, "\x2-=[ Level 90 Commands ]=-\x2");
     send_to_user(from, "\x2SPYLIST  STEAL  RSTEAL  ENFMODES\x2");
     send_to_user(from, "\x2(CMDCHAR)  SERVER  NEXTSERVER  RUH\x2");
}

void do_list95( char *from, char *to, char *rest)
{
     send_to_user(from, "\x2-=[ Level 95 / Asst. Bot Owner Commands ]=-\x2");
     send_to_user(from, "\x2LASTLOG  HEART  CHACCESS  REREAD\x2");
}

void do_list99( char *from, char *to, char *rest)
{
     send_to_user(from, "\x2-=[ Level 99 / Bot Owner Commands ]=-\x2");
     send_to_user(from, "\x2SAVEUH  HN  RHN  RESETUH  LOADUSERS\x2");
     send_to_user(from, "\x2LOADSHIT  KEEPU  KEEPS  RLOGCHAT  DO\x2");
     send_to_user(from, "\x2SPAWN  KILL  REM  CNU  LOGCHAT  LAST10\x2");
}

void do_toglist( char *from, char *to, char *rest)
{
     send_to_user(from, "\x2(--------Toggels   List--------)\x2"); 
     send_to_user(from, "\x2******  Level 70 Toggles  ******\x2");
     send_to_user(from, "\x2(------------------------------)\x2");
     send_to_user(from, "\x2    TOGAOP  TOGSHIT  TOGPUB     \x2");
     send_to_user(from, "\x2(------------------------------)\x2");
     send_to_user(from, "\x2******  Level 80 Toggles  ******\x2");
     send_to_user(from, "\x2(------------------------------)\x2");
     send_to_user(from, "\x2        TOGPROT  TOGWM          \x2");
     send_to_user(from, "\x2(------------------------------)\x2");
     send_to_user(from, "\x2******  Level 90 Toggles  ******\x2");
     send_to_user(from, "\x2(------------------------------)\x2");
     send_to_user(from, "\x2  TOGIT  TOGSP  TOGCC  TOGENFM  \x2");
     send_to_user(from, "\x2    TOGMASS  TOGFLOOD  TOGBK    \x2");
     send_to_user(from, "\x2         TOGMUK  TOGSO          \x2");
     send_to_user(from, "\x2(------------------------------)\x2");
     send_to_user(from, "\x2******  Level 95 Toggles  ******\x2");
     send_to_user(from, "\x2(------------------------------)\x2");
     send_to_user(from, "\x2       TOGLOL  TOGPMLOG         \x2");
     send_to_user(from, "\x2(------------------------------)\x2");
     send_to_user(from, "\x2(----End  Of  Toggles  List----)\x2");
}

void  do_setlist( char *from, char *to, char *rest)
{
      send_to_user(from, "\x2********  Level 80 Sets  ********\x2");
      send_to_user(from, "\x2SETFL  SETMOL  SETMDOL  SETMBL  SETMBL\x2");
      send_to_user(from, "\x2********  Level 85 Sets  ********\x2");
      send_to_user(from, "\x2SETMASS  SETKSL\x2");
      send_to_user(from, "\x2********  Level 90 Sets  ********\x2");
      send_to_user(from, "\x2SETIKL  SETMPL  SETMOL  SETMAL\x2");
}

void  do_join( char *from, char *to, char *rest)
{
  char channel[255];

  if (!*rest)
    {
      send_to_user(from, "\x2What the fuck channel am I supposed to join?\x2");
      return;
    }

  if (*rest)
    strcpy(channel, fix_channel(rest));
  else if (!strcasecmp( current_nick, to))
    strcpy(channel, currentchannel());
  else
    strcpy(channel, to);
  send_to_user(from, "%cAttempting the join of %s\x2", '\x2', channel);
  join_channel(channel);
}

void  do_leave( char *from, char *to, char *rest )
{
  char channel[255];

  if (*rest)
    strcpy(channel, fix_channel(rest));
  else if (!strcasecmp( current_nick, to))
    strcpy(channel, currentchannel());
  else
    strcpy(channel, to);

  if (is_in_list(StealList, channel))
    send_to_user(from, "\x2Sorry, I'm trying to steal %s\x2", channel);
  else
    leave_channel( channel );
}

void  do_nick( char *from, char *to, char *rest )
{

	if( *rest )
	{
	  strcpy(oldnick, "");
		if (strchr(rest, ' ') != NULL)
			*strchr(rest, ' ') = '\0';
		strncpy( current_nick, rest, 9);
		sendnick( current_nick );
	}
	else
	  bot_reply(from, 8);
	return;
}

void  do_die( char *from, char *to, char *rest )
{
	char buf[100];
	int temp;
	temp = logtoggle;
	logtoggle = 1;
	botlog(botlogfile, "KILL issued by: %s", from);
	logtoggle = temp;
        send_to_user(from, "\x2Killed !!!\x2");	
        sprintf(buf, "Authorized Kill By Owner", getnick(from) );
	if( *rest )
		signoff( from, rest );
	else
		signoff( from, buf );
}

void do_add( char *from, char *to, char *rest )
{
	char *userstr, *newlevel, *isaop, *protl, *nuh;
	int prot = 0, aop = 0, level = 0;

	nuh = strtok(rest, " ");
	if (!nuh || !*nuh)
	{
	  bot_reply(from, 4);
	  return;
	}
	nuh = find_person( from, nuh);
	if (!*nuh)
		return;

	if (isuser(nuh))
	{
		level=userlevel(nuh);
		send_to_user( from, "\x2%s Allready Created With Level %i\x2", nuh, level);
		send_to_user( from, "\x2To Change It, Use \"%cclvl\"\x2",
			     cmdchar);
		return;
	}
	if (!(newlevel = strtok( NULL, " ")))
	{
	  bot_reply(from, 9);
	  return;
	}
	if (isaop = strtok( NULL, " "))
	{
		aop = atoi(isaop);
		if (protl = strtok( NULL, " "))
			prot = atoi(protl);
	}
	if( atoi( newlevel ) < 0 )
		send_to_user( from, "\x2Level Should Be >= 0\x2" );
	else if( atoi( newlevel ) > userlevel( from ) )
		send_to_user( from, "\x2Level Must Be <= to yours\x2");
	else if( atoi(newlevel) >= OWNERLEVEL)
	  send_to_user(from, "\x2I Really Don't Think So\x2");
	else
	{
		if (prot > MAXPROTLEVEL)
			prot = MAXPROTLEVEL;
		if (prot < 0)
			prot = 0;
		userstr = cluster(nuh);
		if (add_to_userlist( userstr, atoi( newlevel ), aop ? 1 : 0, prot))
		{
			send_to_user( from, "\x2User %s Created !!\x2", userstr);
			send_to_user( from,
				"%cAccess Level: %i  Auto-Op: %s  Protect Level: %i\x2",
				'\x2', atoi(newlevel), aop ? "Yes" : "No", prot);
			send_to_user( nuh,
				     "\x2%s Has Been Created With Level %i Clearance\x2", getnick(from), atoi(newlevel));
			send_to_user( nuh,
				     "\x2You Are %sBeing Auto-Opped\x2",
				     aop ? "" : "not ");
			if (prot)
			  send_to_user( nuh,
			      "\x2You Are Being Protected With Level %i\x2",
				       prot);
			else
			  send_to_user( nuh,
				       "\x2You Are Not Being Protected\x2");
		}
		else
			send_to_user( from, "\x2User Not Created\x2");
	}
	return;
}                   

void do_aop( char *from, char *to, char *rest )
{
	char *nuh;

	nuh = strtok(rest, " ");
	if (!nuh || !*nuh)
	{
	  bot_reply(from, 4);
	  return;
	}
	nuh = find_person( from, nuh);
	if (!*nuh)
		return;

	if (!isuser(nuh))
	{
		send_to_user( from, "\x2%s is not in my users list\x2", nuh);
		return;
	}
	if (isaop(nuh))
	{
		send_to_user( from, "\x2%s is already being auto-opped\x2", nuh);
		return;
	}
	if (!change_userlist (nuh, -1, 1, -1))
	{
		send_to_user( from, "\x2%s is not in my users list\x2", nuh);
		return;
	}
	send_to_user( from, "\x2%s is now being auto-opped\x2", nuh);
	return;
}

void do_naop( char *from, char *to, char *rest )
{
	char *nuh;

	nuh = strtok(rest, " ");
	if (!nuh || !*nuh)
	{
	  bot_reply(from, 4);
	  return;
	}
	nuh = find_person( from, nuh);
	if (!*nuh)
		return;

	if (!isuser(nuh))
	{
		send_to_user( from, "\x2%s is not in my users list\x2", nuh);
		return;
	}
	if (!isaop(nuh))
	{
		send_to_user( from, "\x2%s is already not being auto-opped\x2", nuh);
		return;
	}
	if (!change_userlist (nuh, -1, 0, -1))
	{
		send_to_user( from, "\x2%s is not in my users list\x2", nuh);
		return;
	}
	send_to_user( from, "\x2%s is now not being auto-opped\x2", nuh);
	return;
}

void do_saveusers( char *from, char *to, char *rest )
{
	if( !write_userlist( usersfile ) )
		send_to_user( from, "\x2Users Not Saved %s\x2", usersfile );
	else
		send_to_user( from, "\x2Users Saved %s\x2", usersfile );
}

void do_saveshit( char *from, char *to, char *rest )
{
	if( !write_shitlist( shitfile ) )
		send_to_user( from, "\x2Shitlist Not Saved %s\x2", shitfile );
	else
		send_to_user( from, "\x2Shitlist Saved %s\x2", shitfile );
}

void do_loadusers( char *from, char *to, char *rest )
{
  if( !read_userlist( usersfile ) )
    send_to_user( from, "\x2Users Could Not Be Loaded %s\x2", usersfile );
  else
    send_to_user( from, "\x2Users Loaded %s\x2", usersfile );
}

void do_loadshit( char *from, char *to, char *rest )
{
  if( !read_shitlist( shitfile ) )
    send_to_user( from, "\x2Shitlist Could Not Be Loaded %s\x2", shitfile );
  else
    send_to_user( from, "\x2Shitlist Loaded %s\x2", shitfile );
}

void do_userdel( char *from, char *to, char *rest )
{
	char *nuh;

	nuh = strtok(rest, " ");
	if (!nuh || !*nuh)
	{
	  bot_reply(from, 4);
	  return;
	}
	nuh = find_person( from, nuh);
	if (!*nuh)
		return;

	if (!isuser(nuh))
	{
		send_to_user( from, "\x2%s Not Found \x2", nuh);
		return;
	}
	if (!matches(owneruhost, nuh))
	  {
	    send_to_user(from, "\x2( Access ) Denied  /  Cannot Delete Owner\x2");
	    return;
	  }
	  
	if (userlevel(from) <= userlevel(nuh))
	{
		send_to_user( from, "\x2%s Has A Higher Or Equal Level Than You\x2", nuh );
		return;
	}
	if (!remove_from_userlist(nuh))
		send_to_user( from, "\x2Problem Deleting %s\x2", nuh);
	else
	  {
		send_to_user( from, "\x2User %s Killed !\x2", nuh );
		send_to_user( nuh, "\x2Your Access Has Been Terminated by %s\x2",
			     getnick(from));
	  }
	return;
}

void do_screwban( char *from, char *to, char *rest )
{
	char *nuh;
	char channel[MAXLEN];
	char *temp;
	int num;

	strcpy(channel, find_channel(to, &rest));

	nuh = strtok(rest, " ");
	if (!nuh || !*nuh)
	{
	  bot_reply(from, 4);
	  return;
	}
	nuh = find_person( from, nuh);
	if (!*nuh)
		return;

	temp = strtok(NULL, "\n");
	if (temp)
	  num = atoi(temp);
	else
	  num = 1;

	if ( (protlevel(nuh) || (userlevel(nuh) >= PROTLEVEL)) &&
		(userlevel(from) != OWNERLEVEL))
	{
		send_to_user(from, "\x2Sorry, that user is protected\x2");
		return;
	}

	screw_ban(channel, nuh, num);
	send_to_user( from, "\x2%s screwbanned on %s\x2", nuh, channel);
}

void do_ban( char *from, char *to, char *rest )
{
	char *nuh;
	char channel[MAXLEN];

	strcpy(channel, find_channel(to, &rest));

	nuh = strtok(rest, " ");
	if (!nuh || !*nuh)
	{
	  bot_reply(from, 4);
	  return;
	}
	nuh = find_person( from, nuh);
	if (!*nuh)
		return;

	if ( (protlevel(nuh) || (userlevel(nuh) >= PROTLEVEL)) &&
		(userlevel(from) != OWNERLEVEL))
	{
		send_to_user(from, "\x2Sorry, that user is protected\x2");
		return;
	}

	ban_user(channel, nuh );
	send_to_user( from, "\x2%s banned on %s\x2", nuh, channel);
}
void do_siteban( char *from, char *to, char *rest)
{
  char *nuh;
  char channel[MAXLEN];

  strcpy(channel, find_channel(to, &rest));

	nuh = strtok(rest, " ");
	if (!nuh || !*nuh)
	{
	  bot_reply(from, 4);
	  return;
	}
	nuh = find_person( from, nuh);
	if (!*nuh)
		return;

	if ( (protlevel(nuh) || (userlevel(nuh) >= PROTLEVEL)) &&
		(userlevel(from) != OWNERLEVEL))
	{
		send_to_user(from, "\x2Sorry, that user is protected\x2");
		return;
	}

	site_ban(channel, nuh);
	send_to_user( from, "\x2%s site-banned on %s\x2", nuh, channel);
}

void do_sitekb( char *from, char *to, char *rest)
{
  char *nuh, *msg;
  char channel[MAXLEN];

  strcpy(channel, find_channel(to, &rest));

	nuh = strtok(rest, " ");
	if (!nuh || !*nuh)
	{
	  bot_reply(from, 4);
	  return;
	}
	nuh = find_person( from, nuh);
	if (!*nuh)
		return;

	if ( (protlevel(nuh) || (userlevel(nuh) >= PROTLEVEL)) &&
		(userlevel(from) != OWNERLEVEL))
	{
		send_to_user(from, "\x2Sorry, that user is protected\x2");
		return;
	}
        msg = strtok(NULL, "\n\0");
 if (!msg)
   msg = "";
  site_ban(channel, nuh );
  sendkick(channel, getnick(nuh), msg);
  send_to_user(from, "\x2%s site-kickbanned on %s\x2", nuh, channel);
}

void do_fuckban( char *from, char *to, char *rest)
{
	char *nuh, *msg;
	char channel[MAXLEN];

	strcpy(channel, find_channel(to, &rest));

	nuh = strtok(rest, " ");
	if (!nuh || !*nuh)
	{
	  bot_reply(from, 4);
	  return;
	}
	nuh = find_person( from, nuh);
	if (!*nuh)
		return;

	if ( (protlevel(nuh) || (userlevel(nuh) >= PROTLEVEL)) &&
		(userlevel(from) != OWNERLEVEL))
	{
		send_to_user(from, "\x2Sorry, that user is protected\x2");
		return;
	}
	  msg = strtok(NULL, "\n\0");
	if (!msg)
	  msg = "";
	ban_user(channel, nuh);
	screw_ban(channel, nuh, 1);
	sendkick( channel, getnick(nuh), msg);
}

void do_kickban( char *from, char *to, char *rest)
{
	char *nuh, *msg;
	char channel[MAXLEN];

	strcpy(channel, find_channel(to, &rest));

	nuh = strtok(rest, " ");
	if (!nuh || !*nuh)
	{
	  bot_reply(from, 4);
	  return;
	}
	nuh = find_person( from, nuh);
	if (!*nuh)
		return;

	if ( (protlevel(nuh) || (userlevel(nuh) >= PROTLEVEL)) &&
		(userlevel(from) != OWNERLEVEL))
	{
		send_to_user(from, "\x2Sorry, that user is protected\x2");
		return;
	}
	msg = strtok(NULL, "\n\0");
 if (!msg)
   msg = "";
	ban_user(channel, nuh);
	sendkick( channel, getnick(nuh), msg);
}

void do_kick( char *from, char *to, char *rest)
{
	char *nuh, *msg;
	char channel[MAXLEN];

	strcpy(channel, find_channel(to, &rest));

	nuh = strtok(rest, " ");
	if (!nuh || !*nuh)
	{
	  bot_reply(from, 4);
	  return;
	}
	nuh = find_person( from, nuh);
	if (!*nuh)
		return;

	if ( (protlevel(nuh) || (userlevel(nuh) >= PROTLEVEL)) &&
		(userlevel(from) != OWNERLEVEL))
	{
		send_to_user(from, "\x2Sorry, that user is protected\x2");
		return;
	}
	  msg = strtok(NULL, "\n\0");
 if (!msg)
   msg = "";
	sendkick( channel, getnick(nuh), msg);
}

void do_showusers( char *from, char *to, char *rest )
{
	char *channel;

	if( (channel = strtok( rest, " " )) == NULL )
		show_users_on_channel( from, currentchannel());
	else
		show_users_on_channel( from, fix_channel(channel) );
}

void do_massop( char *from, char *to, char *rest )
{
	char pattern[100];
	char  *op_pattern;
	char channel[MAXLEN];

	strcpy(channel, find_channel(to, &rest));
	if (*rest)
	{
		op_pattern = strtok(rest, " ");
		if (op_pattern)
			strcpy(pattern, op_pattern);
		else
			strcpy(pattern, "*!*@*");
	}
	else
		strcpy(pattern, "*!*@*");
	channel_massop( channel, pattern );
	return;
}

void do_massdeopnu( char *from, char *to, char *rest)
{
  char channel[MAXLEN];
  strcpy(channel, find_channel(to, &rest));
  channel_massdeop( channel, "");
  return;
}

void do_masskicknu( char *from, char *to, char *rest)
{
  char channel[MAXLEN];
  strcpy(channel, find_channel(to, &rest));
  channel_masskick( channel, "");
  return;
}

void do_massopu( char *from, char *to, char *rest)
{
  char channel[MAXLEN];
  strcpy(channel, find_channel(to, &rest));
  channel_massop( channel, "");
  return;
}

void do_massdeop( char *from, char *to, char *rest )
{
	char pattern[100];
	char  *op_pattern;
	char channel[MAXLEN];

	strcpy(channel, find_channel(to, &rest));
	if (*rest)
	{
		op_pattern = strtok(rest, " ");
		if (op_pattern)
			strcpy(pattern, op_pattern);
		else
			strcpy(pattern, "*!*@*");
	}
	else
		strcpy(pattern, "*!*@*");
	channel_massdeop( channel, pattern );
	return;
}

void do_masskick( char *from, char *to, char *rest )
{
	char pattern[100];
	char  *op_pattern;
	char channel[MAXLEN];

	strcpy(channel, find_channel(to, &rest));
	if (*rest)
	{
		op_pattern = strtok(rest, " ");
		if (op_pattern)
			strcpy(pattern, op_pattern);
		else
			strcpy(pattern, "*!*@*");
	}
	else
		strcpy(pattern, "*!*@*");
	channel_masskick( channel, pattern );
	return;
}

void do_masskickban( char *from, char *to, char *rest )
{
	char pattern[100];
	char  *op_pattern;
	char channel[MAXLEN];

	strcpy(channel, find_channel(to, &rest));
	if (*rest)
	{
		op_pattern = strtok(rest, " ");
		if (op_pattern)
			strcpy(pattern, op_pattern);
		else
			strcpy(pattern, "*!*@*");
	}
	else
		strcpy(pattern, "*!*@*");
	channel_masskickban( channel, pattern );
	return;
}

void do_shit(char *from, char *to, char *rest)
{
	char *newlevel, *isaop, *protl, *nuh, *chan, *userstr;
	int prot = 0, aop = 0, level = 0, uhs = 0;

	nuh = strtok(rest, " ");
	if (!nuh || !*nuh)
	{
	  bot_reply(from, 4);
	  return;
	}
	nuh = find_person( from, nuh);
	if (!*nuh)
		return;

	if (level=shitlevel(nuh))
	{
		send_to_user( from, "\x2%s is already in my shitlist with level %i\x2", nuh, level);
		send_to_user( from, "\x2To change it, delete the user and reshit\x2");
		return;
	}
	if (userlevel(nuh) >= userlevel(from))
	{
		send_to_user(from, "\x2Sorry, %s has a higher or equal access level than you\x2", nuh);
		return;
	}
	if (!matches( owneruhost, nuh))
	  {
	    send_to_user(from, "\x2Sorry, can't shit the owner\x2");
	    return;
	  }

	if (!(newlevel = strtok( NULL, " ")))
	{
	  bot_reply(from, 9);
	  return;
	}

	if( (atoi( newlevel ) <= 0) || (atoi(newlevel) > MAXSHITLEVEL))
	{
	  send_to_user( from, "\x2Valid levels are 1 through %i!!!\x2",
		       MAXSHITLEVEL);
	  return;
	}

	chan = strtok( NULL, "\n\0");

	userstr = cluster(nuh);
	if (!matches(userstr, from))
	{
		send_to_user(from, "\x2Sorry, you can't shit yourself\x2");
		return;
	}
	if (add_to_shitlist( userstr, atoi( newlevel ), chan ) )
	{
		send_to_user( from, "\x2%s has been shitted as %s\x2",
			 nuh, userstr);
		send_to_user( from, "\x2with a shit level of %d\x2",
			 atoi(newlevel));
		send_to_user( from, "\x2Shitted on channels: %s\x2",
			shitchan(userstr));
		send_to_user( nuh,
			"\x2You have been shitlisted by %s\x2", getnick(from));
		if (ischannel(to))
	      shitkick(nuh, to);
		else
	      shitkick(nuh, currentchannel());
	}
	else
		send_to_user( from, "\x2Problem shitting the user\x2");
	return;
}

void do_nshit(char *from, char *to, char *rest)
{
	char *nuh;

	nuh = strtok(rest, " ");
	if (!nuh || !*nuh)
	{
	  bot_reply(from, 4);
	  return;
	}
	nuh = find_person( from, nuh);
	if (!*nuh)
		return;

	if (!shitlevel(nuh))
	{
		send_to_user( from, "\x2%s is not in my shitlist\x2", nuh);
		return;
	}

	if (!remove_from_shitlist(nuh))
		send_to_user( from, "\x2Problem deleting %s\x2", nuh);
	else
		send_to_user( from, "\x2No longer shitting %s\x2", nuh );
	return;
}

void do_prot(char *from, char *to, char *rest)
{
	char *newlevel, *nuh;
	int level = 0;

	nuh = strtok(rest, " ");
	if (!nuh || !*nuh)
	{
	  bot_reply(from, 4);
	  return;
	}
	nuh = find_person( from, nuh);
	if (!*nuh)
		return;

	if (!isuser(nuh))
	{
		send_to_user( from, "\x2%s is not in my users list\x2", nuh);
		return;
	}

	if (!(newlevel = strtok(NULL, " ")))
	{
	  bot_reply(from, 9);
	  return;
	}

	level = protlevel(nuh);

	if ( (atoi(newlevel) <= 0) || (atoi(newlevel)) > MAXPROTLEVEL)
	{
		send_to_user( from, "\x2Valid protect levels are 1 through %i!!!\x2",
			MAXPROTLEVEL);
		send_to_user( from, "\x2To unprotect, use \"%crprot\"\x2",
			     cmdchar);
		return;
	}

	if (!change_userlist (nuh, -1, -1, atoi(newlevel)))
	{
		send_to_user( from, "\x2%s is not in my users list\x2", nuh);
		return;
	}
	if (level)
		send_to_user( from, "\x2Protect level changed from %i to %i\x2",
			level, atoi(newlevel));
	else
		send_to_user( from, "\x2User now protected with level %i\x2",
			atoi(newlevel));
	return;
}

void do_nprot(char *from, char *to, char *rest)
{
	char *nuh;
	int level = 0;

	nuh = strtok(rest, " ");
	if (!nuh || !*nuh)
	{
	  bot_reply(from, 4);
	  return;
	}
	nuh = find_person( from, nuh);
	if (!*nuh)
		return;

	if (!isuser(nuh))
	{
		send_to_user( from, "\x2%s is not in my users list\x2", nuh);
		return;
	}
	level = protlevel(nuh);
	if (!level)
	{
		send_to_user( from, "\x2User is already not being protected\x2");
		return;
	}
	if (!change_userlist (nuh, -1, -1, 0))
	{
		send_to_user( from, "\x2%s is not in my users list\x2", nuh);
		return;
	}
	send_to_user( from, "\x2User is now not being protected\x2");
	return;
}

void do_clvl(char *from, char *to, char *rest)
{
  char *newlevel, *nuh;
  int level=0;

	nuh = strtok(rest, " ");
	if (!nuh || !*nuh)
	{
	  bot_reply(from, 4);
	  return;
	}
	nuh = find_person( from, nuh);
	if (!*nuh)
		return;

  if (!isuser(nuh))
    {
      send_to_user(from, "\x2%s is not in my userlist\x2", nuh);
      return;
    }
  if (!matches(owneruhost, nuh))
    {
      send_to_user(from, "\x2Sorry, can't change owner's level\x2");
      return;
    }
  if (userlevel(nuh) >= userlevel(from))
    {
      send_to_user(from, "\x2Sorry, the user has a higher or equal access level\x2");
      return;
    }

  if (!(newlevel = strtok(NULL, " ")))
  {
    bot_reply(from, 9);
    return;
  }

  level = atoi(newlevel);

  if (level > userlevel(from))
  {
	  send_to_user(from, "\x2Sorry, can't change to higher than your level\x2");
	  return;
  }
  if ((level <= 0) || (level > 100))
  {
	 if (!level)
		send_to_user(from, "\x2Use \"%cdel\" to delete a user\x2", cmdchar);
	 else
		send_to_user(from, "\x2Valid levels are from 1 to 100\x2");
	 return;
  }
  if (!change_userlist(nuh, level, -1, -1))
    {
      send_to_user(from, "\x2Problem changing the level\x2");
      send_to_user(from, "\x2Not in userlist???\x2");
      return;
    }
  send_to_user(from ,"%cAccess level for the user now changed to %i\x2",'\x2',level);
  send_to_user(nuh, "\x2%s has changed your access level to %i\x2", getnick(from), level);
  return;
}

void do_addks(char *from, char *to, char *rest)
{
	char ks[MAXLEN];
	if (!*rest)
	{
		send_to_user( from, "\x2Specify a string of text to kick upon when said\x2");
		return;
	}
	strcpy(ks, "*");
	strcat(ks, rest);
	strcat(ks, "*");
	if (is_in_list(KickSays, ks))
	{
		send_to_user( from, "%cAlready kicking on the say of that\x2",
			'\x2');
		if (KickSayChan == NULL)
			send_to_user( from, "\x2WARNING: No channels selected for auto-kick\x2");
		return;
	}
	if (add_to_list(&KickSays, ks))
	{
		send_to_user( from, "\x2Now kicking on the say of %s\x2", ks);
		if (KickSayChan == NULL)
			send_to_user( from, "\x2WARNING: No channels selected for auto-kick\x2");
	}
	else
		send_to_user( from, "\x2Problem adding the kicksay\x2");
	return;
}

void do_addksc(char *from, char *to, char *rest)
{
	if (!*rest || !ischannel(rest))
	{
	  bot_reply(from, 10);
	  return;
	}
	if (!mychannel(rest) && (*rest != '*'))
	  {
	    bot_reply(from, 11);
	    return;
	  }
	if (is_in_list(KickSayChan, rest))
	{
		send_to_user( from, "%cAlready auto-kicking on that channel\x2",
		    '\x2');
		return;
	}
	if (add_to_list(&KickSayChan, rest))
		send_to_user( from, "\x2Now auto-kicking on %s\x2", rest);
	else
		send_to_user( from, "\x2Problem adding the channel\x2");
	return;
}

void do_clrks(char *from, char *to, char *rest)
{
	if (KickSays == NULL)
	{
		send_to_user( from, "\x2There are no kicksays\x2");
		return;
	}
	remove_all_from_list( &KickSays);
	send_to_user( from, "%cAll kicksays have been removed\x2", '\x2');
	return;
}

void do_clrksc(char *from, char *to, char *rest)
{
	if (KickSayChan == NULL)
	{
		send_to_user( from, "\x2There are no kicksay channels\x2");
		return;
	}
	remove_all_from_list( &KickSayChan);
	send_to_user( from, "%cAll channels have been removed\x2", '\x2');
	return;
}

void do_delks(char *from, char *to, char *rest)
{
	char ks[MAXLEN];
	if (!*rest)
	{
		send_to_user( from, "\x2Specify the string of text to delete\x2");
		return;
	}
	strcpy(ks, rest);
	if (!is_in_list(KickSays, ks))
	{
		send_to_user( from, "\x2I'm not kicking on the say of that\x2");
		return;
	}
	if (remove_from_list(&KickSays, ks))
		send_to_user( from, "\x2The kicksay has been deleted\x2");
	else
		send_to_user( from, "\x2Problem deleting the kicksay\x2");
	return;
}

void do_delksc(char *from, char *to, char *rest)
{
	char ks[MAXLEN];
	if (!*rest)
	{
	  bot_reply(from, 10);
	  return;
	}
	strcpy(ks, rest);
	if (!is_in_list(KickSayChan, ks))
	{
		send_to_user( from, "\x2I'm not auto-kicking on that channel\x2");
		return;
	}
	if (remove_from_list(&KickSayChan, ks))
		send_to_user( from, "\x2The channel has been deleted\x2");
	else
		send_to_user( from, "\x2Problem deleting the channel\x2");
	return;
}

void do_listks(char *from, char *to, char *rest)
{
	ListStruct *temp;
	int i = 0;
	if (KickSays == NULL)
	{
		send_to_user( from, "\x2There are currently no kicksays\x2");
		return;
	}
	send_to_user( from, "--- List of Kicksays: ---" );
	for (temp = KickSays; temp; temp = temp->next)
	{
		i++;
		send_to_user( from, "\x2%2i -- %s\x2", i, temp->name);
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
		send_to_user( from, "\x2There are currently no kicksay channels\x2");
		return;
	}
	send_to_user( from, "--- List of Kicksay channels: ---" );
	for (temp = KickSayChan; temp; temp = temp->next)
	{
		i++;
		send_to_user( from, "\x2%2i -- %s\x2", i, temp->name);
	}
	send_to_user( from, "--- End of list ---");
	return;
}


void do_massunban( char *from, char *to, char *rest )
{
	char  *channel;

	if((channel = strtok(rest, " "))==NULL)
		channel_massunban(currentchannel());
	else
		channel_massunban(channel);
}

void do_unban( char *from, char *to, char *rest )
{
	char *nuh;
	char channel[MAXLEN];

	strcpy(channel, find_channel(to, &rest));

	nuh = strtok(rest, " ");
	if (!nuh || !*nuh)
	{
	  bot_reply(from, 4);
	  return;
	}
	nuh = find_person( from, nuh);
	if (!*nuh)
		return;
	channel_unban(channel, nuh);
	send_to_user( from, "\x2%s unbanned on %s\x2", nuh, channel);
}

void do_unbanme(char *from, char *to, char *rest)
{
  char channel[MAXLEN];
  strcpy(channel, find_channel(to, &rest));
  channel_unban(channel, from);
  send_to_user(from, "\x2You are unbanned on %s\x2", channel);
}

void do_listdcc( char *from, char *to, char *rest )
{
#ifndef MSDOS
	show_dcclist( from );
#endif
}

void do_spawn(char *from, char *to, char *rest)
{
	char buf[MAXLEN];
	char *nick, *args;
	FILE *out;
	int i;

	if (!*rest)
	{
		send_to_user(from , "\x2Usage: Spawn Nick Arguments\x2");
		return;
	}
	nick = strtok(rest, " ");
	args = strtok(NULL, "\n");
	if (!args)
		args = "";
	sprintf(buf, "%s%s.servers", bot_dir, nick);
	if (!exist(buf) && ((out=fopen(buf, "w")) != NULL))
	  {
	    for (i=0;*(bot.servers[i].servername);i++)
	      fprintf(out, "%s\n", bot.servers[i].servername);
	    fclose(out);
	  }
	sprintf(buf, "%s%s.userlist", bot_dir, nick);
	if (!exist(buf))
	  write_userlist( buf);
	sprintf(buf, "%s%s.shitlist", bot_dir, nick);
	if (!exist(buf))
	  write_shitlist( buf);
	sprintf(buf, "%s%s.userhost", bot_dir, nick);
	if (!exist(buf))
	  write_userhost( buf);
	
	sprintf(buf, "-n %s ", nick);
	strcat(buf, args);
	make_process(progname, buf);
}


void do_topic(char *from, char *to, char *rest)
{
	char channel[MAXLEN];
	strcpy(channel, find_channel(to, &rest));
	if (!*rest)
	{
		send_to_user(from, "\x2What's the topic???\x2");
		return;
	}
	send_to_server( "TOPIC %s :%s", channel, rest);
	send_to_user(from, "\x2Topic changed on %s\x2", channel);
	return;
}

void do_spymsg(char *from, char *to, char *rest)
{
	if (is_in_list(SpyMsgList, getnick(from)))
	{
		send_to_user( from, "%cAlready redirecting messages to you\x2",
			'\x2');
		return;
	}
	if (add_to_list(&SpyMsgList, getnick(from)))
		send_to_user( from, "\x2Now redirecting messages to you\x2");
	else
		send_to_user( from, "%cError: Can't redirect you the messages\x2",
			'\x2');
	return;
}

void do_nospymsg(char *from, char *to, char *rest)
{
	if (!is_in_list(SpyMsgList, getnick(from)))
	{
		send_to_user( from, "%cI'm not redirecting messages to you\x2",
			'\x2');
		return;
	}
	if (remove_from_list(&SpyMsgList, getnick(from)))
		send_to_user( from, "\x2No longer redirecting messages to you\x2");
	else
		send_to_user( from, "%cError: Can't stop redirecting you the messages\x2",
			'\x2');
	return;
}


void do_cmdchar(char *from, char *to, char *rest)
{
	char temp;
	if (!*rest)
	{
		send_to_user(from, "\x2What's the new command character?\x2");
		return;
	}
	temp = *rest;
	if ( isalnum(temp))
		send_to_user(from, "\x2Sorry, that command character is not allowed\x2");
	else
	{
		cmdchar = temp;
		send_to_user(from, "\x2My command character is now \"%c\"\x2",
			cmdchar);
	}
	return;
}

void do_holdnick(char *from, char *to, char *rest)
{
	char *themsg;
	if (holdnick)
	{
		if (*rest)
		{
			themsg = strtok(rest, "\n");
			if (themsg)
			  {
			    strcpy(nhmsg, themsg);
			  }
			if (holdingnick)
			  send_to_server("AWAY :%s", nhmsg);
			send_to_user(from, "\x2Now sending new nickhold message\x2");
			return;
		}
		send_to_user(from, "\x2Already holding %s's nick\x2", ownernick);
		return;
	}
	else if (*rest)
	  {
	    themsg = strtok(rest, "\n");
	    if (themsg)
	      {
		strcpy(nhmsg, themsg);
	      }
	  }
	else
	  strcpy(nhmsg, "");
	holdnick = 1;
	send_to_user(from, "\x2Now holding %s's nick\x2", ownernick);
	return;
}

void do_noholdnick(char *from, char *to, char *rest)
{
	if (!holdnick)
	{
		send_to_user(from, "\x2I'm not holding %s's nick\x2", ownernick);
		return;
	}
	holdnick = 0;
	holdingnick = 0;
	send_to_user(from, "\x2No longer holding %s's nick\x2", ownernick);
	return;
}


void do_seen(char *from, char *to, char *rest)
{
	char nuh[255];
	char nuh2[255], buf[30];
	time_t lasttime;
	UserHostStruct *temp = UserHostList;
	  int num;

	if (!*rest && !pubseen)
	{
	  bot_reply(from, 4);
	  return;
	}

	  rest = strtok(rest, " ");
	if (*(rest+strlen(rest)-1) == '?')
	  *(rest+strlen(rest)-1) = '\0';
	num = 0;
	while(temp)
	  {
	    temp = find_userhostfromnick(temp, rest);
	    if (temp)
	      {
		num++;
		strcpy(buf, ctime(&temp->time));
		if (strlen(buf))
		  *(buf+strlen(buf)-1) = '\0';
		if (!pubseen)
		  {
		    send_to_user(from, "\x2%s:\x2", temp->name);
		    send_to_user(from, "   \x2Last seen %s on %s\x2",
			     (temp->time) ? buf : "<UNKNOWN>", temp->channel);
		  }
		else if ((temp->time) && (strcasecmp(temp->channel, "<UNKNOWN>")))
		  {
		    if (num == 2)
		      sendprivmsg(to, "\x2%s\x2 There are more userhosts for that nick...To see the whole seen data do: %cseen <nick>", getnick(from), cmdchar);
		    if (num == 1)
		      {
			sendprivmsg(to, "\x2%s:\x2 I've seen %s", getnick(from), temp->name);
			sendprivmsg(to, "I last saw him/her on %s at %s",
				    temp->channel, buf);
		      
		      }
		  }
		temp = temp->next;
	      }
	  }
       if (num)
	  return;
	if (!pubseen)
	  send_to_user(from, "\x2No userhost information found\x2");
	else
	  sendprivmsg(to, "\x2%s:\x2 I haven't seen %s",getnick(from), rest);
	return;
}

void do_resetuhost(char *from, char *to, char *rest)
{
	remove_all_from_userhostlist();
	send_to_user(from, "\x2Starting fresh userhost list\x2");
	return;
}

void do_getuserhost(char *from, char *to, char *rest)
{
	char nuh[255];
	char nuh2[255];
	  int num = 0;

  UserHostStruct *temp = UserHostList;

	if (!*rest)
	{
	  bot_reply(from, 4);
	  return;
	}
	  rest = strtok(rest, " ");
		 
	num = 0;
	while (temp)
	  {
	    temp = find_userhostfromnick(temp, rest);
	    if (temp)
	      {
		num++;
		send_to_user(from, "Possible userhost: %s\x2", temp->name);
		temp = temp->next;
	      }
	  }
	if (num)
	  return;
	strcpy(nuh, rest);
 strcat(nuh, "!");
 strcpy(nuh2, getuserhost(rest));
 if (!*nuh2)
   {
     send_to_user(from, "\x2There is no information for that nickname\x2");
     return;
   }
 strcat(nuh, nuh2);
 send_to_user(from, "\x2%s is %s\x2", rest, nuh);
 return;
}

void do_totstats(char *from, char *to, char *rest)
{
	char nuh[255];
	char nuh2[255];
	UserHostStruct *user;
	static time_t currenttime, lasttime = 0;
	static int num = 0;

	if (!*rest)
	{
	  bot_reply(from, 4);
	  return;
	}
	if (strchr(rest, ' '))
		*strchr(rest, ' ') = '\0';
	strcpy(nuh, getuserhost_fromlist(rest));
	strcpy(nuh2, rest);
	strcat(nuh2, "!");
	strcat(nuh2, nuh);
	user = find_userhost(nuh2);
	if (!*nuh || !user)
	{
	  send_to_user(from,"\x2There is no information for that nickname\x2");
	  send_to_user(from,"\x2or there is more than one user@host for that nick\x2");
	}
	else
	  {
	    send_to_user(from, "\x2Total Stats for %s:\x2", nuh2);
	    send_to_user(from, "\x2------------------------------\x2");
	    send_to_user(from, "Number of ops given:     %5i", user->totop);
	    send_to_user(from, "Number of deops done:    %5i", user->totdeop);
	    send_to_user(from, "Number of bans done:     %5i", user->totban);
	    strcpy(nuh, ctime(&user->kicktime));
	    if (strchr(nuh, '\n'))
	      *strchr(nuh, '\n') = '\0';
	    if (!user->totkick)
	      strcpy(nuh, "NEVER");
	    send_to_user(from, "Number of people kicked: %5i", user->totkick);
	    send_to_user(from, "     last one at approx:     %s", nuh);
	    strcpy(nuh, ctime(&user->killtime));
	    if (strchr(nuh, '\n'))
		*strchr(nuh, '\n') = '\0';
	    if (!user->totkill)
	      strcpy(nuh, "NEVER");
	    send_to_user(from, "Number of people killed: %5i", user->totkill);
	    send_to_user(from, "     last one at approx:     %s", nuh);
	    send_to_user(from, "\x2------ End of Tot Stats ------\x2");
	  }
	return;
}

void do_getmsg(char *from, char *to, char *rest)
{
  int num;
  UserHostStruct *user;
  MessagesStruct *mess;
  
  user = find_userhost(from);
  if (!user)
    {
      update_userhostlist(from, "", getthetime());
      send_to_user(from, "\x2No more messages\x2");
      return;
    }
  if (!*rest)
    num = user->msgnum+1;
  else
    num = atoi(rest);
  if (num > user->totmsg)
    {
      send_to_user(from, "\x2No more messages\x2");
      return;
    }
  mess = get_message(user->messages, num);
  if (!mess)
    {
      send_to_user(from, "\x2Message #%i doesn't exist\x2", num);
      send_to_user(from, "%cCurrent Msg: #%i,  Last Msg: %i\x2", '\x2',
		   user->msgnum+1, user->totmsg);
      return;
    }
  if (num > user->msgnum)
    user->msgnum = num;

  send_to_user(from, "\x2Message %i of %i\x2", num, user->totmsg);
  send_to_user(from, "%cFrom: %s\x2", '\x2', mess->from);
  send_to_user(from, "\x2Time: %s\x2", ctime(&mess->time));
  send_to_user(from, "\x2-----------------------------------------------\x2");
  send_to_user(from, "\x2%s\x2", mess->text);
  if (num == user->totmsg)
    send_to_user(from, "\x2-------------- No more messages ---------------\x2");
  else
    send_to_user(from, "\x2---------------- More messages ----------------\x2");

  return;
}

void do_sendmsg(char *from, char *to, char *rest)
{
  char nuh[MAXLEN], nuh2[MAXLEN];
  char *nick;
  UserHostStruct *temp;
  char *ptr;

  nick = strtok(rest, " ");

  rest = strtok(NULL, "\n");
  if (!nick || !rest)
    {
      send_to_user(from, "\x2Usage: sm [nick or nuh] [themessage]\x2");
      return ;
    }
  if (!strchr(nick, '@') && (num_userhost(nick) != 1))
    {
      send_to_user(from, "\x2That nick doesn't exist or there is more than\x2");
      send_to_user(from, "\x2one userhost for it...please do sm n!u@h message\x2");
      return;
    }
  if (!strchr(nick, '@'))
    {
      temp = find_userhostfromnick(UserHostList, nick);
      strcpy(nuh, temp->name);
    }
  else
    {
      strcpy(nuh, nick);
      temp = find_userhost(nuh);
      if (!temp)
	{
	  update_userhostlist(nuh, "<UNKNOWN>", 0);
	  temp = find_userhost(nuh);
	  if (!temp)
	  {
	     send_to_user(from, "\x2No such nick!user@host known\x2");
	     return;
	  }
	}
    }
  if (add_message(&temp->messages, from, rest, temp->totmsg+1))
    {
      temp->totmsg = temp->totmsg + 1;
      send_to_user(from, "\x2Message saved to %s\x2", nuh);
      ptr = strchr(nuh, '!');
      ptr++;

      strcpy(nuh2, getuserhost(getnick(nuh)));
      if (*nuh2)
	if (!strcasecmp(nuh2, ptr))
	  {
	    send_to_user(nuh, "\x2You have new mail\x2");
	    send_to_user(nuh, "\x2Use \"%cgetmsg\" to read messages\x2", cmdchar);
	  }
   }
  else
    send_to_user(from, "\x2Problem saving the message\x2");
  return;
}

void do_scanmsg( char *from, char *to, char *rest)
{
  UserHostStruct *user;
  MessagesStruct *mess;
  char fr[255];
  
  user = find_userhost(from);
  if (!user)
    {
      send_to_user(from, "\x2No more messages\x2");
      return;
    }
  send_to_user(from, "\x2 Num         From                    Message\x2");
  for (mess=user->messages;mess;mess = mess->next)
    {
      strcpy(fr, left(mess->from, 20));
      send_to_user(from, "\x2[%3i] %20s %37s\x2", mess->num, fr,
		   left(mess->text,37));
    }
  send_to_user(from, "\x2---------- end of scan -------------\x2");
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
      send_to_user(from, "\x2  Usage: heart where nick\x2");
      send_to_user(from, "%cExample: heart #TeenSex JoeBlow\x2", '\x2');
      return;
    }
  *(nick+10) = '\0';
  for(i=1;i<(10-strlen(nick))/2;i++)
      strcat(nick, " ");
  if (ischannel(sendto))
      {
	sendprivmsg(sendto,"\x2          ..     ..\x2");
	sendprivmsg(sendto,"\x2       .`    `.\'    \'.\x2");
	sendprivmsg(sendto,"\x2       .%10s   .\x2", nick);
	sendprivmsg(sendto,"\x2         .         .\x2");
	sendprivmsg(sendto,"\x2            .   .\x2");
	sendprivmsg(sendto,"\x2              .\x2");
    }
  else
      {
	sendnotice(sendto,"\x2          ..     ..\x2");
	sendnotice(sendto,"\x2       .`    `.\'    \'.\x2");
	sendnotice(sendto,"\x2       .%10s   .\x2", nick);
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

void do_last10(char *from, char *to, char *rest)
{
	int i;

	send_to_user(from, "\x2Last 10 commands:\x2");
	send_to_user(from, "\x2-------------------------\x2");
	for(i=0;i<10;i++)
		send_to_user(from, "\x2%2i: %s\x2", i+1, lastcmds[i]);
	send_to_user(from, "\x2------ end of list ------\x2");
	return;
}

void do_chaccess(char *from, char *to, char *rest)
{
	int i, oldlevel, newlevel;
	char *name, *level;

	name = strtok(rest, " ");
	level = strtok(NULL, " ");
	if (!*rest || !name)
	{
		send_to_user(from, "\x2Usage: chaccess <commandname> [level]\x2");
		return;
	}
	if (level)
	{
		newlevel = atoi(level);
		if ((newlevel < 0) || (newlevel > 100))
		{
			send_to_user(from, "\x2Level must be between 0 and 100\x2");
			return;
		}
	}
	else
		newlevel = -1;
	if (newlevel > userlevel(from))
	{
		send_to_user(from, "%cCan't change level to one higher than yours\x2",
			'\x2');
		return;
	}
	for( i = 0; on_msg_commands[i].name != NULL; i++ )
	{
		if (!strcasecmp( on_msg_commands[i].name, name))
		{
			oldlevel = on_msg_commands[i].userlevel;
			if (newlevel == -1)
			{
				send_to_user(from, "\x2The access level needed for that command is %i\x2",
					oldlevel);
				send_to_user(from, "\x2To change it, specify a level\x2");
				return;
			}
			if (oldlevel > userlevel(from))
			{
				send_to_user(from, "%cCan't change a level that is higher than yours\x2",
					'\x2');
				return;
			}
			if (oldlevel == newlevel)
				send_to_user(from, "\x2The level was not changed\x2");
			else
				send_to_user(from, "\x2Level changed from %i to %i\x2",
					oldlevel, newlevel);
			on_msg_commands[i].userlevel = newlevel;
			return;
		}
	}
	send_to_user(from, "\x2That command is not known\x2");
	return;
}

void do_server(char *from, char *to, char *rest)
{
  char *login, *ircname;

  int i = 1;
  rest = strtok(rest, " ");
  if (!rest || !*rest)
    {
      send_to_user(from, "\x2Usage: server <servername> [login] [ircname]\x2");
      return;
    }
  login = strtok(NULL, " ");
  ircname = strtok(NULL, "\n");
  if (login)
    strcpy(bot.login, login);
  if (ircname)
    strcpy(bot.ircname, ircname);
  send_to_user(from, "\x2Trying new server: %s\x2", rest);
  close(bot.server_sock);
  sleep(1);
  if ((bot.server_sock = connect_by_number(6667, rest)) < 0)
    {
      i = 0;
      reconnect_to_server();
    }
  else
    {
      sendregister(bot.nick, bot.login, bot.ircname);
      reset_channels(HARDRESET);
    }
  if (i)
    send_to_user(from, "\x2Server change successful\x2");
  else
    send_to_user(from, "\x2Server change unsuccessful\x2");
  return;
}

void do_nextserver(char *from, char *to, char *rest)
{
  char *login, *ircname;
  login = strtok(rest, " ");
  ircname = strtok(NULL, "\n");
  if (login)
    strcpy(bot.login, login);
  if (ircname)
    strcpy(bot.ircname, ircname);
  sleep(1);

  send_to_user(from, "\x2Jumping to next server:%s\x2", bot.servers[bot.current].servername);
  reconnect_to_server();
  send_to_user(from, "\x2Now on server:%s\x2", bot.servers[bot.current].servername);
  return;
}

void do_top10kills(char *from, char *to, char *rest)
{
  UserHostStruct *user;
  char buffer[255], nicks[10][100]={"","","","","","","","","",""};
  int i=0, kills[10] = {0,0,0,0,0,0,0,0,0,0};
  int ii, smallest, smallnum, save;
  int temp[10] = {0,1,2,3,4,5,6,7,8,9};
  for (user=UserHostList;user;user=user->next)
    {
      if (user->totkill)
	{
	  for (i=0;i<10;i++)
	    {
	      if (!*nicks[i])
		{
		  strcpy(nicks[i], getnick(user->name));
		  kills[i] = user->totkill;
		  break;
		}
	      if (user->totkill >= kills[i])
		{
		  smallest = kills[0];
		  smallnum = 0;
		  for (ii=1;ii<10;ii++)
		    if (kills[ii] < smallest)
		      {
			smallnum = ii;
			smallest = kills[ii];
		      }
		  kills[smallnum] = user->totkill;
		  strcpy(nicks[smallnum], getnick(user->name));
		  break;
		}
	    }
	}
    }
  for (i=0;i<9;i++)
    for (ii=i+1;ii<10;ii++)
      if (kills[temp[ii]] > kills[temp[i]])
	{
	  save = temp[ii];
	  temp[ii] = temp[i];
	  temp[i] = save;
	}
  send_to_user(from, "\x2------- Top Ten Kills ------\x2");
  send_to_user(from, "\x2   Oper         # of kills\x2");
  send_to_user(from, "\x2------------+---------------\x2");
  for (i=0;i<10;i++)
    send_to_user(from, "\x2 %10s |   %4i\x2", nicks[temp[i]], kills[temp[i]]);
  send_to_user(from, "\x2------------+---------------\x2");
  send_to_user(from, "\x2------- end of list --------\x2");
  return;
}

void do_logchat(char *from, char *to, char *rest)
{
  ListStruct *temp;
  rest = strtok(rest, " ");
  
  if (!rest || !*rest)
    {
      bot_reply(from, 10);
      return;
    }
  if (!mychannel(rest))
    {
      bot_reply(from, 11);
      return;
    }
  if (is_in_list(LogChatList, rest))
    {
      send_to_user( from, "%cAlready logging chat on that channel\x2",
		   '\x2');
      return;
    }
  if (add_to_list(&LogChatList, rest))
    send_to_user( from, "\x2Now logging chat on %s\x2", rest);
  else
    send_to_user( from, "\x2Problem logging chat\x2");
  return;
}

void do_rlogchat(char *from, char *to, char *rest)
{
  ListStruct *temp;
  rest = strtok(rest, " ");
  if (!rest || !*rest)
    {
      bot_reply(from, 10);
      return;
    }
  if (!is_in_list(LogChatList, rest))
    {
      send_to_user( from, "%cAlready not logging chat on that channel\x2",
		   '\x2');
      return;
    }
  if (remove_from_list(&LogChatList, rest))
    send_to_user( from, "\x2No longer logging chat on %s\x2", rest);
  else
    send_to_user( from, "\x2Problem removing the log chat\x2");
  return;
}

void do_removeuh(char *from, char *to, char *rest)
{

  if (!*rest || !strchr(rest, '!') || !strchr(rest, '@'))
    {
      send_to_user(from, "\x2Specify the full nick!user@host to delete\x2");
      return;
    }
  
  if (!remove_userhost(rest))
    {
      send_to_user(from, "\x2%s not found in memory\x2", rest);
      return;
    }
  send_to_user(from, "\x2%s removed from memory\x2", rest);
  return;
}

void do_saveuserhost(char *from, char *to, char *rest)
{
  if (!write_userhost(userhostfile))
      send_to_user(from, "\x2Userhost info could not be save to file: %s\x2",
		   userhostfile);
  else
      send_to_user(from, "\x2Userhost info saved to file: %s\x2",
		   userhostfile);
}

void do_ping(char *from, char *to, char *rest)
{
  if (ischannel(to))
    sendprivmsg(to, "\x2Pong\x2");
  else
    send_to_user(from, "\x2Pong\x2"); 
  return;
}

void do_steal(char *from, char *to, char *rest)
{
  ListStruct *temp;

  if (!*rest || !ischannel(rest))
    {
      send_to_user( from, "\x2Trying to steal the following channels:\x2");
      send_to_user( from, "\x2---------------------------------------\x2");
      for(temp=StealList;temp;temp=temp->next)
	send_to_user( from, "\x2%s\x2", temp->name);
      send_to_user( from, "\x2-------------- end of list ------------\x2");
      return;
    }
  if (is_in_list(StealList, rest))
    {
      send_to_user( from, "%cAlready stealing that channel\x2", '\x2');
      return;
    }
  if (add_to_list(&StealList, rest))
    {
      send_to_user( from, "\x2Now trying to steal %s\x2", rest);
      join_channel(rest);
    }
  else
    send_to_user( from, "\x2Problem adding the channel\x2");
  return;
}

void do_nosteal(char *from, char *to, char *rest)
{
  if (!*rest || !ischannel(rest))
    {
      bot_reply(from, 10);
      return;
    }
  if (!is_in_list(StealList, rest))
    {
      send_to_user( from, "\x2I'm not stealing that channel\x2");
      return;
    }
  if (remove_from_list(&StealList, rest))
    send_to_user( from, "\x2No longer stealing %s\x2", rest);
  else
    send_to_user( from, "\x2Problem deleting the channel\x2");
  leave_channel(rest);
  return;
}

void do_cycle(char *from, char *to, char *rest)
{
  char channel[MAXLEN];
  strcpy(channel, find_channel(to, &rest));
  
  send_to_user(from, "%cCycling channel %s\x2", '\x2', channel);
  cycle_channel(channel);
}

void do_kinit(char *from, char *to, char *rest)
{
#ifndef KINIT
  send_to_user(from, "\x2KINIT not defined in config.h\x2");
  return;
#endif
  make_process(kinitfile, "");
  send_to_user(from, "\x2Running %s\x2", kinitfile);
}

void do_display(char *from, char *to, char *rest)
{
	FILE  *in;
	char buffer[255];
	char filename[255];
	int level, ulevel;


	strcpy(filename, download_dir);

	if (!*rest)
	{
		send_to_user(from,
			 "\x2Use \"%cdisplay <filename>\" to view one of these files:\x2",
			 cmdchar);
		if( ( in = fopen( filedispfile, "r" ) ) == NULL )
		{
			send_to_user(from, "\x2No files available\x2");
			return;
		}
		send_to_user(from, "\x2-------- Filename --------+- Level Needed -\x2");
		while (!feof(in))
		{
		  if (fscanf(in, "%s %i\n", buffer, &level))
		    send_to_user(from, "%25s |    %3i", buffer, level);
		}
		send_to_user(from, "\x2--------------------------+----------------\x2");
		fclose( in );
		return;
	}
	ulevel = userlevel(from);
	if ((in = fopen(filedispfile, "r"))==NULL)
	{
		send_to_user( from, "\x2Problem opening %s\x2", filedispfile);
		return;
	}
	while (!feof(in))
	{
		fscanf(in, "%s %i\n", buffer, &level);
		if (!strcmp(rest, buffer))
		{
			if (level > ulevel)
			{
			  send_to_user(from, "\x2Sorry, you need access level %i to get this file\x2", level);
			  return;
			}
			fclose(in);
			strcat(filename, rest);
			if ((in = fopen(filename, "r")) == NULL)
			{
				send_to_user(from, "\x2Problem opening the file\x2");
				return;
			}
			while (!feof(in))
				if (readln_from_a_file( in, buffer))
					send_to_user(from, buffer);
			fclose(in);
			return;
		}
	}
	send_to_user(from, "\x2That file does not exist...use \"%cdisplay\"\x2",
			 cmdchar);
	fclose(in);
}

void do_spy(char *from, char *to, char *rest)
{
	if (!*rest)
	{
		send_to_user(from,
			 "\x2The least you could do is give me a fucking channel name\x2");
		return;
	}
	if (!mychannel(rest))
	{
		send_to_user(from, "\x2I'm not on that fucking channel\x2");
		return;
	}
	if (add_spy_chan(rest, from))
		send_to_user(from, "\x2Now spying on %s for you\x2", rest);
	else
		send_to_user(from, "\x2You are already spying on %s, dilweed\x2", rest);
	return;
}

void do_rspy(char *from, char *to, char *rest)
{
	if (!*rest)
	{
		send_to_user(from,
			 "\x2The least you could do is give me a fucking channel name\x2");
		return;
	}
	if (!mychannel(rest))
	{
		send_to_user(from, "\x2I'm not on that fucking channel\x2");
		return;
	}
	if (del_spy_chan(rest, from))
		send_to_user(from, "\x2No longer spying on %s for you\x2", rest);
	else
		send_to_user(from, "\x2You are not spying on %s, dilweed\x2", rest);
	return;
}

void do_reread(char *from, char *to, char *rest)
{
	if (rereadsettings(1))
		send_to_user(from, "\x2%s reread into memory\x2", settingsfile);
	else
		send_to_user(from, "\x2Problem rereading %s\x2", settingsfile);
}

void do_spylist(char *from, char *to, char *rest)
{
     if (!*rest)
       {
	 bot_reply(from, 10);
	 return;
       }
     
     send_to_user(from, "\x2--- Spylist for %s\x2", rest);
     show_spy_list(from, rest);
     send_to_user(from, "\x2--- end of list ---\x2");
}

