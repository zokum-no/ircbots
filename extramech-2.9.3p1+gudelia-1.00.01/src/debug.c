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
#ifdef DEBUG

#define boolstr(x)	(x) ? "TRUE" : "FALSE"

#ifdef SUPERDEBUG

struct
{
	char	*name;
	int	size;

} StructList[] =
{
{ "aBan",	sizeof(aBan)		},
{ "aBot",	sizeof(aBot)		},
{ "aBotLink",	sizeof(aBotLink)	},
{ "aChan",	sizeof(aChan)		},
{ "aChanUser",	sizeof(aChanUser)	},
{ "aDCC",	sizeof(aDCC)		},
{ "aKillSock",	sizeof(aKillSock)	},
{ "aLinkConf",	sizeof(aLinkConf)	},
{ "aMsgList",	sizeof(aMsgList)	},
{ "aPipeUser",	sizeof(aPipeUser)	},
{ "aSeen",	sizeof(aSeen)		},
{ "aServer",	sizeof(aServer)		},
{ "aStrp",	sizeof(aStrp)		},
{ "aTime",	sizeof(aTime)		},
{ "aUser",	sizeof(aUser)		},
{ "aVarName",	sizeof(aVarName)	},
{ "aWillWont",	sizeof(aWillWont)	},
{ "MLStub",	sizeof(MLStub)		},
{ "OnMsg",	sizeof(OnMsg)		},
{ "VarStruct",	sizeof(VarStruct)	},
{ "VarType",	sizeof(VarType)		},
{ "aME(*)",	sizeof(aME)		},
{ "aMEA(*)",	sizeof(aMEA)		},
{ NULL,		0			},
};

struct
{
	void	*func;
	char	*name;

} ProcList[] =
{
#ifdef LINKING
{	Link_addbot,		"Link_addbot"		},
#endif /* LINKING */
{	add_bot,		"add_bot"		},
{	add_to_shitlist,	"add_to_shitlist"	},
{	add_to_userlist,	"add_to_userlist"	},
{	addmasktouser,		"addmasktouser"		},
{	addstruct,		"addstruct"		},
{	cfg_addks,		"cfg_addks"		},
{	cfg_addshit,		"cfg_addshit"		},
#ifdef ALIASES
{	cfg_alias,		"cfg_alias"		},
#endif /* ALIASES */
{	cfg_reason,		"cfg_reason"		},
{	cfg_set,		"cfg_set"		},
{	cfg_uchannel,		"cfg_uchannel"		},
{	cfg_who,		"cfg_who"		},
{	copy_vars,		"copy_vars"		},
{	dcc_chat,		"dcc_chat"		},
{	dcc_connect,		"dcc_connect"		},
{	do_die,			"do_die"		},
{	do_kicksay,		"do_kicksay"		},
{	do_set,			"do_set"		},
{	join_channel,		"join_channel"		},
{	killsock,		"killsock"		},
{	make_ban,		"make_ban"		},
{	make_chanuser,		"make_chanuser"		},
#ifdef SEEN
{	make_seen,		"make_seen"		},
#endif /* SEEN */
{	make_time,		"make_time"		},
{	mech_exec,		"mech_exec"		},
{	on_join,		"on_join"		},
{	on_kick,		"on_kick"		},
{	on_mode,		"on_mode"		},
{	on_msg,			"on_msg"		},
{	on_nick,		"on_nick"		},
#ifdef PIPEUSER
{	parse_002,		"parse_002"		},
{	parse_003,		"parse_003"		},
{	parse_004,		"parse_004"		},
#endif /* PIPEUSER */
#ifdef LINKING
{	process_link_input,	"process_link_input"	},
#endif /* LINKING */
{	randstring_getline,	"randstring_getline"	},
{	reverse_topic,		"reverse_topic"		},
{	send_to_user,		"send_to_user"		},
#ifdef UPTIME
{	send_uptime,		"send_uptime"		},
#endif /* UPTIME */
{	set_enftopic,		"set_enftopic"		},
{	set_str_varc,		"set_str_varc"		},
{	sig_hup,		"sig_hup"		},
{	NULL,			NULL			},
};

char *proc_lookup(void *addr)
{
	int	i;

	for(i=0;ProcList[i].name;i++)
	{
		if (ProcList[i].func == addr)
			return(ProcList[i].name);
	}
	return(NULL);
}

#endif /* SUPERDEBUG */

typedef struct DEFstruct
{
	int	id;
	char	*idstr;

} DEFstruct;

