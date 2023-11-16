/*

    EnergyMech, IRC bot software
    Copyright (c) 2000-2001 proton, 2002-2003 emech-dev

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/
#define MCMD_GEN_C
#include "config.h"

/*

	These are defined in config.h

	DCC	0x0100		requires DCC
	CC	0x0200		requires commandchar
	PASS	0x0400		requires password / authentication
	ARGS	0x0800		requires args
	NOPUB	0x1000		ignore in channel (for password commands)
	NOCMD	0x4000		not allowed to be executed thru CMD

	FLAGS	0xff00
	CLEVEL	0x00ff

*/

struct
{
	int	pass;
	char	*name;
	char	*func;
	Ulong	flags;

} pre_mcmd[] =
{
	/* ExtraMech Extensions */
	{ 0, "CSTATS",		"do_cstats",		40	| CC	| PASS			},
	{ 0, "MOTD",		"do_motd",		10	| CC	| PASS	| DCC		},
	{ 0, "CREDITS",		"do_credits",		10	| CC	| PASS	| DCC		},
	{ 0, "PHRASE",		"do_phrase",		50	| CC	| PASS			},
	{ 0, "DEFINE",		"do_define",		20	| CC	| PASS			},
	{ 0, "DEFINEADD",	"do_defineadd",		20	| CC	| PASS			},
	{ 0, "DEFINEDEL",	"do_definedel",		20	| CC	| PASS			},
	/* ExtraMech Extensions */
	{ 0, "ACCESS",		"do_access",		10	| CC	| PASS			},
	{ 0, "BYE",		"do_dcc_kill",		10	| CC				},
	{ 0, "CHAT",		"do_chat",		10	| CC	| PASS	| NOCMD		},
	{ 0, "CMDCHAR",		"do_cmdchar",		10	| CC	| PASS			},
	{ 0, "DOWN",		"do_deopme",		10	| CC				},
	{ 0, "ECHO",		"do_echo",		10	| CC	| PASS			},
	{ 0, "HELP",		"do_help",		10	| CC	| PASS	| DCC		},
	{ 0, "PASSWD",		"do_passwd",		10		| PASS	| NOPUB		},
	{ 0, "USAGE",		"do_usage",		10	| CC	| PASS			},
	{ 0, "VERIFY",		"do_auth",		10			| NOPUB		},
	{ 0, "UNVERIFY",	"do_unverify",		10	| CC	| PASS			},

	{ 0, "HOST",		"do_host",		20	| CC	| PASS			},
	{ 0, "ONTIME",		"do_ontime",		20	| CC	| PASS			},
#ifdef SEEN
	{ 0, "SEEN",		"do_seen",		20	| CC	| PASS			},
#endif
	{ 0, "UPTIME",		"do_uptime",		20	| CC	| PASS			},
	{ 0, "VER",		"do_vers",		20	| CC	| PASS			},
	{ 0, "WHOM",		"do_whom",		20	| CC	| PASS	| DCC		},

#ifdef ALIAS
	{ 0, "ALIAS",		"do_alias",		100	| CC	| PASS			},
#endif
	{ 0, "BAN",		"do_ban",		40	| CC	| PASS			},
	{ 0, "BANLIST",		"do_banlist",		40	| CC	| PASS	| DCC		},
	{ 0, "CCHAN",		"do_cchan",		40	| CC	| PASS			},
	{ 0, "CSERV",		"do_cserv",		40	| CC	| PASS			},
	{ 0, "CHANNELS",	"do_channels",		40	| CC	| PASS	| DCC		},
#ifdef LINKING
	{ 0, "CMD",		"do_linkcmd",		40	| CC	| PASS	| NOCMD		},
#endif
	{ 0, "DEOP",		"do_deop",		40	| CC	| PASS			},
	{ 0, "IDLE",		"do_idle",		40	| CC	| PASS	| ARGS		},
	{ 0, "INVITE",		"do_invite",		40	| CC	| PASS			},
	{ 0, "KB",		"do_kickban",		40	| CC	| PASS			},
	{ 0, "KICK",		"do_kick",		40	| CC	| PASS			},
	{ 0, "LUSERS",		"do_lusers",		40	| CC	| PASS	| DCC		},
	{ 0, "MODE",		"do_mode",		40	| CC	| PASS			},
	{ 0, "NAMES",		"do_names",		40	| CC	| PASS			},
	{ 0, "OP",		"do_op",		40	| CC	| PASS			},
	{ 0, "RT",		"do_randomtopic",	40	| CC	| PASS			},
	{ 0, "SCREW",		"do_screwban",		40	| CC	| PASS			},
	{ 0, "SITEBAN",		"do_siteban",		40	| CC	| PASS			},
	{ 0, "SITEKB",		"do_sitekickban",	40	| CC	| PASS			},
	{ 0, "TIME",		"do_time",		40	| CC	| PASS			},
	{ 0, "TOPIC",		"do_topic",		40	| CC	| PASS			},
#ifdef TRIVIA
	{ 0, "TRIVIA",		"do_trivia",		40	| CC	| PASS			},
        { 0, "TOP10",		"do_triv_wk10",		40	| CC 	| PASS			},
#endif /* TRIVIA */
	{ 0, "UNBAN",		"do_unban",		40	| CC	| PASS			},
	{ 0, "UNVOICE",		"do_unvoice",		40	| CC	| PASS			},
	{ 0, "UP",		"do_opme",		40	| CC	| PASS			},
	{ 0, "VOICE",		"do_voice",		40	| CC	| PASS			},
	{ 0, "WALL",		"do_wall",		40	| CC	| PASS	| ARGS		},
	{ 0, "WHO",		"do_showusers",		40	| CC	| PASS	| DCC		},
	{ 0, "WHOIS",		"do_whois_irc",		40	| CC	| PASS	| DCC		},

	{ 0, "INSULT",		"do_insult",		50	| CC	| PASS			},
	{ 0, "PICKUP",		"do_pickup",		50	| CC	| PASS			},
	{ 0, "QSHIT",		"do_qshit",		50	| CC	| PASS			},
	{ 0, "RSHIT",		"do_rshit",		50	| CC	| PASS			},
	{ 0, "SHIT",		"do_shit",		50	| CC	| PASS			},
	{ 0, "SHITLIST",	"do_shitlist",		50	| CC	| PASS	| DCC		},
	{ 0, "SHITLVL",		"do_cslvl",		50	| CC	| PASS			},

	{ 0, "ADD",		"do_add",		60	| CC	| PASS	| ARGS		},
	{ 0, "CLVL",		"do_clvl",		60	| CC	| PASS			},
	{ 0, "CTCP",		"do_ctcp",		60	| CC	| PASS	| ARGS		},
	{ 0, "DEL",		"do_del",		60	| CC	| PASS			},
	{ 0, "SHOWIDLE",	"do_showidle",		60	| CC	| PASS	| DCC		},
	{ 0, "USER",		"do_user",		60	| CC	| PASS	| ARGS		},
	{ 0, "USERLIST",	"do_userlist",		60	| CC	| PASS	| DCC		},
	{ 0, "USTATS",		"do_stats",		60	| CC	| PASS	| DCC		},

	{ 0, "CYCLE",		"do_cycle",		70	| CC	| PASS			},
	{ 0, "ESAY",		"do_esay",		70	| CC	| PASS			},
	{ 0, "JOIN",		"do_join",		70	| CC	| PASS			},
	{ 0, "KS",		"do_kicksay",		70	| CC	| PASS			},
	{ 0, "KSLIST",		"do_kslist",		70	| CC	| PASS	| DCC		},
	{ 0, "PART",		"do_part",		70	| CC	| PASS			},
	{ 0, "RKS",		"do_rkicksay",		70	| CC	| PASS			},
	{ 0, "SETPASS",		"do_setpass",		70	| CC	| PASS	| NOPUB		},

	{ 0, "ADDSERVER",	"do_addserver",		80	| CC	| PASS	| ARGS		},
	{ 0, "AWAY",		"do_away",		80	| CC	| PASS			},
	{ 0, "DELSERVER",	"do_delserver",		80	| CC	| PASS	| ARGS		},
	{ 0, "FORGET",		"do_forget",		80	| CC	| PASS			},
	{ 0, "LAST",		"do_last",		80	| CC	| PASS	| DCC		},
	{ 0, "LOAD",		"do_loadall",		80	| CC	| PASS			},
#ifdef DYNCMDLEVELS
	{ 0, "LOADLEVELS",	"do_loadlevels",	80	| CC	| PASS			},
#endif
	{ 0, "LOADLISTS",	"do_loadlists",		80	| CC	| PASS			},
	{ 0, "ME",		"do_me",		80	| CC	| PASS			},
	{ 0, "MSG",		"do_msg",		80	| CC	| PASS	| ARGS		},
	{ 0, "NEXTSERVER",	"do_nextserver",	80	| CC	| PASS			},
	{ 0, "REHASH",		"do_rehash2",		80	| CC	| PASS			},
	{ 0, "REPORT",		"do_report",		80	| CC	| PASS	| DCC		},
	{ 0, "RSTATMSG",	"do_rstatmsg",		80	| CC	| PASS	| DCC		},
	{ 0, "SAVE",		"do_saveall",		80	| CC	| PASS			},
#ifdef DYNCMDLEVELS
	{ 0, "SAVELEVELS",	"do_savelevels",	80	| CC	| PASS			},
#endif
	{ 0, "SAVELISTS",	"do_savelists",		80	| CC	| PASS			},
	{ 0, "SAY",		"do_say",		80	| CC	| PASS			},
	{ 0, "SERVER",		"do_server",		80	| CC	| PASS			},
	{ 0, "SERVERLIST",	"do_serverlist",	80	| CC	| PASS	| DCC		},
	{ 0, "STATMSG",		"do_statmsg",		80	| CC	| PASS	| DCC		},
	{ 0, "STATS",		"do_servstats",		80	| CC	| PASS	| DCC		},
	{ 0, "USERHOST",	"do_userhost",		80	| CC	| PASS			},
	{ 0, "VIRTUAL",		"do_virtual",		80	| CC	| PASS			},
#ifdef WINGATES
	{ 0, "WINGATE",		"do_wingate",		80	| CC	| PASS			},
#endif

#ifdef DYNCMDLEVELS
	{ 0, "CHACCESS",	"do_chaccess",		90	| CC	| PASS			},
#endif
	{ 0, "CLEARSHIT",	"do_clearshit",		90	| CC	| PASS			},
	{ 0, "DO",		"do_do",		90	| CC	| PASS			},
#ifdef LINKING
	{ 0, "LINK",		"do_link",		90	| CC	| PASS			},
#endif
	{ 0, "NICK",		"do_nick",		90	| CC	| PASS			},
	{ 0, "RSPY",		"do_rspy",		90	| CC	| PASS			},
	{ 0, "RSPYMSG",		"do_rspymsg",		90	| CC	| PASS			},
	{ 0, "SET",		"do_set",		90	| CC	| PASS			},
	{ 0, "SPY",		"do_spy",		90	| CC	| PASS	| DCC		},
	{ 0, "SPYLIST",		"do_spylist",		90	| CC	| PASS	| DCC		},
	{ 0, "SPYMSG",		"do_spymsg",		90	| CC	| PASS			},
	{ 0, "TOG",		"do_toggle",		90	| CC	| PASS			},

#ifdef SERVICES
	{ 0, "SERVICE",         "do_service",           100     | CC    | PASS  | DCC 		},
#endif /* SERVICES */
	{ 0, "CORE",		"do_core",		100	| CC	| PASS	| DCC		},
	{ 0, "BOOT",		"do_boot",		100     | CC    | PASS  		},
#ifdef DEBUG
	{ 0, "DEBUG",		"do_debug",		100	| CC	| PASS			},
#endif
	{ 0, "DIE",		"do_die",		100	| CC	| PASS			},
	{ 0, "RESET",		"do_reset",		100	| CC	| PASS	| NOCMD		},
#ifdef SPAWN
	{ 0, "SPAWN",		"do_spawn",		100	| CC	| PASS			},
#endif /* SPAWN */
	{ 0, "SHUTDOWN",	"do_shutdown",		100	| CC	| PASS	| NOPUB | NOCMD	},
#ifdef UPTIME
	{ 0, "USET",		"do_uset",		100	| CC	| PASS			},
#endif /* UPTIME */
	{ 0, NULL,		NULL,			0					},
};

