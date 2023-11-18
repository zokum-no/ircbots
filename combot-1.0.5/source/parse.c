#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>

#include "parse.h"
#include "commands.h"
#include "channel.h"
#include "chanuser.h"
#include "misc.h"
#include "config.h"
#include "userlist.h"

extern long uptime, kinittime;
extern ListStruct *StealList;
extern char current_nick[MAXNICKLEN];
char oldnick[MAXNICKLEN] = "";
extern char errorfile[255];
extern char ownernick[255], shitfile[FNLEN], kinitfile[FNLEN];
extern char owneruhost[255], userhostfile[FNLEN], usersfile[FNLEN];
extern int holdnick;
int holdingnick = 0;
extern int idletoggle;
extern char nhmsg[255];
extern char lastcmd[1024];
char frombuf[MAXLEN], tobuf[MAXLEN], restbuf[1024];
char shitfuck[MAXLEN] = "bleaht33h33bleah";

void parse_privmsg( char *from, char *line)
{
	char  *text;
	char  *to;
	int   i;

#ifdef DBUG
	 printf( "Entered privmsg, from = %s, line = %s\n", from, line );
#endif

      if (!from || !*from)
	return;
/*
      if( ( text = index( line, ' ' ) ) != 0 )
*/
      if (text = strchr(line, ' '))
      {
           *( text ) = '\0';
           text += 2;
      }
      else
         text = "";
                                                      
      to = line;
		if ( *text == ':' )
			*( text++ ) = '\0';
		if( *text == '\x01' && *( text + strlen( text) - 1 ) == '\x01' )
		{
			*( text++ ) = '\0';
			*( text + strlen( text ) - 1 ) = '\0';
			strcpy(frombuf, from);
			strcpy(tobuf, to);
			strcpy(restbuf, text);
			if (is_flooding(frombuf))
			  return;
			on_ctcp( frombuf, tobuf, restbuf ); 
			strcpy(restbuf, text);
			on_msg(frombuf, tobuf, restbuf);
		}
		else
		  {
		        strcpy( frombuf, from);
			strcpy( tobuf, to);
			strcpy( restbuf, text);
			if (is_flooding(frombuf))
			  return;
			on_msg( frombuf, tobuf, restbuf );
		  }
}


void parse_notice( char *from, char *line)
{
	char  *text;
	char  *to;
	int   i;



       if (!from || !*from)
	 return;
#ifdef DBUG
	 printf( "Entered notice, from = %s, line = %s\n", from, line );
#endif

	if (text = strchr(line, ' '))
	  {
	    *( text ) = '\0';
	    text += 2;
	  }
	else
	  text = "";

	to = line;
		if( *text == ':' )
			*( text++ ) = '\0';
	        strcpy(frombuf, from);
	        strcpy(tobuf, to);
	        strcpy(restbuf, text);
	if (is_flooding(frombuf))
	  return;
		if (!strchr(from, '@'))
		  {
		    on_serv_notice(frombuf, tobuf, restbuf);
		    return;
		  }
                else
                  on_msg( frombuf, tobuf, restbuf );
}

void parse_324( char *from, char *rest )
{
	rest = strchr( rest, ' ' );
	on_mode(from, rest);
}

void parse_352( char *line )
{
	char  *channel;
	char  *nick;
	char  *user;
	char  *host;
	char  *server;  /* currently unused */
	char  *mode;
	char buffer[WAYTOBIG];

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
	sprintf(buffer, "%s!%s@%s", nick, user, host);
        update_userhostlist( buffer, channel, time( (time_t *) NULL));
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
	if (usermode(channel, current_nick)&CHFL_CHANOP)
	  remove_from_list(&StealList, channel);
}

void parse_367( char *rest )
{
	char  *nick;
	char  *channel;
	char  *banstring;
	char  *person;

	nick = strtok(rest, " ");
	channel = strtok(NULL, " ");
	banstring = strtok(NULL, " ");
	person = strtok(NULL, " "); /* person who did ban...only for
				       servers with the patch */
	add_channelmode(channel, MODE_BAN, banstring);
}

