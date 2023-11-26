/*
 * com-ons.c - kinda like /on ^.. in ircII
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>

#include "global.h"
#include "config.h"
#include "defines.h"
#include "structs.h"
#include "h.h"
#include "commands.h"

void addseen(char *nick, int type, char *rest)
{
	char		*userhost, *kicker, *signoff;
	time_t		thetime;
	aSeen	    *seentemp;
	char		tbuf[MAXLEN];
	
	if (username(nick)) {
		userhost = username(nick);
	}
	else {
		userhost = nick;
	}
	
	if (type == 4) {
		kicker = get_token(&rest, " ");
		sprintf(tbuf, "%s", kicker);
	}
	else {
		sprintf(tbuf, "None");
	}
	
	signoff = rest;
	thetime = getthetime();
	
	seentemp = make_seen(&current->SeenList, nick, userhost, tbuf, thetime, type, signoff);
}

void do_showseen(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	aSeen	*seenptr;
	int i = 0;

	seenptr = current->SeenList;

	while (seenptr != NULL)
	{
/*		switch(seenptr->selector)
		{
			case 0: break;
			case 1:
				send_to_user(from,"\002%s\002 parted from %s on %s", seenptr->userhost, seenptr->signoff, time2str(seenptr->time));
				break;
			case 2:
				send_to_user(from,"\002%s\002 signed off on %s with message [%s]",
					seenptr->userhost, time2str(seenptr->time),
					seenptr->signoff);
				break;
			case 3:
				send_to_user(from,"\002%s\002 changed nicks to \002%s\002 on %s", seenptr->userhost, seenptr->signoff, time2str(seenptr->time));
				break;
			case 4:
				send_to_user(from,"\002%s\002 was kicked on %s by %s with message [%s]",
					seenptr->userhost, time2str(seenptr->time),
					seenptr->kicker, seenptr->signoff);
				break;
			default: break;
		}
*/		
		seenptr = seenptr->next;
		i++;
	}
	send_to_user(from, "\002SeenList currently has %i entries\002", i);
}

void do_seen(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char	*nick, *chan;
	aSeen	*seenptr;
	
	chan = get_channel(to, &rest);

	if (!rest)
	{
		send_to_user(from,"\002Who do you want me look for?\002");
		return;
	}

	nick = get_token(&rest," ");
	
	if (strlen(nick) > MAXNICKLEN) {
		send_to_user(from, "\002Illegal Nick - String Length Exceeded %d\002", MAXNICKLEN);
		return;
	}
	
	if (!matches(nick, current->nick)) {
		if (ischannel(to)) {
			sendprivmsg(chan, "\002%s is me you idiot!\002", current->nick);
		}
		else {
			send_to_user(from, "\002%s is me you idiot\002", current->nick);
		}
		return;
	}
	
	if (!matches(nick, getnick(from))) {
		if (ischannel(to)) {
			sendprivmsg(chan, "\002Trying to find yourself %s?\002", nick);
		}
		else {
			send_to_user(from, "\002Trying to find yourself %s?\002", nick);
		}
		return;
	}

	if ((seenptr = find_seen(&current->SeenList, nick)) == NULL) {
		if (ischannel(to)) {
			sendprivmsg(chan, "\002I have no memory of %s\002", nick);
		}
		else {
			send_to_user(from,"\002I have no memory of %s\002",nick);
		}
		return;
	}

	switch (seenptr->selector)
	{
		case 0:
			if (ischannel(to)) {
				sendprivmsg(chan,"\002%s is right here moron!\002",nick);
			}
			else {
				send_to_user(from, "\002%s is right here moron!\002", nick);
			}
			break;
		case 1:
			if (ischannel(to)) {
				sendprivmsg(chan,"\002%s\002 parted from \037%s\037 on \002%s\002", seenptr->userhost, seenptr->signoff, time2away(seenptr->time));
			}
			else {
				send_to_user(from,"\002%s\002 parted from \037%s\037 on \002%s\002", seenptr->userhost, seenptr->signoff, time2away(seenptr->time));
			}
			break;
		case 2:
			if (ischannel(to)) {
				sendprivmsg(chan,"\002%s\002 signed off on \002%s\002 with message [\037%s\037]", seenptr->userhost, time2away(seenptr->time), seenptr->signoff);
			}
			else {
				send_to_user(from,"\002%s\002 signed off on \002%s\002 with message [\037%s\037]", seenptr->userhost, time2away(seenptr->time), seenptr->signoff);
			}
			break;
		case 3:
			if (ischannel(to)) {
				sendprivmsg(chan,"\002%s\002 changed nicks to \037%s\037 on \002%s\002", seenptr->userhost, seenptr->signoff, time2away(seenptr->time));
			}
			else {
				send_to_user(from,"\002%s\002 changed nicks to \037%s\037 on \002%s\002", seenptr->userhost, seenptr->signoff, time2away(seenptr->time));
			}
			break;
		case 4:
			if (ischannel(to)) {
				sendprivmsg(chan,"\002%s\002 was kicked on \002%s\002 by \037%s\037 with message [\037%s\037]", seenptr->userhost, time2away(seenptr->time), seenptr->kicker,seenptr->signoff);
			}
			else {
				send_to_user(from,"\002%s\002 was kicked on \002%s\002 by \037%s\037 with message [\037%s\037]", seenptr->userhost, time2away(seenptr->time), seenptr->kicker,seenptr->signoff);
			}
			break;
		default: break;
	}
}

/* END SEEN */

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

