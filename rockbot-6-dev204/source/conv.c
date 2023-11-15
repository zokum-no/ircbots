
/*
 * CONV.C   USED FOR THE CONVERSATIONS
 */

#include <stdio.h>
#include <string.h>
#include "conv.h"
#include "config.h"
#include "misc.h"
#include "rockbot.h"
#include "channel.h"
#include "userlist.h"
#include "dcc.h"
#include "ftext.h"
#include "commands.h"
#include "parse.h"
#include "send.h"
#include "match.h"

char *getpokeword(char *sayword)
{
  int my_rand;

  my_rand = (rand() % 25);
  switch (my_rand)
  {
  case 1:
    return ("a Zamboni");
  case 2:
    return ("a Stipe");
  case 3:
    return ("a broken wine bottle");
  case 4:
    return ("an antalope");
  case 0:
    return ("a marshmallow");
  case 5:
    return ("a UFO");
  case 6:
    return ("a pineapple");
  case 7:
    return ("an insult");
  case 8:
    return ("a magic eightball");
  case 9:
    return ("an armchair");
  case 10:
    return ("a socket wrench");
  case 11:
    return ("a large trout");
  case 12:
    return ("a 60lb UNIX manual");
  case 13:
    return ("a coffee table");
  case 14:
    return ("a spork");
  case 15:
    return ("a Matzah ball");
  case 16:
    return ("an imaginary friend");
  case 17:
    return ("ease");
  case 18:
    return ("grace and agility");
  case 19:
    return ("wet buttery popcorn");
  case 20:
    return ("a 3lb burrito");
  case 21:
    return ("one of Mulder's weirdo scripts");
  case 22:
    return ("an AOL disk");
  case 23:
    return ("an MFM hard drive");
  case 24:
    return ("TOG (The old Geezer)");
  default:
    return ("a debug notice: [my_rand != 1-5]");
  }

}

void converse(char *from, char *to, char *nick, char *rest)
{
  char buf[MAXLEN];
  char resttmp[MAXLEN];
  char *tmp1;
  CHAN_list *ChanPtr;

  ChanPtr = search_chan(to);
  if (!ChanPtr)
    ChanPtr = Globals;
  if ((userlevel(from, ChanPtr) >= 50) && (strcasestr(rest, "who's the boss?")))
  {
    sendprivmsg(to, "You are %s Sir!", getnick(from));
  }

  sprintf(buf, "*How are you %s*", nick);
  if (!matches(buf, rest))
  {
    sendprivmsg(to, "All systems functional %s.", getnick(from));
  }
  srand(time(NULL));
  if ((strcasestr(rest, "i like ")))
    if ((rand() % 5) == 1)
    {
      char *liketemp;

      liketemp = strcasestr(rest, "like");
      if (liketemp)
      {
	liketemp = strchr(liketemp, ' ');
	if (liketemp)
	{
	  liketemp++;
	  if (liketemp && *liketemp)
	  {
	    sendprivmsg(to, "I enjoy %s too %s!", liketemp, getnick(from));
	  }
	}
      }
    }

  strcpy(resttmp, rest);
  tmp1 = strtok(resttmp, " ");
  if ((tmp1) && (!strcasecmp(tmp1, "poke")))
  {
    char *poke;

    poke = strcasestr(rest, "poke");
    if (poke)
    {
      poke = strchr(poke, ' ');
      if (poke)
      {
	poke++;
	if (poke && *poke)
	{
	  send_ctcp(to, "ACTION pokes %s with %s!", poke, getpokeword(NULL));
	}
      }
    }
  }
}
