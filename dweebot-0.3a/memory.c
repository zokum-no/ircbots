/*
 * memory.c - manages memorys between IRC users and the bot
 */

#include <stdarg.h>
#include <string.h>
#include "chanuser.h"
#include "channel.h"
#include "dweeb.h"
#include "d-ons.h"
#include "userlist.h"
#include "memory.h"
#include "config.h"
#include "function.h"
#include "fnmatch.h"
#include "debug.h"
		
extern  botinfo *current;
/*
 * No need to put this in a botstruct, it's resetted by every 
 * incoming message.
 */
MemoryStruct *currentmementry;

static char memuserhost[MAXLEN];

int get_protlevel2(char *userhost, char *chan)
{
  CHAN_list *Channel;
  USER_list *User;
  MemoryStruct *memory;
  int level;

  debug(NOTICE, "entered protlevel2, bs: %s, chan: %s", userhost, chan);
  for (memory = current->MemList; memory; memory = memory->next)
	 if (!matches(userhost, memory->user) &&
	(level = get_protlevel(memory->user, chan)))
		{
	debug(NOTICE, "protlevel2: banstring=%s, userhost found=%s, level=%i",
			userhost, memory->user, get_protlevel(memory->user, chan));
	return level;
		}
  for (Channel = current->Channel_list; Channel; Channel = Channel->next)
	 for( User = Channel->users; User; User = User->next )
	if (!matches(userhost, get_username(User)) &&
		  (level=get_protlevel(get_username(User), chan)))
		  return level;
  return 0;
}

int get_shitlevel2(char *userhost, char *chan)
{
  MemoryStruct *memory;
  CHAN_list *Channel;
  USER_list *User;
  int level;

  for (memory = current->MemList; memory; memory = memory->next)
	 if (!matches(userhost, memory->user) &&
	(level = get_shitlevel(memory->user, chan)))
		return level;

  for (Channel = current->Channel_list; Channel; Channel = Channel->next)
	 for( User = Channel->users; User; User = User->next )
		if (!matches(userhost, get_username(User)) &&
		(level=get_shitlevel(get_username(User), chan)))
		  return level;
  return 0;
}

char *get_shituh2(char *userhost, char *chan)
{
  CHAN_list *Channel;
  USER_list *User;
  MemoryStruct *memory;
  ULS *temp;
  
  for (memory = current->MemList; memory; memory = memory->next)
	 if (get_shitlevel(memory->user, chan) &&
		 (!fnmatch(userhost, memory->user, FNM_CASEFOLD)))
		return get_shituh(
	 is_in_shitlist(current->ShitList, memory->user, chan));
  for (Channel = current->Channel_list; Channel; Channel = Channel->next)
	 for( User = Channel->users; User; User = User->next )
		if (get_shitlevel(get_username(User), chan) &&
			 (!fnmatch(userhost, get_username(User), FNM_CASEFOLD)))
		  return get_shituh(
			is_in_shitlist(current->ShitList, get_username(User), chan));
  return NULL;
}


char *getuserhost_frommem(nick)
char *nick;
{
	MemoryStruct *temp;

	strcpy(memuserhost, nick);
	strcat(memuserhost, "!<UNKNOWN>@<UNKNOWN>");
	if (temp = find_memorybynick(nick))
	{
		char *ptr;
		ptr = strchr(temp->user, '!');
		if (ptr)
			ptr++;
		if (ptr)
			strcpy(memuserhost, ptr);
	}
	return memuserhost;
}

MemoryStruct *find_memorybynick(nick)
char *nick;
{
	MemoryStruct *memory, *save;

	save = NULL;
	for (memory = current->MemList; memory; memory = memory->next)
		if (!my_stricmp(nick, getnick(memory->user)) && (!save ||
			  (memory->last_received > save->last_received)))
		save = memory;
	return save;
}

void change_memnick(olduser, newnick)
char *olduser;
char *newnick;
{
	MemoryStruct *memory;
	char buffer[255];

	if (memory = find_memory(olduser))
	{

		char *temp;
		temp = strchr(olduser, '!');
		if (!temp)
			return;
		strcpy(buffer, newnick);
		strcat(buffer, temp);
		if (memory->user)
			free(memory->user);
		mstrcpy(&memory->user, buffer);
	}
}

MemoryStruct *find_memory(user)
char *user;
{
	MemoryStruct *memory;

	for (memory = current->MemList; memory; memory = memory->next)
		if (!my_stricmp(user, memory->user))
			return memory;
	return NULL;
}

MemoryStruct *create_memory(user)
char *user;
{
	MemoryStruct *new_memory;

	if (!(new_memory = (MemoryStruct *) malloc(sizeof(MemoryStruct))))
		return NULL;
	mstrcpy(&new_memory->user, user);
	new_memory->nodcc_to = NULL;
	new_memory->nodcc_rest = NULL;
	new_memory->last_received = getthetime();
	new_memory->flood_start = getthetime();
	new_memory->flood_cnt = 1;                     /* one msg */
	new_memory->flooding = FALSE;
	new_memory->password = 0;
	new_memory->next = current->MemList;
	current->MemList = new_memory;
	return new_memory;
}

int delete_memory(user)
char *user;
{
	MemoryStruct **memory;
	MemoryStruct *old;

	for (memory = &current->MemList; *memory;
		 memory = &(**memory).next)
		if (!my_stricmp((**memory).user, user))
		{
			old = *memory;
			*memory = old->next;
			free(old->user);
			free(old);
			return SUCCESS;
		}
	return FAIL;
}

