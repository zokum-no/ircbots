#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>
#include "config.h"
#include "rockbot.h"
#include "misc.h"
#include "send.h"
#include "server.h"

RockBotStruct bot;

extern char serversfile[FNLEN];
extern char current_nick[MAXLEN];
int mysendq = 0;
time_t sendqtime, oldsendqtime;
time_t timeleft = 0, lasttime = 0;

void AddServer(char *server, unsigned short int port, char *pass)
{
  SERVER_list  *NewServPtr;
  SERVER_list  *ServPtr;

  if(!server)
  {
    Debug(DBGERROR, "Null server pointer in AddServer()!");
    return;
  }
  if(!pass)
    pass = "";
  if(!(NewServPtr = (SERVER_list *) malloc(sizeof(*NewServPtr))))
  {
    Debug(DBGERROR, "Out of ram adding server...");
    return;
  }

  strcpy(NewServPtr->name, server);
  strcpy(NewServPtr->password, pass);
  NewServPtr->port = port;
  NewServPtr->errors = 0;
  NewServPtr->connects = 0;
  NewServPtr->attempts = 0;
  NewServPtr->next = NULL;

/*
 * A harder add function because we want the servers to keep the
 * order they have in the data file, so scan to the end and then
 * add ours after the rest.
 */
  if(!bot.servers)
    bot.servers = NewServPtr;
  else
  {
    for(ServPtr=bot.servers;ServPtr->next;ServPtr=ServPtr->next);
    ServPtr->next = NewServPtr;
  }
}


void startbot(char *nick, char *name, char *login, char *channel)
/*
 * Initializes the bot-structure
 */
{
  strcpy(bot.nick, nick);
  strcpy(bot.ircname, name);
  strcpy(bot.login, login);
  strcpy(bot.channel, channel);
  bot.CurrentServer = bot.servers;
  while (!connect_to_server())
    sleep(2);
  sendregister(bot.nick, bot.login, bot.ircname);
}

int connect_to_server()
/*
 * connects to a server, and if it fails, andvances current
 */
{
  Debug(DBGINFO, "Connecting to port %d of server %s\n", bot.CurrentServer->port,
	 bot.CurrentServer->name);
  close(bot.server_sock);
  if ((bot.server_sock = connect_by_number(bot.CurrentServer->port,
					   bot.CurrentServer->name)) < 0)
  {
    if(bot.CurrentServer->next)
      bot.CurrentServer = bot.CurrentServer->next;
    else
      bot.CurrentServer = bot.servers;
    return (FALSE);
  }
  return (TRUE);
}

void reconnect_to_server()
/*
 * connects to a server (i.e. after a kill) and rejoins all channels
 * in the channellist.
 */
{
  clear_queue(); /* Wipe out any pending notices etc */
  if (bot.CurrentServer->next)
    bot.CurrentServer = bot.CurrentServer->next;
  else
    bot.CurrentServer = bot.servers;
  while(!connect_to_server())
    sleep(2);
  sendregister(current_nick, bot.login, bot.ircname);
}

void set_server_fds(reads, writes)
     fd_set *reads;
     fd_set *writes;
{
  FD_SET(bot.server_sock, reads);
}

int serversock_set(reads)
     fd_set *reads;
{
  return(FD_ISSET(bot.server_sock, reads));
}

int readln(char *buf)
{
  return(read_from_socket(bot.server_sock, buf));
}

/* 
 * send_to_server
 * The new way to write to the server... accepts formatted strings
 *
 * Actually, the old way. This routine does not support several servers
 * so it has to disappear.
 */
int send_to_server(char *format, ...)
{
   char entiremessage[WAYTOBIG];
   va_list args;

   va_start(args, format);
   vsnprintf(entiremessage, WAYTOBIG-1, format, args);
   va_end(args);


   return(send_to_socket(bot.server_sock, "%s", entiremessage));
}
