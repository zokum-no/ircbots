/*
 * cfgfile.c - Simpele parser voor configfile
 * Copyright (C) 1993-94 VladDrac (irvdwijk@cs.vu.nl)
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
 *
 */
#include <string.h>
#ifndef SYSV
#include <strings.h>
#endif
#include <ctype.h>
#include <netdb.h>
#include <stdio.h>
#include <stdarg.h>

#include "cfgfile.h"
#include "debug.h"
#include "misc.h"
#include "parsing.h"
#include "vladbot.h"
#include "config.h"

/*
 * Some short info on how things are parsed.
 * A line can be one of four things:
 * 1) %command 	params		(i.e. %debug 2)
 * 2) BotName	definition	params (i.e. NederServ channel #dutch)
 * 3)		definition	params (when %with was used)
 * 4) # comments....
 *
 * Lines are parsed one by one. When looking at the first token,
 * the parser looks if the token is a command (started with %),
 * a botname (in this case, the definition takes effect on the
 * specified bot) or a just a definition (in this case the default
 * bot is assumed). When the first token starts with '#', the rest 
 * of the line is ignored. 
 * NOTE: botname has higher priority than definition,
 * i.e look at this piece of code:
 * %bot Channel		(weird name for a bot)
 * %bot Blah
 * %with Blah
 * Channel server "irc.funet.fi"
 *
 * The last line will add the server to Channel's serverlist.
 *
 * After all parsing on a line has been finished, the rest of the
 * line is discarded.
 */

struct
{
	char	*name;
	void	(*function)(char *s);
} config_cmds[] =
{
	/* Table with commands used to configure */
	{ "SET",	set_var		},
	{ "DEBUG",	set_debug	},
	{ "CREATE",	create_bot      },
	{ "BOT",	create_bot	},
	{ "LISTSET",	create_userlist	},
	{ "WITH",	set_default     },
	{ NULL,		(void(*)())(NULL) }
};

struct
{
	char	*name;
	int	bot_def;	/* bot or list definition */
	void    (*function)();
} definition_cmds[] =
/* for bots AND for userlists */
{
	/* list definitions */
	{ "USERLIST",		  FALSE,	set_listuser			},
	{ "EXCLUDELIST",	  FALSE,	set_listexclude		},
	{ "PROTLIST",		  FALSE,	set_listprot			},
	{ "SHITLIST",		  FALSE,	set_listshit			},
	{ "PASSWDLIST",	  FALSE, set_listpasswd			},
	{ "FPLIST",			  FALSE, set_listfp				},
	{ "ACCESSLIST",	  FALSE, set_listaccess			},
	{ "NICKLIST",		  FALSE,	set_listnick			},
	/* Bot definitions */
	{ "NICK",			 	TRUE,	set_nick					},
	{ "LOGIN",			 	TRUE,	set_login				},
	{ "IRCHOST",		 	TRUE,	set_irchost				},
	{ "NAME",			 	TRUE,	set_name					},
	{ "SERVER",			 	TRUE,	add_server				},
	{ "CHANNEL",			TRUE,	add_to_channellist	},
	{ "UPLOAD",			 	TRUE,	set_uploaddir			},
	{ "DOWNLOAD",		 	TRUE,	set_downloaddir		},
	{ "EVENTFILE",			TRUE,	set_eventfile			},
	{ "MAILTOFILE",		TRUE,	set_mailtofile			},
	{ "INDEX",			 	TRUE,	set_indexfile			},
	{ "HELP",			 	TRUE,	set_helpfile			},
	{ "LISTSET",		 	TRUE,	set_listset				},
	{ "TALK",			 	TRUE, set_talk					},
	{ "SPYLOG",			 	TRUE, set_spylog				},
	{ "REPLYPING",		 	TRUE, set_replyping			},
	{ "REPLYVERSION",	 	TRUE, set_replyversion		},
	{ "REPLYFINGER",	 	TRUE, set_replyfinger		},
	{ "FLOODPROT_PUBLIC",TRUE,	set_floodprot_public },
	{ "FLOODPROT_NICK",	TRUE,	set_floodprot_nick	},
	{ "FLOODPROT_CLONE",	TRUE,	set_floodprot_clone	},
	{ "FLOODPROT_CTCP",	TRUE,	set_floodprot_ctcp	},
	{ "FLOODPROT_OP",	 	TRUE,	set_floodprot_op		},
	{ "FLOODPROT_AWP", 	TRUE,	set_floodprot_awp		},
	{ "SECURE",			 	TRUE,	set_secure				},
	{ "THRESHOLD",		 	TRUE,	set_threshold			},
	{ "LOCKPASS",		 	TRUE,	set_lockpass			},
#ifdef BackedUp
	{ "BACKUPNICK", 	 	TRUE,	set_backupnick			},
	{ "BACKUPPASSWD",	 	TRUE,	set_backuppasswd		},
#endif
#ifdef BackUp
	{ "BACKUPMASK", 	 	TRUE,	set_backupmask			},
	{ "BACKPASSWD",	 	TRUE,	set_backpasswd			},
#endif
	{ NULL,					 0,	(void(*)())(NULL)		}
};

