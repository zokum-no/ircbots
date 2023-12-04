#include <stdio.h>
#include <string.h>
#include "config.h"

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





