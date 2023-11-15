#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include "userlist.h"
#include "config.h"
#include "misc.h"
#include "function.h"
#include "channel.h"
#include "match.h"
#include "send.h"

extern UserStruct *UserList;
extern ShitStruct *ShitList;
extern FloodStruct *FloodList;

extern char lastcmd[1024];
extern char current_nick[MAXNICKLEN];
extern UserHostStruct *UserHostList;
char schan[MAXLEN] = "";
extern char bot_dir[FNLEN];

time_t getthetime(void)
{
  time_t t;

  t = time((time_t *) NULL);
  return t;
}

int getseconds(time_t seconds)
{
  time_t t;

  t = getthetime();
  return (t - seconds);
}

UserHostStruct *init_userhostlist(void)
{
  UserHostStruct *l_list;

  l_list = (UserHostStruct *) malloc(sizeof(l_list));
  l_list = NULL;
  return (l_list);
}

UserHostStruct *find_userhostfromnick(UserHostStruct * begin, char *nick)
{
  UserHostStruct *Temp;

  if (!nick)
    return NULL;

  for (Temp = begin; Temp; Temp = Temp->next)
    if (!strcasecmp(getnick(Temp->name), nick))
      return Temp;
  return NULL;
}

UserHostStruct *find_userhost(char *fullname)
{
  UserHostStruct *Temp;

  char buffer[255];

  if (!fullname)
    return NULL;
  if (!strchr(fullname, '*'))
  {
    char n[100], u[100], h[100], m[100];

    sep_userhost(fullname, n, u, h, m);
    sprintf(buffer, "%s!%s@*%s", n, u, h);
  }
  else
    strcpy(buffer, fullname);

  for (Temp = UserHostList; Temp; Temp = Temp->next)
    if (!matches(buffer, Temp->name))
      return Temp;
  return NULL;
}

int num_userhost(char *nick)
{
  int num = 0;
  UserHostStruct *Temp;

  Temp = UserHostList;
  while (Temp != NULL)
  {
    Temp = find_userhostfromnick(Temp, nick);
    if (Temp)
    {
      num++;
      Temp = Temp->next;
    }
  }
  return num;
}

char *getuserhost_fromlist(char *nick)
{
  int num;
  UserHostStruct *Temp;
  char *ptr;

  num = num_userhost(nick);
  if (num == 1)
  {
    Temp = find_userhostfromnick(UserHostList, nick);
    ptr = strchr(Temp->name, '!');
    ptr++;
    if (ptr)
      return ptr;
  }
  return (getuserhost(nick));
}

char *getlastchan(char *fullname)
{
  UserHostStruct *Temp;

  Temp = find_userhost(fullname);
  if (Temp)
    return Temp->channel;
  return NULL;
}

time_t getlasttime(char *fullname)
{
  UserHostStruct *Temp;

  Temp = find_userhost(fullname);
  if (Temp)
    return Temp->time;
  return 0;
}

int remove_userhost(char *thename)
{
  UserHostStruct *old;
  UserHostStruct *dummy;

  if ((dummy = find_userhost(thename)) == NULL)
    return (FALSE);
  if (dummy == UserHostList)
  {
    UserHostList = dummy->next;
    free(dummy);
    return TRUE;
  }
  for (old = UserHostList; old; old = old->next)
    if (old->next == dummy)
    {
      old->next = dummy->next;
      free(dummy);
      return (TRUE);
    }
  return (FALSE);
}

int update_userhostlist(char *thename2, char *thechannel, time_t thetime)
{
  char n[100], m[100], u[100], h[100];
  char buffer[255];
  char buf2[255], *thename = buf2;
  UserHostStruct *New_name;

  strcpy(thename, thename2);
  sep_userhost(thename, n, u, h, m);
  strcpy(u, strip(u, '#'));
  strcpy(u, strip(u, '~'));
  sprintf(buffer, "%s!*%s@*%s", n, u, h);
  strcat(lastcmd, "uuok1");
  New_name = find_userhost(buffer);
  strcat(lastcmd, "uuok2");
  if (!New_name)
  {

    if ((New_name = (UserHostStruct *) malloc(sizeof(*New_name))) == NULL)
    {
      strcat(lastcmd, "MEM");
      return FALSE;
    }

    strcpy(New_name->name, thename);
    New_name->next = UserHostList;
    New_name->kicktime = thetime;
    New_name->killtime = 0;
    New_name->bantime = thetime;
    New_name->deoptime = thetime;
    New_name->bannum = 0;
    New_name->deopnum = 0;
    New_name->kicknum = 0;
    New_name->totkick = 0;
    New_name->totkill = 0;
    New_name->totop = 0;
    New_name->totban = 0;
    New_name->totdeop = 0;
    New_name->totmsg = 0;
    New_name->msgnum = 0;
    New_name->messages = init_messages();
    UserHostList = New_name;
    strcpy(New_name->channel, "<UNKNOWN>");
  }
  strcat(lastcmd, "uuok3");
  if (*thechannel)
    strcpy(New_name->channel, thechannel);
  New_name->time = thetime;
  write_userhost("");
  return TRUE;
}

