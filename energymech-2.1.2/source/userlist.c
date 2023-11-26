/*
 * userlist.c - implementation of userlists
 */

/*
#include <stddef.h>
*/
#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "global.h"
#include "config.h"
#include "defines.h"
#include "structs.h"
#include "h.h"

static char tempbuffer[MAXLEN];

void writepartuserlist(l_list, type, fptr)
aUser **l_list;
int type;
FILE *fptr;
{
	register aUser *tmp;
	long int now;

	now = getthetime();
	for (tmp=*l_list;tmp;tmp=tmp->next)
		if (tmp->type & type)
		{
			if ((tmp->type == USERLIST_SHIT) &&
				(now >= tmp->expire))
				continue;
			switch(tmp->type)
			{
				case USERLIST_USER:
				fprintf(fptr,"User= %s\n",tmp->userhost);
				break;
				case USERLIST_SHIT:
				fprintf(fptr,"Shit= %s\n",tmp->userhost);
				break;
				case USERLIST_GREET:
				fprintf(fptr,"Greet= %s\n",tmp->userhost);
				break;
				case USERLIST_PROTNICK:
				fprintf(fptr, "ProtNick= %s\n",tmp->userhost);
				break;
				case USERLIST_KICKSAY:
				fprintf(fptr, "Kicksay= %s\n", tmp->userhost);
				break;
				case USERLIST_SAYSAY:
				fprintf(fptr, "Saysay= %s\n", tmp->userhost);
				break;
			}
			if (tmp->channel && *tmp->channel)
				fprintf(fptr,"\tChannel= %s\n",tmp->channel);
			if (tmp->access)
				fprintf(fptr,"\t Level= %i\n",tmp->access);
			if (tmp->aop)
				fprintf(fptr,"\t   Aop= %i\n",tmp->aop);
			if (tmp->prot)
				fprintf(fptr,"\t  Prot= %i\n",tmp->prot);
			if (tmp->time)
				fprintf(fptr,"\t  Time= %li\n",tmp->time);
			if (tmp->expire)
				fprintf(fptr,"\tExpire= %li\n",tmp->expire);	
			if (tmp->password && *tmp->password)
				fprintf(fptr, "\tPassword= %s\n",tmp->password);
			if (tmp->whoadd)
				fprintf(fptr, "\t  Who= %s\n",tmp->whoadd);
			if (tmp->reason)
				fprintf(fptr, "\t%s= %s\n",
				IsGreet(tmp) || IsSaySay(tmp) || IsKickSay(tmp)
					? "Greeting" : "Reason", tmp->reason);
		}
}

int saveuserlist(l_list, filename)
aUser **l_list;
char *filename;
{
	FILE *ptr;

	if (!(ptr=fopen(current->USERFILE, "wt")))
		return 0;
	writepartuserlist(l_list, USERLIST_USER, ptr);
	fprintf(ptr, "\n");
	writepartuserlist(l_list, USERLIST_SHIT, ptr);
	fprintf(ptr, "\n");
	writepartuserlist(l_list, USERLIST_PROTNICK, ptr);
	fprintf(ptr, "\n");
	writepartuserlist(l_list, USERLIST_GREET, ptr);
	fprintf(ptr, "\n");
	writepartuserlist(l_list, USERLIST_KICKSAY, ptr);
	fprintf(ptr, "\n");
	writepartuserlist(l_list, USERLIST_SAYSAY, ptr);
	fclose(ptr);	
	return 1;
}

int verified(userhost)
char *userhost;
{
	return (find_time(&Auths, userhost) != NULL);
}

int change_password(l_list, userhost, password)
aUser **l_list;
char *userhost;
char *password;
{
	register aUser *user;
	int change;

	change = FALSE;
	if (!userhost)
	  return FALSE;
	for (user = *l_list; user; user = user->next)
		if (IsUser(user) && !matches(user->userhost, userhost))
		{
			change = TRUE;
			MyFree(&user->password);
			mstrcpy(&user->password, password);
		 }
	return change;
}

aUser *check_kicksay(string, channel)
char *string;
char *channel;
{
	register aUser *tmp;

	for(tmp=current->Userlist;tmp;tmp=tmp->next)
	{
		if (!IsKickSay(tmp))
			continue;
		if (!channel || !my_stricmp(channel, tmp->channel) ||
			(*tmp->channel == '*') || (*channel == '*'))
			if (!matches(tmp->userhost, string))
				return tmp;
	}
	return NULL;
}

aUser *find_kicksay(l_list, string, channel)
aUser **l_list;
char *string;
char *channel;
{
        register aUser *tmp, *save;
        register int num, best;

        save = (aUser *) 0;
        num = 0;
        best = 0;
        for(tmp=*l_list;tmp;tmp=tmp->next)
        {
                if (!IsKickSay(tmp))
                        continue;
                if (!channel || !my_stricmp(channel, tmp->channel) ||
                        (*tmp->channel == '*') || (*channel == '*'))
                        num = num_matches(tmp->userhost, string);
                if (num > best)
                {
                        best = num;
                        save = tmp;
                }
        }
        return save;
}

void show_kicksaylist(from, l_list)
char *from;
aUser *l_list;
{
        register aUser *dummy;
	send_to_user(from,
                "--- \002Channel\002 ------- \002String\002 ---------- \002Kick Reason\002 ------");
        for (dummy = current->Userlist; dummy; dummy = dummy->next )
	{
                if (IsKickSay(dummy))
                        send_to_user(from, "%15s %s     %s",
                        	dummy->channel, dummy->userhost,
				dummy->reason);
	}
        send_to_user(from, "--- \002End of kicksay list\002 ---");
}

