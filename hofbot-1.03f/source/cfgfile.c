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
#include "hofbot.h"
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
	{ "ENFTOPTOG",  cfg_enftoptog   },
	{ "RANQUOTETOG", cfg_ranquotetog },
	{ "SDTOG",      cfg_sdtog       },
	{ "IKTOG",      cfg_iktog       },
	{ "IDLETIME",   cfg_idletime    },
	{ "SERVER",	cfg_server	},
	{ "WKTOG",      cfg_wktog       },
	{ "GREETOG",    cfg_greetog     },
	{ "NICKTOG",    cfg_nicktog     },
	{ "RANTOPTOG",  cfg_rantoptog   },
	{ "NEWSTOG",    cfg_newstog     },
	{ "DELAYOPTOG", cfg_delayoptog  },
	{ "CHKPUBTOG",  cfg_chkpubtog   },
	{ "NICKTIME",   cfg_nicktime    },
	{ "ENFTOPTIME", cfg_enftoptime  },
	{ "RANTOPTIME", cfg_rantoptime  },
	{ "RANQUOTETIME", cfg_ranquotetime  },
        { "AWAYMSG",    cfg_setaway     },
        { "GREETMSG",   cfg_setgreet    },
        { "FL",    	cfg_setfl       },
        { "FPL",    	cfg_setfpl      },
        { "BKL",    	cfg_setbkl      },
        { "CKL",    	cfg_setckl      },
        { "MDL",    	cfg_setmdl      }, 
        { "MBL",    	cfg_setmbl      },
        { "MKL",    	cfg_setmkl      },
        { "MPL",    	cfg_setmpl      },
        { "NCL",    	cfg_setncl      },
        { "MAL",    	cfg_setmal      },
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

void cfg_setaway(args)
char *args;
{
        char *temp;

	temp = args;
	if (temp && *temp)
	   strcpy(current->awaymsg, temp);
	else
	   no_arg("SETAWAY");
}

void cfg_setgreet(args)
char *args;
{
        char *temp;

	temp = args;
	if (temp && *temp)
	   strcpy(current->greetmsg, temp);
	else
	   no_arg("GREETMSG");
}

void cfg_server(args)
char *args;
{
	char *temp;

	temp = get_token(&args, " ");
	if (temp && *temp)
	{
		add_server(temp);
		set_server(temp);
	}
	else
		no_arg("SERVER");
}

