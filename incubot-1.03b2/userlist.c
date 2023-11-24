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
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"
#include "fnmatch.h"
#include "misc.h"
#include "send.h"
#include "crc-ons.h"
#include "userlist.h"
#include "tb-ons.h"
#include "incu-ons.h"

userlist_t	*find_userhost(userlist_t **l_list, char *userhost, int type)
/*
 * "type" is 0 for an exact match to "userhost" or 1 for any match.
 *
 */
{
	userlist_t	*User;

	for( User = *l_list; User; User = User->next )
	{
		if (type==0)
		{
			if (STRCASEEQUAL(userhost,User->userhost))
				return(User);
		}
		else if (!mymatch(User->userhost,userhost,FNM_CASEFOLD) ||
				!mymatch(userhost,User->userhost,FNM_CASEFOLD))
			return(User);
	}
/*
	for( User = *l_list; User; User = User->next )
		if( STRCASEEQUAL( userhost, User->userhost ) )
			return(User);
*/
	return(NULL);
}

EXCLUDE_list	*find_excludehost(EXCLUDE_list **l_list, char *userhost, int type)
{
	EXCLUDE_list	*User;

	for( User = *l_list; User; User = User->next )
	{
		if (type==0)
		{
			if (STRCASEEQUAL(userhost,User->userhost))
				return(User);
		}
		else if (!mymatch(User->userhost,userhost,FNM_CASEFOLD) ||
			!mymatch(userhost,User->userhost,FNM_CASEFOLD))
			return(User);
	}

/*
	for( User = *l_list; User; User = User->next )
		if( STRCASEEQUAL( userhost, User->userhost ) )
			return(User);
*/
	return(NULL);
}

userlist_t	**init_levellist( )
{
	userlist_t	**l_list;

	l_list=(userlist_t **)malloc(sizeof(*l_list));
	*l_list=NULL;
	return(l_list);
}

EXCLUDE_list	**init_excludelist( )
{
	EXCLUDE_list	**l_list;

	l_list=(EXCLUDE_list **)malloc(sizeof(*l_list));
	*l_list=NULL;
	return(l_list);
}

int	add_to_levellist(userlist_t	**l_list, char *userhost, int level, char *mailaddr, char *nick)
/*
 * adds a user to the list...
 * should we check here for double entries?
 * Check for level (adding higher level etc)
 */

{
	userlist_t	*New_user;

	if( (New_user = find_userhost(l_list, userhost, 1)) != NULL )
	{
		/* If we didn't find an EXACT match, prevent dupes */
		if (!STRCASEEQUAL(New_user->userhost,userhost))
			return FALSE;

		New_user->access = level;
		free(New_user->mailaddr);
		free(New_user->nick);
		mstrcpy(&New_user->mailaddr, mailaddr);
		mstrcpy(&New_user->nick, nick);
		return TRUE;
	}

	if( (New_user = (userlist_t *)malloc(sizeof(*New_user))) == NULL)
		return FALSE;

	mstrcpy(&New_user->userhost, userhost);
	mstrcpy(&New_user->mailaddr, mailaddr);
	mstrcpy(&New_user->nick, nick);
	New_user->access = level;
	New_user->lock = 0;
	New_user->next = *l_list;
	*l_list = New_user;
	return TRUE;
}

int	add_to_excludelist( l_list, userhost)
EXCLUDE_list	**l_list;
char		*userhost;
/*
 * adds a user to the list...
 * should we check here for double entries?
 * Check for level (adding higher level etc)
 */

{
	EXCLUDE_list	*New_user;

/* if its already in the list, don't add it again */
	if( (New_user = find_excludehost(l_list, userhost, 1)) != NULL )
		return FALSE;

	if( (New_user = (EXCLUDE_list*)malloc(sizeof(*New_user))) == NULL)
		return FALSE;

	mstrcpy(&New_user->userhost, userhost );
	New_user->next = *l_list;
	*l_list = New_user;
	return TRUE;
}

int	add_to_passwdlist( l_list, userhost, passwd )
userlist_t	**l_list;
char		*userhost;
char		*passwd;
/*
 * adds a user to the list... 
 * should we check here for double entries?
 * Check for level (adding higher level etc)
 */