aUser *check_saysay(string, channel)
char *string;
char *channel;
{
	register aUser *tmp;

	for(tmp=current->Userlist;tmp;tmp=tmp->next)
	{
		if (!IsSaySay(tmp))
			continue;
		if (!channel || !my_stricmp(channel, tmp->channel) ||
			(*tmp->channel == '*') || (*channel == '*'))
			if (!matches(tmp->userhost, string))
				return tmp;
	}
	return NULL;
}

aUser *find_saysay(l_list, string, channel)
aUser **l_list;
char *string;
char *channel;
{
        register aUser *tmp, *save;
        register int num, best;

        save = (aUser *) 0;
        num = 0;
        best = 0;
        for(tmp=*l_list;tmp;tmp=tmp->next)
        {
                if (!IsSaySay(tmp))
                        continue;
                if (!channel || !my_stricmp(channel, tmp->channel) ||
                        (*tmp->channel == '*') || (*channel == '*'))
                        num = num_matches(tmp->userhost, string);
                if (num > best)
                {
                        best = num;
                        save = tmp;
                }
        }
        return save;
}

void show_saysaylist(from, l_list)
char *from;
aUser *l_list;
{
        register aUser *dummy;
	send_to_user(from,
                "--- \002Channel\002 ------- \002String\002 ---------- \002Response\002 ------");
        for (dummy = current->Userlist; dummy; dummy = dummy->next )
	{
                if (IsSaySay(dummy))
                        send_to_user(from, "%15s %s     %s",
                        	dummy->channel, dummy->userhost,
				dummy->reason);
	}
        send_to_user(from, "--- \002End of saysay list\002 ---");
}

aUser *find_user(l_list, userhost, channel)
aUser **l_list;
char *userhost;
char *channel;
{
	register aUser *tmp, *save;
	register int num, best;

	save = (aUser *) 0;
	num = 0;
	best = 0;
	for (tmp=*l_list;tmp;tmp=tmp->next)
	{
		if (!IsUser(tmp))
			continue;
		if (!channel || !my_stricmp(channel, tmp->channel) ||
			(*tmp->channel == '*') || (*channel == '*'))
			num = num_matches(tmp->userhost, userhost);
		if (num > best)
		{
			best = num;
			save = tmp;
		}
	}
	return save;
}

int is_user(userhost, channel)
char *userhost;
char *channel;
{
	aUser *tmp;
	if (!my_stricmp(current->nick, getnick(userhost)))
		return TRUE;
	if (is_a_bot(userhost))
		return TRUE;
	return
	(((tmp=find_user(&(current->Userlist), userhost, channel)) != NULL) &&
		tmp->access);
}

void search_userlist(from, l_list, pattern)
char *from, *pattern;
aUser *l_list;
{
	register aUser *dummy;
	register int count;
 
        count=1;
        send_to_user(from, "Searching for \002%s\002", pattern);
	send_to_user(from,"----------------------------------------------------------------");

 	for (dummy = l_list; dummy; dummy = dummy->next) {
		if(!matches(dummy->userhost, pattern) || !matches(pattern, dummy->userhost))
		{
		if (IsShit(dummy))
			{
			send_to_user(from, "\002 Shit:\002 %30s %11s %3d %9s",
			dummy->userhost, dummy->channel, dummy->access, getnick(dummy->whoadd));
			}
		if (IsUser(dummy))
			{
			send_to_user(from, "\002 User:\002 %30s %10s %10s %4d",
			dummy->userhost, getnick(dummy->whoadd), dummy->channel,
			dummy->access);
			}
		if (IsGreet(dummy))
			{
			send_to_user(from, "\002Greet:\002 %-30s  %s",
				dummy->userhost, dummy->reason);
			}
			count++;
		}
	}
	send_to_user(from,"----------------------------------------------------------------");
	send_to_user(from, "  End of Search for \002%s\002.  Found %d matches.", pattern, count - 1);
	send_to_user(from,"----------------------------------------------------------------");
}

void search_userlist2(from, l_list, pattern)
char *from, *pattern;
aUser *l_list;
{
	register aUser *dummy;
	register int count;
	int t1, t2;
 
        count=1;
        send_to_user(from, "Searching for users with \002%s\002 access", pattern);
	send_to_user(from,"----------------------------------------------------------------");

 	    for (dummy = l_list; dummy; dummy = dummy->next) {
 	    	t1 = dummy->access;
 	    	t2 = atoi(pattern);
			if (t1 == t2)
			{
				if (IsUser(dummy))
					{
					send_to_user(from, " \002User:\002 %30s %10s %10s %4d",
					dummy->userhost, getnick(dummy->whoadd), dummy->channel, dummy->access);
					}
					count++;
				}
		}
	send_to_user(from,"----------------------------------------------------------------");
	send_to_user(from, "  End of Search for \002%s\002.  Found %d matches.", pattern, count - 1);
	send_to_user(from,"----------------------------------------------------------------");
}

