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
} userlist_t;

typedef struct usermask_s
{
	char					*userhost;
	struct userlist_s	*owner;
	struct usermask_s	*next;
}usermask_t;

typedef struct userlist_s
{
	char					*nick;
	char					*passwd;
	char					*mailaddr;
	int					access;
	int					prot;
	struct usermask_s	*masks;
	struct userlist_s	*next;
}userlst_t;

userlist_t		*find_userhost				(userlist_t **, char *, int);
EXCLUDE_list	*find_excludehost			(EXCLUDE_list **, char *, int);
userlist_t		**init_levellist			();
EXCLUDE_list	**init_excludelist		();
int				add_to_levellist			(userlist_t **, char *, int, char *, char *);
int				add_to_excludelist		(EXCLUDE_list **l_list, char *);
int				add_to_passwdlist			(userlist_t **, char *, char *);
void				change_passwdlist			(userlist_t **, char *, char *);
int				remove_from_levellist	(userlist_t **, char *);
int				remove_from_excludelist	(EXCLUDE_list **, char *);
void				delete_levellist			(userlist_t **);
void				delete_excludelist		(EXCLUDE_list **);
int				get_level					(userlist_t **, char *);
int				change_email				(userlist_t **, char *, char *);
int				change_userlist_nick		(userlist_t **, char *, char *);
char				*get_email					(userlist_t **, char *);
char				*get_userlist_nick		(userlist_t **, char *);
int				get_userlist_info			(userlist_t **, char *, char *, int);
char				*get_mask					(userlist_t **, char *);
char				*get_name					(userlist_t **, char *);
char				*get_passwd					(userlist_t **, char *);
int 				userpasswdfail				(userlist_t **, char *);
int 				get_userpasswdfail		(userlist_t **, char *);
int 				check_lock					(userlist_t **, char *);
int				lock_passwd					(userlist_t **, char *);
int				unlock_passwd				(userlist_t **, char *);
int				clear_userpasswdfail		(userlist_t **, char *);
int 				get_userpasswdfail		(userlist_t **, char *);
int 				set_userpasswdfail		(userlist_t **, char *);
void				show_lvllist				(char *, userlist_t **, char *, char *, int, int);
int				show_lvllist_pattern		(char *, userlist_t **, char *, char *, int);
void				show_excludelist			(char *, EXCLUDE_list **, char *, char *);
int				show_excludelist_pattern(char *, EXCLUDE_list **, char *, char *);
int				write_lvllist				(userlist_t **, char *);
int				write_excludelist			(EXCLUDE_list **, char *);
int				write_passwdlist			(userlist_t **, char *);

#endif /* USERLIST_H */
