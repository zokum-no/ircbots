/*

    EnergyMech, IRC bot software
    Parts Copyright (c) 1997-2001 proton, 2002-2004 emech-dev

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
#define COM_ONS_C
#include "config.h"

#ifdef SEEN

void do_seen(char *from, char *to, char *rest, int cmdlevel)
{
	aSeen   *Seen;
	char	message[MSGLEN];
	char	*nick,*chan;
	Uchar	c1,c2;

	if (!will.seen)
		return;

	chan = get_channel(to,&rest);
	if (!rest || !*rest)
	{
		send_to_user(from,TEXT_WHERELOOK);
		return;
	}

	nick = chop(&rest);
	if (!isnick(nick))
	{
		send_to_user(from,TEXT_ILLEGALNICK);
		return;
	}

	if (!matches(nick,current->nick))
		sprintf(message,TEXT_ITSME,current->nick);
	else
	if (!matches(nick,getnick(from)))
		sprintf(message,TEXT_FINDYOURSELF,nick);
	else
	{
		c1 = tolowertab[(Uchar)*nick];
		for(Seen=current->SeenList;Seen;Seen=Seen->next)
		{
			c2 = tolowertab[(Uchar)*Seen->nick];
			if (c1 == c2)
			{
				if (!Strcasecmp(nick,Seen->nick))
					break;
			}
			if (c1 > c2)
			{
				Seen = NULL;
				break;
			}
		}
		if (!Seen)
		{
			sprintf(message,TEXT_NOMEMORY,nick);
		}
		else
		switch(Seen->selector)
		{
		case 0:
			sprintf(message,TEXT_RIGHTHERE,nick);
			break;
		case 1:
			sprintf(message,TEXT_PARTEDFROM,Seen->userhost,
				Seen->signoff,idle2str(now-Seen->time,0));
			break;
		case 2:
			sprintf(message,TEXT_SIGNEDOFF,Seen->userhost,
				idle2str(now-Seen->time,0),Seen->signoff);
			break;
		case 3:
			sprintf(message,TEXT_CHANGEDNICK,Seen->userhost,
				Seen->signoff,idle2str(now-Seen->time,0));
			break;
		case 4:
			sprintf(message,TEXT_KICKEDBY,Seen->userhost,
				Seen->kicker,Seen->signoff,idle2str(now-Seen->time,0));
			break;
		}
	}

	if (ischannel(to))
		sendprivmsg(chan,"%s",message);
	else
		send_to_user(from,"%s",message);
}

#endif /* SEEN */

void print_help(char *from, char *line, int len)
{
	char	*lp;
	int	tl;

	tl = TRUE;
	if ((strlen(line) + len) > 70)
	{
		lp = line;
		while(*lp)
		{
			if (*lp == '\037')
				tl = !tl;
			if (tl)
				*lp = tolowertab[(Uchar)*lp];
			lp++;
		}
		send_to_user(from,"%s",line);
		*line = 0;
	}
}

char	*dh_from;
char	*dh_rest;
int	dh_j,dh_i;

int do_help_callback(char *line)
{
	int	level;

	if (*line == ':')
	{
		if (dh_j)
		{
			if (dh_i == 0)
			{
				send_to_user(dh_from,ERR_NOHELPTOPIC);
			}
			send_to_user(dh_from,TEXT_ENDHELP);
			return(TRUE);
		}
		line++;
		if (!Strcasecmp(dh_rest,line))
		{
			send_to_user(dh_from,TEXT_HELPON,line);
			level = level_needed(line);
			if (level > 0)
				send_to_user(dh_from,TEXT_LEVELNEEDED,level);
			dh_j++;
		}
		return(FALSE);
	}
	if (dh_j)
	{
		send_to_user(dh_from,"%s",line);
		dh_i++;
	}
	return(FALSE);
}

