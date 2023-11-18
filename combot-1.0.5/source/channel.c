/*
 * channel.c - a better implementation to handle channels on IRC
 */

#include <stddef.h>
#include <stdio.h>
/*
#include <strings.h>
*/
#include <string.h>
#include "chanuser.h"
#include "channel.h"
#include "function.h"
#include "userlist.h"
#include "misc.h"
#include "config.h"

CHAN_list *Channel_list = NULL;
char mycurchannel[MAXLEN] = "";
static	char	channel_buf[MAXLEN];
char globsign = ' ';
extern int idlelimit, floodlimit, malevel;
extern int idletoggle, floodtoggle;
extern char current_nick[MAXNICKLEN];

CHAN_list *search_chan( char *name )
{
	CHAN_list *Channel;

	for( Channel = Channel_list; Channel; Channel = Channel->next )
		if( !strcasecmp( name, Channel->name ) )
			return(Channel);
	return(NULL);
}

char *get_enforced_modes( char *name )
{
	CHAN_list *Channel;
	static char enf_modes[20];
	*enf_modes = '\0';
	if( (Channel = search_chan( name )) != NULL)
		strcpy(enf_modes, Channel->enfmodes);
	if (!*enf_modes)
	    if (globsign == '+')
	      strcpy(enf_modes, validenforced_modes);
	      
	return enf_modes;
}

int  set_enforced_modes( char *name, char *themodes)
{
	CHAN_list *Channel;
	if ( (Channel = search_chan( name )) != NULL)
	{
		strcpy(Channel->enfmodes, themodes);
		return TRUE;
	}
	return FALSE;
}

int add_spy_chan(char *name, char *nick)
{
	CHAN_list *Channel;
	if ( (Channel = search_chan( name )) != NULL)
		return add_to_list(&(Channel->spylist), nick);
	return FALSE;
}

int del_spy_chan(char *name, char *nick)
{
	CHAN_list *Channel;
	if ( (Channel = search_chan( name )) != NULL)
		return remove_from_list(&(Channel->spylist), nick);
	return FALSE;
}

void show_spy_list(char *from, char *name)
{
   ListStruct *temp;

   CHAN_list *Channel;
   if ((Channel = search_chan(name)) != NULL)
     for (temp=Channel->spylist;temp;temp=temp->next)
       send_to_user(from, temp->name);
}

void send_spy_chan(char *name, char *text)
{
	ListStruct *temp;

	CHAN_list *Channel;
	if ( (Channel = search_chan( name )) != NULL)
		for (temp=Channel->spylist;temp;temp=temp->next)
			send_to_user(temp->name, text);
}

int is_spy_chan(char *name, char *nick)
{
	CHAN_list *Channel;
	if ( (Channel = search_chan( name )) != NULL)
		return is_in_list(Channel->spylist, nick);
	return FALSE;
}

void add_channel( CHAN_list *Channel )
/*
 * adds Channel to the list... 
 */

{
	Channel->next = Channel_list;
	Channel_list = Channel;
}

int  delete_channel( CHAN_list *Channel )
/*
 * removes channel from list 
 */

{
	CHAN_list	**old;

	for( old = &Channel_list; *old; old = &(**old).next )
		if( *old == Channel )
		{
			*old = Channel->next;
			clear_all_users( &(Channel->users) );
			remove_all_from_list( &(Channel->spylist));
			free( Channel );
			return( TRUE );
		}
	return( FALSE );
}

int  join_channel( char *name )
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

	setcurrentchannel( name);

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
		strcpy(Channel->name, name);
		strcpy(Channel->enfmodes, "nt");
		Channel->spylist = init_list();
		Channel->users = NULL;
		Channel->banned = NULL;
		Channel->mode = 0;
		strcpy(Channel->key,"");
		add_channel(Channel);
		sendjoin(name);
	}
	else	/* rejoin */
	{
		clear_all_users(&(Channel->users));
		clear_all_bans(&(Channel->banned));
		Channel->mode = 0;
		/* sendjoin doesn't work with keys (fix this) */
		send_to_server("JOIN %s %s", name, 
			       *(Channel->key)?Channel->key:"");
	}	
/*
 * We probably should check if it's a legal channelname.
 * But that can be done later...
 */
	Channel->active = TRUE;		/* assume it went ok */	
