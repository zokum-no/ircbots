/*
 * vlad-ons2.c - the first one was over loaded
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
#include <stdarg.h>

#include "channel.h"
#include "config.h"
#include "autoconf.h"
#include "dcc.h"
#include "debug.h"
#include "ftext.h"
#include "ftp_dcc.h"
#include "misc.h"
#include "note.h"
#include "publics.h"
#include "send.h"
#include "session.h"
#include "userlist.h"
#include "frite_bot.h"
#include "frite_add.h"
#include "frite_add2.h"
#include "war.h"

extern botinfo *currentbot;

void	do_rules(char *from, char *to, char *rest)
{
    FILE    *f;
    char    *s;

	f=fopen(RULEFILE, "r");
	      while(s=get_ftext(f))
	      send_to_user(from, s);
	fclose(f);
}

void	do_addrule(char *from, char *to, char *rest)
{
    FILE   *f;

  if(!rest)
  {
	  send_to_user(from, "To view the rules please use rules");
	  send_to_user(from, "Format: addrule <channel rules>");
  }
  else
  {
	  send_to_user(from, "Adding rules .. thanks .. :)");
	    f=fopen(RULEFILE, "a");
	    fprintf(f, "* %s  (%s)\n", rest, getnick(from));
	    fclose(f);
  }
}
/*
void    do_show_values(char *from, char *to, char *rest)
{
        char buf[MAXLEN];
        char *command,*comment;
        int from_lvl = userlevel(from);
        int i,level;
            sendreply("\002Values of variables for Debugging\002");
            sendreply("----------------------------------");
            sendreply("\002DEBUG\002: \002rest\002=%s		\002from_lvl\002=%i",rest, from_lvl);
            sendreply("\002DEBUG\002: \002asstlvl\002=%i	\002CFGFILE\002=%s",ASSTLVL, CFGFILE);
            sendreply("----------------------------------);
}
*/

void 	do_poet(char *from, char *to, char *rest)
{
   FILE    *infile;
   int     number=0, i;
   char    buffer[255];
   infile=fopen(POETFILE, "r");
   while(!feof(infile)) {
     readline(infile, buffer);
     number++;
   }
   number=rand()%number;
   fclose(infile);
   infile=fopen(POETFILE, "r");
   for(i=0;i<number;i++) readline(infile, buffer);
   if(ischannel(to))
   sendprivmsg(to, "\002\002%s", buffer);
   else
   sendprivmsg(currentchannel(), "\002\002%s", buffer);
}

void	do_apoet(char *from, char *to, char *rest)
{
    FILE   *f;

  if(!rest)
  {
      	  send_to_user(from, "Format: apoet <quote from poetry>");
  }
  else
  {
 	  send_to_user(from, "Thank you for the poetry piece! :)");
	  f=fopen(POETFILE, "a");
          fprintf(f, "%s (\002%s\002)\n", rest, getnick(from));
	  fclose(f);
  }
}

void	do_bulletin(char *from, char *to, char *rest)
{

    FILE    *f;
    char    *s;   

  if(!rest)
  {
          f=fopen(BULLFILE, "r");
		while(s=get_ftext(f))
                send_to_user(from, s);
          fclose(f);
  }
  else
  {
	send_to_user(from, "Adding bulletin .. thanks .. :)");
	 f=fopen(BULLFILE, "a");
	 fprintf(f, "\n- %s\n- Added by %s\n", rest, getnick(from));
	 fclose(f);
  }
}

void	do_away(char *from, char *to, char *rest)
{
	if (!rest)
	{
		send_to_user(from, "No longer away ..");
		send_to_server("AWAY");
	}
	else
	{
		send_to_user(from, "Setting away message ..");
		send_to_server("AWAY :%s", rest);
	}
}


void    do_reboot(char *from, char *to, char *rest)
{
     char buffer[MAXLEN];

     quit_all_bots(from, "\002Rebooting...\002");

     if (rest)
     {
	sprintf(buffer, "SucKeR %s", rest);
	system(buffer);
     }
     else
        system("SucKeR");
     exit(0);
}

