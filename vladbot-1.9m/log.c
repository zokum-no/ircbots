/*
 *
 */

#include <stdio.h>
#include <time.h>
#include "dcc.h"
#include "config.h"

void 	botlog( logfile, logfmt, arg1, arg2, arg3, arg4, arg5,
                arg6, arg7, arg8,arg9, arg10 )

char 	*logfile, *logfmt;
char 	*arg1, *arg2, *arg3, *arg4, *arg5,
     	*arg6, *arg7, *arg8, *arg9, *arg10;

{
    	char 	entiremessage[WAYTOBIG];
	FILE	*flog;
	time_t	T;
	char	*time_string;

	if( ( T = time( (time_t *)NULL ) ) != -1 )
	{
		time_string = ctime( &T );	
		*( time_string + strlen( time_string ) - 1 ) = '\0';
	}	
	if( ( flog = fopen( logfile, "a" ) ) == NULL )
		return;
    	sprintf( entiremessage, logfmt, arg1, arg2, arg3, arg4, arg5,
                 arg6, arg7, arg8, arg9, arg10 );
	fprintf( flog, "%s: %s\n", time_string, entiremessage );
	fclose( flog );
}
