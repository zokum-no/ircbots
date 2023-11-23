#include "syserr.h"
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#ifndef BOT_FNAME
#define BOT_FNAME "incubot.new"
#endif

void main ()
{
	int pid, *status, trash;
	int pfd[2];
	char fdstr[10], *time_string;
	FILE *logfile;
	time_t T;

	pfd[0] = 0;
	printf("beggining the process\n");
	while(1)
	{
		if (pipe(pfd) == -1)
			syserr("pipe");
		switch (pid = fork())
		{
			case -1:
				syserr("fork");
			case 0:
				if (close(pfd[1]) == -1)
					syserr("close");
				if (close(pfd[0]) == -1)
					syserr("close2");
				sprintf(fdstr, "%d", pfd[0]);
				execl(BOT_FNAME, BOT_FNAME, fdstr, NULL);
				syserr("execl");
		}
		printf("process ID [%d]\n", pid);
		wait(&status);
		logfile = fopen("maint.log", "a+");
		if ((T = time((time_t *)NULL)) != -1)
		{
			time_string = ctime(&T);
			*(time_string + strlen(time_string) -1) = '\0';
		}
		fprintf(logfile, "[Maintainer] %s Bot ID # [%s] died... Restarting\n", time_string, fdstr);
		fclose(logfile);
	}
}


