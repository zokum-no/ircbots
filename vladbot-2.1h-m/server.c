/*
 * server.c - some general serverstuff like connecting and sending.
 * (c) 1993 VladDrac (irvdwijk@cs.vu.nl)
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
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdarg.h>
#include <unistd.h>

#include "config.h"
#include "debug.h"
#include "vladbot.h"

#ifdef NEXT
int	gethostname(char *, int);
int	bcopy(char *, char *, int);
int	bzero(char *, int);
#endif /* NEXT */

extern	botinfo	*currentbot;

int     connect_by_number(int service, char *host)
{
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
                if (host == null(char *))
                {
                        gethostname(buf, sizeof(buf));
                        host = buf;
                }
                if ((service > 0) && ((server.sin_addr.s_addr =
                                inet_addr(host)) == -1))
                {
                        if((hp = gethostbyname(host)))
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
                struct  sockaddr_in 	localaddr;
		struct	hostent		*hp;
		struct	in_addr		MyHostAddr;
		char	localhost[64];

		gethostname(localhost, 64);
		if((hp=gethostbyname(localhost)))
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
}


int	read_from_socket( int s, char *buf )
{
	char	smallbuf;
	int	bufnum;

	bufnum = 0;
	if(s==-1)
		return(-1);

	do
	{
		if( read( s, &smallbuf, 1 ) <= 0 )
			return( -1 );	
		if( bufnum < MAXLEN - 1 )
			buf[bufnum++] = smallbuf;
	}
	while( (smallbuf != '\n') );
	buf[bufnum] = '\0';
	return( bufnum );
}

int	send_to_socket( int sock, char *format, ... )
{
    	char 	bigbuf[WAYTOBIG];          
	va_list	msg;
	
	if(sock==-1)
		return(-1);
	va_start(msg, format);
	vsprintf(bigbuf, format, msg);

#ifdef DBUG
	debug(NOTICE, "send_to_socket(): \"%s\"", bigbuf);
#endif
    	strcat(bigbuf, "\n");

    	bigbuf[WAYTOBIG-1] = '\0';         /* make sure it ends with 0 */
	va_end(msg);
	return(write(sock, bigbuf, strlen(bigbuf)));
}

