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

extern	OnMsg mcmd[];

#ifdef SESSIONS

#define Bdef	BinaryDefault

int savesession(void)
{
#ifdef LINKING
	aLinkConf *lc;
#endif /* LINKING */
#ifdef SERVICES
	aService *tmpserviceptr;
#endif /* SERVICES */

	VarStruct *Vars,*CV;
	aServer	*sp;
	aChan	*Chan;
#ifdef MULTI
	aBot	*bot;
#endif /* MULTI */
	int	i,j,sf;

	if ((sf = open(SESSIONFILE,O_WRONLY|O_CREAT|O_TRUNC,0666)) < 0)
		return(FALSE);

#ifdef LINKING
	if (my_entity[0])
		tofile(sf,"entity %s\n",my_entity);
	if (my_pass[0])
		tofile(sf,"linkpass %s\n",my_pass);
	if (linkport)
		tofile(sf,"linkport %i\n",linkport);

	lc = linkconfBASE;
	for(i=0;i<linkconfSIZE;i++)
	{
		if (lc->used)
		{
			tofile(sf,"link %s %s %s %i\n",lc->name,lc->pass,lc->host,lc->port);
		}
		lc++;
	}
#endif /* LINKING */

#ifdef 	SEEN
	if (!will.seen)
		tofile(sf,"noseen\n");
#endif /* SEEN */
	if (!will.signals)
		tofile(sf,"nosignals\n");
	if (!will.shellcmd)
		tofile(sf,"noshellcmd\n");
	if (will.onotice)
		tofile(sf,"hasonotice\n");
#ifdef LINKING
	if (will.autolink)
		tofile(sf,"autolink\n");
#endif /* LINKING */

	tofile(sf,"\n");

#ifdef MULTI
	for(bot=botlist;bot;bot=bot->next)
	{
#else /* MULTI */
#define bot current
#endif /* MULTI */
		tofile(sf,"nick %s\n",bot->wantnick);
		tofile(sf,"login %s\n",bot->login);
		if (bot->vhost[0])
		{
			switch(bot->vhost_type)
			{
			case VH_IPALIAS:
			case VH_IPALIAS_FAIL:
				tofile(sf,"virtual %s\n",bot->vhost);
				break;
#ifdef WINGATES
			case VH_WINGATE:
			case VH_WINGATE_FAIL:
				tofile(sf,"wingate %s %i\n",bot->vhost,bot->vhost_port);
				break;
#endif /* WINGATES */
			}
		}
		tofile(sf,"ircname %s\n",bot->ircname);
		if (bot->modes_to_send[0])
			tofile(sf,"modes %s\n",bot->modes_to_send);
#ifdef SERVICES
		tmpserviceptr = bot->ServicesList;
		while (tmpserviceptr)
		{
			tofile(sf, "service %s %s\n",
				tmpserviceptr->to,tmpserviceptr->msg);
			tmpserviceptr = tmpserviceptr->next;
		}
#endif /* SERVICES */
		if (bot->cmdchar != DEFAULTCMDCHAR)
			tofile(sf,"cmdchar %c\n",bot->cmdchar);
		tofile(sf,"userfile %s\n\n",bot->Userfile);
		/*
		 *  current->DefaultVars contains channel defaults and global vars
		 */
		Vars = current->DefaultVars;
		for(j=0;VarName[j].name;j++)
		{
			if (!IsGlobal(&Vars[j]))
				continue;
			CV = &Vars[j];
			if (IsInt(CV))
			{
				if (Bdef[j].value != CV->value)
					tofile(sf,"set %s %i\n",VarName[j].name,CV->value);
			}
			else
			if (IsStr(CV))
			{
				if ((CV->strvalue) && (Strcasecmp(Bdef[j].strvalue,CV->strvalue)))
					tofile(sf,"set %s %s\n",VarName[j].name,CV->strvalue);
			}
			else
			if (IsTog(CV))
			{
				if (Bdef[j].value != CV->value)
					tofile(sf,"tog %s %c\n",VarName[j].name,(CV->value) ? '1' : '0');
			}
		}

		for(Chan=bot->Channels;Chan;Chan=Chan->next)
		{
			if ((Chan->active == CH_OLD) && (!Chan->rejoin))
				continue;
			tofile(sf,"channel %s %s\n",Chan->name,(Chan->key && *Chan->key) ? Chan->key : "");
#ifdef SERVICES
                        tmpserviceptr = Chan->ServicesList;                                                                  
                        while (tmpserviceptr)                                                                                
                        {                                                                                                    
                                tofile(sf, "service %s %s\n",                                                                
                                        tmpserviceptr->to,tmpserviceptr->msg);                                               
                                tmpserviceptr = tmpserviceptr->next;                                                         
                        }                           
#endif /* SERVICES */
			Vars = Chan->Vars;
			for(j=0;VarName[j].name;j++)
			{
				if (IsGlobal(&Vars[j]))
					continue;
				CV = &Vars[j];
				if (IsInt(CV))
				{
					if (CV->value != Bdef[j].value)
						tofile(sf,"set %s %i\n",VarName[j].name,CV->value);
				}
				else
				if (IsStr(CV))
				{
					if ((CV->strvalue) && (Strcasecmp(Bdef[j].strvalue,CV->strvalue)))
						tofile(sf,"set %s %s\n",VarName[j].name,CV->strvalue);
				}
				else
				if (IsTog(CV))
				{
					if (CV->value != Bdef[j].value)
						tofile(sf,"tog %s %c\n",VarName[j].name,(CV->value) ? '1' : '0');
				}
			}
		}
#ifdef MULTI
	}
#else /* MULTI */
#undef bot
#endif /* MULTI */

#ifdef ALIASES
	tofile(sf,"\n");

	for(i=0;mcmd[i].name;i++)
	{
		if (mcmd[i].alias)
		{
			tofile(sf,"alias %s %s\n",mcmd[i].name,mcmd[i].alias);
		}
	}
#endif /* ALIASES */

	tofile(sf,"\n");

	sp = serverBASE;
	for(i=0;i<serverSIZE;i++)
	{
		if (sp->used)
		{
			tofile(sf,"server %s %i %s\n",sp->name,(sp->port) ? sp->port : 6667,
				(sp->pass[0]) ? sp->pass : "");
		}
		sp++;
	}

	close(sf);
	return(TRUE);
}

