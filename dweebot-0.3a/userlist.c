/*
 * userlist.c - implementation of userlists
 */

#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include "debug.h"
#include "config.h"
#include "fnmatch.h"
#include "function.h"
#include "dweeb.h"
#include "userlist.h"

extern botinfo *current;
extern char owneruserhost[MAXLEN];
static char tempbuffer[MAXLEN];

int verified(userhost)
char *userhost;
{
  return (get_mempassword(userhost));
}

int change_password(l_list, userhost, password)
ULS **l_list;
char *userhost;
char *password;
{
	ULS *user;
	int change;

	change = FALSE;
	if (!userhost)
	  return FALSE;

	for (user = *l_list; user; user = user->next)
	  if (!matches(user->userhost, userhost))
		 {
			change = TRUE;
			if (user->password)
		free(user->password);
			mstrcpy(&(user->password), password);
		 }
	return change;
}

ULS *is_in_userlist(l_list, userhost, channel)
ULS **l_list;
char *userhost;
char *channel;
{
	ULS *user;
	
	if (!userhost)
	  return NULL;

	for (user = *l_list; user; user = user->next)
	  if (!matches(user->userhost, userhost) &&
			(!my_stricmp(channel, user->channel) ||
			 (*user->channel == '*') || (*channel=='*')))
		 return user;
	return NULL;
}

int is_user(userhost, channel)
char *userhost;
char *channel;
{
  if (!my_stricmp(current->nick, getnick(userhost)))
	 return TRUE;
	return (is_in_userlist(current->UserList, userhost, channel) != NULL);
}

ULS **init_userlist(void)
{
	ULS **l_list;

	l_list=(ULS **) malloc (sizeof(*l_list));
	*l_list=NULL;
	return(l_list);
}

int add_to_userlist(l_list, uh, level, aop, prot, chan, password)
ULS **l_list;
char *uh;
int level;
int aop;
int prot;
char *chan;
char *password;
{
	ULS *New_user;
	
	if (!chan)
	  return FALSE;
	if (*chan == '*')
	  remove_from_userlist(l_list, uh, "*");
	if (New_user = is_in_userlist(l_list, uh, chan))
	  return FALSE;
	if (!(New_user = (ULS *) malloc (sizeof(*New_user))))
	  return FALSE;
	mstrcpy(&New_user->userhost, uh);
	New_user->access = level;
	New_user->aop    = aop ? 1 : 0;
	New_user->prot   = (prot > MAXPROTLEVEL) ? 5 : (prot < 0) ? 0 : prot;
	mstrcpy(&New_user->channel, chan);
	if (password && *password)
		mstrcpy(&New_user->password, password);
	else
		mstrcpy(&New_user->password, "");
	New_user->next = *l_list;
	*l_list = New_user;
	return TRUE;
}

void show_userlist(from, l_list)
char *from;
ULS **l_list;
{
  ULS *dummy;
  send_to_user(from, "\002%s\002", "------------- Userhost ------------- Channel -- Acc AO Prt --");
  for (dummy = *l_list; dummy; dummy = dummy->next )
	 send_to_user( from, " %30s %15s  %3d %d %d", dummy->userhost, dummy->channel, dummy->access, dummy->aop, dummy->prot );
  send_to_user(from, "\002%s\002", "--- end of userlist ---");
}
  

int remove_from_userlist(l_list, userhost, chan)
ULS **l_list;
char *userhost;
char *chan;
{
	ULS **old;
	ULS *dummy;

	if (!(dummy = is_in_userlist(l_list, userhost, chan)))
		return FALSE;
	while (dummy = is_in_userlist(l_list, userhost, chan))
	{
	  for (old = l_list; *old; old = &(*old)->next)
		 if (*old == dummy)
			{
		*old = dummy->next;
		free(dummy->userhost);
		free(dummy->channel);
		free(dummy->password);
		free(dummy);
		return TRUE;
			}
	}
	return TRUE;
}

void delete_userlist(l_list )
ULS **l_list;
{
	ULS *dummy;
	ULS *next;

	dummy = *l_list;
	while (dummy)
	{
		next = dummy->next;
		remove_from_userlist(l_list, dummy->userhost, dummy->channel);
		dummy = next;
	}
}

