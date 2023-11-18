#include <stdio.h>
#include "send.h"
#include "channel.h"
#include "config.h"
#include "ctcp.h"
#include "vlad-ons.h"
#include "dance.h"

FILE *infile;
int number2=0, i;
char buffer[255];

void do_dance(char *from, char *to, char *rest)
{
char *tmp;

  infile=fopen(DANCEFILE, "r");
  while(!feof(infile)) {
    readline(infile, buffer);
    number2++;
  }
  number2=random()%number2;
  fclose(infile);
  infile=fopen(DANCEFILE, "r");
  for(i=0;i<number2;i++) readline(infile, buffer);
  send_ctcp(currentchannel(), "ACTION %s", buffer); /* fixed .. -tf */
}