void do_help(char *from, char *to, char *rest, int cmdlevel)
{
	char	line[MSGLEN];
	int	i,level,axs;
	int	cur,nxt,ct,ci,tl;
	int	in,*ip;

	axs = max_userlevel(from);

	if (!rest || !*rest)
	{
		send_to_user(from,TEXT_CMDSAVAILABLE);
		cur = line[0] = 0;
		while(TRUE)
		{
			ct = 0;
			nxt = OWNERLEVEL;

			for(i=0;mcmd[i].name;i++)
			{
				tl = mcmd[i].flags & CLEVEL;
				if ((tl < nxt) && (tl > cur))
					nxt = tl;
				if (tl != cur)
					continue;
				if (ct == 0)
					sprintf(line,TEXT_CMDLEVEL,cur);
				ct++;
			}
			if (!ct)
			{
				cur = nxt;
				continue;
			}
#ifdef DEBUG
			debug("(do_help) printing all level %i commands\n",cur);
#endif /* DEBUG */
			ci = ct;
			for(i=0;mcmd[i].name;i++)
			{
				tl = mcmd[i].flags & CLEVEL;
				if (tl != cur)
					continue;
				if (ci != ct)
					Strcat(line,", ");
				print_help(from,line,strlen(mcmd[i].name));
				if (*line == 0)
					strcpy(line,"           ");
				Strcat(line,mcmd[i].name);
				ct--;
			}
			print_help(from,line,500);
			if ((cur != OWNERLEVEL) && (nxt <= axs))
				cur = nxt;
			else
			{
				send_to_user(from,TEXT_ENDHELP);
				return;
			}
		}
	}

	if (isdigit((int)*rest))
	{
		ip = Atoi(rest);
		if (!ip)
		{
			usage(from,C_HELP);
			return;
		}
		level = *ip;
		if (level > axs)
			level = axs;
		if (level < 0)
			level = 0;
		send_to_user(from,TEXT_CMDSAVAILATLVL,level);
		line[0] = 0;
		ci = 0;
		for(i=0;mcmd[i].name;i++)
		{
			if ((mcmd[i].flags & CLEVEL) <= level)
			{
				if (ci != 0)
					Strcat(line,", ");
				ci++;
				print_help(from,line,strlen(mcmd[i].name));
				Strcat(line,mcmd[i].name);
			}
		}
		if (ci)
			print_help(from,line,70);
		send_to_user(from,TEXT_ENDHELP);
		return;
	}

	if (strchr(rest,'*'))
	{
		line[0] = 0;
		ci = 0;
		send_to_user(from,TEXT_CMDSMATCHLVL,rest);
		for(i=0;mcmd[i].name;i++)
		{
			if ((!matches(rest,mcmd[i].name)) &&
			   ((mcmd[i].flags & CLEVEL) <= axs))
			{
				if (ci != 0)
					Strcat(line,", ");
				ci++;
				print_help(from,line,strlen(mcmd[i].name));
				Strcat(line,mcmd[i].name);
			}
		}
		if (ci)
			print_help(from,line,500);
		else
			send_to_user(from,"(none)");
		send_to_user(from,TEXT_ENDHELP);
		return;
	}

	if ((in = open(HELPFILE,O_RDONLY)) < 0)
	{
		send_to_user(from,ERR_FILEOPEN,HELPFILE);
		return;
	}

	dh_i = dh_j = 0;
	dh_rest = chop(&rest);
	dh_from = from;
#ifdef DEBUG
	debug("(do_help) helpfile search = '%s'\n",dh_rest);
#endif /* DEBUG */

	readline(in,&do_help_callback);
	close(in);
}

/*
 *  :nick!user@host KICK #channel kicknick :message
 */
void on_kick(char *from, char *rest)
{
#ifdef SEEN
	aChanUser *CU;
#endif /* SEEN */
	aChan	*Chan;
	char	*channel,*nick;
	char	*protnuh;

	channel = chop(&rest);
	if ((Chan = find_channel(channel,CH_ACTIVE)) == NULL)
		return;

	nick = chop(&rest);

	if (rest && *rest == ':')
		rest++;

	strcpy(CurrentNick,getnick(from));

	send_spy(Chan,TEXT_KICKEDBYIRC,nick,CurrentNick,rest);

	if (is_me(nick))
	{
		MyFree(&Chan->kickedby);
		set_mallocdoer(on_kick);
		mstrcpy(&Chan->kickedby,from);
		Chan->active = CH_OLD;
		Chan->sync = TRUE;
		Chan->bot_is_op = FALSE;
		join_channel(Chan->name,(Chan->key && *Chan->key) ? Chan->key : NULL,TRUE);
		if (current->CurrentChan && current->CurrentChan->active == CH_OLD)
		{
			for(Chan=current->Channels;Chan;Chan=Chan->next)
			{
				if (Chan->active == CH_ACTIVE)
					break;
			}
			current->CurrentChan = Chan;
			/*
			 *  Might be set to NULL now, but its supposed to be checked whenever used.
			 *  If not, we get a SEGV; and fix it.
			 */
		}
		return;
	}

	if (Chan->bot_is_op)
	{
		CurrentUser = find_user(from,channel);
		if (!CurrentUser || CurrentUser->access < ASSTLEVEL)
		{
			if (!is_localbot(CurrentNick))
			{
				if (check_mass(from,Chan,CHK_KICK))
					mass_action(from,channel);
			}
		}

		if (((protnuh = find_nuh(nick)) != NULL) && get_protlevel(protnuh,channel))
		{
			if (!is_localbot(CurrentNick))
				prot_action(from,Chan,protnuh);
			to_server("INVITE %s %s",nick,channel);
		}
	}

#ifdef SEEN
	if ((CU = find_chanuser(Chan,nick)) != NULL)
	{
		make_seen(nick,get_nuh(CU),from,now,4,rest);
	}
#endif /* SEEN */
	/*
	 *  Dont delete the poor sod before all has been processed
	 */
	remove_chanuser(Chan,nick);
}

