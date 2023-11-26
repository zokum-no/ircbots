/*
 * send.c - all the send functions...makes things easier =)
 */

#include <stdio.h>
#include <stdarg.h>

#include "global.h"
#include "config.h"
#include "defines.h"
#include "structs.h"
#include "h.h"

int sendoper(char *sendto, char *format, ...)
{
	char buf[HUGE];
	va_list msg;

	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);
	return send_to_server("OPER %s %s", sendto, buf);
}

int sendkline(char *sendto, char *format, ...)
{
	char buf[HUGE];
	va_list msg;

	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);
	return send_to_server("KLINE %s :%s", sendto, buf);
}

int sendkill(char *sendto, char *format, ...)
{
	char buf[HUGE];
	va_list msg;

	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);
	return send_to_server("KILL %s :%s", sendto, buf);
}

int sendprivmsg(char *sendto, char *format, ...)
{
	char buf[HUGE];
	va_list msg;

	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);
	
	current->away_serv = getthetime();
	
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
	int i;

	i = sendnick(nick);
	
	if (!send_to_server("USER %s %s %s :%s", login, getmachinename(), current->serverlist[current->current_server].name, ircname)) {
		return FAIL;
	}
	
	send_to_server("MODE %s %s", nick, current->modes_to_send);
	
	return i;
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

int sendpong(to)
char *to;
{
	return send_to_server("PONG %s", to);
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

int mysend_ctcp(char *to, char *format, ...)
{
	char buf[HUGE];
	va_list msg;
 
	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);
	return send_to_server("PRIVMSG %s :%s", to, buf);
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
	va_list msg;

	if (from_shell)
		return TRUE;
	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);
	if (to)
		if (!send_chat(to, buf))
		{
			if (need_dcc)
				return FALSE;
			else
				return sendnotice(getnick(to), "%s", buf);
		}
		else
			return TRUE;
#ifdef DBUG
	else
		debug(NOTICE, "%s", buf);
#endif
	return FALSE;
}








