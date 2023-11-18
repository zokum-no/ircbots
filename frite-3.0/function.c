/* Functions for Some of the ComBot Commands..
 * By OffSpring .. (Stolen by that is ..)
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>

#include "config.h"
#include "channel.h"
#include "userlist.h"
#include "dcc.h"
#include "ftext.h"
#include "misc.h"
#include "vlad-ons.h"

extern ListStruct *SpyMsgList;

char *left(char *string, int i)
{
  static char temp[255];  
  strcpy(temp, string);
  temp[i] = '\0';  
  return temp;
}

int     matches(ma, na)
char *ma, *na;
{   
        register unsigned char m, *mask = (unsigned char *)ma;
        register unsigned char c, *name = (unsigned char *)na;
 
        for (;; mask++, name++)
            {   
                m = tolower(*mask);
                c = tolower(*name);
                if (c == '\0')
                        break;
                if ((m != '?' && m != c) || c == '*')
                        break;
            }        
        if (m == '*')
            {   
                for ( ; *mask == '*'; mask++);
                if (*mask == '\0')
                        return(0);
                for (; *name && matches((char *)mask, (char *)name); name++);
                return(*name ? 0 : 1);
            }        
        else
                return ((m == '\0' && c == '\0') ? 0 : 1);
      }

void bot_reply(char *to, int num)
{ 
        int count = 0;
        char buffer[MAXLEN] = "";
        FILE *fp;
  
        if (!(fp = fopen(REPLYFILE, "r")))
                return;
        for(;count<num;count++)
                readln_from_a_file(fp, buffer);
        fclose(fp);
  
        strcpy(buffer, strip(buffer, ' '));
        send_to_user(to, buffer);
}

void showspymsg(char *person, char *msg)
{ 
        ListStruct *Temp; 
 
        for (Temp = SpyMsgList;Temp;Temp=Temp->next)
                send_to_user(Temp->name, "*%s* %s", getnick(person), msg);
        return;
 }
