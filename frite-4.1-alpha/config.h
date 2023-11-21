#ifndef CONFIG_H
#define CONFIG_H

/*
 * fRitE v4.0 by OffSpring
 * Look in war.h too .. :)
 */
/* This is what your bot will respond to on publics &  comamnds..
 * If your bots name was 'Sewage', you might want to put just Sewage.
 * If it was 'DreamVoir' you might put 'DV'.
 */
#define PUBLIC_NICK "fRitE"
#define PUB_FILE "publics.bot"
/*
 * Define this if you want the bot to enforce modes +nt-pismlk
 */
#define ENFORCE_MODES
/*
 * Define here the defaults. They will be used if they're not specified
 * in the configfile
 */
#define IRCNAME  	"ORM"	                /* Nick */
#define USERNAME 	"orm"	 	        /* Login */
#define IRCGECOS 	"Who could ask for anything more"	/* Shown in /who */

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
 */
#define USERFILE 	"user.list"
#define SHITFILE 	"shit.list"
#define PROTFILE 	"prot.list"
#define DCCLOGFILE 	"dcc.log"
#define ERRFILE    	"err.log"
#define HELPFILE	"help.bot"
#define FILEROOTDIR	"/tmp/"
#define CONTENTSFILE	"/tmp/INDEX"
#define FILEUPLOADDIR   "/tmp/incoming/"
/* Misc fun files & directories ---------------------------------
   NOTEFILE		File in which notes should be stored
   BONKFILE		The file which the bonk items are in
   QUOTEFILE		The file with the discordian quotes
   LOGFILE		The files the channel logging goes to
   NEWSFILE		The file for the bots news
   BULLFILE		File with bulletins
   SONGDIR		Directory/ with songs etc..
   PICDIR		Directory/ with asci pics in it
   LIBFILE		File where list of ascii pics are located.
   LYRICFILE		This file is where list of bands is placed.
   SPLATFILE		File with Splatter Kick comments
   DANCEFILE		File where the bot gets its dance moves..
   REPLYFILE		File with Bot Replies
   INSULTFILE		File with insults ..
 */
#define NOTEFILE	"note.bot"
#define BONKFILE	"bonk.bot"
#define QUOTEFILE	"quote.bot"
#define LOGFILE		"/tmp/channel.log"
#define BULLFILE	"bulletin.bot"
#define NEWSFILE	"news.bot"
#define PICDIR		"pics/"
#define SONGDIR		"songs/"
#define LIBFILE		"ascii_lib.bot"
#define LYRICFILE	"lyrics.bot"
#define SPLATFILE	"splatk.bot"
#define DANCEFILE	"dance.bot"
#define REPLYFILE	"replies.bot"
#define INSULTFILE	"insults.bot"

/* Misc Settings by PB ----------------------------------------------
   LOG			Uhh Duh? Do you want channel logging?
   GREET	        Greet users when they join .. :)
   GREETLVL		Level for greeting. Keep it high or it gets
			annoying.
   KICKSAYLVL		<= level will be kicked when they say a kicksay.
			0 if no kicksay level
   NO_OPS		Deop if not on userlist.
   NET_OPS		Deop if its a net-split hack.
   PUBLICS		Do you want public & action responses? Make
			sure you defined PUBLIC_NICK!
   SING			Define this so the bot can sing
*/
#define LOG
#define GREET
#define GREETLVL 100
#define KICKSAYLVL 45
#define NO_OPS
#define NET_OPS
#define PUBLICS
#define SING

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
#define DCC_IDLETIMEOUT	1200
#define DCC_WAITTIMEOUT 90
#define DCC_MAXFILESIZE 5000 	/* 5 megs */
#define AUTO_DCC
#define AUTO_OPLVL	50	

/* The command prefix character -------------------------------------
   The prefixchar is only needed for public commands
*/
#define PREFIX_CHAR	'~'

/* Name of the default configurationfile ----------------------------
*/
#define CFGFILE		"infile.cfg"

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
#define	PINGSENDINTERVAL	180
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

#define FLOOD_RATE	6		/* six lines 	*/
#define	FLOOD_INTERVAL	1		/* in one sec 	*/
#define FLOOD_TALKAGAIN	90		/* 90 secs 	*/
#define SESSION_TIMEOUT	300		/* 5 mins 	*/
#define LISTSIZE	128		/* 128 files.. plenty!	*/

/* Debugging -------------------------------------------------------- 
   undefine this is you don't want debugging. Debugging can be turned 
   of at runtime, but when undefined, the binary will be a little 
   smaller. Remember that when sending a bugreport, a DBUG-log is
   appreciated
 */
#define DBUG		

/* Standard stuff ---------------------------------------------------
   don't change anything below here
*/
#define VERSION "4.1"

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

