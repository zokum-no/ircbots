/*
 * structs.c - manages all the structures in the program 
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

#include <stdarg.h>
#include <string.h>

#include "global.h"
#include "config.h"
#include "defines.h"
#include "structs.h"
#include "h.h"

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

