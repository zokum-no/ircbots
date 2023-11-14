#ifndef CHANNEL_H
#define CHANNEL_H

#include <time.h>

#include "userlist.h"
#include "chanuser.h"
#include "config.h"

/*
 * SOFTRESET means: only join "failed" channels
 * HARDRESET: delete all users and join all channels (i.e. after a kill)
 */
#define SOFTRESET 0
#define HARDRESET 1

typedef struct  BAN_struct
{
	char    *banstring;             /* banned nick!user@host */
	char    *bannedby;
	time_t  time;
	struct  BAN_struct *next;       /* Next ban */
} BAN_list;

typedef struct  CHAN_struct
{
	char *name;                  /* channel's name */
	int active;                  /* are we there? */
	USER_list *users;            /* user on that channel */
	BAN_list *banned;            /* banned user on channel */
	unsigned int mode;           /* private etc. */
	/* toggles: */
	int sdtoggle,
		 enfmtoggle,
		 iktoggle,
		 sotoggle,
		 shittoggle,
		 aoptoggle,
		 pubtoggle,
		 toptoggle,
		 prottoggle,
		 masstoggle,
		 bktoggle,
		 cktoggle;
	char *key;                   /* usefull if +k */
	char limit[20];              /* if +l (why char? shut up! :) */
	char *topic;                 /* Enforced-topic */
	char *mod;                   /* Enforced-mode */
	char *kickedby;              /* n!u@h who kicked me */
	int  userlimit;              /* Bleah */
	LS **SpyList;
	TS **HostList;
	struct CHAN_struct *next;    /* ptr to next channel */
} CHAN_list;

#define MODE_CHANOP     CHFL_CHANOP
#define MODE_VOICE      CHFL_VOICE
#define MODE_PRIVATE    0x0004
#define MODE_SECRET     0x0008
#define MODE_MODERATED  0x0010
#define MODE_TOPICLIMIT 0x0020
#define MODE_INVITEONLY 0x0040
#define MODE_NOPRIVMSGS 0x0080
#define MODE_KEY        0x0100
#define MODE_BAN        0x0200  
#define MODE_LIMIT      0x0400  
#define MODE_FLAGS      0x07ff

void    send_common_spy(char *from, char *format, ...);
int     set_kickedby(char *name, char *from);
char    *get_kickedby(char *name);
void    show_clonebots(char *from, char *channel);
void    check_clonebots(char *from, char *channel);
void    check_limit(char *channel);
int     set_limit(char *channel, int num);
int     is_present(char *who);
void    op_all_chan(char *who);
void    check_bans(void);
void    check_shit(void);
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
CHAN_list *search_chan( char *name );
void    add_channel( CHAN_list *Channel );
int     delete_channel( CHAN_list *Channel );
void    delete_all_channels();
void    copy_channellist(CHAN_list *c_list);
int     join_channel(char *name, char *key, int dojoin );
int     reverse_mode();
void    cycle_channel(char *channel);
int     mychannel(char *name);
int     add_spy(char *name, char *nuh);
int     del_spy(char *name, char *nuh);
void    show_spy_list(char *from, char *name);
void    send_spymsg(char *format, ...);
void    send_spy(char *channel, char *format, ...);
int     is_spy(char *chan, char *nuh);
int     get_iktoggle(char *name);
int     get_sdtoggle(char *name);
int     get_aoptoggle(char *name);
int     get_prottoggle(char *name);
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
int     mark_success( char *name );
int     mark_failed( char *name );
void    change_nick( char *oldnick, char *newnick );
void    remove_user( char *nick );
void    show_channellist( char *user );
void    reset_channels( int Mode );
char    *currentchannel();
int     add_user_to_channel( char *channel, char *nick, char *user, char *host );
int     remove_user_from_channel( char *channel, char *nick );
void    change_nick( char *oldnick, char *newnick );
void    remove_user( char *nick );
void    chan_stats(char *from, char *channel);
int     show_users_on_channel( char *from, char *channel );
int     open_channel( char *channel );
void    add_channelmode(char *from, char *channel, unsigned int mode, char *params, time_t time );
void    del_channelmode( char *channel, unsigned int mode, char *params );
void    change_usermode( char *channel, char *user, unsigned int mode );
int     is_opped(char *who, char *channel);
char    *username( char *nick );
unsigned int usermode( char *channel, char *nick );
int     is_banned(char *who, char *channel);
BAN_list *search_ban( BAN_list **b_list, char *banstring );
void    add_ban( BAN_list **b_list, char *from, char *banstring, time_t time );
int     delete_ban(BAN_list **b_list, char *banstring );
void    clear_all_bans( BAN_list **b_list );
int     channel_massop(char *channel, char *pattern);
int     channel_massdeop(char *channel, char *pattern);
int     channel_masskick(char *channel, char *pattern, char *reason);
int     channel_masskickban(char *channel, char *pattern, char *reason);
int     channel_masskickbanold(char *channel, char *pattern, char *reason);
int     channel_massunbanfrom(char *channel, char *pattern, time_t seconds);
int     channel_massunban(char *channel, char *pattern, time_t seconds);
int     channel_unban(char *channel, char *user);

#endif /* CHANNEL_H */







