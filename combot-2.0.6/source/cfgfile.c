/*
 * cfgfile.c - functions to handle the config file
 * (c) 1995 CoMSTuD (cbehrens@slavery.com)
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
#include <unistd.h>
#include <stdlib.h>

#include "global.h"
#include "config.h"
#include "defines.h"
#include "structs.h"
#include "h.h"

char cfgchannel[MAXLEN];
aChannel *Channel;
int commandtype = 0;

typedef struct CommandStruct
{
	char    *name;
	void    (*function)(char *args);
} aCommand;

aCommand config_cmds[] =
{
	/* Table with commands used to configure */
	{ "OWNER",      cfg_owner       },
	{ "USERSFILE",	cfg_usersfile	},
	{ "NICK",       cfg_nickname    },
	{ "CMDCHAR",    cfg_cmdchar     },
	{ "USERNAME",   cfg_login       },
	{ "IRCNAME",    cfg_ircname     },
	{ "CHANNEL",    cfg_channel     },
	{ "NOJOIN",	cfg_nojoin	},
	{ "TOPIC",      cfg_enftopic    },
	{ "TOG",	cfg_tog		},
	{ "SET",	cfg_set		},
	{ "SERVER",	cfg_server	},
	{ "LOG",	cfg_log		},
	{ NULL,		(void(*)(char *))(NULL) }
};

aCommand userlist_cmds[] =
{
	{ "USER",	cfg_adduser },
	{ "CHANNEL",	cfg_uchannel },
	{ "LEVEL",	cfg_access },
	{ "ACCESS",	cfg_access },
	{ "AOP",	cfg_aop },
	{ "PROT",	cfg_prot },
	{ "PASSWORD",	cfg_passwd },
	{ "GREET",	cfg_greet },
	{ "GREETING",	cfg_greeting },
	{ "KICKSAY",	cfg_addks },
	{ "SAYSAY",	cfg_addss },
	{ "SHIT",	cfg_addshit },
	{ "WHO",	cfg_who },
	{ "TIME",	cfg_time },
	{ "REASON",	cfg_reason },
	{ "EXPIRE",	cfg_expire },
	{ "PROTNICK",	cfg_protnick },
	{ NULL,		(void(*)(char *))(NULL) }
};

int readuserlist(filename)
char *filename;
{
        FILE *in;
        char lin[MAXLEN];
        char *ptr, *command;
        int i;

        if (!(in = fopen(filename, "rt")))
		return 0;
	delete_all_userlist(&Userlist);
        while (freadln(in, lin))
        {
                if (!*lin)
                        continue;
                ptr = stripl(lin, " \t");
                command = get_token(&ptr, "= ");
                if (!command)
                        continue;
                for (i=0;userlist_cmds[i].name;i++)
                        if (!my_stricmp(command, userlist_cmds[i].name))
                                break;
                if (userlist_cmds[i].name)
                        userlist_cmds[i].function(ptr);
                else
                     global_dbg(NOTICE, "Unknown command in userlist file: %s",
				command);
        }
        fclose(in);
	return 1;
}

void readcfgfile(void)
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
			config_cmds[i].function(ptr);
		else
			global_dbg(NOTICE, "Unknown command in config file: %s",
				command);
	}
	fclose(in);
	if (!number_of_bots)
	  {
	    printf("No bots in the configfile\n");
	    exit(1);
	  }
	if (Channel)
		copy_vars(DefaultVars, Channel->Vars);
}

void no_arg(command)
char *command;
{
	global_dbg(NOTICE, "No argument given for the %s command", command);
}

void cfg_usersfile(args)
char *args;
{
	char *thefile;

	if ((thefile=get_token(&args, ", ")) == NULL)
	{
		no_arg("USERSFILE");
		return;
	}
	strcpy(USERFILE, thefile);
}

