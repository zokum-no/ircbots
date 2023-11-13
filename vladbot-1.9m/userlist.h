#ifndef USERLIST_H
#define USERLIST_H

#include "config.h"

typedef struct	USERLVL_struct
{
	char	userhost[MAXLEN];	/* VladDrac!irc@blah */
	int	access;			/* accesslevel */
	struct	USERLVL_struct	*next;	/* next user */
} USERLVL_list;



USERLVL_list	*find_userhost( USERLVL_list **l_list, char *userhost );
USERLVL_list	**init_levellist( );
void	add_to_levellist( USERLVL_list **l_list, char *userhost, int level );
int	remove_from_levellist( USERLVL_list **l_list, char *userhost );
int	get_level( USERLVL_list **l_list, char *userhost );
void    show_lvllist( USERLVL_list **l_list, char *from, char *userhost );

#endif /* USERLIST_H */