{
	userlist_t	*New_user;

	if( (New_user = find_userhost(l_list, userhost, 1)) != NULL )
	{
		/* If we didn't find an EXACT match, prevent dupes */
		if (!STRCASEEQUAL(New_user->userhost,userhost))
			return FALSE;

		free (New_user->passwd);
		New_user->passwd = strdup(passwd);
		return TRUE;
	}

	if( (New_user = (userlist_t*)malloc(sizeof(*New_user))) == NULL)
		return FALSE;

	mstrcpy(&New_user->userhost, userhost );
	New_user->passwd = strdup(passwd);
	New_user->access=New_user->lock=0;
	New_user->next = *l_list;
	*l_list = New_user;
	return TRUE;
}

int	remove_from_levellist( l_list, userhost )
userlist_t	**l_list;
char		*userhost;
/*
 * removes the first occurance of userhost from l_list
 */

{
	userlist_t	*old=NULL,*p=*l_list;
	userlist_t	*dummy;

	if( (dummy = find_userhost(l_list, userhost, 0)) == NULL )
		return(FALSE);

	while (p!=NULL && p!=dummy)
		{
		  old=p;
		  p=p->next;
		}

	if (p==NULL)
		return(FALSE);
	if (old==NULL)
		*l_list = (*l_list)->next;
	else
		old->next = p->next;

	free(p->userhost);
	free(p);
	return(TRUE);
}

int	remove_from_excludelist(EXCLUDE_list **l_list, char *userhost)
/*
 * removes the first occurance of userhost from l_list
 */

{
	EXCLUDE_list	*old=NULL,*p=*l_list;
	EXCLUDE_list	*dummy;

	if( (dummy = find_excludehost( l_list, userhost, 0 )) == NULL )
		return(FALSE);

	while (p!=NULL && p!=dummy)
		{
		  old=p;
		  p=p->next;
		}

	if (p==NULL)
		return(FALSE);
	if (old==NULL)
		*l_list = (*l_list)->next;
	else
		old->next = p->next;

	free(p->userhost);
	free(p);
	return(TRUE);
}

void	delete_levellist(userlist_t **l_list )
{
	userlist_t	*dummy;
	userlist_t	*next;

	dummy = *l_list;
	while(dummy)
	{
		next = dummy->next;
		remove_from_levellist(l_list, dummy->userhost);
		dummy = next;
	}
}

void	delete_excludelist(EXCLUDE_list **l_list )
{
	EXCLUDE_list	*dummy;
	EXCLUDE_list	*next;

	dummy = *l_list;
	while(dummy)
	{
		next = dummy->next;
		remove_from_excludelist(l_list, dummy->userhost);
		dummy = next;
	}
}

int	get_level( l_list, userhost )
userlist_t	**l_list;
char		*userhost;

{
	userlist_t	*dummy;
	int				access = 0;

	for( dummy = *l_list; dummy; dummy = dummy->next )
		if( !mymatch( dummy->userhost, userhost, FNM_CASEFOLD ) )
			access = (dummy->access > access) ?
			dummy->access : access;

	return(access);
}

int	change_email	(userlist_t **l_list, char *userhost, char *newaddr)
{
	userlist_t	*dummy;

	for( dummy = *l_list; dummy; dummy = dummy->next )
		if( !mymatch( dummy->userhost, userhost, FNM_CASEFOLD ) )
		{
			free(dummy->mailaddr);
			mstrcpy(&dummy->mailaddr, newaddr);
			return TRUE;
		}

	return FALSE;
}

int	change_userlist_nick	(userlist_t **l_list, char *userhost, char *newnick)
{
	userlist_t	*dummy;

	for( dummy = *l_list; dummy; dummy = dummy->next )
		if( !mymatch( dummy->userhost, userhost, FNM_CASEFOLD ) )
		{
			free(dummy->nick);
			mstrcpy(&dummy->nick, newnick);
			return TRUE;
		}

	return FALSE;
}

char	*get_email	(userlist_t **l_list, char *userhost)
{
	userlist_t	*dummy;

	for( dummy = *l_list; dummy; dummy = dummy->next )
		if( !mymatch( dummy->userhost, userhost, FNM_CASEFOLD ) )
		{
			return(dummy->mailaddr);
		}

	return(NULL);
}

char	*get_userlist_nick	(userlist_t **l_list, char *userhost)
{
	userlist_t	*dummy;

	for( dummy = *l_list; dummy; dummy = dummy->next )
		if( !mymatch( dummy->userhost, userhost, FNM_CASEFOLD ) )
		{
			return(dummy->nick);
		}

	return(NULL);
}

