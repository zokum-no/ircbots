/***************************************************************
 Copyright (c)1996 by Robert Alto (badcrc)
Date.....: May 4 1996
Purpose..: To have a constant chat between all bots and allow
           users to chat between bots during net splits and
           takeovers.
Modified.: None.
***************************************************************/

#include <string.h>
#ifndef SYSV
#include <strings.h>
#endif
#include <stdarg.h>
#include <time.h>

#include "config.h"
#include "log.h"
#include "fnmatch.h"
#include "ftext.h"
#include "misc.h"
#include "send.h"
#include "dcc.h"
#include "parsing.h"
#include "crc-ons.h"
#include "vladbot.h"
#include "vlad-ons.h"
#include "incu-ons.h"
#include "botnet.h"

#ifdef BackUp
#include "mega-ons.h"
extern int Abackup;
#endif
#ifdef BackedUp
extern int Abackedup;
#endif

#undef BOTNET_DBG
 

/************* Global botnet variables ***************/
typedef struct BOTNET_struct
{
	char			*userhost; /* user@host */
	char			*servername; /* Name of the server they are on */
	int			local; /* 1 if local user i.e., on this bot */
	int			local_echo; /* 1 if the user has local echo ON */
	long			jointime; /* time they got on botnet */
	struct BOTNET_struct	*next;
	struct BOTNET_struct	*prev;
} BOTNET_list;

BOTNET_list	*botnet_serverlist = NULL;
BOTNET_list	*botnet_clientlist = NULL;

typedef struct BOTNETBAN_struct
{
	char	*banmask; /* mask of the person banned */
	char	*userhost; /* person who did the ban */
	struct BOTNETBAN_struct	*next;
	struct BOTNETBAN_struct	*prev;
} BOTNETBAN_list;

BOTNETBAN_list	*botnet_banlist = NULL;

typedef struct BOTNETMSG_struct
{
	long	time;		/* time the message was received */
	int	counter;	/* counter to indicate when to delete this id */
	char	*id;		/* unique identification for the msg */
	struct	BOTNETMSG_struct *next;
	struct	BOTNETMSG_struct *prev;
} BOTNETMSG_list;

BOTNETMSG_list	*botnet_msglist = NULL;

struct	TOPIC_struct
{
	char	*text;
	char	*setby;
	long	timeset;
} topic = { NULL, NULL, 0 };

struct	BOTNETSTATS_struct
{
	int	local_kicks;
	int	local_clients;
	int	local_privmsgs;
	int	local_pubmsgs;
	int	local_servers;
	int	local_servjoins;
	int	local_servquits;
	int	remote_clients;
	int	remote_kicks;
	int	remote_privmsgs;
	int	remote_pubmsgs;
	int	remote_servers;
	int	remote_servjoins;
	int	remote_servquits;
} botnetstats = { 0, 0, 0, 0 };

extern	botinfo	*currentbot;

/************* Local function prototypes **************/
void		sbotnet_start			(char *, char *, char *);
void		sbotnet_startack		(char *, char *, char *);
void		sbotnet_speaktome		(char *, char *, char *);
void		update_local_banlist		(char *, char *, char *);
void		update_remote_banlist		(char *);
void		update_local_clientlist		(char *, char *, char *);
void		update_remote_clientlist	(char *);
void		update_local_serverlist		(char *, char *, char *);
void		update_remote_serverlist	(char *);
void		update_local_topic		(char *, char *, char *);
void		update_remote_topic		(char *);
int		add_botnet_client		(char *, char *, int, long);
int		add_botnet_server		(char *,int,long);
int		delete_botnet_client		(char *);
void		delete_botnet_clients_from_server	(char *);
int		delete_botnet_server		(char *);
BOTNET_list	*new_botnet_client		(char *, char *, int, long);
void		send_to_botnet_clients		(char *, char *, ...);
void		send_to_botnet_servers		(char *, char *, char *, ...);
void		cbotnet_ban			(char *, char *, char *);
void		cbotnet_banuser			(char *, char *, char *);
void		sbotnet_banuser			(char *, char *, char *);
void		cbotnet_action			(char *, char *, char *);
void		sbotnet_action			(char *, char *, char *);
void		cbotnet_bankick			(char *, char *, char *);
void		sbotnet_bankick			(char *, char *, char *);
void		cbotnet_join			(char *, char *, char *);
void		sbotnet_join			(char *, char *, char *);
void		cbotnet_kick			(char *, char *, char *);
void		sbotnet_kick			(char *, char *, char *);
void		cbotnet_privmsg			(char *, char *, char *);
void		sbotnet_privmsg			(char *, char *, char *);
void		cbotnet_pubmsg			(char *, char *, char *);
void		sbotnet_pubmsg			(char *, char *, char *);
void		cbotnet_quit			(char *, char *, char *);
void		sbotnet_quit			(char *, char *, char *);
void		cbotnet_echo			(char *, char *, char *);
void		cbotnet_who			(char *, char *, char *);
void		cbotnet_servers			(char *, char *, char *);
void		cbotnet_stats			(char *, char *, char *);
void		cbotnet_topic			(char *, char *, char *);
void		cbotnet_users			(char *, char *, char *);
void		cbotnet_rban			(char *, char *, char *);
void		sbotnet_rban			(char *, char *, char *);
void		cbotnet_showbans		(char *, char *, char *);
void		sbotnet_servquit		(char *, char *, char *);
void		sbotnet_servjoin		(char *, char *, char *);
int		add_botnet_ban			(char *, char *);
void		add_botnet_kick			(char *);
int		delete_botnet_ban		(char *);
BOTNETBAN_list	*new_botnet_ban			(char *, char *);
BOTNETBAN_list	*search_botnet_banlist		(char *);
BOTNET_list	*search_botnet_nick		(char *);
BOTNET_list	*search_botnet_client		(char *);
BOTNET_list	*search_botnet_server		(char *);
void		show_botnet_help		(char *, char *, char *);