struct
{
	char	*name;
	void    (*function)();
} setting_cmds[] =
{	
	{ "IDLETIMEOUT",		set_idletimeout 	},
	{ "WAITTIMEOUT",		set_waittimeout 	},
	{ "MAXUPLOADSIZE",	set_maxuploadsize	},
	{ "NOTEFILE",			set_notefile 		},
	{ "DEBUG",				set_globaldebug	},
	{ "MAINTAINER",		set_maintainer		},
	{ NULL,					(void(*)())(NULL) }
};


extern	int	number_of_bots;
extern	int	find_channel(botinfo *bot, char *channel);
extern	int	add_channel_to_bot(botinfo *bot, char *channel, char *topic, char *mode);
extern	int	find_server(botinfo *bot, char *server, int port);
extern	int	add_server_to_bot(botinfo *bot, char *server, int port);
extern	botinfo	*bot_created(char *s);
extern	botinfo	*add_bot(char *s);
extern	int	idletimeout;
extern	int	waittimeout;
extern	int	maxuploadsize;
extern	char	*notefile;
extern	char	*botmaintainer;

static	botinfo		*defaultbot = NULL;
static	listinfo	*defaultset = NULL;
static	int	linenum = 0;
static	int	dbg_lvl = QUIET;

char	*configfile = "infile.cfg";

/*
 * misc functions
 */

static	void	cfg_debug(int lvl, char *form, ...)
{
	va_list	msg;
	char	buf[MAXLEN];

	if(lvl>dbg_lvl)
		return;
	va_start(msg, form);
	vsprintf(buf, form, msg);
	printf("[%d] %s\n", linenum, buf);
	va_end(msg);
}

/* commands etc. */

void	set_debug(char *s)
/*
 * Sets the debuglevel for the parser.
 * Syntax: %debug INTEGER[0..2]
 */
{
	int	value;
	
	if(readint(&s, &value))
	{
		if(value < QUIET || value > NOTICE)
			cfg_debug(ERROR, "%%DEBUG expects 0 <= argument <= 2!");
		else
		{
			cfg_debug(NOTICE, "DEBUG set to %d", value);
			dbg_lvl = value;
		}
	}
	else
		cfg_debug(ERROR, "%%DEBUG requires an argument!");
}

void	create_bot(char *s)
/*
 * Creates a bot.
 * Syntax: %bot IDENTIFIER
 */
{
	char	botname[MAXLEN];

	if(readident(&s, botname))
	{
		if(listset_created(botname))
		{
			cfg_debug(ERROR, "There is already a listset called \"%s\"", botname);
			return;
		}
		if(bot_created(botname))
		{
			cfg_debug(ERROR, "\"%s\" is already created!", botname);
			return;
		}
		if(!add_bot(botname))
			cfg_debug(ERROR, "Too many bots created!");
		else
			cfg_debug(NOTICE, "CREATE: bot \"%s\"", botname);
	}
	else
		cfg_debug(ERROR, "%%CREATE requires an identifier-argument!");
}

void	create_userlist(char *s)
/*
 * Create a userlist.
 * Syntax: %create IDENTIFIER
 */
{
	char	listname[MAXLEN];
	
	if(readident(&s, listname))
	{
		if(bot_created(listname))
		{
			cfg_debug(ERROR, "There is already a bot called \"%s\"", listname);
			return;
		}
		if(listset_created(listname))
		{
			cfg_debug(ERROR, "\"%s\" is already created!", listname);
			return;
		}
		if(!add_listset(listname))
			cfg_debug(ERROR, "Too many listsets created!");
		else
			cfg_debug(NOTICE, "LISTSET: listset \"%s\"", listname);
	}
	else
		cfg_debug(ERROR, "%%LISTSET requires an identifier-argument!");
}


void	set_default(char *s)
/*
 * Set default bot/listset for %with
 * Syntax: %with IDENTIFIER
 */
{
	char	botname[MAXLEN];

	if(readident(&s, botname))
	{
		if(bot_created(botname))
		{
			cfg_debug(NOTICE, "DEFAULT: bot \"%s\"", botname);
			defaultbot = bot_created(botname);
		}
		else if(listset_created(botname))
		{
			cfg_debug(NOTICE, "DEFAULT: listset \"%s\"", botname);
			defaultset = listset_created(botname);
		}
		else
			cfg_debug(ERROR, "\"%s\" is not created!", botname);
	}
	else
		cfg_debug(ERROR, "%%WITH requires an identifier-argument!");
}

/* settings/definitions */

void	set_listprot(listinfo *list, char *s)
/*
 * Set the protlist for list.
 * Syntax: protlist STRING
 */
{
	char	listname[MAXLEN];
	char	*path;

	if(readstring(&s, listname))
	{
		if(not(path = expand_twiddle(listname)))
		{
			cfg_debug(ERROR, "Error in pathname \"%s\"!", listname);
			path = "";
		}
		free(list->protfile);
		mstrcpy(&list->protfile, path);
		cfg_debug(NOTICE, "Setting protlist for %s to \"%s\"",
			  list->listname, path);
		return;
	}
	else
		cfg_debug(ERROR, "%s protlist expects a pathname (string)!", 
		          list->listname);
}

