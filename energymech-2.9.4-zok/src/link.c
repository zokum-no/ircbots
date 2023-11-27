/*

    EnergyMech, IRC bot software
    Copyright (c) 1997-2001 proton, 2002-2004 emech-dev

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

void needop(char *channel)
{
	aChan	*Chan;
#ifdef MULTI
	aBot	*bot;
#endif /* MULTI */

	if ((Chan = find_channel(channel,CH_ACTIVE)) == NULL)
		return;
	if ((now - Chan->last_needop) < 10)
		return;
	Chan->last_needop = now;

#ifdef MULTI
	backup = current;
	for(bot=botlist;bot;bot=bot->next)
	{
		current = bot;
		if ((Chan = find_channel(channel,CH_ACTIVE)) != NULL)
		{
			if (Chan->bot_is_op)
			{
				sendmode(channel,"+o %s",backup->nick);
				current = backup;
				return;
			}
		}
	}
	current = backup;
#endif /* MULTI */
#ifdef LINKING
	link_bcast(NULL,"$NEEDOP %s %s\n",channel,current->nick);
#endif
}


#ifdef LINKING

#ifdef LINKEVENTS

char	lebuf[2048];

void wle(char *format, ...)
{
	va_list msg;

	va_start(msg,format);
	vsprintf(lebuf,format,msg);
	va_end(msg);

	if (LE < 0)
	{
		LE = open("LinkEvents",O_WRONLY|O_CREAT|O_TRUNC,0666);
	}
	send_to_socket(LE,"%lu  %s",now,lebuf);
}

#endif /* LINKEVENTS */

void setbotnick(aBot *bot, char *nick)
{
	if (!strcmp(bot->nick,nick))
		return;

#ifdef LINKEVENTS
	wle("New Nick    %-10s -> %s\n",bot->nick,nick);
#endif /* LINKEVENTS */

	link_bcast(NULL,"$NICK %s %s %s\n",my_entity,bot->nick,nick);
	strcpy(bot->nick,nick);
}


void link_dccuser(char *nick, int add_del)
{
#ifdef DEBUG
	debug("(link_dccuser) %s DCC user = '%s'\n",(add_del) ? "adding" : "deleting",nick);
#endif /* DEBUG */

	if (add_del)
	{
		link_bcast(NULL,"$LINK USER %s %s %s\n",my_entity,current->nick,nick);
	}
	else
	{
		link_bcast(NULL,"$QUIT %s %s %s\n",my_entity,current->nick,nick);
	}
}

aBotLink *find_entity(char *name, int lntype)
{
	aBotLink *bl;
	int	i;

	bl = botlinkBASE;
	for(i=0;i<botlinkSIZE;i++)
	{
		if ((bl->used) && (bl->nodetype == lntype) && (!Strcasecmp(bl->nick,name)))
			return(bl);
		bl++;
	}
	return(NULL);
}

void remlink(aBotLink *Link, aBotLink *from)
{
	aLinkConf *lc;
	aBotLink *bl,*bl2,*bl3;
	int	i,j,olt;

#ifdef DEBUG
	debug("(remlink) Removing link...\n");
#endif

	if (Link->sock != -1) close(Link->sock);

	if (Link->linktype == LN_UPLINK)
	{
		/*
		 *  Make it so that autolinking to this entity will work again
		 */
		lc = linkconfBASE;
		for(i=0;i<linkconfSIZE;i++)
		{
			if ((lc->used) && (lc->autoidx == LC_LINKED))
			{
#ifdef LINKEVENTS
				wle("LC %s ( H:%s P:%i ) reset to LC_NOTRY (remlink)\n",
					lc->name,lc->host,lc->port);
#endif
				lc->autoidx = LC_NOTRY;
			}
			lc++;
		}
	}

	olt = Link->linktype;

	/*
	 *  First, mark all dead links
	 */
	bl = botlinkBASE;
	for(i=0;i<botlinkSIZE;i++)
	{
		if (bl->used)
		{
			if (bl == Link)
				bl->linktype = LN_LINKDEAD;
			if (bl->uplink == Link->ident)
				bl->linktype = LN_LINKDEAD;
			if (bl->sendlink == Link->ident)
				bl->linktype = LN_LINKDEAD;
		}
		bl++;
	}

	if (Link->auth != TRUE)
		goto rem;
	if (olt == LN_TRYING)
		goto rem;
	if (olt == LN_CONNECTED)
		goto rem;

	/*
	 *  Send $QUIT to all _other_ mechs so they know who we lost...
	 */
	bl = botlinkBASE;
	for(i=0;i<botlinkSIZE;i++)
	{
		if ((bl->used) && (bl->linktype == LN_LINKDEAD))
		{
			switch(bl->nodetype)
			{
			case LN_ENTITY:
				bl2 = botlinkBASE;
				for(j=0;j<botlinkSIZE;j++)
				{
					if ((bl2->used) && (bl2 != from) && (bl2->linktype == LN_UPLINK))
					{
						send_to_link(bl2,"$QUIT %s\n",bl->nick);
					}
					bl2++;
				}
				break;
			case LN_DCCUSER:
				bl2 = botlinkBASE;
				for(j=0;j<botlinkSIZE;j++)
				{
					if ((bl2->used) && (bl2 != from) && (bl2->linktype == LN_UPLINK))
					{
						bl3 = find_aBotLink(bl->uplink);
						send_to_link(bl2,"$QUIT %s %s %s\n",bl3->nick,bl->nick,bl->dccnick);
					}
					bl2++;
				}
				break;
			case LN_BOT:
				bl2 = botlinkBASE;
				for(j=0;j<botlinkSIZE;j++)
				{
					if ((bl2->used) && (bl2 != from) && (bl2->linktype == LN_UPLINK))
					{
						bl3 = find_aBotLink(bl->uplink);
						send_to_link(bl2,"$QUIT %s %s\n",bl3->nick,bl->nick);
					}
					bl2++;
				}
				break;
			}
		}
		bl++;
	}
rem:
	/*
	 *  Then remove them
	 */
	bl = botlinkBASE;
	for(i=0;i<botlinkSIZE;i++)
	{
		if ((bl->used) && (bl->linktype == LN_LINKDEAD))
		{
#ifdef LINKEVENTS
			if (bl->nodetype == LN_DCCUSER)
				wle("Removing    %-10s ( "mx_pfmt" -- S:%i I:%i U:%i L:%i ) DCCUSER (%s)\n",
					(bl->dccnick) ? bl->dccnick : NULLSTR,(mx_ptr)bl,bl->sock,bl->ident,
					bl->uplink,bl->sendlink,(bl->nick) ? bl->nick : NULLSTR);
			else
				wle("Removing    %-10s ( "mx_pfmt" -- S:%i I:%i U:%i L:%i )\n",
					(bl->nick) ? bl->nick : NULLSTR,(mx_ptr)bl,bl->sock,bl->ident,
					bl->uplink,bl->sendlink);
#endif /* LINKEVENTS */
			
			memset(bl,0,sizeof(aBotLink));
		}
		bl++;
	}
#ifdef DEBUG
	debug("(remlink) Link removed.\n");
#endif /* DEBUG */
}

