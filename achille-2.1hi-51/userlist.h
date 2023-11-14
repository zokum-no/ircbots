#ifndef USERLIST_H
#define USERLIST_H

typedef struct	USERLVL_struct
{
  char	*userhost;		/* VladDrac!irc@blah */
  int	access;			/* accesslevel */
  struct	USERLVL_struct	*next;	/* next user */
} USERLVL_list;

USERLVL_list    *exist_userhost (USERLVL_list **, char *);
USERLVL_list	*find_userhost(USERLVL_list **, char *);
USERLVL_list	**init_levellist();
void	add_to_levellist(USERLVL_list **, char *, int);
int	remove_from_levellist(USERLVL_list **, char *);
void	delete_levellist(USERLVL_list **);
int	get_level(USERLVL_list **, char *);
int	get_level_neg(USERLVL_list **, char *, int *);
void    add_to_level (USERLVL_list **, char *, int);
void	show_lvllist(USERLVL_list **, char *, char *);
void    cancel_level(USERLVL_list **, int);
int	write_lvllist(USERLVL_list **, char *);

#endif /* USERLIST_H */
