/*
 * channel.c - a better implementation to handle channels on IRC
 */

#include <stddef.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdarg.h>

#include "channel.h"
#include "chanuser.h"
#include "config.h"
#include "fnmatch.h"
#include "function.h"
#include "userlist.h"
#include "dweeb.h"
#include "debug.h"

extern  botinfo *current;

static  char channel_buf[MAXLEN];

void show_clonebots(from, channel)
char *from;
char *channel;
{
	CHAN_list *Channel;
	TS *Bleah;

	if (!(Channel = search_chan(channel)))
		return;
	for (Bleah = *(Channel->HostList); Bleah;Bleah = Bleah->next)
		send_to_user(from, "%s %s %i", Bleah->name, time2str(Bleah->time), Bleah->num);
}

void check_clonebots(from, channel)
char *from;
char *channel;
{
	CHAN_list *Channel;
	time_t ct;
	TS *Bleah;
	char thehost[100];
	char *temp;

	ct = getthetime();

	if (!(Channel = search_chan(channel)))
		return;
	delete_timelist(Channel->HostList, ct-120); /* delete ones over 2m old */
	if (!(temp = gethost(from)))
		return;
	strcpy(thehost, temp);
	if (!(Bleah = is_in_timelist(Channel->HostList, thehost)))
	{
	  add_to_timelist(Channel->HostList, thehost);
	  return;
	}
	if ((ct - Bleah->time) > 10)
	{
		Bleah->time = ct;
		Bleah->num;
	}
	if (Bleah->num > 5)
	{
		channel_massunban(channel, thehost, 0);
		sendmode(channel, "+b %s", thehost);
		if (i_am_op(channel))
		  channel_masskick(channel, thehost, NULL);
	}
}

void show_idletimes(from, channel, seconds)
char *from;
char *channel;
int seconds;
{
  CHAN_list *Channel;
  USER_list *User;
  time_t ct;

  if (!(Channel = search_chan(channel)))
	 return;
  
  send_to_user(from, "\002Users on %s that are idle more than %i second%s:\002",
			 channel, seconds, EXTRA_CHAR(seconds));
  ct = getthetime();
  for( User = Channel->users; User; User = User->next )
	 if ((ct-User->idletime) > seconds)
	send_to_user(from, "%s: %s", idle2strsmall(ct-User->idletime), get_username(User));
  send_to_user(from, "\002--- end of list ---\002");
}

void check_limit(channel)
char *channel;
{
  CHAN_list *Channel;
  int num;

  if ((Channel = search_chan(channel)) == NULL)
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
  CHAN_list *Channel;
  USER_list *User;

  for (Channel = current->Channel_list; Channel; Channel = Channel->next)
    if (User = search_user(&(Channel->users), who))
      return TRUE;
  return FALSE;
}
	        
void op_all_chan(who)
char *who;   /* should be a nickname */
{
  CHAN_list *Channel;
  USER_list *User;
  char	    buffer[255];

  for (Channel = current->Channel_list; Channel; Channel = Channel->next)
     if (User = search_user(&(Channel->users), who))
       if (i_am_op(Channel->name))
	 if (!(get_usermode(User) & MODE_CHANOP))
           if (get_userlevel(get_username(User), Channel->name))
             if (get_userlevel(get_username(User), Channel->name) >= current->restrict)
                giveop(Channel->name, who);
}
    
void check_bans(void)
{
	CHAN_list	*Channel;
	USER_list	*User;

	for (Channel = current->Channel_list; Channel; Channel = Channel->next)
		for (User = Channel->users; User; User = User->next)
			if (is_banned(get_username(User), Channel->name) &&
				(get_userlevel(get_username(User), Channel->name) < 50))
		sendkick(Channel->name, User->nick, "\002Banned\002");
}

void check_shit(void)
{
  CHAN_list *Channel;
  USER_list *User;
  time_t ct;

  ct = getthetime();
  for (Channel = current->Channel_list; Channel; Channel = Channel->next)
	 for( User = Channel->users; User; User = User->next )
		if (is_shitted(get_username(User), Channel->name))
	shit_action(get_username(User), Channel->name);
}

void check_idle(void)
{
  CHAN_list *Channel;
  USER_list *User;
  time_t ct;

  ct = getthetime();
  for (Channel = current->Channel_list; Channel; Channel = Channel->next)
	 for( User = Channel->users; User; User = User->next )
		if ((ct - User->idletime) >= current->idlelevel)
	{
	  if (!is_user(get_username(User), Channel->name) &&
			Channel->iktoggle &&
			!(get_usermode(User) & MODE_CHANOP))
		 sendkick(Channel->name, User->nick, "\002Idle for %s\002",
			  idle2str(getthetime()-User->idletime));
	  User->idletime = getthetime();
	}
}

