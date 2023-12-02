/*******************************************/
/*    File: hof-dccparty.c 		   */
/*  Author: HOF (jpoon@mathlab.sunysb.edu) */
/* Purpose: Commands for HOFbot            */
/* Version: 2.0B		           */
/*    Date: June, 1996			   */
/*******************************************/
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <strings.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "dcc.h"
#include "function.h"
#include "debug.h"
#include "hofbot.h"
#include "config.h"
#include "memory.h"

extern  botinfo *current;
extern  int     idletimeout;
extern  int     waittimeout;
extern  int     maxuploadsize;

void show_party(char *from)
{
	DCC_list **temp;
	int i;

	send_to_party(from, "I've linked to following hofbots:");
	for (temp = &(current->Client_list), i = 1; *temp; temp = &((*temp)->next), i++)
	   if ((*temp)->partyline == 2 && ((*temp)->flags & DCC_ACTIVE)) 
	      send_to_party(from, "\002%d: %s \002", i, (*temp)->user);
        send_to_party(from, " ");
	send_to_party(from, "======== PartyLine host/guest List ========="); 
	for (temp = &(current->Client_list), i = 1; *temp; temp = &((*temp)->next), i++)
	   if ((*temp)->partyline == 1 && ((*temp)->flags & DCC_ACTIVE)) {
	      send_to_party(from, "\002%d: %s \002", i, (*temp)->user);
	      send_to_party(from, "Current nick: %s", (*temp)->partynick);
	      send_to_party(from, "Idle: %s", 
			idle2str(getthetime() - (*temp)->lasttime));
	      if (*((*temp)->awaymsg) != '\0')
			  send_to_party(from, "%s is away: %s", (*temp)->partynick,
					(*temp)->awaymsg);
	   }

	send_to_party(from, "================ End of List ==============="); 
}

void exit_party(char *from)
/*      By: HOF (jpoon@mathlab.sunysb.edu)
      Date: Feb. 1996
   Purpose: Remove a user from the partyline.
*/
{
	DCC_list **temp;
	int lost = 0;

	for (temp = &(current->Client_list); *temp; temp = &((*temp)->next)) 
	{
	   if ((*temp)->partyline) 
		   if (!my_stricmp(from, (*temp)->user)) {
		        (*temp)->flags |= DCC_DELETE;
			lost = 1;
			break;
		   }
        }
	if (!lost)
		return;
	else
		for (temp = &(current->Client_list); *temp; temp = &((*temp)->next)) 
		{
		   if ((*temp)->partyline) 
		      send_to_party((*temp)->user, "*** %s has left the partyline", from);
		}
}

void newnick_party(char *from, char *nick)
/*      By: HOF (jpoon@mathlab.sunysb.edu)
      Date: Feb. 1996
   Purpose: just like /nick <newnick>  on irc.
*/
{
	DCC_list **temp;
	char oldnick[20], newnick[20];


	for (temp = &(current->Client_list); *temp; temp = &((*temp)->next)) 
	{
                if (!my_stricmp(from, (*temp)->user)) {
			strcpy(oldnick, (*temp)->partynick);
	   		strncpy((*temp)->partynick, nick, 20);
			strncpy(newnick, nick, 20);
		}
        }

	for (temp = &(current->Client_list); *temp; temp = &((*temp)->next)) 
	{
	   if ((*temp)->partyline)
               send_to_party((*temp)->user, "*** \x2%s\x2 is now known as \x2%s\x2", oldnick, newnick); 
        }
}

void whois_party(char *from, char *nick)
/*      By: HOF (jpoon@mathlab.sunysb.edu)
      Date: Feb. 1996
   Purpose: just like /nick <newnick>  on irc.
*/
{
	DCC_list **temp;

	for (temp = &(current->Client_list); *temp; temp = &((*temp)->next)) 
	{
                if (!my_stricmp(nick, (*temp)->partynick))
		{
	   		send_to_party(from, "*** %s is %s", nick, 
					(*temp)->user);
			send_to_party(from, "*** Idle: %s", 
			idle2str(getthetime() - (*temp)->lasttime));
			if (*((*temp)->awaymsg) != '\0')
				send_to_party(from, "*** %s is away: %s", nick, 
					(*temp)->awaymsg);
			return;
	         }
        }
	send_to_party(from, "*** %s is not in partyline", nick);
}

