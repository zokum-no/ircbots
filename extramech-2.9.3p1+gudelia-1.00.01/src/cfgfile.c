/*

    EnergyMech, IRC bot software
    Parts Copyright (c) 1997-2001 proton, 2002-2003 emech-dev

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/
#include "config.h"

#define TOC " =\t\r\n"

#ifndef MEGA
extern	OnMsg mcmd[];
#endif

aChan	*cfgChan;
int	cfgUser;

#ifdef NEWBIE

void cfg_die(char *args)
{
	printf("init: %s\n",args);
	mechexit(0);
}

#endif /* NEWBIE */

#ifdef LINKING

void cfg_linkconf(char *args)
{
	char	*name,*pass,*host,*ps;
	int	port;

	if ((name = get_token(&args,TOC)) == NULL)
		return;
	if ((pass = get_token(&args,TOC)) == NULL)
		return;
	host = get_token(&args,TOC);
	if ((ps = get_token(&args,TOC)))
		port = atoi(ps);
	else
		port = 0;
	if (!host || !*host)
	{
		host = "none";
		port = 0;
	}
	addlinkconf(name,pass,host,port);
}

void cfg_autolink(char *args)
{
	will.autolink = TRUE;
}

#endif /* LINKING */

void cfg_nosignals(char *args)
{
	will.signals = FALSE;
}

void cfg_hasonotice(char *args)
{
	will.onotice = TRUE;
}

void cfg_noshellcmd(char *args)
{
	will.shellcmd = FALSE;
}

#ifdef SEEN

void cfg_noseen(char *args)
{
	will.seen = FALSE;
}

#endif /* SEEN */

void cfg_usersfile(char *args)
{
	char	*fname;

	if ((fname = get_token(&args,TOC)) == NULL)
		return;
	strcpy(current->Userfile,fname);
}

void cfg_nickname(char *args)
{
	aBot	*temp;
	char	*nick;

	if ((current) && (current->Channels == NULL))
	{
		printf(ERR_NOTJOINCHANS,ERR_INIT,current->wantnick);
	}
	cfgChan = NULL;
	nick = get_token(&args,TOC);
	if (nick && *nick)
	{
		if (!isnick(nick))
		{
			printf(ERR_ILLEGALNICK,nick);
			mechexit(1);
		}
		if ((temp = add_bot(nick)) != NULL)
		{
			current = temp;
		}
		else
		{
			printf(ERR_COULDNTADD,nick);
			mechexit(1);
		}
	}
}

void cfg_login(char *args)
{
	char	*login;

	if (!current)
	{
		printf(ERR_STRAYLOGIN,ERR_INIT);
		return;
	}
	login = get_token(&args,TOC);
	if (login || *login)
	{
		strncpy(current->login,login,MAXLOGINLEN);
		current->login[MAXLOGINLEN] = 0;
	}
}

void cfg_ircname(char *args)
{
	if (!current)
	{
		printf(ERR_STRAYIRCNAME,ERR_INIT);
		return;
	}
	if (args && *args)
	{
		strncpy(current->ircname,args,NAMELEN-1);
		current->ircname[NAMELEN-1] = 0;
	}
}

void cfg_channel(char *args)
{
	char	*channel;
	char	*key;

	if ((channel = get_token(&args,TOC)) == NULL)
		return;
	key = get_token(&args,TOC);
	if (!ischannel(channel))
	{
		printf(ERR_INVALIDCHANNEL,ERR_INIT);
		return;
	}
	if (key && !*key)
		key = NULL;
	join_channel(channel,key,FALSE);
	cfgChan = find_channel(channel,CH_ANY);
}

void cfg_server(char *args)
{
	char	*host,*aport,*pass;
	int	port,*ip;

	host = get_token(&args,TOC);
	if (host && *host)
	{
		aport = get_token(&args,TOC);
		pass = get_token(&args,TOC);
		if (aport && *aport == '#')
		{
			aport = pass = NULL;
		}
		if (pass && *pass == '#')
		{
			pass = NULL;
		}
		ip = Atoi(aport);
		if (!ip)
			port = 6667;
		else
			port = *ip;
		add_server(host,port,pass);
	}
}

void cfg_sigmaster(char *args)
{
	char	*fmt;

	fmt =	(sigmaster) ? ERR_MULTISIGMASTER :
		(!current) ? ERR_STRAYSIGMASTER : NULL;
	if (fmt)
	{
		printf(fmt,ERR_INIT);
		return;
	}
	current->sigmaster = TRUE;
	sigmaster++;
}

