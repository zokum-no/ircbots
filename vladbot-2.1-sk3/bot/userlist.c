/*
 * userlist.c - implementation of userlists
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
#include <time.h>

#include "config.h"
#define _GNU_SOURCE
#include "fnmatch.h"
#include "misc.h"
#include "send.h"
#include "userlist.h"


USERLVL_list    *exist_userhost ( l_list, userhost )
USERLVL_list	**l_list;
char		*userhost;

{
  USERLVL_list	*User;

  for( User = *l_list; User; User = User->next )
    if( !fnmatch ( User->userhost, userhost, FNM_CASEFOLD ) )
      return(User);

  return(NULL);
}

USERLVL_list	*find_userhost( l_list, userhost )
USERLVL_list	**l_list;
char		*userhost;

{
	USERLVL_list	*User;

	for( User = *l_list; User; User = User->next )
		if( STRCASEEQUAL( userhost, User->userhost ) )
			return(User);
	return(NULL);
}

USERLVL_list	**init_levellist( )

{
	USERLVL_list	**l_list;

	l_list=(USERLVL_list **)malloc(sizeof(*l_list));
	*l_list=NULL;
	return(l_list);
}

void	add_to_levellist( l_list, userhost, level )
USERLVL_list	**l_list;
char		*userhost;
int		level;
/*
 * adds a user to the list... 
 * should we check here for double entries?
 * Check for level (adding higher level etc)
 */

{
	USERLVL_list	*New_user;

	if( (New_user = find_userhost(l_list, userhost)) != NULL )
	{
		New_user->access   = level;
		return;
	}		
	if( (New_user = (USERLVL_list*)malloc(sizeof(*New_user))) == NULL)
		return;
	mstrcpy(&New_user->userhost, userhost );
	New_user->access = level;
	New_user->next = *l_list;
	*l_list = New_user;
}

int	remove_from_levellist( l_list, userhost )
USERLVL_list	**l_list;
char		*userhost;
/*
 * removes the first occurance of userhost from l_list 
 */

{
	USERLVL_list	**old;
	USERLVL_list	*dummy;

	if( (dummy = find_userhost( l_list, userhost )) == NULL )
		return(FALSE);

	for( old = l_list; *old; old = &(*old)->next )
		if( *old == dummy )
		{
			*old = dummy->next;
			free(dummy->userhost);
			free(dummy );
			return(TRUE);
		}
	return(FALSE);
}

void	delete_levellist(USERLVL_list **l_list )
{
	USERLVL_list	*dummy;
	USERLVL_list	*next;

	dummy = *l_list;
	while(dummy)
	{
		next = dummy->next;
		remove_from_levellist(l_list, dummy->userhost);
		dummy = next;
	}
}

int	get_level( l_list, userhost )
USERLVL_list	**l_list;
char		*userhost;

{
	USERLVL_list	*dummy;
	int		access = 0;

	for( dummy = *l_list; dummy; dummy = dummy->next )
		if( !fnmatch( dummy->userhost, userhost, FNM_CASEFOLD ) )
			access = (dummy->access > access) ? 
				 dummy->access : access;

	return(access);
}

int	get_level_neg ( l_list, userhost , success )
USERLVL_list	**l_list;
char		*userhost;
int             *success;

{
	USERLVL_list	*dummy;
	int		access = -500;

	*success = FALSE;

	for( dummy = *l_list; dummy; dummy = dummy->next )
	  if( !fnmatch( dummy->userhost, userhost, FNM_CASEFOLD ) ) {
	    access = (dummy->access > access) ? 
	      dummy->access : access;
	    *success = TRUE;
	  }

	return(access);
}

void    add_to_level (l_list, userhost, humeur )
USERLVL_list	**l_list;
char		*userhost;
int             humeur;

{
        USERLVL_list	*dummy;
	int		rel = 0;

	for( dummy = *l_list; dummy; dummy = dummy->next )
	  if( !fnmatch( dummy->userhost, userhost, FNM_CASEFOLD ) )
	    dummy->access += humeur;

}

void	show_lvllist( l_list, from, userhost )
USERLVL_list	**l_list;
char		*from;
char		*userhost;

{
	USERLVL_list	*dummy;

	for( dummy = *l_list; dummy; dummy = dummy->next )
		if(!fnmatch(dummy->userhost, userhost, FNM_CASEFOLD) || !*userhost)
			send_to_user( from, " %40s | %-4d", 
			dummy->userhost, dummy->access );
}

void    cancel_level ( l_list, level )
USERLVL_list	**l_list;
int             level;
{
  USERLVL_list *dummy;
  USERLVL_list *next;

  dummy = *l_list;
  while(dummy)
    {
      next = dummy->next;
      if (dummy->access == level)
	remove_from_levellist(l_list, dummy->userhost);
      dummy = next;
    }

}

int	write_lvllist( l_list, filename )
USERLVL_list	**l_list;
char		*filename;

{
	USERLVL_list	*dummy;
        time_t 	T;
	FILE	*list_file;

	if( ( list_file = fopen( filename, "w" ) ) == NULL )
		return( FALSE );
 
        T = time( ( time_t *) NULL);

	fprintf( list_file, "#############################################\n" );
	fprintf( list_file, "## %s\n", filename );
	fprintf( list_file, "## Created: %s", ctime( &T ) );
	fprintf( list_file, "## (c) 1993 VladDrac (irvdwijk@cs.vu.nl)\n" );
	fprintf( list_file, "#############################################\n" );

	for( dummy = *l_list; dummy; dummy = dummy->next )
	    fprintf( list_file,
		     " %40s %d\n", dummy->userhost, dummy->access );
	fprintf( list_file, "# End of %s\n", filename );
	fclose( list_file );
	return( TRUE );

}


