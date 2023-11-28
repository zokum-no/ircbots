#ifndef CONFIG_H
#define CONFIG_H

#define VERSION "1.0"

#define SETTINGSFILE "StelBot.Set"

#define DCC_MAXFILESIZE 2000000   /*  2 Megs...files larger than this will
                                     be refused */
/*
#define KINIT
*/

/* define this if you need the bot to run kinit...
   this is if your system uses the kerberos ticket
   system
*/

#define IRCPORT "6667"
#define OWNERLEVEL  100
#define ASSTLEVEL   90
#define MASSLEVEL   95
#define PROTLEVEL   60
#define MAXPROTLEVEL 5
#define MAXSHITLEVEL 5
#define validenforced_modes "inmlkspt"


#define WAIT_SEC	40	/* Yes! 40 secs! and even that is to little! */
#define WAIT_MSEC	0

/*
 * Number of seconds idle time before dcc-connections are closed
 * 20 minutes should be 'nuff for idle dcc's. 90 secs is plenty for
 * responding
 */
#define DCC_IDLETIMEOUT 1200
#define DCC_WAITTIMEOUT 90
#define AUTO_DCC

/*
 * Define this is you want noisy debugging info
 
#define DBUG		

*/

#undef EMAIL

/*
 * Standard stuff... don't change
 */
#define MAXLEN		255
#define FNLEN           150
#define WAYTOBIG    1024
#define BIG_BUFFER	1024
#define MAXNICKLEN	10

#define null(type) (type) 0L
#define US       1
#define THEM     2

#define SEPERATORS  ".,;\n "
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