command_tbl botnet_commands [] =
{
/* General botnet user commands */
	{ "BAN",	cbotnet_ban,	100,	0,	TRUE  },
	{ "BANUSER",	cbotnet_banuser,100,	0,	TRUE  },
	{ "BANKICK",	cbotnet_bankick,100,	0,	TRUE  },
	{ "BOTNET",	cbotnet_join,	0,	0,	TRUE  },
	{ "ECHO",	cbotnet_echo,	0,	0,	TRUE  },
	{ "HELP",	show_botnet_help,0,	0,	TRUE  },
	{ "KICK",	cbotnet_kick,	100,	0,	TRUE  },
	{ "ME",		cbotnet_action,	0,	0,	TRUE  },
	{ "MSG",	cbotnet_privmsg,0,	0,	TRUE  },
	{ "QUIT",	cbotnet_quit,	0,	0,	TRUE  },
	{ "RBAN",	cbotnet_rban,	0,	0,	TRUE  },
	{ "SAY",	cbotnet_pubmsg,	0,	0,	TRUE  },
	{ "SERVERS",	cbotnet_servers,0,	0,	TRUE  },
	{ "SHOWBANS",	cbotnet_showbans,50,	0,	TRUE  },
	{ "STATS",	cbotnet_stats,	0,	0,	TRUE  },
	{ "TOPIC",	cbotnet_topic,	0,	0,	TRUE  },
	{ "USERS",	cbotnet_users,	0,	0,	TRUE  },
	{ "WHO",	cbotnet_who,	0,	0,	TRUE  },
/* Commands for inter-bot communication */
	{ "BNACTION",	sbotnet_action,	OTHER_BOT_LEVEL,	0,	TRUE  },
	{ "BNADDBAN",	update_local_banlist,	OTHER_BOT_LEVEL,0,	TRUE  },
	{ "BNADDCLIENT",update_local_clientlist,OTHER_BOT_LEVEL,0,	TRUE  },
	{ "BNADDSERVER",update_local_serverlist,OTHER_BOT_LEVEL,0,	TRUE  },
	{ "BNADDTOPIC",	update_local_topic,	OTHER_BOT_LEVEL,0,	TRUE  },
	{ "BNBANUSER",	sbotnet_banuser,OTHER_BOT_LEVEL,	0,	TRUE  },
	{ "BNBANKICK",	sbotnet_bankick,OTHER_BOT_LEVEL,	0,	TRUE  },
	{ "BNJOIN",	sbotnet_join,	OTHER_BOT_LEVEL,	0,	TRUE  },
	{ "BNKICK",	sbotnet_kick,	OTHER_BOT_LEVEL,	0,	TRUE  },
	{ "BNPUBMSG",	sbotnet_pubmsg,	OTHER_BOT_LEVEL,	0,	TRUE  },
	{ "BNPRIVMSG",	sbotnet_privmsg,OTHER_BOT_LEVEL,	0,	TRUE  },
	{ "BNQUIT",	sbotnet_quit,	OTHER_BOT_LEVEL,	0,	TRUE  },
	{ "BNRBAN",	sbotnet_rban,	OTHER_BOT_LEVEL,	0,	TRUE  },
	{ "BNSERVJOIN",	sbotnet_servjoin,OTHER_BOT_LEVEL,	0,	TRUE  },
	{ "BNSERVQUIT",	sbotnet_servquit,OTHER_BOT_LEVEL,	0,	TRUE  },
	{ "BNSTART",	sbotnet_start,	OTHER_BOT_LEVEL,	0,	FALSE },
	{ "BNSTARTACK",	sbotnet_startack,OTHER_BOT_LEVEL,	0,	FALSE },
	{ "SPEAKTOME",	sbotnet_speaktome,OTHER_BOT_LEVEL,	0,	TRUE  },
		  { NULL,         null(void(*)()), 0,     0,      FALSE }
};


int		on_botnet_command	(char *command, char *from, char *to, char *msg, char *msg_copy)
{
	int i;
	DCC_list	*userclient;

	if (ischannel(to))
		return FALSE;

	for (i=0; botnet_commands[i].name != NULL; i++)
	{
		if (STRCASEEQUAL(botnet_commands[i].name,command))
		{
			if (userlevel(from) < botnet_commands[i].userlevel)
				return FALSE;

			if (shitlevel(from) > botnet_commands[i].shitlevel &&
				excludelevel(from)==0)
				return FALSE;

			if (botnet_commands[i].userlevel == OTHER_BOT_LEVEL &&
				userlevel(from) != OTHER_BOT_LEVEL)
				return FALSE;

			userclient = search_client_chat(from);

			if(botnet_commands[i].forcedcc &&
				(!userclient || (userclient->flags&DCC_WAIT)))
								{
#ifdef AUTO_DCC
				dcc_chat(from, msg);
				nodcc_session(from, to, msg_copy);
/*				sendnotice(getnick(from),
					"Please type: /dcc chat %s", currentbot->nick);
*/
#else
				if (userlevel(from)!=OTHER_BOT_LEVEL)
				{
					sendnotice(getnick(from),
						"Sorry, %s is only available through DCC", command);
					sendnotice(getnick(from),
						"Please start a dcc chat connection first");
				}
#endif /* AUTO_DCC */
				return FALSE;
			}
#ifdef BOTNET_DBG
			printf("on_botnet_command: name[%s] from[%s]\n",botnet_commands[i].name,from);
#endif
			botnet_commands[i].function(from,to,*msg?msg:NULL);
			return TRUE;
		}
	}

	if (userlevel(from)==OTHER_BOT_LEVEL)
		return TRUE;

	cbotnet_pubmsg(from,to,msg_copy);

	return TRUE;
}

void	botnet_start	(char *nick)
/*
 * This function is called from parse.c when the bot joins a channel and
 * begins to parse the WHO command of channel users.
 */
{
	if (userlevel(username(nick))!=OTHER_BOT_LEVEL ||
		search_client_chat(username(nick)) ||
		STRCASEEQUAL(nick,currentbot->nick))
		return;

#ifdef BOTNET_DBG
printf("botnet_start: nick[%s]\n",nick);
#endif

	dcc_chat(username(nick), NULL);
	nodcc_session(username(nick), NULL, NULL);
	msend_to_user(username(nick),"%cBNSTART %s",BOTNET_CHAR,currentbot->nick);
}

void	sbotnet_start	(char *from, char *to, char *rest)
/*
 * This function is called when another bot is requesting to get
 * connected to BOTNet.
 */
{
#ifdef BOTNET_DBG
printf("sbotnet_start: from[%s]\n",from);
#endif

	if (STRCASEEQUAL(getnick(from),currentbot->nick))
		return;

	if (!add_botnet_server(from,1,time(NULL)))
		return;

	send_to_botnet_clients(currentbot->nick,
		"[BOTNet:SERVJOIN] %s has connected to %s",
		from,username(currentbot->nick));
	send_to_botnet_servers(username(currentbot->nick),from,
		"%cBNSERVJOIN %s %s",
		BOTNET_CHAR,username(currentbot->nick),from);

	msend_to_user(from,"%cBNSTARTACK %s",BOTNET_CHAR,currentbot->nick);
	update_remote_serverlist(from);
	update_remote_clientlist(from);
	update_remote_banlist(from);
	update_remote_topic(from);
}

void	sbotnet_startack		(char *from, char *to, char *rest)
/*
 * This function is called when another bot has received our request
 * to be connected to BOTNet and has responded.  Things should be OK.
 */
{
	if (!add_botnet_server(from,1,time(NULL)))
		return;

	send_to_botnet_clients(currentbot->nick,
		"[BOTNet:SERVJOIN] %s has established link to %s",
		username(currentbot->nick), from);
	send_to_botnet_servers(username(currentbot->nick),from,
		"%cBNSERVJOIN %s %s",
		BOTNET_CHAR,username(currentbot->nick),from);
}

void	check_botnet_server		(char *remoteserver)
/*
 * This function is called once in a while to verify that we
 * have established a link to all bots in the channel.
 */
{
	DCC_list	*Client;
	long		idletime;

	if (STRCASEEQUAL(getnick(remoteserver),currentbot->nick))
		return;

#ifdef BOTNET_DBG
printf("check_botnet_server: remoteserver[%s]\n",remoteserver);
#endif

	/* If we don't have a chat with the other bot, start one */
	if ((Client = search_client_chat(remoteserver))==NULL)
	{
		dcc_chat(remoteserver, NULL);
		nodcc_session(remoteserver, NULL, NULL);
		sendprivmsg(getnick(remoteserver),
			"%cBNSTART %s",BOTNET_CHAR,currentbot->nick);
		return;
	}

	sbotnet_start(remoteserver,NULL,NULL);

	/* If we do have a chat already, check idle times */
	idletime = (time(NULL)-Client->lasttime);

	if (idletime > 300)
		delete_client(Client);
	else if (idletime > 240)
		send_to_user(remoteserver,"%cSPEAKTOME %s",
			BOTNET_CHAR,currentbot->nick);
}