void kick_party(char *from, char *nick, char *reason)
/*      By: HOF (jpoon@mathlab.sunysb.edu)
      Date: Feb. 1996
   Purpose: kick someone off partyline
*/
{
	DCC_list **temp;
	char kicker[20];
	int kicked = 0;

	if (!reason)
	    strcpy(reason, "LAMER OUT");

	for (temp = &(current->Client_list); *temp; temp = &((*temp)->next)) 
	{
		if (!my_stricmp(from, (*temp)->user))
		{
			strcpy(kicker, (*temp)->partynick);
			break;
		}
	}
	for (temp = &(current->Client_list); *temp; temp = &((*temp)->next)) 
	{
                if (!my_stricmp(nick, (*temp)->partynick))
		{
	   		send_to_party((*temp)->user, "*** You have been kicked off the partyline by %s (%s)",
					kicker, reason);
			(*temp)->flags |= DCC_DELETE;
			kicked = TRUE;
			break;
	         }
        }
	if (!kicked)
		send_to_party(from, "*** %s is not in partyline", nick);
	else
		for (temp = &(current->Client_list); *temp; temp = &((*temp)->next)) 
                   if (my_stricmp(nick, (*temp)->partynick))
			send_to_party((*temp)->user, "*** %s has been kicked off the partyline by %s (%s)", 
				nick, kicker, reason);
}

void setaway_party(char *from, char *rest)
/*      By: HOF (jpoon@mathlab.sunysb.edu)
      Date: Feb. 1996
   Purpose: just like /away [<text>]  on irc.
*/
{
	DCC_list **temp;
	char nick[20];
	int away = 0;

	for (temp = &(current->Client_list); *temp; temp = &((*temp)->next)) 
	{                
		if (!my_stricmp(from, (*temp)->user))
		{
		     away = 1;
		     strcpy(nick, (*temp)->partynick);
		     if (!rest) {
			   *((*temp)->awaymsg) = '\0';
			   send_to_party((*temp)->user, "*** You are no longer marked as being away");
		     }
	             else {
		        strcpy((*temp)->awaymsg, rest);
  		        send_to_party((*temp)->user, "*** You have been marked as being away");
		     }
		     break;
	   	}
        }

	if (away) {
		for (temp = &(current->Client_list); *temp; temp = &((*temp)->next)) 
		{                
			if (!my_stricmp(from, (*temp)->user))
				continue;
		        if (!rest) 
			   send_to_party((*temp)->user, "*** %s is back", nick);
	                else
	   		   send_to_party((*temp)->user, "*** %s is away: %s", nick, rest);
	   	}
        }
}

void shutdown_party(char *from)
{
	DCC_list **temp;

	for (temp = &(current->Client_list); *temp; temp = &((*temp)->next)) 
			send_to_party((*temp)->user, "*** %s IS SHUTTING DOWN... (Authorized by: %s)", current->nick, from);
	close_all_dcc();	
}

void changedcc_party(char *from, char *nick)
/*      By: HOF (jpoon@mathlab.sunysb.edu)
      Date: June 1996
   Purpose: Change user's dcc chat status
*/
{
	DCC_list **temp;

	for (temp = &(current->Client_list); *temp; temp = &((*temp)->next)) 
	{
                if (!my_stricmp(nick, (*temp)->partynick))
		{
	   		send_to_party(from, "*** %s is %s", nick, 
					(*temp)->user);
			send_to_party(from, "*** Idle: %s", 
			idle2str(getthetime() - (*temp)->lasttime));
			if (*((*temp)->awaymsg) != '\0')
				send_to_party(from, "*** %s is away: %s", nick, 
					(*temp)->awaymsg);
			return;
	         }
        }
	send_to_party(from, "*** %s is not in partyline", nick);
}

void change_party(char *from, int level)
/*      By: HOF (jpoon@mathlab.sunysb.edu)
      Date: June, 1996
   Purpose: change user's current dcc chat status.
*/
{
	DCC_list **temp;

	for (temp = &(current->Client_list); *temp; temp = &((*temp)->next)) 
	{
                if (!my_stricmp(getnick(from), (*temp)->partynick))
			if ((*temp)->partyline != level) {
				(*temp)->partyline = level;
				send_to_user(from, "\x2%s %i\x2", 
				"*** your dcc status has changed to ", level);
				return;
			}
	}
	send_to_user(from, "*** your dcc status has not been changed");
}
