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

char	ec_tmp[MAXLEN];

void ec_access(char **pp, char **rp, char *from, char *to)
{
	sprintf(ec_tmp,"%i",get_userlevel(from,to));
	*rp = Strcat(*rp,ec_tmp);
	*pp += 7;
}

void ec_cc(char **pp, char **rp, char *from, char *to)
{
	*rp = Strcat(*rp,(current->CurrentChan) ? current->CurrentChan->name : "(none)");
	*pp += 3;
}

void ec_channels(char **pp, char **rp, char *from, char *to)
{
	aChan	*Chan;

	*pp += 9;
	if ((Chan = current->Channels) == NULL)
	{
		*rp = Strcat(*rp,ERR_NOCHANNELS);
		return;
	}
	ec_tmp[0] = 0;
	for(;Chan;Chan=Chan->next)
	{
		if (Chan->active == CH_ACTIVE)
		{
			if (Chan->bot_is_op)
			{
				**rp = '@';
				(*rp)++;
			}
			*rp = Strcat(*rp,Chan->name);
			**rp = ' ';
			(*rp)++;
		}
	}
	**rp = 0;
}

#ifdef LINKING

void ec_links(char **pp, char **rp, char *from, char *to)
{
	aBotLink *bl,*bl2;
	int	i,j,x,y;

	y = 0;
	ec_tmp[0] = 0;
	bl = botlinkBASE;
	for (i=0;i<botlinkSIZE;i++)
	{
		if ((bl->used) && (bl->nodetype == LN_ENTITY))
		{
			if (bl->linktype == LN_UPLINK)
				sprintf(ec_tmp,"* %s [",bl->nick);
			else
				sprintf(ec_tmp,"= %s [",bl->nick);
			x = 0;
			bl2 = botlinkBASE;
			for (j=0;j<botlinkSIZE;j++)
			{
				if ((bl2->used) && (bl2->nodetype == LN_BOT) && (bl2->uplink == bl->ident))
				{
					if (x)
						Strcat(ec_tmp,",");
					Strcat(ec_tmp,bl2->nick);
					x++;
					y++;
				}
				bl2++;
			}
			Strcat(ec_tmp,"] ");
			*rp = Strcat(*rp,ec_tmp);
		}
		bl++;
	}
	if (y == 0)
		*rp = Strcat(*rp,TEXT_NOLINKS);
	*pp += 6;
}

#endif /* LINKING */

void ec_time(char **pp, char **rp, char *from, char *to)
{
	sprintf(ec_tmp,"%s",time2away(now));
	*rp = Strcat(*rp,ec_tmp);
	*pp += 5;
}

void ec_vars(char **pp, char **rp, char *from, char *to)
{
	char	*sp,c;
	int	i;

	c = *((*pp)+1);
	*pp += 5;

	sp = ec_tmp;
	while(**pp)
	{
		if (**pp == ')')
			break;
		*sp = **pp;
		(*pp)++;
		sp++;
	}
	if (**pp != ')')
		return;
	(*pp)++;
	*sp = 0;

	if (tolowertab[(Uchar)c] == 's')	/* check to see if it was $set(x) */
	{
		if ((i = find_var_name(ec_tmp,INT_VAR)) >= 0)
		{
			sprintf(ec_tmp,"%i",get_int_var(to,i));
			*rp = Strcat(*rp,ec_tmp);
			return;
		}
		if ((i = find_var_name(ec_tmp,STR_VAR)) >= 0)
		{
			*rp = Strcat(*rp,nullstr(get_str_var(to,i)));
			return;
		}
	}
	if ((i = find_var_name(ec_tmp,TOG_VAR)) >= 0)
	{
			*rp = Strcat(*rp,(get_int_var(to,i)) ? "on" : "off");
			return;
	}
};

void ec_on(char **pp, char **rp, char *from, char *to)
{
	sprintf(ec_tmp,"%s",idle2str(now - current->ontime,FALSE));
	*rp = Strcat(*rp,ec_tmp);
	*pp += 3;
}

void ec_up(char **pp, char **rp, char *from, char *to)
{
	sprintf(ec_tmp,"%s",idle2str(now - current->uptime,FALSE));
	*rp = Strcat(*rp,ec_tmp);
	*pp += 3;
}

void ec_ver(char **pp, char **rp, char *from, char *to)
{
	*rp = Strcat(*rp,BOTCLASS);
	**rp = ' ';
	*rp = Strcat((*rp)+1,VERSION);
	*pp += 4;
}

