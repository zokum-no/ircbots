#include <stdio.h>
#ifndef SHITLIST_H
#define SHITLIST_H

typedef struct shitlist_s
{
	char					*mask;
	int					level;
	char					*whyshitted;
	char					*whenshitted;
	char					*whoshitted;
	long					expire;
	struct shitlist_s	*next;
}shitlist_t;


int			remove_from_shitlist			(shitlist_t **, char *);
void			delete_shitlist				(shitlist_t **);
shitlist_t	**init_shitlist				(void);
shitlist_t *find_shitmask					(shitlist_t **t, char *, int);
int			add_to_shitlist				(shitlist_t **, char *, int, char *, char *, char *, long);
void			do_whoshitted					(char *, char *, char *);
void			do_whenshitted					(char *, char *, char *);
void			do_chshitlevel					(char *, char *, char *);
void			do_chwhenshitted				(char *, char *, char *);
void			do_chshitreason				(char *, char *, char *);
void			do_chshitmask					(char *, char *, char *);
void			do_chwhoshitted				(char *, char *, char *);
void			do_expireadd					(char *, char *, char *);
void			do_expiredel					(char *, char *, char *);
void			show_expire_list				(char *, char *, char *);
int			show_expirelist_pattern 	(char *, shitlist_t **, char *, char *);
int			check_expirations				(shitlist_t **);
void			show_shitlist2					(shitlist_t **, char *, char *, char *);
void			show_shitlist					(char *, char *, char *);
void			show_shit_list					(char *, shitlist_t **, char *, char *, int, int);
int			remove_from_shitlist			(shitlist_t **, char *);
int			shitlevel						(char *);
char			*shitmask						(char *);
int			read_from_shitlistfile		(FILE *, char **, int *, char **, char **, char **, long *);
int			readshitlistdatabase			(char *, shitlist_t **);
int			num_shitlist_matches			(shitlist_t **, char *, int);
int			show_shitlist_pattern		(char *, shitlist_t **, char *, char *, int);
int			write_shitlist					(shitlist_t **, char *);
void			do_nshitadd						(char *, char *, char *);
void			do_shitadd						(char *, char *, char *);
void			do_shitwrite					(char *, char *, char *);
void			do_shitdel						(char *, char *, char *);
void			do_nshitdel						(char *, char *, char *);

#endif