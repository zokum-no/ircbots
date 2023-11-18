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
 * BUGS: If a setting/definition requires more that one paramter,
 *       problems will occur when a paramter follows a non-string
 *       parameter, i.e. (imaginary) %set number 10, 12, -4
 *       At this moment, there are no such settings/def's.
 *       (This bug has to do with the fact that get_token skips ',''s
 *       or will see them as a part of the parameter. get_string won't)
 *
 */
#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>

#include "cfgfile.h"
#include "debug.h"
#include "misc.h"
#include "vladbot.h"

int 	atoi(char *);

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
	{ "USERLIST",	FALSE,	set_listuser	},
	{ "PROTLIST",	FALSE,	set_listprot	},
	{ "SHITLIST",	FALSE,	set_listshit	},
	/* Bot definitions */
	{ "NICK",	TRUE,	set_nick	},
	{ "LOGIN",	TRUE,	set_login       },
	{ "NAME",	TRUE,	set_name        },
	{ "SERVER",	TRUE,	add_server      },
	{ "CHANNEL",	TRUE,	add_to_channellist     },
	{ "UPLOAD",	TRUE,	set_uploaddir	},
	{ "DOWNLOAD",	TRUE,	set_downloaddir	},
	{ "INDEX",	TRUE,	set_indexfile	},
	{ "HELP",	TRUE,	set_helpfile	},
	{ "LISTSET",	TRUE,	set_listset	},
	{ NULL,		0,	(void(*)())(NULL) }
};

struct
{
	char	*name;
	void    (*function)();
} setting_cmds[] =
{	
	{ "IDLETIMEOUT",	set_idletimeout },
	{ "WAITTIMEOUT",	set_waittimeout },
	{ "MAXUPLOADSIZE",	set_maxuploadsize },
	{ "NOTEFILE",		set_notefile 	},
	{ "DEBUG",		set_globaldebug	},
	{ "OWNERNICK",		set_ownernick	},
	{ "OWNERHOST",		set_ownerhost	},
	{ NULL,         	(void(*)())(NULL) }
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
extern	char	*ownerhost;
extern	char	*ownernick;

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
	int	nw_dbg;
	char	*dbg;
	
	if((dbg = get_token(&s, " \t")) != NULL)
	{
		nw_dbg = atoi(dbg);
		if(nw_dbg<QUIET || nw_dbg>NOTICE)
			cfg_debug(ERROR, "%%DEBUG expects 0 <= argument <= 2!");
		else
		{
			cfg_debug(NOTICE, "DEBUG set to %d", dbg_lvl);
			dbg_lvl = nw_dbg;
		}
	}
	else
		cfg_debug(ERROR, "%%DEBUG requires an argument!");
}

void	create_bot(char *s)
/*
 * Creates a bot.
 * Syntax: %bot STRING
 */
{
	char	*b_name;

	if((b_name = get_token(&s, " \t")) != NULL)
	{
		if(listset_created(b_name))
		{
			cfg_debug(ERROR, "There is already a listset called \"%s\"", b_name);
			return;
		}
		if(bot_created(b_name))
		{
			cfg_debug(ERROR, "\"%s\" is already created!", b_name);
			return;
		}
		if(!add_bot(b_name))
			cfg_debug(ERROR, "Too many bots created!");
		else
			cfg_debug(NOTICE, "CREATE: bot \"%s\"", b_name);
	}
	else
		cfg_debug(ERROR, "%%CREATE requires an identifier-argument!");
}

void	create_userlist(char *s)
/*
 * Create a userlist.
 * Syntax: %create STRING
 */
{
	char	*l_name;
	
	if((l_name = get_token(&s, " \t")) != NULL)
	{
		if(bot_created(l_name))
		{
			cfg_debug(ERROR, "There is already a bot called \"%s\"", l_name);
			return;
		}
		if(listset_created(l_name))
		{
			cfg_debug(ERROR, "\"%s\" is already created!", l_name);
			return;
		}
		if(!add_listset(l_name))
			cfg_debug(ERROR, "Too many listsets created!");
		else
			cfg_debug(NOTICE, "LISTSET: listset \"%s\"", l_name);
	}
	else
		cfg_debug(ERROR, "%%LISTSET requires an identifier-argument!");
}