#ifdef LINKING
DEFstruct LNdefs[] =
{
{ LN_UNKNOWN,		"LN_UNKNOWN"		},
{ LN_TRYING,		"LN_TRYING"		},
{ LN_CONNECTED,		"LN_CONNECTED"		},
{ LN_UPLINK,		"LN_UPLINK"		},
{ LN_USER,		"LN_USER"		},
{ LN_REMOTE,		"LN_REMOTE"		},
{ LN_LINKDEAD,		"LN_LINKDEAD"		},
{ LN_PASSWAIT,		"LN_PASSWAIT"		},
{ LN_ENTITY,		"LN_ENTITY"		},
{ LN_BOT,		"LN_BOT"		},
{ LN_DCCUSER,		"LN_DCCUSER"		},
{ LN_SAYNICK_SEND,	"LN_SAYNICK_SEND"	},
{ LN_PIPE_HASNICK,	"LN_PIPE_HASNICK"	},
{ 0,			NULL			},
};
#endif /* LINKING */

DEFstruct CNdefs[] =
{
{ CN_NOSOCK,		"CN_NOSOCK"		},
{ CN_TRYING,		"CN_TRYING"		},
{ CN_CONNECTED,		"CN_CONNECTED"		},
{ CN_ONLINE,		"CN_ONLINE"		},
{ CN_DISCONNECT,	"CN_DISCONNECT"		},
{ CN_BOTDIE,		"CN_BOTDIE"		},
{ CN_NEXTSERV,		"CN_NEXTSERV"		},
};

char *strdef(DEFstruct *dtab, int num)
{
	int	i;

	for(i=0;(dtab[i].idstr);i++)
	{
		if (dtab[i].id == num)
			return(dtab[i].idstr);
	}
	return("UNKNOWN");
}

#ifdef SUPERDEBUG

void memreset(void)
{
	aMEA	*mp;
	int	i;

	for(mp=mrrec;mp;mp=mp->next)
	{
		for(i=0;i<MRSIZE;i++)
			mp->mme[i].touch = FALSE;
	}
}

void memtouch(void *addr)
{
	aMEA	*mp;
	int	i;

	if (addr == NULL)
		return;
	for(mp=mrrec;mp;mp=mp->next)
	{
		for(i=0;i<MRSIZE;i++)
			if (mp->mme[i].area == addr)
			{
				mp->mme[i].touch = TRUE;
				return;
			}
	}
}

#endif /* SUPERDEBUG */

char *atime(time_t when)
{
	char	*pt,*zp;

	pt = ctime(&when);
	zp = strchr(pt,'\n');
	*zp = 0;
	return(pt);
}

void debug_server(aServer *sp, char *pad)
{
	int	pl;

	if (!sp)
	{
		debug("%s; ---\n",pad);
		return;
	}
	debug("%s; used\t\t%s\n",pad,boolstr(sp->used));
	if (sp->used)
	{
		pl = (strlen(pad) == 2);
		debug("%s; ident\t\t%i\n",pad,sp->ident);
		debug("%s; name\t\t\"%s\"\n",pad,nullbuf(sp->name));
		debug("%s; pass\t\t\"%s\"\n",pad,nullbuf(sp->pass));
		debug("%s; realname\t\t\"%s\"\n",pad,nullbuf(sp->realname));
		debug("%s; usenum\t\t%i\n",pad,sp->usenum);
		debug("%s; port\t\t%i\n",pad,sp->port);
		debug("%s; err\t\t%s%i\n",pad,(pl) ? "\t" : "",sp->err);
		debug("%s; lastconnect\t%s%s (%lu)\n",pad,(pl) ? "\t" : "",atime(sp->lastconnect),sp->lastconnect);
		debug("%s; lastattempt\t%s%s (%lu)\n",pad,(pl) ? "\t" : "",atime(sp->lastattempt),sp->lastattempt);
		debug("%s; maxontime\t\t%s (%lu)\n",pad,idle2str(sp->maxontime,FALSE),sp->maxontime);
	}
	debug("%s; ---\n",pad);
}

#ifdef SUPERDEBUG

char	tabs[20] = "\t\t\t\t\t\t\t\t\t\t";

