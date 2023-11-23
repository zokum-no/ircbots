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
int 	readuserdatabase			(char *, USERLVL_list **);
int 	readuserlistdatabase		(char *, USERLVL_list **);
int 	readexcludedatabase		(char *, EXCLUDE_list **);
int 	readpasswddatabase		(char *, USERLVL_list **);
int	read_from_userfile2		(FILE *, char *, int *);
int 	readuserdatabase2			(char *, USERLVL_list **);
