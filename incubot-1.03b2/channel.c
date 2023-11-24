/*
 * channel.c - a better implementation to handle channels on IRC
 * (c) 1993 VladDrac (irvdwijk@cs.vu.nl)
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

#include <stddef.h>
#include <stdio.h>
#ifndef SYSV
#include <strings.h>
#endif
#include <string.h>
#include <time.h>

#include "channel.h"
#include "chanuser.h"
#include "config.h"
#include "fnmatch.h"
#include "misc.h"
#include "send.h"
#include "userlist.h"
#include "vladbot.h"
#include "vlad-ons.h"
#include "tb-ons.h"
#include "incu-ons.h"

extern	botinfo	*currentbot;
extern	int	userlevel(char *from);
extern	int	protlevel(char *from);
extern	int	shitlevel(char *from);

extern	short	kick_check;

static	char	channel_buf[MAXLEN];
int		sent_ban=0;

CHAN_list	*search_chan( char *name )
{
	CHAN_list	*Channel;

	for( Channel = currentbot->Channel_list; Channel;
		Channel = Channel->next )
	{
		if( STRCASEEQUAL( name, Channel->name ) )
			return(Channel);
	}

	return(NULL);
}

int	check_channel_flood	(char *user, char *chan, int type)
/*
 * This function checks if a user has public flooded or nick flooded.
 * "type" is 1 for public floods, 2 for nick floods, 3 for CTCP floods
 */
{
	CHAN_list	*Channel;
	USER_list	*User;
	int userlvl;

	/* seems the bot sigsegvs if someone changes nicks immediately
		after it joins cause it hasn't built a channel name list yet */

	if (chan==NULL || user==NULL)
		return FALSE;

	userlvl=userlevel(user);
	if (userlvl > 25)
		return FALSE;

	if ((Channel=search_chan(chan))==NULL)
		return FALSE;

	if ((User=search_user(&(Channel->users),getnick(user)))==NULL)
		return FALSE;

	if (User->kick_flag)
		return TRUE;

	/* Public flood */
	if (type==1)
	{
		if(userlvl == 25 && currentbot->l_25_flags&FL_NOPUBLICKICK)
			return FALSE;
		User->numchanline++;

		if ((time(NULL)-User->firstchanline)<=FLOOD_PUBLIC_INTERVAL)
		{
			if (User->numchanline>FLOOD_PUBLIC_RATE)
			{
				User->kick_flag = 1;
				kick_check = TRUE;
				return TRUE;
			}
			return FALSE;
		}
		else
		{
			User->numchanline=1;
			User->firstchanline=time(NULL);
		}
	}
	/* Nick Flood */
	else if (type==2)
	{
		User->numnickch++;

		if ((time(NULL)-User->firstnickch)<=FLOOD_NICK_INTERVAL)
		{
			if (User->numnickch>FLOOD_NICK_RATE)
			{
				User->kick_flag = 2;
				kick_check = TRUE;
				return TRUE;
			}
			return FALSE;
		}
		else
		{
			User->numnickch=1;
			User->firstnickch=time(NULL);
		}
	}
	/* CTCP Flood */
	else if (type==3)
	{
		User->numctcp++;

		if ((time(NULL)-User->firstctcp)<=FLOOD_CTCP_INTERVAL)
		{
			if (User->numctcp>FLOOD_CTCP_RATE)
			{
				User->kick_flag = 3;
				kick_check = TRUE;
				return TRUE;
			}
			return FALSE;
		}
		else
		{
			User->numctcp=1;
			User->firstctcp=time(NULL);
		}
	}

	return FALSE;
}

void	check_numbans	(char *name)
/*
 * If the ban list is full, remove the last channel ban
 *
 */
{
	BAN_list	*Banlist;
	CHAN_list	*Channel;

	if (name==NULL)
		return;

	if ((Channel=search_chan(name))==NULL)
		return;

	if (Channel->numbans<MAX_CHANNEL_BANS)
		return;

	Banlist = Channel->banned;

	while(Banlist->next!=NULL)
		Banlist = Banlist->next;

	sendmode(Channel->name,"-b %s",Banlist->banstring);
}

