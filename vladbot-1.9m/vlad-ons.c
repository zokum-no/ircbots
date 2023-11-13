/*
 * vlad-ons.c
 * Contains: on msg, on join
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>

#include "config.h"
#include "channel.h"
#include "userlist.h"
#include "dcc.h"
#include "ftext.h"
#include "misc.h"
#include "vlad-ons.h"

extern	char    current_nick[MAXLEN];
extern	long	uptime;
extern  USERLVL_list	**opperlist;
extern	USERLVL_list	**shitlist;
extern	USERLVL_list	**protlist;

struct
{
	char	*name;
	void	(*function)();
	int	userlevel;
	int	shitlevel;
	int	forcedcc;
} on_msg_commands[] =

/* Any command which produces more than 2 lines output can be 
   used to make the bot "excess flood". Make it "forcedcc"     */

/*	Command		function   userlvl   shitlvl  forcedcc */
{
	{ "!HELP",	show_help,	0,	100,	TRUE  }, 
	{ "!WHOAMI",	show_whoami,  	0,	100,	TRUE  },
	{ "!INFO",	show_info,	0,	100,	TRUE  },
	{ "!TIME",	show_time,	0,	100,	FALSE },	
	{ "!USERLIST",	show_userlist,	0,	100,	TRUE  },	
	{ "!SHITLIST",	show_shitlist,	0,	100,	TRUE  },
	{ "!PROTLIST",	show_protlist,	0,	100,	TRUE  },
	{ "!LISTDCC",	do_listdcc,	0,	100,	TRUE  },
	{ "!CHAT",	do_chat,	0,	100,	FALSE },
	{ "!SEND",	do_send,	0,	100,	FALSE },
	{ "!FILES",	do_flist,       0,      100,    TRUE  },
	{ "!FLIST",	do_flist,	0,	100,	TRUE  },
	{ "!OP",	do_op, 		50,	0,	FALSE },
	{ "!GIVEOP",	do_giveop,	50,	0,	FALSE },
	{ "!INVITE",	do_invite,	50,	0,	FALSE },
	{ "!OPEN",	do_open,	100,	0,	FALSE },
	{ "!SAY",	do_say,		100,	0,	FALSE },
	{ "!JOIN",	do_join,	100,	0,	FALSE },
	{ "!LEAVE",	do_leave,	100,	0,	FALSE },
	{ "!NICK",	do_nick,	100,	0,	FALSE },
	{ "!WHOIS",	show_whois,	100,	0,	TRUE  },
	{ "!NWHOIS",	show_nwhois,	100,	0,	TRUE  },
	{ "!NUSERADD",	do_nuseradd,	100,	0,	FALSE },
	{ "!USERWRITE",	do_userwrite,	100,	0,	FALSE },
	{ "!USERDEL",	do_userdel,	100,	0,	FALSE },
	{ "!NSHITADD",	do_nshitadd,	100,	0,	FALSE },
	{ "!SHITWRITE",	do_shitwrite,	100,	0,	FALSE },
	{ "!SHITDEL",	do_shitdel,	100,	0,	FALSE },
        { "!NPROTADD",  do_nprotadd,    100,    0,	FALSE },
        { "!PROTWRITE", do_protwrite,   100,    0,	FALSE },
        { "!PROTDEL",   do_protdel,     100,    0,	FALSE },
	{ "!BANUSER",	do_banuser,	100,	0,	FALSE },
	{ "!UNBAN",	do_unban,	100,	0,	FALSE },
	{ "!USERADD",	do_useradd,	125,	0,	FALSE },
	{ "!SHITADD",	do_shitadd,	125,	0,	FALSE },
        { "!PROTADD",   do_protadd,     125,    0,	FALSE },
	{ "!CHANNELS",	show_channels,	125,	0,	TRUE  },
	{ "!SHOWUSERS",	do_showusers,	125,	0,	TRUE  },
	{ "!MASSOP",	do_massop,	125,	0,	FALSE },
	{ "!MASSDEOP",	do_massdeop,	125,	0,	FALSE },
	{ "!MASSKICK",	do_masskick,	125,	0,	FALSE },
	{ "!MASSUNBAN",	do_massunban,	125,	0,	FALSE },
	{ "!DO",	do_do,		150,	0,	FALSE },
	{ "!DIEDIE",	do_die,		150,	0,	FALSE },
	/*
	 :
	 :
	 */
	{ NULL,		null(void(*)()), 0,	0,	FALSE }
};

