/*
 * server.c - some general serverstuff like connecting and sending.
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include "config.h"
#include "misc.h"
#include "server.h"


int connect_by_number(unsigned int service, char *host)
{
  int s = -1;
  char buf[100];
  struct sockaddr_in server;
  struct hostent *hp;
#ifdef VHOST
  char vhost[MAXLEN];

  strcpy(vhost, VHOST);
#endif
  if (service == -2)
  {
    server = (*(struct sockaddr_in *) host);
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
      {
        Debug(DBGWARNING, "Cant resolve %s", host);
        /*herror(host); */
	return (-2);
      }
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
    struct sockaddr_in localaddr;
    struct hostent *hp;
    struct in_addr MyHostAddr;
    char localhost[64];

    gethostname(localhost, 64);
    if((hp = gethostbyname(localhost)))
      bcopy(hp->h_addr, (char *) &MyHostAddr, sizeof(MyHostAddr));
    bzero(&localaddr, sizeof(struct sockaddr_in));

    localaddr.sin_family = AF_INET;
    if (!service)
      localaddr.sin_addr.s_addr = INADDR_ANY; 
#ifdef VHOST
    else if(*vhost)
      localaddr.sin_addr.s_addr = inet_addr(vhost);
#endif
    else
      localaddr.sin_addr = MyHostAddr;
    localaddr.sin_port = 0;
    if (bind(s, (struct sockaddr *) &localaddr,
	     sizeof(localaddr)) == -1 ||
	(!service && listen(s, 1) == -1))
    {
      close(s);
      return -4;
    }
    service = sizeof(localaddr);
    getsockname(s, (struct sockaddr *) &localaddr, &service);
    return (s);
  }
  else
  {
#ifdef VHOST
    struct sockaddr_in localaddr;
    struct hostent *hp;
    struct in_addr MyHostAddr;

    bzero(&localaddr, sizeof(struct sockaddr_in));     

    localaddr.sin_family = AF_INET;
    localaddr.sin_addr.s_addr = inet_addr(vhost);
    localaddr.sin_port = 0;
    bind(s, (struct sockaddr *) &localaddr, sizeof(localaddr));
#endif
  }
  if (connect(s, (struct sockaddr *) &server, sizeof(server)) < 0)
  {
    close(s);
    return (-4);
  }
  return (s);
}

#ifdef NOTDEF
int fetchit(int LiveSock, char *InBuffer)
{
    fd_set fdvar;
    int c, totalread = 0;
    static struct timeval polltime;
    int done=FALSE;

    polltime.tv_sec = (unsigned long) 0;
    polltime.tv_usec = (unsigned long) 30;

    if (c = read(LiveSock, InBuffer, INBUFFSIZE))
    {
      FD_ZERO(&fdvar);
      FD_SET(LiveSock,&fdvar);
      totalread = c; 
      while ((totalread < INBUFFSIZE) && (!done) )
      {
        polltime.tv_usec = (unsigned long) 30;
        if ((select(LiveSock+1,&fdvar,NULL,NULL,&polltime)) >= 0)
        {
          if (FD_ISSET(LiveSock,&fdvar))
          {
            FD_ZERO(&fdvar);
            FD_SET(LiveSock,&fdvar);
            if (c=read(LiveSock, InBuffer+totalread, INBUFFSIZE - totalread))
              totalread += c;
            else
              done = true;
          }
          else
            done = true;
        }
        else
          done = true;
      }
      bzero(InBuffer+totalread,(INBUFFSIZE - totalread) + 1);
    }
    else
    {
      return(0);
    }
    return(1);
}

#endif

int read_from_socket(int s, char *buf)
{
  char smallbuf;
  int bufnum;

  bufnum = 0;

  do
  {
    if (read(s, &smallbuf, 1) <= 0)
      return (-1);
    if (bufnum < MAXLEN - 1)
      buf[bufnum++] = smallbuf;
  }
  while ((smallbuf != '\n'));
  buf[bufnum] = '\0';
  buf[MAXLEN] = '\0';
  return (bufnum);
}

#ifdef NOTDEF   /* This belongs in a museum.. */
int send_to_socket(sock, format, arg1, arg2, arg3, arg4, arg5,
		   arg6, arg7, arg8, arg9, arg10)

     int sock;
     char *format;
     char *arg1, *arg2, *arg3, *arg4, *arg5, *arg6, *arg7, *arg8, *arg9, *arg10;

{
  char bigbuf[WAYTOBIG];
  int bytes_written;

  snprintf(bigbuf, WAYTOBIG-2, format, arg1, arg2, arg3, arg4, arg5,
	  arg6, arg7, arg8, arg9, arg10);
  bigbuf[WAYTOBIG-1] = '\0';
  Debug(DBGNOTICE, "-> %s", bigbuf);

  strcat(bigbuf, "\n");
  return (write(sock, bigbuf, strlen(bigbuf)));
}
#endif

/* Hey look maw its that new fangled c with va_lists */
int send_to_socket(int sock, char *format, ...)
{
  char bigbuf[WAYTOBIG];
  va_list args;

  va_start(args, format);
  vsnprintf(bigbuf, WAYTOBIG-2, format, args);  /* Make the string */
  va_end(args);

  bigbuf[WAYTOBIG-1] = '\0';                    /* Lets be sure were terminated.. */
  strcat(bigbuf, "\n");                         /* Cap it off */
  Debug(DBGNOTICE, "-> %s", bigbuf);
  return(write(sock, bigbuf, strlen(bigbuf)));  /* Send it out the socket */
}  
