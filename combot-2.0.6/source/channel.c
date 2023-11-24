/*
 * channel.c - a better implementation to handle channels on IRC
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
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#include "global.h"
#include "config.h"
#include "defines.h"
#include "structs.h"
#include "h.h"

static  char channel_buf[MAXLEN];

void check_all_steal()
{
	aChannel *Channel;

	for (Channel = current->Channel_list; Channel; Channel = Channel->next)
		if (!i_am_op(Channel->name) && tot_num_on_channel(Channel->name) == 1)
			cycle_channel(Channel->name);
}
        
void check_clonebots(from, channel)
char *from;
char *channel;
{
	aChannel *Channel;
	time_t ct;
	aTime *Bleah;
	char thehost[100];
	char *temp;

	ct = getthetime();
	if (!(Channel = find_channel(channel)))
		return;
	uptime_time(&Channel->HostList, ct-120); /* delete ones over 2m old */
	if (!(temp = gethost(from)))
		return;
	strcpy(thehost, temp);
	if (!(Bleah = find_time(&Channel->HostList, thehost)))
	{
		make_time(&Channel->HostList, thehost);
		return;
	}
	if ((ct - Bleah->time) > 10)
	{
		Bleah->time = ct;
		Bleah->num = 0;
	}
	Bleah->num++;
	if (Bleah->num >= 5)
	{
		char temp[150];
		strcpy(temp, "*!*@");
		strcat(temp, thehost);
		Bleah->num-=2; /* wait til 2 more join to do kicking again */
		channel_massunban(channel, thehost, 0);
		sendmode(channel, "+mbk %s %s", temp, "clone_protection"); 
		if (get_int_varc(Channel, TOGAS_VAR))
		{
			add_to_shitlist(&Userlist, temp, 2,
				Channel->name, "Auto-Shit",
				"Fucking lame-ass clonebots",
				getthetime(), getthetime() + (30*86400));
#ifdef USE_CCC
			send_to_ccc("1 SHIT Auto-Shit %s %s 2 %li %li %s",
                        Channel->name, temp, getthetime(),
                        getthetime()+(86400 * 30), "Fucking lame-ass clonebots");
#endif
		}	
		if (i_am_op(channel))
			clone_kick(channel, thehost, "Shitlisted for clonebots");
	}
}

long int get_idletime(nick)
char *nick;
{
	register aChannel *Channel;
	aChanUser *User;
	long int temp;

	temp = -1;
	for (Channel = current->Channel_list; Channel; Channel = Channel->next)
		if ((User = find_chanuser(&(Channel->users), nick)) != NULL)
			if ((temp == -1) || (User->idletime < temp))
			temp = User->idletime;
	return temp;
}

void show_idletimes(from, channel, seconds)
char *from;
char *channel;
int seconds;
{
	aChannel *Channel;
	register aChanUser *User;
	time_t ct;

	if (!(Channel = find_channel(channel)))
		return;
  
	send_to_user(from, "\002Users on %s that are idle more than %i second%s:\002",
			 channel, seconds, EXTRA_CHAR(seconds));
	ct = getthetime();
	for (User = Channel->users; User; User = User->next)
	if ((ct-User->idletime) > seconds)
		send_to_user(from, "%s: %s", idle2strsmall(ct-User->idletime), get_username(User));
	send_to_user(from, "\002--- end of list ---\002");
}

void check_limit(channel)
char *channel;
{
	aChannel *Channel;
	int num;

	if ((Channel = find_channel(channel)) == NULL)
		return;
	if (!Channel->userlimit)
		return;
	num = tot_num_on_channel(channel);
	if (Channel->mode & MODE_INVITEONLY)
	{
	  if (Channel->userlimit > 3)
	  {
		 if (num < (Channel->userlimit - 3))
			sendmode(channel, "-i");
	  }
	  else if (num < Channel->userlimit)
		 sendmode(channel, "-i");
	}
	else if (num >= Channel->userlimit)
		sendmode(channel, "+i");
}

int is_present(who)
char *who;   /* should be a nickname */
{
	register aChannel *Channel;
	aChanUser *User;

	for (Channel=current->Channel_list;Channel;Channel=Channel->next)
		if ((User = find_chanuser(&(Channel->users), who)) != NULL)
			return TRUE;
	return FALSE;
}

void op_all_bots()
{
	register aChannel *Channel;
	register aChanUser *User;

	for (Channel=current->Channel_list;Channel;Channel=Channel->next)
	   if (i_am_op(Channel->name))
		for(User=Channel->users;User;User=User->next)
			if (!(get_usermode(User) & MODE_CHANOP))
				if (find_list(&Bots, get_username(User)) != NULL)
					giveop(Channel->name, get_username(User));	
}

void op_bot(who)
char *who;
{
	register aChannel *Channel;
	aChanUser *User;
	char *tmp;

	tmp = getnick(who);
	for (Channel=current->Channel_list;Channel;Channel=Channel->next)
		if (i_am_op(Channel->name))
			if ((User=find_chanuser(&Channel->users, tmp)) != NULL)
				if (!(get_usermode(User) & MODE_CHANOP))
					giveop(Channel->name, tmp);	
}

void op_all_chan(who)
char *who;   /* should be a nickname */
{
	register aChannel *Channel;
	aChanUser *User;

	for (Channel = current->Channel_list; Channel; Channel = Channel->next)
		if ((User = find_chanuser(&(Channel->users), who)) != NULL)
			if (i_am_op(Channel->name))
				if (!(get_usermode(User) & MODE_CHANOP))
					if (get_userlevel(get_username(User),
						Channel->name))
						if (get_userlevel(
			get_username(User), Channel->name) >= current->restrict)
						giveop(Channel->name, who);
}
	 
void check_shit(void)
{
	register aChannel *Channel;
	register aChanUser *User;
	time_t ct;

	ct = getthetime();
	for (Channel = current->Channel_list; Channel; Channel = Channel->next)
		for( User = Channel->users; User; User = User->next )
			if (is_shitted(get_username(User), Channel->name))
				shit_action(get_username(User), Channel->name);
}

