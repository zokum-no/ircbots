/*=====[ ons.c for hopbot ]=================================================*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "hop.h"

int i, spy, births;
char spy_link[MAXLEN];
time_t T;
char * time_string;
static char buf[MAXLEN];
extern int writeln();
extern int alive;
char current_channel[MAXLEN], current_nick[MAXLEN];
char newborn_channel[MAXLEN], newborn_nick[MAXLEN];   
int current_count;

/*=====[ on_join ]===========================================================
  "who" has joined channel "channel" - do whatever
===========================================================================*/
void send_on_join(who, channel)
  char *who;
  char *channel;
{
#ifdef DBUG
  printf("SEND On_join: who = %s, channel = %s\n", who, channel);
#endif
#ifdef DBUG
  printf("SEND Exiting on_join\n");
#endif
}

/*=====[ on_msg ]============================================================
  A message was sent from "from" to "to", the text is in "msg"
===========================================================================*/
void send_on_msg(from, to, msg)
  char *from;
  char *to;
  char *msg;
{
  char *com;    
  char *newborn_channel;

#ifdef DBUG
  printf("SEND On_msg: from = %s, to = %s, msg = %s\n", from, to, msg);
#endif

 if(!strcasestr(from, CONFIRM)) return;
 
 if(strcasestr(msg, "PING") ) {
    strtok(from, "!"); 
    sprintf(buf, "PRIVMSG %s PONG: %s\n", current_channel, from); 
    writeln(buf);  
 }
 
/****** SAY ******/
 if(strcasestr(msg, "S.")) {
     strtok(msg, " ");
     msg = strtok(NULL, ".");
     sprintf(buf, "PRIVMSG %s %s\n", current_channel, msg);
     writeln(buf);
 }

/****** DO ******/
 if(strcasestr(msg, "D.")) {
     strtok(msg, " ");
     msg = strtok(NULL, ":");
     sprintf(buf, "%s\n", msg);  writeln(buf);
 } 


if(strcasestr(msg, "DIE")) { 
     strtok(msg, " ");
     com = strtok(NULL, ":");
     sprintf(buf, "QUIT : %s\n", com);
     writeln(buf);
     alive = 0; 
 }

 if(strcasestr(msg, "J.")) {
     strtok(msg, " ");
     com= strtok(NULL, " ");
     strcpy(current_channel,com); 
     sprintf(buf, "JOIN %s\n", com);
     writeln(buf);
 }

 if(strcasestr(msg, "P.")) {
     strtok(msg, " ");
     com = strtok(NULL, " ");
     sprintf(buf, "PART %s\n", com);
     writeln(buf);
 }

 if(strcasestr(msg, "N.")) {
     strtok(msg, " ");
     com = strtok(NULL, " ");
     sprintf(buf, "NICK %s\n", com);
     writeln(buf);
     sprintf(current_nick, "%s", com);
 }

 if(strcasestr(msg, "C.")) {
     strtok(msg, " ");
     com = strtok(NULL, " "); 
     for(i=0; i< atoi(com); i++) {
          sprintf(newborn_nick, "h%i", rand()%10000);
          birth(newborn_nick, current_channel, ++births);
       }

}


 if(strcasestr(msg, "INFO")) { 
       sprintf(buf, "PRIVMSG %s I'm %s (%i|p%i) t=%i c=%s, b=%i\n",
       current_channel, current_nick, getpid(), getppid(), 
       current_count, current_channel,
       births);
       writeln(buf);
 }

if(strcasestr(msg, "F.")) {    
       strtok(msg, " ");
       com = strtok(NULL, " ");
       sprintf(newborn_nick, "%s", com);  
       birth(newborn_nick, current_channel, ++births);
}

if(strcasestr(msg, "TIME")) {
      if( (T=time((time_t *) 0)) != -1) { 
       time_string = ctime(&T);
       sprintf(buf, "PRIVMSG %s %s\n", current_channel, time_string);
       writeln(buf);
      } 
}


if(strcasestr(msg, "SPY:")) {
      strtok(msg, " ");
      if(strcasestr(msg, "OFF")) {
           spy = 0;
           return;
      }
      com = strtok(NULL, " ");
      strcat(spy_link, com);
      sprintf(buf, "PRIVMSG %s Spy link enabled for %s\n",
             spy_link, spy_link);
      writeln(buf);
      spy = 1;
}

#ifdef DBUG
  printf("SEND Exiting on_msg\n");
#endif
}   

/************/
birth( name, chann , number)
char name[30], chann[30]; int number;
{

 if(!fork()) {   
    if( Start_a_bot( name , chann, number) == 0 ) {
       sprintf(buf, "PRIVMSG %s Newbot died at birth\n", current_channel);
       writeln(buf);
       return(0);
    } else {
       return(1);
    }
} else {   
    return(1);
}

}


