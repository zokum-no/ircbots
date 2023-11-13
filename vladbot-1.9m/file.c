/*
 * file.c
 * Deals with files 'n stuff
 *
 * (c) 1993 VladDrac (irvdwijk@cs.vu.nl)
 */

#include <stdio.h>
#include <strings.h>
#include <string.h>

#include "file.h"
#include "userlist.h"
#include "config.h"

int	read_from_userfile( stream, usrhost, level )
FILE	*stream;
char	*usrhost;
int	*level;

{
	char	s[MAXLEN];
	char	*p;

        do
	   	p = fgets( s, MAXLEN, stream );
        while( ( p != NULL ) && ( *s == '#') ); 

	if( p == NULL )
		return( FALSE );

	sscanf( s, "%s %d", usrhost, level );
	return( TRUE );
}

int 	readuserdatabase( fname, lvllist )
char	*fname;
USERLVL_list	**lvllist;

{
    	FILE 	*fp;
    	char 	usrhost[MAXLEN];
    	int  	usrlevel;

    	if( ( fp = fopen( fname, "r" ) ) == NULL )
	{
		printf( "File \"%s\" not found, aborting\n", fname );
		exit( 0 );
	}

    	while( read_from_userfile( fp, usrhost, &usrlevel ) )
        	add_to_levellist( lvllist, usrhost, usrlevel );
    	fclose( fp );
    	return( TRUE );
}

