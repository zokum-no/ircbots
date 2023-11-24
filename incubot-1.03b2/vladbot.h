#ifndef VLADBOT_H
#define VLADBOT_H

#include <sys/types.h>
#include <netinet/in.h>
#include "session.h"
#include "userlist.h"
#include "channel.h"
#include "crc-ons.h"
#include "incu-ons.h"
#include "dcc.h"
#include "config.h"
#include "shitlist.h"

struct	liststruct
{
	char			*listname;
	char			*opperfile;
	char			*excludefile;
	char			*protfile;
	char			*shitfile;
	char			*passwdfile;
	char			*accessfile;
	char			*fpfile;
	char			*nickfile;
	userlist_t		**opperlist;
	EXCLUDE_list	**excludelist;
	userlist_t		**protlist;
	shitlist_t		**shitlist;
	userlist_t		**passwdlist;
	userlist_t		**accesslist;
};

typedef	struct liststruct	listinfo;

struct	botstruct	
{
	char		*botname;				/* to identify bot */
	char		nick[MAXNICKLEN];		/* nick on IRC */
	char		realnick[MAXNICKLEN];/* what nick should be */
	char		login[MAXNICKLEN];	/* desired login */
	char		*name;					/* "IRCNAME" */

	int		server_sock;

	int		num_of_servers;
	int		current_server;
	struct
	{
		char	*name;
		char	*realname;				/* server might have a different name */
		int	port;
	} serverlist[MAXSERVER];

	CHAN_list		*Channel_list;
	CHAN_list		*Current_chan;	/* Pointer to current channel */
	SPYUSER_list	*Spyuser_list; /* Pointer to spying users */
	DCC_list			*Client_list;
	SESSION_list	*session_list;
	HISTORY_list	*cmdhistory;
	int				num_cmdhist;

	char				*LocalHostName;
	struct in_addr	LocalHostAddr;
	char				*helpfile;
	char				*notefile;
	char				*uploaddir;
	char				*downloaddir;
	char				*indexfile;
	char				*eventfile;
	char				*mailtofile;
	listinfo			*lists;

	int				commandlvl[200];
	int				l_25_flags;
	long				uptime;
	long				lastreset;		/* last time bot was reset */
	long				lastping;		/* to check activity server */
	int				server_ok;

	char				*parent;			/* Not really needed, but nice to know */
	int				secure;
	int				talk;
	int				spylog;
	int				reply_ping;
	int				reply_version;
	int				reply_finger;
	int				floodprot_clone;	/* Action taken when clones are detected */
	int				floodprot_public;	/* Action taken for public floods */
	int				floodprot_nick;	/* Action taken for nick floods */
	int				floodprot_ctcp;	/* Action taken for ctcp floods */
	int				floodprot_op;		/* Action taken for "!op" from non-ops */
	int				floodprot_awp;		/* Action taken for "!awp" from non-ops */
	char				*lockpass;
	int				failurethreshold;
	int				killed;
	long				last_event_mail;
#ifdef BackedUp
	char				*backupnick;
	char				*backupbotpasswd;
#endif
#ifdef BackUp
	char				*backupmask;
	char     		*backuppasswd;
#endif
	char				socketbuf[1024]; /*1-23-99*/
	int				socketbuflen; /*1-23-99*/
};


typedef struct botstruct	botinfo;	

int		find_channel				(botinfo *bot, char *channel);
int		add_channel_to_bot		(botinfo *bot, char *channel, char *topic, char *mod);
int		find_server					(botinfo *bot, char *server, int port);
int		add_server_to_bot			(botinfo *bot, char *server, int port);
int		delete_server_from_bot	(botinfo *bot, char *server, int port);
void		bot_init						();
botinfo	*bot_created				(char *s);
botinfo	*add_bot						(char *s);
listinfo	*listset_created			(char *s);
listinfo	*add_listset				(char *s);
void		delete_listset				(char *s);
int		forkbot						(char *nick, char *login, char *name);
int		killbot						(char *reason);
void		quit_all_bots				(char *from, char *reason);
void		start_all_bots				();
int		connect_to_server			();
void		reconnect_to_server		();
void		set_server_fds				( fd_set *reads, fd_set *writes );
int		send_to_server				(char *format, ...);
int     	change_server				(int serverno);
void		try_reconnect				();
void		reset_botstate				();
void		parse_server_input		( fd_set *read_fds );
void		set_dcc_fds					( fd_set *read_fds, fd_set *write_fds );
void		parse_dcc_input			( fd_set *read_fds );
void		send_pings					();
void		pong_received				(char *nick, char *server);

void		parse_global				(char *from, char *to, char *rest);
void		global_not_imp				(char *from, char *rest);
void		global_die					(char *from, char *rest);
void		global_debug				(char *from, char *rest);
void		global_list					(char *from, char *rest);
void		check_timer_unban			(void);

#endif
