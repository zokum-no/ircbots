#ifndef NOTE_H
#define NOTE_H

#include "config.h"

#define MAXNOTELEN	20
#define PUBLICADDR	"PUBLIC"
#define	OPERADDR	"OPERATOR"
#define skipspc(str)    readspaces(str)

typedef	char	*message[MAXNOTELEN];	/* 20 should be plenty for a message */

typedef	struct	note_struct
{
	char	*received_by;
	char	*from;		/* sender of note */
	char	*to;		/* pattern to whom note is */
	char	*subject;
	int	note_id;	/* id to identify note */
	int	finished;	/* true is msg finished */
	long	created;
	message	msg;
	struct	note_struct	*prev;
	struct	note_struct	*next;
} notelist;

notelist *find_unfinished(char *from);
void    readspaces(char **src);
void	note_create(char *from, char *s);
void	note_delete(char *from, char *s);
void	note_show(char *from, char *s);
void	note_view(char *from, char *s);
void	note_addline(char *from, char *s);
void	note_delline(char *from, char *s);
void	note_replaceline(char *from, char *s);
void	note_list(char *from, char *s);
void	note_send(char *from, char *s);
void	note_dump(char *from, char *s);

void	parse_note(char *from, char *to, char *s);
int	dump_notelist();
int	read_notelist();

void	create_note(char *from, char *to, char *subject);
void	freenote(notelist *note);
void	del_note(char *from, int note_id);
void	show_note(char *from, int note_id);
void	view_note(char *from);
void	add_note(char *from, char *text);
void	list_note(char *from);
void	send_note(char *from);
void	check_note(char *from);

#endif /* NOTE_H */
