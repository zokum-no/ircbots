
typedef struct
{
	char *command;
	char *usage;

} aUsageList;

aUsageList ulist[] =
{
{ S_ACCESS,	"@ [channel] [nick|userhost]"						},
{ S_ADD,	"@ <handle> <channel> <nick|userhost> <level> [aop] [prot] [pass]"	},
{ S_ADDSERVER,	"@ <host> [port]"							},
{ S_AWAY,	"@ [message]"								},
{ S_BAN,	"@ [channel] <nick|mask>"						},
{ S_BANLIST,	"@ [channel]"								},
{ S_CCHAN,	"@ [channel]"								},
#ifdef DYNCMDLEVELS
{ S_CHACCESS,	"@ <command> [level]"							},
#endif
{ S_CHANNELS,	"@"									},
{ S_CHAT,	"@"									},
{ S_CLVL,	"@ <handle> <level>"							},
#ifdef LINKING
{ S_CMD,	"@ [=botnick] <command>"						},
#endif
{ S_CMDCHAR,	"@ [cmdchar]"								},
{ S_CORE,	"@"									},
{ S_CSERV,	"@"									},
{ S_CTCP,	"@ <nick|channel> <request>"						},
{ S_CYCLE,	"@ [channel]"								},
{ S_DEL,	"@ <handle>"								},
{ S_DELSERVER,	"@ <servername> [port]"							},
{ S_DEOP,	"@ [channel] <nick|pattern [...]>"					},
{ S_DIE,	"@ [reason]"								},
{ S_DO,		"@ <raw_irc>"								},
{ S_DOWN,	"@ [channel]"								},
{ S_ECHO,	"@ <on|off>"								},
{ S_ESAY,	"@ [channel] <message>"							},
#ifdef WARRING
{ S_FLOOD,	"@ <nick|channel>"							},
#endif
{ S_FORGET,	"@ <channel>"								},
{ S_HELP,	"@ [topic|command|level|pattern]"					},
{ S_HOST,	"@ <ADD|DEL> <handle> <mask>"						},
{ S_IDLE,	"@ <nick>"								},
{ S_INSULT,	"@ [nick|channel]"							},
{ S_INVITE,	"@ [channel] [nick]"							},
{ S_JOIN,	"@ <channel> [key]"							},
{ S_KB,		"@ [channel] <nick> [reason]"						},
{ S_KICK,	"@ [channel] <nick> [reason]"						},
{ S_KS,		"@ <channel> <\"String to kick on\"> <\"kick reason\">"			},
{ S_KSLIST,	"@"									},
{ S_LAST,	"@ [number of commands]"						},
#ifdef LINKING
{ "LINK-ADD",	"LINK ADD <entity> <linkpass> <host> <linkport>"			},
{ "LINK-DEL",	"LINK DEL <entity>"							},
{ "LINK-PORT",	"LINK PORT <linkport>"							},
{ S_LINK,	"@ <UP|DOWN|ADD|DEL|PORT> <...>"					},
#endif
{ S_LOAD,	"@"									},
#ifdef DYNCMDLEVELS
{ S_LOADLEVELS,	"@"									},
#endif
{ S_LOADLISTS,	"@"									},
{ S_LUSERS,	"@"									},
{ S_ME,		"@ [channel] <action>"							},
{ S_MODE,	"@ <channel> <modes>"							},
{ S_MSG,	"@ <nick|channel> <message>"						},
{ S_NAMES,	"@ [channel]"								},
{ S_NEXTSERVER,	"@"									},
{ S_NICK,	"@ <nick>"								},
{ S_ONTIME,	"@"									},
{ S_OP,		"@ [channel] [nick|mask]"						},
{ S_PART,	"@ <channel>"								},
{ S_PASSWD,	"@ [oldpassword] <newpassword>"						},
{ S_PICKUP,	"@ [nick|channel]"							},
{ S_QSHIT,	"@ <nick> [reason]"							},
{ S_REHASH,	"@"									},
{ S_REPORT,	"@ [channel]"								},
{ S_RESET,	"@"									},
{ S_RKS,	"@ <channel> <pattern of words banned>"					},
{ S_RSHIT,	"@ <channel> <nick|userhost>"						},
{ S_RSPY,	"@ <channel> [nick|channel]"						},
{ S_RSPYMSG,	"@ [nick]"								},
{ S_RSTATMSG,	"@"									},
#ifdef WARRING
{ S_RSTEAL,	"@ <channel>"								},
#endif
{ S_RT,		"@ <channel>"								},
{ S_SAVE,	"@"									},
#ifdef DYNCMDLEVELS
{ S_SAVELEVELS,	"@"									},
#endif
{ S_SAVELISTS,	"@"									},
{ S_SAY,	"@ <channel> <message>"							},
{ S_SCREW,	"@ [channel] <nick> [reason]"						},
#ifdef SEEN
{ S_SEEN,	"@ <nick>"								},
#endif
{ S_SERVER,	"@ <servername> [port] [login] [ircname]"				},
{ S_SERVERLIST,	"@"									},
{ S_SET,	"@ [channel] <setting> <value>"						},
{ S_SETPASS,	"@ <handle> <password>"							},
{ S_SHIT,	"@ <channel> <nick|userhost> <level> [expire] <reason>"			},
{ S_SHITLIST,	"@"									},
{ S_SHITLVL,	"@ <channel> <nick|userhost> <newlevel>"				},
{ S_SHOWIDLE,	"@ [seconds]"								},
{ S_SITEBAN,	"@ [channel] <nick|userhost>"						},
{ S_SITEKB,	"@ [channel] <nick> [reason]"						},
{ S_SPY,	"@ <channel> [channel]"							},
{ S_SPYLIST,	"@ <channel>"								},
{ S_SPYMSG,	"@"									},
{ S_STATMSG,	"@"									},
{ S_STATS,	"@ <type> [servername]"							},
#ifdef WARRING
{ S_STEAL,	"@ <channel>"								},
#endif
{ S_TIME,	"@"									},
{ S_TOG,	"@ [channel] <toggle> [0|1|on|off]"					},
{ S_TOPIC,	"@ [channel] <text>"							},
{ S_UNBAN,	"@ [channel] [nick|userhost]"						},
{ S_UNVOICE,	"@ [channel] <nick|pattern [...]>"					},
{ S_UP,		"@ [channel]"								},
{ S_UPTIME,	"@"									},
{ S_USAGE,	"@ <command>"								},
{ S_USER,	"@ <handle> <modifiers [...]>"						},
{ S_USERHOST,	"@ <nick>"								},
{ S_USERLIST,	"@ [+minlevel] [-maxlevel] [#channel] [usermask] [-B] [-C]"		},
#ifdef UPTIME
{ S_USET,	"@ <PORT port> | <SERVER host> | <SEND>"				},
#endif /* UPTIME */
{ S_USTATS,	"@ <nick|userhost>"							},
{ S_VER,	"@"									},
{ S_VERIFY,	"@ <password>"								},
{ S_VIRTUAL,	"@ <host>"								},
{ S_VOICE, 	"@ [channel] [nick|mask [...]]"						},
{ S_WALL,	"@ [channel] <message>"							},
{ S_WHO,	"@ <channel> [-ops|-nonops] [pattern]"					},
{ S_WHOIS,	"@ <nick>"								},
{ S_WHOM,	"@"									},
#ifdef WINGATE
{ S_WINGATE,	"@ <host> <port>"							},
#endif
{ S_SHUTDOWN,	"@"									},
{ NULL,		NULL									},
};
