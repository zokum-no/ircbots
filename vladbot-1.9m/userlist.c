
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include "userlist.h"
#include "config.h"

USERLVL_list	*find_userhost( l_list, userhost )
USERLVL_list	**l_list;
char		*userhost;

{
	USERLVL_list	*User;

	for( User = *l_list; User; User = User->next )
		if( !strcasecmp( userhost, User->userhost ) )
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
		New_user->access = level;
		return;
	}		
	if( (New_user = (USERLVL_list*)malloc(sizeof(*New_user))) == NULL)
		return;
	strcpy(New_user->userhost, userhost );
	New_user->access = level;
	New_user->next = *l_list;
	*l_list = New_user;
}

int	remove_from_levellist( l_list, userhost )
USERLVL_list	**l_list;
char		*userhost;
/*
 * removes a user from l_list 
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
			free( dummy );
			return(TRUE);
		}
	return(FALSE);
}

int	get_level( l_list, userhost )
USERLVL_list	**l_list;
char		*userhost;

{
	USERLVL_list	*dummy;
	int		access = 0;

	for( dummy = *l_list; dummy; dummy = dummy->next )
		if( !matches( dummy->userhost, userhost ) )
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
		if(!matches(dummy->userhost, userhost) || !*userhost)
			send_to_user( from, " %40s | %d", 
			dummy->userhost, dummy->access );
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


