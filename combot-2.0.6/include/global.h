#ifndef GLOBAL_H
#define GLOBAL_H

#include "config.h"

#ifdef DOUGH_MALLOC
#ifdef DOUGH_MALLOC_LINES
#define MyFree(__x) DoughFree(__x, __LINE__, __FILE__)
#define MyMalloc(__x) DoughMalloc(__x, __LINE__, __FILE__)
#define MyRealloc(__x, __y) DoughRealloc(__x, __y, __LINE__, __FILE__)
#else
#define MyFree(__x) DoughFree(__x)
#define MyMalloc(__x) DoughMalloc(__x)
#define MyRealloc(__x, __y) DoughRealloc(__x, __y)
#endif
#endif

#ifdef MAIN_C
#  define BEG 
#else
#  define BEG extern 
#endif /* MAIN_C */

#ifdef HPUX
#  define SELECT int
#else
#  define SELECT fd_set
#endif
#ifdef linux
#  include <linux/time.h>  /* needed for FD_SET */
#endif

#ifdef NEED_BCOPY
#  define bcopy(_x, _y, _z) memcpy((_y), (_x), (_z))
#  define bzero(_x, _y) memset((_x), 0, (_y))
#endif

#include "defines.h"
#include "structs.h"

BEG char	owneruserhost[MAXLEN];
BEG char	USERFILE[MAXLEN], configfile[MAXLEN];
BEG long	uptime;
BEG int		mypid, from_shell, in_dcc;
BEG aUser	*Userlist;
BEG aList	*Bots;           /* All bots connected */
BEG aTime	*Auths;          /* All people authed */
BEG aBot	thebots[MAXBOTS];
BEG aBot	*current;
BEG int		number_of_bots;
BEG int		idletimeout;
BEG int		waittimeout;
BEG int		maxuploadsize;
BEG int		dbuglevel;
BEG int		need_dcc;
BEG int		ccc_sock, which_ccc, ccc_port;
BEG char	mylocalhost[80], mylocalhostname[80];
BEG aUser	*CurrentUser;
BEG aUser	*CurrentShit;
BEG char	currentnick[15];
BEG time_t      lastrejoin;

#ifdef MAIN_C

LogType Logs =
{

{ "DCC", NULL },
{ "COMMANDS", NULL },
{ "FAILED", NULL },
{ "ERRORS", NULL },
{ "DANGER", NULL },
{ "", NULL }

};

VarType DefaultVars =
{

{ "BT", "Which bans to unban when ban-limit is reached",
	INT_VAR, 0, NULL, 0, 999 },
{ "AUB", "Erases old bans automatically, this sets how old they have to be",
	INT_VAR, 0, NULL, 0, 999 },
{ "BKL", "Beep kick level (How many beeps are allowed)",
	INT_VAR, 0, NULL, 1, 999 },
{ "CKL", "CAPS kick level (How many lines of CAPS are allowed)",
	INT_VAR, 0, NULL, 1, 999 },
{ "MDL", "Mass deop level (How many -o's count as a massdeop)",
	INT_VAR, 20, NULL, 3, 20 },
{ "MBL", "Mass ban level (How many +b's count as a massban)",
	INT_VAR, 20, NULL, 3, 20 },
{ "MKL", "Mass kick level (How many kicks count as a masskick)",
	INT_VAR, 20, NULL, 3, 20 },
{ "MPL", "Mass protection level (What to do with massmoders)",
	INT_VAR, 0, NULL, 0, 3 },
{ "FL", "Flood level (How many lines count as a flood)",
	INT_VAR, 6, NULL, 1, 20 },
{ "FPL", "Flood protection flood (What to do with flooders)",
	INT_VAR, 0, NULL, 0, 2 },
{ "NCL", "Nick change level (How many nick changes allowed)",
	INT_VAR, 4, NULL, 3, 20 },
{ "IKT", "Idle-kick time (How long users can be idle)",
	INT_VAR, 20, NULL, 2, 999 },
{ "MAL", "Mass action level (Max userlevel that mass functions won't affect)",
	INT_VAR, 0, NULL, 0, 99 },
{ "BANMODES", "Number of MODE -b that can be done at a time",
	INT_VAR, 4, NULL, 1, 4 },
{ "OPMODES", "Number of MODE -o that can be done at a time",
	INT_VAR, 3, NULL, 1, 4 },
{ "RK", "Toggle for revenge kicking", 
	TOG_VAR, 0, NULL, 0, 0 },
{ "TOP", "Toggle for topic enforcement",
	TOG_VAR, 0, NULL, 0, 0 },
{ "BK", "Toggle for beep kicking",
	TOG_VAR, 0, NULL, 0, 0 },
{ "CK", "Toggle for CAPS kicking",
	TOG_VAR, 0, NULL, 0, 0 },
{ "AOP", "Toggle for auto-opping",
	TOG_VAR, 0, NULL, 0, 0 },
{ "SHIT", "Toggle for shit-kicking",
	TOG_VAR, 0, NULL, 0, 0 },
{ "PUB", "Toggle for being able to use public commands",
	TOG_VAR, 0, NULL, 0, 0 },
{ "PROT", "Toggle for protection",
	TOG_VAR, 0, NULL, 0, 0 },
{ "AS", "Toggle for auto-shitlisting",
	TOG_VAR, 0, NULL, 0, 0 },
{ "ABK", "Toggle for kicking of banned users rejoining after splits",
	TOG_VAR, 0, NULL, 0, 0 },
{ "IK", "Toggle for the kicking of those who are idle",
	TOG_VAR, 0, NULL, 0, 0 },
{ "SD", "Toggle for deopping those who get opped by a server",
	TOG_VAR, 0, NULL, 0, 0 },
{ "SO", "Toggle for letting users only get opped",
	TOG_VAR, 0, NULL, 0, 0 },
{ "ENFM", "Toggle for mode enforcement",
	TOG_VAR, 0, NULL, 0, 0 },
{ "MASS", "Toggle for checking of massmodes",
	TOG_VAR, 0, NULL, 0, 0 },
{ "ENFM", "The modes to enforce",
	STR_VAR, 0, NULL, 0, 0 },
{ "CC", "Toggle for the necessity of using the command char.",
	TOG_VAR+GLOBAL_VAR, 1, NULL, 0, 0 },
{ "KS", "Toggle for the kicking of saying banned words",
	TOG_VAR, 0, NULL, 0, 0 },
{ "SS", "Toggle for the replying to of certain words",
	TOG_VAR, 0, NULL, 0, 0 },
{ "WL", "Toggle for watching of split servers",
	TOG_VAR+GLOBAL_VAR, 0, NULL, 0, 0 },
{ "IC", "Toggle for ignoring ctcp stuphs",
	TOG_VAR+GLOBAL_VAR, 0, NULL, 0, 0 },
{ "DCC", "Toggle for allowing non-users dcc stuphs",
	TOG_VAR+GLOBAL_VAR, 0, NULL, 0, 0 },
{ "INV", "Toggle for the bot going umode +i on connect",
	TOG_VAR+GLOBAL_VAR, 0, NULL, 0, 0 },
{ "GREET", "Toggle for the bot greeting people",
	TOG_VAR, 0, NULL, 0, 0 },
{ "", "", 0, 0, NULL, 0, 0 }

};

#else
extern VarType DefaultVars;
extern LogType Logs;
#endif /* MAIN_C */

#endif /* GLOBAL_H */

