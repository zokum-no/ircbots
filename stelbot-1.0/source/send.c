/*
 * send.c send stuff to the server, but not the basic stuff,
 *        more specified, like sendprivmsg, sendmode etc...
 *
 */

#include <stdio.h>
#include <string.h>

#include "dcc.h"
#include "config.h"

/*
 * sendprivmsg
 * Basically the same as send_to_server, just a little more easy to use
 *
 */

int 	sendprivmsg( sendto, sendtext, arg1, arg2, arg3, arg4, arg5,
                     arg6, arg7, arg8,arg9, arg10 )

char 	*sendto, *sendtext;
char 	*arg1, *arg2, *arg3, *arg4, *arg5,
     	*arg6, *arg7, *arg8, *arg9, *arg10;

{
    	char 	entiremessage[WAYTOBIG];

    	sprintf( entiremessage, sendtext, arg1, arg2, arg3, arg4, arg5,
                 arg6, arg7, arg8, arg9, arg10 );
    	return( send_to_server( "PRIVMSG %s :%s", sendto, entiremessage ) );
}

int 	sendnotice( sendto, sendtext, arg1, arg2, arg3, arg4, arg5,
                    arg6, arg7, arg8,arg9, arg10 )

char 	*sendto, *sendtext;
char 	*arg1, *arg2, *arg3, *arg4, *arg5,
     	*arg6, *arg7, *arg8, *arg9, *arg10;

{
    	char entiremessage[WAYTOBIG];

    	sprintf( entiremessage, sendtext, arg1, arg2, arg3, arg4, arg5,
                 arg6, arg7, arg8, arg9, arg10 );

    	return( send_to_server( "NOTICE %s :%s", sendto, entiremessage ) );
}

int 	sendregister( nick, login, ircname )
char 	*nick;
char 	*login;
char 	*ircname;

{
    	if(!send_to_server("USER %s null null :%s", login, ircname))
        	return FAIL;
    	return(sendnick(nick)); 
}    

int 	sendping( to )
char 	*to;

{
    	return( send_to_server( "PING %s", to ) );
}

int 	sendnick( nick )
char 	*nick;

{
    	return( send_to_server( "NICK %s", nick ) );
}

int 	sendjoin( channel )
char 	*channel;

{
    	return( send_to_server( "JOIN %s", channel ) );
}

int 	sendpart( channel )
char 	*channel;

{
    	return( send_to_server( "PART %s", channel ) );
}

int 	sendquit( reason )
char 	*reason;

{
    	return( send_to_server( "QUIT :%s", reason ) );
}

int 	sendmode( to, pattern, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10 )    
char 	*to;
char 	*pattern;
char 	*arg1, *arg2, *arg3, *arg4, *arg5, 
     	*arg6, *arg7, *arg8, *arg9, *arg10;

{
    	char 	buf[MAXLEN];

    	sprintf( buf, pattern, arg1, arg2, arg3, arg4, arg5, arg6, arg7, 
		 arg8, arg9, arg10 );
    	return( send_to_server( "MODE %s %s", to, buf ) );
}

int 	sendkick( channel, nick, reason )
char	*channel;
char	*nick;
char	*reason;

{
  char buf[MAXLEN];
  if (!reason || !*reason)
    sprintf(buf, "Later %s", nick);
  else
    strcpy(buf, reason);
    	return( send_to_server( "KICK %s %s :%s", channel, nick, buf ) );
}

int 	send_ctcp_reply( to, format, arg1, arg2, arg3, arg4, 
                         arg5, arg6, arg7, arg8, arg9, arg10 )
char 	*to;
char 	*format;
char 	*arg1, *arg2, *arg3, *arg4, *arg5,
     	*arg6, *arg7, *arg8, *arg9, *arg10;

{
    	char 	buf[MAXLEN];
    	sprintf( buf, format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10 );
    	return( send_to_server( "NOTICE %s :\x01%s\x01", to, buf ) );
}

int 	send_ctcp( to, format, arg1, arg2, arg3, arg4, 
                   arg5, arg6, arg7, arg8, arg9, arg10 )
char 	*to;
char 	*format;
char 	*arg1, *arg2, *arg3, *arg4, *arg5,
     	*arg6, *arg7, *arg8, *arg9, *arg10;

{
    	char 	buf[MAXLEN];
    	sprintf( buf, format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10 );
    	return( send_to_server( "PRIVMSG %s :\x01%s\x01", to, buf ) );
}

int 	sendison( nick )
char 	*nick;

{
    	return( send_to_server( "ISON %s", nick ) );
}

int 	senduserhost( nick )
char	*nick;

{
	return( send_to_server( "USERHOST %s", nick ) );
}

int     send_to_user( sendto, sendtext, arg1, arg2, arg3, arg4, arg5,
                      arg6, arg7, arg8,arg9, arg10 )

char    *sendto, *sendtext;
char    *arg1, *arg2, *arg3, *arg4, *arg5,
        *arg6, *arg7, *arg8, *arg9, *arg10;

{
        char    	entiremessage[WAYTOBIG];
	int		bytessend;

        sprintf( entiremessage, sendtext, arg1, arg2, arg3, arg4, arg5,
                 arg6, arg7, arg8, arg9, arg10 );
	if (strchr(entiremessage, '\n'))
	  *strchr(entiremessage, '\n') = ' ';
	if(!send_chat(sendto, entiremessage))
        	return(sendnotice(getnick(sendto), "%s", entiremessage));
	else
		return(TRUE);
}