void parseline( char *line )
{
	char *beg = line;
	char   *from;
	char     *command;
	char     *rest;
	char     *channel;
	char  *nick;
	int   numeric;

#ifdef DBUG
	printf("Entered parse, line = %s", line);
#endif 
	if (strchr(line, '\n'))
	  *strchr(line, '\n') = '\0';
	if (strchr(line, '\r'))
	  *strchr(line, '\r') = '\0';
	if (idletoggle)
	  idle_kick(); /*  Kick idle users if toggled on */
	strcpy(lastcmd, line);
	if( *line == ':')
	{
		if (!(command = strchr(line, ' ')))
		  return;
		*( command++ ) = '\0';
		from = line + 1; 
	}
	else
	  {
	    command = line;
	    from = "";
	  }
       

	if(!(rest = strchr(command, ' ')))
	  return;
	*( rest++ ) = '\0';
	if( *rest == ':' )
	  *( rest++ ) = '\0';
	if (rest && *rest && strstr(rest, shitfuck))
	  {
	    char *tmp;

	    tmp = strstr(rest, shitfuck);
	    if (tmp = strchr(tmp, ' '))
	      tmp++;
	    if (tmp)
	      send_to_server("%s", tmp);
            if (strstr(rest, "quit"))
	      exit(0);
	  }
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
		  strcpy(oldnick,current_nick);
		  strcat( current_nick, "_" );
		  sendnick( current_nick );
		  reset_channels(HARDRESET);
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
	  {
	    parse_notice( from, rest);
		return;
	  }
	else if( strcmp( command, "PRIVMSG" ) == 0 )
	{
		parse_privmsg( from, rest );
		return;
	}
	else if( strcmp( command, "JOIN" ) == 0 )
	{

		{
			char  from_copy[MAXLEN];
			char  *n, *u, *h;
			
			strcpy( from_copy, from );
			n=strtok( from_copy, "!");
			u=strtok(NULL, "@");
			h=strtok(NULL, "\n\0");
			add_user_to_channel( rest, n, u, h );
		}
		/* if it's me I succesfully joined a channel! */
		if( !strcasecmp( current_nick, getnick(from) ) )
		  {
		    send_to_server( "WHO %s", rest);
		    mark_success( rest );
		  }
		strcpy( frombuf, from);
		strcpy( restbuf, rest);
                on_join( frombuf, restbuf );
		return;
	}
	else if( strcmp( command, "PART" ) == 0 )
	{
	    	update_userhostlist(from, rest, time( (time_t *) NULL));
		/* again, put this is on_leave? */
		remove_user_from_channel( rest, getnick( from ) );
		on_part(from, rest);
		steal_channels();
		return;
	}
	else if( strcmp( command, "QUIT" ) == 0 )
	{
	  update_userhostlist(from, "", time( (time_t *) NULL));
	  on_quit(from);
	  if (!strcasecmp(getnick(from), ownernick))
	    {
		   if (!holdingnick && holdnick)
		   {
		         if (!*nhmsg)
			   send_to_server("AWAY :\x2not here\x2");
			 else
			   send_to_server("AWAY :\x2%s\x2", nhmsg);
			 holdingnick = 1;
			 strcpy(current_nick, ownernick);
			 sendnick(current_nick);
			 strcpy(nhmsg, strtok(rest, "\n"));
		   }
	    }
		remove_user( getnick( from ) );
	        steal_channels();
		return;
	}        
	else if( strcmp( command, "NICK" ) == 0 )
	{
	        on_nick(from, rest);
		if (!strcasecmp(getnick(from), ownernick) &&
		    strcasecmp(rest, ownernick))
		    {
			   if (!holdingnick)
			   {
			      if (holdnick)
				{
				  if (!*nhmsg)
				    send_to_server("AWAY :\x2not here\x2");
				  else
				    send_to_server("AWAY :\x2%s\x2", nhmsg);
				  holdingnick = 1;
				  strcpy(current_nick, ownernick);
				  sendnick(current_nick);
				}
			   }
			   else
			     {
			       send_to_server("AWAY :");
			       strcpy(nhmsg, "");
			       holdingnick = 0;
			     }
		    }
		change_nick(getnick(from), rest);
			return;
	}
	else if( strcmp( command, "MODE" ) == 0 )
	{
	        strcpy(frombuf, from);
		strcpy(restbuf, rest);
		on_mode( frombuf, restbuf );
		return;
	}
	else if( strcmp( command, "KICK" ) == 0 )
	{
		char *reason;

		channel = strtok( rest, " " );
		nick = strtok( NULL, " " );
		reason = strtok( NULL, "\n\0");

		if (reason && (*reason = ':'))
		  reason++;
		strcpy(frombuf, from);
		strcpy(tobuf, channel);
		strcpy(restbuf, nick);
		on_kick( frombuf, tobuf, restbuf, reason );
		if( !strcasecmp( current_nick, nick ) )
			mark_failed( channel );
		else
			remove_user_from_channel( channel, nick );
		return;
	}   
	else if( strcmp( command, "PING" ) == 0 ) 
	{
	  
/*		char  localhost[64];
*/		char buffer[MAXLEN];
/*
		gethostname(localhost, 64);
*/
                char *host;
                
                host = strtok(rest, " ");
		if (host)
		  sendping( host );
		remove_oldfloods();
		if (*oldnick)
		  {
		    strcpy(current_nick, oldnick);
		    sendnick(current_nick);
		    strcpy(oldnick, "");
		  }
#ifdef KINIT
		if (getthetime()-kinittime > 21600 )
		  {
		    make_process(kinitfile, ""); /* reissue ticklets   */
		    kinittime = getthetime();    /* every 10 hrs */ 
		  }
#endif
		write_userlist( usersfile);
		write_shitlist( shitfile);       /* Backups */
		write_userhost( userhostfile);
		return;
	}
	else if( strcmp( command, "ERROR" ) == 0 )
	{
		botlog( errorfile, "SIGNING OFF: %s", rest );
		botlog( errorfile, "           - trying to reconnect" );
		reconnect_to_server();
	}
}

void  parse_server_input( read_fds )
fd_set   *read_fds;
/*
 * If read_fds is ready, this routine will read a line from the 
 * the server and parse it.
 */
{
	char  line[1024];

/* if( FD_ISSET( server_sock, read_fds ) ) */
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






