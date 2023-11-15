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
#include "function.h"
#include "log.h"
#include "send.h"

#define getrandom(min, max) ((rand() % (int)(((max)+1) - (min))) + (min))


extern char lastcmd[1024];
extern char current_nick[MAXLEN];
extern long uptime;
extern UserStruct *UserList, *ShitList;
extern ListStruct *KickSays, *KickSayChan;
extern ListStruct *SpyMsgList;
extern ListStruct *StealList;
extern UserHostStruct *UserHostList;
extern char cmdchar;
extern int sbtype, mplevel;
extern char *progname, ircserver[FNLEN], helpfile[FNLEN], filelistfile[FNLEN];
extern char shitfile[FNLEN], usersfile[FNLEN], botlogfile[FNLEN];
extern char userhostfile[FNLEN], bot_dir[FNLEN];
extern char mailfile[FNLEN], replyfile[FNLEN];
extern int nhmtoggle, holdingnick, holdnick;
extern char nhmsg[255], ownernick[255], owneruhost[255], botdate[255];

extern char cshell[25];
char thechannelfixed[MAXLEN];
char save[1024] = "";
char nuh2[MAXLEN];
int fullspec = 0;

char *GetLevelName(int level)
{
   if(level >= BotOwner)
      return("BotOwner");
   else if(level >= BotCoOwner)
      return("BotCoOwner");
   else if(level >= ChanOwner)
      return("ChanOwner");
   else if(level >= ChanCoOwner)
      return("ChanCoOwner");
   else if(level >= ChanHelper)
      return("ChanHelper");
   else if(level >= ChanOp)
      return("ChanOp");
   else if(level >= Peon)
      return("Peon");
   else return("Nobody");
}

int ConvertAOp(char *s)
{
  char *str;
  
  if(!s)
    return(-1);
  
  if(isdigit(*s))
  {
    for(str = s;*str;str++)
    {
      if(!isdigit(*str))
        return(-1); /* Mixed letters & numbers */
    }
    return(atoi(s));
  }
  else
  {  
    if(!strcasecmp("yes", s))
       return(1);
    else if(!strcasecmp("autoop", s))
       return(1);
    else if(!strcasecmp("true", s))
       return(1);
    else if(!strcasecmp("on", s))
       return(1);
    else if(!strcasecmp("@", s))
       return(1);
    else if(!strcasecmp("op", s))
       return(1);
    else if(!strcasecmp("aop", s))
       return(1);
    else if(!strcasecmp("off", s))
       return(0);
    else if(!strcasecmp("no", s))
       return(0);
    else if(!strcasecmp("false", s))
       return(0);
    else
        return(-1);
  }

}

int ConvertProt(char *s)
{
  char *str;

  if(!s)
    return(-1);

  if(isdigit(*s))
  {
    for(str = s;*str;str++)
    {
      if(!isdigit(*str))
        return(-1); /* Mixed letters & numbers */
    }
    return(atoi(s));
  }
  else
  {
    if(!strcasecmp("full", s))
       return(5);
    else if(!strcasecmp("fuck", s))
       return(5);
    else if(!strcasecmp("screw", s))
       return(4);
    else if(!strcasecmp("ban", s))
       return(3);
    else if(!strcasecmp("kick", s))
       return(2);
    else if(!strcasecmp("deop", s))
       return(1);
    else if(!strcasecmp("none", s))
       return(0);
    else if(!strcasecmp("no", s))
       return(0);
    else if(!strcasecmp("off", s))
       return(0);
    else if(!strcasecmp("on", s))
       return(2);
    else if(!strcasecmp("true", s))
       return(2);
    else
        return(-1);
  }
}

