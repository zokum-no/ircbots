#ifndef DEFINES_H
#define DEFINES_H

#define USERLIST_USER		0x0001
#define USERLIST_SHIT		0x0002
#define USERLIST_GREET		0x0004
#define USERLIST_PROTNICK	0x0008
#define USERLIST_KICKSAY	0x0010
#define USERLIST_SAYSAY		0x0020


#define IsUser(x)	((x)->type & USERLIST_USER)
#define IsShit(x)	((x)->type & USERLIST_SHIT)
#define IsGreet(x)	((x)->type & USERLIST_GREET)
#define IsProtNick(x)	((x)->type & USERLIST_PROTNICK)
#define IsKickSay(x)	((x)->type & USERLIST_KICKSAY)
#define IsSaySay(x)	((x)->type & USERLIST_SAYSAY)

/* Channel Crap */

#define COLLECT_TYPE  1
#define REJOIN_TYPE   2
/* Not used...
#define CYCLE_TYPE    4
*/

#define SOFTRESET 0
#define HARDRESET 1

#define CHFL_CHANOP     0x0001
#define CHFL_VOICE      0x0002
#define CHFL_BAN        0x0004  /* do we need this? */

#define MODE_CHANOP     CHFL_CHANOP
#define MODE_VOICE      CHFL_VOICE
#define MODE_PRIVATE    0x0004
#define MODE_SECRET     0x0008
#define MODE_MODERATED  0x0010
#define MODE_TOPICLIMIT 0x0020
#define MODE_INVITEONLY 0x0040
#define MODE_NOPRIVMSGS 0x0080
#define MODE_KEY        0x0100
#define MODE_BAN        0x0200
#define MODE_LIMIT      0x0400
#define MODE_FLAGS      0x07ff

/* Misc Crap: */

#define EXTRA_CHAR(q)   q==1?"":"s"
#define RANDOM( min, max ) ((rand() % (int)(((max)+1) - (min))) + (min))
#define SPLIT 1

/* Log Crap: */

#define DCC_LOG_VAR             0
#define COMMANDS_LOG_VAR        1
#define FAILED_LOG_VAR          2
#define ERR_LOG_VAR             3
#define DANGER_LOG_VAR          4

/* Debug Crap: */

#define QUIET   0
#define ERROR   1
#define NOTICE  2

/* DCC Crap: */

#define DCC_CHAT        ((unsigned) 0x0001)
#define DCC_FILEOFFER   ((unsigned) 0x0002)
#define DCC_FILEREAD    ((unsigned) 0x0003)
#define DCC_TALK        ((unsigned) 0x0004)
#define DCC_SUMMON      ((unsigned) 0x0005)
#define DCC_RAW_LISTEN  ((unsigned) 0x0006)
#define DCC_RAW         ((unsigned) 0x0007)
#define DCC_TYPES       ((unsigned) 0x000f)

#define DCC_WAIT        ((unsigned) 0x0010)
#define DCC_ACTIVE      ((unsigned) 0x0020)
#define DCC_OFFER       ((unsigned) 0x0040)
#define DCC_DELETE      ((unsigned) 0x0080)
#define DCC_TWOCLIENTS  ((unsigned) 0x0100)
#define DCC_STATES      ((unsigned) 0xfff0)

/* Type of Variable: */

#define INT_VAR		0x01
#define STR_VAR		0x02
#define TOG_VAR		0x04
#define GLOBAL_VAR	0x08
/* #define INTSTR_VAR	0x10 */

#define IsIntType(x)	((x)->type & INT_VAR)
#define IsStrType(x)	((x)->type & STR_VAR)
#define IsTogType(x)	((x)->type & TOG_VAR)
#define IsGlobalType(x)	((x)->type & GLOBAL_VAR)

/* For Variables: */

#define SETBT_VAR 0
#define SETAUB_VAR 1
#define SETBKL_VAR 2
#define SETCKL_VAR 3
#define SETMDL_VAR 4
#define SETMBL_VAR 5
#define SETMKL_VAR 6
#define SETMPL_VAR 7
#define SETFL_VAR       8
#define SETFPL_VAR      9
#define SETNCL_VAR      10
#define SETIKT_VAR      11
#define SETMAL_VAR      12
#define SETBANMODES_VAR 13
#define SETOPMODES_VAR  14
#define TOGRK_VAR       15
#define TOGTOP_VAR      16
#define TOGBK_VAR       17
#define TOGCK_VAR       18
#define TOGAOP_VAR      19
#define TOGSHIT_VAR     20
#define TOGPUB_VAR      21
#define TOGPROT_VAR     22
#define TOGAS_VAR       23
#define TOGABK_VAR      24
#define TOGIK_VAR       25
#define TOGSD_VAR       26
#define TOGSO_VAR       27
#define TOGENFM_VAR     28
#define TOGMASS_VAR     29
#define SETENFM_VAR     30
#define TOGCC_VAR       31
#define TOGKS_VAR	32
#define TOGSS_VAR	33
#define TOGWL_VAR	34
#define TOGIC_VAR	35
#define TOGDCC_VAR	36
#define TOGINV_VAR	37
#define TOGGREET_VAR	38

#endif
