#ifndef DWEEB_H
#define DWEEB_H

#ifdef AIX
#       include <sys/select.h>
#endif /* AIX */

#include "config.h"

#include "channel.h"
#include "chanuser.h"
#include "userlist.h"
#include "dcc.h"
#include "memory.h"

struct botstruct
{

	int             created;

	char            nick[MAXNICKLEN]; 
	char            realnick[MAXNICKLEN];
	char            login[MAXNICKLEN];
	char            ircname[MAXLEN];

	int             server_sock;
	int             maxmodes;
	char            *usersfile;
	char            *shitfile;


	char            cmdchar;
	int             floodlevel;
	int             floodprotlevel;
	int             massprotlevel;
	int             massdeoplevel;
	int             massbanlevel;
	int             masskicklevel;
	int             massnicklevel;
	int             beepkicklevel;
	int             capskicklevel;
	int             idlelevel;
	int             cctoggle;
	int             malevel;
	int             restrict;
	int             filesendallowed;
	int             filerecvallowed;
	char            *uploaddir;
	char            *downloaddir;

	int             num_of_servers;
	int             current_server;

	struct
	{
		char name[MAXLEN];
		char realname[MAXLEN];
		int     port;
	} serverlist[MAXSERVERS];
	CHAN_list       *Channel_list;
	CHAN_list       *Current_chan;
	DCC_list        *Client_list;

	ULS             **UserList;
	SLS             **ShitList;
	LS              **StealList;
	LS              **SpyMsgList;
	TS              **OffendersList;

	MemoryStruct    *MemList;

	long            uptime;

	long            lastreset;      /* last time bot was reset */
	long            lastping;       /* to check activity server */
	int             server_ok;
};

typedef struct botstruct botinfo;

void read_lists_all(void);
void init_bots(void);
botinfo *find_bot(char *s);
botinfo *add_bot(char *nick);
int update_login(char *login);
int update_ircname(char *ircname);
int add_server(char *servername);
void start_all_bots(void);
int kill_bot(char *reason);
void kill_all_bots(char *reason);
void start_all_bots(void);
int connect_to_server(void);
void reconnect_to_server(void);
void set_server_fds(fd_set *reads, fd_set *writes);
int readln(char *buf);
int send_to_server(char *format, ...);
void try_reconnect(void);
void reset_botstate(void);
void parse_server_input(fd_set *read_fds);
void set_dcc_fds(fd_set *read_fds, fd_set *write_fds);
void parse_dcc_input(fd_set *read_fds);
void send_pings(void);
void pong_received(char *nick, char *server);
void parse_global(char *from, char *to, char *rest, int cmdlevel);
void global_not_imp(char *from, char *rest);
void global_debug(char *from, char *rest);
void global_die(char *from, char *rest);
void global_list(char *from, char *rest);
void global_rawmsg(char *from, char *rest);
int forkbot(char *from, char *nick, char *rest);

#endif  /* DWEEB_H */