void search_userlist3(from, l_list, pattern)
char *from, *pattern;
aUser *l_list;
{
	register aUser *dummy;
	register int count;
 
        count=1;
        send_to_user(from, "Searching for users on \002%s\002", pattern);
	send_to_user(from,"----------------------------------------------------------------");

	if (my_stricmp(pattern, "*")) {
 	for (dummy = l_list; dummy; dummy = dummy->next) {
 		if (my_stricmp(dummy->channel, "*")) {
		if(!matches(dummy->channel, pattern) || !matches(pattern, dummy->channel))
		{
		if (IsShit(dummy))
			{
			send_to_user(from, "\002 Shit:\002 %30s %11s %3d %9s",
			dummy->userhost, dummy->channel, dummy->access, getnick(dummy->whoadd));
			}
		if (IsUser(dummy))
			{
			send_to_user(from, "\002 User:\002 %30s %10s %10s %4d",
			dummy->userhost, getnick(dummy->whoadd), dummy->channel,
			dummy->access);
			}
		if (IsGreet(dummy))
			{
			send_to_user(from, "\002Greet:\002 %-30s  %s",
				dummy->userhost, dummy->reason);
			}
			count++;
		}
		}
	}
	}
	else {
	 for (dummy = l_list; dummy; dummy = dummy->next) {
 		if (!my_stricmp(dummy->channel, "*")) {
		if (IsShit(dummy))
			{
			send_to_user(from, "\002 Shit:\002 %30s %11s %3d %9s",
			dummy->userhost, dummy->channel, dummy->access, getnick(dummy->whoadd));
			}
		if (IsUser(dummy))
			{
			send_to_user(from, "\002 User:\002 %30s %10s %10s %4d",
			dummy->userhost, getnick(dummy->whoadd), dummy->channel,
			dummy->access);
			}
		if (IsGreet(dummy))
			{
			send_to_user(from, "\002Greet:\002 %-30s  %s",
				dummy->userhost, dummy->reason);
			}
			count++;
		}
	}
	}
	send_to_user(from,"----------------------------------------------------------------");
	send_to_user(from,"End of Search for users on \002%s\002.  Found %d matches.", pattern, count - 1);
	send_to_user(from,"----------------------------------------------------------------");
}

aUser *add_to_userlist(l_list, uh, level, aop, prot, chan, password)
aUser **l_list;
char *uh;
int level;
int aop;
int prot;
char *chan;
char *password;
{
	aUser *New_user;

	if (!chan)
	  return NULL;
	New_user = make_user(l_list);
	mstrcpy(&New_user->userhost, uh);
	New_user->access = level;
	New_user->aop    = aop ? 1 : 0;
	New_user->prot   = (prot > MAXPROTLEVEL) ? 5 : (prot < 0) ? 0 : prot;
	mstrcpy(&New_user->channel, chan);
	if (password && *password)
		mstrcpy(&New_user->password, password);
	else
		mstrcpy(&New_user->password, "");
	if (New_user->whoadd) {
		MyFree(&New_user->whoadd);
	}
	mstrcpy(&New_user->whoadd, "Creator!god@heavens.net");
	New_user->type = USERLIST_USER;
	return New_user;
}

void show_userlist(from, l_list)
char *from;
aUser *l_list;
{
    register aUser *dummy;
	register int count;

	count = 1;
        send_to_user(from, "%s",
                "------- Mechs' Userhosts ------- Added By - Channel - Acc AO Prt");
        for (dummy = l_list; dummy; dummy = dummy->next ) {
        	if (IsUser(dummy))
			{
        		send_to_user(from, " User: \002%-36s\002 AddedBy: \002%-10s\002", dummy->userhost, getnick(dummy->whoadd));
				send_to_user(from, " Chan: \002%-15s\002  Access: \002%4d\002 Aop: \002%d\002 Prot: \002%d\002", dummy->channel, dummy->access, dummy->aop, dummy->prot);
        		send_to_user(from, "___________");
				count++;
			}
		}
	send_to_user(from, "      End of userlist. Total of \002%d\002 entries", count - 1);
	send_to_user(from, "--------------------------------------------------------------");
}

void remove_user(l_list, cptr)
aUser **l_list;
aUser *cptr;
{
        if (cptr->prev)
                cptr->prev->next = cptr->next;
        else
        {
                *l_list = cptr->next;
                if (*l_list)
                        (*l_list)->prev = NULL;
        }
        if (cptr->next)
                cptr->next->prev = cptr->prev;
	MyFree(&cptr->whoadd);
	MyFree(&cptr->reason);
        MyFree(&cptr->channel);
        MyFree(&cptr->userhost);
        MyFree(&cptr->password);
        MyFree((char **)&cptr);
}

void delete_all_userlist(l_list)
aUser **l_list;
{
        register aUser *cptr, *tmp;

        cptr = *l_list;
        while (cptr)
        {
                tmp = cptr->next;
                remove_user(l_list, cptr);
                cptr = tmp;
        }
        *l_list = NULL; /* just in case */
}

void delete_userlist(l_list)
aUser **l_list;
{
	register aUser *cptr, *tmp;

        cptr = *l_list;
	while (cptr)
	{
                tmp = cptr->next;
		if (IsUser(cptr))
	                remove_user(l_list, cptr);
                cptr = tmp;
        }
}

int max_userlevel(userhost)
char *userhost;
{
	register aUser *user;
	int access;

	if (!matches(owneruserhost, userhost))
		return OWNERLEVEL;
	if (!my_stricmp(current->nick, getnick(userhost)))
		return OWNERLEVEL;
	access = 0;
	for (user=current->Userlist;user;user=user->next)
		if (IsUser(user) && !matches(user->userhost, userhost))
			access = (user->access > access) ?user->access:access;
	return access;
}

int is_aop(userhost, channel)
char *userhost;
char *channel;
{
        aUser *dummy;

        if (!matches(owneruserhost, userhost))
                return OWNERLEVEL;
        if (!my_stricmp(current->nick, getnick(userhost)))
                return OWNERLEVEL;
        if ((dummy=find_user(&(current->Userlist), userhost, channel)) == NULL)
                return 0;
        return dummy->aop;
}

