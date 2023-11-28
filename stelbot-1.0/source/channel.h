#ifndef CHANNEL_H
#define CHANNEL_H

#include "chanuser.h"
#include "userlist.h"
#include "config.h"

/*
 * SOFTRESET means: only join "failed" channels
 * HARDRESET: delete all users and join all channels (i.e. after a kill)
 */
#define SOFTRESET 0
#define HARDRESET 1

typedef struct	BAN_struct
{
	char	banstring[MAXLEN];	/* banned nick!user@host */
	struct	BAN_struct *next;	/* Next ban */
} BAN_list;

typedef	struct	CHAN_struct
{
  char	       name[MAXLEN];	     /* channel's name */
  int	       active;		     /* are we there? */
  char         enfmodes[MAXLEN];
  USER_list    *users;               /* users on that channel */
  BAN_list     *banned;              /* banned user on channel */
  unsigned int mode;	             /* private etc. */
  char	       key[MAXLEN];	     /* usefull if +k */
  char	       limit[20];	     /* if +l (why char? shut up! :) */
  struct ListType *spylist;    
  struct CHAN_struct *next;	     /* ptr to next channel */
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

void show_spy_list(char *from, char *name);
void send_spy_chan(char *name, char *text);
int is_spy_chan(char *name, char *nick);
int add_spy_chan(char *name, char *nick);
int del_spy_chan(char *name, char *nick);
void update_flood(char *channel, char *nuh);
void update_idletime(char *channel, char *nuh);
void idle_kick(void);
void send_kickall( char *nick, char *msg);
void send_banall(char *nuh);
void send_screwbanall(char *nuh);
int mychannel( char *name);
CHAN_list	*search_chan( char *name );
void	add_channel( CHAN_list *Channel );
char *get_enforced_modes( char *name );
int  set_enforced_modes( char *name, char *themodes);
int  delete_channel( CHAN_list *Channel );
int	join_channel( char *name );
int	leave_channel( char *name );
int	mark_success( char *name );
int	mark_failed( char *name );
int     num_on_channel( char *channel, char *uh);
int     tot_num_on_channel(char *channel);
void    cycle_channel(char *channel);
void	change_nick( char *oldnick, char *newnick );
void	remove_user( char *nick );
void	show_channellist( char *user );
void	reset_channels( int Mode );
void setcurrentchannel( char *channel);
char *currentchannel();
int	add_user_to_channel( char *channel, char *nick, char *user, char *host );
int	remove_user_from_channel( char *channel, char *nick );
void	change_nick( char *oldnick, char *newnick );
void	remove_user( char *nick );
int	show_users_on_channel( char *from, char *channel );
int	open_channel( char *channel );
void	add_channelmode( char *channel, unsigned int mode, char *params );
void	del_channelmode( char *channel, unsigned int mode, char *params );
void	change_usermode( char *channel, char *user, unsigned int mode );
void    channel_massop( char *channel, char *pattern );
void    channel_massdeop( char *channel, char *pattern );
void    channel_masskick( char *channel, char *pattern );
void channel_masskickban( char *channel, char *pattern );
char	*username( char *nick );
unsigned int	usermode( char *channel, char *nick );


BAN_list	*search_ban( BAN_list **b_list, char *banstring );
void	add_ban( BAN_list **b_list, char *banstring );
int	delete_ban(BAN_list **b_list, char *banstring );
void    clear_all_bans( BAN_list **b_list );
void	channel_massunban( char *channel );
void    channel_unban( char *channel, char *user );
#endif /* CHANNEL_H */