int max_userlevel(userhost)
char *userhost;
{
	ULS *user;
	int access;

	if (!matches(owneruserhost, userhost))
		return OWNERLEVEL;
	if (!my_stricmp(current->nick, getnick(userhost)))
	  return OWNERLEVEL;
	access = 0;
	for (user = *current->UserList; user; user = user->next)
	  if (!matches(user->userhost, userhost))
			access = (user->access > access) ? user->access : access;
	return access;
}

int get_userlevel(userhost, channel)
char *userhost;
char *channel;
{
	ULS *dummy;

	if (!matches(owneruserhost, userhost))
		return OWNERLEVEL;
	if (!my_stricmp(current->nick, getnick(userhost)))
	  return OWNERLEVEL;

	if (!my_stricmp(channel, "*"))
		return max_userlevel(userhost);

	if (!(dummy=is_in_userlist(current->UserList, userhost, channel)))
		return 0;
	return dummy->access;
}

int is_aop(userhost, channel)
char *userhost;
char *channel;
{
	ULS *dummy;

	if (!(dummy=is_in_userlist(current->UserList, userhost, channel)))
		return 0;
	return dummy->aop;
}

int get_protlevel(userhost, channel)
char *userhost;
char *channel;
{
	ULS *dummy;

	if (!matches(owneruserhost, userhost))
		return MAXPROTLEVEL;

	if (!(dummy=is_in_userlist(current->UserList, userhost, channel)))
		return 0;
	return dummy->prot;
}

int password_needed(userhost)
char *userhost;
{
	ULS *dummy;


	if (!(dummy=is_in_userlist(current->UserList, userhost, "*")))
		return FALSE;
	if (!dummy->password || !*dummy->password)
		return FALSE;
	return TRUE;
}

int correct_password(userhost, password)
char *userhost;
char *password;
{
	ULS *dummy;

	if (!(dummy=is_in_userlist(current->UserList, userhost, "*")))
		return FALSE;    /* or TRUE? */
	if (!dummy->password || !*dummy->password)
		return TRUE;
	if (password && passmatch(password, dummy->password))
/*  OLD:
!my_stricmp(password, dummy->password))   */
		return TRUE;
	debug(NOTICE, "INVALID PASSSWORD: %s %s %s", userhost, password, dummy->password);
	return FALSE;
}

int read_userlist(l_list, filename)
ULS **l_list;
char *filename;
{
  FILE *in;
  char lin[MAXLEN];
  char *ptr, *uh, *level, *aop, *prot, *chan;

  if (!(in=fopen(filename, "rt")))
	 return FALSE;
  delete_userlist(l_list);
  while (freadln(in, lin))
  {
	 ptr = lin;
	 uh = get_token(&ptr, ", ");
	 level=get_token(&ptr, ", ");
	 aop=get_token(&ptr, ", ");
	 prot=get_token(&ptr, ", ");
	 chan=get_token(&ptr, ", ");
	 /* Ptr will now point to a password if given */
	 if (!chan || !*chan)
		chan = "*";
	 if (uh && *uh && level)
		add_to_userlist(l_list, uh, level ? atoi(level) : 0, aop ? atoi(aop) : 0,
				prot ? atoi(prot) : 0, chan, ptr ? ptr : "");
  }
  fclose(in);
  return TRUE;
}

int write_userlist(l_list, filename)
ULS **l_list;
char *filename;
{
	ULS *dummy;
	time_t    T;
	FILE   *list_file;

	if (!(list_file = fopen(filename, "w")))
		return FALSE;
 
	T = getthetime();

	fprintf( list_file, "############################################################################\n" );
	fprintf( list_file, "## %s\n", filename);
	fprintf( list_file, "## Created: %s", ctime(&T) );
	fprintf( list_file, "## DweeB0t  %s\n", VERSION);
	fprintf( list_file, "############################################################################\n" );

	for (dummy = *l_list; dummy; dummy = dummy->next)
		fprintf( list_file,
			" %40s %3d %d %d %20s %s\n", dummy->userhost, dummy->access,
			dummy->aop, dummy->prot, dummy->channel,
			dummy->password ? dummy->password : "");
	fprintf(list_file, "## End of %s\n", filename);
	fclose(list_file);
	return TRUE;

}

