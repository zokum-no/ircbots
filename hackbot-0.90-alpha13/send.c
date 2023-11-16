/*
 * send.c - all the send functions...makes things easier =)
 * (c) 1994 CoMSTuD (cbehrens@iastate.edu)
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
 */

#include <stdio.h>
#include <stdarg.h>

#include "dcc.h"
#include "send.h"
#include "debug.h"
#include "config.h"
#include "function.h"
#include "hackbot.h"

/*
extern  int   send_to_server(char *format, ...);
extern  int   send_chat(char *to, char *text);
*/

int sendprivmsg(char *sendto, char *format, ...)
{
	char buf[HUGE];
	va_list msg;

	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);
	return send_to_server("PRIVMSG %s :%s", sendto, buf);
}

int sendnotice(char *sendto, char *format, ...)
{
	char buf[HUGE];
	va_list msg;

	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);
	return send_to_server( "NOTICE %s :%s", sendto, buf);
}

int sendregister(nick, login, ircname)
char *nick;
char *login;
char *ircname;
{
	if (!send_to_server("USER %s null null :%s", login, ircname))
		return FAIL;
	return sendnick(nick);
}    

int sendtopic(channel, topic)
char *channel;
char *topic;
{
	return send_to_server("TOPIC %s :%s", channel, topic);
}

int sendping(to)
char *to;
{
	return send_to_server("PING %s", to);
}

int sendnick(nick)
char *nick;
{
	return send_to_server("NICK %s", nick);
}

int sendjoin(channel, key)
char *channel;
char *key;
{
	if (key && *key)
		return send_to_server("JOIN %s %s", channel, key);
	return send_to_server("JOIN %s", channel);
}

int sendpart(channel)
char *channel;
{
	return send_to_server("PART %s", channel);
}

int sendquit(reason)
char *reason;
{
	return send_to_server("QUIT :%s", reason);
}

int sendmode(char *to, char *format, ...)
{
	char buf[HUGE];
	va_list msg;

	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);
	return send_to_server("MODE %s %s", to, buf);
}

int sendkick(char *channel, char *nick, char *format, ...)
{
	char buf[HUGE];
	va_list msg;

	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);
	return send_to_server("KICK %s %s :%s", channel, nick, buf);
}

int send_ctcp_reply(char *to, char *format, ...)
{
	char buf[HUGE];
	va_list msg;

	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);
	return send_to_server("NOTICE %s :\001%s\001", to, buf);
}

int send_ctcp(char *to, char *format, ...)
{
	char buf[HUGE];
	va_list msg;

	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);
	return send_to_server("PRIVMSG %s :\001%s\001", to, buf);
}

int sendison(nick)
char *nick;
{
	return send_to_server("ISON %s", nick);
}

int senduserhost(nick)
char *nick;
{
	return send_to_server("USERHOST %s", nick);
}

int send_to_user(char *to, char *format, ...)
{
	char buf[HUGE];
	int bytessend;
	va_list msg;

	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);
	if (to)
		if (!send_chat(to, buf))
			return sendnotice(getnick(to), "%s", buf);
		else
			return(TRUE);
#ifdef DBUG
	else
		debug(NOTICE, "%s", buf);
#endif
}




