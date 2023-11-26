#ifndef CONFIG_H
#define CONFIG_H

#undef HPUX 			   /* Define this for HP_UX Machines */
#undef HAVE_GETDOMAINNAME  /* #undef this if your system doesn't have this */
#undef NEED_STRSTR         /* #define this if your system is screwed */

#undef SERVER_MONITOR

/* Do you want the bot to oper itself on connect?  Define this stuff */
/*  If you don't want it to oper on connect you can use the OPER command
 *  to oper the bot.  A TRACE won't be initiated until the bot gets the
 *  381 "You are now an oper" numeric.  Disadvantages to not defining
 *  the OPER_ON_CON are: if the bot crashes/or is restarted and an oper
 *  that has access to oper the bot is not around you will loose connection
 *  statistics.  If you don't define it, you have a more secure server.
 *  You never know when someone will steal your bot and dissect it...
 *  Could be really disasterous if they can spoof...
 *
 *  I recommend not defining this.
 *  1.  When the bot connects and you have the mode var set correctly in
 *      the .set file it will still recieve some server notices and keep
 *      keep track of that data.  It won't beable to do TRACE searches
 *      for clones though... or kills/klines.
 */
 
#ifdef SERVER_MONITOR
#undef OPER_ON_CON
#ifdef OPER_ON_CON
#define OPER_NAME "something"
#define OPER_PASS "something"
#endif
#endif

#undef RANDQUOTE

/* No Longer Used name of the binary is determined from how it is executed */
/* for purposes of restarting the bot ;) */
/* #define BINARYNAME "./EnergyMech" */

#define BOTDIR          "./"
#define RANDDIR			"./randfiles/"
#define BACKUP			"./backup/"
#define CFGFILE			BOTDIR "EnergyMech.set"
#define HELPFILE        BOTDIR "EnergyMech.help"
#define LEVELFILE       BOTDIR "EnergyMech.levels"
#define PIDFILE			BOTDIR "EnergyMech.pid"
#define MSGFILE			BOTDIR "EnergyMech.msg"
#define LOGDIR			BOTDIR "logs/"
#define EVENTLOG		LOGDIR "events.log"
#define FILELISTNAME	BOTDIR "EnergyMech.files"
#define FILEUPLOADDIR   BOTDIR "dcc/incoming/"
#define FILEDOWNLOADDIR BOTDIR "dcc/"
#define CONTENTSFILE	BOTDIR "EnergyMech.files"

#define PLUGIN1 "./flash3"
#define PLUGIN2 "./flash5"
#define PLUGIN3 "./mflash"
#define PLUGIN4 "./tspoof"
#define PLUGIN5 ""

#define NEWS_FILE			BOTDIR  "news.e"
#define SITEZ_FILE			BOTDIR  "sites.e"
#define EIGHTBALL			RANDDIR "8ball.e"
#define PINGFILE			RANDDIR "randping.e"
#define NICKSFILE			RANDDIR "randnicks.e"
#define SIGNOFFSFILE		RANDDIR "randsignoff.e"
#define RANDKICKSFILE		RANDDIR "randkicks.e"
#define RANDTOPICSFILE		RANDDIR "randsay.e"
#define VERSIONFILE			RANDDIR "randversions.e"
#define AWAYFILE 			RANDDIR "randaway.e"
#define RANDSAYFILE 		RANDDIR "randsay.e"
#define RANDBEAVISFILE 		RANDDIR "randbeavis.e"
#define RANDYOMAMAFILE 		RANDDIR "randyomama.e"
#define RANDINSULTFILE 		RANDDIR "randinsult.e"
#define RANDPICKUPFILE 		RANDDIR "randpickup.e"
#define RANDBONGFILE 		RANDDIR "randbong.e"
#define RANDLASERFILE 		RANDDIR "randlaser.e"
#define RANDFLOODFILE 		RANDDIR "randflood.e"
#define RANDPCJOKEFILE		RANDDIR "randpcjoke.e"
#define RANDFAKELOG			RANDDIR "randfakelog.e"
#define RANDWORDFILE		RANDDIR "randword.e"

#ifdef RANDQUOTE
#define RANDUSERQUOTEFILE	RANDDIR "randuserquote.e"
#endif

/* for use with the adduser command */
#define DEFAULTLEVEL 	"50"
#define DEFAULTCHAN 	"*"
#define DEFAULTAOP 		"0"
#define DEFAULTPROT 	"0"