void	botnet_dcc_close	(char *userhost)
/*
 * This function is called from dcc.c when a dcc chat is closed.
 *
 */
{

	char	*userhostcopy;

	userhostcopy = strdup(userhost);
#ifdef BOTNET_DBG
printf("botnet_dcc_close: userhostcopy[%s]\n",userhostcopy);
#endif

	if (delete_botnet_client(userhostcopy))
	{
		send_to_botnet_clients(currentbot->nick,
			"[BOTNet:QUIT] %s has left from %s",
			userhostcopy,username(currentbot->nick));
		send_to_botnet_servers(username(currentbot->nick),
			username(currentbot->nick),
			"%cBNQUIT %s",
			BOTNET_CHAR,
			userhostcopy);
	}
	else
	{
		delete_botnet_clients_from_server(userhostcopy);

		if (delete_botnet_server(userhostcopy))
		{
			send_to_botnet_clients(currentbot->nick,
				"[BOTNet:SERVQUIT] %s has disconnected from %s",
				userhostcopy,currentbot->nick);
			send_to_botnet_servers(username(currentbot->nick),
				username(currentbot->nick),
				"%cBNSERVQUIT %s %s",
				BOTNET_CHAR,
				username(currentbot->nick),
				userhostcopy);
		}
	}

}

void	sbotnet_speaktome	(char *from, char *to, char *rest)
{
	msend_to_user(from,"I'm here %s",getnick(from));
}

void	update_local_serverlist		(char *from, char *to, char *rest)
{
	char		*servername;
	long		jointime;

	if ((servername=get_token(&rest," "))==NULL)
		return;

	jointime = atol(rest);

	if (jointime<=0)
		jointime = time(NULL);

	add_botnet_server(servername,0,jointime);
}

void	update_remote_serverlist	(char *remoteserver)
{
	BOTNET_list	*botnetptr;

	for (botnetptr = botnet_serverlist; botnetptr;
		botnetptr = botnetptr->next)
	{
		/* Don't tell the remote server to add himself ;) */
		if (!STRCASEEQUAL(botnetptr->userhost,remoteserver))
			msend_to_user(remoteserver,"%cBNADDSERVER %s %ld",
				BOTNET_CHAR,
				botnetptr->userhost,
				botnetptr->jointime);
	}
}

void	update_local_clientlist		(char *from, char *to, char *rest)
{
	char	*userhost;
	char	*servername;
	long	jointime;

	if ((userhost=get_token(&rest," "))==NULL)
		return;

	if ((servername=get_token(&rest," "))==NULL)
		return;

	jointime = atol(rest);

	if (jointime < 0)
		jointime = time(NULL);

	add_botnet_client(userhost,servername,0,jointime);
}

void	update_remote_clientlist	(char *remoteserver)
{
	BOTNET_list	*botnetptr;

	for (botnetptr = botnet_clientlist; botnetptr;
		botnetptr = botnetptr->next)
	{
		msend_to_user(remoteserver,"%cBNADDCLIENT %s %s %ld",
			BOTNET_CHAR,
			botnetptr->userhost,
			botnetptr->servername,
			botnetptr->jointime);
	}
}

void	update_local_banlist	(char *from, char *to, char *rest)
{
	char	*banmask;
	char	*origin;
	char	*userhost;

	if ((origin=get_token(&rest," "))==NULL)
		return;
	if ((banmask=get_token(&rest," "))==NULL)
		return;
	if ((userhost=get_token(&rest," "))==NULL)
		return;

	add_botnet_ban(userhost,banmask);

	send_to_botnet_servers(origin, from,
		"%cBNADDBAN %s %s %s",
		BOTNET_CHAR, origin, banmask, userhost);
}

void	update_remote_banlist		(char *remoteserver)
{
	BOTNETBAN_list	*botnetbanptr;

	for (botnetbanptr = botnet_banlist; botnetbanptr;
		botnetbanptr = botnetbanptr->next)
	{
		msend_to_user(remoteserver,"%cBNADDBAN %s %s %s",
			BOTNET_CHAR,
			username(currentbot->nick),
			botnetbanptr->banmask,
			botnetbanptr->userhost);
	}
}

void	update_local_topic		(char *from, char *to, char *rest)
{
	char	*origin;
	char	*topic_owner;
	char	*topic_time;

	if ((origin=get_token(&rest," "))==NULL)
		return;
	if ((topic_owner=get_token(&rest," "))==NULL)
		return;
	if ((topic_time=get_token(&rest," "))==NULL)
		return;

/*
	if (STRCASEEQUAL(topic.text,rest))
		return;
*/

	free(topic.text);
	free(topic.setby);
	mstrcpy(&topic.setby,topic_owner);
	mstrcpy(&topic.text,rest);
	topic.timeset = atol(topic_time);

	if (topic.timeset <= 0)
		topic.timeset = time(NULL);

	send_to_botnet_clients(from,
		"[BOTNet:TOPIC] %s has changed the topic to: %s",
		topic.setby,topic.text);

	send_to_botnet_servers(origin, from, "%cBNADDTOPIC %s %s %ld %s",
		BOTNET_CHAR, origin, topic.setby, topic.timeset, topic.text);
}

void	update_remote_topic		(char *remoteserver)
{
	if (topic.timeset <= 0)
		return;

	msend_to_user(remoteserver, "%cBNADDTOPIC %s %s %ld %s",
		BOTNET_CHAR,username(currentbot->nick),
		topic.setby,topic.timeset,topic.text);
}

BOTNET_list	*search_botnet_client	(char *userhost)
/*
 * Searches the clientlist for a match to userhost.
 *
 */
{
	BOTNET_list	*botnetptr;
	char		userhostcopy[MAXLEN];
	char		*user;

	user=userhost;

	while (*user!='!')
		user++;

	user++;
	sprintf(userhostcopy,"*!%s",user);

	for (botnetptr = botnet_clientlist; botnetptr;
		botnetptr = botnetptr->next)
		if (!mymatch(userhostcopy,botnetptr->userhost,FNM_CASEFOLD))
			return (botnetptr);

	return NULL;
}

BOTNET_list     *search_botnet_server   (char *userhost)
/*
 * Searches the serverlist for a match to userhost.
 *
 */
{
        BOTNET_list     *botnetptr;
	char		userhostcopy[MAXLEN];
	char		*user;

	user = userhost;

	while (*user!='!')
		user++;

	user++;
	sprintf(userhostcopy,"*!%s",user);

        for (botnetptr = botnet_serverlist; botnetptr;
		botnetptr = botnetptr->next)
	{
		if (!mymatch(userhostcopy,botnetptr->userhost,FNM_CASEFOLD))
                        return (botnetptr);
	}

        return NULL;
}

int	add_botnet_client	(	char	*userhost,
					char	*servername,
					int	local,
					long	jointime)
/*
 * Creates a new botnet client and adds it to the clientlist.
 *
 */
{
	BOTNET_list	*botnetptr;

	if (search_botnet_client(userhost))
		return FALSE;

	if ( (botnetptr=new_botnet_client(userhost,servername,local,jointime)) == NULL )
		return FALSE;

#ifdef BOTNET_DBG
printf("add_botnet_client: userhost[%s] servername[%s] local[%d]\n",userhost,servername,local);
#endif
	if (botnet_clientlist==NULL)
		botnet_clientlist = botnetptr;
	else
	{
		BOTNET_list	*botnetptr2;

		botnetptr2=botnet_clientlist;

		while (botnetptr2->next!=NULL)
			botnetptr2=botnetptr2->next;

		botnetptr->prev = botnetptr2;
		botnetptr2->next = botnetptr;
	}

	if (local)
		botnetstats.local_clients++;
	else
		botnetstats.remote_clients++;

	return TRUE;
}

