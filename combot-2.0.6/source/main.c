/*
 * ComBot 2.00 - a real fucked up bot
 *
 * (c) 1995 CoMSTuD (cbehrens@slavery.com)
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
#define MAIN_C

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#ifdef AIX
#       include <sys/select.h>
#endif /* AIX */
#include <stdlib.h>

#include "global.h"
#include "config.h"
#include "defines.h"
#include "structs.h"
#include "h.h"

void sig_hup(int);
void sig_cntl_c(int);
void sig_bus(int);
void dig_pipe(int);
void sig_segv(int);
void sig_alrm(int);

void sig_hup(int crap)
{
	FILE *in;
	char lin[HUGE];

	log_it(ERR_LOG_VAR, 1, "IGNORING: hangup (sighup) received");
        from_shell = 1;
        if ((in = fopen(MSGFILE, "rt")) != NULL)
        {
                while (freadln(in, lin))
                        on_msg(owneruserhost, current->nick, lin);
                fclose(in);
        }
        from_shell = 0;
        unlink(MSGFILE);
	signal(SIGHUP, sig_hup);	
}

void sig_alrm(int crap)
{
	if (ccc_sock != -1)
		close(ccc_sock);
	ccc_sock = -1;
        signal(SIGALRM, sig_alrm);
}

void sig_int(int crap)
{
	kill_all_bots("Received SIGINT - Control-C!");
	log_it(ERR_LOG_VAR, 1, "SIGNING OFF: Control-C (sigint) received");
	exit(0);
}

void sig_bus(int crap)
{
	kill_all_bots("Received SIGBUS - Passengers dumped");
	log_it(ERR_LOG_VAR, 1, "SIGNING OFF: Bus Error (sigbus) received");
	exit(0);
}

void sig_pipe(int crap)
{
	log_it(ERR_LOG_VAR, 1, "IGNORING: received Broken Pipe!");
#ifdef DBUG
                        global_dbg(NOTICE, "SIGPIPE received");
#endif
	if (!in_dcc)
	{
		start_all_bots();
		try_reconnect();
		send_pings();
		reset_botstate();
	}
	signal(SIGPIPE, sig_pipe);
}

void sig_segv(int crap)
{
	kill_all_bots("Received SIGSEGV - shit!  D'oh!");
	log_it(ERR_LOG_VAR, 1, "SIGNING OFF: Segmentation violation (sigsegv) received");
	exit(0);
}

int start_bots(void)
{
	struct  timeval         timer;
	fd_set rd, wd;
	FILE *out;
#ifdef USE_CCC
	time_t now, lastccccheck;
	int tempsock;
#endif	 

        mypid = (int) getpid();
        if ((out = fopen(PIDFILE, "wt")) != NULL)
        {
                fprintf(out, "%i\n", mypid);
                fclose(out);
        }
	/* don't even ask...it works tho */
	dup_defaultvars();
 	readcfgfile();
	Userlist = NULL;
	Auths = NULL;
	Bots = NULL;
	start_all_bots();
	uptime = getthetime();
#ifdef USE_CCC
	lastccccheck = getthetime();
	alarm(10);
	ccc_sock = connect_by_number(ccc_port, CCC_SITE);
	alarm(0);
	if (ccc_sock <= 0)
	{
		alarm(10);
		ccc_sock = connect_by_number(CCC_PORT2, CCC_SITE2);
		alarm(0);
		if (ccc_sock <= 0)
			ccc_sock = -1;
		else
		{
			which_ccc = 2;
			ccc_connected();
		}
	}
	else
	{
		which_ccc = 1;
		ccc_connected();
	}
#else
	ccc_sock = -1;
#endif
	if ((ccc_sock == -1) && !*USERFILE)
	{
		log_it(ERR_LOG_VAR,1,"No userlist file specified");
		printf("No userlist file specified\n");
		kill_all_bots(NULL);
		MyExit(0);
	}
	if (!read_lists_all() && (ccc_sock == -1))
	{
		log_it(ERR_LOG_VAR, 1, "No userlists were found to load");
		printf("Terminating: No userlists found.\n");
		kill_all_bots(NULL);
		exit(0);
	}
	while (1)
	{
		try_reconnect();
		send_pings();           /* ping servers for activity    */
		reset_botstate();       /* reset nickname and channels  */

		timer.tv_sec = WAIT_SEC;
		timer.tv_usec = WAIT_MSEC;

		FD_ZERO( &rd );
		FD_ZERO( &wd );
#ifdef USE_CCC
		if (ccc_sock != -1)
			FD_SET(ccc_sock, &rd);
#endif
		set_dcc_fds( &rd, &wd );
		set_server_fds( &rd, &wd );
		switch(select( NFDBITS, (SELECT *) &rd, 0, 0, &timer ))
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
#ifdef USE_CCC
			parse_ccc_input( &rd );
#endif
			break;
		}
#ifdef USE_CCC
		if (((ccc_sock==-1) || (which_ccc == 2)) &&
			((now = getthetime())-lastccccheck > 120))
		{
			tempsock = -1;
			lastccccheck = now;
			alarm(10);
			tempsock = connect_by_number(ccc_port, CCC_SITE);
			alarm(0);
			if (tempsock <= 0)
				tempsock = -1;
			if (tempsock != -1)
			{
				if (ccc_sock != -1)
					close(ccc_sock);
				which_ccc = 1;
				ccc_sock = tempsock;
				ccc_connected();
			}
			else if (ccc_sock == -1)
			{
				alarm(10);
				ccc_sock = connect_by_number(CCC_PORT2,
						CCC_SITE2);
				alarm(0);
				if (ccc_sock <= 0)
					ccc_sock = -1;
				else
				{
					which_ccc = 2;
					ccc_connected();
				}
			}
		}
#endif
		update_auths();
	}
}

int main(int argc, char *argv[])
{
	char *arg;
	char *myname;
	char *temp;
	int  do_fork = TRUE;

	strcpy(mylocalhostname, getmachinename());
	strcpy(mylocalhost, host2ip(mylocalhostname));
	strcpy(configfile, CFGFILE);
	*USERFILE = (char) 0;
	in_dcc = 0;
	need_dcc = 0;
	which_ccc = 0;
	ccc_sock = -1;
	ccc_port = CCC_PORT;
	lastrejoin = 0;
	if ((temp = (char *)getenv("CCCPORT")) != (char *) 0)
		ccc_port = atoi(temp); 
	number_of_bots = 0;
	idletimeout = DEFAULT_DCC_IDLETIMEOUT;
	waittimeout = DEFAULT_DCC_WAITTIMEOUT;
	maxuploadsize = DEFAULT_DCC_MAXFILESIZE;
	dbuglevel = QUIET;
	myname = argv[0];
	from_shell = 0;
	printf( "ComBot %s (c) 1995 CoMSTuD, released %s\n", VERSION, BOTDATE);
	printf( "For more info: e-mail cbehrens@slavery.com\n");
#ifdef USE_CCC
	printf( "Note: The CCC Port being used is %i\n", ccc_port); 
#endif
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
			strcpy(configfile,*argv);
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
	signal(SIGALRM, sig_alrm);
	if( do_fork )
	  {
		 printf( "Running %s in background\n", myname );
		 if( !fork() )
			start_bots();
		 else
			exit( 0 );
	  }
	else
	  start_bots();

	return 0;
}