struct
{
	char	*name;
	char	len;
	void	(*function)(char **, char **, char *, char *);
}
ecmd[] =
{
	{ "$access",	7,	ec_access	},
	{ "$cc",	3,	ec_cc		},
	{ "$channels",	9,	ec_channels	},
#ifdef LINKING
	{ "$links",	6,	ec_links	},
#endif /* LINKING */
	{ "$time",	5,	ec_time		},
	{ "$tog(",	5,	ec_vars		},
	{ "$set(",      5,      ec_vars         },
	{ "$on",	3,	ec_on		},
	{ "$up",	3,	ec_up		},
	{ "$ver",	4,	ec_ver		},
	{ NULL,		0,	NULL		},
};

void do_esay(char *from, char *to, char *rest, int cmdlevel)
{
	char	res[MAXLEN];
	char	*ptr,*rs,*chan;
	Uchar	ch,*chp;
	int	i,rlen;

	chan = get_channel(to,&rest);

	if (!rest)
	{
		usage(from,C_ESAY);
		return;
	}

	if (get_userlevel(from,chan) < cmdlevel)
		return;

	memset(ec_tmp,0,sizeof(ec_tmp));
	memset(res,0,sizeof(res));

	ptr = rest;
	rlen = strlen(ptr);
	rs = res;
	ch = 0;
	chp = NULL;
	while(*ptr)
	{
		if (*ptr == '$')
		{
			for(i=0;ecmd[i].name;i++)
			{
				if (rlen > ecmd[i].len)
				{
					chp = ptr + ecmd[i].len;
					ch = *chp;
					*chp = 0;
				}
				if (!Strcasecmp(ecmd[i].name,ptr))
				{
					if (ch && (*chp = ch))
						ch = 0;
					ecmd[i].function(&ptr,&rs,from,chan);
					rlen = strlen(ptr);
					break;
				}
				if (ch && (*chp = ch))
					ch = 0;
			}
			if (ecmd[i].name == NULL)
			{
				*(rs++) = *(ptr++);
			}
		}
		else
		{
			*(rs++) = *(ptr++);
		}
	}
	sendprivmsg(chan,"%s",res);
}

char *subststrvar(char *line)
{
	static char res[MAXLEN];
	char *rs;
	
	memset(res,0,sizeof(res));
	rs = res;
	
	while(*line)
	{
		if (*line == '$')
		{
			if (!strncasecmp(line,"$nick",5))
			{
				strcpy(rs,current->nick);
				while (*rs) rs++;
				line += 5;
			}
			else					
			{
				*(rs)++ = *(line)++;
			}
		}
		else
		{
			*(rs)++ = *(line)++;
		}
	}
	return(res);
}

void do_pickup(char *from, char *to, char *rest, int cmdlevel)
{
	char	*chan,*nick,*pickup;

	nick = chop(&rest);
	chan = get_channel(to,&rest);

	if ((pickup = randstring(RANDPICKUPFILE)) == NULL)
	{
		send_to_user(from,ERR_FILEOPEN,RANDPICKUPFILE);
		return;
	}
	if (rest)
	{
		to_server("PRIVMSG %s :%s",nick,pickup);
		send_to_user(from,"(%s) %s",nick,pickup);
		return;
	}
	if (ischannel(to))
	{
		sendprivmsg(chan,"%s",pickup);
		return;
	}
	usage(from,C_PICKUP);
}

void do_insult(char *from, char *to, char *rest, int cmdlevel)
{
	char	*chan,*nick,*insult;

	nick = chop(&rest);
	chan = get_channel(to, &rest);

	if ((insult = randstring(RANDINSULTFILE)) == NULL)
	{
		send_to_user(from,ERR_FILEOPEN,RANDINSULTFILE);
		return;
	}
	if (rest)
	{
		to_server("PRIVMSG %s :%s",nick,insult);
		send_to_user(from,"(%s) %s",nick,insult);
		return;
	}
	if(ischannel(to))
	{
		sendprivmsg(chan,"%s",insult);
		return;
	}
	usage(from,C_INSULT);
}

