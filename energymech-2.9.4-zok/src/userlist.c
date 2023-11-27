/*

    EnergyMech, IRC bot software
    Parts Copyright (c) 1997-2001 proton, 2002-2004 emech-dev

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/
#include "config.h"

int saveuserlist(void)
{
	aStrp	*ump;
	aUser	*User;
	int	i,f;
#ifdef DEBUG
	int	dodeb;
#endif /* DEBUG */

	if ((f = open(current->Userfile,O_WRONLY|O_CREAT|O_TRUNC,0666)) < 0)
		return(FALSE);

#ifdef DEBUG
	dodeb = dodebug;
	dodebug = FALSE;
#endif /* DEBUG */
	User = current->userlistBASE;
	for(i=0;i<current->userlistSIZE;i++)
	{
		if (!User->used)
		{
			User++;
			continue;
		}
		if ((User->type == USERLIST_SHIT) && (now >= User->expire))
		{
			User++;
			continue;
		}

		switch(User->type)
		{
		case USERLIST_USER:
			tofile(f,"\nhandle\t\t%s\n",User->name);
			ump = User->parm.umask;
			while(ump)
			{
				tofile(f,"mask\t\t%s\n",ump->p);
				ump = ump->next;
			}
			if (*User->pass)
				tofile(f,"pass\t\t%s\n",User->pass);
			if (User->prot)
				tofile(f,"prot\t\t%i\n",User->prot);
			if (User->aop)
				tofile(f,"aop\n");
			if (User->avoice)
				tofile(f,"avoice\n");
			if (User->echo)
				tofile(f,"echo\n");

			break;
		case USERLIST_SHIT:
			tofile(f,"\nshit\t\t%s\n",User->parm.shitmask);
			tofile(f,"time\t\t%li\n",User->time);
			tofile(f,"expire\t\t%li\n",User->expire);	
			break;
		case USERLIST_KICKSAY:
			tofile(f,"\nkicksay\t\t%s\n",User->parm.kicksay);
			break;
		}
		tofile(f,"channel\t\t%s\n",User->chanp);
		if (User->access)
			tofile(f,"access\t\t%i\n",User->access);
		if (User->whoadd)
			tofile(f,"who\t\t%s\n",User->whoadd);
		if (User->reason)
			tofile(f,"reason\t\t%s\n",User->reason);
		User++;
	}
	close(f);
#ifdef DEBUG
	dodebug = dodeb;
#endif /* DEBUG */
	return(TRUE);
}

aUser *check_kicksay(char *string, char *channel)
{
	aUser	*User;
	int	i;

	User = current->userlistBASE;
	for(i=0;i<current->userlistSIZE;i++)
	{
		if ((User->used) && (User->type == USERLIST_KICKSAY))
		{
			if (!channel || !Strcasecmp(channel,User->chanp) ||
			    (*User->chanp == '*') || (*channel == '*'))
				if (!matches(User->parm.kicksay,string))
					return(User);
		}
		User++;
	}
	return(NULL);
}

aUser *find_kicksay(char *string, char *channel)
{
	aUser	*User,*save;
	int	i,num,best;

	save = NULL;
	best = 0;

	User = current->userlistBASE;
	for(i=0;i<current->userlistSIZE;i++)
	{
		if ((User->used) && (User->type == USERLIST_KICKSAY))
		{
			if (!channel || !Strcasecmp(channel,User->chanp) ||
			    (*User->chanp == '*') || (*channel == '*'))
			{
				num = num_matches(User->parm.kicksay,string);
				if (num > best)
				{
					best = num;
					save = User;
				}
			}
		}
		User++;
	}
	return(save);
}

void do_kslist(char *from, char *to, char *rest, int cmdlevel)
{
	aUser	*User;
	int	i;

	send_to_user(from,TEXT_KSLISTHEADER);
	User = current->userlistBASE;
	for(i=0;i<current->userlistSIZE;i++)
	{
		if ((User->used) && (User->type == USERLIST_KICKSAY))
		{
			send_to_user(from,"%15s %s     %s",User->chanp,User->parm.kicksay,User->reason);
		}
		User++;
	}
	send_to_user(from,TEXT_KSLISTFOOTER);
}


