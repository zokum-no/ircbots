/*
 * channel.c - a better implementation to handle channels on IRC
 */

#include <stddef.h>
#include <stdio.h>
/*
 * #include <strings.h>
 */
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "chanuser.h"
#include "channel.h"
#include "function.h"
#include "userlist.h"
#include "misc.h"
#include "config.h"
#include "rockbot.h"
#include "send.h"
#include "match.h"

CHAN_list *Channel_list = NULL;
char mycurchannel[MAXLEN] = "";
static char channel_buf[MAXLEN];
char globsign = ' ';
extern char current_nick[MAXNICKLEN];
extern char bot_dir[MAXLEN];
extern char upload_dir[FNLEN];
extern char download_dir[FNLEN];
extern char nickname[255];
extern char USERNAME[20];
extern char ircserver[255];
extern char ircname[255];
extern char nhmsg[255], ownernick[255], owneruhost[255], botdate[255];
extern char cmdchar;
extern SettingStruct set_commands[];
extern RockBotStruct bot;

void SaveDatFile(CHAN_list *ChanPtr)
{
 FILE *DatFile;
 UserStruct *OpPtr;
 ShitStruct *BanPtr;
 RESPONDER_list *rptr;
 int i;
 char ModeString[200];
 CHAN_list *ChanTmp;
 SERVER_list *ServPtr;

 if(!ChanPtr)
   return;
 errno = 0;
 if((DatFile = fopen("~Temp.dat", "w")))
 {
   fprintf(DatFile, "#########################################################\n");
   fprintf(DatFile, "##  RockBot %10s                 settings file   ##\n", ChanPtr->name);
   fprintf(DatFile, "#########################################################\n");
   if(ChanPtr == Globals)
   {
     /* Stuff in the globals.dat file, such as nick, cmdchar, etc */
     fprintf(DatFile, "#The USER key defines the bots start out nick.\n");
     fprintf(DatFile, "NICK %s\n", nickname);
     fprintf(DatFile, "#\n");
     fprintf(DatFile, "# The USERID key is what the username will be if ident fails.\n");
     fprintf(DatFile, "USERID %s\n", USERNAME);
     fprintf(DatFile, "#\n");
     fprintf(DatFile, "# The SERVER key is a server the bot should use. Multiple SERVER keys are ok.\n");
     fprintf(DatFile, "# Syntax: SERVER <server> <port> [password]\n");
     for(ServPtr = bot.servers;ServPtr;ServPtr=ServPtr->next)
     {
       fprintf(DatFile, "SERVER %s %d %s\n", ServPtr->name, ServPtr->port, ServPtr->password);
     }
     fprintf(DatFile, "#\n");
     fprintf(DatFile, "# The IRCNAME key is the 'realname' field on irc.\n");
     fprintf(DatFile, "IRCNAME %s\n", ircname);
     fprintf(DatFile, "#\n");
     fprintf(DatFile, "# The OWERNICK key is what nick the bot will save for the HN command.\n");
     fprintf(DatFile, "OWNERNICK %s\n", ownernick);
     fprintf(DatFile, "#\n");
     fprintf(DatFile, "# The OWERMASK key is a way for the bot to know you, when it has no userlist yet.\n");
     fprintf(DatFile, "OWNERMASK %s\n", owneruhost);
     fprintf(DatFile, "#\n");
     fprintf(DatFile, "# The CMDCHAR key tells the bot what command charactor to use.\n");
     fprintf(DatFile, "CMDCHAR %c\n", cmdchar);
     fprintf(DatFile, "#\n");
/*
     fprintf(DatFile, "# The BOTDIR key is where the bot data files are.\n");
     fprintf(DatFile, "BOTDIR %s\n", bot_dir);
     fprintf(DatFile, "#\n");
     fprintf(DatFile, "# The UPDIR key is what directory to put uploaded files\n");
     fprintf(DatFile, "UPDIR %s\n", upload_dir);
     fprintf(DatFile, "#\n");
*/
     fprintf(DatFile, "# The DOWNDIR key is what directory the files to download are in.\n");
     fprintf(DatFile, "DOWNDIR %s\n", download_dir);
     fprintf(DatFile, "#\n");
     fprintf(DatFile, "# The CHANNEL key tells the bot to read in a channel data file and join a channel\n");
     fprintf(DatFile, "# CHANNEL <name>\n");
     for(ChanTmp = Channel_list;ChanTmp;ChanTmp=ChanTmp->next)
     {
       fprintf(DatFile, "CHANNEL %s\n", ChanTmp->name);
     }
   }
   /* Settings etc */
   fprintf(DatFile, "#\n");
   fprintf(DatFile, "# The TOPIC key defines the channels default topic\n");
   fprintf(DatFile, "TOPIC %s\n", ChanPtr->topic);
   GetModeString(ModeString, ChanPtr->PlusModes, ChanPtr->MinModes, ChanPtr->Key,
                 ChanPtr->Limit);
   fprintf(DatFile, "#\n");
   fprintf(DatFile, "# The ENFMODES key is a string defining the default modes to be enforced.\n");
   fprintf(DatFile, "ENFMODES %s\n", *ModeString?ModeString:"*" );
   fprintf(DatFile, "#\n");
   fprintf(DatFile, "# The GETOP key is a string we send to get ops if we are lacking them.\n");
   fprintf(DatFile, "# example: PRIVMSG $C .UP  or PRIVMSG X2 $C UP\n");
   fprintf(DatFile, "GETOP %s\n", ChanPtr->getop);
   fprintf(DatFile, "#\n");
   fprintf(DatFile, "# The RESPONDER key is a get/send pair to automate logging in or\n");
   fprintf(DatFile, "# really anything you can think of. Sort of rudimentry script support.\n");
   fprintf(DatFile, "# RESPONDER <Name> <expect>:<send>\n");
   for(rptr=ChanPtr->Responder;rptr;rptr = rptr->next)
   {
       fprintf(DatFile, "RESPONDER %s %s:%s\n", rptr->name, rptr->expect, rptr->send);
   }
   fprintf(DatFile, "#\n");
   /* USER */
   fprintf(DatFile, "# The USER key Defines the channel's(or global) users.\n"); 
   fprintf(DatFile, "# hostmask   = nick!user@host with standard * and ? as wild cards.\n");
   fprintf(DatFile, "# handle     = This is the persons 'normal' name. If a person has multiple\n");
   fprintf(DatFile, "#               hosts, setting each hosts handle the same makes dealing with\n");
   fprintf(DatFile, "#              the userlist much simpler.\n");
   fprintf(DatFile, "# pass       = This can be set to * to disable the need, or if set to anything\n");
   fprintf(DatFile, "#              else the bot will require you to authenticate first.\n");
   fprintf(DatFile, "# userlevel  = 01-10:Peon \n");
   fprintf(DatFile, "#               11-20:ChanOp \n");
   fprintf(DatFile, "#              21-30:ChanHelper \n");
   fprintf(DatFile, "#              31-40:ChanCoOwner \n");
   fprintf(DatFile, "#              41-60:ChanOwner \n");
   fprintf(DatFile, "#              61-80:BotCoOwner \n");
   fprintf(DatFile, "#              81-100:BotOwner\n");
   fprintf(DatFile, "# protectlev = 0:none\n");
   fprintf(DatFile, "#              1:unban/reop but dont punish\n");
   fprintf(DatFile, "#              2:unban/reop & deop perpitrator\n");
   fprintf(DatFile, "#              3:unban/reop & kick perpitrator\n");
   fprintf(DatFile, "#              4:unban/reop & kickban perpitrator\n");
   fprintf(DatFile, "#              5:unban/reop & screwban perpitrator\n");
   fprintf(DatFile, "# autoop     = 0:dont auto-op\n");
   fprintf(DatFile, "#              1:auto-op\n");
   fprintf(DatFile, "# security   = 0:secure hostmask. No pass required\n");
   fprintf(DatFile, "#              1:insecure hostmask. Require AUTH command before granting privlidges.\n");
   fprintf(DatFile, "# info       = The bot will say this in the chan when you join\n");
   fprintf(DatFile, "#\n");
   fprintf(DatFile, "# Syntax: USER <hostmask> <handle> <pass> <userlevel> <protectlevel> <autoop> <security> <announce info line>\n");
   fprintf(DatFile, "#         HOST <additional hostmask>\n");
   for(OpPtr=ChanPtr->OpList;OpPtr;OpPtr=OpPtr->next)
   {        /* Save OpList */
     fprintf(DatFile, "USER %s %s %s %d %d %d %d %s\n", 
          OpPtr->userhosts->userhost, OpPtr->handle, OpPtr->password, OpPtr->level, OpPtr->protlevel,
          OpPtr->isaop, OpPtr->userhosts->security, OpPtr->info);
     if(OpPtr->userhosts->next)
     {
        HostListStruct *HostPtr;

        for(HostPtr=OpPtr->userhosts->next;HostPtr;HostPtr=HostPtr->next)
        {
           fprintf(DatFile, "HOST %s %d\n", HostPtr->userhost, HostPtr->security);
        }
     }
   }
   /* Bans */
   fprintf(DatFile, "#\n");
   fprintf(DatFile, "# BAN defines bans for either a channel or over all global.\n");
   fprintf(DatFile, "# hostmask = nick!user@host with standard * and ? as wild cards.\n");
   fprintf(DatFile, "# level    = What level of ban (0 - 5)\n");
   fprintf(DatFile, "# bywho    = nick!user@host of the person who set the ban.\n");
   fprintf(DatFile, "# when     = timestamp the ban was set\n");
   fprintf(DatFile, "# expire   = timestamp the ban should be removed (0 for none)\n");
   fprintf(DatFile, "# reason   = duh. .\n");
   fprintf(DatFile, "#\n");
   fprintf(DatFile, "# Syntax: BAN <hostmask> <level> <bywho> <when> <expiretime> <reason>\n");
   for(BanPtr=ChanPtr->ShitList;BanPtr;BanPtr=BanPtr->next)
   {       /* Save Bans */
     fprintf(DatFile, "BAN %s %d %s %lu %lu %s\n", 
          BanPtr->userhost, BanPtr->level, BanPtr->bannedby, BanPtr->timestamp, BanPtr->expiretime, BanPtr->reason);
   }
   fprintf(DatFile, "#\n");
   fprintf(DatFile, "# The SETTING key defines one of the set options, and its value.\n");
   fprintf(DatFile, "# SETTING <Set name> <value>\n");
   for (i = 0; set_commands[i].name; i++)
   {
      fprintf(DatFile, "SETTING %s %d\n", set_commands[i].name, ChanPtr->Settings[i]);
   }
   fprintf(DatFile, "#\n");
   fprintf(DatFile, "# End of %s config file %s.\n", ChanPtr->name, ChanPtr->datfile);
   fclose(DatFile);
   if(!errno)
   {
     unlink(ChanPtr->datfile);
     rename("~Temp.dat", ChanPtr->datfile);
   }
   else
   {
     Debug(DBGERROR, "Error saving data file, changes not saved!");
     Debug(DBGERROR, "Writing ~Temp.dat for %s: %s", ChanPtr->datfile, strerror(errno));
     exit(1);
   }
 }
}