void	set_listshit(listinfo *list, char *s)
/*
 * Set the shitlist for list.
 * Syntax: shitlist STRING
 */
{
	char	listname[MAXLEN];
	char	*path;
	
	if(readstring(&s, listname))
	{
		if(not(path = expand_twiddle(listname)))
		{
			cfg_debug(ERROR, "Error in pathname \"%s\"!", listname);
			path = "";
		}
		free(list->shitfile);
		mstrcpy(&list->shitfile, path);
		cfg_debug(NOTICE, "Setting shitlist for %s to \"%s\"",
			  list->listname, path);
		return;
	}
	else
		cfg_debug(ERROR, "%s shitlist expects a pathname (string)!", 
		          list->listname);
}

void	set_listuser(listinfo *list, char *s)
/*
 * Set the userlist for list.
 * Syntax: userlist STRING
 */
{
	char	listname[MAXLEN];
	char	*path;
	
	if(readstring(&s, listname))
	{
		if(not(path = expand_twiddle(listname)))
		{
			cfg_debug(ERROR, "Error in pathname \"%s\"!", listname);
			path = "";
		}		
		free(list->opperfile);
		mstrcpy(&list->opperfile, path);
		cfg_debug(NOTICE, "Setting userlist for %s to \"%s\"",
			  list->listname, path);
		return;
	}
	else
		cfg_debug(ERROR, "%s userlist expects a pathname (string)!", 
		          list->listname);
}

void	set_listexclude(listinfo *list, char *s)
/*
 * Set the userlist for list.
 * Syntax: userlist STRING
 */
{
	char	listname[MAXLEN];
	char	*path;
	
	if(readstring(&s, listname))
	{
		if(not(path = expand_twiddle(listname)))
		{
			cfg_debug(ERROR, "Error in pathname \"%s\"!", listname);
			path = "";
		}		
		free(list->excludefile);
		mstrcpy(&list->excludefile, path);
		cfg_debug(NOTICE, "Setting excludelist for %s to \"%s\"",
			  list->listname, path);
		return;
	}
	else
		cfg_debug(ERROR, "%s excludelist expects a pathname (string)!", 
		          list->listname);
}

void	set_listfp(listinfo *list, char *s)
/*
 * Set the userlist for list.
 * Syntax: userlist STRING
 */
{
	char	listname[MAXLEN];
	char	*path;
	
	if(readstring(&s, listname))
	{
		if(not(path = expand_twiddle(listname)))
		{
			cfg_debug(ERROR, "Error in pathname \"%s\"!", listname);
			path = "";
		}		
		free(list->fpfile);
		mstrcpy(&list->fpfile, path);
		cfg_debug(NOTICE, "Setting fplist for %s to \"%s\"",
			  list->listname, path);
		return;
	}
	else
		cfg_debug(ERROR, "%s fplist expects a pathname (string)!", 
		          list->listname);
}

void	set_listaccess(listinfo *list, char *s)
/*
 * Set the userlist for list.
 * Syntax: userlist STRING
 */
{
	char	listname[MAXLEN];
	char	*path;
	
	if(readstring(&s, listname))
	{
		if(not(path = expand_twiddle(listname)))
		{
			cfg_debug(ERROR, "Error in pathname \"%s\"!", listname);
			path = "";
		}		
		free(list->accessfile);
		mstrcpy(&list->accessfile, path);
		cfg_debug(NOTICE, "Setting accesslist for %s to \"%s\"",
			  list->listname, path);
		return;
	}
	else
		cfg_debug(ERROR, "%s accesslist expects a pathname (string)!", 
		          list->listname);
}

void	set_listnick(listinfo *list, char *s)
/*
 * Set the userlist for list.
 * Syntax: userlist STRING
 */
{
	char	listname[MAXLEN];
	char	*path;
	
	if(readstring(&s, listname))
	{
		if(not(path = expand_twiddle(listname)))
		{
			cfg_debug(ERROR, "Error in pathname \"%s\"!", listname);
			path = "";
		}		
		free(list->nickfile);
		mstrcpy(&list->nickfile, path);
		cfg_debug(NOTICE, "Setting nicklist for %s to \"%s\"",
			  list->listname, path);
		return;
	}
	else
		cfg_debug(ERROR, "%s nicklist expects a pathname (string)!", 
		          list->listname);
}

void	set_listpasswd(listinfo *list, char *s)
/*
 * Set the userlist for list.
 * Syntax: userlist STRING
 */
{
	char	listname[MAXLEN];
	char	*path;
	
	if(readstring(&s, listname))
	{
		if(not(path = expand_twiddle(listname)))
		{
			cfg_debug(ERROR, "Error in pathname \"%s\"!", listname);
			path = "";
		}		
		free(list->passwdfile);
		mstrcpy(&list->passwdfile, path);
		cfg_debug(NOTICE, "Setting passwdlist for %s to \"%s\"",
			  list->listname, path);
		return;
	}
	else
		cfg_debug(ERROR, "%s passwdlist expects a pathname (string)!", 
		          list->listname);
}

