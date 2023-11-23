/*
 * parsing.c - some general string parsing routines
 * Copyright (C) 1993/94 VladDrac (irvdwijk@cs.vu.nl)
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

#include <ctype.h>
#include <string.h>
#include "parsing.h"
#include "misc.h"
#include "config.h"

#define is_special(char)	(strchr("-[]\\`^{}|", (char)) != NULL)
#define isvalid(c) (((c) >= 'A' && (c) <= '~') || isdigit(c) || (c) == '-')

int	readint(char **src, int *dest)
/*
 * int ::= number { number }
 *
 */
{
	char	result[MAXLEN];
	char	*s = result;

	skipspc(src);
	if(not(**src && (isdigit(**src) || **src == '-')))
		return FALSE;
	if(**src == '-')
	{
		*s = '-';
		s++; (*src)++;
		if(not(isdigit(**src)))
			return FALSE;
	}
	
	while(**src && isdigit(**src))
	{
		*s = **src;
		s++; (*src)++;
	}
	*dest = atoi(result);
	return TRUE;
}

int	readstring(char **src, char *dest)
/*
 * string ::= '"' { character } '"'
 * where character can be any character except ", unless it's
 * escaped.
 *
 * NOTE: The string will be copied to dest WITHOUT THE QUOTES!
 */
{
	skipspc(src);
	if(**src != '"')
		return FALSE;
	(*src)++;
	while(**src && **src != '"')
	{
		if(**src == '\\')
		{
			(*src)++;
			if(not(**src))
				return FALSE;
			switch(**src)
			{
			case 'b':
				*dest = '\002';	/* bold */
				break;
			case 'u':
				*dest = '\022';
				break;
			case 'i':
				*dest = '\031';
				break;
			default:
				/* i.e. \\ or " */
				*dest = **src;
				break;
			}
		}
		else
			*dest = **src;
		dest++; (*src)++;
	}
	if(**src == '"')
	{
		(*src)++;
		*(dest++) = '\0';
		return TRUE;
	}
	return FALSE;
}

int	readident(char **src, char *dest)
/*
 * ident ::= letter { letter | number | special_char }
 * special_char ::= '_' | '@' | '.'
 */
{
	skipspc(src);
	if(not(**src && isalpha(**src)))
		return FALSE;

	while(**src && (isdigit(**src) || isalpha(**src) || 
	                **src == '@' || **src == '_'))
	{
		*dest = **src;
		dest++; (*src)++;
	}
	*(dest++) = '\0';
	return TRUE;
}

int	readboolean(char **src, int *dest)
/*
 * boolean ::= "TRUE" | "FALSE" | "true" | "false"
 *
 * dest will be set to either 1 or 0, according to the boolean value
 */
{
	skipspc(src);
	if(strncasecmp(*src, "true", 4) == 0)
	{
		(*src) += 4;
		*dest = TRUE;
		return TRUE;
	}
	if(strncasecmp(*src, "false", 5) == 0)
	{
		(*src) += 5;
		*dest = FALSE;
		return TRUE;
	}
	return FALSE;
}

void	readspaces(char **src)
/*
 * skips all spaces, tabs and newlines in *src
 */
{
	while(**src && (**src == ' ' || **src == '\t' || **src == '\n'))
		(*src)++;
}

int	readnick(char **src, char *dest)
{
	skipspc(src);

	if(!(**src) || **src == '-' || isdigit(**src))
		return FALSE;

	while(**src && isvalid(**src))
	{
		*dest = **src;
		dest++; (*src)++;
	}
	*dest = '\0';
	return TRUE;
}

int	readchannel(char **src, char *dest)
{
	skipspc(src);
	if(not(**src) || not(**src == '#' || **src == '&'))
		return FALSE;
	*dest = **src;
	dest++; (*src)++;
	while(**src && (**src != ' ' && **src != '\t' && **src != '\007' && **src != ','
	            && **src != '\n' && **src != '\013'))
	{
		*dest = **src;
		dest++; (*src)++;
	}
	*dest = '\0';
	return TRUE;
}
