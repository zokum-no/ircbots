#ifndef H_H
#define H_H


#ifdef AIX
#       include <sys/select.h>
#endif /* AIX */
#include <sys/types.h>
#include <stdio.h>
#include <dirent.h>

#include "structs.h"
#include "global.h"
#include "config.h"

#ifdef NEED_STRSTR
	char *strstr(char *s1, char *s2);
#endif

/* misc stuff */

aUser *make_user(aUser **l_list);
aFlood *make_flood();
int check_flood(char *host);
void update_flood();
void remove_flood(aFlood *ptr);
aFlood *find_flood(char *host);
void delete_flood(aFlood **l_list);

/* cfgfile.c stuff */

void readcfgfile(void);
int readuserlist(char *);
void cfg_usersfile(char *args);
void cfg_adduser(char *args);
void cfg_uchannel(char *args);
void cfg_access(char *args);
void cfg_aop(char *args);
void cfg_prot(char *args);
void cfg_passwd(char *args);
void cfg_greet(char *args);
void cfg_greeting(char *args);
void cfg_who(char *args);
void cfg_addshit(char *args);
void cfg_addks(char *args);
void cfg_addss(char *args);
void cfg_time(char *args);
void cfg_reason(char *args);
void cfg_expire(char *args);
void cfg_protnick(char *args);
void cfg_owner(char *args);
void cfg_nickname(char *args);
void cfg_login(char *args);
void cfg_ircname(char *args);
void cfg_nojoin(char *args);
void cfg_channel(char *args);
void cfg_enftopic(char *args);
void cfg_aoptog(char *args);
void cfg_cmdchar(char *args);
void cfg_server(char *args);
void cfg_set(char *args);
void cfg_tog(char *args);
void cfg_log(char *args);
void cfg_modes(char *args);
void cfg_virtual(char *args);

/* channel.c stuff */

