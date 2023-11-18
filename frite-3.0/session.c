/*
 * session.c - manages sessions between IRC users and the bot
 * Copyright (C) 1993-94 VladDrac (irvdwijk@cs.vu.nl)
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
 *
 * A session is created:
 * - When a user starts a DCC chat connection
 * - When a user sends a legal command (public or private)
 * - When a user sends an illegal private message (for floodcontrol)
 *
 * A session is deleted when the user has been idle for a defined
 * amount of time.
 *
 * Sessions can be used for "remembering" the cwd (with filetransfer),
 * last used command, floodingcontrol etc.
 */

#include <stdarg.h>
#include "vladbot.h"
#include "vlad-ons.h"
#include "session.h"
#include "config.h"
#include "misc.h"
		
extern	botinfo	*currentbot;
/*
 * No need to put this in a botstruct, it's resetted by every 
 * incoming message.
 */
SESSION_list	*currentsession;

SESSION_list	*find_session(char *user)
{
	SESSION_list	*session;

	for(session = currentbot->session_list; session; session = session->next)
		if(STRCASEEQUAL(user, session->user))
			return session;
	return NULL;
}

SESSION_list	*create_session(char *user)
/*
 * Initializes a session_struct and places it in the list
 */
{
	SESSION_list	*new_session;

	if((new_session = (SESSION_list *) 
	                   malloc(sizeof(SESSION_list))) == NULL)
		return NULL;
	mstrcpy(&new_session->user, user);
	mstrcpy(&new_session->cwd, "/");
	new_session->nodcc_to = NULL;
	new_session->nodcc_rest = NULL;
	new_session->currentfile = 0;
	new_session->maxfile = 0;
	new_session->last_received = time(NULL);
	new_session->flood_start = time(NULL);
	new_session->flood_cnt = 1;			/* one msg */
	new_session->flooding = FALSE;
	new_session->next = currentbot->session_list;
	currentbot->session_list = new_session;
	return new_session;
}

int	delete_session(char *user)
/*
 * Removes session from list
 */
{
	SESSION_list	**session;
	SESSION_list	*old;
	int	i;
	
	for(session = &currentbot->session_list; *session; 
	    session = &(**session).next)
		if(STRCASEEQUAL((**session).user, user))
		{
			old = *session;
			*session = old->next;
			free(old->user);
			free(old->cwd);
			if(old->maxfile != 0)
				for(i=0; i<old->maxfile; i++)
					free(old->file_list[i]);
			free(old);
			return SUCCESS;
		}
	return FAIL;
}

/*
 * So far for the basic stuff. Now some real work :)
 */

void	cleanup_sessions()
/*
 * Allows flooding users that have been quiet long enough
 * to talk again and removes old sessions
 */
{
	SESSION_list	*session;

	for(session = currentbot->session_list; session; 
	    session = session->next)
	{
		if((time(NULL) - session->last_received) > SESSION_TIMEOUT)
			delete_session(session->user);
		else if(((time(NULL) - session->flood_start) > FLOOD_TALKAGAIN)
			&& session->flooding)
		{
			session->flooding = FALSE;
			session->flood_cnt = 0;
			send_to_user(session->user, "You may speak again");
		}
	}
}

int	check_session(char *user)
/*
 * Creates a new entry if non is found, checks for flooding
 * and gathers some information (has dcc etc.)
 */
{
	if((currentsession = find_session(user)) == NULL)
		currentsession = create_session(user);
	if(check_flood(user))
		return IS_FLOODING;
	currentsession->last_received = time(NULL);
	return IS_OK;
}

