/*
 * parse.c
 *
 * (c) 1993 VladDrac (irvdwijk@cs.vu.nl)
 */

#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>

#include "parse.h"
#include "vlad-ons.h"
#include "channel.h"
#include "misc.h"
#include "config.h"

extern	char	current_nick[MAXNICKLEN];

void 	parse_privmsg(from, line)
char	*from;
char  	*line;
/*
 * This function parses the "rest" when a PRIVMSG was sent by the
 * server. It filters out some stuff and calls the apropriate routine(s)
 * (on_ctcp / on_msg)
 */

{
    	char  	*text;
    	char  	*to;
    	int   	i;

#ifdef DBUG
  	printf( "Entered privmsg, from = %s, line = %s\n", from, line );
#endif

    	if( ( text = index( line, ' ' ) ) != 0 ) 
        	*( text ) = '\0'; text += 2;
  
    	to = line;
    	if( *text == ':' )
        	*( text++ ) = '\0';

/* This sometimes fails if the string is longer than MAXLEN bytes (the last 
   \x01 is gone. Not very interesting, because normal CTCP requests aren't 
   that long....    */
    	if( *text == '\x01' && *( text + strlen( text) - 1 ) == '\x01' )
    	{
        	*( text++ ) = '\0';
        	*( text + strlen( text ) - 1 ) = '\0';
        	on_ctcp( from, to, text ); 
    	}
    	else
        	on_msg( from, to, text );
}

void	parse_324( from, rest )
char	*from;
char	*rest;

{
	rest = strchr( rest, ' ' );
/*
 * Can we do this?
 * Yes we can.
 */
	on_mode(from, rest);
}

void	parse_352( line )
char	*line;

{
	char	*channel;
	char	*nick;
	char	*user;
	char	*host;
	char	*server;  /* currently unused */
	char	*mode;

	/* skip my own nick */
        line = strtok( line, " " );
	/* Is it a header? return.. */
        if(*line == 'C')
                return;
        channel = strtok( NULL, " " );
        user = strtok( NULL, " " );
	host = strtok( NULL, " " );
	server = strtok( NULL, " " );
	nick = strtok( NULL, " " );
	mode = strtok( NULL, " " );
	add_user_to_channel( channel, nick, user, host );
	while( *mode )
	{
		switch( *mode )
		{
		case 'H':
		case 'G':
		case '*':
			break;
		case '@':
			change_usermode( channel, nick, MODE_CHANOP );
			break;
		case '+':
			change_usermode( channel, nick, MODE_VOICE );
			break;
		default:
#ifdef DBUG
			printf("MODE: unknown mode %c\n", *mode );
#endif
			break;
		}
		mode++;
	}
}

void	parse_367( rest )
char	*rest;

{
	char	*nick;
	char	*channel;
	char	*banstring;

	nick = strtok(rest, " ");
	channel = strtok(NULL, " ");
	banstring = strtok(NULL, "\n\0");
	add_channelmode(channel, MODE_BAN, banstring);
}

void 	parseline( line )
char  	*line;