void debug_memory(void)
{
	aChanUser *CU;
	aStrp	*sp;
	aChan	*Chan;
	aUser	*User;
#ifdef SEEN
	aSeen	*Seen;
#endif /* SEEN */
	aBot	*bot;
	aBan	*ban;
	aDCC	*Client;
	aMEA	*mea;
	char	t[100],*pt,*funcname;
	int	i;

	memreset();
	memtouch(serverBASE);
#ifdef LINKING
	memtouch(botlinkBASE);
#endif /* LINKING */
	for(bot=botlist;bot;bot=bot->next)
	{
		memtouch(bot);
		for(Chan=bot->Channels;Chan;Chan=Chan->next)
		{
			memtouch(Chan);
			memtouch(Chan->name);
			memtouch(Chan->key);
			memtouch(Chan->topic);
			memtouch(Chan->kickedby);
			for(CU=Chan->users;CU;CU=CU->next)
				memtouch(CU);
			for(ban=Chan->banned;ban;ban=ban->next)
			{
				memtouch(ban);
				memtouch(ban->banstring);
				memtouch(ban->bannedby);
			}
		}
		for(Client=bot->ClientList;Client;Client=Client->next)
		{
			memtouch(Client);
		}
		for(i=0;i<LASTCMDSIZE;i++)
		{
			memtouch(bot->lastcmds[i]);
		}
		memtouch(bot->msglistBASE);
		memtouch(bot->userlistBASE);
		User = bot->userlistBASE;
		for(i=0;i<bot->userlistSIZE;i++)
		{
			memtouch(User->chanp);
			if ((User->type == USERLIST_KICKSAY) || (User->type == USERLIST_SHIT))
			{
				memtouch(User->whoadd);
				memtouch(User->reason);
			}
			if (User->type == USERLIST_KICKSAY)
				memtouch(User->parm.kicksay);
			else
			if (User->type == USERLIST_SHIT)
				memtouch(User->parm.shitmask);
			else
			if (User->type == USERLIST_USER)
			{
				for(sp=User->parm.umask;sp;sp=sp->next)
					memtouch(sp);
			}
			User++;
		}
#ifdef SEEN
		memtouch(bot->seenfile);
		for(Seen=bot->SeenList;Seen;Seen=Seen->next)
			memtouch(Seen);
#endif /* SEEN */
#ifdef PIPEUSER
		memtouch(bot->pipeuser);
		memtouch(bot->saved_002);
		memtouch(bot->saved_003);
		memtouch(bot->saved_004);
#endif /* PIPEUSER */
	}
	debug("> Memory allocations\n");
	for(mea=mrrec;(mea);mea=mea->next)
	{
		for(i=0;i<MRSIZE;i++)
		{
			if (mea->mme[i].area)
			{
				if ((funcname = proc_lookup(mea->mme[i].doer)) != NULL)
					strcpy(t,funcname);
				else
					sprintf(t,mx_pfmt,(mx_ptr)mea->mme[i].doer);
				pt = &tabs[10 - ((23 - strlen(t)) / 8)];
				debug("  ; "mx_pfmt"\t\t%s%s\t%i\t%s\t%s\n",(mx_ptr)mea->mme[i].area,t,pt,mea->mme[i].size,
					(mea->mme[i].touch) ? "" : "(Leak)",atime(mea->mme[i].when));
			}
		}
	}
	debug("  ; ---\n");
}

#endif /* SUPERDEBUG */

