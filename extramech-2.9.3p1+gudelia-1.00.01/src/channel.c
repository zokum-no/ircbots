/*

    EnergyMech, IRC bot software
    Parts Copyright (c) 1997-2001 proton, 2002-2003 emech-dev

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

void check_all_steal(void)
{
	aChan	*Chan;

	for(Chan=current->Channels;Chan;Chan=Chan->next)
	{
		if (Chan->active != CH_ACTIVE)
			continue;
		if (Chan->users && Chan->users->next == NULL)
		{
			if (Chan->users->flags & CU_CHANOP)
				continue;
			cycle_channel(Chan->name);
		}
	}
}

int get_idletime(char *nick)
{
	aChanUser *User;
	aChan	*Chan;
	int	temp;

	temp = -1;
	for(Chan=current->Channels;Chan;Chan=Chan->next)
	{
		if ((User = find_chanuser(Chan,nick)) != NULL)
		{
			if ((temp == -1) || (User->idletime < temp))
				temp = User->idletime;
		}
	}
	return(temp);
}

void show_idletimes(char *from, char *channel, int seconds)
{
	aChanUser *User;
	aChan	*Chan;
	time_t	ct;

	if ((Chan = find_channel(channel,CH_ACTIVE)) == NULL)
		return;
	send_to_user(from,TEXT_USERSIDLE,
		Chan->name,seconds,EXTRA_CHAR(seconds));
	for(User=Chan->users;User;User=User->next)
	{
		if ((ct = now - User->idletime) > seconds)
			send_to_user(from,"%s: %s",idle2str(ct,TRUE),get_nuh(User));
	}
	send_to_user(from,TEXT_ENDOFLIST);
}

int is_present(char *who)
{
	aChan	*Chan;

	for(Chan=current->Channels;Chan;Chan=Chan->next)
	{
		if ((Chan->active == CH_ACTIVE) && (find_chanuser(Chan,who) != NULL))
			return(TRUE);
	}
	return(FALSE);
}

void op_all_chan(char *who)
{
	aChanUser *CU;
	aChan	*Chan;
	aUser	*User;

	for(Chan=current->Channels;Chan;Chan=Chan->next)
	{
		if (!Chan->bot_is_op)
			continue;
		if (get_int_varc(Chan,TOGAOP_VAR) == 0)
			continue;
		if ((CU = find_chanuser(Chan,who)) == NULL)
			continue;
		if (get_usermode(CU) & CU_CHANOP)
			continue;
		if ((User = find_user(get_nuh(CU),Chan->name)) == NULL)
			continue;
		if ((User->access <= 0) || (User->aop == FALSE))
			continue;
		sendmode(Chan->name,"+o %s",who);
	}
}

#ifdef SERVICES

void needop_via_msg(char *channel)
{
	aChan 		*Chan;
	aService	*tmpserviceptr;
	int		cycled;
	
	if ((Chan = find_channel(channel,CH_ACTIVE)) == NULL)
		return;
	
	if (Chan->ServicesList == NULL) 
		return;
	
	if (is_opped(current->nick,channel)) 
		return;
	
	tmpserviceptr = Chan->nextservice;
	
	cycled = 0;
	while (!cycled)
	{
		if (tmpserviceptr == NULL) 
			tmpserviceptr = Chan->ServicesList;

		if (is_opped(tmpserviceptr->to,channel))
		{
			to_server("PRIVMSG %s :%s",tmpserviceptr->to,
				subststrvar(tmpserviceptr->msg));
			Chan->nextservice = tmpserviceptr->next;
			return;
		}	

		tmpserviceptr = tmpserviceptr->next;
		if (tmpserviceptr == Chan->nextservice)
			cycled = 1;
	}
}

#endif /* SERVICES */

void check_shit(void)
{
	aChanUser *User;
	aChan	*Chan;

	for(Chan=current->Channels;Chan;Chan=Chan->next)
	{
		for(User=Chan->users;User;User=User->next)
		{
			shit_action(get_nuh(User),Chan);
		}
	}
}

void update_idletime(char *from, char *channel)
{
	aChanUser *User;
	aChan	*Chan;

	if ((Chan = find_channel(channel,CH_ACTIVE)) == NULL)
		return;
	if ((User = find_chanuser(Chan,CurrentNick)) == NULL)
		return;
	User->idletime = now;
}