int ConvertLevel(char *s)
{
  char *str;

  if(!s) 
    return(-1);

  if(isdigit(*s))
  {
    for(str = s;*str;str++)
    {
      if(!isdigit(*str))
        return(-1); /* Mixed letters & numbers */
    }
    return(atoi(s));
  }
  else
  {
    if(!strcasecmp("BotOwner", s))
       return(BotOwner);
    else if(!strcasecmp("BotCoOwner", s))
       return(BotCoOwner);

    else if(!strcasecmp("ChanOwner", s))
       return(ChanOwner);
    else if(!strcasecmp("Owner", s))
       return(ChanOwner);

    else if(!strcasecmp("ChanCoOwner", s))
       return(ChanCoOwner);
    else if(!strcasecmp("CoOwner", s))
       return(ChanCoOwner);

    else if(!strcasecmp("ChanHelper", s))
       return(ChanHelper);
    else if(!strcasecmp("Helper", s))
       return(ChanHelper);

    else if(!strcasecmp("ChanOp", s))
       return(ChanOp);
    else if(!strcasecmp("OP", s))
       return(ChanOp);

    else if(!strcasecmp("ChanPeon", s))
       return(Peon);
    else if(!strcasecmp("Peon", s))
       return(Peon);

    else return(-1);
  }
}

int exist(char *filename)
{
  FILE *temp;

  if ((temp = fopen(filename, "r")) == NULL)
    return FALSE;
  fclose(temp);
  return TRUE;
}

char *left(char *string, int i)
{
  static char temp[255];

  strcpy(temp, string);
  temp[i] = '\0';
  return temp;
}

void shitkick(char *who, char *channel)
{
  char *temp;
  CHAN_list *ChanPtr;

  ChanPtr = search_chan(channel);
  temp = shitchan(who);

  if (!(strstr(temp, channel) || strcmp(temp, "*")))
    return;
  if (shitlevel(who, ChanPtr) >= 3)
  {
    ban_user(channel, who);
    botlog(botlogfile, "Banned shitlisted user %s", who);
  }
  if (shitlevel(who, ChanPtr) >= 4)
  {
    screw_ban(channel, who, 1);
    botlog(botlogfile, "Screwbanned shitlisted user %s", who);
  }

  if (shitlevel(who, ChanPtr) >= 2)
  {
    sendkick(channel, getnick(who), "\x2YOU ARE SHITLISTED!!!\x2");
    botlog(botlogfile, "Kicked shitlisted user %s", who);
    return;
  }
  if (shitlevel(who, ChanPtr) < 2)
    takeop(channel, getnick(who));
}

void showspymsg(char *person, char *msg)
{
  ListStruct *Temp;

  for (Temp = SpyMsgList; Temp; Temp = Temp->next)
    send_to_user(Temp->name, "\x2*%s*\x2 %s", getnick(person), msg);
  return;
}

void make_process(char *name, char *args)
{
  int pid, p0[2], p1[2], p2[2];
  char buf[MAXLEN];

  strcpy(buf, name);
  strcat(buf, " ");
  strcat(buf, args);
  strcat(buf, " > /dev/null");
  p0[0] = p0[1] = -1;
  p1[0] = p1[1] = -1;
  p2[0] = p2[1] = -1;
#ifndef MSDOS
  pid = fork();
  if (pid == 0)
  {
    setpgrp();
    signal(SIGINT, SIG_IGN);
    dup2(p0[0], 0);
    dup2(p1[1], 1);
    dup2(p2[1], 2);
    close(p0[1]);
    close(p1[0]);
    close(p2[0]);
    execl(cshell, cshell, "-c", buf, NULL);
  }
#endif
}


void massprot_action(char *from, char *channel, char *buffer)
{
  CHAN_list *ChanPtr;

  ChanPtr = search_chan(channel);
  if(!ChanPtr)
    return; /* Not our channel */
  if (userlevel(from, ChanPtr) >= ASSTLEVEL)
    return;
  if (ChanPtr->Settings[MASSPROT] == 0)
    return;
  bot_reply(channel, 7);
  switch(ChanPtr->Settings[MPLEVEL])
  {
     case 0: /* Take ops */
             takeop(channel, getnick(from));
             return;
     case 1: /* Kick */
             sendkick(channel, getnick(from), buffer);
             return;
    case 2: /* kick bann */
            ban_user(from, channel);
            return;
    default: Debug(DBGERROR, "Unknown MPLEVEL setting %d", ChanPtr->Settings[MPLEVEL]);
  }
}