void update_idletime(from, channel)
char *from;
char *channel;
{
  CHAN_list *Channel;
  USER_list *User;
  if (!(Channel = search_chan(channel)))
	 return;
  if (!(User = search_user(&(Channel->users), getnick(from))))
	 return;
  User->idletime = getthetime();
}

int check_beepkick(from, channel, num)
char *from;
char *channel;
int num;
{
	CHAN_list *Channel;
	USER_list *User;

	if (!(Channel = search_chan(channel)))
		return FALSE;
	if (!(User = search_user(&(Channel->users), getnick(from))))
		return;
	if ((getthetime() - User->beeptime) > 10)
	{
		User->beeptime = getthetime();
		User->beepnum = 0;
	}
	if (!is_user(get_username(User), Channel->name) &&
			!(get_usermode(User) & MODE_CHANOP))
		User->beepnum += num;
	if ((User->beepnum >= current->beepkicklevel) && (Channel->bktoggle))
		return TRUE;
	return FALSE;
}

int check_capskick(from, channel, num)
char *from;
char *channel;
int num;
{
  CHAN_list *Channel;
  USER_list *User;

  if (!(Channel = search_chan(channel)))
	 return FALSE;
  if (!(User = search_user(&(Channel->users), getnick(from))))
	 return;
  if ((getthetime() - User->capstime) > 10)
	 {
		User->capstime = getthetime();
		User->capsnum = 0;
	 }
	if (!is_user(get_username(User), Channel->name) &&
			!(get_usermode(User) & MODE_CHANOP))
		User->capsnum += num;
  if ((User->capsnum >= current->capskicklevel) && (Channel->cktoggle))
	 return TRUE;
  return FALSE;
}

int check_massdeop(from, channel)
char *from;
char *channel;
{
  CHAN_list *Channel;
  USER_list *User;
  
  if (!(Channel = search_chan(channel)))
	 return FALSE;
  if (!(User = search_user(&(Channel->users), getnick(from))))
	 return;
  if ( (getthetime() - User->deoptime) > 10)
	 {
		User->deoptime = getthetime();
		User->deopnum = 0;
	 }
  
  User->deopnum++;
  if (User->deopnum >= current->massdeoplevel)
	 return TRUE;
  return FALSE;
}

int check_massban(from, channel)
char *from;
char *channel;
{
  CHAN_list *Channel;
  USER_list *User;
  
  if (!(Channel = search_chan(channel)))
	 return FALSE;
  if (!(User = search_user(&(Channel->users), getnick(from))))
	 return;
  if ( (getthetime() - User->bantime) > 10)
	 {
		User->bantime = getthetime();
		User->bannum = 0;
	 }
  User->bannum++;
  if (User->bannum >= current->massbanlevel)
	 return TRUE;
  return FALSE;
}

int check_masskick(from, channel)
char *from;
char *channel;
{
  CHAN_list *Channel;
  USER_list *User;
  
  if (!(Channel = search_chan(channel)))
	 return FALSE;
  if (!(User = search_user(&(Channel->users), getnick(from))))
	 return;
  if ( (getthetime() - User->kicktime) > 10)
	 {
		User->kicktime = getthetime();
		User->kicknum = 0;
	 }
  User->kicknum++;
  if (User->kicknum >= current->masskicklevel)
	 return TRUE;
  return FALSE;
}

int check_nickflood(from)
char *from;
{
  CHAN_list *Channel;
  USER_list *User;
  int flag;
  flag=FALSE;
  for (Channel = current->Channel_list; Channel; Channel = Channel->next)
	 if (User = search_user(&(Channel->users), getnick(from)))
  {
	 if ( (getthetime() - User->nicktime) > 10)
		{
	User->nicktime = getthetime();
	User->nicknum = 0;
		}
	 User->nicknum++;
	 if (User->nicknum >= current->massnicklevel)
		{
	char *temp;
	flag = TRUE;
	temp = format_uh(from, 1);
	if (!is_user(from, Channel->name) &&
		 !(get_usermode(User) & MODE_CHANOP))
	  {
		 add_to_shitlist(current->ShitList, temp, 2, Channel->name, "Auto-Shit",
				 "Quit the nick floods you lamer", getthetime());

	  }
	if (i_am_op(Channel->name))
	  {
	deop_ban(Channel->name, getnick(from), temp);
	sendkick(Channel->name, getnick(from), "\002%s\002", "Get the hell out nick-flooding lamer!!!");
         }
      }
  }
  return flag;
}

int check_pubflooding(from, channel)
char *from;
char *channel;
{
  CHAN_list *Channel;
  USER_list *User;
  
  if (!(Channel = search_chan(channel)))
	 return FALSE;
  if (!(User = search_user(&(Channel->users), getnick(from))))
	 return;

  if ( (getthetime() - User->floodtime) > 5)
	 {
		User->floodtime = getthetime();
		User->floodnum = 0;
	 }
  User->floodnum++;
  if (get_usermode(User) & MODE_CHANOP)
	 return FALSE;
 
  if (User->floodnum >= current->floodlevel)
	 return current->floodprotlevel;
  return FALSE;
}

