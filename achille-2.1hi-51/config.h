#ifndef CONFIG_H
#define CONFIG_H

/*
 * Define here the defaults. They will be used if they're not specified
 * in the configfile
 */
#define IRCNAME  	"NederServ"		/* Nick */
#define USERNAME 	"NederServ"		/* Login */
#define IRCGECOS 	"/msg NederServ !help"	/* Shown in /who */

/* Default filenames for logging, access etc ----------------------------
   USER/SHIT/PROTFILE	These files contain accesslevels and will be used 
			if no other files are specified in the configfile
   BOTFILE              Contains bots and their channels.
   RELFILE              Idem but for relations with the bot.
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
#define USERFILE 	"user.list"
#define SHITFILE 	"shit.list"
#define PROTFILE 	"prot.list"
#define RELFILE         "rel.list"
#define BOTFILE         "bot.list"
#define LOCUTEURFILE    "locuteurs.list"
#define DCCLOGFILE 	"dcc.log"
#define ERRFILE    	"err.log"
#define HELPFILE	"help.bot"
#define STIMFILE        "Citations.stim"
#define REPFILE         "Citations.reps"
#define FILEROOTDIR	"/tmp/irc/files/" 
#define CONTENTSFILE	"/tmp/irc/INDEX"
#define FILEUPLOADDIR	"/tmp/irc/incoming/"
#define NOTEFILE	"note.bot"
#define LOGFILE         "bot.log"

/* Default DCC settings ---------------------------------------------
   DCC_IDLETIMEOUT	Time in secs before idle connection should be
			closed
   DCC_WAITTIMEOUT	Time in secs before waiting connection should
			be closed
   DCC_MAXFILESIZE	Max. uploadable filesize (to prevent ppl from
			sending huge files) in 1024 kb blocks
   AUTO_DCC		If defined, the bot will automatically start
			a dcc connection when needed (if not, the 
			user is asked to start one)
   AUTO_OPLVL		Minimum userlevel required for auto-op
   SYMPA_LVL            Minimum rellevel required for sympathy
   CONFIANCE_LVL        Minimum rellevel rellevel for confiance
   DEFAUT_LVL           Default rellevel, defines the bot's attitude toward 
                        unknown users
*/
#define DCC_IDLETIMEOUT	1200
#define DCC_WAITTIMEOUT 90
#define DCC_MAXFILESIZE 5000 	/* 5 megs */
#define AUTO_DCC
#define AUTO_OPLVL	70	
#define SYMPA_LVL       10
#define CONFIANCE_LVL   20
#define DEFAUT_LVL      5       /* Default rel level. Let's be
                                   optimistic... */

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
			(15)
   MAXSERVER		Maximum number of servers per bot
*/
#define	MAXBOTS		5
#define	MAXSERVER	10

/* session-stuff ----------------------------------------------------
   Defaults for session control. None are configurable in the config-
   file
   FLOOD_RATE
   FLOOD_INTERVAL	Number of lines received in an interval
   			needed to mark a user flooding.
   			There is no average measured to determine if
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

#define FLOOD_RATE	6		/* six lines 	*/
#define	FLOOD_INTERVAL	1		/* in one sec 	*/
#define FLOOD_TALKAGAIN	90		/* 90 secs 	*/
#define SESSION_TIMEOUT	600		/* 10 mins 	*/
#define LISTSIZE	128		/* 128 files.. plenty! 		*/

/* Misc stuff -------------------------------------------------------
   ONLY_OPS_FOR_REGISTRED	Only allow registred people to be 
                    	opped. If someone ops a non-registred user,
			deop him (her)
 */

#undef	ONLY_OPS_FOR_REGISTRED

/* Debugging -------------------------------------------------------- 
   undefine this is you don't want debugging. Debugging can be turned 
   off at runtime, but when undefined, the binary will be a little 
   smaller. Remember that when sending a bugreport, a DBUG-log is
   appreciated
 */
#define DBUG		


/* Logging ----------------------------------------------------------
   undefine this if you don't want logging.
   */
#define LOG

/* Customization ----------------------------------------------------
   Achille may have another nickname.
   */
#define SURNOM "Chichill"


/* Standard stuff ---------------------------------------------------
   don't change anything below here
*/
#define VERSION		"2.1hi-51"

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
