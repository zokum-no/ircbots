/*
 * file.h
 * Deals with files 'n stuff
 *
 * (c) 1993 VladDrac (irvdwijk@cs.vu.nl)
 */

#include <stdio.h>
#include "userlist.h"

int	read_from_userfile		(FILE *, char *, int *);
int	read_from_userlistfile	(FILE *, char *, int *, char *, char *);
int	read_from_excludefile	(FILE *, char *);
int	read_from_passwdfile		(FILE *, char *, char *);
int 	readuserdatabase			(char *, userlist_t **);
int 	readuserlistdatabase		(char *, userlist_t **);
int 	readexcludedatabase		(char *, EXCLUDE_list **);
int 	readpasswddatabase		(char *, userlist_t **);
int	read_from_userfile2		(FILE *, char *, int *);
int 	readuserdatabase2			(char *, userlist_t **);