void prot_action(char *from, char *protnuh, char *channel)
{
  CHAN_list *ChanPtr;
  int level;
  char buf[MAXLEN];

  ChanPtr = search_chan(channel);
  sprintf(buf, "\x2%s is Protected!\x2", protnuh);
  level = protlevel(protnuh, ChanPtr);
  if (!level)
    level = protlevelbm(protnuh, ChanPtr);
  if (!(protlevel(from, ChanPtr) || (userlevel(from, ChanPtr) >= PROTLEVEL)))
  {
    if (level >= 5)
    {
      botlog(botlogfile, "Protection: %s screwbanned on %s",
	     from, channel);
      screw_ban(channel, from, 1);
    }
    if (level >= 4)
    {
      botlog(botlogfile, "Protection: %s banned on %s",
	     from, channel);
      ban_user(channel, from);
    }
    if (level >= 3)
    {
      botlog(botlogfile, "Protection: %s kicked from %s",
	     from, channel);
      sendkick(channel, getnick(from), buf);
    }
    if (level >= 2)
    {
      botlog(botlogfile, "Protection: %s deopped on %s",
	     from, channel);
      takeop(channel, getnick(from));
    }
    if (level <= 2)
      send_to_user(from, "\x2%s is Protected!\x2", protnuh);
  }
  else
    bot_reply(from, 5);
}

void takeop(char *channel, char *nicks)
{
  mode3(channel, "-ooo", nicks);
}

void giveop(char *channel, char *nicks)
{
  mode3(channel, "+ooo", nicks);
}

void sep_userhost(char *uhost, char *nick, char *user, char *host, char *machine)
{
  char *userhost, *temp;
  char ending[20];
  char theuh2[255];

  strcpy(theuh2, uhost);
  userhost = theuh2;
  strcpy(user, "");
  strcpy(host, "");
  strcpy(machine, "");
  strcpy(nick, "*");

  temp = strchr(userhost, '!');
  if (temp != NULL)
  {
    *(temp++) = '\0';
    strcpy(nick, userhost);
    userhost = temp;
  }
  temp = strchr(userhost, '@');
  if (temp != NULL)
  {
    *(temp++) = '\0';
    strcpy(user, userhost);
    userhost = temp;
  }
  temp = strrchr(userhost, '.');
  if (temp != NULL)
  {
    strcpy(ending, temp);
    *temp = '\0';
    temp = strrchr(userhost, '.');
    if (temp == NULL)
    {
      strcpy(host, userhost);
      strcat(host, ending);
      return;
    }
    strcpy(host, temp + 1);
    *temp = '\0';
    strcat(host, ending);
    strcpy(machine, userhost);
  }
}

void ban_user(char *channel, char *who)
/*
 * user nick!user@host.domain will be banned as:
 *      *!*user@*
 *      *!*user@*host
 */
{

  sendmode(channel, "-o+b %s %s", getnick(who), cluster(who));
}

void site_ban(char *channel, char *who)
{
  char userban[MAXLEN], n[100], u[100], h[100], m[100];
  char *user;

  user = cluster(who);
  sep_userhost(user, n, u, h, m);
  sprintf(userban, "*!*@%s%s%s", m, (*m) ? "." : "", h);
  sendmode(channel, "+b %s", userban);
}

void screw_ban(char *channel, char *who, int num)
{
  char userban[MAXLEN];

  strcpy(userban, cluster(who));
/*
  if (!sbtype)
    return;
  if (sbtype == 2)
*/
  {
    char temp[10] = "+";
    int i;

    if (num > 3)
      num = 3;
    if (num <= 0)
      num = 1;
    for (i = 0; i < num; i++)
      strcat(temp, "b");
    strcpy(userban, rand_ques(userban, num));
    sendmode(channel, "%s %s", temp, userban);
  }
/*
  else
  {
    alt_ques(userban);
    sendmode(channel, "+b %s", userban);
  }
*/
}

#ifdef NOTDEF
void save_files(char *from)
{
  send_to_user(from, "\x2Saving userlist...Shitlist...userhost...\x2");
  if (!write_userlist(usersfile))
    send_to_user(from, "%cCould not save userlist\x2", '\x2');
  if (!write_shitlist(shitfile))
    send_to_user(from, "%cCould not save shitlist\x2", '\x2');
  if (!write_userhost(userhostfile))
    send_to_user(from, "%cCould not save userhost information\x2", '\x2');
  return;
}
#endif

