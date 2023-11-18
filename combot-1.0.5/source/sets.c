#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#ifndef MSDOS
	#include <strings.h>
#endif
#include <time.h>
#include <signal.h>
#include "config.h"
#include "channel.h"
#include "userlist.h"
#include "dcc.h"
#include "ftext.h"
#include "misc.h"
#include "commands.h"

extern char lastcmd[1024];
extern char current_nick[MAXLEN];
extern long uptime;
extern int kslevel, idlelimit, sbtype, malevel, mplevel;
extern int maxsameuser;
extern int masskicklimit, massdeoplimit, massbanlimit, floodlimit;
extern int minoplevel, minbanlevel, mindeoplevel, minunbanlevel;
extern int holdingnick, holdnick;
extern char nhmsg[255];

void do_setfl(char *from, char *to, char *rest)
{
  int level;
  if (!*rest)
	 {
		send_to_user( from, "\x2The current flood level is %i\x2", floodlimit);
		send_to_user( from, "\x2To change it, specify a level\x2");
		return;
	 }
  level = atoi(rest);
  if (level <= 0)
	 {
		send_to_user( from, "\x2The level has to be greater than 0\x2");
		return;
	 }
  if (level == floodlimit)
	 send_to_user( from, "%cFlood level not changed...\x2", '\x2');
  else
	 {
		send_to_user( from, "%cFlood level changed from %i to %i\x2",
			'\x2', floodlimit, level);
		floodlimit = level;
	 }
  return;
}

void do_setikl(char *from, char *to, char *rest)
{
	int level;
	if (!*rest)
	{
		send_to_user( from, "\x2The current idle-kick limit is %i\x2", idlelimit);
		send_to_user( from, "\x2To change it, specify how many seconds\x2");
		return;
	}
	level = atoi(rest);
	if (level <= 0)
	{
		send_to_user( from, "\x2Level must be greater than 0\x2");
		return;
	}
	if (level == idlelimit)
		send_to_user( from, "\x2Idle-kick limit not changed...\x2");
	else
	{
		send_to_user( from, "\x2Idle-kick limit changed from %i to %i seconds\x2",
			idlelimit, level);
		idlelimit = level;
	}
	return;
}

void do_setmol(char *from, char *to, char *rest)
{
	int level;
	if (!*rest)
	{
		send_to_user( from, "\x2The current min op level is %i\x2", minoplevel);
		send_to_user( from, "\x2To change it, specify a level\x2");
		return;
	}
	level = atoi(rest);
	if ((level < 0) || (level > OWNERLEVEL))
	{
		send_to_user( from, "\x2Valid levels are 0 through %i\x2",
			OWNERLEVEL);
		return;
	}
	if (level == minoplevel)
		send_to_user( from, "\x2Min op level not changed...\x2");
	else
	{
		send_to_user( from, "\x2Min op level changed from %i to %i\x2",
			minoplevel, level);
		minoplevel = level;
	}
	return;
}

void do_setmdol(char *from, char *to, char *rest)
{
	int level;
	if (!*rest)
	{
		send_to_user( from, "\x2The current min deop level is %i\x2", mindeoplevel);
		send_to_user( from, "\x2To change it, specify a level\x2");
		return;
	}
	level = atoi(rest);
	if ((level < 0) || (level > OWNERLEVEL))
	{
		send_to_user( from, "\x2Valid levels are 0 through %i\x2",
			OWNERLEVEL);
		return;
	}
	if (level == mindeoplevel)
		send_to_user( from, "\x2Min deop level not changed...\x2");
	else
	{
		send_to_user( from, "\x2Min deop level changed from %i to %i\x2",
			mindeoplevel, level);
		mindeoplevel = level;
	}
	return;
}

void do_setmbl(char *from, char *to, char *rest)
{
	int level;
	if (!*rest)
	{
		send_to_user( from, "\x2The current min ban level is %i\x2", minbanlevel);
		send_to_user( from, "\x2To change it, specify a level\x2");
		return;
	}
	level = atoi(rest);
	if ((level < 0) || (level > OWNERLEVEL))
	{
		send_to_user( from, "\x2Valid levels are 0 through %i\x2",
			OWNERLEVEL);
		return;
	}
	if (level == minbanlevel)
		send_to_user( from, "\x2Min ban level not changed...\x2");
	else
	{
		send_to_user( from, "\x2Min ban level changed from %i to %i\x2",
			minbanlevel, level);
		minbanlevel = level;
	}
	return;
}

void do_setmubl(char *from, char *to, char *rest)
{
	int level;
	if (!*rest)
	{
		send_to_user( from, "\x2The current min unban level is %i\x2", minunbanlevel);
		send_to_user( from, "\x2To change it, specify a level\x2");
		return;
	}
	level = atoi(rest);
	if ((level < 0) || (level > OWNERLEVEL))
	{
		send_to_user( from, "\x2Valid levels are 0 through %i\x2",
			OWNERLEVEL);
		return;
	}
	if (level == minunbanlevel)
		send_to_user( from, "\x2Min unban level not changed...\x2");
	else
	{
		send_to_user( from, "\x2Min unban level changed from %i to %i\x2",
			minunbanlevel, level);
		minunbanlevel = level;
	}
	return;
}

