#ifndef CONFIG_H
#define CONFIG_H

#define CFGFILE "HackBot.Set"

#define BOTDATE "May 31, 1994"
#define VERSION "0.90 Alpha 13"

#define DEFAULT_OWNERUH "*!*behrens*@*iastate.edu"
#define DEFAULT_SERVER  "irc.iastate.edu"
#define DEFAULT_PORT    6667
#define DEFAULT_IRCNAME "Blow me, okay?"
#define DEFAULT_LOGIN   "behrensc"
#define DEFAULT_CMDCHAR '-'


#define BOTDIR "/home/cbehrens/HackBot/"
#define FILEUPLOADDIR   ""
#define FILEDOWNLOADDIR ""

#define DEFAULT_DCC_IDLETIMEOUT  1200
#define DEFAULT_DCC_WAITTIMEOUT  90
#define DEFAULT_DCC_SEND_ALLOWED 0
#define DEFAULT_DCC_RECV_ALLOWED 0
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
#define FLOOD_IGNORESEC   30
#define FLOOD_INTERVAL    15
#define FLOOD_RATE        10


#define OWNERLEVEL     100
#define ASSTLEVEL       90
#define MAXPROTLEVEL     4
#define MAXSHITLEVEL     3
#define ENFORCED_MODES "stinklmp"

#define DCCLOGFILE "dcc.log"
#define ERRFILE    "err.log"
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