void signoff(char *from, char *reason)
{
/*
  save_files(from);
*/
  sendquit(reason);
  sleep(3);
  exit(0);
}

void mode3(char *channel, char *mode, char *data)
{
  char *temp = data;
  char *start;

  for (; *temp && (*temp == ' '); temp++) ;
  start = temp;
  temp = strchr(temp, ' ');
  if (temp != NULL)
  {
    for (; *temp && (*temp == ' '); temp++) ;
    temp = strchr(temp, ' ');
  }
  if (temp != NULL)
  {
    for (; *temp && (*temp == ' '); temp++) ;
    temp = strchr(temp, ' ');
  }
  if (temp != NULL)
  {
    mode3(channel, mode, temp);
    *temp = '\0';
  }
#ifdef DBUG
  printf("Sending mode: %s %s %s\n", channel, mode, start);
#endif
  sendmode(channel, "%s %s", mode, start);
}

void alt_ques(char *string)
{
  int i;

  for (i = 0; i < strlen(string); i += 2)
    if (!strchr("!@*", *(string + i)))
      *(string + i) = '?';
}

char *rand_ques(char *string, int num)
{
  char temp[255];
  int i;

  strcpy(save, "");
  for (i = 0; i < num; i++)
  {
    strcpy(temp, string);
    rand_quesfunc(temp);
    strcat(save, temp);
    strcat(save, " ");
  }
  return save;
}

void rand_quesfunc(char *string)
{
  int i, ii, iii, iiii;

  char th[5] = "?*??";

  i = getrandom(4, (strlen(string) - 5));
  for (ii = 0; ii < i; ii++)
  {
    iii = getrandom(0, (strlen(string) - 1));
    if (!strchr("!@*?", string[iii]))
    {
      iiii = getrandom(0, 3);
      *(string + iii) = th[iiii];
    }
  }
  return;
}

char *fix_channel(char *channel)
{
/* canot return local var: char thechannelfixed[MAXLEN]; */
  strcpy(thechannelfixed, channel);
  if (*channel != '#')
  {
    strcpy(thechannelfixed, "#");
    strcat(thechannelfixed, channel);
  }
  return thechannelfixed;
}

/* I'm not exactly sure what this does, or why, but it returns a local
 * variable and isnt really relivent anymore now that we always know the
 * target channel for commands.  It can probably be safely removed after
 * everything is fixed to work without it.
 */
char *find_channel(char *to, char **rest)
{
  char found[255];
  char *chan = found;

  strcpy(chan, to);
  if (!ischannel(to))
    strcpy(chan, currentchannel());
  if (**rest)
  {
    if (ischannel(*rest))
    {
      strcpy(chan, *rest);
      if (strchr(chan, ' ') != NULL)
	*strchr(chan, ' ') = '\0';
      *rest = strchr(*rest, ' ');
      if (*rest != NULL)
	*rest = *rest + 1;
      else
	*rest = "";
    }
  }
  return chan;
}

char *getuserhost2(char *nick)
{
  UserHostStruct *temp;

/* canot return local var!!  char nuh2[MAXLEN]; */
  char *temp2;

  if (num_userhost(nick) != 1)
    return "";
  temp = find_userhostfromnick(UserHostList, nick);
  if (temp)
  {
    strcpy(nuh2, temp->name);
    temp2 = strchr(nuh2, '!');
    if (temp2)
      strcpy(nuh2, temp2 + 1);
    return nuh2;
  }
  else
    return "";
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
    if (!*temp2)
      strcpy(temp2, getuserhost2(nick));
    if (!*temp2)
    {
      send_to_user(from, "\x2No information for %s\x2", nick);
      return "";
    }
/*    strcpy(temp, nick);
    strcat(temp, "!"); */
    strcpy(temp, temp2);
    fullspec = 0;
  }
  return temp;
}

void steal_channels(void)
{
  ListStruct *temp;

  for (temp = StealList; temp; temp = temp->next)
  {
    if (tot_num_on_channel(temp->name) == 1)
      cycle_channel(temp->name);
  }
}

int numchar(char *string, char c)
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

char *right(char *string, int num)
{
  if (strlen(string) < num)
    return string;
  return (string + strlen(string) - num);
}

