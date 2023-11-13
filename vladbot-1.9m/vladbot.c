/*
 *
 *   VladBot - servicebot for IRC
 *   Copyright (C) 1993 Ivo van der Wijk
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
 *
 * The "old" vlad-bot.c, which really was just the main loop, has been
 * renamed to main.c. This file manages the implementation of several bots
 * (several connections to servers). For now, well just stick to one
 * (a lot of routines have to be changed...)
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <strings.h>
#include <string.h>
#include "channel.h"
#include "config.h"

typedef struct	server_struct
{
	char	servername[MAXLEN];
	int	serverport;
} serv_struc;

typedef	struct	vladbot_struct
{
	char		nick[MAXNICKLEN];
	char		ircname[MAXLEN];
	char		login[MAXLEN];
	char		channel[MAXLEN];
	serv_struc	servers[10];
	int		current;
	int		server_sock;
/*
 * To add:
 * - Channellist
 * - DCC list
 */
}	vladbot;		

vladbot	bot;

int	read_serverlist(defaultserver, defaultport)
char	*defaultserver;
int	defaultport;
/*
 * read in all the servers from a file. Will (hopefully) disappear
 * soon
 */
{
	FILE	*serverfile;
	char	line[MAXLEN];
	char	*port;
	int	i=0;

	strcpy(bot.servers[i].servername, defaultserver);
	bot.servers[i].serverport=defaultport;
	i++;

	if((serverfile = fopen(SERVERFILE, "r")) == NULL)
		return(FALSE);

	while(fgets(line, MAXLEN, serverfile))
	{
		if(*(line+strlen(line)-1)=='\n')
			*(line+strlen(line)-1)='\0';
		strcpy(bot.servers[i].servername, strtok(line, " "));
		if((port = strtok(NULL, " "))==NULL)
			bot.servers[i].serverport=6667;
		else
			bot.servers[i].serverport=atoi(port);
		i++;
	}
	fclose(serverfile);
	bot.current=0;
	strcpy(bot.servers[i].servername, "");
	return(i);
}	

void	startbot( nick, name, login, channel )
char	*nick;
char	*name;
char	*login;
char	*channel;
/*
 * Initializes the bot-structure
 */
{
	strcpy(bot.nick, nick);
	strcpy(bot.ircname, name);
	strcpy(bot.login, login);
	strcpy(bot.channel, channel);
	while(!connect_to_server())
		sleep(1);
}

int	connect_to_server()
/*
 * connects to a server, and if it fails, andvances current
 */
{
#ifdef DBUG
	printf("Connecting to port %d of server %s\n", bot.servers[bot.current].serverport,
		bot.servers[bot.current].servername);
#endif
	close(bot.server_sock);
	if((bot.server_sock = connect_by_number(bot.servers[bot.current].serverport, 
	    bot.servers[bot.current].servername)) < 0)
	{
		if(*(bot.servers[bot.current+1].servername))
			bot.current++;
		else
			bot.current = 0;
		return(FALSE);
	}
	sendregister(bot.nick, bot.login, bot.ircname);
	join_channel(bot.channel);
	return(TRUE);
}

void	reconnect_to_server()
/*
 * connects to a server (i.e. after a kill) and rejoins all channels
 * in the channellist.
 */
{
	while(!connect_to_server())
		sleep(1);
	reset_channels(HARDRESET);
}

void	set_server_fds( reads, writes )
fd_set	*reads;
fd_set	*writes;
{
	FD_SET( bot.server_sock, reads );
}

int	serversock_set( reads )
fd_set	*reads;

{
	return(FD_ISSET(bot.server_sock, reads));
}

int 	readln( buf )
char 	*buf;

{
	return(read_from_socket( bot.server_sock, buf ));
}


/* 
 * send_to_server
 * The new way to write to the server... accepts formatted strings
 *
 * Actually, the old way. This routine does not support several servers
 * so it has to disappear.
 */

int 	send_to_server( format, arg1, arg2, arg3, arg4, arg5,
                        arg6, arg7, arg8,arg9, arg10 )

char 	*format;
char 	*arg1, *arg2, *arg3, *arg4, *arg5,
     	*arg6, *arg7, *arg8, *arg9, *arg10;

{
	return(send_to_socket(bot.server_sock, format, arg1, 
	       arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10));
}