void addmasktouser(aUser *User, char *mask)
{
	aStrp	*ump,*new;

	ump = User->parm.umask;
	while(ump)
	{
		if (!Strcasecmp(ump->p,mask))
			return;
		ump = ump->next;
	}
	set_mallocdoer(addmasktouser);
	new = (aStrp*)MyMalloc(strlen(mask)+1+PTR_SIZE);
	strcpy(new->p,mask);
	if (!User->parm.umask)
	{
		User->parm.umask = new;
		return;
	}
	ump = User->parm.umask;
	while(ump->next)
		ump = ump->next;
	ump->next = new;
}

void delmaskfromuser(aUser *User, char *mask)
{
	aStrp	**pp,*ump;

	pp = &(User->parm.umask);
	while(*pp)
	{
		if (!Strcasecmp((*pp)->p,mask))
		{
			ump = *pp;
			*pp = ump->next;
			MyFree((char**)&ump);
			return;
		}
		pp = &(*pp)->next;
	}
}

void do_host(char *from, char *to, char *rest, int cmdlevel)
{
	aStrp	*ump;
	aUser	*User;
	char	*incmd,*handle,*mask;
	int	ul;

	incmd = chop(&rest);
	if ((handle = chop(&rest)) == NULL)
		goto err;
	if ((mask = chop(&rest)) == NULL)
		goto err;
	if ((User = find_handle(handle)) == NULL)
	{
		send_to_user(from,TEXT_INVALIDHANDLE);
		return;
	}
	ul = get_userlevel(from,User->chanp);
	if ((ul != OWNERLEVEL) && (ul < User->access))
	{
		send_to_user(from,TEXT_ACCESSDENIED);
		return;
	}
	if (incmd && !Strcasecmp(incmd,"ADD"))
	{
		if ((ump = User->parm.umask))
		{
			while(ump)
			{
				if (!Strcasecmp(ump->p,mask))
				{
					send_to_user(from,TEXT_MASKEXISTS,mask,User->name);
					return;
				}
				ump = ump->next;
			}
		}
		if (!matches(mask,"!@"))
		{
			send_to_user(from,TEXT_MASKPROBADDGLOBAL,mask);
			return;
		}
		if (matches("*!*@*.*",mask))
		{
			send_to_user(from,TEXT_MASKPROBADDINVALID,mask);
			return;
		}
		addmasktouser(User,mask);
		send_to_user(from,TEXT_MASKADDED,mask,User->name);
		return;
	}
	if (incmd && !Strcasecmp(incmd,"DEL"))
	{
		delmaskfromuser(User,mask);
		send_to_user(from,TEXT_MASKDELETED,mask,User->name);
		return;
	}
err:
	usage(from,C_HOST);
	return;
}

aUser *add_to_userlist(char *handle, int axs, int aop, int prot, char *chan, char *pass)
{
	aUser	new,*up;
	int	uid,sid;

#ifdef DEBUG
	if (!handle || !*handle || !chan || !*chan)
	{
		debug("(add_to_userlist) PANIC: never happen scenario\n");
		return(NULL);
	}
#endif /* DEBUG */

	memset(&new,0,sizeof(aUser));
	new.used = TRUE;
	new.ident = userident++;
	new.access = axs;
	new.aop = (aop) ? 1 : 0;
	new.prot = (prot > MAXPROTLEVEL) ? MAXPROTLEVEL : (prot < 0) ? 0 : prot;
	strncpy(new.name,handle,MAXNICKLEN);
	new.name[MAXNICKLEN] = 0;
	set_mallocdoer(add_to_userlist);
	mstrcpy(&new.chanp,chan);
	if (pass && *pass)
	{
		strncpy(new.pass,pass,PASSLEN);
		new.pass[PASSLEN] = 0;
	}
	new.type = USERLIST_USER;
	/*
	 *  Kludge to avoid SEGV in case userlistBASE is realloc'd and moved
	 */
	uid = sid = -1;
	if (CurrentUser)
		uid = CurrentUser->ident;
	if (CurrentShit)
		sid = CurrentShit->ident;  
	up = add_aUser(&new);
	if (uid != -1) 
		CurrentUser = find_aUser(uid);
	if (sid != -1)
		CurrentShit = find_aUser(sid);
	return(up);
}

