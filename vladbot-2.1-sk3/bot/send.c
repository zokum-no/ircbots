/*
 * send.c send stuff to the server, but not the basic stuff,
 *        more specified, like sendprivmsg, sendmode etc...
 * (c) 1993 VladDrac (irvbdwijk@cs.vu.nl)
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
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "channel.h"
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
	char buf[WAYTOBIG];
	va_list	msg;

	//todo : factoriser
	char  *pbuf = (char*)&buf;
	char utfized[WAYTOBIG*2], *putf = (char*)&utfized;
	CHAN_list *Channel_to;
	size_t lin, lout;
	
	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);
	//et factoriser ce bloc aussi
	Channel_to = search_chan(sendto);
	if(Channel_to && Channel_to->encoder != (iconv_t)-1){
		memset(utfized, 0, sizeof(utfized));
		lin = strlen(buf);
		lout = sizeof(utfized);
		iconv(Channel_to->encoder, &pbuf,&lin, &putf, &lout);
		return(send_to_server("PRIVMSG %s :%s", sendto, utfized));
	}
	else
		return(send_to_server("PRIVMSG %s :%s", sendto, buf));
}

int sendaction (char *sendto, char *format, ...) 
{
	char 	buf[WAYTOBIG];
	va_list	msg;
	char  *pbuf = (char*)&buf;
	char utfized[WAYTOBIG*2], *putf = (char*)&utfized;
	CHAN_list *Channel_to;
	size_t lin, lout;
  
	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);
	Channel_to = search_chan(sendto);
	if(Channel_to && Channel_to->encoder != (iconv_t)-1){
		memset(utfized, 0, sizeof(utfized));
		lin = strlen(buf);
		lout = sizeof(utfized);
		iconv(Channel_to->encoder, &pbuf,&lin, &putf, &lout);
		return(send_to_server("PRIVMSG %s :\001ACTION %s\001", sendto, utfized));
	}
	else
		return(send_to_server("PRIVMSG %s :\001ACTION %s\001", sendto, buf));
}


int	sendnotice(char *sendto, char *format, ...)
{
	char 	buf[WAYTOBIG];
	va_list	msg;
	char  *pbuf = (char*)&buf;
	char utfized[WAYTOBIG*2], *putf = (char*)&utfized;
	CHAN_list *Channel_to;
	size_t lin, lout;

	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);
	Channel_to = search_chan(sendto);
	if(Channel_to && Channel_to->encoder != (iconv_t)-1){
		memset(utfized, 0, sizeof(utfized));
		lin = strlen(buf);
		lout = sizeof(utfized);
		iconv(Channel_to->encoder, &pbuf,&lin, &putf, &lout);
    	return(send_to_server( "NOTICE %s :%s", sendto, utfized));
	}
	else
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

int 	sendpong(char *to)
{
	return(send_to_server("PONG %s", to));
}

int 	sendnick(char *nick)
{
	return(send_to_server("NICK %s", nick));
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