void 	on_join( who, channel )
char    *who;
char    *channel;

{
    	if( shitlevel( who ) == 100 )
    	{
		ban_user( who );
        	sendkick( channel, getnick( who ), "User has shitlevel 100" );
        	return;
    	}
    	if( userlevel( who ) >= 50 && shitlevel( who ) == 0 )
        	giveop( channel, getnick( who ) );
}

void	on_mode( from, rest )
char	*from;
char	*rest;

{
	int	did_change=FALSE;	/* If we have to change anything */
	char	*channel;
	char	*mode;

	char	*chanchars;	/* i.e. +oobli */
	char	*params;
	char	sign;		/* +/-, i.e. +o, -b etc */

	char	unminmode[MAXLEN];	/* used to undo modechanges */
	char	unminparams[MAXLEN];
	char	unplusmode[MAXLEN];	/* used to undo modechanges */
	char	unplusparams[MAXLEN];

	/* these are here merely to make things clearer */
	char	*nick;
	char	*banstring;
	char	*key;
	char	*limit;

#ifdef DBUG
	printf("Entered mode, from = %s, rest = %s\n", from, rest);
#endif

	strcpy(unminmode, "");		/* min-modes, i.e. -bi */
	strcpy(unminparams, "");	/* it's parameters */
	strcpy(unplusmode, "");		/* plus-modes, i.e. +oo */
	strcpy(unplusparams, "");	/* parameters */

	channel = strtok(rest, " ");
	if(!strcasecmp(channel, current_nick))	/* if target is me... */
		return;			/* mode NederServ +i etc */

	mode = strtok(NULL, "\n\0");

	params = strchr(mode, ' ')+1;
	chanchars = strtok(mode, " ");

	while( *chanchars )
	{
		switch( *chanchars )
		{
		case '+':
			sign = '+';
			break;
		case '-':
			sign = '-';
			break;
		case 'o':
			nick = strtok( params, " " );
			params = strtok(NULL, "\0\n");
			if( sign == '+' )
			{
				add_channelmode( channel, CHFL_CHANOP, nick );
				if(shitlevel(username(nick))>0)
				{
					strcat(unminmode, "o");
					strcat(unminparams, nick);
					strcat(unminparams, " ");
					send_to_user(from, 
						"Sorry, but %s is shitted!",
						     nick);
					did_change=TRUE;
				}
			}
			else
			{
				del_channelmode( channel, CHFL_CHANOP, nick );
				if((protlevel(username(nick))>=100)&&
				   (shitlevel(username(nick))==0))
				{
					strcat(unplusmode, "o");
					strcat(unplusparams, nick);
					strcat(unplusparams, " ");
					send_to_user(from, 
						"Sorry, but %s is protected :)",
						     nick);
					did_change=TRUE;
				}	
			}
			break;
		case 'v':
                        nick = strtok( params, " " );
                        params = strtok(NULL, "\0\n"); 
			if( sign == '+' )
				add_channelmode( channel, CHFL_VOICE, nick );
			else
				del_channelmode( channel, CHFL_VOICE, nick );
			break;
		case 'b':
                        banstring = strtok( params, " " );
                        params = strtok(NULL, "\0\n"); 
                        if( sign == '+' )
                                add_channelmode(channel, MODE_BAN, banstring);
                        else
                                del_channelmode(channel, MODE_BAN, banstring);
                        break;
		case 'p':
			if( sign == '+' )
				add_channelmode(channel, MODE_PRIVATE, "");
			else
				del_channelmode(channel, MODE_PRIVATE, "");
			break;
		case 's':
			if( sign == '+' )
				add_channelmode(channel, MODE_SECRET, "");
			else
				del_channelmode(channel, MODE_SECRET, "");
			break;
		case 'm':
			if( sign == '+' )
				add_channelmode(channel, MODE_MODERATED, "");
			else
				del_channelmode(channel, MODE_MODERATED, "");
			break;
		case 't':
			if( sign == '+' )
				add_channelmode(channel, MODE_TOPICLIMIT, "");
			else
				del_channelmode(channel, MODE_TOPICLIMIT, "");
			break;
		case 'i':
			if( sign == '+' )
				add_channelmode(channel, MODE_INVITEONLY, "");
			else
				del_channelmode(channel, MODE_INVITEONLY, "");
			break;
		case 'n':
			if( sign == '+' )
				add_channelmode(channel, MODE_NOPRIVMSGS, "");
			else
				del_channelmode(channel, MODE_NOPRIVMSGS, "");
			break;
		case 'k':
			if( sign == '+' )
			{
                        	key = strtok( params, " " );
                        	params = strtok(NULL, "\0\n"); 
				add_channelmode(channel, MODE_KEY, 
						key?key:"???");
			}
			else
				del_channelmode(channel, MODE_KEY, "");
			break;
		case 'l':
			if( sign == '+' )
			{
                        	limit = strtok( params, " " );
                        	params = strtok(NULL, "\0\n"); 
				add_channelmode(channel, MODE_LIMIT, limit);
			}
			else
				del_channelmode(channel, MODE_LIMIT, "");
			break;
		default:
#ifdef DBUG
			printf("MODE: unknown mode %c", *chanchars);
#endif
			break;
		}
		chanchars++;
	}
	/* restore unwanted modechanges */
	if(did_change)
		sendmode( channel, "+%s-%s %s %s", unplusmode, 
			  unminmode, unplusparams, unminparams);
}