int tot_num_on_channel(channel)
char *channel;
{
  CHAN_list *Channel;
  USER_list *User;
  int num = 0;
  if ((Channel = search_chan(channel)) == NULL)
	 return 0;
  for (User=Channel->users;User;User=User->next)
	 num++;
  return num;
}

CHAN_list *search_chan(name)
char *name;
{
	CHAN_list       *Channel;

	for (Channel = current->Channel_list; Channel; Channel = Channel->next)
		if (!my_stricmp(name,Channel->name))
			return(Channel);
	return(NULL);
}

void add_channel(channel)
CHAN_list *channel;
{
	channel->next = current->Channel_list;
	current->Channel_list = channel;
}

int delete_channel(channel)
CHAN_list *channel;
{
	CHAN_list **old;

	for (old = &(current->Channel_list); *old; old = &(**old).next)
		if (*old == channel)
		{
			*old = channel->next;
			clear_all_users(&(channel->users));
			clear_all_bans(&(channel->banned));
			remove_all_from_list(channel->SpyList);
			remove_all_from_timelist(channel->HostList);
			free(channel->name);
			free(channel->key);
			free(channel->topic);
			free(channel->mod);
			free(channel);
			return TRUE;
		}
	return FALSE;
}

void delete_all_channels(void)
{
	CHAN_list *channel;

	for(; channel = current->Channel_list;)  /* Why not while ? -- CS */
		delete_channel(channel);
}

void copy_channellist(c_list)
CHAN_list *c_list;
{
	CHAN_list *tmp;

	for (tmp = c_list; tmp; tmp = tmp->next)
	{
		join_channel(tmp->name, tmp->key, FALSE);
		set_enfmodes(tmp->name, tmp->mod);
		set_enftopic(tmp->name, tmp->topic);
	}

}

int join_channel(name, key, dojoin)
char *name;
char *key;
int dojoin;
/*
 * Tries to join channel "name" and adds it to the channellist.
 * If the join is not succesfull, it'll be noticed by parseline
 * and active can be marked FALSE
 *
 * It could very well be that the userlist is not empty (after a kick,
 * kill). So clean it up first. Same for modes.
 */

{
	CHAN_list       *Channel;

	if(!ischannel(name))
		return(FALSE);

	if( (Channel = search_chan( name )) == NULL )
	{
/*
 * A completely new channel 
 */
		if( (Channel = (CHAN_list*)malloc( sizeof( *Channel ) ) ) 
			  == NULL )
			return(FALSE);
		mstrcpy(&Channel->name, name);
		Channel->users = NULL;
		Channel->banned = NULL;
		Channel->mode = 0;
		Channel->sdtoggle = 1;
		Channel->iktoggle = 0;
		Channel->sotoggle = 0;
		Channel->pubtoggle = 1;
		Channel->enfmtoggle = 0;
		Channel->toptoggle = 0;
		Channel->prottoggle = 1;
		Channel->aoptoggle = 1;
		Channel->shittoggle = 1;
		Channel->masstoggle = 1;
		Channel->bktoggle = 0;
		Channel->cktoggle = 0;
		mstrcpy(&Channel->key, key);
		Channel->topic = NULL;
		Channel->mod = NULL;
		Channel->kickedby = NULL;
		Channel->userlimit = 0;
		Channel->SpyList = init_list();
		Channel->HostList = init_timelist();
		add_channel(Channel);
		if (dojoin)
		{
		  sendjoin(name, key);
		  Channel->active = FALSE;/* TRUE; test -- assume it went ok */
		}
		else
			Channel->active = FALSE;
	}
	else    /* rejoin */
	{
		clear_all_users(&(Channel->users));
		clear_all_bans(&(Channel->banned));
		Channel->mode = 0;
		sendjoin(name, Channel->key);
		Channel->active = FALSE; /* TRUE; test -- assume it went ok */ 
	}       
/*
 * here we should send something like sendwho channel );
 * Yup..., also send something to get the channelmodes
 * -no, do this when we joined the channel
 */
	current->Current_chan = Channel;
	return( TRUE );
}

void remove_channel(name)
char *name;
/*
 removes channel from memory
*/
{
  CHAN_list *Dummy;
  
  if (Dummy=search_chan(name))
	 {
		if (Dummy==current->Current_chan)
	current->Current_chan = current->Channel_list;
		delete_channel(Dummy);
	 }
}

int leave_channel(name)
char *name;
/*
 * removes channel "name" from the list and actually leaves
 * the channel ---- no just sends the part
 */

{
	CHAN_list *Dummy;

	if((Dummy = search_chan(name)) != NULL)
	{
		sendpart(name);

		if (Dummy == current->Current_chan)
			current->Current_chan = current->Channel_list;
		delete_channel(Dummy);
		/*   Channel was found and we left it
		 * (or at least  removed it from the list)
		 */
		return(TRUE);           
	}
	/*
	 * There was obviously no such channel in the list
	 */
	return(FALSE);
}

