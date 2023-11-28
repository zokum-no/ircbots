/*  
 * misc.h
 *
 */

#include <stdio.h>
#include <time.h>
#define EXTRA_CHAR(s)	s==1?"":"s"

char *strlwr(char *string);
char	*strcasestr( char *s1, char *s2 );
int 	ison( char *nick );
char 	*getuserhost( char *nick );
int 	ischannel( char *channel );
char	*getnick( char *nick_userhost );
char *gethost(char *nick_userhost);
char	*time2str( time_t time );
char	*idle2str( long time );

char *strip(char *temp, char c);
char *fixuser(char *temp);





