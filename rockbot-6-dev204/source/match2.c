/*
 * **  Compare if a given string (name) matches the given
 * **  mask (which can contain wild cards: '*' - match any
 * **  number of chars, '?' - match any single character.
 * **
 * **      return  0, if match
 * **              1, if no match
 */

/*
 * ** _match()
 * ** Iterative matching function, rather than recursive.
 * ** Written by Douglas A Lewis (dalewis@acsu.buffalo.edu)
 */

static int _match(mask, name)
     char *mask, *name;
{
  Reg1 u_char *m = (u_char *) mask, *n = (u_char *) name;
  char *ma = mask, *na = name;
  int wild = 0, q = 0;

  while (1)
  {
    if (calls++ > MAX_CALLS)
      return 1;
    if (*m == '*')
    {
      while (*m == '*')
	m++;
      wild = 1;
      ma = (char *) m;
      na = (char *) n;
    }

    if (!*m)
    {
      if (!*n)
	return 0;
      for (m--; (m > (u_char *) mask) && (*m == '?'); m--)
	;
      if ((*m == '*') && (m > (u_char *) mask) &&
	  (m[-1] != '\\'))
	return 0;
      if (!wild)
	return 1;
      m = (u_char *) ma;
      n = (u_char *)++ na;
    }
    else if (!*n)
    {
      while (*m == '*')
	m++;
      return (*m != 0);
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
      if (!wild)
	return 1;
      m = (u_char *) ma;
      n = (u_char *)++ na;
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

/*
 * External interfaces to the above matching routine.
 */
int match(ma, na)
     char *ma, *na;
{
  calls = 0;
  return _match(ma, na);
}

int matches(ma, na)
     char *ma, *na;
{
  int r;

  calls = 0;
  return _match(ma, na);
}

/*
 * ** collapse a pattern string into minimal components.
 * ** This particular version is "in place", so that it changes the pattern
 * ** which is to be reduced to a "minimal" size.
 */
char *collapse(pattern)
     char *pattern;
{
  Reg1 char *s = pattern, *s1, *t;

  if (BadPtr(pattern))
    return pattern;
  /*
   * Collapse all \** into \*, \*[?]+\** into \*[?]+
   */
  for (; *s; s++)
    if (*s == '\\')
      if (!*(s + 1))
	break;
      else
	s++;
    else if (*s == '*')
    {
      if (*(t = s1 = s + 1) == '*')
	while (*t == '*')
	  t++;
      else if (*t == '?')
	for (t++, s1++; *t == '*' || *t == '?'; t++)
	  if (*t == '?')
	    *s1++ = *t;
      while ((*s1++ = *t++))
	;
    }
  return pattern;
}
