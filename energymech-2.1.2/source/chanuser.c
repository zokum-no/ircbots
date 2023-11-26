/*
 * chanuser.c - register users on a channel
 */

/*
#include <stddef.h>
*/
#include <stdio.h>

#include "global.h"
#include "config.h"
#include "defines.h"
#include "structs.h"
#include "h.h"

static char buffer[MAXLEN];

aChanUser *find_chanuser(u_list, nick)
aChanUser **u_list; 
char *nick;
{
	register aChanUser *User;

	for(User=*u_list;User;User=User->next)
		if (!my_stricmp(nick, User->nick))
			return User;
	return NULL;
}

aChanUser *make_chanuser(u_list, nick, user, host, server)
aChanUser **u_list;
char *nick;
char *user;
char *host;
char *server;
{
	aChanUser *New_user;
	time_t t;

	if ((New_user = find_chanuser(u_list, nick)) != NULL) {
		mstrcpy(&New_user->server, server);
		return 0; /* or delete and readd ? */
	}
	New_user = (aChanUser *) MyMalloc(sizeof(aChanUser));
	strcpy(New_user->nick, nick);
	mstrcpy(&New_user->user, user);
	mstrcpy(&New_user->host, host);
	mstrcpy(&New_user->server, server);
       	New_user->mode = 0;
	New_user->floodnum = 0;
	New_user->bannum = 0;
	New_user->deopnum = 0;
	New_user->kicknum = 0;
	New_user->nicknum = 0;
	New_user->beepnum = 0;
	New_user->capsnum = 0;
	t = getthetime();
	New_user->floodtime = t;
	New_user->bantime = t;
	New_user->deoptime = t;
	New_user->kicktime = t;
	New_user->nicktime = t;
	New_user->idletime = t;
	New_user->beeptime = t;
	New_user->capstime = t;
        New_user->prev = NULL;
        if (*u_list)
                (*u_list)->prev = New_user;
        New_user->next = *u_list;
        *u_list = New_user;
	return New_user;
}

void remove_chanuser(l_list, cptr)
aChanUser **l_list;
aChanUser *cptr;
{
        if (cptr->prev)
                cptr->prev->next = cptr->next;
        else
        {
                *l_list = cptr->next;
                if (*l_list)
                        (*l_list)->prev = NULL;
        }
        if (cptr->next)
                cptr->next->prev = cptr->prev;
        MyFree(&cptr->user);
        MyFree(&cptr->host);
        MyFree((char **)&cptr);
}

int delete_chanuser(u_list, nick)
aChanUser **u_list;
char *nick;
{
	aChanUser *Dummy;

	if ((Dummy = find_chanuser(u_list, nick)) == NULL)
		return FALSE;
	remove_chanuser(u_list, Dummy);
	return TRUE;
}

int change_chanuser_nick(u_list, oldnick, newnick)
aChanUser **u_list;
char *oldnick;
char *newnick;
{
	aChanUser *Dummy;

	if ((Dummy = find_chanuser(u_list, oldnick)) != NULL)
	{
		strcpy(Dummy->nick, newnick);
		return TRUE;
	}
	return FALSE;
}

void delete_chanusers(u_list)
aChanUser **u_list;
{
	register aChanUser *old, *tmp;

	old = *u_list;
	while (old)
	{
		tmp = old->next;
		remove_chanuser(u_list, old);
		old = tmp;
	}
	*u_list = NULL;  /* Just in case */
}

unsigned int get_usermode(User)
aChanUser *User;
{
	return User->mode;
}

unsigned int get_opermode(User)
aChanUser *User;
{
	return User->oper;
}

char *get_username(User)
aChanUser *User;
{
	sprintf(buffer, "%s!%s@%s", User->nick, 
		User->user, User->host );
	return buffer;
}

char *get_usernick(User)
aChanUser *User;
{
	return User->nick;
}

void add_mode(u_list, mode, param)
aChanUser **u_list;
unsigned int mode;
char *param;
{
	aChanUser *Dummy;

	if ((Dummy = find_chanuser(u_list, param)) == NULL)
		return;
	Dummy->mode |= mode;
}

void add_oper(u_list, oper, param)
aChanUser **u_list;
unsigned int oper;
char *param;
{
	aChanUser *Dummy;

	if ((Dummy = find_chanuser(u_list, param)) == NULL)
		return;
	Dummy->oper |= oper;
}


void del_mode(u_list, mode, param)
aChanUser **u_list;
unsigned int mode;
char *param;
{
	aChanUser *Dummy;

	if ((Dummy = find_chanuser(u_list, param)) == NULL)
		return;
	Dummy->mode &= ~mode;
}

void del_oper(u_list, oper, param)
aChanUser **u_list;
unsigned int oper;
char *param;
{
	aChanUser *Dummy;

	if ((Dummy = find_chanuser(u_list, param)) == NULL)
		return;
	Dummy->oper &= ~oper;
}

