#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#ifdef AIX
#       include <sys/select.h>
#endif /* AIX */

#include "log.h"
#include "channel.h"
#include "config.h"
#include "debug.h"
#include "parse.h"
#include "memory.h"
#include "userlist.h"
#include "hofbot.h"

char owneruserhost[MAXLEN];
long    uptime;             /* time when bot was started   */
extern  char    *configfile;
int     pid;

int    fork();
void   readlevelfiles();

void sig_hup();
void sig_cntl_c();
void sig_bus();
void dig_pipe();
void sig_segv();

void sig_hup()
{
	if( !fork() ) {
	  sleep(5);		/* wait for 5 secs */
	  globallog(ERRFILE, "SIGNING OFF: hangup (sighup) received");
	  start_bots();         /* restart a new bot */
 	}
	else {
	  kill_all_bots("ERROR1: Received SIGHUP - HANGUP!");
	  exit(0);
	}

}

void sig_int()
{
	if( !fork() ) {
	  sleep(5);
	  globallog(ERRFILE, "SIGNING OFF: Control-C (sigint) received");
	  start_bots();
 	}
	else
	{
	  kill_all_bots("ERROR2: Received SIGINT - Control-C!");
	  exit(0);
	}
}

void sig_bus()
{
	if( !fork() ) {
	  sleep(5);
  	  globallog(ERRFILE, "SIGNING OFF: Bus Error (sigbus) received");
	  start_bots();
 	}
	else {
   	  kill_all_bots("ERROR3: Received SIGBUS - Passengers dumped");
 	  exit( 0 );
	}
}

void sig_pipe()
{
	if( !fork() ) {
	  sleep(5);
       	  globallog(ERRFILE, "IGNORING: received Broken Pipe!");
	  start_bots();
 	}
        else {
   	  kill_all_bots("ERROR4: Received broken pipe!");
          exit( 0 );
	}
}

void sig_segv()
{
	if( !fork() ) {
	  sleep(5);
	  globallog(ERRFILE, "SIGNING OFF: Segmentation violation (sigsegv) received");
	  start_bots();
	}
	else {
   	  kill_all_bots("PANIC: Received SIGSEGV - Segmentation fault");
	  exit( 0 );
	}
}

int start_bots()
{
	struct  timeval         timer;
	fd_set  rd, wd;

	readcfgfile();
	start_all_bots();
	/* set uptime */
	uptime = getthetime();
	/* not really rehash, but read cfgfile etc. */
	read_lists_all();
	while (1)
	{
		try_reconnect();
		send_pings();           /* ping servers for activity    */
		reset_botstate();       /* reset nickname and channels  */

		timer.tv_sec = WAIT_SEC;
		timer.tv_usec = WAIT_MSEC;

		FD_ZERO( &rd );
		FD_ZERO( &wd );

		set_dcc_fds( &rd, &wd );
		set_server_fds( &rd, &wd );
		switch(select( NFDBITS, &rd, 0, 0, &timer ))
		{
		case 0:
#ifdef DBUG
			global_dbg(NOTICE, "SELECT: timeout");
			break;
#endif
		case -1:
#ifdef DBUG
			global_dbg(ERROR, "SELECT: error");
#endif
			break;
		default:
			parse_server_input( &rd );
			parse_dcc_input( &rd );
			break;
		}
	}
}


void main(int argc, char *argv[])
{
	char    *arg;
	char    *myname;
	int     do_fork = TRUE;

	myname = argv[0];

	printf( "HOFbotv2.0B (C) 1995-96 by HOF\n");
	printf( "For more info: e-mail jpoon@mathlab.sunysb.edu\n");

	while( argc > 1 && argv[1][0] == '-' )
	{
		argc--;
		arg = *++argv;
		switch( arg[1] )
		{
		case 'h':
			printf("usage: %s [switches [args]]\n", myname);
			printf("-h               shows this help\n");
			printf("-b               doesn't run %s in the background\n",
				myname);
			printf("-f file          Read configfile 'file'\n");
#ifdef DBUG
			printf("-d [0|1|2]    Set debuglevel.\n");
			printf("                   0 = debugging off\n");
			printf("                   1 = show errors\n");
			printf("                   2 = show detailed information\n");
#endif
			exit(0);
			break;
		case 'b':
			do_fork = FALSE;
			break;
		case 'f':
			++argv;
			if(!*argv)
			{
				printf("No configfile specified\n");
				exit(0);
			}
			configfile = *argv;
			argc--;
			printf("Using config file: %s\n", configfile);
			break;
		case 'd':
#ifdef DBUG
			++argv;
			argc--;
			if (*argv && set_debuglvl(atoi(*argv)))
				printf("Debuglevel set to %d\n", atoi(*argv));
			else
			{
				printf("Invalid debug value!\n");
				exit(0);
			}
#else
			printf("This version was not compiled with debugging enabled\n");
#endif
			break;
		default:
			printf("Unknown option %s\n", arg);
			exit(1);
			break;
		}
	}

	signal(SIGHUP, sig_hup);
	signal(SIGINT, sig_int);
	signal(SIGBUS, sig_bus);
	signal(SIGPIPE, sig_pipe);
	signal(SIGSEGV, sig_segv);
	if( do_fork )
	  {
		 printf( "Running %s in background\n", myname );
		 if( !fork() )
			{
		start_bots();
		printf("pid = %i\n", (int) getpid());
			}
		 else
			exit( 0 );
	  }
	else
	  start_bots();
}