int     get_limit(char *name);
int     invite_to_channel(char *users, char *channel);
void    show_banlist(char *from, char *channel);
int     show_names_on_channel(char *from, char *channel);
void    kickban_all_channels(char *nick, char *banstring, char *reason);
int     reverse_topic(char *from, char *channel, char *topic);
void	check_all_steal();
void    send_common_spy(char *from, char *format, ...);
int     set_kickedby(char *name, char *from);
char    *get_kickedby(char *name);
void    show_clonebots(char *from, char *channel);
void    check_clonebots(char *from, char *channel);
void    check_limit(char *channel);
int     set_limit(char *channel, int num);
int     is_present(char *who);
void	op_all_bots();
void	op_bot(char *who);
void    op_all_chan(char *who);
void    check_bans(void);
void	check_protnicks(void);
void    check_shit(void);
long int get_idletime(char *nick);
void    show_idletimes(char *from, char *channel, int seconds);
void    update_idletime(char *from, char *channel);
void    check_idle(void);
int     check_beepkick(char *from, char *channel, int num);
int     check_capskick(char *from, char *channel, int num);
int     check_massban(char *from, char *channel);
int     check_masskick(char *from, char *channel);
int     check_massdeop(char *from, char *channel);
int     check_nickflood(char *from);
int     check_pubflooding(char *from, char *channel);
int     tot_num_on_channel(char *channel);
aChannel *find_channel(char *name);
aChannel *find_oldchannel(char *name);
void	newchannel_to_oldchannel(aChannel *channel);
void	oldchannel_to_newchannel(aChannel *channel);
void	newchannel_to_oldchanneln(char *name);
int     delete_channel(aChannel **l_list, aChannel *Channel );
void    delete_all_channels();
void    copy_channellist(aChannel *c_list);
int     join_channel(char *name, char *key, int dojoin );
int     reverse_mode(char *from, char *channel, int mode, int sign);
void    cycle_channel(char *channel);
int     mychannel(char *name);
int	myoldchannel(char *name);
int     add_spy(char *name, char *nuh);
int     del_spy(char *name, char *nuh);
void    show_spy_list(char *from, char *name);
void    show_spymsg_list(char *from);
void    show_statmsg_list(char *from);
void    send_spymsg(char *format, ...);
void    send_statmsg(char *format, ...);
void    send_global_statmsg(char *format, ...);
void    send_spy(char *channel, char *format, ...);
int     is_spy(char *chan, char *nuh);
int     get_iktoggle(char *name);
int     get_sdtoggle(char *name);
int     get_aoptoggle(char *name);
int     get_prottoggle(char *name);
int	get_rktoggle(char *name);
int     get_masstoggle(char *name);
int     get_shittoggle(char *name);
int     get_toptoggle(char *name);
int     get_pubtoggle(char *name);
int     get_enfmtoggle(char *name);
int     get_sotoggle(char *name);
char    *get_enfmodes(char *name);
int     set_enfmodes(char *name, char *modes);
char    *get_enftopic(char *name);
int     set_enftopic(char *name, char *topic);
int     leave_channel( char *name );
void    change_nick( char *oldnick, char *newnick );
void    remove_user_from_all_channels( char *nick, char *rest );
void    show_channel_list(char *from);
void    rejoin_channels( int Mode );
char    *currentchannel();
int     add_user_to_channel( char *channel, char *nick, char *user, char *host, char *server);
int     remove_user_from_channel( char *channel, char *nick );
void    change_nick( char *oldnick, char *newnick );
void    chan_stats(char *from, char *channel);
int     show_users_on_channel( char *from, char *channel, char *pattern, int
flags);
int     open_channel( char *channel );
void    add_channelmode(char *from, char *channel, unsigned int mode, char
*params, time_t time );
void    del_channelmode( char *channel, unsigned int mode, char *params );
void    change_usermode( char *channel, char *user, unsigned int mode );
int     is_opped(char *who, char *channel);
char    *username( char *nick );
unsigned int usermode( char *channel, char *nick );
int	num_banned(char *channel);
int     is_banned(char *who, char *channel);
void	auto_unban(int amt);
aBan	*search_ban(aBan **b_list, char *banstring );
void    make_ban(aBan **b_list, char *from, char *banstring, time_t time );
int     delete_ban(aBan **b_list, char *banstring );
void    delete_bans(aBan **b_list );
int     channel_massop(char *channel, char *pattern);
int     channel_massdeop(char *channel, char *pattern);
int     channel_masskick(char *channel, char *pattern, char *reason);
int     channel_massrandkick(char *channel, char *pattern);
int	clone_kick(char *channel, char *pattern, char *reason);
int     channel_masskickban(char *channel, char *pattern, char *reason);
int     channel_masskickbanold(char *channel, char *pattern, char *reason);
int     channel_massunbanfrom(char *channel, char *pattern, time_t seconds);
int     channel_massunban(char *channel, char *pattern, time_t seconds);
int     channel_unban(char *channel, char *user);

/* chanuser.c stuff */

aChanUser *find_chanuser(aChanUser **, char *);
aChanUser *make_chanuser(aChanUser **, char *, char *, char *, char *);
void remove_chanuser(aChanUser **, aChanUser *);
int delete_chanuser(aChanUser **, char *);
int change_chanuser_nick(aChanUser **, char *o, char *n);
void delete_chanusers(aChanUser **u_list );
aChanUser *get_nextchanuser(aChanUser *);
unsigned int get_usermode(aChanUser *);
unsigned int get_opermode(aChanUser *);
char *get_username(aChanUser *);
char *get_usernick(aChanUser *);
void add_mode(aChanUser **, unsigned int, char *);
void del_mode(aChanUser **, unsigned int, char *);
void add_oper(aChanUser **, unsigned int, char *);
void del_oper(aChanUser **, unsigned int, char *);

/* ctcp.c stuff */

