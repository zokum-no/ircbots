#include <stdio.h>

void syserr(char *msg) /* print system call error message and terminate */
{
  extern int errno, sys_nerr;
/*  extern char *sys_errlist[];
*/
  fprintf(stderr, "ERROR: %s (%d", msg, errno);
  if (errno > 0 && errno < sys_nerr)
    fprintf(stderr, "; %s)\n", sys_errlist[errno]);
  else
    fprintf(stderr, ")\n");
  exit(1);
}