void	check_channel_bans	(char *name)
/*
 * This function checks the ban list and kicks users matching the
 * ban masks.
 */
{
		CHAN_list *Channel;
		BAN_list	*Banlist;
      USER_list	*User;
		int userlvl;
		int excludelvl;

	if (name==NULL)
		return;

	if ((Channel=search_chan(name))==NULL)
		return;

	for (User=Channel->users; User; User=User->next)
	{
		userlvl = userlevel(get_username(User));
		excludelvl = excludelevel(get_username(User));
		if (userlvl < MIN_USERLEVEL_FOR_OPS && !excludelvl)
		{
			if(userlvl == 25 && currentbot->l_25_flags&FL_NOBANKICK)
				continue;
			for (Banlist=Channel->banned; Banlist; Banlist=Banlist->next)
			{
				if (!mymatch(Banlist->banstring,get_username(User),FNM_CASEFOLD))
				{
					kick_check=TRUE;
					User->kick_flag = 5;
				}
			}
		}
	}
}

void	check_channel_clones (char *user, char *chan)
{
/*	BAN_list	*Banlist; */
	CHAN_list	*Channel;
	USER_list	*User;
	char		usermask[MAXLEN];
	char		*userm;
	int		count=0;
	int		userlvl;
/*	int		numkicks=0; */

	if (chan==NULL || user==NULL)
		return;

	userlvl = userlevel(user);

	if(currentbot->floodprot_clone == 0)
		return;
	if (userlvl > 25 || (userlvl == 25 && currentbot->l_25_flags&FL_NOCLONEKICK))
	{
		return;
	}

	if ((Channel=search_chan(chan))==NULL)
		return;

	if (sent_ban)
		return;

	/* If they are already banned, let check_ops() or on_mode()
		handle the kicking */

/*
	for (Banlist=Channel->banned; Banlist; Banlist=Banlist->next)
		if (!mymatch(Banlist->banstring,user,FNM_CASEFOLD))
	{
			return;
	}
*/

	userm = user;

	while (*userm!='!')
		userm++;

	userm++;
	sprintf(usermask,"*!%s",userm);

	for (User=Channel->users; User; User=User->next)
	{
		userlvl = userlevel(get_username(User));
		if (!mymatch(usermask,get_username(User),FNM_CASEFOLD))
		{
			if(userlvl > 25 || (userlvl==25 && currentbot->l_25_flags&FL_NOCLONEKICK))
				continue;

			count++;

			if (count == 3)
			{
				switch(currentbot->floodprot_clone)
				{
					case 2:
						ban_user(get_username(User), Channel->name);
						sent_ban=1;
						break;
					case 3:
						ban_user_timed(get_username(User), Channel->name, time(NULL) + 300);
						sent_ban=1;
						break;
					default:
						break;
				}
			}
			if (count>2)
			{
				User->kick_flag = 4;
				kick_check = TRUE;
			}
		}
	}
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
	CHAN_list	*old=NULL,*p=currentbot->Channel_list;

	while (p != Channel && p!=NULL)
	{
		old=p;
		p=p->next;
	}

	if (p==NULL) return(FALSE);
	if (old==NULL)
		currentbot->Channel_list=currentbot->Channel_list->next;
	else
		old->next=p->next;

	clear_all_users(&(p->users));
	free(p->name);
	free(p->key);
	free(p->topic);
	free(p->mod);
	free(p);
	return(TRUE);

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
		if( (Channel = (CHAN_list*)malloc( sizeof( *Channel ) ) )
			  == NULL )
			return(FALSE);
		mstrcpy(&Channel->name, name);
		Channel->numbans = 0;
		Channel->users = NULL;
		Channel->banned = NULL;
		Channel->mode = 0;
		mstrcpy(&Channel->key,"");
		mstrcpy(&Channel->topic, topic?topic:"");
		mstrcpy(&Channel->mod, mode?mode:"");
		add_channel(Channel);
		Channel->joined = FALSE;
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
		Channel->numbans = 0;
		Channel->mode = 0;
		/* sendjoin doesn't work with keys (fix this) */
		send_to_server("JOIN %s %s", name,
					 *(Channel->key)?Channel->key:"");
		Channel->active = TRUE;		/* assume it went ok */
		Channel->joined = FALSE;	/* i'm not there yet */
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
		Dummy->joined = TRUE;
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
		Dummy->joined = FALSE;
		return(TRUE);
	}
	return(FALSE);
}