int get_userlevel(userhost, channel)
char *userhost;
char *channel;
{
	aUser *dummy;
	
	if (!matches(owneruserhost, userhost))
		return OWNERLEVEL;
	if (!my_stricmp(current->nick, getnick(userhost)))
		return OWNERLEVEL;
	if (bot_matches(userhost))
		return OWNERLEVEL;
	if ((dummy=find_user(&(current->Userlist), userhost, channel)) == NULL)
		return 0;
	return dummy->access;
}

int get_protlevel(userhost, channel)
char *userhost;
char *channel;
{
	aUser *dummy;

	if (!matches(owneruserhost, userhost))
		return 4;
	if (!my_stricmp(current->nick, getnick(userhost)))
		return 4;
	if (bot_matches(userhost))
		return 4;
	if ((dummy=find_user(&(current->Userlist), userhost, channel)) == NULL)
		return 0;
	return dummy->prot;
}
	
int password_needed(userhost)
char *userhost;
{
	aUser *dummy;


	if (!(dummy=find_user(&(current->Userlist), userhost, "*")))
		return FALSE;
	if (!dummy->password || !*dummy->password)
		return FALSE;
	return TRUE;
}

int correct_password(userhost, password)
char *userhost;
char *password;
{
	aUser *dummy;

	if (!(dummy=find_user(&(current->Userlist), userhost, "*")))
		return FALSE;    /* or TRUE? */
	if (!dummy->password || !*dummy->password)
		return TRUE;
	if (password && passmatch(password, dummy->password))
		return TRUE;
	return FALSE;
}

aUser *find_shit(l_list, userhost, channel)
aUser **l_list;
char *userhost;
char *channel;
{
	register aUser *save, *user;
	int temp, best;

	if (!userhost)
	  return NULL;
        save = (aUser *) 0;
	temp = 0;
	best = 0;
	for (user = *l_list; user; user = user->next)
	{
		if (!IsShit(user))
			continue;
		if (!channel || !my_stricmp(channel, user->channel) ||
			(*user->channel == '*') || (*channel == '*'))
			temp = num_matches(user->userhost, userhost);
		if (temp > best)
		{
			best = temp;
			save = user;
		}
	}
        return (save && (!save->expire || (save->expire > getthetime()))) ? save : NULL;
}

aUser *add_to_shitlist(l_list, uh, level, chan, from, reason, time, expire)
aUser **l_list;
char *uh;
int level;
char *chan;
char *from;
char *reason;
long int time;
long int expire;
{
	aUser *New_user;

	if ((New_user = find_shit(l_list, uh, chan)) != NULL)
		remove_user(l_list, New_user);
	New_user = make_user(l_list);
	mstrcpy(&New_user->userhost, uh);
	New_user->access = level;
	mstrcpy(&New_user->channel, chan);
	mstrcpy(&New_user->whoadd, from);  
	mstrcpy(&New_user->reason, reason);
	New_user->time = time;
	New_user->expire = expire;
	New_user->type = USERLIST_SHIT;
	return New_user;
}

void show_shitlist(from, l_list)
char *from;
aUser *l_list;
{
	register aUser *dummy;

	send_to_user(from,
		"------- \002Losers' Userhosts\002 ----------- \002Channel\002 - \002Lvl\002 - \002Shitter\002");
	for (dummy = l_list; dummy; dummy = dummy->next )
	{
		if (IsShit(dummy))
		{
			send_to_user( from, "%32s %12s  %3d %10s",
				dummy->userhost, dummy->channel, dummy->access,
				getnick(dummy->whoadd));
		}
	}
	send_to_user(from,
		"--------------------------------------------------------------");
/*	send_to_user(from,
		"   End of lame shitlist. Total of \002%d\002 entries", bleah - 1);
	send_to_user(from,
		"------------------------------------------------------------------");
*/
}

void delete_shitlist(l_list)
aUser **l_list;
{
        register aUser *cptr, *tmp;

        cptr = *l_list;
        while (cptr)
        {
                tmp = cptr->next;
		if (IsShit(cptr))
	                remove_user(l_list, cptr);
                cptr = tmp;
        }
}

aUser *find_protnick(l_list, userhost)
aUser **l_list;
char *userhost;
{
        register aUser *tmp, *save;
        register int num, best;

        save = (aUser *) 0;
        num = 0;
        best = 0;
        for (tmp=*l_list;tmp;tmp=tmp->next)
        {
                if (!IsProtNick(tmp))
                        continue;
                num = num_matches(tmp->userhost, userhost);
                if (num > best)
                {
                        best = num;
                        save = tmp;
                }
        }
        return save;
}

aUser *find_greet(l_list, userhost)
aUser **l_list;
char *userhost;
{
        register aUser *tmp, *save;
        register int num, best;

        save = (aUser *) 0;
        num = 0;
        best = 0;
        for (tmp=*l_list;tmp;tmp=tmp->next)
        {
                if (!IsGreet(tmp))
                        continue;
                num = num_matches(tmp->userhost, userhost);
                if (num > best)
                {
                        best = num;
                        save = tmp;
                }
        }
        return save;
}


void show_greetlist(from, l_list)
char *from;
aUser *l_list;
{
        register aUser *dummy;
        send_to_user(from, "\002%s\002",
                "------ \002Userhost\002 ------- \002Greeting\002 -----");
	for (dummy = current->Userlist; dummy; dummy = dummy->next )
	{
		if (IsGreet(dummy))
	                send_to_user( from, " %-30s  %s",
       	                 dummy->userhost, dummy->reason);
	}
        send_to_user(from, "--- \002End of greet list\002 ---");
}