int remove_all_from_userhostlist(void)
{
  UserHostStruct *dummy;
  UserHostStruct *tmp;

  if (dummy) ;
  for (dummy = UserHostList; dummy; /* dummy = dummy->next */ )
  {
    tmp = dummy->next;
    if (dummy)
      free(dummy);
    dummy = tmp;
  }
  UserHostList = init_userhostlist();
  return(TRUE);
}

extern char writeruhost[255];
extern char owneruhost[255];

ListStruct *find_list(ListStruct * WhichList, char *thename)
{
  ListStruct *Temp;

  if (!thename)
    return NULL;

  for (Temp = WhichList; Temp; Temp = Temp->next)
    if (!matches(Temp->name, thename))
      return Temp;
  return NULL;
}

int is_in_list(ListStruct * WhichList, char *thename)
{
  return (find_list(WhichList, thename) != NULL);
}

ListStruct *init_list(void)
{
  ListStruct *l_list;

  l_list = (ListStruct *) malloc(sizeof(l_list));
  l_list = NULL;
  return (l_list);
}

int add_to_list(ListStruct ** WhichList, char *thename)
{
  ListStruct *New_name;

  if ((New_name = find_list(*WhichList, thename)) != NULL)
    return FALSE;
  if ((New_name = (ListStruct *) malloc(sizeof(*New_name))) == NULL)
    return FALSE;

  strcpy(New_name->name, thename);
  New_name->next = *WhichList;
  *WhichList = New_name;
  return TRUE;
}

int remove_from_list(ListStruct ** WhichList, char *thename)
{
  ListStruct *old;
  ListStruct *dummy;

  if ((dummy = find_list(*WhichList, thename)) == NULL)
    return (FALSE);
  if (dummy == *WhichList)
  {
    *WhichList = dummy->next;
    free(dummy);
    return TRUE;
  }
  for (old = *WhichList; old; old = old->next)
    if (old->next == dummy)
    {
      old->next = dummy->next;
      free(dummy);
      return (TRUE);
    }
  return (FALSE);
}

int remove_all_from_list(ListStruct ** WhichList)
{
  ListStruct *dummy;
  ListStruct *tmp;

  if (dummy) ;
  for (dummy = *WhichList; dummy; /* dummy = dummy->next */ )
  {
    tmp = dummy->next;
    if (dummy)
      free(dummy);
    dummy = tmp;
  }
  *WhichList = init_list();
  return(TRUE);
}

MessagesStruct *init_messages(void)
{
  MessagesStruct *l_list;

  l_list = (MessagesStruct *) malloc(sizeof(l_list));
  l_list = NULL;
  return (l_list);
}

int add_message(MessagesStruct ** mess, char *from, char *message, int num)
{
  MessagesStruct *newmsg;

  if ((newmsg = (MessagesStruct *) malloc(sizeof(*newmsg))) == NULL)
    return FALSE;
  strcpy(newmsg->from, from);
  strcpy(newmsg->text, message);
  newmsg->time = getthetime();
  newmsg->num = num;
  newmsg->next = *mess;
  *mess = newmsg;
  return TRUE;
}

MessagesStruct *get_message(MessagesStruct * mess, int num)
{
  MessagesStruct *temp;

  for (temp = mess; temp; temp = temp->next)
    if (temp->num == num)
      return temp;
  return NULL;
}

FloodStruct *init_flood(void)
{
  FloodStruct *l_list;

  l_list = (FloodStruct *) malloc(sizeof(l_list));
  l_list = NULL;
  return l_list;
}