int reverse_topic(from, channel, topic)
char *from;
char *channel;
char *topic;
{
	CHAN_list *Chan;

	if (!(Chan = search_chan(channel)))
        	return 0;
	if (!get_toptoggle(channel))
        {
		if (Chan->topic)
			free(Chan->topic);
		mstrcpy(&Chan->topic, topic);
		return 0;
        }
	if (get_userlevel(from, channel) >= ASSTLEVEL)
	{
               	if (Chan->topic)
			free(Chan->topic);
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

int reverse_mode(from, channel, mode, sign)
char *from;
char *channel;
char mode;
char sign;
{
	char *ptr;

	if (!get_enfmtoggle(channel))
		return FALSE;
	if (!(ptr = get_enfmodes(channel)))
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
/*          send_to_user(from, "\002Now enforcing %s on %s\002",
			 buffer, channel); */
		 set_enfmodes(channel, buffer);
		 return FALSE;
	  }
	return TRUE;
}

char *get_enfmodes(name)
char *name;
{
	CHAN_list *Dummy;

	if (Dummy = search_chan(name))
		return Dummy->mod;
	return NULL;
}

void cycle_channel(channel)
char *channel;
{
  sendpart(channel);
  join_channel(channel, "", TRUE);
}

int mychannel(name)
char *name;
{
	return (search_chan(name) != NULL);
}

int add_spy(name, nuh)
char *name;
char *nuh;
{
	CHAN_list *Channel;
	if ((Channel = search_chan(name)))
		return add_to_list(Channel->SpyList, nuh);
	return FALSE;
}

int del_spy(name, nuh)
char *name;
char *nuh;
{
	CHAN_list *Channel;
	if ((Channel = search_chan(name)))
		return remove_from_list(Channel->SpyList, nuh);
	return FALSE;
}

void show_spy_list(from, name)
char *from;
char *name;
{
	LS *temp;

	CHAN_list *Channel;
	if ((Channel = search_chan(name)))
	  for (temp=*Channel->SpyList;temp;temp=temp->next)
		 send_to_user(from, temp->name);
}

void send_spymsg(char *format, ...)
{
  char buf[HUGE];
  va_list msg;
  LS *temp;
  
  va_start(msg, format);
  vsprintf(buf, format, msg);
  va_end(msg);
  
  for(temp=*current->SpyMsgList;temp;temp=temp->next)
	 send_to_user(temp->name, "%s", buf);
}

void send_common_spy(char *from, char *format, ...)
{
  char buf[HUGE];
  va_list msg;
  CHAN_list *Channel;
  char *thenick;

  va_start(msg, format);
  vsprintf(buf, format, msg);
  va_end(msg);

  thenick = getnick(from);
  for (Channel = current->Channel_list; Channel; Channel = Channel->next)
    if (search_user(&(Channel->users), thenick))
      send_spy(Channel->name, "%s", buf);
}
   

void send_spy(char *channel, char *format, ...)
{
	char buf[HUGE];
	va_list msg;
	LS *temp;
	CHAN_list *Channel;

	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);

	if ((Channel = search_chan(channel)))
	  for (temp=*Channel->SpyList;temp;temp=temp->next)
		 if (ischannel(temp->name))
			sendprivmsg(temp->name, "%s", buf);
		 else
			send_to_user(temp->name, "%s", buf);
}

int is_spy(chan, nuh)
char *chan;
char *nuh;
{
	CHAN_list *Channel;
	if ((Channel = search_chan(chan)))
		return (is_in_list(Channel->SpyList, nuh) != NULL);
	return FALSE;
}

int get_toptoggle(name)
char *name;
{
  CHAN_list *Dummy;

  if (Dummy = search_chan(name))
	 return Dummy->toptoggle;
  return 0;
}

int get_prottoggle(name)
char *name;
{
  CHAN_list *Dummy;

  if (Dummy = search_chan(name))
	 return Dummy->prottoggle;
  return 0;
}

int get_masstoggle(name)
char *name;
{
  CHAN_list *Dummy;

  if (Dummy = search_chan(name))
	 return Dummy->masstoggle;
  return 0;
}

int get_enfmtoggle(name)
char *name;
{
  CHAN_list *Dummy;

  if (Dummy = search_chan(name))
	 return Dummy->enfmtoggle;
  return 0;
}

int get_aoptoggle(name)
char *name;
{
  CHAN_list *Dummy;

  if (Dummy = search_chan(name))
	 return Dummy->aoptoggle;
  return 0;
}

int get_iktoggle(name)
char *name;
{
  CHAN_list *Dummy;
  
  if (Dummy = search_chan(name))
	 return Dummy->iktoggle;
  return 0;
}