void do_core(char *from, char *to, char *rest, int cmdlevel)
{
	aServer *sp;
	aUser	*User;
	aChan	*Chan;
	char	tmp[MAXLEN];
	int	i,u,su,bu;

	u = su = bu = 0;
	User = current->userlistBASE;
	for(i=0;i<current->userlistSIZE;i++)
	{
		if ((User->used) && (User->type == USERLIST_USER))
		{
			u++;
			if (User->access == OWNERLEVEL)
				su++;
			if (User->access == BOTLEVEL)
				bu++;
		}
		User++;
	}

	send_to_user(from,"[\037Core Information\037]\n");
	if (strcmp(current->nick,current->wantnick))
		send_to_user(from,TEXT_CURNICKWANTED,current->nick,current->wantnick);
	else
		send_to_user(from,TEXT_CURNICK,current->nick);
	send_to_user(from,TEXT_USERSSUPER,
		u,su,EXTRA_CHAR(su),bu,EXTRA_CHAR(bu));

	tmp[0] = i = u = 0;
	Chan = current->Channels;
	while(Chan)
	{
		if (u && (strlen(tmp) + strlen(Chan->name) > 57))
		{
			if (!i)
				send_to_user(from,TEXT_ACTIVECHANS,tmp);
			else
				send_to_user(from,"                   %s",tmp);
			tmp[0] = 0;
			i++;
		}
		Strcat(tmp," ");
		if (Chan->bot_is_op)
			Strcat(tmp,"@");
		if (Chan == current->CurrentChan)
			sprintf(tmp,"%s\037%s\037",tmp,Chan->name);
		else
			Strcat(tmp,Chan->name);
		u++;
		Chan = Chan->next;
	}
	if (!i)
		send_to_user(from,TEXT_ACTIVECHANS,tmp);
	else
		send_to_user(from,"                   %s",tmp);
	if (!u)
	{
		send_to_user(from,TEXT_ACTIVECHANSNONE);
	}

	if (current->vhost[0])
	{
		switch(current->vhost_type)
		{
		case VH_IPALIAS:
			send_to_user(from,TEXT_VHOSTACTIVE,current->vhost); 
			break;
		case VH_IPALIAS_FAIL:
			send_to_user(from,TEXT_VHOSTINACTIVE,current->vhost); 
			break;
#ifdef WINGATES
		case VH_WINGATE:
			send_to_user(from,TEXT_VHOSTGATEACTIVE,
				current->vhost,current->vhost_port); 
			break;
		case VH_WINGATE_FAIL:
			send_to_user(from,TEXT_VHOSTGATEINACTIVE,
				current->vhost,current->vhost_port);
			break;
#endif /* WINGATES */
		}
	}
	sp = find_aServer(current->server);
	if (sp)
		send_to_user(from,TEXT_CURSVR,
			(sp->realname[0]) ? sp->realname : sp->name,sp->port);
	else
		send_to_user(from,TEXT_CURSVRNOTINLIST);
	send_to_user(from,TEXT_SVRONTIME,idle2str(now - current->ontime,FALSE));
	send_to_user(from,TEXT_SVRMODE,
		(current->modes[0]) ? current->modes : "(none)");
#ifdef LINKING
#define ENAME	"Entity Name         %s"
	if (my_entity[0])
		send_to_user(from,(my_pass[0]) ? ENAME " (Linkpass: %s)" : ENAME,
			my_entity,my_pass);
	if (linkport != -1)
		send_to_user(from,TEXT_LINKPORT,linkport,
			(linksock != -1) ? "(Active)" : "(Inactive)");
#endif /* LINKING */
	send_to_user(from,TEXT_CURTIME2,time2str(now));
	send_to_user(from,TEXT_STARTED,time2str(uptime));
	send_to_user(from,TEXT_UPTIME,idle2str(now - uptime,FALSE));
	send_to_user(from,TEXT_VERSION,VERSION,SRCDATE);
	send_to_user(from,TEXT_FEATURES,(__mx_opts[0]) ? __mx_opts : "(core only)");
	send_to_user(from,TEXT_CMDCHAR,current->cmdchar);

	send_to_user(from,TEXT_COREEND);
}

void do_cserv(char *from, char *to, char *rest, int cmdlevel)
{
	aServer	*sp;

	sp = find_aServer(current->server);
	if (sp)
		send_to_user(from,TEXT_CURSVR,
			(sp->realname[0]) ? sp->realname : sp->name,sp->port);
	else
		send_to_user(from,TEXT_CURSVRNOTINLIST);
}

