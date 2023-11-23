#ifndef USERLIST_H
#define USERLIST_H

typedef struct	EXCLUDE_struct
{
	char	*userhost;		/* badcrc@wewp.com */
	struct	EXCLUDE_struct	*next; /* next excluded user */
} EXCLUDE_list;

typedef struct	USERLVL_struct
{
	char	*userhost;		/* VladDrac!irc@blah */
	char	*passwd;
	char	*mailaddr;		/* email address */
	char	*nick;			/* Usual nickname */
	int		access;			/* accesslevel */
	int		lock;
	struct	USERLVL_struct	*next;	/* next user */
} USERLVL_list;
typedef struct	USERLVL_struct2
{
	char	*userhost;		/* VladDrac!irc@blah */
	int	access;			/* accesslevel */
	char	*greetings[10];
	struct	USERLVL_struct2	*next;	/* next user */
} USERLVL_list2;

USERLVL_list	*find_userhost				(USERLVL_list **, char *, int);
EXCLUDE_list	*find_excludehost			(EXCLUDE_list **, char *, int);
USERLVL_list	**init_levellist			();
EXCLUDE_list	**init_excludelist		();
int				add_to_levellist			(USERLVL_list **, char *, int, char *, char *);
int				add_to_excludelist		(EXCLUDE_list **l_list, char *);
int				add_to_passwdlist			(USERLVL_list **, char *, char *);
void				change_passwdlist			(USERLVL_list **, char *, char *);
int				remove_from_levellist	(USERLVL_list **, char *);
int				remove_from_excludelist	(EXCLUDE_list **, char *);
void				delete_levellist			(USERLVL_list **);
void				delete_excludelist		(EXCLUDE_list **);
int				get_level					(USERLVL_list **, char *);
int				change_email				(USERLVL_list **, char *, char *);
int				change_userlist_nick		(USERLVL_list **, char *, char *);
char				*get_email					(USERLVL_list **, char *);
char				*get_userlist_nick		(USERLVL_list **, char *);
int				get_userlist_info			(USERLVL_list **, char *, char *, int);
char				*get_mask					(USERLVL_list **, char *);
char				*get_name					(USERLVL_list **, char *);
char				*get_passwd					(USERLVL_list **, char *);
int 				userpasswdfail				(USERLVL_list **, char *);
int 				get_userpasswdfail		(USERLVL_list **, char *);
int 				check_lock					(USERLVL_list **, char *);
int				lock_passwd					(USERLVL_list **, char *);
int				unlock_passwd				(USERLVL_list **, char *);
int				clear_userpasswdfail		(USERLVL_list **, char *);
int 				get_userpasswdfail		(USERLVL_list **, char *);
int 				set_userpasswdfail		(USERLVL_list **, char *);
int				get_level2					(USERLVL_list2 **, char *, char ***);
void				show_lvllist				(char *, USERLVL_list **, char *, char *, int, int);
int				show_lvllist_pattern		(char *, USERLVL_list **, char *, char *, int);
void				show_excludelist			(char *, EXCLUDE_list **, char *, char *);
int				show_excludelist_pattern(char *, EXCLUDE_list **, char *, char *);
int				write_lvllist				(USERLVL_list **, char *);
int				write_excludelist			(EXCLUDE_list **, char *);
int				write_passwdlist			(USERLVL_list **, char *);

#endif /* USERLIST_H */