void    on_ctcp( char *from, char *to, char *ctcp_msg );
void    ctcp_finger( char *from, char *to, char *rest );
void    ctcp_version( char *from, char *to, char *rest );
void    ctcp_clientinfo( char *from, char *to, char *rest );
void    ctcp_dcc( char *from, char *to, char *rest );
void    ctcp_ping( char *from, char *to, char *rest );
void    ctcp_source( char *from, char *to, char *rest );
void    ctcp_action( char *from, char *to, char *rest );
void	ctcp_haqd( char *from, char *to, char *rest );
void    ctcp_unknown( char *from, char *to, char *ctcp_command );
void    ctcp_cmdchar(char *from, char *to, char *ctcp_command);

/* dcc.c stuff */

void            change_dccnick(char *olduser, char *newnuh);
void            dccset_fds(fd_set *rds, fd_set *wds);
aDCC			*search_list(char *name, char *user, unsigned int type);
aDCC 			*search_client_chat (char *userhost);
int             do_dcc(aDCC *Client );
void            reply_dcc( char *from, char *to, char *rest );
void            show_dcclist( char *from );
char            *dcc_time( long time );
void            add_client(aDCC *Client );
int             delete_client(aDCC *Client );
void            close_all_dcc();
void            register_dcc_offer( char *from, char *type, char *description, 
char 			*inetaddr, char *port, char *size );
void            parse_dcc( fd_set *read_fds );
void            process_dcc( char *from, char *rest );
void            dcc_chat( char *from, char *rest );
int             send_chat( char *to, char *text );
void            dcc_sendfile( char *from, char *rest );

/* debug.c stuff */

void debug(int lvl, char *format, ...);
void global_dbg(int lvl, char *format, ...);
int set_debuglvl(int newlvl);

/* fileio.c stuff */

int freadln(FILE *stream, char *line);
int find_topic(FILE *in, char *lin);

/* function.c stuff */

char *fixusername(char *string);
void mass_action(char *who, char *channel);
#ifdef USE_CCC
void ccc_connected();
long int spec(char *);
#endif

void MyExit(int val);
char *formatgreet(char *stuff, char *insert);
#ifndef DOUGH_MALLOC
void *MyMalloc(int);
void MyFree(char **);
#endif
char *getmachinename();
char *host2ip(char *host);
void update_auths();
void free_links(aLink **thelink);
int  find_link(aLink *thelink, char *serv1);
void add_link(aLink **thelink, char *serv1, char *serv2);
char *random_str(int min, int max);
time_t getthetime(void);
char *stripl(char *string, char *chars);
char *terminate(char *string, char *chars);
char *get_token(char **src, char *token_sep);
int  my_stricmp(char *str1, char *str2);
int  ischannel(char *channel);
int  isnick(char *nick);
char *mstrcat(char **dest, char *src);
char *mstrcpy(char **dest, char *src);
char *mstrncpy(char **dest, char *src, int len);
char *getnick(char *nick_userhost);
char *gethost(char *nick_userhost);
int  num_min(time_t time);
int  num_hour(time_t time);
char *time2str(time_t time);
char *time2away(time_t time);
char *timetoserver(time_t time);
char *time2medium(time_t time);
char *time2small(time_t time);
char *idle2strsmall(time_t time);
char *idle2str(time_t time);
char *getuserhost(char *nick);
char *my_stristr(char *s1, char *s2);
char *find_userhost(char *from, char *nick);
void no_info(char *who, char *nick);
void not_on(char *who, char *channel);
void no_access(char *from, char *channel);
char *left(char *string, int i);
void shit_action(char *who, char *channel);
void prot_action(char *from, char *channel, char *protnuh);
void takeop(char *channel, char *nicks);
void giveop(char *channel, char *nicks);
void mode3(char *channel, char *mode, char *data);
int is_me(char *nick);
int  i_am_op(char *channel);
int  is_a_bot(char *nick);
int bot_matches(char *nuh);
char *get_channel_and_userlevel(char *to, char **rest, char *from, int *l);
char *get_channel(char *to, char **rest);
char *get_channel2(char *to, char **rest);
char *nick2uh(char *from, char *userhost, int type);
char *format_nuh(char *userhost);
char *format_nuh2(char *userhost);
char *format_uh(char *userhost, int type);
void deop_ban(char *channel, char *nick, char *nuh);
void deop_siteban(char *channel, char *nick, char *nuh);
void deop_bannick(char *channel, char *nick);
void deop_sitebannick(char *channel, char *nick);
void not_opped(char *from, char *channel);
int  toggle(char *from, int *bleah, char *what, ...);
void signoff(char *from, char *reason);
char *cluster(char *host);
int  numchar(char *string, int c);
char *right(char *string, int num);
void no_chan(char *from);
int string_all_caps(char *string);
int num_words_all_caps(char *string);
int percent_caps(char *string);
char *encode(char *passwd);
int passmatch(char *plain, char *encoded);
char *lastuh(void);
int check_for_number(char *from, char *string);
void do_who(char *chan);

