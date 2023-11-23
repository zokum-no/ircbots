#ifndef SESSION_H
#define SESSION_H

#include "config.h"

#define IS_FLOODING	1
#define IS_OK		2

typedef struct	session_struct
{
	char	*user;			/* nick!user@host.domain 		*/
	long	last_received;		/* (info)  when did we receive last cmd */
	char	*nodcc_to;		/* "to" when there was no dcc		*/
	char	*nodcc_rest;		/* "rest" when there was no dcc		*/
	char	*cwd;			/* (dcc)   current working directory 	*/
	int	currentfile;		/* (dcc)   current file to send		*/
	int	maxfile;		/* (dcc)   total number of files	*/
	struct
	{
		char	*path;
		char	*name;
	} file_list[LISTSIZE];		/* (dcc)   list of files to send 	*/
	long	flood_start;		/* (flood) used for timing-stuff 	*/
	int	flood_cnt;		/* (flood) Number of lines received	*/	
	int	flooding;		/* (flood) Marked true if flooding	*/
	struct	session_struct	*next;	/* next session in list 		*/
} SESSION_list;	
	
SESSION_list	*find_session(char *user);
SESSION_list	*find_session2(char *user);
SESSION_list	*create_session(char *user);
int	delete_session(char *user);
void	cleanup_sessions();
int	check_session(char *user, char *to);
int	check_flood(char *user, char *to);
void	nodcc_session(char *from, char *to, char *rest);
void	hasdcc_session(char *from);
char	*get_cwd(char *user);
void	change_dir(char *user, char *new_dir);
void    dcc_sendnext(char *user);
int     dcc_sendlist(char *user, char *path, char *file_list[], int n);
void	do_showqueue();
void	sendreply(char *s, ...);

#endif