int get_sotoggle(name)
	  char *name;
{
  CHAN_list *Dummy;

  if (Dummy = search_chan(name))
	 return Dummy->sotoggle;
  return 0;
}

int get_shittoggle(name)
char *name;
{
	CHAN_list *Dummy;

	if (Dummy = search_chan(name))
		return Dummy->shittoggle;
	return 0;
}

int get_sdtoggle(name)
char *name;
{
	CHAN_list *Dummy;

	if (Dummy = search_chan(name))
		return Dummy->sdtoggle;
	return 0;
}

int get_pubtoggle(name)
char *name;
{
  CHAN_list *Dummy;

  if (Dummy = search_chan(name))
	 return Dummy->pubtoggle;
  return 0;
}

int get_limit(name)
char *name;
{
  CHAN_list *Dummy;
  
  if (Dummy = search_chan(name))
	 return Dummy->userlimit;
  return 0;
}

int set_limit(name, num)
char *name;
int num;
{
  CHAN_list *Dummy;
  
  if (Dummy = search_chan(name))
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
	CHAN_list *Dummy;

	if (Dummy = search_chan(name))
	{
	  if (Dummy->kickedby)
		 free(Dummy->kickedby);
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
	CHAN_list *Dummy;

	if (Dummy = search_chan(name))
		return Dummy->kickedby;
	return NULL;
}

int set_enfmodes(name, modes)
char *name;
char *modes;
{
	CHAN_list *Dummy;

	if (Dummy = search_chan(name))
	{
	  if (Dummy->mod)
		 free(Dummy->mod);
	  mstrcpy(&Dummy->mod, modes);
	  return TRUE;
	}
	return FALSE;
}

char *get_enftopic(name)
char *name;
{
	CHAN_list *Dummy;

	if (Dummy = search_chan(name))
		return Dummy->topic;
	return NULL;
}

int set_enftopic(name, topic)
char *name;
char *topic;
{
	CHAN_list *Dummy;

	if (Dummy = search_chan(name))
	{
	  if (Dummy->topic)
		 free(Dummy->topic);
	  mstrcpy(&Dummy->topic, topic);
	  return TRUE;
	}
	return FALSE;
}

int mark_success(name)
char *name;
{
	CHAN_list       *Dummy;

	if (Dummy = search_chan(name))
	{
		Dummy->active = TRUE;
		return(TRUE);
	}
	return(FALSE);
}

int mark_failed(name)
char *name;
{
	CHAN_list       *Dummy;

	if( (Dummy = search_chan( name ) ) != NULL )
	{
		Dummy->active = FALSE;
		return(TRUE);
	}
	return(FALSE);
}

void show_channellist(user)
char *user;
{
	CHAN_list       *Channel;
	unsigned int    mode;
	char            modestr[MAXLEN];

	if( current->Channel_list == NULL )
	{
		send_to_user( user, "\002%s\002", "I'm not active on any channels" );
		return;
	}

	send_to_user( user, "\002Active on: %s\002", current->Channel_list->name );
	for( Channel = current->Channel_list; Channel; Channel = Channel->next )
	{
		mode = Channel->mode;
		strcpy( modestr, "" );
		if( mode&MODE_PRIVATE ) strcat( modestr, "p" );
		if( mode&MODE_SECRET ) strcat( modestr, "s" );
		if( mode&MODE_MODERATED ) strcat( modestr, "m" );
		if( mode&MODE_TOPICLIMIT ) strcat( modestr, "t" );
		if( mode&MODE_INVITEONLY ) strcat( modestr, "i" );
		if( mode&MODE_NOPRIVMSGS ) strcat( modestr, "n" );
		if( mode&MODE_KEY ) 
		{
			strcat( modestr, "k " );
			strcat( modestr, Channel->key?Channel->key:"???" );
			strcat( modestr, " " );
		}
		if( mode&MODE_LIMIT )
		{
			strcat( modestr, "l " );
			strcat( modestr, Channel->limit?Channel->limit:"???" );
		}
		send_to_user( user, "        -  %s, mode: +%s", 
		Channel->name, modestr );
	}
	return;
} 

void reset_channels(Mode)
int Mode;
{
	CHAN_list       *Channel;

	for(Channel = current->Channel_list; Channel; Channel = Channel->next)
		if((Mode == HARDRESET) || !(Channel->active))
			join_channel(Channel->name, Channel->key, TRUE);
}

char *currentchannel(void)
/*
 * returns name of current channel, i.e. the last joined
 */
{
  if (current->Current_chan)
    {
      if (!current->Current_chan->name)
	{
	  debug(NOTICE, "BIG FUCKING ERROR: Current_chan->name = NULL");
	  strcpy(channel_buf, "#0");
	}
      else
	strcpy(channel_buf, current->Current_chan->name);
    }
  else
    strcpy(channel_buf, "#0");
  return channel_buf;
}

/*
 * Ok, that was the basic channel stuff... Now some functions to
 * add users to a channel, remove them and one to change a nick on ALL
 * channels.
 */

int add_user_to_channel(channel, nick, user, host)
char *channel;
char *nick;
char *user;
char *host;
{
	CHAN_list       *Dummy;

	if (!(Dummy = search_chan(channel)))
		return(FALSE);
	add_user( &(Dummy->users), nick, user, host );
	return(TRUE);
}

int remove_user_from_channel(channel, nick)
char *channel;
char *nick;
{
	CHAN_list *Dummy;

	if (!(Dummy = search_chan(channel)))
		return(FALSE);
	delete_user( &(Dummy->users), nick );
	return(TRUE);
}

void change_nick(oldnick, newnick)
char *oldnick;
char *newnick;
{
	CHAN_list *Channel;

	for (Channel = current->Channel_list; Channel; Channel = Channel->next)
		change_user_nick(&(Channel->users), oldnick, newnick);
}

void remove_user(nick)
char *nick;
/*
 * Remove a user from all channels (signoff)
 */
{
	CHAN_list       *Channel;

	for (Channel = current->Channel_list; Channel; Channel = Channel->next)
	  {
		delete_user(&(Channel->users), nick);
		check_limit(Channel->name);
	  }
}

void ban_all_channels(banstring)
char *banstring;
{
  CHAN_list *Channel;
  for (Channel = current->Channel_list; Channel; Channel = Channel->next)
	 if (i_am_op(Channel->name))
		sendmode("%s +b %s", Channel->name, banstring);
}

void kick_all_channels(nick, reason)
char *nick;
char *reason;
{
  CHAN_list *Channel;
  for (Channel = current->Channel_list; Channel; Channel = Channel->next)
	 if (i_am_op(Channel->name))
		sendkick(Channel->name, nick, "%s", reason);
}

void kickban_all_channels(nick, banstring, reason)
char *nick;
char *banstring;
char *reason;
{
 CHAN_list *Channel;
 for (Channel = current->Channel_list; Channel; Channel = Channel->next)
	if (i_am_op(Channel->name))
	  {
		 sendmode("%s +b %s", Channel->name, banstring);
		 sendkick(Channel->name, nick, "%s", reason);
	  }
}

void chan_stats(from, channel)
char *from;
char *channel;
{
  CHAN_list       *Dummy;
  USER_list       *Blah;
  int ops, tot;

  ops = 0;
  tot = 0;

  if (!(Dummy = search_chan(channel)))
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
		Blah = get_nextuser( Blah );
	 }
  send_to_user(from, "\002Out of %i people on %s, %i are ops\002",
			 tot, channel, ops);
}


