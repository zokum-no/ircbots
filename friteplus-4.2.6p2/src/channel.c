/*
 * channel.c - a better implementation to handle channels on IRC
 * (c) 1995 OffSpring (cracker@cati.CSUFresno.EDU)
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

#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <strings.h>
#include <string.h>

#include "channel.h"
#include "chanuser.h"
#include "config.h"
#include "autoconf.h"
#include "fnmatch.h"
#include "misc.h"
#include "send.h"
#include "userlist.h"
#include "frite_bot.h"
#include "war.h"

extern	botinfo	*currentbot;
extern	int	userlevel(char *from);
extern	int	protlevel(char *from);
extern	int	shitlevel(char *from);

static	char	channel_buf[MAXLEN];

CHAN_list	*search_chan( char *name )
{
	CHAN_list	*Cool;

	for( Cool = currentbot->Channel_list; Cool; Cool = Cool->next )
		if( STRCASEEQUAL( name, Cool->name ) )
			return(Cool);
	return(NULL);
}
int mychannel( char *name)
{ 
        return ( search_chan( name ) != NULL);
}

void	add_channel( CHAN_list *Channel )
/*
 * adds Channel to the list... 
 */

{
	Channel->next = currentbot->Channel_list;
	currentbot->Channel_list = Channel;
}

int	delete_channel( CHAN_list *Channel )
/*
 * removes channel from list 
 */

{
	CHAN_list	**old;

	for( old = &(currentbot->Channel_list); *old; old = &(**old).next )
		if( *old == Channel )
		{
			*old = Channel->next;
			clear_all_users(&(Channel->users));
			free(Channel->name);
			free(Channel->key);
			free(Channel->topic);
			free(Channel->mod);
			free(Channel->spylist);
			free(Channel);
			return(TRUE);
		}
	return( FALSE );
}

void	delete_all_channels()
{
	CHAN_list	*Channel;

	for(;(Channel = currentbot->Channel_list);)
		delete_channel(Channel);
}

void	copy_channellist(CHAN_list *c_list)
{
	CHAN_list *tmp;

	for(tmp = c_list; tmp; tmp = tmp->next)
		join_channel(tmp->name, tmp->topic, tmp->mod, FALSE);
}

int	join_channel(char *name, char *topic, char *mode, int dojoin)
/*
 * Tries to join channel "name" and adds it to the channellist.
 * If the join is not succesfull, it'll be noticed by parseline
 * and active can be marked FALSE
 *
 * It could very well be that the userlist is not empty (after a kick,
 * kill). So clean it up first. Same for modes.
 */

{
	CHAN_list	*Channel;

	if(!ischannel(name))
		return(FALSE);

	if( (Channel = search_chan( name )) == NULL )
	{
/*
 * A completely new channel 
 */
		if( (Channel = (CHAN_list*)malloc( sizeof( *Channel ) ) ) == NULL )
			return(FALSE);
		mstrcpy(&Channel->name, name);
		Channel->users = NULL;
		Channel->banned = NULL;
		Channel->mode = 0;
		mstrcpy(&Channel->key,"");
		mstrcpy(&Channel->topic, topic?topic:"");
		mstrcpy(&Channel->mod, mode?mode:"");
		Channel->log = 0;
		Channel->aop = 1;
		Channel->shit = 1;
		Channel->prot = 1;
		Channel->sd = 0;
		Channel->so = 0;
		Channel->mass = 0;
		Channel->ik = 0;
		Channel->bk = 0;
		Channel->ck = 0;
		Channel->sing = 1;
		Channel->bonk = 1;
		Channel->greet = 0;
		Channel->pub = 1;
		Channel->wall = 0;
		Channel->userlimit = 0;
                Channel->kickedby = NULL;
		Channel->spylist = NULL;
		Channel->HostList = NULL;
		add_channel(Channel);
		if(dojoin)
		{
			sendjoin(name);
			Channel->active = TRUE;		/* assume it went ok */	
		}
		else
			Channel->active = FALSE;
	}
	else	/* rejoin */
	{
		clear_all_users(&(Channel->users));
		clear_all_bans(&(Channel->banned));
		Channel->mode = 0;
		/* sendjoin doesn't work with keys (fix this) */
		send_to_server("JOIN %s %s", name, 
			       *(Channel->key)?Channel->key:"");
		Channel->active = TRUE;		/* assume it went ok */	
	}	
/*
 * here we should send something like sendwho channel );
 * Yup..., also send something to get the channelmodes
 * -no, do this when we joined the channel
 */
 	currentbot->Current_chan = Channel;
	return( TRUE );
}

