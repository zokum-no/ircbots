/*
 * function.c - misc. functions that were needed
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
#include <ctype.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>
 
#include "config.h"
#include "autoconf.h"
#include "channel.h"
#include "userlist.h"
#include "dcc.h"
#include "ftext.h"
#include "misc.h"
#include "frite_add.h"
#include "frite_bot.h"
int fullspec = 0;
extern ListStruct *SpyMsgList;
extern ListStruct *StealList;
extern botinfo *currentbot;
char usernick[MAXLEN];
char *left(char *string, int i)
{
  static char temp[255];  
  strcpy(temp, string);
  temp[i] = '\0';  
  return temp;
}
char *stripl(string, chars)
char *string;
char *chars;
{
	if (!string || !chars)
	 return "";
	for (;*string && strchr(chars, *string); string++);
	return string;
}
char *gethost(nick_userhost)
char *nick_userhost;
{
	char *temp, *ptr;
	strcpy(usernick, nick_userhost);
	temp = usernick;
	ptr = get_token(&temp, "@");
	if (!temp || !*temp)
		return "";
	return (cluster(temp));
}
 
 
int my_stricmp(str1, str2)
char *str1;
char *str2;
{
	if (!str1 && !str2)
		return 0;
	else if (!str1 || !str2)
		return 1;
	while (*str1 && *str2 && (toupper(*str1) == toupper(*str2)))
	{
		str1++;
		str2++;
	}
	if (*str1 || *str2)
		return 1;
	return 0;
}
 
int     matches(ma, na)
char *ma, *na;
{   
        register unsigned char m, *mask = (unsigned char *)ma;
        register unsigned char c, *name = (unsigned char *)na;
 
        for (;; mask++, name++)
            {   
                m = tolower(*mask);
                c = tolower(*name);
                if (c == '\0')
                        break;
                if ((m != '?' && m != c) || c == '*')
                        break;
            }        
        if (m == '*')
            {   
                for ( ; *mask == '*'; mask++);
                if (*mask == '\0')
                        return(0);
                for (; *name && matches((char *)mask, (char *)name); name++);
                return(*name ? 0 : 1);
            }        
        else
                return ((m == '\0' && c == '\0') ? 0 : 1);
}
 
void showspymsg(char *person, char *msg)
{ 
        ListStruct *Temp; 
 
        for (Temp = SpyMsgList;Temp;Temp=Temp->next)
                send_to_user(Temp->name, "\002*%s*\002 %s", person, msg);
        return;
}
time_t getthetime(void)
{
        time_t t;
 
        t = time( ( time_t *) NULL);
        return t;
}
 
int percent_caps(char *string)
{
    int len, tot;
 
 
    tot = 0;
    if (!string || !*string)
       return 0;
    len = strlen(string);
    while (*string)
    {
      if (isalpha(*string) && isupper(*string))
         tot++;
      string++;
    }
    return (100*tot)/len;
}
 
 
 
char *find_person(char *from, char *nick)
{
  static char temp[MAXLEN];
  char temp2[MAXLEN];
 
  if (strchr(nick, '@') || strchr(nick, '!'))
  {
      if (!strchr(nick, '!'))
      {
          strcpy(temp, "*!");
          strcat(temp, nick);
      }
      else if (!strchr(nick, '@'))
      {
          strcpy(temp, nick);
          strcat(temp, "@*");
      }
      else
        strcpy(temp, nick);
      fullspec = 1;
  }
  else
  {
      strcpy(temp2, getuserhost(nick));
      if (!temp2)
      {
          send_to_user(from, "No information for %s", nick);
          return "";
      }
      strcpy(temp, nick);
      strcat(temp, "!");
      strcat(temp, temp2);
      fullspec = 0;
  }
  return temp;
}
 
 
char *format_uh(char *userhost, int type)
{
	char *n, *u, *h;
	static char buffer[MAXLEN];
 
	if (strchr(userhost, '*'))
		return userhost;
	n = get_token(&userhost, "!");
	u = get_token(&userhost, "@");
	h = userhost;
	u = stripl(u, "~#");
	if (!u || !*u)
	  return NULL;
	if (!h || !*h)
	  return NULL;
	if ((type == 0) || (type == 1))
		sprintf(buffer, "*!*%s@%s", u, cluster(h));
	else
		sprintf(buffer, "*!*@%s", cluster(h));
	return buffer;
}
 
char *nick2uh(from, userhost, type)
char *from;
char *userhost;
int type;
{
	static char buffer[WAYTOBIG];
	char *uh;
 
	if (!userhost || !*userhost)
		return NULL;
	uh = buffer;
	if (strchr(userhost, '!') && strchr(userhost, '@'))
		return userhost;
 
	strcpy(uh, userhost);
	if (!strchr(userhost, '!') && !strchr(userhost, '@'))
	{
		char *temp;
		temp = find_userhost(from, userhost);
		if (!temp)
			return NULL;
		if (type)
			temp = format_uh(temp, type);
		strcpy(buffer, temp);
	}
	else
	{
		strcpy(buffer, "*!");
		if (!strchr(userhost, '@'))
			strcat(buffer, "*");
		strcat(buffer, userhost);
	}
	return buffer;
}
 
char *cluster(hostname)
char *hostname;
{
	static char result[1024];
	char	    temphost[255];
	char	    *host;
 
	if (!hostname)
                return (char *) 0;
	host = temphost;
	strcpy(result, "");
	if (strchr(hostname, '@'))
        {
            strcpy(result, hostname);
            *strchr(result, '@') = '\0';
            strcat(result, "@");
	    if (!(hostname = strchr(hostname, '@')))
            	return (char *) 0;
            hostname++;
        }
	strcpy(host, hostname);
 
	if (isdigit(*host))
	{
		int i;
		char *tmp;
                tmp = host;
		for (i=0;i<2;i++)
			tmp = strchr(tmp, '.')+1;
		*tmp = '\0';
		strcat(result, host);
		strcat(result, "*.*");
	}
	else
	{
                char	*tmp;
                int	num;
 
		num = 1;
		tmp = right(host, 3);
		if (!STRCASEEQUAL(tmp, "com") &&
			!STRCASEEQUAL(tmp, "edu") &&
			(strstr(host, "com") ||
				strstr(host, "edu")))
          		num = 2;
		while (host && *host && (numchar(host, '.') > num))
                {
			if ((host = strchr(host, '.')) != NULL)
				host++;
                        else
                                return (char *) NULL;
		}
		strcat(result, "*");
		if (my_stricmp(host, temphost))
			strcat(result, ".");
                strcat(result, host);
	}
	return result;
}
 
void steal_channels(void)
{
	ListStruct *temp;
	int number;
 
	for (temp=StealList;temp;temp=temp->next)
	{
	  number = tot_num_on_channel(temp->name);
	  if (number == 1)
		 cycle_channel(temp->name);
	}
}
 
void no_info(who, nick)
char *who;
char *nick;
{
	sendreply("No information found for %s", nick);
}
 
 
int numchar(string, c)
char *string;
char c;
{
	int num = 0;
	while (*string)
	{
		if (tolower(*string) == tolower(c))
			num++;
		string++;
	}
	return num;
}
 
char *my_stristr(s1, s2)
char *s1;
char *s2;
{
	char n1[WAYTOBIG], n2[WAYTOBIG];
	char *temp, *ptr1, *ptr2;
 
 
	ptr1 = n1;
	ptr2 = n2;
	if (!s1 || !s2)
	  return NULL;
	while (*s1)
	  *(ptr1++) = toupper(*(s1++));
	while (*s2)
	  *(ptr2++) = toupper(*(s2++));
	*ptr1 = '\0';
	*ptr2 = '\0';
	temp = strstr(n1, n2);
	if (temp)
	  return (s1 + (temp-n1));
	return NULL;
}
 
int i_am_op(channel)
char *channel;
{ 
        return is_opped(currentbot->nick, channel);
}
 
void not_opped(from, channel)
char *from;
char *channel;
{ 
        send_to_user(from, "I am not opped on %s", channel);
} 
 
void no_chan(from)
char *from;
{
  send_to_user(from, "No channel given");
}
 
void deop_ban(channel, nick, nuh)
char *channel;
char *nick;
char *nuh;
{
	if (!channel || !nick || !nuh)
		return;
	sendmode(channel, "-o+b %s %s", nick, format_uh(nuh, 1));
}
void my_db(channel, nick, uh)
char *channel;
char *nick;
char *uh;
{
    if (!channel || !nick || !uh)
        return;
    sendmode(channel, "-o+b %s %s", nick, uh);
}
 
void deop_bannick(channel, nick)
char *channel;
char *nick;
{
	char *uh;
 
	uh = nick2uh(NULL, nick, 1);
	sendmode(channel, "-o+b %s %s", nick, uh);
}
 
void deop_siteban(channel, nick, nuh)
char *channel;
char *nick;
char *nuh;
{
	if (!channel || !nick || !nuh)
		return;
	if (strchr(nuh, '*'))
        	sendmode(channel, "-o+b %s %s", nick, nuh);
        else
		sendmode(channel, "-o+b %s %s", nick, format_uh(nuh, 2));
}
 
void deop_sitebannick(channel, nick)
char *channel;
char *nick;
{
	char *uh;
 
	uh = nick2uh(NULL, nick, 2);
	sendmode(channel, "-o+b %s %s", nick, uh);
}
 
 
void mode3(char *channel, char *mode, char *data) {
	char *temp, *start;
 
	temp = data;
	temp = stripl(temp, " ");
	start = temp;
	temp = strchr(temp, ' ');
	if (temp)
	{
		temp = stripl(temp, " ");
		temp = strchr(temp, ' ');
	}
	if (temp)
	{
		temp = stripl(temp, " ");
		temp = strchr(temp, ' ');
	}
	if (temp)
	{
		mode3(channel, mode, temp);
		*temp = '\0';
	}
	sendmode(channel, "%s %s", mode, start);
}
 
void takeop(char *channel, char *nicks) {
	if (i_am_op(channel))
		mode3(channel, "-ooo", nicks);
}
 
 
void mass_action(who, channel)
char *who;
char *channel;
{
  char *temp;
 
  if (!get_masstog(channel))
	 return;
 
  temp = format_uh(who, 1);
  if (currentbot->massprotlevel >= 3)
  {
		if (!userlevel(who) &&
			!(usermode(channel, getnick(who))&MODE_CHANOP))
		{
/*			add_to_shitlist(current->ShitList, temp, 2, channel, "Auto-Shit",
			  "Quit the damn mass modes", getthetime());
			sendnotice(channel, "\002%s shitlisted for mass moding\002", getnick(who));
 */
		}
  }
  if (currentbot->massprotlevel >= 2)
	 deop_ban(channel, getnick(who), temp);
  if (currentbot->massprotlevel >= 1)
  {
	 splatter(channel, getnick(who));
	 send_wall(NULL, channel, "Mass protection activated against %s", getnick(who));
  }
}
 
