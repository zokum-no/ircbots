/*

    EnergyMech, IRC bot software
    Parts Copyright (c) 1997-2001 proton, 2002-2004 emech-dev

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/
#define MAIN_C
#include "config.h"

#ifdef MEGA
#include "cfgfile.c"
#include "channel.c"
#include "com-ons.c"
#include "combot.c"
#include "commands.c"
#include "dcc.c"
#include "debug.c"
#include "function.c"
#include "link.c"
#include "parse.c"
#include "socket.c"
#include "userlist.c"
#include "vars.c"
#include "xmech.c"
#endif /* MEGA */

#define SAYNICK "Please enter your handle.\n"

void mech_exec(void)
{
	struct	stat s;
	char	*p;
	int	i;

	if (stat(executable,&s) < 0)
	{
#ifdef DEBUG
		debug("(mech_exec) unable to stat executable\n");
#ifdef SUPERDEBUG
		if (debug_on_exit)
			do_debug(NULL,NULL,NULL,0);
#endif /* SUPERDEBUG */
#endif /* DEBUG */
		exit(1);
	}
	s.st_atime = 0;
	s.st_mtime = 0;
	s.st_ctime = 0;
	if (memcmp(&s,&mechbin,sizeof(mechbin)))
	{
#ifdef DEBUG
		debug("(mech_exec) executable has been altered\n");
#ifdef SUPERDEBUG
		if (debug_on_exit)
			do_debug(NULL,NULL,NULL,0);
#endif /* SUPERDEBUG */
#endif /* DEBUG */
		exit(1);
	}
	for(i=0;i<256;i++)
	{
		close(i);
	}
	set_mallocdoer(mech_exec);
	p = MyMalloc(strlen(executable)+4);
	sprintf(p,"%s %i",executable,respawn);
	execl(executable,p,(char *)NULL);
#ifdef SUPERDEBUG
	if (debug_on_exit)
		do_debug(NULL,NULL,NULL,0);
#endif /* SUPERDEBUG */
	exit(1);
}

int matches(char *ma, char *na)
{
	Uchar	*m = (Uchar *)ma;
	Uchar	*n = (Uchar *)na;
	Uchar	*mask = (Uchar *)ma;
	int	wild = 0, q = 0;

	nummatch = 0;
	if (!m || !n)
		return(TRUE);

	while(TRUE)
	{
		if (!*m)
		{
			if (!*n)
				return(FALSE);
			for (m--;((*m == '?') && (m > mask));m--)
				;
			if ((*m == '*') && (m > mask) && (m[-1] != '\\'))
				return(FALSE);
			if (wild)
			{
				m = (u_char *)ma;
				n = (u_char *)++na;
			}
			else
				return(TRUE);
		}
		else
		if (!*n)
		{
			while(*m == '*')
				m++;
			return(*m != 0);
		}

		if (*m == '*')
		{
			while (*m == '*')
				m++;
			wild = 1;
			ma = (char *)m;
			na = (char *)n;
		}
		
		if ((*m == '\\') && ((m[1] == '*') || (m[1] == '?')))
		{
			m++;
			q = 1;
		}
		else
			q = 0;

		if ((tolower(*m) != tolower(*n)) && ((*m != '?') || q))
		{
			if (wild)
			{
				m = (u_char *)ma;
				n = (u_char *)++na;
			}
			else
				return(1);
		}
		else
		{
			nummatch++;
			if (*m)
				m++;
			if (*n)
				n++;
		}
	}
}

int num_matches(char *spec, char *what)
{
	if (!matches(spec,what))
		return(nummatch+1);

	return(FALSE);
}

int	r_ct;
char	*r_str;

int randstring_count(char *line)
{
	r_ct++;
	return(FALSE);
}

int randstring_getline(char *line)
{
	if (--r_ct == 0)
	{
		set_mallocdoer(randstring_getline);
		mstrcpy(&r_str,line);
		return(TRUE);
	}
	return(FALSE);
}

char *randstring(char *file)
{
	int	in;

	if ((in = open(file,O_RDONLY)) < 0)
		return(NULL);
	if (r_str)
		MyFree(&r_str);
	r_ct = 0;
	readline(in,&randstring_count);
	close(in);
	in = open(file,O_RDONLY);
	r_ct = RANDOM(1,r_ct);
	readline(in,&randstring_getline);
	close(in);
	return(r_str);
}

