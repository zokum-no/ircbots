/*
 * file.c - Deals with files 'n stuff
 * (c) 1995 OffSpring (cracker@cati.CSUFresno.EDU)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <strings.h>
#include <string.h>

#include "file.h"
#include "userlist.h"
#include "config.h"
#include "autoconf.h"

int	read_from_userfile(FILE *stream, char *usrhost, int *level)
{
	char	s[MAXLEN];
	char	*p;

        do
	   	p = fgets(s, MAXLEN, stream);
        while((p != NULL) && (*s == '#')); 

	if( p == NULL )
		return( FALSE );

	sscanf(s, "%s %d", usrhost, level);
	return(TRUE);
}
int     readuserdatabase(char *fname, USERLVL_list **lvllist)
{
    	FILE 	*fp;
    	char 	usrhost[MAXLEN];
    	int  	usrlevel;

    	if((fp = fopen( fname, "r")) == NULL)
	{
		printf("File \"%s\" not found, aborting\n", fname);
		exit(0);
	}

    	while(read_from_userfile(fp, usrhost, &usrlevel))
        	add_to_levellist(lvllist, usrhost, usrlevel);
    	fclose(fp);
    	return(TRUE);
}