void debug_core(void)
{
#ifdef SEEN
	aSeen	*Seen;
#endif /* SEEN */
	aChanUser *CU;
	aServer *sp;
	aStrp	*st;
	aUser	*User;
	aDCC	*Client;
	aChan	*Chan;
	aBot	*bot;
	char	*p;
	int	i;

	debug("; VERSION\t\t\"%s\"\n",VERSION);
	debug("; SRCDATE\t\t\"%s\"\n",SRCDATE);
	debug("; BOTLOGIN\t\t\"%s\"\n",BOTLOGIN);
	debug("; BOTCLASS\t\t\"%s\"\n",BOTCLASS);
#ifdef SUPERDEBUG
	debug("> StructList\n");
	for(i=0;StructList[i].name;i++)
	{
		debug("  ; %s\t\t%i\n",StructList[i].name,StructList[i].size);
	}
	debug("  ; ---\n");
#endif /* SUPERDEBUG */
	debug("; current\t\t"mx_pfmt"\n",(mx_ptr)current);
	debug("; backup\t\t"mx_pfmt"\n",(mx_ptr)backup);
	debug("; executable\t\t\"%s\"\n",executable);
	debug("; configfile\t\t\"%s\"\n",configfile);
	debug("> will\n");
	debug("  ; seen\t\t%s\n",boolstr(will.seen));
	debug("  ; shellcmd\t\t%s\n",boolstr(will.shellcmd));
	debug("  ; signals\t\t%s\n",boolstr(will.signals));
	debug("  ; onotice\t\t%s\n",boolstr(will.onotice));
	debug("  ; sessions\t\t%s\n",boolstr(will.sessions));
	debug("  ; autolink\t\t%s\n",boolstr(will.autolink));
	debug("; uptime\t\t%s (%lu)\n",atime(uptime),uptime);
	debug("; short_tv\t\t%s (%is wait)\n",boolstr(short_tv),(short_tv) ? 1 : 30);
	debug("; userident\t\t%i\n",userident);
	debug("; serverSIZE\t\t%i\n",serverSIZE);
	debug("> serverBASE\t\t"mx_pfmt"\n",(mx_ptr)serverBASE);
        sp = serverBASE;
        for(i=0;i<serverSIZE;i++)
	{
		debug_server(sp,"  ");
		sp++;
	}
	debug("> botlist\t\t"mx_pfmt"\n",(mx_ptr)botlist);
	for(bot=botlist;bot;bot=bot->next)
	{
		debug("  ; bot\t\t\t"mx_pfmt"\n",(mx_ptr)bot);
		debug("  ; next\t\t"mx_pfmt"\n",(mx_ptr)bot->next);
		debug("  ; connect\t\t%s (%i)\n",strdef(CNdefs,bot->connect),bot->connect);
		debug("  ; sock\t\t%i\n",bot->sock);
#ifdef IPV6
		debug("  ; ip\t\t\t%s\n",ip2string(bot->ip.sin6.sin6_addr));
#else
		debug("  ; ip\t\t\t%s\n",inet_ntoa(bot->ip.sin.sin_addr));
#endif /* IPV6 */
		debug("  > server\t\t%i\n",bot->server);
		sp = find_aServer(bot->server);
		if (sp)
		{
			debug("    ; Server\t\t"mx_pfmt"\n",(mx_ptr)sp);
			debug_server(sp,"    ");
		}
		debug("  > nextserver\t\t%i\n",bot->nextserver);
		sp = find_aServer(bot->nextserver);
		if (sp)
		{
			debug("    ; Server\t\t"mx_pfmt"\n",(mx_ptr)sp);
			debug_server(sp,"    ");
		}
		debug("  ; nick\t\t\"%s\"\n",bot->nick);
		debug("  ; wantnick\t\t\"%s\"\n",bot->wantnick);
		debug("  ; login\t\t\"%s\"\n",bot->login);
		debug("  ; ircname\t\t\"%s\"\n",bot->ircname);
		debug("  ; cmdchar\t\t'%c'\n",bot->cmdchar);

		debug("  ; userlistSIZE\t%i\n",bot->userlistSIZE);
		debug("  > userlistBASE\t"mx_pfmt"\n",(mx_ptr)bot->userlistBASE);
		User = bot->userlistBASE;
		for(i=0;i<bot->userlistSIZE;i++)
		{
			debug("    ; User\t\t"mx_pfmt"\n",(mx_ptr)User);
			debug("    ; used\t\t%s\n",boolstr(User->used));
			switch(User->type)
			{
			case USERLIST_USER:
				p = "USERLIST_USER";
				break;
			case USERLIST_KICKSAY:
				p = "USERLIST_KICKSAY";
				break;
			case USERLIST_SHIT:
				p = "USERLIST_SHIT";
				break;
			default:
				p = "(unknown)";
			}
			debug("    ; type\t\t%s (%i)\n",p,User->type);
			if (User->used)
			{
				debug("    ; ident\t\t%i\n",User->ident);
				debug("    ; chan\t\t\"%s\"\n",nullstr(User->chanp));
				if (User->type == USERLIST_USER)
				{
					debug("    ; name\t\t\"%s\"\n",User->name);
					debug("    ; pass\t\t\"%s\"\n",User->pass);
					debug("    > parm.umask\t"mx_pfmt"\n",(mx_ptr)User->parm.umask);
					st = User->parm.umask;
					while(st)
					{
						debug("      ; next\t\t"mx_pfmt"\n",(mx_ptr)st->next);
						debug("      ; mask\t\t\"%s\"\n",st->p);
						st = st->next;
					}
					debug("      ; ---\n");
				}
				if ((User->type == USERLIST_KICKSAY) || (User->type == USERLIST_SHIT))
				{
					debug("    ; whoadd\t\t\"%s\"\n",nullstr(User->whoadd));
					debug("    ; reason\t\t\"%s\"\n",nullstr(User->reason));
				}
				if (User->type == USERLIST_KICKSAY)
				{
					debug("    ; parm.kicksay\t\"%s\"\n",nullstr(User->parm.kicksay));
				}
				if (User->type == USERLIST_SHIT)
				{
					debug("    ; parm.shitmask\t\"%s\"\n",nullstr(User->parm.shitmask));
					debug("    ; time\t\t%s\n",atime(User->time));
					debug("    ; expire\t\t%s\n",atime(User->expire));
				}
			}
			debug("    ; ---\n");
			User++;
		}
		debug("  > Channels\t\t"mx_pfmt"\n",(mx_ptr)bot->Channels);
		for(Chan=bot->Channels;Chan;Chan=Chan->next)
		{
			debug("    ; channel\t\t"mx_pfmt"\n",(mx_ptr)Chan);
			debug("    ; name\t\t\"%s\"\n",Chan->name);
			debug("    > Chan->users\n");
			for(CU=Chan->users;CU;CU=CU->next)
			{
				debug("      ; next\t\t"mx_pfmt"\n",(mx_ptr)CU->next);
				debug("      ; nick\t\t\"%s\"\n",nullstr(CU->nick));
				debug("      ; user@host\t\"%s@%s\"\n",nullstr(CU->user),nullstr(CU->host));
				debug("      ; ---\n");
			}
			debug("    ; ---\n");
		}
		debug("  ; CurrentChan\t\t"mx_pfmt"\n",(mx_ptr)bot->CurrentChan);
		debug("  > ClientList\t\t"mx_pfmt"\n",(mx_ptr)bot->ClientList);
		for(Client=bot->ClientList;Client;Client=Client->next)
		{
			debug("    ; Client\t\t"mx_pfmt"\n",(mx_ptr)Client);
			debug("    ; ---\n");
		}
#ifdef SEEN
		debug("  ; seenfile\t\t\"%s\"\n",nullstr(bot->seenfile));
		debug("  > SeenList\t\t"mx_pfmt"\n",(mx_ptr)bot->SeenList);
		for(Seen=bot->SeenList;Seen;Seen=Seen->next)
		{
			debug("    ; Seen\t\t"mx_pfmt"\n",(mx_ptr)Seen);
			debug("    ; next\t\t"mx_pfmt"\n",(mx_ptr)Seen->next);
			debug("    ; nick\t\t"mx_pfmt" -> \"%s\"\n",(mx_ptr)Seen->nick,nullstr(Seen->nick));
			debug("    ; userhost\t\t"mx_pfmt" -> \"%s\"\n",(mx_ptr)Seen->userhost,nullstr(Seen->userhost));
			debug("    ; signoff\t\t"mx_pfmt" -> \"%s\"\n",(mx_ptr)Seen->signoff,nullstr(Seen->signoff));
			debug("    ; kicker\t\t"mx_pfmt" -> \"%s\"\n",(mx_ptr)Seen->kicker,nullstr(Seen->kicker));
			debug("    ; time\t\t%s (%lu)\n",atime(Seen->time),Seen->time);
			debug("    ; selector\t\t%i\n",Seen->selector);
			debug("    ; ---\n");
		}
#endif /* SEEN */
		debug("  ; lastreset\t\t%s (%lu)\n",atime(bot->lastreset),bot->lastreset);
		debug("  ; lastping\t\t%s (%lu)\n",atime(bot->lastping),bot->lastping);
		debug("  ; lastrejoin\t\t%s (%lu)\n",atime(bot->lastrejoin),bot->lastrejoin);
		debug("  ; lastsave\t\t%s (%lu)\n",atime(bot->lastsave),bot->lastsave);
		debug("  ; laststatus\t\t%s (%lu)\n",atime(bot->laststatus),bot->laststatus);
		debug("  ; lastctcp\t\t%s (%lu)\n",atime(bot->lastctcp),bot->lastctcp);
		debug("  ; conntry\t\t%s (%lu)\n",atime(bot->conntry),bot->conntry);
		debug("  ; activity\t\t%s (%lu)\n",atime(bot->activity),bot->activity);
		debug("  ; uptime\t\t%s (%lu)\n",atime(bot->uptime),bot->uptime);
		debug("  ; ontime\t\t%s (%lu)\n",atime(bot->ontime),bot->ontime);
		debug("  ; ---\n");
	}
}