char *cluster(char *userhost)
{
  char *nick, *user;
  char nuh[255];
  char fullhost[255];
  static char buffer[255];
  char *temp, *tptr;
  int numofdots, x, isnd = 0;
  int didtruncate = 0;

  numofdots = 1;
  strcpy(nuh, userhost);
  nick = (char *) strtok(nuh, "!");
  if (!nick || !*nick)
    nick = "*";
  user = (char *) strtok(NULL, "@");
  if (user)
    user = (char *) strip(strip(user, '~'), '#');
  if (!user || !*user)
    user = "*";
  temp = (char *) strtok(NULL, "\n\0");
  if (temp && *temp)
    strcpy(fullhost, temp);
  else
    strcpy(fullhost, "*");
  temp = (char *) right(fullhost, 2);
/* This will have to be reworked when the new domains are deploid */
  tptr = &temp[strlen(temp) - 4];
  if (!strcasecmp(tptr, ".com") ||
      !strcasecmp(tptr, ".edu") ||
      !strcasecmp(tptr, ".org") ||
      !strcasecmp(tptr, ".net") ||
      !strcasecmp(tptr, ".gov") ||
      !strcasecmp(tptr, ".mil"))
  {
    numofdots = 1;
  }
  else
  {
    numofdots = 2;
  }
  temp = fullhost;
  for (x = 0; x <= (strlen(fullhost) - 1); x++) 
  {
    if (!(fullhost[x] == '.') && !isdigit(fullhost[x]))
      isnd++;
  }

  if (isnd == 0)                /* If its digits */
  {
    while (temp && *temp && (numchar(temp, '.') > 1))
    {
      temp = (char *) strchr(temp, '.');
      if (temp)
        temp++;
      else
        return "";
    }
    if (temp)
      *temp = '\0';
    strcat(fullhost, "*");
    temp = fullhost;
  }
  else                          /* If its a hostname */
    while (temp && *temp && (numchar(temp, '.') > numofdots))
    {
      didtruncate++;
      temp = (char *) strchr(temp, '.');
      if (temp)
        temp++;
      else
        return "";
    }   
  /* We must omit the * after ! if the username is 10, because in
   * "standard" ircd's you canot ban 11 char usernames and it counts
   * the "*" as a char. so *!*1234567890@ will result in the 0 being
   * dropped and the ban being uneffective. -Rubin */
  if (strlen(user) > 9)
    strcpy(buffer, "*!");
  else
    strcpy(buffer, "*!*");
  strcat(buffer, user);
  if (isnd == 0)
    strcat(buffer, "@");
  else
  {
    if (didtruncate)
      strcat(buffer, "@*.");
    else
      strcat(buffer, "@");
  }
  strcat(buffer, temp);
  return buffer;
}

char *cluster_busted( char *userhost)
{
        char *nick, *user;
        char nuh[255];
        char fullhost[255];
        static char buffer[255];
        char *temp, *tptr;
        int numofdots, x, isnd = 0;
        int didtruncate = 0;
 
        numofdots = 1;
        strcpy(nuh, userhost);
        nick = (char *) strtok(nuh, "!");
        if (!nick || !*nick)
                nick = "*";
        user = (char *) strtok(NULL, "@");
        if (user)
                user = (char *) strip(strip(user, '~'), '#');
        if (!user || !*user)
                user = "*";
        temp = (char *) strtok(NULL, "\n\0");
        if (temp && *temp)
          strcpy(fullhost, temp);
        else
          strcpy(fullhost, "*");
        temp = (char *) right(fullhost, 2);
/* This will have to be reworked when the new domains are deploid */
        tptr = &temp[strlen(temp)-4];
        if ( !strcasecmp(tptr, ".com") || 
             !strcasecmp(tptr, ".edu") ||
             !strcasecmp(tptr, ".org") || 
             !strcasecmp(tptr, ".net") || 
             !strcasecmp(tptr, ".gov") || 
             !strcasecmp(tptr, ".mil") )
        {
              numofdots = 1;
        }
        else
        {
              numofdots = 2;
        }
        temp = fullhost;
        for(x=0;x <= (strlen(fullhost)-1);x++)
        {
                if(!(fullhost[x] == '.') && !isdigit(fullhost[x]))
                  isnd++;
        }
        if(isnd == 0) /* If its digits */
        {
             while (temp && *temp && (numchar(temp, '.') > 1))
             {
                     temp = (char *) strchr(temp, '.');
                     if (temp)
                             temp++;
                     else
                             return "";
             }
             if (temp)
                     *temp = '\0';
             strcat(fullhost, "*");
             temp = fullhost;
        }
        else /* If its a hostname */
        while (temp && *temp && (numchar(temp, '.') > numofdots))
        {
              didtruncate++;
              temp = (char *) strchr(temp, '.');
              if (temp)
                  temp++;
              else
                  return "";
        }
        /* We must omit the * after ! if the username is 10, because in
         * "standard" ircd's you canot ban 11 char usernames and it counts
         * the "*" as a char. so *!*1234567890@ will result in the 0 being
         * dropped and the ban being uneffective. -Rubin */
        if(strlen(user) > 9)
          strcpy(buffer, "*!");
        else
          strcpy(buffer, "*!*");
        strcat(buffer, user);
        if(isnd == 0)
          strcat(buffer, "@");
        else
        {
          if(didtruncate)
            strcat(buffer, "@*.");
          else
            strcat(buffer, "@");
        }
        strcat(buffer, temp);
        return buffer;
}