void	on_msg( from, to, msg )
char	*from;
char	*to;
char	*msg;

{
	int	i;
	char	*command;
	char	*rest;
	DCC_list	*userclient;


	for( ; *msg && *msg == ' '; msg++ );
	if( ( rest = strpbrk( msg, SEPERATORS ) ) != NULL )
		rest++;

	if( ( command = strtok( msg, SEPERATORS ) ) == NULL )
		return;			/* NULL-command */

	if( !strcasecmp( current_nick, command ) && rest != NULL )
	{
		for( ;*rest == ' ' && *rest; rest++ );
		if( ( rest = strpbrk( rest, SEPERATORS ) ) != NULL )
			rest++;
		if( ( command = strtok( NULL, SEPERATORS ) ) == NULL )
			return;		/* NULL-command */
	}

	for( i = 0; on_msg_commands[i].name != NULL; i++ )
	/* Look is the message is private. If so, commands without a leading
	   '!' are allowed. If a match is found, look if the users levels
	   are ok. Check if the command requires DCC, and if so, check if
	   the users already has a connection. Wow.. so much in so little
	   code :) 							   */

		if( ( !strcasecmp( on_msg_commands[i].name, command ) ||
		    ( !strcasecmp( on_msg_commands[i].name+1, command ) &&
		      !strcasecmp( to, current_nick ) ) ) )
			if( ( userlevel( from ) >= 
                              on_msg_commands[i].userlevel ) &&
		            ( shitlevel( from ) <= 
			      on_msg_commands[i].shitlevel ) )
			{
				userclient = 
				search_list( "chat", from, DCC_CHAT, 1 );
				if( on_msg_commands[i].forcedcc && 
			    	  (!userclient || (userclient->flags&DCC_WAIT)))
				{
#ifdef AUTO_DCC
					dcc_chat( from, rest );
					sendnotice( getnick(from), 
					"Please type: /dcc chat %s", 
					current_nick );
                                	sendnotice( getnick(from),
                                	"After that, please retype: /msg %s %s %s",
                                	current_nick, msg, rest ? rest : "" );
#else
					sendnotice( getnick(from), 
					"Sorry, %s is only available through DCC",
					on_msg_commands[i].name );
                			sendnotice( getnick(from), 
					"Please start a dcc chat connection first" );
#endif /* AUTO_DCC */
                			return;
				}
				on_msg_commands[i].function( from, to, rest );
				return;
			}
			else
			{
				send_to_user( from, "Sorry, incorrect levels" );
				return;
			}
	if( !strcasecmp( to, current_nick ) )
		send_to_user( from, "%s %s?!? What's that?",
			      command, rest ? rest : ""  );
}

