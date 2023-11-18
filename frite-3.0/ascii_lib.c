/* Ok .. Here is my ascii library .. I had it allllll planned out
   but my code drifted away with anubis.org .. So .. Umm .. have fun :)
   Note: All Ascii Pics must have a .pic suffix
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
#include "dcc.h"
#include "debug.h"
#include "ftext.h"
#include "ftp_dcc.h"
#include "misc.h"
#include "note.h"
#include "session.h"
#include "send.h"
#include "userlist.h"
#include "vladbot.h"
#include "vlad-ons.h"
#include "ascii_lib.h"

char *sfx;
FILE *infile;
char *out;

void	do_ascii(char *from, char *to, char *rest)
{
    char *sfx = ".pic";

/*	if(!strcmp("",rest)) */
	if(!rest)
		infile=fopen(LIBFILE, "r");
	else
		infile=fopen(strcat(rest, sfx), "r");
    if(infile)
    {
          while(out=get_ftext(infile))
          send_to_user(from, out);
          fclose(infile);
          return;
    }
	send_to_user(from, "Invalid Pic. /msg fRitE ~ascii for a list.");
	return;
}

