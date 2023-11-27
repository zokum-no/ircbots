#ifndef _H_MCMD
#define _H_MCMD

#define S_ACCESS	"ACCESS"
#define S_ADD		"ADD"
#define S_ADDSERVER	"ADDSERVER"
#define S_AWAY		"AWAY"
#define S_BAN		"BAN"
#define S_BANLIST	"BANLIST"
#define S_BOOT		"BOOT"
#define S_BYE		"BYE"
#define S_CCHAN		"CCHAN"
#define S_CHACCESS	"CHACCESS"
#define S_CHANNELS	"CHANNELS"
#define S_CHAT		"CHAT"
#define S_CLEARSHIT	"CLEARSHIT"
#define S_CLVL		"CLVL"
#define S_CMD		"CMD"
#define S_CMDCHAR	"CMDCHAR"
#define S_CORE		"CORE"
#define S_CSERV		"CSERV"
#define S_CTCP		"CTCP"
#define S_CYCLE		"CYCLE"
#define S_DEBUG		"DEBUG"
#define S_DEL		"DEL"
#define S_DELSERVER	"DELSERVER"
#define S_DEOP		"DEOP"
#define S_DIE		"DIE"
#define S_DO		"DO"
#define S_DOWN		"DOWN"
#define S_ECHO		"ECHO"
#define S_ESAY		"ESAY"
#define S_FORGET	"FORGET"
#define S_HELP		"HELP"
#define S_HOST		"HOST"
#define S_IDLE		"IDLE"
#define S_INSULT	"INSULT"
#define S_INVITE	"INVITE"
#define S_JOIN		"JOIN"
#define S_KB		"KB"
#define S_KICK		"KICK"
#define S_KS		"KS"
#define S_KSLIST	"KSLIST"
#define S_LAST		"LAST"
#define S_LINK		"LINK"
#define S_LOAD		"LOAD"
#define S_LOADLEVELS	"LOADLEVELS"
#define S_LOADLISTS	"LOADLISTS"
#define S_LUSERS	"LUSERS"
#define S_ME		"ME"
#define S_MODE		"MODE"
#define S_MSG		"MSG"
#define S_NAMES		"NAMES"
#define S_NEXTSERVER	"NEXTSERVER"
#define S_NICK		"NICK"
#define S_ONTIME	"ONTIME"
#define S_OP		"OP"
#define S_PART		"PART"
#define S_PASSWD	"PASSWD"
#define S_PICKUP	"PICKUP"
#define S_QSHIT		"QSHIT"
#define S_REHASH	"REHASH"
#define S_REPORT	"REPORT"
#define S_RESET		"RESET"
#define S_RKS		"RKS"
#define S_RSHIT		"RSHIT"
#define S_RSPY		"RSPY"
#define S_RSPYMSG	"RSPYMSG"
#define S_RSTATMSG	"RSTATMSG"
#define S_RT		"RT"
#define S_SAVE		"SAVE"
#define S_SAVELEVELS	"SAVELEVELS"
#define S_SAVELISTS	"SAVELISTS"
#define S_SAY		"SAY"
#define S_SCREW		"SCREW"
#define S_SEEN		"SEEN"
#define S_SERVER	"SERVER"
#define S_SERVERLIST	"SERVERLIST"
#define S_SERVICE	"SERVICE"
#define S_SET		"SET"
#define S_SETPASS	"SETPASS"
#define S_SHIT		"SHIT"
#define S_SHITLIST	"SHITLIST"
#define S_SHITLVL	"SHITLVL"
#define S_SHOWIDLE	"SHOWIDLE"
#define S_SHUTDOWN	"SHUTDOWN"
#define S_SITEBAN	"SITEBAN"
#define S_SITEKB	"SITEKB"
#define S_SPY		"SPY"
#define S_SPYLIST	"SPYLIST"
#define S_SPYMSG	"SPYMSG"
#define S_STATMSG	"STATMSG"
#define S_STATS		"STATS"
#define S_TIME		"TIME"
#define S_TOG		"TOG"
#define S_TOPIC		"TOPIC"
#define S_UNBAN		"UNBAN"
#define S_UNVERIFY	"UNVERIFY"
#define S_UNVOICE	"UNVOICE"
#define S_UP		"UP"
#define S_UPTIME	"UPTIME"
#define S_USAGE		"USAGE"
#define S_USER		"USER"
#define S_USERHOST	"USERHOST"
#define S_USERLIST	"USERLIST"
#define S_USET		"USET"
#define S_USTATS	"USTATS"
#define S_VER		"VER"
#define S_VERIFY	"VERIFY"
#define S_VIRTUAL	"VIRTUAL"
#define S_VOICE		"VOICE"
#define S_WALL		"WALL"
#define S_WHO		"WHO"
#define S_WHOIS		"WHOIS"
#define S_WHOM		"WHOM"
#define S_WINGATE	"WINGATE"

