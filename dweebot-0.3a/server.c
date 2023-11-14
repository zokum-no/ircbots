/*
 * server.c - some general serverstuff like connecting and sending.
 */

#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <stdarg.h>

#include "debug.h"
#include "dweeb.h"
#include "config.h"

extern  botinfo *currentbot;

int connect_by_number(service, host)
int service;
char *host;
{
#ifndef MSDOS             /* For testing at home */
	int     s = -1;
	char    buf[100];
	struct  sockaddr_in server;
	struct  hostent *hp;

	if (service == -2)
	{
		server=(*(struct sockaddr_in *) host);
	}
	else if (service > 0)
	{
		if (!host)
		{
			gethostname(buf, sizeof(buf));
			host = buf;
		}
		if ((service > 0) && ((server.sin_addr.s_addr =
				inet_addr(host)) == -1))
		{
			if (hp = gethostbyname(host))
			{
				bzero((char *) &server, sizeof(server));
				bcopy(hp->h_addr, (char *) &server.sin_addr,
					hp->h_length);
				server.sin_family = hp->h_addrtype;
			}
			else
				return (-2);
		}
		else
			server.sin_family = AF_INET;
		server.sin_port = (unsigned short) htons(service);
	}
	if (((service == -1) && ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)) ||
	    ((service != -1) && ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)))
		return (-3);
	if (service != -1)
	{
		setsockopt(s, SOL_SOCKET, SO_LINGER, 0, 0);
		setsockopt(s, SOL_SOCKET, SO_REUSEADDR, 0, 0);
		setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, 0, 0);
	}
	if (service <= 0 && service != -2)
	{
		struct  sockaddr_in     localaddr;
		struct  hostent         *hp;
		struct  in_addr         MyHostAddr;
		char    localhost[64];

		gethostname(localhost, 64);
		if(hp=gethostbyname(localhost))
			bcopy(hp->h_addr, (char *) &MyHostAddr, sizeof(MyHostAddr));
		bzero((char*)&localaddr, sizeof(struct sockaddr_in));
		localaddr.sin_family = AF_INET;
		if (!service)
			localaddr.sin_addr.s_addr = INADDR_ANY;
		else
			localaddr.sin_addr=MyHostAddr;
		localaddr.sin_port = 0;
		if (bind(s, (struct sockaddr *) &localaddr,
			sizeof(localaddr)) == -1 ||
			(!service && listen(s, 1) == -1))
		{
			close(s);
			return -4;
		}
		service = sizeof(localaddr);
		getsockname(s, (struct  sockaddr *) &localaddr, &service);
		return (s);
	}
	if (connect(s, (struct sockaddr *) & server, sizeof(server)) < 0)
	{
		close(s);
		return (-4);
	}
	return (s);
#endif
}


int read_from_socket(s, buf)
int s;
char *buf;
{
	char    smallbuf;
	int     bufnum;

	bufnum = 0;
	if (s==-1)
		return(-1);

	do
	{
		if (read( s, &smallbuf, 1 ) <= 0 )
			return -1;
		if (bufnum < HUGE - 1 )
			buf[bufnum++] = smallbuf;
	}
	while (smallbuf != '\n');
	buf[bufnum] = '\0';
	return bufnum;
}

int send_to_socket(int sock, char *format, ... )
{
	char bigbuf[HUGE];
	int  bytes_written;
	va_list msg;
	
	if (sock==-1)
		return(-1);
	va_start(msg, format);
	vsprintf(bigbuf, format, msg);

#ifdef DBUG
	debug(NOTICE, "send_to_socket(): \"%s\"", bigbuf);
#endif
	strcat(bigbuf, "\n");

	bigbuf[HUGE-1] = '\0';         /* make sure it ends with 0 */
	va_end(msg);
	return(write(sock, bigbuf, strlen(bigbuf)));
}