/*
 *  Signal handlers
 *  ~~~~~~~~~~~~~~~
 *  SIGHUP	Read and execute all commands in `mech.msg' file.
 *  SIGCHLD	Take care of zombies
 *  SIGALRM	Ignore ALRM signals
 *  SIGPIPE	Ignore PIPE signals
 *  SIGINT	Exit gracefully on ^C
 *  SIGBUS	(Try to) Exit gracefully on bus faults
 *  SIGSEGV	(Try to) Exit gracefully on segmentation violations
 *  SIGTERM	Exit gracefully when killed
 *  SIGUSR1	Jump (a) bot to a new server
 *  SIGUSR2	Call do_debug() (DEBUG command)
 */

char	*shellmask;

int sig_hup_callback(char *line)
{
	strcpy(shellmask,(char*)SHELLMASK);
	on_msg(shellmask,current->nick,line);
	return(FALSE);
}

void sig_hup(int crap)
{
	int	in;

	time(&now);

#ifdef DEBUG
	debug("(sighup)\n");
#endif /* DEBUG */

	signal(SIGHUP,sig_hup);
	if ((!will.signals) || (!will.shellcmd))
		return;

	backup = current;
	current = botlist;
	do_rehash2(current->nick,current->nick,NULL,OWNERLEVEL);
	if ((in = open(MSGFILE,O_RDONLY)) >= 0)
	{
		shell_cmd = TRUE;
		set_mallocdoer(sig_hup);
		shellmask = MyMalloc(strlen(SHELLMASK)+1);
		readline(in,&sig_hup_callback);
		MyFree((char**)&shellmask);
		close(in);
		unlink(MSGFILE);
		shell_cmd = FALSE;
	}
	current = backup;
}

/*
 *  Figured it out the hard way:
 *  DONT PUT THE signal() CALL BEFORE THE ACTUAL wait()!!
 *  Fucked up SunOS will hang/sigsegv!
 */
void sig_child(int crap)
{
#ifdef DEBUG
	debug("(sig_child)\n");
#endif /* DEBUG */

	while(1)
	{
		if (waitpid(-1,NULL,WNOHANG) <= 0)
			break;
	}
	signal(SIGCHLD,sig_child);
}

void sig_alrm(int crap)
{
#ifdef DEBUG
	debug("(sigalrm)\n");
#endif /* DEBUG */
	signal(SIGALRM,sig_alrm);
}

void sig_pipe(int crap)
{
#ifdef DEBUG
	debug("(sigpipe)\n");
#endif /* DEBUG */
	signal(SIGPIPE,sig_pipe);
}

void sig_int(int crap)
{
	time(&now);

#ifdef DEBUG
	debug("(sigint)\n");
#endif /* DEBUG */

	if (!will.signals)
	{
		signal(SIGINT,sig_int);
		return;
	}

#ifdef TRIVIA                                                                                                                
        /*                                                                                                                   
         *  trivia is global and is saved in the same way (and places) as session                                            
         */                                                                                                                  
        write_triviascore();                                                                                                 
#endif /* TRIVIA */

#ifdef SESSIONS
	savesession();
#endif /* SESSIONS */

	close_all_dcc();
	kill_all_bots(TEXT_SIGINT);
	/* NOT REACHED */
}

void sig_bus(int crap)
{
	time(&now);

	respawn++;
	if (respawn > 10)
		mechexit(1);

#ifdef DEBUG
	debug("(sigbus)\n");
#endif /* DEBUG */

	if (!will.signals)
	{
		signal(SIGBUS,sig_bus);
		return;
	}

#ifdef SESSIONS
	savesession();
#endif /* SESSIONS */

	close_all_dcc();
	do_exec = TRUE;
	kill_all_bots(TEXT_SIGBUS);
	/* NOT REACHED */
}

void sig_segv(int crap)
{
	time(&now);

	respawn++;
	if (respawn > 10)
		mechexit(1);

#ifdef DEBUG
	debug("(sigsegv)\n");
#endif /* DEBUG */

	if (!will.signals)
	{
		signal(SIGSEGV,sig_segv);
		return;
	}

#ifdef SUPERDEBUG
	if (debug_on_exit)
	{
		do_debug(NULL,NULL,NULL,0);
		debug_on_exit = FALSE;
	}
#endif /* DEBUG */

#ifdef SESSIONS
	savesession();
#endif /* SESSIONS */

	close_all_dcc();
	do_exec = TRUE;
	kill_all_bots(TEXT_SIGSEGV);
	/* NOT REACHED */
}