char *ConvertChanMode(char *StrModes, unsigned char Modes)
{
  unsigned char Mode;
  char *tmp;

  tmp = StrModes;
  Mode = CMODEn;
  if ((Mode & Modes) != 0)
    *StrModes++ = 'n';

  Mode = CMODEt;
  if ((Mode & Modes) != 0)
    *StrModes++ = 't';

  Mode = CMODEs;
  if ((Mode & Modes) != 0)
    *StrModes++ = 's';

  Mode = CMODEp;
  if ((Mode & Modes) != 0)
    *StrModes++ = 'p';

  Mode = CMODEi;
  if ((Mode & Modes) != 0)
    *StrModes++ = 'i';

  Mode = CMODEm;
  if ((Mode & Modes) != 0)
    *StrModes++ = 'm';

  Mode = CMODEk;
  if ((Mode & Modes) != 0)
    *StrModes++ = 'k';

  Mode = CMODEl;
  if ((Mode & Modes) != 0)
    *StrModes++ = 'l';

  *StrModes = '\0';
  return (tmp);
}

char *GetModeString(char *ModeString, int PlusModes, int MinModes, char *key, char *limit)
{
  char tmpstr[200];

  strcpy(ModeString, "");
  if (PlusModes)
  {
    strcat(ModeString, "+");
    strcat(ModeString, ConvertChanMode(tmpstr, PlusModes));
  }
  if (MinModes)
  {
    strcat(ModeString, "-");
    strcat(ModeString, ConvertChanMode(tmpstr, MinModes));
  }
  if (PlusModes & CMODEk)
  {
    strcat(ModeString, " ");
    strcat(ModeString, key);
  }
  if (PlusModes & CMODEl)
  {
    strcat(ModeString, " ");
    strcat(ModeString, limit);
  }
  return (ModeString);
}