void check_idlekick(void)
{
	aChanUser *User;
	aChan	*Chan;
	time_t	timeout;

	for(Chan=current->Channels;Chan;Chan=Chan->next)
	{
		if (
			(!get_int_varc(Chan,TOGIK_VAR)) ||
			(!is_opped(current->nick,Chan->name))
		)
			continue;
		timeout = 60 * get_int_varc(Chan,SETIKT_VAR);
		for(User=Chan->users;User;User=User->next)
		{
			if ((now - User->idletime) < timeout)
				continue;
			if (!is_user(get_nuh(User),Chan->name))
				continue;
			if (get_usermode(User) & CU_CHANOP)
				continue;
			sendkick(Chan->name,User->nick,TEXT_IDLEFORMIN,get_int_varc(Chan,SETIKT_VAR));
		}
	}
}

int check_mass(char *from, aChan *Chan, int type)
{
	aChanUser *User;

	if ((User = find_chanuser(Chan,CurrentNick)) == NULL)
		return(FALSE);

	if (type & (CHK_CAPS|CHK_PUB))
	{
		if (((User->flags & CU_CHANOP)) || (get_userlevel(get_nuh(User),Chan->name)))
			return(FALSE);
	}

	switch(type)
	{
	/*
	 *  Two forms of annoying behaviour
	 */
	case CHK_CAPS:
		if ((now - User->capstime) > 10)
		{
			User->capstime = now;
			User->capsnum = 0;
		}
		User->capsnum++;
		if ((User->capsnum >= get_int_varc(Chan,SETCKL_VAR)) && get_int_varc(Chan,TOGCK_VAR))
			return(TRUE);
		break;
	case CHK_PUB:
		if ((now - User->floodtime) > 10)
		{
			User->floodtime = now;
			User->floodnum = 0;
		}
		User->floodnum++;
		if (User->floodnum >= get_int_varc(Chan,SETFL_VAR))
			return(get_int_varc(Chan,SETFPL_VAR));
		break;
	/*
	 *  Three forms of ChanOp abuse
	 */
	case CHK_DEOP:
		if ((now - User->deoptime) > 10)
		{
			User->deoptime = now;
			User->deopnum = 0;
		}
		User->deopnum++;
		if (User->deopnum >= get_int_varc(Chan,SETMDL_VAR))
			return(TRUE);
		break;
	case CHK_BAN:
		if ((now - User->bantime) > 10)
		{
			User->bantime = now;
			User->bannum = 0;
		}
		User->bannum++;
		if (User->bannum >= get_int_varc(Chan,SETMBL_VAR))
			return(TRUE);
		break;
	case CHK_KICK:
		if ((now - User->kicktime) > 10)
		{
			User->kicktime = now;
			User->kicknum = 0;
		}
		User->kicknum++;
		if (User->kicknum >= get_int_varc(Chan,SETMKL_VAR))
			return(TRUE);
		break;
	}
	return(FALSE);
}

int check_nickflood(char *from)
{
	aChanUser *ChanUser;
	aChan	*Chan;
	char	*pt;
	int	flag = FALSE;

	for(Chan=current->Channels;Chan;Chan=Chan->next)
	{
		if (!Chan->bot_is_op)
			continue;
		if ((ChanUser = find_chanuser(Chan,CurrentNick)) != NULL)
		{
			if ((now - ChanUser->nicktime) > 10)
			{
				ChanUser->nicktime = now;
				ChanUser->nicknum = 0;
			}
			ChanUser->nicknum++;
			if (ChanUser->nicknum >= get_int_varc(Chan,SETNCL_VAR))
			{
				flag = TRUE;
				pt = format_uh(from,1);
				if (!is_user(from,Chan->name) && !get_chanopmode(ChanUser))
				{
					if (get_int_varc(Chan,TOGAS_VAR))
					{
						add_to_shitlist(pt,2,Chan->name,TEXT_AUTOSHIT,
							TEXT_AUTOSHITREASON,
							now,now + 86400);
					}
				}
				deop_ban(Chan->name,CurrentNick,pt);
				sendkick(Chan->name,CurrentNick,TEXT_GETOUTNICKFLOOD);
				send_statmsg(TEXT_KICKNICKFLOOD,from,Chan->name);
			}
		}
	}
	return(flag);
}

aChan *find_channel(char *name, int active)
{
	aChan	*Chan;

	for(Chan=current->Channels;Chan;Chan=Chan->next)
	{
		if ((Chan->active & active) == 0)
			continue;
		if (!Strcasecmp(name,Chan->name))
			return(Chan);
	}
	return(NULL);
}

