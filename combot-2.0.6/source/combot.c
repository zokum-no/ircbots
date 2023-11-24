/*
 * combot.c - functions to maintain the bot structures
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>


#include "global.h"
#include "config.h"
#include "defines.h"
#include "structs.h"
#include "h.h"

void init_bots(void)
{
	int i;

	current=NULL;
	for (i=0; i<MAXBOTS; i++)
		thebots[i].created = 0;
}

int read_lists_all(void)
{
	int i;

	read_levelfile(LEVELFILE);
	readuserlist(USERFILE);
        for (i=0;i<MAXBOTS;i++)
                if (thebots[i].created)
		   read_timelist(&thebots[i].KickList, thebots[i].kickfile);
	return (Userlist != NULL);
}

aBot *find_bot(s)
char *s;
{
	int     i;

	for(i=0; i<MAXBOTS; i++)
		if (thebots[i].created && !my_stricmp(thebots[i].nick, s))
			return &(thebots[i]);
	return NULL;
}

aBot *add_bot(nick)
char *nick;
{
	int i;

	for (i=0; i<MAXBOTS; i++)
		if (!thebots[i].created)
		{
			char buffer[MAXLEN], buffer2[MAXLEN];
			number_of_bots++;
			thebots[i].created = TRUE;
			strcpy(thebots[i].nick, nick);
			strcpy(thebots[i].realnick, nick);
			strcpy(thebots[i].login, DEFAULT_LOGIN);
			strcpy(thebots[i].ircname, DEFAULT_IRCNAME);
			strcpy(buffer, BOTDIR);
			strcat(buffer, nick);
			strcpy(buffer2, buffer);
			strcat(buffer2, ".memory");
			mstrcpy(&thebots[i].kickfile, buffer2);
			thebots[i].server_sock = -1;
			thebots[i].cmdchar = DEFAULT_CMDCHAR;
			thebots[i].restrict = 0;
			strcpy(thebots[i].serverlist[0].name, DEFAULT_SERVER);
			thebots[i].serverlist[0].port = DEFAULT_PORT;
			thebots[i].filesendallowed = DEFAULT_DCC_SEND_ALLOWED;
			thebots[i].filerecvallowed = DEFAULT_DCC_RECV_ALLOWED;
			mstrcpy(&thebots[i].uploaddir, FILEUPLOADDIR);
			mstrcpy(&thebots[i].downloaddir, FILEDOWNLOADDIR);
			thebots[i].num_of_servers = 1;
			thebots[i].current_server = 0;
			thebots[i].Floods = NULL;
			thebots[i].Channel_list = NULL;
			thebots[i].OldChannel_list = NULL;
			thebots[i].Current_chan = NULL;
			thebots[i].StealList = NULL;
			thebots[i].SpyMsgList = NULL;
			thebots[i].OffendersList = NULL;
			thebots[i].KickList = NULL;
			thebots[i].MemList = NULL;
			thebots[i].Client_list = NULL;
			thebots[i].TheLinks = NULL;
			thebots[i].TempLinks = NULL;
			thebots[i].uptime = thebots[i].lastping = thebots[i].lastreset =
				getthetime();
			thebots[i].server_ok = TRUE;
			return &(thebots[i]);
		}
	return NULL;
}


int update_login(login)
char *login;
{
	if (!current)
		return FALSE;
	strcpy(current->login, login);
	return TRUE;
}

int update_ircname(ircname)
char *ircname;
{
	if (!current)
		return FALSE;
	strcpy(current->ircname, ircname);
	return TRUE;
}

int set_server(servername, port)
char *servername;
int port;
{
	int temp;

	for (temp=0;temp<current->num_of_servers;temp++)
		if ((port == current->serverlist[temp].port) &&
			!my_stricmp(current->serverlist[temp].name, servername))
		{
			current->current_server = temp;
			return TRUE;
		}
	return FALSE;
}

int add_server(servername, port)
char *servername;
int port;
{
	int temp;

	if (!current)
		return FALSE;

	if (current->num_of_servers >= MAXSERVERS)
		return FALSE;
	for (temp=0;temp<current->num_of_servers;temp++)
		if ((current->serverlist[temp].port == port) &&
			!my_stricmp(current->serverlist[temp].name, servername))
			return TRUE;
	strcpy(current->serverlist[current->num_of_servers].name, servername);
	current->serverlist[current->num_of_servers].port = port; 
	(current->num_of_servers)++;
	return TRUE;
}

int kill_bot(reason)
char *reason;
{
	int     i;
	int     botnum;

	/* first find bot's slot */
	for (i=0; i<MAXBOTS; i++)
		if (thebots[i].created && !my_stricmp(thebots[i].nick, current->nick))
			botnum = i;

	/* quit should be send by calling function */
	close_all_dcc();
	delete_all_channels();
	if (current->server_sock != -1)
	{
		sendquit(reason);
		close(current->server_sock);
		current->server_sock = -1;
	}
	MyFree(&current->uploaddir);
	MyFree(&current->downloaddir);
	MyFree(&current->kickfile);
	delete_list(&current->StealList);
	delete_list(&current->SpyMsgList);
	delete_time(&current->KickList);
	delete_time(&current->OffendersList);
	delete_flood(&current->Floods);
	delete_memory(&current->MemList);
	free_links(&current->TheLinks);
	free_links(&current->TempLinks);

 	current->created = 0; /* end changed */
	
	/* find a bot.. doesn't matter which one.. just to be sure */
	number_of_bots--;
	for (i=0; i<MAXBOTS; i++)
		if (thebots[i].created)
		{
			current=&(thebots[i]);
			return 1;
		}
	/* No bot found! */
	return 0;
}