char *get_enforced_modes(char *name)
{
  return ("");
}

int set_enforced_modes(char *name, char *themodes)
{
  return (TRUE);
}

CHAN_list *search_chan(char *name)
{
  CHAN_list *Channel;

  for (Channel = Channel_list; Channel; Channel = Channel->next)
    if (!strcasecmp(name, Channel->name))
    {
      return (Channel);
    }
  return (NULL);
}

/* void  init_enforced_modes( char *themodes)
 * {
 * CHAN_list *Channel;
 * channel = search_chan( name ))
 * strcpy(Channel->enfmodes, themodes);
 * }
 */
int add_spy_chan(char *name, char *nick)
{
  CHAN_list *Channel;

  if ((Channel = search_chan(name)) != NULL)
    return add_to_list(&(Channel->spylist), nick);
  return FALSE;
}

int del_spy_chan(char *name, char *nick)
{
  CHAN_list *Channel;

  if ((Channel = search_chan(name)) != NULL)
    return remove_from_list(&(Channel->spylist), nick);
  return FALSE;
}

void show_spy_list(char *from, char *name)
{
  ListStruct *temp;

  CHAN_list *Channel;

  if ((Channel = search_chan(name)) != NULL)
    for (temp = Channel->spylist; temp; temp = temp->next)
      send_to_user(from, temp->name);
}

void send_spy_chan(char *name, char *text)
{
  ListStruct *temp;

  CHAN_list *Channel;

  if ((Channel = search_chan(name)) != NULL)
    for (temp = Channel->spylist; temp; temp = temp->next)
      send_to_user(temp->name, text);
}