void on_join(char *who)
{
	aStrp	*sp,*nx;
	aBan	*ban;
	char	*channel;
	int	banmatch;
	int	avuser;

	/*
	 *  Satisfy spies before we continue...
	 */
	send_spy(CurrentChan,"*** Joins: %s (%s)",CurrentNick,getuh(who));
	/*
	 *  No further actions to be taken if the bot isnt opped
	 */
	if (!CurrentChan->bot_is_op)
		return;
	channel = CurrentChan->name;
	/*
	 *  Some stuff only applies to non-users
	 */
	if (!CurrentUser)
	{
		/*
		 *  Kick banned (desynched) users if ABK is set
		 */
		if (get_int_varc(CurrentChan,TOGABK_VAR))
		{
			banmatch = 0;
			for(ban=CurrentChan->banned;ban;ban=ban->next)
			{
				if (!matches(ban->banstring,who))
				{
					banmatch++;
					break;
				}
			}
			if (banmatch)
			{
				sendkick(channel,CurrentNick,TEXT_BANNEDONSERVER);
				return;
			}
		}
		/*
		 *  Kickban users with control chars in their ident
		 *  (which doesnt violate RFC1413 but is bloody annoying)
		 */
		if (get_int_varc(CurrentChan,TOGCTL_VAR))
		{
			if (strchr(who,'\031') || strchr(who,'\002') ||  strchr(who,'\022') || strchr(who,'\026'))
			{
				deop_siteban(channel,getnick(who),who);
				sendkick(channel,getnick(who),TEXT_CTRLCHARSUSERID);
				return;
			}
		}
	}
	/*
	 *  If they're shitted, they're not allowed to be opped or voiced
	 */
	if (CurrentShit)
	{
		shit_action(who,CurrentChan);
		return;
	}
	/*
	 *  Check for +ao users if AOP is toggled on
	 */
	if (get_int_varc(CurrentChan,TOGAOP_VAR))
	{
		if (is_aop(who,channel) && (!password_needed(who) || verified(who)))
		{
			sendmode(channel,"+o %s",CurrentNick);
			return;
		}
	}
	/*
	 *  If AVOICE eq 0 we have nothing more to do
	 */
	avuser = FALSE;
	switch(get_int_varc(CurrentChan,SETAVOICE_VAR))
	{
	case 0:
		return;
	case 1:
		if (!is_avoice(who,channel))
			return;
		avuser = TRUE;
	}
	set_mallocdoer(on_join);
	sp = MyMalloc(strlen(CurrentNick)+1+PTR_SIZE);
	strcpy(sp->p,CurrentNick);
	short_tv |= TV_AVOICE;
	if (!CurrentChan->last_voice)
		CurrentChan->last_voice = (now - AVOICE_DELAY_MIN);
	if ((avuser) || (!CurrentChan->avoice))
	{
		sp->next = CurrentChan->avoice;
		CurrentChan->avoice = sp;
		return;
	}
	/*
	 *  Find the last link
	 */
	nx = CurrentChan->avoice;
	while(nx->next)
		nx = nx->next;
	nx->next = sp;
}

void on_nick(char *from, char *newnick)
{
	aChanUser *CU;
	aChan	*Chan;
	aTime	*tmp;
	char	newnuh[NAMELEN];
	int	isbot;

	strcpy(CurrentNick,getnick(from));
	sprintf(newnuh,"%s%s",newnick,strchr(from,'!'));

#ifdef SEEN
	make_seen(CurrentNick,from,"none",now,3,newnick);
	make_seen(newnick,newnuh,"none",now,0,"no_info");
#endif /* SEEN */

#ifdef FASTNICK
	if (!Strcasecmp(CurrentNick,current->wantnick))
		to_server("NICK %s\n",current->wantnick);
#endif /* FASTNICK */

	/*
	 *  Kicking taken care of in this function
	 */
	check_nickflood(from);
	send_common_spy(from,TEXT_NOWKNOWNAS,CurrentNick,newnick);

	change_dccnick(from,newnuh);
	if ((tmp = find_time(&current->Auths,from)) != NULL)
	{
		MyFree((char**)&tmp->name);
		set_mallocdoer(on_nick);
		mstrcpy(&tmp->name,newnuh);
	}

	if ((isbot = is_me(CurrentNick)))
		setbotnick(current,newnick);

	for(Chan=current->Channels;Chan;Chan=Chan->next)
	{
		if ((CU = find_chanuser(Chan,CurrentNick)) != NULL)
		{
			/*
			 *  We only need to realloc the buffer if its too small
			 */
			if (strlen(CU->nick) >= strlen(newnick))
			{
				strcpy(CU->nick,newnick);
			}
			else
			{
				MyFree((char**)&CU->nick);
				set_mallocdoer(on_nick);
				mstrcpy(&CU->nick,newnick);
			}
			if (!isbot)
				shit_action(get_nuh(CU),Chan);
		}
	}
}