void	set_listset(botinfo *bot, char *s)
/*
 * Set the listset for bot.
 * Syntax: listset IDENTIFIER
 */
{
	char	listname[MAXLEN];
	
	if(readident(&s, listname))
	{
		if(listset_created(listname))
		{
			bot->lists = listset_created(listname);
			cfg_debug(NOTICE, "Setting lists for %s to \"%s\"",
				  bot->botname, listname);
		}
		else
			cfg_debug(ERROR, "No listset \"%s\"!", listname);
	}
	else
		cfg_debug(ERROR, "%s listset expects a identifier-argument!",
			  bot->botname);
}

void	set_nick(botinfo *bot, char *s)
/*
 * Set the nickname for bot.
 * Syntax: nick STRING
 */
{
	char	nickname[MAXLEN];
	
	if(readstring(&s, nickname))
	{
		if(!isnick(nickname))
		{
			cfg_debug(ERROR, "Illegal nickname \"%s\"!", nickname);
			return;
		}
		cfg_debug(NOTICE, "Setting nick for %s to \"%s\"", 
			  bot->botname, nickname);
		strcpy(bot->nick, nickname);
		strcpy(bot->realnick, nickname);
	}
	else
		cfg_debug(ERROR, "%s nick expects a nick (string)!", 
			  bot->botname);
}

void	set_eventfile(botinfo *bot, char *s)
/*
 * Set the eventfile for bot.
 */
{
	char	eventfile[MAXLEN];
	char	*path;

	if(readstring(&s, eventfile))
	{
		if(not(path = expand_twiddle(eventfile)))
		{
			cfg_debug(ERROR, "Error in pathname \"%s\"!", eventfile);
			path = "";
		}
		free(bot->eventfile);
		mstrcpy(&bot->eventfile, path);
		cfg_debug(NOTICE, "Setting eventfile for %s to \"%s\"",
			  bot->botname, path);
		return;
	}
	else
		cfg_debug(ERROR, "%s eventfile expects a pathname (string)!",
					 bot->botname);
}

void	set_mailtofile(botinfo *bot, char *s)
/*
 * Set the mailto file for bot.
 */
{
	char	mailtofile[MAXLEN];
	char	*path;

	if(readstring(&s, mailtofile))
	{
		if(not(path = expand_twiddle(mailtofile)))
		{
			cfg_debug(ERROR, "Error in pathname \"%s\"!", mailtofile);
			path = "";
		}
		free(bot->mailtofile);
		mstrcpy(&bot->mailtofile, path);
		cfg_debug(NOTICE, "Setting mailtofile for %s to \"%s\"",
			  bot->botname, path);
		return;
	}
	else
		cfg_debug(ERROR, "%s mailtofile expects a pathname (string)!",
					 bot->botname);
}


void	set_threshold(botinfo *bot, char *s)
/*
 * Set the nickname for bot.
 * Syntax: nick STRING
 */
{
		if(atoi(s)<0)
		{
			cfg_debug(ERROR, "Illegal threshold %s!", s);
			return;
		}
		cfg_debug(NOTICE, "Setting threshold for %s to %s", 
			  bot->botname, s);
		bot->failurethreshold=atoi(s);
}

void	set_talk(botinfo *bot, char *s)
/*
 * Set the nickname for bot.
 * Syntax: nick STRING
 */
{
	
		if(atoi(s)<0 || atoi(s)>MAXTALK)
		{
			cfg_debug(ERROR, "Illegal talk value %s!", s);
			return;
		}
		cfg_debug(NOTICE, "Setting talk for %s to %s", 
			  bot->botname, s);
		bot->talk=atoi(s);
}

void    set_spylog(botinfo *bot, char *s)
/*
 * Set the nickname for bot.
 * Syntax: nick STRING
 */
{
 
                if(atoi(s)!=0 && atoi(s)!=1)
                {
                        cfg_debug(ERROR, "Illegal spylog value %s!", s);
                        return;
                }
                cfg_debug(NOTICE, "Setting spylog for %s to %s",
                          bot->botname, s);
                bot->spylog=atoi(s);
}

void    set_replyping(botinfo *bot, char *s)
/*
 Determines if/who the bot replies to CTCP PING
 */
{
 
                if(atoi(s)<0 || atoi(s)>MAX_REPLYPING)
                {
                        cfg_debug(ERROR, "Illegal replyping value %s!", s);
                        return;
                }
                cfg_debug(NOTICE, "Setting replyping for %s to %s",
                          bot->botname, s);
                bot->reply_ping=atoi(s);
}

void    set_replyversion(botinfo *bot, char *s)
/*
 Determines if/who the bot replies to CTCP VERSION
 */
{
 
                if(atoi(s)<0 || atoi(s)>MAX_REPLYVERSION)
                {
                        cfg_debug(ERROR, "Illegal replyversion value %s!", s);
                        return;
                }
                cfg_debug(NOTICE, "Setting replyversion for %s to %s",
                          bot->botname, s);
                bot->reply_version=atoi(s);
}

void    set_replyfinger(botinfo *bot, char *s)
/*
 Determines if/who the bot replies to CTCP FINGER
 */
{
 
                if(atoi(s)<0 || atoi(s)>MAX_REPLYFINGER)
                {
                        cfg_debug(ERROR, "Illegal replyfinger value %s!", s);
                        return;
                }
                cfg_debug(NOTICE, "Setting replyfinger for %s to %s",
                          bot->botname, s);
                bot->reply_finger=atoi(s);
}