/* for use with the qshit command */
#define DEFAULTSHITCHAN 	"*"
#define DEFAULTSHITLEVEL 	"2"
#define DEFAULTSHITLENGTH 	"90" /* in days */

/* for the time command */
#define PHYSICAL_LOCATION "Polygon's Dystopia"

/* For Anonymous listing of user in the sites list. */
#undef ANON_SITEZ

#define EXVERSION "Running PhoEniX v2.35 by Vassago"
#define EXFINGER "hey, that tickles"

/* Phoenix Style */
#define AWAYFORM "AWAY :%s (Away since %s)"
/* BitchX Style */
/* 	#define AWAYFORM "AWAY :(%s) \002s\002CZ!\002tE\04500207.12.96" */
/* No Time (GENERIC) */
/*	#define AWAYFROM "AWAY :%s" */

/* Define this if you want the command char neccessary for all FTP commands */
#undef CCFTP

#define DBUG
#ifdef DBUG
#undef 	DBUGLOG
#endif

#define DEFAULT_DCC_SEND_ALLOWED 1
#define DEFAULT_DCC_RECV_ALLOWED 1
#define DCC_LEVEL 				 50
#define CMDCHAR_CTCP_LEVEL 		 50
#define DEFAULT_DCC_IDLETIMEOUT  600
#define DEFAULT_DCC_WAITTIMEOUT  90
#define DEFAULT_DCC_MAXFILESIZE  10240    /* in kb's */
#define AUTO_DCC

#define PINGINTERVAL            40
#define PINGSENDINTERVAL        210
#define RESETINTERVAL           90
#define RESETTIME				3		  /* Time of Day in 24 Hour Time to Reset Logs ect. */
#define LINKTIME 				120
#define NOIDLETIME				30		  /* Send PING/PRIVMSG every how many minutes? */

#ifndef SERVER_MONITOR
#define MAXBOTS         10
#else
#define MAXBOTS			1
#endif
#ifndef SERVER_MONITOR
#define MAXSERVERS      30
#else
#define	MAXSERVERS		1
#endif
#ifndef SERVER_MONITOR
#define MAXCHANNELS     10
#else
#define	MAXCHANNELS		1
#endif

#define SUCCESS 			1
#define MEMORY_LIFETIME 	300
#define SEEN_TIME 			7	   /* How Long in Days to keep track of a record? */
#define AUTH_TIMEOUT		300    /* 5 minutes */
#define FLOOD_IGNORESEC   	60
#define FLOOD_INTERVAL    	15
#define FLOOD_RATE        	10
#define FLOOD_RESET	  	  	10
#define FLOOD_LINES			4

#define FLOOD_TALKAGAIN	180		/* 90 secs 	*/
#define SESSION_TIMEOUT	300		/* 5 mins 	*/
#define LISTSIZE		128		/* 128 files.. plenty! 		*/

#define SUPERUSER     1000
#define OWNERLEVEL     100
#define ASSTLEVEL       99
#define MAXPROTLEVEL     4
#define MAXSHITLEVEL     3
#define ENFORCED_MODES "stinklmp"

#define WAIT_SEC        40      /* 40 secs. Could be much bigger */
#define WAIT_MSEC       0       /* Leave this zero.. */

#define MAXNICKLEN 		33		/* Thanks to DALnet */
#define MAXBOTNICKLEN 	10
#define MAXLEN 			1024
#define HUGE  			2048
#define BIG_BUFFER 		2048
#ifndef TRUE
#define TRUE  			1
#endif
#ifndef FALSE
#define FALSE 			0
#endif
#define FAIL  			0

#define BOTDATE "January 16, 1997"
#define VERSION "2.1.2"
#define REVISION "X-Zero"
#ifndef SERVER_MONITOR
#define CLASS "XV-XAOS19970116"
#else
#define CLASS "XV-XAOS19970116 (Monitor)"
#endif

#define DEFAULT_OWNERUH "*!*polygon@*rox.org"
#define DEFAULT_IRCNAME "I crush you"
#define DEFAULT_LOGIN   "polygon"
#define DEFAULT_CMDCHAR '-'

#define RANDOM(min, max) ((rand() % (int)(((max)+1) - (min))) + (min))

/* ccc doesn't even exist anymore.... */

#undef USE_CCC /* #undef this if there is no CCC Site */
 
/* * * * * * * * * * * * * */
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

#endif