void do_userlist(char *from, char *to, char *rest, int cmdlevel)
{
	aStrp	*ump;
	aUser	*User;
	char	*chan,*mask;
	int	minlevel = 0;
	int	maxlevel = BOTLEVEL;
	int	botsonly = FALSE;
	int	chanonly = FALSE;
	int	i,show,count,cmdok;

	chan = NULL;
	mask = NULL;

	if ((rest) && (*rest))
	{
		cmdok = FALSE;
		if (*rest == '+')
		{
			rest = &rest[1];
			if ((*rest >= '0') && (*rest <= '9'))
			{
				minlevel = atoi(rest);
				cmdok = TRUE;
			}
		}
		else
		if (*rest == '-')
		{
			rest = &rest[1];
			if ((*rest >= '0') && (*rest <= '9'))
			{
				maxlevel = atoi(rest);
				cmdok = TRUE;
			}
			if ((*rest == 'B') || (*rest == 'b'))
			{
				botsonly = TRUE;
				cmdok = TRUE;
			}
			if ((*rest == 'C') || (*rest == 'c'))
			{
				chanonly = TRUE;
				cmdok = TRUE;
			}
		}
		else
		if ((*rest == '#') || (*rest == '&'))
		{
			chan = rest;
			cmdok = TRUE;
		}
		else
		if (strchr(rest,'*') != NULL)
		{
			mask = rest;
			cmdok = TRUE;
		}
		if (!cmdok)
		{
			usage(from,C_USERLIST);
			return;
		}
	}

#ifdef DEBUG
	debug("(do_userlist) mask=%s minlevel=%i maxlevel=%i botsonly=%s chanonly=%s\n",
		(mask) ? mask : "NOMASK",minlevel,maxlevel,
		(botsonly) ? "Yes" : "No",(chanonly) ? "Yes" : "No");
#endif

	count = 1;
	send_to_user(from,TEXT_ULISTHEADER,current->nick);
	send_to_user(from," ");
	User = current->userlistBASE;
	for(i=0;i<current->userlistSIZE;i++)
	{
		if ((User->used) && (User->type == USERLIST_USER))
		{
			show = TRUE;

			if (User->access < minlevel)
				show = FALSE;
			if (User->access > maxlevel)
				show = FALSE;
			if ((chan) && (Strcasecmp(chan,User->chanp)) && (Strcasecmp("*",User->chanp)))
				show = FALSE;
			if (mask)
			{
				ump = User->parm.umask;
				while(ump)
				{
					if (matches(mask,ump->p))
					{
						show = FALSE;
						break;
					}
					ump = ump->next;
				}
			}
			if ((botsonly) && (!(User->access == BOTLEVEL)))
				show = FALSE;
			if ((chanonly) && (*User->chanp == '*'))
				show = FALSE;

			if (show)
			{
#ifdef LINKING
				send_to_user(from," User   : %-11s   [%3i/%s/%s/%s/P%d]   C:%s",
					User->name,User->access,(User->aop)?"AO":"--",
					(User->avoice)?"AV":"--",(User->pass[0])?"PW":"--",
					User->prot,User->chanp);
#else /* LINKING */
				send_to_user(from," User   : %-11s   [%3i/%s/%s/%s/P%d]   C:%s",
					User->name,User->access,(User->aop)?"AO":"--",
					(User->avoice)?"AV":"--",(User->pass[0])?"PW":"--",
					User->prot,User->chanp);
#endif /* LINKING */

				if ((ump = User->parm.umask))
				{
					send_to_user(from," Mask(s): %s",ump->p);
					while((ump = ump->next))
					{
						send_to_user(from,"          %s",ump->p);
					}
				}
				send_to_user(from," ");
				count++;
			}
		}
		User++;
	}
	send_to_user(from,TEXT_ULISTTOTAL, count - 1);
	send_to_user(from,TEXT_ULISTFOOTER);
}