void cfg_enftopic(char *args)
{
	char	*topic;

	if (cfgChan == NULL)
		return;
	topic = get_token(&args,TOC);
	if (topic && *topic)
		set_enftopic(cfgChan->name,topic);
}

void cfg_cmdchar(char *args)
{
	if (!current)
	{
		printf(ERR_STRAYCMDCHAR,ERR_INIT);
		return;
	}
	if (args && *args)
		current->cmdchar = *args;
}

void cfg_set(char *args)
{
	VarStruct *var;
	char	*setname,*value;
	int	which;
	int	num;

	if (!current)
	{
		printf(ERR_STRAYSET,ERR_INIT);
		return;
	}

	if ((setname = get_token(&args,TOC)) == NULL)
		return;
	value = args;
	if ((which = find_var_name(setname,INT_VAR+STR_VAR)) == -1)
	{
		printf(ERR_UNKNOWNSET,ERR_INIT,setname,configfile);
		return;
	}
	if (!value || !*value)
		return;
	if (IsGlobal(&current->DefaultVars[which]))
		var = &current->DefaultVars[which];
	else
	{
		if (cfgChan == NULL)
		{
			printf(ERR_SETNOACTIVECHAN,ERR_INIT,setname);
			return;
		}
		var = find_var(cfgChan->name,which);
	}
	if (!var)
		return;
	if (IsInt(var))
	{
		num = atoi(value);
		if (num < var->min || num > var->max)
			return;
		var->value = num;
		return;
	}
	set_mallocdoer(cfg_set);
	mstrcpy(&var->strvalue,value);
}

void cfg_tog(char *args)
{
	VarStruct *var;
	char	*setname;
	char	*value;
	int	which,num;

	if (!current)
	{
		printf(ERR_STRAYTOG,ERR_INIT);
		return;
	}
	if ((setname = get_token(&args,TOC)) == NULL)
		return;
	value = args;
	if ((which = find_var_name(setname,TOG_VAR)) == -1)
	{
		printf(ERR_UNKNOWNTOG,ERR_INIT,setname,configfile);
		return;
	}
	if (IsGlobal(&current->DefaultVars[which]))
		var = &current->DefaultVars[which];
	else
	{
		if (cfgChan == NULL)
		{
			printf(ERR_TOGNOACTIVECHAN,ERR_INIT,setname);
			return;
		}
		var = find_var(cfgChan->name,which);
	}
	if (!var)
		return;
	num = (value && *value) ? atoi(value) : 1;
	if (num)
		num = 1;
	var->value = num;
}

void cfg_modes(char *args)
{
	char	*tmp;

	if (!current)
	{
		printf(ERR_STRAYMODE,ERR_INIT);
		return;
	}

	tmp = get_token(&args,TOC);
	if (!tmp || !*tmp)
		return;
	strcpy(current->modes_to_send,tmp);
}

void cfg_virtual(char *args)
{
	char	*tmp;

	if (!current)
	{
		printf(ERR_STRAYVIRTUAL,ERR_INIT);
		return;
	}

	tmp = get_token(&args,TOC);
	if (!tmp || !*tmp)
		return;
	strcpy(current->vhost,tmp);
	current->vhost_type = VH_IPALIAS;
}

#ifdef WINGATES

void cfg_wingate(char *args)
{
	char	*host,*ap;
	int	*ip;

	if (!current)
	{
		printf(ERR_STRAYWINGATE,ERR_INIT);
		return;
	}
	host = get_token(&args,TOC);
	if ((ap = get_token(&args,TOC)) == NULL)
		return;
	ip = Atoi(ap);
	if (ip && *ip)
	{
		strcpy(current->vhost,host);
		current->vhost_port = *ip;
		current->vhost_type = VH_WINGATE;
	}
	else
		printf(ERR_WINGATENOPORT);
}

#endif /* WINGATES */

#ifdef LINKING

void cfg_linkport(char *args)
{
	char	*tmp;

	tmp = get_token(&args,TOC);
	if (!tmp || !*tmp)
		return;
	linkport = atoi(tmp);
}

void cfg_linkpass(char *args)
{
	char	*tmp;

	if ((tmp = get_token(&args,TOC)) == NULL)
		return;
	if (strlen(tmp) >= ENTITYLEN)
		return;
	strcpy(my_pass,tmp);
}

