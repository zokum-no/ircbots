/* inform command */

#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <strings.h>
#include <string.h>
#include <stddef.h>
#include "config.h"
#include "inform.h"
#include "userlist.h"
#include "send.h"
#include "function.h"
#include "vlad-ons.h"
#include "channel.h"
#include "misc.h"

extern 	ListStruct *InformList;

void show_inform_list(char *from)
{
   ListStruct *temp;
 
     for (temp=InformList;temp;temp=temp->next)
       send_to_user(from, temp->name);
}
void send_inform(char *text)
{
   ListStruct *temp;
     for (temp=InformList;temp;temp=temp->next)
       send_to_user(temp->name, text);
}

void do_inform(char *from, char *to, char *rest)
{
        add_to_list(&InformList, from);
        send_to_user(from, "Now informing of kicks and deops.");
}
 
void do_noinform(char *from, char *to, char *rest)
{
        remove_from_list(&InformList, from);
        send_to_user(from, "No longer spying informing you.");
}
 
void do_informlist(char *from, char *to, char *rest)
{
        send_to_user(from, "--- Inform List ---", rest);
        show_inform_list(from);
        send_to_user(from, "--- end of list ---");
}
 