void	show_channellist(char *user)
{
	CHAN_list	*Channel;
	unsigned int	mode;
	char		modestr[MAXLEN];

	if( currentbot->Channel_list == NULL )
	{
		send_to_user( user, "I'm not active on any channels" );
		return;
	}

	send_to_user( user, "Active on: %s", currentbot->Channel_list->name );
	for( Channel = currentbot->Channel_list; Channel; Channel = Channel->next )
	{
		if(Channel->joined && Channel->active)
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
			send_to_user( user, "    %co    -  %s, mode: +%s", usermode(Channel->name,currentbot->nick)&MODE_CHANOP?'+':'-', Channel->name, modestr);
		}
		else
		{
			send_to_user( user, "             %s Not joined", Channel->name);
		}
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

int	add_user_to_channel( char *channel, char *nick, char *user, char *host )
{
	CHAN_list	*Dummy;

	if( (Dummy = search_chan( channel )) == NULL )
		return(FALSE);
	add_user( &(Dummy->users), nick, user, host );
	addseen(nick,0,"no info");
	return(TRUE);
}

int	remove_user_from_channel( char *channel, char *nick )
{
	CHAN_list	*Dummy;
	USER_list	*Blah;

	if( (Dummy = search_chan( channel )) == NULL )
		return(FALSE);
	delete_user( &(Dummy->users), nick );
	Blah=Dummy->users;
	while (Blah!=NULL && strcmp(Blah->nick,currentbot->nick)!=0) Blah=Blah->next;
	if (Dummy->users!=NULL && Dummy->users->next==NULL && !(Blah->mode&MODE_CHANOP))
		do_cycle(currentbot->nick,currentbot->nick,channel);
	return(TRUE);
}

void	change_nick( char *oldnick, char *newnick )
/*
 * Searches all channels for oldnick and changes it into newnick
 */
{
	CHAN_list	*Channel;

	addseen(oldnick,3,newnick);
	addseen(newnick,0,"no info");

	for( Channel = currentbot->Channel_list; Channel; Channel = Channel->next )
		change_user_nick( &(Channel->users), oldnick, newnick );
}

void	remove_user( char *nick, char *rest )
/*
 * Remove a user from all channels (signoff)
 */
{
	CHAN_list	*Channel;

	addseen(nick,2,rest);
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

	Blah = Dummy->users;
	while( Blah )
	{
		strcpy(modestr, "");
		if( get_usermode(Blah)&MODE_CHANOP ) strcat(modestr, "+o");
		if( get_usermode(Blah)&MODE_VOICE ) strcat(modestr, "+v" );
		send_to_user( from, "%s: %3du (%c) %3ds %3dp, mode: %4s %s",
					channel,
					userlevel(get_username(Blah)),
					Blah->auth?'*':' ',
					shitlevel(get_username(Blah)),
					protlevel(get_username(Blah)),
					modestr,
					get_username(Blah) );
		Blah = get_nextuser( Blah );
	}
	return(TRUE);
}

/*
 * now some functions to keep track of modes
 */

void	add_channelmode( char *channel, unsigned int mode, char *params )
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
		if (add_ban(&(Channel->banned), params, 0))
			Channel->numbans++;
		break;
	case MODE_BAN_367: /* Called from parse_367() bans need to be added in reverse */
		if (add_ban(&(Channel->banned), params, 1))
			Channel->numbans++;
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
		if (delete_ban(&(Channel->banned), params) &&
			(Channel->numbans > 1))
			Channel->numbans--;
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
					 sendmode(channel, "-iplk %s", Channel->key);
	else
		sendmode(channel, "-ipl");
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
		strcpy(opstring, "+ooo ");
		i=0;
		while(Dummy && (i<3))
		{
			if(!mymatch(pattern, get_username(Dummy), FNM_CASEFOLD)&&
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
					 strcpy(deopstring, "-ooo ");
					 i=0;
					 while(Dummy && (i<3))
		{
								if(!mymatch(pattern, get_username(Dummy), FNM_CASEFOLD)&&
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

void    channel_masskick	(char *from, char *channel, char *pattern)
{
	CHAN_list       *Channel;
	USER_list       *Dummy;
	int				NumKicks=0;

	if( (Channel = search_chan( channel )) == NULL )
		return;

	Dummy = Channel->users;
	while(Dummy)
	{
		if(!mymatch(pattern, get_username(Dummy), FNM_CASEFOLD)&&
			(protlevel(get_username(Dummy))<100) &&
			(userlevel(get_username(Dummy))!=OTHER_BOT_LEVEL) &&
			(!STRCASEEQUAL(Dummy->nick,currentbot->nick)))
		{
			sendkick(channel, get_usernick(Dummy),"**Masskick**");
			NumKicks++;
		}

		Dummy=get_nextuser(Dummy);
	}

	send_to_user(from,"%d users were kicked that matched %s",NumKicks,pattern);
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

char	*userchannel( char *nick )
/*
 * Searches all lists for nick and if it finds it, returns
 * first channel user is in.
 */

{
	CHAN_list	*Channel;
	USER_list	*User;

	for(Channel = currentbot->Channel_list; Channel; Channel = Channel->next)
		if((User = search_user(&(Channel->users), nick)))
			return(Channel->name);
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

int add_ban(BAN_list **b_list, char *banstring, int order)
{
	BAN_list	*New_ban;
	BAN_list	*dummy;

	if( (New_ban = (BAN_list*)malloc(sizeof(*New_ban))) == NULL)
		return FALSE;

	if( search_ban(b_list, banstring) != NULL )
	{
		free(New_ban);
		return FALSE;
	}

	mstrcpy(&New_ban->banstring, banstring );
	New_ban->unban_time = -1;

	if(!order)
	{
		New_ban->next = NULL;

		if (*b_list)
		{
			for (dummy = *b_list; dummy->next; dummy = dummy->next);
			dummy->next = New_ban;
		}
		else
			*b_list = New_ban;
	}
	else
	{
		New_ban->next = *b_list;
		*b_list = New_ban;
	}

	return TRUE;
}

int add_timed_ban(BAN_list **b_list, char *banstring, long bantime)
{
	BAN_list	*New_ban;

	if( (New_ban = (BAN_list*)malloc(sizeof(*New_ban))) == NULL)
		return FALSE;

	if( search_ban(b_list, banstring) != NULL )
	{
		free(New_ban);
		return FALSE;
	}

	mstrcpy(&New_ban->banstring, banstring );
	New_ban->unban_time = bantime;
	New_ban->next = *b_list;
	*b_list = New_ban;

	return TRUE;
}

int delete_ban(BAN_list **b_list, char *banstring)
{
	BAN_list	*old=NULL,*p=*b_list;
	BAN_list	*Dummy;

	if((Dummy = search_ban(b_list, banstring)) == NULL)
		return(FALSE);

	while (p!=NULL && p!=Dummy)
	{
		old=p;
		p=p->next;
	}

	if (p==NULL)
		return FALSE;

	if (old==NULL)
		*b_list = (*b_list)->next;
	else
		old->next = p->next;

	free(p->banstring);
	free(p);

	return(TRUE);
}

void clear_all_bans(BAN_list **b_list)
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
		free(del);
	}

	*b_list=NULL;
}

void channel_massunban(char *channel)
{
	CHAN_list   *Channel;
	BAN_list    *Dummy;
	char			unbanmode[5];
	char        unbanstring[MAXLEN];
	int         i;

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

			if (i!=0)
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
			if(!mymatch(Dummy->banstring, user, FNM_CASEFOLD))
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

int	find_highest(char *channel, char *pattern, int type)
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
				if(!mymatch(pattern, get_username(Dummy), FNM_CASEFOLD)&&
                  (shitlevel(get_username(Dummy))==0))
					switch(type)
					{
						case 1:
							if(protlevel(get_username(Dummy)) > highest)
								highest = protlevel(get_username(Dummy));
							break;
						case 2:
				         if(protlevel(get_username(Dummy)) > highest)
								if(userlevel(get_username(Dummy)) > highest && (checkauth(getnick(get_username(Dummy))) || Dummy->mode&MODE_CHANOP))
									highest = protlevel(get_username(Dummy));
							break;
					}

				Dummy=get_nextuser(Dummy);
        }
	return highest;
}

void addban(char *banmask, char *channel, char *nick)
/*
 *   Due to lame IRCD code we'll have to do things the hard way here.
 *   First, check to see if an exact duplicate ban exists and bail if so.
 *   Then cycle through the banlist and clear out any matching bans.
 *   Then cycle through again and keep removing bans until there are 20 bans or less.
 *   Finally, cycle through again until the total length of all banstrings put together is less than 1024.
 */
{
	CHAN_list	*Channel;
	BAN_list		*ban;
	char			modestr[20];
	char			modelist[512];
	int			numbans;
	int			num_modes = 0;
	char			*newban = NULL;
	int			totalbytes = 0; /* Shut up compiler warning */
	int			banlen;

	if((Channel = search_chan(channel)) == NULL)
		return;

	for(ban = Channel->banned; ban; ban = ban->next)
	{
		if(!strcasecmp(banmask, ban->banstring)) // Person is already banned. Lets bail.
			return;
		totalbytes += strlen(ban->banstring);
	}

	strcpy(modestr, "-");
	strcpy(modelist, "");

	numbans = Channel->numbans;

	if(nick && *nick)
	{
		strcat(modestr, "o");
		strcat(modelist, " ");
		strcat(modelist, nick);
		num_modes++;
	}

	for(ban = Channel->banned; ban; ban = ban->next)
	{ /* Cycle through the banlist and clear out any matching bans. */
		if(!mymatch(ban->banstring, banmask, FNM_CASEFOLD) || !mymatch(banmask, ban->banstring, FNM_CASEFOLD))
		{
			newban = ban->banstring;
		}

		banlen = newban?strlen(newban):0;

		if((newban && (banlen + strlen(modelist) + 1 >= 190)) || num_modes == 4)
		{
			sendmode(channel, "%s %s", modestr, modelist);
			num_modes = 0;
			strcpy(modestr, "-");
			*modelist = '\0';
		}

		if(newban)
		{
			strcat(modestr, "b");
			strcat(modelist, " ");
			strcat(modelist, newban);
			num_modes++;
			numbans--;
			totalbytes -= banlen;
			newban = NULL;
		}
	}
	for(ban = Channel->banned; numbans > 20; ban = ban->next)
	{ /* Cycle through again and keep removing bans until there are 20 bans or less. */
		banlen = strlen(ban->banstring);

		if((banlen + strlen(modelist) + 1 >= 190) || num_modes == 4)
		{
			sendmode(channel, "%s %s", modestr, modelist);
			num_modes = 0;
			strcpy(modestr, "-");
			*modelist = '\0';
		}

		strcat(modestr, "b");
		strcat(modelist, " ");
		strcat(modelist, ban->banstring);
		num_modes++;
		numbans--;
		totalbytes -= banlen;
	}

	for(ban = Channel->banned; totalbytes > 1024; ban = ban->next)
	{ /* Cycle through again until the total length of all banstrings put together is less than 1024. */
		banlen = strlen(ban->banstring);

		if((banlen + strlen(modelist) + 1 >= 190) || num_modes == 4)
		{
			sendmode(channel, "%s %s", modestr, modelist);
			num_modes = 0;
			strcpy(modestr, "-");
			*modelist = '\0';
		}

		strcat(modestr, "b");
		strcat(modelist, " ");
		strcat(modelist, ban->banstring);
		num_modes++;
		totalbytes -= banlen;
	}

	if(strlen(banmask) + strlen(modelist) + 1 >= 190)
	{
		sendmode(channel, "%s %s", modestr, modelist);
		sendmode(channel, "+b %s", banmask);
	}
	else
	{
		strcat(modestr, "+b");
		strcat(modelist, " ");
		strcat(modelist, banmask);
		sendmode(channel, "%s %s", modestr, modelist);
	}
}