void sig_term(int crap)
{
	time(&now);

#ifdef DEBUG
	debug("(sigterm)\n");
#endif /* DEBUG */

	if (!will.signals)
	{
		signal(SIGTERM,sig_term);
		return;
	}

#ifdef SESSIONS
	savesession();
#endif /* SESSIONS */

	close_all_dcc();
	kill_all_bots(TEXT_SIGTERM);
	/* NOT REACHED */
}

void sig_usr1(int crap)
{
	time(&now);

#ifdef DEBUG
	debug("(sigusr1)\n");
#endif /* DEBUG */

	signal(SIGUSR1,sig_usr1);
	if (!will.signals)
		return;

	backup = current;
	current = botlist;
	if (current->connect == CN_ONLINE)
		to_server(TEXT_SIGUSR1);
	else
	{
		if (current->sock != -1)
			close(current->sock);
		current->sock = -1;
	}
	current = backup;
}

#ifdef DEBUG

void sig_usr2(int crap)
{
	time(&now);

	debug("(sigusr2)\n");
	signal(SIGUSR2,sig_usr2);
	do_debug(NULL,NULL,NULL,0);
}

#endif /* DEBUG */

/*
 *  Uptime stuffies
 */
#ifdef UPTIME

#define UPTIME_ENERGYMECH	1
#define UPTIME_WINMECH		4
#define UPTIMEHOST		"uptime.energymech.net"

int	uptimeport = 9969;
char	*uptimehost;
int	uptimesock;
int	uptimecount;
union sockaddr_union uptime_so;
Ulong	uptimecookie;
time_t	uptimelast;

typedef struct PackUp
{
	int	regnr;
	int	pid;
	int	type;
	Ulong	cookie;
	Ulong	uptime;
	Ulong	ontime;
	Ulong	now;
	Ulong	sysup;
	char	string[3];

} PackUp;

PackUp	upPack;

int init_uptime(void)
{
	union	sockaddr_union so;

	upPack.regnr = 0;
	upPack.pid = 0;

#ifndef __CYGWIN__
	upPack.type = htonl(UPTIME_ENERGYMECH);
#else
	upPack.type = htonl(UPTIME_WINMECH);
#endif

	upPack.cookie = 0;
	upPack.uptime = htonl(uptime);

	uptimecookie = rand();
	uptimecount = 0;
	uptimelast = 0;
	memset(&uptime_so, 0, sizeof(union sockaddr_union));
	uptime_so.sin.sin_family = AF_INET;
	uptime_so.sin.sin_port = htons(uptimeport);
	uptime_so.sin.sin_addr.s_addr = -1;

	set_mallocdoer(init_uptime);
	mstrcpy(&uptimehost,UPTIMEHOST);

	if ((uptimesock = socket(AF_INET,SOCK_DGRAM,0)) < 0)
		return((uptimesock = -1));

	memset(&so,0,sizeof(union sockaddr_union));

	so.sin.sin_addr.s_addr = INADDR_ANY;
	so.sin.sin_family = AF_INET;

	if (bind(uptimesock,&so.sa,sizeof(union sockaddr_union)) < 0)
	{
		close(uptimesock);
		return((uptimesock = -1));
	}
	fcntl(uptimesock,F_SETFL,O_NONBLOCK | fcntl(uptimesock,F_GETFL));
	return(0);
}

void send_uptime(void)
{
	struct	stat st;
	aServer	*sp;
	PackUp	*mem;
	char	*server;
	int	len;

	if (!upPack.pid)
		upPack.pid = htonl(getpid());
	          

	uptimecookie = (uptimecookie + 1) * 18457;
	upPack.cookie = htonl(uptimecookie);
	upPack.now = htonl(time(NULL));
	if (stat("/proc",&st) < 0)
		upPack.sysup = 0;
	else
		upPack.sysup = htonl(st.st_ctime);

	/*
	 *  set ontime and server
	 */
	upPack.ontime = htonl(botlist->conntry);
	sp = find_aServer(botlist->server);
	if (!sp)
		server = "unknown";
	else
		server = (sp->realname[0]) ? sp->realname : sp->name;

	uptimecount++;
	if (((uptimecount & 0x7) == 0) || (uptime_so.sin.sin_addr.s_addr == -1))
	{
		if (get_ip(uptimehost, &uptime_so))
			return;
	}

	len = sizeof(upPack) + strlen(botlist->nick) + strlen(server) + strlen(VERSION);
	set_mallocdoer(send_uptime);
	mem = (PackUp*)MyMalloc(len);
	memcpy(mem,&upPack,sizeof(upPack));
	sprintf(mem->string,"%s %s %s",botlist->nick,server,VERSION);

	sendto(uptimesock,(void*)mem,len,0,&uptime_so.sa,SIZEOF_SOCKADDR(uptime_so));
	MyFree((void*)&mem);
}

