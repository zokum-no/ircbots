/*
 * bonk.c - a funny command that deserves it's own file
 * (c) 1995 PosterBoy (aporter@cati.CSUFresno.EDU)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>

#include "channel.h"
#include "config.h"
#include "autoconf.h"
#include "dcc.h"
#include "debug.h"
#include "ftext.h"
#include "ftp_dcc.h"
#include "misc.h"
#include "note.h"
#include "session.h"
#include "userlist.h"
#include "frite_bot.h"
#include "frite_add.h"
#include "bonk.h"

FILE *bonkfile;
char bonklist[200][100];
int number, i;

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
	CHAN_list *Channel;
	char *chan;
	
	chan = findchannel(to, &rest);

	if (!(Channel = search_chan(chan)))
	{
		not_on(from, chan);
		return;
	}	
	if (!Channel->bonk)
	{
		sendreply("Bonking on %s is off :(", chan);
		return;
	}

	i=rand()%(number);
	  if(rest)
	      if((bonklist[i][0]=='a')||(bonklist[i][0]=='e')||
		 (bonklist[i][0]=='i')||(bonklist[i][0]=='o')||
		 (bonklist[i][0]=='u'))
		send_ctcp(chan, "ACTION bonks %s with an %s!", rest, bonklist[i]);
	      else
		send_ctcp(chan, "ACTION bonks %s with a %s!", rest, bonklist[i]);
	  else
	    sendreply("I don't know who to bonk");
}

int power(a, b)
int a,b;
{
  int k, j=a;

  if(b==0) return(1);
  for(k=1;k<b;k++) a=a*j;
  return(a);
}
