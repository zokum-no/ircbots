/*=====[ ons.c ]=============================================================
  This would be the only file for end users to confiure..  Has all the 
  on_* functions that the parser recognizes.

============================================================================*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "bot.h"

static char buf[MAXLEN];
extern int writeln();

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
char *strcasestr (s1, s2)
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
	return (strstr (n1, n2));
}

/*=====[ on_join ]===========================================================
  "who" has joined channel "channel" - do whatever
===========================================================================*/
void on_join(who, channel)
  char *who;
  char *channel;
{
#ifdef DBUG
  printf("On_join: who = %s, channel = %s\n", who, channel);
#endif

  sprintf(buf, "PRIVMSG %s Hey %s!  Welcome to %s.\n", channel, who, channel);
  writeln(buf);

#ifdef DBUG
  printf("Exiting on_join\n");
#endif
}

/*=====[ on_msg ]============================================================
  A message was sent from "from" to "to", the text is in "msg"
===========================================================================*/
void on_msg(from, to, msg)
  char *from;
  char *to;
  char *msg;
{
  char *replyto;
  char *you="you";
#ifdef DBUG
  printf("On_msg: from = %s, to = %s, msg = %s\n", from, to, msg);
#endif

  /* no authorization yet.*/
  if( !strcmp(from, MASTERNICK) && strstr(msg, "bye bye bot"))
    writeln("QUIT\n");
  else {
    if(is_channel(to)) replyto=to;
    else {replyto=from; from=you; }
    sprintf(buf, "PRIVMSG %s Wow, %s said \"%s\"\n", replyto, from, msg);
    writeln(buf);
  }

#ifdef DBUG
  printf("Exiting on_msg\n");
#endif
}