FloodStruct *find_flood(char *hostname)
{
  FloodStruct *temp;

  if (!hostname)
    return NULL;
  for (temp = FloodList; temp; temp = temp->next)
    if (!strcasecmp(temp->host, hostname))
      return temp;
  return NULL;
}

int add_flood(char *userhost)
{
  FloodStruct *New_user;
  char *hostname, *temp;

  temp = cluster(userhost);
  hostname = gethost(temp);
  if ((New_user = find_flood(hostname)) != NULL)
    return FALSE;
  if ((New_user = (FloodStruct *) malloc(sizeof(*New_user))) == NULL)
    return FALSE;
  strcpy(New_user->host, hostname);
  New_user->num = 0;
  New_user->thetime = getthetime();
  New_user->next = FloodList;
  FloodList = New_user;
  return TRUE;
}

int is_flooding(char *userhost)
{
  FloodStruct *dummy;
  char *hostname, *temp;
  time_t currenttime;

  if (!strchr(userhost, '@'))
    return 0;

  if (!strcasecmp(getnick(userhost), current_nick))
    return 0;
  temp = cluster(userhost);
  hostname = gethost(temp);

  if (!(dummy = find_flood(hostname)))
  {
    add_flood(userhost);
    if (!(dummy = find_flood(hostname)))
      return 0;
  }

  currenttime = getthetime();
  if ((currenttime - dummy->thetime) > 30)
  {
    dummy->num = 0;
    dummy->thetime = currenttime;
  }
  dummy->num++;
  if (dummy->num >= 15)
  {
    if (dummy->num == 20)
    {
      if(userlevel(userhost, Globals) < BotCoOwner)
      {
         send_to_user(userhost, "\002You are being ignored for a while...\002");
         dummy->thetime = currenttime;
      }
      else
      {
         send_to_user(userhost, "\002Easy there, dont flood me.\002");
      }
    }
    return TRUE;
  }
  return FALSE;
}

int remove_floodstruct(FloodStruct * dummy)
{
  FloodStruct *old;

  if (!dummy)
    return FALSE;
  if (dummy == FloodList)
  {
    FloodList = dummy->next;
    free(dummy);
    return TRUE;
  }
  for (old = FloodList; old; old = old->next)
    if (old->next == dummy)
    {
      old->next = dummy->next;
      free(dummy);
      return TRUE;
    }
  return FALSE;
}

void remove_oldfloods(void)
{
  FloodStruct *temp, *temp2;

  while (FloodList && ((getthetime() - FloodList->thetime) > 30))
  {
    temp = FloodList;
    FloodList = FloodList->next;
    free(temp);
  }
  temp = FloodList;
  while (temp && temp->next)
  {
    if ((getthetime() - (temp->next)->thetime) > 30)
    {
      temp2 = temp->next;
      temp->next = (temp->next)->next;
      free(temp2);
    }
    else
      temp = temp->next;
  }
  return;
}

UserStruct *init_userlist(void)
{
  UserStruct *l_list;

  l_list = (UserStruct *) malloc(sizeof(l_list));
  l_list = NULL;
  return (l_list);
}

UserStruct *FindOpByHandle(CHAN_list *ChanPtr, char *handle)
{
  UserStruct *UserPtr;

  if(!ChanPtr)
    return(NULL);
  if(!ChanPtr->OpList)
    return(NULL);
  for(UserPtr=ChanPtr->OpList; UserPtr; UserPtr=UserPtr->next)
  {
     if(!strcasecmp(UserPtr->handle, handle))
        return(UserPtr);
  }
  return(NULL);
}

UserStruct *find_user(char *userhost, CHAN_list * ChanPtr)
{
  UserStruct *User;
  HostListStruct *Host;
  UserStruct *ret;

  ret = NULL;
  if (!userhost)
    return NULL;

  if (!ChanPtr)
    ret = NULL;
  else if (!(ChanPtr->OpList))
    ret = NULL;		/* No users for this channel */
  else
  {
    for(User = ChanPtr->OpList; User; User = User->next)
      for(Host = User->userhosts;Host;Host = Host->next)
        if (!matches(Host->userhost, userhost))
          ret = User;
  }
  return(ret);
}