void do_qshit(char *from, char *to, char *rest, int cmdlevel)
{
	char	*chan, *nick, *uh, *uh2;
	char	whoshit[MAXHOSTLEN+NAMELEN];
	int	uaxs,shaxs;

	nick = chop(&rest);

	chan = DEFAULTSHITCHAN;
	if (!nick)
	{
		usage(from,C_QSHIT);
		return;
	}

	uaxs = get_userlevel(from,chan);
	if (uaxs < cmdlevel)
		return;

	if (!(uh2 = nick2uh(from, nick, 0))) return;
#ifdef DEBUG
	debug("(do_qshit) shitting %s\n",uh2);
#endif /* DEBUG */

	if (is_shitted(uh2,chan))
	{
		send_to_user(from,TEXT_UHINSHIT,uh2);
		return;
	}
	shaxs = get_userlevel(uh2,chan);
	if ((shaxs >= uaxs) && (uaxs != OWNERLEVEL))
	{
		send_to_user(from,TEXT_NOSHITNOACCESS,uh2);
		return;
	}
	uh = format_uh(uh2,1);
	if (!uh)
		return;
	strcpy(whoshit,from);

	if (add_to_shitlist(uh,DEFAULTSHITLEVEL,chan,whoshit,
		(rest && *rest) ? rest : "Leave Lamer!",
		now,now + (86400 * DEFAULTSHITLENGTH)))
	{
		send_to_user(from,TEXT_USERSHITON, uh, chan);
		send_to_user(from,TEXT_SHITWILLEXPIRE,
			time2str(now + (86400 * DEFAULTSHITLENGTH)));
		check_shit();
	}
	else
		send_to_user(from,TEXT_PROBSHITUSER);
}

void do_wall(char *from, char *to, char *rest, int cmdlevel)
{
	aChanUser *User;
	aChan	*Chan;
	char	*nick,*channel;

	nick = getnick(from);
	channel = get_channel(to,&rest);

	if ((Chan = find_channel(channel,CH_ACTIVE)) == NULL)
	{
		send_to_user(from,ERR_CHAN,channel);
		return;
	}
	if (will.onotice)
	{
		to_server("NOTICE @%s :[Wallop/%s] %s",channel,channel,rest);
	}
	else
	{
		User = Chan->users;
		while(User)
		{
			if (get_chanopmode(User))
			{
				to_server("NOTICE %s :[Wallop/%s] %s",
					User->nick,channel,rest);
			}
			User = User->next;
		}
	}
	send_to_user(from,"Sent wallop to %s",channel);
}

void do_mode(char *from, char *to, char *rest, int cmdlevel)
{
	char	*channel;

	channel = chop(&rest);
	if (!channel || !rest)
	{
		usage(from,C_MODE);
		return;
	}

	if (get_userlevel(from,channel) < cmdlevel)
		return;

	sendmode(channel,"%s",rest);
}

void do_rehash2(char *from, char *to, char *rest, int cmdlevel)
{
	aBot	*bot;
	aChan	*Chan;

	backup = current;
	send_global_statmsg("[Rehash] from %s",getnick(from));
	for(bot=botlist;bot;bot=bot->next)
	{
		current = bot;
		send_statmsg("Saving Lists");
#ifdef DYNCMDLEVELS
		if (!write_levelfile())
			send_statmsg(ERR_LEVELSNOSAVE,LEVELFILE);
#endif /* DYNCMDLEVELS */
		if (!saveuserlist())
			send_statmsg(TEXT_NOSAVE,current->Userfile);
#ifdef SEEN
		if (!write_seenlist(&current->SeenList,current->seenfile))
			send_statmsg(TEXT_SEENNOSAVEFILE,current->seenfile);
		read_seenlist();
#endif /* SEEN */
		for(Chan=current->Channels;Chan;Chan=Chan->next)
			to_server("WHO %s",Chan->name);
	}
	current = backup;
}

void do_clearshit(char *from, char *to, char *rest, int cmdlevel)
{
	delete_shitlist();
	send_to_user(from,TEXT_SHITALLCLEAR);
}

void do_ctcp(char *from, char *to, char *rest, int cmdlevel)
{
	char	*target;

	target = chop(&rest);
	if (target && rest && *rest)
	{
		if (!Strcasecmp(rest,"PING"))
			send_ctcp(target,"PING %lu",now);
		else
			send_ctcp(target,"%s",rest);
	}
	else
		usage(from,C_CTCP);
}

