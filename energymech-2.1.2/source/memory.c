/*
 * memory.c - manages memorys between IRC users and the bot
 */

#include <stdarg.h>
#include <string.h>

#include "global.h"
#include "config.h"
#include "defines.h"
#include "structs.h"
#include "h.h"

/*
 * No need to put this in a botstruct, it's resetted by every 
 * incoming message.
 */
aMemory *currentmementry;

static char memuserhost[MAXLEN];

void    dcc_sendnext(user)
char *user;
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

int get_protlevel_matches(char *userhost, char *chan)
{
	register aChannel *Channel;
	register aChanUser *User;
	register aMemory *memory;
	int level;
	char *tmp;

	for (memory = current->MemList; memory; memory = memory->next)
		if (!matches(userhost, memory->user) &&
			(level = get_protlevel(memory->user, chan)))
			return level;
	for (Channel = current->Channel_list; Channel; Channel = Channel->next)
		for( User = Channel->users; User; User = User->next )
		{
			tmp = get_username(User);
			if (!matches(userhost, tmp) &&
				(level=get_protlevel(tmp, chan)))
			  return level;
		}
	return 0;
}

aUser *get_shitrec_matches(char *userhost, char *channel)
{
	register aMemory *memory;
	register aChannel *Channel;
	register aChanUser *User;
	register aUser *temp;
	char *tmp;

	for (memory = current->MemList; memory; memory = memory->next)
		if (!matches(userhost, memory->user) &&
			((temp=find_shit(&(current->Userlist),
				memory->user, channel)) != NULL))
		return temp;
	for (Channel = current->Channel_list; Channel; Channel = Channel->next)
		for( User = Channel->users; User; User = User->next )
		{
			tmp = get_username(User);
			if (!matches(userhost, tmp) &&
				((temp=find_shit(&(current->Userlist),
					tmp, channel)) != NULL))
			  return temp;
		}
	return NULL;
}

char *getuserhost_frommem(nick)
char *nick;
{
	aMemory *temp;
	char unknown[] = "<UNKNOWN>@<UNKNOWN>";

	strcpy(memuserhost, nick);
	strcat(memuserhost, "!");
	if ((temp = find_memorybynick(nick)) != NULL)
	{
		char *ptr;
		ptr = strchr(temp->user, '!');
		if (ptr)
		{
			ptr++;
			strcat(memuserhost, ptr);
		}
		else
			strcat(memuserhost, unknown);
	}
	else
		strcat(memuserhost, unknown);
	return memuserhost;
}

aMemory *find_memorybynick(nick)
char *nick;
{
	register aMemory *memory, *save;

	save = NULL;
	for (memory = current->MemList; memory; memory = memory->next)
		if (!my_stricmp(nick, getnick(memory->user)) && (!save ||
			  (memory->last_received > save->last_received)))
		save = memory;
	return save;
}

void change_memnick(olduser, newnuh)
char *olduser;
char *newnuh;
{
	aMemory *memory;

	if ((memory = find_memory(olduser)) != NULL)
	{
		MyFree(&memory->user);
		mstrcpy(&memory->user, newnuh);
	}
}

aMemory *find_memory(user)
char *user;
{
	register aMemory *memory;

	for (memory = current->MemList; memory; memory = memory->next)
		if (!my_stricmp(user, memory->user))
			break;
	return memory;
}

aMemory *make_memory(user)
char *user;
{
	aMemory *new_memory;

	new_memory = (aMemory *) MyMalloc(sizeof(aMemory));
	mstrcpy(&new_memory->user, user);
	new_memory->nodcc_to = NULL;
	new_memory->nodcc_rest = NULL;
	new_memory->last_received = getthetime();
        new_memory->prev = NULL;
        if (current->MemList)
                current->MemList->prev = new_memory;
        new_memory->next = current->MemList;
        current->MemList = new_memory;
        return new_memory;
}

void remove_memory(cptr)
aMemory *cptr;
{
        if (cptr->prev)
                cptr->prev->next = cptr->next;
        else
        {
                current->MemList = cptr->next;
                if (current->MemList)
                        current->MemList->prev = NULL;
        }
        if (cptr->next)
                cptr->next->prev = cptr->prev;
        MyFree(&cptr->user);
	MyFree(&cptr->nodcc_to);
	MyFree(&cptr->nodcc_rest);
	MyFree((char **)&cptr);
}

void delete_memory(l_list)
aMemory **l_list;
{
	register aMemory *cptr, *tmp;
	cptr = *l_list;
	while(cptr)
	{
		tmp = cptr->next;
		remove_memory(cptr);
		cptr = tmp;
	}
	*l_list = NULL;
}

void cleanup_memory(void)
{
	register aMemory *cptr, *tmp;
	time_t now;

	now = getthetime();
	cptr = current->MemList;
	while(cptr)
	{
		tmp = cptr->next;
		if ((now - cptr->last_received) > MEMORY_LIFETIME)
		{
			if (!is_user(cptr->user, "*") &&
				 !is_shitted(cptr->user, "*"))
					 /* keep if has access on the bot or is shitted 
							--- needed for protection */
				remove_memory(cptr);
		}
		cptr = tmp;
	}
}

void check_memory(user)
char *user;
{
	if (!(currentmementry = find_memory(user)))
		currentmementry = make_memory(user);
	currentmementry->last_received = getthetime();
}

void nodcc_session(char *from, char *to, char *rest)
{
	if (!(currentmementry = find_memory(from)))
		currentmementry = make_memory(from);

	currentmementry->last_received = getthetime();
	mstrcpy(&currentmementry->nodcc_to, to);
	mstrcpy(&currentmementry->nodcc_rest, rest);
}

void hasdcc_session(char *from)
{
	if (!(currentmementry = find_memory(from)))
		currentmementry = make_memory(from);
	else if (currentmementry->nodcc_to)
	{
                CurrentUser = find_user(&(current->Userlist), currentmementry->user, "*");
                CurrentShit = find_shit(&(current->Userlist), currentmementry->user, "*");
                strcpy(currentnick, getnick(currentmementry->user));
		on_msg(currentmementry->user,
				 currentmementry->nodcc_to,
				 currentmementry->nodcc_rest);
	}
	MyFree(&currentmementry->nodcc_to);
	MyFree(&currentmementry->nodcc_rest);
	currentmementry->nodcc_to = currentmementry->nodcc_rest = NULL;
	currentmementry->last_received = getthetime();
}