void on_msg(char *from, char *to, char *msg)
{
	static	char msg_copy[MSGLEN];
#ifdef LINKING
	aBotLink *bl;
#endif /* LINKING */
	aDCC	*userclient,*Client;
	aTime	*tmp;
#ifdef MULTI
	aBot	*bot;
#endif /* MULTI */
	char	*command,*rest,*pt;
	int	has_cc,pubcmd,cmdlvl;
	int	mul;
	int	i,j;

	/*
	 *  No line sent to this routine should be longer than MSGLEN
	 *  Callers responsibility to check that from, to and msg is
	 *  non-NULL and non-zerolength
	 */

	/*
	 *  We ignore everything from shitted users
	 */
	if (CurrentShit)
		return;

	/*
	 *  Touch auth record for user
	 */
	if ((tmp = find_time(&current->Auths,from)) != NULL)
		tmp->time = now;

	/*
	 * If the message is for a channel and we dont accept
	 * public commands, we can go directly to on_public()
	 */
	pubcmd = ischannel(to);

	if (pubcmd && !get_int_var(to,TOGPUB_VAR))
	{
		on_public(from,to,msg);
		return;
	}

	/*
	 *  If its a bot we want nothing to do with it
	 */
	if ((mul = max_userlevel(from)) > OWNERLEVEL)
		return;

	/*
	 *  Make copies of the input before we shred it
	 */
	strcpy(msg_copy,msg);

	userclient = search_list(from,DCC_CHAT);

	if (get_int_varc(NULL,TOGCC_VAR))
	{
		has_cc = shell_cmd;
	}
	else
		has_cc = TRUE;

	if ((command = chop(&msg)) == NULL)
		return;

	if (is_me(command))
	{
		if ((command = chop(&msg)) == NULL)
			return;
		has_cc = TRUE;
	}
	if (*command == current->cmdchar)
	{
		has_cc = TRUE;
		command++;
	}

	for(pt=command;*pt;pt++)
		*pt = toupper(*pt);

	for(i=0;mcmd[i].name;i++)
	{
		if (!has_cc && (mcmd[i].flags & CC))
			continue;
		if (!Strcasecmp(mcmd[i].name,command) || (mcmd[i].alias &&
		    !Strcasecmp(mcmd[i].alias,command)))
		{
			cmdlvl = (mcmd[i].flags & CLEVEL);
			if ((mcmd[i].flags & NOPUB) && pubcmd)
			{
#ifdef DEBUG
				debug("(on_msg) Public command (%s) ignored\n",command);
#endif /* DEBUG */
				return;
			}
			if (mul < cmdlvl)
			{
				if (pubcmd && !(mcmd[i].flags & CC))
					on_public(from,to,msg_copy);
				return;
			}
			if (password_needed(from) && (mcmd[i].flags & PASS) && !verified(from))
			{
				if ((mcmd[i].flags & CC) || !pubcmd)
					send_to_user(from,TEXT_USEVERIFY);
				return;
			}
			need_dcc = (mcmd[i].flags & DCC);
			if (*from == '$')
				need_dcc = FALSE;
			if (need_dcc && (!userclient || (userclient->flags & DCC_WAIT)))
			{
				need_dcc = TRUE;
				dcc_chat(from,msg_copy);
				return;
			}
			if (get_int_varc(NULL,TOGSPY_VAR))
			{
				send_statmsg(TEXT_EXECUTING,CurrentNick,mul,command,cmdlvl);
			}
			rest = (!msg || !*msg) ? NULL : msg;
			if (!rest && (mcmd[i].flags & ARGS))
			{
				usage(from,command);
				return;
			}
			mcmd[i].function(from,to,rest,cmdlvl);

			MyFree(&current->lastcmds[LASTCMDSIZE-1]);
			for(j=LASTCMDSIZE-2;j>=0;j--)
				current->lastcmds[j+1] = current->lastcmds[j];
			pt = gethost(from);
			set_mallocdoer(on_msg);
			current->lastcmds[0] = (char*)MyMalloc(strlen(pt) + 45);
			sprintf(current->lastcmds[0],"[%s] %-10s %11s[%i] (*@%s)",
				time2medium(now),command,
				(CurrentUser && CurrentUser->ident) ? CurrentUser->name : "Unknown",
				(CurrentUser && CurrentUser->ident) ? CurrentUser->access : 0,pt);

			return;
		}
	}
	if (pubcmd)
	{
		on_public(from,to,msg_copy);
		return;
	}
	if (has_cc && *command && mul)
	{
		send_to_user(from,TEXT_SQUEEZEME);
	}
	else
	if (userclient && (userclient->flags & DCC_CHAT))
	{
#ifdef MULTI
		backup = current;
		for(bot=botlist;bot;bot=bot->next)
		{
			current = bot;
#endif /* MULTI */
#ifdef PIPEUSER
			if (current->pipeuser)
			{
				send_to_socket(current->pipeuser->sock,
					":%s PRIVMSG %s :%s\n",(char*)PUMASK,current->nick,msg_copy);
			}
#endif /* PIPEUSER */
			for(Client=current->ClientList;Client;Client=Client->next)
			{
				if ((Client->flags & DCC_CHAT) && (Client->flags & DCC_ACTIVE))
				{
					if (((Ulong)Client == (Ulong)userclient) &&
					    ((CurrentUser) && (CurrentUser->echo == FALSE)))
						continue;
					send_to_user(Client->user,"<%s> %s",CurrentNick,msg_copy);
				}
			}
#ifdef MULTI
		}
		current = backup;
#endif /* MULTI */
#ifdef LINKING
		bl = botlinkBASE;
		for(i=0;i<botlinkSIZE;i++)
		{
			if ((bl->used) && (bl->linktype == LN_UPLINK))
			{
				send_to_link(bl,"$MSG %s@%s %s\n",CurrentNick,
					current->nick,msg_copy);
			}
			bl++;
		}
#endif /* LINKING */
		return;
	}
	else
	{
		if (get_int_varc(NULL,TOGIWM_VAR))
		{
			if (strchr("[(\002\026\037",msg_copy[0]))
				return;
		}
		send_spymsg("[message] <%s> %s",CurrentNick,msg_copy);
  	}
}