/* com-ons.c stuff */
void addseen(char *nick, int type, char *rest);
void do_showseen(char *from, char *to, char *rest, int cmdlevel);
void do_seen(char *from, char *to, char *rest, int cmdlevel);

void    on_part(char *who, char *channel);
void    on_nick(char *who, char *newnick);
void    on_quit(char *from, char *rest);
void	on_topic(char *from, char *channel, char *topic);
void    on_msg( char *from, char *to, char *next );
void    on_mode( char *from, char *to, char *rest );
void    on_kick(char *from, char *channel, char *nick, char *reason);
void    on_join( char *who, char *channel );
void    on_public( char *from, char *to, char *rest);
int     level_needed(char *name);
int	set_access(char *name, int level);
int	write_levelfile(char *filename);
int	read_levelfile(char *filename);

/* combot.c stuff */

char *readline(FILE *f, char *s);

int read_lists_all(void);
void init_bots(void);
aBot *find_bot(char *s);
aBot *add_bot(char *nick);
int update_login(char *login);
int update_ircname(char *ircname);
int set_server(char *servername, int port);
int add_server(char *servername, int port);
void start_all_bots(void);
int kill_bot(char *reason);
void kill_all_bots(char *reason);
void start_all_bots(void);
int connect_to_server(void);
void reconnect_to_server(void);
void set_server_fds(fd_set *reads, fd_set *writes);
int readln(char *buf);
int send_to_server(char *format, ...);
#ifdef USE_CCC
int send_to_ccc(char *format, ...);
#endif
void try_reconnect(void);
void reset_botstate(void);
void parse_server_input(fd_set *read_fds);
void parse_ccc_input(fd_set *read_fds);
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

/* log.c stuff */

char *get_logname(int which);
int set_logname(char *name, char *value);
void log_it(int which, int global, char *fmt, ...);
void free_logs(void);

/* match.c stuff */

int matches(char *ma, char *na);
int num_matches(char *spec, char *what);

/* memory.c stuff */

void dcc_sendnext(char *user);
char *getuserhost_frommem(char *nick);
aMemory *find_memorybynick(char *nick);
aMemory *find_memory(char *user);
aMemory *create_memory(char *user);
void    change_memnick(char *olduser, char *newnick);
void   remove_memory(aMemory *cptr);
void    delete_memory(aMemory **l_list);
void    cleanup_memory(void);
void     check_memory(char *user);
void    nodcc_session(char *from, char *to, char *rest);
void    hasdcc_session(char *from);
int	get_protlevel_matches(char *userhost, char *chan);
aUser	*get_shitrec_matches(char *userhost, char *channel);

/* parse.c stuff */