int is_spy_chan(char *name, char *nick)
{
  CHAN_list *Channel;

  if ((Channel = search_chan(name)) != NULL)
    return is_in_list(Channel->spylist, nick);
  return FALSE;
}

void AddChanResponder(CHAN_list *ChanPtr, char *name, char *expect, char *send)
{
   RESPONDER_list *rptr;

   rptr = malloc(sizeof(*rptr));
   if(!rptr)
   {
       Debug(DBGERROR, "Out of ram adding responder");
       return; 
   }
   rptr->name = malloc(sizeof(name) + 1);
   rptr->expect = malloc(sizeof(expect) + 1);
   rptr->send = malloc(sizeof(send) + 1);
   strcpy(rptr->name, name);
   strcpy(rptr->expect, expect);
   strcpy(rptr->send, send);

   rptr->next = ChanPtr->Responder;
   ChanPtr->Responder = rptr;
}

void AddRegChan(char *name)
{
  CHAN_list *ChanPtr;
  int i;
  extern char instancename[MAXLEN];

  if((ChanPtr = malloc(sizeof(*ChanPtr))))
  {
    strcpy(ChanPtr->name, name);
    sprintf(ChanPtr->datfile, "%s/%s/%s.dat", bot_dir, instancename, name);
    strcpy(ChanPtr->topic, "*");
    ChanPtr->active = 1;
    ChanPtr->PlusModes = 0;
    ChanPtr->MinModes = 0;
    ChanPtr->Key = NULL;
    ChanPtr->Limit = NULL;
    strcpy(ChanPtr->CurrentKey, "");
    ChanPtr->Modes = 0;
    ChanPtr->users = NULL;
    ChanPtr->banned = NULL;
    ChanPtr->OpList = NULL;
    ChanPtr->ShitList = NULL;
    ChanPtr->Responder = NULL;
    strcpy(ChanPtr->getop, "*");
    for (i = 0; i < NUMOFCHANSETTINGS; i++)
      ChanPtr->Settings[i] = Globals->Settings[i];
    ChanPtr->spylist = NULL;
    ChanPtr->next = Channel_list;
    Channel_list = ChanPtr;
  }
  else
  {
    printf("Error were out of memory!\n");
    exit(1);
  }
}

void add_channel(CHAN_list * Channel)
/*
 * adds Channel to the list... 
 */

{
  Channel->next = Channel_list;
  Channel_list = Channel;
  /* Were going to actualy initalize the userlist here, 
   * dont faint.. */
}

int delete_channel(CHAN_list * Channel)
/*
 * removes channel from list 
 */

{
  CHAN_list **old;

  for (old = &Channel_list; *old; old = &(**old).next)
    if (*old == Channel)
    {
      *old = Channel->next;
      clear_all_users(&(Channel->users));
      remove_all_from_list(&(Channel->spylist));
      free(Channel);
      return (TRUE);
    }
  return (FALSE);
}

int join_channel(char *name, int join_mode)
{
  CHAN_list *ChanPtr;

  if(!ischannel(name))
    return (FALSE);

  ChanPtr = search_chan(name);
  if(!ChanPtr)   /* New channel */
  {
    return(FALSE);
  }
  else
  {
    clear_all_users(&(ChanPtr->users));
    clear_all_bans(&(ChanPtr->banned));
    send_command(1000, "JOIN %s %s", name, ChanPtr->CurrentKey);
    ChanPtr->active = TRUE;
  }
  return TRUE;
}

int mychannel(char *name)
{
  return (search_chan(name) != NULL);
}

int leave_channel(char *name)
/*
 * removes channel "name" from the list and actually leaves
 * the channel
 */

{
  CHAN_list *Dummy;

  if ((Dummy = search_chan(name)) != NULL)
  {
    sendpart(name);
    delete_channel(Dummy);
    /* Channel was found and we left it
     * (or at least  removed it from the list)
     */
    return (TRUE);
  }
  /*
   * There was obviously no such channel in the list
   */
  return (FALSE);
}

int mark_success(char *name)
/*
 * Marks channel "name" active
 */
{
  CHAN_list *Dummy;

  if ((Dummy = search_chan(name)) != NULL)
  {
    Dummy->active = TRUE;
    return (TRUE);
  }
  return (FALSE);
}

int mark_failed(char *name)
/*
 * Marks channel "name" not active
 */
{
  CHAN_list *Dummy;

  if ((Dummy = search_chan(name)) != NULL)
  {
    Dummy->active = FALSE;
    return (TRUE);
  }
  return (FALSE);
}

void show_channellist(char *user)
{
  CHAN_list *ChanPtr;

  if(!Channel_list)
  {
     send_to_user(user, "\002No channels registered.\002");
     return;
  }
  send_to_user(user, "\002---Channel-----+--Status--+--Topic-\002");
  for(ChanPtr = Channel_list;ChanPtr;ChanPtr = ChanPtr->next)
  {
     send_to_user(user, "\002%14s\002 - %8s \002-\002 %s", ChanPtr->name, ChanPtr->active?"Active":"Inactive", ChanPtr->topic);
  }
  send_to_user(user, "\002----------End of chan list---------\002");
}