void do_linkcmd(char *from, char *to, char *rest, int cmdlevel)
{
	aBot	*bot;
	char	buf[MAXLEN];
	char	*doer;

	if (!rest || !*rest)
	{
		usage(from,C_CMD);
		return;
	}

	if (*rest == '=')
	{
		rest++;
		doer = chop(&rest);
		if (!doer || !*doer)
		{
			usage(from,C_CMD);
			return;
		}
	}
	else
		doer = "*";

	backup = current;
	for(bot=botlist;bot;bot=bot->next)
	{
		current = bot;
		if ((is_me(doer)) || (*doer == '*'))
		{
			sprintf(buf,"%c%s",current->cmdchar,rest);
			CurrentUser = find_user(from,to);
			CurrentShit = NULL;
			strcpy(CurrentNick,getnick(from));
			on_msg(from,current->nick,buf);
			if (*doer != '*')
				return;		/* found target already */
		}
	}
	current = backup;

	link_bcast(NULL,"$CMD $%s@%s %s %s\n",current->nick,from,doer,rest);

#ifdef DEBUG
	debug("(do_linkcmd) $CMD $%s@%s %s %s\n",current->nick,from,doer,rest);
#endif /* DEBUG */
}

void addlinkconf(char *name, char *pass, char *host, int port)
{
	aLinkConf *lc,new;
	int	i;

	if (!Strcasecmp(name,my_entity))
		return;

	lc = linkconfBASE;
	for(i=0;i<linkconfSIZE;i++)
	{
		if ((lc->used) && (!Strcasecmp(lc->name,name)))
			return;
		lc++;
	}

	memset(&new,0,sizeof(aLinkConf));
	new.used = TRUE;
	strcpy(new.name,name);
	strcpy(new.pass,pass);
	if (host && *host)
		strcpy(new.host,host);
	new.port = port;
	new.autoidx = LC_NOTRY;
	if (new.port == 0)
		new.autoidx = LC_NOPORT;
	add_aLinkConf(&new);
}

void do_link(char *from, char *to, char *rest, int cmdlevel)
{
	aLinkConf *lc,*lc2;
	aBotLink *bl;
	char	tmp[MSGLEN];
	char	*arg;
	int	i,valid;

#ifdef DEBUG
	debug("(do_link) %s\n",(rest)?rest:"(null)");
#endif

	if (!my_entity[0])
	{
		send_to_user(from,TEXT_NOENTITYSET);
		return;
	}

	arg = chop(&rest);
	if (!arg)
	{
		valid = 0;
		send_to_user(from,TEXT_LINKHEADER);
		lc = linkconfBASE;
		for(i=0;i<linkconfSIZE;i++)
		{
			if (lc->used)
			{
				if (lc->port)
					sprintf(tmp,"%i",lc->port);
				else
					strcpy(tmp,"none");
				send_to_user(from,"  %-11s  %-13s  %-30s  %s",lc->name,lc->pass,
					(lc->host) ? lc->host:"none",tmp);
				valid++;
			}
			lc++;
		}
		send_to_user(from,TEXT_LINKFOOTER,valid);
		return;
	}
	if (!Strcasecmp(arg,"UP"))
	{
		aBotLink new;

		arg = chop(&rest);
		if (!arg)
		{
			send_to_user(from,TEXT_USAGELINKUP);
			return;
		}
		if (!Strcasecmp(arg,my_entity))
		{
			send_to_user(from,TEXT_ENTITYISME,arg);
			return;
		}
		lc = linkconfBASE;
		lc2 = NULL;
		for(i=0;i<linkconfSIZE;i++)
		{
			if ((lc->used) && (!Strcasecmp(lc->name,arg)))
			{
				lc2 = lc;
				break;
			}
			lc++;
		}
		if (!lc2)
		{
			send_to_user(from,TEXT_ENTITYUNKNOWN,arg);
			return;
		}
		lc = lc2;
		if (!*lc->host)
		{
			send_to_user(from,TEXT_ENTITYNOHOST,arg);
			return;
		}
		if (lc->port == 0)
		{
			send_to_user(from,TEXT_ENTITYNOPORT,arg);
			return;
		}
		bl = botlinkBASE;
		for(i=0;i<botlinkSIZE;i++)
		{
			if ((bl->used) && (!Strcasecmp(bl->nick,arg)))
			{
				send_to_user(from,TEXT_ENTITYALREADYLINK,arg);
				return;
			}
			bl++;
		}
		memset(&new,0,sizeof(aBotLink));
		new.used = TRUE;
		new.ident = botlinkident++;
		new.auth = TRUE;
		strcpy(new.nick,arg);
		new.uptime = now;
		new.linktype = LN_TRYING;
		new.sock = SockConnect(lc->host,lc->port,FALSE);
		add_aBotLink(&new);
		send_to_user(from,TEXT_ENTITYTRYLINK,arg);
		return;
	}
	if (!Strcasecmp(arg,"ADD"))
	{
		char	*name,*pass,*host,*ps;
		int	port;

		name = chop(&rest);
		pass = chop(&rest);
		host = chop(&rest);
		ps = chop(&rest);
		if (!name || !*name || !pass || !*pass)
		{
			usage(from,"LINK-ADD");
			return;
		}
		if (!host || !*host)
			host = "none";
		if (!ps || !*ps)
			port = 0;
		else
			port = atoi(ps);
		addlinkconf(name,pass,host,port);
		send_to_user(from,TEXT_ENTITYADDED,name);
		return;
	}
	if (!Strcasecmp(arg,"DEL"))
	{
		char	*name;

		name = chop(&rest);
		if (!name)
		{
			usage(from,"LINK-DEL");
			return;
		}
		lc = linkconfBASE;
		for(i=0;i<linkconfSIZE;i++)
		{
			if ((lc->used) && (!Strcasecmp(lc->name,name)))
			{
				memset(lc,0,sizeof(aLinkConf));
				send_to_user(from,TEXT_ENTITYREMOVED,name);
				return;
			}
			lc++;
		}
		send_to_user(from,TEXT_ENTITYNOTFOUND,name);
		return;
	}
	if (!Strcasecmp(arg,"DOWN"))
	{
		aUser	*User;
		char	*name;

		name = chop(&rest);

		if ((bl = find_entity(name,LN_ENTITY)) == NULL)
		{
			send_to_user(from,TEXT_ENTITYUNKNOWN,name);
			return;
		}

		User = find_user(from,NULL);

		link_bcast(NULL,TEXT_ENTITYFORCEDELINK,
			name,getnick(from),User->access,my_entity);

		remlink(bl,NULL);
		send_to_user(from,TEXT_ENTITYUNLINKED,name);
		return;
	}
	if (!Strcasecmp(arg,"PORT"))
	{
		char	*ps;
		int	*port;

		ps = chop(&rest);
		if (!ps || !*ps)
		{
			usage(from,"LINK-PORT");
			return;
		}
		port = Atoi(ps);
#ifdef DEBUG
		debug("(do_linkcmd) linkport, ps = %s, port = %i\n",nullstr(ps),(port) ? *port : 0);
#endif
		if (!port || (*port < 1) || (*port > 65535))
		{
			send_to_user(from,TEXT_ENTITYINVALIDPORT);
			return;
		}
		send_to_user(from,TEXT_ENTITYCHANGEPORT,*port);
		close(linksock);
		linksock = -1;
		linkport = *port;
		return;
	}
	usage(from,C_LINK);
}

