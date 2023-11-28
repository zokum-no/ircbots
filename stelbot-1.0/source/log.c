/*
 *
 */

#include <stdio.h>
#include <time.h>
#include "dcc.h"
#include "config.h"
#include "function.h"
#include "misc.h"

extern int logtoggle;
extern char *botlogfile[FNLEN];
extern char bot_dir[FNLEN], current_nick[MAXNICKLEN];

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

	if (!logtoggle && !strcasecmp(logfile, botlogfile))
	  return;

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

void logchat(char *channel, char *msg)
{
  char *temp, filename[FNLEN];
  char buffer[MAXLEN];
  FILE *out;
  time_t currenttime;

  strcpy(buffer, strip(channel, '#'));
  strlwr(buffer);
  sprintf(filename, "%s%s.logchat.%s", bot_dir, current_nick, buffer);
  currenttime = getthetime();
  temp = ctime(&currenttime);
  *(temp+strlen(temp)-1) = '\0';
  sprintf(buffer, "%s: ", temp);
  strcat(buffer, msg);

  if ((out = fopen(filename, "a")) == NULL)
    return;
  fprintf(out, "%s\n", buffer);
  fclose(out);
}


