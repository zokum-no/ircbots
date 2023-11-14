#ifndef VLADBOT_H
#define VLADBOT_H

#ifdef AIX
#	include <sys/select.h>
#endif /* AIX */
#include "session.h"
#include "userlist.h"
#include "channel.h"
#include "dcc.h"
#include "config.h"

struct	liststruct
{
	char		*listname;
	char		*opperfile;
	char		*protfile;
	char		*shitfile;
	USERLVL_list	**opperlist;
	USERLVL_list	**protlist;
	USERLVL_list	**shitlist;
};

typedef	struct liststruct	listinfo;

struct	botstruct	
{
	char		*botname; 		/* to identify bot */
	char		nick[MAXNICKLEN];	/* nick on IRC */
	char		realnick[MAXNICKLEN];	/* what nick should be */
	char		login[MAXNICKLEN];	/* desired login */
	char		*name;			/* "IRCNAME" */

	int		server_sock;	

	int		num_of_servers;
	int		current_server;
	struct
	{
		char	*name;
		char	*realname;	/* server might have a different name */
		int	port;
	} serverlist[MAXSERVER];

	CHAN_list	*Channel_list;
	CHAN_list	*Current_chan;	/* Pointer to current channel */
	DCC_list	*Client_list;
	SESSION_list	*session_list;

	char		*helpfile;
	char		*notefile;
	char		*uploaddir;
	char		*downloaddir;
	char		*indexfile;
	listinfo	*lists;

	long		uptime;
	long		lastreset;	/* last time bot was reset */
	long		lastping;	/* to check activity server */
	int		server_ok;
	
	char		*parent;	/* Not really needed, but nice to know */
};


typedef struct botstruct	botinfo;	

int	find_channel(botinfo *bot, char *channel);
int	add_channel_to_bot(botinfo *bot, char *channel, char *topic, char *mod);
int	find_server(botinfo *bot, char *server, int port);
int	add_server_to_bot(botinfo *bot, char *server, int port);
int	delete_server_from_bot(botinfo *bot, char *server, int port);
void	bot_init();
botinfo	*bot_created(char *s);
botinfo	*add_bot(char *s);
listinfo	*listset_created(char *s);
listinfo	*add_listset(char *s);
void	delete_listset(char *s);
int	forkbot(char *nick, char *login, char *name);
int	killbot(char *reason);
void	quit_all_bots(char *from, char *reason);
void	start_all_bots();
int	connect_to_server();
void	reconnect_to_server();
void	set_server_fds( fd_set *reads, fd_set *writes );
int 	readln( char *buf );
int	send_to_server(char *format, ...);
void	try_reconnect();
void	reset_botstate();
void	parse_server_input( fd_set *read_fds );
void	set_dcc_fds( fd_set *read_fds, fd_set *write_fds );
void	parse_dcc_input( fd_set *read_fds );
void	send_pings();
void	pong_received(char *nick, char *server);

void	parse_global(char *from, char *to, char *rest);
void	global_not_imp(char *from, char *rest);
void	global_die(char *from, char *rest);
void	global_debug(char *from, char *rest);
void	global_list(char *from, char *rest);

#endif