void cfg_enftopic(args)
char *args;
{
	char *temp;

	temp = args;
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

void cfg_enftoptog(args)
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


void cfg_ranquotetog(args)
char *args;
{
	char *temp;

	temp = get_token(&args, " ");

	if (!Channel)
		return;
	if (temp && *temp)
		Channel->ranquotetoggle = atoi(temp) ? 1 : 0;
	else
		Channel->ranquotetoggle = 1;
}

void cfg_delayoptog(args)
char *args;
{
	char *temp;

	temp = get_token(&args, " ");

	if (!Channel)
		return;
	if (temp && *temp)
		Channel->delayoptoggle = atoi(temp) ? 1 : 0;
	else
		Channel->delayoptoggle = 1;
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

void cfg_wktog(args)
char *args;
{
	char *temp;

	temp = get_token(&args, " ");

	if (!Channel)
		return;
	if (temp && *temp)
		Channel->wktoggle = atoi(temp) ? 1 : 0;
	else
		Channel->wktoggle = 1;
}
void cfg_nicktog(args)
char *args;
{
	char *temp;

	temp = get_token(&args, " ");
	if (temp && *temp)
		current->nicktoggle = atoi(temp) ? 1 : 0;
	else
		current->nicktoggle = 1;
}
void cfg_rantoptog(args)
char *args;
{
	char *temp;

	temp = get_token(&args, " ");
	if (temp && *temp)
		current->rantopictoggle = atoi(temp) ? 1 : 0;
	else
		current->rantopictoggle = 1;
}
void cfg_greetog(args)
char *args;
{
	char *temp;

	temp = get_token(&args, " ");
	if (temp && *temp)
		current->greetoggle = atoi(temp) ? 1 : 0;
	else
		current->greetoggle = 1;
}

void cfg_newstog(args)
char *args;
{
	char *temp;

	temp = get_token(&args, " ");
	if (temp && *temp)
		current->newstoggle = atoi(temp) ? 1 : 0;
	else
		current->newstoggle = 1;
}

void cfg_nicktime(args)
char *args;
{
  char *temp;

  temp = get_token(&args, " ");
  if (temp && *temp)
    current->nicklevel = atoi(temp) <= 120 ? atoi(temp) : 30;
  else
    current->nicklevel = 30;
}

void cfg_enftoptime(args)
char *args;
{
  char *temp;

  temp = get_token(&args, " ");
  if (temp && *temp)
    current->enftopiclevel = atoi(temp) <= 120 ? atoi(temp) : 30;
  else
    current->enftopiclevel = 30;
}

void cfg_rantoptime(args)
char *args;
{
  char *temp;

  temp = get_token(&args, " ");
  if (temp && *temp)
    current->rantopiclevel = atoi(temp) <= 120 ? atoi(temp) : 30;
  else
    current->rantopiclevel = 30;
}

void cfg_ranquotetime(args)
char *args;
{
  char *temp;

  temp = get_token(&args, " ");
  if (temp && *temp)
    current->ranquotelevel = atoi(temp) <= 120 ? atoi(temp) : 30;
  else
    current->ranquotelevel = 30;
}

void cfg_setfl(args)
char *args;
{
  char *temp;

  temp = get_token(&args, " ");
  if (temp && *temp)
    current->floodlevel = atoi(temp) <= 20 ? atoi(temp) : 5;
  else
    current->floodlevel = 5;
}

void cfg_setfpl(args)
char *args;
{
  char *temp;

  temp = get_token(&args, " ");
  if (temp && *temp)
    current->floodprotlevel = atoi(temp) <= 3 ? atoi(temp) : 1;
  else
    current->floodprotlevel = 1;
}

void cfg_setbkl(args)
char *args;
{
  char *temp;

  temp = get_token(&args, " ");
  if (temp && *temp)
    current->beepkicklevel = atoi(temp) <= 20 ? atoi(temp) : 2;
  else
    current->beepkicklevel = 2;
}

void cfg_setckl(args)
char *args;
{
  char *temp;

  temp = get_token(&args, " ");
  if (temp && *temp)
    current->capskicklevel = atoi(temp) <= 20 ? atoi(temp) : 2;
  else
    current->capskicklevel = 2;
}

void cfg_setmdl(args)
char *args;
{
  char *temp;

  temp = get_token(&args, " ");
  if (temp && *temp)
    current->massdeoplevel = atoi(temp) <= 20 ? atoi(temp) : 6;
  else
    current->massdeoplevel = 6;
}

void cfg_setmbl(args)
char *args;
{
  char *temp;

  temp = get_token(&args, " ");
  if (temp && *temp)
    current->massbanlevel = atoi(temp) <= 20 ? atoi(temp) : 6;
  else
    current->massbanlevel = 6;
}

void cfg_setmkl(args)
char *args;
{
  char *temp;

  temp = get_token(&args, " ");
  if (temp && *temp)
    current->masskicklevel = atoi(temp) <= 20 ? atoi(temp) : 6;
  else
    current->masskicklevel = 6;
}

void cfg_setmpl(args)
char *args;
{
  char *temp;

  temp = get_token(&args, " ");
  if (temp && *temp)
    current->massprotlevel = atoi(temp) <= 3 ? atoi(temp) : 2;
  else
    current->massprotlevel = 3;
}

void cfg_setncl(args)
char *args;
{
  char *temp;

  temp = get_token(&args, " ");
  if (temp && *temp)
    current->massnicklevel = atoi(temp) <= 20 ? atoi(temp) : 3;
  else
    current->massnicklevel = 3;
}

void cfg_setmal(args)
char *args;
{
  char *temp;

  temp = get_token(&args, " ");
  if (temp && *temp)
    current->malevel = atoi(temp) <= 99 ? atoi(temp) : 50;
  else
    current->malevel = 50;
}

void cfg_chkpubtog(args)
char *args;
{
	char *temp;

	temp = get_token(&args, " ");
	if (temp && *temp)
		current->chkpubtoggle = atoi(temp) ? 1 : 0;
	else
		current->chkpubtoggle = 1;
}