int is_shitted(userhost, channel)
char *userhost;
char *channel;
{
	aUser *dummy;

	if (!matches(owneruserhost, userhost))
		return FALSE;        /* just in case =) */
	dummy = find_shit(&(current->Userlist), userhost, channel);
	if (!dummy)
		return FALSE;
	return (dummy->expire > getthetime()) ? TRUE : FALSE;
}

long int get_shitexpire(userhost, channel)
char *userhost;
char *channel;
{
	aUser *dummy;

	if (!(dummy = find_shit(&(current->Userlist), userhost, channel)))
		return 0;
	return dummy->expire;
}

int get_shitlevel(userhost, chan)
char *userhost;
char *chan;
{
	aUser *dummy;

	if (!matches(owneruserhost, userhost))
		return 0;        /* just in case =) */

	if (!(dummy=find_shit(&(current->Userlist), userhost, chan)))
		return 0;
	if (dummy->expire > getthetime())
		return dummy->access;
	return 0;
}

char *get_shituserhost(userhost, channel)
char *userhost;
char *channel;
{
	aUser *dummy;
	if (!(dummy = find_shit(&(current->Userlist), userhost, channel)))
		return NULL;
	return dummy->userhost;
}

char *get_shit_reason(userhost, channel)
char *userhost;
char *channel;
{
	aUser *dummy;
	if (!(dummy = find_shit(&(current->Userlist), userhost, channel)))
		return NULL;
	return (dummy->reason && *dummy->reason) ? dummy->reason : "<No reason>";
}


char *get_shituh(dummy)
aUser *dummy;
{
	strcpy(tempbuffer, "");
	if (dummy)
	{
		strcpy(tempbuffer, dummy->userhost);
		if (isnick(getnick(tempbuffer)))
		{
			char *temp, *temp2;
			char tempbuffer2[MAXLEN];
			strcpy(tempbuffer2, tempbuffer);
			temp2 = tempbuffer2;
			temp = get_token(&temp2, "!");
			strcpy(tempbuffer, getnick(dummy->userhost));
			strcat(tempbuffer, "!");
			strcat(tempbuffer, temp2);
		}
	}
	return tempbuffer;
}

char *get_shitreason(dummy)
aUser *dummy;
{
	strcpy(tempbuffer, "\002");
	if (dummy)
	{
		char *crap;
		if (!dummy->whoadd || !*(dummy->whoadd))
			crap="";
		else
			crap = getnick(dummy->whoadd);
		if (!my_stricmp(crap, "<Anonymous>"))
			crap="";	
		if (dummy->time)
		{
			strcat(tempbuffer, time2small(dummy->time));
			if (*crap)
				strcat(tempbuffer, " ");
		}
		if (!*crap)
			strcat(tempbuffer, ": ");
		else
		{
			strcat(tempbuffer, crap);
			strcat(tempbuffer, ": ");
		 }
		if (!dummy->reason)
			strcat(tempbuffer, "GET THE HELL OUT!!!");
		else
			strcat(tempbuffer, dummy->reason);
		strcat(tempbuffer, "\002");
  	}
	else
		strcat(tempbuffer, "YOU ARE SHITLISTED!!!\002");
	return tempbuffer;
}

aList *find_list(l_list, thing)
aList **l_list;
char *thing;
{
	register aList *bleah;
	if (!thing)
	  return NULL;
	for (bleah = *l_list;bleah;bleah=bleah->next)
		if (!my_stricmp(bleah->name, thing))
			  break;
	return bleah;
}

aList *make_list(l_list, thing)
aList **l_list;
char *thing;
{
	aList *New_entry;

	if ((New_entry = find_list(l_list, thing)) != NULL)
	  return New_entry;
	New_entry = (aList *) MyMalloc(sizeof(aList));
	mstrcpy(&New_entry->name, thing);
        New_entry->prev = NULL;
        if (*l_list)
                (*l_list)->prev = New_entry;
        New_entry->next = *l_list;
        *l_list = New_entry;
        return *l_list;
}

aList *find_list2(l_list, thing)
aList **l_list;
char *thing;
{
	register aList *bleah;
	if (!thing)
	  return NULL;
	for (bleah = *l_list; bleah; bleah = bleah->next)
		if (!my_stricmp(getnick(bleah->name), thing))
			  break;
	return bleah;
}

void remove_list(l_list, cptr)
aList **l_list;
aList *cptr;
{
        if (cptr->prev)
                cptr->prev->next = cptr->next;
        else
        {
                *l_list = cptr->next;
                if (*l_list)
                        (*l_list)->prev = NULL;
        }
        if (cptr->next)
                cptr->next->prev = cptr->prev;
        MyFree(&cptr->name);
        MyFree((char **)&cptr);
}

void delete_list(l_list)
aList **l_list;
{
        register aList *cptr, *tmp;

        cptr = *l_list;
        while (cptr)
        {
                tmp = cptr->next;
                remove_list(l_list, cptr);
                cptr = tmp;
        }
        *l_list = NULL;
}

aTime *find_time(l_list, name)
aTime **l_list;
char *name;
{
	register aTime *user;

	if (!name)
	  return NULL;
	for (user = *l_list; user; user = user->next)
	  if (!my_stricmp(name, user->name))
		break;
	return user;
}

