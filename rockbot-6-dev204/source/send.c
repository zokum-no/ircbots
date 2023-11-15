/*
 * send.c send stuff to the server, but not the basic stuff,
 *        more specified, like sendprivmsg, sendmode etc...
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#include "dcc.h"
#include "misc.h"
#include "config.h"
#include "rockbot.h"
#include "send.h"

QueueListType Queue;

extern time_t timeleft, lasttime;

int queue_isto_nick(QueueListType qptr, char *nick)
{
   char *buff;
   char *p1;
   char *p2;
   /* Check for notices etc to a spesific nick. */

   if(!qptr || !qptr->buff)
      return(0);

   buff = (char *)malloc(strlen(qptr->buff)+1);

   strcpy(buff, qptr->buff);

   p1 = (char *)strtok(buff, " ");
   p2 = (char *)strtok(NULL, " ");
   
   if(!p2)
   {
      free(buff);
      return(0);
   }

   if(!strcasecmp(p1, "NOTICE"))
   {
      if(!strcasecmp(p2, nick))
      {
         free(buff);
         return(1);
      }
   }
   free(buff);
   return(0);
}

void clear_queue_to_one(char *nick)
{
    QueueListType qptr, nextq, prevq;
   /* Go though the queue, and remove any
    * pending notices to that person. 
    * Called after a "no such nick". */
    prevq = NULL;
    for(qptr = Queue;qptr;qptr = nextq)
    {
        nextq = qptr->Next;
        if(queue_isto_nick(qptr, nick))
        {
           if(prevq)
              prevq->Next = qptr->Next;
           else
              Queue = qptr->Next;
           free(qptr->buff);
           free(qptr);
        }
        else
          prevq = qptr;
    }

}

void clear_lowpri_queue()
{
  QueueListType qptr, nextq, prevq;

  prevq = NULL;
  for(qptr = Queue;qptr;qptr = qptr->Next)
  {
     if(qptr->priority <= 200)
     { 
         if(prevq)
            prevq->Next = NULL; /* Terminate the list */
         else
            Queue = NULL;
         /* Delete all remaining queues. */
         for(;qptr;qptr = nextq)
         {
            nextq = qptr->Next;
            free(qptr->buff);
            free(qptr);
         }
         return;
     }
     prevq = qptr;
  }
}

void clear_queue()
{
   QueueListType qptr, nextq;

   for(qptr = Queue;qptr;qptr = nextq)
   {
      nextq = qptr->Next;
      free(qptr->buff);
      free(qptr);
   }
   Queue = NULL;
}

int send_from_queue()
{
   QueueListType qptr;

   while(1)
   {
      if(Queue == NULL)
         return(WAIT_SEC);

      timeleft = timeleft + (time(NULL) - lasttime);
      if(timeleft > 9)
         timeleft = 9;
      lasttime = time(NULL);
      if(timeleft < 1)
        return(2);
      timeleft = timeleft - 2;


      qptr = Queue;

      send_to_server(qptr->buff);

      Queue = Queue->Next;
      free(qptr->buff);
      free(qptr);
   }

}

/*
 * send_command()
 * This queues up a command for sending out to the server. 
 * commands are organized by priority; highest first out.
 * this way, we can send large ammounts of output slowly without
 * lagging the whole bot (:
 */

int send_command(int priority, char *format, ...)
{
   extern int ACTIVE_USERLEVEL;
   va_list args;
   QueueListType prevq, qptr, newq;
   char buff[4096];
   va_start(args, format);
   vsprintf(buff, format, args);
   va_end(args);

   Debug(DBGNOTICE, "*> %s", buff);
   priority = priority + ACTIVE_USERLEVEL;
   newq = (QueueListType) malloc(sizeof(struct QueueStruct));
   newq->priority = priority;
   newq->buff = (char *) malloc(strlen(buff)+1);
   strcpy(newq->buff, buff);

  /* add buff to the list */
   prevq = NULL;
   for(qptr = Queue;qptr&&priority <= qptr->priority;qptr = qptr->Next)
   {
      prevq = qptr;
   }

   /* If qptr is null, we hit the end of the list. */
   /* Otherwise, we must insert in the middle. */
   if(qptr != NULL)
   {
     newq->Next = qptr;
   }
   else
   {
     newq->Next = NULL;
   }

   /* If prevq is null, then we are at the beginning of the list. */
   if(prevq == NULL)
   {
      Queue = newq;
   }
   else
   {
      prevq->Next = newq;
   }
   return(0);
}

/*
 * sendprivmsg
 * Basically the same as send_to_server, just a little more easy to use
 *
 */
