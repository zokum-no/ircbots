#ifndef CONFIG_H
#define CONFIG_H

/*
 * Define here the defaults. They will be used if they're not specified
 * in the configfile
 */
#define IRCNAME  	"ChRoniC"		/* Nick */
#define USERNAME 	"chronic"		/* Login */
#define IRCGECOS 	"I'm smokin da chronic"	/* Shown in /who */

/************************************************************************
  Incubus's mod definitions

	FLOOD_PROT_CLONE	- 0-disabled 1-kick 2-bankick 3-bankick for 5 minutes
	FLOOD_PROT_NICK	- 0-disabled 1-kick 2-bankick 3-bankick for 5 minutes
	FLOOD_PROT_PUBLIC	- 0-disabled 1-kick 2-bankick 3-bankick for 5 minutes
	FLOOD_PROT_CTCP	- 0-disabled 1-kick 2-bankick 3-bankick for 5 minutes
	FLOOD_PROT_OP		- 0-disabled 1-kick 2-bankick 3-bankick for 5 minutes
	FLOOD_PROT_AWP		- 0-disabled 1-kick 2-bankick 3-bankick for 5 minutes
	BOOTLAMENICKS		- Bankick lame nicks ("mode", "irc", and "[")
	OLD_USERLEVELS		- If defined, level 200 is invalid
	USE_PIDFILE       - If defined, the bot will use a file to store the
                       current PID. The bot won't start if a process with
                       a matching PID is already running. (Ripped from
                       eggdrops)
	PID_FILE				- This is the filename of the PID file.
	USE_IRCD_MATCH		- When defined, the wildcard matching routine from
                       IRCD is used instead of the old routine. The new
                       routine doesn't handle "/", "[", and "]" differently
                       than any other character (except "*" and "?").

*************************************************************************/
#define  FLOOD_PROT_CLONE  1
#define  FLOOD_PROT_NICK   1
#define  FLOOD_PROT_PUBLIC 1
#define  FLOOD_PROT_CTCP   1
#define  FLOOD_PROT_OP     3
#define  FLOOD_PROT_AWP    3
#undef   BOOTLAMENICKS
#undef   OLD_USERLEVELS
#define  USE_PIDFILE       /* Define this if you want to use the new
                              anti-clone code*/
#define  PID_FILE          "incubot.pid"
#define	USE_IRCD_MATCH    /* #undef this if you want to use the old
                              wildcard match routine */
/************************************************************************
  badcrc's mod definitions

  BOTNET                - Undefine this if you don't want botnet functions
  BOTNET_CHAR		- Prefix command character for BOTNet commands
  BOTNET_CHECK_INTERVAL - Seconds before checking for botnet connections
  FLOOD_PUBLIC_RATE	- Max public lines allowed during INTERVAL
  FLOOD_PUBLIC_INTERVAL	- Interval in seconds to check for RATE before kick
  FLOOD_NICK_RATE	- Max nick changes allowed during INTERVAL
  FLOOD_NICK_INTERVAL	- Interval in seconds to check for RATE before kick
  FLOOD_CTCP_RATE	- Max CTCPs allowed during INTERVAL
  FLOOD_CTCP_INTERVAL	- Interval in seconds to check for RATE before kick
  MAIL_EVENTS           - Time the events file is mailed.
  MAX_CHANNEL_BANS	- Maximum bans for a channel
  MAX_KICKS		- Maximum number of kicks at once
  MAX_KICK_INTERVAL	- Seconds to wait before sending another MAX_KICKS
  MAX_SEEN              - Max number of seen records to keep at one time
  OTHER_BOT_LEVEL       - Level reserved for other bots
  *** REPLY values: 0-no one 1-users in list 2-everyone
  REPLY_CTCP_INTERVAL	- Max seconds to elapse before replying to another CTCP
  REPLY_FINGER          - Who to send finger replys to.
  REPLY_PING            - Who to send ping replys to.
  REPLY_VERSION         - Who to send version replys to.
  SPYLOG                - 1-log spy info to spy.log  0-don't log
*************************************************************************/
#undef BOTNET
#define BOTNET_CHAR	'@'
#define BOTNET_CHECK_INTERVAL	180
#define FLOOD_PUBLIC_RATE	4  /* 4 public text lines allowed */
#define FLOOD_PUBLIC_INTERVAL	2  /* in 2 seconds */
#define FLOOD_NICK_RATE		4  /* 4 nick changes allowed */
#define FLOOD_NICK_INTERVAL	8  /* in 8 seconds */
#define FLOOD_CTCP_RATE		3  /* 3 CTCPs allowed */
#define FLOOD_CTCP_INTERVAL	6  /* in 6 seconds */
#define MAIL_EVENTS		"05:00:" /* 5am */
#define MAX_CHANNEL_BANS	20
#define MAX_KICKS		4  /* 4 kicks */
#define MAX_KICK_INTERVAL	4  /* in 4 seconds */
#define MAX_SEEN		300
#define OTHER_BOT_LEVEL		105
#define REPLY_CTCP_INTERVAL	2
#define REPLY_PING		1
#define REPLY_VERSION		1
#define REPLY_FINGER		1
#define SPYLOG			0