aTime *make_time(l_list, name)
aTime **l_list;
char *name;
{
	aTime *New_user;

	if (!name)
	  return FALSE;

	if ((New_user = find_time(l_list, name)) != NULL)
	  return New_user;
	New_user = (aTime *) MyMalloc(sizeof(aTime));
	mstrcpy(&New_user->name, name);
	New_user->num = 1;
	New_user->time = getthetime();
        New_user->prev = NULL;
        if (*l_list)
                (*l_list)->prev = New_user;
        New_user->next = *l_list;
        *l_list = New_user;
        return New_user;
}

void remove_time(l_list, cptr)
aTime **l_list;
aTime *cptr;
{
        if (cptr->prev)
                cptr->prev->next = cptr->next;
        else
        {
                *l_list = cptr->next;
                if (*l_list)
                        (*l_list)->prev = NULL;
        }
        if (cptr->next)
                cptr->next->prev = cptr->prev;
        MyFree(&cptr->name);
        MyFree((char **)&cptr);
}

void delete_time(l_list)
aTime **l_list;
{
        register aTime *cptr, *tmp;

        cptr = *l_list;
        while (cptr)
        {
                tmp = cptr->next;
                remove_time(l_list, cptr);
                cptr = tmp;
        }
        *l_list = NULL;
}

void uptime_time(l_list, thetime)
aTime **l_list;
time_t thetime;
{
        register aTime *cptr, *tmp;

        cptr = *l_list;
        while (cptr)
        {
                tmp = cptr->next;
		if (thetime >= cptr->time)
	                remove_time(l_list, cptr);
                cptr = tmp;
        }
        *l_list = NULL;
}

int write_timelist(l_list, filename)
aTime **l_list;
char *filename;
{
        aTime *dummy;
        time_t    T;
        FILE   *list_file;

        if (!(list_file = fopen(filename, "w")))
                return FALSE;

        T = getthetime();

        fprintf(list_file, "############################################################################\n");
	fprintf(list_file, "## %s\n", filename);
        fprintf(list_file, "## Created: %s", ctime(&T));
        fprintf(list_file, "############################################################################\n");

        for (dummy = *l_list; dummy; dummy = dummy->next)
                fprintf(list_file,
                        " %20s %3d %li\n", dummy->name, dummy->num,
                        dummy->time);
        fprintf(list_file, "## End of %s\n", filename);
        fclose(list_file);
        return TRUE;
}

int read_timelist(l_list, filename)
aTime **l_list;
char *filename;
{
        FILE *in;
        char lin[MAXLEN];
        char *ptr, *nick, *num, *time;
		time_t thetime;
		aTime *Temp;
	
#ifdef DBUG
				debug(NOTICE, "Reading TimeList %s", filename);
#endif

        if (!(in=fopen(filename, "rt")))
                return FALSE;
        delete_time(l_list);
        while (freadln(in, lin))
        {
                ptr = lin;
                nick=get_token(&ptr, ", ");
                num=get_token(&ptr, ", ");
		time=get_token(&ptr, ", ");
		thetime = (time && *time) ? atol(time) : getthetime();
                if (nick && *nick && num)
		{
                        if ((Temp = make_time(l_list, nick)) != NULL)
				Temp->num = atoi(num);
		}
        }
        fclose(in);
        return TRUE;
}

int write_seenlist(l_list, filename)
aSeen **l_list;
char *filename;
{
        aSeen *dummy;
        time_t    T;
        FILE   *list_file;
        char tbuf[MAXLEN];

        if (!(list_file = fopen(filename, "w")))
                return FALSE;

        T = getthetime();

        fprintf(list_file, "############################################################################\n");
		fprintf(list_file, "## %s\n", filename);
        fprintf(list_file, "## Created: %s", ctime(&T));
        fprintf(list_file, "############################################################################\n");

        for (dummy = *l_list; dummy; dummy = dummy->next) {
        	if (dummy->time-getthetime() > (86400 * SEEN_TIME)) {
        		continue;
        	}
        	else {
        		if (!(dummy->selector == 0)) {
					fprintf(list_file, "%10s %50s %50s %li %i %s\n", dummy->nick, dummy->userhost, dummy->kicker, dummy->time, dummy->selector, dummy->signoff);
        		}
        	}
        }
        fprintf(list_file, "## End of %s\n", filename);
        fclose(list_file);
        return TRUE;
}

int read_seenlist(l_list, filename)
aSeen **l_list;
char *filename;
{
        FILE *in;
        char lin[MAXLEN], tbuf[MAXLEN];
        char *ptr, *nick, *userhost, *signoff, *kicker, *time, *selector;
        int theselector;
		time_t thetime;
		aSeen *Temp;
	
#ifdef DBUG
				debug(NOTICE, "Reading SeenList %s", filename);
#endif

        if (!(in=fopen(filename, "rt"))) {
			return FALSE;
        }
        delete_seen(l_list);
        while (freadln(in, lin))
        {
			ptr = lin;
			nick = get_token(&ptr, ", ");
			userhost = get_token(&ptr, ", ");
			kicker = get_token(&ptr, ", ");
			time = get_token(&ptr, ", ");
			selector = get_token(&ptr, ", ");
			signoff = get_token(&ptr, ", ");
			if (signoff) {
                strcpy(tbuf, signoff);
                while (signoff = get_token(&ptr, ", ")) {
                	sprintf(tbuf, "%s %s", tbuf, signoff);
                }
            }
			thetime = (time && *time) ? atol(time) : getthetime();
			theselector = (selector && *selector) ? atoi(selector) : 0;
        	
        	if (nick && *nick) {
        		if ((getthetime() - thetime) > (SEEN_TIME * 86400)) {
        			continue;
        		}
        		else {
					if ((Temp = make_seen(l_list, nick, userhost, kicker, thetime, theselector, tbuf)) != NULL) {
						continue;
					}
				}
			}
        }
        fclose(in);
        return TRUE;
}

