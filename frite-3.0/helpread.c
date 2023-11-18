/*
 * helpread.c - small utility to read help outside IRC
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
#include "ftext.h"
#include "config.h"

void	show_help( topic )
char	*topic;

{
	int 	i, j;
	FILE	*f;
	char	*s;

	f=fopen("help.bot", "r");
	
	if(!find_topic( f, topic ))
		printf("No help available for \"%s\"\n", topic);
	else
		while(s=get_ftext(f))
			printf("%s\n", s);
	fclose(f);
}

int	main(argc, argv)
int	argc;
char	**argv;

{
	int	i;

	for(i=1; i<argc; i++)
		show_help(argv[i]);
}