void check_protnicks(void)
{
	register aChannel *Channel;
	register aChanUser *User;
	aUser *Lame;
	time_t ct;

	ct = getthetime();
	for (Channel = current->Channel_list; Channel; Channel = Channel->next)
		for( User = Channel->users; User; User = User->next )
		{
			if ((Lame=find_shit(&Userlist,get_username(User), "*")) != NULL)
				if (max_userlevel(get_username(User)) < Lame->access)
				{
					sendmode(Channel->name,
							"-o+b %s *!*%s@%s",
					 User->nick, fixusername(User->user),
						cluster(User->host)); 
					sendkick(Channel->name,
				 User->nick, "\002%s\002", Lame->reason);
				 }
		}
}

void check_idle(void)
{
	register aChannel *Channel;
	register aChanUser *User;
	time_t ct;

	ct = getthetime();
	for (Channel = current->Channel_list; Channel; Channel = Channel->next)
		for( User = Channel->users; User; User = User->next )
			if ((ct - User->idletime) >=
				(60*get_int_varc(Channel, SETIKT_VAR)))
			{
				if (!is_user(get_username(User),
						Channel->name) &&
					get_int_varc(Channel, TOGIK_VAR) &&
					!(get_usermode(User) & MODE_CHANOP))
					sendkick(Channel->name, User->nick,
					"\002Idle for %s\002",
					idle2strsmall(getthetime()-
						User->idletime));
				User->idletime = getthetime();
			}
}

void update_idletime(from, channel)
char *from;
char *channel;
{
	aChannel *Channel;
	aChanUser *User;
	if (!(Channel = find_channel(channel)))
		return;
	if (!(User = find_chanuser(&(Channel->users), currentnick)))
		return;
	User->idletime = getthetime();
}

int check_beepkick(from, channel, num)
char *from;
char *channel;
int num;
{
	aChannel *Channel;
	aChanUser *User;

	if (!(Channel = find_channel(channel)))
		return FALSE;
	if (!(User = find_chanuser(&(Channel->users), currentnick)))
		return FALSE;
	if ((getthetime() - User->beeptime) > 10)
	{
		User->beeptime = getthetime();
		User->beepnum = 0;
	}
	if (!is_user(get_username(User), Channel->name) &&
			!(get_usermode(User) & MODE_CHANOP))
		User->beepnum += num;
	if ((User->beepnum >= get_int_varc(Channel, SETBKL_VAR)) && 
		get_int_varc(Channel, TOGBK_VAR))
		return TRUE;
	return FALSE;
}

int check_capskick(from, channel, num)
char *from;
char *channel;
int num;
{
	aChannel *Channel;
	aChanUser *User;

	if (!(Channel = find_channel(channel)))
		return FALSE;
	if (!(User = find_chanuser(&(Channel->users), currentnick)))
		return FALSE;
	if ((getthetime() - User->capstime) > 10)
	{
		User->capstime = getthetime();
		User->capsnum = 0;
	}
	if (!is_user(get_username(User), Channel->name) &&
			!(get_usermode(User) & MODE_CHANOP))
		User->capsnum += num;
	if ((User->capsnum >= get_int_varc(Channel, SETCKL_VAR)) &&
			get_int_varc(Channel, TOGCK_VAR))
		return TRUE;
	return FALSE;
}

int check_massdeop(from, channel)
char *from;
char *channel;
{
	aChannel *Channel;
	aChanUser *User;
  
	if (!(Channel = find_channel(channel)))
		return FALSE;
	if (!(User = find_chanuser(&(Channel->users), currentnick)))
		return FALSE;
	if ( (getthetime() - User->deoptime) > 10)
	{
		User->deoptime = getthetime();
		User->deopnum = 0;
	}
	User->deopnum++;
	if (User->deopnum >= get_int_varc(Channel, SETMDL_VAR))
		return TRUE;
	return FALSE;
}

int check_massban(from, channel)
char *from;
char *channel;
{
	aChannel *Channel;
	aChanUser *User;
  
	if (!(Channel = find_channel(channel)))
		return FALSE;
	if (!(User = find_chanuser(&(Channel->users), currentnick)))
		return FALSE;
	if ( (getthetime() - User->bantime) > 10)
	{
		User->bantime = getthetime();
		User->bannum = 0;
	}
	User->bannum++;
	if (User->bannum >= get_int_varc(Channel, SETMBL_VAR))
		return TRUE;
	return FALSE;
}

int check_masskick(from, channel)
char *from;
char *channel;
{
	aChannel *Channel;
	aChanUser *User;
  
	if (!(Channel = find_channel(channel)))
		return FALSE;
	if (!(User = find_chanuser(&(Channel->users), currentnick)))
		return FALSE;
	if ((getthetime() - User->kicktime) > 10)
	{
		User->kicktime = getthetime();
		User->kicknum = 0;
	}
	User->kicknum++;
	if (User->kicknum >= get_int_varc(Channel, SETMKL_VAR))
		return TRUE;
	return FALSE;
}

int check_nickflood(from)
char *from;
{
	register aChannel *Channel;
	aChanUser *User;
	int flag = FALSE;

	for (Channel = current->Channel_list; Channel; Channel = Channel->next)
		if ((User = find_chanuser(&(Channel->users), currentnick)) != NULL)
		{
			if ((getthetime() - User->nicktime) > 10)
			{
				User->nicktime = getthetime();
				User->nicknum = 0;
			}
			User->nicknum++;
			if (User->nicknum >= get_int_varc(Channel, SETNCL_VAR))
			{
				char *temp;
				flag = TRUE;
				temp = format_uh(from, 1);
				if (!is_user(from, Channel->name) &&
					!(get_usermode(User) & MODE_CHANOP))
				{
					if (get_int_varc(Channel, TOGAS_VAR))
					{
						add_to_shitlist(
							&Userlist,
							temp, 2,
							Channel->name,
							"Auto-Shit",
				 "Quit the nick floods you lamer",
							getthetime(),
			getthetime() + (long) 86400);   /* 86400 == 1 day */
#ifdef USE_CCC
                send_to_ccc("1 SHIT Auto-Shit %s %s 2 %li %li %s",
                        Channel->name, temp, getthetime(),
                        getthetime()+86400,
			"Quit the nick floods you lamer");
#endif
					}
				}
				if (i_am_op(Channel->name))
				{
					deop_ban(Channel->name, currentnick,
						temp);
					sendkick(Channel->name, currentnick,
	"\002Get the hell out nick-flooding lamer!!!\002");
				}
			}
		}
	return flag;
}