int	check_flood(char *user)
/*
 * Checks if this user is flooding (PUBLIC or PRIVATE. WALLOPS and NOTICES
 * are always ignored).
 * If so, this function takes appropriate action and returns TRUE so the
 * calling function knows it should ignore the text.
 *
 * Three possibilities:
 * - User is unknown to me, so he cannot be flooding -> create entry
 * - User is known to me and is already marked flooding -> ignore
 * - User is know to me and is not flooding -> look if flooding now
 */
{
	if((currentsession = find_session(user)) == NULL)
	{	
		currentsession = create_session(user);
		return FALSE;
	}
	else if(currentsession->flooding)
	{
		currentsession->last_received = time(NULL);
		return TRUE;
	}	
	else
	{
		currentsession->last_received = time(NULL);
		currentsession->flood_cnt++;
		if(time(NULL) - currentsession->flood_start <= FLOOD_INTERVAL)
		{	/* user is flooding */
			if(currentsession->flood_cnt < FLOOD_RATE)
				return FALSE;
			send_to_user(user, "I'm gonna ignore you for a while, LAMER!");
			currentsession->flooding = TRUE;
			return TRUE;
		}
		else
		{	/* reset values */
			currentsession->flood_start = time(NULL);
			currentsession->flood_cnt = 1;
			currentsession->flooding = FALSE;
			return FALSE;
		}
	}
}

void	nodcc_session(char *from, char *to, char *rest)
{
	if((currentsession = find_session(from)) == NULL)
		currentsession = create_session(from);

	currentsession->last_received = time(NULL);
	mstrcpy(&currentsession->nodcc_to, to);
	mstrcpy(&currentsession->nodcc_rest, rest);
}

void	hasdcc_session(char *from)
{
	if((currentsession = find_session(from)) == NULL)
		currentsession = create_session(from);
	else if(currentsession->nodcc_to)
		on_msg(currentsession->user, 
		       currentsession->nodcc_to, 
		       currentsession->nodcc_rest);
	free(currentsession->nodcc_to);
	free(currentsession->nodcc_rest);
	currentsession->nodcc_to = currentsession->nodcc_rest = NULL;
	currentsession->last_received = time(NULL);
}

/* 
 * some "ftp" commands 
 */
char	*get_cwd(char *user)
{
	if((currentsession = find_session(user)) == NULL)
		currentsession = create_session(user);
	currentsession->last_received = time(NULL);
	return currentsession->cwd;
}

void	change_dir(char *user, char *new_dir)
{
	if((currentsession = find_session(user)) == NULL)
		currentsession = create_session(user);
	free(currentsession->cwd);
	mstrcpy(&currentsession->cwd, new_dir);
	currentsession->last_received = time(NULL);
}

void	dcc_sendnext(char *user)
{
        if((currentsession = find_session(user)) == NULL)
		return;

	currentsession->last_received = time(NULL);
	if(currentsession->maxfile == 0)
		return;

	if(currentsession->currentfile == currentsession->maxfile)
	{
		int	i;

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
	
}

int	dcc_sendlist(char *user, char *path, char *file_list[], int n)
/*
 * Sends all files in file_list[] (located in path) which contains n 
 * entries to user "user" one-by-one!
 * If internally the maxfile != 0, it means there are already files
 * being queued and the function will return 0, else 1
 *
 */
{
	int	i,j;
	int	files_to_copy;

	if((currentsession = find_session(user)) == NULL)
	                currentsession = create_session(user);

	currentsession->last_received = time(NULL);
	files_to_copy = (currentsession->maxfile+n>LISTSIZE)?
	                (LISTSIZE-currentsession->maxfile):
			n;
	sendreply("%d File%s added to filequeue", files_to_copy,
		  files_to_copy==1?"":"s");

	/* copy the filelist */
	for(i=0, j=currentsession->maxfile; i<files_to_copy; i++, j++)
	{
		mstrcpy(&(currentsession->file_list[j].path), path);
		mstrcpy(&(currentsession->file_list[j].name), file_list[i]);
	}
	currentsession->maxfile += files_to_copy;
	if(currentsession->currentfile == 0)
		dcc_sendnext(user);
	return TRUE;
}

void	do_showqueue()
{
	int	i;

	sendreply("Maxfile = %d, currentfile = %d", currentsession->maxfile,
		   currentsession->currentfile);
	sendreply("Filequeue:");
	for(i=0; i<currentsession->maxfile; i++)
		sendreply("path: %s, file %s", currentsession->file_list[i].path,
					       currentsession->file_list[i].name);
}

/*
 * additional functions
 */
void	sendreply(char *s, ...)
{
	va_list	msg;
	char	buf[WAYTOBIG];

	va_start(msg, s);
	vsprintf(buf, s, msg);
	send_to_user(currentsession->user, buf);
	va_end(msg);
}