#endif /* SESSIONS */

/* dynamic limit support */
#ifdef DYNCHANLIMIT
void check_dynamic(aChan *Chan)
{
	aChanUser *cu;
	char	ascnum[11];
	int	n,wind;

	n = 0;

	for (cu=Chan->users;cu;cu=cu->next)
		n++;

	wind = n / 10;
	if (wind < 4)
		wind = 4;

	wind += n;

	n = wind - Chan->limit;

	if (((n < -2) || (n > 1)) || (Chan->limitmode == 0))
	{
		sprintf(ascnum,"%i",wind);
		sendmode(Chan->name,"+l %s",ascnum);
	}
}
#endif /* DYNCHANLIMIT */

/*
 *  Bot adding and killing
 */

#ifdef SERVICES
int add_service(aService **l_list, char *to, char *msg)
{
	aService 	*tmpserviceptr;

	if ((tmpserviceptr = (aService*)MyMalloc(sizeof(aService))) == NULL)
		return(FALSE);

	tmpserviceptr->next = *l_list;
	mstrcpy(&tmpserviceptr->to,to);
	mstrcpy(&tmpserviceptr->msg,msg);
	*l_list = tmpserviceptr;	
	return(TRUE);
}

int delete_service(aService **l_list, aService *service)
{
	aService 	**old;
	
	for(old=l_list;*old;old=&(**old).next)
		if (*old == service)
		{
			*old = service->next;
			MyFree(&service->to);
			MyFree(&service->msg);
			MyFree((char **)&service);
			return(TRUE);
		}
	return(FALSE);
}	

