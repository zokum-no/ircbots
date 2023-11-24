/*
 * greeting.c - another funny function that needs its own file
 * (c) 1995 OffSpring (cracker@cati.CSUFresno.EDU)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>

#include "channel.h"
#include "config.h"
#include "autoconf.h"
#include "dcc.h"
#include "debug.h"
#include "ftext.h"
#include "ftp_dcc.h"
#include "misc.h"
#include "note.h"
#include "session.h"
#include "userlist.h"
#include "frite_bot.h"
#include "frite_add.h"
#include "greeting.h"

#define NO_GREETINGS 10

extern botinfo *currentbot;
struct {
  char *pre, *post;
  int action;
} greeting_list[] =

/*  Before nick           After nick          Action flag */
{
  { "looks up from his work long enough to notice ",   " walk in...",	1 },
  { "*warmcuddlesfuzzywuzzyticklehugs* ",             " .. tee hee ..", 1 },
  { "gives ",             " a SucKeR",          1 },
  { "",                   "!",                     0 },
  { "runs up and greets ",            " enthusiastically.",    1 },
  { "hugs ",              ".",                     1 },
  { "*flyingtackletothegroundbeargropingticklecuddlehugs*, ",   "!",		           0 },
  { "*warmfuzyhugs* ",	  " ..",		   1 },
  { "*ticklehugs* ",	  " ..",		   1 },
  { "*flyingtacklehugs* ", " ..",		   1 },
};

void do_greeting(char *nickname, char *channel)
{
  int i;

  i=(int) (rand()%NO_GREETINGS);
  if(!STRCASEEQUAL(currentbot->nick, nickname)) {
    if(greeting_list[i].action)
      send_ctcp(channel, "ACTION %s%s%s", greeting_list[i].pre, nickname,
         greeting_list[i].post);
    else
      sendprivmsg(channel, "%s%s%s", greeting_list[i].pre, nickname,
         greeting_list[i].post);
  }
}