void remove_user(aUser *User)
{
	aStrp	*ump,*nxt;

	if (User->used)
	{
		MyFree(&User->chanp);
		MyFree(&User->whoadd);
		MyFree(&User->reason);
		switch(User->type)
		{
		case USERLIST_USER:
			ump = User->parm.umask;
			while(ump)
			{
				nxt = ump->next;
				MyFree((char**)&ump);
				ump = nxt;
			}
			break;
		case USERLIST_SHIT:
		case USERLIST_KICKSAY:
			MyFree((char**)&User->parm.shitmask);
			break;
		}
		memset(User,0,sizeof(aUser));
	}
}

aUser *find_handle(char *handle)
{
	aUser 	*User;
	int	i;

	User = current->userlistBASE;
	for(i=0;i<current->userlistSIZE;i++)
	{
		if ((User->used) && (User->type == USERLIST_USER))
		{
			if (!Strcasecmp(handle,User->name))
				return(User);
		}
		User++;
	}
	return(NULL);
}

/*
 *  Find the user that best matches the userhost
 */
aUser *find_user(char *userhost, char *channel)
{
	aStrp	*ump;
	aUser	*User,*save;
	int	i,num,best;

	/*
	 *  Telnet users has a special n!u@h
	 */
	if (!Strcasecmp("telnet@energymech",getuh(userhost)))
	{
		User = find_handle(getnick(userhost));
		if (!channel || *User->chanp == '*' || !Strcasecmp(User->chanp,channel))
			return(User);
	}
	save = NULL;
	best = 0;
	User = current->userlistBASE;
	for(i=0;i<current->userlistSIZE;i++)
	{
		if ((User->used) && (User->type == USERLIST_USER))
		{
			if (!channel || !Strcasecmp(channel,User->chanp) ||
			    (*User->chanp == '*') || (*channel == '*'))
			{
				ump = User->parm.umask;
				while(ump)
				{
					num = num_matches(ump->p,userhost);
					if (num > best)
					{
						best = num;
						save = User;
					}
					ump = ump->next;
				}
			}
		}
		User++;
	}
	return(save);
}

int get_userlevel(char *userhost, char *channel)
{
	aUser	*User;

#ifdef LINKING
	if (*userhost == '$')
	{
		while(*userhost && (*userhost != '@'))
			userhost++;
		if (*userhost == '@')
			userhost++;
		else
			return(0);
	}
#endif /* LINKING */
	if (!Strcasecmp(SHELLMASK,userhost))
		return(OWNERLEVEL);
	if (is_localbot(userhost))
		return(BOTLEVEL);
	if (current->userlistSIZE == 0)
		return(OWNERLEVEL);
	if (!Strcasecmp("telnet@energymech",getuh(userhost)))
	{
		User = find_handle(getnick(userhost));
		if (!channel || *User->chanp == '*' || !Strcasecmp(User->chanp,channel))
			return(User->access);
#ifdef DEBUG
		debug("[GuL] (telnet user %s[%i]) \"%s\" != \"%s\"\n",User->name,User->access,
			nullstr(channel),nullstr(User->chanp));
#endif /* DEBUG */
	}
	if ((User = find_user(userhost,channel)) == NULL)
		return(0);
	return(User->access);
}

int max_userlevel(char *userhost)
{
	aStrp	*ump;
	aUser	*User;
	int	i,axs;

#ifdef LINKING
	if (*userhost == '$')
	{
		while(*userhost && (*userhost != '@'))
			userhost++;
		if (*userhost == '@')
			userhost++;
		else
			return(0);
	}
#endif /* LINKING */
	if (!Strcasecmp(SHELLMASK,userhost))
		return(OWNERLEVEL);
	if (is_localbot(userhost))
		return(BOTLEVEL);
	if (current->userlistSIZE == 0)
		return(0);

#ifdef TELNET
	if (!Strcasecmp("telnet@energymech",getuh(userhost)))
	{
		User = find_handle(getnick(userhost));
		axs = User->access;
		goto access_ok;
	}
#endif /* TELNET */

	axs = 0;
	User = current->userlistBASE;
	for(i=0;i<current->userlistSIZE;i++)
	{
		if ((User->used) && (User->type == USERLIST_USER))
		{
			ump = User->parm.umask;
			while(ump)
			{
				if (!matches(ump->p,userhost))
					axs = (User->access > axs) ? User->access : axs;
				ump = ump->next;
			}
		}
		User++;
	}
#ifdef TELNET
access_ok:
#endif /* TELNET */
#ifdef DEBUG
	debug("[MuL] %s == %i\n",userhost,axs);
#endif /* DEBUG */
	return(axs);
}