void cfg_entity(char *args)
{
	char	*tmp;

	if ((tmp = get_token(&args, TOC)) == NULL)
		return;
	if (strlen(tmp) >= ENTITYLEN)
		return;
	strcpy(my_entity,tmp);
}

#endif /* LINKING */

/*
 *   Userlist_cmds
 */

void cfg_addhandle(char *args)
{
	aUser	User;
	char	*name;

	if ((name = get_token(&args,TOC)) == NULL)
		return;

	memset(&User,0,sizeof(aUser));

	strncpy(User.name,name,NICKBUF);
	User.name[NICKBUF] = 0;

	User.used = TRUE;
	User.ident = cfgUser = userident++;
	User.type = USERLIST_USER;

	add_aUser(&User);
}

void cfg_addmask(char *args)
{
	aUser	*User;
	char	*mask;

	if ((mask = get_token(&args,TOC)) == NULL)
		return;
	if ((User = find_aUser(cfgUser)) == NULL)
		return;
	addmasktouser(User,mask);
}

void cfg_uchannel(char *args)
{
	aUser	*User;
	char	*channel;

	if ((channel = get_token(&args,TOC)) == NULL)
		return;
	if ((User = find_aUser(cfgUser)) == NULL)
		return;
	set_mallocdoer(cfg_uchannel);
	mstrcpy(&User->chanp,channel);
}

void cfg_access(char *args)
{
	aUser	*User;
	char	*as;
	int	level;

	if ((as = get_token(&args,TOC)) == NULL)
		return;
	if ((User = find_aUser(cfgUser)) == NULL)
		return;
	level = atoi(as);
	if (level < 0)
		return;
	if ((level > OWNERLEVEL) && (level != BOTLEVEL))
		return;
	User->access = level;
}

void cfg_aop(char *args)
{
	aUser 	*User;
	char	*as;

	if ((User = find_aUser(cfgUser)) == NULL)
		return;
	if ((as = get_token(&args,TOC)) == NULL)
	{
		User->aop = TRUE;
		return;
	}
	if (as[1] != 0)
		return;
	switch(as[0])
	{
	case '0':
		User->aop = FALSE;
		break;
	case '1':
		User->aop = TRUE;
		break;
	default:
		return;
	}
}

void cfg_avoice(char *args)
{
	aUser 	*User;
	char	*as;

	if ((User = find_aUser(cfgUser)) == NULL)
		return;
	if ((as = get_token(&args,TOC)) == NULL)
	{
		User->avoice = TRUE;
		return;
	}
	if (as[1] != 0)
		return;
	switch(as[0])
	{
	case '0':
		User->avoice = FALSE;
		break;
	case '1':
		User->avoice = TRUE;
		break;
	default:
		return;
	}
}

void cfg_echo(char *args)
{
	aUser	*User;

	if ((User = find_aUser(cfgUser)) == NULL)
		return;
	User->echo = TRUE;
}

void cfg_prot(char *args)
{
	aUser	*User;
	char	*as;
	int	prot;

	if ((as = get_token(&args,TOC)) == NULL)
		return;
	if ((User = find_aUser(cfgUser)) == NULL)
		return;
	prot = atoi(as);
	if (prot < 0)
		return;
	if (prot > MAXPROTLEVEL)
		return;
	User->prot = prot;
}

void cfg_passwd(char *args)
{
	aUser	*User;
	char	*pass;

	if ((pass = get_token(&args,TOC)) == NULL)
		return;
	if ((User = find_aUser(cfgUser)) == NULL)
		return;
	strncpy(User->pass,pass,PASSLEN);
	User->pass[PASSLEN] = 0;
}

void cfg_addks(char *args)
{
	aUser	User;

	memset(&User,0,sizeof(aUser));
	User.used = TRUE;
	User.ident = cfgUser = userident++;
	User.type = USERLIST_KICKSAY;
	set_mallocdoer(cfg_addks);
	mstrcpy(&User.parm.kicksay,args);
	add_aUser(&User);
}

void cfg_addshit(char *args)
{
	aUser	User;
	char	*name;

	if ((name = get_token(&args,TOC)) == NULL)
		return;

	memset(&User,0,sizeof(aUser));
	User.used = TRUE;
	User.ident = cfgUser = userident++;
	User.type = USERLIST_SHIT;
	set_mallocdoer(cfg_addshit);
	mstrcpy(&User.parm.shitmask,name);
	add_aUser(&User);
}