void link_write(aBotLink *Link, char *arg)
{
#ifdef DEBUG
	debug("[LiW] N:%s B:%s",(Link->nick[0]) ? Link->nick : "<unknown>",arg);
#endif /* DEBUG */
#ifdef LINKEVENTS
	wle("Net Write   %-10s ( "mx_pfmt" -- S:%i I:%i U:%i L:%i ): %s",
		(Link->nick[0]) ? Link->nick : NULLSTR,(mx_ptr)Link,Link->sock,Link->ident,
		Link->uplink,Link->sendlink,arg);
#endif /* LINKEVENTS */
	if (write(Link->sock,arg,strlen(arg)) < 0)
	{
#ifdef LINKEVENTS
		wle("Write Error %-10s ( "mx_pfmt" -- S:%i I:%i U:%i L:%i ): %s (%i)\n",
			(Link->nick[0]) ? Link->nick : NULLSTR,(mx_ptr)Link,
			Link->sock,Link->ident,Link->uplink,Link->sendlink,
			strerror(errno),errno);
#endif /* LINKEVENTS */
		remlink(Link,NULL);
	}
}

void send_to_link(aBotLink *Link, char *format, ...)
{
	char	buf[HUGE];
	va_list msg;

	if ((Link->sock == -1) || (Link->linktype == LN_LINKDEAD))
		return;
	va_start(msg,format);
	vsprintf(buf,format,msg);
	va_end(msg);

	link_write(Link,buf);
}

void link_bcast(aBotLink *ExLink, char *format, ...)
{
	aBotLink *bl;
	char	buf[HUGE];
	va_list	msg;
	int	i;

	va_start(msg,format);
	vsprintf(buf,format,msg);
	va_end(msg);

	bl = botlinkBASE;
	for(i=0;i<botlinkSIZE;i++)
	{
		if ((bl->used) && (bl != ExLink) && (bl->linktype == LN_UPLINK))
			link_write(bl,buf);
		bl++;
	}
}

void newlinksock(int newsock)
{
	aBotLink new;
	int	flags;

#ifdef DEBUG
	debug("(newsocklink) Adding new link...\n");
#endif
	flags = O_NONBLOCK | fcntl(newsock,F_GETFL);
	fcntl(newsock,F_SETFL,flags);

	memset(&new,0,sizeof(aBotLink));
	new.used = TRUE;
	new.ident = botlinkident++;
	new.auth = FALSE;
	new.sock = newsock;
	new.uptime = now;
	new.uplink = 0;
	new.linktype = LN_UNKNOWN;
	new.nodetype = LN_SAYNICK_SEND;
	short_tv |= TV_TELNET_NICK;
	add_aBotLink(&new);
}

int checklinkpass(char *ent, char *pass)
{
	aLinkConf *lc;
	int	i;

	lc = linkconfBASE;
	for(i=0;i<linkconfSIZE;i++)
	{
		if ((lc->used) && (!Strcasecmp(lc->name,ent)))
		{
			if (!strcmp(lc->pass,pass))
				return(TRUE);
			return(FALSE);
		}
		lc++;
	}
	return(FALSE);
}

void Link_addbot(aBotLink *Link, char *args)
{
	aBotLink *bl,*bl2;
	aBot	*bot;
	char	*entity,*pass;
	int	i;

	if (!args || !*args)
		return;

	entity = chop(&args);
	pass = chop(&args);

	if (!entity || !pass)
	{
		close(Link->sock);
		memset(Link,0,sizeof(aBotLink));
		return;
	}

#ifdef DEBUG
	debug("(Link_addbot) entity = '%s' pass = '%s'\n",entity,pass);
#endif /* DEBUG */

	if ((strlen(entity) >= ENTITYLEN) || (strlen(pass) >= ENTITYLEN))
	{
		close(Link->sock);
		memset(Link,0,sizeof(aBotLink));
		return;
	}

	bl = botlinkBASE;
	for(i=0;i<botlinkSIZE;i++)
	{
		if ((bl->used) && (bl->nodetype == LN_ENTITY) &&
		    (!Strcasecmp(entity,bl->nick)))
		{
#ifdef DEBUG
			debug("(Link_addbot) entity already linked\n");
#endif /* DEBUG */
			close(Link->sock);
			memset(Link,0,sizeof(aBotLink));
			return;
		}
		bl++;
	}

	/** authenticate the link if its not already authenticated **/
	if (!(Link->auth) && !checklinkpass(entity,pass))
	{
#ifdef DEBUG
		debug("(Link_addbot) Authentication failed\n");
#endif /* DEBUG */
		remlink(Link,NULL);
		return;
	}

	strcpy(Link->nick,entity);
	Link->auth = TRUE;
	Link->linktype = LN_UPLINK;
	Link->nodetype = LN_ENTITY;

	send_global_statmsg(TEXT_LINKUP,entity);

	/*
	 *  If he's sending the pass, we must send a $LOGIN...
	 */
	if (Strcasecmp(pass,"AUTH"))
		send_to_link(Link,"$LOGIN %s AUTH\n",my_entity);

	/*
	 *  We send this on to everyone in the botnet
	 */
	link_bcast(Link,"$LINK ENTITY %s\n",entity);

	/*
	 *  Send our active bots as $LINK BOTs
	 */
	for(bot=botlist;bot;bot=bot->next)
		send_to_link(Link,"$LINK BOT %s %s\n",my_entity,bot->nick);

	/*
	 *  Linking-table
	 *  First send all entities
	 */
	bl = botlinkBASE;
	for(i=0;i<botlinkSIZE;i++)
	{
		if ((bl->used) && (bl != Link) && (bl->nodetype == LN_ENTITY))
			send_to_link(Link,"$LINK ENTITY %s\n",bl->nick);
		bl++;
	}

	/*
	 *  Then all botlinks
	 */
	bl = botlinkBASE;
	for(i=0;i<botlinkSIZE;i++)
	{
		if ((bl->used) && (bl != Link) && (bl->nodetype == LN_BOT))
		{
			bl2 = find_aBotLink(bl->uplink);
			send_to_link(Link,"$LINK BOT %s %s\n",bl2->nick,bl->nick);
		}
		bl++;
	}

#ifdef DEBUG
	debug("(Link_addbot) Link added = '%s'\n",Link->nick);
#endif /* DEBUG */
}

