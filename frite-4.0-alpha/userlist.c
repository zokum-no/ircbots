/*
 * userlist.c - implementation of userlists
 * (c) 1995 OffSpring (cracker@cati.CSUFresno.EDU)
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

#include <stdio.h>
#include <time.h>

#include "config.h"
#include "fnmatch.h"
#include "misc.h"
#include "userlist.h"

USERLVL_list	*finduserhost( l_list, userhost )
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
/* Channel Spy Code in work here */
ListStruct *find_list(ListStruct *WhichList, char *thename)
{       
        ListStruct *Temp;
 
        if (!thename)
                return NULL;
 
        for (Temp = WhichList; Temp; Temp = Temp->next )
                if (!matches( Temp->name, thename))
                        return Temp;
 
        return NULL;
}       

int add_to_list( ListStruct **WhichList, char *thename)
{    
     ListStruct *New_name;
     if( (New_name = find_list(*WhichList, thename)) != NULL )
        return FALSE;
     if( (New_name = (ListStruct *) malloc (sizeof(*New_name))) == NULL)
        return FALSE;
     strcpy(New_name->name, thename );
     New_name->next = *WhichList;
     *WhichList = New_name;
     return TRUE;
}

int is_in_list( ListStruct *WhichList, char *thename)
{  
	return (find_list( WhichList, thename) != NULL);
}       
int remove_from_list( ListStruct **WhichList, char *thename )
{
        ListStruct *old;
        ListStruct *dummy;
        if( (dummy = find_list( *WhichList, thename )) == NULL )
                return(FALSE);
        if (dummy == *WhichList)
        {
                *WhichList = dummy->next;
                free(dummy);
                return TRUE;
        }        
        for( old = *WhichList; old; old = old->next )
                if( old->next == dummy )
                {
                        old->next = dummy->next;
                        free( dummy );
                        return(TRUE);
                }
        return(FALSE);
}

/* */
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

	if( (New_user = finduserhost(l_list, userhost)) != NULL )
	{
		New_user->access = level;
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

	if( (dummy = finduserhost( l_list, userhost )) == NULL )
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
void	show_lvllist( l_list, from, userhost )
USERLVL_list	**l_list;
char		*from;
char		*userhost;

{
	USERLVL_list	*dummy;

	for( dummy = *l_list; dummy; dummy = dummy->next )
		if(!fnmatch(dummy->userhost, userhost, FNM_CASEFOLD) || !*userhost)
			send_to_user( from, " %40s | %d", 
			dummy->userhost, dummy->access );
}
int	count_lvllist( from, l_list )
char *from;
USERLVL_list    **l_list;
{
	USERLVL_list *dummy;
        int total=0;

        for( dummy = *l_list; dummy;dummy = dummy->next )
           total++;

        send_to_user(from, "I have %i users in my database.", total);
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
	fprintf( list_file, "## OffSpring (cracker@cati.CSUFresno.EDU)\n" );
	fprintf( list_file, "#############################################\n" );

	for( dummy = *l_list; dummy; dummy = dummy->next )
		fprintf( list_file,
		" %40s %d\n", dummy->userhost, dummy->access );
	fprintf( list_file, "# End of %s\n", filename );
	fclose( list_file );
	return( TRUE );

}
int remove_all_from_list( ListStruct **WhichList )
{
        ListStruct *dummy;

        for( dummy = *WhichList; dummy; dummy = dummy->next )
                free(dummy);
        *WhichList = init_list();
}
ListStruct *init_list( void )
{
        ListStruct *l_list;
        l_list = (ListStruct *) malloc (sizeof(l_list));
        l_list = NULL;
        return (l_list);
}