aService *find_service(aService **l_list, char *to, char *msg)
{
	aService 	*tmpserviceptr;
	
	tmpserviceptr = *l_list;
	
	while (tmpserviceptr)
	{
		if (!strcasecmp(to,tmpserviceptr->to))
			if (!strcasecmp(msg,tmpserviceptr->msg))
				return(tmpserviceptr);
		tmpserviceptr = tmpserviceptr->next;
	}
	return(NULL);			
}	

void delete_all_services(aService *ServicesList)
{
	while (ServicesList)
		delete_service(&ServicesList,ServicesList);
}
#endif /* SERVICES */

aBot *add_bot(char *nick)
{
#ifdef MULTI
	aBot	*bot;
#endif /* MULTI */
#ifdef SEEN
	char	buffer[FILENAMELEN];
#endif /* SEEN */

#ifdef MULTI
	set_mallocdoer(add_bot);
	bot = (aBot*)MyMalloc(sizeof(aBot));
#else /* MULTI */
#define bot current
	memset(bot,0,sizeof(aBot));
	bot->created++;
#endif /* MULTI */
	bot->connect = CN_NOSOCK;
	bot->sock = -1;
	bot->cmdchar = DEFAULTCMDCHAR;
	strcpy(bot->nick,nick);
	strcpy(bot->wantnick,nick);
	strcpy(bot->login,BOTLOGIN);
	sprintf(bot->ircname,"%s %s",BOTCLASS,VERSION);
	copy_vars(bot->DefaultVars,BinaryDefault);
#ifdef SEEN
	sprintf(buffer,BOTDIR "%s.seen",nick);
	set_mallocdoer(add_bot);
	mstrcpy(&bot->seenfile,buffer);
#endif /* SEEN */
	bot->uptime = now;
#ifdef MULTI
	bot->next = botlist;
	botlist = bot;
	return(bot);
#else /* MULTI */
#undef bot
	return(current);
#endif /* MULTI */
}

#ifdef MULTI

#ifdef SPAWN