void    set_floodprot_clone(botinfo *bot, char *s)
/*
	Enables/disables the clonebot detection
 */
{
                if(atoi(s)<0 || atoi(s)>3)
                {
                        cfg_debug(ERROR, "Illegal floodprot_clone value %s!", s);
                        return;
                }
                cfg_debug(NOTICE, "Setting floodprot_clone for %s to %s",
                          bot->botname, s);
                bot->floodprot_clone=atoi(s);
}
 
void    set_floodprot_nick(botinfo *bot, char *s)
/*
	Enables/disables the nick flood detection
 */
{
                if(atoi(s)<0 || atoi(s)>3)
                {
                        cfg_debug(ERROR, "Illegal floodprot_nick value %s!", s);
                        return;
                }
                cfg_debug(NOTICE, "Setting floodprot_nick for %s to %s",
                          bot->botname, s);
                bot->floodprot_nick=atoi(s);
}
 
void    set_floodprot_public(botinfo *bot, char *s)
/*
	Enables/disables the public flood detection
 */
{
                if(atoi(s)<0 || atoi(s)>3)
                {
                        cfg_debug(ERROR, "Illegal floodprot_public value %s!", s);
                        return;
                }
                cfg_debug(NOTICE, "Setting floodprot_public for %s to %s",
                          bot->botname, s);
                bot->floodprot_public=atoi(s);
}

void    set_floodprot_ctcp(botinfo *bot, char *s)
/*
	Enables/disables the ctcp flood detection
 */
{
                if(atoi(s)<0 || atoi(s)>3)
                {
                        cfg_debug(ERROR, "Illegal floodprot_ctcp value %s!", s);
                        return;
                }
                cfg_debug(NOTICE, "Setting floodprot_ctcp for %s to %s",
                          bot->botname, s);
                bot->floodprot_ctcp=atoi(s);
}
 
void    set_floodprot_op(botinfo *bot, char *s)
/*
	Enables/disables the ctcp flood detection
 */
{
                if(atoi(s)<0 || atoi(s)>3)
                {
                        cfg_debug(ERROR, "Illegal floodprot_op value %s!", s);
                        return;
                }
                cfg_debug(NOTICE, "Setting floodprot_op for %s to %s",
                          bot->botname, s);
                bot->floodprot_op=atoi(s);
}
 
void    set_floodprot_awp(botinfo *bot, char *s)
/*
	Enables/disables the ctcp flood detection
 */
{
                if(atoi(s)<0 || atoi(s)>3)
                {
                        cfg_debug(ERROR, "Illegal floodprot_awp value %s!", s);
                        return;
                }
                cfg_debug(NOTICE, "Setting floodprot_awp for %s to %s",
                          bot->botname, s);
                bot->floodprot_awp=atoi(s);
}
 
void	set_secure(botinfo *bot, char *s)
/*
 * Set the nickname for bot.
 * Syntax: nick STRING
 */
{
		if(atoi(s)<0 || atoi (s)>MAX_SECURE)
		{
			cfg_debug(ERROR, "Illegal secure value %s!", s);
			return;
		}
		cfg_debug(NOTICE, "Setting secure for %s to %s", 
			  bot->botname, s);
		bot->secure=atoi(s);
}

void	set_lockpass(botinfo *bot, char *s)
/*
 * Set the nickname for bot.
 * Syntax: nick STRING
 */
{
	char	nickname[MAXLEN];
	
	if(readstring(&s, nickname))
	{
		cfg_debug(NOTICE, "Setting lockpass for %s to \"%s\"", 
			  bot->botname, nickname);
		free(bot->lockpass);
		mstrcpy(&bot->lockpass, nickname);
	}
	else
		cfg_debug(ERROR, "%s lockpass expects a lockpass (string)!", 
			  bot->botname);
}

#ifdef BackedUp

void    set_backupnick(botinfo *bot, char *s)
/*
 * Set the nickname for bot.
 * Syntax: nick STRING
 */
{
        char    nickname[MAXLEN];

        if(readstring(&s, nickname))
        {
                cfg_debug(NOTICE, "Setting backupnick for %s to \"%s\"",
                          bot->botname, nickname);
                free(bot->backupnick);
                mstrcpy(&bot->backupnick, nickname);
        }
        else
                cfg_debug(ERROR, "%s backupnick expects a backupnick (string)!",
                          bot->botname);
}

void    set_backuppasswd(botinfo *bot, char *s)
/*
 * Set the nickname for bot.
 * Syntax: nick STRING
 */
{
        char    nickname[MAXLEN];

        if(readstring(&s, nickname))
        {
                cfg_debug(NOTICE, "Setting backuppasswd for %s to \"%s\"",
                          bot->botname, nickname);
                free(bot->backupbotpasswd);
                mstrcpy(&bot->backupbotpasswd, nickname);
        }
        else
                cfg_debug(ERROR, "%s backuppasswd expects a backuppasswd (string)!",
                          bot->botname);
}

#endif