#ifdef USE_CCC
void	ccc_cuser(char *lin);
void	ccc_cshit(char *lin); 
void	ccc_die(char *lin);
void	ccc_add_bot(char *lin);
void	ccc_del_bot(char *lin);
void	ccc_del_all(char *lin);
void	ccc_rshit_all(char *lin);
void	ccc_add_user(char *lin);
void	ccc_del_user(char *lin);
void	ccc_add_shit(char *lin);
void	ccc_del_shit(char *lin);
void	ccc_add_auth(char *lin);
void	ccc_del_auth(char *lin);
void	ccc_do_nick(char *lin);
void	parsecccline(char *lin);
#endif /* USE_CCC */
void	parse_topic(char *from, char *rest);
void    parse_invite(char *from, char *rest);
void    parse_privmsg(char *from, char *rest);
void    parse_notice(char *from, char *rest);
void    parse_wallops(char *from, char *rest);
void    parse_join(char *from, char *rest);
void    parse_part(char *from, char *rest);
void    parse_mode(char *from, char *rest);
void    parse_nick(char *from, char *rest);
void    parse_kick(char *from, char *rest);
void    parse_ping(char *from, char *rest);
void    parse_pong(char *from, char *rest);
void    parse_quit(char *from, char *rest);
void    parse_error(char *from, char *rest);
void    parse_001(char *from, char *rest);
void    parse_200(char *from, char *rest);
void    parse_204(char *from, char *rest);
void    parse_205(char *from, char *rest);
void    parse_213(char *from, char *rest);
void    parse_219(char *from, char *rest);
void    parse_243(char *from, char *rest);
void    parse_251(char *from, char *rest);
void    parse_252(char *from, char *rest);
void    parse_253(char *from, char *rest);
void    parse_254(char *from, char *rest);
void    parse_255(char *from, char *rest);
void	parse_262(char *from, char *rest);
void    parse_301(char *from, char *rest);
void    parse_311(char *from, char *rest);
void    parse_312(char *from, char *rest);
void    parse_313(char *from, char *rest);
void    parse_317(char *from, char *rest);
void    parse_318(char *from, char *rest);
void    parse_319(char *from, char *rest);
void    parse_324(char *from, char *rest);
void    parse_352(char *from, char *rest);
void	parse_361(char *from, char *rest);
void	parse_364(char *from, char *rest);
void	parse_365(char *from, char *rest);
void    parse_367(char *from, char *rest);
void    parse_368(char *from, char *rest);
void    parse_381(char *from, char *rest);
void    parse_433(char *from, char *rest);
void    parse_451(char *from, char *rest);
void    parse_471(char *from, char *rest);
void    parse_481(char *from, char *rest);
void    parse_491(char *from, char *rest);
void    parseline(char *line);

/* send.c stuff */

int     sendoper(char *sendto, char *format, ...);
int     sendkill(char *sendto, char *format, ...);
int     sendkline(char *sendto, char *format, ...);
int     sendprivmsg(char *sendto, char *format, ...);
int     sendnotice(char *sendto, char *format, ...);
int     sendregister(char *nick, char *login, char *ircname);
int     sendtopic(char *channel, char *topic);
int     sendping(char *to);
int     sendpong(char *to);
int     sendnick(char *nick);
int     sendjoin(char *channel, char *key);
int     sendpart(char *channel);
int     sendquit(char *reason);
int     sendmode(char *to, char *format, ...);
int     sendkick( char *channel, char *nick, char *format, ... );
int     mysend_ctcp(char *to, char *format, ...);
int     send_ctcp_reply(char *to, char *format, ...);
int     send_ctcp(char *to, char *format, ...);
int     sendison(char *nick);
int     senduserhost(char *nick);
int     send_to_user(char *to, char *format, ...);

/* server.c stuff */

int connect_by_number(int service, char *host);
int read_from_socket(int s, char *buf);
int send_to_socket(int sock, char *format, ... );

/* userlist.c stuff */