void cfg_who(char *args)
{
	aUser	*User;
	char	*who;

	if ((who = get_token(&args,TOC)) == NULL)
		return;
	if ((User = find_aUser(cfgUser)) == NULL)
		return;
	if (User->type != USERLIST_USER)
	{
		set_mallocdoer(cfg_who);
		mstrcpy(&User->whoadd,who);
	}
}

void cfg_time(char *args)
{
	aUser	*User;
	char	*ts;

	if ((ts = get_token(&args,TOC)) == NULL)
		return;
	if ((User = find_aUser(cfgUser)) == NULL)
		return;
	if (User->type != USERLIST_USER)
		User->time = atol(ts);
}

void cfg_reason(char *args)
{
	aUser	*User;

	if (!args || !*args)
		return;
	if ((User = find_aUser(cfgUser)) == NULL)
		return;
	if (User->type != USERLIST_USER)
	{
		set_mallocdoer(cfg_reason);
		mstrcpy(&User->reason,args);
	}
}

void cfg_expire(char *args)
{
	aUser	*User;
	char	*ts;

	if ((ts = get_token(&args,", ")) == NULL)
		return;
	if ((User = find_aUser(cfgUser)) == NULL)
		return;
	if (User->type != USERLIST_USER)
		User->expire = atol(ts);
}

#ifdef ALIASES
void cfg_alias(char *args)
{
	add_alias(NULL, args, 0);
}
#endif /* ALIASES */

#ifdef SERVICES
void cfg_service(char *args)
{
	char 	 	*to,*msg;
	aService 	**servicesptr;
	
	to = get_token(&args,TOC);
	msg = args;
	
        if (!msg || !*msg) return;
        
	if (cfgChan) 
	    servicesptr = &(cfgChan->ServicesList);
	else 
	    servicesptr = &(current->ServicesList);
	
	if (find_service(servicesptr,to,msg))
	{
		printf(ERR_SERVICEDUPED,to);
		return;
	}
	
	if (!add_service(servicesptr,to,msg))
		printf(ERR_SERVICEADDFAIL,to);
	
	return;
}

#endif /* SERVICES */

typedef struct CommandStruct
{
	char	*name;
	void	(*function)(char *args);
	char	chop;

} aCommand;

aCommand config_cmds[] =
{
{ "HASONOTICE",	cfg_hasonotice,	0	},
{ "NOSIGNALS",	cfg_nosignals,	0	},
{ "NOSHELLCMD",	cfg_noshellcmd,	0	},
#ifdef SEEN
{ "NOSEEN",	cfg_noseen,	0	},
#else /* SEEN */
{ "NOSEEN",	NULL,		0	},
#endif /* SEEN */
#ifdef LINKING
{ "AUTOLINK",	cfg_autolink,	0	},
{ "ENTITY",	cfg_entity,	0	},
{ "LINKPASS",	cfg_linkpass,	0	},
{ "LINKPORT",	cfg_linkport,	0	},
{ "LINK",	cfg_linkconf,	0	},
#endif /* LINKING */
{ "USERFILE",	cfg_usersfile,	0	},
{ "NICK",	cfg_nickname,	0	},
{ "CMDCHAR",	cfg_cmdchar,	0	},
{ "LOGIN",	cfg_login,	0	},
{ "IRCNAME",	cfg_ircname,	0	},
{ "CHANNEL",	cfg_channel,	0	},
{ "TOPIC",	cfg_enftopic,	0	},
{ "TOG",	cfg_tog,	0	},
{ "SET",	cfg_set,	0	},
{ "SERVER",	cfg_server,	0	},
{ "SIGMASTER",	cfg_sigmaster,	0	},
{ "MODES",	cfg_modes,	0	},
{ "VIRTUAL",	cfg_virtual,	0	},
#ifdef WINGATES
{ "WINGATE",	cfg_wingate,	0	},
#endif /* WINGATES */
#ifdef NEWBIE
{ "DIE",	cfg_die,	0	},
#else /* NEWBIE */
{ "DIE",	NULL,		0	},
#endif /* NEWBIE */
#ifdef ALIASES
{ "ALIAS",	cfg_alias,	0	},
#endif /* ALIASES */
#ifdef SERVICES
{ "SERVICE",    cfg_service,    0       },
#endif /* SERVICES */
{ NULL,		NULL,		0	}
};