int is_aop(char *userhost, char *channel)
{
	aUser	*User;

	if (is_me(getnick(userhost)))
		return(OWNERLEVEL);
	if ((User = find_user(userhost,channel)) == NULL)
		return(0);
	return(User->aop);
}

int is_avoice(char *userhost, char *channel)
{
	aUser	*User;

	if ((User = find_user(userhost,channel)) == NULL)
		return(FALSE);
	return(User->avoice);
}

int is_user(char *userhost, char *channel)
{
	aUser	*User;

	if (is_localbot(userhost))
		return(TRUE);
	if ((User = find_user(userhost,channel)) == NULL)
		return(FALSE);
	return(User->access);
}

int get_protlevel(char *userhost, char *channel)
{
	aUser	*User;

	if (is_me(getnick(userhost)))
		return(SELFPROTLEVEL);
	if ((User = find_user(userhost,channel)) != NULL)
		return(User->prot);
	return(0);
}

int get_protuseraccess(char *userhost, char *channel)
{
	aChanUser *CU;
	aChan   *Chan;
	char    *p;
	int     prot;

#ifdef DEBUG
	debug("(get_protuseraccess) userhost = '%s', channel = '%s'\n",
		nullstr(userhost),nullstr(channel));
#endif /* DEBUG */
	for(Chan=current->Channels;Chan;Chan=Chan->next)
	{
		for(CU=Chan->users;CU;CU=CU->next)
		{
			p = get_nuh(CU);
			if (matches(userhost,p))
				continue;
			if ((prot = get_protlevel(p,channel)))
				return(prot);
		}
	}
	return(0);
}

aUser *get_shituser(char *userhost, char *channel)
{
	aChanUser *CU;
	aChan	*Chan;
	aUser	*User;
	char	*p;

#ifdef DEBUG
	debug("(get_shituser) userhost = '%s', channel = '%s'\n",
		nullstr(userhost),nullstr(channel));
#endif /* DEBUG */
	if (is_me(getnick(userhost)))
		return(NULL);
	for(Chan=current->Channels;Chan;Chan=Chan->next)
	{
		for(CU=Chan->users;CU;CU=CU->next)
		{
			p = get_nuh(CU);
			if (matches(userhost,p))
				continue;
			if ((User = find_shit(p,channel)) != NULL)
				return(User);
		}
	}
	return(NULL);
}

int verified(char *userhost)
{
	aUser	*User;

	if (!Strcasecmp("telnet@energymech",getuh(userhost)))
	{
		User = find_handle(getnick(userhost));
		return((User != NULL));
	}
	return((find_time(&current->Auths,userhost) != NULL));
}
	
int password_needed(char *userhost)
{
	aUser *dummy;

	if (!(dummy = find_user(userhost,"*")))
		return(FALSE);
	if (!dummy->pass[0])
		return(FALSE);
	return(TRUE);
}

int correct_password(char *userhost, char *password)
{
	aUser	*User;

	if ((User = find_user(userhost,NULL)) == NULL)
		return(FALSE);
	if (User->pass[0] == 0)
		return(TRUE);
	if (password && passmatch(password,User->pass))
		return(TRUE);
	return(FALSE);
}

aUser *find_shit(char *userhost, char *channel)
{
	aUser	*User,*save;
	int	i,num,best;

	if (!userhost)
		return(NULL);
	save = NULL;
	best = 0;
	User = current->userlistBASE;
	for(i=0;i<current->userlistSIZE;i++)
	{
		if ((User->used) && (User->type == USERLIST_SHIT))
		{
			if (!channel || !Strcasecmp(channel,User->chanp) ||
			    (*User->chanp == '*') || (*channel == '*'))
			{
				num = num_matches(User->parm.shitmask,userhost);
				if (num > best)
				{
					best = num;
					save = User;
				}
			}
		}
		User++;
	}
	if (save && save->expire < now)
	{
		remove_user(save);
		return(NULL);
	}
	return(save);
}