int     add_botnet_server       (char *userhost, int local, long jointime)
/*
 * Creates a new server and adds it to the serverlist.
 *
 */
{
        BOTNET_list     *botnetptr;

        if ( (botnetptr=search_botnet_server(userhost))!=NULL )
	{
		if (search_client_chat(userhost))
			botnetptr->local = 1;
                return FALSE;
	}

        if ( (botnetptr=new_botnet_client(userhost,NULL,local,jointime)) == NULL )
                return FALSE;

#ifdef BOTNET_DBG
printf("add_botnet_server: userhost[%s]\n",userhost);
#endif

        if (botnet_serverlist==NULL)
                botnet_serverlist = botnetptr;
        else
        {
                BOTNET_list     *botnetptr2;

                botnetptr2=botnet_serverlist;

                while (botnetptr2->next!=NULL)
                        botnetptr2=botnetptr2->next;

                botnetptr->prev = botnetptr2;
                botnetptr2->next = botnetptr;
        }

	botnetstats.remote_servers++;
	return TRUE;
}
 
int		delete_botnet_client	(char *userhost)
/*
 * Deletes a client from the client list.
 *
 *
 */
{
	BOTNET_list	*botnetptr;
	BOTNET_list	*botnetptr2;

	if ( (botnetptr = search_botnet_client(userhost)) == NULL)
		return FALSE;

#ifdef BOTNET_DBG
printf("delete_botnet_client: userhost[%s]\n",userhost);
#endif

	if (botnetptr->prev == NULL)
	{
		botnet_clientlist = botnetptr->next;

		if (botnetptr->next != NULL)
			botnet_clientlist->prev = NULL;
	}
	else
	{
		botnetptr2 = botnetptr->prev;
		botnetptr2->next = botnetptr->next;

		if (botnetptr->next!=NULL)
		{
			botnetptr2 = botnetptr->next;
			botnetptr2->prev = botnetptr->prev;
		}
	}

#ifdef BOTNET_DBG
printf("delete_botnet_client: botnetptr->local[%d] clients[%d/%d local/remote]\n",
	botnetptr->local,botnetstats.local_clients,botnetstats.remote_clients);
#endif

	if (botnetptr->local)
	{
		if (botnetstats.local_clients > 0)
			botnetstats.local_clients--;
	}
	else
	{
		if (botnetstats.remote_clients > 0)
			botnetstats.remote_clients--;
	}

	free(botnetptr->userhost);
	free(botnetptr->servername);
	free(botnetptr);
#ifdef BOTNET_DBG
printf("delete_botnet_client(END): clients[%d/%d local/remote]\n",
	userhost,botnetstats.local_clients,botnetstats.remote_clients);
#endif
	return TRUE;
}

void		delete_botnet_clients_from_server	(char *servername)
/*
 * Deletes all clients that were using servername from the local
 * client list after a server has left botnet.
 *
 */
{
	BOTNET_list	*botnetptr;
	BOTNET_list	*botnetptr2;
	char		userhostcopy[MAXLEN];
	char		servhostcopy[MAXLEN];

	botnetptr = botnet_clientlist;

	while (botnetptr != NULL)
	{
		if (STRCASEEQUAL(botnetptr->servername,servername))
		{
			if (botnetptr->prev==NULL)
			{
				botnet_clientlist = botnetptr->next;
				if (botnetptr->next != NULL)
					botnet_clientlist->prev = NULL;
				botnetptr2 = botnetptr->next;
			}
			else
			{
				botnetptr2 = botnetptr->prev;
				botnetptr2->next = botnetptr->next;

				if (botnetptr->next!=NULL)
				{
					botnetptr2 = botnetptr->next;
					botnetptr2->prev = botnetptr->prev;
				}
			}

			if (botnetptr->local)
			{
				if (botnetstats.local_clients > 0)
					botnetstats.local_clients--;
			}
			else if (botnetstats.remote_clients > 0)
					botnetstats.remote_clients--;

			strcpy(userhostcopy,botnetptr->userhost);
			strcpy(servhostcopy,botnetptr->servername);

			free(botnetptr->userhost);
			free(botnetptr->servername);
			free(botnetptr);
			botnetptr = botnetptr2;

			send_to_botnet_clients(currentbot->nick,
				"[BOTNet:QUIT] %s has left from %s",
				userhostcopy,servhostcopy);
		}
		else
			botnetptr = botnetptr->next;
	}
}

int             delete_botnet_server    (char *userhost)
{
        BOTNET_list     *botnetptr;

        if ( (botnetptr = search_botnet_server(userhost)) == NULL)
                return FALSE;

#ifdef BOTNET_DBG
printf("delete_botnet_server: userhost[%s]\n",userhost);
#endif

        if (botnetptr->prev == NULL)
	{
                botnet_serverlist = botnetptr->next;
		if (botnetptr->next != NULL)
			botnet_serverlist->prev = NULL;
	}
        else
        {
	        BOTNET_list     *botnetptr2;

                botnetptr2 = botnetptr->prev;
                botnetptr2->next = botnetptr->next;

		if (botnetptr->next!=NULL)
		{
			botnetptr2 = botnetptr->next;
			botnetptr2->prev = botnetptr->prev;
		}
        }

	if (botnetstats.remote_servers > 0)
		botnetstats.remote_servers--;

        free(botnetptr->userhost);
	free(botnetptr->servername);
        free(botnetptr);
        return TRUE;
}

BOTNET_list	*new_botnet_client	(	char	*userhost,
						char	*servername,
						int	local,
						long	jointime)
{
	BOTNET_list	*botnetptr;

	if ( (botnetptr = (BOTNET_list*)malloc(sizeof(*botnetptr))) == NULL)
		return NULL;

	mstrcpy(&botnetptr->userhost,userhost);
	mstrcpy(&botnetptr->servername,servername);
	botnetptr->local = local;
	botnetptr->local_echo = 1;

	if (jointime<=0)
		botnetptr->jointime = time(NULL);
	else
		botnetptr->jointime = jointime;

	botnetptr->next = NULL;
	botnetptr->prev = NULL;
	return(botnetptr);
}

void	send_to_botnet_clients	(char *from, char *format, ...)
{
	BOTNET_list	*botnetptr;
        char		buf[WAYTOBIG];
        va_list		msg;

        va_start(msg, format);
        vsprintf(buf, format, msg);
        va_end(msg);

	botnetptr = botnet_clientlist;

	while (botnetptr != NULL)
	{
		if (botnetptr->local)
		{
			if (search_client_chat(botnetptr->userhost))
			{
				if (!STRCASEEQUAL(from,botnetptr->userhost) ||
					botnetptr->local_echo)
					send_to_user(botnetptr->userhost,"%s",buf);
			}
		}
		botnetptr = botnetptr->next;
	}
}

void	send_to_botnet_servers	(char *origin, char *from, char *format, ...)
{
        BOTNET_list     *botnetptr;
        char            buf[WAYTOBIG];
        va_list         msg;

        va_start(msg, format);
        vsprintf(buf, format, msg);
        va_end(msg);

        botnetptr = botnet_serverlist;

        while (botnetptr != NULL)
        {
		if (botnetptr->local)
		{
			if (search_client_chat(botnetptr->userhost) &&
				!STRCASEEQUAL(origin,botnetptr->userhost) &&
				!STRCASEEQUAL(from,botnetptr->userhost))
			{
				send_to_user(botnetptr->userhost,"%s",buf);
			}
		}
		botnetptr = botnetptr->next;
        }
}