#ifdef BackUp
void    set_backupmask(botinfo *bot, char *s)
/*
 * Set the nickname for bot.
 * Syntax: nick STRING
 */
{
        char    nickname[MAXLEN];

        if(readstring(&s, nickname))
        {
                cfg_debug(NOTICE, "Setting backupmask for %s to \"%s\"",
                          bot->botname, nickname);
                free(bot->backupmask);
                mstrcpy(&bot->backupmask, nickname);
        }
        else
                cfg_debug(ERROR, "%s backupmask expects a backupmask (string)!",
                          bot->botname);
}

void    set_backpasswd(botinfo *bot, char *s)
/*
 * Set the nickname for bot.
 * Syntax: nick STRING
 */
{
        char    nickname[MAXLEN];

        if(readstring(&s, nickname))
        {
                cfg_debug(NOTICE, "Setting backpasswd for %s to \"%s\"",
                          bot->botname, nickname);
                free(bot->backuppasswd);
                mstrcpy(&bot->backuppasswd, nickname);
        }
        else
                cfg_debug(ERROR, "%s backpasswd expects a backpasswd (string
)!",
                          bot->botname);
}

#endif

void	set_login(botinfo *bot, char *s)
/*
 * Set the loginname for bot.
 * Syntax: login STRING
 */
{
	char	loginname[MAXLEN];
	
	if(readstring(&s, loginname))
	{
		cfg_debug(NOTICE, "Setting login for %s to \"%s\"", 
                          bot->botname, loginname);
		strcpy(bot->login, loginname);
	}
	else
		cfg_debug(ERROR, "%s login expects a login (string)!", bot->botname);
}

void	set_irchost(botinfo *bot, char *s)
/*
 * Set the irchost for bot.
 * Syntax: IRCHOST STRING
 */
{
	struct hostent *hp;
	char irchost[512];
	
	if(readstring(&s, irchost))
	{
		cfg_debug(NOTICE, "Setting irchost for %s to \"%s\"", 
                          bot->botname, irchost);
		bot->LocalHostName = strdup(irchost);
		bzero((void *)&(bot->LocalHostAddr), sizeof(bot->LocalHostAddr));
		if ((hp = gethostbyname(bot->LocalHostName)))
			bcopy(hp->h_addr, (void *)&(bot->LocalHostAddr), sizeof(bot->LocalHostAddr));

	}
	else
		cfg_debug(ERROR, "%s irchost expects a hostname (string)!", bot->botname);
}

void	set_name(botinfo *bot, char *s)
/*
 * Set the name for bot.
 * Syntax: name STRING
 */
{  
	char	realname[MAXLEN];
	
	if(readstring(&s, realname))
	{
                cfg_debug(NOTICE, "Setting name for %s to \"%s\"", 
			  bot->botname, realname);
		free(bot->name);
		mstrcpy(&bot->name, realname);
	}
        else
                cfg_debug(ERROR, "%s name expects a name (string)!", bot->botname);
}

void	add_server(botinfo *bot, char *s)
/*
 * Adds a server to the serverlist of bot
 * Syntax: server STRING [, INTEGER]
 *                (name)    (port)
 */
{  
	char	servername[MAXLEN];
	int	port = 0;
	
	if(readstring(&s, servername))
	{
		skipspc(&s);
		if(*s == ',')		/* read port */
		{
			s++;
			if(not(readint(&s, &port)))
			{
				port = 6667;
				cfg_debug(ERROR, "Port (integer) expected after ','!");
			}
		}
		else
			port = 6667;
		
		if(find_server(bot, servername, port))
			cfg_debug(ERROR, "Server \"%s\" port %d already in list!", 
				  servername, port);
		else
			if(add_server_to_bot(bot, servername, port))
				cfg_debug(NOTICE, 
                                          "Server \"%s\" with port %d added to \"%s\"",
					  servername, port, bot->botname);
			else
				cfg_debug(ERROR, "Serverlist full!");
	}
        else
                cfg_debug(ERROR, "%s server expects a server (string)!", 
			  bot->botname);
}

void	add_to_channellist(botinfo *bot, char *s)
/*
 * Adds a channel to bot's channellist.
 * Syntax: channel STRING [, STRING [, STRING ]]
 *                 (name)    (mode)    (topic)
 */
{  
	char	channelname[MAXLEN];
	char	mode[MAXLEN];
	char	topic[MAXLEN];
	
	strcpy(mode, "");
	strcpy(topic, "");

	if(readstring(&s, channelname))
	{
		skipspc(&s);
		if(*s == ',')		/* read mode */
		{
			s++;
			skipspc(&s);
			if(*s != ',')	/* No mode, read topic */
				if(not(readstring(&s, mode)))
				{
					strcpy(mode, "");
					cfg_debug(ERROR, "Mode (string) expected!");
					return;
				}
			skipspc(&s);
			if(*s == ',')	/* read topic */
			{
				s++;
				skipspc(&s);
				if(not(readstring(&s, topic)))
				{
					strcpy(topic, "");
					cfg_debug(ERROR, "Topic (string) expected!");
					return;
				}
			}
		}
		if(find_channel(bot, channelname))
			cfg_debug(ERROR, "Channel \"%s\" already in list!", channelname);
		else
			if(add_channel_to_bot(bot, channelname, topic, mode))
				cfg_debug(NOTICE, "Channel \"%s\" \"%s\" \"%s\" added to %s", 
					  channelname, mode, topic, bot->botname);
			else
				cfg_debug(ERROR, "Channellist full!");
	}
        else
                cfg_debug(ERROR, "%s channel expects a channel (string)!", bot->botname);
}