char *cluster_orig(char *userhost)
{
  char *nick, *user;
  char nuh[255];
  char fullhost[255];
  static char buffer[255];
  char *temp;
  int num = 1;
  int x, isnd = 0;

  strcpy(nuh, userhost);
  if (fullspec)
  {
    fullspec = 0;
    strcpy(buffer, nuh);
    return buffer;
  }
  nick = strtok(nuh, "!");
  if (!nick || !*nick)
    nick = "*";
  user = strtok(NULL, "@");
  if (user)
    user = strip(strip(user, '~'), '#');
  if (!user || !*user)
    user = "*";
  temp = strtok(NULL, "\n\0");
  if (temp && *temp)
    strcpy(fullhost, temp);
  else
    strcpy(fullhost, "*");
  temp = right(fullhost, 2);
/*      if (strcasecmp(temp, "com") && strcasecmp(temp, "edu"))
 * num = 2; */
  temp = fullhost;
  for (x = 0; x <= (strlen(fullhost) - 1); x++)
  {
    if (!(fullhost[x] == '.') && !isdigit(fullhost[x]))
      isnd++;
  }

  if (isnd == 0)
  {
    while (temp && *temp && (numchar(temp, '.') > 1))
    {
      temp = strchr(temp, '.');
      if (temp)
	temp++;
      else
	return "";
    }
    if (temp)
      *temp = '\0';
    strcat(fullhost, "*");
    temp = fullhost;
  }
  else
    while (temp && *temp && (numchar(temp, '.') > num))
    {
      temp = strchr(temp, '.');
      if (temp)
	temp++;
      else
	return "";
    }
  /* We must omit the * after ! if the username is 10, because in 
   * "standard" ircd's you canot ban 11 char usernames and it counts
   * the "*" as a char. so *!*1234567890@ will result in the 0 being
   * dropped and the ban being uneffective. -Rubin */
  if (strlen(user) > 9)
    strcpy(buffer, "*!");
  else
    strcpy(buffer, "*!*");
  strcat(buffer, user);
  if (isnd == 0)
    strcat(buffer, "@");
  else
    strcat(buffer, "@*");
  strcat(buffer, temp);
  return buffer;
}

void bot_reply(char *to, int num)
{
  extern char instancename[MAXLEN];
  int count = 0;
  char buffer[MAXLEN] = "";
  char filename[MAXLEN];
  FILE *fp;

  sprintf(filename, "%s/%s/%s", bot_dir, instancename, REPLYFILE);

  if (!(fp = fopen(filename, "r")))
  {
    Debug(DBGERROR, "Cant find reply file %s", replyfile);
    return;
  }
  for (; count < num; count++)
    readln_from_a_file(fp, buffer);
  fclose(fp);

  strcpy(buffer, strip(buffer, ' '));
  if (*buffer)
  {
    if (ischannel(to))
      sendprivmsg(to, buffer);
    else
      send_to_user(to, buffer);
  }
}
