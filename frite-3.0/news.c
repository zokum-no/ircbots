
/* oh no .. not a news file :) */
/* 
		I got it Holo! :)
   			-TF
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
#include "userlist.h"
#include "vladbot.h"
#include "vlad-ons.h"
#include "news.h"

/* mommy mommy i wrote this code all by myself :) */

void    show_news(char *from, char *to, char *rest)
{
    int     i, j;
    FILE    *f;
    char    *s;
 
          f=fopen(NEWSFILE, "r");
            while(s=get_ftext(f))
                send_to_user(from, s);
          fclose(f);
}