void on_actionmsg(char *from, char *to, char *rest)
{
	aUser	*kicksay;
	aDCC	*userclient,*Client;
	aChan	*Chan;
	aTime	*auth;
	char	*pt;
	int	i;

	if (CurrentShit)
		return;

	if ((auth = find_time(&current->Auths,from)) != NULL)
		auth->time = now;

	if (ischannel(to))
	{
		if ((Chan = find_channel(to,CH_ACTIVE)) == NULL)
			return;
		send_spy(Chan,"* %s %s",CurrentNick,rest);
		update_idletime(from,to);
		if ((capslevel(rest) >= 50) && (check_mass(from,Chan,CHK_CAPS)))
		{
			sendkick(to,CurrentNick,TEXT_LOTSOCAPS);
		}
		else
		if ((i = check_mass(from,Chan,CHK_PUB)) >= 1)
		{
			if (i >= 2)
				deop_ban(to,CurrentNick,from);
			sendkick(to,CurrentNick,TEXT_FLOODER);
			send_statmsg(TEXT_KICKFLOODER,from,to);
		}
		else
		if (get_int_varc(Chan,TOGKS_VAR))
		{
			if ((kicksay = check_kicksay(rest,to)) != NULL)
			{
				if (!CurrentUser || !CurrentUser->access)
				{
					pt = formatgreet(kicksay->reason,CurrentNick);
					sendkick(to,CurrentNick,"%s",pt);
				}
			}
		}
		return;
	}
	userclient = search_list(from,DCC_CHAT);
	if (userclient)
	{
		for(Client=current->ClientList;Client;Client=Client->next)
		{
			if (Client->flags & DCC_CHAT)
				send_to_user(Client->user,"* %s %s",CurrentNick,rest);
		}
		return;
	}
	if (get_int_varc(NULL,TOGIWM_VAR))
	{
		if (strchr("[(\002\026\037",rest[0]))
			return;
	}
	send_spymsg("[message] * %s %s",CurrentNick,rest);
}