aCommand userlist_cmds[] =
{
{ "HANDLE",	cfg_addhandle,	0	},
{ "MASK",	cfg_addmask,	0	},
{ "CHANNEL",	cfg_uchannel,	0	},
{ "ACCESS",	cfg_access,	0	},
{ "AOP",	cfg_aop,	0	},
{ "AVOICE",	cfg_avoice,     0       },
{ "ECHO",	cfg_echo,	0	},
{ "PROT",	cfg_prot,	0	},
{ "PASS",	cfg_passwd,	0	},
{ "KICKSAY",	cfg_addks,	0	},
{ "SHIT",	cfg_addshit,	0	},
{ "WHO",	cfg_who,	0	},
{ "TIME",	cfg_time,	0	},
{ "REASON",	cfg_reason,	0	},
{ "EXPIRE",	cfg_expire,	0	},
{ NULL,		NULL,		0	}
};

int readuserlist_callback(char *line)
{
	char	*command;
	int	i;

	command = get_token(&line,TOC);
	for(i=0;userlist_cmds[i].name;i++)
	{
		if (!Strcasecmp(command,userlist_cmds[i].name))
			break;
	}
	if (userlist_cmds[i].name)
	{
		if (userlist_cmds[i].chop)
		{
			command = get_token(&line,TOC);
			line = command;
		}
		userlist_cmds[i].function(line);
	}
	return(FALSE);
}

int readuserlist(void)
{
	aUser	*User;
	int	i,in;

	if ((in = open(current->Userfile,O_RDONLY)) < 0)
		return(FALSE);

	User = current->userlistBASE;
	for(i=0;i<current->userlistSIZE;i++)
	{
		remove_user(User);
		User++;
	}

	readline(in,&readuserlist_callback);
	close(in);

	return(TRUE);
}

int readcfgfile_callback(char *line)
{
	char	*command;
	int	i;

	command = get_token(&line,TOC);
	if (!command || !*command || *command == '#')
		return(FALSE);
	for(i=0;config_cmds[i].name;i++)
	{
		if (!Strcasecmp(command,config_cmds[i].name))
			break;
	}
	if ((config_cmds[i].name) && (config_cmds[i].function))
	{
		config_cmds[i].function(line);
	}
	else
	{
		printf(ERR_UNKNOWNCONFIGITEM,command);
	}
	return(FALSE);
}

#ifdef __GNUC__
inline
#endif /* __GNUC__ */
void readcfgfile(void)
{
#ifdef MULTI
	aBot	*bot;
#endif /* MULTI */
	int	oc,in;

	cfgChan = NULL;
	current = NULL;
	cfgUser = 0;

#ifdef LINKING
	memset(my_entity,0,sizeof(my_entity));
	memset(my_pass,0,sizeof(my_pass));
#endif /* LINKING */

	oc = TRUE;
	in = -1;

#ifdef SESSIONS
	if (!strcmp(CFGFILE,configfile))
	{
		if ((in = open(SESSIONFILE,O_RDONLY)) >= 0)
		{
			printf(TEXT_RESTORESESSION);
			oc = FALSE;
		}
	}
#endif /* SESSIONS */

	if (oc)
	{
		if ((in = open(configfile,O_RDONLY)) < 0)
		{
			printf(ERR_OPENFILE,configfile);
			mechexit(1);
		}
	}

	readline(in,&readcfgfile_callback);
	close(in);

#ifdef MULTI
	if (!botlist)
#else /* MULTI */
	if (!current->created)
#endif /* MULTI */
	{
		printf(ERR_NOBOTSINCONFIG);
		mechexit(1);
	}
	/*
	 *  Select a default sigmaster (last added bot)
	 */
	if (!sigmaster)
	{
		botlist->sigmaster = TRUE;
		sigmaster++;
	}

#ifdef LINKING
	if ((linkport != -1) && !(my_entity[0]))
	{
		printf(ERR_ENTITYNOTSET,ERR_INIT);
		linkport = -1;
	}
#endif /* LINKING */
	if ((current) && (current->Channels == NULL))
		printf(ERR_NOTJOINCHANS,ERR_INIT,current->nick);

#ifdef MULTI
	oc = 0;
	printf(TEXT_MECHSADDEDMULTI);
	for(bot=botlist;bot;bot=bot->next)
	{
		if (oc > 30)
		{
			printf(", ...");
			break;
		}
		printf("%s%s",(oc > 0) ? ", " : "",bot->nick);
		oc += strlen(bot->nick);
	}
	printf(" ]\n");
#else /* MULTI */
	printf(TEXT_MECHSADDEDSINGLE,current->nick);
#endif /* MULTI */
}
