/*
 * chanuser.c - register users on a channel
 * (c) 1993 VladDrac (irvdwijk@cs.vu.nl)
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
#include <string.h>

#include "chanuser.h"
#include "channel.h"
#include "misc.h"
#include "config.h"
#include "vladbot.h"

extern	botinfo	*currentbot;
static	char		buffer[MAXLEN];

USER_list	*search_user( USER_list **u_list, char *nick )
{
	USER_list	*User;

	for( User = *u_list; User; User = User->next )
		if( STRCASEEQUAL( nick, User->nick ) )
			return(User);		
	return(NULL);
}

void	add_user( USER_list **u_list, char *nick, 
                  char *user, char *host )
/*
 * adds a user to the list... 
 */
{
	USER_list	*New_user, *srchuser;
	CHAN_list	*chan;

	if( search_user(u_list, nick) != NULL )
		return;
	if( (New_user = (USER_list*)malloc(sizeof(*New_user))) == NULL)
		return;
	strcpy(New_user->nick, nick );
	mstrcpy(&New_user->user, user );
	mstrcpy(&New_user->host, host );
	New_user->mode = 0;
	New_user->auth = 0;
	New_user->kick_flag = 0;
	New_user->numchanline=0;
	New_user->numnickch=0;
	New_user->numctcp=0;
	New_user->firstchanline=0;
	New_user->firstnickch=0;
	New_user->firstctcp=0;
	New_user->next = *u_list;
	*u_list = New_user;

	for(chan = currentbot->Channel_list; chan; chan = chan->next)
	{
		if((srchuser = search_user(&(chan->users), nick)) != NULL)
			if(srchuser->auth != 0)
				New_user->auth = srchuser->auth;
	}
}

int	delete_user( USER_list **u_list, char *nick )
/*
 * removes a user from u_list 
 */

{
	USER_list	*old=NULL,*p=*u_list;
	USER_list	*Dummy;

	if( (Dummy = search_user( u_list, nick )) == NULL )
		return(FALSE);

	while (p!=NULL && p!=Dummy)
		{
		  old=p;
		  p=p->next;
		}

	if (p==NULL)
		return(FALSE);
	if (old==NULL)
		*u_list = (*u_list)->next;
	else
		old->next = p->next;

	free(p->user);
	free(p->host);
	free(p);
	return(TRUE);

}

int	change_user_nick( USER_list **u_list, 
                          char *oldnick, char *newnick )
{
	USER_list	*Dummy;

	if( (Dummy = search_user( u_list, oldnick )) != NULL )
	{
		strcpy( Dummy->nick, newnick );
		return(TRUE);
	}
	return(FALSE);
}

void	clear_all_users( USER_list **u_list )
/*
 * Removes all entries in the list
 */
{
	USER_list	*old,*p=*u_list;

	while (p!=NULL)
		{
		  old=p;
		  p=p->next;
		  free(old);
		}

	*u_list=NULL;

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

USER_list	*get_nextuser( USER_list *Old )
{
	return( Old->next );
}

unsigned	int	get_usermode( USER_list *User )
{
	return(User->mode);
}

char	*get_username( USER_list *User )
{
	sprintf(buffer, "%s!%s@%s", User->nick, 
		User->user, User->host );
	return(buffer);
}

char	*get_usernick( USER_list *User )
{
	return(User->nick);
}

/*
 * Some stuff to keep track of usermodes (not swi)
 */
void	add_mode( USER_list **u_list, 
                  unsigned int mode, char *param )
{
	USER_list	*Dummy;

	if( (Dummy = search_user( u_list, param )) == NULL )	
		return;
	Dummy->mode |= mode;
}

void	del_mode( USER_list **u_list, 
                  unsigned int mode, char *param )
{
	USER_list	*Dummy;

	if( (Dummy = search_user( u_list, param )) == NULL )	
		return;
	Dummy->mode &= ~mode;
}

