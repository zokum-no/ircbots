/* Sing code by PosterBoy ..
   User types: ~sing grateful dead
  lyrics would be in file: grateful dead.song
*/

#include <stdio.h>
#include "send.h"
#include "channel.h"
#include "config.h"
#include "sing.h"
extern char *ownernick;
char *sfx;
FILE *infile;
int singnum=0, i;
char buffer[255];

void do_lyrics(char *from, char *to, char *rest)
{
if (rest)
{
	if((infile = fopen( LYRICFILE, "a")) == NULL)
	send_to_user(from, "Saving lyrics .. %s will add them later .. Thanks!", ownernick);
	fprintf(infile, "%s: %s\n", getnick(from), rest);
}
else
{
	send_to_user(from, "Please enter a band name and lyrics.");
	send_to_user(from, "ex: ~lyrics Meat Puppets: Whatever they sing");
}
}
void do_sing(char *from, char *to, char *rest)
{
char *sfx = ".song";
char *tmp;

	sprintf(tmp, "%s%s", rest, sfx);
	infile=fopen(tmp, "r");

     if(!infile)
     {
     send_to_user(from, "Sorry, I don't know that group. Use ~lyrics to gimme some lyrics.");
     send_to_user(from, "Make sure to type the name in all lowercase letters ..");
     return;
     }
	    while(!feof(infile)) {
	    readline(infile, buffer);
	    singnum++;
  }
  singnum=random()%singnum;
  fclose(infile);
  for(i=0;i<singnum;i++) readline(infile, buffer);
  if(ischannel(to)) sendprivmsg(to, "%s", buffer);
  else
  sendprivmsg(currentchannel(), "%s", buffer);
}
