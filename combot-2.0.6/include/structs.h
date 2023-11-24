#ifndef STRUCTS_H
#define STRUCTS_H

#include <time.h>
#include <sys/time.h>
#include <sys/types.h>

typedef struct irc_server
{
	char *name;
	char *link;
	unsigned short int status;
	int num;
	struct irc_server *next;
} aLink;

#define SIZE_LOGS 6

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

typedef struct VarShit
{
        char name[20];
        char longname[100];
        int  type;
        int  value;
        char *strvalue;
        int  min;
        int  max;
} VarStruct;

#define SIZE_VARS     100

typedef VarStruct VarType[SIZE_VARS];

typedef struct UserListStruct
{
        char  *userhost;
	int   type;
        int   access;
        int   aop;
        int   prot;
        char  *channel;
        char  *password;
	char  *whoadd;
	char  *reason;
	long  time;
	long  expire;
        struct UserListStruct *prev;
        struct UserListStruct *next;
} aUser;
/*
typedef struct ShitListStruct
{
        char  *userhost;
        int   access;
        char  *channel;
        char  *shitby;
        char  *reason;
        long  time;
        long  expire;
        struct ShitListStruct *prev;
        struct ShitListStruct *next;
} aShit;
*/
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
        unsigned int mode;
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

typedef struct botstruct
{
        int             created;

        char            nick[MAXNICKLEN];
        char            realnick[MAXNICKLEN];
        char            login[MAXNICKLEN];
        char            ircname[MAXLEN];

        int             server_sock;
        char            *kickfile;

        char            cmdchar;
        int             restrict;
        int             filesendallowed;
        int             filerecvallowed;
        char            *uploaddir;
        char            *downloaddir;

        int             num_of_servers;
        int             current_server;

        struct
        {
                char name[MAXLEN];
                char realname[MAXLEN];
                int     port;
        } serverlist[MAXSERVERS];
        aChannel       *Channel_list;
        aChannel       *OldChannel_list;
        aChannel       *Current_chan;
	aDCC		*Client_list;

        aList           *StealList;
        aList           *SpyMsgList;
        aTime           *OffendersList;
        aTime           *KickList;

        aFlood          *Floods;
	aMemory		*MemList;

	aLink		*TheLinks;
	aLink		*TempLinks;

        long            uptime;

        long            lastreset;      /* last time bot was reset */
        long            lastping;       /* to check activity server */
        int             server_ok;
} aBot;

#endif /* STRUCTS_H */