aBot *forkbot(char *from, char *nick, char *rest)
{
	aChan	*Chan;
	aBot	*newbot;
	char	*option,*login,*channel,*server,*cmdchar,*ircname;
	char	*vhost,*port;
	int	error,iport,vt;
#ifdef WINGATES
	char	*vhp;
	int	vhpi = 0,*ip;
#endif /* WINGATES */

	error = vt = 0;
	vhost = port = cmdchar = ircname = login = server = channel = NULL;
	while(rest && *rest)
	{
		option = chop(&rest);
		if (*option == '-')
		{
			option++;
			switch(*option)
			{
				case 'c':
					channel = chop(&rest);
					break;
				case 'i':
					ircname = rest;
					rest = NULL;
					break;
				case 'l':
					login = chop(&rest);
					break;
				case 'p':
					port = chop(&rest);
					if (atoi(port) == 0)
						error++;
					break;
				case 's':
					server = chop(&rest);
					break;
				case 'v':
					vhost = chop(&rest);
					vt = VH_IPALIAS;
					break;
#ifdef WINGATES
				case 'w':
					vhost = chop(&rest);
					vt = VH_WINGATE;
					if ((vhp = strchr(vhost,':')) != NULL)
					{
						*vhp = 0;
						vhp++;
						if ((ip = Atoi(vhp)) == NULL)
							error++;
						else
							vhpi = *ip;
					}
					else
						error++;
					break;
#endif /* WINGATES */
				case 'z':
					cmdchar = chop(&rest);
					break;
				default:
					error++;
			}
		}
	}
	if (error)
	{
		usage(from,C_SPAWN);
		return(NULL);
	}

	if ((newbot = add_bot(nick)) != NULL)
	{
		backup = current;
		current = newbot;
		nick[MAXNICKLEN] = 0;
		strcpy(newbot->nick,nick);
		strcpy(newbot->wantnick,nick);
		strcpy(newbot->modes_to_send,current->modes_to_send);
		sprintf(newbot->Userfile,"users.%s",newbot->nick);

		if (login && *login)
		{
			strncpy(newbot->login,login,MAXLOGINLEN);
			newbot->login[MAXLOGINLEN] = 0;
		}
		else
			strcpy(newbot->login,current->login);

		if (ircname && *ircname)
		{
			strncpy(newbot->ircname,ircname,NAMELEN-1);
			newbot->ircname[NAMELEN-1] = 0;
		}
		else
			strcpy(newbot->ircname,current->ircname);

		if (vhost && *vhost)
		{
#ifdef WINGATES
			if (vt == VH_WINGATE)
				newbot->vhost_port = vhpi;
#endif /* WINGATES */
			strncpy(newbot->vhost,vhost,MAXHOSTLEN-1);
			newbot->vhost[MAXHOSTLEN-1] = 0;
			newbot->vhost_type = vt;
		}
		else
			strcpy(newbot->vhost,current->vhost);

		newbot->cmdchar = (cmdchar && *cmdchar) ? *cmdchar : current->cmdchar;
		newbot->server = current->server;
		if (port && *port)
			iport = atoi(port);
		else
			iport = 6667;
		if (server)
			add_server(server,iport,NULL);
		if (!channel)
		{
			for(Chan=backup->Channels;Chan;Chan=Chan->next)
			{
				if ((Chan->active == CH_ACTIVE) || (Chan->rejoin))
				{
					join_channel(Chan->name,Chan->key,FALSE);
					set_enftopic(Chan->name,Chan->topic);
				}
			}
		}
		else
			join_channel(channel,NULL,FALSE);
		current = backup;
		return(newbot);
	}
	return(NULL);
}

#endif /* SPAWN */

#endif /* MULTI */

