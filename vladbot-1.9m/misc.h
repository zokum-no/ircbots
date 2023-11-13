/*  
 * misc.h
 *
 */

#include <stdio.h>
#define EXTRA_CHAR(s)	s==1?"":"s"

char	*strcasestr( char *s1, char *s2 );
int 	ison( char *nick );
char 	*getuserhost( char *nick );
int 	ischannel( char *channel );
char	*getnick( char *nick_userhost );
char	*time2str( long time );
char	*idle2str( long time );