void do_uset(char *from, char *to, char *rest, int cmdlevel)
{
	char	*arg;
	int	x;

	if ((x = get_userlevel(from,NULL)) < cmdlevel)
		return;
	arg = chop(&rest);
	if (!Strcasecmp(arg,"port"))
	{
		x = atoi(rest);
		if ((x < 1) || (x > 65535))
		{
			send_to_user(from,TEXT_ENTITYINVALIDPORT); /* NOTE: I know it says ENTITY and it's not but the text was the same */
			return;
		}
		uptimeport = x;
		return;
	}
	else
	if (!Strcasecmp(arg,"server"))
	{
		MyFree(&uptimehost);
		set_mallocdoer(do_uset);
		mstrcpy(&uptimehost,rest);
		return;
	}
	else
	if (!Strcasecmp(arg,"send"))
	{
		send_uptime();
		return;
	}
	usage(from,C_USET);
}

#endif /* UPTIME */

/*
 *  The main loop
 */

#define chkhigh(x)	if (x > hisock) hisock = x;

void doit(void)
{
#ifdef LINKING
	aBotLink *bl;
	int	i;
#endif /* LINKING */
	union	sockaddr_union so;
	struct	timeval tv;
	aDCC	*Client;
	aBot	*bot,*bot2;
	int	f,j,hisock, uldie = 0;

	readcfgfile();

#ifdef LINKING
	botlinkBASE = NULL;
	botlinkSIZE = -1;
	linksock = -1;
#endif /* LINKING */

#ifdef DYNCMDLEVELS
	read_levelfile();
#endif /* DYNCMDLEVELS */

	for(bot=botlist;bot;bot=bot->next)
	{
		for(bot2=bot->next;bot2;bot2=bot2->next)
		{
			if (!Strcasecmp(bot->Userfile,bot2->Userfile))
			{
				printf(ERR_USERLISTMATCH,
					bot->nick,bot2->nick);
				printf(TEXT_NOSHAREUSERLIST);
				mechexit(0);
			}
		}
		current = bot;
		readuserlist();
#ifdef SEEN
		read_seenlist();
#endif /* SEEN */
	}

	for(bot=botlist;bot;bot=bot->next)
	{
		if (bot->userlistSIZE == 0) {
			printf(ERR_NOUSERLIST
				"run ./genuser %s\n",bot->wantnick, 
				bot->Userfile);
			uldie++;
		}
	}
	if (uldie) {
		printf(ERR_BOTSNOUSERLIST
			"running.\n", uldie);
		_exit(1);
	}

	printf("init: EnergyMech running...\n");

	if (getuid() == 0 || geteuid() == 0) _exit(0); /* Damn Kiddies */

	if (do_fork)
	{
		close(0);
		close(1);
		close(2);

		switch(fork())
		{
		case 0:
			while(getppid() != 1)
#ifdef HAS_USLEEP
				usleep(50000);
#else /* HAS_USLEEP */
			{
				tv.tv_sec = 0;
				tv.tv_usec = 50000;
				select(0,NULL,NULL,NULL,&tv);
			}
#endif /* HAS_USLEEP */
			break;
		default:
#ifdef SUPERDEBUG
			debug_on_exit = FALSE;
#endif /* SUPERDEBUG */
		case -1:
			mechexit2(0);
		}
		setsid();
	}

	if ((f = open(PIDFILE,O_WRONLY|O_CREAT|O_TRUNC,0600)) >= 0)
	{
		send_to_socket(f,"%i\n",getpid());
		close(f);
	}
#ifdef LINKEVENTS
	wle("EnergyMech started...\n");
#endif /* LINKEVENTS */

	time(&now);

mainloop:
	/*
	 *  The main loop, the core of the energymech
	 */
#ifdef DEBUG
	if ((runtime) && (runtime < now))
	{
		kill_all_bots(NULL);
		mechexit(0);
	}
#endif /* DEBUG */

#ifdef LINKING
	if ((linkport > 0) && (linksock == -1))
	{
		linksock = SockListener(linkport);
		if (linksock != -1)
		{
#ifdef LINKEVENTS
			wle("Linksock active: %i ( S:%i )\n",linkport,linksock);
#endif /* LINKEVENTS */
#ifdef DEBUG
			debug("(doit) Linksocket is active (%i)\n",linkport);
#endif /* DEBUG */
		}
	}
#endif /* LINKING */

	/*
	 *  Reset nickname and channels
	 */
	reset_botstate();

	/*
	 *  Longer delay saves CPU but some features require shorter delays
	 */
	tv.tv_sec = (short_tv) ? 1 : 30;
	tv.tv_usec = 0;

	FD_ZERO(&read_fds);
	FD_ZERO(&write_fds);
	hisock = -1;

	j = 0; /* TV_SERVCONNECT */
	for(bot=botlist;bot;bot=bot->next)
	{
		current = bot;
		if (current->sock == -1)
		{
			j++;
#ifdef DEBUG
			debug("(doit) CDELAY: %i/%i\n",now-current->conntry,get_int_varc(NULL,SETCDELAY_VAR)); 
#endif /* DEBUG */
			if ((now - current->conntry) >= get_int_varc(NULL,SETCDELAY_VAR))
				connect_to_server();
		}
		if (current->sock != -1)
		{
			if (current->ip.sa.sa_family == 0)
			{
				j = sizeof(union sockaddr_union);
				if (getsockname(current->sock,&so.sa,&j) == 0)
				{
					memcpy(&current->ip,&so,SIZEOF_SOCKADDR(so));
				}
			}
			if ((current->connect == CN_TRYING) || (current->connect == CN_CONNECTED))
			{
				j++;
#ifdef DEBUG
				debug("(doit) CTIMEOUT: %i/%i\n",now-current->conntry,get_int_varc(NULL,SETCTIMEOUT_VAR)); 
#endif /* DEBUG */
				if ((now - current->conntry) > get_int_varc(NULL,SETCTIMEOUT_VAR))
				{
#ifdef DEBUG
					debug("(doit) {%i} Connection timed out\n",current->sock);
#endif /* DEBUG */
					close(current->sock);
					current->sock = -1;
					goto restart_dcc;
				}
				if (current->connect == CN_TRYING)
					FD_SET(current->sock,&write_fds);
			}
			chkhigh(current->sock);
			FD_SET(current->sock,&read_fds);
		}
		/*
		 *  Clean out DCC_DELETE clients
		 */
restart_dcc:
		for(Client=current->ClientList;Client;Client=Client->next)
		{
			if (Client->flags == DCC_DELETE)
			{
				delete_client(&Client);
				goto restart_dcc;
			}
		}
		for(Client=current->ClientList;Client;Client=Client->next)
		{
			if (Client->flags & DCC_ASYNC)
			{
				chkhigh(Client->sock);
				FD_SET(Client->sock,&write_fds);
			}
			if (Client->sock != -1)
			{
				chkhigh(Client->sock);
				FD_SET(Client->sock,&read_fds);
			}
		}
#ifdef PIPEUSER
		if (current->pipeuser)
		{
			chkhigh(current->pipeuser->sock);
			FD_SET(current->pipeuser->sock,&read_fds);
		}
#endif /* PIPEUSER */
	}
	if (!j)
		short_tv &= ~TV_SERVCONNECT;
	else
		short_tv |= TV_SERVCONNECT;

#ifdef LINKING
	if (linksock != -1)
	{
		chkhigh(linksock);
		FD_SET(linksock,&read_fds);
	}
	j = 0;
	bl = botlinkBASE;
	for(i=0;i<botlinkSIZE;i++)
	{
		if ((bl->used) && (bl->sock != -1))
		{
			if (bl->nodetype == LN_SAYNICK_SEND)
			{
				if ((now - bl->uptime) >= 2)
				{
					bl->nodetype = LN_UNKNOWN;
					send_to_link(bl,SAYNICK);
				}
				else
					j++;
			}
			chkhigh(bl->sock);
			if (bl->linktype == LN_TRYING)
				FD_SET(bl->sock,&write_fds);
			else
				FD_SET(bl->sock,&read_fds);
		}
		bl++;
	}
	if (!j)
	{
		short_tv &= ~TV_TELNET_NICK;
	}
#endif /* LINKING */

	if ((select(++hisock,&read_fds,&write_fds,0,&tv) == -1) && (errno == EINTR))
		goto mainloop;

	/*
	 *  Update current time
	 */
	time(&now);

#ifdef LINKING
	parse_link_input();
#endif /* LINKING */

	for(bot=botlist;bot;bot=bot->next)
	{
		current = bot;
		parse_dcc_input();
		if (current->sock != -1)
			parse_server_input();
#ifdef PIPEUSER
		if ((current->pipeuser) && (FD_ISSET(current->pipeuser->sock,&read_fds)))
			parse_pipe_input();
#endif /* PIPEUSER */
		if (current->connect == CN_ONLINE)
		{
			update_auths();
			if (get_int_varc(NULL,TOGNOIDLE_VAR))
			{
				if ((now - current->lastping) > PINGSENDINTERVAL)
				{
					to_server("PRIVMSG * :0");
					current->lastping = now;
				}
			}
		}
	}
	/*
	 *  Check for do_die()'d bots...
	 */
restart_die:
	for(bot=botlist;bot;bot=bot->next)
	{
		if (bot->connect == CN_BOTDIE)
		{
			current = bot;
			signoff(current->from,current->signoff);
			goto restart_die;
		}
	}
#ifdef UPTIME
        if ((uptimesock > 0) && FD_ISSET(uptimesock,&read_fds))
	{
		int	res,len;
		struct
		{
			int	regnr;
			Ulong	cookie;

		} regPack;

		len = sizeof(union sockaddr_union);
		res = recvfrom(uptimesock,(void*)&regPack,sizeof(regPack),0,&so.sa,&len);
		if (res == sizeof(regPack))
		{
			if (upPack.cookie == ntohl(regPack.cookie))
			{
				if (upPack.regnr == 0)
					upPack.regnr = ntohl(regPack.regnr);
			}
		}
	}
	if (uptimelast < now)
	{
		uptimelast = now + 21600;
		send_uptime();
	}
#endif /* UPTIME */

#ifdef TRIVIA                                                                                                                
        trivia_tick();                                                                                                       
#endif /* TRIVIA */ 

	/*
	 *  Check killsocks
	 */
	if (killsocks)
		killsock(-1);

	goto mainloop;
}