int	get_userlist_info	(userlist_t **l_list, char *from, char *userstr, int what2show)
/*
	what2show:
                0 - Match nick.
                1 - Match email.
                2 - Match hostname.
              3-5 - Same as 0-2 except all matches are listed instead of just the first one.
*/
{
	userlist_t	*dummy;
	int		match=0;

	for( dummy = *l_list; dummy; dummy = dummy->next )
	{
		switch(what2show)
		{
			case 0:
				if(!mymatch(userstr, dummy->nick, FNM_CASEFOLD))
				{
					match++;
					send_to_user(from,"Email: %s  Nick: %s",dummy->mailaddr,dummy->nick);
					return (match);
				}
				break;
			case 1:
				if (!mymatch(userstr, dummy->mailaddr, FNM_CASEFOLD))
				{
					match++;
					send_to_user(from,"Email: %s  Nick: %s",dummy->mailaddr,dummy->nick);
					return (match);
				}
				break;
			case 2:
				if (!mymatch(userstr, dummy->userhost, FNM_CASEFOLD))
				{
					match++;
					send_to_user(from,"Mask: %s  Email: %s  Nick: %s",dummy->userhost, dummy->mailaddr, dummy->nick);
					return (match);
				}
				break;
			case 3:
				if(!mymatch(userstr, dummy->nick, FNM_CASEFOLD))
				{
					match++;
					send_to_user(from,"Email: %s  Nick: %s",dummy->mailaddr,dummy->nick);
				}
				break;
			case 4:
				if (!mymatch(userstr, dummy->mailaddr, FNM_CASEFOLD))
				{
					match++;
					send_to_user(from,"Email: %s  Nick: %s",dummy->mailaddr,dummy->nick);
				}
				break;
			case 5:
				if (!mymatch(userstr, dummy->userhost, FNM_CASEFOLD))
				{
					match++;
					send_to_user(from,"Mask: %s  Email: %s  Nick: %s",dummy->userhost, dummy->mailaddr, dummy->nick);
				}
				break;
		}
	}
	return (match);
}

char	*get_mask( l_list, userhost )
userlist_t	**l_list;
char		*userhost;

{
	userlist_t	*dummy;
	int				access = 0;
	static char		*mask=NULL;

	if (mask)
	{
		free(mask);
		mask = NULL;
	}

	for( dummy = *l_list; dummy; dummy = dummy->next )
		if( !mymatch( dummy->userhost, userhost, FNM_CASEFOLD ) )
			if (dummy->access > access)
			{
				access=dummy->access;
				if (mask)
					free(mask);
				mask=strdup(dummy->userhost);
			}

	return(mask);
}

char	*get_name( l_list, userhost )
userlist_t	**l_list;
char		*userhost;

{
	userlist_t	*dummy;

	for( dummy = *l_list; dummy; dummy = dummy->next )
		if( !mymatch( dummy->userhost, userhost, FNM_CASEFOLD ) )
			return dummy->userhost;

	return(NULL);
}

char 	*get_passwd( l_list, userhost )
userlist_t	**l_list;
char		*userhost;

{
	userlist_t	*dummy;
	char *access=NULL;

	for( dummy = *l_list; dummy; dummy = dummy->next )
	{
		if( !mymatch( dummy->userhost, userhost, FNM_CASEFOLD ) )
			access = dummy->passwd;
	}

	return(access);
}

int 	get_userpasswdfail( l_list, userhost )
userlist_t	**l_list;
char		*userhost;

{
	userlist_t	*dummy;
	int access=-1;

	for( dummy = *l_list; dummy; dummy = dummy->next )
	{
		if( !mymatch( dummy->userhost, userhost, FNM_CASEFOLD ) )
			access = dummy->access;
	}

	return(access);
}

int 	check_lock( l_list, userhost )
userlist_t	**l_list;
char		*userhost;

{
	userlist_t	*dummy;

	for( dummy = *l_list; dummy; dummy = dummy->next )
	{
		if( !mymatch( dummy->userhost, userhost, FNM_CASEFOLD ) )
			return dummy->lock;
	}

	return 0;
}

int 	lock_passwd( l_list, userhost )
userlist_t	**l_list;
char		*userhost;

{
	userlist_t	*dummy;

	for( dummy = *l_list; dummy; dummy = dummy->next )
	{
		if( !mymatch( dummy->userhost, userhost, FNM_CASEFOLD ) )
		{
			dummy->lock=1;
			return 1;
		}
	}

  return 0;

}

int 	unlock_passwd( l_list, userhost )
userlist_t	**l_list;
char		*userhost;