/*
 * here we should send something like sendwho channel );
 * Yup..., also send something to get the channelmodes
 */
	send_to_server( "WHO %s", name );
	send_to_server( "MODE %s", name );
	send_to_server( "MODE %s b", name );
        send_to_server( "MODE %s +isw", current_nick);

/*
 * We should define some nice constants like
 * ALREADY_ONCHANNEL and ISNO_CHANNEL or whatever.
 * This way the bot can return error and stuff.
 * I'll do this later...
 */
	return( TRUE );
}

int mychannel( char *name)
{
	return ( search_chan( name ) != NULL);
}

int  leave_channel( char *name )
/*
 * removes channel "name" from the list and actually leaves
 * the channel
 */

{
	CHAN_list	*Dummy;

	if( (Dummy = search_chan( name )) != NULL )
	{
		sendpart( name );
		delete_channel( Dummy );
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

int  mark_success( char *name )
/*
 * Marks channel "name" active
 */
{
	CHAN_list	*Dummy;

	if( (Dummy = search_chan( name ) ) != NULL )
	{
		Dummy->active = TRUE;
		return(TRUE);
	}
	return(FALSE);
}

int  mark_failed( char *name )
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

void show_channellist( char *user )
{
	CHAN_list	*Channel;
	unsigned int	mode;
	char		modestr[MAXLEN];

	if( Channel_list == NULL )
	{
		send_to_user( user, "I'm not active on any channels" );
		return;
	}

	send_to_user( user, "Active on: %s", Channel_list->name );
	for( Channel = Channel_list; Channel; Channel = Channel->next )
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
			strcat( modestr, Channel->key );
			strcat( modestr, " " );
		}
		if( mode&MODE_LIMIT )
		{
			strcat( modestr, "l " );
			strcat( modestr, Channel->limit );
		}
		send_to_user( user, "        -  %s, mode: +%s", Channel->name,
			modestr );
	}
	return;
} 

void reset_channels( int Mode )
/*
 * This function tries to join all inactive channels
 */

{
	CHAN_list	*Channel;

	for( Channel = Channel_list; Channel; Channel = Channel->next )
		if( (Mode == HARDRESET) || !(Channel->active) )
			join_channel( Channel->name );
}

void setcurrentchannel( char *channel)
{
	strcpy(mycurchannel, channel);
}

char	*currentchannel()
/*
 * returns name of current channel, i.e. the last joined
 */
{
	CHAN_list *Dummy;

	if( (Dummy = search_chan( mycurchannel )) == NULL )
	{
		if( Channel_list )
			strcpy( channel_buf, Channel_list->name );
		else
			strcpy( channel_buf, "#0" );
	}
	else
		strcpy( channel_buf, mycurchannel);
	return(channel_buf);
}

/*
 * Ok, that was the basic channel stuff... Now some functions to
 * add users to a channel, remove them and one to change a nick on ALL
 * channels.
 */

int  add_user_to_channel( char *channel, char *nick, char *user, char *host )
{
	CHAN_list	*Dummy;

	if( (Dummy = search_chan( channel )) == NULL )
		return(FALSE);
	add_user( &(Dummy->users), nick, user, host );
	return(TRUE);
}

int  remove_user_from_channel( char *channel, char *nick )
{
	CHAN_list	*Dummy;

	if( (Dummy = search_chan( channel )) == NULL )
		return(FALSE);
	delete_user( &(Dummy->users), nick );
	return(TRUE);
}

void change_nick( char *oldnick, char *newnick )
/*
 * Searches all channels for oldnick and changes it into newnick
 */
{
	CHAN_list	*Channel;

	for( Channel = Channel_list; Channel; Channel = Channel->next )
		change_user_nick( &(Channel->users), oldnick, newnick, Channel->name );
}

void remove_user( char *nick )
/*
 * Remove a user from all channels (signoff)
 */
{
	CHAN_list	*Channel;

	for( Channel = Channel_list; Channel; Channel = Channel->next )
		delete_user( &(Channel->users), nick );
}

void send_kickall( char *nick, char *msg)
{
	CHAN_list *Channel;
	for (Channel=Channel_list; Channel; Channel = Channel->next)
		sendkick(Channel->name, nick, msg);
}

