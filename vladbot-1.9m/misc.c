/*  
 * misc.c
 * I moved most of this stuff to server.c
 *
 */

#include <stdio.h>                 /* clean this up! */
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <signal.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/time.h>
#include "misc.h"
#include "config.h"

static	char	usernick[MAXLEN];
static	char	timebuf[MAXLEN];
static 	char	idlestr[MAXLEN];

int ison( nick )
char *nick;

{
    char ison_line[MAXLEN];
    char *line;

    line = ison_line;

    sendison( nick );
    while( readln( line ) == 0 ) {};
    
    line = strrchr( line, ':' ) + 1;
    line = strtok( line, " " );
    if( !strcasecmp( line, nick ) )
        return( TRUE );
    return( FALSE );
}

char *getuserhost( nick )
char *nick;

{
    char *line;

    if( ( line = (char *)malloc( MAXLEN ) ) == NULL )
	return;
    senduserhost( nick );
    do
	;
    while( !readln( line ) );

    line = strrchr( line, ':' ) + 1;
    if( strcasestr( line, nick ) != (char *)NULL )
    {
        line = strrchr( line, '+' ) + 1;
        return( line );
    }
    return( (char *)NULL );
}

int ischannel( channel )
char      *channel;

{
    return( *channel == '#' || *channel == '&' );
}

char *strcasestr ( s1, s2 )
char      *s1; 
char      *s2;
{
    char n1[256], n2[256];
    int i;

    for ( i = 0; s1[i] != '\0'; i++ )
        n1[i] = toupper( s1[i] );
    n1[i] = '\0';

    for ( i = 0; s2[i] != '\0'; i++ )
        n2[i] = toupper( s2[i] );
    n2[i] = '\0';
    return( strstr( n1,n2 ) );

}

char	*getnick( nick_userhost )
char	*nick_userhost;

{
	strcpy( usernick, nick_userhost );
	return( strtok( usernick, "!" ) );
}

char	*time2str( time )
long	time;

{
	struct	tm	*btime;
	static	char	*months[] =
	{
		"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};

	btime = localtime( &time );
	if( time && (sprintf( timebuf, "%-2.2d:%-2.2d:%-2.2d %s %-2.2d %d", 
		      btime->tm_hour, btime->tm_min, btime->tm_sec, 
		      months[btime->tm_mon], btime->tm_mday, 
		      btime->tm_year + 1900 ) ) )
		return timebuf;
	return( NULL );
} 

char	*idle2str( time )
long	time;

{
	int	days,
		hours,
		mins,
		secs;
	
	days  = (time/86400);
	hours = ((time - (days*86400))/3600);
	mins  = ((time - (days*86400) - (hours*3600))/60);
	secs  = (time - (days*86400) - (hours*3600) - (mins*60));

	sprintf( idlestr, "%d day%s, %d hour%s, %d minute%s and %d second%s",
		 days, EXTRA_CHAR(days), hours, EXTRA_CHAR(hours),
		 mins, EXTRA_CHAR(mins), secs, EXTRA_CHAR(secs) ); 
	return idlestr;
}
