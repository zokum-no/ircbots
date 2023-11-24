#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef SYSV
#include <strings.h>
#endif
#include <ctype.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>

#include "channel.h"
#include "config.h"
#include "dcc.h"
#include "debug.h"
#include "ftext.h"
#include "ftp_dcc.h"
#include "misc.h"
#include "parsing.h"
#include "send.h"
#include "session.h"
#include "userlist.h"
#include "vladbot.h"
#include "vlad-ons.h"
#include "mega-ons.h"

extern  botinfo *currentbot;

#ifdef BackUp
void    do_hotlink(char *from, char *to, char *rest)
{
        char    *password;
        char    *subcommand;
        char    *host;
        char    *level;
        char    *extra;

        subcommand = get_token(&rest, " ");

        if (!strcmp(password, currentbot->backuppasswd))
        {
           if (!strcmp(subcommand, "useradd"))
              {
                 host = get_token(&rest, " ");
                 level = get_token(&rest, " ");
                 add_to_levellist(currentbot->lists->opperlist, host, atoi(level));
              }
           if (!strcmp(subcommand, "shitadd"))
              {
                 host = get_token(&rest, " ");
                 level = get_token(&rest, " ");
                 extra = get_token(&rest, " ");
                 add_to_levellist(currentbot->lists->shitlist, host, atoi(level));
                 add_to_passwdlist(currentbot->lists->shitlist2, host, extra);
              }
           if (!strcmp(subcommand, "passwdadd"))
              {
                 host = get_token(&rest, " ");
                 extra = get_token(&rest, " ");
                 add_to_passwdlist(currentbot->lists->passwdlist, host, extra);
              }
           if (!strcmp(subcommand, "protadd"))
              {
                 host = get_token(&rest, " ");
                 level = get_token(&rest, " ");
                 add_to_levellist(currentbot->lists->protlist, host, atoi(level));
              }
           if (!strcmp(subcommand, "userdel"))
              {
                 host = get_token(&rest, " ");
                 remove_from_levellist(currentbot->lists->opperlist, host);
              }
           if (!strcmp(subcommand, "shitdel"))
              {
                 host = get_token(&rest, " ");
                 remove_from_levellist(currentbot->lists->shitlist, host);
                 remove_from_levellist(currentbot->lists->shitlist2, host);
              }
           if (!strcmp(subcommand, "protdel"))
              {
                 host = get_token(&rest, " ");
                 remove_from_levellist(currentbot->lists->protlist, host);
              }
           if (!strcmp(subcommand, "userwrite"))
                 write_lvllist(currentbot->lists->opperlist, currentbot->lists->opperfile);
           if (!strcmp(subcommand, "shitwrite"))
              {
                 write_lvllist(currentbot->lists->shitlist, currentbot->lists->shitfile);
                 write_lvllist(currentbot->lists->shitlist2, currentbot->lists->shitfile2);
              }
           if (!strcmp(subcommand, "protwrite"))
                 write_lvllist(currentbot->lists->protlist, currentbot->lists->protfile);
        }
        else
           send_to_user(from, "Invalid syntax. Die, bitch. =]");
}

#endif
