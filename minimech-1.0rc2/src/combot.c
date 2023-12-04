/*

    Starglider Class EnergyMech, IRC bot software
    Parts Copyright (c) 1997-2000  proton

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
	VarStruct *Vars,*CV;
	aServer	*sp;
	aChan	*Chan;
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

	tofile(sf,"nick %s\n",current.wantnick);
	tofile(sf,"login %s\n",current.login);
	if (current.vhost[0])
	{
		switch(current.vhost_type)
		{
		case VH_IPALIAS:
		case VH_IPALIAS_FAIL:
			tofile(sf,"virtual %s\n",current.vhost);
			break;
#ifdef WINGATES
		case VH_WINGATE:
		case VH_WINGATE_FAIL:
			tofile(sf,"wingate %s %i\n",current.vhost,current.vhost_port);
			break;
#endif /* WINGATES */
		}
	}
	tofile(sf,"ircname %s\n",current.ircname);
	if (current.modes[0])
		tofile(sf,"modes %s\n",current.modes);
	if (current.cmdchar != DEFAULTCMDCHAR)
		tofile(sf,"cmdchar %c\n",current.cmdchar);
	tofile(sf,"userfile %s\n\n",current.Userfile);
	/*
	 *  current.DefaultVars contains channel defaults and global vars
	 */
	Vars = current.DefaultVars;
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

	for(Chan=current.Channels;Chan;Chan=Chan->next)
	{
		if ((Chan->active == CH_OLD) && (!Chan->rejoin))
			continue;
		tofile(sf,"channel %s\n",Chan->name,(Chan->key && *Chan->key) ? Chan->key : "");
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

/*
 *  Bot adding and killing
 */

aBot *create_bot(char *nick)
{
#ifdef SEEN
	char	buffer[FILENAMELEN];
#endif /* SEEN */

	memset(&current,0,sizeof(aBot));
	current.connect = CN_NOSOCK;
	current.sock = -1;
	current.cmdchar = DEFAULTCMDCHAR;
	strcpy(current.nick,nick);
	strcpy(current.wantnick,nick);
	strcpy(current.login,BOTLOGIN);
	sprintf(current.ircname,"%s %s",BOTCLASS,VERSION);
	copy_vars(current.DefaultVars,BinaryDefault);
#ifdef SEEN
	sprintf(buffer,BOTDIR "%s.seen",nick);
	set_mallocdoer(create_bot);
	mstrcpy(&current.seenfile,buffer);
#endif /* SEEN */
	current.uptime = now;
	return(&current);
}

void signoff(char *from, char *reason)
{
	aStrp	*sp,*spn;
	aUser	*User;
	int	i;

	if (from)
	{
		send_to_user(from,"Killing mech: %s",current.nick);
		send_to_user(from,"Saving the lists...");
	}
	if (!saveuserlist() && from)
	{
		send_to_user(from,ERR_NOSAVE,current.Userfile);
	}
#ifdef SEEN
	if (!write_seenlist(&current.SeenList,current.seenfile) && from)
	{
		send_to_user(from,"SeenList could not be saved to file %s",current.seenfile);
	}
#endif /* SEEN */
#ifdef DYNCMDLEVELS
	if (!write_levelfile() && from)
	{
		send_to_user(from,"Levels could not be saved to %s",LEVELFILE);
	}
#endif /* DYNCMDLEVELS */
	if (from)
	{
		send_to_user(from,"ShutDown Complete");
	}

	close_all_dcc();
	delete_all_channels();

	if (current.sock != -1)
	{
		if (!reason || !*reason)
			reason = randstring(SIGNOFFSFILE);
		send_to_server("QUIT :%s",(reason && *reason) ? reason : "");
		killsock(current.sock);
		current.sock = -1;
	}

#ifdef PIPEUSER
#ifdef DEBUG
	debug("(signoff) Removing pipuser stuff...\n");
#endif /* DEBUG */
	if (current.pipeuser && current.pipeuser->sock >= 0)
		killsock(current.pipeuser->sock);
	MyFree((char**)&current.pipeuser);
	MyFree(&current.saved_002);
	MyFree(&current.saved_003);
	MyFree(&current.saved_004);
#endif /* PIPEUSER */

#ifdef SEEN
#ifdef DEBUG
	debug("(signoff) Removing seenlist...\n");
#endif /* DEBUG */
	delete_seen();
	MyFree(&current.seenfile); 
#endif /* SEEN */

#ifdef DEBUG
	debug("(signoff) Removing userlist...\n");
#endif /* DEBUG */
	User = current.userlistBASE;
	for(i=0;i<current.userlistSIZE;i++)
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
	MyFree((char**)&current.userlistBASE);

#ifdef DEBUG
	debug("(signoff) Removing lastcmd list...\n");
#endif /* DEBUG */
	for(i=0;i<LASTCMDSIZE;i++)
	{
		MyFree(&current.lastcmds[i]);
	}

	delete_time(&current.ParseList);

	/*
	 *  These 2 are used by do_die() to pass reason and doer.
	 */
	MyFree((char**)&current.signoff);
	MyFree((char**)&current.from);

	while(killsock(-2));
	mechexit(0);
}

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
	current.server = sp->ident;
	if ((current.sock = SockConnect(sp->name,sp->port,TRUE)) < 0)
	{
		sp->err = SP_ERRCONN;
		return(-1);
	}
	current.away = FALSE;
	current.connect = CN_TRYING;
	current.activity = current.conntry = now;
	return(current.sock);
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
	current.conntry = now;

	/*
	 *  Is this the proper action if there is no serverlist?
	 */
	if (!serverBASE)
		return;

	if (current.Channels)
	{
#ifdef DEBUG
		debug("[CtS] Setting rejoin- and synced-status for all channels\n");
#endif /* DEBUG */
		Chan = current.Channels;
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

	if (current.nextserver)
	{
		sp = find_aServer(current.nextserver);
		current.nextserver = 0;
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
	send_to_server("%s %s\n",modes,nicks);
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
	char	*temp,*chantemp;
	int	x;

	ltime = localtime(&now);
	x = 0;

	if (current.connect == CN_ONLINE)
	{
		if ((now - current.lastrejoin) > 8)
		{
			current.lastrejoin = now;
			rejoin_channels();
		}
		x = 0;
		for(Chan=current.Channels;Chan;Chan=Chan->next)
		{
			if (Chan->avoice && ((now - Chan->last_voice) >= AVOICE_DELAY))
				parse_avoice(Chan);
			if (Chan->avoice)
				x++;
#ifdef LINKING
			if (!Chan->bot_is_op && Chan->active == CH_ACTIVE)
				needop(Chan->name);
#endif /* LINKING */
		}
		if (!x)
		{
			short_tv &= ~TV_AVOICE;
		}
		if (now - current.lastreset > RESETINTERVAL)
		{
			uptime_time(&current.ParseList,now - 1800);
			current.lastreset = now;
			if (strcmp(current.nick,current.wantnick))
				send_to_server("NICK %s",current.wantnick);
			check_idlekick();
			if ((x = get_int_varc(NULL,SETAAWAY_VAR) != 0) && current.away == FALSE)
			{
				if ((now - current.activity) > (x * 60))
				{
					temp = randstring(AWAYFILE);
					send_to_server(AWAYFORM,(temp && *temp) ? temp : "auto-away",time2away(now));
					current.away = TRUE;
				}
			}
		}
		/*
		 *  10 minute update.
		 */
		if (((ltime->tm_min % 10) == 0) && (now - current.laststatus > 120))
		{
			for(Chan=current.Channels;Chan;Chan=Chan->next)
			{
				if (get_int_varc(Chan,SETAUB_VAR))
					auto_unban(get_int_varc(Chan,SETAUB_VAR));
				x++;
			}
			chantemp = (current.CurrentChan) ? current.CurrentChan->name : "(none)";
			if ((sp = find_aServer(current.server)) != NULL)
			{
				send_statmsg("C:%s AC:%i CS:%s:%i",
					chantemp,x,(sp->realname[0]) ? sp->realname : sp->name,sp->port);
			}
			else
			{
				send_statmsg("C:%s AC:%i CS:(not in serverlist)",chantemp,x);
			}
#ifdef SEEN
			if (!write_seenlist(&current.SeenList,current.seenfile))
				send_statmsg("SeenList could not be saved to file %s",current.seenfile);
#endif /* SEEN */
			current.laststatus = now;
		}
		/*
		 *  Hourly update.
		 */
		if (!ltime->tm_min && ((now - current.lastsave) > 120))
		{
			send_statmsg("Saving Lists");
			current.lastsave = now;
			if (!saveuserlist())
				send_statmsg(ERR_NOSAVE,current.Userfile);
		}
	}

#if defined(SESSIONS) || defined(DYNCMDLEVELS)
	if (!ltime->tm_min && ((now - lastsave) > 120))
	{
		lastsave = now;
#ifdef SESSIONS
		if (!savesession())
			send_global_statmsg("Session could not be saved to file %s",SESSIONFILE);
#endif /* SESSIONS */
#ifdef DYNCMDLEVELS
		if (!write_levelfile())
			send_global_statmsg("Levels could not be saved to %s",LEVELFILE);
#endif /* DYNCMDLEVELS */
	}
#endif /* SESSIONS || DYNCMDLEVELS */
}

void parse_server_input(void)
{
	aServer	*sp;
	char	linebuf[MSGLEN];
	char	*res;
	int	sendpass;

	if (FD_ISSET(current.sock,&write_fds))
	{
		setbotnick(current.wantnick);
#ifdef DEBUG
		debug("[PSI] {%i} connection established (%s) [ASYNC]\n",
			current.sock,current.wantnick);
#endif /* DEBUG */
		current.connect = CN_CONNECTED;
		current.conntry = now;
		sp = find_aServer(current.server);
#ifdef WINGATES
		if (current.vhost && current.vhost_type == VH_WINGATE)
		{
			if (!sp)
			{
				close(current.sock);
				current.sock = -1;
				return;
			}
#ifdef DEBUG
			debug("[PSI] Connecting via WinGate proxy...\n");
#endif /* DEBUG */
			send_to_server("%s %i\n",sp->name,sp->port);
			/*
			 *  If we fail here, we'd better drop the WinGate
			 *  and retry the SAME server again
			 */
			if (current.sock == -1)
			{
				current.nextserver = sp->ident;
				current.vhost_type = VH_WINGATE_FAIL;
			}
			current.connect = CN_WINGATEWAIT;
			return;
		}
#endif /* WINGATES */
		sendpass = (sp && sp->pass[0]);
		send_to_server("%s%s\nNICK %s\nUSER %s . . :%s\n",
			(sendpass) ? "PASS :" : "",
			(sendpass) ? sp->pass : "",
			current.wantnick,
			current.login,
			current.ircname);
		return;
	}
	if (FD_ISSET(current.sock,&read_fds))
	{
get_line:
		errno = EAGAIN;
		res = sockread(current.sock,current.sd,linebuf);
		if (res)
		{
#ifdef PIPEUSER
			if (current.pipeuser)
			{
				if ((send_to_socket(current.pipeuser->sock,"%s\n",linebuf)) < 0)
				{
#ifdef DEBUG
					debug("[PSI] {%i} errno = %i; closing pipeuser connection\n",
						current.pipeuser->sock,errno);
#endif /* DEBUG */
					close(current.pipeuser->sock);
					MyFree((char**)&current.pipeuser);
				}
			}
#endif /* PIPEUSER */
#ifdef WINGATES
			if (current.connect == CN_WINGATEWAIT)
			{
				if (!strncmp("Connecting to host ",linebuf,19))
				{
					if (!matches("*Connected",linebuf))
					{
#ifdef DEBUG
						debug("[PSI] WinGate proxy active\n");
#endif /* DEBUG */
						sp = find_aServer(current.server);
						sendpass = (sp && sp->pass[0]);
						send_to_server("%s%s\nNICK %s\nUSER %s . . :%s\n",
							(sendpass) ? "PASS :" : "",
							(sendpass) ? sp->pass : "",
							current.wantnick,
							current.login,
							current.ircname);
						current.connect = CN_CONNECTED;
						current.conntry = now;
						goto get_line;
					}
					else
					{
						close(current.sock);
						current.sock = -1;
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
			debug("[PSI] {%i} errno = %i; closing server socket\n",current.sock,errno);
#endif /* DEBUG */
			memset(current.sd,0,sizeof(current.sd));
			close(current.sock);
			current.sock = -1;
			current.connect = CN_NOSOCK;
			break;
		}
	}
}

