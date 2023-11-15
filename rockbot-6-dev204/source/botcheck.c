/* This program is copyright 1995 Steve Havelka
   Part of the RockBot archive.  This program
   can be used to check if bots died during 
   some system event.  I am not responsible
   for anything.

   Email: shavelk@agora.rdrop.com
   WWW: http://www.rdrop.com/~shavelk       */

#include <stdio.h>

void main(int argc,char *argv[])
{
	char buf[80];
	int botcheck=0;
	if(argc!=2) {
		fprintf(stderr,"No parameter!\n");
		exit(1);
	}
	while(!feof(stdin)) {
		fgets(buf,80,stdin);
		if(strstr(buf,argv[1]))
			botcheck++;
	}
	if(!botcheck)
		system("bots");
}
