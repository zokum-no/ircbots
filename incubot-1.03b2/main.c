/*
 * VladBot  - servicebot for IRC.
 *
 * (c) 1993-94 VladDrac (irvdwijk@cs.vu.nl)
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
#include <unistd.h>
#ifndef SYSV
#include <strings.h>
#else
#define bzero(s,l)      memset(s,0,l)
#define bcopy(s,d,l)    memmove(d,s,l) 
#endif
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/stat.h>
#ifdef AIX
#	include <sys/select.h>
#endif /* AIX */

#include "channel.h"
#include "config.h"
#include "debug.h"
#include "log.h"
#include "parse.h"
#include "session.h"
#include "userlist.h"
#include "tb-ons.h"
#include "vladbot.h"

extern 	long	 uptime;	/* time when bot was started   */
extern 	char 	*configfile;
extern 	int 	rehash;
extern	int	ALARM2;
extern	char	localhost[256];

#ifdef NEXT
int select(int, fd_set *, fd_set *, fd_set *, struct timeval *);
int bzero(char *, int);
#endif /* NEXT */

/*int 	fork ();*/
int 	atoi (char *);
void 	readcfg ();
void 	readlevelfiles ();

void 	sig_hup ();
void 	sig_cntl_c ();
void 	sig_bus ();
void 	sig_pipe ();
void 	sig_segv ();
extern void		sig_alrm ();
unsigned int	alarm		(unsigned int);
int	select(int, fd_set *, fd_set *, fd_set *, struct timeval *);
void 	sig_hup ()

{
  globallog (0, ERRFILE, "REHASHING: Hangup (sighup) received");
  rehash = TRUE;
  signal (SIGHUP, sig_hup);
}

void 	sig_int ()

{
	quit_all_bots (NULL, "Received SIGINT - Control-C!");
	globallog (0, ERRFILE, "SIGNING OFF: Control-C (sigint) received");
#ifdef USE_PIDFILE
	unlink(PID_FILE);
#endif
	exit (0);
}

void 	sig_bus ()

{
	quit_all_bots (NULL, "Received SIGBUS");
	globallog (0, ERRFILE, "SIGNING OFF: received Sigbus!");
#ifdef USE_PIDFILE
	unlink(PID_FILE);
#endif
	exit (0);
	signal (SIGBUS, sig_bus);
}

void 	sig_pipe ()

{
  	globallog (0, ERRFILE, "IGNORING: received Broken Pipe!");
  	signal (SIGPIPE, sig_pipe);
}

void 	sig_segv ()

{
  	quit_all_bots (NULL, "Received SIGSEGV - core dumped");
  	globallog (0, ERRFILE, "SIGNING OFF: Segmentation violation (sigsegv) received");
#ifdef USE_PIDFILE
	unlink(PID_FILE);
#endif
  	exit (0);
}

void 	bot_rehash ()
{
	quit_all_bots (NULL, "Rebooting");
  	readcfg ();
  	readlevelfiles ();
  	start_all_bots ();

  	rehash = FALSE;
}

int start_bots ()
{
  	struct timeval timer;
  	fd_set rd, wd;
#ifdef USE_PIDFILE
	FILE	*fp;
#endif

	strcpy(localhost, "");
	gethostname(localhost,sizeof(localhost));
  	bot_init ();
   uptime = time (NULL);/* set uptime */
   rehash = TRUE;/* not really rehash, but read cfgfile etc. */
#ifdef USE_PIDFILE
	unlink(PID_FILE);
	if ((fp=fopen(PID_FILE,"w")) != NULL) {
		fprintf(fp, "%u\n", getpid());
		fclose(fp);
	}
	else printf("* Warning!  Could not write %s file!\n", PID_FILE);
#endif

	while (1)
	{
		if (ALARM2>1)
			checkalarms();

		if (rehash)
		{
			bot_rehash ();
		}
		else
		{
			/* first, try to reconnect any dead sockets */
			try_reconnect ();
			send_pings ();		/* ping servers for activity 	*/
			reset_botstate ();	/* reset nickname and channels 	*/
			check_timer_unban(); /* check timed bans in all channels. unban if expired */
		}

		timer.tv_sec = WAIT_SEC;
		timer.tv_usec = WAIT_MSEC;

		FD_ZERO (&rd);
		FD_ZERO (&wd);

		set_dcc_fds (&rd, &wd);
		set_server_fds (&rd, &wd);
		switch (select (NFDBITS, &rd, 0, 0, &timer))
		{
			case 0:
#ifdef DBUG
				global_dbg (NOTICE, "SELECT: timeout");
			break;
#endif
			case -1:
#ifdef DBUG
				global_dbg (ERROR, "SELECT: error");
#endif
				break;
			default:
				parse_server_input (&rd);
				parse_dcc_input (&rd);
			break;
		}
	}
}