void Link_quit(aBotLink *Link, char *args)
{
	aBotLink *bl;
	char	*entity,*bot,*nick;
	int	i;

	if (!args || !*args)
		return;

	entity = chop(&args);
	bot = chop(&args);
	nick = chop(&args);

#ifdef DEBUG
	debug("(Link_quit) removing :: entity = %s, bot = %s, nick = %s\n",
		entity,nullstr(bot),nullstr(nick));
#endif /* DEBUG */

	if ((!bot || !*bot) && !Strcasecmp(Link->nick,entity))
	{
		remlink(Link,NULL);
		return;
	}

	if (nick && *nick)
	{
		bl = botlinkBASE;
		for(i=0;i<botlinkSIZE;i++)
		{
			if ((bl->used) && (bl->nodetype == LN_DCCUSER)
			    && (!Strcasecmp(bl->dccnick,nick)) && (!Strcasecmp(bl->nick,bot)))
			{
				remlink(bl,Link);
				return;
			}
			bl++;
		}
	}
	else
	if (bot && *bot)
	{
		bl = botlinkBASE;
		for(i=0;i<botlinkSIZE;i++)
		{
			if ((bl->used) && (bl->nodetype == LN_BOT)
			    && (!Strcasecmp(bl->nick,bot)))
			{
				remlink(bl,Link);
				return;
			}
			bl++;
		}
	}
	else
	{
		bl = botlinkBASE;
		for(i=0;i<botlinkSIZE;i++)
		{
			if ((bl->used) && (bl->nodetype == LN_ENTITY) && (!Strcasecmp(bl->nick,entity)))
			{
				remlink(bl,Link);
				return;
			}
			bl++;
		}
	}
}

void Link_msg(aBotLink *Link, char *args)
{
	aDCC	*Client;
	aBot	*bot;
	char	*from;

	if (!args || !*args)
		return;
	from = chop(&args);
	if (!args || !*args)
		return;

	if (!strcmp(from,"$"))
	{
		send_global_statmsg("%s",args);
	}
	else
	for(bot=botlist;bot;bot=bot->next)
	{
		current = bot;
		for(Client=current->ClientList;Client;Client=Client->next)
			send_to_user(Client->user,"<%s> %s",from,args);
	}

	link_bcast(Link,"$MSG %s %s\n",from,args);
}

void Link_link(aBotLink *Link, char *args)
{
	aBotLink *bl,new;
	char	*type,*entity,*bot,*nick;
	int	lntype;

	if (!args || !*args)
		return;

	/*
	 *  Pass it on to everyone else
	 */
	link_bcast(Link,"$LINK %s\n",args);

	type = chop(&args);
	entity = chop(&args);
	bot = chop(&args);
	nick = chop(&args);

	if (!type || !*type || !entity || !*entity)
		return;

	if (!Strcasecmp(type,"ENTITY"))
		lntype = LN_ENTITY;
	else
	if (!Strcasecmp(type,"BOT"))
		lntype = LN_BOT;
	else
	if (!Strcasecmp(type,"USER"))
		lntype = LN_DCCUSER;
	else
		return;

	if (strlen(entity) >= ENTITYLEN)
		return;

	if ((lntype == LN_BOT) || (lntype == LN_DCCUSER))
	{
		if (!bot || !*bot || (strlen(bot) >= ENTITYLEN))
			return;
		if (lntype == LN_USER)
		{
			if (!nick || !*nick || (strlen(nick) >= MAXNICKLEN))
				return;
		}
	}

#ifdef DEBUG
	debug("(Link_link) adding new link :: entity = %s, type = %s, bot = %s, nick = %s\n",
		entity,type,nullstr(bot),nullstr(nick));
#endif

	memset(&new,0,sizeof(aBotLink));
	new.used = TRUE;
	new.ident = botlinkident++;
	new.auth = TRUE;
	new.sock = -1;
	new.uptime = now;
	new.linktype = LN_REMOTE;
	new.nodetype = lntype;
	new.sendlink = Link->ident;
	if (lntype == LN_ENTITY)
	{
		strcpy(new.nick,entity);
		new.uplink = 0;
	}
	else
	if (lntype == LN_BOT)
	{
		strcpy(new.nick,bot);
		bl = find_entity(entity,LN_ENTITY);
		new.uplink = bl->ident;
	}
	else
	{
		strcpy(new.nick,bot);
		strcpy(new.dccnick,nick);
		bl = find_entity(entity,LN_ENTITY);
		new.uplink = bl->ident;
	}
	add_aBotLink(&new);
	last_autolink = now;
}

void Link_cmd(aBotLink *Link, char *args)
{
	aBotLink *bl,*bl2;
	aBot	*bot;
	char	*from,*to,*pt;
	char	buf[MAXLEN];
	int	i;

	from = chop(&args);
	to = chop(&args);

	if (!args || !*args)
		return;

        if (!Strcasecmp("energymech",gethost(from)))
        	return;

	/*
	 *  Check for commands that arent allowed to be executed thru CMD
	 */
	pt = args;
	while(*pt && *pt != ' ')
		pt++;
	strncpy(buf,args,(pt - args));
	buf[(pt - args)] = 0;
#ifdef DEBUG
	debug("(do_linkcmd) Checking command '%s' (%i) ...\n",buf,strlen(buf));
#endif /* DEBUG */
	for(i=0;(mcmd[i].name);i++)
	{
		if (!Strcasecmp(mcmd[i].name,buf) || (mcmd[i].alias &&
		    !Strcasecmp(mcmd[i].alias,buf)))
		{
			if ((mcmd[i].flags & NOCMD) == NOCMD)
				return;
			break;
		}
	}

	CurrentShit = NULL;
	strcpy(CurrentNick,getnick(from));

	for(bot=botlist;bot;bot=bot->next)
	{
		current = bot;
		if ((*to == '*') || (is_me(to)))
		{
			/*
			 *  Different userlist for each bot
			 */
			CurrentUser = find_user(from,NULL);

			/*
			 *  Needs to re-sprintf this each time cuz on_msg()
			 *  shreds the string with get_token().
			 */
 			sprintf(buf,"%c%s",current->cmdchar,args);
			on_msg(from,current->nick,buf);
			if (*to != '*')
				return;		/* Im the only one */
		}
	}

	/*
	 *  Pass it on to everyone else...
	 */
	if (*to == '*')
	{
		link_bcast(Link,"$CMD %s %s %s\n",from,to,args);
		return;
	}

	/** or find the specified bot and send it on towards it... **/
	if ((bl = find_entity(to,LN_BOT)) == NULL)
		return;
	if ((bl2 = find_aBotLink(bl->sendlink)) == NULL)
		return;
	send_to_link(bl2,"$CMD %s %s %s\n",from,to,args);
}

