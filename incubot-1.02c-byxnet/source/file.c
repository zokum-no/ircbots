/*
 * file.c - Deals with files 'n stuff
 * (c) 1993 VladDrac (irvdwijk@cs.vu.nl)
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
#ifndef SYSV
#include <strings.h>
#endif
#include <string.h>

#include "file.h"
#include "userlist.h"
#include "config.h"

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

int	read_from_userlistfile(FILE *stream, char *usrhost, int *level, char *mailaddr, char *nick)
{
	char	s[MAXLEN];
	char	*p;

		  do
			p = fgets(s, MAXLEN, stream);
		  while((p != NULL) && (*s == '#'));

	if( p == NULL )
		return( FALSE );

	sscanf(s, "%s %3d %s %s", usrhost, level, mailaddr, nick);
	return(TRUE);
}

int	read_from_excludefile(FILE *stream, char *usrhost)
{
	char	s[MAXLEN];
	char	*p;

		  do
			p = fgets(s, MAXLEN, stream);
		  while((p != NULL) && (*s == '#'));

	if( p == NULL )
		return( FALSE );

	sscanf(s, "%s", usrhost);
	return(TRUE);
}

int	read_from_passwdfile(FILE *stream, char *usrhost, char *passwd)
{
	char	s[MAXLEN];
	char	*p,*p2;

		  do
			p = fgets(s, MAXLEN, stream);
		  while((p != NULL) && (*s == '#'));

	if( p == NULL )
		return( FALSE );

	if (*p=='\n')
		return (FALSE);
	p=s;
	while (*p==' ') p++;
	p2=strstr(p," ");
	*p2 = '\0';
	p2++;
	strcpy(usrhost,p);
	p2[strlen(p2)-1]='\0';
	strcpy(passwd,p2);

	return(TRUE);
}

int 	readuserdatabase(char *fname, USERLVL_list **lvllist)
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
			add_to_levellist(lvllist, usrhost, usrlevel, NULL, NULL);
		fclose(fp);
		return(TRUE);
}

int 	readuserlistdatabase(char *fname, USERLVL_list **lvllist)
{
		FILE 	*fp;
		char 	usrhost[MAXLEN];
		char	mailaddr[MAXLEN];
		char	nick[MAXLEN];
		int  	usrlevel;

		if((fp = fopen( fname, "r")) == NULL)
	{
		printf("File \"%s\" not found, aborting\n", fname);
		exit(0);
	}

		while(read_from_userlistfile(fp, usrhost, &usrlevel, mailaddr, nick))
			add_to_levellist(lvllist, usrhost, usrlevel, mailaddr, nick);

		fclose(fp);
		return(TRUE);
}

int 	readexcludedatabase(char *fname, EXCLUDE_list **lvllist)
{
		FILE 	*fp;
		char 	usrhost[MAXLEN];

		if((fp = fopen( fname, "r")) == NULL)
	{
		printf("File \"%s\" not found, aborting\n", fname);
		exit(0);
	}
		while(read_from_excludefile(fp, usrhost))
			add_to_excludelist(lvllist, usrhost);

		fclose(fp);
		return(TRUE);
}

int 	readpasswddatabase(char *fname, USERLVL_list **lvllist)
{
		FILE 	*fp;
		char 	usrhost[MAXLEN];
		char  	usrpasswd[MAXLEN];

		if((fp = fopen( fname, "r")) == NULL)
	{
		printf("File \"%s\" not found, aborting\n", fname);
		exit(0);
	}
		while(read_from_passwdfile(fp, usrhost, usrpasswd))
			add_to_passwdlist(lvllist, usrhost, usrpasswd);

		fclose(fp);
		return(TRUE);
}

int	read_from_userfile2(FILE *stream, char *usrhost, int *level)
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

int 	readuserdatabase2(char *fname, USERLVL_list **lvllist)
{
		FILE 	*fp;
		char 	usrhost[MAXLEN];
		int  	usrlevel;

		if((fp = fopen( fname, "r")) == NULL)
	{
		printf("File \"%s\" not found, aborting\n", fname);
		exit(0);
	}

		while(read_from_userfile2(fp, usrhost, &usrlevel))
			add_to_levellist(lvllist, usrhost, usrlevel, NULL, NULL);
		fclose(fp);
		return(TRUE);
}