#define C_ACCESS	mcmd[0].name
#define C_ADD		mcmd[1].name
#define C_ADDSERVER	mcmd[2].name
#define C_AWAY		mcmd[3].name
#define C_BAN		mcmd[4].name
#define C_BANLIST	mcmd[5].name
#define C_BOOT		mcmd[6].name
#define C_BYE		mcmd[7].name
#define C_CCHAN		mcmd[8].name
#define C_CHACCESS	mcmd[9].name
#define C_CHANNELS	mcmd[10].name
#define C_CHAT		mcmd[11].name
#define C_CLEARSHIT	mcmd[12].name
#define C_CLVL		mcmd[13].name
#define C_CMD		mcmd[14].name
#define C_CMDCHAR	mcmd[15].name
#define C_CORE		mcmd[16].name
#define C_CSERV		mcmd[17].name
#define C_CTCP		mcmd[18].name
#define C_CYCLE		mcmd[19].name
#define C_DEBUG		mcmd[20].name
#define C_DEL		mcmd[21].name
#define C_DELSERVER	mcmd[22].name
#define C_DEOP		mcmd[23].name
#define C_DIE		mcmd[24].name
#define C_DO		mcmd[25].name
#define C_DOWN		mcmd[26].name
#define C_ECHO		mcmd[27].name
#define C_ESAY		mcmd[28].name
#define C_FORGET	mcmd[29].name
#define C_HELP		mcmd[30].name
#define C_HOST		mcmd[31].name
#define C_IDLE		mcmd[32].name
#define C_INSULT	mcmd[33].name
#define C_INVITE	mcmd[34].name
#define C_JOIN		mcmd[35].name
#define C_KB		mcmd[36].name
#define C_KICK		mcmd[37].name
#define C_KS		mcmd[38].name
#define C_KSLIST	mcmd[39].name
#define C_LAST		mcmd[40].name
#define C_LINK		mcmd[41].name
#define C_LOAD		mcmd[42].name
#define C_LOADLEVELS	mcmd[43].name
#define C_LOADLISTS	mcmd[44].name
#define C_LUSERS	mcmd[45].name
#define C_ME		mcmd[46].name
#define C_MODE		mcmd[47].name
#define C_MSG		mcmd[48].name
#define C_NAMES		mcmd[49].name
#define C_NEXTSERVER	mcmd[50].name
#define C_NICK		mcmd[51].name
#define C_ONTIME	mcmd[52].name
#define C_OP		mcmd[53].name
#define C_PART		mcmd[54].name
#define C_PASSWD	mcmd[55].name
#define C_PICKUP	mcmd[56].name
#define C_QSHIT		mcmd[57].name
#define C_REHASH	mcmd[58].name
#define C_REPORT	mcmd[59].name
#define C_RESET		mcmd[60].name
#define C_RKS		mcmd[61].name
#define C_RSHIT		mcmd[62].name
#define C_RSPY		mcmd[63].name
#define C_RSPYMSG	mcmd[64].name
#define C_RSTATMSG	mcmd[65].name
#define C_RT		mcmd[66].name
#define C_SAVE		mcmd[67].name
#define C_SAVELEVELS	mcmd[68].name
#define C_SAVELISTS	mcmd[69].name
#define C_SAY		mcmd[70].name
#define C_SCREW		mcmd[71].name
#define C_SEEN		mcmd[72].name
#define C_SERVER	mcmd[73].name
#define C_SERVERLIST	mcmd[74].name
#define C_SERVICE	mcmd[75].name
#define C_SET		mcmd[76].name
#define C_SETPASS	mcmd[77].name
#define C_SHIT		mcmd[78].name
#define C_SHITLIST	mcmd[79].name
#define C_SHITLVL	mcmd[80].name
#define C_SHOWIDLE	mcmd[81].name
#define C_SHUTDOWN	mcmd[82].name
#define C_SITEBAN	mcmd[83].name
#define C_SITEKB	mcmd[84].name
#define C_SPY		mcmd[85].name
#define C_SPYLIST	mcmd[86].name
#define C_SPYMSG	mcmd[87].name
#define C_STATMSG	mcmd[88].name
#define C_STATS		mcmd[89].name
#define C_TIME		mcmd[90].name
#define C_TOG		mcmd[91].name
#define C_TOPIC		mcmd[92].name
#define C_UNBAN		mcmd[93].name
#define C_UNVERIFY	mcmd[94].name
#define C_UNVOICE	mcmd[95].name
#define C_UP		mcmd[96].name
#define C_UPTIME	mcmd[97].name
#define C_USAGE		mcmd[98].name
#define C_USER		mcmd[99].name
#define C_USERHOST	mcmd[100].name
#define C_USERLIST	mcmd[101].name
#define C_USET		mcmd[102].name
#define C_USTATS	mcmd[103].name
#define C_VER		mcmd[104].name
#define C_VERIFY	mcmd[105].name
#define C_VIRTUAL	mcmd[106].name
#define C_VOICE		mcmd[107].name
#define C_WALL		mcmd[108].name
#define C_WHO		mcmd[109].name
#define C_WHOIS		mcmd[110].name
#define C_WHOM		mcmd[111].name
#define C_WINGATE	mcmd[112].name