int check_pubflooding(from, channel)
char *from;
char *channel;
{
	aChannel *Channel;
	aChanUser *User;
  
	if (!(Channel = find_channel(channel)))
		return FALSE;
	if (!(User = find_chanuser(&(Channel->users), currentnick)))
		return FALSE;
	if ( (getthetime() - User->floodtime) > 5)
	{
		User->floodtime = getthetime();
		User->floodnum = 0;
	}
	User->floodnum++;
	if (get_usermode(User) & MODE_CHANOP)
		return FALSE;
	if (User->floodnum >= get_int_varc(Channel, SETFL_VAR))
		return get_int_varc(Channel, SETFPL_VAR);
	return FALSE;
}

int tot_num_on_channel(channel)
char *channel;
{
	aChannel *Channel;
	register aChanUser *User;
	int num;

	num = 0;
	if ((Channel = find_channel(channel)) == NULL)
		return 0;
	for (User=Channel->users;User;User=User->next)
		num++;
	return num;
}

aChannel *find_channel(name)
char *name;
{
	register aChannel *Channel;

	for (Channel = current->Channel_list; Channel; Channel = Channel->next)
		if (!my_stricmp(name, Channel->name))
			return Channel;
	return NULL;
}

aChannel *find_oldchannel(name)
char *name;
{
	register aChannel *Channel;
	for (Channel = current->OldChannel_list; Channel; Channel = Channel->next)
		if (!my_stricmp(name, Channel->name))
			return Channel;
	return NULL;
}

void newchannel_to_oldchannel(channel)
aChannel *channel;
{
	aChannel **old;
	
	for (old = &(current->Channel_list); *old; old = &(**old).next)
		if (*old == channel)
		{
			*old = channel->next;
			break;
		}
	channel->next = current->OldChannel_list;
	current->OldChannel_list = channel;
}

void oldchannel_to_newchannel(channel)
aChannel *channel;
{
	aChannel **old;
	
	for (old = &(current->OldChannel_list); *old; old = &(**old).next)
		if (*old == channel)
		{
			*old = channel->next;
			break;
		}
	channel->next = current->Channel_list;
	current->Channel_list = channel;

}

int delete_channel(l_list, channel)
aChannel **l_list;
aChannel *channel;
{
	aChannel **old;

	for (old = l_list; *old; old = &(**old).next)
		if (*old == channel)
		{
			*old = channel->next;
			delete_chanusers(&(channel->users));
			delete_bans(&(channel->banned));
			delete_list(&channel->SpyList);
			delete_time(&channel->HostList);
			MyFree(&channel->name);
			MyFree(&channel->key);
			MyFree(&channel->topic);
			MyFree(&channel->kickedby);
			free_strvars(channel->Vars);
			MyFree(&channel->collectby);
			MyFree((char **)&channel);
			return TRUE;
		}
	return FALSE;
}

void delete_all_channels(void)
{
	aChannel *channel;

	for(; (channel = current->Channel_list) != NULL;)  /* Why not while ? -- CS */
		delete_channel(&current->Channel_list, channel);
	for(; (channel = current->OldChannel_list) != NULL;)
		delete_channel(&current->OldChannel_list, channel);
}

void copy_channellist(c_list)
aChannel *c_list;
{
	aChannel *tmp;

	for (tmp = c_list; tmp; tmp = tmp->next)
	{
		join_channel(tmp->name, tmp->key, FALSE);
		set_enftopic(tmp->name, tmp->topic);
	}

}

int join_channel(name, key, dojoin)
char *name;
char *key;
int dojoin;
{
	aChannel *Channel;

	if (!ischannel(name))
		return(FALSE);

	if (!(Channel = find_channel(name)))
	{
		if (!(Channel = find_oldchannel(name)))      
		{
			Channel = (aChannel *) MyMalloc(sizeof(aChannel));
			mstrcpy(&Channel->name, name);
/*
	MyMalloc zero's these now...
			Channel->users = NULL;
			Channel->banned = NULL;
			Channel->mode = 0;
			Channel->topic = NULL;
			Channel->kickedby = NULL;
			Channel->userlimit = 0;
			Channel->collectby = NULL;
			Channel->SpyList = NULL;
			Channel->HostList = NULL;
*/
			Channel->flags |= REJOIN_TYPE;
			mstrcpy(&Channel->key, key);
			copy_vars(Channel->Vars, DefaultVars);
			Channel->next = current->OldChannel_list;
			current->OldChannel_list = Channel;
 			if (dojoin)
				sendjoin(name, key);
		}
		else   /* in the old channel struct */
		{
			Channel->flags |= REJOIN_TYPE;
			if (key && (key != Channel->key))
			{
				MyFree(&Channel->key);
				mstrcpy(&Channel->key, key);
			}
			if (dojoin)
				sendjoin(name, Channel->key);
		}
	}
	else
	{
		current->Current_chan = Channel;
		if (key && (key != Channel->key))
		{
			MyFree(&Channel->key);
			mstrcpy(&Channel->key, key);
		}
		if (dojoin)
			sendjoin(name, Channel->key);
	}       
	return TRUE;
}

void remove_channel(name)
char *name;
{
	aChannel *Dummy;
  
	if ((Dummy=find_channel(name)) != NULL)
	{
		if (Dummy==current->Current_chan)
			current->Current_chan = current->Channel_list;
		delete_channel(&current->Channel_list, Dummy);
	}
	if ((Dummy=find_oldchannel(name)) != NULL)
		delete_channel(&current->OldChannel_list, Dummy);
}