void uptime_time(aTime **l_list, time_t thetime);
int saveuserlist(aUser **l_list, char *filename);
int verified(char *userhost);
int change_password(aUser **l_list, char *userhost, char *password);
aUser *check_kicksay(char *string, char *channel);
aUser *find_kicksay(aUser **l_list, char *string, char *channel);
void show_kicksaylist(char *from, aUser *l_list);
aUser *check_saysay(char *string, char *channel);
aUser *find_saysay(aUser **l_list, char *string, char *channel);
void show_saysaylist(char *from, aUser *l_list);
aUser *find_user(aUser **l_list, char *userhost, char *channel);
int is_user(char *userhost, char *channel);
aUser *add_to_userlist(aUser **, char *, int, int, int, char *, char *);
void search_userlist(char *from, aUser *l_list, char *pattern);
void search_userlist2(char *from, aUser *l_list, char *pattern);
void search_userlist3(char *from, aUser *l_list, char *pattern);
void show_userlist(char *from, aUser *l_list);
void remove_user(aUser **, aUser *);
void delete_all_userlist(aUser **);
void delete_userlist(aUser **);
int password_needed(char *userhost);
int correct_password(char *userhost, char *password);
int is_aop(char *userhost, char *channel);
int max_userlevel(char *userhost);
int get_userlevel(char *userhost, char *channel);
int get_protlevel(char *userhost, char *channel);
int write_userlist(aUser **l_list, char *filename);
int read_userlist(aUser **l_list, char *filename);
aUser *find_shit(aUser **, char *, char *);
aUser *find_greet(aUser **, char *);
void show_greetlist(char *from, aUser *l_list);
aUser *find_protnick(aUser **, char *);
aUser *add_to_shitlist(aUser **, char *, int, char *, char *, char *, long int,
	long int);
void show_shitlist(char *from, aUser *l_list);
void remove_shit(aUser **, aUser *);
void delete_shitlist(aUser **);
int is_shitted(char *userhost, char *channel);
char *who_shitted(char *userhost, char *channel);
char *get_shit_reason(char *userhost, char *channel);
long int get_shitexpire(char *userhost, char *channel);
int change_shitlevel(char *userhost, char *channel, int newlevel);
int change_shitreason(char *from, char *userhost, char *channel, char
*newreason);
int get_shitlevel(char *userhost, char *chan);
char *get_shituserhost(char *userhost, char *channel);
char *get_shituh(aUser *dummy);
char *get_shitreason(aUser *dummy);
aList *find_list(aList **l_list, char *thing);
aList *find_list2(aList **l_list, char *thing);
aList *make_list(aList **l_list, char *thing);
void remove_list(aList **l_list, aList *cptr);
void delete_list(aList **l_list);
aTime *find_time(aTime **l_list, char *name);
aTime *make_time(aTime **l_list, char *name);
void remove_time(aTime **l_list, aTime *cptr);
void delete_time(aTime **l_list);
void update_time(aTime **l_list, time_t time);
int read_timelist(aTime **l_list, char *filename);
int write_timelist(aTime **l_list, char *filename);
int write_seenlist(aSeen **l_list, char *filename);
int read_seenlist(aSeen **l_list, char *filename);
void remove_seen(aSeen **l_list, aSeen *cptr);
void delete_seen(aSeen **l_list);
aSeen *find_seen(aSeen **l_list, char *nick);
aSeen *make_seen(aSeen **l_list, char *nick, char *userhost, char *kicker, time_t time, int selector, char *signoff);
int write_biglist(aBigList **l_list, char *filename);
int read_biglist(aBigList **l_list, char *filename);
void remove_biglist(aBigList **l_list, aBigList *cptr);
void delete_biglist(aBigList **l_list);
aBigList *find_biglist(aBigList **l_list, char *user);
aBigList *make_biglist(aBigList **l_list, char *user, time_t time, int num, char *data1, char *data2);

/* vars.c stuff */

int  get_int_var(char *name, int which);
char *get_str_var(char *name, int which);
int  get_int_varc(aChannel *channel, int which);
char *get_str_varc(aChannel *channel, int which);
void set_int_varc(aChannel *channel, int which, int value);
void set_str_varc(aChannel *channel, int which, char *value);
int set_str_var(char *name, int which, char *value);
int set_int_var(char *name, int which, int value);
int find_var_name(char *name, int type);
VarStruct *find_var(char *name, int which);
void free_strvars(VarStruct *vars);
void copy_vars(VarStruct *old, VarStruct *new);
void dup_defaultvars(void);

/* energy.c stuff */

void	screwban_format(char *string);
void	deop_screwban(char *channel, char *nick, char *nuh);
char	*randstring(char *filebing);
int 	randx(int min, int max);

/* ftp_dcc.c */