void	set_default(char *s)
/*
 * Set default bot/listset for %with
 * Syntax: %with STRING
 */
{
	char	*b_name;

	if((b_name = get_token(&s, " \t")) != NULL)
	{
		if(bot_created(b_name))
		{
			cfg_debug(NOTICE, "DEFAULT: bot \"%s\"", b_name);
			defaultbot = bot_created(b_name);
		}
		else if(listset_created(b_name))
		{
			cfg_debug(NOTICE, "DEFAULT: listset \"%s\"", b_name);
			defaultset = listset_created(b_name);
		}
		else
			cfg_debug(ERROR, "\"%s\" is not created!", b_name);
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
	char	*l_name;
	char	*path;
	
	if((l_name = get_string(&s)) != NULL)
	{
		if(!(path = expand_twiddle(l_name)))
		{
			cfg_debug(ERROR, "Error in pathname \"%s\"!", l_name);
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
	char	*l_name;
	char	*path;
	
	if((l_name = get_string(&s)) != NULL)
	{
		if(!(path = expand_twiddle(l_name)))
		{
			cfg_debug(ERROR, "Error in pathname \"%s\"!", l_name);
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
	char	*l_name;
	char	*path;
	
	if((l_name = get_string(&s)) != NULL)
	{
		if(!(path = expand_twiddle(l_name)))
		{
			cfg_debug(ERROR, "Error in pathname \"%s\"!", l_name);
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

void	set_listset(botinfo *bot, char *s)
/*
 * Set the listset for bot.
 * Syntax: listset STRING
 */
{
	char	*s_name;
	
	if((s_name = get_token(&s, " \t")) != NULL)
	{
		if(listset_created(s_name))
		{
			bot->lists = listset_created(s_name);
			cfg_debug(NOTICE, "Setting lists for %s to \"%s\"",
				  bot->botname, s_name);
		}
		else
			cfg_debug(ERROR, "No listset \"%s\"!!", s_name);
	}
	else
		cfg_debug(ERROR, "%s listset expects a listset (string)!", 
			  bot->botname);
}

void	set_nick(botinfo *bot, char *s)
/*
 * Set the nickname for bot.
 * Syntax: nick STRING
 */
{
	char	*n_name;
	
	if((n_name = get_string(&s)) != NULL)
	{
		if(!isnick(n_name))
		{
			cfg_debug(ERROR, "Illegal nickname \"%s\"!", n_name);
			return;
		}
		cfg_debug(NOTICE, "Setting nick for %s to \"%s\"", 
			  bot->botname, n_name);
		strcpy(bot->nick, n_name);
		strcpy(bot->realnick, n_name);
	}
	else
		cfg_debug(ERROR, "%s nick expects a nick (string)!", 
			  bot->botname);
}

void	set_login(botinfo *bot, char *s)
/*
 * Set the loginname for bot.
 * Syntax: login STRING
 */
{
	char	*l_name;
	
	if((l_name = get_string(&s)) != NULL)
	{
		cfg_debug(NOTICE, "Setting login for %s to \"%s\"", 
                          bot->botname, l_name);
		strcpy(bot->login, l_name);
	}
	else
		cfg_debug(ERROR, "%s login expects a login!", bot->botname);
}

void	set_name(botinfo *bot, char *s)
/*
 * Set the name for bot.
 * Syntax: name STRING
 */
{  
	char	*n_name;
	
	if((n_name = get_string(&s)) != NULL)
	{
                cfg_debug(NOTICE, "Setting name for %s to \"%s\"", 
			  bot->botname, n_name);
		free(bot->name);
		mstrcpy(&bot->name, n_name);
	}
        else
                cfg_debug(ERROR, "%s name expects a name!", bot->botname);
}

void	add_server(botinfo *bot, char *s)
/*
 * Adds a server to the serverlist of bot
 * Syntax: server STRING [, INTEGER]
 *                (name)    (port)
 */
{  
	char	*s_name;
	char	*p;
	int	port = 0;
	
        if((s_name = get_string(&s)) != NULL)
	{
		SKIPSPC(s);
		if(*s == ',')		/* read port */
		{
			s++;
			p = get_token(&s, " \t");
			if(p && atoi(p))
				port = atoi(p);
			else
				cfg_debug(ERROR, 
					  "Port (integer) expected after ','!");
		}
		else
			port = 6667;
		
		if(find_server(bot, s_name, port))
			cfg_debug(ERROR, "Server \"%s\" port %d already in list!", 
				  s_name, port);
		else
			if(add_server_to_bot(bot, s_name, port))
				cfg_debug(NOTICE, 
                                          "Server \"%s\" with port %d added to \"%s\"",
					  s_name, port, bot->botname);
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
	char	*c_name;
	char	*mode = "";
	char	*topic = "";
	
	if((c_name = get_string(&s)) != NULL)
	{
		SKIPSPC(s);
		if(*s == ',')		/* read mode */
		{
			s++;
			SKIPSPC(s);
			if(*s != ',')	/* No mode, read topic */
				if((mode = get_string(&s)) == NULL)
				{
					mode = "";
					cfg_debug(ERROR, "Mode (string) expected!");
					return;
				}
			SKIPSPC(s);
			if(*s == ',')	/* read topic */
			{
				s++;
				SKIPSPC(s);
				if((topic = get_string(&s)) == NULL)
				{
					topic = "";
					cfg_debug(ERROR, "Topic (string) expected!");
					return;
				}
			}
		}
		if(find_channel(bot, c_name))
			cfg_debug(ERROR, "Channel \"%s\" already in list!", c_name);
		else
			if(add_channel_to_bot(bot, c_name, topic, mode))
				cfg_debug(NOTICE, "Channel \"%s\" \"%s\" \"%s\" added to %s", 
					  c_name, mode, topic, bot->botname);
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
	char	*u_name;
	char	*path;

	if((u_name = get_string(&s)) != NULL)
        {
       		if(!(path = expand_twiddle(u_name)))
		{
			cfg_debug(ERROR, "Error in pathname \"%s\"!", u_name);
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
	char	*d_dir;
	char	*path;

        if((d_dir = get_string(&s)) != NULL)
        {
       		if(!(path = expand_twiddle(d_dir)))
		{
			cfg_debug(ERROR, "Error in pathname \"%s\"!", d_dir);
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
	char	*i_file;
	char	*path;

	if((i_file = get_string(&s)) != NULL)
        {
       		if(!(path = expand_twiddle(i_file)))
		{
			cfg_debug(ERROR, "Error in pathname \"%s\"!", i_file);
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
	char	*h_file;
	char	*path;
	
	if((h_file = get_string(&s)) != NULL)
        {
       		if(!(path = expand_twiddle(h_file)))
		{
			cfg_debug(ERROR, "Error in pathname \"%s\"!", h_file);
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
void	set_idletimeout(char *s)
/*
 * Sets global idletimeout.
 * Syntax: idletimeout INTEGER
 */
{
	char	*i_val;

	if((i_val = get_token(&s, " \t")) != NULL)
	{
		cfg_debug(NOTICE, "Setting idletimeout to %d", atoi(i_val));
		idletimeout = atoi(i_val);
	}
	else
		cfg_debug(ERROR, "idletimeout expects a number (integer)!");
}

void	set_waittimeout(char *s)
/*
 * Sets global waittimeout.
 * Syntax: waittimeout INTEGER
 */
{
	char	*w_val;

	if((w_val = get_token(&s, " \t")) != NULL)
	{
		cfg_debug(NOTICE, "Setting waittimeout to %d", atoi(w_val));
		waittimeout = atoi(w_val);
	}
	else
		cfg_debug(ERROR, "waittimeout expects a number (integer)!");
}

void	set_maxuploadsize(char *s)
/*
 * Sets global maxuploadsize.
 * Syntax: maxuploadsize INTEGER
 */
{
	char	*m_val;

	if((m_val = get_token(&s, " \t")) != NULL)
        {
		cfg_debug(NOTICE, "Setting maxuploadsize to %d", atoi(m_val));
		maxuploadsize = atoi(m_val);
	}
	else
		cfg_debug(ERROR, "maxuploadsize expects a number (integer)!");
}

void	set_notefile(char *s)
/*
 * Sets global notefile.
 * Syntax: notefile STRING
 */
{
	char	*n_file;
	char	*path;

	if((n_file = get_string(&s)) != NULL)
        {
		if(!(path = expand_twiddle(n_file)))
		{
			cfg_debug(ERROR, "Error in pathname \"%s\"!", n_file);
			path = "";
		}		cfg_debug(NOTICE, "Setting notefile to \"%s\"", n_file);
		mstrcpy(&notefile, n_file);
	}
	else
		cfg_debug(ERROR, "notefile expects a pathname (string)!");
}

void	set_globaldebug(char *s)
/*
 * Sets global debug value.
 * Syntax: debug INTEGER
 */
{
	char	*d_val;

	if((d_val = get_token(&s, " \t")) != NULL)
        {
		if(set_debuglvl(atoi(d_val)))
			cfg_debug(NOTICE, "Setting debug to %d", atoi(d_val));
		else
			cfg_debug(ERROR, "debug expects a number 0..2 (integer)!");
		
	}
	else
		cfg_debug(ERROR, "debug expects a number 0..2 (integer)!");
}

void	set_var(char *s)
/*
 * Assigns value to variable.
 * Syntax: %set VARIABLE VALUE [, VALUE [, ...]]
 */
{
	int	i;
        char    *command;

        if((command = get_token(&s, " \t")) == NULL)
	{
		cfg_debug(ERROR, "Setting expected after %%SET");
		return;
	}
	for(i=0; setting_cmds[i].name; i++)
		if(STRCASEEQUAL(setting_cmds[i].name, command))
		{
			SKIPSPC(s);
			setting_cmds[i].function(s);
			return;
		}	
	cfg_debug(ERROR, "Unknown setting %s", command);
}


void	parsecommand(char *s)

{
        int     i;
        char    *command;

        s++;
        if((command = get_token(&s, " \t")) == NULL)
		return;
        for(i=0; config_cmds[i].name != NULL; i++)
                if(STRCASEEQUAL(config_cmds[i].name, command))
                {
			SKIPSPC(s);
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
	char	*firsttok;
	char	*command;
	botinfo	*defbot = NULL;
	listinfo *deflist = NULL;
	int	i;

	SKIPSPC(s);
	if((firsttok=get_token(&s, " \t")) == NULL)
		return;

/* BotNick has higer priority (because command could be nick) */
	if((defbot = bot_created(firsttok)) || 
            (deflist = listset_created(firsttok)))
	{
		if((command=get_token(&s, " \t")) == NULL)
		{
			cfg_debug(ERROR, "Definition expected!");
			return;
		}
		for(i=0; definition_cmds[i].name; i++)
			if(STRCASEEQUAL(definition_cmds[i].name, command))
			{
				SKIPSPC(s);
				if(definition_cmds[i].bot_def)
					if(defbot == NULL)
					{
						cfg_debug(ERROR,
						"No default bot set!");
						return;
					}
					else
						definition_cmds[i].function(defbot, s);
				else
					if(deflist == NULL)
					{
						cfg_debug(ERROR,
						"No default list set!");
						return;
					}
					else
						definition_cmds[i].function(deflist, s);
				return;
			}
		cfg_debug(ERROR, "Unknown definition %s", command);
	}
	else
	{
		if(!defaultbot && !defaultset)
		{
			cfg_debug(ERROR, "ERROR: No default bot/listset set!");
			return;
		}	
                command=firsttok;
                for(i=0; definition_cmds[i].name; i++)
                        if(STRCASEEQUAL(definition_cmds[i].name, command))
                        {
				SKIPSPC(s);
       				if(definition_cmds[i].bot_def)
					definition_cmds[i].function(defaultbot, s);
				else
					definition_cmds[i].function(defaultset, s);
	                        return;
                        }
		cfg_debug(ERROR, "Unknown definition %s", command);
	}
}


void	parsecfg(char *s)

{
	KILLNEWLINE(s);
	SKIPSPC(s);
	switch(*s)
	{
	case '%':
		cfg_debug(NOTICE, ">> %s", s);
		return(parsecommand(s));
		break;
	case '#':
		break;
	default:
		cfg_debug(NOTICE, " > %s", s);
		return(parsedef(s));
		break;
	}
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
void    set_ownerhost(char *s)
/*
 * Sets the name of the owners host
 * Syntax: ownerhost STRING
 */
{
	char *name;

	if((name = get_string(&s)) != NULL)
        {
                if(ownerhost)
                        free(ownerhost);
                mstrcpy(&ownerhost, name);
                cfg_debug(NOTICE, "Setting ownerhost to \"%s\"", ownerhost);
        }
        else
                cfg_debug(ERROR, "ownerhost expects a name (string)!");
}
void    set_ownernick(char *s)
/*
 * Sets the name of the owners nick
 * Syntax: ownernick STRING
 */
{
	char *name;
	if((name = get_string(&s)) != NULL)
        {
                if(ownernick)
                        free(ownernick);
                mstrcpy(&ownernick, name);
                cfg_debug(NOTICE, "Setting ownernick to \"%s\"", ownernick);
        }
        else
                cfg_debug(ERROR, "ownernick expects a name (string)!");
}

char    *readline(FILE *f, char *s)

{
        if(fgets(s,255,f))
                return s;
        return NULL;
}

