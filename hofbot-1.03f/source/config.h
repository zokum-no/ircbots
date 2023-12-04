#ifndef CONFIG_H
#define CONFIG_H

#define BOTDATE "Febuary, 1996"
#define VERSION "v1.03F"


#define DEFAULT_OWNERUH "*!*jpoon@*.sunysb.edu"
#define DEFAULT_SERVER  "irc.apk.net"
#define DEFAULT_PORT    6667
#define DEFAULT_IRCNAME "http://www.ug.cs.sunysb.edu/~poonj/hofbot.html"
#define DEFAULT_LOGIN   "jpoon"
#define DEFAULT_CMDCHAR '@'


#define BOTDIR ""
#define FILEUPLOADDIR   "./upload/"
#define FILEDOWNLOADDIR "./incoming/"
#define FILELISTNAME    "HOFbot.filelist"

#define DEFAULT_DCC_IDLETIMEOUT  1200
#define DEFAULT_DCC_WAITTIMEOUT  90
#define DEFAULT_DCC_SEND_ALLOWED 1
#define DEFAULT_DCC_RECV_ALLOWED 0
#define DEFAULT_DCC_MAXFILESIZE  0000    /* in kb's */
#define AUTO_DCC 0

#define PINGINTERVAL            40
#define PINGSENDINTERVAL        210
#define RESETINTERVAL           90

#define MAXBOTS         4
#define MAXSERVERS      10
#define MAXCHANNELS     10


#undef DBUG

#define SUCCESS 1
#define MEMORY_LIFETIME 3600
#define FLOOD_IGNORESEC   120
#define FLOOD_INTERVAL    15
#define FLOOD_RATE        10


#define OWNERLEVEL     100
#define ASSTLEVEL       90
#define MAXGREETLEVEL   99
#define MAXPROTLEVEL     4
#define MAXSHITLEVEL     3
#define ENFORCED_MODES "stinklmp"

#define CFGFILE 	"hofbot.set"
#define HELPFILE 	"hofbot.help"
#define DCCLOGFILE 	"dcc.log"
#define ERRFILE  	"err.log"
#define LAMERLOGFILE 	"lamers.log"
#define MSGLOGFILE 	"msg.log"
#define COMMANDLOGFILE  "command.log"
#define NETSPLITFILE    "netsplit.log"
#define QUOTEFILE 	"quotes.bot"
#define TOPICFILE 	"topics.bot"
#define GREETFILE 	"greets.bot"
#define NICKFILE 	"nicks.bot"
#define NEWSFILE 	"news.bot"

#define WAIT_SEC        40      /* 40 secs. Could be much bigger */
#define WAIT_MSEC       0       /* Leave this zero.. */

#define MAXNICKLEN 	10
#define MAXLEN 		255
#define HUGE  		1024
#define BIG_BUFFER 	1024
#define TRUE  		1
#define FALSE 		0
#define FAIL  		0

#define RANDOM(min, max) ((rand() % (int)(((max)+1) - (min))) + (min))

#endif