/*
 *  main(), we love it and cant live without it
 */

char	*bad_exe = "init: Error: Improper executable name\n";

int main(int argc, char **argv)
{
	char	*opt = NULL;
	int	*ip;
	int	versiononly = FALSE;

#ifndef MULTI
	current = singlebot;
#endif /* MULTI */
	time(&now);

#ifdef DEBUG
	runtime = 0;
	dodebug = FALSE;
	debug_fd = -1;
	debugfile[0] = 0;
#ifdef SUPERDEBUG
	mrrec = calloc(sizeof(aMEA),1);
	debug_on_exit = FALSE;
#endif /* SUPERDEBUG */
#endif /* DEBUG */

#ifdef LINKEVENTS
	LE = -1;
#endif /* LINKEVENTS */

	if (!*argv || !**argv)
	{
		printf(bad_exe);
		_exit(1);
	}

	respawn = 0;
	if ((opt = strchr(*argv,' ')) != NULL)
	{
		*(opt++) = 0;
		if ((ip = Atoi(opt)) == NULL)
		{
			printf(bad_exe);
			_exit(1);
		}
		respawn = *ip;
	}

	if (stat(argv[0],&mechbin) < 0)
	{
		printf(ERR_NOSTART);
		_exit(1);
	}
	mechbin.st_atime = 0;
	mechbin.st_mtime = 0;
	mechbin.st_ctime = 0;
	strcpy(executable,*argv);
	strcpy(configfile,CFGFILE);

	need_dcc = FALSE;
	shell_cmd = FALSE;
	versiononly = FALSE;

	will.signals = TRUE;
	will.shellcmd = TRUE;
	will.seen = TRUE;
	will.onotice = FALSE;
	will.autolink = FALSE;

	sigmaster = 0;

	do_fork = TRUE;
	do_exec = FALSE;

	killsocks = NULL;
	botlist = NULL;

	serverBASE = NULL;
	serverSIZE = 0;
	serverident = 1;

	userident = 1;

#ifdef LINKING
	/*
	 *  Give 10 seconds of breathing-room before trying to autolink 1st time
	 */
	last_autolink = (now - (AUTOLINK_DELAY - 10));

	botlinkBASE = NULL;
	botlinkSIZE = 0;
	botlinkident = 1;

	linkconfBASE = NULL;
	linkconfSIZE = 0;

	linkport = -1;
#endif /* LINKING */

	makecore = FALSE;

	srand(now);

	opt = NULL;
	while((argc > 1) && (argv[1][0] == '-'))
	{
		argc--;
		argv++;
		opt = *argv;
		switch(opt[1])
		{
		case 'v':
			versiononly = TRUE;
			break;
		case 'h':
			printf(TEXT_USAGE1NOR,executable);
			printf(TEXT_USAGE2NOR);
			printf(TEXT_USAGE3NOR);
#ifdef DEBUG
			printf(TEXT_USAGE4DBG);
			printf(TEXT_USAGE5DBG);
			printf(TEXT_USAGE6DBG);
#ifdef SUPERDEBUG
			printf(TEXT_USAGE7SDBG);
#endif /* SUPERDEBUG */
#endif /* DEBUG */
			printf(TEXT_USAGE8NOR);
			printf(TEXT_USAGE9NOR);
			_exit(0);
		case 'c':
			makecore = TRUE;
			break;
#ifdef DEBUG
		case 'r':
			if (opt[2] != 0)
			{
				ip = Atoi(&opt[2]);
			}
			else
			{
				argv++;
				if (!*argv)
				{
					printf(TEXT_ARGRMISSING);
					_exit(0);
				}
				ip = Atoi(*argv);
				argc--;
			}
			if (!ip || *ip < 30)
			{
				printf(TEXT_INVALIDRUNTIME);
				_exit(0);
			}
			runtime = now + *ip;
			break;
		case 'd':
			dodebug = TRUE;
			do_fork = FALSE;
			break;
		case 'o':
			if (opt[2] != 0)
			{
				strcpy(debugfile,&opt[2]);
			}
			else
			{
				++argv;
				if (!*argv)
				{
					printf(TEXT_NODBGFILE);
					_exit(0);
				}
				strcpy(debugfile,*argv);
				argc--;
			}
			do_fork = TRUE;
			break;
#ifdef SUPERDEBUG
		case 'X':
			debug_on_exit = TRUE;
			break;
#endif /* SUPERDEBUG */
#endif /* DEBUG */
		case 'f':
			if (opt[2] != 0)
			{
				strcpy(configfile,&opt[2]);
			}
			else
			{
				++argv;
				if(!*argv)
				{
					printf(TEXT_NOCFGFILE);
					_exit(0);
				}
				strcpy(configfile,*argv);
				argc--;
			}
			printf(TEXT_USINGCFG,configfile);
			break;
		default:
			printf(TEXT_UNKNOWNOPT,opt);
			_exit(1);
		}
	}

	printf("EnergyMech %s, %s\n",VERSION,SRCDATE);
	printf("Compiled on " __DATE__ " " __TIME__ "\n");
	printf("Features: %s\n",(__mx_opts[0]) ? __mx_opts : "(core only)");

	if (versiononly)
		_exit(0);	/* _exit() here because we dont want a profile file to be written */

#ifdef __linux__
	signal(SIGCHLD,SIG_IGN);
#else
	signal(SIGCHLD,sig_child);
#endif
	signal(SIGHUP,sig_hup);
	signal(SIGINT,sig_int);
	signal(SIGBUS,sig_bus);
	signal(SIGPIPE,sig_pipe);
	signal(SIGALRM,sig_alrm);
	signal(SIGTERM,sig_term);
	signal(SIGUSR1,sig_usr1);
#ifdef DEBUG
	signal(SIGUSR2,sig_usr2);
#else /* DEBUG */
	signal(SIGUSR2,SIG_IGN);
#endif /* DEBUG */
	if (!makecore)
		signal(SIGSEGV,sig_segv);

	time(&uptime);
	time(&now);
#ifdef UPTIME
	init_uptime();
#endif
	doit();

	return(0);
}