int	leave_channel(char *name)
/*
 * removes channel "name" from the list and actually leaves
 * the channel
 */

{
	CHAN_list	*Dummy;

	if((Dummy = search_chan(name)) != NULL)
	{
		sendpart(name);
		if(Dummy == currentbot->Current_chan)
			currentbot->Current_chan = currentbot->Channel_list;
		delete_channel(Dummy);
		/* Channel was found and we left it
     		 * (or at least  removed it from the list)
		 */
		return(TRUE);		
	}
	/*
	 * There was obviously no such channel in the list
	 */
	return(FALSE);
}

int	mark_success(char *name)
/*
 * Marks channel "name" active
 */
{
	CHAN_list	*Dummy;

	if((Dummy = search_chan(name)) != NULL)
	{
		Dummy->active = TRUE;
		/* Perhaps this shouldn't be here, but it makes things 
		   alot easier */
		if(*Dummy->topic)
			sendtopic(name, Dummy->topic);
		sendmode(name, "%s", Dummy->mod);
		return(TRUE);
	}
	return(FALSE);
}

int	mark_failed( char *name )
/*
 * Marks channel "name" not active
 */
{
	CHAN_list	*Dummy;

	if( (Dummy = search_chan( name ) ) != NULL )
	{
		Dummy->active = FALSE;
		return(TRUE);
	}
	return(FALSE);
}

void	show_channellist(char *user)
{
	CHAN_list	*Channel;
	unsigned int	mode;
	char		modestr[MAXLEN];
	int		number;

	if( currentbot->Channel_list == NULL )
	{
		send_to_user( user, "I'm not active on any channels" );
		return;
	}

	send_to_user( user, "Active on: %s", currentbot->Channel_list->name );
	for( Channel = currentbot->Channel_list; Channel; Channel = Channel->next )
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
	        number = tot_num_on_channel(Channel->name);
	    if(Channel->spylist)
		send_to_user( user, "    \002spy\002 -  %s, users: %2i, mode: +%s",
		Channel->name, number, modestr );
	    else
	   	send_to_user( user, "        -  %s, users: %2i, mode: +%s",
		Channel->name, number, modestr );
	}
		return;
} 

void	reset_channels( int Mode )
/*
 * This function tries to join all inactive channels
 */
{
	CHAN_list	*Channel;

	for(Channel = currentbot->Channel_list; Channel; Channel = Channel->next)
		if((Mode == HARDRESET) || !(Channel->active))
			join_channel(Channel->name, Channel->topic, Channel->mod, TRUE);
}

char	*currentchannel()
/*
 * returns name of current channel, i.e. the last joined
 */
{
	if( currentbot->Channel_list )
		strcpy( channel_buf, currentbot->Channel_list->name );
	else
		strcpy( channel_buf, "#0" );
	return(channel_buf);
}

/*
 * Ok, that was the basic channel stuff... Now some functions to
 * add users to a channel, remove them and one to change a nick on ALL
 * channels.
 */

int	add_user_to_channel( char *channel, char *nick, 
                             char *user, char *host )
{
	CHAN_list	*Dummy;

	if( (Dummy = search_chan( channel )) == NULL )
		return(FALSE);
	add_user( &(Dummy->users), nick, user, host );
	return(TRUE);
}

int	remove_user_from_channel( char *channel, char *nick )
{
	CHAN_list	*Dummy;

	if( (Dummy = search_chan( channel )) == NULL )
		return(FALSE);
	delete_user( &(Dummy->users), nick );
	return(TRUE);
}

