/*
 * chanuser.c - register users on a channel
 * (c) 1994 CoMSTuD (cbehrens@iastate.edu)
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

#include <stddef.h>
#include <stdio.h>
#include "chanuser.h"
#include "function.h"
#include "config.h"

static  char    buffer[MAXLEN];
  

USER_list *search_user(u_list, nick)
USER_list **u_list; 
char *nick;
{
	USER_list       *User;

	for( User = *u_list; User; User = User->next )
		if (!my_stricmp(nick,User->nick))
			return(User);           
	return(NULL);
}

void add_user(u_list, nick, user, host)
USER_list **u_list;
char *nick;
char *user;
char *host;
{
	USER_list *New_user;
	time_t t;

	if (!(New_user = (USER_list*) malloc(sizeof(*New_user))))
		return;
	if( search_user(u_list, nick) != NULL )
		return;
	strcpy(New_user->nick, nick );
	mstrcpy(&New_user->user, user );
	mstrcpy(&New_user->host, host );
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
	New_user->next = *u_list;
	*u_list = New_user;
}

int delete_user(u_list, nick)
USER_list **u_list;
char *nick;
{
	USER_list       **old;
	USER_list       *Dummy;

	if( (Dummy = search_user( u_list, nick )) == NULL )
		return(FALSE);

	for (old = u_list; *old; old = &(*old)->next)
		if (*old == Dummy)
		{
			*old = Dummy->next;
			free(Dummy->user);
			free(Dummy->host);
			free(Dummy);
			return(TRUE);
		}
	return(FALSE);
}

int change_user_nick(u_list, oldnick, newnick)
USER_list **u_list;
char *oldnick;
char *newnick;
{
	USER_list       *Dummy;

	if ((Dummy = search_user( u_list, oldnick )) != NULL)
	{
		strcpy(Dummy->nick, newnick);
		return(TRUE);
	}
	return(FALSE);
}

void clear_all_users(u_list)
USER_list **u_list;
{
	USER_list       **old;

	for (old = u_list; *old; )
	{
		*old = (*old)->next;
		free( *old );
	}
}

/*
 * There need to be some functions which return 
 * elements (next elements). This way the information
 * in the userlists can be used to mass-do things,
 * show a list etc.
 *
 * Actually, to make it a little "opaque", these functions
 * should only be used by channel.c, i.e. everything goes
 * through channel.c...
 */

/* This one is needed to get the next user. 
 */

USER_list *get_nextuser(Old)
USER_list *Old;
{
	return( Old->next );
}

unsigned int get_usermode(User)
USER_list *User;
{
	return(User->mode);
}

char *get_username(User)
USER_list *User;
{
	sprintf(buffer, "%s!%s@%s", User->nick, 
		User->user, User->host );
	return(buffer);
}

char *get_usernick(User)
USER_list *User;
{
	return(User->nick);
}

void add_mode(u_list, mode, param)
USER_list **u_list;
unsigned int mode;
char *param;
{
	USER_list *Dummy;

	if ((Dummy = search_user( u_list, param )) == NULL)
		return;
	Dummy->mode |= mode;
}

void del_mode(u_list, mode, param)
USER_list **u_list;
unsigned int mode;
char *param;
{
	USER_list *Dummy;

	if (!(Dummy = search_user( u_list, param )))
		return;
	Dummy->mode &= ~mode;
}