/*      Command     function     userlvl   forcedcc    needcmdchar */
{
/* Some more ftp-dcc functions				*/
#ifdef CCFTP
	{ "CD",			do_cd,			  1,	 TRUE,     TRUE,  TRUE },
	{ "FILES",		do_flist,		  1,	 TRUE,     TRUE,  TRUE },
	{ "FLIST",		do_flist,		  1,	 TRUE,     TRUE,  TRUE },
	{ "GET",		do_send,		  1,	 TRUE,     TRUE,  TRUE },
	{ "LS",			show_dir,		  1,	 TRUE,     TRUE,  TRUE },
	{ "MGET",		do_send,		  1,	 TRUE,     TRUE,  TRUE },
	{ "PWD",		show_cwd,		  1,	 TRUE,     TRUE,  TRUE },
	{ "QUEUE",		show_queue,		  1,	 TRUE,     TRUE,  TRUE },
	{ "SEND",		do_send,		  1,	 TRUE,     TRUE,  TRUE },
	{ "CQUEUE",		do_clearqueue,	  1,	 TRUE,     TRUE,  TRUE },
#else
	{ "CD",			do_cd,			  1,	 TRUE,    FALSE,  TRUE },
	{ "FILES",		do_flist,		  1,	 TRUE,    FALSE,  TRUE },
	{ "FLIST",		do_flist,		  1,	 TRUE,    FALSE,  TRUE },
	{ "GET",		do_send,		  1,	 TRUE,    FALSE,  TRUE },
	{ "LS",			show_dir,		  1,	 TRUE,    FALSE,  TRUE },
	{ "MGET",		do_send,		  1,	 TRUE,    FALSE,  TRUE },
	{ "PWD",		show_cwd,		  1,	 TRUE,    FALSE,  TRUE },
	{ "QUEUE",		show_queue,		  1,	 TRUE,    FALSE,  TRUE },
	{ "SEND",		do_send,		  1,	 TRUE,    FALSE,  TRUE },
	{ "CQUEUE",		do_clearqueue,	  1,	 TRUE,    FALSE,  TRUE },
#endif

#ifdef SERVER_MONITOR
	{ "MONITOR", 	do_monitor,     100,     TRUE,     TRUE,  TRUE },
#endif
	{ "CUMONITOR", 	do_cumonitor,   100,     TRUE,     TRUE,  TRUE },
	{ "RCUMONITOR", do_rcumonitor,  100,     TRUE,     TRUE,  TRUE },

	{ "SPANK", 		do_spank, 	  	 50,    FALSE,    FALSE,  TRUE },
	{ "WHOM", 		do_whom, 	  	  1,     TRUE,     TRUE,  TRUE },
	{ "LOOKUP", 	do_dictionary, 	  1,    FALSE,     TRUE,  TRUE },
	{ "DELWORD", 	do_d_dictionary, 90,    FALSE,     TRUE,  TRUE },
	{ "WSTATS", 	do_dictionary_s, 90,    FALSE,     TRUE,  TRUE },
	{ "ADDWORD", 	do_a_dictionary,  1,    FALSE,     TRUE,  TRUE },
	{ "ADDRQUOTE", 	do_addrandquote, 90,    FALSE,     TRUE,  TRUE },
	{ "RQ", 		do_randquote,    50,    FALSE,     TRUE,  TRUE },
	{ "RANDQUOTE", 	do_randquote,    50,    FALSE,     TRUE,  TRUE },
	{ "PINSULT", 	do_pinsult,    	 50,    FALSE,     TRUE,  TRUE },
	{ "PYOMAMA", 	do_pyomama,    	 50,    FALSE,     TRUE,  TRUE },
	{ "STATS", 		do_servstats,    90,     TRUE,     TRUE,  TRUE },
	{ "DEOPER", 	do_deoper,    	 99,    FALSE,     TRUE,  TRUE },
	{ "TRACE", 		do_trace,    	 99,     TRUE,     TRUE,  TRUE },
	{ "WHOIS", 		do_whois_irc,    99,     TRUE,     TRUE,  TRUE },
	{ "LUSERS", 	do_lusers,   	 99,     TRUE,     TRUE,  TRUE },
	{ "REPEAT", 	do_repeat,   	 99,    FALSE,     TRUE,  TRUE },
	{ "RCTCP", 		do_ctcp,   	 	 99,    FALSE,     TRUE,  TRUE },
	{ "CTCP", 		do_send_ctcp,    99,    FALSE,     TRUE,  TRUE },
	{ "BACKWARDS", 	do_backwards,    50,    FALSE,     TRUE,  TRUE },
	{ "DYSLEXIA", 	do_dyslexia,   	 50,    FALSE,     TRUE,  TRUE },
	
	{ "SEEN", 		do_seen,    	  1,    FALSE,     TRUE,  TRUE },
	{ "SHOWSEEN", 	do_showseen,     99,    FALSE,     TRUE,  TRUE },
	{ "ADDPING", 	do_addping,    	 99,    FALSE,     TRUE,  TRUE },
	{ "ADDTOPIC", 	do_addtopic,     99,    FALSE,     TRUE,  TRUE },
	{ "ADDVERSION", do_addversion,   99,    FALSE,     TRUE,  TRUE },
	{ "ADDAWAY", 	do_addaway,    	 99,    FALSE,     TRUE,  TRUE },
	{ "MODE", 		do_mode,    	 99,    FALSE,     TRUE,  TRUE },
	{ "CLEARSHIT", 	do_clearshit,    99,    FALSE,     TRUE,  TRUE },
	{ "LMATCH", 	do_match,    	 99,     TRUE,     TRUE,  TRUE },
	{ "CMATCH", 	do_cmatch,    	 99,     TRUE,     TRUE,  TRUE },
	{ "CDEL", 		do_cdelmatch,   100,    FALSE,     TRUE,  TRUE },
	{ "UPDATE", 	do_update,   	 50,    FALSE,     TRUE,  TRUE },
	
	{ "FLASH5", 	do_flash5,    	100,    FALSE,     TRUE,  TRUE },
	{ "TSPOOF", 	do_tspoof,    	100,    FALSE,     TRUE,  TRUE },
	
	{ "NSL",		do_nsl,	 		 50,	FALSE,	   TRUE,  TRUE },
	{ "FINGER",		do_finger,	 	 50,	FALSE,	   TRUE,  TRUE },
	{ "INWHOIS",	do_whois,	 	 50,	 TRUE,	   TRUE,  TRUE },
	
	{ "ADDNEWS",	do_addnews,	 	 50,	 TRUE,	   TRUE,  TRUE },
	{ "ERASENEWS",	do_erasenews,	 99,	 TRUE,	   TRUE,  TRUE },
	{ "ADDSITE",	do_addsite,	 	 50,	 TRUE,	   TRUE,  TRUE },
	{ "SITES",		do_sitez,	 	 50,	 TRUE,	   TRUE,  TRUE },
	{ "ERASESITES",	do_erasesitez,	 99,	 TRUE,	   TRUE,  TRUE },
	{ "NEWS",       do_news,         50,     TRUE,     TRUE,  TRUE },
		
	{ "RANDAWAY",  	do_randaway,     50,    FALSE,     TRUE,  TRUE },
	{ "RA",  		do_randaway,     50,    FALSE,     TRUE,  TRUE },
	{ "RANDSAY",  	do_randsay,   	 50,    FALSE,     TRUE,  TRUE },
	{ "BEAVIS",  	do_beavis,    	 50,    FALSE,     TRUE,  TRUE },
	{ "YOMAMA",  	do_yomama,    	 50,    FALSE,     TRUE,  TRUE },
	{ "INSULT",  	do_insult,    	 50,    FALSE,     TRUE,  TRUE },
	{ "PCJOKE", 	do_pcjoke,		 50,	FALSE,     TRUE,  TRUE },
	{ "PICKUP",  	do_pickup,    	 50,    FALSE,     TRUE,  TRUE },
	{ "BONG",  		do_bong,    	 50,    FALSE,    FALSE,  TRUE },
	{ "LASERKICK",  do_laserkick,    50,    FALSE,     TRUE,  TRUE },
	{ "LK",  		do_laserkick,    50,    FALSE,     TRUE,  TRUE },
	{ "FLOOD",  	do_flood,    	 99,    FALSE,     TRUE,  TRUE },
	{ "F",  		do_flood,   	 99,    FALSE,     TRUE,  TRUE },
	{ "RANDFLOOD",  do_randflood,    99,    FALSE,     TRUE,  TRUE },
	{ "RF",  		do_randflood,    99,    FALSE,     TRUE,  TRUE },
	{ "TSUNAMI",  	do_tsunami,      99,    FALSE,     TRUE,  TRUE },
	{ "TSU",  		do_tsunami,      99,    FALSE,     TRUE,  TRUE },
	{ "MEGAFLOOD",  do_megaflood,   100,    FALSE,     TRUE,  TRUE },
	{ "MF",  		do_megaflood,   100,    FALSE,     TRUE,  TRUE },
	{ "SKITZO",  	do_schitzo,      99,    FALSE,     TRUE,  TRUE },
	{ "FLASHMSG",  	do_flashmsg,     99,    FALSE,     TRUE,  TRUE },
	{ "NRF",  		do_nrandflood,  100,    FALSE,     TRUE,  TRUE },
	{ "NRANDFLOOD", do_nrandflood,  100,    FALSE,     TRUE,  TRUE },
	{ "ASSAULT", 	do_assault,   	100,    FALSE,     TRUE,  TRUE },
	{ "ADDUSER", 	do_adduser2,   	 99,    FALSE,     TRUE,  TRUE },
	{ "DELUSER", 	do_minususer,    99,    FALSE,     TRUE,  TRUE },
	{ "FLASH3", 	do_flash3,    	100,    FALSE,     TRUE,  TRUE },
	{ "MFLASH", 	do_mflash,      100,    FALSE,     TRUE,  TRUE },
	{ "APHEX", 		do_aphex,       100,    FALSE,     TRUE,  TRUE },
	{ "CSERV", 		do_cserv,       100,    FALSE,     TRUE,  TRUE },
	{ "CORE", 		do_core,        100,     TRUE,     TRUE,  TRUE },
	{ "FAKELOG", 	do_fakelog,      80,    FALSE,     TRUE,  TRUE },
	{ "RANDLOG", 	do_randlog,      80,    FALSE,     TRUE,  TRUE },
	{ "CSTATS", 	do_cstats,       80,     TRUE,     TRUE,  TRUE },
	{ "QSHIT",		do_qshit2,		 50,	FALSE,	   TRUE,  TRUE },
	{ "PINGFLOOD",	do_pingflood,	 99,	FALSE,	   TRUE,  TRUE },
	{ "PF",			do_pingflood,	 99,	FALSE,	   TRUE,  TRUE },
	{ "ELEET",		do_eleet,		 50,	FALSE,	   TRUE,  TRUE },
	{ "COOL",		do_cool,		 50,	FALSE,	   TRUE,  TRUE },
	{ "ANNOY",		do_annoy,		 50,	FALSE,	   TRUE,  TRUE },
	{ "EPING",		do_eping,		 50,	FALSE,	   TRUE,  TRUE },
	{ "CPING",		do_cping,		 50,	FALSE,	   TRUE,  TRUE },
	{ "APING",		do_aping,		 50,	FALSE,	   TRUE,  TRUE },
	{ "UPTIME",		do_uptime,		 50,	FALSE,	   TRUE,  TRUE },
	{ "WALL",		do_wall,		 50,	FALSE,	   TRUE,  TRUE },
	{ "4OP",		do_4op,		 	 50,	FALSE,	   TRUE,  TRUE },
	{ "8BALL",		do_8ball,		 50,	FALSE,	  FALSE,  TRUE },
	{ "VERSIONFLOOD",	do_versionflood,		 99,	FALSE,	   TRUE,  TRUE },
	{ "VERS",		do_vers,		  1,	FALSE,	   TRUE,  TRUE },
	{ "REHASH",		do_rehash2,		 99,	FALSE,	   TRUE,  TRUE },
	{ "RESET",		do_rehash,		 99,	FALSE,	   TRUE,  TRUE },


	{ "ADDSERVER",  do_addserver,    99,    FALSE,     TRUE,  TRUE },
	{ "AWAY",       do_away,         90,    FALSE,     TRUE,  TRUE },
	{ "DISPLAY",	do_display,	 	 99,	 TRUE,	   TRUE,  TRUE },
	{ "GETBOMB",    do_getbomb,      99,    FALSE,     TRUE,  TRUE },
	{ "HEART",      do_heart,        50,    FALSE,     TRUE,  TRUE },
	{ "KILL",       do_kill,        100,    FALSE,     TRUE,  TRUE },
	{ "KLINE",      do_kline,       100,    FALSE,     TRUE,  TRUE },
	{ "LAST",       do_last,        100,     TRUE,     TRUE,  TRUE },
	{ "OPER",       do_oper,         50,    FALSE,     TRUE,  TRUE },
	{ "RANDKICK",   do_randomkick,   50,    FALSE,     TRUE,  TRUE },
	{ "RANDTOPIC",  do_randomtopic,  50,    FALSE,     TRUE,  TRUE },
	{ "RK",         do_randomkick,   50,    FALSE,     TRUE,  TRUE },
	{ "RMK",        do_randommasskick,50,   FALSE,     TRUE,  TRUE },
	{ "RANDMASSKICK",  do_randommasskick,50,   FALSE,     TRUE,  TRUE },
	{ "RANDNICK",   do_swapnick,     50,    FALSE,     TRUE,  TRUE },
	{ "RT",         do_randomtopic,  50,    FALSE,     TRUE,  TRUE },
	{ "SC",         do_screwban,     50,    FALSE,     TRUE,  TRUE },
	{ "SCREWBAN",   do_screwban,     50,    FALSE,     TRUE,  TRUE },
	{ "UMATCH",     do_search,       99,     TRUE,     TRUE,  TRUE },
	{ "SERVERLIST", do_serverlist,   50,     TRUE,     TRUE,  TRUE },

	{ "ACCESS",     do_access,        1,    FALSE,     TRUE, FALSE },
	{ "ADD",        do_add,          99,    FALSE,     TRUE,  TRUE },
	{ "AOP",        do_aop,          99,    FALSE,     TRUE,  TRUE },
	{ "VERIFY",     do_auth,          1,    FALSE,    FALSE, FALSE },
	{ "B",          do_ban,          50,    FALSE,     TRUE,  TRUE },
	{ "BAN",        do_ban,          50,    FALSE,     TRUE,  TRUE },
	{ "BANLIST",    do_banlist,      50,     TRUE,     TRUE,  TRUE },
	{ "BANS",       do_banlist,      50,     TRUE,     TRUE,  TRUE },
	{ "CHACCESS",   do_chaccess,    100,    FALSE,     TRUE,  TRUE },
	{ "CHANNELS",   do_channels,     50,     TRUE,     TRUE,  TRUE },
	{ "CHANSTATS",  do_chanstats,    50,    FALSE,     TRUE,  TRUE },
	{ "CHAT",       do_chat,          1,    FALSE,     TRUE,  TRUE },
	{ "CLVL",       do_clvl,        100,    FALSE,     TRUE,  TRUE },
	{ "CMDCHAR",    do_cmdchar,     100,    FALSE,     TRUE,  TRUE },
	{ "COLLECT",    do_collect,      50,    FALSE,     TRUE,  TRUE },
	{ "CSLVL",      do_cslvl,        50,    FALSE,     TRUE,  TRUE },
	{ "CYCLE",      do_cycle,        99,    FALSE,     TRUE,  TRUE },
	{ "DCCLIST",    do_listdcc,      50,     TRUE,     TRUE,  TRUE },
	{ "DEL",        do_del,          99,    FALSE,     TRUE,  TRUE },
	{ "DEOP",       do_deop,         50,    FALSE,     TRUE,  TRUE },
	{ "DIE",        do_die,         100,    FALSE,     TRUE,  TRUE },
	{ "DO",         do_do,           99,    FALSE,     TRUE,  TRUE },
	{ "DOWN",       do_deopme,        1,    FALSE,    FALSE, FALSE },
	{ "EXEC",       do_exec,        100,    FALSE,     TRUE,  TRUE },
	{ "FORGET",     do_forget,       90,    FALSE,     TRUE,  TRUE },
	{ "CCHAN",      do_getch,        50,    FALSE,     TRUE,  TRUE },
	{ "GLOBAL",     parse_global,   100,    FALSE,     TRUE,  TRUE },
	{ "GREET",      do_greet,        50,    FALSE,     TRUE,  TRUE },
	{ "GREETLIST",  do_greetlist,    50,     TRUE,     TRUE,  TRUE },
	{ "HELP",       do_help,          1,     TRUE,     TRUE, FALSE },
	{ "HELPFULL",   do_helpfull,      1,     TRUE,     TRUE, FALSE },
	{ "IDLE",       do_idle,          1,    FALSE,     TRUE, FALSE },
	{ "INFO",       do_info,          1,     TRUE,     TRUE, FALSE },
	{ "INVITE",     do_invite,       50,    FALSE,     TRUE,  TRUE },
	{ "JOIN",       do_join,         99,    FALSE,     TRUE,  TRUE },
	{ "K",          do_kick,         50,    FALSE,     TRUE,  TRUE },
	{ "KB",         do_kickban,      50,    FALSE,     TRUE,  TRUE },
	{ "KICK",       do_kick,         50,    FALSE,     TRUE, FALSE },
	{ "KICKS",      do_kicks,         1,    FALSE,     TRUE, FALSE },
	{ "KS",         do_kicksay,      90,    FALSE,     TRUE,  TRUE },
	{ "KSLIST",     do_kicksaylist,	 90,     TRUE,     TRUE,  TRUE },
	{ "LEAVE",      do_leave,        99,    FALSE,     TRUE,  TRUE },
	{ "LIMIT",      do_limit,        90,    FALSE,     TRUE,  TRUE },
	{ "LINKS",      do_links,        90,     TRUE,     TRUE,  TRUE },
	{ "LOADALL",    do_loadall,      90,    FALSE,     TRUE,  TRUE },
	{ "LOADKICKS",  do_loadkicks,    90,    FALSE,     TRUE,  TRUE },
	{ "LOADLEVELS", do_loadlevels,	 90,    FALSE,     TRUE,  TRUE },
	{ "LOADLISTS",  do_loadlists,    90,    FALSE,     TRUE,  TRUE },
	{ "MD",         do_massdeop,     90,    FALSE,     TRUE,  TRUE },
	{ "MDEOP",      do_massdeop,     90,    FALSE,     TRUE,  TRUE },
	{ "MDEOPNU",    do_massdeopnu,   90,    FALSE,     TRUE,  TRUE },
	{ "ME",         do_me,           90,    FALSE,     TRUE,  TRUE },
	{ "MK",         do_masskick,     90,    FALSE,     TRUE,  TRUE },
	{ "MKB",        do_masskickban,  90,    FALSE,     TRUE,  TRUE },
	{ "MKNU",       do_masskicknu,   90,    FALSE,     TRUE,  TRUE },
	{ "MOP",        do_massop,       90,    FALSE,     TRUE,  TRUE },
	{ "MOPU",       do_massopu,      50,    FALSE,     TRUE,  TRUE },
	{ "MSG",        do_msg,          90,    FALSE,     TRUE,  TRUE },
	{ "MUB",        do_massunban,    50,    FALSE,     TRUE,  TRUE },
	{ "MUBF",       do_massunbanfrom,50,    FALSE,     TRUE,  TRUE },
	{ "NAMES",      do_names,        90,    FALSE,     TRUE,  TRUE },
	{ "NEXTSERVER", do_nextserver,   99,    FALSE,     TRUE,  TRUE },
	{ "NICK",       do_nick,         99,    FALSE,     TRUE,  TRUE },
	{ "NICKLIST",   do_nicklist,     90,     TRUE,     TRUE,  TRUE },
	{ "OP",         do_op,           90,    FALSE,     TRUE,  TRUE },
	{ "PART",       do_leave,        99,    FALSE,     TRUE,  TRUE },
	{ "PASSWD",     do_passwd,        1,    FALSE,     TRUE, FALSE },
	{ "PING",       do_ping,          1,    FALSE,     TRUE, FALSE },
	{ "PROT",       do_prot,        100,    FALSE,     TRUE,  TRUE },
	{ "PROTNICK",   do_protnick,     99,    FALSE,     TRUE,  TRUE },
	{ "RAOP",       do_raop,        100,    FALSE,     TRUE,  TRUE },
	{ "RELOGIN",    do_relogin,      99,    FALSE,     TRUE,  TRUE },
	{ "REPORT",     do_report,       90,     TRUE,     TRUE,  TRUE },
	{ "RESTRICT",   do_restrict,     90,    FALSE,     TRUE,  TRUE },
	{ "RGREET",     do_rgreet,       90,    FALSE,     TRUE,  TRUE },
	{ "RKS",        do_rkicksay,     90,    FALSE,     TRUE,  TRUE },
	{ "ROLLCALL",   do_rollcall,      1,    FALSE,    FALSE, FALSE },
	{ "RPROT",      do_rprot,        90,    FALSE,     TRUE,  TRUE },
	{ "RPROTNICK",  do_rprotnick,    90,    FALSE,     TRUE,  TRUE },
	{ "RSHIT",      do_rshit,        50,    FALSE,     TRUE,  TRUE },
	{ "RSPY",       do_rspy,         99,    FALSE,     TRUE,  TRUE },
	{ "RSPYMSG",    do_rspymsg,      90,    FALSE,     TRUE,  TRUE },
	{ "RCUSTAT",    do_rstatmsg,     90,    FALSE,     TRUE,  TRUE },
	{ "RSS",        do_rsaysay,      90,    FALSE,     TRUE,  TRUE },
	{ "RSTEAL",     do_rsteal,       90,    FALSE,     TRUE,  TRUE },
	{ "SAVE",       do_savelists,    90,    FALSE,     TRUE,  TRUE },
	{ "SAVEALL",    do_saveall,      90,    FALSE,     TRUE,  TRUE },
	{ "SAVEKICKS",  do_savekicks,    90,    FALSE,     TRUE,  TRUE },
	{ "SAVELEVELS", do_savelevels,   90,    FALSE,     TRUE,  TRUE },
	{ "SAVELISTS",  do_savelists,    90,    FALSE,     TRUE,  TRUE },
	{ "SAY",        do_say,          99,    FALSE,     TRUE,  TRUE },
	{ "SB",         do_siteban,      50,    FALSE,     TRUE,  TRUE },
	{ "SERVER",     do_server,       99,    FALSE,     TRUE,  TRUE },
	{ "SET",        do_set,          99,    FALSE,     TRUE,  TRUE },
	{ "SETPASS",    do_setpass,      99,    FALSE,     TRUE,  TRUE },
	{ "SETSHITREASON", do_setshitreason, 10,FALSE,     TRUE,  TRUE },
	{ "SHIT",       do_shit,         50,    FALSE,     TRUE,  TRUE },
	{ "SHITLIST",   do_shitlist,     90,     TRUE,     TRUE,  TRUE },
	{ "SHOWIDLE",   do_showidle,     90,     TRUE,     TRUE,  TRUE },
	{ "SHOWUSERS",  do_showusers,    90,     TRUE,     TRUE,  TRUE },
	{ "SKB",        do_sitekickban,  90,    FALSE,     TRUE,  TRUE },
	{ "SPAWN",      do_spawn,        99,    FALSE,     TRUE,  TRUE },
	{ "SPLITS",     do_splits,       90,     TRUE,     TRUE,  TRUE },
	{ "SPY",        do_spy,          99,     TRUE,     TRUE,  TRUE },
	{ "SPYLIST",    do_spylist,      99,    FALSE,     TRUE,  TRUE },
	{ "SPYMSG",     do_spymsg,       99,    FALSE,     TRUE,  TRUE },
	{ "CUSTAT",     do_statmsg,      99,    FALSE,     TRUE,  TRUE },
	{ "SS",         do_saysay,       90,    FALSE,     TRUE,  TRUE },
	{ "SSLIST",     do_saysaylist,   90,     TRUE,     TRUE,  TRUE },
	{ "USTATS",     do_stats,         1,     TRUE,     TRUE,  TRUE },
	{ "STEAL",      do_steal2,       99,    FALSE,     TRUE,  TRUE },
	{ "T",          do_topic,        50,    FALSE,    FALSE,  TRUE },
	{ "TIME",       do_time ,        50,    FALSE,     TRUE,  TRUE },
	{ "TOG",        do_toggle,       99,    FALSE,     TRUE,  TRUE },
	{ "TOPIC",      do_topic,        50,    FALSE,     TRUE,  TRUE },
	{ "TOPKICKS",   do_topkicks,      1,     TRUE,     TRUE, FALSE },
	{ "TOPSPLITS",  do_topsplits,    90,     TRUE,     TRUE,  TRUE },
	{ "UB",         do_unban,        50,    FALSE,     TRUE,  TRUE },
	{ "UP",         do_opme,         50,    FALSE,    FALSE,  TRUE },
	{ "USERHOST",   do_userhost,     90,    FALSE,     TRUE,  TRUE },
	{ "USERLIST",   do_userlist,     90,     TRUE,     TRUE,  TRUE },
	{ "VERSION",    do_version,       1,    FALSE,     TRUE, FALSE },
	{ "WHO",        do_showusers,    90,     TRUE,     TRUE,  TRUE },
	/*
	 :
	 :
	 */
	{ NULL,         (void(*)(char *, char *, char *, int))(NULL), 0, FALSE, FALSE }
};