void	show_help( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
	int 	i, j;
	FILE	*f;
	char	*s;

	if((f=fopen(HELPFILE, "r"))==NULL)
	{
		send_to_user(from, "Helpfile missing");
		return;
	}
	
	if( rest == NULL )
	{
		find_topic(f, "standard");
		while(s=get_ftext(f))
			send_to_user(from, s);
	}
	else
		if(!find_topic( f, rest ))
			send_to_user(from, "No help available for \"%s\"", rest);
		else
			while(s=get_ftext(f))
				send_to_user(from, s);
	fclose(f);
}

void	show_whoami( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
        send_to_user( from, "You are %s. Your levels are:", 
		      from );
	send_to_user( from, "-- User -+- Shit -+- Protect --" );
	send_to_user( from, "    %3d  |   %3d  |      %3d", userlevel( from ),
		      shitlevel( from ), protlevel( from ) );
        return;
}

void	show_info( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
	send_to_user( from, "I am VladBot version %s (%s)", 
		      VERSION, current_nick );
	send_to_user( from, "Started: %-20.20s", time2str(uptime) );
	send_to_user( from, "Up: %s", idle2str(time(NULL)-uptime));
        return;
}

void	show_time( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
	send_to_user( from, "Current time: %s", time2str(time(NULL)));
}

void	show_userlist( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
	send_to_user( from, " userlist: %30s   %s", "nick!user@host", "level" );
        send_to_user( from, " -----------------------------------------+------" );
	show_lvllist( opperlist, from, rest?rest:"" );
}

void	show_shitlist( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
	send_to_user( from, " shitlist: %30s   %s", "nick!user@host", "level" );
        send_to_user( from, " -----------------------------------------+------" );
	show_lvllist( shitlist, from, rest?rest:"" );
}

void    show_protlist( from, to, rest )
char    *from;
char    *to;
char    *rest;

{
        send_to_user( from, " protlist: %30s   %s", "nick!user@host", "level" );
        send_to_user( from, " -----------------------------------------+------" );
	show_lvllist( protlist, from, rest?rest:"" );
}

void	do_op( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
	if(usermode(strcasecmp(to, current_nick) ? to : currentchannel(),  
	   getnick(from))&MODE_CHANOP)
		send_to_user(from, "You're already channel operator!");
	else
        	giveop(strcasecmp(to, current_nick) ? to : currentchannel(),  
		       getnick( from ) );
}

void	do_giveop( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
	if( rest != NULL )
            	sendmode( to, "+ooo %s", rest );
        else
            	send_to_user( from, "Who do you want me to op?" );
        return;
}

void	do_invite( from, to, rest )
char    *from;
char    *to;
char    *rest;

{
        if( rest != NULL )
	{
		if(!invite_to_channel(from, rest))	
			send_to_user(from, "I'm not on channel %s", rest);
	}
	else
		invite_to_channel(from, currentchannel());
}
			
void	do_open( from, to, rest )
char    *from;
char    *to;
char    *rest;

{
        if( rest != NULL )
        {
		if(!open_channel(rest))
			send_to_user(from, "I could not open %s!", rest);
		else
			channel_unban(rest, from);
	}
	else
	{
		open_channel(currentchannel());
		channel_unban(currentchannel(), from);
	}
}

void    do_chat( from, to, rest )
char    *from;
char    *to;
char    *rest;
 
{
	dcc_chat( from, rest );
	sendnotice( getnick(from), "Please type: /dcc chat %s", current_nick );
}

void    do_send( from, to, rest )
char    *from;
char    *to;
char    *rest;
 
{
	char	buf[MAXLEN];

	if( rest != NULL )
		dcc_sendfile( from, rest );
	else
		send_to_user( from, "Please specify a filename (use !files)" );
}

void	do_flist( from, to, rest )
char    *from;
char    *to;
char    *rest;
 