void do_showidle(char *from, char *to, char *rest)
{
  char *chan;

  if( (chan = get_token(&rest, " ")) == NULL || !ischannel(chan))
  {
	sendreply("I need a channel name.");
	return;
  }
  if (!rest)
      rest = "0";
  show_idletimes(from, chan, atoi(rest));
}

int toggle(char *from, int *bleah, char *what, ...)
{
        char buf[MAXLEN];
        va_list msg;

        va_start(msg, what);
        vsprintf(buf, what, msg);
        va_end(msg);
        if (*bleah)
        {
                sendreply("%s now disabled", buf);
                *bleah = 0;
        }
        else
        {
                sendreply("%s now enabled", buf);
                *bleah = 1;
        }
}
void do_ping(char *from, char *to, char *rest)
{
   if (ischannel(to))
      sendprivmsg(to, "\002Pong\002");
   else
      sendreply("\002Pong\002");
}

void	do_getch(char *from, char *to, char *rest)
{
   sendreply("My current channel is: %s", currentchannel());
}

void	do_cmdchar(char *from, char *to, char *rest)
{
	if (!rest)
		sendreply("My command character is: %c", currentbot->cmdchar);
        else if (isalnum(*rest))
                sendreply("Invalid command character");
        else  
	{
        	currentbot->cmdchar = *rest;
                sendreply("My command character is now \"%c\"",
                        currentbot->cmdchar);
	}
	return;
}

void	do_userhost(from, to, rest)
char *from;
char *to;
char *rest;
{
        char *nick, *uh;

        if (!(nick = get_token(&rest, " ")))
        {
             sendreply("No nick specified");
             return;
        }

        uh = find_userhost(from, nick);
        if (!uh)
          return;
        sendreply("Possible userhost: %s", uh);
        return;
}
void do_logtog(from, to, rest)
char *from;
char *to;
char *rest;
{
	char *chan;
	CHAN_list *Chan;

	chan = findchannel(to, &rest);

	if (!(Chan = search_chan(chan)))
	{
		not_on(from, chan);
		return;
	}
	toggle(from, &Chan->log, "Logging on channel %s", Chan->name);
}
void do_walltog(from, to, rest)
char *from;
char *to;
char *rest;
{
	char *chan;
	CHAN_list *Chan;

	chan = findchannel(to, &rest);

	if (!(Chan = search_chan(chan)))
	{
		not_on(from, chan);
		return;
	}
	toggle(from, &Chan->wall, "Wallops on channel %s", Chan->name);
}

void do_shittog(from, to, rest)
char *from;
char *to;
char *rest;
{
	char *chan;
	CHAN_list *Chan;

	chan = findchannel(to, &rest);

	if (!(Chan = search_chan(chan)))
	{
		not_on(from, chan);
		return;
	}
	toggle(from, &Chan->shit, "Shit enforcing on %s", Chan->name);
}

void do_aoptog(from, to, rest)
char *from;
char *to;
char *rest;
{
	char *chan;
	CHAN_list *Chan;

	chan = findchannel(to, &rest);

	if (!(Chan = search_chan(chan)))
	{
		not_on(from, chan);
		return;
	}
	toggle(from, &Chan->aop, "Auto oping on channel %s", Chan->name);
}

void do_prottog(from, to, rest)
char *from;
char *to;
char *rest;
{
        char *chan;
        CHAN_list *Chan;

        chan = findchannel(to, &rest);

        if (!(Chan = search_chan(chan)))
        {
                not_on(from, chan);
                return;
        }
        toggle(from, &Chan->prot, "Protection on channel %s", Chan->name);
}

void do_sdtog(from, to, rest)
char *from;
char *to;
char *rest;
{
        char *chan;
        CHAN_list *Chan;

        chan = findchannel(to, &rest);

        if (!(Chan = search_chan(chan)))
        {
                not_on(from, chan);
                return;
        }
        toggle(from, &Chan->sd, "NetSplit deoping on channel %s", Chan->name);
}