{
    	char  	*from; 
    	char  	*command;
    	char  	*rest;
    	char  	*channel;
	char	*nick;
    	int   	numeric;

#ifdef DBUG
    	printf("Entered parse, line = %s", line);
#endif 

    	if( *( line + strlen( line ) - 1 ) == '\n' )
            *( line + strlen( line ) - 1 ) = '\0';

/* The new servers put a \r before the \n */

    	if( *( line + strlen( line ) - 1 ) == '\r' )
            *( line + strlen( line ) - 1 ) = '\0';

    	if( *line == ':' ) 
    	{
         	if( ( command = index( line, ' ' ) ) == 0 )
            		return;
         	*( command++ ) = '\0';
         	from = line + 1; 
    	} 
    	else 
    	{
        	command = line;
        	from = NULL;
    	}
  
    	if( ( rest = index( command, ' ' ) ) == 0 )
        	return;
      
    	*( rest++ ) = '\0';
  
    	if( *rest == ':' )
        	*( rest++ ) = '\0';

    	if( numeric = atoi( command ) )  
    	{
		/* 324 is a modereply.. to set initial channelmode */
		if( numeric == 324 )
		{
			parse_324(from, rest);
			return;
		}
		/* 352 is a whoreply... add it to the channeluserlist */
		if( numeric == 352 )
		{
			parse_352( rest );
			return;
		}
		/* 367 is the Channel Ban list */
		if( numeric == 367 )
		{
			parse_367( rest );
			return;
		}
        	/* nickname errors */
        	if( numeric == 433 )  /* 431..436 */  
        	{
            		strcat( current_nick, "_" );
            		sendnick( current_nick );
        		return;	
		}      
        	/* All kinds of kicks/bans etc. */
        	if((numeric > 470) && (numeric < 477) && (numeric != 472))
        	{
            		rest = strchr( rest, '#' );
            		channel = strtok( rest, " " );
            		mark_failed( channel );
            		return;
        	}
    	}
    	if( strcmp( command, "NOTICE" ) == 0 )
		return;

    	else if( strcmp( command, "PRIVMSG" ) == 0 ) 
	{
            	parse_privmsg( from, rest );
		return;
	}
    	else if( strcmp( command, "JOIN" ) == 0 )
	{
		/* Should this be here, or in on_join? */
		/* This sux... I can't do who nick because it failes
                   when I'm on channel #b2 and #b3 (b3=active), and
                   someone joines #b2... Perhaps look use NAMES? 
		   (combined with whois or whatever) 
		   Nop... The info from join is sufficient, we know
		   nick, user, host, and the modes are 0 :) */
		/* Btw... this is ugly, but first let's see if it works */
		{
			char	from_copy[MAXLEN];
			char	*n, *u, *h;
			
			strcpy( from_copy, from );
			n=strtok( from_copy, "!");
			u=strtok(NULL, "@");
			h=strtok(NULL, "\n\0");
			add_user_to_channel( rest, n, u, h );
		}
		/* if it's me I succesfully joined a channel! */
		if( !strcasecmp( current_nick, from ) )
			mark_success( rest );
            	on_join( from, rest );
		return;
	}
	else if( strcmp( command, "PART" ) == 0 )
	{
		/* again, put this is on_leave? */
		remove_user_from_channel( rest, getnick( from ) );
		return;
	}
	else if( strcmp( command, "QUIT" ) == 0 )
	{
		remove_user( getnick( from ) );
		return;
	}			
	else if( strcmp( command, "NICK" ) == 0 )
	{
		change_nick( getnick( from ), rest );
		return;
	}
    	else if( strcmp( command, "MODE" ) == 0 )
	{
		on_mode( from, rest );
		return;
	}
    	else if( strcmp( command, "KICK" ) == 0 )
        {
            	channel = strtok( rest, " " );
		nick = strtok( NULL, " " );
		if( !strcasecmp( current_nick, nick ) )
            		mark_failed( channel );
		else
			remove_user_from_channel( channel, nick );
            	return;
        }   
    	else if( strcmp( command, "PING" ) == 0 ) 
	{
		char	localhost[64];
		gethostname(localhost, 64);
            	sendping( localhost );
		return;
	}
    	else if( strcmp( command, "ERROR" ) == 0 )
	{
		botlog( ERRFILE, "SIGNING OFF: %s", rest );
		botlog( ERRFILE, "           - trying to reconnect" );
		reconnect_to_server();
	}
}

void	parse_server_input( read_fds )
fd_set	*read_fds;
/*
 * If read_fds is ready, this routine will read a line from the 
 * the server and parse it.
 */
{
	char	line[MAXLEN];

/*	if( FD_ISSET( server_sock, read_fds ) ) */
	if( serversock_set( read_fds ) )
		if( readln( line ) > 0 )
			parseline( line );
		else
		{
#ifdef DBUG
			printf("Server read FAILED!\n");
#endif
			reconnect_to_server();
		}
}
