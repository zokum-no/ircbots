#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "config.h"

extern	char	current_nick[MAXLEN];
static	char	ftext[MAXLEN];
static	char	fword[MAXLEN];
static	char	escape[MAXLEN];


char	*read_word( ffile )
FILE	*ffile;
/*
 * Pre: ffile is opened, word points to allocated memory (string)
 * Post: read_word returns word
 *
 * reads from file until ' ' or '\n' and returns word.
 * assumes no eof() is encountered
 */
{
	char	c;

	*fword='\0';
	while(1)
	{	
		c=fgetc(ffile);
		switch(c)
		{
		case ' ': 
			return(fword);
		case '\n': 
			return(fword);
		default:
			sprintf(fword, "%s%c", fword, c);
			break;
		}	
	}
}

void	translate_escape( s, c )
char	*s;
char	c;
/*
 * Translates an escaped character c using to following rules:
 * 	c	return
 *      \	\
 *	#	#
 *	b	^B
 *	u	^_
 *	i	^V
 *	n	bot's nick
 *	v	version
 */

{
	switch(c)
	{
	case 'b':
		strcat(s, "\x2");
		break;
	case 'u':
		strcat(s, "\x1f");
		break;
	case 'i':
		strcat(s, "\x16");
		break;
	case 'n':
		strcat(s, current_nick);
		break;
	case 'v':
		strcat(s, VERSION);
		break;
	case '\\':
		strcat(s, "\\");
		break;
	case '#':
		strcat(s, "#");
                break;
	default:
		printf("Unknown escape sequence \'%c\'\n", c);
		break;
	}
}
	
void skipcomment( FILE *ffile )
/*
 * Read characters from the file until eoln.
 */
{
	while(fgetc(ffile)!='\n')
		;
}

int  find_topic( FILE *ffile, char *topic )
/*
 * moves the filepointer until the beginning of topic "topic".
 * returns 1 if topic found, else 0
 */
{
	char	c;
	int	esc;
	char	command[MAXLEN];

	while(!feof(ffile))
	{
		c=fgetc(ffile);
		switch(c)
		{
		case '\\':
			esc=TRUE;
			break;			
		case '#':
			if(!esc)
				skipcomment(ffile);
			else
				esc=FALSE;
			break;
		case '%':
			if(!esc)
			{
				strcpy(command, read_word(ffile));
				if(!strcasecmp(command, "subject"))
					if(!strcasecmp(read_word(ffile), topic))
						return(1);
			}
			else
				esc=FALSE;
			break;
		default:
			esc=FALSE;
			break;
		}
	}
	return(0);
}

char *get_ftext( FILE *ffile )
/*
 * Pre: ffile has been opened
 * Post: get_ftext returns NULL on eof, or else a parsed string
 */
{
	char	command[MAXLEN];
	char	c;
	
	strcpy(ftext, "");

	while(!feof(ffile))
	{
		c=fgetc(ffile);
		switch(c)
		{
		case '%':
			/* we read a command */
			strcpy(command, read_word(ffile));
			if(!strcasecmp(command, "end"))
				return(NULL);
			if(!strcasecmp(command, "line"))
			{
				strcat(ftext, "   ");
				return(ftext);
			}
			break;
		case '\\':
			c=fgetc(ffile);
			translate_escape(ftext, c);
			break;	
		case '#':
			skipcomment(ffile);
			break;
		case '\n':
			return(ftext);
			break;
		default:
			sprintf(ftext, "%s%c", ftext, c);
			break;
		}	
	}
	return(NULL);
}