#define MAX_REPLYPING		2
#define MAX_REPLYVERSION	2
#define MAX_REPLYFINGER		2

/* define BackedUp if you have a backup bot setup */
/*#define BackedUp */
/* define BackUp if this bot is a backup of another one */
/*#define BackUp*/

#ifdef BackUp
#define BACKUPBOTMASK   "blah"
#define BACKUPPASSWD "blah"
#endif
#ifdef BackedUp
#define BACKUPBOTNICK   "blah"
#define BACKUPBOTPASSWD "blah"
#endif
#define SECURE		0
#define TALK		1

#define MAX_SECURE		4
#define	MAXTALK			4

/* Password failure threshold before locking */
#define FAILURETHRESHOLD	5
#define LOCKPASS		"^@!P&*$%@#$%^"

/* Default filenames for logging, access etc ----------------------------
   USER/SHIT/PROTFILE	These files contain accesslevels and will be used 
			if no other files are specified in the configfile
   DCCLOGFILE		DCC logging (up/downloading) will be written to 
			this file
   ERRFILE		File for errorloging
   FILEROOTDIR		Place where downloadable files are located. This 
			is a directory and should end with '/'
   CONTENTSFILE		Place and name of the INDEX file. "Subfiles" can 
			be created using a .sub suffix, i.e. INDEX.gifs
   FILEUPLOADDIR	Place where uploaded files should be placed. End
			with a '/'
   NOTEFILE		File in which notes should be stored
*/
#define LIST50				"50.list"
#define LIST75				"75.list"
#define LIST100			"100.list"
#define LIST115			"115.list"
#define LIST125			"125.list"
#define LIST150			"150.list"
#define MSG50				"50.msgs"
#define MSG75				"75.msgs"
#define MSG100				"100.msgs"
#define MSG115				"115.msgs"
#define MSG125				"125.msgs"
#define MSG150				"150.msgs"
#define USERFILE			"user.list"
#define EXCLUDEFILE		"exclude.list"
#define SHITFILE			"shit.list"
#define PROTFILE			"prot.list"
#define NICKFILE			"nick.list"
#define SHITFILE2			"shit.list2"
#define EXPIREFILE		"expire.list"
#define MAILTOFILE		"useradd.notify"
#define PASSWDFILE		"passwd.list"
#define ACCESSFILE		"access.list"
#define FPFILE				"fp.list"
#define DCCLOGFILE		"dcc.log"
#define ERRFILE			"err.log"
#define DONELOG         "done.log"
#define JOINLOG         "join.log"
#define SYSTEMLOG       "system.log"
#define EVENTLOG			"event.list"
#define LOCKLOG			"lock.log"
#define HELPFILE			"help.bot"
#define FILEROOTDIR		"/tmp/irc/files/" 
#define CONTENTSFILE		"/tmp/irc/INDEX"
#define FILEUPLOADDIR	"/tmp/irc/incoming/"
#define NOTEFILE			"note.bot"

/* Default DCC settings ---------------------------------------------
   DCC_IDELTIMEOUT	Time in secs before idle connection should be
			closed
   DCC_WAITTIMEOUT	Time in secs before waiting connection should
			be closed
   DCC_MAXFILESIZE	Max. uploadable filesize (to prevent ppl from
			sending huge files) in 1024 kb blocks
   AUTO_DCC		If defined, the bot will automatically start
			a dcc connection when needed (if not, the 
			user is asked to start one)
   AUTO_OPLVL		Minimum userlevel required for auto-op
*/
#define DCC_IDLETIMEOUT	360
#define DCC_WAITTIMEOUT 180
#define DCC_MAXFILESIZE 5000 	/* 5 megs */
#define AUTO_DCC
#define AUTO_OPLVL	50	

