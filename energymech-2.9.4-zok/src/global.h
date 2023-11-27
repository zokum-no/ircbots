/*

    EnergyMech, IRC bot software
    Parts Copyright (c) 1997-2001 proton, 2002-2004 emech-dev

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

char VERSION[]		= "2.9.4";
char SRCDATE[]		= "May 28th, 2004";
char BOTCLASS[]		= "EnergyMech";
char BOTLOGIN[]		= "emech";

char NULLSTR[]		= "<NULL>";

char ERR_CHAN[]		= TEXT_GLH_NOTONCHAN;
char ERR_FILEOPEN[]	= TEXT_GLH_CANTOPEN;
char ERR_INIT[]		= TEXT_GLH_INITWARNING;
char ERR_NICK[]		= TEXT_GLH_INVALIDNICK;
char ERR_NOCHANNELS[]	= TEXT_GLH_NOACTIVECHANS;
char ERR_NOSAVE[]	= TEXT_GLH_CANTSAVELISTS;
char ERR_NOTOPPED[]	= TEXT_GLH_IMNOTOPPED;

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
extern	char ERR_INIT[];
extern	char ERR_NICK[];
extern	char ERR_NOCHANNELS[];
extern	char ERR_NOSAVE[];
extern	char ERR_NOTOPPED[];

extern	char SHELLMASK[];
extern	char PUMASK[];

#endif /* MAIN_C */

#ifdef MULTI
BEG aBot	*botlist;
BEG aBot	*current;
BEG aBot	*backup;
#else /* MULTI */
BEG aBot	singlebot;
BEG aBot	*current;
#endif /* MULTI */

BEG char	executable[FILENAMELEN];
BEG char	configfile[FILENAMELEN];

BEG aWillWont	will;				/* features enable/disable */
BEG time_t	uptime;
BEG int		sigmaster;
BEG int		shell_cmd,need_dcc;
BEG struct	stat mechbin;
BEG int		do_exec;			/* call mech_exec on mechexit */
BEG int		do_fork;
BEG int		nummatch;			/* for match() */
BEG int		makecore;
BEG int		respawn;

BEG aKillSock	*killsocks;

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
{ /* TR */              INT_VAR,                0,      NULL,   0,      1500    },
{ /* BANMODES */	INT_VAR+GLOBAL_VAR,	4,	NULL,	1,	10	},
{ /* OPMODES */		INT_VAR+GLOBAL_VAR,	4,	NULL,	1,	10	},
{ /* CTIMEOUT */        INT_VAR+GLOBAL_VAR,     30,     NULL,   10,     300     },
{ /* CDELAY */          INT_VAR+GLOBAL_VAR,     2,      NULL,   0,      60      },
{ /* RK */		TOG_VAR,		0,	NULL,	0,	0	},
{ /* TOP */		TOG_VAR,		0,	NULL,	0,	0	},
{ /* CK */		TOG_VAR,		0,	NULL,	0,	0	},
{ /* AOP */		TOG_VAR,		1,	NULL,	0,	0	},
{ /* SHIT */		TOG_VAR,		1,	NULL,	0,	0	},
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
#ifdef DYNCHANLIMIT
{ /* DYNAMIC */		TOG_VAR,		0,	NULL,	0,	0	},
#endif /* DYNCHANLIMIT */
{			0,			0,	NULL,	0,	0	}
};

const aVarName VarName[SIZE_VARS] =
{
{ "BT",		TEXT_GLH_VAR_BT		},
{ "AUB",	TEXT_GLH_VAR_AUB	},
{ "CKL",	TEXT_GLH_VAR_CKL	},
{ "MDL",	TEXT_GLH_VAR_MDL	},
{ "MBL",	TEXT_GLH_VAR_MBL	},
{ "MKL",	TEXT_GLH_VAR_MKL	},
{ "MPL",	TEXT_GLH_VAR_MPL	},
{ "FL",		TEXT_GLH_VAR_FL		},
{ "FPL",	TEXT_GLH_VAR_FPL	},
{ "NCL",	TEXT_GLH_VAR_NCL	},
{ "IKT",	TEXT_GLH_VAR_IKT	},
{ "MAL",	TEXT_GLH_VAR_MAL	},
{ "TR",         TEXT_GLH_VAR_TR         },
{ "BANMODES",	TEXT_GLH_VAR_BANMODES	},
{ "OPMODES",	TEXT_GLH_VAR_OPMODES	},
{ "CTIMEOUT",   TEXT_GLH_VAR_CTIMEOUT   },
{ "CDELAY",     TEXT_GLH_VAR_CDELAY     },
{ "RK",		TEXT_GLH_VAR_RK		},
{ "TOP",	TEXT_GLH_VAR_TOP	},
{ "CK",		TEXT_GLH_VAR_CK		},
{ "AOP",	TEXT_GLH_VAR_AOP	},
{ "SHIT",	TEXT_GLH_VAR_SHIT	},
{ "PUB",	TEXT_GLH_VAR_PUB	},
{ "PROT",	TEXT_GLH_VAR_PROT	},
{ "AS",		TEXT_GLH_VAR_AS		},
{ "ABK",	TEXT_GLH_VAR_ABK	},
{ "IK",		TEXT_GLH_VAR_IK		},
{ "SD",		TEXT_GLH_VAR_SD		},
{ "SO",		TEXT_GLH_VAR_SO		},
{ "ENFM",	TEXT_GLH_VAR_ENFMTOG	},
{ "MASS",	TEXT_GLH_VAR_MASS	},
{ "ENFM",	TEXT_GLH_VAR_ENFMODE	},
{ "CC",		TEXT_GLH_VAR_CC		},
{ "KS",		TEXT_GLH_VAR_KS		},
{ "DCC",	TEXT_GLH_VAR_DCC	},
{ "CTL",	TEXT_GLH_VAR_CTL	},
{ "SPY",	TEXT_GLH_VAR_SPY	},
{ "IWM",	TEXT_GLH_VAR_IWM	},
{ "CLOAK",	TEXT_GLH_VAR_CLOAK	},
{ "RV",		TEXT_GLH_VAR_RV		},
{ "RF",		TEXT_GLH_VAR_RF		},
{ "AAWAY",	TEXT_GLH_VAR_AAWAY	},
{ "NOIDLE",	TEXT_GLH_VAR_NOIDLE	},
{ "CHANLOG",	TEXT_GLH_VAR_CHANLOG	},
{ "AVOICE",	TEXT_GLH_VAR_AVOICE	},
#ifdef DYNCHANLIMIT
{ "DYNAMIC",	TEXT_GLH_VAR_DYNAMIC	},
#endif /* DYNCHANLIMIT */
{ NULL,		NULL			}
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