void send_banall(char *nuh)
{
	CHAN_list *Channel;
	for (Channel=Channel_list; Channel; Channel = Channel->next)
		send_to_server("MODE %s +b %s", Channel->name, nuh);
}

void send_screwbanall(char *nuh)
{
	CHAN_list *Channel;
	for (Channel=Channel_list; Channel; Channel = Channel->next)
		screw_ban(Channel->name, nuh, 1);
}

void update_idletime(char *channel, char *nuh)
{
	CHAN_list *Channel;
	USER_list *User;
	if ((Channel = search_chan(channel)) == NULL)
		return;
	if ((User = search_user(&(Channel->users), getnick(nuh))) == NULL)
		return;
	User->idletime = getthetime();
}

void update_flood(char *channel, char *nuh)
{
	CHAN_list *Channel;
	USER_list *User;
	if ((Channel = search_chan(channel)) == NULL)
		return;
	if ((User = search_user(&(Channel->users), getnick(nuh))) == NULL)
		return;
	if ((getthetime() - User->floodtime) > 5)
	{
		User->floodtime = getthetime();
		User->floodnum = 0;
	}
	User->floodnum++;
	if ((User->floodnum >= floodlimit) && floodtoggle)
	{
		if (protlevel(get_username(User)) <= 2)
			sendkick(channel, User->nick, "\x2 flooding is not allowed\x2");
	}
}

int num_on_channel(char *channel, char *uh)
{
  CHAN_list *Channel;
  USER_list *User;
  int num = 0;
  char n[100], u[100], h[100], m[100];

  sep_userhost(uh, n, u, h, m);
  strcpy(m, fixuser(m));
  if ((Channel = search_chan(channel)) == NULL)
    return 0;
  for (User=Channel->users;User;User=User->next)
    {
      if (!strcasecmp(fixuser(User->host), m))
	num++;
    }
  return num;
}

int tot_num_on_channel(char *channel)
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

void cycle_channel(char *channel)
{
  sendpart(channel);
  sendjoin(channel);
}

void idle_kick(void)
{
  CHAN_list *Channel;
  USER_list *User;
  time_t currenttime;
  char buffer[MAXLEN];
  
  currenttime = getthetime();
  sprintf(buffer, "\x2Idle for more than %i minutes\x2", idlelimit/60);
  
  for (Channel=Channel_list; Channel; Channel=Channel->next)
    for (User=Channel->users;User;User=User->next)
      {
	if ((currenttime - User->idletime) > idlelimit)
	  {
	    if (!protlevel(get_username(User)) && !userlevel(get_username(User)))
	      {
		sendkick(Channel->name, User->nick, buffer);
		User->idletime = User->idletime + 20;
	      }
	  }
      }
}

