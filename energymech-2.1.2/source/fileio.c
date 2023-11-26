/*
 * fileio.c - file io functions
 * (c) 1995 CoMSTuD (comstud@texas.net)
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
#include <string.h>

#include "global.h"
#include "config.h"
#include "defines.h"
#include "structs.h"
#include "h.h"

int  freadln(stream, lin)
FILE *stream;
char *lin;
{
	char *p;

		do
			p = fgets(lin, MAXLEN, stream);
		while (p && (*lin == '#'));

		if (!p)
			return FALSE;

		terminate(lin, "\n\r");
/*      p = stripl(lin, " ");
		strcpy(lin, p);        */

	return TRUE;
}

int find_topic(in, lin)
FILE *in;
char *lin;
{
  while (freadln(in, lin) && lin && (*lin != ':') && !feof(in))
    ;
  if (feof(in))
    return 0;
  return (lin != NULL);
}