SLS *is_in_shitlist(l_list, userhost, channel)
SLS **l_list;
char *userhost;
char *channel;
{
	SLS *user;
	if (!userhost)
	  return NULL;
	for (user = *l_list; user; user = user->next)
		if (!matches(user->userhost, userhost) &&
			  (!my_stricmp(channel, user->channel) ||
			  (*user->channel == '*') || (*channel=='*')))
		  return user;
	return NULL;
}

SLS **init_shitlist(void)
{
	SLS **l_list;

	l_list=(SLS **) malloc (sizeof(*l_list));
	*l_list=NULL;
	return(l_list);
}

int add_to_shitlist(l_list, uh, level, chan, from, reason, time)
SLS **l_list;
char *uh;
int level;
char *chan;
char *from;
char *reason;
long int time;
{
	SLS *New_user;

	if (*chan == '*')
	  remove_from_shitlist(l_list, uh, "*");
	if (New_user = is_in_shitlist(l_list, uh, chan))
	  return FALSE;
	if (!(New_user = (SLS *) malloc (sizeof(*New_user))))
	  return FALSE;
	mstrcpy(&New_user->userhost, uh);
	New_user->access = level;
	mstrcpy(&New_user->channel, chan);
	mstrcpy(&New_user->shitby, from);  
	mstrcpy(&New_user->reason, reason);
   
	New_user->time = time;
	New_user->next = *l_list;
	*l_list = New_user;
	return TRUE;
}
void show_shitlist(from, l_list)
char *from;
SLS **l_list;
{
  SLS *dummy;
  send_to_user(from, "\002%s\002", "--------- Userhost --------- Channel ---- Level ---");
  for (dummy = *l_list; dummy; dummy = dummy->next )
	 send_to_user( from, " %20s %15s  %3d", dummy->userhost, dummy->channel, dummy->access);
  send_to_user(from, "\002%s\002", "--- end of shitlist ---");
}

int remove_from_shitlist(l_list, userhost, chan)
SLS **l_list;
char *userhost;
char *chan;
{
	SLS **old;
	SLS *dummy;

	if (!(dummy = is_in_shitlist(l_list, userhost, chan)))
		return FALSE;

	while (dummy = is_in_shitlist(l_list, userhost, chan))
	{
	  for (old = l_list; *old; old = &(*old)->next)
		 if (*old == dummy)
			{
		*old = dummy->next;
		free(dummy->userhost);
		free(dummy->channel);
		free(dummy->shitby);
		free(dummy->reason);
		free(dummy);
		return TRUE;
			}
	}
	return TRUE;
}

void delete_shitlist(l_list)
SLS **l_list;
{
	SLS *dummy;
	SLS *next;

	dummy = *l_list;
	while (dummy)
	{
		next = dummy->next;
		remove_from_shitlist(l_list, dummy->userhost, dummy->channel);
		dummy = next;
	}
}

int is_shitted(userhost, channel)
char *userhost;
char *channel;
{
	if (!matches(owneruserhost, userhost))
		return FALSE;        /* just in case =) */

	return (is_in_shitlist(current->ShitList, userhost, channel) != NULL);
}

char *who_shitted(userhost, channel)
char *userhost;
char *channel;
{
  SLS *dummy;
  if (!matches(owneruserhost, userhost))
    return NULL;        /* just in case =) */

  if (!(dummy = is_in_shitlist(current->ShitList, userhost, channel)))
    return NULL;
  return dummy->shitby;
}

int get_shitlevel(userhost, chan)
char *userhost;
char *chan;
{
	SLS *dummy;

	if (!matches(owneruserhost, userhost))
		return 0;        /* just in case =) */

	if (!(dummy=is_in_shitlist(current->ShitList, userhost, chan)))
		return 0;
	return dummy->access;
}

char *get_shituh(dummy)
SLS *dummy;
{
	strcpy(tempbuffer, "");
	if (dummy)
	{
		strcpy(tempbuffer, dummy->userhost);
		if (isnick(getnick(tempbuffer)))
		{
			char *temp, *temp2;
			char tempbuffer2[MAXLEN];
			strcpy(tempbuffer2, tempbuffer);
			temp2 = tempbuffer2;
			temp = get_token(&temp2, "!");
			strcpy(tempbuffer, getnick(dummy->userhost));
			strcat(tempbuffer, "!");
			strcat(tempbuffer, temp2);
		}
	}
  return tempbuffer;
}

