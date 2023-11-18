/*  
 * misc.c
 *
 */

#include <stdio.h>                 /* clean this up! */
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include "misc.h"
#include "config.h"

static	char	usernick[MAXLEN];
static	char	timebuf[MAXLEN];
static 	char	idlestr[MAXLEN];

char  *strlwr(char *string)
{
  int i;
  for (i=0;i<strlen(string);i++)
    string[i] = tolower(string[i]);
  return string;
}

int ison( char *nick )
{
    char ison_line[MAXLEN];
    char *line;

    line = ison_line;

    sendison( nick );
    while( readln( line ) <= 0 ) {};
    
    line = strrchr( line, ':' ) + 1;
    line = strtok( line, " " );
    if( !strcasecmp( line, nick ) )
        return( TRUE );
    return( FALSE );
}

char *getuserhost( nick )
char *nick;

{
  char line[MAXLEN];
  char *ptr, *ptr2;
  
  if (!nick || !*nick)
    return "";

  senduserhost( nick );

  do
    {
      while( readln( line ) <= 0 );
      ptr2 = strcasestr( line, "302");
      if (ptr2 && strcasestr(line,nick))
	{
	  if (!(ptr = strrchr(line, '+')))
	    ptr = strrchr(line, '-');
	  if (!ptr)
	    return "";
	  ptr++;
	  if (!ptr)
	    return "";
	  if (strchr(ptr, '\n'))
	    *strchr(ptr, '\n') = '\0';
	  if (strchr(ptr, '\r'))
	    *strchr(ptr, '\r') = '\0';
	  return( ptr );
	}
      if (!ptr2)
	parseline(line);
    } while (!ptr2);
  return "";
}

int ischannel( char *channel )
{
    return( *channel == '#' || *channel == '&' );
}

char *strcasestr ( s1, s2 )
char      *s1; 
char      *s2;
{
    char n1[256], n2[256];
    char *temp;
    int i;

    for ( i = 0; s1[i] != '\0'; i++ )
        n1[i] = toupper( s1[i] );
    n1[i] = '\0';

    for ( i = 0; s2[i] != '\0'; i++ )
        n2[i] = toupper( s2[i] );
    n2[i] = '\0';
    temp = strstr( n1, n2);
    if (temp != NULL)
      return (s1 + (temp-n1));
    return NULL;

}

char	*getnick( char *nick_userhost )
{
  strcpy(usernick, nick_userhost);
  return(strtok(usernick, "!"));
}
char *gethost(char *nick_userhost)
{
  char *temp;
  temp = strchr(nick_userhost, '@');
  if (temp)
    temp++;
  if (temp)
    temp = strtok(temp, "\r\n\0");
  if (temp)
    strcpy(usernick, temp);
  else
    strcpy(usernick, "");
  return usernick;
}

char *time2str( time_t time )
{
  struct tm   *btime;
  static char *months[] =
    {
      "Jan", "Feb", "Mar", "Apr", "May", "Jun", 
      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
      };
  
  btime = localtime( (time_t *) &time );
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


char *strip(char *temp, char c)
{
  if (temp)
    for (; (temp != NULL) && (*temp == c) ; temp++) ;
  return temp;
}

char *fixuser(char *temp)
{
  temp = strip(temp, '~');
  temp = strip(temp, '#');
}