int delete_channel(aChan **l_list, aChan *Chan)
{
	aChan	**old;

	for(old=l_list;*old;old=&(**old).next)
	{
		if (*old == Chan)
		{
			*old = Chan->next;
			delete_chanusers(Chan);
			delete_bans(&Chan->banned);
			MyFree(&Chan->name);
			MyFree(&Chan->key);
			MyFree(&Chan->topic);
			MyFree(&Chan->kickedby);
			free_strvars(Chan->Vars);
			MyFree((char **)&Chan);
			return(TRUE);
		}
	}
	return(FALSE);
}

void delete_all_channels(void)
{
	aChan	*Chan;

	for(;(Chan=current->Channels) != NULL;)
		delete_channel(&current->Channels,Chan);
}

void copy_channellist(aChan *c_list)
{
	aChan	*Chan;

	for(Chan=c_list;Chan;Chan=Chan->next)
	{
		join_channel(Chan->name,Chan->key,FALSE);
		set_enftopic(Chan->name,Chan->topic);
	}
}

void join_channel(char *name, char *key, int dojoin)
{
	aChan	*Chan;

	if (!ischannel(name))
		return;

	if ((Chan = find_channel(name,CH_ANY)) == NULL)
	{
		set_mallocdoer(join_channel);
		Chan = (aChan*)MyMalloc(sizeof(aChan));
		set_mallocdoer(join_channel);
		mstrcpy(&Chan->name,name);
		set_mallocdoer(join_channel);
		mstrcpy(&Chan->key,key);
		copy_vars(Chan->Vars,current->DefaultVars);
		Chan->last_needop = 0;
#ifdef DYNCHANLIMIT
		Chan->last_limit = 0;
#endif /* DYNCHANLIMIT */
		Chan->next = current->Channels;
		current->Channels = Chan;
		Chan->rejoin = TRUE;
		Chan->active = CH_OLD;
		if (dojoin)
		{
			to_server("JOIN %s %s",name,(key && *key) ? key : "");
			Chan->sync = FALSE;
		}
		else
			Chan->sync = TRUE;
		return;
	}
	if (Chan->active == CH_ACTIVE)
	{
		current->CurrentChan = Chan;
		if (key && (key != Chan->key))
		{
			MyFree(&Chan->key);
			set_mallocdoer(join_channel);
			mstrcpy(&Chan->key,key);
		}
		return;
	}
	/*
	 *  If its not CH_ACTIVE, its CH_OLD; there are only those 2 states.
	 */
	if (key && (key != Chan->key))
	{
		MyFree(&Chan->key);
		set_mallocdoer(join_channel);
		mstrcpy(&Chan->key,key);
	}
	Chan->rejoin = TRUE;
	if (dojoin && Chan->sync)
	{
		to_server("JOIN %s %s",name,(key && *key) ? key : "");
		Chan->sync = FALSE;
	}
}

void reverse_topic(char *from, char *channel, char *topic)
{
	aChan	*Chan;

	if ((Chan = find_channel(channel,CH_ACTIVE)) == NULL)
		return;
	if (!get_int_varc(Chan,TOGTOP_VAR))
	{
		MyFree(&Chan->topic);
		set_mallocdoer(reverse_topic);
		mstrcpy(&Chan->topic,topic);
		return;
	}
	if (get_userlevel(from,channel) >= ASSTLEVEL)
	{
		MyFree(&Chan->topic);
		set_mallocdoer(reverse_topic);
		mstrcpy(&Chan->topic,topic);
		return;
	}
	if ((Chan->topic) && (Strcasecmp(Chan->topic,topic)))
	{
		to_server("TOPIC %s :%s",channel,Chan->topic);
		return;
	}
	set_mallocdoer(reverse_topic);
	mstrcpy(&Chan->topic,topic);
}

int reverse_mode(char *from, aChan *Chan, int m, int s)
{
	char	buffer[100];
	char	*ptr,*ptr2;
	char	mode,sign;

	if (!Chan->bot_is_op || !get_int_varc(Chan,TOGENFM_VAR) ||
		((ptr = get_str_varc(Chan,SETENFM_VAR)) == NULL))
		return(FALSE);

	mode = (char)m;
	sign = (char)s;

	if (strchr(ptr,mode) && (sign == '+'))
		return(FALSE);
	if (!strchr(ptr,mode) && (sign == '-'))
		return(FALSE);
	if (get_userlevel(from,Chan->name) >= ASSTLEVEL)
	{
		ptr2 = buffer;
		if (sign == '-')
		{
			while(*ptr)
			{
				if (*ptr != mode)
					*ptr2++ = *ptr;
				ptr++;
			}
			*ptr2 = 0;
		}
		else
		{
			buffer[0] = mode;
			buffer[1] = 0;
			Strcat(buffer,ptr);
		}
		set_str_varc(Chan,SETENFM_VAR,buffer);
		return(FALSE);
	}
	return(TRUE);
}