{
	userlist_t	*dummy;

	for( dummy = *l_list; dummy; dummy = dummy->next )
	{
		if( !mymatch( dummy->userhost, userhost, FNM_CASEFOLD ) )
			{dummy->lock=0; return 1;}
	}

   return 0;

}

int 	clear_userpasswdfail( l_list, userhost )
userlist_t	**l_list;
char		*userhost;

{
	userlist_t	*dummy;
	int access=0;

	for( dummy = *l_list; dummy; dummy = dummy->next )
	{
		if( !mymatch( dummy->userhost, userhost, FNM_CASEFOLD ) )
		{
			dummy->access=0;
			access=1;
		}
	}

	return(access);
}

int userpasswdfail( l_list, userhost )
userlist_t	**l_list;
char		*userhost;

{
	userlist_t	*dummy;

	for( dummy = *l_list; dummy; dummy = dummy->next )
	{
		if( !mymatch( dummy->userhost, userhost, FNM_CASEFOLD ) )
			return (++dummy->access);
	}

	return 0;

}

void	change_passwdlist( l_list, userhost, passwd )
userlist_t	**l_list;
char		*userhost;
char		*passwd;
/*
 * adds a user to the list... 
 * should we check here for double entries?
 * Check for level (adding higher level etc)
 */

{
        userlist_t    *dummy;

        for( dummy = *l_list; dummy; dummy = dummy->next )
                if( !mymatch( dummy->userhost, userhost, FNM_CASEFOLD ) )
	{
	
		free (dummy->passwd);
		dummy->passwd = strdup(passwd);
		return;
	}		
}

void	show_lvllist(char *pretxt, userlist_t **l_list, char *from, char *userhost, int listtype, int authed)
/*
	List types:
		0 - !whois
		1 - !userlist
		2 - !protlist !shitlist
		3 - !emaillist
		4 - !passwdlist
		5 - !whoami !nwhois
*/
{
	userlist_t	*dummy;

	for( dummy = *l_list; dummy; dummy = dummy->next )
		if(!mymatch(dummy->userhost, userhost, FNM_CASEFOLD) || !*userhost)
		{
			switch(listtype)
			{
				case 0:
					send_to_user(from, "%s%3d | %s | %s",
						pretxt, dummy->access, dummy->userhost, dummy->nick);
					break;
				case 1:
					send_to_user(from, "%s%3d | %-45s | %s",
						pretxt, dummy->access, dummy->userhost, dummy->nick);
					break;
				case 2:
					send_to_user(from, "%s%3d | %s",
						pretxt, dummy->access, dummy->userhost);
					break;
				case 3:
					if (!STRCASEEQUAL(dummy->nick,"None") || !STRCASEEQUAL(dummy->mailaddr,"None"))
					{
						send_to_user(from, "%s%20s | %s",
							pretxt, dummy->nick, dummy->mailaddr);
					}
					break;
				case 4:
					send_to_user(from, "%s%3s | %s",
						pretxt, "-", dummy->userhost);
					break;
				case 5:
					send_to_user(from, "%s%3d | %s | %s | (%c)",
						pretxt, dummy->access, dummy->userhost, dummy->nick, authed?'*':' ');
					break;
			}
		}
}

int	show_lvllist_pattern	(char *pretxt, userlist_t **l_list, char *from, char *userhost, int listtype)
{
	int				match=0;
	userlist_t	*dummy;

	for( dummy = *l_list; dummy; dummy = dummy->next )
	{
		if(!mymatch(userhost, dummy->userhost, FNM_CASEFOLD) || !*userhost)
		{
			if (listtype==0)
				send_to_user(from, "%s%3d | %s | %s",
					pretxt, dummy->access, dummy->userhost, dummy->nick);
			else if (listtype==1)
				send_to_user(from, "%s%3d | %-45s | %s",
					pretxt, dummy->access, dummy->userhost, dummy->nick);
			else if (listtype==2)
				send_to_user(from, "%s%3d | %s",
					pretxt, dummy->access, dummy->userhost);
			else if (listtype==3)
			{
				if (!STRCASEEQUAL(dummy->nick,"None") || !STRCASEEQUAL(dummy->mailaddr,"None"))
				{
					send_to_user(from, "%s%20s | %s",
						pretxt, dummy->nick, dummy->mailaddr);
				}
			}
			else if (listtype==4)
				send_to_user(from, "%s%3s | %s",
					pretxt, "-", dummy->userhost);
			match++;
		}
	}

	return match;
}