void do_setmass(char *from, char *to, char *rest)
{
	int deopl, banl, kickl;
	char *deop, *ban, *kick;

	deop = strtok(rest, " ");
	ban = strtok(NULL, " ");
	kick = strtok(NULL, " ");
	if (!*rest || !deop || !ban || !kick)
	{
		send_to_user( from, "\x2The current mass limits are:\x2");
		send_to_user( from, "%cDeop = %i, Ban = %i, Kick = %i\x2",
			'\x2', massdeoplimit,massbanlimit, masskicklimit);
		send_to_user( from, "\x2To change do: setmass deoplimit banlimit kicklimit\x2");
		return;
	}

	deopl = atoi(deop);
	banl = atoi(ban);
	kickl = atoi(kick);
	if ((deopl <= 0) || (banl <= 0) || (kickl <= 0))
	{
		send_to_user(from, "\x2Levels must be greater than 0\x2");
		return;
	}
	massdeoplimit = deopl;
	massbanlimit = banl;
	masskicklimit = kickl;
	send_to_user(from, "\x2Mass limits are now as follows:\x2");
	send_to_user( from, "%cDeop = %i, Ban = %i, Kick = %i\x2",
		'\x2', massdeoplimit,massbanlimit, masskicklimit);
	return;
}

void do_setksl(char *from, char *to, char *rest)
{
	int level;
	if (!*rest)
	{
		send_to_user( from, "\x2The current kicksay level is %i\x2", kslevel);
		send_to_user( from, "\x2To change it, specify a level\x2");
		return;
	}
	level = atoi(rest);
	if ((level < 0) || (level > OWNERLEVEL))
	{
		send_to_user( from, "\x2Valid levels are 0 through %i\x2",
			OWNERLEVEL);
		return;
	}
	if (level == kslevel)
		send_to_user( from, "\x2Kicksay level not changed...\x2");
	else
	{
		send_to_user( from, "\x2Kicksay level changed from %i to %i\x2",
			kslevel, level);
		kslevel = level;
	}
	return;
}

void do_setsbt(char *from, char *to, char *rest)
{
  char *type;
  int num;
  
  type = strtok(rest, " ");
  if (!*rest || !type)
    {
      send_to_user(from, "\x2The current screwban type is %i\x2", sbtype);
      send_to_user(from, "\x2To change it, specify a type\x2");
      return;
    }
  num = atoi(type);
  if ((num < 0) || (num > 2))
    {
      send_to_user(from, "\x2The current choices are 0 (off) through 2\x2");
      return;
    }
  if (sbtype == num)
    send_to_user(from, "\x2Screwban type not changed\x2");
  else
    send_to_user(from, "\x2Screwban type changed from %i to %i\x2",
		 sbtype, num);
  sbtype = num;
  return;
}

void do_setmal(char *from, char *to, char *rest)
{
  int level;
  if (!*rest)
    {
      send_to_user(from, "\x2The current min mass action level is %i\x2",
		   malevel);
      send_to_user(from, "\x2To change it, specify a level\x2");
      return;
    }
  level = atoi(rest);
  if ((level < 0) || (level >= OWNERLEVEL))
    {
      send_to_user(from, "\x2The level must be between 0 and %i\x2",
		   OWNERLEVEL-1);
      return;
    }
  if (level == malevel)
    send_to_user(from, "\x2Min mass action level not changed\x2");
  else
    send_to_user(from, "\x2Min mass action level changed from %i to %i\x2",
		 malevel, level);
  malevel = level;
  return;
}

void do_setmpl(char *from, char *to, char *rest)
{
  int level;
  if (!*rest)
    {
      send_to_user(from, "\x2The current mass protection level is %i\x2",
                   mplevel);
      send_to_user(from, "\x2To change it, specify a level\x2");
      return;
    }
  level = atoi(rest);
  if ((level < 1) || (level > 3))
    {
      send_to_user(from, "\x2The level must be between 1 and 3\x2");
      return;
    }
  if (level == mplevel)
    send_to_user(from, "\x2Mass protection level not changed\x2");
  else
    send_to_user(from, "\x2Mass protection level changed from %i to %i\x2",
                 mplevel, level);
  return;
}

void do_setmul(char *from, char *to, char *rest)
{
  int num;
  if (!*rest)
    {
      send_to_user(from, "\x2The current max number of same user@* allowed is: %i\x2", maxsameuser);
      send_to_user(from, "\x2To change it, specify the max number\x2");
      return;
    }
  num = atoi(rest);
  if (num < 1)
    {
      send_to_user(from, "\x2Sorry, the number must be greater than 0\x2");
      return;
    }
  if (num==maxsameuser)
    send_to_user(from, "\x2The level was not changed\x2");
  else
    send_to_user(from, "\x2The level has been changed from %i to %i\x2",
		maxsameuser, num);
  maxsameuser = num;
  return;
}

