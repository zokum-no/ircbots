#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void main(int argc, char *argv[])
{
	char buffer[400];
	char nick[400];
	char filename[400];
	char useradd_notify[400];

	if(argc != 4)
	{
		printf("Invalid parameters\n");
		return;
	}

	strcpy(nick, argv[1]);
	strcpy(filename,argv[2]);
	strcpy(useradd_notify,argv[3]);

	if(access(filename,0))
	{
		return;
	}

	sprintf(buffer,"cat %s | mail -s \"Event list for %s\" `cat %s`",
	filename,nick, useradd_notify);
	system(buffer);
	remove(filename);

}