int show_users_on_channel(from, channel)
char *from;
char *channel;
{
	CHAN_list       *Dummy;
	USER_list       *Blah;
	char            modestr[MAXLEN];

	if (!(Dummy = search_chan(channel)))
	  {
		 send_to_user(from, "\002I'm not on %s\002", channel);
		 return(FALSE);
	  }
	
	Blah = Dummy->users;
	send_to_user(from, "\002Users on %s:\002", channel);
	while (Blah)
	{
		strcpy(modestr, "");
		if (get_usermode(Blah)&MODE_CHANOP) strcat(modestr, "+o");
		if (get_usermode(Blah)&MODE_VOICE)  strcat(modestr, "+v");
		send_to_user( from, "%3du %ds, mode: %-4s -> %s", 
				  get_userlevel(get_username(Blah), channel),
				  get_shitlevel(get_username(Blah), channel),
				  modestr,
				  get_username(Blah));
		Blah = get_nextuser( Blah );
	}
	return(TRUE);
}

/*
 * now some functions to keep track of modes
 */

void add_channelmode(from, channel, mode, params, time)
char *from;
char *channel;
unsigned int mode;
char *params;
time_t time;
{
	CHAN_list *Channel;

	if (!(Channel = search_chan(channel)))
		return;

	switch(mode)
	{
		case MODE_CHANOP:
		case MODE_VOICE:
			add_mode(&(Channel->users), mode, params);
			break;
		case MODE_KEY:
			Channel->mode |= MODE_KEY;
			free(Channel->key);
			mstrcpy(&Channel->key, params);
			break;
		case MODE_BAN:
			add_ban(&(Channel->banned), from, params, time);
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
	CHAN_list       *Channel;

	if (!(Channel = search_chan(channel)))
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
	CHAN_list *Channel;
	
	if (!(Channel = search_chan(channel)))
		return;

	add_mode( &(Channel->users), mode, user );
}
	
int open_channel(channel)
char *channel;
{
	CHAN_list       *Channel;

	if (!(Channel = search_chan(channel)))
		return FALSE;

	if ((Channel->mode)&MODE_KEY)
		sendmode(channel, "-ipslk %s", Channel->key);
	else
		sendmode(channel, "-ipsl");
	return TRUE;
}

int is_opped(nick, channel)
char *nick;
char *channel;
{
	return (usermode(channel, nick) & MODE_CHANOP);
}


/*
 * Some misc. function which deal with channels and users
 */

int invite_to_channel(user, channel)
char *user;
char *channel;
{
	if (search_chan(channel))
	{
		send_to_server("INVITE %s %s", user, channel);
		return(TRUE);
	}
	return(FALSE);
}

char *username(nick)
char *nick;
/*
 * Searches all lists for nick and if it finds it, returns 
 * nick!user@host
 */

{
	CHAN_list *Channel;
	USER_list *User;

	for (Channel = current->Channel_list; Channel; Channel = Channel->next)
		if (User = search_user(&(Channel->users), nick))
			return(get_username(User));
	return(NULL);
}

unsigned int usermode(channel, nick)
char *channel;
char *nick;
/*
 * returns the usermode of nick on channel
 */
{
	CHAN_list *Channel;
	USER_list *Blah;

	if (!(Channel = search_chan(channel)))
		return (0);      /* or -1? */

	Blah=Channel->users;
	while (Blah)
	{
		if (!my_stricmp(nick, get_usernick(Blah)))
			return(get_usermode(Blah));
		Blah=get_nextuser(Blah);
	}
	return(0);
}
	
/*
 * Here are the functions to bookkeep the banlist
 */

int is_banned(who, channel)
char *who;
char *channel;
{
  CHAN_list *Channel;
  BAN_list *Banned;


  if (!(Channel = search_chan(channel)))
                return FALSE;
  for (Banned = Channel->banned; Banned; Banned = Banned->next)
	if (!matches(Banned->banstring, who))
		return TRUE;
  return FALSE;
}

BAN_list *search_ban(BAN_list **b_list, char *banstring)
{
	BAN_list *Banned;

	for (Banned = *b_list; Banned; Banned = Banned->next)
		if (!my_stricmp(Banned->banstring, banstring))
			return(Banned);         
	return(NULL);
}

void show_banlist(char *from, char *channel)
{
  CHAN_list *Channel;
  BAN_list *Banned;


  if (!(Channel = search_chan(channel)))
	 {
		send_to_user(from, "\002I'm not on %s\002", channel);
		return;
	 }
  for (Banned = Channel->banned; Banned; Banned = Banned->next)
	 {
		send_to_user(from, "\002%s\002", Banned->banstring);
		send_to_user(from, "\002   banned by %s at %s\002", Banned->bannedby,
			time2str(Banned->time));
	 }
}

void add_ban(BAN_list **b_list, char *from, char *banstring, time_t time)
{
	BAN_list        *New_ban;

	if ((New_ban = (BAN_list*)malloc(sizeof(*New_ban))) == NULL)
		return;
	if (search_ban(b_list, banstring) != NULL)
		return;
	mstrcpy(&New_ban->banstring, banstring );
	if (from)
	  mstrcpy(&New_ban->bannedby, from);
	else
	  mstrcpy(&New_ban->bannedby, "<UNKNOWN>");
	New_ban->time = time;
	New_ban->next = *b_list;
	*b_list = New_ban;
}

int delete_ban(BAN_list **b_list, char *banstring)
{
	BAN_list        **old;
	BAN_list        *Dummy;

	if((Dummy = search_ban(b_list, banstring)) == NULL)
		return(FALSE);

	for(old = b_list; *old; old = &(*old)->next)
		if(*old == Dummy)
		{
			*old = Dummy->next;
			free(Dummy->banstring);
			free(Dummy->bannedby);
			free(Dummy);
			return(TRUE);
		}
	return(FALSE);
}

void clear_all_bans(BAN_list **b_list)
/*
 * Removes all entries in the list
 */
{
	BAN_list        **old;
	BAN_list        *del;

	for(old = b_list; *old;)
	{
		del = *old;
		*old = (*old)->next;
		free(del->banstring);
		free(del->bannedby);
		free(del);
	}
}

/*
 * Here is some mass-stuff. Perhaps it doesn't belong here (though..
 * I think this is a quite good place...).
 */

int channel_massop(channel, pattern)
char *channel;
char *pattern;
{
	CHAN_list *Channel;
	USER_list *Dummy;
	char      opstring[MAXLEN];
	int       i, count;

	if (!(Channel = search_chan(channel)))
		return;
   	count = 0;
	Dummy = Channel->users;
	while (Dummy)
	{
		strcpy(opstring, "+oooo ");
		i=0;
		while (Dummy && (i < current->maxmodes))
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
			Dummy=get_nextuser(Dummy);
		}
		sendmode(channel, "%s", opstring);
	}
	return count;
}

