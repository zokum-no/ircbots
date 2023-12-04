/*

    Starglider Class EnergyMech, IRC bot software
    Parts Copyright (c) 1997-2000  proton

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
#ifdef MEGA
#  define MAIN_C
#endif /* MEGA */

#ifdef MAIN_C
#  define BEG
#else
#  define BEG extern
#endif /* MAIN_C */

#define DEFAULTCMDCHAR	'-'

#ifdef MAIN_C

char VERSION[]		= "1.0rc2";
char SRCDATE[]		= "October 17th, 2000";
char BOTCLASS[]		= "MiniMech";
char BOTLOGIN[]		= "mmech";

char NULLSTR[]		= "<NULL>";

char ERR_CHAN[]		= "I'm not on %s";
char ERR_FILEOPEN[]	= "Couldn't open the file %s";
char ERR_INITW[]	= "init: Warning: ";
char ERR_INITE[]	= "init: Error: ";
char ERR_NICK[]		= "Invalid nickname: %s";
char ERR_NOCHANNELS[]	= "I'm not active on any channels";
char ERR_NOSAVE[]	= "Lists could not be saved to file %s";
char ERR_NOTOPPED[]	= "I'm not opped on %s";

const char SHELLMASK[]	= "s!shell@energymech";
const char PUMASK[]	= "0!pipe@energymech";

#else /* MAIN_C */

extern	char VERSION[];
extern	char SRCDATE[];
extern	char BOTCLASS[];
extern	char BOTLOGIN[];

extern	char NULLSTR[];

extern	char ERR_CHAN[];
extern	char ERR_FILEOPEN[];
extern	char ERR_INITW[];
extern	char ERR_INITE[];
extern	char ERR_NICK[];
extern	char ERR_NOCHANNELS[];
extern	char ERR_NOSAVE[];
extern	char ERR_NOTOPPED[];

extern	char SHELLMASK[];
extern	char PUMASK[];

#endif /* MAIN_C */

BEG aBot	current;

BEG char	executable[FILENAMELEN];
BEG char	configfile[FILENAMELEN];

BEG aWillWont	will;				/* features enable/disable */
BEG time_t	uptime;
BEG int		sigmaster;
BEG int		shell_cmd,need_dcc;
BEG int		dccKill;
BEG struct	stat mechbin;
BEG int		do_exec;			/* call mech_exec on mechexit */
BEG int		do_fork;
BEG int		nummatch;			/* for match() */
BEG int		makecore;
BEG int		respawn;

BEG aKillSock	*killsocks;

BEG aXDCC	*xdccList;			/* XDCC filesends */

BEG aChan	*CurrentChan;
BEG aUser	*CurrentUser;
BEG aUser	*CurrentShit;
BEG char	CurrentNick[NUHLEN];

BEG int		userident;

BEG fd_set	read_fds;
BEG fd_set	write_fds;
BEG int		short_tv;

/*
 *  There is no need to call time() each time a timestamp
 *  needs to be updated. Clock rarely ticks a step unless
 *  its in select()
 */
BEG time_t	now;

#if defined(SESSIONS) || defined(DYNCMDLEVELS)
BEG time_t	lastsave;
#endif

#ifdef LINKING

BEG int		botlinkSIZE;
BEG aBotLink	*botlinkBASE;

BEG int		linkconfSIZE;
BEG aLinkConf	*linkconfBASE;

BEG char	my_entity[ENTITYLEN];
BEG char	my_pass[ENTITYLEN];
BEG int		linkport;
BEG int		linksock;
BEG time_t	last_autolink;
BEG int		botlinkident;

#ifdef LINKEVENTS
BEG int		LE;
#endif /* LINKEVENTS */

#ifdef PIPEUSER
BEG char	*PUnick;
#endif /* PIPEUSER */

#endif /* LINKING */

BEG aServer	*serverBASE;
BEG int		serverSIZE;
BEG int		serverident;

#ifdef DEBUG

BEG int		dodebug;
BEG char	debugbuf[HUGE];
BEG char	lastline[MAXLEN];
BEG int		debug_fd;
BEG char	debugfile[FILENAMELEN];
BEG time_t	runtime;

