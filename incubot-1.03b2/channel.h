#ifndef CHANNEL_H
#define CHANNEL_H

#include "chanuser.h"
#include "config.h"

/*
 * SOFTRESET means: only join "failed" channels
 * HARDRESET: delete all users and join all channels (i.e. after a kill)
 */
#define SOFTRESET 0
#define HARDRESET 1

typedef struct	BAN_struct
{
	char		*banstring;		/* banned nick!user@host */
	long		unban_time;    /*  */
	struct	BAN_struct	*next;	/* Next ban */
} BAN_list;

typedef	struct	CHAN_struct
{
	char	*name;			/* channel's name */
	int	active;			/* are we there? */
	int	joined;			/* are we REALLY there? */
	int	numbans;
	USER_list	*users;		/* user on that channel */
	BAN_list	*banned;	/* banned user on channel */
	unsigned	int	mode;	/* private etc. */
	char	*key;			/* usefull if +k */
	char	limit[20];		/* if +l (why char? shut up! :) */
	char	*topic;			/* Startup-topic */
	char	*mod;			/* Startup-mode */
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
#define MODE_BAN_367	0x0201
#define MODE_LIMIT	0x0400	
#define MODE_FLAGS	0x07ff


CHAN_list	*search_chan(char *);
int			check_channel_flood(char *,char *, int);
void			check_numbans(char *);
void			check_channel_bans(char *);
void			check_channel_clones(char *, char *);
void			add_channel(CHAN_list *);
int			delete_channel(CHAN_list *);
void			delete_all_channels();
void			copy_channellist(CHAN_list *);
int			join_channel(char *, char *, char *, int);
int			leave_channel(char *);
int			mark_success(char *);
int			mark_failed(char *);
void			show_channellist(char *);
void			reset_channels(int);
char			*currentchannel();
int			add_user_to_channel(char *, char *, char *, char *);
int			remove_user_from_channel( char *, char *);
void			change_nick(char *, char *);
void			remove_user(char *, char *);
int			show_users_on_channel(char *, char *);
void			add_channelmode(char *, unsigned int, char *);
void			del_channelmode(char *, unsigned int, char *);
void			change_usermode(char *, char *, unsigned int);
int			open_channel(char *);
void			channel_massop(char *, char *);
void			channel_massdeop(char *, char *);
void			channel_masskick	(char *, char *, char *);
int			invite_to_channel(char *, char *);
char			*username(char *);
char			*userchannel(char *);
unsigned int usermode(char *, char *);
BAN_list		*search_ban(BAN_list **, char *);
int			add_ban(BAN_list **, char *, int);
int			add_timed_ban(BAN_list **, char *, long);
int			delete_ban(BAN_list **, char *);
void			clear_all_bans(BAN_list **);
void			channel_massunban(char *);
void			channel_unban(char *, char *);
int			find_highest(char *, char *, int);
void			addban(char *, char *, char *);

#endif /* CHANNEL_H */