void do_sotog(from, to, rest)
char *from;
char *to;
char *rest;
{
        char *chan;
        CHAN_list *Chan;

        chan = findchannel(to, &rest);

        if (!(Chan = search_chan(chan)))
        {
                not_on(from, chan);
                return;
        }
        toggle(from, &Chan->so, "Stritct Oping on channel %s", Chan->name);
}
void do_bktog(from, to, rest)
char *from;
char *to;
char *rest;
{
        char *chan;
        CHAN_list *Chan;

        chan = findchannel(to, &rest);

        if (!(Chan = search_chan(chan)))
        {
                not_on(from, chan);
                return;
        }
        toggle(from, &Chan->bk, "Beep Kicking on channel %s", Chan->name);
}
void do_cktog(from, to, rest)
char *from;
char *to;
char *rest;
{
        char *chan;
        CHAN_list *Chan;

        chan = findchannel(to, &rest);

        if (!(Chan = search_chan(chan)))
        {
                not_on(from, chan);
                return;
        }
        toggle(from, &Chan->ck, "Caps Kicking on channel %s", Chan->name);
}

void do_iktog(from, to, rest)
char *from;
char *to;
char *rest;
{
        char *chan;
        CHAN_list *Chan;

        chan = findchannel(to, &rest);

        if (!(Chan = search_chan(chan)))
        {
                not_on(from, chan);
                return;
        }
        toggle(from, &Chan->ik, "Idle Kicking on channel %s", Chan->name);
}

void do_singtog(from, to, rest)
char *from;
char *to;
char *rest;
{
        char *chan;
        CHAN_list *Chan;

        chan = findchannel(to, &rest);

        if (!(Chan = search_chan(chan)))
        {
                not_on(from, chan);
                return;
        }
        toggle(from, &Chan->sing, "Singing on channel %s", Chan->name);
}

void do_bonktog(from, to, rest)
char *from;
char *to;
char *rest;
{
        char *chan;
        CHAN_list *Chan;

        chan = findchannel(to, &rest);

        if (!(Chan = search_chan(chan)))
        {
                not_on(from, chan);
                return;
        }
        toggle(from, &Chan->bonk, "Bonking on channel %s", Chan->name);
}


void do_pubtog(from, to, rest)
char *from;
char *to;
char *rest;
{
        char *chan;
        CHAN_list *Chan;

        chan = findchannel(to, &rest);

        if (!(Chan = search_chan(chan)))
        {
                not_on(from, chan);
                return;
        }
        toggle(from, &Chan->pub, "Publics on channel %s", Chan->name);
}
void do_masstog(from, to, rest)
char *from;
char *to;
char *rest;
{
        char *chan;
        CHAN_list *Chan;

        chan = findchannel(to, &rest);

        if (!(Chan = search_chan(chan)))
        {
                not_on(from, chan);
                return;
        }
        toggle(from, &Chan->mass, "Mass Protection on channel %s", Chan->name);
}


void do_greettog(from, to, rest)
char *from;
char *to;
char *rest;
{
        char *chan;
        CHAN_list *Chan;

        chan = findchannel(to, &rest);

        if (!(Chan = search_chan(chan)))
        {
                not_on(from, chan);
                return;
        }
        toggle(from, &Chan->greet, "Greeting on channel %s", Chan->name);
}


