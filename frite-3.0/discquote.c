#include <stdio.h>
#include "send.h"
#include "channel.h"
#include "config.h"

FILE *infile;
int number=0, i;
char buffer[255];

char *dreadline(FILE *f, char *s);

void do_discquote(char *from, char *to, char *rest)
{
  infile=fopen(QUOTEFILE, "r");
  while(!feof(infile)) {
    readline(infile, buffer);
    number++;
  }
  number=random()%number;
  fclose(infile);
  infile=fopen(QUOTEFILE, "r");
  for(i=0;i<number;i++) readline(infile, buffer);
  if(ischannel(to))
  sendprivmsg(to, "%s", buffer);
  else
  sendprivmsg(currentchannel(), "%s", buffer);
}

char *dreadline(FILE *f, char *s)
{
        if(fgets(s,255,f))
                return s;
        return NULL;
}


void do_disctopic(char *from, char *to, char *rest)
{
  infile=fopen(QUOTEFILE, "r");
  while(!feof(infile)) {
    readline(infile, buffer);
    number++;
  }
  number=random()%number;
  fclose(infile);
  infile=fopen(QUOTEFILE, "r");
  for(i=0;i<number;i++) readline(infile, buffer);
  if(ischannel(to))
  send_to_server("TOPIC %s :%s", to, buffer);
  else
  send_to_server("TOPIC %s :%s", currentchannel(), buffer);
}

