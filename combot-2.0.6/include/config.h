#ifndef CONFIG_H
#define CONFIG_H

#define BOTDIR          "./"
#define CFGFILE		BOTDIR "Bot.Set"
#define HELPFILE        BOTDIR "Bot.Help"
#define LEVELFILE       BOTDIR "Bot.Levels"
#define PIDFILE		BOTDIR "Bot.pid"
#define MSGFILE		BOTDIR "Bot.msg"
#define LOGDIR		BOTDIR "logs/"
#define FILEUPLOADDIR   BOTDIR "upload/"
#define FILEDOWNLOADDIR BOTDIR "download/"

/*
#define DOUGH_MALLOC
#define DOUGH_MALLOC_LINES
#define DEBUG_MALLOC_ANAL
*/

#undef USE_CCC /* #undef this if there is no CCC Site */

#undef HAVE_GETDOMAINNAME  /* #undef this if your system doesn't have this */
#undef NEED_STRSTR         /* #define this if your system is screwed */
 
/* Please note...the SITE must be an IP# */
#define CCC_SITE	"129.186.3.20"
#define CCC_PORT	9585
/* This pertains to CCC Version 1.00:
   Note: The bot will try the second CCC if the first goes down,
	but realize if the second one has an older userlist/shitlist,
	the bot will use these
   Also, future versions of CCC will include the main CCC talking to
	the backup CCC, making sure it gets updated
*/
#define CCC_SITE2	"198.70.38.5"
#define CCC_PORT2	9474
/* * * * * * * * * * * * * */

#define LINKTIME 120

#undef HPUX /* Define this for HP_UX Machines */

#define DEFAULT_DCC_SEND_ALLOWED 0
#define DEFAULT_DCC_RECV_ALLOWED 0

#define BOTDATE "Oct  5, 1996"
#define VERSION "2.06"

#define DEFAULT_OWNERUH "*!*cbehrens@*slavery.com"
#define DEFAULT_SERVER  "fucking.fuck.i.don't.exist"
#define DEFAULT_PORT    6667
#define DEFAULT_IRCNAME "Blow me, okay?"
#define DEFAULT_LOGIN   "cbehrens"
#define DEFAULT_CMDCHAR '-'

#define DEFAULT_DCC_IDLETIMEOUT  1200
#define DEFAULT_DCC_WAITTIMEOUT  90
#define DEFAULT_DCC_MAXFILESIZE  0000    /* in kb's */
#define AUTO_DCC

#define PINGINTERVAL            40
#define PINGSENDINTERVAL        210
#define RESETINTERVAL           90

#define MAXBOTS         10
#define MAXSERVERS      15
#define MAXCHANNELS     10


#define DBUG

#define SUCCESS 1
#define MEMORY_LIFETIME 3600
#define AUTH_TIMEOUT	 300   /* 5 minutes */
#define FLOOD_IGNORESEC   30
#define FLOOD_INTERVAL    15
#define FLOOD_RATE        10
#define FLOOD_RESET	  10
#define FLOOD_LINES       7

#define OWNERLEVEL     100
#define ASSTLEVEL       90
#define MAXPROTLEVEL     4
#define MAXSHITLEVEL     3
#define ENFORCED_MODES "stinklmp"

#define WAIT_SEC        40      /* 40 secs. Could be much bigger */
#define WAIT_MSEC       0       /* Leave this zero.. */

#define MAXNICKLEN 10
#define MAXLEN 255
#define HUGE  1024
#define BIG_BUFFER 1024
#define TRUE  1
#define FALSE 0
#define FAIL  0

#define RANDOM(min, max) ((rand() % (int)(((max)+1) - (min))) + (min))

#endif