void signoff(char *from, char *reason)
{
	aStrp	*sp,*spn;
	aUser	*User;
#ifdef MULTI
	aBot	*bot;
#endif /* MULTI */
	int	i;

	if (from)
	{
		send_to_user(from,TEXT_KILLINGMECH,current->nick);
		send_to_user(from,TEXT_SAVELISTS);
	}
	if (!saveuserlist() && from)
	{
		send_to_user(from,TEXT_NOSAVE,current->Userfile);
	}
#ifdef SEEN
	if (!write_seenlist(&current->SeenList,current->seenfile) && from)
	{
		send_to_user(from,ERR_SEENLISTNOSAVE,current->seenfile);
	}
#endif /* SEEN */
#ifdef DYNCMDLEVELS
	if (!write_levelfile() && from)
	{
		send_to_user(from,ERR_LEVELSNOSAVE,LEVELFILE);
	}
#endif /* DYNCMDLEVELS */
	if (from)
	{
		send_to_user(from,TEXT_SHUTDOWN);
	}

	close_all_dcc();
	delete_all_channels();

	if (current->sock != -1)
	{
		if (!reason || !*reason)
			reason = randstring(SIGNOFFSFILE);
		to_server("QUIT :%s",(reason && *reason) ? reason : "");
		killsock(current->sock);
		current->sock = -1;
	}

#ifdef PIPEUSER
#ifdef DEBUG
	debug(TEXT_REMOVEPIPEUSER);
#endif /* DEBUG */
	if (current->pipeuser && current->pipeuser->sock >= 0)
		killsock(current->pipeuser->sock);
	MyFree((char**)&current->pipeuser);
	MyFree(&current->saved_002);
	MyFree(&current->saved_003);
	MyFree(&current->saved_004);
#endif /* PIPEUSER */

#ifdef SEEN
#ifdef DEBUG
	debug(TEXT_REMOVESEENLIST);
#endif /* DEBUG */
	delete_seen();
	MyFree(&current->seenfile); 
#endif /* SEEN */
#ifdef SERVICES
	delete_all_services(current->ServicesList);
#endif /* SERVICES */

#ifdef TRIVIA                                                                                                                
        write_triviascore();                                                                                         
#endif /* TRIVIA */

#ifdef DEBUG
	debug(TEXT_REMOVEUSERLIST);
#endif /* DEBUG */
	User = current->userlistBASE;
	for(i=0;i<current->userlistSIZE;i++)
	{
		MyFree((char**)&User->chanp);
		if (User->type == USERLIST_USER)
		{
			sp = User->parm.umask;
			while(sp)
			{
				spn = sp->next;
				MyFree((char**)&sp);
				sp = spn;
			}
		}
		if ((User->type == USERLIST_SHIT) || (User->type == USERLIST_KICKSAY))
		{
			MyFree((char**)&User->whoadd);
			MyFree((char**)&User->reason);
		}
		if (User->type == USERLIST_SHIT)
		{
			MyFree((char**)&User->parm.shitmask);
		}
		if (User->type == USERLIST_KICKSAY)
		{
			MyFree((char**)&User->parm.kicksay);
		}
		User++;
	}
	MyFree((char**)&current->userlistBASE);

#ifdef DEBUG
	debug(TEXT_REMOVELASTCMD);
#endif /* DEBUG */
	for(i=0;i<LASTCMDSIZE;i++)
	{
		MyFree(&current->lastcmds[i]);
	}

	delete_time(&current->ParseList);

	/*
	 *  These 2 are used by do_die() to pass reason and doer.
	 */
	MyFree((char**)&current->signoff);
	MyFree((char**)&current->from);

#ifdef MULTI
#ifdef DEBUG
	debug(TEXT_UNLINKBOTRECORD);
#endif /* DEBUG */
	if (current == botlist)
	{
		botlist = current->next;
	}
	else
	for(bot=botlist;bot;bot=bot->next)
	{
		if (bot->next == current)
		{
			bot->next = current->next;
			break;
		}
	}
	MyFree((char**)&current);
	if ((current = botlist) == NULL)
	{
		while(killsock(-2));
		mechexit(0);
	}
#else /* MULTI */
	while(killsock(-2));
	mechexit(0);
#endif /* MULTI */
#ifdef DEBUG
	debug(TEXT_ALLDONE);
#endif /* DEBUG */
}

#ifdef MULTI

void kill_all_bots(char *reason)
{
	while(botlist)
	{
		current = botlist;
		signoff(NULL,reason);
	}
	/* NOT REACHED */
}

#endif /* MULTI */

/*
 *  Server lists, connects, etc...
 */

aServer *add_server(char *host, int port, char *pass)
{
	aServer	*sp,new;
	int	i;

	sp = serverBASE;
	for(i=0;i<serverSIZE;i++)
	{
		if ((sp->used) && (sp->port == port) &&
		    ((!Strcasecmp(host,sp->name)) || (!Strcasecmp(host,sp->realname))))
			return(sp);
		sp++;
	}
	memset(&new,0,sizeof(new));
	new.used = TRUE;
	new.ident = serverident++;
	strcpy(new.name,host);
	if (pass && *pass)
		strcpy(new.pass,pass);
	new.port = (port)?port:6667;
	return(add_aServer(&new));
}

int try_server(aServer *sp)
{
	sp->lastattempt = now;
	sp->usenum++;
	current->server = sp->ident;
	if ((current->sock = SockConnect(sp->name,sp->port,TRUE)) < 0)
	{
		sp->err = SP_ERRCONN;
		return(-1);
	}
	current->away = FALSE;
	current->connect = CN_TRYING;
	current->activity = current->conntry = now;
	return(current->sock);
}