void remove_seen(l_list, cptr)
aSeen **l_list;
aSeen *cptr;
{
        if (cptr->prev)
                cptr->prev->next = cptr->next;
        else
        {
                *l_list = cptr->next;
                if (*l_list)
                        (*l_list)->prev = NULL;
        }
        if (cptr->next)
                cptr->next->prev = cptr->prev;
        MyFree(&cptr->nick);
        MyFree(&cptr->userhost);
        MyFree(&cptr->signoff);
        MyFree(&cptr->kicker);
        MyFree((char **)&cptr);
}

void delete_seen(l_list)
aSeen **l_list;
{
        register aSeen *cptr, *tmp;

        cptr = *l_list;
        while (cptr)
        {
                tmp = cptr->next;
                remove_seen(l_list, cptr);
                cptr = tmp;
        }
        *l_list = NULL;
}

aSeen *find_seen(l_list, nick)
aSeen **l_list;
char *nick;
{
	register aSeen *user;

	if (!nick)
	  return NULL;
	for (user = *l_list; user; user = user->next)
	  if (!my_stricmp(nick, user->nick))
		break;
	return user;
}

aSeen *make_seen(l_list, nick, userhost, kicker, time, selector, signoff)
aSeen **l_list;
char *nick;
char *userhost;
char *kicker;
time_t time;
int selector;
char *signoff;
{
	aSeen *New_user;

	if (!nick) {
	  	return NULL;
	}

	if ((New_user = find_seen(l_list, nick)) != NULL) {
		mstrcpy(&New_user->nick, nick);
		mstrcpy(&New_user->userhost, userhost);
		mstrcpy(&New_user->kicker, kicker);
		mstrcpy(&New_user->signoff, signoff);
		New_user->time = time;
		New_user->selector = selector;
	  	return New_user;
	}
	New_user = (aSeen *) MyMalloc(sizeof(aSeen));
	mstrcpy(&New_user->nick, nick);
	mstrcpy(&New_user->userhost, userhost);
	mstrcpy(&New_user->kicker, kicker);
	mstrcpy(&New_user->signoff, signoff);
	New_user->time = time;
	New_user->selector = selector;
        New_user->prev = NULL;
        if (*l_list)
                (*l_list)->prev = New_user;
        New_user->next = *l_list;
        *l_list = New_user;
        return New_user;
}

/* aBigList Stuff */

int write_biglist(l_list, filename)
aBigList **l_list;
char *filename;
{
        aBigList *dummy;
        time_t    T;
        FILE   *list_file;
        char tbuf[MAXLEN];

        if (!(list_file = fopen(filename, "w")))
                return FALSE;

        T = getthetime();

        fprintf(list_file, "############################################################################\n");
		fprintf(list_file, "## %s\n", filename);
        fprintf(list_file, "## Created: %s", ctime(&T));
        fprintf(list_file, "############################################################################\n");

        for (dummy = *l_list; dummy; dummy = dummy->next) {
			fprintf(list_file, "%s %li %i %sÆ%s°\n", dummy->user, dummy->time, dummy->num, dummy->data1, dummy->data2);
        }
        fprintf(list_file, "## End of %s\n", filename);
        fclose(list_file);
        return TRUE;
}

int read_biglist(l_list, filename)
aBigList **l_list;
char *filename;
{
        FILE *in;
        char lin[MAXLEN], tbuf[MAXLEN], tbuf2[MAXLEN], abuf[MAXLEN];
        char *ptr, *user, *data1, *data2, *time, *num, *aptr;
        int thenum;
		time_t thetime;
		aBigList *Temp;
	
#ifdef DBUG
				debug(NOTICE, "Reading aBigList %s", filename);
#endif

        if (!(in=fopen(filename, "rt"))) {
			return FALSE;
        }
        delete_biglist(l_list);       
#ifdef DBUG
				debug(NOTICE, "Reading aBigList %s [1]", filename);
#endif
        while (freadln(in, lin))
        {
			ptr = lin;
			user = get_token(&ptr, " ");
			time = get_token(&ptr, " ");
			num = get_token(&ptr, " ");
			data1 = get_token(&ptr, "Æ");
			data2 = get_token(&ptr, "°");
/*			if (data1) {
                strcpy(tbuf, signoff);
                while (signoff = get_token(&ptr, " ")) {
                	sprintf(tbuf, "%s %s", tbuf, signoff);
                }
            }
*/			
			thetime = (time && *time) ? atol(time) : getthetime();
			thenum = (num && *num) ? atoi(num) : 0;
        	
        	if (user && *user) {
				if ((Temp = make_biglist(l_list, user, thetime, thenum, data1, data2)) != NULL) {
/*					if (isspace(data1[0])) {
						strcpy(abuf, data1++);
						mstrcpy(&Temp->data1, abuf);
					}
					if (isspace(data2[0])) {
						strcpy(abuf, "");
						strcpy(abuf, data2++);
						mstrcpy(&Temp->data2, abuf);
					}
*/					continue;
				}
			}
        }
        fclose(in);
        return TRUE;
}