#ifdef SUPERDEBUG
BEG aMEA	*mrrec;
BEG void	*mallocdoer;
BEG int		debug_on_exit;
#endif /* SUPERDEBUG */

#endif /* DEBUG */

#ifdef MAIN_C

const VarType BinaryDefault =
{
/*
  NAME			TYPE			DEF.	STR.	MIN.	MAX.
*/
{ /* BT */		INT_VAR,		0,	NULL,	0,	999	},
{ /* AUB */		INT_VAR,		0,	NULL,	0,	999	},
{ /* CKL */		INT_VAR,		0,	NULL,	1,	999	},
{ /* MDL */		INT_VAR,		7,	NULL,	3,	20	},
{ /* MBL */		INT_VAR,		7,	NULL,	3,	20	},
{ /* MKL */		INT_VAR,		7,	NULL,	3,	20	},
{ /* MPL */		INT_VAR,		1,	NULL,	0,	3	},
{ /* FL */		INT_VAR,		6,	NULL,	1,	20	},
{ /* FPL */		INT_VAR,		0,	NULL,	0,	2	},
{ /* NCL */		INT_VAR,		4,	NULL,	3,	20	},
{ /* IKT */		INT_VAR,		20,	NULL,	2,	999	},
{ /* MAL */		INT_VAR,		90,	NULL,	0,	100	},
{ /* BANMODES */	INT_VAR+GLOBAL_VAR,	4,	NULL,	1,	10	},
{ /* OPMODES */		INT_VAR+GLOBAL_VAR,	4,	NULL,	1,	10	},
{ /* RK */		TOG_VAR,		0,	NULL,	0,	0	},
{ /* TOP */		TOG_VAR,		0,	NULL,	0,	0	},
{ /* CK */		TOG_VAR,		0,	NULL,	0,	0	},
{ /* AOP */		TOG_VAR,		0,	NULL,	0,	0	},
{ /* SHIT */		TOG_VAR,		0,	NULL,	0,	0	},
{ /* PUB */		TOG_VAR,		1,	NULL,	0,	0	},
{ /* PROT */		TOG_VAR,		0,	NULL,	0,	0	},
{ /* AS */		TOG_VAR,		0,	NULL,	0,	0	},
{ /* ABK */		TOG_VAR,		0,	NULL,	0,	0	},
{ /* IK */		TOG_VAR,		0,	NULL,	0,	0	},
{ /* SD */		TOG_VAR,		0,	NULL,	0,	0	},
{ /* SO */		TOG_VAR,		0,	NULL,	0,	0	},
{ /* ENFM */		TOG_VAR,		0,	NULL,	0,	0	},
{ /* MASS */		TOG_VAR,		1,	NULL,	0,	0	},
{ /* ENFM */		STR_VAR,		0,	NULL,	0,	0	},
{ /* CC */		TOG_VAR+GLOBAL_VAR,	1,	NULL,	0,	0	},
{ /* KS */		TOG_VAR,		0,	NULL,	0,	0	},
{ /* DCC */		TOG_VAR+GLOBAL_VAR,	0,	NULL,	0,	0	},
{ /* CTL */		TOG_VAR,		0,	NULL,	0,	0	},
{ /* SPY */		TOG_VAR+GLOBAL_VAR,	0,	NULL,	0,	0	},
{ /* IWM */		TOG_VAR+GLOBAL_VAR,	0,	NULL,	0,	0	},
{ /* CLOAK */		TOG_VAR+GLOBAL_VAR,	1,	NULL,	0,	0	},
{ /* RV */		TOG_VAR+GLOBAL_VAR,	0,	NULL,	0,	0	},
{ /* RF */		TOG_VAR+GLOBAL_VAR,	0,	NULL,	0,	0	},
{ /* AAWAY */		INT_VAR+GLOBAL_VAR,	0,	NULL,	0,	999	},
{ /* NOIDLE */		TOG_VAR+GLOBAL_VAR,	0,	NULL,	0,	0	},
{ /* CHANLOG */		TOG_VAR,		0,	NULL,	0,	0	},
{ /* AVOICE */		INT_VAR,		0,	NULL,	0,	2	},
{			0,			0,	NULL,	0,	0	}
};

