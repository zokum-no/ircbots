/*
 * server.c - some general serverstuff like connecting and sending.
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <stdarg.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "global.h"
#include "config.h"
#include "defines.h"
#include "structs.h"
#include "h.h"

static unsigned long getiaddr(void)
{
  struct hostent *hp;
  char s[121];
  unsigned long ip;
  struct in_addr *in;
  /* could be pre-defined */
  /* also could be pre-defined */
  if (vhost[0]) hp = gethostbyname(vhost);
  else {
    gethostname(s,120);
    hp=gethostbyname(s);
  }
  if (hp==NULL) {
  	return 0;
  }
  in=(struct in_addr *)(hp->h_addr_list[0]);
  ip= (unsigned long) (in->s_addr);
  return ip;
}

int connect_by_number(service, host)
int service;
char *host;
{
	int     s = -1, pram;
	char    buf[100];
	struct  sockaddr_in server;
	struct  hostent *hp;
	
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
#ifdef DBUG
		debug(NOTICE,"connect_by_number(%d, %s) [1]\n", service, host);
#endif
		return (-3);
	}
	else {
#ifdef DBUG
	debug(NOTICE,"connect_by_number() Socket = %d\n", s);
#endif
	pram = 0;	setsockopt(s, SOL_SOCKET, SO_LINGER, (char*) &pram, sizeof(int));
	pram = 1;	setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char*) &pram, sizeof(int));

	}
		
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
			if ((hp = gethostbyname(host)) != NULL)
			{
  bzero((char *)&server,sizeof(struct sockaddr_in));
  server.sin_family=AF_INET;
  server.sin_addr.s_addr=(*vhost ? getiaddr() : INADDR_ANY);
  if (bind(s,(struct sockaddr *)&server,sizeof(server))<0) {
  	perror("bind");
#ifdef DBUG
  	debug(NOTICE,"connect_by_number(%d, %s) [3]\n", service, host);
#endif
    return -1;
  }
  bzero((char *)&server,sizeof(struct sockaddr_in));
  server.sin_family=AF_INET;
  server.sin_port=htons(service);
  /* numeric IP? */
  if ((vhost[strlen(vhost)-1] >= '0') && (vhost[strlen(vhost)-1] <= '9'))
    server.sin_addr.s_addr=inet_addr(host);
  else {
    /* no, must be host.domain */
    alarm(10); hp=gethostbyname(host); alarm(0);
    if (hp==NULL) { 
#ifdef DBUG
		debug(NOTICE,"connect_by_number(%d, %s) [3.5]\n", service, host);
#endif
		return -6;
	}
    memcpy((char *)&server.sin_addr,hp->h_addr,hp->h_length);
    server.sin_family=hp->h_addrtype;
  }
			}
			else {
#ifdef DBUG
				debug(NOTICE,"connect_by_number(%d, %s) [4]\n", service, host);
#endif
				return (-2);
			}
		}
		else {
  bzero((char *)&server,sizeof(struct sockaddr_in));
  server.sin_family=AF_INET;
  server.sin_addr.s_addr=(*vhost ? getiaddr() : INADDR_ANY);
  if (bind(s,(struct sockaddr *)&server,sizeof(server))<0) {
#ifdef DBUG
  	debug(NOTICE,"connect_by_number(%d, %s) [5]\n", service, host);
#endif
    return -1;
  }
  bzero((char *)&server,sizeof(struct sockaddr_in));
  server.sin_family=AF_INET;
  server.sin_port=htons(service);
  /* numeric IP? */
  if ((vhost[strlen(vhost)-1] >= '0') && (vhost[strlen(vhost)-1] <= '9'))
    server.sin_addr.s_addr=inet_addr(host);
  else {
    /* no, must be host.domain */
    alarm(10); hp=gethostbyname(host); alarm(0);
    if (hp==NULL) {
#ifdef DBUG
    	debug(NOTICE,"connect_by_number(%d, %s) [6]\n", service, host);
    	return -6;
#endif
    }
    memcpy((char *)&server.sin_addr,hp->h_addr,hp->h_length);
    server.sin_family=hp->h_addrtype;
  }
}
}

	if ((service <= 0) && (service != -2))
	{
		struct  sockaddr_in     localaddr;
		struct  hostent         *hp;
		struct  in_addr         MyHostAddr;
		char    localhost[64];

		gethostname(localhost, 64);
		if((hp=gethostbyname(localhost)) != NULL)
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
#ifdef DBUG
		debug(NOTICE,"connect_by_number(%d, %s) [7]\n", service, host);
#endif
		return (-4);
	}
#ifdef DBUG
	debug(NOTICE,"connect_by_number(%d, %s) [%d]\n", service, host, s);
#endif
	return (s);
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