void	set_uploaddir(botinfo *bot, char *s)
/*
 * Sets the uploaddir for bot.
 * Syntax: upload STRING
 */
{
	char	upload[MAXLEN];
	char	*path;

	if(readstring(&s, upload))
        {
       		if(not(path = expand_twiddle(upload)))
		{
			cfg_debug(ERROR, "Error in pathname \"%s\"!", upload);
			path = "";
		}         
		cfg_debug(NOTICE, "Setting upload for %s to \"%s\"", 
			  bot->botname, path);
		free(bot->uploaddir);
                mstrcpy(&bot->uploaddir, path);
        }
        else
                cfg_debug(ERROR, "%s upload expects a path (string)!", 
			  bot->botname);
}

void	set_downloaddir(botinfo *bot, char *s)
/*
 * Sets downloaddir for bot.
 * Syntax: download STRING
 */
{
	char	download[MAXLEN];
	char	*path;

        if(readstring(&s, download))
        {
       		if(not(path = expand_twiddle(download)))
		{
			cfg_debug(ERROR, "Error in pathname \"%s\"!", download);
			path = "";
		}         
		cfg_debug(NOTICE, "Setting download for %s to \"%s\"",
                          bot->botname, path);
		free(bot->downloaddir);
                mstrcpy(&bot->downloaddir, path);
        }                            
        else
                cfg_debug(ERROR, "%s download expects a path (string)!",
                          bot->botname);
}

void	set_indexfile(botinfo *bot, char *s)
/*
 * Sets indexfile for bot.
 * Syntax: index STRING
 */
{
	char	index[MAXLEN];
	char	*path;

	if(readstring(&s, index))
        {
       		if(not(path = expand_twiddle(index)))
		{
			cfg_debug(ERROR, "Error in pathname \"%s\"!", index);
			path = "";
		}         
		cfg_debug(NOTICE, "Setting indexfile for %s to \"%s\"",
                          bot->botname, path);
		free(bot->indexfile);
                mstrcpy(&bot->indexfile, path);
        }
        else
                cfg_debug(ERROR, "%s index expects a filename (string)!",
                          bot->botname);
}

void    set_helpfile(botinfo *bot, char *s)
/*
 * Sets helpfile for bot.
 * Syntax: help STRING
 */
{
	char	help[MAXLEN];
	char	*path;
	
	if(readstring(&s, help))
        {
       		if(not(path = expand_twiddle(help)))
		{
			cfg_debug(ERROR, "Error in pathname \"%s\"!", help);
			path = "";
		}         
		cfg_debug(NOTICE, "Setting helpfile for %s to \"%s\"",
                          bot->botname, path);
		free(bot->helpfile);
                mstrcpy(&bot->helpfile, path);
        }
        else
                cfg_debug(ERROR, "%s help expects a filename (string)!",
                          bot->botname);
}

/* settings */

void	set_notefile(char *s)
/*
 * Sets global notefile.
 * Syntax: notefile STRING
 */
{
	char	nfile[MAXLEN];
	char	*path;

	if(readstring(&s, nfile))
        {
		if(not(path = expand_twiddle(nfile)))
		{
			cfg_debug(ERROR, "Error in pathname \"%s\"!", nfile);
			path = "";
		}		
		else
		{
			cfg_debug(NOTICE, "Setting notefile to \"%s\"", path);
			mstrcpy(&notefile, path);
		}
	}
	else
		cfg_debug(ERROR, "notefile expects a pathname (string)!");
}

void	set_idletimeout(char *s)
/*
 * Sets global idletimeout.
 * Syntax: idletimeout INTEGER
 */
{
	if(readint(&s, &idletimeout))
		cfg_debug(NOTICE, "Setting idletimeout to %d", idletimeout);
	else
	{
		cfg_debug(ERROR, "idletimeout expects a number (integer)!");
		idletimeout = DCC_IDLETIMEOUT;
	}
}

void	set_waittimeout(char *s)
/*
 * Sets global waittimeout.
 * Syntax: waittimeout INTEGER
 */
{
	if(readint(&s, &waittimeout))
		cfg_debug(NOTICE, "Setting waittimeout to %d", waittimeout);
	else
	{
		cfg_debug(ERROR, "waittimeout expects a number (integer)!");
		waittimeout = DCC_WAITTIMEOUT;
	}
}

void	set_maxuploadsize(char *s)
/*
 * Sets global maxuploadsize.
 * Syntax: maxuploadsize INTEGER
 */
{
	if(readint(&s, &maxuploadsize))
		cfg_debug(NOTICE, "Setting maxuploadsize to %d", maxuploadsize);
	else
	{
		cfg_debug(ERROR, "maxuploadsize expects a number (integer)!");
		maxuploadsize = DCC_MAXFILESIZE;
	}
}