void cycle_channel(char *channel)
{
	aChan	*Chan;

	if ((Chan = find_channel(channel,CH_ACTIVE)) == NULL)
		return;
	if (!Chan->sync)
		return;
	to_server("PART %s\nJOIN %s %s",channel,
		channel,(Chan->key)?Chan->key:"");
	Chan->rejoin = TRUE;
}

void send_spymsg(char *format, ...)
{
	aMsgList *pt;
	char	buf[HUGE];
	va_list msg;
	int	i;

	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);

	pt = (aMsgList*)current->msglistBASE;
	for(i=0;i<current->msglistSIZE;i++)
	{
		if ((pt->used) && (!Strcasecmp(pt->type,"spymsg")))
			if (!send_chat(pt->who,buf))
				memset(pt,0,sizeof(aMsgList));
		pt++;
	}
}

void send_statmsg(char *format, ...)
{
	aMsgList *pt;
	va_list msg;
	char	buf[HUGE],buf2[HUGE];
	int	i;

	va_start(msg,format);
	vsprintf(buf,format,msg);
	va_end(msg);

	sprintf(buf2,"[%s] %s",time2medium(now),buf);
	pt = current->msglistBASE;
	for(i=0;i<current->msglistSIZE;i++)
	{
		if ((pt->used) && (!Strcasecmp(pt->type,"statmsg")))
			if (!send_chat(pt->who,buf2))
				memset(pt,0,sizeof(aMsgList));			
		pt++;
	}
}

void send_global_statmsg(char *format, ...)
{
	va_list msg;
	aMsgList *pt;
#ifdef MULTI
	aBot	*bot;
#endif /* MULTI */
	char	buf[HUGE],buf2[HUGE];
	int	i;

	va_start(msg,format);
	vsprintf(buf,format,msg);
	va_end(msg);

	sprintf(buf2,"[%s] %s",time2medium(now),buf);

#ifdef MULTI
	backup = current;
	for(bot=botlist;bot;bot=bot->next)
	{
		if (bot->msglistBASE)
		{
			current = bot;
#endif /* MULTI */
			pt = current->msglistBASE;
			for(i=0;i<current->msglistSIZE;i++)
			{
				if ((pt->used) && (!Strcasecmp(pt->type,"statmsg")))
					if (!send_chat(pt->who,buf2))
						memset(pt,0,sizeof(aMsgList));
				pt++;
			}
#ifdef MULTI
		}
	}
	current = backup;
#endif /* MULTI */
}

void send_common_spy(char *from, char *format, ...)
{
	va_list	msg;
	aChan	*Chan;
	char	buf[HUGE];

	va_start(msg,format);
	vsprintf(buf,format,msg);
	va_end(msg);

	for(Chan=current->Channels;Chan;Chan=Chan->next)
	{
		if (find_chanuser(Chan,CurrentNick) != NULL)
			send_spy(Chan,"%s",buf);
	}
}

void send_spy(aChan *Chan, char *format, ...)
{
	va_list	msg;
	aMsgList *pt;
	char	*ptr,buf[HUGE],fchan[FILENAMELEN];
	char  *chanshort;
	int	i,log;
	struct	tm *stm;

	va_start(msg,format);
	vsprintf(buf,format,msg);
	va_end(msg);

	if ((get_int_varc(Chan,TOGCHANLOG_VAR)))
	{
		stm = localtime(&now);
		chanshort = Chan->name;
		if ((*chanshort == '&') || (*chanshort == '+') || (*chanshort == '#')) 
			chanshort++; /* Nos cargamos el primer caracter del canal */
		
		snprintf(fchan, FILENAMELEN-1, "logs/%s.%d.%d.%d",
			chanshort,
			stm->tm_mon+1, stm->tm_mday, stm->tm_year+1900);

		fchan[FILENAMELEN-1] = 0;

		ptr = fchan;
		while(*ptr)
		{
			*ptr = (char)tolowertab[(Uchar)*ptr];
			ptr++;
		}
		if ((log = open(fchan,O_WRONLY|O_CREAT|O_APPEND,0666)) >= 0)
		{
			send_to_socket(log,"[%s] %s\n",time2medium(now),buf);
			close(log);
		}
	}
	pt = current->msglistBASE;
	for(i=0;i<current->msglistSIZE;i++)
	{
		if ((pt->used) && (!Strcasecmp(pt->type,Chan->name)))
		{
			if (ischannel(pt->who))
				sendprivmsg(pt->who,"[%s] %s",Chan->name,buf);
			else
				send_to_user(pt->who,"[%s] %s",Chan->name,buf);
		}
		pt++;
	}
}