void do_whom(char *from, char *to, char *rest, int cmdlevel)
{
#ifdef LINKING
	aBotLink *bl,*bl2;
	int	i,j;
#endif /* LINKING */
	aServer *sp;
	char	sps[NUHLEN];
	char	stt[NUHLEN];
	aDCC	*Client;
	aBot	*bot;
	int	m,s;

	send_to_user(from,TEXT_PARTYLINE);
	for(bot=botlist;bot;bot=bot->next)
	{
		sprintf(sps,"\037%s\037",bot->nick);
		if (bot->connect == CN_ONLINE)
		{
			sp = find_aServer(bot->server);
			if (sp)
			{
				sprintf(stt,"%s:%i",(sp->realname[0]) ? sp->realname : sp->name,sp->port);
			}
			else
			{
				strcpy(stt,TEXT_UNKNOWNSVR);
			}
		}
		else
		{
			strcpy(stt,TEXT_NOTCON);
		}
		send_to_user(from,"%-25s   %s",sps,stt);
		for(Client=bot->ClientList;Client;Client=Client->next)
		{
			if (Client->flags & DCC_CHAT)
			{
				m = (now - Client->lasttime) / 60;
				s = (now - Client->lasttime) % 60;
				sprintf(sps,"u%i",max_userlevel(Client->user));
				send_to_user(from,"* %-16s %3s   %s (idle %i min, %i sec)",
					getnick(Client->user),sps,getuh(Client->user),m,s);
			}
		}
	}
#ifdef LINKING
	bl = botlinkBASE;
	for(i=0;i<botlinkSIZE;i++)
	{
		if ((bl->used) && (bl->nodetype == LN_BOT))
		{
			bl2 = find_aBotLink(bl->uplink);
			sprintf(sps,"\037%s\037",bl->nick);
			send_to_user(from,"%-25s   (Linked; %s)",sps,bl2->nick);
			bl2 = botlinkBASE;
			for(j=0;j<botlinkSIZE;j++)
			{
				if ((bl2->used) && (bl2->nodetype == LN_DCCUSER) && (bl2->uplink == bl->uplink)
				    && (!Strcasecmp(bl2->nick,bl->nick)))
				{
					send_to_user(from,"* %s",bl2->dccnick);
				}
				bl2++;
			}
		}
		bl++;
	}
#endif /* LINKING */
	send_to_user(from,TEXT_ENDLIST);
}

void do_echo(char *from, char *to, char *rest, int cmdlevel)
{
	char	*tmp;

	tmp = chop(&rest);
	if (!tmp || !*tmp)
	{
		usage(from,C_ECHO);
		return;
	}
	if (!Strcasecmp(tmp,"on"))
	{
		CurrentUser->echo = TRUE;
		send_to_user(from,TEXT_PARTYECHOON);
		return;
	}
	if (!Strcasecmp(tmp,"off"))
	{
		CurrentUser->echo = FALSE;
		send_to_user(from,TEXT_PARTYECHOOFF);
		return;
	}
	usage(from,C_ECHO);
}

void do_vers(char *from, char *to, char *rest, int cmdlevel)
{
	send_to_user(from,"%s %s",BOTCLASS,VERSION);
}

void screwban_format(char *string)
{
	char	th[5] = "?*??";
	int	i,ii,iii,iiii;

#ifdef DEBUG
	debug("[screwban_format] %s\n",string);
#endif /* DEBUG */
	if ((i = (strlen(string)-5)) < 4) return;
	i = RANDOM(4,(i-5));
	for (ii = 0; ii < i; ii++)
	{
		iii = RANDOM(0,(strlen(string)-1));
		if (!strchr("!@*?",string[iii]))
		{
			iiii = RANDOM(0,3);
			*(string+iii) = th[iiii];
		}
	}
}

void deop_screwban(char *channel, char *nick, char *nuh)
{
	char	temp[MAXLEN];
	char	temp2[MAXLEN];

	if(!channel || !nick || !nuh)
		return;
	strcpy(temp,format_uh(nuh,1));
	strcpy(temp2,format_uh(nuh,1));
	screwban_format(temp);
	screwban_format(temp2);
	sendmode(channel,"-o+bb %s %s %s",nick,temp,temp2);
}

void do_last(char *from, char *to, char *rest, int cmdlevel)
{
	char	*pt,*thenum;
	int	i,num;

	if (!rest || !*rest)
		num = 5;
	else
	{
		thenum = chop(&rest);
		num = atoi(thenum);
	}
	if ((num < 1) || (num > LASTCMDSIZE))
		usage(from,C_LAST);
	else
	{
		pt = (char*)1;
		send_to_user(from,TEXT_LSTCMDSHEADER,num);
		for(i=0;i<num;i++)
		{
			if (!pt)
				break;
			pt = current->lastcmds[i];
			send_to_user(from,"%s",(pt) ? pt : "(none)");
		}
		send_to_user(from,TEXT_ENDLIST);
	}
}