void	change_nick( char *oldnick, char *newnick )
/*
 * Searches all channels for oldnick and changes it into newnick
 */
{
	CHAN_list	*Channel;

	for( Channel = currentbot->Channel_list; Channel; Channel = Channel->next )
		change_user_nick( &(Channel->users), oldnick, newnick );
}

void	remove_user( char *nick )
/*
 * Remove a user from all channels (signoff)
 */
{
	CHAN_list	*Channel;

	for( Channel = currentbot->Channel_list; Channel; Channel = Channel->next )
		delete_user( &(Channel->users), nick );
}
int	show_users_on_channel( char *from, char *channel )
{
        CHAN_list       *Dummy;
	USER_list	*Blah;
	char		modestr[MAXLEN];

        if( (Dummy = search_chan( channel )) == NULL )
                return(FALSE);
	
		send_to_user( from, "Users on \002%s\002:", channel);
	Blah = Dummy->users;
	while( Blah )
	{
		strcpy(modestr, "");
		if( get_usermode(Blah)&MODE_CHANOP ) strcat(modestr, "+o");
		if( get_usermode(Blah)&MODE_VOICE ) strcat(modestr, "+v" );
	send_to_user( from, "%3du %3ds, mode: %3s, %30s",
		userlevel(get_username(Blah)),
		shitlevel(get_username(Blah)),
		modestr, get_username(Blah));
		Blah = get_nextuser( Blah );
	}
	return(TRUE);
}
void	send_wall(char *from, char *channel, char *wall, ...)
{
	CHAN_list	*Dummy;
	USER_list	*Blah;
	char		out[MAXLEN];
        char    buf[WAYTOBIG];
        va_list msg;

        va_start(msg, wall);
        vsprintf(buf, wall, msg);
        va_end(msg);

        if( (Dummy = search_chan( channel )) == NULL )
                return;

        if (!get_walltog(channel))
		return;

	sprintf(out, "[WallOp/%s] %s", channel, buf);
        Blah = Dummy->users;
        while( Blah )
        {
                if( get_usermode(Blah)&MODE_CHANOP )
		    sendnotice(get_usernick(Blah), out);
                Blah = get_nextuser( Blah );
        }
}
/*
 * now some functions to keep track of modes
 */