void Link_reply(aBotLink *Link, char *args)
{
	aBotLink *bl,*bl2;
	aDCC	*Client;
	aBot	*bot;
	char	tmp[256],tmp2[MAXLEN];
	char	*from,*to,*botnick,*user;
	int	m;

	from = chop(&args);
	to = chop(&args);

	if (!to || (*to != '$'))
		return;

	/*
	 *  This is a bit of reverse psychology stuff (muahaha).
	 *  First *user is pointed at "bot@user" with the strcpy,
	 *  next the get_token returns "bot" to *botnick pointer
	 *  and *user is moved on to the "user" string.
	 */
	user = strcpy(tmp,&to[1]);
	botnick = get_token(&user,"@");

#ifdef DEBUG
	debug("(Link_reply) from,to,rest: %s %s %s\n",from,to,args);
#endif /* DEBUG */

	m = FALSE;
	for(bot=botlist;bot;bot=bot->next)
	{
		if (!Strcasecmp(botnick,bot->nick))
		{
			m = TRUE;
			current = bot;
			for(Client=current->ClientList;Client;Client=Client->next)
			{
				if ((Client->flags & DCC_CHAT) && (Client->flags & DCC_ACTIVE))
				{
					if (!Strcasecmp(user,Client->user))
					{
						unescape_string(args);
						sprintf(tmp2,"[%s] %s",from,args);
						send_chat(user,tmp2);
						return;
					}
				}
			}
		}
	}

	if (m)
		return;
	if ((bl = find_entity(botnick,LN_BOT)) == NULL)
		return;
	if ((bl2 = find_aBotLink(bl->sendlink)) == NULL)
		return;
	send_to_link(bl2,"$RPL %s %s %s\n",from,to,args);
}

void Link_needop(aBotLink *Link, char *args)
{
	aChanUser *CU;
	aChan	*Chan;
	aUser	*User;
	aBot	*bot;
	char	*chan,*nick;
	
	chan = chop(&args);
	nick = chop(&args);

	backup = current;
	for(bot=botlist;bot;bot=bot->next)
	{
		current = bot;
		if ((Chan = find_channel(chan,CH_ACTIVE)) == NULL)
			continue;
		if (!Chan->bot_is_op)
			continue;
		if ((CU = find_chanuser(Chan,nick)) == NULL)
			continue;
		User = find_user(get_nuh(CU),chan);
		if (User && (User->access == BOTLEVEL) && ((CU->flags & CU_CHANOP) == 0))
		{
			sendmode(chan,"+o %s",nick);
			return;
		}
	}
	link_bcast(Link,"$NEEDOP %s %s\n",chan,nick);
	current = backup;
}

void Link_needinvite(aBotLink *Link, char *args)
{
	aBot	*bot;
	char	*channel,*nuh;

	channel = chop(&args);
	nuh = chop(&args);

	if (!nuh || !*nuh)
		return;

	backup = current;
	for(bot=botlist;bot;bot=bot->next)
	{
		current = bot;
		if (is_opped(current->nick,channel))
		{
			if ((get_userlevel(nuh,channel)) >= MINUSERLEVEL)
			{
				to_server("INVITE %s %s",getnick(nuh),channel);
				current = backup;
				return;
			}
		}
	}
	current = backup;

	link_bcast(Link,"$NI %s %s\n",channel,nuh);
}

void Link_needkey(aBotLink *Link, char *args)
{
	aBotLink *bl,*bl2;
	aBot	*bot;
	aChan	*Chan;
	char	*channel,*nuh,*key;

	channel = chop(&args);
	nuh = chop(&args);
	key = chop(&args);

	if (!nuh || !*nuh)
		return;

	backup = current;

	for(bot=botlist;bot;bot=bot->next)
	{
		current = bot;

		if (key && *key)
		{
			if ((Chan = find_channel(channel,CH_ANY)) == NULL)
				continue;

			if (Strcasecmp(getnick(nuh), bot->nick))
				continue;

			MyFree(&Chan->key);
			set_mallocdoer(Link_needkey);
			mstrcpy(&Chan->key, key);
			if (Chan->retry) {
				to_server("JOIN %s %s\n", channel, key);
				Chan->retry = FALSE;
			}
			current = backup;
			return;
		}

		if ((Chan = find_channel(channel,CH_ACTIVE)) == NULL)
			continue;

		if (!(Chan->bot_is_op && Chan->key && *Chan->key))
			continue;
		
		if ((get_userlevel(nuh,channel)) >= MINUSERLEVEL)
		{
			if (((bl = find_entity(getnick(nuh),LN_BOT)) != NULL) &&
			    ((bl2 = find_aBotLink(bl->sendlink)) != NULL))
				send_to_link(bl2, "$K %s %s %s\n", channel,
							nuh, Chan->key);
			current = backup;
			return;
		}
	}
	current = backup;
	link_bcast(Link,"$K %s %s\n",channel,nuh);
}

void Link_nick(aBotLink *Link, char *args)
{
	aBotLink *bl;
	char	*ent;
	char	*old,*new;
	int	uplink,i;

	ent = chop(&args);
	old = chop(&args);
	new = chop(&args);

	if (!new || !*new)
		return;

	if ((bl = find_entity(ent,LN_ENTITY)) == NULL)
		return;
	uplink = bl->ident;

	bl = botlinkBASE;
	for(i=0;i<botlinkSIZE;i++)
	{
		if (bl->used)
		{
			if ((bl != Link) && (bl->linktype == LN_UPLINK))
				send_to_link(bl,"$NICK %s %s %s\n",ent,old,new);
			if ((bl->nodetype == LN_BOT) && (bl->uplink == uplink) && 
			    (!Strcasecmp(bl->nick,old)))
			{
				strcpy(bl->nick,new);
			}
		}
		bl++;
	}
}

void Link_userverify(aBotLink *Link, char *args)
{
#ifdef MULTI
	aTime	*tmp;
	aBot	*bot;
	int	pwn,v;
#endif /* MULTI */
	char	*from,*pass;

	from = chop(&args);
	pass = chop(&args);

	if (!from || !*from || !pass || !*pass)
		return;

#ifdef MULTI
	backup = current;
	for(bot=botlist;bot;bot=bot->next)
	{
		current = bot;
		if ((Link == NULL) && (backup == current))
			continue;
		pwn = password_needed(from);
		v = verified(from);
		if (pwn && v)
			continue;
		if (!pwn)
			continue;
		if (!pass || !*pass)
			continue;
		if (correct_password(from,pass))
		{
			if ((tmp = make_time(&current->Auths,from)) != NULL)
				tmp->time = now;
		}
	}
	current = backup;
#endif /* MULTI */

	link_bcast(Link,"$VRFY %s %s\n",from,pass);
}

typedef struct LinkCmdStruct
{
	char	*name;
	void	(*function)(aBotLink *Link, char *args);

} aLinkCmd;

