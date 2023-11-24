#ifndef USERLIST_H
#define USERLIST_H

#include "config.h"
#include "autoconf.h"

typedef struct	USERLVL_struct
{
	char	*userhost;		/* OffSpring!User@Host */
	int	access;			/* accesslevel */
	struct	USERLVL_struct	*next;	/* next user */
} USERLVL_list;

/* Channel Steal Code */
typedef struct ListType
{
        char name[MAXLEN];
       struct ListType *next;
} ListStruct;
typedef struct ShitType
{ 
        char userhost[MAXLEN];          /* OffSpring!User@Host */
        int  level;                     /* Access level or ShitLevel */
        struct ListType *channels;
        struct ShitType *next;    /* next user */
} ShitStruct;
typedef struct TimeStruct
{
	char   *name;
	int	time;
	int   	num;
	struct TimeStruct *next;
} TS;


USERLVL_list	*finduserhost( USERLVL_list **l_list, char *userhost );
USERLVL_list	**init_levellist( );
ListStruct *find_list(ListStruct *WhichList, char *thename);
int	add_to_list( ListStruct **WhichList, char *thename);
int	is_in_list( ListStruct *WhichList, char *thename);
int	remove_from_list( ListStruct **WhichList, char *thename );
void	add_to_levellist( USERLVL_list **l_list, char *userhost, int level );
int	remove_from_levellist( USERLVL_list **l_list, char *userhost );
void	delete_levellist( USERLVL_list **l_list );
int	get_level( USERLVL_list **l_list, char *userhost );
int	count_lvllist( char *from, USERLVL_list **l_list );
void    show_lvllist( USERLVL_list **l_list, char *from, char *userhost );
int remove_all_from_list( ListStruct **WhichList );
ListStruct *init_list( void );
TS *is_in_timelist(TS **l_list, char *name);
TS **init_timelist(void);
int add_to_timelist(TS **l_list, char *name);
int remove_from_timelist(TS **l_list, char *name);
void delete_timelist(TS **l_list, int time);
void remove_all_from_timelist(TS **l_list);

#endif /* USERLIST_H */
