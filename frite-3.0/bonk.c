#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>

#include "channel.h"
#include "config.h"
#include "dcc.h"
#include "debug.h"
#include "ftext.h"
#include "ftp_dcc.h"
#include "misc.h"
#include "note.h"
#include "session.h"
#include "userlist.h"
#include "vladbot.h"
#include "vlad-ons.h"
#include "bonk.h"

FILE *bonkfile;
char bonklist[200][100];
int number, i, bonkflag=1;

int power();

void init_bonk(bonkfilename)
char *bonkfilename;
{
  bonkfile=fopen(bonkfilename, "r");
  for(number=0;!feof(bonkfile);number++) {
    fscanf(bonkfile, "%s", bonklist[number]);
    for(i=0;i<strlen(bonklist[number]);i++) 
      if(bonklist[number][i]=='_') bonklist[number][i]=' ';
  }
  fclose(bonkfile);
  srandom((int) (getpid()*random()));
}

void	do_bonk(char *from, char *to, char *rest)
{
	int i;

	i=random()%(number);
	if(bonkflag) {
	  if(rest)
	    if(ischannel(to))
	      if((bonklist[i][0]=='a')||(bonklist[i][0]=='e')||
		 (bonklist[i][0]=='i')||(bonklist[i][0]=='o')||
		 (bonklist[i][0]=='u'))
		send_ctcp(to, "ACTION bonks %s with an %s!", rest, bonklist[i]);
	      else
		send_ctcp(to, "ACTION bonks %s with a %s!", rest, bonklist[i]);
	    else
	      if((bonklist[i][0]=='a')||(bonklist[i][0]=='e')||
		 (bonklist[i][0]=='i')||(bonklist[i][0]=='o')||
		 (bonklist[i][0]=='u'))
		send_ctcp(currentchannel(), "ACTION bonks %s with an %s!", rest, bonklist[i]);
	      else
		send_ctcp(currentchannel(), "ACTION bonks %s with a %s!", rest, bonklist[i]);
	  else
	    send_to_user(from, "I don't know who to bonk");
	}
	else send_to_user(from, "Sorry, bonking has been turned off");
        return;
}

void do_bonk_off(char *from, char *to, char *rest)
{
  bonkflag=0;
  send_to_user(from, "Bonking off.");
}

void do_bonk_on(char *from, char *to, char *rest)
{
  bonkflag=1;
  send_to_user(from, "Bonking on.");
}

int power(a, b)
int a,b;
{
  int k, j=a;

  if(b==0) return(1);
  for(k=1;k<b;k++) a=a*j;
  return(a);
}