void kill_all_bots(reason)
char *reason;
{
	int i;

	for (i=0; i<MAXBOTS ;i++)
	{
		if (thebots[i].created)
		{
			current = &(thebots[i]);
			kill_bot(reason);
		}
	}
	MyExit(0);
}

void start_all_bots(void)
{
	int     i;

	for (i=0; i<MAXBOTS; i++)
	{
		if (thebots[i].created)
		{
			current = &(thebots[i]);
			connect_to_server();
		}
	}
}

int connect_to_server(void)
/*
 * connects to a server, and returns -1 if it fails.
 */
{
#ifdef DBUG
	debug(NOTICE, "connect_to_server(): Connecting to port %d of server %s",
			 current->serverlist[current->current_server].port,
			 current->serverlist[current->current_server].name);
#endif
	if ((current->server_sock = connect_by_number(
		 current->serverlist[current->current_server].port,
		 current->serverlist[current->current_server].name)) < 0)
	{
		close(current->server_sock);
		current->server_sock = -1;
		return 0;
	}

	sendregister(current->nick, current->login, current->ircname);
	/* channels will be joined ("reset") when we receive a '001' */
	return TRUE;
}

void reconnect_to_server(void)
/*
 * connects to a server (i.e. after a kill) and rejoins all channels
 * in the channellist.
 */
{
	if (current->server_sock != -1)
		close(current->server_sock);
	/* Let try_reconnect do the work */
	current->server_sock = -1;
}

void set_server_fds(reads, writes)
fd_set *reads;
fd_set *writes;
{
  int     i;
  
  for (i=0; i<MAXBOTS; i++)
	 if (thebots[i].created && thebots[i].server_sock != -1)
		FD_SET( thebots[i].server_sock, reads );
}

int readln(buf)
char *buf;
{
	return read_from_socket(current->server_sock, buf);
}

int send_to_server(char *format, ...)
{
	va_list msg;
	char buf[HUGE];
	int temp;

	if (current->server_sock == -1)
	  return 0;
	va_start(msg, format);
	vsprintf(buf, format, msg);
	temp = send_to_socket(current->server_sock, "%s", buf);
	va_end(msg);
	return temp;
}

#ifdef USE_CCC

int send_to_ccc(char *format, ...)
{
	va_list msg;
	char buf[HUGE];
	int temp;

	if (ccc_sock == -1)
		return 0;
        va_start(msg, format);
        vsprintf(buf, format, msg);
	temp = send_to_socket(ccc_sock, "%s", buf);
	va_end(msg);
	return temp;
}

#endif /* USE_CCC */