void cfg_adduser(args)
char *args;
{
	char *theuser;
	aUser *tmp;
	if ((theuser=get_token(&args, ", ")) == NULL)
	{
		no_arg("USER");
		return;
	}	
	tmp = make_user(&Userlist);
	mstrcpy(&tmp->userhost, theuser);
	tmp->type |= USERLIST_USER;
}

void cfg_uchannel(args)
char *args;
{
	char *channel;

	if (!Userlist)
		return;
	if ((channel=get_token(&args, ", ")) == NULL)
	{
		no_arg("CHANNEL");
		return;
	}
	MyFree(&(Userlist->channel));
	mstrcpy(&Userlist->channel, channel);
}

void cfg_access(args)
char *args;
{
	int tmp;

	if (!Userlist)
		return;
	tmp = (args && *args) ? atoi(args) : 0;
	Userlist->access = (tmp < 0) ? 0 : (tmp > OWNERLEVEL) ? OWNERLEVEL:tmp; 
}

void cfg_aop(args)
char *args;
{
	if (!Userlist)
		return;
	Userlist->aop = (args && *args && atoi(args)) ? 1 : 0;
}

void cfg_prot(args)
char *args;
{
	int tmp;

	if (!Userlist)
		return;
	tmp = (args && *args) ? atoi(args) : 0;
	Userlist->prot = (tmp < 0) ? 0 : (tmp > MAXPROTLEVEL)?MAXPROTLEVEL:tmp;
}

void cfg_passwd(args)
char *args;
{
	char *passwd;

	if (!Userlist)
		return;
	if ((passwd = get_token(&args, ", ")) == NULL)
	{
		no_arg("PASSWD");
		return;
	}
	MyFree(&(Userlist->password));
	mstrcpy(&Userlist->password, passwd);
}

void cfg_greet(args)
char *args;
{
        char *theuser;
        aUser *tmp;
        if ((theuser=get_token(&args, ", ")) == NULL)
        {
                no_arg("GREET");
                return;
        }
        tmp = make_user(&Userlist);
        mstrcpy(&tmp->userhost, theuser);
        tmp->type |= USERLIST_GREET;
}

void cfg_greeting(args)
char *args;
{
	if (!Userlist)
		return;
	if (!args || !*args)
	{
		remove_user(&Userlist, Userlist);
		return;
	}
	MyFree(&Userlist->reason);
	mstrcpy(&Userlist->reason, args);
}

void cfg_addks(args)
char *args;
{
	aUser *tmp;

	if (!args || !*args)
	{
		no_arg("KICKSAY");
		return;
	}
	tmp = make_user(&Userlist);
	mstrcpy(&tmp->userhost, args);
	tmp->type |= USERLIST_KICKSAY;
}

void cfg_addss(args)
char *args;
{
	aUser *tmp;

	if (!args || !*args)
	{
		no_arg("SAYSAY");
		return;
	}
	tmp = make_user(&Userlist);
	mstrcpy(&tmp->userhost, args);
	tmp->type |= USERLIST_SAYSAY;
}

void cfg_addshit(args)
char *args;
{
        char *theuser;
        aUser *tmp;
        if ((theuser=get_token(&args, ", ")) == NULL)
        {
                no_arg("SHIT");
                return;
        }
        tmp = make_user(&Userlist);
        mstrcpy(&tmp->userhost, theuser);
        tmp->type |= USERLIST_SHIT;
}

void cfg_who(args)
char *args;
{
        char *who;

        if (!Userlist)
                return;
        if ((who = get_token(&args, ", ")) == NULL)
        {
                no_arg("WHO");
                return;
        }
        MyFree(&(Userlist->whoadd));
        mstrcpy(&Userlist->whoadd, who);
}

void cfg_time(args)
char *args;
{
        char *time;

        if (!Userlist)
                return;
        if ((time = get_token(&args, ", ")) == NULL)
        {
                no_arg("TIME");
                return;
        }
	Userlist->time = atol(time);
}