int set_enftopic(char *name, char *topic)
{
	aChan	*Chan;

	if ((Chan = find_channel(name,CH_ACTIVE)) != NULL)
	{
		MyFree(&Chan->topic);
		set_mallocdoer(set_enftopic);
		mstrcpy(&Chan->topic,topic);
		return(TRUE);
	}
	return(FALSE);
}

char *chan_modestr(aChan *Chan)
{
	static	char modestr[MSGLEN];
	char	*pt;

	pt = modestr;
	*(pt++) = '+';
	if (Chan->private)
		*(pt++) = 'p';
	if (Chan->secret)
		*(pt++) = 's';
	if (Chan->moderated)
		*(pt++) = 'm';
	if (Chan->topprot)
		*(pt++) = 't';
	if (Chan->invite)
		*(pt++) = 'i';
	if (Chan->nomsg)
		*(pt++) = 'n';
	if ((Chan->limitmode) && (Chan->limit))
		*(pt++) = 'l';
	if (Chan->keymode)
		*(pt++) = 'k';

	if ((Chan->limitmode) && (Chan->limit))
	{
		sprintf(pt," %i",Chan->limit);
		while(*pt)
			pt++;
	}
	if (Chan->keymode)
	{
		sprintf(pt," %s",(Chan->key) ? Chan->key : "???");
		while(*pt)
			pt++;
	}
	*pt = 0;
	return(modestr);
}

void do_channels(char *from, char *to, char *rest, int cmdlevel)
{
	aChanUser *CU;
	aChan	*Chan;
	char	t1[MSGLEN],t2[MSGLEN];
	char	umode;
	int	u,o,v;

	if (current->Channels == NULL)
	{
		send_to_user(from,ERR_NOCHANNELS);
		return;
	}
	/*
	 *  This is only sent thru DCC/telnet so "\n" can be used.
	 */
	send_to_user(from,"[\037Channel\037]                             "
		"[\037Users\037]   [\037Ops\037]   [\037Voiced\037]   [\037Modes\037]\n");
	for(Chan=current->Channels;Chan;Chan=Chan->next)
	{
		if (Chan->active == CH_ACTIVE)
		{
			umode = (Chan->bot_is_op) ? '@' : ' ';
			u = o = v = 0;
			for(CU=Chan->users;CU;CU=CU->next)
			{
				u++;
				if (CU->flags & CU_CHANOP)
					o++;
				else
				if (CU->flags & CU_VOICE)
					v++;
			}
			sprintf(t2,"%-10i%-8i%-11i%s",u,o,v,chan_modestr(Chan));
		}
		else
		{
			sprintf(t2,"--        --      --         %s",(Chan->rejoin) ? "(Trying to rejoin...)" : "(Inactive)");
			umode = ' ';
		}
		sprintf(t1,"%c%s%s",umode,Chan->name,(Chan == current->CurrentChan) ? " (current)" : "");
		send_to_user(from,"%-35s   %s",t1,t2);
	}
	send_to_user(from,"\n[\037End of Channels\037]");
}

void rejoin_channels(void)
{
	aChan	*Chan;

	for(Chan=current->Channels;Chan;Chan=Chan->next)
	{
		if (Chan->active == CH_ACTIVE)
			continue;
		if (Chan->rejoin) {
			Chan->retry = TRUE;
			join_channel(Chan->name,Chan->key,TRUE);
		}
	}
}

int show_names_on_channel(char *from, char *channel)
{
	aChanUser *Blah;
	aChan	*Dummy;
	char	buffer[HUGE];
	int	sendit;

	
	if (!(Dummy = find_channel(channel,CH_ACTIVE)))
	{
		send_to_user(from,TEXT_CHECKOLDCHANS,channel);
		if (!(Dummy = find_channel(channel,CH_OLD)))
		{
			send_to_user(from,TEXT_NOTFOUND);
			return(FALSE);
		}
		send_to_user(from,TEXT_USERSLASTTIME,channel);
	}
	else
		send_to_user(from,TEXT_NAMESON,channel);
	Blah = Dummy->users;
	while (Blah)
	{
		sendit = 0;
		strcpy(buffer,"");
		while (Blah && (strlen(buffer) < 400))
		{
			sendit++;
			if (get_usermode(Blah) & CU_VOICE)
				Strcat(buffer,"+");
			if (get_usermode(Blah) & CU_CHANOP)
				Strcat(buffer,"@");
			Strcat(buffer,Blah->nick);
			Strcat(buffer," ");
			Blah = Blah->next;
		}
		if (sendit)
			send_to_user(from,"%s",buffer);
	}
	return TRUE;
}