void try_reconnect(void)
/*
 * check for every bot if the socket is still connected (i.e. not -1)
 * and if a ping has been received lately. If not, automatically go
 * to the next server
 */
{
	int     i;
	char    reason[MAXLEN];

	/*
		Check for every slot if it contains a bot. If so, reconnect if
		the socket = -1 or the server hasn't responded to a ping
	 */
	for (i=0; i<MAXBOTS; i++)
		if (thebots[i].created && (
			(thebots[i].server_sock == -1) || ((
			(getthetime()-(thebots[i].lastping))>PINGINTERVAL) &&
			  !(thebots[i].server_ok))))
		{
			current = &(thebots[i]);
#ifdef DBUG
			debug(ERROR, "try_reconnect(): Server %s not\
 responding, closing connection",
				current->serverlist[current->current_server].name);
#endif
			/* Make sure we'll connect to another server */
			if (current->server_sock != -1)
			{
				close(current->server_sock);
				current->server_sock = -1;
			}
			if (current->current_server <
				current->num_of_servers-1)
				current->current_server++;
			else
				current->current_server = 0;
			sprintf(reason, "Changing servers. Connecting to %s",
				current->serverlist[current->current_server].name);
			sendquit(reason);
			connect_to_server();
			/* assume server is ok */
			current->lastping = getthetime();
			current->server_ok = TRUE;
		}
}

void reset_botstate(void)
/*
 * reset nickname to realnick and join all channels
 * the bot might have been kicked off.
 */
{
	int     i;
	static time_t bleah = 0;

	if (!bleah)
		bleah = getthetime();
	for(i=0; i<MAXBOTS; i++)
		if (thebots[i].created)
		{
			current = &(thebots[i]);
			if ((getthetime()-lastrejoin) > 5)
			{
				rejoin_channels(SOFTRESET);
				lastrejoin = getthetime();
			}
			if (get_int_varc(NULL, TOGWL_VAR))
				if ((getthetime()-bleah) > LINKTIME)
			{
				send_to_server("LINKS");
				bleah = getthetime();
			}
			if (getthetime()-thebots[i].lastreset > RESETINTERVAL)
			{

#ifdef DBUG
				debug(NOTICE, "Resetting botstate");
#endif
				uptime_time(&current->OffendersList, getthetime()-1800);
				
				current->lastreset = getthetime();
 				sendprivmsg("blehdhfsddd", "%s", "lkdsjfdlk");
				cleanup_memory();
				write_timelist(&current->KickList,
					current->kickfile);
				if (!is_me(current->realnick))
				{
					strcpy(current->nick,
				 		 current->realnick);
					 sendnick(current->nick);
				}
			}
		}
}

void parse_server_input(read_fds)
/*
 * If read_fds is ready, this routine will read a line from the
 * the server and parse it.
 */
fd_set *read_fds;
{
	char    line[HUGE];
	int     i;

	for (i=0; i<MAXBOTS; i++)
	{
		if (thebots[i].created && thebots[i].server_sock != -1)
		{
			current = &(thebots[i]);
			if ( FD_ISSET( current->server_sock, read_fds ) )
				if (readln( line ) > 0 )
					parseline(line);
				else
				{
#ifdef DBUG
					debug(ERROR, "parse_server_input(): Server read FAILED!");
#endif
					close(current->server_sock);
					current->server_sock = -1;
				}
		}
	}
}

#ifdef USE_CCC

void parse_ccc_input(read_fds)
fd_set *read_fds;
{
        char line[HUGE];

	if (ccc_sock != -1)
	{
		if (FD_ISSET(ccc_sock, read_fds))
		{
			if (read_from_socket(ccc_sock, line) > 0 )
				parsecccline(line);
			else
			{
				close(ccc_sock);
				ccc_sock = -1;
			}
		}
	}
}

#endif

void set_dcc_fds(read_fds, write_fds)
fd_set *read_fds;
fd_set *write_fds;
{
	int     i;

	for(i=0; i<MAXBOTS; i++)
	{
		if (thebots[i].created)
		{
			current = &(thebots[i]);
			dccset_fds(read_fds, write_fds);
		}
	}
}

void parse_dcc_input(read_fds)
/*
 * Scan all bots for dcc input
 */
fd_set *read_fds;
{
	int     i;

	for (i=0; i<MAXBOTS; i++)
	{
		if (thebots[i].created)
		{
			current = &(thebots[i]);
			parse_dcc(read_fds);
		}
	}
}

