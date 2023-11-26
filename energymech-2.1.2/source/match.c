/*
 * match.c - match functions
 */

#include <ctype.h>
#include <stdio.h>

#include "global.h"
#include "h.h"

/*
** matches()
**
** Iterative matching function, rather than recursive.
** Written by Douglas A Lewis (dalewis@acsu.buffalo.edu)
*/

int nummatch;

int	matches(ma, na)
char	*ma, *na;
{
	int	wild = 0, q = 0;
	u_char	*m = (u_char *)ma, *n = (u_char *)na;
	u_char  *mask = (u_char *)ma;
	

        nummatch = 0;
	if (!m || !n)
          return 1;
	while (1)
	    {
		if (!*m)
		    {
	  		if (!*n)
				return 0;
	  		for (m--; (*m == '?') && (m>mask); m--)
				;
			if ((*m == '*') && (m>mask) && (m[-1]!='\\'))
				return 0;
			if (wild) 
			    {
				m = (u_char *)ma;
				n = (u_char *)++na;
			    }
			else
				return 1;
		    }
		else if (!*n)
		    {
			while(*m == '*')
				m++;
			return (*m != 0);
		    }
      
	      if (*m == '*')
		{
			while (*m == '*')
				m++;
			wild = 1;
			ma = (char *)m;
			na = (char *)n;
		 }
		
		if ((*m == '\\') && ((m[1] == '*') || (m[1] == '?')))
		    {
			m++;
			q = 1;
		    }
		else
			q = 0;

		if ((tolower(*m) != tolower(*n)) && ((*m != '?') || q))
		    {
			if (wild)
			    {
				m = (u_char *)ma;
				n = (u_char *)++na;
			    }
			else
				return 1;
		    }
		else
		    {
			nummatch++;
			if (*m)
				m++;
			if (*n)
				n++;
		    }
	    }
}

int num_matches(spec, what)
char *spec;
char *what;
{
	if (!matches(spec, what))
		return nummatch+1;
	return 0;
}


