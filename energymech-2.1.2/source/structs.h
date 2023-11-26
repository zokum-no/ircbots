#ifndef STRUCTS_H
#define STRUCTS_H

#include <time.h>
#include <sys/time.h>
#include <sys/types.h>

struct	stat;

typedef struct irc_server
{
	char *name;
	char *link;
	unsigned short int status;
	int num;
	struct irc_server *next;
} aLink;

#define SIZE_LOGS 24

typedef struct LogStruct
{
        char name[20];
        char *value;
} LogType[SIZE_LOGS];

typedef struct DCC_struct
{
        unsigned flags;
        int     read;
        int     write;
        int     file;
        int     filesize;
        char    description[MAXLEN];
        char    user[MAXLEN];
        char    ip_addr[20];            /* remote in ircII */
        int     port;                   /* remport */
        long    bytes_read;
        long    bytes_sent;
        long    lasttime;               /* can be used for idletime */
        long    starttime;
        struct  DCC_struct      *next;
} aDCC;

typedef struct VarStructure
{
        char name[20];
        char longname[100];
        int  type;
        int  value;
        char *strvalue;
        int  min;
        int  max;
} VarStruct;

#define SIZE_VARS     113

typedef VarStruct VarType[SIZE_VARS];

typedef struct ListStruct
{
        char  *name;
        struct ListStruct *prev;   /* prev entry */
        struct ListStruct *next;   /* next entry */
} aList;

typedef struct TimeStruct
{
        char   *name;
        time_t time;
        int    num;
        struct TimeStruct *prev;
        struct TimeStruct *next;
} aTime;

typedef struct UserListStruct
{
	char* userhost;
	int type;
	int access;
	int aop;
	int prot;
	char* channel;
	char* password;
	char* whoadd;
	char* reason;
	long time;
	long expire;
	
	struct UserListStruct *prev;
	struct UserListStruct *next;
} aUser;

typedef struct FloodType
{
	char *hostname;
	time_t last;
	int num;
	struct FloodType *prev;
	struct FloodType *next;
} aFlood;

typedef struct MemoryStruct
{
        char    *user;              /* nick!user@host.domain                */
        long    last_received;      /* (info)  when did we receive last cmd */
        char    *nodcc_to;          /* "to" when there was no dcc           */
        char    *nodcc_rest;        /* "rest" when there was no dcc         */
        struct  MemoryStruct *prev; /* prev entry in list                   */
        struct  MemoryStruct *next; /* next entry in list                   */
} aMemory;

typedef struct BigList
{
        char    *user;
        char    *data1;
        char    *data2;
        time_t  time;
        long	num;
        struct  BigList *prev;
        struct  BigList *next;
} aBigList;

typedef struct BanStruct
{
        char *banstring;
        char *bannedby;
        time_t time;
        struct BanStruct *prev;
        struct BanStruct *next;
} aBan;

typedef struct ChanUserStruct
{
        char nick[MAXNICKLEN];
        char *user;
        char *host;
        char *server;
        unsigned int mode;
        unsigned int oper;
        int floodnum,
                bannum,
                deopnum,
                kicknum,
                nicknum,
                beepnum,
                capsnum;
        long int floodtime,
                 bantime,
                 deoptime,
                 kicktime,
                 nicktime,
                 beeptime,
                 capstime,
                 idletime;
        struct ChanUserStruct *prev;
        struct ChanUserStruct *next;
} aChanUser;

typedef struct ChannelStruct
{
        char *name;
        int active;
        aChanUser *users;
        aBan *banned;
        unsigned int mode;
        char *key;
        char limit[20];
        char *topic;
        char *kickedby;
        int  userlimit;
        int  flags;
        char *collectby;
        aList *SpyList;
        aTime *HostList;
        VarType Vars;
        struct ChannelStruct *next;
} aChannel;

typedef struct SeenStruct
{
        char	*nick;
        char	*userhost;
        char	*signoff;
        char	*kicker;
        time_t 	time;
        int    	selector;
        struct 	SeenStruct *prev;
        struct 	SeenStruct *next;
} aSeen;