void	sbotnet_servjoin	(char *from, char *to, char *rest)
{
	char *origin;
	char *newserver;

	if (!rest)
		return;

	if ((origin=get_token(&rest," "))==NULL)
		return;

	if ((newserver=get_token(&rest," "))==NULL)
		return;

#ifdef BOTNET_DBG
printf("sbotnet_servjoin: rest[%s] from[%s]\n",rest,from);
#endif

	send_to_botnet_clients(currentbot->nick,
		"[BOTNet:SERVJOIN] %s has connected to %s",
		newserver,origin);

	send_to_botnet_servers(origin,from,"%cBNSERVJOIN %s %s",
		BOTNET_CHAR,origin,newserver);
}

void	sbotnet_servquit	(char *from, char *to, char *rest)
{
	char *origin;

	if ((origin=get_token(&rest," "))==NULL)
		return;

	if (!rest)
		return;

#ifdef BOTNET_DBG
printf("sbotnet_servquit: rest[%s] from[%s]\n",rest,from);
#endif

	send_to_botnet_clients(currentbot->nick,
		"[BOTNet:SERVQUIT] %s has disconnected from %s",
		rest,origin);
}

/*
void	cbotnet_help	(char *from, char *to, char *rest)
{
	if (!search_botnet_client(from))
	{
		send_to_user(from, "You're not on BOTNet!");
		return;
	}

	send_to_user(from," BOTNet Help ");
	send_to_user(from," %c%-15s - %s",BOTNET_CHAR,"BAN <mask>","Bans an address");
	send_to_user(from," %c%-15s - %s",BOTNET_CHAR,"BANKICK <nick>","Bans and kicks a user");
	send_to_user(from," %c%-15s - %s",BOTNET_CHAR,"BANUSER <nick>","Bans a user");
	send_to_user(from," %c%-15s - %s",BOTNET_CHAR,"ECHO","Toggles local echo on/off");
	send_to_user(from," %c%-15s - %s",BOTNET_CHAR,"KICK <nick>","Kicks <nick> off BOTNet");
	send_to_user(from," %c%-15s - %s",BOTNET_CHAR,"ME <msg>","Displays an action from you");
	send_to_user(from," %c%-15s - %s",BOTNET_CHAR,"MSG <nick>","Send a private msg to a user");
	send_to_user(from," %c%-15s - %s",BOTNET_CHAR,"RBAN #","Unbans the mask in slot # from SHOWBANS");
	send_to_user(from," %c%-15s - %s",BOTNET_CHAR,"SAY <msg>","Sends <msg> to everyone");
	send_to_user(from," %c%-15s - %s",BOTNET_CHAR,"STATS","Displays current clients/servers");
	send_to_user(from," %c%-15s - %s",BOTNET_CHAR,"SERVERS","Display currently connected servers");
	send_to_user(from," %c%-15s - %s",BOTNET_CHAR,"SHOWBANS","Displays current bans");
	send_to_user(from," %c%-15s - %s",BOTNET_CHAR,"TOPIC [topic]","Displays OR sets the topic");
	send_to_user(from," %c%-15s - %s",BOTNET_CHAR,"USERS","Display users with server details");
	send_to_user(from," %c%-15s - %s",BOTNET_CHAR,"WHO","Display users with address details");
}
*/

void	cbotnet_join	(char *from, char *to, char *rest)
{
	if (search_botnet_client(from))
	{
		send_to_user(from, "You're already on BOTNet!");
		return;
	}

	if (search_botnet_banlist(from))
	{
		send_to_user(from, "You're banned from BOTNet!");
		return;
	}

#ifdef BOTNET_DBG
printf("cbotnet_join: from[%s]\n",from);
#endif

	add_botnet_client(from, username(currentbot->nick), 1, time(NULL));

	send_to_botnet_clients(currentbot->nick,
		"[BOTNet:JOIN] %s has joined from %s",
		from, username(currentbot->nick));

	send_to_botnet_servers(username(currentbot->nick),
		username(currentbot->nick),
		"%cBNJOIN %s %s",BOTNET_CHAR,username(currentbot->nick),from);

	send_to_user(from, "Hello %s, Welcome to BOTNet. The prefix command character is '%c'",getnick(from),BOTNET_CHAR);
	send_to_user(from, "This server is running %s.", VERSION);
	send_to_user(from, "There is currently %d/%d local/remote clients, and %d servers.",
		botnetstats.local_clients, botnetstats.remote_clients,
		botnetstats.remote_servers+1);

	if (topic.timeset <= 0)
		send_to_user(from,"No topic is currently set");
	else
		send_to_user(from,
			"Topic set by %s on %s: %s",
			topic.setby,get_ctime(topic.timeset),topic.text);
}

void	sbotnet_join	(char *from, char *to, char *rest)
{
	char *client;
	char *origin;

	if ((origin=get_token(&rest," "))==NULL)
		return;

	if ((client=get_token(&rest," "))==NULL)
		return;

	if (strstr(client,"@")==NULL)
		return;

#ifdef BOTNET_DBG
printf("sbotnet_join: client[%s] from[%s]\n",client,from);
#endif

	add_botnet_client(client, from, 0, time(NULL));

	send_to_botnet_clients(currentbot->nick,
		"[BOTNet:JOIN] %s has joined from %s",
		client, origin);

	send_to_botnet_servers(origin, from,
		"%cBNJOIN %s %s",BOTNET_CHAR,origin,client);
}

void	cbotnet_quit	(char *from, char *to, char *rest)
{
	char	*user;

	user = strdup(from);

#ifdef BOTNET_DBG
printf("cbotnet_quit: user[%s]\n",user);
#endif

	if (delete_botnet_client(user))
	{
		send_to_botnet_clients(user,
			"[BOTNet:QUIT] %s has left from %s",
			user,username(currentbot->nick));
	}

	send_to_botnet_servers(username(currentbot->nick),
		username(currentbot->nick),
		"%cBNQUIT %s %s",BOTNET_CHAR,username(currentbot->nick),user);
}

void	sbotnet_quit	(char *from, char *to, char *rest)
{
	char *client;
	char *origin;

	if ((origin=get_token(&rest," "))==NULL)
		return;
	if ((client=get_token(&rest," "))==NULL)
		return;
	if (strstr(client,"@")==NULL)
		return;

#ifdef BOTNET_DBG
printf("sbotnet_quit: client[%s] from[%s]\n",client,from);
#endif

	if (delete_botnet_client(client))
		send_to_botnet_clients(from,
			"[BOTNet:QUIT] %s has left from %s",
			client, from);

	send_to_botnet_servers(origin, from,
		"%cBNQUIT %s %s",BOTNET_CHAR,origin,client);
}

void	cbotnet_pubmsg	(char *from, char *to, char *rest)
{
	if (!search_botnet_client(from))
	{
	/*	send_to_user(from,"You're not on BOTNet!");*/
		return;
	}

	if (!rest)
		return;

	send_to_botnet_clients(from,
		"<%s> %s", getnick(from), rest);

	send_to_botnet_servers(username(currentbot->nick),
		username(currentbot->nick),
		"%cBNPUBMSG %s %s %s", BOTNET_CHAR,
		username(currentbot->nick), from, rest);
}