void remove_biglist(l_list, cptr)
aBigList **l_list;
aBigList *cptr;
{
#ifdef DBUG
				debug(NOTICE, "Removing aBigList [1]");
#endif
        if (cptr->prev)
                cptr->prev->next = cptr->next;
        else
        {
                *l_list = cptr->next;
                if (*l_list)
                        (*l_list)->prev = NULL;
        }
        if (cptr->next)
                cptr->next->prev = cptr->prev;
#ifdef DBUG
				debug(NOTICE, "Removing aBigList [2] %s %s %s %li %li", cptr->user, cptr->data1, cptr->data2, cptr->time, cptr->num);
#endif
        MyFree(&cptr->user);
        MyFree(&cptr->data1);
        MyFree(&cptr->data2);
        MyFree((char **)&cptr);
}

void delete_biglist(l_list)
aBigList **l_list;
{
        register aBigList *cptr, *tmp;
        
#ifdef DBUG
				debug(NOTICE, "Deleting aBigList [1]");
#endif

        cptr = *l_list;
        while (cptr)
        {
                tmp = cptr->next;
                remove_biglist(l_list, cptr);
                cptr = tmp;
        }
        *l_list = NULL;
}

aBigList *find_biglist(l_list, user)
aBigList **l_list;
char *user;
{
	register aBigList *userh;

	if (!user)
	  return NULL;
	for (userh = *l_list; userh; userh = userh->next)
	  if (!my_stricmp(user, userh->user))
		break;
	return userh;
}

aBigList *make_biglist(l_list, user, time, num, data1, data2)
aBigList **l_list;
char *user;
time_t time;
int num;
char *data1;
char *data2;
{
	aBigList *New_user;

	if (!user) {
	  	return NULL;
	}

/* Allow duplicates if num == -20 */
	if (num != -20) {
		if ((New_user = find_biglist(l_list, user)) != NULL) {
			mstrcpy(&New_user->user, user);
			mstrcpy(&New_user->data1, data1);
			mstrcpy(&New_user->data2, data2);
			New_user->time = time;
			New_user->num = New_user->num + 1;
	  		return New_user;
		}
	}
	New_user = (aBigList *) MyMalloc(sizeof(aBigList));
	mstrcpy(&New_user->user, user);
	mstrcpy(&New_user->data1, data1);
	mstrcpy(&New_user->data2, data2);
	New_user->time = time;
	New_user->num = num;
        New_user->prev = NULL;
        if (*l_list)
                (*l_list)->prev = New_user;
        New_user->next = *l_list;
        *l_list = New_user;
        return New_user;
}

/*
 *  structs.c - manages all the structures in the program
 *  -- below here used to be the structs.c file...
 */

aUser *make_user(l_list)
aUser **l_list;
{
        aUser *New_user;

        New_user = (aUser *) MyMalloc(sizeof(aUser));
        
	New_user->userhost = NULL;
        New_user->access = 0;
        New_user->aop    = 0;
        New_user->prot   = 0;
	New_user->type   = 0;
	New_user->channel = NULL;
	mstrcpy(&New_user->channel, "*");
	New_user->password = NULL;
	New_user->whoadd = NULL;
	New_user->time = getthetime();
	New_user->expire = 0;
	New_user->reason = NULL;
        New_user->prev = NULL;
        if (*l_list)
                (*l_list)->prev = New_user;
        New_user->next = *l_list;
        *l_list = New_user;
        return New_user;
}

aFlood *make_flood()
{
	register aFlood *ptr = NULL;
        register size = sizeof(aFlood);
        if (!(ptr = (aFlood *)MyMalloc(size)))
                return NULL;
	ptr->hostname = NULL;
        ptr->num = 0;
        ptr->last = time(NULL);
        ptr->prev = NULL;
        if (current->Floods)
                current->Floods->prev = ptr;
        ptr->next = current->Floods;
        current->Floods = ptr;
        return ptr;
}

aFlood *find_flood(host)
char *host;
{
        register aFlood *ptr;

        for(ptr=current->Floods;ptr;ptr=ptr->next)
                if (!my_stricmp(ptr->hostname, host))
                        return ptr;
        return NULL;
}

int check_flood(host)
char *host;
{
	aFlood *ptr;
	time_t now;

	now = time(NULL);
	if ((ptr = find_flood(host)) == NULL)
	{
		if ((ptr = make_flood()) == NULL)
			return FALSE;
		mstrcpy(&ptr->hostname, host);
	}
	if ((now - ptr->last) > FLOOD_RESET)
		ptr->num = 0;
	ptr->num++;
	ptr->last = now;
	if (ptr->num >= FLOOD_LINES)
		return TRUE;
	return FALSE;
}

void remove_flood(ptr)
aFlood *ptr;
{
        if (ptr->prev)
                ptr->prev->next = ptr->next;
        else
        {
                current->Floods = ptr->next;
                if (current->Floods)
                        current->Floods->prev = NULL;
        }
        if (ptr->next)
                ptr->next->prev = ptr->prev;
	MyFree((char **)&ptr->hostname);
        MyFree((char **)&ptr);
}

void update_flood()
{
        register aFlood *ptr = current->Floods;
        register aFlood *temp;
        time_t old;

        old = time(NULL);
        while (ptr != NULL)
        {
                temp = ptr->next;
                if ((old - ptr->last) > FLOOD_RESET)
                        remove_flood(ptr);
                ptr = temp;
        }
}

void delete_flood(l_list)
aFlood **l_list;
{
	register aFlood *cptr, *tmp;
	cptr = *l_list;
	while(cptr)
	{
		tmp = cptr->next;
		remove_flood(cptr);
		cptr = tmp;
	}
	*l_list = NULL;
}