void add_channelmode(char *from, char *channel, unsigned int mode, char *params, int time)
{
	CHAN_list	*Channel;

	if((Channel = search_chan(channel))==NULL)
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

void	del_channelmode( char *channel, unsigned int mode, char *params )
{
	CHAN_list	*Channel;

	if((Channel = search_chan(channel))==NULL)
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

void	change_usermode( char *channel, char *user, unsigned int mode )
{
	CHAN_list	*Channel;
	
	if( (Channel = search_chan( channel )) == NULL )
		return;		/* should not happen */

	add_mode( &(Channel->users), mode, user );
}
	
int	open_channel( char *channel )
{
        CHAN_list       *Channel;

        if( (Channel = search_chan( channel )) == NULL )
                return(FALSE);

        if((Channel->mode)&MODE_KEY)
                sendmode(channel, "-ipslk %s", Channel->key);
	else
		sendmode(channel, "-ipsl");
	return(TRUE);
}
 

/*
 * Here is some mass-stuff. Perhaps it doesn't belong here (though..
 * I think this is a quite good place...).
 */

void	channel_massop( char *channel, char *pattern )
{
        CHAN_list       *Channel;
	USER_list	*Dummy;
	char		opstring[MAXLEN];
	int		i;

        if( (Channel = search_chan( channel )) == NULL )
                return;

	Dummy = Channel->users;
	while(Dummy)
	{
		strcpy(opstring, "+oooo ");
		i=0;
		while(Dummy && (i<4))
		{
			if(!fnmatch(pattern, get_username(Dummy), FNM_CASEFOLD)&&
			   !(get_usermode(Dummy)&MODE_CHANOP) &&
			   (shitlevel(get_username(Dummy))==0))
			{
				strcat(opstring, " ");
				strcat(opstring, get_usernick(Dummy));
				i++;
			}
			Dummy=get_nextuser(Dummy);
		}
		sendmode(channel, "%s", opstring);
	}
}
void    channel_massdeop( char *channel, char *pattern )
{
        CHAN_list       *Channel;
        USER_list       *Dummy;
        char            deopstring[MAXLEN];
        int             i;

        if( (Channel = search_chan( channel )) == NULL )
                return;

        Dummy = Channel->users;
        while(Dummy)
        {
                strcpy(deopstring, "-oooo ");
                i=0;
                while(Dummy && (i<4))
		{
                        if(!fnmatch(pattern, get_username(Dummy), FNM_CASEFOLD)&&
                          (get_usermode(Dummy)&MODE_CHANOP)&&
			  (protlevel(get_username(Dummy))<100))
                        {
                                strcat(deopstring, " ");
                                strcat(deopstring, get_usernick(Dummy));
                                i++;
                        }
                	Dummy=get_nextuser(Dummy);
		}
                sendmode(channel, "%s", deopstring);
        }
}
 
void    channel_masskick( char *channel, char *pattern )
{
        CHAN_list       *Channel;
        USER_list       *Dummy;

        if( (Channel = search_chan( channel )) == NULL )
                return;

        Dummy = Channel->users;
        while(Dummy)
        {
                if(!fnmatch(pattern, get_username(Dummy), FNM_CASEFOLD)&&
                  (protlevel(get_username(Dummy))<100))
                	sendkick(channel, get_usernick(Dummy),
				 "**Masskick**");
                Dummy=get_nextuser(Dummy);
        }
}

/*
 * Some misc. function which deal with channels and users
 */

int	invite_to_channel( char *user, char *channel )
{
	if(search_chan( channel ))
	{
		send_to_server("INVITE %s :%s", user, channel);
		return(TRUE);
	}
	return(FALSE);
}
char    *myuser( char *nick )
{
     CHAN_list *Channel;
     USER_list *User;
	for(Channel = currentbot->Channel_list; Channel; Channel = Channel->next)
		if((User = search_user(&(Channel->users), nick)))
			return(get_username(User));
	return(NULL);
}
char	*username( char *nick )
/*
 * Searches all lists for nick and if it finds it, returns 
 * nick!user@host
 */

{
	CHAN_list	*Channel;
	USER_list	*User;

	for(Channel = currentbot->Channel_list; Channel; Channel = Channel->next)
		if((User = search_user(&(Channel->users), nick)))
			return(get_username(User));
	return(NULL);
}

unsigned int	usermode( char *channel, char *nick )
/*
 * returns the usermode of nick on channel
 */
{
	CHAN_list	*Channel;
	USER_list	*Blah;

	if((Channel = search_chan(channel))==NULL)
		return(0);	/* or -1? */

	Blah=Channel->users;
	while( Blah )
	{
		if(STRCASEEQUAL(nick, get_usernick(Blah)))
			return(get_usermode(Blah));
		Blah=get_nextuser(Blah);
	}
	return(0);
}
	
/*
 * Here are the functions to bookkeep the banlist
 */

BAN_list	*search_ban(BAN_list **b_list, char *banstring)
{
	BAN_list	*Banned;

	for(Banned = *b_list; Banned; Banned = Banned->next)
		if(STRCASEEQUAL(Banned->banstring, banstring))
			return(Banned);		
	return(NULL);
}

void	add_ban(BAN_list **b_list, char *from, char *banstring, int time)
{
	BAN_list	*New_ban;

	if( (New_ban = (BAN_list*)malloc(sizeof(*New_ban))) == NULL)
		return;
	if( search_ban(b_list, banstring) != NULL )
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

int	delete_ban(BAN_list **b_list, char *banstring)
{
	BAN_list	**old;
	BAN_list	*Dummy;

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

void    clear_all_bans(BAN_list **b_list)
/*
 * Removes all entries in the list
 */
{
        BAN_list        **old;
	BAN_list	*del;

        for(old = b_list; *old;)
        {
		del = *old;
                *old = (*old)->next;
		free(del->banstring);
		free(del->bannedby);
                free(del);
        }
}

void    channel_massunban(char *channel)
{
        CHAN_list       *Channel;
        BAN_list        *Dummy;
	char		unbanmode[5];
        char            unbanstring[MAXLEN];
        int             i;

        if( (Channel = search_chan( channel )) == NULL )
                return;

        Dummy = Channel->banned;
        while(Dummy)
        {
		/* We can't just -bbb ban1 ban2,
		   because the third b will make the server
	           show the banlist etc. */
		strcpy(unbanmode, "-");
                strcpy(unbanstring, "");
                i=0;
                while(Dummy && (i<3))
                {
			strcat(unbanmode,"b");
                        strcat(unbanstring, " ");
                        strcat(unbanstring, Dummy->banstring);
                        i++;
                        Dummy=Dummy->next;
                }
                sendmode(channel, "%s %s", unbanmode, unbanstring);
        }
}

void	channel_unban(char *channel, char *user)
{
        CHAN_list       *Channel;
        BAN_list        *Dummy;
	char		unbanmode[5];
        char            unbanstring[MAXLEN];
        int             i;

        if( (Channel = search_chan( channel )) == NULL )
                return;

        Dummy = Channel->banned;
        while(Dummy)
        {
		strcpy(unbanmode, "-");
                strcpy(unbanstring, "");
                i=0;
                while(Dummy && (i<3))
                {
			if(!fnmatch(Dummy->banstring, user, FNM_CASEFOLD))
			{
				strcat(unbanmode, "b");
                        	strcat(unbanstring, " ");       
                        	strcat(unbanstring, Dummy->banstring);          
                        	i++;
			}        
                        Dummy=Dummy->next;        
                }
                sendmode(channel, "%s %s", unbanmode, unbanstring);      
        }
}

int	find_highest(char *channel, char *pattern)
/*
 * Find user that matches pattern with the highest protlevel,
 * and return this level. Shitlevel must be 0
 *
 * Kinda kludgy, but I have to clean channel.c up anyhow...
 */
{
	CHAN_list	*Channel;
        USER_list       *Dummy;
	int		highest = 0;

	if((Channel = search_chan(channel)) == NULL)
		return 0;

        Dummy = Channel->users;
        while(Dummy)
        {
                if(!fnmatch(pattern, get_username(Dummy), FNM_CASEFOLD)&&
                  (shitlevel(get_username(Dummy))==0))
			if(protlevel(get_username(Dummy)) > highest)
				highest = protlevel(get_username(Dummy));  
                Dummy=get_nextuser(Dummy);
        }
	return highest;
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
	 return FALSE;
  if ( (getthetime() - User->kicktime) > 10)
	 {
		User->kicktime = getthetime();
		User->kicknum = 0;
	 }
  User->kicknum++;
  if (User->kicknum >= currentbot->masskicklevel)
	 return TRUE;
  return FALSE;
}


int check_massdeop(char *from, char *channel)
{
  CHAN_list *Channel;
  USER_list *User;

  if (!(Channel = search_chan(channel)))
         return FALSE;
  if (!(User = search_user(&(Channel->users), getnick(from))))
         return FALSE;
  if ( (getthetime() - User->deoptime) > 10)
         {
                User->deoptime = getthetime();
                User->deopnum = 0;
         }
  User->deopnum++;
  if (User->deopnum >= currentbot->massdeoplevel)
	return TRUE;
  return FALSE;
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

  send_to_user(from, "Users on %s that are idle more than %i second%s:",
                         channel, seconds, EXTRA_CHAR(seconds));
  ct = getthetime();
  for( User = Channel->users; User; User = User->next )
         if ((ct-User->idletime) > seconds)
        send_to_user(from, "%s: %s", idle2str(ct-User->idletime), get_username(User));
  send_to_user(from, "--- end of list ---");
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

int check_massban(from, channel)
char *from;
char *channel;
{
  CHAN_list *Channel;
  USER_list *User;
  
  if (!(Channel = search_chan(channel)))
	 return FALSE;
  if (!(User = search_user(&(Channel->users), getnick(from))))
	 return FALSE ; 
  if ( (getthetime() - User->bantime) > 10)
	 {
		User->bantime = getthetime();
		User->bannum = 0;
	 }
  User->bannum++;
  if (User->bannum >= currentbot->massbanlevel)
	 return TRUE;
  return FALSE;
}


int check_nickflood(char *from)
{
  CHAN_list *Channel;
  USER_list *User;
  int flag;
  char *temp;
  flag=FALSE;
  for ((Channel = currentbot->Channel_list); Channel; Channel = Channel->next)
  if (User = search_user(&(Channel->users), getnick(from)))
  {
       if ( (getthetime() - User->nicktime) > 10)
       {
          User->nicktime = getthetime();
          User->nicknum = 0;
       }
       User->nicknum++;
       if (User->nicknum >= currentbot->massnicklevel)
      {
	flag = TRUE;
        temp = format_uh(from, 1);
	if (!protlevel(from))
        {
		 add_to_levellist(currentbot->lists->shitlist, temp, 100);
		/* Auto Shitlist :> */
	}
	if (i_am_op(Channel->name))
	{
	   deop_ban(Channel->name, getnick(from), format_uh(from, 1));
	   sendkick(Channel->name, getnick(from), "Don't nickflood on MY channel!");
        }
       }
  }
  return flag;
}


int is_opped(char *nick, char *channel)
{
        return (usermode(channel, nick) & MODE_CHANOP);
}

int set_kickedby(char *name, char *from)
{
        CHAN_list *Dummy;

        if ((Dummy = search_chan(name)))
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

char *get_kickedby(char *name)
{
        CHAN_list *Dummy;

        if ((Dummy = search_chan(name)))
                return Dummy->kickedby;
        return NULL;
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
         return FALSE;

  if ( (getthetime() - User->floodtime) > 5)
         {
                User->floodtime = getthetime();
                User->floodnum = 0;
         }
  User->floodnum++;
  if (get_usermode(User) & MODE_CHANOP)
	 return FALSE;
 
  if (User->floodnum >= currentbot->floodlevel)
	 return currentbot->floodprotlevel;
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
         return FALSE;
  if ((getthetime() - User->capstime) > CAPSTIME)
         {
                User->capstime = getthetime();
                User->capsnum = 0;
         }
                User->capsnum += num;
  if (User->capsnum >= CAPSCNT)
         return TRUE;
  return FALSE;
}

void cycle_channel(channel)
char *channel;
{
  sendpart(channel);
  join_channel(channel, "", "", TRUE);
}

int   mycheckmode(char *from)
{
#ifdef ENFORCE_MODES
   if (userlevel(from)<ASSTLVL) 
        return TRUE;
   else
        return FALSE;
#else
   return FALSE;
#endif
}

char *findchannel(to, rest)
char *to;
char **rest;
{
	static char chan[MAXLEN];

	strcpy(chan, to);

	if (!ischannel(to))
		strcpy(chan, currentchannel());
	if (*rest && **rest)
	  if (ischannel(*rest))
	    strcpy(chan, get_token(rest, " "));
	return chan;
}

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

void check_bans(void)
{
	CHAN_list	*Channel;
	USER_list	*User;

	for (Channel = currentbot->Channel_list; Channel; Channel = Channel->next)
		for (User = Channel->users; User; User = User->next)
			if (is_banned(get_username(User), Channel->name) &&
				(userlevel(get_username(User)) < 50))
		sendkick(Channel->name, User->nick, "Banned");
}

void check_shit(void)
{
  CHAN_list *Channel;
  USER_list *User;

  for (Channel = currentbot->Channel_list; Channel; Channel = Channel->next)
	 for( User = Channel->users; User; User = User->next )
		if (shitlevel(get_username(User)))
		    shit_action(get_username(User), Channel->name);
		else if (userlevel(get_username(User)) && !(get_usermode(User) & MODE_CHANOP))
		    giveop(Channel->name, get_usernick(User));
}

int get_logtog(name)
char *name;
{
  CHAN_list *Dummy;

  if ((Dummy = search_chan(name)))
	 return Dummy->log;
  return 0;
}
int get_walltog(name)
char *name;
{
  CHAN_list *Dummy;

  if ((Dummy = search_chan(name)))
	 return Dummy->wall;
  return 0;
}
int get_prottog(name)
char *name;
{
  CHAN_list *Dummy;

  if ((Dummy = search_chan(name)))
	 return Dummy->prot;
  return 0;
}

int get_aoptog(name)
char *name;
{
  CHAN_list *Dummy;

  if ((Dummy = search_chan(name)))
	 return Dummy->aop;
  return 0;
}
int get_shittog(name)
char *name;
{
  CHAN_list *Dummy;

  if ((Dummy = search_chan(name)))
	 return Dummy->shit;
  return 0;
}
int get_masstog(name)
char *name;
{
  CHAN_list *Dummy;

  if ((Dummy = search_chan(name)))
         return Dummy->mass;
  return 0;
}
int get_singtog(name)
char *name;
{
  CHAN_list *Dummy;

  if ((Dummy = search_chan(name)))
         return Dummy->sing;
  return 0;
}
int get_sdtog(name)
char *name;
{
  CHAN_list *Dummy;

  if ((Dummy = search_chan(name)))
	 return Dummy->sd;
  return 0;
}

int get_sotog(name)
char *name;
{
  CHAN_list *Dummy;

  if ((Dummy = search_chan(name)))
         return Dummy->so;
  return 0;
}

void show_clonebots(from, channel)
char *from;
char *channel;
{
	CHAN_list *Channel;
	TS *Bleah;

	if (!(Channel = search_chan(channel)))
		return;
	for (Bleah = *Channel->HostList; Bleah;Bleah = Bleah->next)
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
		Bleah->num++;
	}
	if (Bleah->num > 5)
	{
/*		channel_massunban(channel, thehost, 0); */
		sendmode(channel, "+b %s", thehost);
		if (i_am_op(channel))
		  channel_masskick(channel, thehost);
	}
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

int get_greettog(name)
char *name;
{
  CHAN_list *Dummy;

  if ((Dummy = search_chan(name)))
        return Dummy->greet;
  return 0;
}
int get_pubtog(name)
char *name;
{
  CHAN_list *Dummy;

  if ((Dummy = search_chan(name)))
        return Dummy->pub;
  return 0;
}
int get_bonktog(name)
char *name;
{
  CHAN_list *Dummy;

  if ((Dummy = search_chan(name)))
        return Dummy->bonk;
  return 0;
}

int get_iktog(name)
char *name;
{
  CHAN_list *Dummy;
  
  if ((Dummy = search_chan(name)))
	 return Dummy->ik;
  return 0;
}
int get_limit(name)
char *name;
{
  CHAN_list *Dummy;
  
  if ((Dummy = search_chan(name)))
	 return Dummy->userlimit;
  return 0;
}

int set_limit(name, num)
char *name;
int num;
{
  CHAN_list *Dummy;
  
  if ((Dummy = search_chan(name)))
	 {
		Dummy->userlimit = num;
		return TRUE;
	 }
  return FALSE;
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
		return FALSE;
	if ((getthetime() - User->beeptime) > 10)
	{
		User->beeptime = getthetime();
		User->beepnum = 0;
	}
	if (!userlevel(get_username(User)) &&
			!(get_usermode(User) & MODE_CHANOP))
		User->beepnum += num;
	if ((User->beepnum >= currentbot->beepkicklevel) && (Channel->bk))
		return TRUE;
	return FALSE;
}

void check_idle(void)
{
  CHAN_list *Channel;
  USER_list *User;
  time_t ct;

  ct = getthetime();
  for (Channel = currentbot->Channel_list; Channel; Channel = Channel->next)
	 for( User = Channel->users; User; User = User->next )
		if ((ct - User->idletime) >= currentbot->idlelevel)
	{
	  if (!userlevel(get_username(User)) &&
			Channel->ik &&
			!(get_usermode(User) & MODE_CHANOP))
		 sendkick(Channel->name, User->nick, "Idle for %s",
			  idle2str(getthetime()-User->idletime));
	  User->idletime = getthetime();
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
		send_to_user(from, "I'm not on %s", channel);
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
  send_to_user(from, "Out of %i people on %s, %i are ops",
			 tot, channel, ops);

}