#define IS_FLOODING	1
#define IS_OK		2

typedef struct	session_struct
{
	char	*user;			/* nick!user@host.domain 		*/
	long	last_received;		/* (info)  when did we receive last cmd */
	char	*nodcc_to;		/* "to" when there was no dcc		*/
	char	*nodcc_rest;		/* "rest" when there was no dcc		*/
	char	*cwd;			/* (dcc)   current working directory 	*/
	int	currentfile;		/* (dcc)   current file to send		*/
	int	maxfile;		/* (dcc)   total number of files	*/
	struct
	{
		char	*path;
		char	*name;
	} file_list[LISTSIZE];		/* (dcc)   list of files to send 	*/
	long	flood_start;		/* (flood) used for timing-stuff 	*/
	int	flood_cnt;		/* (flood) Number of lines received	*/	
	int	flooding;		/* (flood) Marked true if flooding	*/
	struct	session_struct	*next;	/* next session in list 		*/
} SESSION_list;	

SESSION_list	*find_session(char *user);
SESSION_list	*find_session2(char *user);
SESSION_list	*create_session(char *user);

typedef struct botstruct
{
        int             created;

        char            nick[MAXBOTNICKLEN];
        char            realnick[MAXBOTNICKLEN];
        char            login[MAXBOTNICKLEN];
        char            ircname[MAXLEN];
        char			mymodes[32];
        char			modes_to_send[32];
        
        aUser			*Userlist;
        char			USERFILE[MAXLEN];

        int             server_sock;
        char            *kickfile;
        char			*seenfile;
        char			*dfile;

        char            cmdchar;
        int             restrict;
        int             filesendallowed;
        int             filerecvallowed;
        char            *uploaddir;
        char            *downloaddir;
        char			*indexfile;

        int             num_of_servers;
        int             current_server;

        struct
        {
                char name[MAXLEN];
                char realname[MAXLEN];
                int port;
				int	active;
        } serverlist[MAXSERVERS];

        aChannel		*Channel_list;
        aChannel		*OldChannel_list;
        aChannel		*Current_chan;
		aDCC			*Client_list;
		SESSION_list	*session_list;

        aList           *StealList;
        aList           *SpyMsgList;
        aList			*StatMsgList;
        aTime           *OffendersList;
        aTime           *KickList;
        
        aBigList		*dList;
        
        aTime			*ParseList;

        aFlood          *Floods;
		aMemory			*MemList;

		aLink			*TheLinks;
		aLink			*TempLinks;
	
		aSeen			*SeenList;
		
		char 			*tempvar;		/* used for certain parse functions */

        long            uptime;
        long            lastreset;      /* last time bot was reset */
        long            lastping;       /* to check activity server */
        long 			active_serv;	/* For Updating channels */
        long			away_serv;		/* Away Timer */
        long			log_update;		/* For changing log files */
        long			staus_line;		/* For 5 Mins Status Line */
        long			anti_idle;		/* For Anti-Idle Toggle */
        int				am_Away;		/* Am I away? */
        int             server_ok;
        int				isIRCop;
        int				hasOLine;
		int				exitcounter;
		int				serv_lag;
		
		aList 			*monitorlist;

#ifdef SERVER_MONITOR

	char *kfile;
	char *kfile2;
	char *tfile;
	

	aBigList *KillList;
	aTime *KillList2;
	aTime *totalList;
        
	long day_timer;
	
	int total_klines;
	int today_klines;

	int total_kills;
	int today_kills;
	
	int total_connections;
	int today_connections;
	
	int total_exits;
	int today_exits;
	
	int total_rejects;
	int today_rejects;
	
	int total_invu;
	int today_invu;
	
	int total_conf;
	int today_conf;
	
	int total_ucon;
	int today_ucon;
	
	int total_links;
	int today_links;
	
	int total_flood;
	int today_flood;
	
	int total_ipmis;
	int today_ipmis;
	
	aTime *conList;
	aTime *rejectList;
	aTime *botList;
	
	int trace_tog;
	int trace_kill_tog;
	
#endif
} aBot;

#endif /* STRUCTS_H */