void	sbotnet_pubmsg	(char *from, char *to, char *rest)
{
	char	*origin;
	char	*user;

	if ((origin=get_token(&rest," "))==NULL)
		return;
	if ((user=get_token(&rest," "))==NULL)
		return;

	if (!rest)
		return;

	send_to_botnet_clients(from,
		"<%s> %s", getnick(user), rest);

	send_to_botnet_servers(origin, from,
		"%cBNPUBMSG %s %s %s",
		BOTNET_CHAR, origin, user, rest);
}

void	cbotnet_action	(char *from, char *to, char *rest)
{

	if (!search_botnet_client(from))
	{
		send_to_user(from,"You're not on BOTNet!");
		return;
	}
        if (!rest)
                return;

        send_to_botnet_clients(from,
		"* %s %s", getnick(from), rest);

        send_to_botnet_servers(username(currentbot->nick),
		username(currentbot->nick),
		"%cBNACTION %s %s %s",
		BOTNET_CHAR, username(currentbot->nick), from, rest);
}

void    sbotnet_action	(char *from, char *to, char *rest)
{
	char	*origin;
        char    *user;

	if ((origin=get_token(&rest," "))==NULL)
		return;

        if ((user=get_token(&rest," "))==NULL)
                return;

        if (!rest)
                return;

        send_to_botnet_clients(from, "* %s %s", getnick(user), rest);

        send_to_botnet_servers(origin, from,
		"%cBNACTION %s %s %s",
		BOTNET_CHAR, origin, user, rest);
}

void	cbotnet_privmsg	(char *from, char *to, char *rest)
{
	BOTNET_list	*botnetptr;
	char		*nick1;

	if (!search_botnet_client(from))
	{
		send_to_user(from,"You're not on BOTNet!");
		return;
	}

	if ((nick1=get_token(&rest," "))==NULL)
		return;

	if (!rest)
		return;

        if ((botnetptr = search_botnet_nick(nick1))==NULL)
	{
		send_to_user(from,"[BOTNet:ERROR] %s isn't on BOTNet",
				nick1);
		return;
	}

	if (botnetptr->local == 1)
		send_to_user(botnetptr->userhost,
			"[BOTNet:PRIVMSG] %s says to YOU: %s",
			from, rest);
	else
		msend_to_user(botnetptr->servername, "%cBNPRIVMSG %s %s %s",
			BOTNET_CHAR, from, botnetptr->userhost, rest);

	if (botnetptr->local_echo)
		send_to_user(from, "[BOTNet:PRIVMSG] Told %s: %s",
			botnetptr->userhost, rest);
	return;
}

void	sbotnet_privmsg	(char *from, char *to, char *rest)
{
	char		*userto;
	char		*userfrom;
	BOTNET_list	*botnetptr;

	if ((userfrom=get_token(&rest," "))==NULL)
		return;

	if ((userto=get_token(&rest," "))==NULL)
		return;

	if (!rest)
		return;

	if ((botnetptr=search_botnet_client(userto))==NULL)
		return;

	if (botnetptr->local==0)
		return;

	send_to_user(botnetptr->userhost,
		"[BOTNet:PRIVMSG] %s says to YOU: %s",
		userfrom, rest);
}

void	cbotnet_echo	(char *from, char *to, char *rest)
{
	BOTNET_list	*botnetptr;

	if ((botnetptr=search_botnet_client(from))==NULL)
	{
		send_to_user(from,"You're not on BOTNet!");
		return;
	}

	if (botnetptr->local_echo)
	{
		botnetptr->local_echo = 0;
		send_to_user(from,"[BOTNet:INFO] Local echo is now OFF");
	}
	else
	{
		botnetptr->local_echo = 1;
		send_to_user(from,"[BOTNet:INFO] Local echo is now ON");
	}
}

void	cbotnet_who	(char *from, char *to, char *rest)
{
        BOTNET_list     *botnetptr;

	if (!search_botnet_client(from))
	{
		send_to_user(from,"You're not on BOTNet!");
		return;
	}

	send_to_user(from, " %40s | %s", "User", "Location");
	send_to_user(from, "------------------------------------------+---------------------");

        for (botnetptr = botnet_clientlist; botnetptr;
		botnetptr = botnetptr->next)
	{
		send_to_user(from, " %40s | %s",
			botnetptr->userhost,
			getnick(botnetptr->servername));
	}
}

void	cbotnet_users	(char *from, char *to, char *rest)
{
	BOTNET_list	*botnetptr;

	if (!search_botnet_client(from))
	{
		send_to_user(from,"You're not on BOTNet!");
		return;
	}

	send_to_user(from, " %12s | %-20s | %s", "Nick", "Join Time", "Location");
	send_to_user(from, "--------------+----------------------+---------------");

	for (botnetptr = botnet_clientlist; botnetptr;
		botnetptr = botnetptr->next)
	{
		send_to_user(from, " %12s | %-20s | %s",
			getnick(botnetptr->userhost),
			time2str(botnetptr->jointime),
			botnetptr->servername);
	}
}

void	cbotnet_servers	(char *from, char *to, char *rest)
{
	BOTNET_list	*botnetptr;

	if (!search_botnet_client(from))
	{
		send_to_user(from,"You're not on BOTNet!");
		return;
	}

	send_to_user(from, " %40s | %s", "BOTNet Server List", "Join Time");
	send_to_user(from, "------------------------------------------+---------------------");
	send_to_user(from, " %40s | %s",
		username(currentbot->nick), time2str(currentbot->uptime));
	for (botnetptr = botnet_serverlist; botnetptr;
		botnetptr = botnetptr->next)
	{
		send_to_user(from, " %40s | %s",
			botnetptr->userhost,
			time2str(botnetptr->jointime));
	}
}

void	cbotnet_stats	(char *from, char *to, char *rest)
{
	if (!search_botnet_client(from))
	{
		send_to_user(from,"You're not on BOTNet!");
		return;
	}

	send_to_user(from, "--------------+-------+--------+-------");
	send_to_user(from, " BOTNet Stats | Local | Remote | Total");
	send_to_user(from, "--------------+-------+--------+-------");
	send_to_user(from, "      Clients | %4d  | %5d  | %4d",
		botnetstats.local_clients,
		botnetstats.remote_clients,
		botnetstats.local_clients+botnetstats.remote_clients);
	send_to_user(from, "      Servers | %4d  | %5d  | %4d",
		1, botnetstats.remote_servers, botnetstats.remote_servers+1);
	send_to_user(from, "        Total | %4d  | %5d  | %4d",
		botnetstats.local_clients+1,
		botnetstats.remote_servers+botnetstats.remote_clients,
		botnetstats.local_clients+botnetstats.remote_clients+botnetstats.remote_servers+1);
}

void	cbotnet_topic	(char *from, char *to, char *rest)
{

	if (!search_botnet_client(from))
	{
		send_to_user(from,"You're not on BOTNet!");
		return;
	}

	if (!rest)
	{
		if (topic.timeset <= 0)
			send_to_user(from,"[BOTNet:INFO] No topic is set");
		else
			send_to_user(from,
				"[BOTNet:INFO] Topic set by %s on %s: %s",
				topic.setby,get_ctime(topic.timeset),topic.text);
		return;
	}
/*
	if (strlen(newtopic)>MAXLEN)
	{
		send_to_user(from,"That topic is too damn big fool!");
		return;
	}
*/

	free(topic.text);
	free(topic.setby);
	mstrcpy(&topic.setby,from);
	mstrcpy(&topic.text,rest);
	topic.timeset = time(NULL);

	send_to_botnet_clients(from,"[BOTNet:TOPIC] %s has changed the topic to: %s",
		from,topic.text);

	send_to_botnet_servers(username(currentbot->nick),
		username(currentbot->nick),
		"%cBNADDTOPIC %s %s %ld %s",
		BOTNET_CHAR,
		username(currentbot->nick),
		topic.setby, topic.timeset, topic.text);
}

