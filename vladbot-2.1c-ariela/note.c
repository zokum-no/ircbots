/*
 * note.c - implementation of a simple messaging system for vladbot
 * Copyright (C) 1993 VladDrac (irvdwijk@cs.vu.nl) 
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

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "fnmatch.h"
#include "note.h"
#include "misc.h"
#include "vladbot.h"

extern	botinfo	*currentbot;
extern	char	*notefile;

extern	int	send_to_user(char *to, char *format, ...);
extern	char	*time2small(long time);
extern	char	*getnick(char *s);
extern	int	userlevel(char *from);
extern	int	shitlevel(char *from);
extern	int	protlevel(char *from);

static	notelist *notestail=NULL;
static	notelist *noteshead=NULL;

static	int	 lastnote = 1;

struct
{
	char	*name;
	void	(*function)(char*, char*);
} note_cmds[] =
{	
	{ "CREATE",	note_create 	},
	{ "DELETE",	note_delete	},
	{ "DEL",	note_delete	},
	{ "SHOW",	note_show	},
	{ "READ",	note_show	},
	{ "VIEW",	note_view	},
	{ "ADDLINE",	note_addline	},
	{ "ADDL",	note_addline	},
	{ "DELLINE",	note_delline	},
	{ "DELL",	note_delline	},
	{ "REPLACELINE",note_replaceline },
	{ "REPLACEL",	note_replaceline },
	{ "REPL",	note_replaceline },
	{ "LIST",	note_list	},
	{ "SEND",	note_send	},
	{ "DONE",	note_send	},
	{ "DUMP",	note_dump	},
	{ NULL,		null(void(*)())	}
};

void	parse_note(char *from, char *to, char *s)
{
	int	i;
	char	*command;

	/* if no command was given, show notelist */

	if(!s || ((command = get_token(&s, " "))==NULL))
	{
		note_list(from, NULL);
		return;
	}

	for(i=0; note_cmds[i].name; i++)
		if(STRCASEEQUAL(note_cmds[i].name, command))
		{
			note_cmds[i].function(from, *s?s:NULL);
			return;
		}
	send_to_user(from, "Unknown note command %s", command);
}

int	dump_notelist()
/*
 * Writes all notes to a file
 */
{
	FILE		*nfile;
	notelist	*tmp;

	if((nfile = fopen(notefile, "w")) == NULL)
		return 0;

	fprintf(nfile, "%d\n", lastnote);
	for(tmp=notestail; tmp; tmp=tmp->prev)
	{
		int 	i;

                fprintf(nfile, "%s\n", tmp->received_by);
		fprintf(nfile, "%s\n", tmp->from);
		fprintf(nfile, "%s\n", tmp->to);
		fprintf(nfile, "%s\n", tmp->subject);
		fprintf(nfile, "%d\n", tmp->note_id);
		fprintf(nfile, "%d\n", tmp->finished);
		fprintf(nfile, "%lu\n", tmp->created);
		/* reserved space (future use) */
		fprintf(nfile, "\n\n\n");
		for(i=0; tmp->msg[i]; i++)
			fprintf(nfile, "%s\n", tmp->msg[i]);
		/* use 4 ^A's to mark end of text */
		fprintf(nfile, "\001\001\001\001\n");
	}
	fclose(nfile);
	return 1;		/* success */		
}

int	read_notelist()
/*
 * Read the entire notelist. This should pnly be done
 * WHEN THE LIST IS EMPTY! I assume the file has the correct
 * format (i.e. created by !note dump)
 */
{
	FILE		*nfile;
	notelist	*tmp;
	char		buf[MAXLEN];
	int		i;

	if((nfile = fopen(notefile, "r")) == NULL)
		return 0;

	fscanf(nfile, "%d", &lastnote);
	/* try to read the "Received by - line,
	   continue if not eof encountered */
	while(fscanf(nfile, "%s", buf) && !feof(nfile))
	{
		tmp = (notelist*)malloc(sizeof(*tmp));
		if(!tmp)
			return;
		mstrcpy(&tmp->received_by, buf);
		fscanf(nfile, "%s", buf);
		mstrcpy(&tmp->from, buf);
		fscanf(nfile, "%s", buf);
		mstrcpy(&tmp->to, buf);
		/* Skip the newline */
		fgets(buf, MAXLEN, nfile);
		fgets(buf, MAXLEN, nfile);
		KILLNEWLINE(buf);
		mstrcpy(&tmp->subject, buf);
		fscanf(nfile, "%d", &tmp->note_id);		
		fscanf(nfile, "%d", &tmp->finished);		
		fscanf(nfile, "%u", &tmp->created);		
		/* Skip the newline and three reserved fields */
		fgets(buf, MAXLEN, nfile);
		fgets(buf, MAXLEN, nfile);
		fgets(buf, MAXLEN, nfile);
		fgets(buf, MAXLEN, nfile);
		for(i=0; !STREQUAL(fgets(buf, MAXLEN, nfile), 
                                "\001\001\001\001\n")!=0; i++)
		{
			KILLNEWLINE(buf);
			mstrcpy(&tmp->msg[i], buf);
		}
		if(i<MAXNOTELEN-1)
			tmp->msg[i] = NULL;
		if(notestail == NULL)
			notestail = tmp;
		tmp->next = noteshead;
		if(tmp->next)
			tmp->next->prev = tmp;
		tmp->prev = NULL; 
		noteshead = tmp;
	}
	fclose(nfile);
	return 1;
}
	
