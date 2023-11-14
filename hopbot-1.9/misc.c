/*  
 *  misc.c
*/
#include <stdio.h>
#include <strings.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "hop.h"

int s;                        /* IRC socket                  */ 
char buf[MAXLEN];             /* global text data buffer     */
char localhost[64];	          /* the local machine's name    */

/*===[ call_socket ]==========================================================
  Connect to port IRCPORT on host 'hostname', returning the socket
  value.  Return -1 on any errors.
============================================================================*/
int call_socket(hostname)
  char *hostname;
{
  struct sockaddr_in sa;
  struct hostent     *hp;
  int    a, s;

#ifdef DBUG
  printf("Entered call_socket, hostname = %s\n", hostname);
#endif

  if ((hp=gethostbyname(hostname))==NULL) {
    errno=ECONNREFUSED;
    return(-1);
  }
  bzero(&sa, sizeof(sa));
  bcopy(hp->h_addr, (char *)&sa.sin_addr, hp->h_length);
  sa.sin_family = hp->h_addrtype;
  sa.sin_port = htons((u_short)IRCPORT);

  if((s=socket(hp->h_addrtype, SOCK_STREAM, 0)) < 0)
    return(-1);
  if(connect(s, &sa, sizeof(sa)) < 0) {
    close(s);
    return(-1);
  }
#ifdef DBUG
  printf("Exiting call_socket correctly, socket = %d\n", s);
#endif
  return(s);
}

/*===[ readln ]=======================================================
  Read all characters from socket s until a newline.  Put resulting
  string in buf, ignoring all after the MAXLEN'th character.
====================================================================*/
int readln(buf)
  char *buf;
{
  int to=0;
  char c;
#ifdef DBUG
  printf("Entering readln\n");
#endif 

  do {
    if(read(s, &c, 1)<1) 
      return(0);
    if((c >= ' ') || (c <= 126))
      if(to<MAXLEN-1) 
        buf[to++] = c;
  } while (c != '\n');

  buf[to] = '\0';
   
#ifdef DBUG
  printf("buf = %s", buf);
  printf("Exiting readln correctly\n");
#endif

  return(1);
}

/*===[ writeln ]======================================================
  Send contents of buf to socket s.  Return 0 if the entire buf 
  wasn't written.  Return 1 on a successful write.
====================================================================*/
int writeln(buf)
  char *buf;
{
  int to=0;

#ifdef DBUG
  printf("Entered writeln\n");
  printf("buf = %s\n", buf);
#endif

  buf[MAXLEN-1] = '\0';
  if( write(s, buf, strlen(buf)) < to )
    return(0);
    
  
#ifdef DBUG
   printf("Exited writeln correctly.\n");
#endif

  return(1);
}




/*=====[ is_channel ]========================================================
  Sees if a name is a channel name..
===========================================================================*/
int is_channel(to)
char *to;
{
    return (isdigit(*to) || (*to == '+') ||
	    (*to == '#') || (*to == '-'));
}

/*=====[ strcasestr ]========================================================
  NOT used.  I just pulled it out of Sagebot for possible future use.
===========================================================================*/
/*char *   */
strcasestr (s1, s2)
char *s1, *s2;
{
	char n1[256], n2[256];
	int i;

	for (i=0;s1[i] != '\0';i++)
		n1[i] = toupper (s1[i]);
	n1[i] = '\0';
	for (i=0;s2[i] != '\0';i++)
		n2[i] = toupper (s2[i]);
	n2[i] = '\0';
	return(strstr(n1,n2));

}