char *find_nuh(char *nick)
{
	aChanUser *CU;
	aChan	*Chan;

	for(Chan=current->Channels;Chan;Chan=Chan->next)
	{
		if ((CU = find_chanuser(Chan,nick)) != NULL)
		{
			return(get_nuh(CU));
		}
	}
	return(NULL);
}

int usermode(char *channel, char *nick)
{
	aChanUser *User;
	aChan	*Chan;

	if ((Chan = find_channel(channel,CH_ACTIVE)) == NULL)
		return(0);
	if ((User = find_chanuser(Chan,nick)) != NULL)
		return(User->flags);
	return(0);
}
	
int num_banned(char *channel)
{
	aChan	*Chan;
	aBan	*Ban;
	int	n;

	if ((Chan = find_channel(channel,CH_ACTIVE)) == NULL)
		return(0);
	n = 0;
	for(Ban=Chan->banned;Ban;Ban=Ban->next)
		n++;
	return(n);
}

aBan *find_ban(aBan **b_list, char *banstring)
{
	aBan	*Ban;

	for(Ban=*b_list;Ban;Ban=Ban->next)
		if (!Strcasecmp(Ban->banstring,banstring))
			return(Ban);
	return(NULL);
}

void make_ban(aBan **b_list, char *from, char *banstring, time_t thetime)
{
	aBan *New_ban;

	if (find_ban(b_list,banstring) != NULL)
		return;
	set_mallocdoer(make_ban);
	New_ban = (aBan*)MyMalloc(sizeof(aBan));
	set_mallocdoer(make_ban);
	mstrcpy(&New_ban->banstring,banstring);
	set_mallocdoer(make_ban);
	mstrcpy(&New_ban->bannedby,(from) ? from : "<UNKNOWN>");
	New_ban->time = thetime;
	New_ban->prev = NULL;
	if (*b_list)
		(*b_list)->prev = New_ban;
 	New_ban->next = *b_list;
	*b_list = New_ban;
}

void remove_ban(aBan **b_list, aBan *cptr)
{
	if (cptr->prev)
		cptr->prev->next = cptr->next;
	else
	{
		*b_list = cptr->next;
		if (*b_list)
			(*b_list)->prev = NULL;
	}
	if (cptr->next)
		cptr->next->prev = cptr->prev;
	MyFree(&cptr->banstring);
	MyFree(&cptr->bannedby);
	MyFree((char **)&cptr);
}

int delete_ban(aBan **b_list, char *banstring)
{
	aBan *Dummy;

	if((Dummy = find_ban(b_list, banstring)) == NULL)
		return(FALSE);
	remove_ban(b_list, Dummy);
	return TRUE;	
}

void delete_bans(aBan **b_list)
{
	aBan	*old,*tmp;

	old = *b_list;
	while (old)
	{
		tmp = old->next;
		remove_ban(b_list, old);
		old = tmp;
	}
}