#ifdef LINKING

void debug_links(void)
{
	aLinkConf *lc;
	aBotLink *bl,*bl2;
	int	i;

	debug("; my_entity\t\t\"%s\"\n",nullbuf(my_entity));
	debug("; my_pass\t\t\"%s\"\n",nullbuf(my_pass));
	debug("; linkport\t\t%i\n",linkport);
	debug("; linksock\t\t%i\n",linksock);
	debug("; last_autolink\t\t%s (%lu)\n",atime(last_autolink),last_autolink);
	debug("; botlinkident\t\t%i\n",botlinkident);
	debug("> botlinkBASE\t\t"mx_pfmt"\n",(mx_ptr)botlinkBASE);

	bl = botlinkBASE;
	for(i=0;i<botlinkSIZE;i++)
	{
		debug("  ; Link\t\t"mx_pfmt"\n",(mx_ptr)bl);
		debug("  ; used\t\t%s\n",boolstr(bl->used));
		if (bl->used)
		{
			debug("  ; sock\t\t%i\n",bl->sock);
			debug("  ; nick\t\t\"%s\"\n",nullbuf(bl->nick));
			debug("  ; ident\t\t%i\n",bl->ident);
			debug("  ; linktype\t\t%s (%i)\n",strdef(LNdefs,bl->linktype),bl->linktype);
			debug("  ; nodetype\t\t%s (%i)\n",strdef(LNdefs,bl->nodetype),bl->nodetype);
			bl2 = find_aBotLink(bl->uplink);
			debug("  ; uplink\t\t%i: \"%s\"\n",bl->uplink,(bl2) ? bl2->nick : NULLSTR);
			bl2 = find_aBotLink(bl->sendlink);
			debug("  ; sendlink\t\t%i: \"%s\"\n",bl->sendlink,(bl2) ? bl2->nick : NULLSTR);
		}
		debug("  ; ---\n");
		bl++;
	}

	debug("> linkconfBASE\t\t"mx_pfmt"\n",(mx_ptr)linkconfBASE);
	lc = linkconfBASE;
	for(i=0;i<linkconfSIZE;i++)
	{
		debug("%-4i: Ptr      = 0x"mx_pfmt"\n",i,(mx_ptr)lc);
		if (!lc->used)
			debug("\tUnused\n");
		debug("\tIdent    = %i\n",lc->ident);
		debug("\tName     = '%s'\n",nullbuf(lc->name));
		debug("\tPass     = '%s'\n",nullbuf(lc->pass));
		debug("\tHost     = '%s'\n",nullbuf(lc->host));
		debug("\tPort     = %i\n",lc->port);
		debug("\tAutoIdx  = %i\n",lc->autoidx);
		lc++;
	}
}