void on_mode(char *from, char *channel, char *rest)
{
	aChan	*Chan;
	aUser	*User;
	char	unminmode[MAXLEN],unminparams[MAXLEN];
	char	unplusmode[MAXLEN],unplusparams[MAXLEN];
	char	*nick,*parm,*nickuh;
	char	*chanchars,*params,sign = '+';
	int	i,enfm,togprot,*ip;

	if ((Chan = find_channel(channel,CH_ACTIVE)) == NULL)
		return;

	togprot = get_int_varc(Chan,TOGPROT_VAR);

	strcpy(unminmode,"");		/* min-modes, i.e. -bi		*/
	strcpy(unminparams,"");		/* it's parameters		*/
	strcpy(unplusmode,"");		/* plus-modes, i.e. +oo		*/
	strcpy(unplusparams,"");	/* parameters			*/

	send_spy(Chan,TEXT_SETSMODE,getnick(from),rest);

	chanchars = chop(&rest);
	params = rest;

	while(*chanchars)
	{
	switch(*chanchars)
	{
	case '+':
	case '-':
		sign = *chanchars;
		break;
/* o */
	case 'o':
		nick = chop(&params);
		nickuh = find_nuh(nick);
		if (!nickuh)
		{
			/* basically we're out of sync, this should absolutely not happen ... */
			break;
		}
		i = get_userlevel(nickuh,channel);
/* +o */	if(sign == '+')
		{
		add_mode(Chan,CU_CHANOP,nick);
		del_mode(Chan,CU_DEOPPED,nick);
		if ((i == BOTLEVEL) && (is_me(nick)))
		{
			Chan->bot_is_op = TRUE;

			check_shit();
			if (Chan->kickedby && get_int_varc(Chan,TOGRK_VAR))
			{
				if (get_userlevel(Chan->kickedby,channel) < ASSTLEVEL)
					sendkick(channel,getnick(Chan->kickedby),"Doh! This is revenge fucker!");
			}
			MyFree(&Chan->kickedby);
			to_server("MODE %s\n", Chan->name);
			break;
		}
		if (i == 0)
		{
			if (get_shitlevel(nickuh,channel))
			{
				Strcat(unminmode,"o");
				Strcat(unminparams,nick);
				Strcat(unminparams," ");
			}
			else
			if (get_int_varc(Chan,TOGSD_VAR) && !strchr(from,'@'))
			{
				Strcat(unminmode,"o");
				Strcat(unminparams,nick);
				Strcat(unminparams," ");
			}
			else
			if (get_int_varc(Chan,TOGSO_VAR))
			{
				if (get_userlevel(from,channel) < OWNERLEVEL)
				{
					Strcat(unminmode,"o");
					Strcat(unminparams,nick);
					Strcat(unminparams," ");
				}
			}
		}
		}
/* -o */	else
		{
		del_mode(Chan,CU_CHANOP|CU_DEOPPED,nick);
		if (i == BOTLEVEL)
		{
			if (is_me(nick))
			{
				Chan->bot_is_op = FALSE;
				needop(Chan->name);
				break;
			}
		}
		/*
		 *  Idiots deopping themselves
		 */
		if (!Strcasecmp(CurrentNick,nick))
			break;
		/*
		 *  1. Use enfm var to temporarily store users access
		 *  2. get_userlevel also checks is_localbot()...
		 */
		if ((enfm = get_userlevel(from,channel)) >= OWNERLEVEL)
			break;
		if ((enfm < ASSTLEVEL) && check_mass(from,Chan,CHK_DEOP))
			mass_action(from,channel);
		if (togprot && get_protlevel(nickuh,channel) && !get_shitlevel(nickuh,channel))
		{
			if (!password_needed(nickuh) || verified(nickuh) || is_localbot(nickuh))
			{
				Strcat(unplusmode,"o");
				Strcat(unplusparams,nick);
				Strcat(unplusparams," ");
				prot_action(from,Chan,nick);
			}
		}
		}
		break;
/* v */
	case 'v':
		nick = chop(&params);
		if (sign == '+')
			add_mode(Chan,CU_VOICE,nick);
		else
			del_mode(Chan,CU_VOICE,nick);
		break;
/* b */
	case 'b':
		parm = chop(&params);
/* +b */	if (sign == '+')
		{
		make_ban(&Chan->banned,from,parm,now);
		if (get_int_varc(Chan,SETBT_VAR) && (num_banned(channel) == 20))
			channel_massunbanfrom(channel,"*!*@*",60 * get_int_varc(Chan,SETBT_VAR));
		if (CurrentUser && CurrentUser->access == BOTLEVEL)
			break;
		if (is_localbot(from))
			break;
		if (check_mass(from,Chan,CHK_BAN) && (!CurrentUser || CurrentUser->access < ASSTLEVEL))
			mass_action(from,channel);
		if (togprot && get_protuseraccess(parm,channel))
		{
			User = get_shituser(parm,channel);
			if (!User || !User->access)
			{
				Strcat(unminmode,"b");
				Strcat(unminparams,parm);
				Strcat(unminparams," ");
				prot_action(from,Chan,parm);
			}
		}
		}
/* -b */	else
		/* sign == '-' */
		{
		delete_ban(&Chan->banned,parm);
		if (!get_int_varc(Chan,TOGSHIT_VAR))
			break;
		User = get_shituser(parm,channel);
		i = (User) ? User->access : 0;
		if (i < 3)
		{
			User = find_shit(parm,channel);
			i = (User) ? User->access : 0;
		}
		if ((i >= 3) && (User->parm.shitmask))
		{
			Strcat(unplusmode,"b");
			Strcat(unplusparams,User->parm.shitmask);
			Strcat(unplusparams," ");
		}
		}
		break;
/* p */
	case 'p':
		enfm = reverse_mode(from,Chan,'p',sign);
		if (enfm && (sign == '+'))
			Strcat(unminmode,"p");
		else
		if (enfm)
			Strcat(unplusmode,"p");
		Chan->private = (sign == '+') ? TRUE : FALSE;
		break;
/* s */
	case 's':
		enfm = reverse_mode(from,Chan,'s',sign);
		if (enfm && (sign == '+'))
			Strcat(unminmode,"s");
		else
		if (enfm)
			Strcat(unplusmode,"s");
		Chan->secret = (sign == '+') ? TRUE : FALSE;
		break;
/* m */
	case 'm':
		enfm = reverse_mode(from,Chan,'m',sign);
		if (enfm && (sign == '+'))
			Strcat(unminmode,"m");
		else
		if (enfm)
			Strcat(unplusmode,"m");
		Chan->moderated = (sign == '+') ? TRUE : FALSE;
		break;
/* t */
	case 't':
		enfm = reverse_mode(from,Chan,'t',sign);
		if (enfm && (sign == '+'))
			Strcat(unminmode,"t");
		else
		if (enfm)
			Strcat(unplusmode,"t");
		Chan->topprot = (sign == '+') ? TRUE : FALSE;
		break;
/* i */
	case 'i':
		enfm = reverse_mode(from,Chan,'i',sign);
		if (enfm && (sign == '+'))
			Strcat(unminmode,"i");
		else
		if (enfm)
			Strcat(unplusmode,"i");
		Chan->invite = (sign == '+') ? TRUE : FALSE;
		break;
/* n */
	case 'n':
		enfm = reverse_mode(from,Chan,'n',sign);
		if (enfm && (sign == '+'))
			Strcat(unminmode,"n");
		else
		if (enfm)
			Strcat(unplusmode,"n");
		Chan->nomsg = (sign == '+') ? TRUE : FALSE;
		break;
/* k */
	case 'k':
		parm = chop(&params);
		enfm = reverse_mode(from,Chan,'k',sign);
		if (sign == '+')
		{
			Chan->keymode = TRUE;
			/*
			 *  Undernet clueless-coder-kludge
			 */
			Chan->hiddenkey = (parm) ? FALSE : TRUE;
			if (enfm && parm)
			{
				Strcat(unminmode,"k");
				Strcat(unminparams,parm);
				Strcat(unminparams," ");
			}
			MyFree(&Chan->key);
			set_mallocdoer(on_mode);
			mstrcpy(&Chan->key,(parm) ? parm : "???");
		}
		else
		{
			MyFree(&Chan->key);
			Chan->keymode = FALSE;
			if (enfm && parm)
			{
				Strcat(unplusmode,"k");
				Strcat(unplusparams,parm);
				Strcat(unplusparams," ");
			}
		}
		break;
/* l */
	case 'l':
		if (sign == '+')
		{
			parm = chop(&params);
			ip = Atoi(parm);
			Chan->limit = (ip) ? *ip : 0;
			Chan->limitmode = TRUE;
		}
		else
		{
			Chan->limit = 0;
			Chan->limitmode = FALSE;
		}
		enfm = reverse_mode(from,Chan,'l',sign);
		if (enfm)
		{
			if (sign == '+')
			{
				Strcat(unminmode,"l");
			}
			else
			{
				Strcat(unplusmode,"l");
				sprintf(unplusparams,"%s %i ",unplusparams,Chan->limit);
			}
		}
		break;
	default:
		break;
	}
	chanchars++;
	}
	/*
	 *  Restore unwanted modechanges
	 */
	if ((unplusmode[0] || unminmode[0]) && (Chan->bot_is_op))
		sendmode(channel,"+%s-%s %s %s",unplusmode,unminmode,unplusparams,unminparams);
}