int leave_channel(name)
char *name;
{
	aChannel *Dummy;

	if ((Dummy = find_channel(name)) != NULL)
	{
		newchannel_to_oldchannel(Dummy);
		sendpart(name);

		if (Dummy == current->Current_chan)
			current->Current_chan = current->Channel_list;
		return 1;
	}
	if ((Dummy = find_oldchannel(name)) == NULL)
		return 0;
	if (Dummy->flags & REJOIN_TYPE)
	{
		Dummy->flags &= ~REJOIN_TYPE;
		return 2;
	}
	return 0;
}

int reverse_topic(from, channel, topic)
char *from;
char *channel;
char *topic;
{
	aChannel *Chan;

	if (!(Chan = find_channel(channel)))
		return 0;
	if (!get_int_varc(Chan, TOGTOP_VAR))
	{
		MyFree(&Chan->topic);
		mstrcpy(&Chan->topic, topic);
		return 0;
	}
	if (get_userlevel(from, channel) >= ASSTLEVEL)
	{
		MyFree(&Chan->topic);
		mstrcpy(&Chan->topic, topic);
		return 0;
	}
	if (Chan->topic)
	{
		send_to_server("TOPIC %s :%s", channel, Chan->topic);
		return 1;
	}
	mstrcpy(&Chan->topic, topic);
	return 0;
}

int reverse_mode(from, channel, m, s)
char *from;
char *channel;
int m;
int s;
{
	char *ptr;
	char mode, sign;

	mode = (char) m;
	sign = (char) s;
	if (!get_int_var(channel, TOGENFM_VAR))
		return FALSE;
	if (!(ptr = get_str_var(channel, SETENFM_VAR)))
		return FALSE;
	if (strchr(ptr, mode) && (sign == '+'))
		return FALSE;
	if (!strchr(ptr, mode) && (sign == '-'))
		return FALSE;
	if (get_userlevel(from, channel) >= ASSTLEVEL)
	{
		char buffer[100];
		char *ptr2;

		ptr2 = buffer;
		if (sign == '-')
		{
			while (*ptr)
			{
				if (*ptr != mode)
					*ptr2++ = *ptr;
				ptr++;
			}
			*ptr2 = '\0';
		}
		else
		{
			buffer[0] = mode;
			buffer[1] = '\0';
			strcat(buffer, ptr);
		}
		set_str_var(channel, SETENFM_VAR, buffer);
		return FALSE;
	}
	return TRUE;
}

void cycle_channel(channel)
char *channel;
{
	aChannel *Chan;

	if (!(Chan = find_channel(channel)))
		return;
	Chan->flags |= REJOIN_TYPE;
	leave_channel(channel);
	join_channel(Chan->name, Chan->key, 1);
}

int mychannel(name)
char *name;
{
	return (find_channel(name) != NULL);
}

int myoldchannel(name)
char *name;
{
	return (find_oldchannel(name) != NULL);
}

int add_spy(name, nuh)
char *name;
char *nuh;
{
	aChannel *Channel;
	if ((Channel = find_channel(name)))
		return (make_list(&Channel->SpyList, nuh)!=NULL);
	return FALSE;
}

int del_spy(name, nuh)
char *name;
char *nuh;
{
	aList *tmp;
	aChannel *Channel;
	if ((Channel = find_channel(name)))
	{
		if ((tmp=find_list(&Channel->SpyList, nuh)) == NULL)
			return 0;
		remove_list(&Channel->SpyList, tmp);
		return 1;
	}
	return FALSE;
}

void show_spy_list(from, name)
char *from;
char *name;
{
	register aList *temp;
	aChannel *Channel;

	if ((Channel = find_channel(name)))
	  for (temp=Channel->SpyList;temp;temp=temp->next)
		 send_to_user(from, temp->name);
}

void send_spymsg(char *format, ...)
{
  char buf[HUGE];
  va_list msg;
  aList *temp;
  
  va_start(msg, format);
  vsprintf(buf, format, msg);
  va_end(msg);
  
  for(temp=current->SpyMsgList;temp;temp=temp->next)
	 send_to_user(temp->name, "%s", buf);
}

void send_common_spy(char *from, char *format, ...)
{
	char buf[HUGE];
	va_list msg;
	register aChannel *Channel;
	char *thenick;

	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);

	thenick = currentnick;
	for (Channel = current->Channel_list; Channel; Channel = Channel->next)
	   if (find_chanuser(&(Channel->users), thenick) != NULL)
		send_spy(Channel->name, "%s", buf);
}

void send_spy(char *channel, char *format, ...)
{
	char buf[HUGE];
	va_list msg;
	register aList *temp;
	aChannel *Channel;

	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);

	if ((Channel = find_channel(channel)))
	  for (temp=Channel->SpyList;temp;temp=temp->next)
		 if (ischannel(temp->name))
			sendprivmsg(temp->name, "%s", buf);
		 else
			send_to_user(temp->name, "%s", buf);
}

int is_spy(chan, nuh)
char *chan;
char *nuh;
{
	aChannel *Channel;
	if ((Channel = find_channel(chan)))
		return (find_list(&Channel->SpyList, nuh) != NULL);
	return FALSE;
}

int get_limit(name)
char *name;
{
	aChannel *Dummy;
  
	if ((Dummy = find_channel(name)) != NULL)
		return Dummy->userlimit;
	return 0;
}

int set_limit(name, num)
char *name;
int num;
{
	aChannel *Dummy;
  
	if ((Dummy = find_channel(name)) != NULL)
	{
		Dummy->userlimit = num;
		return TRUE;
	}
	return FALSE;
}

int set_kickedby(name, from)
char *name;
char *from;
{
	aChannel *Dummy;

	if ((Dummy = find_channel(name)) != NULL)
	{
		MyFree(&Dummy->kickedby);
		if (from)
			mstrcpy(&Dummy->kickedby, from);
		else
			Dummy->kickedby = NULL;
		return TRUE;
	}
	return FALSE;
}