void cfg_reason(args)
char *args;
{
	if (!Userlist)
		return;
	if (!args || !*args)
	{
		no_arg("REASON");
		return;
	}
	MyFree(&Userlist->reason);
	mstrcpy(&Userlist->reason, args);
}

void cfg_expire(args)
char *args;
{
        char *time;

        if (!Userlist)
                return;
        if ((time = get_token(&args, ", ")) == NULL)
        {
                no_arg("EXPIRE");
                return;
        }
        Userlist->expire = atol(time);
}

void cfg_protnick(args)
char *args;
{
        char *theuser;
        aUser *tmp;
        if ((theuser=get_token(&args, ", ")) == NULL)
        {
                no_arg("PROTNICK");
                return;
        }
        tmp = make_user(&Userlist);
        mstrcpy(&tmp->userhost, theuser);
        tmp->type |= USERLIST_PROTNICK;
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
	aBot *temp;

	if (args && *args)
	{
		if ((temp = add_bot(args)) != NULL)
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

void cfg_nojoin(args)
char *args;
{
	char *value;
	int num;

	if (!Channel)
		return;
	value = get_token(&args, " ");
        num = (value && *value) ? atoi(value) : 1;
	Channel->flags = num ? 0 : REJOIN_TYPE;
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
		Channel = find_oldchannel(temp);
	}
	else
		no_arg("CHANNEL");
}

void cfg_server(args)
char *args;
{
	char *temp, *port;
	int theport;

	temp = get_token(&args, " ");
	if (temp && *temp)
	{
		port = get_token(&args, ", ");
		theport = (port && *port && atoi(port)) ? atoi(port) : 6667;
		add_server(temp, theport);
		set_server(temp, theport);
	}
	else
		no_arg("SERVER");
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

void cfg_cmdchar(args)
char *args;
{
  if (args && *args)
    current->cmdchar = *args;
  else
    no_arg("CMDCHAR");
}

void cfg_set(args)
char *args;
{
	char *setname;
	int which;
	VarStruct *var;
	char *value;

	if (!(setname = get_token(&args, " =")))
	{
		no_arg("SET");
		return;
	}
	value = args;
	if ((which = find_var_name(setname, INT_VAR+STR_VAR)) == -1)
	{
		global_dbg(NOTICE, "Variable name doesn't exist:", setname);
		return;
	}
	if (!value || !*value)
	{
		global_dbg(NOTICE, "No value given for SET %s", setname);
		return;
	}
	if (IsGlobalType(&DefaultVars[which]))
		var = &DefaultVars[which];
	else
	{
		if (!Channel)
			return;
		var = find_var(Channel->name, which);
	}
	if (!var)
		return;
        if (IsIntType(var))
        {
                int num;
                num = atoi(value);
                if (num < var->min || num > var->max)
                        return;
                var->value = num;
                return;
        }
        mstrcpy(&var->strvalue, value);
}

void cfg_tog(args)
char *args;
{
        char *setname;
        int which, num;
        VarStruct *var;
        char *value;

        if (!(setname = get_token(&args, " ")))
        {
                no_arg("TOG");
                return;
        }
        value = args;
        if ((which = find_var_name(setname, TOG_VAR)) == -1)
        {
                global_dbg(NOTICE, "Variable name doesn't exist:", setname);
                return;
        }
	if (IsGlobalType(&DefaultVars[which]))
		var = &DefaultVars[which];
	else
	{
		if (!Channel)
			return;
		var = find_var(Channel->name, which);
	}
        if (!var)
                return;
	num = (value && *value) ? atoi(value) : 1;
	if (num)
		num = 1;
	var->value = num;
}

void cfg_log(args)
char *args;
{
	char *logtype, *filename;

        if (!(logtype = get_token(&args, " =")))
        {
                no_arg("LOG");
                return;
        }
	if (!(filename = get_token(&args, " ")))
		return;
	if (!*logtype || !*filename)
		return;
	if (!(set_logname(logtype, filename)))
                global_dbg(NOTICE, "Log file type doesn't exist:", logtype);
}
 