void do_randomtopic(char *from, char *to, char *rest, int cmdlevel)
{
	aChan	*Chan;
	char	*channel,*topic;

	channel = get_channel(to,&rest);
	if (get_userlevel(from,channel) < cmdlevel)
		return;
	if ((Chan = find_channel(channel,CH_ACTIVE)) == NULL)
	{
		send_to_user(from,ERR_CHAN,channel);
		return;
	}
	if ((Chan->topprot == FALSE) || (Chan->bot_is_op))
	{
		topic = randstring(RANDTOPICSFILE);
		if (topic == NULL)
		{
			send_to_user(from,ERR_FILEOPEN,RANDTOPICSFILE);
			return;
		}
		to_server("TOPIC %s :%s",channel,topic);
		send_to_user(from,"Topic changed on %s",channel);
		return;
	}
	send_to_user(from,ERR_NOTOPPED,channel);
}

void do_screwban(char *from, char *to, char *rest, int cmdlevel)
{
	char	*chan, *nick, *nuh, *yuck;
	char	buffer[MAXLEN];
	int	ul;

	chan = get_channel(to,&rest);
	if ((ul = get_userlevel(from,chan)) < cmdlevel)
		return;
	if (!(nick = chop(&rest)))
	{
		usage(from,C_SCREW);
		return;
	}
	if (!isnick(nick))
	{
		send_to_user(from,ERR_NICK,nick);
		return;
	}
	if ((nuh = nick2uh(from,nick,0)) == NULL)
		return;

	if (get_userlevel(nuh,chan) > ul)
	{
		sendkick(chan,CurrentNick,TEXT_BANATTEMPTNUH,nuh);
		return;
	}

	yuck = randstring(RANDKICKSFILE);
	if ((!yuck) || (!*yuck))
		strcpy(buffer,TEXT_OUTYOUGO);
	else
		strcpy(buffer,yuck);
	deop_screwban(chan,nick,nuh);
	sendkick(chan,nick,"%s",(rest && *rest) ? rest : buffer);
}

void do_reset(char *from, char *to, char *rest, int cmdlevel)
{
	char	*rs;

	if ((rs = randstring(SIGNOFFSFILE)) == NULL)
		rs = "rehash!";
	close_all_dcc();
	do_exec = TRUE;
	kill_all_bots(rs);
	/* NOT REACHED */
}

void do_addserver(char *from, char *to, char *rest, int cmdlevel)
{
	if (strlen(rest) >= MAXHOSTLEN-1)
	{
		send_to_user(from,TEXT_HOSTNAMETOOLONG);
		return;
	}
	send_to_user(from,TEXT_SVRATTEMPTADD,rest);
	cfg_server(rest);
}

void do_delserver(char *from, char *to, char *rest, int cmdlevel)
{
	aServer *sp,*dp;
	char	*name,*port;
	int	i,n,iport,*ip;

	if (!serverBASE)
	{
		send_to_user(from,TEXT_SVRNOSVRS);
		return;
	}
	name = chop(&rest);
	port = chop(&rest);
	if (port)
	{
		ip = Atoi(port);
		if (!ip)
		{
			usage(from,C_DELSERVER);
			return;
		}
		iport = *ip;
	}
	else
		iport = 0;
	n = 0;
	dp = NULL;
	sp = serverBASE;
	for(i=0;i<serverSIZE;i++)
	{
		if ((sp->used) && ((!Strcasecmp(name,sp->name)) || (!Strcasecmp(name,sp->realname))))
		{
			if (iport)
			{
				if (sp->port == iport)
				{
					dp = sp;
					n++;
				}
			}
			else
			{
				dp = sp;
				n++;
			}
		}
		sp++;
	}
	switch(n)
	{
	case 0:
		if (iport)
			send_to_user(from,TEXT_NOMATCHSVRPORT,name,iport);
		else
			send_to_user(from,TEXT_NOMATCHSVR,name);
		break;
	case 1:
		send_to_user(from,TEXT_SVRDELETED,name,dp->port);
		memset(dp,0,sizeof(aServer));
		break;
	default:
		send_to_user(from,TEXT_SVRSEVERAL,name);
	}
}

