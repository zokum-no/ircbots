/*
 * file.c
 * Deals with files 'n stuff
 *
 * (c) 1993 VladDrac (irvdwijk@cs.vu.nl)
 */

#include <stdio.h>
#include "userlist.h"

int	read_from_userfile( FILE *stream, char *usrhost, int *level );
int 	readuserdatabase( char *fname, USERLVL_list **list );