#ifndef _POSIX_SOURCE
#  ifndef _S_IFMT
#    define _S_IFMT 0170000
#  endif /* _S_IFMT */
#  ifndef _S_IFLNK
#    define _S_IFLNK 0120000
#  endif /* _S_IFLNK */
#  ifndef _S_IFSOCK
#    define _S_IFSOCK 0140000
#  endif /* _S_IFSOCK */
#  ifndef S_ISLNK
#    define S_ISLNK(mode)	(((mode) & (_S_IFMT)) == (_S_IFLNK))
#  endif /* S_ISLNK */
#  ifndef S_ISSOCK 
#    define S_ISSOCK(mode)	(((mode) & (_S_IFMT)) == (_S_IFSOCK))
#  endif /* S_ISSOCK */
#endif /* NOT _POSIX_SOURCE */

void pwd(char *from);
char *real_root(char *path, char *buf);
int rr_stat(char *name, struct stat *buf);
int rr_access(char *name, int mode);
DIR *rr_opendir(char *dirname);
char *make_dir(char *old, char *new, char *dirbuffer);
void sort_dir(char *list[], int n);
void output_dir(char *from, char *path, char *list[], int n);
int read_dir(char *list[], char *path, char *pattern);
void close_dir(char *list[], int n);
void do_ls(char *from, char *path);
void send_file(char *from, char *name);
int send_file_from_list(char *user, char *path, char *file);
void do_chdir(char *from, char *new_dir);
FILE *openindex(char *from, char *name);

char *get_ftext( FILE *ffile );
void add_list_event	(char *, ...);

/* session.h */

int		delete_session(char *user);
void	cleanup_sessions();
int		check_session(char *user, char *to);
int		check_flood2(char *user, char *to);
char	*get_cwd(char *user);
void	change_dir(char *user, char *new_dir);
void    dcc_sendnext2(char *user);
int     dcc_sendlist(char *user, char *path, char *file_list[], int n);
void	do_showqueue();
void	sendreply(char *s, ...);

/* monitor.h */
#ifdef SERVER_MONITOR

void parse_server_notice(char *from, char *rest);
void parse_serv_nkline(char *from, char *rest);
void parse_serv_con(char *from, char *rest);
void parse_serv_exit(char *from, char *rest);
void parse_serv_rejecti(char *from, char *rest);
void parse_serv_rejectb(char *from, char *rest);
void parse_serv_invu(char *from, char *rest);
void parse_serv_conf(char *from, char *rest);
void parse_serv_ipmis(char *from, char *rest);
void parse_serv_ucon(char *from, char *rest);
void parse_serv_links(char *from, char *rest);
void parse_serv_flood(char *from, char *rest);
void parse_serv_kill(char *from, char *rest);
void do_monitor(char *from, char *to, char *rest, int cmdlevel);
void do_monitor_clones(char *from, char *to, char *rest, int cmdlevel);
void do_monitor_bots(char *from, char *to, char *rest, int cmdlevel);
void do_monitor_stats(char *from, char *to, char *rest, int cmdlevel);
void do_monitor_help(char *from, char *to, char *rest, int cmdlevel);
void do_monitor_klinec(char *from, char *to, char *rest, int cmdlevel);
void do_monitor_killc(char *from, char *to, char *rest, int cmdlevel);
void do_monitor_klineb(char *from, char *to, char *rest, int cmdlevel);
void do_monitor_killb(char *from, char *to, char *rest, int cmdlevel);
void do_monitor_rehash(char *from, char *to, char *rest, int cmdlevel);
void do_monitor_servkills(char *from, char *to, char *rest, int cmdlevel);
void do_monitor_killsfor(char *from, char *to, char *rest, int cmdlevel);
void do_monitor_killsfrom(char *from, char *to, char *rest, int cmdlevel);
void do_monitor_topkills(char *from, char *to, char *rest, int cmdlevel);
#endif
void send_monitormsg(char *format, ...);
void do_cumonitor(char *from, char *to, char *rest, int cmdlevel);
void do_rcumonitor(char *from, char *to, char *rest, int cmdlevel);




#endif /* H_H */