char *gethandle(char *userhost, CHAN_list * ChanPtr)
{
  UserStruct *User;
  HostListStruct *Host;
  char *ret;

  ret = NULL;
  if (!userhost)
    return NULL;

  if (!ChanPtr)
    ret = NULL;
  else if (!(ChanPtr->OpList))
    ret = NULL;         /* No users for this channel */
  else
  {
    for(User = ChanPtr->OpList; User; User = User->next)
      for(Host = User->userhosts;Host;Host = Host->next)
        if(!matches(Host->userhost, userhost))
          ret = User->handle;
    for(User = Globals->OpList; User; User = User->next)
      for(Host = User->userhosts;Host;Host = Host->next)
        if(!matches(Host->userhost, userhost))
          ret = User->handle;

  }
  return(ret);
}

int isuser(char *userhost, CHAN_list * ChanPtr)
{
  return (find_user(userhost, ChanPtr) != NULL);
}

void DelChanListUser(CHAN_list *ChanPtr, UserStruct *OpTarget)
{
   UserStruct *Last;
   UserStruct *OpPtr;
   HostListStruct *HostPtr, *TmpHost;

   Last = OpPtr;
   for(OpPtr=ChanPtr->OpList;OpPtr;OpPtr=OpPtr->next)
   {
     if(OpPtr == OpTarget)
     {
        if(OpPtr == ChanPtr->OpList)
          ChanPtr->OpList = OpPtr->next;
        else
          Last->next = OpPtr->next;
        for(HostPtr = OpPtr->userhosts;HostPtr;HostPtr = TmpHost)
        {
           TmpHost = HostPtr->next;
           free(HostPtr);
        }
        free(OpPtr);
        return;
     }
     Last = OpPtr;
   }
   Debug(DBGERROR, "Trying to delete a user but couldnt find him!\002");
}

int AddChanListAdditionalHost(CHAN_list *ChanPtr, UserStruct *UserPtr, char *userhost, int security)
{
   HostListStruct *new_host;

   if((find_user(userhost, ChanPtr)) != NULL)
   {
      Debug(DBGINFO, "Host %s already in list, not adding.\n", userhost);
      return(FALSE);
   }
   if((new_host = (HostListStruct *) malloc(sizeof(*new_host))) == NULL)
   {
      Debug(DBGINFO, "Out of memory, not adding %s\n", userhost);
      return(FALSE);
   }
   strcpy(new_host->userhost, userhost);
   new_host->security = security;
   new_host->next = UserPtr->userhosts;
   UserPtr->userhosts = new_host;
   return(TRUE);
}

UserStruct *AddChanListUser(CHAN_list * ChanPtr,
		    char *userhost,
		    char *handle,
		    char *password,
		    int level,
		    int prot,
		    int aop,
                    int security,
		    char *info)
{
  UserStruct *New_user;

  if ((New_user = find_user(userhost, ChanPtr)) != NULL)
  {
    Debug(DBGINFO, "User %s already in list, not adding.\n", userhost);
    return FALSE;		/* already have that user */
  }
  if ((New_user = (UserStruct *) malloc(sizeof(*New_user))) == NULL)
  {
    Debug(DBGERROR, "Out of memory. Not adding %s\n", userhost);
    return FALSE;		/* Out of memory */
  }
  New_user->userhosts=NULL;
  AddChanListAdditionalHost(ChanPtr, New_user, userhost, security);
  strcpy(New_user->handle, handle);
  strcpy(New_user->password, password);
  strcpy(New_user->info, info);
  New_user->level = level;
  New_user->isaop = aop;
  New_user->security = security;
  New_user->protlevel = prot;
  New_user->next = ChanPtr->OpList;
  ChanPtr->OpList = New_user;
/*
//  Debug(DBGINFO, "Added user %s to %s userlist.\n", New_user->userhosts->userhost, ChanPtr->name);
*/
  return(New_user);
}

int remove_from_userlist(char *userhost, CHAN_list * ChanPtr)
{
  UserStruct *old;
  UserStruct *dummy;

  if ((dummy = find_user(userhost, ChanPtr)) == NULL)
    return (FALSE);
  if (dummy == ChanPtr->OpList)
  {
    ChanPtr->OpList = dummy->next;
    free(dummy);
    return TRUE;
  }
  for (old = ChanPtr->OpList; old; old = old->next)
    if (old->next == dummy)
    {
      old->next = dummy->next;
      free(dummy);
      return (TRUE);
    }
  return (FALSE);
}