int sendprivmsg(char *sendto, char *format, ...)
{
   char entiremessage[WAYTOBIG];
   va_list args;


   memset(entiremessage, '\0', WAYTOBIG);
   va_start(args, format);
   vsnprintf(entiremessage, WAYTOBIG-1, format, args);
   va_end(args);
   return(send_command(0, "PRIVMSG %s :%s", sendto, entiremessage));
}

void sendwelcome(char *welcome, char *nick, char *to)
{
   char Welcome[MAXLEN];
   char *tmpbuf;
   
   char buf1[MAXLEN], buf2[MAXLEN];

   /* Protect our string from nullification */
   strcpy(Welcome, welcome);

   if((tmpbuf = strtok(Welcome, "$")))
     strcpy(buf1, tmpbuf);
   else
     strcpy(buf1, "");
   if((tmpbuf = strtok(NULL, "\n\0")))
     strcpy(buf2, tmpbuf);
   else
     strcpy(buf2, "");
   if(!buf1)
   {
     Debug(DBGERROR, "No welcome string!");
     return;
   }
   sendprivmsg(to, "%s%s%s", buf1, nick, buf2?buf2:"" );
}

int sendnotice(char *sendto, char *format, ...)
{
  char entiremessage[WAYTOBIG];
  va_list args;

  memset(entiremessage, '\0', WAYTOBIG);
  va_start(args, format);
  vsnprintf(entiremessage, WAYTOBIG-1, format, args);
  va_end(args);

  return (send_command(0, "NOTICE %s :%s", sendto, entiremessage));
}

int sendregister(char *nick, char *login, char *ircname)
{
  if (!send_to_server("USER %s blah.host blah :%s", login, ircname))
    return FALSE;
  sendnick(nick);
  return TRUE;
}

int sendping(char *to)
{
  return (send_command(2000,"PING %s", to));
}

int sendnick(char *nick)
{
  return (send_command(1000, "NICK %s", nick));
}

int sendjoin(char *channel, char *key)
{
  return (send_command(1000, "JOIN %s %s", channel, key));
}

int sendpart(char *channel)
{
  return (send_command(1000,"PART %s", channel));
}

int sendquit(char *reason)
{
  int tmpret;

  tmpret = send_to_server("QUIT :%s\n", reason);
  sleep(5);
  return (tmpret);
}

int sendmode(char *to, char *format, ...)
{
  char buf[MAXLEN];
  va_list args;

  memset(buf, '\0', MAXLEN);
  va_start(args, format);
  vsnprintf(buf, MAXLEN-1, format, args);
  va_end(args);
  return(send_command(1000,"MODE %s %s", to, buf));
}

int sendkick(char *channel, char *nick, char *reason)
{
  char buf[MAXLEN];

  if (!reason || !*reason)
    sprintf(buf, "Later %s", nick);
  else
    strcpy(buf, reason);
  return (send_command(1000, "KICK %s %s :%s", channel, nick, buf));
}

int send_ctcp_reply(char *to, char *format, ...)
{
  char buf[MAXLEN];
  va_list args;

  memset(buf, '\0', MAXLEN);
  va_start(args, format);
  vsnprintf(buf, MAXLEN-1, format, args);
  va_end(args);

  return (send_command(1000, "NOTICE %s :\001%s\001", to, buf));
}

int send_ctcp(char *to, char *format, ...)
{
  char buf[MAXLEN];
  va_list args;

  memset(buf, '\0', MAXLEN);
  va_start(args, format);
  vsnprintf(buf, MAXLEN, format, args);
  va_end(args);
  return(send_command(0,"PRIVMSG %s :\x01%s\x01", to, buf));
}

int sendison(char *nick)
{
  return (send_command(1000,"ISON %s", nick));
}

/* This is not used anymore */
int senduserhost(char *nick)
{
  return (send_command(0, "USERHOST %s", nick));
}

int send_to_user(char *sendto, char *format, ...)
{
  char entiremessage[WAYTOBIG];
  va_list args;

  memset(entiremessage, '\0', WAYTOBIG);
  va_start(args, format);
  vsnprintf(entiremessage, WAYTOBIG-1, format, args);
  va_end(args);

  if(strchr(entiremessage, '\n'))
    *strchr(entiremessage, '\n') = ' ';  /* Convert multiple lines to one for irc.. */
  if(!send_chat(sendto, entiremessage))  /* Try to send via dcc.. if no link, send it as NOTICE */
    return(sendnotice(getnick(sendto), "%s", entiremessage));
  else
    return (TRUE);
  return(TRUE);
}
