#include <stdio.h>
#include "send.h"
#include "channel.h"
#include "config.h"
#include "ctcp.h"
#include "vlad-ons.h"
#include "splatk.h"

FILE *infile;
int number3=0, i;
char buffer[255];

void do_splatk(char *from, char *to, char *rest)
{
        char    *nuh;
	char *tmp;

  infile=fopen(SPLATFILE, "r");
  while(!feof(infile)) {
    readline(infile, buffer);
    number3++;
  }
  number3=random()%number3;
  fclose(infile);
  infile=fopen(SPLATFILE, "r");
  for(i=0;i<number3;i++) readline(infile, buffer);

        if(rest && ((nuh = username(rest)) != NULL))
                if((protlevel(username(rest))>=100)&&
                   (shitlevel(username(rest))==0))
                        send_to_user(from, "%s is protected!", rest);
                else if(ischannel(to))
                        sendkick(to, rest, buffer);
                else    
			sendkick(currentchannel(), rest, buffer);
        else    
		bot_reply(from, 4);
}