void on_public(char *from, char *channel, char *rest)
{
	aUser	*triggers;
	aChan	*Chan;
	char	*temp;
	int	i;

	if ((Chan = find_channel(channel,CH_ACTIVE)) == NULL)
		return;

	send_spy(Chan,"<%s> %s",CurrentNick,rest);
	update_idletime(from,channel);

	if (!Chan->bot_is_op)
		return;

	if ((capslevel(rest) >= 50) && (check_mass(from,Chan,CHK_CAPS)))
	{
		sendkick(channel,CurrentNick,TEXT_LOTSOCAPS);
	}

	if ((i = check_mass(from,Chan,CHK_PUB)) > 0)
	{
		if (i >= 2)
			deop_ban(channel,CurrentNick,from);
		sendkick(channel,CurrentNick,TEXT_FLOODER);
		send_statmsg(TEXT_KICKFLOODER,from,channel);
	}

	if (get_int_varc(Chan,TOGKS_VAR))
	{
		if ((triggers = check_kicksay(rest,channel)) != NULL)
		{
			if (!CurrentUser || !CurrentUser->access)
			{
				temp = formatgreet(triggers->reason,CurrentNick);
				sendkick(channel,CurrentNick,"%s",temp);
			}
		}
	}
}

#ifdef DYNCMDLEVELS

