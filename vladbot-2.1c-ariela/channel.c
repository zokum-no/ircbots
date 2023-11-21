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
#include <strings.h>
#include <string.h>

#include "channel.h"
#include "chanuser.h"
#include "config.h"
#include "fnmatch.h"
#include "misc.h"
#include "userlist.h"
#include "vladbot.h"

extern	botinfo	*currentbot;
extern	int	userlevel(char *from);
extern	int	protlevel(char *from);
extern	int	shitlevel(char *from);

static	char	channel_buf[MAXLEN];

CHAN_list	*search_chan( char *name )
{
	CHAN_list	*Channel;

	for( Channel = currentbot->Channel_list; Channel; Channel = Channel->next )
		if( STRCASEEQUAL( name, Channel->name ) )
			return(Channel);
	return(NULL);
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
			free(Channel);
			return(TRUE);
		}
	return( FALSE );
}

void	delete_all_channels()
{
	CHAN_list	*Channel;

	for(; Channel = currentbot->Channel_list;)
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
		Channel->users = NULL;
		Channel->banned = NULL;
		Channel->mode = 0;
		mstrcpy(&Channel->key,"");
		mstrcpy(&Channel->topic, topic?topic:"");
		mstrcpy(&Channel->mod, mode?mode:"");
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
		send_to_user( user, "        -  %s, mode: +%s", 
		Channel->name, modestr );
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
	
	Blah = Dummy->users;
	while( Blah )
	{
		strcpy(modestr, "");
		if( get_usermode(Blah)&MODE_CHANOP ) strcat(modestr, "+o");
		if( get_usermode(Blah)&MODE_VOICE ) strcat(modestr, "+v" );
		send_to_user( from, "%s: %30s, %3du %3ds %3dp, mode: %s", 
			      channel, get_username(Blah),
			      userlevel(get_username(Blah)),
			      shitlevel(get_username(Blah)),
			      protlevel(get_username(Blah)), 
			      modestr );
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
		add_ban(&(Channel->banned), params);
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
		strcpy(opstring, "+ooo ");
		i=0;
		while(Dummy && (i<3))
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
                strcpy(deopstring, "-ooo ");
                i=0;
                while(Dummy && (i<3))
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
		send_to_server("INVITE %s %s", channel, user);
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
		if(User = search_user(&(Channel->users), nick))
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

void	add_ban(BAN_list **b_list, char *banstring)
{
	BAN_list	*New_ban;

	if( (New_ban = (BAN_list*)malloc(sizeof(*New_ban))) == NULL)
		return;
	if( search_ban(b_list, banstring) != NULL )
		return;
	mstrcpy(&New_ban->banstring, banstring );
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