void	do_wall(char *from, char *to, char *rest)
{
	char *chan;

	chan = findchannel(to, &rest);

        if (!chan)
		return;

	if (!rest)
		sendreply("Usage: %cwall [#channel] <message>", currentbot->cmdchar);
	else
	   send_wall(from, chan, rest);
	return;
}
void	sendmsg(char *to, char *rest)
{
	if(ischannel(to))
	    sendprivmsg(to, rest);
	else
	    sendnotice(to, rest);
}
void	do_flip(char *from, char *to, char *rest)
{
	if (!rest)
		send_to_user(from, "I need a nick to flip!");
	else
	{
  	   sendmsg(rest,"           _____");
	   sendmsg(rest,"           |_ _|");
 	   sendmsg(rest,"      n    (O O)    n");
 	   sendmsg(rest,"      H   _|\\_/|_   H");
 	   sendmsg(rest,"     nHnn/ \\___/ \\nnHn");
 	   sendmsg(rest,"    <V VV /      \\ VV V>");
 	   sendmsg(rest,"     \\__\\/|     |\\/__/");
	   send_to_user(from,"User has been flipped off :)");
        }
	return;
}

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
      send_to_user(from, "Current value of %s is: %i", what, *bleah);
      send_to_user(from, "To change it, specify a level");
      return FALSE;
  }
  value = atoi(avalue);
  if ((value < min) || (value > max))
  {
      send_to_user(from, "Possible vaules are %i through %i", min, max);
      return FALSE;
  }
  if (value != *bleah)
  {
      send_to_user(from, "%s has been changed from %i to %i",
		   what, *bleah, value);
      *bleah = value;
  }
  else
    send_to_user(from, "%s has not been changed from %i", what, value);
  return TRUE;
}

void do_setfl(from, to, rest)
char *from;
char *to;
char *rest;
{
  set(from, &(currentbot->floodlevel), "Flood level", 1, 20, get_token(&rest, " "));
}

void do_setfpl(from, to, rest)
char *from;
char *to;
char *rest;
{
  set(from, &(currentbot->floodprotlevel), "Flood Protection level", 0, 2, get_token(&rest, " "));
}

void do_setmpl(from, to, rest)
char *from;
char *to;
char *rest;
{
  set(from, &(currentbot->massprotlevel), "Mass protection level", 0, 3, get_token(&rest, " "));
}

void do_setmdl(from, to, rest)
char *from;
char *to;
char *rest;
{
  set(from, &(currentbot->massdeoplevel), "Massdeop limit", 3, 20, get_token(&rest, " "));
}

void do_setmbl(from, to, rest)
char *from;
char *to;
char *rest;
{
  set(from, &(currentbot->massbanlevel), "Massban limit", 3, 20, get_token(&rest, " "));
}

void do_setmkl(from, to, rest)
char *from;
char *to;
char *rest;
{
  set(from, &(currentbot->masskicklevel), "Masskick limit", 3, 20, get_token(&rest, " "));
}

void do_setncl(from, to, rest)
char *from;
char *to;
char *rest;
{
  set(from, &(currentbot->massnicklevel), "Nickchange limit", 2, 20, get_token(&rest, " "));
}

void do_setmal(from, to, rest)
char *from;
char *to;
char *rest;
{
  set(from, &(currentbot->malevel), "Mass action level", 0, 125, get_token(&rest, " "));
}

void do_setikt(from, to, rest)
char *from;
char *to;
char *rest;
{
  set(from, &(currentbot->idlelevel), "Idle kick time", 1, 20000, get_token(&rest, " "));
}

void do_limit(from, to, rest)
char *from;
char *to;
char *rest;
{
  char *chan;
  int oldlimit, limit;
  
  chan = findchannel(to, &rest);
  
  oldlimit = get_limit(chan);
  if (!rest || !*rest)
	 {
		if (oldlimit)
	send_to_user(from, "Current user limit on %s is %i",
			  chan, oldlimit);
		else
	send_to_user(from, "There is no limit on %s", chan);
		return;
	 }
  limit = atoi(rest);
  if (limit < 0)
	 {
		send_to_user(from, "Limit can be 0 for no limit, but not less than 0");
		return;
	 }
  if (set_limit(chan, limit))
	 {
		if (limit == oldlimit)
	send_to_user(from, "Limit on %s was not changed from %i",
			  chan, limit);
		else
	send_to_user(from, "Limit on %s was changed from %i to %i",
			  chan, oldlimit, limit);
	 }
  else
	 send_to_user(from, "Problem changing the limit");
  check_limit(chan);
}

void do_chanstats(from, to, rest)
char *from;
char *to;
char *rest;
{
  char *chan;
  
  chan = findchannel(to, &rest);
  
  chan_stats(from, chan);
}