/* The command prefix character -------------------------------------
   The prefixchar is only needed for public commands
*/
#define PREFIX_CHAR	'!'

/* Name of the default configurationfile ----------------------------
*/
#define CFGFILE		"./infile.cfg"

/* Select() - stuff ------------------------------------------------- 
   Number of seconds before VladBot should stop waiting for input 
   and re-check servers, channels, dcc's etc.
*/
#define WAIT_SEC	40	/* 40 secs. Could be much bigger */
#define WAIT_MSEC	0	/* Leave this zero.. */

/* Ping-defines -----------------------------------------------------
   PINGINTERVAL		The time (in secs) for the server to respond
			to a ping. If the server doesn't reply within
			this limit, the bot will reconnect.
   PINGSENDINTERVAL	Timeinterval between pings sent by the bot.
			Make this value large if you trust the server.
   RESETINTERVAL	Time between bot-resets. A bot reset is 
			re-joining all channels (when kicked etc) and
			a nickname change to the original nick
*/
#define	PINGINTERVAL		40
#define	PINGSENDINTERVAL	210
#define RESETINTERVAL		90

/* Some maxima ------------------------------------------------------
   MAXBOTS		The maximum number of bots that should run in
			one process (More bots take more resources!)
   MAXSERVER		Maximum number of servers per bot
*/
#define	MAXBOTS		15
#define	MAXSERVER	10

/* session-stuff ----------------------------------------------------
   Defaults for session control. None are configurable in the config-
   file
   FLOOD_RATE
   FLOOD_INTERVAL	Number of lines received in an interval
   			needed to mark a user flooding.
   			There is no avarge measured to determine if
			a user is flooding, so 6/1 is different from
			12/2 (With 12/2 a user may send 10 lines the
			first second and 1 the next without being
			marked flooding. With 6/1 (s)he will be
			marked flooding!
   FLOOD_TALKAGAIN	How long a flooding user will be ignored
   SESSION_TIMEOUT	How long a session may be idle before it's
   			deleted
   LISTSIZE		Maximum number of files in the file queue.
   			The filequeue is not  circular. If 120 files
			were in the queue of which 100 have been sent
			and 20 new files will be added, the queue 
			will overflow and some files will discarded.
 */

#define FLOOD_RATE	5		/* five lines 	*/
#define	FLOOD_INTERVAL	1		/* in one sec 	*/
#define FLOOD_TALKAGAIN	180		/* 90 secs 	*/
#define SESSION_TIMEOUT	300		/* 5 mins 	*/
#define LISTSIZE	128		/* 128 files.. plenty! 		*/

/* Misc stuff -------------------------------------------------------
 * MIN_USERLEVEL_FOR_OP         Minimum userlevel for someone to be
 *                       able to get ops (and keep them). otehrwise the
 *		         user gets deopped
 * MIN_USERLEVEL_FOR_BANS       Minimum userlevel for someone to be
 *                        able to do a ban. otherwise the ban is removed.
 * MAX_CHANNEL_BANS  Max channel bans before clearing when someone
 *                   who is shitted joins the channel.
 */

#define MIN_USERLEVEL_FOR_OPS 50
#define MIN_USERLEVEL_FOR_BANS 50

/* Debugging -------------------------------------------------------- 
   undefine this is you don't want debugging. Debugging can be turned 
   of at runtime, but when undefined, the binary will be a little 
   smaller. Remember that when sending a bugreport, a DBUG-log is
   appreciated
 */
#undef DBUG		

/* Standard stuff ---------------------------------------------------
   don't change anything below here
*/
#define VERSION		"IncuBot v1.03 beta 3 by Incubus"

#define MAXLEN		255
#define WAYTOBIG        1024
#define BIG_BUFFER	1024
#define MAXNICKLEN	10	
#define NICKLEN		9	

#define null(type) (type) 0L

#define FAIL	0
#define SUCCESS 1

#ifndef TRUE
#define TRUE     1
#define FALSE    0
#endif

#endif /* CONFIG_H */