void	show_excludelist (char *pretxt, EXCLUDE_list **l_list, char *from, char *userhost)
{
	EXCLUDE_list	*dummy;

	for( dummy = *l_list; dummy; dummy = dummy->next )
		if(!mymatch(dummy->userhost, userhost, FNM_CASEFOLD) || !*userhost)
			send_to_user( from, "%s%s", pretxt, dummy->userhost);
}

int	show_excludelist_pattern (char *pretxt, EXCLUDE_list **l_list, char *from, char *userhost)
{
	int				match=0;
	EXCLUDE_list	*dummy;

	for( dummy = *l_list; dummy; dummy = dummy->next )
	{
		if(!mymatch(userhost, dummy->userhost, FNM_CASEFOLD) || !*userhost)
		{
			send_to_user( from, "%s%s", pretxt, dummy->userhost);
			match++;
		}
	}

	return match;
}

int	write_lvllist(userlist_t **l_list, char *filename)
{
	userlist_t	*dummy;
	FILE			*list_file;
	int			blah = 0;

	if((list_file = fopen("temp.list", "w")) == NULL)
		return FALSE;

	blah += safe_fprintf(list_file, "#############################################\n");
	blah += safe_fprintf(list_file, "## IncuBot %s\n", filename);
	blah += safe_fprintf(list_file, "## Created: %s\n", get_ctime(0));
	blah += safe_fprintf(list_file, "## (c) 1997 Incubus\n");
	blah += safe_fprintf(list_file, "#############################################\n");

	for(dummy = *l_list; dummy; dummy = dummy->next)
	{
		if (dummy->mailaddr == NULL)
			blah += safe_fprintf( list_file, " %40s %3d\n", dummy->userhost, dummy->access);
		else
			blah += safe_fprintf( list_file, " %40s %3d %s %s\n",	dummy->userhost, dummy->access, dummy->mailaddr, dummy->nick);
	}
	blah += safe_fprintf(list_file, "# End of %s\n", filename);
	fclose(list_file);

	if(blah)
		return FALSE;

	unlink(filename);
	if(!rename("temp.list",filename))
		chmod(filename, S_IRUSR | S_IWUSR);
	else
		return FALSE;

	return TRUE;
}

int	write_excludelist (EXCLUDE_list **l_list, char *filename)
{
	EXCLUDE_list	*dummy;
	FILE				*list_file;
	int				blah = 0;

	if((list_file = fopen("temp.list", "w")) == NULL)
		return FALSE;

	blah += safe_fprintf(list_file, "#############################################\n");
	blah += safe_fprintf(list_file, "## IncuBot %s\n", filename);
	blah += safe_fprintf(list_file, "## Created: %s\n", get_ctime(0));
	blah += safe_fprintf(list_file, "## (c) 1997 Incubus\n");
	blah += safe_fprintf(list_file, "#############################################\n");

	for(dummy = *l_list; dummy; dummy = dummy->next)
		blah += safe_fprintf( list_file, " %40s\n", dummy->userhost);

	blah += safe_fprintf(list_file, "# End of %s\n", filename);
	fclose(list_file);

	if(blah)
		return FALSE;

	unlink(filename);

	if(!rename("temp.list", filename))
		chmod(filename, S_IRUSR | S_IWUSR);
	else
		return FALSE;

	return TRUE;
}

int	write_passwdlist(userlist_t **l_list, char *filename)
{
	userlist_t	*dummy;
	FILE			*list_file;
	int			blah = 0;

	if((list_file = fopen("temp.list", "w")) == NULL)
		return FALSE;
 
	blah += safe_fprintf(list_file, "#############################################\n");
	blah += safe_fprintf(list_file, "## IncuBot %s\n", filename);
	blah += safe_fprintf(list_file, "## Created: %s\n", get_ctime(0));
	blah += safe_fprintf(list_file, "## (c)1997 Incubus\n");
	blah += safe_fprintf(list_file, "#############################################\n");

	for(dummy = *l_list; dummy; dummy = dummy->next)
		blah += safe_fprintf(list_file, " %40s %s\n", dummy->userhost, dummy->passwd);
	blah += safe_fprintf(list_file, "# End of %s\n", filename);
	fclose(list_file);

	if(blah)
		return FALSE;

	unlink(filename);

	if(!rename("temp.list", filename))
		chmod(filename, S_IRUSR | S_IWUSR);
	else
		return FALSE;

	return TRUE;

}