const aVarName VarName[SIZE_VARS] =
{
{ "BT",		"Which bans to unban when ban-limit is reached"				},
{ "AUB",	"Erases old bans automatically, this sets how old they have to be"	},
{ "CKL",	"CAPS kick level (How many lines of CAPS are allowed)"			},
{ "MDL",	"Mass deop level (How many -o's count as a massdeop)"			},
{ "MBL",	"Mass ban level (How many +b's count as a massban)"			},
{ "MKL",	"Mass kick level (How many kicks count as a masskick)"			},
{ "MPL",	"Mass protection level (What to do with massmoders)"			},
{ "FL",		"Flood level (How many lines count as a flood)"				},
{ "FPL",	"Flood protection level (What to do with flooders)"			},
{ "NCL",	"Nick change level (How many nick changes allowed)"			},
{ "IKT",	"Idle-kick time (How long users can be idle)"				},
{ "MAL",	"Mass action level (Max userlevel that mass functions won't affect)"	},
{ "BANMODES",	"Number of MODE -b that can be done at a time"				},
{ "OPMODES",	"Number of MODE -o that can be done at a time"				},
{ "RK",		"Toggle for revenge kicking"						},
{ "TOP",	"Toggle for topic enforcement"						},
{ "CK",		"Toggle for CAPS kicking"						},
{ "AOP",	"Toggle for auto-opping"						},
{ "SHIT",	"Toggle for shit-kicking"						},
{ "PUB",	"Toggle for being able to use public commands"				},
{ "PROT",	"Toggle for protection"							},
{ "AS",		"Toggle for auto-shitlisting"						},
{ "ABK",	"Toggle for kicking banned users rejoining after splits"		},
{ "IK",		"Toggle for the kicking of those who are idle"				},
{ "SD",		"Toggle for deopping those who get opped by a server"			},
{ "SO",		"Toggle for letting only users get opped"				},
{ "ENFM",	"Toggle for mode enforcement"						},
{ "MASS",	"Toggle for checking of massmodes"					},
{ "ENFM",	"The modes to enforce"							},
{ "CC",		"Toggle for the necessity of using the command char"			},
{ "KS",		"Toggle for kicking on banned words"					},
{ "DCC",	"Toggle for allowing non-users dcc stuphs"				},
{ "CTL",	"Toggle for kicking users that have control chars in there userid"	},
{ "SPY",	"Toggle seeing who does commands"					},
{ "IWM",	"Toggle for ignoring wallops in spymsg"					},
{ "CLOAK",	"Toggle for ignoring all CTCP requests"					},
{ "RV",		"Toggle for random CTCP VERSION replies"				},
{ "RF",		"Toggle for random CTCP FINGER finger replies"				},
{ "AAWAY",	"Set for setting the bot automatically away"				},
{ "NOIDLE",	"Toggle for making the bots idle time always low"			},
{ "CHANLOG",	"Toggle for channel logging"						},
{ "AVOICE",	"Auto-voicing mode"							},
{ NULL,		NULL									}
};

/*
 *  tolowertab blatantly ripped from ircu2.9.32
 */