void	cbotnet_ban	(char *from, char *to, char *rest)
{
	if (!search_botnet_client(from))
	{
		send_to_user(from,"You're not on BOTNet!");
		return;
	}

	if (!rest)
	{
		send_to_user(from,"What address do you want me to ban?");
		return;
	}

	if (strcmp(rest,"*!*@*")==0 || strstr(rest,"@")==NULL ||
		strstr(rest,"!")==NULL)
	{
		send_to_user(from,"Shitty ban mask!");
		return;
	}

	if(add_botnet_ban(from,rest))
	{
		send_to_botnet_clients(currentbot->nick,
			"[BOTNet:BAN] %s sets mode: +b %s",
			from, rest);
	}

	send_to_botnet_servers(username(currentbot->nick),
		username(currentbot->nick),
		"%cBNADDBAN %s %s %s",
		BOTNET_CHAR, username(currentbot->nick), rest, from);
}

void	cbotnet_banuser	(char *from, char *to, char *rest)
{
	BOTNET_list	*botnetptr;
	char		*banmask;
	char		*nick;

	if (!search_botnet_client(from))
	{
		send_to_user(from,"You're not on BOTNet!");
		return;
	}

	if ((nick=get_token(&rest," "))==NULL)
	{
		send_to_user(from,"Who do you want me to ban?");
		return;
	}

	if ((botnetptr=search_botnet_nick(nick))==NULL)
	{
		send_to_user(from, "[BOTNet:ERROR] %s isn't on BOTNet!",
			nick);
		return;
	}

	banmask = strdup(get_add_mask(botnetptr->userhost,0));

	if(add_botnet_ban(from,banmask))
	{
		send_to_botnet_clients(currentbot->nick,
			"[BOTNet:BAN] %s sets mode: +b %s",
			from, banmask);
	}

	send_to_botnet_servers(username(currentbot->nick),
		username(currentbot->nick),
		"%cBNBANUSER %s %s %s",
		BOTNET_CHAR, username(currentbot->nick), from, banmask);
}

void	sbotnet_banuser	(char *from, char *to, char *rest)
{
	char	*origin;
	char	*userhost;
	char	*banmask;

	if ((origin=get_token(&rest," "))==NULL)
		return;
	if ((userhost=get_token(&rest," "))==NULL)
		return;
	if ((banmask=get_token(&rest," "))==NULL)
		return;

	if (add_botnet_ban(userhost,banmask))
		send_to_botnet_clients(currentbot->nick,
		"[BOTNet:BAN] %s sets mode: +b %s",
		userhost, banmask);

	send_to_botnet_servers(origin, from, "%cBNBANUSER %s %s %s",
		BOTNET_CHAR, origin, userhost, banmask);
}

void	cbotnet_bankick	(char *from, char *to, char *rest)
{
	BOTNET_list	*botnetptr;
	char		*banmask;
	char		*nick;
	char		*reason;

	if (!search_botnet_client(from))
	{
		send_to_user(from,"You're not on BOTNet!");
		return;
	}

	if ((nick=get_token(&rest," "))==NULL)
		return;

	if ((reason=get_token(&rest," "))==NULL)
		reason = "Bite Me";

	if ((botnetptr=search_botnet_nick(nick))==NULL)
	{
		send_to_user(from, "[BOTNet:ERROR] %s isn't on BOTNet!",
			nick);
		return;
	}

	banmask = strdup(get_add_mask(botnetptr->userhost,0));

	if(add_botnet_ban(from,banmask))
	{
		send_to_botnet_clients(currentbot->nick,
			"[BOTNet:BAN] %s sets mode: +b %s",
			from, banmask);
	}

	if (botnetptr->local)
		send_to_user(botnetptr->userhost,
			"[BOTNet:KICK] You have been kicked by %s for %s",
			from, reason);

	send_to_botnet_clients(botnetptr->userhost,
		"[BOTNet:KICK] %s has been kicked by %s for %s",
		botnetptr->userhost, from, reason);

	send_to_botnet_servers(username(currentbot->nick),
		username(currentbot->nick),
		"%cBNBANKICK %s %s %s %s",
		BOTNET_CHAR,
		username(currentbot->nick), from, botnetptr->userhost, reason);

	add_botnet_kick(botnetptr->userhost);
}

void	sbotnet_bankick		(char *from, char *to, char *rest)
{
	BOTNET_list	*botnetptr;
	char	*origin;
	char	*userhost;
	char	*banneduser;
	char	*banmask;
	char	*reason;

	if ((origin=get_token(&rest," "))==NULL)
		return;
	if ((userhost=get_token(&rest," "))==NULL)
		return;
	if ((banneduser=get_token(&rest," "))==NULL)
		return;
	if ((reason=get_token(&rest," "))==NULL)
		reason = "Bite Me";

	if ((botnetptr=search_botnet_client(banneduser))==NULL)
		return;

	banmask = strdup(get_add_mask(botnetptr->userhost,0));

	if(add_botnet_ban(userhost,banmask))
	{
		send_to_botnet_clients(currentbot->nick,
			"[BOTNet:BAN] %s sets mode: +b %s",
			userhost, banmask);
	}

	if (botnetptr->local)
		send_to_user(botnetptr->userhost,
			"[BOTNet:KICK] You have been kicked by %s for %s",
			userhost, reason);

	send_to_botnet_clients(botnetptr->userhost,
		"[BOTNet:KICK] %s has been kicked by %s for %s",
		botnetptr->userhost, userhost, reason);

	send_to_botnet_servers(origin,from,
		"%cBNBANKICK %s %s %s %s",
		BOTNET_CHAR,
		origin, userhost, banneduser, reason);

	add_botnet_kick(botnetptr->userhost);
}

void	cbotnet_kick		(char *from, char *to, char *rest)
{
	BOTNET_list	*botnetptr;
	char		*nick;

	if (!search_botnet_client(from))
	{
		send_to_user(from,"You're not on BOTNet!");
		return;
	}

	if ((nick=get_token(&rest," "))==NULL)
		return;

	if ((botnetptr=search_botnet_nick(nick))==NULL)
	{
		send_to_user(from, "[BOTNet:ERROR] %s isn't on BOTNet!",
			nick);
		return;
	}

	if (botnetptr->local)
		send_to_user(botnetptr->userhost,
			"[BOTNet:KICK] You have been kicked by %s for %s",
			from, *rest ? rest : "Bite Me");

	send_to_botnet_clients(botnetptr->userhost,
		"[BOTNet:KICK] %s has been kicked by %s for %s",
		botnetptr->userhost, from, *rest ? rest : "Bite Me");

	send_to_botnet_servers(username(currentbot->nick),
		username(currentbot->nick),
		"%cBNKICK %s %s %s %s", BOTNET_CHAR,
		username(currentbot->nick), from,
		botnetptr->userhost, *rest ? rest : "Bite Me");

	add_botnet_kick(botnetptr->userhost);
}