aUser *add_to_shitlist(char *uh, int level, char *chan, char *from, char *reason, int when, int expire)
{
	aUser	new,*up;
	int	uid,sid;

	memset(&new,0,sizeof(aUser));
	new.used = TRUE;
	new.type = USERLIST_SHIT;
	new.access = level;
	new.time = when;
	new.expire = expire;
	set_mallocdoer(add_to_shitlist);
	mstrcpy(&new.chanp,chan);
	set_mallocdoer(add_to_shitlist);
	mstrcpy(&new.parm.shitmask,uh);
	set_mallocdoer(add_to_shitlist);
	mstrcpy(&new.whoadd,from);
	set_mallocdoer(add_to_shitlist);
	mstrcpy(&new.reason,reason);
	/*
	 *  Kludge to avoid SEGV in case userlistBASE is realloc'd and moved
	 */
	uid = sid = -1;
	if (CurrentUser)
		uid = CurrentUser->ident;
	if (CurrentShit)
		sid = CurrentShit->ident;  
	up = add_aUser(&new);
	if (uid != -1) 
		CurrentUser = find_aUser(uid);
	if (sid != -1)
		CurrentShit = find_aUser(sid);
	return(up);
}

void do_shitlist(char *from, char *to, char *rest, int cmdlevel)
{
	aUser	*User;
	int	i;

	send_to_user(from,TEXT_SHITHEADER);
	User = current->userlistBASE;
	for(i=0;i<current->userlistSIZE;i++)
	{
		if ((User->used) && (User->type == USERLIST_SHIT))
		{
			send_to_user(from,"  %-30s %12s  %3d %10s",User->parm.shitmask,User->chanp,
				User->access,getnick(User->whoadd));
		}
		User++;
	}
	send_to_user(from,TEXT_SHITFOOTER);
}

void delete_shitlist(void)
{
	aUser	*User;
	int	i;

	User = current->userlistBASE;
	for(i=0;i<current->userlistSIZE;i++)
	{
		if ((User->used) && (User->type == USERLIST_SHIT))
			remove_user(User);
		User++;
	}
}

int is_shitted(char *userhost, char *channel)
{
	aUser	*User;

	if ((User = find_shit(userhost,channel)) == NULL)
		return(FALSE);
	return(TRUE);
}

int get_shitlevel(char *userhost, char *chan)
{
	aUser	*User;

	if ((User = find_shit(userhost,chan)) == NULL)
		return(0);
	return(User->access);
}

char *get_shitreason(aUser *User)
{
	static	char res[MSGLEN];
	char	*crap;

	res[0] = 0;
	if (User)
	{
		crap = getnick(User->whoadd);
		if (User->time)
		{
			Strcat(res,time2small(User->time));
			Strcat(res," ");
		}
		Strcat(res,crap);
		Strcat(res,": ");
		if (!User->reason)
			Strcat(res,"GET THE HELL OUT!!!");
		else
			Strcat(res,User->reason);
	}
	else
		Strcat(res,"YOU ARE SHITLISTED!!!");
	return(res);
}

aTime *find_time(aTime **l_list, char *name)
{
	aTime	*user;

	if (!name)
		return(NULL);
	if (*name == '$')
	{
		while(*name && (*name != '@')) name++;
		if (*name == '@') name++;
		else return(NULL);
	}
	for (user = *l_list; user; user = user->next)
		if (!Strcasecmp(name, user->name))
			break;
	return(user);
}

aTime *make_time(aTime **l_list, char *name)
{
	aTime	*new;

	if (!name)
		return(FALSE);

	if ((new = find_time(l_list,name)) != NULL)
		return(new);
	set_mallocdoer(make_time);
	new = (aTime*)MyMalloc(sizeof(aTime));
	set_mallocdoer(make_time);
	mstrcpy(&new->name,name);
	new->num = 1;
	new->time = now;
	new->prev = NULL;
	if (*l_list)
		(*l_list)->prev = new;
	new->next = *l_list;
	*l_list = new;
	return(new);
}

