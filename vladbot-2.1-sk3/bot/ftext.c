/*
 * ftext.c - formatted text for VladBot (filelist and help)
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

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <strings.h>
#include <assert.h>

#include "vladbot.h"
#include "misc.h"
#include "config.h"

#ifndef	STAND_ALONE
extern	botinfo	*currentbot;
#endif
static	char	ftext[MAXLEN];
static	char	fword[MAXLEN];


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

	assert(ffile != NULL);

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
		strcat(s, "\002");
		break;
	case 'u':
		strcat(s, "\031");
		break;
	case 'i':
		strcat(s, "\022");
		break;
	case 'n':
#ifdef STAND_ALONE
		strcat(s, "(botname)");
#else
		strcat(s, currentbot->nick);
#endif
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
	
void	skipcomment( ffile )
FILE	*ffile;
/*
 * Read characters from the file until eoln.
 */
{
	assert(ffile != NULL);
	while(fgetc(ffile)!='\n')
		;
}

int	find_topic( ffile, topic )
FILE	*ffile;
char	*topic;
/*
 * moves the filepointer until the beginning of topic "topic".
 * returns 1 if topic found, else 0
 */
{
	char	c;
	int	esc = FALSE;
	char	command[MAXLEN];

	assert(ffile != NULL);

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
				if(STRCASEEQUAL(command, "subject"))
					if(STRCASEEQUAL(read_word(ffile), topic))
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

char	*get_ftext( ffile )
FILE	*ffile;
/*
 * Pre: ffile has been opened
 * Post: get_ftext returns NULL on eof, or else a parsed string
 */
{
	char	command[MAXLEN];
	char	c;

	assert(ffile != NULL);

	strcpy(ftext, "");

	while(!feof(ffile))
	{
		c=fgetc(ffile);
		switch(c)
		{
		case '%':
			/* we read a command */
			strcpy(command, read_word(ffile));
			if(STRCASEEQUAL(command, "end"))
				return(NULL);
			if(STRCASEEQUAL(command, "line"))
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