void do_helpfull(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
   char line[MAXLEN], token[32];
   int i, j, count, access;
   
   j = count = 0;
   access = max_userlevel(from);
   strcpy(line, "");
   send_to_user(from, "\002Available commands for access level %i\002",
access);
   for ( i = 0; on_msg_commands[i].name; i++)
     {
	if (on_msg_commands[i].userlevel <= access)
	  {
	     sprintf(token, "%-15s", on_msg_commands[i].name);
	     strcat(line,token);
	     token[0]='\0';
	     j++;
	     count++;
	  }
	if(!on_msg_commands[i+1].name || (j > 3))
	  {
	     send_to_user(from, "\002%s\002",line);
             line[0]='\0';
	     j=0;
	  }
     }
   send_to_user(from,"\002Total of %i commands are available to you\002", count);
   return;
}

void do_help(from, to, rest, cmdlevel)
char *from, *to, *rest;
int cmdlevel;
{
   char lin[MAXLEN], line[MAXLEN], token[45], tt2[16];
   FILE *in;
   int i, j, level, access;
   
   j=0;
   access = max_userlevel(from);
   if (!rest || !*rest)
     {
	send_to_user(from, "\002Usage: HELP <topics | command | level | *pattern*>
[options]\002");
	return;
     }
   strcpy(token, "");
   strcpy(line, "");
   if(isdigit(*rest))
     {
	level = atoi(rest);
	if ( level > access)
	  level = access;
	if ( level <= 0 )
	  level = access;
	send_to_user(from, "\002Commands available to access level %i\002",level);
	for ( i = 0; on_msg_commands[i].name; i++)
	  {
	     if (on_msg_commands[i].userlevel == level)
	       {
		  sprintf(token, "%-15s", on_msg_commands[i].name);
		  strcat(line, token);
		  token[0]='\0';
		  j++;
	       }
	     if(!on_msg_commands[i+1].name || (j > 3))
	       {
		  send_to_user(from, "\002%s\002",line);
		  line[0]='\0';
		  j=0;
	       }
	  }
	return;
     }

   if(strchr(rest,'*'))
     {
	strncpy(tt2,rest,16);
	send_to_user(from, "\002Commands that match query %s\002", rest); 
	for ( i = 0; on_msg_commands[i].name; i++)
	  {
	     if (!matches(tt2, on_msg_commands[i].name))
	       if(on_msg_commands[i].userlevel <= access)
	       {
		  sprintf(token,"%-15s",on_msg_commands[i].name);
		  strcat(line,token);
		  token[0]='\0';
		  j++;
	       }
	     if(!on_msg_commands[i+1].name || (j > 3))
	       {
		  send_to_user(from, "\002%s\002",line);
		  line[0]='\0';
		  j=0;
	       }
	  }
	return;
     }
   
   
   if (!(in = fopen(HELPFILE, "rt")))
     {
	send_to_user(from, "\002Couldn't open the helpfile (%s)\002",
		     HELPFILE);
	return;
     }

   while (find_topic(in, lin) && !feof(in))
     if (!my_stricmp((char *) lin+1, rest))
     {
	int level;
	if (strchr(rest, ' '))
	  {
	     int num;
	     char buffer[MAXLEN];
	     strcpy(buffer, "");
	     num = 0;
	     while (freadln(in, lin) && !feof(in) && my_stricmp(lin, ":endtopic"))
	       if (lin && (*lin == ':'))
	       {
		  num++;
		  /*
		  if (num != 1)
		    strcat(buffer, " "); */
		  sprintf(token,"%-15s",(char *) lin+1);
		  strcat(buffer,token);
		  strcpy(token, "");
/*		  strcat(buffer, (char *) lin+1); */
		  if (num == 4)
		    {
		       send_to_user(from, "%s", buffer);
		       num = 0;
		       strcpy(buffer, "");
		    }
	       }
	     if (num)
	       send_to_user(from, "%s", buffer);
	     return;
	  }
	send_to_user(from, "HELP on %s", rest);
	level = level_needed(rest);
	if (level >= 0)
	  send_to_user(from, "Level needed: %i", level);
	while (freadln(in, lin) && lin && (*lin != ':') && !feof(in))
	  send_to_user(from, "%s", lin);
	return;
     }
}

void on_kick(from, channel, nick, reason)
char *from;
char *channel;
char *nick;
char *reason;
{
	char	*temp;
	aTime	*Kicks;
	aChannel *Dummy;

	send_spy(channel, "\002%s\002 has been kicked off channel \002%s\002 by \002%s\002 (\037%s\037)",
		nick, channel, getnick(from), reason);
	if (my_stricmp(currentnick, current->nick))
		if (check_masskick(from, channel))
			if (!CurrentUser || CurrentUser->access < ASSTLEVEL)
				mass_action(from, channel);
	if (!my_stricmp(current->nick, nick))
	{
		set_kickedby(channel, from);
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

	send_spy(channel, "\002%s\002 has joined channel \002%s\002", who, channel);

	if (get_int_var(channel, TOGABK_VAR) && is_banned(who, channel))
	{
		sendmode(channel, "-o %s", currentnick);
		sendkick(channel, currentnick, "\002%s\002",
			"You are banned on this server");
	} 
	if(!CurrentUser && get_int_var(channel, TOGCTL_VAR))
	{
		if(numchar(who,'\031') || numchar(who,'\002') ||
			numchar(who,'\022') || numchar(who, '\026'))
			if(is_user(format_uh(who,2),channel))
			{
				sendkick(channel, getnick(who),
					"\002%s\002","Lame Control-Chars in UserId");
				return;
			}
			else
			{
				deop_siteban(channel, getnick(who), who);
				sendkick(channel, getnick(who),
					"\002%s\002","Lame Control-Chars in UserId");
				return;
			}
	}

	if (!is_a_bot(who) && (!CurrentUser || CurrentUser->access < 50))
		check_clonebots(who, channel);

	if (CurrentShit && CurrentShit->access && !CurrentUser &&
		!is_a_bot(who))
	{
		shit_action(who, channel);
		return;
	}
	Lame = find_protnick(&(current->Userlist), who);
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
		if ((Lame=find_greet(&(current->Userlist), who)) != NULL)
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
	send_spy(channel, "\002%s\002 has left channel \002%s\002",
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
	send_common_spy(who, "\002%s\002 is now known as \002%s\002", who, newnick);
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
        Lame = find_protnick(&(current->Userlist), newnuh);
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
	send_common_spy(who, "Signoff \002%s\002 (\037%s\037)", who, reason);
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
	int needcc, pubcommand;
	int i, ii;
	aDCC *userclient, *client;
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
	
/* SuperUser (BOT) Stuff */

	if (maxul > OWNERLEVEL) {
		if (msg_copy[0] == current->cmdchar) {
			send_to_user(from, "\002You are not allowed to do commands\002");
			if (get_int_varc(NULL, TOGSPY_VAR)) {
				send_statmsg(":\002%s\002[%i]: -> SuperUser <- Attempted to use a command", currentnick, maxul);
			}	
			log_it(FAILED_LOG_VAR, 0, "%s[%i] tried: %s", from, maxul, msg_copy);
			return;
		}
		else {
			return;
		}
	}
		
/* End SuperUser (BOT) Stuff */
		
	if ((tmp = find_time(&Auths, from)) != NULL)
		tmp->time = getthetime();
	if (ulevel < 99)
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
			if (!strcmp(on_msg_commands[i].name, "VERIFY") &&
				pubcommand)
			{
				send_to_user(from,
				"\002Are you stupid or something? Don't give out your password :P\002");
				return;
			}

/* Check for FTP PUB commands */
			
			if (!strcmp(on_msg_commands[i].name, "GET") && pubcommand) {
				log_it(FAILED_LOG_VAR, 0, "%s tried in public: %s", from, msg_copy);
				return;
			}
			else if (!strcmp(on_msg_commands[i].name, "MGET") && pubcommand) {
				log_it(FAILED_LOG_VAR, 0, "%s tried in public: %s", from, msg_copy);
				return;
			}
			else if (!strcmp(on_msg_commands[i].name, "LS") && pubcommand) {
				log_it(FAILED_LOG_VAR, 0, "%s tried in public: %s", from, msg_copy);
				return;
			}
			else if (!strcmp(on_msg_commands[i].name, "PWD") && pubcommand) {
				log_it(FAILED_LOG_VAR, 0, "%s tried in public: %s", from, msg_copy);
				return;
			}
			else if (!strcmp(on_msg_commands[i].name, "SEND") && pubcommand) {
				log_it(FAILED_LOG_VAR, 0, "%s tried in public: %s", from, msg_copy);
				return;
			}
			else if (!strcmp(on_msg_commands[i].name, "CD") && pubcommand) {
				log_it(FAILED_LOG_VAR, 0, "%s tried in public: %s", from, msg_copy);
				return;
			}
			
			if (maxul < on_msg_commands[i].userlevel)
			{
				if (pubcommand && !on_msg_commands[i].needcc)				
					on_public(from, to, msg_copy);
				else {
				log_it(FAILED_LOG_VAR, 0, "%s tried: %s",
					from, msg_copy);
				}
				return;
			}
			if ((maxul < current->restrict) &&
				 (on_msg_commands[i].userlevel != -1))
			{
				log_it(FAILED_LOG_VAR, 0, "%s tried: %s",
                                        from, msg_copy);
				send_to_user(from,
				"\002Achtung! Restricted to level %i or higher\002",
					current->restrict);
				return;
			}
			if (!from_shell && password_needed(from) &&
				 on_msg_commands[i].needpass && !verified(from))
			  {
			    if (on_msg_commands[i].needcc || !pubcommand)
			      send_to_user(from, "\002Use \"verify\" to get verified first\002");
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
/* Marker */	if (get_int_varc(NULL, TOGSPY_VAR)) {
/*				if (CurrentUser->access > 99) {
*/				/*	send_to_user(from, "Executing command \002%s\002 (%i/%i) at %s", on_msg_commands[i].name, maxul, on_msg_commands[i].userlevel, time2medium(getthetime())); */
					send_statmsg(":\002%s\002[%i]: Executing \002%s\002[%i]", currentnick, maxul, on_msg_commands[i].name, on_msg_commands[i].userlevel);
/*				}
*/			}			
			on_msg_commands[i].function(from, to, (msg && !*msg) ? NULL : msg,
				on_msg_commands[i].userlevel);
                        for(ii = 30; ii >= 0; ii--)
				strcpy(lastcmds[ii+1], lastcmds[ii]);
				strcpy(lastcmds[0], on_msg_commands[i].name);
				strcat(lastcmds[0], " by ");
				strcat(lastcmds[0], left(from, 35));
				strcat(lastcmds[0], " at [\002");
				strcat(lastcmds[0], time2medium(getthetime()));
				strcat(lastcmds[0], "\002]");
			return;
		}
			}
	if (*command == current->cmdchar)
		log_it(FAILED_LOG_VAR, 0, "%s tried: %s", from, msg_copy);
	if (!pubcommand)
	  {
		if (*command == current->cmdchar) {
			send_to_user(from, "\002%s\002", "Squeeze me?");
		}
		else if (userclient && (userclient->flags & DCC_CHAT)) {
			for (client = current->Client_list; client; client = client->next) {
				if (client->flags & DCC_CHAT) {
					if (!my_stricmp(getnick(client->user), currentnick)) {
						send_to_user(client->user, "\002<\002%s\002>\002 %s", currentnick, msg_copy);
					}
					else {
						send_to_user(client->user, "<%s> %s", currentnick, msg_copy);
					}
				}
			}
			return;
		}
		else {
			log_it(MSG_LOG_VAR, 0, "%s said: %s", from, msg_copy);

				if (('[' == msg_copy[0]) && (get_int_varc(NULL, TOGIWM_VAR))) {
					return;
				}
				else if (('(' == msg_copy[0]) && (get_int_varc(NULL, TOGIWM_VAR))) {
					return;
				}
				else if (('\002' == msg_copy[0]) && (get_int_varc(NULL, TOGIWM_VAR))) {
					return;
				}
				else if (('\037' == msg_copy[0]) && (get_int_varc(NULL, TOGIWM_VAR))) {
					return;
				}
				else if (('\026' == msg_copy[0]) && (get_int_varc(NULL, TOGIWM_VAR))) {
					return;
				}
				else {
					send_spymsg("*\002%s\002* %s", currentnick, msg_copy);
				}
	  }
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
	send_spy(channel, "Mode change \002\"%s\"\002 on channel \002%s\002 by \002%s\002",
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
/* 1 */		del_channelmode( channel, CHFL_VOICE, nick );
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
			if (check_massdeop(from, channel) && (!CurrentUser || CurrentUser->access < ASSTLEVEL))
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
			dummy = find_shit(&(current->Userlist),
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
			add_channelmode(from, channel, MODE_KEY, key ? key : "???",
getthetime());
		else
			del_channelmode(channel, MODE_KEY, "");
		break;
		case 'l':
		if (sign == '+')
		{
			limit = get_token(&params, " ");
			add_channelmode(from, channel, MODE_LIMIT, limit ? limit : "0",
getthetime());
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
	send_spy(to, "\002<%s:\037%s\037>\002 %s", currentnick, to, rest);

	if ((level=check_pubflooding(from, to)) && (!CurrentUser || !CurrentUser->prot))
	{
		if (Temp)
		{
			Temp->num++;
			if (Temp->num >= 4)
			{
                           if ((!CurrentUser || !CurrentUser->access) && get_int_var(to, TOGAS_VAR))
				add_to_shitlist(&(current->Userlist), format_uh(from,1), 2,
					to, "Auto-Shit", 
					 "Bah...flooding and or beeping/CAPSing",
					getthetime(), getthetime() + (long) 86400);
				/* auto shitlist */
			}
		}
		if (level >= 2)
		  deop_ban(to, currentnick, from);
		if (level >= 1)
		  sendkick(to, currentnick, "\002Get out you flooder!\002");
		send_statmsg("\002%s\002 kicked from \002%s\002 for flooding", from, to);
		log_it(DANGER_LOG_VAR, 0, "%s kicked from %s for flooding", from, to);
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
				sendprivmsg(to, "%s", temp);
			}
	if (get_int_var(to, TOGKS_VAR))	
		if ((triggers = check_kicksay(rest, to)) != NULL)
			if (!CurrentUser || !CurrentUser->access)
			{
				char *temp;
				temp = formatgreet(triggers->reason,
					currentnick);
				sendkick(to, currentnick, "%s", temp);
			}
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
	  send_to_user(from, "\002Immortality level must be between 0 and %i\002",
OWNERLEVEL);
	  return;
	}
	 }
  else
	 newlevel = -1;
  if (newlevel > get_userlevel(from, "*"))
	 {
		send_to_user(from, "\002%s\002", "Can't change level to one higher than
yours");
		return;
	 }
  for (i = 0; on_msg_commands[i].name; i++ )
	 {
		if (!my_stricmp(on_msg_commands[i].name, name))
	{
	  oldlevel = on_msg_commands[i].userlevel;
	  if (newlevel == -1)
		 {
			send_to_user(from, "\002The level needed for that command is %i\002",
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
		 send_to_user(from, "\002Immortality level changed from %i to %i\002",
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

        fprintf( list_file,

"############################################################################\n"
);
        fprintf( list_file, "## %s\n", filename);
        fprintf( list_file, "## Created: %s", ctime(&T) );
        fprintf( list_file,

"############################################################################\n"
);

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
