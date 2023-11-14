/*
 * send.c send stuff to the server, but not the basic stuff,
 *        more specified, like sendprivmsg, sendmode etc...
 * (c) 1993 VladDrac (irvbdwijk@cs.vu.nl)
 */

#include <stdio.h>
#include <stdarg.h>

#include "dcc.h"
#include "send.h"
#include "debug.h"
#include "config.h"

extern	int	send_to_server(char *format, ...);
extern	int 	send_chat(char *to, char *text);
extern	char	*getnick(char *nick_userhost);

int	sendprivmsg(char *sendto, char *format, ...)
/*
 * Basically the same as send_to_server, just a little more easy to use
 */
{
    	char 	buf[WAYTOBIG];
	va_list	msg;

	va_start(msg, format);
	vsprintf(buf, format, msg);
    	return(send_to_server("PRIVMSG %s :%s", sendto, buf));
	va_end(msg);
}

int	sendaction(char *sendto, char *format, ...)
{
        char    buf[WAYTOBIG];
        va_list msg;

        va_start(msg, format);
        vsprintf(buf, format, msg);
        return(send_to_server( "PRIVMSG %s :\001ACTION %s\001", sendto, buf));
        va_end(msg);
}



int	sendnotice(char *sendto, char *format, ...)
{
    	char 	buf[WAYTOBIG];
	va_list	msg;

	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);
    	return(send_to_server( "NOTICE %s :%s", sendto, buf));
}

int 	sendregister(char *nick, char *login, char *ircname)
{
    	if(!send_to_server("USER %s null null :%s", login, ircname))
        	return FAIL;
    	return(sendnick(nick)); 
}    

int	sendtopic(char *channel, char *topic)
{
	return(send_to_server("TOPIC %s :%s", channel, topic));
}

int 	sendping(char *to)
{
    	return(send_to_server("PING %s", to));
}

int 	sendping2(char *to)
{
    	return(send_to_server("PONG :%s", to));
}

int     sendnick(char *nick)
{
        if(!send_to_server("NICK %s", nick))
                return FAIL;
        return(send_to_server("MODE %s +i", nick)); 
} 

int 	sendjoin(char *channel)
{
    	return(send_to_server("JOIN %s", channel));
}

int 	sendpart(char *channel)
{
    	return( send_to_server( "PART %s", channel ) );
}

int 	sendquit(char *reason)
{
    	return( send_to_server( "QUIT :%s", reason ) );
}

int	sendmode(char *to, char *format, ...)
{
    	char 	buf[MAXLEN];
	va_list	msg;

	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);
    	return( send_to_server( "MODE %s %s", to, buf ) );
}

int 	sendkick( char *channel, char *nick, char *reason )
{
    	return( send_to_server( "KICK %s %s :%s", channel, nick, reason ) );
}

int	send_ctcp_reply(char *to, char *format, ...)
{
    	char 	buf[MAXLEN];
	va_list	msg;

	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);
    	return( send_to_server("NOTICE %s :\001%s\001", to, buf));
}

int	send_ctcp(char *to, char *format, ...)
{
    	char 	buf[MAXLEN];
	va_list	msg;

	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);
    	return(send_to_server( "PRIVMSG %s :\001%s\001", to, buf));
}

int 	sendison(char *nick)
{
    	return(send_to_server("ISON %s", nick));
}

int 	senduserhost(char *nick)

{
	return(send_to_server("USERHOST %s", nick));
}

int	send_to_user(char *to, char *format, ...)
{
    	char 	buf[WAYTOBIG];
	va_list	msg;

	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);
	if(to)
		if(!send_chat(to, buf))
        		return sendnotice(getnick(to), "%s", buf);
		else
			return TRUE;
#ifdef DBUG
	else
		debug(NOTICE, "%s", buf);
#endif
	return TRUE;
}
