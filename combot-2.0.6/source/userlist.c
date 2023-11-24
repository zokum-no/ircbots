/*
 * userlist.c - implementation of userlists
 * (c) 1995 CoMSTuD (cbehrens@slavery.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
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
				IsGreet(tmp) || IsSaySay(tmp)
					? "Greeting" : "Reason",
				tmp->reason);
		}
}

int saveuserlist(l_list, filename)
aUser **l_list;
char *filename;
{
	FILE *ptr;

	if (!(ptr=fopen(USERFILE, "wt")))
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

	for(tmp=Userlist;tmp;tmp=tmp->next)
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

aUser *check_saysay(string, channel)
char *string;
char *channel;
{
	register aUser *tmp;

	for(tmp=Userlist;tmp;tmp=tmp->next)
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
	(((tmp=find_user(&Userlist, userhost, channel)) != NULL) &&
		tmp->access);
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
	New_user->type = USERLIST_USER;
	return New_user;
}

void show_userlist(from, l_list)
char *from;
aUser *l_list;
{
	register aUser *dummy;
	send_to_user(from,
"\002------------- Userhost ------------- Channel -- Acc AO Prt --\002");
	for (dummy = l_list; dummy; dummy = dummy->next )
		if (IsUser(dummy))
			send_to_user( from, " %30s %15s  %3d %d %d",
			dummy->userhost, dummy->channel, dummy->access,
			dummy->aop, dummy->prot );
	send_to_user(from, "\002--- end of userlist ---\002");
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
	for (user=Userlist;user;user=user->next)
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
        if ((dummy=find_user(&Userlist, userhost, channel)) == NULL)
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
	if ((dummy=find_user(&Userlist, userhost, channel)) == NULL)
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
	if ((dummy=find_user(&Userlist, userhost, channel)) == NULL)
		return 0;
	return dummy->prot;
}
	
int password_needed(userhost)
char *userhost;
{
	aUser *dummy;


	if (!(dummy=find_user(&Userlist, userhost, "*")))
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

	if (!(dummy=find_user(&Userlist, userhost, "*")))
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
		"\002--------- Userhost --------- Channel ---- Level ---\002");
	for (dummy = l_list; dummy; dummy = dummy->next )
		if (IsShit(dummy))
			send_to_user( from, " %20s %15s  %3d %10s %s",
			dummy->userhost, dummy->channel, dummy->access,
			getnick(dummy->whoadd), time2str(dummy->expire));
	send_to_user(from, "\002--- end of shitlist ---\002");
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

int is_shitted(userhost, channel)
char *userhost;
char *channel;
{
	aUser *dummy;

	if (!matches(owneruserhost, userhost))
		return FALSE;        /* just in case =) */
	dummy = find_shit(&Userlist, userhost, channel);
	if (!dummy)
		return FALSE;
	return (dummy->expire > getthetime()) ? TRUE : FALSE;
}

long int get_shitexpire(userhost, channel)
char *userhost;
char *channel;
{
	aUser *dummy;

	if (!(dummy = find_shit(&Userlist, userhost, channel)))
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

	if (!(dummy=find_shit(&Userlist, userhost, chan)))
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
	if (!(dummy = find_shit(&Userlist, userhost, channel)))
		return NULL;
	return dummy->userhost;
}

char *get_shit_reason(userhost, channel)
char *userhost;
char *channel;
{
	aUser *dummy;
	if (!(dummy = find_shit(&Userlist, userhost, channel)))
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
        fprintf(list_file, "## (c) 1995 CoMSTuD (cbehrens@slavery.com)\n");
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