notelist	*find_unfinished(char *from)
/*
 * Find a ('the') note written by 'from' that hasn't
 * been finished. Return NULL is no note found
 *
 * I assume there is not more than one unfinished note
 * in the list (note_create() should check this)
 */
{
	notelist	*tmp;

	/* search backwards.. probalby the fastest result */
	for(tmp=notestail; tmp; tmp=tmp->prev)
		if(!fnmatch(tmp->from, from, FNM_CASEFOLD) && !tmp->finished)
			return tmp;
	return NULL;	
}

/* These functions are called when a notecommand is given.
   They parse the given string and call the apropriate 
   function
*/

void	note_dump(char *from, char *rest)
{
	dump_notelist();
}

void	note_create(char *from, char *s)
{
	char	*to,
		*subject;

	if(s && (to=get_token(&s, " ")))
	{
		subject = s;
		if(!find_unfinished(from))
		{
			send_to_user(from, "Creating note for %s, id %d", 
				     to, lastnote);
			create_note(from, to, subject);
		}
		else
			send_to_user(from, "You haven't finished your previous note!");
	}
	else
		send_to_user(from, "To whom do you want to send this note?");
}

void	note_delete(char *from, char *s)
{
	int	id;

	if(s && (id=atoi(get_token(&s, " "))))
		del_note(from, id);
	else
		send_to_user(from, "Note id expected");
}

void	note_show(char *from, char *s)
{
	int	id;

	if(s && (id=atoi(get_token(&s, " "))))
		show_note(from, id);
	else
		send_to_user(from, "Note id expected");
}

void	note_view(char *from, char *s)
{
	if(find_unfinished(from))
		view_note(from);
	else
		send_to_user(from, "You haven't created a note");
}

void	note_addline(char *from, char *s)
{
	if(find_unfinished(from))
	{
		add_note(from, s);
		send_to_user(from, "Line \"%s\" added to note", s?s:" ");
	}
	else
		send_to_user(from, "You haven't created a note");
}

void	note_delline(char *from, char *s)
{
	send_to_user(from, "Not implemented");
}

void	note_replaceline(char *from, char *s)
{
	send_to_user(from, "Not implemented");
}

void	note_list(char *from, char *s)
{
	list_note(from);
}

void	note_send(char *from, char *s)
{
	if(find_unfinished(from))
		send_note(from);
	else
		send_to_user(from, "You haven't created a note");
}

/* functions that actually do something :) */

void	create_note(char *from, char *to, char *subject)
{
	notelist	*newnote;

	newnote = (notelist*)malloc(sizeof(*newnote));
	if(!newnote)
		return;
	mstrcpy(&newnote->received_by, currentbot->botname);
	mstrcpy(&newnote->from, from);
	mstrcpy(&newnote->to, to);
	if(subject)
		mstrcpy(&newnote->subject, subject);
	else
		mstrcpy(&newnote->subject, "(No Subject)");
	newnote->note_id = lastnote;
	newnote->msg[0] = NULL;
	newnote->created = time(NULL);
	newnote->finished = FALSE;

/* The notelist should be doule-linked (else it'll get messed up when
   writing/reading the file, and I don't feel like sorting :) */
	if(noteshead == NULL)
		noteshead = newnote;
	newnote->prev = notestail;
	if(newnote->prev)
		newnote->prev->next = newnote;
	newnote->next = NULL; 
	notestail = newnote;

	lastnote++;
	return;
}	

void	freenote(notelist *note)
{
	int	i;

	free(note->received_by);
	free(note->from);
	free(note->to);
	free(note->subject);
	for(i=0; note->msg[i] && i<MAXNOTELEN; i++)
		free(note->msg[i]);
	free(note);
}