void do_serverlist(char *from, char *to, char *rest, int cmdlevel)
{
	aServer *sp;
	char	*last;
	int	i;

	send_to_user(from,TEXT_SVRLISTHEADER);

	sp = serverBASE;
	for (i=0;i<serverSIZE;i++)
	{
		if (sp->used)
		{
			if (sp->lastconnect)
				last = idle2str(now - sp->lastconnect,FALSE);
			else
			{
				switch(sp->err)
				{
				case SP_NOAUTH:
					last = TEXT_SVRNOAUTH;
					break;
				case SP_KLINED:
					last = TEXT_SVRKLINE;
					break;
				case SP_FULLCLASS:
					last = TEXT_SVRCLASSFULL;
					break;
				case SP_TIMEOUT:
					last = TEXT_SVRTIMEOUT;
					break;
				case SP_ERRCONN:
					last = TEXT_SVRNOCON;
					break;
				case SP_DIFFPORT:
					last = TEXT_SVRDIFFPORT;
					break;
				default:
					last = TEXT_SVRNEVER;
				}
			}
			send_to_user(from,"%-30s   %-5i   %s%s%s",
				(sp->realname[0])?sp->realname:sp->name,sp->port,
				last,(sp->lastconnect) ? " ago" : "",
				(sp->ident == current->server) ? " (current)" : "");
		}
		sp++;
	}
	send_to_user(from,TEXT_SVRLISTFOOTER);
}

void do_away(char *from, char *to, char *rest, int cmdlevel)
{
	if (!rest || !*rest)
	{
		to_server("AWAY");
		send_to_user(from,TEXT_BACK);
		current->away = FALSE;
		current->activity = now;
		return;
	}
	to_server(AWAYFORM,rest,time2away(now));
	send_to_user(from,TEXT_AWAY);
	current->away = TRUE;
}

void do_servstats(char *from, char *to, char *rest, int cmdlevel)
{
	char	*line, *serv;
	aTime	*aVarX;

	line = chop(&rest);
	serv = chop(&rest);

	if (!line)
	{
		usage(from,C_STATS);
		return;
	}
	if (!serv)
		to_server("STATS %s", line);
	else
		to_server("STATS %s %s", line, serv);

	aVarX = make_time(&current->ParseList, from);
	aVarX->num = PA_STATS;
}

void do_whois_irc(char *from, char *to, char *rest, int cmdlevel)
{
	aTime	*aVarX;
	char	*nick;

	nick = chop(&rest);
	if (!nick)
	{
		usage(from,C_WHOIS);
		return;
	}
	else
		to_server("WHOIS %s", nick);

	aVarX = make_time(&current->ParseList, from);
	aVarX->num = PA_WHOIS;
}

void do_lusers(char *from, char *to, char *rest, int cmdlevel)
{
	aTime	*aVarX;

	to_server("LUSERS");

	aVarX = make_time(&current->ParseList, from);
	aVarX->num = PA_LUSERS;
}

void do_userhost(char *from, char *to, char *rest, int cmdlevel)
{
	aTime	*aVarX;
	char	*nick;

	if ((nick = chop(&rest)) == NULL)
	{
		usage(from,C_USERHOST);
		return;
	}
	aVarX = make_time(&current->ParseList,from);
	aVarX->num = PA_USERHOST;
	to_server("WHOIS %s",nick);
}

void do_dcc_kill(char *from, char *to, char *rest, int cmdlevel)
{
	aDCC 	*Client;
	
	if ((Client = search_list(from,DCC_CHAT)) != NULL)
	{
		send_to_user(from,TEXT_DCCKILL);
		Client->flags = DCC_DELETE;
	}
}

#define _UNDEF	-1
#define _ADD	1
#define _SUB	2

