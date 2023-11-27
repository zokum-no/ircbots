/*

    EnergyMech, IRC bot software
    Copyright (c) 1997-2001 proton, 2002-2004 emech-dev

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
#define is_me(x)		(!Strcasecmp(current->nick,x))
#define is_opped(x,y)		(usermode(y,x) & CU_CHANOP)
#define get_chanopmode(x)	(x->flags & CU_CHANOP)
#define get_usermode(x)		(x->flags)
#define set_int_varc(c,w,v)	{ c->Vars[w].value = v; }
#define ischannel(x)		(*x == '#' || *x == '&')

#define tofile			send_to_socket

#define superuser(x)		((x->access == OWNERLEVEL) && (*x->chanp == '*'))

#define nullstr(x)		(x) ? x : NULLSTR
#define nullbuf(x)		(x[0]) ? x : NULLSTR

#ifndef MULTI
#define is_localbot(x)		is_me(x)
#endif /* MULTI */

#ifdef IPV6
#define SIZEOF_SOCKADDR(so) ((so).sa.sa_family == AF_INET6 ? sizeof(so.sin6) : sizeof(so.sin))

#ifndef ip2string
char ntop_tmp[MAXHOSTLEN];
#define ip2string(addr) inet_ntop((IN6_IS_ADDR_V4MAPPED(&addr))?AF_INET:AF_INET6, \
(IN6_IS_ADDR_V4MAPPED(&addr))?(void *)&(((uint32_t *)&(addr))[3]):(void *)&(addr), ntop_tmp, MAXHOSTLEN)
#endif /* ip2string */

#else
#define SIZEOF_SOCKADDR(so) (sizeof(so.sin))
#endif

#ifdef SUPERDEBUG

#define set_mallocdoer(x)	mallocdoer = x;

#define mechexit(x)				\
{						\
	if (debug_on_exit)			\
		do_debug(NULL,NULL,NULL,0);	\
	if (do_exec)				\
		mech_exec();			\
	exit(x);				\
}

#define mechexit2(x)				\
{						\
	if (debug_on_exit)			\
		do_debug(NULL,NULL,NULL,0);	\
	if (do_exec)				\
		mech_exec();			\
	_exit(x);				\
}

#else /* SUPERDEBUG */

#define set_mallocdoer(x)	;

#define mechexit(x)				\
{						\
	if (do_exec)				\
		mech_exec();			\
	exit(x);				\
}

#define mechexit2(x)				\
{						\
	if (do_exec)				\
		mech_exec();			\
	_exit(x);				\
}

#endif /* SUPERDEBUG */

#define find_aBotLink(id)	(aBotLink*)findstruct((char*)botlinkBASE,botlinkSIZE,sizeof(aBotLink),id)
#define find_aServer(id)	(aServer*)findstruct((char*)serverBASE,serverSIZE,sizeof(aServer),id)
#define find_aUser(id)		(aUser*)findstruct((char*)current->userlistBASE,current->userlistSIZE,sizeof(aUser),id)

#define add_aBotLink(new)	(aBotLink*)addstruct((char**)&botlinkBASE,(char*)new,&botlinkSIZE,sizeof(aBotLink))
#define add_aLinkConf(new)	(aLinkConf*)addstruct((char**)&linkconfBASE,(char*)new,&linkconfSIZE,sizeof(aLinkConf));
#define add_aMsgList(a,b,c)	(aMsgList*)addstruct((char**)a,(char*)b,c,sizeof(aMsgList))
#define add_aServer(new)	(aServer*)addstruct((char**)&serverBASE,(char*)new,&serverSIZE,sizeof(aServer))
#define add_aUser(new)		(aUser*)addstruct((char**)&current->userlistBASE,(char*)new,&current->userlistSIZE,sizeof(aUser))

#ifdef LINKING
void setbotnick(aBot *, char *);
#else /* LINKING */
#define setbotnick(x,y)		strcpy(x->nick,y)
#endif /* LINKING */

#ifdef SYSTEM_IS_AIX

/*---------------------------------------*
|   This is for fucked up AIX systems.   |
*---------------------------------------*/

int accept(int s, void * addr, int * addrlen);
int bind(int sockfd, void * my_addr, int addrlen);
int connect(int sockfd, void * serv_addr, int addrlen);
int getsockname(int s, void * name, int * namelen);
int listen(int s, int backlog);
int setsockopt(int s, int level, int optname, const void *optval, int optlen);
int socket(int domain, int type, int protocol);
void usleep(unsigned long usec); 

#endif /* SYSTEM_IS_AIX */