char *get_kickedby(name)
char *name;
{
	aChannel *Dummy;

	if ((Dummy = find_channel(name)) != NULL)
		return Dummy->kickedby;
	return NULL;
}

char *get_enftopic(name)
char *name;
{
	aChannel *Dummy;

	if ((Dummy = find_channel(name)) != NULL)
		return Dummy->topic;
	return NULL;
}

int set_enftopic(name, topic)
char *name;
char *topic;
{
	aChannel *Dummy;

	if ((Dummy = find_channel(name)) != NULL)
	{
		MyFree(&Dummy->topic);
		mstrcpy(&Dummy->topic, topic);
		return TRUE;
	}
	return FALSE;
}

void show_channel_listp(from, l_list)
char *from;
aChannel *l_list;
{
	register aChannel *Channel;
	unsigned int mode;
	char modestr[MAXLEN];

	for (Channel = l_list; Channel;Channel=Channel->next )
	{
		mode = Channel->mode;
		strcpy( modestr, "" );
/*		if (mode & MODE_ANONYMOUS)  strcat(moedstr, "a");
*/		if (mode & MODE_PRIVATE)    strcat(modestr, "p");
		if (mode & MODE_SECRET)     strcat(modestr, "s");
		if (mode & MODE_MODERATED)  strcat(modestr, "m");
		if (mode & MODE_TOPICLIMIT) strcat(modestr, "t");
		if (mode & MODE_INVITEONLY) strcat(modestr, "i");
		if (mode & MODE_NOPRIVMSGS) strcat(modestr, "n");
		if (mode & MODE_KEY) 
		{
			strcat( modestr, "k " );
			strcat( modestr, Channel->key?Channel->key:"???" );
			strcat( modestr, " " );
		}
		if (mode & MODE_LIMIT)
		{
			strcat( modestr, "l " );
			strcat( modestr, Channel->limit ? Channel->limit:"???");
		}
		send_to_user(from, "        -  %s, mode: +%s%s%s", 
			Channel->name, modestr,
			Channel->flags ? "  " : "",
			Channel->flags == 0 ? "" :
			Channel->flags & COLLECT_TYPE ? "\002Trying to collect data...\002" :
			Channel->flags & REJOIN_TYPE ? "\002Trying to rejoin...\002" :
					"");
	}
} 

void show_channel_list(from)
char *from;
{
	if (current->Channel_list == NULL)
		send_to_user(from, "\002I'm not active on any channels\002");
	else
	{
		send_to_user(from, "\002Current channel: %s\002",
			current->Channel_list->name);
		send_to_user(from, "\002Channels I'm currently on:\002");
		show_channel_listp(from, current->Channel_list);
	}
	if (!current->OldChannel_list)
		send_to_user(from,
			"\002There are no other channels in memory\002");
	else
	{
		send_to_user(from, "\002Other channels in memory:\002");
		show_channel_listp(from, current->OldChannel_list);
	}
}
         
void rejoin_channels(Mode)
int Mode;
{
	aChannel *Channel;

	for (Channel=current->Channel_list;Channel;Channel=Channel->next)
		if (Mode == HARDRESET)
			join_channel(Channel->name, Channel->key, TRUE);
	for (Channel=current->OldChannel_list;Channel;Channel=Channel->next)
		if (Channel->flags & REJOIN_TYPE)
			join_channel(Channel->name, Channel->key, TRUE);
}

char *currentchannel(void)
{
	if (current->Current_chan)
		if (!current->Current_chan->name)
			strcpy(channel_buf, "#0");
		else
			strcpy(channel_buf, current->Current_chan->name);
	else
		strcpy(channel_buf, "#0");
	return channel_buf;
}

int add_user_to_channel(channel, nick, user, host)
char *channel;
char *nick;
char *user;
char *host;
{
	aChannel *Dummy;

	if (!(Dummy = find_channel(channel)))
		if ((Dummy = find_oldchannel(channel)) == NULL)
			return(FALSE);
	make_chanuser( &(Dummy->users), nick, user, host );
	return(TRUE);
}

int remove_user_from_channel(channel, nick)
char *channel;
char *nick;
{
	aChannel *Dummy;

	if ((Dummy = find_channel(channel)) == NULL)
		if ((Dummy = find_oldchannel(channel)) == NULL)
			return FALSE;
	delete_chanuser(&(Dummy->users), nick);
	return TRUE;
}

void change_nick(oldnick, newnick)
char *oldnick;
char *newnick;
{
	aChannel *Channel;

	for (Channel = current->Channel_list; Channel; Channel = Channel->next)
		change_chanuser_nick(&(Channel->users), oldnick, newnick);
}

void remove_user_from_all_channels(nick)
char *nick;
/*
 * Remove a user from all channels (signoff)
 */
{
	register aChannel *Channel;

	for (Channel = current->Channel_list; Channel; Channel = Channel->next)
	{
		delete_chanuser(&(Channel->users), nick);
		check_limit(Channel->name);
	}
}

void ban_all_channels(banstring)
char *banstring;
{
	register aChannel *Channel;
	for (Channel = current->Channel_list; Channel; Channel = Channel->next)
		if (i_am_op(Channel->name))
			sendmode("%s +b %s", Channel->name, banstring);
}

void kick_all_channels(nick, reason)
char *nick;
char *reason;
{
	register aChannel *Channel;
	for (Channel = current->Channel_list; Channel; Channel = Channel->next)
		if (i_am_op(Channel->name))
			sendkick(Channel->name, nick, "%s", reason);
}

void kickban_all_channels(nick, banstring, reason)
char *nick;
char *banstring;
char *reason;
{
	register aChannel *Channel;
	for (Channel = current->Channel_list; Channel; Channel = Channel->next)
		if (i_am_op(Channel->name))
	  	{
			sendmode("%s %s+b %s%s%s",
				nick ? "-o" : "",
				nick ? nick : "",
				nick ? " " : "",
				Channel->name, banstring);
			sendkick(Channel->name, nick, "%s", reason);
	  	}
}

