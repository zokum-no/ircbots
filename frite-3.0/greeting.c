#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>

#include "channel.h"
#include "config.h"
#include "dcc.h"
#include "debug.h"
#include "ftext.h"
#include "ftp_dcc.h"
#include "misc.h"
#include "note.h"
#include "session.h"
#include "userlist.h"
#include "vladbot.h"
#include "vlad-ons.h"
#include "greeting.h"

#define NO_GREETINGS 10

extern botinfo *currentbot;

int greetingflag=1;
struct {
  char *pre, *post;
  int action;
} greeting_list[] =

/*  Before nick           After nick          Action flag */
{
  { "OH my GAWD it's ",   "!!!",                   0 },
  { "pokes ",             " .. tee hee ..",        1 },
  { "gives ",             " a lollipop.",          1 },
  { "",                   "!",                     0 },
  { "greets ",            " enthusiastically.",    1 },
  { "hugs ",              ".",                     1 },
  { "Glad your back, ",   "!",		           0 },
  { "*warmfuzyhugs* ",	  " ..",		   1 },
  { "*ticklehugs* ",	  " ..",		   1 },
  { "*flyingtacklehugs* ", " ..",		   1 },
};

void do_greeting(char *nickname, char *channel)
{
#ifdef GREET
  int i;

  i=(int) (random()%NO_GREETINGS);
  if((strcmp(currentbot->nick, nickname)!=0)&&(greetingflag)) {
    if(greeting_list[i].action) send_ctcp(channel, "ACTION %s%s%s", greeting_list[i].pre, nickname, greeting_list[i].post);
    else sendprivmsg(channel, "%s%s%s", greeting_list[i].pre, nickname, greeting_list[i].post);
  }
#endif
}

void do_greet_off(char *from, char *to, char *rest)
{
  greetingflag=0;
  send_to_user(from, "Greetings off.");
}

void do_greet_on(char *from, char *to, char *rest)
{
  greetingflag=1;
  send_to_user(from, "Greetings on.");
}
