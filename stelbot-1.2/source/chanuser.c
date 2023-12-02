/*
 * chanuser.c - register users on a channel
 */

#include <stddef.h>
#include <stdio.h>
#include "chanuser.h"
#include "config.h"
#include "function.h"

static char  buffer[MAXLEN];
extern char current_nick[MAXLEN];

USER_list   *search_user( USER_list **u_list, char *nick)
{
	USER_list	*User;

	for( User = *u_list; User; User = User->next )
		if( !strcasecmp( nick, User->nick ) )
			return(User);		
	return(NULL);
}

void	add_user( u_list, nick, user, host )
USER_list	**u_list;
char		*nick;
char		*user;
char		*host;
/*
 * adds a user to the list... 
 */
{
	USER_list	*New_user;

	if( (New_user = (USER_list*)malloc(sizeof(*New_user))) == NULL)
		return;
	if( search_user(u_list, nick) != NULL )
		return;
	strcpy(New_user->nick, nick );
	strcpy(New_user->user, user );
	strcpy(New_user->host, host );
	New_user->mode = 0;
	New_user->idletime = getthetime();
	New_user->nctime = getthetime();
	New_user->floodtime = getthetime();
	New_user->floodnum = 0;
	New_user->numnc = 0;
	New_user->next = *u_list;
	*u_list = New_user;
}

int	delete_user( u_list, nick )
USER_list	**u_list;
char		*nick;
/*
 * removes a user from u_list 
 */

{
	USER_list	**old;
	USER_list	*Dummy;

	if( (Dummy = search_user( u_list, nick )) == NULL )
		return(FALSE);

	for( old = u_list; *old; old = &(*old)->next )
		if( *old == Dummy )
		{
			*old = Dummy->next;
			free( Dummy );
			return(TRUE);
		}
	return(FALSE);
}

int change_user_nick( USER_list **u_list, char *oldnick, char *newnick, char *channel)
{
	USER_list	*Dummy;
	char buffer[MAXLEN];

	if( (Dummy = search_user( u_list, oldnick )) != NULL )
	{
		if ((getthetime() - Dummy->nctime) > 5)
		{
			Dummy->nctime = getthetime();
			Dummy->numnc = 0;
		}
		Dummy->numnc++;
		if ((Dummy->numnc >= 3) && strcasecmp(newnick, current_nick) &&
			strcasecmp(Dummy->nick, current_nick))
		{
			sprintf(buffer, "%s!%s@%s", Dummy->nick, Dummy->user,
				Dummy->host);
			ban_user(channel, buffer);
			sendkick(channel, Dummy->nick, "\x2Your outta here LAMER!!!\x2");
		}
		strcpy( Dummy->nick, newnick );
		return(TRUE);
	}
	return(FALSE);
}

void	clear_all_users( u_list )
USER_list	**u_list;
/*
 * Removes all entries in the list
 */
{
	USER_list	**old;

	for( old = u_list; *old; )
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

USER_list	*get_nextuser( Old )
USER_list	*Old;

{
	return( Old->next );
}

unsigned	int	get_usermode( User )
USER_list	*User;

{
	return(User->mode);
}

char	*get_username( User )
USER_list	*User;

{
	sprintf(buffer, "%s!%s@%s", User->nick, 
		User->user, User->host );
	return(buffer);
}

char	*get_usernick( User )
USER_list	*User;

{
	return(User->nick);
}

/*
 * Some stuff to keep track of usermodes (not swi)
 */
void	add_mode( u_list, mode, param )
USER_list	**u_list;
unsigned int	mode;
char		*param;
{
	USER_list	*Dummy;

	if( (Dummy = search_user( u_list, param )) == NULL )	
		return;
	Dummy->mode |= mode;
}

void	del_mode( u_list, mode, param )
USER_list	**u_list;
unsigned int	mode;
char		*param;
{
	USER_list	*Dummy;

	if( (Dummy = search_user( u_list, param )) == NULL )	
		return;
	Dummy->mode &= ~mode;
}