void chan_stats(from, channel)
char *from;
char *channel;
{
	aChannel *Dummy;
	register aChanUser *Blah;
	int ops, tot;

	ops = 0;
	tot = 0;

	if (!(Dummy = find_channel(channel)))
	{
		send_to_user(from, "\002I'm not on %s\002", channel);
		return;
	}

	Blah = Dummy->users;
	while (Blah)
	{
		tot++;
		if (get_usermode(Blah)&MODE_CHANOP)
			ops++;
		Blah = Blah->next;
	}
	send_to_user(from, "\002Out of %i people on %s, %i are ops\002",
			 tot, channel, ops);
}

int show_names_on_channel(from, channel)
char *from;
char *channel;
{
	aChannel *Dummy;
	register aChanUser *Blah;
	char     buffer[HUGE];
	int      sendit;

        if (!(Dummy = find_channel(channel)))
        {
                send_to_user(from,
                "\002I'm not on %s, checking old channels...\002", channel);
                if (!(Dummy = find_oldchannel(channel)))
                {
                        send_to_user(from, "\002Nope...not found\002");
                        return FALSE;
                }
                send_to_user(from,
                        "\002Users on %s the last time I was there:\002",
				channel);
        }
        else
                send_to_user(from, "\002Names on %s:\002", channel);
	Blah = Dummy->users;
	while (Blah)
	{
		sendit = 0;
		strcpy(buffer, "");
		while (Blah && (strlen(buffer) < 400))        
		{
			sendit++;
			if (get_usermode(Blah)&MODE_VOICE)
				strcat(buffer, "+");
			if (get_usermode(Blah)&MODE_CHANOP)
				strcat(buffer, "@");
			strcat(buffer, Blah->nick);
			strcat(buffer, " ");
			Blah = Blah->next;
		}
		if (sendit)
			send_to_user(from, "\002%s\002", buffer);
	}
	return TRUE;
}

int show_users_on_channel(from, channel, pattern, flags)
char *from;
char *channel;
char *pattern;
int  flags;
{
	aChannel *Dummy;
	register aChanUser *Blah;
	char modestr[MAXLEN];

	if (!(Dummy = find_channel(channel)))
	{
		send_to_user(from,
		"\002I'm not on %s, checking old channels...\002", channel);
		if (!(Dummy = find_oldchannel(channel)))
		{
			send_to_user(from, "\002Nope...not found\002");
			return FALSE;
		}
		send_to_user(from,
			"\002Users on %s the last time I was there:\002",
				channel);
	}
	else
		send_to_user(from, "\002Users on %s:\002", channel);	
	Blah = Dummy->users;
	while (Blah)
	{
		char *temp;
		int themode;

		temp = get_username(Blah);
		themode = get_usermode(Blah);

		if ((!flags || ((flags==1)&&(themode&MODE_CHANOP)) ||
			((flags==2)&& !(themode&MODE_CHANOP))) &&
			(!pattern || !*pattern || !matches(pattern, temp)))
		{ 
			strcpy(modestr, "");
		if (get_usermode(Blah)&MODE_CHANOP) strcat(modestr, "+o");
		if (get_usermode(Blah)&MODE_VOICE)  strcat(modestr, "+v");
		send_to_user( from, "%3du %ds, mode: %-4s -> %s", 
				  get_userlevel(temp, channel),
				  get_shitlevel(temp, channel),
				  modestr, temp);
		}
		Blah = Blah->next;
	}
	return TRUE;
}

void add_channelmode(from, channel, mode, params, time)
char *from;
char *channel;
unsigned int mode;
char *params;
time_t time;
{
	aChannel *Channel;

	if (!(Channel = find_channel(channel)))
		return;

	switch(mode)
	{
		case MODE_CHANOP:
		case MODE_VOICE:
			add_mode(&(Channel->users), mode, params);
			break;
		case MODE_KEY:
			Channel->mode |= MODE_KEY;
			MyFree(&Channel->key);
			mstrcpy(&Channel->key, params);
			break;
		case MODE_BAN:
			make_ban(&(Channel->banned), from, params, time);
			break;
		case MODE_LIMIT:
			Channel->mode |= MODE_LIMIT;
			strcpy(Channel->limit, params);
			break;
		default:
			Channel->mode |= mode;
			break;
	}
}

void del_channelmode(channel, mode, params)
char *channel;
unsigned int mode;
char *params;
{
	aChannel *Channel;

	if (!(Channel = find_channel(channel)))
		return;

	switch(mode)
	{
		case MODE_CHANOP:
		case MODE_VOICE:
			del_mode( &(Channel->users), mode, params );
			break;
		case MODE_KEY:
			Channel->mode &= ~MODE_KEY;
			break;
		case MODE_BAN:
			delete_ban(&(Channel->banned), params);
			break;
		case MODE_LIMIT:
			Channel->mode &= ~MODE_LIMIT;
			break;
		default:
			Channel->mode &= ~mode;
			break;
	}
}

void change_usermode(channel, user, mode)
char *channel;
char *user;
unsigned int mode;
{
	aChannel *Channel;
	
	if (!(Channel = find_channel(channel)))
		return;
	add_mode(&(Channel->users), mode, user );
}
	
int open_channel(channel)
char *channel;
{
	aChannel *Channel;

	if (!(Channel = find_channel(channel)))
		return FALSE;
	if ((Channel->mode)&MODE_KEY)
		sendmode(channel, "-mipslk %s", Channel->key);
	else
		sendmode(channel, "-mipsl");
	return TRUE;
}

int is_opped(nick, channel)
char *nick;
char *channel;
{
	return (usermode(channel, nick) & MODE_CHANOP);
}

int invite_to_channel(users, channel)
char *users;
char *channel;
{
	if (find_channel(channel))
	{
		 while (users && *users)
			send_to_server("INVITE %s %s", get_token(&users, " "),
				 channel);
		return TRUE;
	}
	return FALSE;
}

char *username(nick)
char *nick;
{
	aChannel *Channel;
	aChanUser *User;

	for (Channel = current->Channel_list; Channel; Channel = Channel->next)
		if ((User = find_chanuser(&(Channel->users), nick)) != NULL)
			return get_username(User);
	return NULL;
}