int channel_massdeop(channel, pattern)
char *channel;
char *pattern;
{
	CHAN_list *Channel;
	USER_list *Dummy;
	char      deopstring[MAXLEN];
	int       i, count;

	if( (Channel = search_chan( channel )) == NULL )
		return 0;
	count = 0;
	Dummy = Channel->users;
	while(Dummy)
	{
		strcpy(deopstring, "-oooo ");
		i=0;
		while(Dummy && (i < current->maxmodes))
		{
			if ((get_usermode(Dummy) & MODE_CHANOP) &&
				 ((*pattern && !matches(pattern, get_username(Dummy)) &&
						(get_userlevel(get_username(Dummy), channel) <= current->malevel)) ||
				  (!*pattern && !is_user(get_username(Dummy), channel))))
			{
				strcat(deopstring, " ");
				strcat(deopstring, get_usernick(Dummy));
				i++;
                        	count++;
			}
			Dummy=get_nextuser(Dummy);
		}
		sendmode(channel, "%s", deopstring);
	}
	return count;
}

int  channel_masskick(channel, pattern, reason)
char *channel;
char *pattern;
char *reason;
{
	CHAN_list *Channel;
	USER_list *Dummy;
	char buffer[MAXLEN];
	int count;

	if( (Channel = search_chan( channel )) == NULL )
		return 0;
	count = 0;
	sprintf(buffer, "(%s) \002%s\002",
		(pattern && *pattern) ? pattern : "non-users",
		(reason && *reason) ? reason : "idiot");
	Dummy = Channel->users;
	while(Dummy)
	{
		if ((*pattern && !matches(pattern, get_username(Dummy)) &&
				  (get_userlevel(get_username(Dummy), channel) <= current->malevel)) ||
			 (!*pattern && !is_user(get_username(Dummy), channel)))
                {
			sendkick(channel, get_usernick(Dummy), buffer);
                	count++;
                }
		Dummy=get_nextuser(Dummy);
	}
	return count;
}