#ifdef NOTDEF
void show_channellist(char *user)
{
  CHAN_list *Channel;
  unsigned int mode;
  char modestr[MAXLEN];

  if (Channel_list == NULL)
  {
    send_to_user(user, "I'm not active on any channels");
    return;
  }

  send_to_user(user, "Active on: %s", mycurchannel);
  for (Channel = Channel_list; Channel; Channel = Channel->next)
  {
    mode = Channel->mode;
    strcpy(modestr, "");
    if (mode & MODE_PRIVATE)
      strcat(modestr, "p");
    if (mode & MODE_SECRET)
      strcat(modestr, "s");
    if (mode & MODE_MODERATED)
      strcat(modestr, "m");
    if (mode & MODE_TOPICLIMIT)
      strcat(modestr, "t");
    if (mode & MODE_INVITEONLY)
      strcat(modestr, "i");
    if (mode & MODE_NOPRIVMSGS)
      strcat(modestr, "n");
    if (mode & MODE_KEY)
    {
      strcat(modestr, "k ");
      strcat(modestr, Channel->key);
      strcat(modestr, " ");
    }
    if (mode & MODE_LIMIT)
    {
      strcat(modestr, "l ");
      strcat(modestr, Channel->limit);
    }
    send_to_user(user, "        -  %s, mode: +%s", Channel->name,
		 modestr);
  }
  return;
}
#endif

void reset_channels(int Mode)
/*
 * This function tries to join all inactive channels
 */
{
  CHAN_list *Channel;

  for (Channel = Channel_list; Channel; Channel = Channel->next)
    if ((Mode == HARDRESET) || !(Channel->active))
      join_channel(Channel->name, 1);
}

void setcurrentchannel(char *channel)
{
  strcpy(mycurchannel, channel);
}

char *currentchannel()
/*
 * returns name of current channel, i.e. the last joined
 */
{
  CHAN_list *Dummy;

  if ((Dummy = search_chan(mycurchannel)) == NULL)
  {
    if (Channel_list)
      strcpy(channel_buf, Channel_list->name);
    else
      strcpy(channel_buf, "#0");
  }
  else
    strcpy(channel_buf, mycurchannel);
  return (channel_buf);
}

/*
 * Ok, that was the basic channel stuff... Now some functions to
 * add users to a channel, remove them and one to change a nick on ALL
 * channels.
 */

int add_user_to_channel(char *channel, char *nick, char *user, char *host)
{
  CHAN_list *Dummy;

  if ((Dummy = search_chan(channel)) == NULL)
    return (FALSE);
  add_user(&(Dummy->users), nick, user, host);
  return (TRUE);
}

int remove_user_from_channel(char *channel, char *nick)
{
  CHAN_list *Dummy;

  if ((Dummy = search_chan(channel)) == NULL)
    return (FALSE);
  delete_user(&(Dummy->users), nick);
  return (TRUE);
}

void show_chanusers(char *nick, char *channel)
{
  CHAN_list *Dummy;

  if ((Dummy = search_chan(channel)) == NULL)
  {
    send_to_user(nick, "\x2I'm not on %s\x2", channel);
    return;
  }
  send_to_user(nick, "\x2------ Users on %s ------\x2", channel);
  send_to_user(nick, "\x2nick      username  host\x2");
  send_to_user(nick, "\x2-------------------------\x2");
  display_chanusers(nick, Dummy->users);
  send_to_user(nick, "\x2--------- End of list ---------\x2");
}

void change_nick(char *oldnick, char *newnick)
/*
 * Searches all channels for oldnick and changes it into newnick
 */
{
  CHAN_list *Channel;

  for (Channel = Channel_list; Channel; Channel = Channel->next)
    change_user_nick(&(Channel->users), oldnick, newnick, Channel->name);
}

void remove_user(char *nick)
/*
 * Remove a user from all channels (signoff)
 */
{
  CHAN_list *Channel;

  for (Channel = Channel_list; Channel; Channel = Channel->next)
    delete_user(&(Channel->users), nick);
}

void send_kickall(char *nick, char *msg)
{
  CHAN_list *Channel;

  for (Channel = Channel_list; Channel; Channel = Channel->next)
    sendkick(Channel->name, nick, msg);
}

/* This is NEVER called anywhere.. */
void send_banall(char *nuh)
{
  CHAN_list *Channel;

  for (Channel = Channel_list; Channel; Channel = Channel->next)
    send_command(0, "MODE %s +b %s", Channel->name, nuh);
}

void send_screwbanall(char *nuh)
{
  CHAN_list *Channel;

  for (Channel = Channel_list; Channel; Channel = Channel->next)
    screw_ban(Channel->name, nuh, 1);
}