int  show_users_on_channel( char *from, char *channel )
{
	CHAN_list  *Dummy;
	USER_list  *Blah;
	char        modestr[MAXLEN];

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

void add_channelmode( char *channel, unsigned int mode, char *params )
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
		strcpy(Channel->key, params);
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

void del_channelmode( char *channel, unsigned int mode, char *params )
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

void change_usermode( char *channel, char *user, unsigned int mode )

{
	CHAN_list	*Channel;
	
	if( (Channel = search_chan( channel )) == NULL )
		return;		/* should not happen */

	add_mode( &(Channel->users), mode, user );
}
	
int  open_channel( char *channel )
{
	CHAN_list *Channel;

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

void channel_massop( char *channel, char *pattern )
{
	CHAN_list *Channel;
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
		   if ((*pattern && (!matches(pattern, get_username(Dummy))&&
			   !(get_usermode(Dummy)&MODE_CHANOP))) ||
			     (!*pattern && userlevel(get_username(Dummy))))
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

void channel_massdeop( char *channel, char *pattern )
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
		   if ((*pattern && (!matches(pattern, get_username(Dummy))&&
			(get_usermode(Dummy)&MODE_CHANOP)&&
			(userlevel(get_username(Dummy)) <= malevel))) ||
			  (!*pattern && !userlevel(get_username(Dummy))))
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
 
void channel_masskick( char *channel, char *pattern )
{
        CHAN_list       *Channel;
        USER_list       *Dummy;
	char buffer[MAXLEN];

	sprintf(buffer, "\x2Mass Kick of %s\x2", *pattern ? pattern : "non users");
        if( (Channel = search_chan( channel )) == NULL )
                return;

        Dummy = Channel->users;
        while(Dummy)
        {
             if ((*pattern && (!matches(pattern, get_username(Dummy))&&
		  (userlevel(get_username(Dummy)) <= malevel))) ||
		 (!*pattern && !userlevel(get_username(Dummy))))
		 sendkick(channel, get_usernick(Dummy), buffer);

                Dummy=get_nextuser(Dummy);
        }
}

void channel_masskickban( char *channel, char *pattern )
{
		  CHAN_list       *Channel;
		  USER_list       *Dummy;
	char buffer[MAXLEN];

	sprintf(buffer, "\x2Mass Kick-Ban of %s\x2", pattern);
		  if( (Channel = search_chan( channel )) == NULL )
					 return;

		  Dummy = Channel->users;
		  while(Dummy)
		    {
		      if(!matches(pattern, get_username(Dummy))&&
			 (userlevel(get_username(Dummy)) <= malevel))
			{
			  ban_user(channel, get_username(Dummy));
			  sendkick(channel, get_usernick(Dummy), buffer);
			}
		      Dummy=get_nextuser(Dummy);
		  }
}

/*
 * Some misc. function which deal with channels and users
 */

int	invite_to_channel( user, channel )
char	*user;
char	*channel;

{
	if(search_chan( channel ))
	{
		send_to_server("INVITE %s %s", user, channel);
		return(TRUE);
	}
	return(FALSE);
}

char	*username( nick )
char	*nick;

/*
 * Searches all lists for nick and if it finds it, returns 
 * nick!user@host
 */

{
	CHAN_list	*Channel;
	USER_list	*User;

	for(Channel = Channel_list; Channel; Channel = Channel->next)
		if(User = search_user(&(Channel->users), nick))
			return(get_username(User));
	return(NULL);
}

unsigned int	usermode( channel, nick )
char	*channel;
char	*nick;
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
		if(!strcasecmp(nick, get_usernick(Blah)))
			return(get_usermode(Blah));
		Blah=get_nextuser(Blah);
	}
	return(0);
}
	
/*
 * Here are the functions to bookkeep the banlist
 */

BAN_list	*search_ban( b_list, banstring )
BAN_list	**b_list;
char		*banstring;

{
	BAN_list	*Banned;

	for( Banned = *b_list; Banned; Banned = Banned->next )
		if(!strcasecmp(Banned->banstring, banstring))
			return(Banned);		
	return(NULL);
}

void	add_ban( b_list, banstring )
BAN_list	**b_list;
char		*banstring;

{
	BAN_list	*New_ban;

	if( (New_ban = (BAN_list*)malloc(sizeof(*New_ban))) == NULL)
		return;
	if( search_ban(b_list, banstring) != NULL )
		return;
	strcpy(New_ban->banstring, banstring );
	New_ban->next = *b_list;
	*b_list = New_ban;
}

int	delete_ban( b_list, banstring )
BAN_list	**b_list;
char		*banstring;
/*
 */

{
	BAN_list	**old;
	BAN_list	*Dummy;

	if( (Dummy = search_ban( b_list, banstring )) == NULL )
		return(FALSE);

	for( old = b_list; *old; old = &(*old)->next )
		if( *old == Dummy )
		{
			*old = Dummy->next;
			free( Dummy );
			return(TRUE);
		}
	return(FALSE);
}

void  clear_all_bans( b_list )
BAN_list        **b_list;
/*
 * Removes all entries in the list
 */
{
        BAN_list        **old;

        for( old = b_list; *old; )
        {
                *old = (*old)->next;
                free( *old );
        }
}

void channel_massunban( char *channel )
{
	CHAN_list  *Channel;
	BAN_list   *Dummy;
	char       unbanmode[5];
	char       unbanstring[MAXLEN];
	int        i;

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

void channel_unban( char *channel, char *user )
{
	CHAN_list  *Channel;
	BAN_list   *Dummy;
	char       unbanmode[5];
	char       unbanstring[MAXLEN];
	int        i;

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
			if(!matches(Dummy->banstring, user))
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




