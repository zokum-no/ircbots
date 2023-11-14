/* Copyright (C) 1991, 1992, 1993 Free Software Foundation, Inc.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifdef HAVE_CONFIG_H
#if defined (CONFIG_BROKETS)
/* We use <config.h> instead of "config.h" so that a compilation
   using -I. -I$srcdir will use ./config.h rather than $srcdir/config.h
   (which it would do because it found this file in $srcdir).  */

#include <config.h>
#else
#include "config.h"
#endif
#endif

#include <errno.h>
#include "fnmatch.h"
#include <ctype.h>


/* Comment out all this code if we are using the GNU C Library, and are not
   actually compiling the library itself.  This code is part of the GNU C
   Library, but also included in many other GNU distributions.  Compiling
   and linking in this code is a waste when using the GNU C library
   (especially if it is a shared library).  Rather than having every GNU
   program understand `configure --with-gnu-libc' and omit the object files,
   it is simpler to just do this in the source for each such file.  */


typedef unsigned char u_char;

#if defined (_LIBC) || !defined (__GNU_LIBRARY__)

#ifndef MSDOS
#if !defined(__GNU_LIBRARY__) && !defined(STDC_HEADERS)
extern int errno;
#endif
#endif /* MSDOS */

/* Match STRING against the filename pattern PATTERN, returning zero if
   it matches, nonzero if not.  */
int fnmatch (pattern, string, flags)
const char *pattern;
const char *string;
int flags;
{
  register const char *p = pattern, *n = string;
  register char c;

/* Note that this evalutes C many times.  */
#define FOLD(c) ((flags & FNM_CASEFOLD) && isupper (c) ? tolower (c) : (c))

  while ((c = *p++) != '\0')
    {
      c = FOLD (c);

      switch (c)
	{
	case '?':
	  if (*n == '\0')
	    return FNM_NOMATCH;
	  else if ((flags & FNM_FILE_NAME) && *n == '/')
	    return FNM_NOMATCH;
	  else if ((flags & FNM_PERIOD) && *n == '.' &&
		   (n == string || ((flags & FNM_FILE_NAME) && n[-1] == '/')))
	    return FNM_NOMATCH;
	  break;

	case '\\':
	  if (!(flags & FNM_NOESCAPE))
	    {
	      c = *p++;
	      c = FOLD (c);
	    }
	  if (FOLD (*n) != c)
	    return FNM_NOMATCH;
	  break;

	case '*':
	  if ((flags & FNM_PERIOD) && *n == '.' &&
	      (n == string || ((flags & FNM_FILE_NAME) && n[-1] == '/')))
	    return FNM_NOMATCH;

	  for (c = *p++; c == '?' || c == '*'; c = *p++, ++n)
	    if (((flags & FNM_FILE_NAME) && *n == '/') ||
		(c == '?' && *n == '\0'))
	      return FNM_NOMATCH;

	  if (c == '\0')
	    return 0;

	  {
	    char c1 = (!(flags & FNM_NOESCAPE) && c == '\\') ? *p : c;
	    c1 = FOLD (c1);
	    for (--p; *n != '\0'; ++n)
	      if ((c == '[' || FOLD (*n) == c1) &&
		  fnmatch (p, n, flags & ~FNM_PERIOD) == 0)
		return 0;
	    return FNM_NOMATCH;
	  }

	case '[':
	  {
	    /* Nonzero if the sense of the character class is inverted.  */
	    register int not;

	    if (*n == '\0')
	      return FNM_NOMATCH;

	    if ((flags & FNM_PERIOD) && *n == '.' &&
		(n == string || ((flags & FNM_FILE_NAME) && n[-1] == '/')))
	      return FNM_NOMATCH;

	    not = (*p == '!' || *p == '^');
	    if (not)
	      ++p;

	    c = *p++;
	    for (;;)
	      {
		register char cstart = c, cend = c;

		if (!(flags & FNM_NOESCAPE) && c == '\\')
		  cstart = cend = *p++;

		cstart = cend = FOLD (cstart);

		if (c == '\0')
		  /* [ (unterminated) loses.  */
		  return FNM_NOMATCH;

		c = *p++;
		c = FOLD (c);

		if ((flags & FNM_FILE_NAME) && c == '/')
		  /* [/] can never match.  */
		  return FNM_NOMATCH;

		if (c == '-' && *p != ']')
		  {
		    cend = *p++;
		    if (!(flags & FNM_NOESCAPE) && cend == '\\')
		      cend = *p++;
		    if (cend == '\0')
		      return FNM_NOMATCH;
		    cend = FOLD (cend);

		    c = *p++;
		  }

		if (FOLD (*n) >= cstart && FOLD (*n) <= cend)
		  goto matched;

		if (c == ']')
		  break;
	      }
	    if (!not)
	      return FNM_NOMATCH;
	    break;

	  matched:;
	    /* Skip the rest of the [...] that already matched.  */
	    while (c != ']')
	      {
		if (c == '\0')
		  /* [... (unterminated) loses.  */
		  return FNM_NOMATCH;

		c = *p++;
		if (!(flags & FNM_NOESCAPE) && c == '\\')
		  /* XXX 1003.2d11 is unclear if this is right.  */
		  ++p;
	      }
	    if (not)
	      return FNM_NOMATCH;
	  }
	  break;

	default:
	  if (c != FOLD (*n))
	    return FNM_NOMATCH;
	}

      ++n;
    }

  if (*n == '\0')
    return 0;

  if ((flags & FNM_LEADING_DIR) && *n == '/')
    /* The FNM_LEADING_DIR flag says that "foo*" matches "foobar/frobozz".  */
    return 0;

  return FNM_NOMATCH;
}

#endif  /* _LIBC or not __GNU_LIBRARY__.  */

/*
** matches()
**
** Iterative matching function, rather than recursive.
** Written by Douglas A Lewis (dalewis@acsu.buffalo.edu)
*/

int	matches(ma, na)
char	*ma, *na;
{
	int	wild = 0, q = 0;
	u_char	*m = (u_char *)ma, *n = (u_char *)na;
	u_char  *mask = (u_char *)ma;
	

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
			if (*m)
				m++;
			if (*n)
				n++;
		    }
	    }
}


