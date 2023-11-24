#ifndef CHANNEL_H
#define CHANNEL_H

#include "chanuser.h"
#include "config.h"
#include "autoconf.h"
#include "userlist.h"

/*
 * SOFTRESET means: only join "failed" channels
 * HARDRESET: delete all users and join all channels (i.e. after a kill)
 */
#define SOFTRESET 0
#define HARDRESET 1

typedef struct	BAN_struct
{
	char	*banstring;		/* banned nick!user@host */
	char	*bannedby;
	int	time;
	struct	BAN_struct	*next;	/* Next ban */
} BAN_list;

typedef	struct	CHAN_struct
{
	char	*name;			/* channel's name */
	int	active;			/* are we there? */
	USER_list	*users;		/* user on that channel */
	BAN_list	*banned;	/* banned user on channel */
	unsigned	int	mode;	/* private etc. */
	char	*key;			/* usefull if +k */
	char	limit[20];		/* if +l (why char? shut up! :) */
	char	*topic;			/* Startup-topic */
	char	*mod;			/* Startup-mode */
	int	log;			/* Channel logging on/off */
	int	shit;			/* Toggle for shit kicks */
	int	aop;			/* Toggle for auto oping */
	int	prot;			/* Toggle for prot enfment */
	int	so;			/* Toggle for strict oping */
	int	sd;			/* Toggle for server deoping */
	int	ik;			/* Idlekick toggle */
	int	mass;			/* Toggle for mass prot */
	int	bk;			/* Toggle for Beep Kick */
	int	ck;			/* Toggle for Caps Kick */
	int	sing;
	int	bonk;
	int	pub;
	int	greet;
	int	wall;
	int	userlimit;		/* used in ~limit command */
        char 	*kickedby;              /* n!u@h who kicked me -pb */
	struct ListType *spylist; /* Channel Spy ;) -pb */
	TS 		**HostList;
	struct	CHAN_struct	*next;	/* ptr to next channel */
} CHAN_list;

#define MODE_CHANOP	CHFL_CHANOP
#define MODE_VOICE	CHFL_VOICE
#define MODE_PRIVATE	0x0004
#define MODE_SECRET	0x0008
#define MODE_MODERATED	0x0010
#define MODE_TOPICLIMIT	0x0020
#define MODE_INVITEONLY 0x0040
#define MODE_NOPRIVMSGS	0x0080
#define MODE_KEY	0x0100
#define MODE_BAN	0x0200	
#define MODE_LIMIT	0x0400	
#define MODE_FLAGS	0x07ff


CHAN_list	*search_chan(char *);
int	mychannel( char *name);
void	add_channel(CHAN_list *);
int	delete_channel(CHAN_list *);
void	delete_all_channels();
void	copy_channellist(CHAN_list *);
int	join_channel(char *, char *, char *, int);
int	leave_channel(char *);
int	mark_success(char *);
int	mark_failed(char *);
void	show_channellist(char *);
void	reset_channels(int);
char	*currentchannel();
int	add_user_to_channel(char *, char *, char *, char *);
int	remove_user_from_channel( char *, char *);
void	change_nick(char *, char *);
void	remove_user(char *);
void	shit_check(char *channel);
int	show_users_on_channel(char *, char *);
void	del_channelmode(char *, unsigned int, char *);
void	change_usermode(char *, char *, unsigned int);
int	open_channel(char *);
void	channel_massop(char *, char *);
void    channel_massdeop(char *, char *);
void    channel_masskick(char *, char *);
int	invite_to_channel(char *, char *);
char	*myuser(char *);
char	*username(char *);
unsigned int	usermode(char *, char *);
BAN_list	*search_ban(BAN_list **, char *);
void    add_ban(BAN_list **, char *, char *, int);
int	delete_ban(BAN_list **, char *);
void    clear_all_bans(BAN_list **);
void    channel_massunban(char *);
void	channel_unban(char *, char *);
int	find_highest(char *, char *);
void show_idletimes(char *from, char *channel, int seconds);
int check_massdeop(char *from, char *channel);
void update_idletime(char *from, char *channel);
int check_nickflood(char *from);
int is_opped(char *nick, char *channel);
char *get_kickedby(char *name);
int set_kickedby(char *name, char *from);
int check_pubflooding(char *from, char *channel);
int tot_num_on_channel(char *channel);
int check_capskick(char *from, char *channel, int num);
void cycle_channel(char *channel);
int   mycheckmode(char *from);
char *findchannel(char *to, char **rest);
void	send_wall(char *from, char *channel, char *wall, ...);

#endif /* CHANNEL_H */
