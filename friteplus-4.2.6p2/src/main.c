/*
 *
 * (c) 1995 OffSpring (cracker@cati.CSUFresno.EDU)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#ifdef AIX
#	include <sys/select.h>
#endif /* AIX */

#include "channel.h"
#include "config.h"
#include "autoconf.h"
#include "debug.h"
#include "log.h"
#include "note.h"
#include "parse.h"
#include "session.h"
#include "userlist.h"
#include "frite_bot.h"
#include "bonk.h"
#include "note.h"
#include "publics.h"

extern	long	uptime;			/* time when bot was started   */
extern	char	*configfile;
extern	int	rehash;
ListStruct *KickSays, *KickSayChan;	/* kick say -pb */
ListStruct	*StealList;		/* Channel Steal -pb */
ListStruct	*IgnoreList;		/* ignore -pb */
ListStruct	*InformList;		/* inform -pb */
ListStruct	*SpyMsgList;		/* spymsg -pb */
char lastcmds[10][100];			/* Used for Last10 -pb */
char lasttop[10][100];			/* last topics */
char lasttime[10][100];			/* last topics */
int	malevel = 90;			/* Channel Steal -pb */
int	fork();
int kslevel = KICKSAYLVL; 			/* kicksay level */
int	atoi(char *);
int	reload,reloadl;
void	readcfg();
void	readlevelfiles();

void	sig_hup();
void	sig_cntl_c();
/*
void	sig_bus();
void	sig_pipe();
 */
void	sig_segv();	

void	sig_hup()
{
	globallog(ERRFILE, "REHASHING: Hangup (sighup) received");
	rehash = TRUE;
	signal(SIGHUP, sig_hup);
}

void	sig_term()
{
	quit_all_bots(NULL, "Received SIGTERM - Memory Usage to High - Rebooting");
	globallog(ERRFILE, "Memory Usage To High");
	dump_notelist();
	sleep(1);
	exit(0);
}

void	sig_int()
{
	quit_all_bots(NULL, "Received SIGINT - Control-C!");
	globallog(ERRFILE, "SIGNING OFF: Control-C (sigint) received");
	dump_notelist();
	exit(0);
}
/*
void    sig_bus()
{
	globallog(ERRFILE, "IGNORING: received Sigbus!");
	signal(SIGBUS, sig_bus);
}
void    sig_pipe()
{
	globallog(ERRFILE, "IGNORING: received Broken Pipe!");
	signal(SIGPIPE, sig_pipe);
}
*/
void	sig_segv()
{
	quit_all_bots(NULL, "Received SIGSEGV - CALL A DOCTOR !!!!!!!!!!");
	globallog(ERRFILE, "SIGNING OFF: Segmentation violation (sigsegv) received");
	dump_notelist();
	abort();
}

void	bot_rehash()
{
	quit_all_bots(NULL, "\002Resetting All files.. BRB I Hope\002");
	readcfg();
	readlevelfiles();
	start_all_bots();

	rehash = FALSE;
}
 
int	start_bots()
{
    	struct 	timeval		timer;
    	fd_set	rd, wd;
	read_notelist();
        KickSays = init_list();
        KickSayChan = init_list();
	bot_init();
    	uptime = time(NULL);
	rehash = TRUE;
    	while( 1 )
    	{   
		if(rehash)
		{
			bot_rehash();
		}
		else
		{
			/* first, try to reconnect any dead sockets */
			try_reconnect();
			send_pings(); 		/* ping servers for activity 	*/
			reset_botstate(); 	/* reset nickname and channels 	*/
		}
		timer.tv_sec = WAIT_SEC;

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


void	main(int argc, char *argv[])
{
    	char  	*arg;
    	char  	*myname;
	int	do_fork = TRUE;

    	myname = argv[0];

	printf( "=--------------------------------------------------------=\n");
    	printf( "= fRitE+ %5s (c) 1995 by Pyber                   =\n", VERSION   );
	printf( "= For info e-mail: pyber@alias.undernet.org              =\n" );
	printf( "=--------------------------------------------------------=\n");
	printf( "= If you are reporting a bug or error, please include as =\n");
	printf( "= much information as possible. (A compressed core and   =\n");
	printf( "= error.log would be nice.                               =\n");
	printf( "=--------------------------------------------------------=\n");

    	while( argc > 1 && argv[1][0] == '-' )
    	{
        	argc--;    
        	arg = *++argv;
        	switch( arg[1] )
        	{
            	case 'h':
                     	printf("usage: %s [switches [args]]\n", myname);
                     	printf("-h               shows this help\n");
			printf("-b               run fRitE in the foregrround\n");
			printf("-c file          Read configfile 'file'\n");
#ifdef DBUG
			printf("-d [0|1|2]	 Set debuglevel.\n");
			printf("                   0 = debugging off\n");
			printf("                   1 = show errors\n");
			printf("                   2 = show channel msgs\n");
#endif
			exit(0);
			break;
		case 'b':
			do_fork = FALSE;
			break;
		case 'c':
			++argv;
			if(!*argv)
			{
				printf("No configfile specified\n");
				exit(0);
			}
			configfile = *argv;
			argc--;
			printf("Using cfg %s\n", configfile);
			break; 
		case 'd':
#ifdef DBUG
			++argv;
			argc--;
			if(*argv && set_debuglvl(atoi(*argv)))
				printf("Debuglevel set to %d\n", atoi(*argv));
			else
			{
				printf("Invalid debugvalue!\n");
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
	signal(SIGTERM, sig_term);
/*	signal(SIGBUS, sig_bus);
	signal(SIGPIPE, sig_pipe); */
	signal(SIGSEGV, sig_segv);

	init_bonk(BONKFILE);

	if(do_fork)
		if(!fork())
		{
			printf("\nRunning %s in background\n", myname);
    			start_bots();
		}
		else
			exit(0);
	else
		start_bots();
}

