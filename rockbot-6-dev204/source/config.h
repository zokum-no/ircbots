#ifndef CONFIG_H
#define CONFIG_H
#include <time.h>
#include <stdlib.h>
#include "setup.h"


/* Define this to hide the bot as a pirch client 
#define VERSION "PIRCH:MS Windows/WIN95:Beta Version 0.76: 96.04.05 c191452"
*/
#define VERSION "RockBot 6 (Dev version) [build 204]"

/* Some basic data file names */
#define WELCOMEFILE  "welcome.dat"
#define USERHOSTFILE "userhost.dat"
#define HELPFILE     "help.dat"
#define FILELISTFILE "files.dat"
#define REPLYFILE    "replies.dat"

/* Files larger then this (in bytes) will be refused.. since rockbot doesn't accept
   dcc's this is kinda curious... maybe it'll be added some day.
*/
#define DCC_MAXFILESIZE 2000000

/* define this to use the kerbos ticket system.  This is most likely broken because
   it hasn't been tested ever in the existance of RockBot.  It might work.. someone
   let me know.  If you don't know what kerbos is, you can ignore this.

#define KINIT
*/

#define IRCPORT 6667
#define OWNERLEVEL  100
#define ASSTLEVEL   90
#define MASSLEVEL   95
#define PROTLEVEL   60
#define MAXPROTLEVEL 5
#define MAXSHITLEVEL 5
#define validenforced_modes "inmlkspt"
#define DEFAULTMODES "nt"
#define SPACER "       ----------        \n"
#define Reg1 register

/* Octal codes that are handy to have around.. mostly for reference */
#define BOLD \002          /* ^B  */
#define DCC  \001          /* ^A  */
#define NOCOLOR \017       /* ^O  */
#define INVERSE \026       /* ^V  */
#define UNDERLINE \037     /* ^_  */


#define WAIT_SEC	40	/* How long to wait for input before timeout */
#define WAIT_MSEC	0

/*
 * Number of seconds idle time before dcc-connections are closed
 * 20 minutes should be 'nuff for idle dcc's. 90 secs is plenty for
 * responding
 */
#define DCC_IDLETIMEOUT 2400
#define DCC_WAITTIMEOUT 90
#define AUTO_DCC

/* How often should i check if inactive but authorized people
 * are still online? (in seconds)  2 minutes is good i think. Too short
 * a value could cause lag.. too long is less secure.*/
#define AUTHCHECKTIME 120

/* * Define this is you want noisy debugging info */
 
#undef DBUG		
#define DBUG

/* Defining this makes the bot use the IP address the server tells it it has, instead of
   the one the system reports.  This can be very handy if you run from behind a firewall
   or in a situation where remote users cant get DCC's to work when the bot tells them
   the local (unreal) IP address.  
   This is the same as mIRC's "Lookup method: ()Normal ()Server" question.
   if your connecting to a network which masks ip's or hostname, and DCC doesnt work,
   try undefining this.
*/
#define USE_SERVER_HOST

/* Define this if you have an O:line (operator) on the server.  Use at your
   own risk!  currently the bot wont automaticaly use O: line powers for
   anything..  but beware if someone gets access to the bot @ level 99 
   they are effectively an ircop...
*/
#define OLINE

/*
 * Standard stuff... change only if you know what your doing..
 * usualy modifying these causes segfaults and things.
 */
#define MAXLEN		255
#define INBUFFSIZE      1024
#define FNLEN           150
#define WAYTOBIG 	1024
#define BIG_BUFFER	1024
#define MAXNICKLEN	35

/* Some defines to make code more readable */
#define null(type) (type) 0L
#define US       1
#define THEM     2
#define FAIL    0
#define SUCCESS 1

/* Used in some strtok functions as deliniators.. */
#define SEPERATORS  ".,;\n "

#ifndef NULL
#define NULL(type) (type)0;
#endif

/* These are handy to have around... */
#ifndef TRUE
#define TRUE     1
#define FALSE    0
#endif

#ifndef true
#define true     1
#define false    0
#endif

#define T        1
#define F        0

/* The debug output levels */
#define DBGNOTICE   10
#define DBGINFO     20
#define DBGWARNING  30
#define DBGERROR    40
#define DBGSEVERE   50