void channel_massmode(char *channel, char *pattern, int filtmode, char mode, char typechar)
{
	aChanUser *User;
	aChan	*Chan;
	char	*pat,*uh,burst[MAXLEN],deopstring[MAXLEN],dstring[MAXLEN];
	int	i,maxmode,mal,willdo,lvl,ispat;

	pat = chop(&pattern);
	if (!pat || !*pat)
		return;
	ispat = (strchr(pat,'*')) ? TRUE : FALSE;

	if ((Chan = find_channel(channel,CH_ACTIVE)) == NULL)
		return;
	maxmode = get_int_varc(NULL,SETOPMODES_VAR);
	mal = get_int_varc(Chan,SETMAL_VAR);

	burst[0] = 0;
	sprintf(dstring,"MODE %s %c",channel,mode);

	uh = dstring;
	while(*uh)
		uh++;
	for(i=0;i<maxmode;i++)
		*(uh++) = typechar;
	*uh = 0;

	User = Chan->users;
	while(User)
	{
		strcpy(deopstring,dstring);
		i = 0;
		while(User && (i < maxmode))
		{
			willdo = FALSE;
			if ((mode == '+') && ((User->flags & filtmode) == 0))
				willdo = TRUE;
			if ((mode == '-') && ((User->flags & filtmode) != 0))
				willdo = TRUE;
			uh = get_nuh(User);
#ifdef DEBUG
			lvl = 0;
#endif
			if (willdo)
			{
				willdo = FALSE;
				lvl = get_userlevel(uh,channel);
				if (ispat && !matches(pat,uh))
				{
					if (typechar == 'v')
						willdo = TRUE;
					if ((mode == '+') && (lvl >= mal))
						willdo = TRUE;
					if ((mode == '-') && (lvl < mal))
						willdo = TRUE;
				}
				else
				if (!ispat && !Strcasecmp(pat,User->nick))
				{
					if (mode == '-')
					{
						if (!is_me(pat))
							willdo = TRUE;
					}
					else
						willdo = TRUE;
				}
			}

#ifdef DEBUG
			debug("(massmode(2)) willdo = %s (%s[%i]) (pat=%s)\n",
				(willdo) ? "TRUE" : "FALSE",uh,lvl,pat);
#endif
			if ((willdo) && ((User->flags & CU_MASSTMP) == 0))
			{
				Strcat(deopstring," ");
				Strcat(deopstring,User->nick);
				User->flags |= CU_MASSTMP;
				i++;
			}
			User = User->next;
			if (User == NULL)
			{
				pat = chop(&pattern);
				if (pat && *pat)
				{
					ispat = (strchr(pat,'*')) ? TRUE : FALSE;
					User = Chan->users;
				}
			}
		}
		if (i)
		{
			if ((strlen(deopstring)+strlen(burst)) < 480)
			{
				Strcat(burst,deopstring);
				Strcat(burst,"\n");
			}
			else
			{
#ifdef DEBUG
				debug("(channel_massmode)\n%s\n",burst);
#endif
				write(current->sock,burst,strlen(burst));
				strcpy(burst,deopstring);
				Strcat(burst,"\n");
			}
		}
	}
	if (strlen(burst))
	{
#ifdef DEBUG
		debug("(...)\n%s\n",burst);
#endif
		write(current->sock,burst,strlen(burst));
	}
	User = Chan->users;
	while(User)
	{
		User->flags &= ~CU_MASSTMP;
		User = User->next;
	}
}

int channel_massunbanfrom(char *channel, char *pattern, time_t seconds)
{
	aChan	*Channel;
	aBan	*Dummy;
	char	unbanmode[5];
	char	unbanstring[MAXLEN];
	int	i,count,maxmode;

	if ((Channel = find_channel(channel,CH_ACTIVE)) == NULL)
		return(0);
	maxmode = get_int_varc(NULL,SETBANMODES_VAR);
	count = 0;
	Dummy = Channel->banned;
	while(Dummy)
	{
		strcpy(unbanmode, "-");
		strcpy(unbanstring, "");
		i=0;
		while(Dummy && (i < maxmode))
		{
			if (!matches(pattern, Dummy->bannedby))
			{
				if ((get_shitlevel(Dummy->banstring,channel)) != 3)
				{
					if (!seconds || ((now - Dummy->time) > seconds))
					{
						Strcat(unbanmode,"b");
						Strcat(unbanstring, " ");
						Strcat(unbanstring, Dummy->banstring);
						i++;
						count++;
					}
				}
			}
			Dummy=Dummy->next;
		}
		sendmode(channel, "%s %s", unbanmode, unbanstring);
	}
	return count;
}

int channel_massunban(char *channel, char *pattern, time_t seconds)
{
	aChan	*Channel;
	aBan	*Dummy;
	char	unbanmode[5];
	char	unbanstring[MAXLEN];
	int	i,count,maxmode;

	if ((Channel = find_channel(channel,CH_ACTIVE)) == NULL)
		return(0);
	maxmode = get_int_varc(NULL,SETBANMODES_VAR);
	count = 0;
	Dummy = Channel->banned;
	while(Dummy)
	{
		strcpy(unbanmode, "-");
		strcpy(unbanstring, "");
		i=0;
		while(Dummy && (i < maxmode))
		{
			if (!matches(pattern, Dummy->banstring) ||
			    !matches(Dummy->banstring, pattern))
			{
				if ((get_shitlevel(Dummy->banstring,channel)) != 3)
				{
					if (!seconds || ((now - Dummy->time) > seconds))
					{
						Strcat(unbanmode,"b");
						Strcat(unbanstring, " ");
						Strcat(unbanstring, Dummy->banstring);
						i++;
						count++;
					}
				}
			}
			Dummy=Dummy->next;
		}
		sendmode(channel, "%s %s", unbanmode, unbanstring);
	}
	return count;
}