#endif /* LINKING */

void do_debug(char *from, char *to, char *rest, int cmdlevel)
{
	char	fname[FILENAMELEN];
	int	fd,backup_fd,backup_dodebug;

	debug("(do_debug) init...\n");

	backup_dodebug = dodebug;
	backup_fd = debug_fd;
	sprintf(fname,"debug.%lu",now);
	if ((fd = open(fname,O_WRONLY|O_CREAT|O_TRUNC,0666)) < 0)
	{
		if (from)
			send_to_user(from,"Unable to write debug information to file\n");
		return;
	}
	debug_fd = fd;
	dodebug = TRUE;

	debug_core();

#ifdef LINKING
	debug_links();
#endif /* LINKING */
#ifdef SUPERDEBUG
	debug_memory();
#endif /* SUPERDEBUG */

	close(fd);
	debug_fd = backup_fd;
	dodebug = backup_dodebug; 

	debug("(do_debug) all done.\n");
	if (from)
		send_to_user(from,"Debug information has been written to file");
}

void debug(char *format, ...)
{
	va_list msg;

	if (!dodebug)
		return;

	if (debug_fd == -1)
	{
		if (debugfile[0])
		{
			if ((debug_fd = open(debugfile,O_CREAT|O_TRUNC|O_WRONLY,S_IRUSR|S_IWUSR)) < 0)
			{
				dodebug = FALSE;
				return;
			}
		}
		else
		{
			debug_fd = 0;
		}
	}

	va_start(msg,format);
	vsprintf(debugbuf,format,msg); 
	va_end(msg);

	if ((write(debug_fd,debugbuf,strlen(debugbuf))) < 0)
		dodebug = FALSE;
}

#endif /* DEBUG */