void shit_action(who, channel)
char *who;
char *channel;
{
	int	level;
 
	if (!get_shittog(channel))
	  return;
 
	if (userlevel(who))  /* bypass users */
	        return;
	 
	if (!(level = shitlevel(who)))
		return;
 
	if (!i_am_op(channel))
		return;
 
	if (shitlevel(who) >= 100)
	{
	   if (userlevel(who))
	       return;
/*	   for(dummy = currentbot->lists->shitlist; dummy; dummy = dummy->next )
           {
	      sprintf(temp, "%s", dummy->userhost);
              if (!matches(temp, who))
              {
	         my_db(channel, getnick(who), temp); */
		 deop_ban(channel, getnick(who), who); 
	         splatter(channel, getnick(who));
                 send_wall(NULL, channel, "Shitlist kick on %s", getnick(who));
	         return;
/*              } 
           } */
	}
	else
	   takeop(channel, getnick(who));
}
 
void prot_action(from, channel, protnuh)
char *from;
char *channel;
char *protnuh;
{
	int level;
 
	if (!get_prottog(channel))
		return;
 
	level = protlevel(protnuh);
	if (!level || !i_am_op(channel))
		return;
 
	if (!protlevel(from))
	{
	  char *banuh;
	  banuh = format_uh(from, 1);
	  if (level >= 125)
		 deop_ban(channel, getnick(from), banuh);
	  if (level >= 100)
		 sendkick(channel, getnick(from), "%s is protected you fuck!", protnuh);
	  if (level == 50)
		 takeop(channel, getnick(from));
	}
}
 
char *right(string, num)
char *string;
int num;
{
	if (strlen(string) < num)
		return string;
	return (string+strlen(string)-num);
}
 
 
void not_on(who, channel)
char *who;
char *channel;
{
	send_to_user(who, "I'm not on %s", channel);
}
 
 