ShitStruct *init_shitlist(void)
{
  ShitStruct *l_list;

  l_list = (ShitStruct *) malloc(sizeof(l_list));
  l_list = NULL;
  return (l_list);
}

ShitStruct *find_shit(char *userhost, CHAN_list * ChanPtr)
{
  ShitStruct *User;

  if (!userhost)
    return NULL;

  if (!ChanPtr)
  {
    return NULL;		/* No channel */
  }
  if (!(ChanPtr->ShitList))
    return NULL;		/* No shits for this channel */
  for (User = ChanPtr->ShitList; User; User = User->next)
    if (!matches(User->userhost, userhost))
      return (User);
  return (NULL);
}

ShitStruct *find_shitbm(char *userhost)
{
  ShitStruct *User;

  if (!userhost)
    return NULL;
  for (User = ShitList; User; User = User->next)
    if (!bancmp(User->userhost, userhost))
      return (User);
  return (NULL);
}

void AddChanListBan(CHAN_list * ChanPtr, char *hostmask, int level, char *bywho,
		    time_t timestamp, time_t expiretime, char *reason)
{
  ShitStruct *ShitPtr;

  if (find_shit(hostmask, ChanPtr))
    return;			/* Already in the list */
  if((ShitPtr = (ShitStruct *) malloc(sizeof(*ShitPtr))))
  {
    strcpy(ShitPtr->userhost, hostmask);
    strcpy(ShitPtr->bannedby, bywho);
    strcpy(ShitPtr->reason, reason);
    ShitPtr->level = level;
    ShitPtr->timestamp = timestamp;
    ShitPtr->expiretime = expiretime;
    ShitPtr->next = ChanPtr->ShitList;
    ChanPtr->ShitList = ShitPtr;
  }
  else
  {
    printf("Out of memory!\n");
    exit(1);
  }
}

int add_to_userlist(char *userhost, int level, int aop, int prot)
{
  return (TRUE);
}
int add_to_shitlist(char *userhost, int level, char *channels)
{
  return (TRUE);
}
#ifdef NOTDEF
int add_to_shitlist(char *userhost, int level, char *channels)
{
  ShitStruct *New_user;
  char *channame;
  char buffer[MAXLEN] = "\0";

  if ((New_user = find_shit(userhost)) != NULL)
    return FALSE;
  if ((New_user = (ShitStruct *) malloc(sizeof(*New_user))) == NULL)
    return FALSE;

  strcpy(New_user->userhost, userhost);
  New_user->level = level;
  New_user->next = ShitList;
  New_user->channels = init_list();
  if (channels)
    channame = strtok(channels, SEPERATORS);
  if (channels && channame)
    do
    {
      if (!ischannel(channame) && strcmp(channame, "*"))
      {
	strcpy(buffer, "#");
	strcat(buffer, channame);
	add_to_list(&(New_user->channels), buffer);
      }
      else
      {
	if (strcmp(channame, "*") == 0)
	  add_to_list(&(New_user->channels), "");
	else
	  add_to_list(&(New_user->channels), channame);
      }
    }
    while (channame = strtok(NULL, SEPERATORS));
  ShitList = New_user;
  return TRUE;
}
#endif

int remove_from_shitlist(char *userhost, CHAN_list * ChanPtr)
{
  ShitStruct *old;
  ShitStruct *dummy;

  if ((dummy = find_shit(userhost, ChanPtr)) == NULL)
    return (FALSE);
  if (dummy == ChanPtr->ShitList)
  {
    ChanPtr->ShitList = dummy->next;
    free(dummy);
    return TRUE;
  }
  for (old = ChanPtr->ShitList; old; old = old->next)
    if (old->next == dummy)
    {
      old->next = dummy->next;
      free(dummy);
      return (TRUE);
    }
  return (FALSE);
}

int change_userlist(char *userhost, int level, int aop, int prot, CHAN_list * ChanPtr)
{
  UserStruct *dummy;

  if ((dummy = find_user(userhost, ChanPtr)) != NULL)
  {
    if (level != -1)
      dummy->level = level;
    if (aop != -1)
      dummy->isaop = aop;
    if (prot != -1)
      dummy->protlevel = prot;
    return TRUE;
  }
  return FALSE;
}