int main (int argc, char *argv[])
{
	char 	*arg;
	char 	*myname;
	int 	do_fork = FALSE;
#ifdef USE_PIDFILE
	int	xx;
	FILE	*f;
	char	s[520];
#endif

	umask(066); /* All created files will be +rw for owner only */

	myname = argv[0];

	printf ("%s %s\n", myname, VERSION);

#ifdef USE_PIDFILE
	if((f=fopen(PID_FILE,"r"))!=NULL) {
		fgets(s,10,f);
		xx=atoi(s);
		kill(xx,SIGCHLD);   /* meaningless kill to determine if pid is used */
		if (errno!=ESRCH) {
			printf("I detect an IncuBot already running from this directory.\n");
			printf("If this is incorrect, erase the '%s' file.\n\n",PID_FILE);
			exit(1);
		}
	}
#endif
	while (argc > 1 && argv[1][0] == '-')
	{
		argc--;
		arg = *++argv;
		switch (arg[1])
		{
			case 'h':
				printf ("usage: %s [switches [args]]\n", myname);
				printf ("-h               shows this help\n");
				printf ("-b               run %s in the background\n",myname);
				printf ("-f file          Read configfile 'file'\n");
#ifdef DBUG
				printf ("-d [0|1|2]	 Set debuglevel.\n");
				printf ("                   0 = debugging off\n");
				printf ("                   1 = show errors\n");
				printf ("                   2 = show detailed information\n");
#endif
				exit (0);
				break;
			case 'b':
				do_fork = TRUE;
				break;
			case 'f':
				++argv;
				if (!*argv)
				{
						printf ("No configfile specified\n");
						exit (0);
				}
				configfile = *argv;
				argc--;
				printf ("Using cfg %s\n", configfile);
				break;
			case 'd':
#ifdef DBUG
				++argv;
				argc--;
				if (*argv && set_debuglvl (atoi (*argv)))
					printf ("Debuglevel set to %d\n", atoi (*argv));
				else
				{
					printf ("Invalid debugvalue!\n");
					exit (0);
				}
#else
				printf ("This version was not compiled with debugging enabled\n");
#endif
				break;
			case 'l':
				++argv;
				argc--;
				if (*argv && set_loglevel (atoi (*argv)))
					printf ("Loglevel set to %d\n", atoi (*argv));
				else
				{
					printf ("Invalid loglevel!\n");
					exit (0);
				}
				break;
			default:
				printf ("Unknown option %s\n", arg);
				exit (1);
				break;
		}
	}

	set_loglevel(3);

	signal (SIGHUP, sig_hup);
	signal (SIGINT, sig_int);
	signal (SIGBUS, sig_bus);
	signal (SIGPIPE, sig_pipe);
	signal (SIGSEGV, sig_segv);
	signal (SIGALRM, sig_alrm);
	alarm(120);

	if (do_fork)
	{
		if (!fork ())
		{
			printf ("Running %s in background\n", myname);
			start_bots ();
		}
		else
			exit (0);
	}
	else
		start_bots ();

	return 0;
}