int channel_masskickban(channel, pattern, reason)
char *channel;
char *pattern;
char *reason;
{
	CHAN_list *Channel;
	USER_list *Dummy;
	char buffer[MAXLEN];
        int count;

	if (!(Channel = search_chan(channel)))
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
				  (get_userlevel(get_username(Dummy), channel) <= current->malevel)) ||
			 (!*pattern && !is_user(get_username(Dummy), channel)))
                {
			sendkick(channel, get_usernick(Dummy), buffer);
			count++;
		}
		Dummy=get_nextuser(Dummy);
	}
	return count;
}

int channel_masskickbanold(channel, pattern, reason)
char *channel;
char *pattern;
char *reason;
{
	CHAN_list *Channel;
	USER_list *Dummy;
	char buffer[MAXLEN];
	int count;

	if (!(Channel = search_chan(channel)))
		return 0;
	count = 0;
	sprintf(buffer, "(%s) \002%s\002",
		(pattern && *pattern) ? pattern : "non-users",
		(reason && *reason) ? reason : "idiot");
	Dummy = Channel->users;
	while (Dummy)
	{
		if ((*pattern && !matches(pattern, get_username(Dummy)) &&
				  (get_userlevel(get_username(Dummy), channel) <= current->malevel)) ||
			 (!*pattern && !is_user(get_username(Dummy), channel)))
		{
			deop_ban(channel, get_usernick(Dummy), format_uh(get_username(Dummy), 1));
			sendkick(channel, get_usernick(Dummy), buffer);
			count++;
		}
		Dummy=get_nextuser(Dummy);
	}
	return count;
}

int channel_massunbanfrom(char *channel, char *pattern, time_t seconds)
{
	CHAN_list       *Channel;
	BAN_list        *Dummy;
	char            unbanmode[5];
	char            unbanstring[MAXLEN];
	int             i, count;

	if( (Channel = search_chan( channel )) == NULL )
		return 0;
   	count = 0;
	Dummy = Channel->banned;
	while(Dummy)
	{
		strcpy(unbanmode, "-");
		strcpy(unbanstring, "");
		i=0;
		while(Dummy && (i<3))
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
}

int channel_massunban(char *channel, char *pattern, time_t seconds)
{
	CHAN_list       *Channel;
	BAN_list        *Dummy;
	char            unbanmode[5];
	char            unbanstring[MAXLEN];
	int             i, count;

	if( (Channel = search_chan( channel )) == NULL )
		return 0;
	count = 0;
	Dummy = Channel->banned;
	while(Dummy)
	{
		strcpy(unbanmode, "-");
		strcpy(unbanstring, "");
		i=0;
		while(Dummy && (i<3))
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
	CHAN_list       *Channel;
	BAN_list        *Dummy;
	char            unbanmode[5];
	char            unbanstring[MAXLEN];
	int             i, count;

	if ((Channel = search_chan( channel )) == NULL)
		return 0;
	count = 0;
	Dummy = Channel->banned;
	while(Dummy)
	{
		strcpy(unbanmode, "-");
		strcpy(unbanstring, "");
		i=0;
		while(Dummy && (i<3))
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


