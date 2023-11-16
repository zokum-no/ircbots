/*===[ bot.c ]================================================================
  Bot v0.01 
  11/7/91 - Peyote Coyote 
  
  Generic bot shell.
============================================================================*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "bot.h"
#include "ons.h" 

int s;                        /* IRC socket                  */ 
int log_on = 0;               /* debuging log on/off         */
FILE *log;                    /* debugging file              */
char buf[MAXLEN];             /* global text data buffer     */
char localhost[64];	      /* the local machine's name    */

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
  if(log_on) fprintf(log, "R: %s", buf);
#ifdef DBUG
  printf("buf = %s", buf);
  printf("Exiting readln correctly\n\n");
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
  printf("buf = %s", buf);
#endif
  buf[MAXLEN-1] = '\0';
  if( write(s, buf, strlen(buf)) < to )
    return(0);
  if(log_on) fprintf(log, "W: %s", buf);
#ifdef DBUG
  printf("Exited writeln correctly.\n\n");
#endif
  return(1);
}

/*===[ privmsg ]==============================================================
  Got a private message from 'from'.  Find out what he wants.
============================================================================*/
void privmsg(from, line)
  char *from;
  char *line;
{
  char *text;
  char *to;
  int i;

#ifdef DBUG
  printf("Privmsg: from = %s, line = %s\n", from ,line);
#endif

  if((text = index(line, ' ')) != 0 ) {
    *(text) = '\0'; text += 2;
  } 
  to = line;
/*  for(i=0;text[i];i++) text[i] = tolower(text[i]); */
  on_msg(from, to, text);

#ifdef DEBUG
  printf("text from %s to %s: %s \n", from, to, text);
  printf("Exiting privmsg correctly\n");
#endif
}

/*===[ parse ]================================================================
============================================================================*/
int parse(line)
  char *line;
{
  char *from;
  char *comm;
  char *rest;
  int  numeric;
 
#ifdef DBUG
  printf("Entered parse\n");
  printf("line = %s", line);
#endif 

  if(*(line + strlen(line) - 1) == '\n')
    *(line + strlen(line) - 1) = '\0';
  if(*line == ':') {
    if((comm = index(line, ' ')) == 0)
      return;
    *(comm++) = '\0';
    from = line + 1; 
  } else {
      comm = line;
      from = NULL;
  }
  if ((rest = index(comm, ' ')) == 0)
      return;
  *(rest++) = '\0';
  if (numeric = atoi(comm))
    /*numbered_command(from, numeric, rest)*/;
  else if(strcmp(comm, "PRIVMSG") == 0)
    privmsg(from, rest);
  else if (strcmp(comm, "JOIN") == 0)
    on_join(from, rest);
  else if (strcmp(comm, "PING") == 0) {
    sprintf(buf, "PONG %s\n", localhost);
    writeln(buf);
  }

#ifdef DBUG
  printf("comm = %s, from = %s.\n", comm, from);
  printf("Exiting parse correctly\n\n");
#endif
}

/*===[ main ]=================================================================
  Yes, main.  amazing.
============================================================================*/
main(argc, argv)
  int argc;
  char **argv;
{
  char hostname[64];
  char *logfile=NULL;
  int c, errflag;
  extern int optind, opterr;
  extern char *optarg;
  char line[MAXLEN];

#ifdef DBUG
  printf("Entered main, argc = %d\n", argc);
#endif

  while((c=getopt(argc, argv, "?s:l:")) != EOF)
    switch(c) {
      case 's' : strcpy(hostname,optarg); 
                 break;
      case 'l' : logfile=optarg;
                 break;
      case '?' : errflag++;
    }
  if(errflag){
    fprintf(stderr, "Usage: %s [-?] [-s server] [-l logfile]\n", argv[0]);
    exit(2);
  }

  if (!*hostname) strcpy(hostname, IRCSERVER);
  if (logfile) {
    if((log=fopen(argv[2], "a"))==NULL) {
      printf("Error opening %s as debug log file.\n", argv[2]);
      exit(1);
    }
    log_on=1;
  }

  gethostname(localhost, 64);
  if ((s=call_socket(hostname))==-1) {
    fprintf(stderr, "Could not connect to %s, aborting\n", hostname);
    exit(0);
  }
  sprintf(buf, "NICK %s\n", IRCNAME);
  writeln(buf);
  sprintf(buf, "USER %s 1 1 %s\n", IRCNAME, IRCGECOS);
  writeln(buf);
  while(readln(line)) if (!strncmp(line, "NOTICE * :* End", 15)) break;
  writeln("JOIN #koolkat\n");
 
  while(readln(line)) {
    parse(line);
  }

#ifdef DBUG
  printf("Exited main correctly.\n");
#endif
  return(1);
}