void send_pings(void)
/*
 * Send a ping to all servers and set lastping to NOW
 */
{
	int     i;

	for (i=0; i<MAXBOTS; i++)
	{
		if (thebots[i].created)
		{
			current = &(thebots[i]);
			if (getthetime() - current->lastping /*send*/ >
				PINGSENDINTERVAL)
			{
#ifdef DBUG
				debug(NOTICE,
					 "send_pings(): Sending ping to server %s[%s]",
					 current->serverlist[
					 current->current_server].realname,
					 current->serverlist[
					 current->current_server].name);
#endif
				sendping(current->serverlist[current->current_server].realname);
				current->server_ok = FALSE;
				current->lastping = getthetime();
			}
		}
	}
}

void pong_received(nick, server)
/*
 * Pong received from current->serverlist[current->current_server]
 * Server must be ok.
 */
char *nick;
char *server;
{
	current->server_ok = TRUE;
#ifdef DBUG
	debug(NOTICE, "pong_received(): server %s[%s] ok",
	current->serverlist[current->current_server].realname,
	current->serverlist[current->current_server].name);
#endif
}

void global_bah(char *from, char *rest);

/* Global commands */
struct
{
	char    *name;
	void    (*function)(char*, char*);
	int     userlevel;
	int     forcedcc;
} global_cmds[] =
{
	{ "LIST",       global_list,     99,    TRUE    },
	{ "INFO",       global_list,     99,    TRUE    },
	{ "DIE",        global_die,      99,    FALSE   },
	{ "DEBUG",      global_debug,    99,    FALSE   },
	{ "RAWMSG",     global_rawmsg,   99,    FALSE   },
	{ "BAH",        global_bah,       0,    FALSE   },
	{ NULL,         (void(*)())(NULL) }
};