/*
**  EnergyMech linking protocol
**

 $LOGIN
    <entity>        == name of connecting entity
    <password>      == password, "AUTH" if the entity is the one accepting the connection

 $QUIT | $Q
    <entity>        == entity name to delete
    [bot]           == bot nick to delete, child of <entity>

 $MSG | $M
    <from>          == sender of message, "$" for global_statmsg
    <message ...>   == message

 $LINK | $L
    <type>          == ENTITY | BOT | USER
    <entity>        == name of entity
    [nick]          == name of bot if <type> == BOT | USER
    [user]          == name of user if <type> == USER

 $CMD | $C
    <from>          == sender of command
    <to>            == target bot, '*' for all
    <cmd>           == command to execute

 $RPL | $R
    <from>          == sender of command reply
    <to>            == target, format == $<bot>@<nick!user@host>
    <message ...>   == reply

 $NEEDOP | $NO
    <channel>       == channel where +o is wanted
    <nick>          == nick to +o

 $NI
    <channel>       == channel where INVITE is wanted
    <nick>          == nick to INVITE

 $NICK | $N
    <entity>        == entity name
    <oldnick>       == old bot nick
    <newnick>       == new bot nick

 $VRFY | $V
    <from>          == whom to verify
    <password>      == password

 $K
    <channel>	    == Channel where key is needed
    <nick>	    == Nick of bot that needs key
    [key]	    == Key of channel(reply only)

*/

aLinkCmd link_cmds[] =
{
	/* command	function		*/
	{ "$LOGIN",	Link_addbot,		},
	/* old long name commands */
	{ "$QUIT",	Link_quit,		},
	{ "$MSG",	Link_msg,		},
	{ "$LINK",	Link_link,		},
	{ "$CMD",	Link_cmd,		},
	{ "$RPL",	Link_reply,		},
	{ "$NEEDOP",	Link_needop,		},
	{ "$NICK",	Link_nick,		},
	{ "$VRFY",	Link_userverify,	},
	/* next generation short link commands */
	{ "$C",		Link_cmd,		},
	{ "$L",		Link_link,		},
	{ "$M",		Link_msg,		},
	{ "$N",		Link_nick,		},
	{ "$NO",	Link_needop,		},
	{ "$NI",	Link_needinvite,	},
	{ "$Q",		Link_quit,		},
	{ "$R",		Link_reply,		},
	{ "$V",		Link_userverify,	},
	{ "$K",		Link_needkey,		},
	{ NULL,		NULL			}
};

char *LN(int ln);

#ifdef PIPEUSER

#define	pusock	current->pipeuser->sock

void pipeuser_refresh(char *nick)
{
	aServer	*sp;
	aChan	*Chan;

	sp = find_aServer(current->server);
	if (!sp)
		return;

	current->pipeuser->has_nick = TRUE;

	send_to_socket(pusock,":%s 001 %s :Welcome to the Internet Relay Network %s\n",sp->realname,nick,nick);
	send_to_socket(pusock,":%s 002 %s %s\n",sp->realname,nick,current->saved_002);
	send_to_socket(pusock,":%s 003 %s %s\n",sp->realname,nick,current->saved_003);
	send_to_socket(pusock,":%s 004 %s %s\n",sp->realname,nick,current->saved_004);
	send_to_socket(pusock,":%s!%s NICK :%s\n",nick,current->userhost,current->nick);
	for(Chan=current->Channels;Chan;Chan=Chan->next)
	{
		if ((Chan->active & CH_ACTIVE) == CH_ACTIVE)
		{
			/*
			 *  When you join a channel it normally sends NAMES output to the
			 *  client, instead of parsing the chanuser list, we're acting
			 *  lazy and send a NAMES request to the server ;)
			 */
			to_server("NAMES %s\n",Chan->name);
			send_to_socket(pusock,":%s!%s JOIN %s\n",current->nick,
				current->userhost,Chan->name);
		}
	}
}

#endif /* PIPEUSER */

