/*
 * parse.c - server input parsing
 * (c) 1994 CoMSTuD (cbehrens@iastate.edu)
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
#include <stdio.h>
#include <ctype.h>
#ifndef MSDOS
  #include <strings.h>
  #include <sys/time.h>
#endif
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include "userlist.h"
#include "parse.h"
#include "hack-ons.h"
#include "channel.h"
#include "debug.h"
#include "function.h"
#include "hackbot.h"
#include "config.h"

extern  botinfo *current;

struct
{
	char    *name;
	void    (*function)(char *from, char *rest);
} parse_funcs[] =
{
	{ "PRIVMSG",    parse_privmsg   },
	{ "NOTICE",     parse_notice    },
	{ "INVITE",     parse_invite    },
	{ "JOIN",       parse_join      },
	{ "PART",       parse_part      },
	{ "MODE",       parse_mode      },      
	{ "NICK",       parse_nick      },
	{ "KICK",       parse_kick      },
	{ "PING",       parse_ping      },
	{ "PONG",       parse_pong      },
	{ "QUIT",       parse_quit      },
	{ "ERROR",      parse_error     },
	{ "001",        parse_001       },
	{ "324",        parse_324       },
	{ "352",        parse_352       },
	{ "367",        parse_367       },
	{ "433",        parse_433       },
	{ "471",        parse_471       },
	{ "473",        parse_471       },
	{ "474",        parse_471       },
	{ "475",        parse_471       },
	{ "476",        parse_471       },
	{ "",         (void(*)())(NULL) }
};

void parse_invite(char *from, char *rest)
{
	char *to;
	char *chan;

	if (max_userlevel(from) >= ASSTLEVEL)
	{
		to = get_token(&rest, " ");
		chan = get_token(&rest, " ");
		join_channel(chan, "", 1);
	}
}

void parse_privmsg(char *from, char *rest)
{
	char    *text;
	char    *to;
	int     i;


	/*      WHAT THE FUCK IS INDEX??????
	if ( ( text = index( rest, ' ' ) ) != 0 )
		*( text ) = '\0'; text += 2;
	*/

	if (text = strchr(rest, ' '))
		*( text ) = '\0';
	text += 2;

	to = rest;
	if( *text == ':' )
		*( text++ ) = '\0';

	if (*text == '\001' && *( text + strlen( text) - 1 ) == '\001' )
	{
		*( text++ ) = '\0';
		terminate(text, "\001");
		on_ctcp( from, to, text );
	}
	else
		on_msg( from, to, text );
}

void parse_notice(char *from, char *rest)
{
	/* ignore notices! */
}

void parse_join(char *from, char *rest)
{
	char    from_copy[MAXLEN];
	char    *f = from_copy;         /* blah */
	char    *n, *u, *h;
			
	strcpy(from_copy, from);
	n=get_token(&f, "!");
	u=get_token(&f, "@");
	h=get_token(&f, "");
	mark_success(rest);
	add_user_to_channel(rest, n, u, h);
	check_limit(rest);
	/* if it's me I succesfully joined a channel! */
	if (!my_stricmp(current->nick, n))
	{

		/* mark_success(rest); */
		/* get channelinfo */

		send_to_server( "WHO %s", rest );
		send_to_server( "MODE %s", rest );
		send_to_server( "MODE %s b", rest );
	}
	on_join(from, rest);
}

void parse_part(char *from, char *rest)
{
	char *resttmp;
	mstrcpy(&resttmp, rest);
	on_part(from, rest);
	remove_user_from_channel(resttmp, getnick(from));
/*
	if (!my_stricmp(getnick(from), current->nick))
	  remove_channel(resttmp);
*/
	steal_channels();
	check_limit(resttmp);
	free(resttmp);
}

void parse_mode(char *from, char *rest)
{
	on_mode(from, rest);
}

void parse_nick(char *from, char *rest)
{
	on_nick(from, rest);
	change_nick(getnick(from), rest);
	change_memnick(from, rest);

	check_shit();
}

void parse_kick(char *from, char *rest)
{
	char    *channel;
	char    *nick;

	channel = get_token(&rest, " ");
	nick = get_token(&rest, " ");

	on_kick(from, channel, nick, rest);

	if (!my_stricmp(current->nick, nick))
		mark_failed(channel);
	else                     /* ?????????????? */
		remove_user_from_channel(channel, nick);
	check_limit(channel);
}

void parse_ping(char *from, char *rest)
{
	char    localhost[64];

/*
	gethostname(localhost, 64);     I don't get it...why??????
	sendping( localhost );                        */
	sendping(rest);         /* <---- seems better idea to me -- CS */
	/* No need to make a seperate on_ping */
	current->lastping = getthetime();
}

void parse_pong(char *from, char *rest)
{
	char *nick, *server;

	server = get_token(&rest, " ");
	nick = get_token(&rest, "");
	if (*nick == ':')
		nick++;
	pong_received(nick, server);
}