void connect_to_server(void)
{
	aServer	*sp,*sptry;
	aChan	*Chan;
	int	i;

	/*
	 *  This should prevent the bot from chewing up too
	 *  much CPU when it fails to connect to ANYWHERE
	 */
	current->conntry = now;

	/*
	 *  Is this the proper action if there is no serverlist?
	 */
	if (!serverBASE)
		return;

	if (current->Channels)
	{
#ifdef DEBUG
		debug(TEXT_SETREJOINSYNC);
#endif /* DEBUG */
		Chan = current->Channels;
		while(Chan)
		{
			if (Chan->active == CH_ACTIVE)
			{
				Chan->active = CH_OLD;
				Chan->rejoin = TRUE;
			}
			Chan->sync = TRUE;
			Chan->bot_is_op = FALSE;
			Chan = Chan->next;
		}
	}

	if (current->nextserver)
	{
		sp = find_aServer(current->nextserver);
		current->nextserver = 0;
		if (sp && (try_server(sp) >= 0))
			return;
	}

	/*
	 *  The purpose of this kludge is to find the least used server
	 */
	sptry = NULL;
	sp = serverBASE;
	for(i=0;i<serverSIZE;i++)
	{
		if ((sp->used) && (sp->lastattempt != now))
		{
			if ((!sptry) || (sp->usenum < sptry->usenum))
				sptry = sp;
		}
		sp++;
	}
	/*
	 *  Connect...
	 */
	if (sptry)
		try_server(sptry);
}

void parse_avoice(aChan *Chan)
{
	aChanUser *CU;
	aStrp	*sp,*nx,*end;
	char	modes[MSGLEN],nicks[MSGLEN];
	int	n,max;

	Chan->last_voice = now;
	max = get_int_varc(NULL,SETOPMODES_VAR);
	n = 0;
	sprintf(modes,"MODE %s +",Chan->name);
	nicks[0] = 0;
	end = NULL;
	for(sp=Chan->avoice;sp;sp=sp->next)
	{
		if (n >= max)
		{
			end = sp;
			break;
		}
		if ((CU = find_chanuser(Chan,sp->p)) != NULL)
		{
			if ((CU->flags & CU_MODES) == 0)
			{
				Strcat(modes,"v");
				if (n)
					Strcat(nicks," ");
				Strcat(nicks,sp->p);
				CU->flags |= CU_VOICETMP;
				n++;
			}
		}
	}
#ifdef DEBUG
	debug("(parse_avoice) %s %s\n",modes,nicks);
#endif /* DEBUG */
	sp = Chan->avoice;
	while(sp)
	{
		nx = sp->next;
		MyFree((char**)&sp);
		if (nx == end)
			break;
		sp = nx;
	}
	Chan->avoice = end;
	to_server("%s %s\n",modes,nicks);
	/*
	 *   Reset the CU_VOICETMP flag
	 */
	for(CU=Chan->users;CU;CU=CU->next)
		CU->flags &= ~CU_VOICETMP;
}