void DelAuth(char *rest)
{
   AuthStruct AuthPtr, PrevPtr = NULL;
   extern AuthStruct AuthList;
   
   for(AuthPtr = AuthList;AuthPtr;AuthPtr=AuthPtr->next)
   {
       if(!strcasecmp(getnick(AuthPtr->userhost), rest))
       {
           if(PrevPtr)
              PrevPtr->next = AuthPtr->next;
           else
              AuthList = AuthPtr->next;
           free(AuthPtr); 
           return;
       }
       PrevPtr = AuthPtr;
   }
}

int isauth(char *userhost, CHAN_list *ChanPtr)
{
   UserStruct *UsrPtr;
   AuthStruct AuthPtr;
   HostListStruct *HostPtr;
   extern AuthStruct AuthList;

   UsrPtr = find_user(userhost, ChanPtr);
   if(!UsrPtr)
       UsrPtr = find_user(userhost, Globals);
   if(!UsrPtr)
   {
       Debug(DBGINFO, "isauth cant find the users account.");
       return(0);
   }
   for(HostPtr = UsrPtr->userhosts;HostPtr;HostPtr = HostPtr->next)
   {
       if(!matches(HostPtr->userhost, userhost))
       {
          if(HostPtr->security == SECURE)
             return(1); /* Does not need authorization. */
          else
             break;
       }
   }
   if(!HostPtr)
   {
       Debug(DBGERROR, "Couldnt find matching host yet they suposidly have an account.. (isauth())");
       return(0);
   }
   for(AuthPtr = AuthList;AuthPtr;AuthPtr=AuthPtr->next)
   {
       if(!strcasecmp(AuthPtr->userhost, userhost))
       {
           return(1);  /* User is in authorized list */
       }
   }
   return(0); /* Couldnt find them.. */
}

int userlevel(char *userhost, CHAN_list * ChanPtr)
{
  UserStruct *dummy;

  if (!userhost)
    return 0;
  if (!matches(owneruhost, userhost))
    return 100;
  if ((dummy = find_user(userhost, Globals)) != NULL)
    return (dummy->level);
  if ((dummy = find_user(userhost, ChanPtr)) != NULL)
    return (dummy->level);
  return 0;
}

int isowner(char *userhost)
{
  if(!userhost)
    return(FALSE);
  if(!matches(owneruhost, userhost))
    return(TRUE);
  return(FALSE);
}

int isaop(char *userhost, CHAN_list * ChanPtr)
{
  UserStruct *dummy;

  if((dummy = find_user(userhost, ChanPtr)) != NULL)
    return (dummy->isaop);
  if((dummy = find_user(userhost, Globals)) != NULL)
    return(dummy->isaop);
  return 0;
}

int protlevel(char *userhost, CHAN_list * ChanPtr)
{
  UserStruct *dummy;

  if (!ChanPtr->Settings[PROTECTION])
    return 0;
  if ((dummy = find_user(userhost, ChanPtr)) != NULL)
    return (dummy->protlevel);
  return 0;
}

int protlevelbm(char *userhost, CHAN_list *ChanPtr)
{
  UserStruct *User;
  HostListStruct *Host;

  if(!ChanPtr)
    return(0);
  if(!ChanPtr->Settings[PROTECTION])
    return(0);
  for(User = ChanPtr->OpList;User;User = User->next)
  {
    for(Host = User->userhosts;Host;Host = Host->next)
    {
      if(!(bancmp(Host->userhost, userhost)))
        return(User->protlevel);
    }
  }
  for(User = Globals->OpList; User; User = User->next)
  {
    for(Host = User->userhosts;Host;Host = Host->next)
    {
      if(!bancmp(Host->userhost, userhost))
        return User->protlevel;
    }
  }
  Debug(DBGINFO, "Blah test %s", ChanPtr->name);
  return 0;
}

char *shitchan(char *userhost)
{
  return ("");
}
#ifdef NOTDEF
char *shitchan(char *userhost)
{
  ShitStruct *dummy;
  ListStruct *temp;

/* canot return local var!!  char schan[MAXLEN]=""; */

  if ((dummy = find_shit(userhost)) != NULL)
  {
    for (temp = dummy->channels; temp; temp = temp->next)
    {
      strcat(schan, temp->name);
      strcat(schan, " ");
      if (!strcmp(temp->name, "*"))
	return temp->name;
    }
    return schan;
  }
  return "";
}
#endif