void do_chaccess(char *from, char *to, char *rest, int cmdlevel)
{
	int	i,oldlevel,newlevel;
	char	*name,*level;

	name = chop(&rest);
	level = chop(&rest);

	if (!level && !name)
	{
		usage(from,C_CHACCESS);
		return;
	}

	if (level)
	{
		newlevel = atoi(level);
		if ((newlevel < 0) || (newlevel > OWNERLEVEL))
		{
			send_to_user(from,TEXT_LEVELBETWEEN,OWNERLEVEL);
			return;
		}
	}
	else
		newlevel = -1;

	if (newlevel > get_userlevel(from,NULL))
	{
		send_to_user(from,TEXT_NOCHANGELVLHIGH);
		return;
	}

	for(i=0;mcmd[i].name;i++)
	{
		if (!Strcasecmp(mcmd[i].name,name))
		{
			oldlevel = (mcmd[i].flags & CLEVEL);
			if (newlevel == -1)
			{
				send_to_user(from,TEXT_LEVELNEEDEDCMD,oldlevel);
				send_to_user(from,TEXT_SPECIFYLEVEL);
				return;
			}
			if (oldlevel > get_userlevel(from,NULL))
			{
				send_to_user(from,TEXT_NOCHGLVLHIGHYOURS);
				return;
			}
			if (oldlevel == newlevel)
				send_to_user(from,TEXT_LVLNOCHANGE);
			else
				send_to_user(from,TEXT_IMORTALLVLCHANGE,oldlevel, newlevel);
			mcmd[i].flags = (mcmd[i].flags & FLAGS) | newlevel;
			return;
		}
	}
	send_to_user(from,TEXT_CMDUNKNOWN);
}

#endif /* DYNCMDLEVELS */

int level_needed(char *name)
{
	int	i;

	for(i=0;mcmd[i].name;i++)
		if (!Strcasecmp(mcmd[i].name,name))
			return((mcmd[i].flags & CLEVEL) > 0) ? (mcmd[i].flags & CLEVEL) : 0;
	return(-1);
}

#ifdef DYNCMDLEVELS

int write_levelfile(void)
{
	int	i,f;
#ifdef DEBUG
	int	dodeb;
#endif /* DEBUG */

	if ((f = open(LEVELFILE,O_WRONLY|O_CREAT|O_TRUNC,0666)) < 0)
		return(FALSE);

#ifdef DEBUG
	dodeb = dodebug;
	dodebug = FALSE;
#endif /* DEBUG */
	for(i=0;mcmd[i].name;i++)
	{
		send_to_socket(f,"%s %d\n",mcmd[i].name,(mcmd[i].flags & CLEVEL));
	}
	close(f);
#ifdef DEBUG
	dodebug = dodeb;
#endif /* DEBUG */
	return(TRUE);
}

int read_levelfile_callback(char *level)
{
	char	*name;
	int	i,*ip;

	name = chop(&level);
	if (!name || !*name || !level || !*level)
		return(FALSE);
	ip = Atoi(level);
	if (!ip || *ip < 0 || *ip > OWNERLEVEL+1)
		return(FALSE);
	for(i=0;mcmd[i].name;i++)
	{
		if (!Strcasecmp(mcmd[i].name,name))
			mcmd[i].flags = (mcmd[i].flags & FLAGS) | *ip;
	}
	return(FALSE);
}

int read_levelfile(void)
{
	int	in;
#ifdef DEBUG
	int	dodeb;
#endif /* DEBUG */

	if ((in = open(LEVELFILE,O_RDONLY)) < 0)
		return(FALSE);

#ifdef DEBUG
	dodeb = dodebug;
	dodebug = FALSE;
#endif /* DEBUG */
	readline(in,read_levelfile_callback);
	close(in);
#ifdef DEBUG
	dodebug = dodeb;
#endif /* DEBUG */
	return(TRUE);
}

#endif /* DYNCMDLEVELS */

void usage(char *to, char *arg)
{
	char	*pt;
	int	i;

	for(i=0;ulist[i].command;i++)
	{
		if (!Strcasecmp(arg,ulist[i].command))
		{
			pt = ulist[i].usage;
			send_to_user(to,"Usage: %s%s",
				(*pt == '@') ? ulist[i].command : "",
				(*pt == '@') ? &pt[1] : pt);
			return;
		}
	}
	if (!Strcasecmp(arg,"w2k"))
	{
		send_to_user(to,TEXT_NOWIN2K);
		return;
	}
	send_to_user(to,TEXT_NOUSAGE);
}

void do_usage(char *from, char *to, char *rest, int cmdlevel)
{
	char	*cmd;
	int	i;

	cmd = chop(&rest);
	if (!cmd || !*cmd)
	{
		usage(from,C_USAGE);
		return;
	}
	for(i=0;mcmd[i].name;i++)
	{
		if (!Strcasecmp(cmd,mcmd[i].name))
		{
			usage(from,mcmd[i].name);
			return;
		}
		if (!Strcasecmp(cmd,mcmd[i].alias))
		{
			usage(from,mcmd[i].name);
			return;
		}
	}
	send_to_user(from,TEXT_UNKNOWNCMD,cmd);
}