void reset_botstate(void)
{
	struct	tm *ltime;
	aServer *sp;
	aChan	*Chan;
#ifdef MULTI
	aBot	*bot;
#endif /* MULTI */
	char	*temp,*chantemp;
	int	x;
	int 	trvar;

	ltime = localtime(&now);
	x = 0;

	short_tv &= ~TV_AVOICE;
#ifdef DYNCHANLIMIT
	short_tv &= ~TV_DYNAMIC;
#endif /* DYNCHANLIMIT */

#ifdef MULTI
	for(bot=botlist;bot;bot=bot->next)
	{
#else /* MULTI */
#define bot current
#endif /* MULTI */
		if (bot->connect == CN_ONLINE)
		{
#ifdef MULTI
			current = bot;
#endif /* MULTI */
			if ((now - current->lastrejoin) > 8)
			{
				current->lastrejoin = now;
				rejoin_channels();
			}
			x = 0;
			for(Chan=current->Channels;Chan;Chan=Chan->next)
			{
				if (Chan->avoice && ((now - Chan->last_voice) >= AVOICE_DELAY))
					parse_avoice(Chan);
				if (Chan->avoice)
					short_tv |= TV_AVOICE;

                                trvar = get_int_varc(Chan,SETTR_VAR);                                                        
                                if (trvar && ((now - Chan->last_topic) >= (trvar * 60)))                                     
                                {                                                                                            
                                        if (Chan->topic)                                                                     
                                        {                                                                                    
                                                to_server("TOPIC %s :",Chan->name);        //ircu 2.10.11 kludge
                                                to_server("TOPIC %s :%s",Chan->name,Chan->topic);                       
                                                Chan->last_topic = now;                                                      
                                        }                                                                                    
                                } 

/* Dynamic limit support */
#ifdef DYNCHANLIMIT
				if ((get_int_varc(Chan,TOGDYNAMIC_VAR)) && (Chan->sync))
				{
					short_tv |= TV_DYNAMIC;
					if (((now - Chan->last_limit) >= 45) && (Chan->bot_is_op))
					{
						check_dynamic(Chan);
						Chan->last_limit = now;
					}
				}
#endif /* DYNCHANLIMIT */

#ifdef LINKING
				if (!Chan->bot_is_op && Chan->active == CH_ACTIVE)
					needop(Chan->name);
#endif /* LINKING */
#ifdef SERVICES
				if ((now - Chan->lastservice) > 10)
				{
					if ((usermode(Chan->name,current->nick) & CU_CHANOP) == 0)
						needop_via_msg(Chan->name);
					Chan->lastservice = now;
				}
#endif /* SERVICES */
			}
			if (now - current->lastreset > RESETINTERVAL)
			{
				uptime_time(&current->ParseList,now - 1800);
				current->lastreset = now;
				if (strcmp(current->nick,current->wantnick))
					to_server("NICK %s",current->wantnick);
				check_idlekick();
				if ((x = get_int_varc(NULL,SETAAWAY_VAR) != 0) && current->away == FALSE)
				{
					if ((now - current->activity) > (x * 60))
					{
						temp = randstring(AWAYFILE);
						to_server(AWAYFORM,(temp && *temp) ? temp : "auto-away",time2away(now));
						current->away = TRUE;
					}
				}
			}
			/*
			 *  10 minute update.
			 */
			if (((ltime->tm_min % 10) == 0) && (now - current->laststatus > 120))
			{
				for(Chan=current->Channels;Chan;Chan=Chan->next)
				{
					if (get_int_varc(Chan,SETAUB_VAR))
						channel_massunban(Chan->name,"*!*@*",
							get_int_varc(Chan,SETAUB_VAR)*60);
					x++;
				}
				chantemp = (current->CurrentChan) ? current->CurrentChan->name : "(none)";
				if ((sp = find_aServer(current->server)) != NULL)
				{
					send_statmsg("C:%s AC:%i CS:%s:%i",
						chantemp,x,(sp->realname[0]) ? sp->realname : sp->name,sp->port);
				}
				else
				{
					send_statmsg(TEXT_NOTINSVRLIST,chantemp,x);
				}
#ifdef SEEN
				if (!write_seenlist(&current->SeenList,current->seenfile))
					send_statmsg(TEXT_SEENNOSAVEFILE,current->seenfile);
#endif /* SEEN */
				current->laststatus = now;
				current->lastrejoin = now;
			}
			/*
			 *  Hourly update.
			 */
			if (!ltime->tm_min && ((now - current->lastsave) > 120))
			{
				send_statmsg("Saving Lists");
				current->lastsave = now;
				if (!saveuserlist())
					send_statmsg(TEXT_NOSAVE,current->Userfile);
			}
		}
#ifdef MULTI
	}

#if defined(SESSIONS) || defined(DYNCMDLEVELS)
	if (!ltime->tm_min && ((now - lastsave) > 120))
	{
		lastsave = now;
#ifdef SESSIONS
		if (!savesession())
			send_global_statmsg(TEXT_SESSIONNOSAVEFILE,SESSIONFILE);
#endif /* SESSIONS */
#ifdef DYNCMDLEVELS
		if (!write_levelfile())
			send_global_statmsg(TEXT_LEVELNOSAVE,LEVELFILE);
#endif /* DYNCMDLEVELS */
	}
#endif

#else /* MULTI */
#undef bot
#endif /* MULTI */
}

