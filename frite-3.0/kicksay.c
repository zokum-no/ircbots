#include "misc.h"
#include "note.h"
#include "session.h"
#include "userlist.h"
#include "vladbot.h"
#include "vlad-ons.h"
#include "send.h"
#include "function.h"
#include "kicksay.h"

extern ListStruct *KickSays, *KickSayChan; /* kick say -pb */
extern int kslevel;

void do_addks(char *from, char *to, char *rest)
{
        char ks[MAXLEN];
        if (!rest)
        {
                send_to_user( from, "Specify a string of text to kick upon when said");
                return;
        }
        strcpy(ks, "*");
        strcat(ks, rest);
        strcat(ks, "*");
        if (is_in_list(KickSays, ks))
        {
                send_to_user( from, "Already kicking on the say of that");
                if (KickSayChan == NULL)
                        send_to_user( from, "WARNING: No channels selected for auto-kick");
                return;
        }
        if (add_to_list(&KickSays, ks))
        {
                send_to_user( from, "Now kicking on the say of %s", ks);
                if (KickSayChan == NULL)
                        send_to_user( from, "WARNING: No channels selected for auto-kick");
        }
        else
                send_to_user( from, "Problem adding the kicksay");
        return;
}


void do_addksc(char *from, char *to, char *rest)
{
        if (!rest || !ischannel(rest))
        {
          bot_reply(from, 10);
          return;
        }
        if (!mychannel(rest) && (*rest != '*'))
          {
            bot_reply(from, 11);
            return;
          }
        if (is_in_list(KickSayChan, rest))
        {
                send_to_user( from, "Already auto-kicking on that channel");
                return;
        }
        if (add_to_list(&KickSayChan, rest))
                send_to_user( from, "Now auto-kicking on %s", rest);
        else
                send_to_user( from, "Problem adding the channel");
        return;
}


void do_clrks(char *from, char *to, char *rest)
{
        if (KickSays == NULL)
        {
                send_to_user( from, "There are no kicksays");
                return;
        }
        remove_all_from_list( &KickSays);
        send_to_user( from, "All kicksays have been removed");
        return;
}

void do_clrksc(char *from, char *to, char *rest)
{
        if (KickSayChan == NULL)
        {
                send_to_user( from, "There are no kicksay channels");
                return;
        }
        remove_all_from_list( &KickSayChan);
        send_to_user( from, "All channels have been removed");
        return;
}
void do_delks(char *from, char *to, char *rest)
{
        char ks[MAXLEN];
        if (!rest)
        {
                send_to_user( from, "Specify the string of text to delete");
                return;
        }
        strcpy(ks, rest);
        if (!is_in_list(KickSayChan, ks))
        {
                send_to_user( from, "I'm not auto-kicking on that channel");
                return;
        }
        if (remove_from_list(&KickSayChan, ks))
                send_to_user( from, "The channel has been deleted");
        else
                send_to_user( from, "Problem deleting the channel");
        return;
}
void do_listks(char *from, char *to, char *rest)
{
        ListStruct *temp;
        int i = 0;
        if (KickSays == NULL)
        {
                send_to_user( from, "There are currently no kicksays");
                return;
        }
        send_to_user( from, "--- List of Kicksays: ---" );
        for (temp = KickSays; temp; temp = temp->next)
        {
                i++;
                send_to_user( from, "%2i -- %s", i, temp->name);
        }
        send_to_user( from, "--- End of list ---");
        return;
}
void do_listksc(char *from, char *to, char *rest)
{
        ListStruct *temp;
        int i = 0;
        if (KickSayChan == NULL)
        {
                send_to_user( from, "There are currently no kicksay channels");
                return;
        }
        send_to_user( from, "--- List of Kicksay channels: ---" );
        for (temp = KickSayChan; temp; temp = temp->next)
        {
                i++;
                send_to_user( from, "%2i -- %s", i, temp->name);
        }
        send_to_user( from, "--- End of list ---");
        return;
}

void do_delksc(char *from, char *to, char *rest)
{
        char ks[MAXLEN];
        if (!rest)
        {
          bot_reply(from, 10);
          return;
        }
        strcpy(ks, rest);
        if (!is_in_list(KickSayChan, ks))
        {
                send_to_user( from, "I'm not auto-kicking on that channel");
                return;
        }
        if (remove_from_list(&KickSayChan, ks))
                send_to_user( from, "The channel has been deleted");
        else
                send_to_user( from, "Problem deleting the channel");
        return;
}