void	sbotnet_kick		(char *from, char *to, char *rest)
{
	BOTNET_list	*botnetptr;
	char		*origin;
	char		*userhost;
	char		*kickeduser;

	if ((origin=get_token(&rest," "))==NULL)
		return;
	if ((userhost=get_token(&rest," "))==NULL)
		return;
	if ((kickeduser=get_token(&rest," "))==NULL)
		return;
	if ((botnetptr=search_botnet_client(kickeduser))==NULL)
		return;

	if (botnetptr->local)
		send_to_user(botnetptr->userhost,
			"[BOTNet:KICK] You have been kicked by %s for %s",
			userhost, *rest ? rest : "Bite Me");

	send_to_botnet_clients(botnetptr->userhost,
		"[BOTNet:KICK] %s has been kicked by %s for %s",
		botnetptr->userhost, userhost, *rest ? rest : "Bite Me");

	send_to_botnet_servers(origin,from,
		"%cBNKICK %s %s %s %s", BOTNET_CHAR,
		origin, userhost, kickeduser, *rest ? rest : "Bite Me");

	add_botnet_kick(botnetptr->userhost);
}

void	add_botnet_kick		(char *user)
{
	delete_client(search_client_chat(user));
}

int	add_botnet_ban		(char *from, char *banmask)
{
	BOTNETBAN_list	*botnetbanptr;

	if (search_botnet_banlist(banmask))
		return FALSE;

	if ((botnetbanptr=new_botnet_ban(from,banmask))==NULL)
		return FALSE;

	if (botnet_banlist==NULL)
		botnet_banlist = botnetbanptr;
	else
	{
		BOTNETBAN_list	*botnetbanptr2;

		botnetbanptr2 = botnet_banlist;

		while (botnetbanptr2->next!=NULL)
			botnetbanptr2=botnetbanptr2->next;

		botnetbanptr2->next = botnetbanptr;
		botnetbanptr->prev = botnetbanptr2;
	}

	return TRUE;
}

int	delete_botnet_ban	(char *banmask)
{
	BOTNETBAN_list	*botnetbanptr;

	if ((botnetbanptr=search_botnet_banlist(banmask))==NULL)
		return FALSE;

	if (botnetbanptr->prev==NULL)
	{
		botnet_banlist = botnetbanptr->next;

		if (botnetbanptr->next != NULL)
			botnet_banlist->prev = NULL;
	}
	else
	{
	        BOTNETBAN_list     *botnetbanptr2;

                botnetbanptr2 = botnetbanptr->prev;
                botnetbanptr2->next = botnetbanptr->next;

		if (botnetbanptr->next!=NULL)
		{
			botnetbanptr2 = botnetbanptr->next;
			botnetbanptr2->prev = botnetbanptr->prev;
		}
	}

	free(botnetbanptr->userhost);
	free(botnetbanptr->banmask);
	free(botnetbanptr);
	return TRUE;
}

void	cbotnet_showbans	(char *from, char *to, char *rest)
{
	BOTNETBAN_list	*botnetbanptr;
	int i;

	if (!search_botnet_client(from))
	{
		send_to_user(from,"You're not on BOTNet!");
		return;
	}

	if (botnet_banlist==NULL)
	{
		send_to_user(from,"[BOTNet:INFO] No bans are set.");
		return;
	}

	send_to_user(from,"%s Ban List",currentbot->nick);
	send_to_user(from,"--------------------------------+---+-----------------------------");
	for (i = 1, botnetbanptr = botnet_banlist; botnetbanptr;
		botnetbanptr = botnetbanptr->next, i++)
	{
		send_to_user(from," %30s |%2d | %s",
			botnetbanptr->banmask,i,botnetbanptr->userhost);
	}
}

void	cbotnet_rban		(char *from, char *to, char *rest)
{
	BOTNETBAN_list	*botnetbanptr;
	char *banmask;
	int i;
	int ban2del;

	if (!rest)
		return;

	if (!search_botnet_client(from))
	{
		send_to_user(from,"You're not on BOTNet!");
		return;
	}

	if (!readint(&rest,&ban2del) || ban2del<1)
	{
		send_to_user(from,"[BOTNet:ERROR] Illegal ban number!");
		return;
	}

	for (i = 1, botnetbanptr = botnet_banlist; botnetbanptr;
		botnetbanptr = botnetbanptr->next, i++)
	{
		if (i==ban2del)
		{
			banmask = strdup(botnetbanptr->banmask);
			if (delete_botnet_ban(botnetbanptr->banmask))
			{
				send_to_botnet_clients(currentbot->nick,
					"[BOTNet:UNBAN] %s sets mode: -b %s",
					from, banmask);
			}

			send_to_botnet_servers(username(currentbot->nick),
				username(currentbot->nick),
				"%cBNRBAN %s %s %s",
				BOTNET_CHAR,
				username(currentbot->nick),from,banmask);
			return;
		}
	}
	send_to_user(from,"[BOTNet:ERROR] Ban not found!");
}

void	sbotnet_rban		(char *from, char *to, char *rest)
{
	char	*origin;
	char	*userhost;
	char	*banmask;

	if ((origin=get_token(&rest," "))==NULL)
		return;
	if ((userhost=get_token(&rest," "))==NULL)
		return;
	if ((banmask=get_token(&rest," "))==NULL)
		return;

	if (delete_botnet_ban(banmask))
		send_to_botnet_clients(currentbot->nick,
			"[BOTNet:UNBAN] %s sets mode: -b %s",
			userhost, banmask);

	send_to_botnet_servers(origin,from,
		"%cBNRBAN %s %s %s",
		BOTNET_CHAR,
		origin,userhost,banmask);
}

BOTNETBAN_list	*new_botnet_ban		(char *from, char *banmask)
{
	BOTNETBAN_list	*botnetbanptr;

	if ( (botnetbanptr = (BOTNETBAN_list*)malloc(sizeof(*botnetbanptr))) == NULL)
		return NULL;

	mstrcpy(&botnetbanptr->userhost,from);
	mstrcpy(&botnetbanptr->banmask,banmask);
	botnetbanptr->next = NULL;
	botnetbanptr->prev = NULL;
	return(botnetbanptr);
}

BOTNETBAN_list	*search_botnet_banlist	(char *userhost)
{
	BOTNETBAN_list	*botnetbanptr;

	for (botnetbanptr = botnet_banlist; botnetbanptr;
		botnetbanptr = botnetbanptr->next)
	{
		if (!mymatch(botnetbanptr->banmask,userhost,FNM_CASEFOLD))
			return(botnetbanptr);
	}

	return NULL;
}

BOTNET_list	*search_botnet_nick	(char *nick)
{
	BOTNET_list	*botnetptr;

	for (botnetptr = botnet_clientlist; botnetptr;
		botnetptr = botnetptr->next)
	{
                if (STRCASEEQUAL(getnick(botnetptr->userhost),nick))
			return(botnetptr);
	}
	return NULL;
}

void		show_botnet_help	(char *from, char *to, char *rest)
{
        FILE    *f;
        char    *s;

        if((f=fopen("help.botnet", "r"))==NULL)
        {
                send_to_user(from, "BOTNet helpfile missing");
                globallog(0, ERRFILE,
                        "couldn't find botnet helpfile to help %s", from);
                return;
        }

        if(rest == NULL)
        {
					 find_topic(f, "standard");
					 while((s=get_ftext(f, "standard")))
								send_to_user(from, s);
		  }
		  else
					 if(!find_topic( f, rest ))
								send_to_user(from,
										  "No BOTNet help available for \"%s\"", rest);
					 else
								while((s=get_ftext(f, rest)))
                                send_to_user(from, s);
        fclose(f);
}