void parse_server_input(void)
{
	aServer	*sp;
	char	linebuf[MSGLEN];
	char	*res;
	int	sendpass;

	if (FD_ISSET(current->sock,&write_fds))
	{
		setbotnick(current,current->wantnick);
#ifdef DEBUG
		debug("[PSI] {%i} connection established (%s) [ASYNC]\n",
			current->sock,current->wantnick);
#endif /* DEBUG */
		current->connect = CN_CONNECTED;
		current->conntry = now;
		sp = find_aServer(current->server);
#ifdef WINGATES
		if (current->vhost && current->vhost_type == VH_WINGATE)
		{
			if (!sp)
			{
				close(current->sock);
				current->sock = -1;
				return;
			}
#ifdef DEBUG
			debug(TEXT_WINGATECONNECT);
#endif /* DEBUG */
			to_server("%s %i\n",sp->name,sp->port);
			/*
			 *  If we fail here, we'd better drop the WinGate
			 *  and retry the SAME server again
			 */
			if (current->sock == -1)
			{
				current->nextserver = sp->ident;
				current->vhost_type = VH_WINGATE_FAIL;
			}
			current->connect = CN_WINGATEWAIT;
			return;
		}
#endif /* WINGATES */
		sendpass = (sp && sp->pass[0]);
		to_server("%s%s\nNICK %s\nUSER %s . . :%s\n",
			(sendpass) ? "PASS :" : "",
			(sendpass) ? sp->pass : "",
			current->wantnick,
			current->login,
			current->ircname);
		return;
	}
	if (FD_ISSET(current->sock,&read_fds))
	{
get_line:
		errno = EAGAIN;
		res = sockread(current->sock,current->sd,linebuf);
		if (res)
		{
#ifdef PIPEUSER
			if (current->pipeuser)
			{
				if ((send_to_socket(current->pipeuser->sock,"%s\n",linebuf)) < 0)
				{
#ifdef DEBUG
					debug("[PSI] {%i} errno = %i; closing pipeuser connection\n",
						current->pipeuser->sock,errno);
#endif /* DEBUG */
					close(current->pipeuser->sock);
					MyFree((char**)&current->pipeuser);
				}
			}
#endif /* PIPEUSER */
#ifdef WINGATES
			if (current->connect == CN_WINGATEWAIT)
			{
				if (!strncmp("Connecting to host ",linebuf,19))
				{
					if (!matches("*Connected",linebuf))
					{
#ifdef DEBUG
						debug(TEXT_WINGATEACTIVE);
#endif /* DEBUG */
						sp = find_aServer(current->server);
						sendpass = (sp && sp->pass[0]);
						to_server("%s%s\nNICK %s\nUSER %s . . :%s\n",
							(sendpass) ? "PASS :" : "",
							(sendpass) ? sp->pass : "",
							current->wantnick,
							current->login,
							current->ircname);
						current->connect = CN_CONNECTED;
						current->conntry = now;
						goto get_line;
					}
					else
					{
						close(current->sock);
						current->sock = -1;
						return;
					}
				}
			}
#endif /* WINGATE */
			parseline(linebuf);
			goto get_line;
		}
		switch(errno)
		{
		case EINTR:
		case EAGAIN:
			break;
		default:
#ifdef DEBUG
			debug(TEXT_CLOSESVRSOCKET,current->sock,errno);
#endif /* DEBUG */
			memset(current->sd,0,sizeof(current->sd));
			close(current->sock);
			current->sock = -1;
			current->connect = CN_NOSOCK;
			break;
		}
	}
}