#define __define_strng	3
#define __define_print	2
#define __struct_print	1

int main(int argc, char **argv)
{
	char	tmp[100];
	char	*pt;
	int	i,j,wh;
	int	pass;
	int	ct;
	int	sl;

	pass = __define_strng;
	ct = 0;

	printf("#ifndef _H_MCMD\n#define _H_MCMD\n\n");

	while(pass)
	{
		if (pass == __struct_print)
		{
			printf("OnMsg mcmd[] =\n{\n");
		}
		for(i=0;pre_mcmd[i].name;i++)
		{
			pt = 0;
			wh = 0;
			for(j=0;pre_mcmd[j].name;j++)
			{
				if (pre_mcmd[j].pass != pass)
				{
					pt = pre_mcmd[j].name;
					wh = j;
					break;
				}
			}
			for(j=0;pre_mcmd[j].name;j++)
			{
				if ((pre_mcmd[j].pass != pass) && (strcmp(pt,pre_mcmd[j].name) > 0))
				{
					pt = pre_mcmd[j].name;
					wh = j;
				}
			}
			if (pass == __define_strng)
			{
				printf("#define S_%s%s\t\"%s\"\n",pt,((strlen(pt) + 2) < 8) ? "\t" : "",pt);
			}
			if (pass == __define_print)
			{
				printf("#define C_%s%s\tmcmd[%i].name\n",pt,((strlen(pt) + 2) < 8) ? "\t" : "",ct);
				ct++;
			}
			if (pass == __struct_print)
			{
				sprintf(tmp,"%lu\t",(pre_mcmd[wh].flags & CLEVEL));
				if (pre_mcmd[wh].flags & DCC)
					strcat(tmp,"|DCC");
				if (pre_mcmd[wh].flags & CC)
					strcat(tmp,"|CC");
				if (pre_mcmd[wh].flags & PASS)
					strcat(tmp,"|PASS");
				if (pre_mcmd[wh].flags & ARGS)
					strcat(tmp,"|ARGS");
				if (pre_mcmd[wh].flags & NOPUB)
					strcat(tmp,"|NOPUB");
				if (pre_mcmd[wh].flags & NOCMD)
					strcat(tmp,"|NOCMD");
				sl = strlen(tmp) - 1;
				while(sl <= 16)
				{
					strcat(tmp,"\t");
					sl += 8;
				}
				printf(
					"\t{ \"%s\",%s\tNULL,\t%s,%s\t%s\t},\n",
					pt,((strlen(pt) + 5) < 8) ? "\t" : "",pre_mcmd[wh].func,
					((strlen(pre_mcmd[wh].func) + 1) < 8) ? "\t" : "",tmp
				);
			}
			pre_mcmd[wh].pass = pass;
		}
		if (pass == __define_strng)
		{
			printf("\n");
		}
		if (pass == __define_print)
		{
			printf("\n");
			printf("\n#ifdef COM_ONS_C\n\n");
		}
		if (pass == __struct_print)
			printf("\t{ NULL,\t\tNULL,\tNULL,\t\t0\t\t\t\t}\n};\n\n");
		pass--;
	}
	printf("#else\n\nextern\tOnMsg\tmcmd[];\n\n");
	printf("#endif /" "*" " COM_ONS_C " "*" "/\n\n#endif /" "*" " _H_MCMD " "*" "/\n\n");
	return(0);
}