{
	FILE	*ls_file;
	char	indexfile[MAXLEN];
	char	*s;
	char	*p;

	strcpy(indexfile, CONTENTSFILE );
	if( rest )
	{
		for(p=rest; *p; p++) *p=tolower(*p);
		sprintf(indexfile, "%s.%s", CONTENTSFILE, rest);
		if( ( ls_file = fopen( indexfile, "r" ) ) == NULL )
		{
			send_to_user(from, "No indexfile for %s", rest);
			return;
		}
	}
	else
		if( ( ls_file = fopen( CONTENTSFILE, "r" ) ) == NULL )
		{
			send_to_user(from, "No indexfile");
			return;
		}

	while(s=get_ftext(ls_file))
		send_to_user(from, s);
	fclose( ls_file );
}	

void	do_say( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
	if( rest != NULL )
            	if( ischannel( to ) )
                	sendprivmsg( to, "%s", rest );
            	else
                	sendprivmsg( currentchannel(), "%s", rest );
        else
            	send_to_user( from, "I don't know what to say" );
        return;
}

void	do_do( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
	if( rest != NULL )
            send_to_server( rest );
        else
            send_to_user( from, "What do you want me to do?" );
        return;
}

void	show_channels( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
        show_channellist( from );
        return;
}


void	do_join( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
	if( rest != NULL )
            join_channel( rest );
        else
            send_to_user( from, "What channel do you want me to join?" );
        return;
}

void	do_leave( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
	if( rest != NULL )
            	leave_channel( rest ); 
        else
		if( !strcasecmp( current_nick, to ) )
			leave_channel( currentchannel() );
		else
			leave_channel( to );
        return;
}
 
void	do_nick( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
	if( rest != NULL )
        {
            	sendnick( rest );
            	strcpy( current_nick, rest );
        }
        else
            	send_to_user( from, "You need to tell me what nick to use" );
        return;
}

void	do_die( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
	if( rest != NULL )
            	signoff( from, rest );
        else
            	signoff( from, "Bye bye!" );
}

void    show_whois( from, to, rest )
char    *from;
char    *to;
char    *rest;
 
{
	if( rest == NULL )
	{
		send_to_user( "Please specify a user" );
		return;
	} 
 
        send_to_user( from, "%s's levels are:",
                      rest );
        send_to_user( from, "-- User -+- Shit -+- Protect --" );
        send_to_user( from, "    %3d  |   %3d  |      %3d", userlevel( rest ),
                      shitlevel( rest ), protlevel( rest ) );
        return;
}

void	show_nwhois( from, to, rest )
char    *from;
char    *to;
char    *rest;

{
	char	*nuh;

        if( rest == NULL )
        {
                send_to_user( from, "Please specify a nickname" );
                return;
        }
        if((nuh=username(rest))==NULL)
        {
                send_to_user(from, "%s is not on this channel!", rest);
                return;
        }

        send_to_user(from, "USERLIST:-------------Matching pattern(s) + level");
	show_lvllist( opperlist, from, nuh);
	send_to_user(from, "SHITLIST:---------------------------------+");
        show_lvllist( shitlist, from, nuh);
	send_to_user(from, "PROTLIST:---------------------------------+");
        show_lvllist( protlist, from, nuh);
	send_to_user(from, "End of nwhois-----------------------------+");
}

void	do_nuseradd( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
	char	*newuser;
	char	*newlevel;
	char	*nuh;		/* nick!@user@host */

        if( ( newuser = strtok( rest, " " ) ) == NULL )
	{
             	send_to_user( from, "Who do you want me to add?" );
		return;
	}
	if((nuh=username(newuser))==NULL)
	{
		send_to_user(from, "%s is not on this channel!", newuser);
		return;
	}
        if( ( newlevel = strtok( NULL, " " ) ) == NULL )
	{
                send_to_user( from, "What level should %s have?", newuser );
		return;
	}


        if( atoi( newlevel ) < 0 )
		send_to_user( from, "level should be >= 0!" );
	else if( atoi( newlevel ) > userlevel( from ) )
		send_to_user( from, "Sorry bro, can't do that!" );
	else if( userlevel( from ) < userlevel( nuh ) )
		send_to_user( from, "Sorry, %s has a higher level", newuser );
	else
	{
		char	*nick;
		char	*user;
		char	*host;
		char	*domain;
		char	userstr[MAXLEN];

		nick=strtok(nuh, "!");
		user=strtok(NULL,"@");
		if(*user == '~' || *user == '#') user++;
		host=strtok(NULL, ".");
		domain=strtok(NULL,"\0\n");

		sprintf(userstr, "*!*%s@*%s", user, domain?domain:host);
               	send_to_user( from, "User %s added with access %d as %s", 
			      newuser, atoi( newlevel ), userstr );
               	add_to_levellist( opperlist, userstr, atoi( newlevel ) );
	}
        return;
}                   