void cleanup_memory(void)
{
	MemoryStruct *memory;

	for (memory = current->MemList; memory;memory = memory->next)
	{
	  if (((getthetime() - memory->last_received) > 600) &&
	    !is_present(getnick(memory->user)))
		 if (memory->password)
			{
		memory->password = 0;
		send_to_user(memory->user, "\002Authorization has expired\002");
			}
		  
		if ((getthetime() - memory->last_received) > MEMORY_LIFETIME)
		{
			if (!is_user(memory->user, "*") &&
				 !is_shitted(memory->user, "*"))
					 /* keep if has access on the bot or is shitted 
							--- needed for protection */
				delete_memory(memory->user);
		}
		else if(((getthetime() - memory->flood_start) > FLOOD_IGNORESEC)
			&& memory->flooding)
		{
			memory->flooding = FALSE;
			memory->flood_cnt = 0;
		}
	}
}

int check_memory(user)
char *user;
{
	if (!(currentmementry = find_memory(user)))
		currentmementry = create_memory(user);
	if (check_flood(user))
		return IS_FLOODING;
	currentmementry->last_received = time(NULL);
	return IS_OK;
}

int check_flood(user)
char *user;
{
	if ((currentmementry = find_memory(user)) == NULL)
	{       
		currentmementry = create_memory(user);
		return FALSE;
	}
	else if(currentmementry->flooding)
	{
		currentmementry->last_received = getthetime();
		return TRUE;
	}       
	else
	{
		if (getthetime() - currentmementry->flood_start > FLOOD_INTERVAL)
		{       /* reset values */
			currentmementry->flood_start = getthetime();
			currentmementry->flood_cnt = 0;
			currentmementry->flooding = FALSE;
		}
		currentmementry->last_received = getthetime();
		currentmementry->flood_cnt++;
		if (currentmementry->flood_cnt >= FLOOD_RATE)
		  {
			currentmementry->flooding = TRUE;
			send_to_user(currentmementry->user, "\002%s\002", "You are being ignored for a while");
		  }
		return currentmementry->flooding;
	}
}

void set_mempassword(char *from, int num)
{
  if (!(currentmementry = find_memory(from)))
	 currentmementry=create_memory(from);
  
  currentmementry->password = num;
}

int get_mempassword(char *from)
{
  if (!(currentmementry = find_memory(from)))
	 currentmementry=create_memory(from);

  return currentmementry->password;
}

void nodcc_session(char *from, char *to, char *rest)
{
	if (!(currentmementry = find_memory(from)))
		currentmementry = create_memory(from);

	currentmementry->last_received = getthetime();
	mstrcpy(&currentmementry->nodcc_to, to);
	mstrcpy(&currentmementry->nodcc_rest, rest);
}

void hasdcc_session(char *from)
{
	if (!(currentmementry = find_memory(from)))
		currentmementry = create_memory(from);
	else if (currentmementry->nodcc_to)
		on_msg(currentmementry->user,
				 currentmementry->nodcc_to,
				 currentmementry->nodcc_rest);
	free(currentmementry->nodcc_to);
	free(currentmementry->nodcc_rest);
	currentmementry->nodcc_to = currentmementry->nodcc_rest = NULL;
	currentmementry->last_received = getthetime();
}

void    dcc_sendnext(char *user)
{
  return;
/*
	if((currentsession = find_session(user)) == NULL)
		return;

	currentsession->last_received = time(NULL);
	if(currentsession->maxfile == 0)
		return;

	if(currentsession->currentfile == currentsession->maxfile)
	{
		int     i;

		for(i=0; i<currentsession->maxfile; i++)
		{
			free(currentsession->file_list[i].name);
			free(currentsession->file_list[i].path);
		}
		currentsession->maxfile = 0;
		currentsession->currentfile = 0;
		return;
	}

	while((currentsession->currentfile < currentsession->maxfile) &&
			 !send_file_from_list(user, 
			 currentsession->file_list[currentsession->currentfile].path, 
			 currentsession->file_list[currentsession->currentfile].name))
		currentsession->currentfile++;

	currentsession->currentfile++;
*/
}

int     dcc_sendlist(char *user, char *path, char *file_list[], int n)
/*
 * Sends all files in file_list[] (located in path) which contains n 
 * entries to user "user" one-by-one!
 * If internally the maxfile != 0, it means there are already files
 * being queued and the function will return 0, else 1
 *
 */
{
	int     i,j;
	int     files_to_copy;
/*
	if((currentsession = find_session(user)) == NULL)
			currentsession = create_session(user);

	currentsession->last_received = time(NULL);
	files_to_copy = (currentsession->maxfile+n>LISTSIZE)?
			(LISTSIZE-currentsession->maxfile):
			n;
	sendreply("%d File%s added to filequeue", files_to_copy,
		  files_to_copy==1?"":"s");

		 
	for(i=0, j=currentsession->maxfile; i<files_to_copy; i++, j++)
	{
		mstrcpy(&(currentsession->file_list[j].path), path);
		mstrcpy(&(currentsession->file_list[j].name), file_list[i]);
	}
	currentsession->maxfile += files_to_copy;
	if(currentsession->currentfile == 0)
		dcc_sendnext(user); */
	return TRUE; 
}

void    do_showqueue()
{
	int     i;
/*
	sendreply("Maxfile = %d, currentfile = %d", currentsession->maxfile,
			currentsession->currentfile);
	sendreply("Filequeue:");
	for(i=0; i<currentsession->maxfile; i++)
		sendreply("path: %s, file %s", currentsession->file_list[i].path
,
							 currentsession->file_list[i].name
);
*/
}







