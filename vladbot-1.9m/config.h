#ifndef CONFIG_H
#define CONFIG_H
/*
#define IRCSERVER "svbs01.bs.win.tue.nl "
#define IRCSERVER "violet.sci.kun.nl"
#define IRCSERVER "nic.funet.fi"
#define IRCSERVER "irc.funet.fi"
#define IRCSERVER "svbs01.bs.win.tue.nl"
*/
/*
 * Define default server. There is no serverlist of any form
 * at this moment, so you're  stuck to this one. You can,
 * however, change the server at the commandline (-s).
 */
#define IRCSERVER "ircserver.et.tudelft.nl"
#define IRCPORT	  "6667"

/*
 * Define here stuff like standard nickname, ircname etc.
 * nick (-n) and channel (-c) can be changed at commandline
 */
#define IRCNAME  "NederServ"
#define USERNAME "NederServ"
#define IRCGECOS "/msg NederServ !help"
#define CHANNEL  "#Dutch"
#define VERSION  "1.9m"

/*
 * Files from which information should be read/written to
 */
#define USERFILE 	"user.list"
#define SHITFILE 	"shit.list"
#define PROTFILE 	"prot.list"
#define DCCLOGFILE 	"dcc.log"
#define ERRFILE    	"err.log"
#define HELPFILE	"help.bot"
#define SERVERFILE	"servers"

/* Don't forget to put /'s after directories! */
#define FILEROOTDIR	"/tmp/irc/files/"
/* More indexfiles can be defined by adding ".type" to the indexfile,
 * ie. CONTENTSFILE = /tmp/irc/INDEX
 * INDEX.gif can be read by /msg'ing files gif
 */
#define CONTENTSFILE	"/tmp/irc/INDEX"
#define FILEUPLOADDIR	"/tmp/irc/incoming/"

/*
 * Define here how long VladBot should wait for data before doing other stuff
 * (read: nothing :)
 */
#define WAIT_SEC	40	/* Yes! 40 secs! and even that is to little! */
#define WAIT_MSEC	0

/*
 * Number of seconds idle time before dcc-connections are closed
 * 20 minutes should be 'nuff for idle dcc's. 90 secs is plenty for
 * responding
 */
#define DCC_IDLETIMEOUT	1200
#define DCC_WAITTIMEOUT 90
#define DCC_MAXFILESIZE 5000000 	/* 5 megs */

/*
 * Undefine this if you do not want vladbot to automatically
 * start a dcc connection when required
 */
#define AUTO_DCC

/*
 * Define this is you want noisy debugging info
 */
#define DBUG		

/*
 * Standard stuff... don't change
 */
#define MAXLEN		255
#define WAYTOBIG        1024
#define BIG_BUFFER	1024
#define MAXNICKLEN	10

#define null(type) (type) 0L
#define US       1
#define THEM     2

#define SEPERATORS	".,;:\n "
#define FAIL	0
#define SUCCESS 1

#ifndef NULL
#define NULL(type) (type)0;
#endif
#ifndef TRUE
#define TRUE     1
#define FALSE    0
#endif

#endif /* CONFIG_H */
