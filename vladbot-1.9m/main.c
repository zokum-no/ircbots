/*
 * VladBot  - servicebot for IRC.
 *
 * (c) 1993 VladDrac (irvdwijk@cs.vu.nl)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>

#include "config.h"
#include "userlist.h"
#include "channel.h"
#include "parse.h"

char    current_nick[MAXNICKLEN];
char	*ircserver,			/* obsolete! */
	*portnumber;

USERLVL_list	**opperlist;
USERLVL_list	**shitlist;
USERLVL_list	**protlist;

long	uptime;			/* time when bot was started   */

void	sig_hup();
void	sig_cntl_c();
void	sig_segv();	

void	sig_hup()

{
	sendquit( "Received SIGHUP - Hangup" );
	botlog(ERRFILE, "SIGNING OFF: Hangup (sighup) received");
	exit(0);
}

void	sig_int()

{
	sendquit( "Received SIGINT - Control-C!" );
	botlog(ERRFILE, "SIGNING OFF: Control-C (sigint) received");
	exit(0);
}

void	sig_segv()

{
	sendquit("Received SIGSEGV - core dumped");
	botlog(ERRFILE, "SIGNING OFF: Segmentation violation (sigsegv) received");
	exit(0);
}
 
void	readlevelfiles()

{
	opperlist = init_levellist();
	shitlist = init_levellist();
	protlist = init_levellist();
	readuserdatabase( USERFILE, opperlist );
	readuserdatabase( SHITFILE, shitlist );
	readuserdatabase( PROTFILE, protlist );
}

int	start_a_bot( name , channel )
char    *name, 
        *channel;  

{
    	struct 	timeval		timer;
    	fd_set	rd, wd;


    	strcpy( current_nick, name );

	/* set uptime */
    	uptime = time(NULL);
	readlevelfiles();
	read_serverlist(ircserver, atoi(portnumber));

	startbot(current_nick, IRCGECOS, USERNAME, channel);

    	while( 1 )
    	{   
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
			printf("SELECT: timeout\n");
			break;
#endif
		case -1:
#ifdef DBUG
			printf("SELECT: error\n");
#endif
			break;
		default:
			parse_server_input( &rd );
			parse_dcc_input( &rd );
			/* Only join channels we might have been kicked of */
	       		reset_channels( SOFTRESET );
			break;
		}
    	}
}


int	main( argc, argv )
int     argc;
char    *argv[];

{
    	char  	*nickname;
    	char  	*channel;
    	char  	*arg;
    	char  	*myname;
	int	do_fork = FALSE;

    	myname = argv[0];

    	printf( "%s %s (c) 1993 VladDrac\n", myname, VERSION );
    	printf( "For more info: e-mail irvdwijk@cs.vu.nl\n" );

	ircserver = IRCSERVER;
	portnumber = IRCPORT;
	nickname = IRCNAME;
	channel = CHANNEL;

    	while( argc > 1 && argv[1][0] == '-' )
    	{
        	argc--;    
        	arg = *++argv;
        	switch( arg[1] )
        	{
            	case 'h':
                     	printf( "-------------- HELP --------------\n" );
                     	printf( "usage: %s [switches [args]]\n", myname );
                     	printf( "-h               shows this help\n" );
			printf( "-b               run %s in the background\n", 
				myname );
                     	printf( "-n nick          use <nick> as nick\n" );
                     	printf( "-s server        use <server> as server\n" );
                     	printf( "-p port          use port <port>\n" );
                     	printf( "-c channel       join channel <channel>\n\n" );
                     	exit( 0 );
		case 'b':
			do_fork = TRUE;
			break;
            	case 'n':
                     	if( argc > 1 )
                     	{
				nickname = *++argv;
                         	argc--;
                     	}
                     	break;
            	case 's':
                     	if( argc > 1 )
                     	{
                         	ircserver = *++argv;
                         	argc--;
                     	}
                     	break;  
            	case 'p':
                     	if( argc > 1 )
                     	{
                         	portnumber = *++argv;
                         	argc--;
                     	}
                     	break;  
            	case 'c':
                     	if( argc > 1 )
                     	{
				channel = *++argv;
                         	argc--;
                     	}
                     	break; 
            	default:  
                     	printf( "Unknown option %s\n", arg );   
                     	exit( 1 );
        	}         
    	}

	signal(SIGHUP, sig_hup);
	signal(SIGINT, sig_int);
	signal(SIGSEGV, sig_segv);

	if( do_fork )
		if( !fork() )
		{
			printf( "Running %s in background\n", myname );
    			start_a_bot( nickname, channel );
		}
		else
			exit( 0 );
	else
		start_a_bot( nickname, channel );
}