void update_idletime(char *channel, char *nuh)
{
  CHAN_list *Channel;
  USER_list *User;

  if ((Channel = search_chan(channel)) == NULL)
    return;
  if ((User = search_user(&(Channel->users), getnick(nuh))) == NULL)
    return;
  User->idletime = getthetime();
}

void update_flood(char *channel, char *nuh)
{
  CHAN_list *Channel;
  USER_list *User;
  int floodlimit;
  CHAN_list *ChanPtr;

  ChanPtr = search_chan(channel);
  if(!ChanPtr)
    return; /* Not our channel */
  if ((Channel = search_chan(channel)) == NULL)
    return;
  if ((User = search_user(&(Channel->users), getnick(nuh))) == NULL)
    return;
  if ((getthetime() - User->floodtime) > 5)
  {
    User->floodtime = getthetime();
    User->floodnum = 0;
  }
  User->floodnum++;

  switch(ChanPtr->Settings[FLOODKICK])
  {
    case 0: /* None */
            return;
    case 1: /* Low value */
            floodlimit = 10;
            break;
    case 2: /* Medium */
            floodlimit = 7;
            break;
    case 3: /* High */
            floodlimit = 5;
            break;
    default: Debug(DBGERROR, "Unknown FLOODKICK value!");
  }
  if (User->floodnum >= floodlimit)
  {
    if (protlevel(get_username(User), Channel) <= 2)
      sendkick(channel, User->nick, "\002NO FLOODING!!!\002");
  }
}

int num_on_channel(char *channel, char *uh)
{
  CHAN_list *Channel;
  USER_list *User;
  int num = 0;
  char n[100], u[100], h[100], m[100];

  sep_userhost(uh, n, u, h, m);
  strcpy(u, fixuser(u));
  if ((Channel = search_chan(channel)) == NULL)
    return 0;
  for (User = Channel->users; User; User = User->next)
  {
    if (!strcasecmp(fixuser(User->user), u))
      num++;
  }
  return num;
}

int tot_num_on_channel(char *channel)
{
  CHAN_list *Channel;
  USER_list *User;
  int num = 0;

  if ((Channel = search_chan(channel)) == NULL)
    return 0;
  for (User = Channel->users; User; User = User->next)
    num++;
  return num;
}

void cycle_channel(char *channel)
{
  CHAN_list *ChanPtr;

  if(!ChanPtr)
  {
    Debug(DBGWARNING, "Channel not found during cycle");
    return;
  }
  sendpart(channel);
  sendjoin(channel, ChanPtr->CurrentKey);
}

void idle_kick(void)
{
  CHAN_list *ChanPtr;
  USER_list *User;
  time_t currenttime;
  char buffer[MAXLEN];
  int idlelimit;

  currenttime = getthetime();
  for(ChanPtr = Channel_list; ChanPtr; ChanPtr = ChanPtr->next)
  {
     switch(ChanPtr->Settings[IDLEKICK])
     {
       case 0: /* DISABLED */
               continue;
       case 1: /* 10 minutes */
               idlelimit = 10 * 60;
               break;
       case 2: /* 30 minues */
               idlelimit = 30 * 60;
               break;
       case 3: /* 60 minutes */
               idlelimit = 60 * 60;
               break;
       case 4: /* 4 hours */
               idlelimit = 60 * 60 * 4;
               break;
       case 5: /* 12 hours */
               idlelimit = 60 * 60 * 12;
               break;
       default: Debug(DBGERROR, "Unknown IDLEKICK setting %d", ChanPtr->Settings[IDLEKICK]);
     }
     sprintf(buffer, "\002Idle for more than %d minutes\002", idlelimit / 60);
     for (User = ChanPtr->users; User; User = User->next)
     {
       if((currenttime - User->idletime) > idlelimit)
       {
         if(!protlevel(get_username(User), ChanPtr) && !userlevel(get_username(User), ChanPtr))
         {
           sendkick(ChanPtr->name, User->nick, buffer);
           User->idletime = User->idletime + 20;
         }
       }
     }
  }
}

int show_users_on_channel(char *from, char *channel)
{
  CHAN_list *Dummy;
  USER_list *Blah;
  char modestr[MAXLEN];

  if ((Dummy = search_chan(channel)) == NULL)
    return (FALSE);

  Blah = Dummy->users;
  while (Blah)
  {
    strcpy(modestr, "");
    if (get_usermode(Blah) & MODE_CHANOP)
      strcat(modestr, "+o");
    if (get_usermode(Blah) & MODE_VOICE)
      strcat(modestr, "+v");
    send_to_user(from, "%s: %30s, %3du %3ds %3dp, mode: %s",
		 channel, get_username(Blah),
		 userlevel(get_username(Blah), Dummy),
		 shitlevel(get_username(Blah), Dummy),
		 protlevel(get_username(Blah), Dummy),
		 modestr);
    Blah = get_nextuser(Blah);
  }
  return (TRUE);
}

/*
 * now some functions to keep track of modes
 */