char *get_shitreason(dummy)
SLS *dummy;
{
  strcpy(tempbuffer, "\002");
  if (dummy)
  {
    char *crap;
    if (!dummy->shitby || !*(dummy->shitby))
      crap="";
    else
      crap = getnick(dummy->shitby);

    if (dummy->time)
      {
	strcat(tempbuffer, time2small(dummy->time));
	if (*crap)
	  strcat(tempbuffer, " ");
      }
    if (!*crap)
      strcat(tempbuffer, ": ");
    else
      {
	strcat(tempbuffer, crap);
	strcat(tempbuffer, ": ");
      }
    if (!dummy->reason)
      strcat(tempbuffer, "GET THE HELL OUT!!!");
    else
      strcat(tempbuffer, dummy->reason);
    strcat(tempbuffer, "\002");
  }
  else
	strcat(tempbuffer, "YOU ARE SHITLISTED!!!\002");
  return tempbuffer;
}

int read_shitlist(l_list, filename)
SLS **l_list;
char *filename;
{
  FILE *in;
  char lin[MAXLEN];
  char *ptr, *uh, *level, *chan, *from, *timea;

  if (!(in=fopen(filename, "rt")))
	 return FALSE;
  delete_shitlist(l_list);
  while (freadln(in, lin))
  {
	 ptr = lin;
	 uh = get_token(&ptr, ", ");
	 level=get_token(&ptr, ", ");
	 chan=get_token(&ptr, " ");
         from=get_token(&ptr, " ");
	 timea=get_token(&ptr, " ");
	 /* ptr now points to the reason */
	 if (!chan || !*chan)
		chan = "*";
         if (!from)
             from = "";
	 if (!timea || !*timea)
		timea = "0";
	 if (uh && *uh && level)
		add_to_shitlist(l_list, uh, level ? atoi(level) : 1, chan, from,
			(ptr && *ptr) ? ptr : "", atol(timea));
  }
  fclose(in);
  return TRUE;
}

int write_shitlist(l_list, filename)
SLS **l_list;
char *filename;
{
	SLS *dummy;
	time_t    T;
	FILE   *list_file;

	if (!(list_file = fopen(filename, "w")))
		return FALSE;

	T = getthetime();

	fprintf(list_file, "############################################################################\n");
	fprintf(list_file, "## %s\n", filename);
	fprintf(list_file, "## Created: %s", ctime(&T));
	fprintf(list_file, "## DweeB0t %s\n", VERSION);
	fprintf(list_file, "############################################################################\n");

	for (dummy = *l_list; dummy; dummy = dummy->next)
	  fprintf(list_file,
			" %40s %3d %15s %25s %li %s\n", dummy->userhost, dummy->access,
		  dummy->channel, dummy->shitby,
				dummy->time, dummy->reason);
	fprintf(list_file, "## End of %s\n", filename);
	fclose(list_file);
	return TRUE;
}

LS *is_in_list(l_list, thing)
LS **l_list;
char *thing;
{
	LS *bleah;
	if (!thing)
	  return NULL;
	for (bleah = *l_list; bleah; bleah = bleah->next)
		if (!my_stricmp(bleah->name, thing))
			  return bleah;
	return NULL;
}

LS **init_list(void)
{
	LS **l_list;

	l_list=(LS **) malloc (sizeof(*l_list));
	*l_list=NULL;
	return(l_list);
}

int add_to_list(l_list, thing)
LS **l_list;
char *thing;
{
	LS *New_entry;

	if (New_entry = is_in_list(l_list, thing))
	  return FALSE;
	if (!(New_entry = (LS *) malloc (sizeof(*New_entry))))
	  return FALSE;
	mstrcpy(&New_entry->name, thing);
	New_entry->next = *l_list;
	*l_list = New_entry;
	return TRUE;
}

LS *is_in_list2(l_list, thing)
LS **l_list;
char *thing;
{
	LS *bleah;
	if (!thing)
	  return NULL;
	for (bleah = *l_list; bleah; bleah = bleah->next)
		if (!my_stricmp(getnick(bleah->name), thing))
			  return bleah;
	return NULL;
}