int shitlevel(char *userhost, CHAN_list * ChanPtr)
{
  ShitStruct *dummy;

  if ((dummy = find_shit(userhost, ChanPtr)) != NULL)
    return (dummy->level);
  return 0;
}

int shitlevelbm(char *userhost, CHAN_list * ChanPtr)
{
  ShitStruct *User;

  if (!ChanPtr->Settings[SHITPROT])
    return 0;
  for (User = ChanPtr->ShitList; User; User = User->next)
    if (!bancmp(User->userhost, userhost))
      return User->level;
  for (User = Globals->ShitList; User; User = User->next)
    if (!bancmp(User->userhost, userhost))
      return User->level;
  return 0;
}

void show_shitlist(char *from, CHAN_list * ChanPtr, char *userhost)
{
  ShitStruct *dummy;
  char *temp;

  for (dummy = ChanPtr->ShitList; dummy; dummy = dummy->next)
    if (!matches(dummy->userhost, userhost) || !*userhost)
      if (dummy->level)
      {
	temp = shitchan(dummy->userhost);
	send_to_user(from, " %40s | %3d  | %s",
		     dummy->userhost, dummy->level, temp);
      }
}

void show_userlist(char *from, CHAN_list * ChanPtr, char *userhost)
{
  UserStruct *dummy;

  for (dummy = ChanPtr->OpList; dummy; dummy = dummy->next)
    if (!matches(dummy->userhosts->userhost, userhost) || !*userhost)
      if (dummy->level)
	send_to_user(from, " %40s | %d", dummy->userhosts->userhost, dummy->level);
}

void show_protlist(char *from, CHAN_list * ChanPtr, char *userhost)
{
  UserStruct *dummy;

  for (dummy = ChanPtr->OpList; dummy; dummy = dummy->next)
    if (!matches(dummy->userhosts->userhost, userhost) || !*userhost)
      if (dummy->protlevel)
	send_to_user(from, " %40s | %d",
		     dummy->userhosts->userhost, dummy->protlevel);
}


int read_userhost(char *InstanceName)
{
  UserHostStruct *dummy;
  FILE *in;
  char filename[MAXLEN];

  sprintf(filename, "%s/%s/%s", bot_dir, InstanceName, USERHOSTFILE);
  if ((in = fopen(filename, "r")) == NULL)
  {
    printf("Error reading %s\n", filename);
    return (FALSE);
  }
  while (!feof(in))
  {
    if ((dummy = (UserHostStruct *) malloc(sizeof(*dummy))) == NULL)
    {
      fclose(in);
      return (TRUE);
    }
    fscanf(in, "%s %s %li %li %li %li %li %i %i %i %i %i %i %i %i %i %i\n",
	   dummy->name, dummy->channel, &dummy->time, &dummy->kicktime,
	   &dummy->bantime, &dummy->deoptime, &dummy->killtime, &dummy->bannum, &dummy->deopnum,
	   &dummy->kicknum, &dummy->totmsg, &dummy->msgnum, &dummy->totop,
	   &dummy->totban, &dummy->totdeop, &dummy->totkick, &dummy->totkill);
    dummy->messages = init_messages();
    dummy->next = UserHostList;
    UserHostList = dummy;
  }
  fclose(in);
  return (TRUE);
}

int write_userhost(char *unused)
{
  UserHostStruct *dummy;
  FILE *out;
  char filename[MAXLEN];
  extern char instancename[MAXLEN];

  sprintf(filename, "%s/%s/%s", bot_dir, instancename, USERHOSTFILE);
  if ((out = fopen(filename, "w")) == NULL)
    return (FALSE);

  for (dummy = UserHostList; dummy; dummy = dummy->next)
    fprintf(out, "%s %s %li %li %li %li %li %i %i %i %i %i %i %i %i %i %i\n",
	    dummy->name, dummy->channel, dummy->time, dummy->kicktime,
	    dummy->bantime, dummy->deoptime, dummy->killtime, dummy->bannum, dummy->deopnum,
	    dummy->kicknum, dummy->totmsg, dummy->msgnum, dummy->totop,
	    dummy->totban, dummy->totdeop, dummy->totkick, dummy->totkill);
  fclose(out);
  return (TRUE);
}

