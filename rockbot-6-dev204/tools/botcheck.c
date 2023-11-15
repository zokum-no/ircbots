/*
botcheck.c distributed with rockbot 4.1.  This utility can be used
in conjunction with unix's crontab deamon to automaticaly start a rockbot
(or any other program) when it dies.  Recomended for advanced users only
on more then one ocation errors with my system or my process have resulted
in over a thousand emails in my emailbox.. be warned (:
directions:
  crontab -e will get you the crontab list.. use man crontab for help
on what this does.  basicly the idea is to auto-run a scrip every few
minutes that runs the folowing command line:
ps -x | botcheck RockBot
this program then takes the output of the ps -x, and if it doesnt see
"RockBot" in the list, exicutes RockBot/bots, a script which runs the bot.
if you use another method besides this one, the  botcheck RockBot command
will be in the ps list and this program will think its your bot.. 
i recomend redirecting all output of your crontab line to /dev/null to 
avoid MOST errors from generating an email every time the script is run.
This script is not suported, its just here coz i use it for my own bot..
botcheck is copyright 1995 Steve Havelka distribute freely
*/
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
	if(!botcheck) {
		printf("Process not detected, starting bot..\n");
		system("./RockBot &");
		}
}