void	do_useradd( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
	char	*newuser;
	char	*newlevel;

        if( ( newuser = strtok( rest, " " ) ) == NULL )
	{
             	send_to_user( from, "Who do you want me to add?" );
		return;
	}
        if( ( newlevel = strtok( NULL, " " ) ) == NULL )
	{
                send_to_user( from, "What level should %s have?", newuser );
		return;
	}
        if( atoi( newlevel ) < 0 )
		send_to_user( from, "level should be >= 0!" );
	else if( atoi( newlevel ) > userlevel( from ) )
		send_to_user( from, "Sorry bro, can't do that!" );
	else if( userlevel( from ) < userlevel( newuser ) )
		send_to_user( from, "Sorry, %s has a higher level", newuser );
	else
	{
               	send_to_user( from, "User %s added with access %d", 
			      newuser, atoi( newlevel ) );
               	add_to_levellist( opperlist, newuser, atoi( newlevel ) );
	}
        return;
}                   

void    do_userwrite( from, to, rest )
char    *from;
char    *to;
char    *rest;
 
{
        if( !write_lvllist( opperlist, USERFILE ) )
                send_to_user( from, "Userlist could not be written to file %s", USERFILE );
        else
                send_to_user( from, "Userlist written to file %s", USERFILE );
}

void	do_userdel( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
	char	*lameuser;

        if( ( lameuser = strtok( rest, " " ) ) == NULL )
            send_to_user( from, "Who do you want me to delete?" );
        else
        {
            	if( !remove_from_levellist( opperlist, lameuser ) )
                	send_to_user( from, "%s has no level!", lameuser );
            	else if( userlevel( from ) < userlevel( lameuser ) )
			send_to_user( from, "%s has a higer level.. sorry", 
				      lameuser );
	    	else
                	send_to_user( from, "User %s has been deleted", 
				      lameuser );
        }
        return;
}

void	do_nshitadd( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
	char	*newuser;
	char	*newlevel;
	char	*nuh;		/* nick!@user@host */

        if( ( newuser = strtok( rest, " " ) ) == NULL )
	{
             	send_to_user( from, "Who do you want me to add?" );
		return;
	}
	if((nuh=username(newuser))==NULL)
	{
		send_to_user(from, "%s is not on this channel!", newuser);
		return;
	}
        if( ( newlevel = strtok( NULL, " " ) ) == NULL )
	{
                send_to_user( from, "What level should %s have?", newuser );
		return;
	}


        if( atoi( newlevel ) < 0 )
		send_to_user( from, "level should be >= 0!" );
	else if( atoi( newlevel ) > userlevel( from ) )
		send_to_user( from, "Sorry bro, can't do that!" );
	else if( userlevel( from ) < userlevel( nuh ) )
		/* This way, someone with level 100 can't shit someone with level 150 */
		send_to_user( from, "Sorry, %s has a higher level", newuser );
	else
	{
		char	*nick;
		char	*user;
		char	*host;
		char	*domain;
		char	userstr[MAXLEN];

		nick=strtok(nuh, "!");
		user=strtok(NULL,"@");
		if(*user == '~' || *user == '#') user++;
		host=strtok(NULL, ".");
		domain=strtok(NULL,"\0\n");

		sprintf(userstr, "*!*%s@*%s", user, domain?domain:host);
               	send_to_user( from, "User %s shitted with access %d as %s", 
			      newuser, atoi( newlevel ), userstr );
               	add_to_levellist( shitlist, userstr, atoi( newlevel ) );
	}
        return;
}                   