#ifdef COM_ONS_C

OnMsg mcmd[] =
{
	{ "ACCESS",	NULL,	do_access,	10	|CC|PASS		},
	{ "ADD",	NULL,	do_add,		60	|CC|PASS|ARGS		},
	{ "ADDSERVER",	NULL,	do_addserver,	80	|CC|PASS|ARGS		},
	{ "AWAY",	NULL,	do_away,	80	|CC|PASS		},
	{ "BAN",	NULL,	do_ban,		40	|CC|PASS		},
	{ "BANLIST",	NULL,	do_banlist,	40	|DCC|CC|PASS		},
	{ "BOOT",	NULL,	do_boot,	100	|CC|PASS		},
	{ "BYE",	NULL,	do_dcc_kill,	10	|CC			},
	{ "CCHAN",	NULL,	do_cchan,	40	|CC|PASS		},
	{ "CHACCESS",	NULL,	do_chaccess,	90	|CC|PASS		},
	{ "CHANNELS",	NULL,	do_channels,	40	|DCC|CC|PASS		},
	{ "CHAT",	NULL,	do_chat,	10	|CC|PASS|NOCMD		},
	{ "CLEARSHIT",	NULL,	do_clearshit,	90	|CC|PASS		},
	{ "CLVL",	NULL,	do_clvl,	60	|CC|PASS		},
	{ "CMD",	NULL,	do_linkcmd,	40	|CC|PASS|NOCMD		},
	{ "CMDCHAR",	NULL,	do_cmdchar,	10	|CC|PASS		},
	{ "CORE",	NULL,	do_core,	100	|DCC|CC|PASS		},
	{ "CSERV",	NULL,	do_cserv,	40	|CC|PASS		},
	{ "CTCP",	NULL,	do_ctcp,	60	|CC|PASS|ARGS		},
	{ "CYCLE",	NULL,	do_cycle,	70	|CC|PASS		},
	{ "DEBUG",	NULL,	do_debug,	100	|CC|PASS		},
	{ "DEL",	NULL,	do_del,		60	|CC|PASS		},
	{ "DELSERVER",	NULL,	do_delserver,	80	|CC|PASS|ARGS		},
	{ "DEOP",	NULL,	do_deop,	40	|CC|PASS		},
	{ "DIE",	NULL,	do_die,		100	|CC|PASS		},
	{ "DO",		NULL,	do_do,		90	|CC|PASS		},
	{ "DOWN",	NULL,	do_deopme,	10	|CC			},
	{ "ECHO",	NULL,	do_echo,	10	|CC|PASS		},
	{ "ESAY",	NULL,	do_esay,	70	|CC|PASS		},
	{ "FORGET",	NULL,	do_forget,	80	|CC|PASS		},
	{ "HELP",	NULL,	do_help,	10	|DCC|CC|PASS		},
	{ "HOST",	NULL,	do_host,	20	|CC|PASS		},
	{ "IDLE",	NULL,	do_idle,	40	|CC|PASS|ARGS		},
	{ "INSULT",	NULL,	do_insult,	50	|CC|PASS		},
	{ "INVITE",	NULL,	do_invite,	40	|CC|PASS		},
	{ "JOIN",	NULL,	do_join,	70	|CC|PASS		},
	{ "KB",		NULL,	do_kickban,	40	|CC|PASS		},
	{ "KICK",	NULL,	do_kick,	40	|CC|PASS		},
	{ "KS",		NULL,	do_kicksay,	70	|CC|PASS		},
	{ "KSLIST",	NULL,	do_kslist,	70	|DCC|CC|PASS		},
	{ "LAST",	NULL,	do_last,	80	|DCC|CC|PASS		},
	{ "LINK",	NULL,	do_link,	90	|CC|PASS		},
	{ "LOAD",	NULL,	do_loadall,	80	|CC|PASS		},
	{ "LOADLEVELS",	NULL,	do_loadlevels,	80	|CC|PASS		},
	{ "LOADLISTS",	NULL,	do_loadlists,	80	|CC|PASS		},
	{ "LUSERS",	NULL,	do_lusers,	40	|DCC|CC|PASS		},
	{ "ME",		NULL,	do_me,		80	|CC|PASS		},
	{ "MODE",	NULL,	do_mode,	40	|CC|PASS		},
	{ "MSG",	NULL,	do_msg,		80	|CC|PASS|ARGS		},
	{ "NAMES",	NULL,	do_names,	40	|CC|PASS		},
	{ "NEXTSERVER",	NULL,	do_nextserver,	80	|CC|PASS		},
	{ "NICK",	NULL,	do_nick,	90	|CC|PASS		},
	{ "ONTIME",	NULL,	do_ontime,	20	|CC|PASS		},
	{ "OP",		NULL,	do_op,		40	|CC|PASS		},
	{ "PART",	NULL,	do_part,	70	|CC|PASS		},
	{ "PASSWD",	NULL,	do_passwd,	10	|PASS|NOPUB		},
	{ "PICKUP",	NULL,	do_pickup,	50	|CC|PASS		},
	{ "QSHIT",	NULL,	do_qshit,	50	|CC|PASS		},
	{ "REHASH",	NULL,	do_rehash2,	80	|CC|PASS		},
	{ "REPORT",	NULL,	do_report,	80	|DCC|CC|PASS		},
	{ "RESET",	NULL,	do_reset,	100	|CC|PASS|NOCMD	},
	{ "RKS",	NULL,	do_rkicksay,	70	|CC|PASS		},
	{ "RSHIT",	NULL,	do_rshit,	50	|CC|PASS		},
	{ "RSPY",	NULL,	do_rspy,	90	|CC|PASS		},
	{ "RSPYMSG",	NULL,	do_rspymsg,	90	|CC|PASS		},
	{ "RSTATMSG",	NULL,	do_rstatmsg,	80	|DCC|CC|PASS		},
	{ "RT",		NULL,	do_randomtopic,	40	|CC|PASS		},
	{ "SAVE",	NULL,	do_saveall,	80	|CC|PASS		},
	{ "SAVELEVELS",	NULL,	do_savelevels,	80	|CC|PASS		},
	{ "SAVELISTS",	NULL,	do_savelists,	80	|CC|PASS		},
	{ "SAY",	NULL,	do_say,		80	|CC|PASS		},
	{ "SCREW",	NULL,	do_screwban,	40	|CC|PASS		},
	{ "SEEN",	NULL,	do_seen,	20	|CC|PASS		},
	{ "SERVER",	NULL,	do_server,	80	|CC|PASS		},
	{ "SERVERLIST",	NULL,	do_serverlist,	80	|DCC|CC|PASS		},
	{ "SERVICE",	NULL,	do_service,	100	|DCC|CC|PASS		},
	{ "SET",	NULL,	do_set,		90	|CC|PASS		},
	{ "SETPASS",	NULL,	do_setpass,	70	|CC|PASS|NOPUB		},
	{ "SHIT",	NULL,	do_shit,	50	|CC|PASS		},
	{ "SHITLIST",	NULL,	do_shitlist,	50	|DCC|CC|PASS		},
	{ "SHITLVL",	NULL,	do_cslvl,	50	|CC|PASS		},
	{ "SHOWIDLE",	NULL,	do_showidle,	60	|DCC|CC|PASS		},
	{ "SHUTDOWN",	NULL,	do_shutdown,	100	|CC|PASS|NOPUB|NOCMD	},
	{ "SITEBAN",	NULL,	do_siteban,	40	|CC|PASS		},
	{ "SITEKB",	NULL,	do_sitekickban,	40	|CC|PASS		},
	{ "SPY",	NULL,	do_spy,		90	|DCC|CC|PASS		},
	{ "SPYLIST",	NULL,	do_spylist,	90	|DCC|CC|PASS		},
	{ "SPYMSG",	NULL,	do_spymsg,	90	|CC|PASS		},
	{ "STATMSG",	NULL,	do_statmsg,	80	|DCC|CC|PASS		},
	{ "STATS",	NULL,	do_servstats,	80	|DCC|CC|PASS		},
	{ "TIME",	NULL,	do_time,	40	|CC|PASS		},
	{ "TOG",	NULL,	do_toggle,	90	|CC|PASS		},
	{ "TOPIC",	NULL,	do_topic,	40	|CC|PASS		},
	{ "UNBAN",	NULL,	do_unban,	40	|CC|PASS		},
	{ "UNVERIFY",	NULL,	do_unverify,	10	|CC|PASS		},
	{ "UNVOICE",	NULL,	do_unvoice,	40	|CC|PASS		},
	{ "UP",		NULL,	do_opme,	40	|CC|PASS		},
	{ "UPTIME",	NULL,	do_uptime,	20	|CC|PASS		},
	{ "USAGE",	NULL,	do_usage,	10	|CC|PASS		},
	{ "USER",	NULL,	do_user,	60	|CC|PASS|ARGS		},
	{ "USERHOST",	NULL,	do_userhost,	80	|CC|PASS		},
	{ "USERLIST",	NULL,	do_userlist,	60	|DCC|CC|PASS		},
	{ "USET",	NULL,	do_uset,	100	|CC|PASS		},
	{ "USTATS",	NULL,	do_stats,	60	|DCC|CC|PASS		},
	{ "VER",	NULL,	do_vers,	20	|CC|PASS		},
	{ "VERIFY",	NULL,	do_auth,	10	|NOPUB			},
	{ "VIRTUAL",	NULL,	do_virtual,	80	|CC|PASS		},
	{ "VOICE",	NULL,	do_voice,	40	|CC|PASS		},
	{ "WALL",	NULL,	do_wall,	40	|CC|PASS|ARGS		},
	{ "WHO",	NULL,	do_showusers,	40	|DCC|CC|PASS		},
	{ "WHOIS",	NULL,	do_whois_irc,	40	|DCC|CC|PASS		},
	{ "WHOM",	NULL,	do_whom,	20	|DCC|CC|PASS		},
	{ "WINGATE",	NULL,	do_wingate,	80	|CC|PASS		},
	{ NULL,		NULL,	NULL,		0				}
};

#else

extern	OnMsg	mcmd[];

#endif /* COM_ONS_C */

#endif /* _H_MCMD */