Uchar tolowertab[256] =
{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	' ',  '!',  '"',  '#',  '$',  '%',  '&',  0x27,
	'(',  ')',  '*',  '+',  ',',  '-',  '.',  '/',
	'0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',
	'8',  '9',  ':',  ';',  '<',  '=',  '>',  '?',
	'@',  'a',  'b',  'c',  'd',  'e',  'f',  'g',
	'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',
	'p',  'q',  'r',  's',  't',  'u',  'v',  'w',
	'x',  'y',  'z',  '{',  '|',  '}',  '~',  '_',
	'`',  'a',  'b',  'c',  'd',  'e',  'f',  'g',
	'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',
	'p',  'q',  'r',  's',  't',  'u',  'v',  'w',
	'x',  'y',  'z',  '{',  '|',  '}',  '~',  0x7f,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
	0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
	0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
	0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
	0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
	0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
	0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
	0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
	0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
	0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
	0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
	0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
	0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
	0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

#define NUM	0x01
#define NICK	0x02
#define FIRST	0x04

#define FNICK	(NICK|FIRST)
#define NNICK	(NICK|NUM)

Uchar attrtab[256] =
{
	0,	0,	0,	0,	0,	0,	0,	0,	/* 0x00 - 0x07 */
	0,	0,	0,	0,	0,	0,	0,	0,	/* 0x08 - 0x0F */
	0,	0,	0,	0,	0,	0,	0,	0,	/* 0x10 - 0x17 */
	0,	0,	0,	0,	0,	0,	0,	0,	/* 0x18 - 0x1F */
	0,	0,	0,	0,	0,	0,	0,	0,	/* 0x20 - 0x27 */
	0,	0,	0,	0,	0,	NICK,	0,	0,	/* 0x28 - 0x2F */
	NNICK,	NNICK,	NNICK,	NNICK,	NNICK,	NNICK,	NNICK,	NNICK,	/* 0x30 - 0x37 */
	NNICK,	NNICK,	0,	0,	0,	0,	0,	0,	/* 0x38 - 0x3F */
	0,	FNICK,	FNICK,	FNICK,	FNICK,	FNICK,	FNICK,	FNICK,	/* 0x40 - 0x47 */
	FNICK,	FNICK,	FNICK,	FNICK,	FNICK,	FNICK,	FNICK,	FNICK,	/* 0x48 - 0x4F */
	FNICK,	FNICK,	FNICK,	FNICK,	FNICK,	FNICK,	FNICK,	FNICK,	/* 0x50 - 0x57 */
	FNICK,	FNICK,	FNICK,	FNICK,	FNICK,	FNICK,	FNICK,	FNICK,	/* 0x58 - 0x5F */
	FNICK,	FNICK,	FNICK,	FNICK,	FNICK,	FNICK,	FNICK,	FNICK,	/* 0x60 - 0x67 */
	FNICK,	FNICK,	FNICK,	FNICK,	FNICK,	FNICK,	FNICK,	FNICK,	/* 0x68 - 0x6F */
	FNICK,	FNICK,	FNICK,	FNICK,	FNICK,	FNICK,	FNICK,	FNICK,	/* 0x70 - 0x77 */
	FNICK,	FNICK,	FNICK,	FNICK,	FNICK,	FNICK,	0,	0,	/* 0x78 - 0x7F */
	0,	0,	0,	0,	0,	0,	0,	0,	/* 0x80 - 0x87 */
	0,	0,	0,	0,	0,	0,	0,	0,	/* 0x88 - 0x8F */
	0,	0,	0,	0,	0,	0,	0,	0,	/* 0x90 - 0x97 */
	0,	0,	0,	0,	0,	0,	0,	0,	/* 0x98 - 0x9F */
	0,	0,	0,	0,	0,	0,	0,	0,	/* 0xA0 - 0xA7 */
	0,	0,	0,	0,	0,	0,	0,	0,	/* 0xA8 - 0xAF */
	0,	0,	0,	0,	0,	0,	0,	0,	/* 0xB0 - 0xB7 */
	0,	0,	0,	0,	0,	0,	0,	0,	/* 0xB8 - 0xBF */
	0,	0,	0,	0,	0,	0,	0,	0,	/* 0xC0 - 0xC7 */
	0,	0,	0,	0,	0,	0,	0,	0,	/* 0xC8 - 0xCF */
	0,	0,	0,	0,	0,	0,	0,	0,	/* 0xD0 - 0xD7 */
	0,	0,	0,	0,	0,	0,	0,	0,	/* 0xD8 - 0xDF */
	0,	0,	0,	0,	0,	0,	0,	0,	/* 0xE0 - 0xE7 */
	0,	0,	0,	0,	0,	0,	0,	0,	/* 0xE8 - 0xEF */
	0,	0,	0,	0,	0,	0,	0,	0,	/* 0xF0 - 0xF7 */
	0,	0,	0,	0,	0,	0,	0,	0,	/* 0xF8 - 0xFF */
};

#else /* MAIN_C */

#define NUM	0x01
#define NICK	0x02
#define FIRST	0x04
    
#define FNICK	(NICK|FIRST)
#define NNICK	(NICK|NUM)

extern	Uchar tolowertab[];
extern	Uchar attrtab[];

/*
 *  The compile-time default values
 */
extern	VarType BinaryDefault;

/*
 *  Long variable names for when we need them
 */
extern	aVarName VarName[SIZE_VARS];

#endif /* MAIN_C */