void	do_shitadd( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
	char	*newuser;
	char	*newlevel;

        if( ( newuser = strtok( rest, " " ) ) == NULL )
	{
             	send_to_user( from, "Who do you want me to add?" );
		return;
	}
        if( ( newlevel = strtok( NULL, " " ) ) == NULL )
	{
                send_to_user( from, "What level should %s have?", newuser );
		return;
	}
        if( atoi( newlevel ) < 0 )
                send_to_user( from, "level should be >= 0!" );
        else  
        {
                send_to_user( from, "User %s shitted with access %d", newuser, 
		              atoi( newlevel ) );
                add_to_levellist( shitlist, newuser, atoi( newlevel ) );
        } 
        return;
}                   

void	do_shitwrite( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
	if( !write_lvllist( shitlist, SHITFILE ) )
		send_to_user( from, "Shitlist could not be written to file %s", SHITFILE );
	else
		send_to_user( from, "Shitlist written to file %s", SHITFILE );
}

void	do_shitdel( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
	char	*lameuser;

        if( ( lameuser = strtok( rest, " " ) ) == NULL )
            send_to_user( from, "Who do you want me to delete?" );
        else
        {
            	if( !remove_from_levellist( shitlist, lameuser ) )
	      		send_to_user( from, "%s is not shitted!", lameuser );
            	else
                	send_to_user( from, "User %s has been deleted", lameuser );
        }
        return;
}

void	do_nprotadd( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
	char	*newuser;
	char	*newlevel;
	char	*nuh;		/* nick!@user@host */

        if( ( newuser = strtok( rest, " " ) ) == NULL )
	{
             	send_to_user( from, "Who do you want me to add?" );
		return;
	}
	if((nuh=username(newuser))==NULL)
	{
		send_to_user(from, "%s is not on this channel!", newuser);
		return;
	}
        if( ( newlevel = strtok( NULL, " " ) ) == NULL )
	{
                send_to_user( from, "What level should %s have?", newuser );
		return;
	}


        if( atoi( newlevel ) < 0 )
		send_to_user( from, "level should be >= 0!" );
	else
	{
		char	*nick;
		char	*user;
		char	*host;
		char	*domain;
		char	userstr[MAXLEN];

		nick=strtok(nuh, "!");
		user=strtok(NULL,"@");
		if(*user == '~' || *user == '#') user++;
		host=strtok(NULL, ".");
		domain=strtok(NULL,"\0\n");

		sprintf(userstr, "*!*%s@*%s", user, domain?domain:host);
               	send_to_user( from, "User %s protected with access %d as %s", 
			      newuser, atoi( newlevel ), userstr );
               	add_to_levellist( protlist, userstr, atoi( newlevel ) );
	}
        return;
}                   

void    do_protadd( from, to, rest )
char    *from;
char    *to;
char    *rest;

{
        char    *newuser;
        char    *newlevel;

        if( ( newuser = strtok( rest, " " ) ) == NULL )
	{
             	send_to_user( from, "Who do you want me to add?" );
		return;
	}
        if( ( newlevel = strtok( NULL, " " ) ) == NULL )
	{
                send_to_user( from, "What level should %s have?", newuser );
		return;
	}
        if( atoi( newlevel ) < 0 )
                send_to_user( from, "level should be >= 0!" );
        else  
        {
                send_to_user( from, "User %s protected with access %d", newuser,
                              atoi( newlevel ) );
                add_to_levellist( protlist, newuser, atoi( newlevel ) );
        }
        return;
}

void    do_protwrite( from, to, rest )
char    *from;
char    *to;
char    *rest;

{
        if( !write_lvllist( protlist, PROTFILE ) )
                send_to_user( from, "Protlist could not be written to file %s", PROTFILE );
        else
                send_to_user( from, "Protlist written to file %s", PROTFILE );
}

void    do_protdel( from, to, rest )
char    *from;
char    *to;
char    *rest;

{
        char    *lameuser;

        if( ( lameuser = strtok( rest, " " ) ) == NULL )
            send_to_user( from, "Who do you want me to delete?" );
        else
        {
            	if( !remove_from_levellist( protlist, lameuser ) )
           		send_to_user( from, "%s is not protected!", lameuser );
            	else
                	send_to_user( from, "User %s has been deleted", lameuser );
        }
        return;
}

void	do_banuser( from, to, rest )
char    *from;
char    *to;
char    *rest;