unsigned int usermode(channel, nick)
char *channel;
char *nick;
{
	aChannel *Channel;
	register aChanUser *User;

	if (!(Channel = find_channel(channel)))
		return 0;
	if ((User = find_chanuser(&(Channel->users), nick)) != NULL)
		return get_usermode(User);
	return 0;
}
	
int num_banned(channel)
char *channel;
{
	aChannel *Channel;
	aBan *Banned;
	int num;

	num = 0;
	if (!(Channel = find_channel(channel)))
		return 0;
	for (Banned = Channel->banned; Banned; Banned = Banned->next)
		num++;
	return num;
}

void auto_unban(amt)
int amt;
{
	register aChannel *Channel;
	static time_t last = 0;

	if (getthetime()-last < 30)
		return;
	last = getthetime();
	for (Channel = current->Channel_list; Channel; Channel = Channel->next)
		channel_massunban(Channel->name, "*!*@*", amt*60);
}
 
int is_banned(who, channel)
char *who;
char *channel;
{
	aChannel *Channel;
	register aBan *Banned;

	if (!(Channel = find_channel(channel)))
		return FALSE;
	for (Banned = Channel->banned; Banned; Banned = Banned->next)
		if (!matches(Banned->banstring, who))
			return TRUE;
	return FALSE;
}

aBan *find_ban(b_list, banstring)
aBan **b_list;
char *banstring;
{
	register aBan *Banned;

	for (Banned = *b_list; Banned; Banned = Banned->next)
		if (!my_stricmp(Banned->banstring, banstring))
			return Banned;
	return NULL;
}

void show_banlist(char *from, char *channel)
{
	aChannel *Channel;
	register aBan *Banned;
	long int time;

	time = getthetime();
	if (!(Channel = find_channel(channel)))
	{
		send_to_user(from, "\002I'm not on %s\002", channel);
		return;
	}
	for (Banned = Channel->banned; Banned; Banned = Banned->next)
		send_to_user(from, "\002%s %s ago, by %s\002",
			Banned->banstring,
			idle2strsmall(time-Banned->time),
			Banned->bannedby);
}

void make_ban(b_list, from, banstring, thetime)
aBan **b_list;
char *from;
char *banstring;
time_t thetime;
{
	aBan *New_ban;

	if (find_ban(b_list, banstring) != NULL)
        	return;
	New_ban = (aBan *)MyMalloc(sizeof(aBan));
	mstrcpy(&New_ban->banstring, banstring);
	if (from)
		mstrcpy(&New_ban->bannedby, from);
	else
		mstrcpy(&New_ban->bannedby, "<UNKNOWN>");
	New_ban->time = thetime;
        New_ban->prev = NULL;
        if (*b_list)
                (*b_list)->prev = New_ban;
 	New_ban->next = *b_list;
	*b_list = New_ban;
}

void remove_ban(b_list, cptr)
aBan **b_list;
aBan *cptr;
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

int delete_ban(b_list, banstring)
aBan **b_list;
char *banstring;
{
	aBan *Dummy;

	if((Dummy = find_ban(b_list, banstring)) == NULL)
		return(FALSE);
	remove_ban(b_list, Dummy);
	return TRUE;	
}

void delete_bans(b_list)
aBan **b_list;
{
	register aBan *old, *tmp;

	old = *b_list;
	while (old)
	{
		tmp = old->next;
		remove_ban(b_list, old);
		old = tmp;
	}
}

int channel_massop(channel, pattern)
char *channel;
char *pattern;
{
	aChannel *Channel;
	register aChanUser *Dummy;
	char      opstring[MAXLEN];
	int       i, count;

	if (!(Channel = find_channel(channel)))
		return 0;
	count = 0;
	Dummy = Channel->users;
	while (Dummy)
	{
		strcpy(opstring, "+oooo ");
		i=0;
		while (Dummy && (i < get_int_varc(Channel, SETOPMODES_VAR)))
		{
			if (!(get_usermode(Dummy) & MODE_CHANOP) &&
				 ((*pattern && !matches(pattern, get_username(Dummy))) ||
				  (!*pattern && is_user(get_username(Dummy), channel))))
			{
				strcat(opstring, " ");
				strcat(opstring, get_usernick(Dummy));
				i++;
				count++;
			}
			Dummy=Dummy->next;
		}
		sendmode(channel, "%s", opstring);
	}
	return count;
}

int channel_massdeop(channel, pattern)
char *channel;
char *pattern;
{
	aChannel *Channel;
	register aChanUser *Dummy;
	char      deopstring[MAXLEN];
	int       i, count;

	if ((Channel = find_channel(channel)) == NULL)
		return 0;
	count = 0;
	Dummy = Channel->users;
	while(Dummy)
	{
		strcpy(deopstring, "-oooo ");
		i=0;
		while(Dummy && (i < get_int_varc(Channel, SETOPMODES_VAR)))
		{
			if ((get_usermode(Dummy) & MODE_CHANOP) &&
				 ((*pattern && !matches(pattern, get_username(Dummy)) &&
						(get_userlevel(get_username(Dummy), channel) <= get_int_varc(Channel, SETMAL_VAR))) ||
				  (!*pattern && !is_user(get_username(Dummy), channel))))
			{
				strcat(deopstring, " ");
				strcat(deopstring, get_usernick(Dummy));
				i++;
				count++;
			}
			Dummy=Dummy->next;
		}
		sendmode(channel, "%s", deopstring);
	}
	return count;
}

int channel_masskick(channel, pattern, reason)
char *channel;
char *pattern;
char *reason;
{
	aChannel *Channel;
	register aChanUser *Dummy;
	char buffer[MAXLEN];
	int count;

	if( (Channel = find_channel(channel)) == NULL )
		return 0;
	count = 0;
	sprintf(buffer, "(%s) \002%s\002",
		(pattern && *pattern) ? pattern : "non-users",
		(reason && *reason) ? reason : "idiot");
	Dummy = Channel->users;
	while(Dummy)
	{
		if ((*pattern && !matches(pattern, get_username(Dummy)) &&
				  (get_userlevel(get_username(Dummy), channel) <= get_int_varc(Channel, SETMAL_VAR))) ||
			 (!*pattern && !is_user(get_username(Dummy), channel)))
		{
			sendkick(channel, get_usernick(Dummy), buffer);
			count++;
		}
		Dummy=Dummy->next;
	}
	return count;
}