void add_channelmode(char *channel, unsigned int mode, char *params)
{
}
void del_channelmode(char *channel, unsigned int mode, char *params)
{
}

#ifdef NOTDEF
void add_channelmode(char *channel, unsigned int mode, char *params)
{
  CHAN_list *Channel;

  if ((Channel = search_chan(channel)) == NULL)
    return;

  switch (mode)
  {
  case MODE_CHANOP:
  case MODE_VOICE:
    add_mode(&(Channel->users), mode, params);
    break;
  case MODE_KEY:
    Channel->mode |= MODE_KEY;
    strcpy(Channel->key, params);
    break;
  case MODE_BAN:
    add_ban(&(Channel->banned), params);
    break;
  case MODE_LIMIT:
    Channel->mode |= MODE_LIMIT;
    strcpy(Channel->limit, params);
    break;
  default:
    Channel->mode |= mode;
    break;
  }
}

void del_channelmode(char *channel, unsigned int mode, char *params)
{
  CHAN_list *Channel;

  if ((Channel = search_chan(channel)) == NULL)
    return;

  switch (mode)
  {
  case MODE_CHANOP:
  case MODE_VOICE:
    del_mode(&(Channel->users), mode, params);
    break;
  case MODE_KEY:
    Channel->mode &= ~MODE_KEY;
    break;
  case MODE_BAN:
    delete_ban(&(Channel->banned), params);
    break;
  case MODE_LIMIT:
    Channel->mode &= ~MODE_LIMIT;
    break;
  default:
    Channel->mode &= ~mode;
    break;
  }
}
#endif

void change_usermode(char *channel, char *user, unsigned int mode)
{
  CHAN_list *Channel;

  if ((Channel = search_chan(channel)) == NULL)
    return;			/* should not happen */

  add_mode(&(Channel->users), mode, user);
}

int open_channel(char *channel)
{
  CHAN_list *ChanPtr;

  if ((ChanPtr = search_chan(channel)) == NULL)
    return (FALSE);

  if ((ChanPtr->Modes & CMODEk) && *ChanPtr->CurrentKey)
    sendmode(channel, "-ipslk %s", ChanPtr->CurrentKey);
  else
    sendmode(channel, "-ipsl");
  return (TRUE);
}

/*
 * Here is some mass-stuff. Perhaps it doesn't belong here (though..
 * I think this is a quite good place...).
 */

void channel_massop(char *channel, char *pattern)
{
  CHAN_list *Channel;
  USER_list *Dummy;
  char opstring[MAXLEN];
  int i;

  if ((Channel = search_chan(channel)) == NULL)
    return;

  Dummy = Channel->users;
  while (Dummy)
  {
    strcpy(opstring, "+ooo ");
    i = 0;
    while (Dummy && (i < 3))
    {
      if ((*pattern && (!matches(pattern, get_username(Dummy)) &&
			!(get_usermode(Dummy) & MODE_CHANOP))) ||
	  (!*pattern && userlevel(get_username(Dummy), Channel)))
      {
	strcat(opstring, " ");
	strcat(opstring, get_usernick(Dummy));
	i++;
      }
      Dummy = get_nextuser(Dummy);
    }
    sendmode(channel, "%s", opstring);
  }
}

void channel_massdeop(char *channel, char *pattern)
{
  CHAN_list *Channel;
  USER_list *Dummy;
  char deopstring[MAXLEN];
  int i;

  if ((Channel = search_chan(channel)) == NULL)
    return;

  Dummy = Channel->users;
  while (Dummy)
  {
    strcpy(deopstring, "-ooo ");
    i = 0;
    while (Dummy && (i < 3))
    {
      if ((*pattern && (!matches(pattern, get_username(Dummy)) &&
			(get_usermode(Dummy) & MODE_CHANOP) &&
			(userlevel(get_username(Dummy), Channel) < ChanOp))) ||
	  (!*pattern && (userlevel(get_username(Dummy), Channel) < ChanOp)))
      {
	strcat(deopstring, " ");
	strcat(deopstring, get_usernick(Dummy));
	i++;
      }
      Dummy = get_nextuser(Dummy);
    }
    sendmode(channel, "%s", deopstring);
  }
}

void channel_masskick(char *channel, char *pattern)
{
  CHAN_list *Channel;
  USER_list *Dummy;
  char buffer[MAXLEN];

  sprintf(buffer, "\x2Mass Kick of %s\x2", *pattern ? pattern : "non users");
  if ((Channel = search_chan(channel)) == NULL)
    return;

  Dummy = Channel->users;
  while (Dummy)
  {
    if ((*pattern && (!matches(pattern, get_username(Dummy)) &&
		      (userlevel(get_username(Dummy), Channel) <= ChanOp))) ||
	(!*pattern && !userlevel(get_username(Dummy), Channel)))
      sendkick(channel, get_usernick(Dummy), buffer);

    Dummy = get_nextuser(Dummy);
  }
}