void remove_time(aTime **l_list, aTime *cptr)
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

void delete_time(aTime **l_list)
{
	aTime	*cptr,*tmp;

	cptr = *l_list;
	while (cptr)
	{
		tmp = cptr->next;
		remove_time(l_list, cptr);
		cptr = tmp;
	}
	*l_list = NULL;
}

void uptime_time(aTime **l_list, time_t thetime)
{
	aTime	*cptr,*tmp;

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

/*
 *  SEEN functions
 */

#ifdef SEEN

int write_seenlist(aSeen **l_list, char *filename)
{
	aSeen	*Seen;
	int	f;

	if (!will.seen)
		return(TRUE);

	if ((f = open(filename,O_WRONLY|O_CREAT|O_TRUNC,0666)) < 0)
		return(FALSE);

	for(Seen=*l_list;Seen;Seen=Seen->next)
	{
		if ((Seen->time - now) > (86400 * SEEN_TIME))
			continue;
		else
		{
			if (Seen->selector != 0)
			{
				tofile(f,"%s %s %s %li %i %s\n",
					Seen->nick,Seen->userhost,Seen->kicker,
					Seen->time,Seen->selector,Seen->signoff);
			}
		}
	}
	close(f);
	return(TRUE);
}

int read_seenlist_callback(char *line)
{
	time_t	thetime;
	char	*nick,*userhost,*kicker,*timestr,*selector;
	int	theselector;

	nick = chop(&line);
	userhost = chop(&line);
	kicker = chop(&line);
	timestr = chop(&line);
	selector = chop(&line);
	thetime = (timestr && *timestr) ? atol(timestr) : now;
	if ((now - thetime) < (SEEN_TIME * 86400))
	{
		theselector = (selector && *selector) ? atoi(selector) : 0;
		make_seen(nick,userhost,kicker,thetime,theselector,line);
	}
	return(FALSE);
}

int read_seenlist(void)
{
	int	in;

	if ((in = open(current->seenfile,O_RDONLY)) < 0)
		return(FALSE);

	delete_seen();
	readline(in,&read_seenlist_callback);
	close(in);
	return(TRUE);
}

void delete_seen(void)
{
	aSeen	*Seen,*nxt;

	Seen=current->SeenList;
	while(Seen)
	{
		nxt = Seen->next;
		MyFree((char**)&Seen);
		Seen = nxt;
	}
	current->SeenList = NULL;
}

aSeen *make_seen(char *nick, char *userhost, char *kicker, time_t when, int selector, char *msg)
{
	aSeen	*Seen,**pSeen;
	Uchar	c1,c2;

	c1 = tolowertab[(Uchar)*nick];
	pSeen = &current->SeenList;
	while(*pSeen)
	{
		c2 = tolowertab[(Uchar)*(*pSeen)->nick];
		if (c1 == c2)
		{
			if (!Strcasecmp(nick,(*pSeen)->nick))
			{
				Seen = *pSeen;
				*pSeen = Seen->next;
				MyFree((char**)&Seen);
				break;
			}
		}
		else
		if (c1 > c2)
		{
			break;
		}
		pSeen = &(*pSeen)->next;
	}

	set_mallocdoer(make_seen);
	Seen = (aSeen*)MyMalloc(sizeof(aSeen)+strlen(nick)+strlen(userhost)+strlen(kicker)+strlen(msg));
	Seen->userhost = Seen->nick + strlen(nick) + 1;
	Seen->signoff = Seen->userhost + strlen(userhost) + 1;
	Seen->kicker = Seen->signoff + strlen(msg) + 1;
	strcpy(Seen->nick,nick);
	strcpy(Seen->userhost,userhost);
	strcpy(Seen->signoff,msg);
	strcpy(Seen->kicker,kicker);
	Seen->time = when;
	Seen->selector = selector;
	Seen->next = *pSeen;
	*pSeen = Seen;
	return(Seen);
}

#endif /* SEEN */