int remove_from_list2(l_list, thing)
LS **l_list;
char *thing;
{
	LS *old;
	LS *dummy;
	char buffer[255];

	strcpy(buffer, getnick(thing));
	if (!(dummy = is_in_list2(l_list, buffer)))
		return FALSE;
	if (dummy == *l_list)
	  {
		 *l_list = dummy->next;
		 free(dummy->name);
		 free(dummy);
		 return TRUE;
	  }
	for( old = *l_list; old; old = old->next )
	  if( old->next == dummy )
		 {
			old->next = dummy->next;
			free(dummy->name);
			free( dummy );
			return(TRUE);
		 }
	return FALSE;
}

int remove_from_list(l_list, thing)
LS **l_list;
char *thing;
{
	LS *old;
	LS *dummy;

	if (!(dummy = is_in_list(l_list, thing)))
		return FALSE;
	if (dummy == *l_list)
	  {
		 *l_list = dummy->next;
		 free(dummy->name);
		 free(dummy);
		 return TRUE;
	  }
	for( old = *l_list; old; old = old->next )
	  if( old->next == dummy )
		 {
			old->next = dummy->next;
			free(dummy->name);
			free( dummy );
			return(TRUE);
		 }
 
/*
	for (old = l_list; *old; old = &(*old)->next)
	  if (*old == dummy)
		 {
			*old = dummy->next;
			free(dummy->name);
			free(dummy);
			return TRUE;
		 }
*/
	return FALSE;
}

void remove_all_from_list(l_list)
LS **l_list;
{
	LS        **old;
	LS        *del;

	for(old = l_list; *old;)
	{
		del = *old;
		*old = (*old)->next;
		free(del->name);
		free(del);
	}
}

void delete_list(l_list)
LS **l_list;
{
	LS *dummy;
	LS *next;

	dummy = *l_list;
	while (dummy)
	{
		next = dummy->next;
		remove_from_list(l_list, dummy->name);
		dummy = next;
	}
}

int change_userlist(char *userhost, char *chan, int level, int aop, int prot)
{
  ULS *dummy;

  if ((dummy = is_in_userlist(current->UserList, userhost, chan)))
	 {
		if (level != -1)
	dummy->access = level;
		if (aop != -1)
	dummy->aop = aop;
		if (prot != -1)
	dummy->prot = prot;
		return TRUE;
	 }
  return FALSE;
}

TS *is_in_timelist(l_list, name)
TS **l_list;
char *name;
{
	TS *user;

	if (!name)
	  return NULL;

	for (user = *l_list; user; user = user->next)
	  if (!my_stricmp(name, user->name))
		return user;
	return NULL;
}

TS **init_timelist(void)
{
	TS **l_list;

	l_list=(TS **) malloc (sizeof(*l_list));
	*l_list=NULL;
	return(l_list);
}

int add_to_timelist(l_list, name)
TS **l_list;
char *name;
{
	TS *New_user;

	if (!name)
	  return FALSE;

	if (New_user = is_in_timelist(l_list, name))
	  return FALSE;
	if (!(New_user = (TS *) malloc (sizeof(*New_user))))
	  return FALSE;
	mstrcpy(&New_user->name, name);
	New_user->num = 1;
	New_user->time = getthetime();
	New_user->next = *l_list;
	*l_list = New_user;
	return TRUE;
}

int remove_from_timelist(l_list, name)
TS **l_list;
char *name;
{
	TS **old;
	TS *dummy;

	if (!(dummy = is_in_timelist(l_list, name)))
		return FALSE;
	while (dummy = is_in_timelist(l_list, name))
	{
	  for (old = l_list; *old; old = &(*old)->next)
		 if (*old == dummy)
		 {
			*old = dummy->next;
			free(dummy->name);
			free(dummy);
			return TRUE;
		 }
	}
	return TRUE;
}

void delete_timelist(l_list, time)
TS **l_list;
time_t time;
{
	TS *user;

	for (user = *l_list; user; user = user->next)
	  if (time >= user->time)
			remove_from_timelist(l_list, user->name);

}

void remove_all_from_timelist(l_list)
TS **l_list;
{
	TS        **old;
	TS        *del;

	for(old = l_list; *old;)
	{
		del = *old;
		*old = (*old)->next;
		free(del->name);
		free(del);
	}
}