void	set_globaldebug(char *s)
/*
 * Sets global debug value.
 * Syntax: debug INTEGER
 */
{
	int	value;

	if(readint(&s, &value))
        {
		if(set_debuglvl(value))
			cfg_debug(NOTICE, "Setting debug to %d", value);
		else
			cfg_debug(ERROR, "debug expects a number 0..2 (integer)!");
		
	}
	else
		cfg_debug(ERROR, "debug expects a number 0..2 (integer)!");
}

void	set_maintainer(char *s)
/*
 * Sets the name of the botmaintainer
 * Syntax: maintainer STRING
 */
{
	char	name[MAXLEN];

	if(readstring(&s, name))
        {
		if(botmaintainer)
			free(botmaintainer);
		mstrcpy(&botmaintainer, name);
		cfg_debug(NOTICE, "Setting maintainer to \"%s\"", botmaintainer);
	}
	else
		cfg_debug(ERROR, "maintainer expects a name (string)!");
}

void	set_var(char *s)
/*
 * Assigns value to variable.
 * Syntax: %set VARIABLE VALUE [, VALUE [, ...]]
 */
{
	int	i;
        char    command[MAXLEN];;

	if(not(readident(&s, command)))
	{
		cfg_debug(ERROR, "Setting expected after %%SET");
		return;
	}
	for(i=0; setting_cmds[i].name; i++)
		if(STRCASEEQUAL(setting_cmds[i].name, command))
		{
			skipspc(&s);
			setting_cmds[i].function(s);
			return;
		}	
	cfg_debug(ERROR, "Unknown setting %s", command);
}

void	parsecommand(char *s)
{
        int     i;
        char    command[MAXLEN];

        s++;	/* skip % */
	if(not(readident(&s, command)))
		return;
        for(i=0; config_cmds[i].name != NULL; i++)
                if(STRCASEEQUAL(config_cmds[i].name, command))
                {
			skipspc(&s);
                        config_cmds[i].function(s);
                        return;
                }
        cfg_debug(ERROR, "ERROR: unknow command %%%s", command);
        return;
}

void	parsedef(char *s)
/*
 * BotNick	Definition	Values
 * (botnick created)
 * Definition	Values
 * (default bot set)
 */
{
	char	firsttok[MAXLEN];
	char	command[MAXLEN];
	botinfo	*defbot = NULL;
	listinfo *deflist = NULL;
	int	i;

	skipspc(&s);
	if(not(readident(&s, firsttok)))
		return;

/* BotNick has higer priority (because command could be nick) */
	if((defbot = bot_created(firsttok)) || 
            (deflist = listset_created(firsttok)))
	{
		if(not(readident(&s, command)))
		{
			cfg_debug(ERROR, "Definition expected!");
			return;
		}
		skipspc(&s);
		for(i=0; definition_cmds[i].name; i++)
			if(STRCASEEQUAL(definition_cmds[i].name, command))
			{ 
				if(definition_cmds[i].bot_def)
				{
					if(defbot == NULL)
					{
						cfg_debug(ERROR, "No default bot set!");
						return;
					}
					else
						definition_cmds[i].function(defbot, s);
				}
				else
					if(deflist == NULL)
					{
						cfg_debug(ERROR, "No default list set!");
						return;
					}
					else
						definition_cmds[i].function(deflist, s);
				return;
			}
		if (command[0]!='l' && !isdigit(command[1]))
			cfg_debug(ERROR, "Unknown definition %s", command);
	}
	else
	{
		if(!defaultbot && !defaultset)
		{
			cfg_debug(ERROR, "ERROR: No default bot/listset set!");
			return;
		}	
                strcpy(command, firsttok);
                for(i=0; definition_cmds[i].name; i++)
                        if(STRCASEEQUAL(definition_cmds[i].name, command))
                        {
				skipspc(&s);
       				if(definition_cmds[i].bot_def)
				{	
					if(defaultbot == NULL)
					{
						cfg_debug(ERROR, "No default bot set!");
						return;
					}
					definition_cmds[i].function(defaultbot, s);
				}
				else
				{
					if(defaultset == NULL)
					{
						cfg_debug(ERROR, "No default list set!");
						return;
					}
					definition_cmds[i].function(defaultset, s);
	                        }
				return;
                        }
		if (command[0]!='l' && !isdigit(command[1]))
			cfg_debug(ERROR, "Unknown definition %s", command);
	}
}

void	parsecfg(char *s)
{
	KILLNEWLINE(s);
	skipspc(&s);
	switch(*s)
	{
	case '%':
		cfg_debug(NOTICE, ">> %s", s);
		parsecommand(s);
		break;
	case '#':
		break;
	default:
		cfg_debug(NOTICE, " > %s", s);
		parsedef(s);
		break;
	}
}

char	*readline(FILE *f, char *s)
{
	if(fgets(s,255,f))
		return s;
	return NULL;
}

void	readcfg()
{
	FILE	*infile;
	char	buf[255];

	defaultbot = NULL;
	defaultset = NULL;
	linenum = 0;
	dbg_lvl = QUIET;
	
	if(STREQUAL(configfile, "-"))
		infile = stdin;
	else if((infile = fopen(configfile, "r")) == NULL)
	{
		printf("Configfile '%s' could not be read!\n",
			configfile);
		exit(1);
	}
	while(readline(infile, buf))
	{
		linenum++;
		parsecfg(buf);
	}
	fclose(infile);
}