int get_ip(char *, union sockaddr_union *);
VarStruct *find_var(char *, int);
aBot *add_bot(char *);
aBot *forkbot(char *, char *, char *);
aChan *find_channel(char *, int);
aChanUser *find_chanuser(aChan *, char *);
aChanUser *make_chanuser(char *, char *, char *);
aDCC *search_list(char *, Ulong);
aSeen *make_seen(char *, char *, char *, time_t, int, char *);
aServer *add_server(char *, int, char *);
#ifdef SERVICES
aService *find_service(aService **, char *, char *);
#endif /* SERVICES */
aTime *find_time(aTime **, char *);
aTime *make_time(aTime **, char *);
aUser *add_to_shitlist(char *, int, char *, char *, char *, int, int);
aUser *add_to_userlist(char *, int, int, int, char *, char *);
aUser *check_kicksay(char *, char *);
aUser *find_handle(char *);
aUser *find_kicksay(char *, char *);
aUser *find_shit(char *, char *);
aUser *find_user(char *, char *);
aUser *get_shituser(char *, char *);
char *Strcat(char *, char *);
char *addstruct(char **, char *, int *, int);
char *chop(char **);
char *cipher(char *);
char *cluster(char *);
char *escape_string(char *);
char *find_nuh(char *);
char *find_userhost(char *, char *);
char *findstruct(char *, int, int, int);
char *format_uh(char *, int);
char *formatgreet(char *, char *);
char *get_channel(char *, char **);
char *get_channel2(char *, char **);
char *get_nuh(aChanUser *);
char *get_shitreason(aUser *);
char *get_str_var(char *, int);
char *get_str_varc(aChan *, int);
char *get_token(char **, char *);
char *gethost(char *);
char *getnick(char *);
char *getuh(char *);
char *idle2str(time_t, int);
char *mstrcpy(char **, char *);
char *nick2uh(char *, char *, int);
char *randstring(char *);
char *right(char *, int);
char *sockread(int, char *, char *);
char *subststrvar(char *);
char *terminate(char *, char *);
char *time2away(time_t);
char *time2medium(time_t);
char *time2small(time_t);
char *time2str(time_t);
char *unescape_string(char *);
int *Atoi(char *);
#ifdef SERVICES
int add_service(aService **, char *, char *);
#endif /* SERVICES */
int capslevel(char *);
int channel_massunban(char *, char *, time_t);
int channel_massunbanfrom(char *, char *, time_t);
int channel_unban(char *, char *);
int check_for_number(char *, char *);
int check_mass(char *, aChan *, int);
int check_nickflood(char *);
int check_pubflooding(char *, char *);
int correct_password(char *, char *);
int delete_ban(aBan **, char *);
int delete_channel(aChan **, aChan *);
int remove_chanuser(aChan *, char *);
#ifdef SERVICES
int delete_service(aService **, aService *);
#endif /* SERVICES */
int find_var_name(char *, int);
int get_idletime(char *);
int get_int_var(char *, int);
int get_int_varc(aChan *, int);
int get_protlevel(char *, char *);
int get_protuseraccess(char *, char *);
int get_shitlevel(char *, char *);
int get_userlevel(char *, char *);
int is_aop(char *, char *);
int is_avoice(char *, char *);
#ifdef MULTI
int is_localbot(char *);
#endif /* MULTI */
#ifndef STRICTAUTH
int is_netsplit(char *);
#endif /* STRICTAUTH */
int is_present(char *);
int is_shitted(char *, char *);
int is_user(char *, char *);
int isnick(char *);
int killsock(int);
int level_needed(char *);
int matches(char *, char *);
int max_userlevel(char *);
int num_banned(char *);
int num_matches(char *, char *);
int numchar(char *, int);
int passmatch(char *, char *);
int password_needed(char *);
int randstring_getline(char *);
int read_from_socket(int, char *);
int read_levelfile(void);
int read_seenlist(void);
int readuserlist(void);
int reverse_mode(char *, aChan *, int, int);
int savesession(void);
int saveuserlist(void);
int send_chat(char *, char *);
int send_to_socket(int, char *, ...);
int set_enftopic(char *, char *);
int shit_action(char *, aChan *);
int show_names_on_channel(char *, char *);
int SockConnect(char *, int, int);
int SockListener(int);
int Strcasecmp(const char *, const char *);
#ifdef TRIVIA
int trivia_score_callback(char *);
#endif /* TRIVIA */
int usermode(char *, char *);
int verified(char *userhost);
int write_levelfile(void);
int write_seenlist(aSeen **, char *);
void add_alias(char *, char *, int);
void *MyMalloc(int);
void Link_addbot(aBotLink *, char *);
void Link_needinvite(aBotLink *, char *);
void Link_needkey(aBotLink *, char *);
void Link_reply(aBotLink *, char *);
void Link_userverify(aBotLink *, char *);
void MyFree(char **);
void add_client(aDCC *);
void add_mode(aChan *, Ulong, char *);
void addlinkconf(char *, char *, char *, int);
void addmasktouser(aUser *, char *);
void cfg_addks(char *);
void cfg_addshit(char *);
void cfg_alias(char *);
void cfg_reason(char *);
void cfg_server(char *);
void cfg_set(char *);
void cfg_uchannel(char *);
void cfg_who(char *);
void change_dccnick(char *, char *);
void channel_massmode(char *, char *, int, char, char);
void check_all_steal(void);
void check_idlekick(void);
void check_shit(void);
void close_all_dcc(void);
void connect_to_server(void);
void copy_vars(VarStruct *, const VarStruct*);
void cycle_channel(char *);
void dcc_chat(char *, char *);
void dcc_connect(char *, char *);
void debug(char *, ...);
void del_mode(aChan *, Ulong, char *);
void delete_all_channels(void);
#ifdef SERVICES
void delete_all_services(aService *);
#endif /* SERVICES */
void delete_bans(aBan **);
void delete_chanusers(aChan *);
void delete_client(aDCC **);
void delete_seen(void);
void delete_shitlist(void);
void delete_time(aTime **);
void deop_ban(char *, char *, char *);
void deop_siteban(char *, char *, char *);
void do_access(char *, char *, char *, int);
void do_add(char *, char *, char *, int);
void do_addserver(char *, char *, char *, int);
#ifdef ALIAS
void do_alias(char *, char *, char *, int);
#endif
void do_auth(char *, char *, char *, int);
void do_unverify(char *, char *, char *, int);
void do_away(char *, char *, char *, int);
void do_ban(char *, char *, char *, int);
void do_banlist(char *, char *, char *, int);
void do_boot(char *,char *, char *, int);
void do_cchan(char *, char *, char *, int);
void do_chaccess(char *, char *, char *, int);
void do_channels(char *, char *, char *, int);
void do_chat(char *, char *, char *, int);
void do_clearshit(char *, char *, char *, int);
void do_clvl(char *, char *, char *, int);
void do_cmdchar(char *, char *, char *, int);
void do_core(char *, char *, char *, int);
void do_cserv(char *, char *, char *, int);
void do_cslvl(char *, char *, char *, int);
void do_ctcp(char *, char *, char *, int);
void do_cycle(char *, char *, char *, int);
void do_dcc_kill(char *, char *, char *, int);
void do_debug(char *, char *, char *, int);
void do_del(char *, char *, char *, int);
void do_delserver(char *, char *, char *, int);
void do_deop(char *, char *, char *, int);
void do_deopme(char *, char *, char *, int);
void do_die(char *, char *, char *, int);
void do_do(char *, char *, char *, int);
void do_echo(char *, char *, char *, int);
void do_esay(char *, char *, char *, int);
void do_flood(char *, char *, char *, int);
void do_forget(char *, char *, char *, int);
void do_help(char *, char *, char *, int);
void do_host(char *, char *, char *, int);
void do_idle(char *, char *, char *, int);
void do_insult(char *, char *, char *, int);
void do_invite(char *, char *, char *, int);
void do_join(char *, char *, char *, int);
void do_kick(char *, char *, char *, int);
void do_kickban(char *, char *, char *, int);
void do_kicksay(char *, char *, char *, int);
void do_kslist(char *, char *, char *, int);
void do_last(char *, char *, char *, int);
void do_link(char *, char *, char *, int);
void do_linkcmd(char *, char *, char *, int);
void do_loadall(char *, char *, char *, int);
void do_loadlevels(char *, char *, char *, int);
void do_loadlists(char *, char *, char *, int);
void do_lusers(char *, char *, char *, int);
void do_me(char *, char *, char *, int);
void do_mode(char *, char *, char *, int);
void do_msg(char *, char *, char *, int);
void do_names(char *, char *, char *, int);
void do_nextserver(char *, char *, char *, int);
void do_nick(char *, char *, char *, int);
void do_ontime(char *, char *, char *, int);
void do_op(char *, char *, char *, int);
void do_opme(char *, char *, char *, int);
void do_part(char *, char *, char *, int);
void do_passwd(char *, char *, char *, int);
void do_pickup(char *, char *, char *, int);
void do_qshit(char *, char *, char *, int);
void do_randomtopic(char *, char *, char *, int);
void do_reset(char *, char *, char *, int);
void do_rehash2(char *, char *, char *, int);
void do_report(char *, char *, char *, int);
void do_rkicksay(char *, char *, char *, int);
void do_rshit(char *, char *, char *, int);
void do_rspy(char *, char *, char *, int);
void do_rspymsg(char *, char *, char *, int);
void do_rstatmsg(char *, char *, char *, int);
void do_rsteal(char *, char *, char *, int);
void do_saveall(char *, char *, char *, int);
void do_savelevels(char *, char *, char *, int);
void do_savelists(char *, char *, char *, int);
void do_say(char *, char *, char *, int);
void do_screwban(char *, char *, char *, int);
void do_seen(char *, char *, char *, int);
void do_send(char *, char *, char *, int);
void do_server(char *, char *, char *, int);
#ifdef SERVICES
void do_service(char *, char *, char *, int);
#endif /* SERVICES */
void do_serverlist(char *, char *, char *, int);
void do_servstats(char *, char *, char *, int);
void do_set(char *, char *, char *, int);
void do_setpass(char *, char *, char *, int);
void do_shit(char *, char *, char *, int);
void do_shitlist(char *, char *, char *, int);
void do_showidle(char *, char *, char *, int);
void do_showusers(char *, char *, char *, int);
void do_shutdown(char *, char *, char *, int);
void do_siteban(char *, char *, char *, int);
void do_sitekickban(char *, char *, char *, int);
#ifdef SPAWN
void do_spawn(char *, char *, char *, int);
#endif
void do_spy(char *, char *, char *, int);
void do_spylist(char *, char *, char *, int);
void do_spymsg(char *, char *, char *, int);
void do_statmsg(char *, char *, char *, int);
void do_stats(char *, char *, char *, int);
void do_steal(char *, char *, char *, int);
void do_time(char *, char *, char *, int);
void do_toggle(char *, char *, char *, int);
void do_topic(char *, char *, char *, int);
#ifdef TRIVIA
void do_trivia(char *, char *, char *, int);
void do_triv_wk10(char *,char *,char *, int);
#endif /* TRIVIA */
void do_unban(char *, char *, char *, int);
void do_unvoice(char *, char *, char *, int);
void do_uptime(char *, char *, char *, int);
void do_usage(char *, char *, char *, int);
void do_user(char *, char *, char *, int);
void do_userhost(char *, char *, char *, int);
void do_userlist(char *, char *, char *, int);
void do_uset(char *, char *, char *, int);
void do_vers(char *, char *, char *, int);
void do_virtual(char *, char *, char *, int);
void do_voice(char *, char *, char *, int);
void do_wall(char *, char *, char *, int);
void do_whois_irc(char *, char *, char *, int);
void do_whom(char *, char *, char *, int);
void do_wingate(char *, char *, char *, int);
void free_strvars(VarStruct *);
void join_channel(char *, char *, int);
void kill_all_bots(char *);
void link_dccuser(char *, int);
void link_bcast(aBotLink *, char *, ...);
void make_ban(aBan **, char *, char *, time_t);
void makepass(char *, char *);
void mass_action(char *, char *);
void mech_exec(void);
void needop(char *);
void needop_via_msg(char *);
void no_info(char *, char *);
void on_actionmsg(char *, char *, char *);
void on_ctcp(char *, char *, char *);
void on_join(char *);
void on_kick(char *, char *);
void on_mode(char *, char *, char *);
void on_msg(char *, char *, char *);
void on_nick(char *, char *);
void on_public(char *, char *, char *);
void op_all_chan(char *);
void parse_002(char *, char *);
void parse_003(char *, char *);
void parse_004(char *, char *);
void parse_dcc_input(void);
void parse_link_input(void);
void parse_pipe_input(void);
void parse_server_input(void);
void parseline(char *);
void process_link_input(aBotLink *, char *);
void prot_action(char *, aChan *, char *);
void readcfgfile(void);
void readline(int s, int (*callback)(char *));
#ifdef TRIVIA
void read_triviascore(void);
#endif /* TRIVIA */
void rejoin_channels(void);
void remove_time(aTime **, aTime *);
void remove_user(aUser *);
void reset_botstate(void);
void reverse_topic(char *, char *, char *);
void send_common_spy(char *, char *, ...);
void send_ctcp(char *, char *, ...);
void send_ctcp_reply(char *, char *, ...);
void send_global_statmsg(char *, ...);
void send_spy(aChan *, char *, ...);
void send_spymsg(char *, ...);
void send_statmsg(char *, ...);
void send_to_link(aBotLink *, char *, ...);
void to_server(char *, ...);
void send_to_user(char *, char *, ...);
void send_uptime(void);
void sendkick(char *, char *, char *, ...);
void sendmode(char *, char *, ...);
void sendprivmsg(char *, char *, ...);
void set_str_varc(aChan *, int, char *);
void show_idletimes(char *, char *, int);
void sig_hup(int);
void signoff(char *, char *);
#ifdef TRIVIA
void trivia_check(char *, char *);
void trivia_cleanup(void);
void trivia_no_answer(void);
void trivia_question(void);
void trivia_tick(void);
void trivia_week_toppers(void);
#endif /* TRIVIA */
void update_auths(void);
void update_idletime(char *, char *);
void uptime_time(aTime **, time_t);
void usage(char *, char *);
void wle(char *, ...);
#ifdef TRIVIA
void write_triviascore(void);
#endif /* TRIVIA */
