/*
	Spy code by PosterBoy :)
*/

#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <strings.h>
#include <string.h>
#include <stddef.h>
#include "config.h"
#include "spy.h"
#include "userlist.h"
#include "send.h"
#include "function.h"
#include "vlad-ons.h"
#include "channel.h"
#include "misc.h"
extern ListStruct *SpyMsgList;

int add_spy_chan(char *name, char *nick)
{
        CHAN_list *Channel;
        if ( (Channel = search_chan( name )) != NULL)
                return add_to_list(&(Channel->spylist), nick);
        return FALSE;
}
int del_spy_chan(char *name, char *nick)
{
        CHAN_list *Channel;
        if ( (Channel = search_chan( name )) != NULL)
                return remove_from_list(&(Channel->spylist), nick);
        return FALSE;
}
void show_spy_list(char *from, char *name)
{
   ListStruct *temp;
 
   CHAN_list *Channel;
   if ((Channel = search_chan(name)) != NULL)
     for (temp=Channel->spylist;temp;temp=temp->next)
       send_to_user(from, temp->name);
}
void send_spy_chan(char *name, char *text)
{
        ListStruct *temp;
        CHAN_list *Channel;
        if ((Channel = search_chan( name )) != NULL)
                for (temp=Channel->spylist;temp;temp=temp->next)
                        send_to_user(temp->name, text);
}
int is_spy_chan(char *name, char *nick)
{
        CHAN_list *Channel;
        if ( (Channel = search_chan( name )) != NULL)
                return is_in_list(Channel->spylist, nick);
        return FALSE;
}

void do_spy(char *from, char *to, char *rest)
{
        if (!rest)
        {
                send_to_user(from,
                         "The least you could do is give me a channel name");
                return;
        }        
        if (!mychannel(rest))
        {
                send_to_user(from, "I'm not on that channel");
                return;
        }        
        if (add_spy_chan(rest, from))
                send_to_user(from, "Now spying on %s for you", rest);
        else    
                send_to_user(from, "You are already spying on %s", rest);
        return;
}
 
void do_rspy(char *from, char *to, char *rest)
{
        if (!rest)
        {
                send_to_user(from,
                         "The least you could do is give me a channel name");
                return;
        }
        if (!mychannel(rest))
        {
                send_to_user(from, "I'm not on that channel");
                return;
        }
        if (del_spy_chan(rest, from))
                send_to_user(from, "No longer spying on %s for you", rest);
        else    
                send_to_user(from, "You are not spying on %s", rest);
        return;
}
 
void do_spylist(char *from, char *to, char *rest)
{
        if(!rest)
        {
          bot_reply(from, 8);
          return;
        }
 
        send_to_user(from, "--- Spylist for %s", rest);
        show_spy_list(from, rest);
        send_to_user(from, "--- end of list ---");
}
 
void do_spymsg(char *from, char *to, char *rest)
{
        if (is_in_list(SpyMsgList, getnick(from)))
        {
                send_to_user( from, "Already redirecting messages to you");
                return;
        }
        if (add_to_list(&SpyMsgList, getnick(from)))
                send_to_user( from, "Now redirecting messages to you");
        else    
                send_to_user( from, "Error: Can't redirect you the messages");
        return;
}
void do_nospymsg(char *from, char *to, char *rest)
{
        if (!is_in_list(SpyMsgList, getnick(from)))
        {
                send_to_user( from, "I'm not redirecting messages to you");
                return;
        }
        if (remove_from_list(&SpyMsgList, getnick(from)))
                send_to_user( from, "No longer redirecting messages to you");
        else    
                send_to_user( from, "Error: Can't stop redirecting you the messages");
        return;
}