{
	char	*user_2b_banned;

	if( ( user_2b_banned = strtok( rest, SEPERATORS ) ) != NULL )
	{
/*		if( !ban_user( getnick( user_2b_banned ) ) )
			send_to_user( from, "Huh? who? what? %s?!?!", user_2b_banned );	
*/	}
	else
		send_to_user( from, "No." );
}

void	do_showusers( from, to, rest )
char    *from;
char    *to;
char    *rest;

{
        char    *channel;

	if( (channel = strtok( rest, " " )) == NULL )
		show_users_on_channel( from, currentchannel());
	else
		show_users_on_channel( from, channel );
}

void	do_massop( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
	char	*op_pattern;

        if( ( op_pattern = strtok( rest, " " ) ) == NULL )
             send_to_user( from, "Please specify a pattern" );
        else
            channel_massop( ischannel( to ) ? to : currentchannel(), 
			    op_pattern );    
        return;
}

void    do_massdeop( from, to, rest )
char    *from;
char    *to;
char    *rest;
 
{
        char    *op_pattern;
 
        if( ( op_pattern = strtok( rest, " " ) ) == NULL )
            send_to_user( from, "Please specify a pattern" );
        else
            channel_massdeop( ischannel( to ) ? to : currentchannel(), 
			      op_pattern );
        return;
}

void	do_masskick( from, to, rest )
char    *from;
char    *to;
char    *rest;

{
        char    *op_pattern;

        if( ( op_pattern = strtok( rest, " " ) ) == NULL )
            send_to_user( from, "Please specify a pattern" );
        else
            channel_masskick( ischannel( to ) ? to : currentchannel(), 
			      op_pattern );
        return;
}

void	do_massunban( from, to, rest )
char    *from;
char    *to;
char    *rest;

{
	char	*channel;

	if((channel = strtok(rest, " "))==NULL)
		channel_massunban(currentchannel());
	else
		channel_massunban(channel);
}

void	do_unban( from, to, rest )
char    *from;
char    *to;
char    *rest;

{
	        char    *channel;

        if((channel = strtok(rest, " "))==NULL)
                channel_unban(currentchannel(), from);
	else
		channel_unban(channel, from);
}

void	do_listdcc( from, to, rest )
char    *from;
char    *to;
char    *rest;

{
	show_dcclist( from );
}

void 	giveop( channel, nicks )
char    *channel;
char    *nicks;

{
    	sendmode( channel, "+ooo %s", nicks );
}

int 	userlevel( from )
char      *from;

{
	return( get_level( opperlist, from ) );
}

int 	shitlevel( from )
char	*from;

{
	return( get_level( shitlist, from ) );
}

int     protlevel( from )
char    *from;

{
	return( get_level( protlist, from ) );
}

void	ban_user( who )
char	*who;
/*
 * this functions bans the user with nick 'nick'.
 * the user to be banned should be online, else
 * no userhost information is available 
 * USE: kick with shitlevel 100, possible kill etc.
 *
 * user nick!user@host.domain will be banned as:
 *      nick!*@*
 *      *!user@*
 *      nick!*user@host.domain
 */
{
	char	userban[MAXLEN];
	char	*nick;
	char	*user;
	char	*host;
	char	*domain;
	
	strcpy( userban, who );
	nick = strtok( userban, "!" );
	user = strtok( NULL, "@" );
	if(*user == '~' || *user == '#') user++;
	host = strtok( NULL, "." );	
	domain = strtok( NULL, "\0\n" );

	sendmode( currentchannel(), " +bbb *!*%s@* %s!*@*%s %s!*%s@%s ", 
	  	  user, nick, domain?domain:host, nick, user, host );
}

void	signoff( from, reason )
char	*from;
char	*reason;

{
	send_to_user( from, "Saving lists..." );
        if( !write_lvllist( opperlist, USERFILE ) )
		send_to_user( from, "Could not save opperlist" );
        if( !write_lvllist( shitlist, SHITFILE ) )
		send_to_user( from, "Could not save shitlist" );
        if( !write_lvllist( protlist, PROTFILE ) )
		send_to_user( from, "Could not save protlist" );

	sendquit( reason );
	exit(0);
}
