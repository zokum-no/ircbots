#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config.h"
#include "hofbot.h"

extern botinfo *current;

int set(from, bleah, what, min, max, avalue)
char *from;
int *bleah;
char *what;
int min;
int max;
char *avalue;
{
  int value;
  if (!avalue || !*avalue)
    {
      send_to_user(from, "\002Current value of %s is: %i\002", what, *bleah);
      send_to_user(from, "\002To change it, specify a level\002");
      return FALSE;
    }
  value = atoi(avalue);
  if ((value < min) || (value > max))
    {
      send_to_user(from, "\002Possible vaules are %i through %i\002", min, max);
      return FALSE;
    }
  if (value != *bleah)
    {
      send_to_user(from, "\002%s has been changed from %i to %i\002",
		   what, *bleah, value);
      *bleah = value;
    }
  else
    send_to_user(from, "\002%s has not been changed from %i\002", what, value);
  return TRUE;
}

void do_setnick(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  set(from, &(current->nicklevel), "Nick change", 1, 200, get_token(&rest, " "));
}

void do_setopic(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  set(from, &(current->rantopiclevel), "Random topic change", 1, 200, get_token(&rest, " "));
}

void do_setenftopic(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  set(from, &(current->enftopiclevel), "Enforce topic change", 1, 200, get_token(&rest, " "));
}

void do_setranquote(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  set(from, &(current->ranquotelevel), "Random quote change", 1, 200, get_token(&rest, " "));
}

void do_setfl(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  set(from, &(current->floodlevel), "Flood level", 1, 20, get_token(&rest, " "));
}

void do_setfpl(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  set(from, &(current->floodprotlevel), "Flood Protection level", 0, 2, get_token(&rest, " "));
}

void do_setmpl(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  set(from, &(current->massprotlevel), "Mass protection level", 0, 3, get_token(&rest, " "));
}

void do_setmdl(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  set(from, &(current->massdeoplevel), "Massdeop limit", 3, 20, get_token(&rest, " "));
}

void do_setmbl(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  set(from, &(current->massbanlevel), "Massban limit", 3, 20, get_token(&rest,
 " "));
}

void do_setmkl(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  set(from, &(current->masskicklevel), "Masskick limit", 3, 20, get_token(&rest,
 " "));
}

void do_setncl(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  set(from, &(current->massnicklevel), "Nickchange limit", 2, 20, get_token(&rest,
 " "));
}

void do_setmal(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  set(from, &(current->malevel), "Mass action level", 0, 99, get_token(&rest,
 " "));
}

void do_setikt(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  set(from, &(current->idlelevel), "Idle-kick time", 120, 20000, get_token(&rest, " "));
}
  
void do_setbkl(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  set(from, &(current->beepkicklevel), "Beep-kick level", 1, 20000, get_token(&rest,
 " "));
}

void do_setckl(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  set(from, &(current->capskicklevel), "Caps-kick level", 1, 20000, get_token(&rest,
 " "));
}