/* The userlevel defines... */
#define BotOwner      81
/*        BotOwner      81-100   Owns the bot.  This person can
                               make it send raw stuff to the server
                               execute arbitrary script, delete files
                               anything.. 
*/
#define BotCoOwner    61
/*        BotCoOwner    61-80    This person can make it join/part channels
                               send msgs .. do almost anything pre-programmed.
*/
#define ChanOwner     41
/*        ChanOwner     41-60    This person owns a channel, and 60 is the
                               highest command available on a chan userlist.
                               He can add permbans, add other chan users,
                               make the bot say thing sin his channel ownly, etc.
*/
#define ChanCoOwner   31
/*
        ChanCoOwner   31-40    This person cant make the bot talk in the channel,
                               or anything too raw, but can add bans/ops etc.
*/
#define ChanHelper    21
/*        ChanHelper    21-30    These people can add bans, set modes, add ops, etc
*/
#define ChanOp        11
/*        ChanOp        11-20    These people get auto-opped and can make the bot
                               do kicks n stuff but thats about it.
*/
#define Peon            1
/*        Peon          01-10    These people cant do anything or get ops, except
                               this access clears them to get info from the bot,
                               etc.  By default anyone who msgs the bot whos
                               not known, might be auto-added as level 1.
*/
#define All           0

#define EXTRA_CHAR(s)   s==1?"":"s"

/*
 * SOFTRESET means: only join "failed" channels
 * HARDRESET: delete all users and join all channels (i.e. after a kill)
 */
#define SOFTRESET 0
#define HARDRESET 1

typedef struct SettingsType
{
        char  *name;
        int   index;
        int   min;
        int   max;
        int   def;
        int   changelevel;
        char  *description;
        char  *pre;
        char  settings[6][120];
} SettingStruct;

typedef struct ListType
{
        char name[MAXLEN];
        struct ListType *next;
} ListStruct;

typedef struct FloodType
{
  char host[MAXLEN];
  int num;
  time_t thetime;
  struct FloodType *next;
} FloodStruct;

typedef struct MessagesType
{
  char from[MAXLEN];
  char text[MAXLEN];
  time_t time;
  int num;
  struct MessagesType *next;
} MessagesStruct;

typedef struct UserHostType
{
        char name[100];
        char channel[100];
        time_t time;
        time_t kicktime, bantime, deoptime, killtime;
        int bannum, deopnum, kicknum;
        int totmsg, msgnum;
        int totop, totban, totdeop, totkick, totkill;
        struct MessagesType *messages;
        struct UserHostType *next;
} UserHostStruct;

typedef struct HostListType
{
        char userhost[MAXLEN];
        int security;
        struct HostListType *next;
} HostListStruct;

/* These define various levels the "security"  variable
 * represents.. from authorization not required, to 
 * authorized as needed.
 */
#define SECURE 0
#define INSECURE 1
#define AUTHORIZED 1
#define UNAUTHORIZED 2

typedef struct UserType
{
        struct HostListType *userhosts;     /* *!*user@*host mask list */
        char handle[MAXLEN];            /* Whats this users name */
        char password[MAXLEN];          /* Users Password */
        char info[MAXLEN];              /* Stuff to say when they join */
        int  level;                     /* Access level or ShitLevel */
        int  isaop;                     /* Is Auto-Op'd */
        int  protlevel;                 /* Protect Level */
        int  security;                  /* Require password? */
        struct UserType *next;    /* next user */
} UserStruct;

typedef struct AuthType
{
        char userhost[MAXLEN];          /* Who EXACTLY was ID'd */
        time_t timestamp;               /* When were they last ID'd */
        struct AuthType *next;          /* next struct */
} *AuthStruct;

typedef struct ShitType
{
        char userhost[MAXLEN];          /* Nick!User@Host */
        char bannedby[MAXLEN];          /* who added them */
        char reason[MAXLEN];            /* Why are they shitlisted */
        int  level;                     /* ShitLevel */
        time_t timestamp;               /* when they were added */
        time_t expiretime;              /* When the shit expires */
        struct ShitType *next;    /* next user */
} ShitStruct;

typedef struct	BAN_struct
{
	char	banstring[MAXLEN];	/* banned nick!user@host */
	struct	BAN_struct *next;	/* Next ban */
} BAN_list;

typedef struct SERVER_struct
{
  char                name[MAXLEN];      /* Servername */
  char                password[MAXLEN];  /* Password */
  unsigned int        port;              /* Port */
  unsigned int        errors;        /* How many errors from this server */
  unsigned int        connects;      /* How many successfull connects */
  unsigned int        attempts;      /* How many attempts to connect */
  struct SERVER_struct *next;             /* Next server */
} SERVER_list;