void parse_global(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	int      i;
	char     *command;
	aDCC *dccclient;

	if (!(command = get_token(&rest, " ")))
	{
		send_to_user(from, "\002Global expects a subcommand!\002");
		return;
	}

	for (i=0; global_cmds[i].name; i++)
		if(!my_stricmp(global_cmds[i].name, command))
		{
			if ((get_userlevel(from, "*") < global_cmds[i].userlevel) ||
				 get_shitlevel(from, "*") > 0)
			{
				send_to_user(from, "\002Incorrect levels\002");
				return;
			}
			dccclient = search_list("chat", from, DCC_CHAT);
			if (global_cmds[i].forcedcc &&
			  (!dccclient || (dccclient->flags&DCC_WAIT)))
			{
#ifdef AUTO_DCC
				dcc_chat(from, rest);
				sendnotice(getnick(from), "Please type: /dcc chat %s",
					current->nick);
				sendnotice(getnick(from),
					"After that, please retype: /msg %s GLOBAL %s %s",
					current->nick, command,
					rest?rest:"");
#else
				sendnotice(getnick(from),
					"Sorry, GLOBAL %s is only available through DCC Chat",
					command);
				sendnotice(getnick(from,
					"Please start a dcc chat connection first");
#endif
				return;
			}
				global_cmds[i].function(from, *rest?rest:NULL);
				return;
		}
	send_to_user(from, "Unknown global command");
	return;
}

void global_not_imp(from, rest)
char *from;
char *rest;
{
	send_to_user(from, "Global command not implemented");
}

void global_debug(from, rest)
char *from;
char *rest;
{
#ifdef DBUG
	if (!rest)
	{
		send_to_user(from, "\002Pls specify a level between 0 and 2\002");
		return;
	}
	if (set_debuglvl(atoi(rest)))
		send_to_user(from, "\002Debug set to debuglevel %d\002", atoi(rest));
	else
		send_to_user(from, "\002Could not set debuglevel\002");
#else
	send_to_user(from, "This version was not compiled with debugging enabled");
#endif
}

void global_die(from, rest)
char *from;
char *rest;
{
	aBot *bot;

	if (rest)
	{
		if ((bot=find_bot(rest)) != NULL)
		{
			current = bot;
			signoff(from, "Global kill");
			if (number_of_bots == 0)
				MyExit(0);
		}
		else
			send_to_user(from, "\002No such bot\002");
	}
	else
	{
		kill_all_bots("Global die");
		MyExit(0);
	}
}

void global_list(char *from, char *rest)
{
	int        i;
	aChannel *chan;
	aBot    *bot;

	rest = stripl(rest, " ");

	if (rest && (bot = find_bot(rest)))
	{
		send_to_user(from, "\002nick\002 %s \002realnick\002 %s", bot->nick, bot->realnick);
		send_to_user(from, "%d servers in list:", bot->num_of_servers);
		for (i=0; i<bot->num_of_servers; i++)
		{
			send_to_user(from, "\002server\002 %s[%s] \002port\002 %d%s",
			bot->serverlist[i].realname, bot->serverlist[i].name,
			bot->serverlist[i].port, i==bot->current_server?", current":"");
		}
		/* show_channellist(from); won't work. It'll show the channellist
			of current. And setting current to thebots[i] won't work
			either, the server_sock for sending will point to the wrong
			server. */
		if (!bot->Channel_list)
			send_to_user(from, "Not active on any channels!");
		else
			for (chan = bot->Channel_list; chan; chan = chan->next)
				send_to_user(from, "\002Active on:\002 %s", chan->name);
		send_to_user(from, "\002uploaddir:\002   %s", bot->uploaddir);
		send_to_user(from, "\002downloaddir:\002 %s", bot->downloaddir);
		return;
	}
	/* else */
	send_to_user(from, "Total number:  %d", MAXBOTS);
	send_to_user(from, "Total running: %d", number_of_bots);
	send_to_user(from, "Total free:    %d", MAXBOTS-number_of_bots);
	send_to_user(from, "Started:       %-20.20s",
			  time2str(uptime));
	send_to_user(from, "Uptime:        %-30s",
			  idle2str(getthetime()-uptime));
	send_to_user(from, " ");
	send_to_user(from, "Num nickname  current server");
	for (i=0; i<MAXBOTS; i++)
		if (thebots[i].created)
			send_to_user(from, "%2d: %-9s %s", i, thebots[i].nick,
			thebots[i].serverlist[thebots[i].current_server].realname);
		else
			send_to_user(from, "%2d: Free", i);
}

void global_rawmsg(from, rest)
char *from;
char *rest;

{
  char *to;

  if (!rest || !*rest)
	 return;
  if (!(to = get_token(&rest, " ")))
	 return;
  if (rest)
	 send_to_server("PRIVMSG %s :%s", to, rest);
}

void global_bah(from, rest)
char *from;
char *rest;
{
	;
}

int forkbot(from, nick, rest)
char *from;
char *nick;
char *rest;
/*
 * Adds bot to list and starts it.
 * except for nick, login and name the bot is an identical
 * copy of its "parent"
 */
{
	aChannel *c_list;
	aBot *newbot;
	aBot *oldbot;
	char *option, *login, *channel, *server, *cmdchar, *ircname;
	int i, error;

	error = 0;
	cmdchar = ircname = login = server = channel = NULL;
	oldbot = current;
	while (rest && *rest)
	{
		option = get_token(&rest, " ");
		if (*option == '-')
		{
			option++;
			switch(*option)
			{
				case 'u':
					login = get_token(&rest, " ");
					break;
				case 'i':
					ircname = rest;
					rest = NULL;
					break;
				case 's':
					server = get_token(&rest, " ");
					break;
				case 'c':
					channel = get_token(&rest, " ");
					break;
				case 'z':
					cmdchar = get_token(&rest, " ");
					break;
				default:
					error++;
			}
		}
	}
	if (error)
	{
		send_to_user(from, "\002%s\002", "Invalid arguments given");
		return FALSE;
	}
	if (!cmdchar)
		cmdchar = &current->cmdchar;
	if ((newbot = add_bot(nick)) != NULL)
	{
		nick[9] = '\0';
		strcpy(newbot->nick, nick);
		strcpy(newbot->realnick, nick);
		if (login && (strlen(login) > MAXNICKLEN))
			login[9] = '\0';
		strcpy(newbot->login, login?login:current->login);
		strcpy(newbot->ircname, ircname?ircname:current->ircname);
		newbot->cmdchar = (cmdchar && *cmdchar) ? *cmdchar : current->cmdchar;
		for (i=0; i < current->num_of_servers; i++)
			newbot->serverlist[i] = current->serverlist[i];
		newbot->num_of_servers = current->num_of_servers;
		newbot->SpyMsgList = current->SpyMsgList;
		newbot->StealList = current->StealList;
		c_list = current->Channel_list;
		current = newbot;
		if (server)
		{
			add_server(server, 6667);
			set_server(server, 6667);
		}
		if (!channel)
			copy_channellist(c_list);
		else
			join_channel(channel, "", FALSE);
		connect_to_server();
		return 1;
	}
	return 0;
}