int channel_unban(char *channel, char *user)
{
	aChan	*Channel;
	aBan	*Dummy;
	char	unbanmode[5];
	char	unbanstring[MAXLEN];
	int	i,count;

	if ((Channel = find_channel(channel,CH_ACTIVE)) == NULL)
		return(0);
	count = 0;
	Dummy = Channel->banned;
	while(Dummy)
	{
		strcpy(unbanmode, "-");
		strcpy(unbanstring, "");
		i=0;
		while(Dummy && (i < get_int_varc(Channel, SETBANMODES_VAR)))
		{
			if (!matches(Dummy->banstring, user))
			{
				Strcat(unbanmode,"b");
				Strcat(unbanstring," ");
				Strcat(unbanstring,Dummy->banstring);
				i++;
				count++;
			}
			Dummy=Dummy->next;
		}
		sendmode(channel, "%s %s", unbanmode, unbanstring);
	}
	return count;
}

/*
 *  Channel userlist stuff
 */

aChanUser *find_chanuser(aChan *Chan, char *nick)
{
	aChanUser *User;

	/*
	 *  A small quick'n'dirty cache
	 */
	if ((Chan->cacheuser) && (!Strcasecmp(nick,Chan->cacheuser->nick)))
		return(Chan->cacheuser);

	/*
	 *  No luck with the cache? Oh bummer...
	 */
	for(User=Chan->users;User;User=User->next)
	{
		if (User == Chan->cacheuser)
			continue;
		if (!Strcasecmp(nick,User->nick))
		{
			Chan->cacheuser = User;
			return(User);
		}
	}
	return(NULL);
}

/*
 *  Requires CurrentChan to be set properly
 */
aChanUser *make_chanuser(char *nick, char *user, char *host)
{
	aChanUser *new;

	set_mallocdoer(make_chanuser);
	/*
	 *  malloc aChanUser record with buffer space for user and host in
	 *  a single chunk and calculate the offsets for the strings
	 */
	new = (aChanUser*)MyMalloc(sizeof(aChanUser)+strlen(user)+strlen(host));
	new->host = new->user + strlen(user) + 1;
	strcpy(new->user,user);
	strcpy(new->host,host);
	set_mallocdoer(make_chanuser);
	mstrcpy(&new->nick,nick);
	new->next = CurrentChan->users;
	CurrentChan->users = new;
	new->idletime = now;
	new->floodtime = new->bantime = new->deoptime = new->kicktime = new->nicktime = new->capstime = now;
	return(new);
}

int remove_chanuser(aChan *Chan, char *nick)
{
	aChanUser *CU,**u_list;

	/*
	 *  Dont call find_chanuser() because it caches the found user
	 *  and we dont want to cache a user who quits/parts/is kicked...
	 */
	u_list = &Chan->users;
	while(*u_list)
	{
		if (!Strcasecmp(nick,(*u_list)->nick))
			break;
		u_list = &(*u_list)->next;
	}
	if (*u_list)
	{
		if ((CU = *u_list) == Chan->cacheuser)
			Chan->cacheuser = NULL;
		*u_list = (*u_list)->next;
		MyFree((char**)&CU->nick);
		MyFree((char**)&CU);
	}
	return(TRUE);
}

void delete_chanusers(aChan *Chan)
{
	aChanUser *rem,*nxt;

	rem = Chan->users;
	while(rem)
	{
		nxt = rem->next;
		MyFree((char**)&rem->nick);
		MyFree((char**)&rem);
		rem = nxt;
	}
	Chan->users = Chan->cacheuser = NULL;
}

static	char get_nuhbuf[NAMELEN];

char *get_nuh(aChanUser *User)
{
	sprintf(get_nuhbuf,"%s!%s@%s",User->nick,User->user,User->host);
	return(get_nuhbuf);
}

void add_mode(aChan *Chan, Ulong mode, char *param)
{
	aChanUser *User;

	if ((User = find_chanuser(Chan,param)) != NULL)
		User->flags |= mode;
}

void del_mode(aChan *Chan, Ulong mode, char *param)
{
	aChanUser *User;

	if ((User = find_chanuser(Chan,param)) != NULL)
		User->flags &= ~mode;
}