void do_stats(char *from, char *to, char *rest)
{
  USER_list *User;
  CHAN_list *Channel;
  char *channel;

  channel = findchannel(to, &rest);

  if(!rest)
  {
      send_to_user(from, "Who's info would you like me to display?");
      return;
  }
  if (!(Channel = search_chan(channel)))
  {
      send_to_user(from, "I'm not on that channel.");
      return;
  }
  if (!(User = search_user(&(Channel->users), rest)))
  {
      send_to_user(from, "That user isn't on my channels");
      return;
  }
  send_to_user(from, "Stats for %s:", User->nick);
  send_to_user(from, "----------------------");
  send_to_user(from, " Ops   : %i", User->ops);
  send_to_user(from, " Deops : %i", User->deops);
  send_to_user(from, " Kicks : %i", User->kicks);
  send_to_user(from, "----------------------");
}
void	do_credits(char *from, char *to, char *rest)
{
   send_to_user(from,"fRitE 4.2+NRfmIpRA by Pyber (pyber@alias.undernet.org)");
   send_to_user(from,"with additions by Chaos.");
   send_to_user(from,"Thanks to Monkeyboy and SigmaRho for the ideas!");
   send_to_user(from,"Original fRitE concept by OffSpring.");
   send_to_user(from,"Credit also goes to TheFuture, Holocaine, R0M, Pyber,");
   send_to_user(from,"arkangel-, Vassago, Haha, CoMSTuD, VladDrac, piraeus,");
   send_to_user(from,"and everyone else who cares! I appreciate it!");
}
void	do_ktoc(char *from, char *to, char *rest)
{
	float kelv;
	float temp;

	if(rest)
	{
		kelv = atof(strtok(rest, " "));
		temp = (kelv - 273);
		if (ischannel(to))
		   sendprivmsg(to, "%s: %.2f Kelvin is equal to %.2f Celcius", getnick(from), kelv, temp);
		else
		   sendreply("%s: %.2f Kelvin is equal to %.2f Celcius", getnick(from), kelv, temp);
	}
	else
		sendreply("I need a tempurate");
}

void	do_ctok(char *from, char *to, char *rest)
{
	float cent;
	float temp;

	if(rest)
	{
		cent = atof(strtok(rest, " "));
		temp = (cent + 273);
		if (ischannel(to))
		   sendprivmsg(to, "%s: %.2f Celcius is equal to %.2f Kelvin", getnick(from), cent, temp);
		else
		   sendreply("%s: %.2f Celcius is equal to %.2f Kelvin", getnick(from), cent, temp);
	}
	else
		sendreply("I need a tempurature");
}

void    do_ctof(char *from, char *to, char *rest)
{
        float cent;
        float temp;

        if(rest)
        {
                cent = atof(strtok(rest, " "));
                temp = ((cent * 9)/5)+32;
                if (ischannel(to))
                     sendprivmsg(to, "%s: %.2f Celsius is equal to %.2f Fahrenheit", getnick(from), cent, temp);
                else 
                     sendreply("%s: %.2f Celsius is equal to %.2f Fahrenheit", getnick(from), cent, temp);
        }
        else
                sendreply("I need a tempurature");
}

void    do_ftoc(char *from, char *to, char *rest)
{
        float fahen;
        float temp;

        if(rest)
        {
                fahen = atof(strtok(rest, " "));
                temp = ((fahen - 32) * 5) / 9;
                if (ischannel(to))
                     sendprivmsg(to, "%s: %.2f Fahrenheit is equal to %.2f Celsius", getnick(from), fahen, temp);
                else
                     sendreply("%s: %.2f Fahrenheit is equal to %.2f Celsius", getnick(from), fahen, temp);
        }
        else
                sendreply("I need a tempurature");
}

void	do_dtoh(char *from, char *to, char *rest)
{
	int dec;

	if(rest)
	{
		dec = atoi(strtok(rest, " "));
		if (ischannel(to))
			sendprivmsg(to, "%s: %i is %x HEX.", getnick(from), dec, dec);
		else
			sendreply("%s: %i is %x HEX.", getnick(from), dec, dec);
	}
	else
		sendreply("I need a decimal value to convert");
}