void channel_masskickban(char *channel, char *pattern)
{
  CHAN_list *Channel;
  USER_list *Dummy;
  char buffer[MAXLEN];

  sprintf(buffer, "\x2Mass Kick-Ban of %s\x2", pattern);
  if ((Channel = search_chan(channel)) == NULL)
    return;

  Dummy = Channel->users;
  while (Dummy)
  {
    if (!matches(pattern, get_username(Dummy)) &&
	(userlevel(get_username(Dummy), Channel) <= ChanOp))
    {
      ban_user(channel, get_username(Dummy));
      sendkick(channel, get_usernick(Dummy), buffer);
    }
    Dummy = get_nextuser(Dummy);
  }
}

/*
 * Some misc. function which deal with channels and users
 */

int invite_to_channel(user, channel)
     char *user;
     char *channel;

{
  if (search_chan(channel))
  {
    send_command(700, "INVITE %s %s", user, channel);
    return (TRUE);
  }
  return (FALSE);
}

char *username(nick)
     char *nick;

/*
 * Searches all lists for nick and if it finds it, returns 
 * nick!user@host
 */

{
  CHAN_list *Channel;
  USER_list *User;

  for (Channel = Channel_list; Channel; Channel = Channel->next)
    if((User = search_user(&(Channel->users), nick)))
      return (get_username(User));
  return (NULL);
}

unsigned int usermode(channel, nick)
     char *channel;
     char *nick;

/*
 * returns the usermode of nick on channel
 */
{
  CHAN_list *Channel;
  USER_list *Blah;

  if ((Channel = search_chan(channel)) == NULL)
    return (0);			/* or -1? */

  Blah = Channel->users;
  while (Blah)
  {
    if (!strcasecmp(nick, get_usernick(Blah)))
      return (get_usermode(Blah));
    Blah = get_nextuser(Blah);
  }
  return (0);
}

/*
 * Here are the functions to bookkeep the banlist
 */

BAN_list *search_ban(b_list, banstring)
     BAN_list **b_list;
     char *banstring;

{
  BAN_list *Banned;

  for (Banned = *b_list; Banned; Banned = Banned->next)
    if (!strcasecmp(Banned->banstring, banstring))
      return (Banned);
  return (NULL);
}

void add_ban(BAN_list **b_list, char *banstring)
{
  BAN_list *New_ban;

  if ((New_ban = (BAN_list *) malloc(sizeof(*New_ban))) == NULL)
    return;
  if (search_ban(b_list, banstring) != NULL)
    return;
  strcpy(New_ban->banstring, banstring);
  New_ban->next = *b_list;
  *b_list = New_ban;
}

int delete_ban(BAN_list **b_list, char *banstring)
{
  BAN_list **old;
  BAN_list *Dummy;

  if ((Dummy = search_ban(b_list, banstring)) == NULL)
    return (FALSE);

  for (old = b_list; *old; old = &(*old)->next)
    if (*old == Dummy)
    {
      *old = Dummy->next;
      free(Dummy);
      return (TRUE);
    }
  return (FALSE);
}

void clear_all_bans(b_list)
     BAN_list **b_list;

/*
 * Removes all entries in the list
 */
{
  BAN_list **old;
  BAN_list *tmp;

  for (old = b_list; *old;)
  {
    tmp = (*old)->next;
    free(*old);
    *old = tmp;
  }
}

void channel_massunban(char *channel)
{
  CHAN_list *Channel;
  BAN_list *Dummy;
  char unbanmode[5];
  char unbanstring[MAXLEN];
  int i;

  if ((Channel = search_chan(channel)) == NULL)
    return;
  Dummy = Channel->banned;
  while (Dummy)
  {
    /* We can't just -bbb ban1 ban2,
     * because the third b will make the server
     * show the banlist etc. */
    strcpy(unbanmode, "-");
    strcpy(unbanstring, "");
    i = 0;
    while (Dummy && (i < 3))
    {
      strcat(unbanmode, "b");
      strcat(unbanstring, " ");
      strcat(unbanstring, Dummy->banstring);
      i++;
      Dummy = Dummy->next;
    }
    sendmode(channel, "%s %s", unbanmode, unbanstring);
  }
}

void channel_unban(char *channel, char *user)
{
  CHAN_list *Channel;
  BAN_list *Dummy;
  char unbanmode[5];
  char unbanstring[MAXLEN];
  int i;

  if ((Channel = search_chan(channel)) == NULL)
    return;

  Dummy = Channel->banned;
  while (Dummy)
  {
    strcpy(unbanmode, "-");
    strcpy(unbanstring, "");
    i = 0;
    while (Dummy && (i < 3))
    {
      if (!matches(Dummy->banstring, user))
      {
	strcat(unbanmode, "b");
	strcat(unbanstring, " ");
	strcat(unbanstring, Dummy->banstring);
	i++;
      }
      Dummy = Dummy->next;
    }
    if(*unbanstring)
      sendmode(channel, "%s %s", unbanmode, unbanstring);
  }
}