#ifdef NOTDEF
int write_userlist(char *filename)
{
  UserStruct *dummy;
  time_t Tme;
  FILE *list_file;

  if ((list_file = fopen(filename, "w")) == NULL)
    return (FALSE);

  Tme = time((time_t *) NULL);

  fprintf(list_file, "#############################################\n");
  fprintf(list_file, "## %s -- Userlist\n", filename);
  fprintf(list_file, "## Created: %s", ctime(&Tme));
  fprintf(list_file, "#############################################\n");

  for (dummy = UserList; dummy; dummy = dummy->next)
    fprintf(list_file, " %40s %3d %1d %1d\n",
	    dummy->userhost, dummy->level, dummy->isaop, dummy->protlevel);
  fprintf(list_file, "# End of %s\n", filename);
  fclose(list_file);
  return (TRUE);
}

int write_shitlist(char *filename)
{
  return (TRUE);
}
int write_shitlist(char *filename)
{
  ShitStruct *dummy;
  time_t Tme;
  FILE *list_file;
  ListStruct *channel;

  if ((list_file = fopen(filename, "w")) == NULL)
    return (FALSE);

  Tme = time((time_t *) NULL);

  fprintf(list_file, "#############################################\n");
  fprintf(list_file, "## %s -- Shitlist\n", filename);
  fprintf(list_file, "## Created: %s", ctime(&Tme));
  fprintf(list_file, "#############################################\n");

  for (dummy = ShitList; dummy; dummy = dummy->next)
  {
    fprintf(list_file, " %40s %3d ", dummy->userhost, dummy->level);
    for (channel = dummy->channels; channel; channel = channel->next)
      if (channel->next)
	fprintf(list_file, "%s ", channel->name);
      else
	fprintf(list_file, "%s", channel->name);
    fprintf(list_file, "\n");
  }
  fprintf(list_file, "# End of %s\n", filename);
  fclose(list_file);
  return (TRUE);
}
#endif

int readln_from_a_file(FILE * stream, char *lin)
{
  char *p;

  do
    p = fgets(lin, MAXLEN, stream);
  while ((p != NULL) && (*lin == '#'));

  if (p == NULL)
    return (FALSE);
  if (strchr(lin, '\n'))
    *strchr(lin, '\n') = '\0';
  if (strchr(lin, '\r'))
    *strchr(lin, '\r') = '\0';
  return (TRUE);
}

int read_shitlist(char *filename)
{
  return (TRUE);
}
#ifdef NOTDEF
int read_shitlist(char *filename)
{
  FILE *fp;
  char lin[MAXLEN];
  char usrhost[MAXLEN];
  int level;
  char *channels;

  ShitStruct *dummy;


  if ((fp = fopen(filename, "r")) == NULL)
    return FALSE;

  for (dummy = ShitList; dummy; dummy = dummy->next)
  {
    remove_all_from_list(&(dummy->channels));
    free(dummy);
  }
  ShitList = init_shitlist();

  while (readln_from_a_file(fp, lin))
  {
    sscanf(lin, "%s %d\n", usrhost, &level);
    channels = strstr(lin, usrhost);
    if (channels)
    {
      channels = strtok(channels, " ");
      channels = strtok(NULL, " ");
      channels = strtok(NULL, " ");
    }
    if (channels)
      add_to_shitlist(usrhost, level, channels);
    else
      add_to_shitlist(usrhost, level, "*");
  }
  fclose(fp);
  return (TRUE);
}
#endif

int read_userlist(char *filename)
{
  FILE *fp;
  char lin[MAXLEN];
  char usrhost[MAXLEN];
  int level, aop, prot;
  UserStruct *dummy;


  if ((fp = fopen(filename, "r")) == NULL)
    return FALSE;

  for (dummy = UserList; dummy; dummy = dummy->next)
    free(dummy);
  UserList = init_userlist();

  while (readln_from_a_file(fp, lin))
  {
    sscanf(lin, "%s %d %d %d\n", usrhost, &level, &aop, &prot);
    add_to_userlist(usrhost, level, aop, prot);
  }
  fclose(fp);
  return (TRUE);
}
