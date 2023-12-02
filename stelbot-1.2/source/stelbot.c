#include "stelbot.h"

StelBotStruct bot;

extern char serversfile[FNLEN];

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

	if((serverfile = fopen(serversfile, "r")) == NULL)
		return(FALSE);

	while(readln_from_a_file(serverfile, line))
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
	sendregister(bot.nick, bot.login, bot.ircname);

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
	return(TRUE);
}

void	reconnect_to_server()
/*
 * connects to a server (i.e. after a kill) and rejoins all channels
 * in the channellist.
 */
{
        if (*(bot.servers[bot.current+1].servername))
	  bot.current++;
	else
	  bot.current = 0;
	while(!connect_to_server())
		sleep(1);
	sendregister(bot.nick, bot.login, bot.ircname);
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