void	del_note(char *from, int note_id)
{
	notelist	*tmp;

	for(tmp=noteshead; tmp; tmp=tmp->next)
		if(tmp->note_id == note_id)
		{
			if(!fnmatch(tmp->to, from, FNM_CASEFOLD)  ||
			   (userlevel(from) >= 125) ||
			/* user is allowed to delete his own note if it's not
			   finished */
			   (STRCASEEQUAL(tmp->from, from) && !tmp->finished))
			{
				send_to_user(from, "Note %d deleted", note_id);
				/* note is last in list */
				if(noteshead == notestail)
				{
					noteshead = notestail = NULL;
					freenote(tmp);
					return;
				}
				/* note is tail: */
				if(tmp == notestail)
				{
					notestail = tmp->prev;
					if(tmp->prev)
						tmp->prev->next = NULL;
					freenote(tmp);
					return;
				}
				/* note is head */
				if(tmp == noteshead)
				{
					noteshead = tmp->next;
					if(tmp->next)
						tmp->next->prev = NULL;	
					freenote(tmp);
					return;
				}
				/* else */
				tmp->next->prev = tmp->prev;
				tmp->prev->next = tmp->next;
				/* free */
				return;
			}
			else
				send_to_user(from, "You can only delete your own notes");
			return;
		}
	/* note not found */	
	return;
}

void	show_note(char *from, int note_id)
{
	notelist	*tmp;

	for(tmp=notestail; tmp; tmp=tmp->prev)
		if((tmp->note_id == note_id) && tmp->finished)
		{
			if(!fnmatch(tmp->to, from, FNM_CASEFOLD) || 
			   STRCASEEQUAL(tmp->to, PUBLICADDR))
			{
				int 	i;

				send_to_user(from, 
                                          "Received by: %s", tmp->received_by);
				send_to_user(from, "From: %s", tmp->from);
				send_to_user(from, "To: %s", tmp->to);
				send_to_user(from, "Date: %s", 
                                          time2str(tmp->created));
				send_to_user(from, "Subject: %s", tmp->subject);
				send_to_user(from, "  ");

				for(i=0; tmp->msg[i] && i<MAXNOTELEN; i++)
					send_to_user(from, "%s", tmp->msg[i]);
			}
			else
				send_to_user(from, 
                                "You're not allowed to read this message");
			return;
		}
	send_to_user(from, "Note %d not found", note_id);
	return;
}

void	view_note(char *from)
/* Kinda like show_note, but is used to look at your message to be send */
{
	notelist	*tmp;

	if(tmp=find_unfinished(from))
	{
		int 	i;

		send_to_user(from, "Received by: %s", 
			     tmp->received_by);
		send_to_user(from, "From: %s", tmp->from);
		send_to_user(from, "To: %s", tmp->to);
		send_to_user(from, "Date: %s", 
			     time2str(tmp->created));
		send_to_user(from, "Subject: %s", tmp->subject);
		send_to_user(from, "  ");

		for(i=0; tmp->msg[i]; i++)
			send_to_user(from, "[%d] %s", i+1, 
				     tmp->msg[i]);
	}
	else
		/* Should not happen (note_view should check) */
		send_to_user(from, "You haven't created a note");
	return;
}

void	add_note(char *from, char *text)
/* adds a line to a note */
{
	notelist	*tmp;

	if(!text)
		text = " ";
	if(tmp=find_unfinished(from))
	{
		int 	i;

		for(i=0; tmp->msg[i]; i++);
			if(i<MAXNOTELEN-1)
			{
				mstrcpy(&tmp->msg[i], text);
				tmp->msg[i+1] = NULL;
			}
			else
				send_to_user(from, "Note too long!");
	}
	else
		/* Should not happen (note_view should check) */
		send_to_user(from, "You haven't created a note");
	return;
}

void	list_note(char *from)
/* lists all notes that match from */
{
	notelist	*tmp;
	int		num_notes = 0;

	send_to_user(from, "\002Id   From      Subject\002");
	/* display list backwards */
	for(tmp=notestail; tmp; tmp=tmp->prev)
		if((!fnmatch(tmp->to, from, FNM_CASEFOLD)||
                   STRCASEEQUAL(tmp->to, PUBLICADDR)) && tmp->finished)
		{
			send_to_user(from, "%-4d %-9s %s %s", tmp->note_id, 
				     getnick(tmp->from), tmp->subject,
				     STRCASEEQUAL(tmp->to, PUBLICADDR)?"\002PUBLIC\002":"");
			num_notes++;
		}
	if(num_notes == 0)
		send_to_user(from, "No notes for you right now");
}

void	send_note(char *from)
/* Mark the note finished. It can't be changed anymore */
{
	notelist	*tmp;

	if(tmp=find_unfinished(from))
	{
		tmp->finished = TRUE;
		send_to_user(from, "Note sent to %s",
			     tmp->to);
	}
	else
		send_to_user(from, "You haven't created a note");
}