void process_link_input(aBotLink *Link, char *buf)
{
#ifdef PIPEUSER
	aPipeUser *PU;
	char	*nick;
#ifndef TELNET
	aUser	*User;
	char	tmp[MSGLEN];
#ifdef MULTI
	aBot	*bot;
#endif /* MULTI */
#endif /* !TELNET */
#endif /* PIPEUSER */
#ifdef TELNET
	aDCC	*Client;
	aUser	*User;
#ifdef MULTI
	aBot	*bot;
#endif /* MULTI */
	char	nuh[NUHLEN];
	char	tmp[MSGLEN];
#endif /* TELNET */
	char	*cmd,*rest;
	int	i;

	cmd = chop(&buf);
	rest = buf;

	/*
	 *  $LOGIN is a special case
	 */
	if (!Strcasecmp(cmd,link_cmds[0].name))
	{
		link_cmds[0].function(Link,rest);
		return;
	}
	if (Link->auth)
	{
		for(i=1;link_cmds[i].name;i++)
		{
			if (!Strcasecmp(cmd,link_cmds[i].name))
			{
				link_cmds[i].function(Link,rest);
				return;
			}
		}
	}
#ifdef PIPEUSER
	if ((Link->nodetype == LN_SAYNICK_SEND) || (Link->nodetype == LN_PIPE_HASNICK))
	{
		if (!Strcasecmp(cmd,"USER"))
			return;
		if (!Strcasecmp(cmd,"NICK"))
		{
			if (PUnick)
				goto pipe_cont;
			Link->nodetype = LN_PIPE_HASNICK;
			set_mallocdoer(process_link_input);
			mstrcpy(&PUnick,rest);
#ifdef DEBUG
			debug("[PLI] {%i} Saving nick for pipeuser; %s\n",Link->sock,PUnick);
#endif /* DEBUG */
			return;
		}
		if (!Strcasecmp(cmd,"PASS"))
		{
			if (!rest || !*rest)
				goto pipe_cont;
			if (*rest == ':')
				rest++;
			nick = get_token(&rest,",");
#ifdef MULTI
			current = NULL;
			for(bot=botlist;bot;bot=bot->next)
			{
				if ((!Strcasecmp(nick,bot->nick)) ||
				    (!Strcasecmp(nick,bot->wantnick)))
				{
					current = bot;
					break;
				}
			}
			if (!current)
			{
				current = botlist;
#ifdef DEBUG
				debug("[PLI] {%i} No matching botnick found for pipeuser request\n");
#endif /* DEBUG */
				goto pipe_cont;
			}
#else /* MULTI */
			if (Strcasecmp(nick,current->nick))
				goto pipe_cont;
#endif /* MULTI */
			nick = get_token(&rest,",");
			if ((User = find_handle(nick)) == NULL)
#ifdef DEBUG
			{
				debug("[PLI] {%i} No such user: %s\n",Link->sock,nick);
				goto pipe_cont;
			}
#else /* DEBUG */
				goto pipe_cont;
#endif /* DEBUG */
			if (Strcasecmp(User->chanp,"*"))
#ifdef DEBUG
			{
				debug("[PLI] {%i} User %s doesn't have global access\n",Link->sock,nick);
				goto pipe_cont;
			}
#else /* DEBUG */
				goto pipe_cont;
#endif /* DEBUG */
			if ((User->access < OWNERLEVEL) || (!passmatch(rest,User->pass)))
#ifdef DEBUG
			{
				debug("[PLI] {%i} Access below 100 (%i) or password mismatch\n",
					Link->sock,User->access);
				goto pipe_cont;
			}
#else /* DEBUG */
				goto pipe_cont;
#endif /* DEBUG */
			if (current->pipeuser)
			{
				sprintf(tmp,"NOTICE %s :(%s) Pipe is already in use by %s\n",
					User->name,current->nick,current->pipeuser->name);
				write(Link->sock,tmp,strlen(tmp));
				goto pipe_cont;
			}
			set_mallocdoer(process_link_input);
			PU = (aPipeUser*)MyMalloc(sizeof(aPipeUser));
			PU->sock = Link->sock;
			PU->lasttime = now;
			strcpy(PU->name,nick);
			current->pipeuser = PU;
			memset(Link,0,sizeof(Link));
#ifdef DEBUG
			debug("[PLI] {%i} Link converted to pipeuser: %s\n",PU->sock,PU->name);
#endif /* DEBUG */
			return;
		}
pipe_cont:
		if (PUnick)
			MyFree(&PUnick);
		close(Link->sock);
		memset(Link,0,sizeof(Link));
		return;
	}
#endif /* PIPEUSER */
#ifdef TELNET
	if ((Link->linktype == LN_CONNECTED) || (Link->linktype == LN_UPLINK))
		return;
	if (Link->nodetype == LN_UNKNOWN)
	{
#ifdef MULTI
		for(bot=botlist;bot;bot=bot->next)
		{
			current = bot;
#endif /* MULTI */
			if ((User = find_handle(cmd)))
			{
				strcpy(Link->nick,User->name);
				send_to_link(Link,"Please enter your password.\n");
				Link->nodetype = LN_PASSWAIT;
				Link->bot = current;
#ifdef DEBUG
				debug("[PLI] Telnet login for %s (%s[%i])\n",cmd,User->name,User->access);
#endif
				return;
			}
		}

		send_to_link(Link,"Please enter your password.\n");
		Link->nodetype = LN_PASSWAIT;
#ifdef DEBUG
		debug("[PLI] Telnet login for %s (<NO MATCH>)\n",cmd);
#endif /* DEBUG */
		return;
	}
	if (Link->nodetype == LN_PASSWAIT)
	{
#ifdef DEBUG
		debug("[PLI] Passwait: Link->nick = %s, Link->uplink = %i\n",Link->nick,Link->uplink);
#endif /* DEBUG */
#ifdef MULTI
		for(bot=botlist;bot;bot=bot->next)
		{
			if (bot == Link->bot)
				break;
		}
		if ((Link->nick[0]) && (current = bot))
#else /* MULTI */
		if (Link->nick[0])
#endif /* MULTI */
		{
			User = find_handle(Link->nick);
			if ((User) && (User->access < MINUSERLEVEL))
				User = NULL;
			if ((User) && (User->access == BOTLEVEL))
				User = NULL;
			if ((User) && ((User->pass[0]) && (!passmatch(cmd,User->pass))))
				User = NULL;
			if (User)
			{
				sprintf(nuh,"%s!telnet@energymech",User->name);
#ifdef DEBUG
				debug("[PLI] Making telnet %s[%i] -> %s\n",User->name,User->access,nuh);
#endif /* DEBUG */
				set_mallocdoer(process_link_input);
				Client = (aDCC*)MyMalloc(sizeof(aDCC));
				strcpy(Client->user,nuh);

				Client->sock = Link->sock;
				Client->flags = DCC_CHAT | DCC_TELNET | DCC_ACTIVE;
				Client->lasttime = now;
				add_client(Client);

				sprintf(tmp,"[%s] %s[%i] has connected",
					current->nick,User->name,User->access);
				if ((send_to_socket(Client->sock,"[%s] %s\n",time2medium(now),tmp)) < 0)   
				{
					Client->flags = DCC_DELETE;
				}
				else
				{
					send_global_statmsg(tmp);
					if (User->access == OWNERLEVEL)
						do_statmsg(Client->user,current->nick,NULL,OWNERLEVEL);
#ifdef DEBUG
					debug("[PLI] Conversion done\n");
#endif /* DEBUG */
					memset(Link,0,sizeof(Link));
					return;
				}
			}
		}
		close(Link->sock);
		memset(Link,0,sizeof(Link));
	}
#endif /* TELNET */
}

#ifdef PIPEUSER

/*
 *  This is where we read & parse pipeuser input
 */

void pu_user(char *rest)
{
	/* DO NOTHING */
}

void pu_nick(char *rest)
{
	char	*nick;

	nick = chop(&rest);
	if (!nick || !*nick)
		return;
	if (*nick == ':')
		nick++;
	if (!current->pipeuser->has_nick)
	{
		pipeuser_refresh(nick);
		return;
	}
	if (!isnick(nick))
	{
		send_to_socket(pusock,":%s NOTICE %s :%s: Illegal nick\n",
			(char*)PUMASK,current->nick,nick);
		return;
	}
	strcpy(current->wantnick,nick);
	to_server("NICK %s\n",nick);
}

void pu_quit(char *rest)
{
	aChan	*Chan;

	/*
	 *  Parting all channels that the pipeuser joined
	 */
	for(Chan=current->Channels;Chan;Chan=Chan->next)
	{
		if (Chan->pipeuser)
		{
			Chan->rejoin = FALSE;
			Chan->active = CH_OLD;
			to_server("PART %s",Chan->name);
		}
	}
	if (current->CurrentChan->active != CH_ACTIVE)
	{
		for(Chan=current->Channels;Chan;Chan=Chan->next)
		{
			if (Chan->active == CH_ACTIVE)
				break;
		}
		current->CurrentChan = Chan;
	}
	close(pusock);
	MyFree((char**)&current->pipeuser);
	return;
}

void pu_join(char *rest)
{
	aChan	*Chan;
	char	*channel;

	channel = get_token(&rest,", ");
	if (!channel || *channel != '#')
		return;
	if (find_channel(channel,CH_ACTIVE))
		return;
	join_channel(channel,rest,TRUE);
	Chan = find_channel(channel,CH_ANY);
	Chan->pipeuser = TRUE;
}

void pu_part(char *rest)
{
	aChan	*Chan;
	char	*channel;

	channel = get_token(&rest,", ");
	if (!channel || *channel != '#')
		return;
	if ((Chan = find_channel(channel,CH_ACTIVE)))
	{
		Chan->rejoin = FALSE;
		Chan->active = CH_OLD;
		to_server("PART %s",Chan->name);
		if (Chan == current->CurrentChan)
		{
			for(Chan=current->Channels;Chan;Chan=Chan->next)
			{
				if (Chan->active == CH_ACTIVE)
					break;
			}
			current->CurrentChan = Chan;
		}
	}
}

void pu_privmsg(char *rest)
{
	char	*dest;

	dest = chop(&rest);
	if (!dest || !*dest)
		return;
	if (!Strcasecmp(dest,"0"))
	{
		/*
		**  on_msg(current->pipeuser->name,current->nick,rest);
		*/
		return;
	}
	to_server("PRIVMSG %s %s\n",dest,rest);
}

