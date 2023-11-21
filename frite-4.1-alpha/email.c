/* Ok .. Here is a nice module that will send a user a file through email
   I hope this works ..

   - OffSpring
     cracker@cati.CSUFresno.EDU
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>
#include <stdarg.h>

#include "config.h"
#include "vladbot.h"
#include "vlad-ons.h"
#include "send.h"
#include "misc.h"
#include "email.h"

extern botinfo *currentbot;

struct
{
       char	*name;          /* name   */ 
       char	*desc;          /* desc.  */
       char	*file;		/* actuall filename */
} email_stuff[] = 
{
   {"UnderNet.FAQ", "Frequent Questions and Answers of the UnderNet",
	"/usr/tmp/files/undernet.faq"},
   {"UserCom.FAQ", "Information about the UnderNet User Commitiee",
	"/usr/tmp/files/UserCom.FAQ"},
   /*
    : Add Files here .. :)
    : 
    */
   {NULL,NULL,NULL}
};

void do_email(char *from, char *to, char *rest)
{
     int i;
     char    *userstr, *blah;
     int     num;
        char    *from_copy;

        strcpy(from_copy, from);

  if (rest==NULL)
  {
     send_to_user(from, " Name         / Description");
     send_to_user(from, "--------------------------------------");
     for(i=0;email_stuff[i].name;i++)
     {
       if (email_stuff[i].name != NULL)
          send_to_user(from, " %15s / %s", email_stuff[i].name,
             email_stuff[i].desc);
     }
     send_to_user(from, "--------------------------------------");
     send_to_user(from, "To get a file, /msg %s %cmail <File.Name>",
       currentbot->nick, currentbot->cmdchar);
       return;
  }
  else
  {
     for(i=0;email_stuff[i].name;i++)
     {
       if(STRCASEEQUAL(email_stuff[i].name, rest))
       {
           if((userstr = myuh(from))==NULL) {
	      sendreply("Cant get your hostname ..");
	      return;
     	   }
            send_to_user( from, "Mailing file %s to %s ..",
                email_stuff[i].name, userstr);
            strcpy(blah, "mail ");
            strcat(blah, userstr);
            strcat(blah, " < ");
            strcat(blah, email_stuff[i].file);
            system(blah);
            return;
       }
     }
       sendreply("No such file. For a list, %cmail", currentbot->cmdchar);
   }
}    
