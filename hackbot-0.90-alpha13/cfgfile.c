/*
 * cfgfile.c - functions to handle the config file
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
#include "config.h"
#include "function.h"
#include "hackbot.h"
#include "cfgfile.h"
#include "debug.h"
#include "channel.h"

extern char owneruserhost[MAXLEN];
char cfgchannel[MAXLEN];
char *configfile = CFGFILE;
CHAN_list *Channel;

extern int number_of_bots;
extern botinfo thebots[MAXBOTS];
extern botinfo *current;

struct
{
	char    *name;
	void    (*function)(char *args);
} config_cmds[] =
{
	/* Table with commands used to configure */
	{ "OWNER",      cfg_owner       },
	{ "NICK",       cfg_nickname    },
	{ "CMDCHAR",    cfg_cmdchar     },
	{ "USERNAME",   cfg_login       },
	{ "IRCNAME",    cfg_ircname     },
	{ "CHANNEL",    cfg_channel     },
	{ "MODE",       cfg_enfmodes    },
	{ "TOPIC",      cfg_enftopic    },
	{ "AOPTOG",     cfg_aoptog      },
	{ "ENFMTOG",    cfg_enfmtog     },
	{ "SHITTOG",    cfg_shittog     },
	{ "PROTTOG",    cfg_prottog     },
	{ "PUBTOG",     cfg_pubtog      },
	{ "MASSTOG",    cfg_masstog     },
	{ "TOPTOG",     cfg_toptog      },
	{ "SDTOG",      cfg_sdtog       },
	{ "IKTOG",      cfg_iktog       },
	{ "IDLETIME",   cfg_idletime    },
	{ NULL,         (void(*)())(NULL) }
};

int readcfgfile(void)
{
	FILE *in;
	char lin[MAXLEN];
	char *ptr, *command;
	int i;

	Channel = NULL;
	strcpy(cfgchannel, "");
	init_bots();
	
	if (!(in = fopen(configfile, "rt")))
	  {
	    printf("Couldn't open: %s\n", configfile);
	    exit(1);
	  }

	while (freadln(in, lin))
	{
		if (!*lin)
			continue;
		ptr = stripl(lin, " \t");
		command = get_token(&ptr, "= ");
		if (!command)
			continue;
		for (i=0;config_cmds[i].name;i++)
			if (!my_stricmp(command, config_cmds[i].name))
				break;
		if (config_cmds[i].name)
		  {
			config_cmds[i].function(ptr);
			global_dbg(NOTICE, "command: %s", config_cmds[i].name);
		      }
		else
			global_dbg(NOTICE, "Unknown command in config file: %s", command);
	}
	fclose(in);
	if (!number_of_bots)
	  {
	    printf("No bots in the configfile\n");
	    exit(1);
	  }
}

void no_arg(command)
char *command;
{
	global_dbg(NOTICE, "No argument given for the %s command", command);
}

void cfg_owner(args)
char *args;
{
	if (args && *args)
		strcpy(owneruserhost, args);
	else
		no_arg("OWNER");
}

void cfg_nickname(args)
char *args;
{
	botinfo *temp;

	if (args && *args)
	{
		if (temp = add_bot(args))
			current = temp;
		else
			global_dbg(NOTICE, "Bot cannot be created: %s", args);
	}
	else
		no_arg("NICK");
}

void cfg_login(args)
char *args;
{
	char *temp;

	temp = get_token(&args, " ");
	if (temp && *temp)
		update_login(temp);
	else
		no_arg("USERNAME");
}

void cfg_ircname(args)
char *args;
{
	if (args && *args)
		update_ircname(args);
	else
		no_arg("IRCNAME");
}

void cfg_channel(args)
char *args;
{
	char *temp;

	temp = get_token(&args, " ");
	if (temp && *temp)
	{
		join_channel(temp, "", FALSE);
		strcpy(cfgchannel, temp);
		Channel = search_chan(temp);
	}
	else
		no_arg("CHANNEL");
}

void cfg_enfmodes(args)
char *args;
{
	char *temp;

	temp = get_token(&args, " ");
	if (temp && *temp)
		set_enfmodes(cfgchannel, temp);
	else
		no_arg("MODE");
}

void cfg_enftopic(args)
char *args;
{
	char *temp;

	temp = get_token(&args, " ");
	if (temp && *temp)
		set_enftopic(cfgchannel, temp);
	else
		no_arg("TOPIC");
}

void cfg_enfmtog(args)
char *args;
{
	char *temp;

	temp = get_token(&args, " ");

	if (!Channel)
		return;

	if (temp && *temp)
		Channel->enfmtoggle = atoi(temp) ? 1 : 0;
	else
		Channel->enfmtoggle = 1;
}

void cfg_shittog(args)
char *args;
{
	char *temp;

	temp = get_token(&args, " ");
	if (!Channel)
		return;

	if (!Channel)
		return;

	if (temp && *temp)
		Channel->shittoggle = atoi(temp) ? 1 : 0;
	else
		Channel->shittoggle = 1;
}

void cfg_prottog(args)
char *args;
{
	char *temp;

	temp = get_token(&args, " ");

	if (!Channel)
		return;

	if (temp && *temp)
		Channel->prottoggle = atoi(temp) ? 1 : 0;
	else
		Channel->prottoggle = 1;
}

void cfg_pubtog(args)
char *args;
{
	char *temp;

	temp = get_token(&args, " ");

	if (!Channel)
		return;

	if (temp && *temp)
		Channel->pubtoggle = atoi(temp) ? 1 : 0;
	else
		Channel->pubtoggle = 1;
}

void cfg_idletime(args)
char *args;
{
  char *temp;

  temp = get_token(&args, " ");
  if (temp && *temp)
    current->idlelevel = atoi(temp) >= 120 ? atoi(temp) : 600;
  else
    current->idlelevel = 600;
}

void cfg_sdtog(args)
char *args;
{
	char *temp;

	temp = get_token(&args, " ");

	if (!Channel)
		return;

	if (temp && *temp)
		Channel->sdtoggle = atoi(temp) ? 1 : 0;
	else
		Channel->sdtoggle = 1;
}

void cfg_iktog(args)
char *args;
{
  char *temp;

  temp = get_token(&args, " ");

	if (!Channel)
		return;

	if (temp && *temp)
		Channel->iktoggle = atoi(temp) ? 1 : 0;
	else
		Channel->iktoggle = 1;
}

void cfg_aoptog(args)
char *args;
{
	char *temp;

	temp = get_token(&args, " ");

	if (!Channel)
		return;
	if (temp && *temp)
		Channel->aoptoggle = atoi(temp) ? 1 : 0;
	else
		Channel->aoptoggle = 1;
}

void cfg_masstog(args)
char *args;
{
	char *temp;

	temp = get_token(&args, " ");

	if (!Channel)
		return;
	if (temp && *temp)
		Channel->masstoggle = atoi(temp) ? 1 : 0;
	else
		Channel->masstoggle = 1;
}

void cfg_toptog(args)
char *args;
{
	char *temp;

	temp = get_token(&args, " ");

	if (!Channel)
		return;
	if (temp && *temp)
		Channel->toptoggle = atoi(temp) ? 1 : 0;
	else
		Channel->toptoggle = 1;
}

void cfg_cmdchar(args)
char *args;
{
  char temp;
  
  if (args && *args)
    current->cmdchar = *args;
  else
    no_arg("CMDCHAR");
}








