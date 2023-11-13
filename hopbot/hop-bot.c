/*  hopbot   */   
/*====[bot.c] Bot v0.01   11/7/91 - Peyote Coyote  Generic bot shell ==*/

#include <stdio.h>
#include <strings.h>
#include "hop.h"
char spy_link[MAXLEN];
int spy;
int alive = 1;
int current_count;
char current_channel[MAXLEN], current_nick[MAXLEN]; 
int s;                        /* IRC socket                  */ 
char buf[MAXLEN];             /* global text data buffer     */
char localhost[64];	      /* the local machine's name    */
char buff[MAXLEN];

/*===[ privmsg ]==============================================================
  Got a private message from 'from'.  Find out what he wants.
============================================================================*/
void send_privmsg(from, line)
  char *from;
  char *line;
{
  char *text;
  char *to;
  int i;

#ifdef DBUG
  printf("SEND Privmsg: from = %s, line = %s\n", from ,line);
#endif

  if((text = index(line, ' ')) != 0 ) {
    *(text) = '\0'; text += 2;
  } 
  to = line;
/*  for(i=0;text[i];i++) text[i] = tolower(text[i]); */
  send_on_msg(from, to, text);
  if(spy) {
     strtok(from, "!");
     sprintf(buf, "PRIVMSG %s spy: %s %s %s\n",
             spy_link, from, to, text);
     writeln(buf);
  }




#ifdef DEBUG
  printf("SEND text from %s to %s: %s \n", from, to, text);
  printf("SEND Exiting privmsg correctly\n");
#endif
}

/*===[ parse ]================================================================
============================================================================*/
int send_parse(line)
  char *line;
{
  char *from; 
  char *comm;
  char *rest;
  int  numeric;
  char *from_nick;


#ifdef DBUG
  printf("SEND Entered parse, line = %s", line);
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
  
  if (numeric = atoi(comm))  {} 
    /*numbered_command(from, numeric, rest)*/

  if(strcmp(comm, "KILL") == 0) {
      sprintf(buf, "QUIT\n");
      writeln(buf);
      Start_a_bot(current_nick, current_channel, current_count);
  }
 else  
  if(numeric == 433) {
          strcat(current_nick, "_");
          sprintf(buf, "NICK %s\n", current_nick);
          writeln(buf);
  } 
  else if(strcmp(comm, "PRIVMSG") == 0) {
         send_privmsg(from, rest);
  }
 
 else if (strcmp(comm, "JOIN") == 0)
    send_on_join(from, rest);
    
  else if (strcmp(comm, "PING") == 0) {
    sprintf(buf, "PONG %s\n", localhost);
    writeln(buf);
    sprintf(buf, "PRIVMSG %s PONG\n", current_channel); 
    writeln(buf);
  }

#ifdef DBUG
  printf("SEND comm = %s, from = %s.\n", comm, from);
  printf("Exiting parse correctly\n\n");
#endif
}

/*===[ main ]=================================================================
  Yes, main.  amazing.
============================================================================*/
main()
{
   Start_a_bot( IRCNAME, CHANNEL, 0);
}


/*===================================================================*/
Start_a_bot( name , channel , number)
char name[30], channel[30];  
int number;
{
 
  char hostname[64];
  char *logfile=NULL;
  int c, errflag;
  char line[MAXLEN];
  int it_is_written=TRUE;
 char ircname[40]; 
#ifdef DBUG
  printf("Entered Send_main\n");
#endif

  sprintf(current_channel, channel); 
  sprintf(hostname, "%s", IRCSERVER);
  gethostname(localhost, 64);
  
  if ((s=call_socket(hostname))==-1) {
      fprintf(stderr, "Could not connect to %s, aborting\n", hostname);
      exit(0);
  }
  sprintf(current_nick, "%s", name);
  sprintf(buf, "NICK %s\n", current_nick);
  writeln(buf);
  
  sprintf(ircname, "G!%i", number); 
  sprintf(buf, "USER %s 1 1 %s\n", ircname, IRCGECOS);
  writeln(buf);
  
  sprintf(buf, "JOIN %s\n", channel );
  writeln(buf);

 
  while( alive ) {   
      current_count++;
      readln(line);
      send_parse(line);
  }

}