struct
{
	char	*command;
	void	(*func)(char *);
	
} pucmd[] =
{
{ "USER",	pu_user		},
{ "NICK",	pu_nick		},
{ "QUIT",	pu_quit		},
{ "JOIN",	pu_join		},
{ "PART",	pu_part		},
{ "PRIVMSG",	pu_privmsg	},
{ NULL,		NULL		},
};

void parse_pipe_input(void)
{
	char	linebuf[MSGLEN];
	char	*res,*cmd;
	int	i;

	if ((!current->pipeuser->has_nick) && (PUnick))
	{
		pipeuser_refresh(PUnick);
		MyFree(&PUnick);
	}
	res = sockread(current->pipeuser->sock,current->pipeuser->sd,linebuf);
	if (res)
	{
		cmd = chop(&res);
		for(i=0;pucmd[i].command;i++)
		{
			if (!Strcasecmp(cmd,pucmd[i].command))
			{
				pucmd[i].func(res);
				return;
			}
		}
		to_server("%s %s\n",cmd,res);
		return;
	}
	switch(errno)
	{
	case EAGAIN:
	case EINTR:
		return;
	default:
#ifdef DEBUG
		debug("[PPI] Closing connection to pipeuser\n");
#endif /* DEBUG */
		MyFree((char**)&current->pipeuser);
		return;
	}
}
#endif /* PIPEUSER */

void parse_link_input(void)
{
	aLinkConf *lc;
	aBotLink *bl,new;
	union sockaddr_union so;
	char	linebuf[MSGLEN];
	char	*res;
	int	newS,blen,i;

	if ((linksock != -1) && (FD_ISSET(linksock,&read_fds)))
	{
		last_autolink = now;
#ifdef LINKEVENTS
		wle("Socket accept ( linksock:%i )\n",linksock);
#endif /* LINKEVENTS */
		blen = sizeof(union sockaddr_union);
		newS = accept(linksock,&so.sa,&blen);
		if (newS == -1)
		{
			if (errno != EWOULDBLOCK)
			{
#ifdef LINKEVENTS
				wle("accept() error; %s (%i)\n",strerror(errno),errno);
#endif /* LINKEVENTS */
				close(linksock);
				linksock = -1;
			}
		}
		else
			newlinksock(newS);
	}

	bl = botlinkBASE;
	for(i=0;i<botlinkSIZE;i++)
	{
		if (bl->used)
		{
			if ((bl->sock != -1) && (FD_ISSET(bl->sock,&write_fds)))
			{
#ifdef DEBUG
				debug("[PLI] Asynch connect\n");
#endif /* DEBUG */
				bl->linktype = LN_CONNECTED;
				send_to_link(bl,"$LOGIN %s %s\n",my_entity,my_pass);
			}
			else
			if ((bl->sock != -1) &&
			    ((FD_ISSET(bl->sock,&read_fds)) || (bl->hasdata)))
			{
				/*
				 *  Commands might changed the botlink-list,
				 *  so returns here are necessary
				 */
				res = sockread(bl->sock,bl->sd,linebuf);
				if (res)
				{
#ifdef LINKEVENTS
					wle("Net Read    %-10s ( "mx_pfmt" -- S:%i I:%i U:%i L:%i ): %s\n",
						(bl->nick[0]) ? bl->nick : NULLSTR,(mx_ptr)bl,bl->sock,
						bl->ident,bl->uplink,bl->sendlink,res);
#endif /* LINKEVENTS */
					process_link_input(bl,res);
					bl->hasdata = TRUE;
					return;
				}
				switch(errno)
				{
				case EAGAIN:
				case EINTR:
					bl->hasdata = FALSE;
					return;
				default:
					remlink(bl,NULL);
					return;
				}
			}
			else
			if ((bl->linktype == LN_TRYING) &&
			    ((now - bl->uptime) >= WAITTIMEOUT))
			{
				if (!bl->autolinked)
					send_global_statmsg("Link to \"%s\" failed",bl->nick);
				close(bl->sock);
#ifdef DEBUG
				debug("[PLI] Timeout for '%s'; removing...\n",bl->nick);
#endif /* DEBUG */
#ifdef LINKEVENTS
				wle("Timeout for '%s' (0x"mx_pfmt"); removing... ( S:%i )\n",
					nullstr(bl->nick),(mx_ptr)bl,bl->sock);
#endif /* LINKEVENTS */
				memset(bl,0,sizeof(aBotLink));
			}
		}
		bl++;
	}

	if (!linkconfBASE)
		return;
	if (!will.autolink)
		goto no_autolink;
	if (now - last_autolink < AUTOLINK_DELAY)
		goto no_autolink;
	last_autolink = now;

#ifdef DEBUG
	debug("[PLI] Trying to autolink...\n");
#endif /* DEBUG */

auto_retry:
	lc = linkconfBASE;
	for(i=0;i<linkconfSIZE;i++)
	{
		if ((lc->used) && (lc->port) && (lc->autoidx == LC_NOTRY))
			goto auto_try;
		lc++;
	}
	lc = linkconfBASE;
	for(i=0;i<linkconfSIZE;i++)
	{
		if (lc->used)
		{
			if ((lc->autoidx == LC_HASTRY) || (lc->autoidx == LC_LINKED))
			{
				lc->autoidx = LC_NOTRY;
#ifdef LINKEVENTS
				wle("LC %s ( H:%s P:%i ) reset to LC_NOTRY\n",
					lc->name,lc->host,lc->port);
#endif /* LINKEVENTS */
			}
		}
		lc++;
	}
	goto no_autolink;

auto_try:
	bl = botlinkBASE;
	for(i=0;i<botlinkSIZE;i++)
	{
		if ((bl->used) && (bl->nodetype == LN_ENTITY) && (!Strcasecmp(lc->name,bl->nick)))
		{
			lc->autoidx = LC_LINKED;
#ifdef LINKEVENTS
			wle("LC %s ( H:%s P:%i ) set to LC_LINKED\n",
				lc->name,lc->host,lc->port);
#endif /* LINKEVENTS */
			goto auto_retry;
		}
		bl++;
	}

	/* got a live one here... */
#ifdef LINKEVENTS
	wle("LC %s ( H:%s P:%i ) set to LC_HASTRY\n",lc->name,lc->host,lc->port);
	wle("Autolink attempt to %s ( H:%s P:%i )\n",lc->name,lc->host,lc->port);
#endif /* LINKEVENTS */
	lc->autoidx = LC_HASTRY;

	memset(&new,0,sizeof(aBotLink));
	new.used = TRUE;
	new.ident = botlinkident++;
	new.auth = TRUE;
	new.autolinked = TRUE;
	strcpy(new.nick,lc->name);
	new.uptime = now;
	new.linktype = LN_TRYING;
	new.sock = SockConnect(lc->host,lc->port,FALSE);
	add_aBotLink(&new);

no_autolink:	/* end of autolinking */

	return;
}

#endif /* LINKING */