#define NUMOFCHANSETTINGS 20

typedef	struct	CHAN_struct
{
  char	         name[MAXLEN];	 /* channel's name */
  char           datfile[MAXLEN];/* Full path to our data files */
  char           topic[MAXLEN];  /* default channel topic */
  char           getop[MAXLEN];  /* what I do to get ops */
  int	         active;	 /* are we in there? */
  unsigned int   PlusModes;      /* Modes to enforce enabled */
  unsigned int   MinModes;       /* Modes to enforce disabled */
  char           *Key;           /* Key to enforce if +k in PlusMode */
  char           *Limit;         /* Limit to enforce if +l in PlusMode */
  char           CurrentKey[30]; /* The key thats currently active*/
  unsigned int   Modes;      /* n t s p i l k m = modes */
  struct USER_struct  *users;         /* users currently in the channel */
  struct BAN_struct   *banned;        /* Active bans in the channel */
  struct UserType     *OpList;        /* This channels registered user masks */
  struct ShitType     *ShitList;      /* Perminant bans */
  int            Settings[NUMOFCHANSETTINGS];   /* Integer settings. Defined below */
  struct ListType *spylist;      /* Who's spying on this channel */
  struct RESPONDER_struct *Responder;  /* List of expect/send pairs */
  struct CHAN_struct *next;	 /* ptr to next channel */
} CHAN_list;

typedef struct RESPONDER_struct
{
   char    *name;   /* Some short identifier */
   char    *expect; /* The match() string to look for */
   char    *send;   /* The string to send. */
   struct RESPONDER_struct *next;
} RESPONDER_list;


extern CHAN_list *Globals;

/* These are all the settings the rockbot can do... */

#define LOGGING    0
#define AUTOOP     1
#define PROTECTION 2
#define SHITPROT   3
#define ENFM       4
#define IDLEKICK   5
#define CLONEKICK  6
#define FLOODKICK  7
#define RIDEPROT   8
#define STRICTOP   9
#define WM        10
#define PUBSEEN   11
#define TALKING   12
#define MASSPROT  13
#define KSLEVEL   14
#define MPLEVEL   15
#define PUBTOGGLE 16
#define CCREQ     17
#define PMLOG     18

#define MODE_CHANOP	CHFL_CHANOP
#define MODE_VOICE	CHFL_VOICE
#define MODE_PRIVATE	0x0004
#define MODE_SECRET	0x0008
#define MODE_MODERATED	0x0010
#define MODE_TOPICLIMIT	0x0020
#define MODE_INVITEONLY 0x0040
#define MODE_NOPRIVMSGS	0x0080
#define MODE_KEY	0x0100
#define MODE_BAN	0x0200	
#define MODE_LIMIT	0x0400	
#define MODE_FLAGS	0x07ff

/* These are for channel mode parsing */
#define NOMODES         0
#define CMODEn          128
#define CMODEt          64
#define CMODEs          32
#define CMODEp          16
#define CMODEi          8
#define CMODEl          4
#define CMODEk          2
#define CMODEm          1

/* These are for User mode parsing.. */
#define UMODEs          64
#define UMODEk          32
#define UMODEo          16
#define UMODEd          8
#define UMODEw          4
#define UMODEi          2
#define UMODEg          1

typedef struct QueueStruct
{
  int priority;
  char *buff;

  struct QueueStruct *Next;

} *QueueListType;

typedef struct	server_struct
{
	char	servername[MAXLEN];
	int	serverport;
} serv_struc;

typedef	struct	rockbot_struct
{
	char		nick[MAXNICKLEN];
	char		ircname[MAXLEN];
	char		login[MAXLEN];
	char		channel[MAXLEN];
        SERVER_list     *servers;
        SERVER_list     *CurrentServer;
	int		current;
	int		server_sock;
/*
 * Should the channel list be in here with all the
 * userlists and things hanging off of that? Will we
 * ever want to run 2 bots from one process? good question.. -Rubin
 * To add:
 * - Channellist
 * - DCC list
 */
}	RockBotStruct;	

extern QueueListType Queue;


typedef struct USER_struct
{
    char nick[MAXNICKLEN];
    char user[MAXLEN];
    char host[MAXLEN];
    unsigned int mode;
    time_t nctime, idletime, floodtime;
    int numnc, floodnum;
    struct USER_struct *next;
} USER_list;

#define CHFL_CHANOP 0x0001
#define CHFL_VOICE  0x0002
#define CHFL_BAN    0x0004

#endif