int clone_kick(channel, pattern, reason)
char *channel;
char *pattern;
char *reason;
{
	aChannel *Channel;
	register aChanUser *Dummy;
        char buffer[MAXLEN];
        int count;

        if( (Channel = find_channel(channel)) == NULL)
                return 0;
        count = 0;
        sprintf(buffer, "(%s)\002%s\002", pattern, reason);
        Dummy = Channel->users;
        while (Dummy)
        {
		if (!matches(pattern, get_username(Dummy)) &&
			!(get_usermode(Dummy) & MODE_CHANOP) &&
        			!get_userlevel(get_username(Dummy), channel))
		{
			sendkick(channel, get_usernick(Dummy), buffer);
			count++;
		}
                Dummy=Dummy->next;
        }
        return count;
}

int channel_masskickban(channel, pattern, reason)
char *channel;
char *pattern;
char *reason;
{
	aChannel *Channel;
	register aChanUser *Dummy;
	char buffer[MAXLEN];
		  int count;

	if (!(Channel = find_channel(channel)))
		return 0;
	count = 0;
	sprintf(buffer, "(%s) \002%s\002",
		(pattern && *pattern) ? pattern : "non-users",
		(reason && *reason) ? reason : "idiot");
	if (!strchr(pattern, '!'))
	{
		char tempbuf[255];
		strcpy(tempbuf, "*!");
		if (!strchr(pattern, '@'))
			strcat(tempbuf, "*@");
		strcat(tempbuf, pattern);
		send_to_server("MODE %s +b %s", channel, tempbuf);
	}
	else
			send_to_server("MODE %s +b %s", channel, pattern);
	Dummy = Channel->users;
	while (Dummy)
	{
		if ((*pattern && !matches(pattern, get_username(Dummy)) &&
				  (get_userlevel(get_username(Dummy), channel) <= get_int_varc(Channel, SETMAL_VAR))) ||
			 (!*pattern && !is_user(get_username(Dummy), channel)))
					 {
			sendkick(channel, get_usernick(Dummy), buffer);
			count++;
		}
		Dummy=Dummy->next;
	}
	return count;
}

int channel_masskickbanold(channel, pattern, reason)
char *channel;
char *pattern;
char *reason;
{
	aChannel *Channel;
	aChanUser *Dummy;
	char buffer[MAXLEN];
	int count;

	if (!(Channel = find_channel(channel)))
		return 0;
	count = 0;
	sprintf(buffer, "(%s) \002%s\002",
		(pattern && *pattern) ? pattern : "non-users",
		(reason && *reason) ? reason : "idiot");
	Dummy = Channel->users;
	while (Dummy)
	{
		if ((*pattern && !matches(pattern, get_username(Dummy)) &&
				  (get_userlevel(get_username(Dummy), channel) <= get_int_varc(Channel, SETMAL_VAR))) ||
			 (!*pattern && !is_user(get_username(Dummy), channel)))
		{
			deop_ban(channel, get_usernick(Dummy), format_uh(get_username(Dummy), 1));
			sendkick(channel, get_usernick(Dummy), buffer);
			count++;
		}
		Dummy=Dummy->next;
	}
	return count;
}

int channel_massunbanfrom(char *channel, char *pattern, time_t seconds)
{
	aChannel *Channel;
	register aBan *Dummy;
	char unbanmode[5];
	char unbanstring[MAXLEN];
	int i = 0, count;

	if ((Channel = find_channel(channel)) == NULL)
		return 0;
	count = 0;
	Dummy = Channel->banned;
	while(Dummy)
	{
		strcpy(unbanmode, "-");
		strcpy(unbanstring, "");
		i=0;
		while(Dummy && (i< get_int_varc(Channel, SETBANMODES_VAR)))
		{
		 if (!matches(pattern, Dummy->bannedby))
		 {
			  if (!seconds || ((getthetime()-Dummy->time) > seconds))
			  {
				 strcat(unbanmode,"b");
				 strcat(unbanstring, " ");
				 strcat(unbanstring, Dummy->banstring);
				 i++;
			  }
			}
			Dummy=Dummy->next;
		}
		sendmode(channel, "%s %s", unbanmode, unbanstring);
	}
	return i;
}

int channel_massunban(char *channel, char *pattern, time_t seconds)
{
	aChannel *Channel;
	register aBan *Dummy;
	char unbanmode[5];
	char unbanstring[MAXLEN];
	int i, count;

	if ((Channel = find_channel(channel)) == NULL)
		return 0;
	count = 0;
	Dummy = Channel->banned;
	while(Dummy)
	{
		strcpy(unbanmode, "-");
		strcpy(unbanstring, "");
		i=0;
		while(Dummy && (i < get_int_varc(Channel, SETBANMODES_VAR)))
		{
			if (!matches(pattern, Dummy->banstring) ||
				!matches(Dummy->banstring, pattern))
			{
				if (!seconds || ((getthetime()-Dummy->time) > seconds))
				{
					strcat(unbanmode,"b");
					strcat(unbanstring, " ");
					strcat(unbanstring, Dummy->banstring);
					i++;
													 count++;
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
	aChannel *Channel;
	register aBan *Dummy;
	char unbanmode[5];
	char unbanstring[MAXLEN];
	int i, count;

	if ((Channel = find_channel(channel)) == NULL)
		return 0;
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
				strcat(unbanmode, "b");
				strcat(unbanstring, " ");       
				strcat(unbanstring, Dummy->banstring);          
				i++;
									count++;
			}        
			Dummy=Dummy->next;        
		}
		sendmode(channel, "%s %s", unbanmode, unbanstring);      
	}
	return count;
}