void parse_quit(char *from, char *rest)
{
	on_quit(from, rest);
	remove_user(getnick(from));
	steal_channels();
}

void parse_error(char *from, char *rest)
{
	botlog(ERRFILE, "SIGNING OFF: %s", rest);
	botlog(ERRFILE, "           - trying to reconnect");
	reconnect_to_server();
}

void parse_001(char *from, char *rest)
/*
 * Use the 001-reply (Welcome to the Internet Relay Network NICK)
 * to determine the server's name (this might not be the name in the
 * serverlist!). All we have to do is look at "from"
 */
{
	debug(NOTICE, "Current server calls himself %s", from);
	strcpy(current->serverlist[current->current_server].realname, from);  
	/* Successfull connect so we can do the join-stuff */
	reset_channels(HARDRESET);
}

void parse_324(char *from, char *rest)
{
	rest = strchr( rest, ' ' );
	on_mode(from, rest);
}

void    parse_352( char *from, char *rest )
/* 352: who-reply */
{
	char    *channel;
	char    *nick;
	char    *user;
	char    *host;
	char    *server;  /* currently unused */
	char    *mode;
	
	/* skip my own nick */
	get_token(&rest, " " );
	/* Is it a header? return.. */
	if (*rest == 'C')              /* NO NO NO! */
		return;
	channel = get_token(&rest, " ");
	user = get_token(&rest, " ");
	host = get_token(&rest, " ");
	server = get_token(&rest, " ");
	nick = get_token(&rest, " ");
	mode = get_token(&rest, " ");
	add_user_to_channel( channel, nick, user, host );
	if (username(nick) && i_am_op(channel))
	  if (is_shitted(username(nick), channel))
		 shit_action(username(nick), channel);
	while (*mode )
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
			debug(ERROR, "parse_352: unknown mode %c", *mode);
#endif
			break;
		}
		mode++;
	}
}

void parse_367(char *from, char *rest )
{
	char    *nick;
	char    *channel;
	char    *banstring;
	char    *banfrom;
	char    *bantime;

	nick = get_token(&rest, " ");
	channel = get_token(&rest, " ");
	banstring = get_token(&rest, " ");
	banfrom = get_token(&rest, " ");/* these are for servers with banpatch */
	bantime = get_token(&rest, " ");
	add_channelmode((banfrom && *banfrom) ? banfrom : from, channel, MODE_BAN, banstring, (bantime && *bantime) ? atol(bantime) : getthetime());
}

void parse_433(char *from, char *rest) /* 431..436! */
/*
 * How to determine the new nick: 
 * - If it's possible to add a '_' to the nick, do it (done)
 * - else loop through the nick from begin to end and replace
 *   the first non-'_' with a '_'. i.e. __derServ -> ___erServ
 * - If the nick is 9 '_''s, try the original nick with something random
 */
{
	char    *s;
	
	if (strlen(current->nick) == MAXNICKLEN)
	{
		for(s=current->nick; *s && (*s == '_'); s++)
			;
		if(*s)
			*s = '_';
		else
		{
			char    smallnick[6];
			char    random[5];
			/* Try to create a unique botname */
			strncpy(current->nick, current->realnick, 5);
			/* Could someone pls tell me what the f*ck I'm 
				doing wrong and why the next line is needed!? */
			/* Yes...strncpy doesn't append a '\0' if the string to be
				copied is longer than 5  -- CS
			*/
			current->nick[5] = '\0';
			sprintf(random, "%d", (int)(rand() % 10000));
			strcat(current->nick, random);
		}
	}
	else
		strcat(current->nick, "_");
	sendnick(current->nick);
}

void parse_471(char *from, char *rest)
{
	char    *channel;
return;    /* test */
	rest = strchr(rest, '#');
	channel = get_token(&rest, " ");
	mark_failed(channel);
}

void parseline(char *line)
{
	char    *from; 
	char    *command;
	char    *rest;
	int     i;


	/* terminate the line where either \r or \n exist */
	/* The new servers put a \r before the \n */

	terminate(line, "\r\n");
	check_idle();

#ifdef DBUG
	debug(NOTICE, "parseline(\"%s\")", line);
#endif 

	if (*line == ':' )
	{
	  /* ??????????????????????
		if (( command = index( line, ' ' ) ) == 0)
			return;
		*/

		if (!(command = strchr(line, ' ')))
		  return;

		*(command++) = '\0';
		from = line + 1; 
	} 
	else 
	{
		command = line;
		from = NULL;
	}
  
	/* ?????????????????
	if ((rest = index( command, ' ' ) ) == 0)
		return;
	*/
	
	if(!(rest = strchr(command, ' ')))
	  return;

	*( rest++ ) = '\0';
  
	if( *rest == ':' )
		*( rest++ ) = '\0';

	for (i=0; parse_funcs[i].name[0]; i++)
	  if (!my_stricmp(parse_funcs[i].name, command))
		 {
			parse_funcs[i].function(from, rest);
			return;
		 } 
}




