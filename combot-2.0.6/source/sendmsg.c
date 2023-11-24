#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include "config.h"

void main()
{
	char lin[HUGE];
	FILE *in, *out;
	int thepid;

	if ((in = fopen(PIDFILE, "rt")) != NULL)
	{
		fscanf(in, "%i", &thepid);
		fclose(in);
		do
		{
			gets(lin);
			if (*lin && (out = fopen(MSGFILE, "wt")))
			{
				fprintf(out, "%s\n", lin);
				fclose(out);
				kill(thepid, SIGALRM);
			}
			else if (*lin)
			{
				printf("Couldn't open the file\n");
				exit(0);
			}
		} while (*lin);
	}
}
		

