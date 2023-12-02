#ifndef CHANUSER_H
#define CHANUSER_H

#include "config.h"
#include "time.h"

typedef struct USER_struct
{
	char	nick[MAXNICKLEN];
	char	user[MAXLEN];		
	char	host[MAXLEN];
	unsigned  int mode;
	time_t nctime, idletime, floodtime;
	int numnc, floodnum;
	struct USER_struct *next;
} USER_list;

#define CHFL_CHANOP 0x0001
#define CHFL_VOICE  0x0002
#define CHFL_BAN    0x0004   /* do we need this? */

USER_list *search_user( USER_list **u_list, char *nick );
void	  add_user( USER_list **u_list, char *nick, char *user, char *host );
int	  delete_user( USER_list **u_list, char *nick );
USER_list *next_user( USER_list *Old );
int       change_user_nick( USER_list **u_list, char *oldnick, 
				  char *newnick, char *channel );
void		clear_all_users( USER_list **u_list );
USER_list	*get_nextuser( USER_list *old );
unsigned int get_usermode( USER_list *User );
char *get_username( USER_list *User );
char *get_usernick( USER_list *User );
void		add_mode( USER_list **u_list, unsigned int mode, char *param );
void		del_mode( USER_list **u_list, unsigned int mode, char *param );
#endif /* CHANUSER_H */