void do_user(char *from, char *to, char *rest, int cmdlevel)
{
	aUser	*User,*doer;
	char	*handle,*pt;
	char	mode;
	int	change,parms;
	int	av,ao,pm,pl,ec;
	av = ao = pm = ec = _UNDEF;
	pl = 0;

	handle = chop(&rest);

	if (!handle || !*handle)
		goto usage;

	if ((User = find_handle(handle)) == NULL)
	{
		send_to_user(from,TEXT_USERUNKNOWN,handle);
		return;
	}
	if ((doer = find_user(from,User->chanp)) == NULL)
	{
		send_to_user(from,TEXT_ACCESSDENIEDON,User->chanp);
		return;
	}

	/* lower access cant modif higher access */
	if ((doer->access < User->access) && (!superuser(doer)))
	{
		send_to_user(from,TEXT_ACCESSDENIEDHIGH,User->name);
		return;
	}

	mode = 0;
	change = FALSE;
	parms = 0;

	while(TRUE)
	{
		pt = chop(&rest);
		if (!pt || !*pt)
			break;
		switch(*pt)
		{
		case '+':
			mode = _ADD;
			break;
		case '-':
			mode = _SUB;
			break;
		default:
			goto usage;
		}
		pt++;
		if (!*pt)
			goto usage;

		if (!Strcasecmp(pt,"AV"))
			av = mode;
		else
		if (!Strcasecmp(pt,"AO"))
			ao = mode;
		else
		if (!Strcasecmp(pt,"ECHO"))
			ec = mode;
		else
		if ((*pt == 'P') || (*pt == 'p'))
		{
			pt++;
			if ((*pt >= '0') && (*pt <= '4') && (pt[1] == 0))
			{
				pm = mode;
				pl = *pt - '0';
			}
			else
			if ((*pt == 0) && (mode == _SUB))
			{
				pm = mode;
				pl = 0;
			}
			else
				goto usage;
		}
		else
			goto usage;
		parms++;
	}
	if (!parms)
		goto usage;

	/* implement the actual changes */

	/* +|- autovoice */
	if ((av == _ADD) && (User->avoice == FALSE))
	{
		User->avoice = TRUE;
		change++;
	}
	if ((av == _SUB) && (User->avoice == TRUE))
	{
		User->avoice = FALSE;
		change++;
	}

	/* +|- autoop */
	if ((ao == _ADD) && (User->aop == FALSE))
	{
		User->aop = TRUE;
		change++;
	}
	if ((ao == _SUB) && (User->aop == TRUE))
	{
		User->aop = FALSE;
		change++;
	}

	/* +|- echo */
	if ((ec == _ADD) && (User->echo == FALSE))
	{
		User->echo = TRUE;
		change++;
	}
	if ((ec == _SUB) && (User->echo == TRUE))
	{
		User->echo = FALSE;
		change++;
	}

	/* +|- protect [0..4] */
	if ((pm == _ADD) && (User->prot != pl))
	{
		User->prot = pl;
		change++;
	}
	if ((pm == _SUB) && (User->prot != 0))
	{
		User->prot = 0;
		change++;
	}

	if (change)
		send_to_user(from,TEXT_USERMOD,User->name);
	else
		send_to_user(from,TEXT_USERMODNO,User->name);
	return;
usage:
	usage(from,C_USER);
}

void do_cmdchar(char *from, char *to, char *rest, int cmdlevel)
{
	if (!rest || !*rest)
	{
		send_to_user(from,TEXT_CURCMDCHAR,current->cmdchar);
		return;
	}
	if (get_userlevel(from,NULL) != OWNERLEVEL)
		return;
	if (*rest && *(rest+1) == 0)
	{
		current->cmdchar = *rest;
		send_to_user(from,TEXT_CMDCHARSET,*rest);
		return;
	}
	send_to_user(from,TEXT_CMDCHARSINGLE);
}

void do_virtual(char *from, char *to, char *rest, int cmdlevel)
{
	char	*vhost;

	vhost = chop(&rest);
	if (!vhost || !*vhost)
	{
		usage(from,C_VIRTUAL);
		return;
	}
	if (strlen(vhost) >= MAXHOSTLEN-1)
	{
		send_to_user(from,TEXT_HOSTNAMEMAX,MAXHOSTLEN-1);
		return;
	}
	strcpy(current->vhost,vhost);
	current->vhost_type = VH_IPALIAS;
	send_to_user(from,TEXT_VHOSTSETTO,current->vhost);
	send_to_user(from,TEXT_VHOSTRESTARTBOT); 
}

#ifdef WINGATES

void do_wingate(char *from, char *to, char *rest, int cmdlevel)
{
	char	*wing,*ps;
	int	*ip;

	wing = chop(&rest);
	ps = chop(&rest);
	if (!ps || !*ps)
	{
		usage(from,C_WINGATE);
		return;
	}
	if (!strlen(wing) >= MAXHOSTLEN-1)
	{
		send_to_user(from,TEXT_HOSTNAMEMAX,MAXHOSTLEN-1);
		return;
	}
	ip = Atoi(ps);
	if (!ip || *ip < 1 || *ip > 65535)
	{
		send_to_user(from,TEXT_ENTITYINVALIDPORT); /* Once again I know it says ENTITY but the text is the same */
		return;
	}
	strcpy(current->vhost,wing);
	current->vhost_port = *ip;
	current->vhost_type = VH_WINGATE;
	send_to_user(from,TEXT_GATESETTO,
		current->vhost,current->vhost_port);
	send_to_user(from,TEXT_VHOSTRESTARTBOT); /* See above disclaimer! */
}

#endif /* WINGATES */

void do_shutdown(char *from, char *to, char *rest, int cmdlevel)   
{
	int	ul;

	if ((ul = get_userlevel(from,NULL)) < cmdlevel)
		return;
	send_global_statmsg(TEXT_SHUTDOWNBY,getnick(from),ul);
#ifdef SESSIONS
	savesession();
#endif /* SESSIONS */
	kill_all_bots(NULL);
	mechexit(0);
}
