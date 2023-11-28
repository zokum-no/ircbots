/*
 * server.c - some general serverstuff like connecting and sending.
 *
 */

#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "config.h"


int connect_by_number(int service, char *host)
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
		struct	hostent		*hp;
		struct	in_addr		MyHostAddr;
		char	localhost[64];

		gethostname(localhost, 64);
		if(hp=gethostbyname(localhost))
			bcopy(hp->h_addr, (char *) &MyHostAddr, sizeof(MyHostAddr));
		bzero(&localaddr, sizeof(struct sockaddr_in));
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


int	read_from_socket( s, buf )
int	s;
char	*buf;

{
	char	smallbuf;
	int	bufnum;

	bufnum = 0;

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

int	send_to_socket( sock, format, arg1, arg2, arg3, arg4, arg5,
		        arg6, arg7, arg8,arg9, arg10 )

int  	sock;
char 	*format;
char 	*arg1, *arg2, *arg3, *arg4, *arg5,
     	*arg6, *arg7, *arg8, *arg9, *arg10;

{
    	char 	bigbuf[WAYTOBIG];          
	int	bytes_written;

    	sprintf(bigbuf, format, arg1, arg2, arg3, arg4, arg5,
                 arg6, arg7, arg8, arg9, arg10);
#ifdef DBUG
    	printf("Writing to socket: \"%s\"\n", bigbuf);
#endif
    	strcat(bigbuf, "\n");

    	bigbuf[WAYTOBIG-1] = '\0';         /* make sure it ends with 0 */

	return(write(sock, bigbuf, strlen(bigbuf)));
}


