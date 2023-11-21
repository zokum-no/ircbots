#ifndef CHANUSER_H
#define CHANUSER_H

#include "config.h"

typedef struct	USER_struct
{
	char	nick[MAXNICKLEN];
	char	*user;		
	char	*host;
	unsigned 	int 	mode;	/* ov etc. */
	int		ops;
	int		deops;
	int		kicks;
        int floodnum,
               bannum,
               deopnum,
               kicknum,
               nicknum,
               beepnum,
               capsnum;
        long int floodtime,
               bantime,
               deoptime,
               kicktime,
               nicktime,
               beeptime,
               capstime,
               idletime;
	struct	USER_struct	*next;	/* next user */
} USER_list;

#define	CHFL_CHANOP	0x0001
#define CHFL_VOICE	0x0002
#define CHFL_BAN	0x0004	/* do we need this? */

USER_list	*search_user( USER_list **u_list, char *nick );
void		add_user( USER_list **u_list, char *nick, 
			  char *user, char *host );
int		delete_user( USER_list **u_list, char *nick );
USER_list	*next_user( USER_list *Old );
int		change_user_nick( USER_list **u_list, char *oldnick, 
				  char *newnick );
void		clear_all_users( USER_list **u_list );
USER_list	*get_nextuser( USER_list *old );
unsigned int 	get_usermode( USER_list *User );
char		*get_username( USER_list *User );
char		*get_myuser( USER_list *User );
char		*get_usernick( USER_list *User );
void		add_mode( USER_list **u_list, unsigned int mode, char *param );
void		del_mode( USER_list **u_list, unsigned int mode, char *param );
#endif /* CHANUSER_H */
