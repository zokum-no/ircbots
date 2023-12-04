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

void parse_error(char *from, char *rest)
{
	aServer	*sp;

	if (rest && *rest)
	{
		sp = find_aServer(current.server);
		if (sp)
		{
			if (!matches("*no authorization*",rest))
				sp->err = SP_NOAUTH;
			else
			if (!matches("*k*lined*",rest))
				sp->err = SP_KLINED;
			else
			if (!matches("*connection*class*",rest))
				sp->err = SP_FULLCLASS;
			else
			if (!matches("*different port*",rest))
				sp->err = SP_DIFFPORT;
			else
				sp->err = SP_NULL;
		}
	}
	close(current.sock);
	current.sock = -1;
}

void parse_invite(char *from, char *rest)
{
	char	*to;
	char	*chan;
	int	i;

	to = chop(&rest);
	chan = chop(&rest);
	if (chan && (*chan == ':'))
		chan++;
	i = get_userlevel(from,chan);
	if ((i >= JOINLEVEL) && (i < BOTLEVEL))
	{
		join_channel(chan,NULL,1);
		current.lastrejoin = now;
	}
}

void parse_join(char *from, char *rest)
{
	static	char copy[NAMELEN];
	char	*n,*u,*h;

	h = strcpy(copy,from);
	n = get_token(&h,"!");
	u = get_token(&h,"@");

	CurrentChan = find_channel(rest,CH_ANY);

	if (!Strcasecmp(current.nick,n))
	{
		if (!CurrentChan)
		{
			send_to_server("PART %s",rest);
			return;
		}
		CurrentChan->active = CH_ACTIVE;
		CurrentChan->bot_is_op = FALSE;
		delete_chanusers(CurrentChan);
		delete_bans(&CurrentChan->banned);
		CurrentChan->private = CurrentChan->secret = CurrentChan->moderated =
		CurrentChan->topprot = CurrentChan->limitmode = CurrentChan->invite =
		CurrentChan->nomsg = CurrentChan->keymode = 0;
		current.CurrentChan = CurrentChan;
		CurrentChan->rejoin = FALSE;
		CurrentChan->whois = FALSE;
		send_to_server("WHO %s\nMODE %s\nMODE %s b",rest,rest,rest);
#ifdef LINKING
		needop(rest);
#endif /* LINKING */
	}
	if (CurrentChan->active == CH_ACTIVE)
	{
		if (CurrentChan->whois)
			make_chanuser(n,u,h);
		CurrentUser = find_user(from,rest);
		CurrentShit = find_shit(from,rest);
		strcpy(CurrentNick,getnick(from));
		on_join(from);
	}
}

void parse_mode(char *from, char *rest)
{
	char	*to;

	if ((to = chop(&rest)) == NULL)
		return;

	if (ischannel(to))
	{
		CurrentUser = find_user(from,to);
		CurrentShit = find_shit(from,to);
		strcpy(CurrentNick,getnick(from));
		on_mode(from,to,rest);
	}
	else
	if (!Strcasecmp(current.nick,to))
	{
		char	tbuf[32],tbuf2[32],tbuf3[32];
		char	*themodes,*aptr,*aptr2,sign;
		int	modelen,i;

		sign = '+';

		if (current.modes)
			strcpy(tbuf,current.modes);
		else
			strcpy(tbuf,"");
		themodes = chop(&rest);
		if (*themodes == ':')
			themodes++;
		modelen = strlen(themodes);
		for(i=0;i<modelen;i++)
		{
			switch(themodes[i])
			{
			case '+':
			case '-':
				sign = themodes[i];
				break;
			default:
				if (sign == '+')
				{
					sprintf(tbuf,"%s%c",tbuf,themodes[i]);
					break;
				}
				else if (sign == '-')
				{
					sprintf(tbuf3,"%c",themodes[i]);
					aptr = strcpy(tbuf2,tbuf);
					aptr2 = get_token(&aptr,tbuf3);
					sprintf(tbuf,"%s%s",aptr2,aptr);
					break;
				}
			}
		}
		strcpy(current.modes,tbuf);
	}
}

void parse_notice(char *from, char *rest)
{
	aChan	*Chan;
	aTime	*auth;
	char	*nick,*to;
	char	*p;
	int	*ip,ping;

	if (!strchr(from,'@'))
		return;

	to = chop(&rest);
	if (*rest == ':')
		rest++;

	nick = getnick(from);

	if ((auth = find_time(&current.Auths,from)) != NULL)
		auth->time = now;

	if (*rest == 1)
	{
		send_statmsg("[CTCP Reply From %s] %s",nick,rest);
		p = chop(&rest);
		if (!Strcasecmp(p,"PING"))
		{
			p = get_token(&rest," \001");
			ping = (ip = Atoi(p)) ? *ip : now;
			send_statmsg("[LAG] %li second(s) lag from %s",(now - ping),nick);
		}
		return;
	}

	if (ischannel(to))
	{
		if ((Chan = find_channel(to,CH_ACTIVE)))
		{
			send_spy(Chan,"-%s:%s- %s",nick,to,rest);
		}
		return;
	}
	send_spymsg("[message] -%s- %s",nick,rest);
}

void parse_part(char *from, char *rest)
{
	aChan	*Chan;
	char	*channel;

	channel = chop(&rest);
	if ((Chan = find_channel(channel,CH_ANY)) == NULL)
		return;

	strcpy(CurrentNick,getnick(from));
	send_spy(Chan,"*** Parts: %s (%s)",CurrentNick,getuh(from));

	if (is_me(CurrentNick))
	{
		Chan->active = CH_OLD;
		Chan->bot_is_op = FALSE;
	}

#ifdef SEEN
	make_seen(CurrentNick,from,"none",now,1,channel);
#endif /* SEEN */

	remove_chanuser(Chan,CurrentNick);
	check_all_steal();
}

void parse_ping(char *from, char *rest)
{
	send_to_server("PONG :%s",rest);
}

void parse_privmsg(char *from, char *rest)
{
	char	*to,*channel;

	if (!strchr(from,'@'))
		return;
	to = chop(&rest);
	if (*rest == ':')
		rest++;
	channel = (ischannel(to)) ? to : NULL;
	CurrentUser = find_user(from,channel);
	CurrentShit = find_shit(from,channel);
	strcpy(CurrentNick,getnick(from));
	if (*rest == 1)
	{
		rest++;
		terminate(rest,"\001");
		on_ctcp(from,to,rest);
		return;
	}
	on_msg(from,to,rest);
}

void parse_quit(char *from, char *rest)
{
	aChan	*Chan;

	strcpy(CurrentNick,getnick(from));

#ifdef SEEN
	make_seen(CurrentNick,from,"none",now,2,rest);
#endif /* SEEN */

	send_common_spy(from,"*** Quits: %s (%s)",CurrentNick,rest);

	for(Chan=current.Channels;Chan;Chan=Chan->next)
	{
		remove_chanuser(Chan,CurrentNick);
	}

	check_all_steal();
}

void parse_topic(char *from, char *rest)
{
	aChan	*Chan;
	char	*channel;

	channel = chop(&rest);
	if ((Chan = find_channel(channel,CH_ACTIVE)) == NULL)
		return;

	if (rest && *rest == ':')
		rest++;
	reverse_topic(from,channel,rest);
	send_spy(Chan,"*** %s changes topic to \"%s\"",getnick(from),rest);
}

void parse_wallops(char *from, char *rest)
{
	char	*ptr;

	ptr = strchr(from,'!');
	if (ptr && *ptr == '!')
		*ptr = 0;
	send_statmsg("!%s! %s",from,rest);
}

/*
 *
 *  Numerics
 *
 */

void parse_EOMOTD(char *from, char *rest)
{
	aServer *sp;

	if ((sp = find_aServer(current.server)))
	{
		if (sp->realname[0] == 0)
		{
			strncpy(sp->realname,from,NAMELEN-1);
			sp->realname[NAMELEN-1] = 0;
		}
		sp->lastconnect = now;
	}
	current.connect = CN_ONLINE;
	current.ontime = now;
	send_to_server("WHOIS %s",current.nick,current.ontime);
	memset(current.modes,0,sizeof(current.modes));
	if (current.modes_to_send[0])
		send_to_server("MODE %s %s",current.nick,current.modes_to_send);
}

void parse_001(char *from, char *rest)
{
	aServer *sp;
	char	*newnick;

	if ((newnick = chop(&rest)) == NULL)
		return;
	if (Strcasecmp(current.nick,newnick))
	{
#ifdef DEBUG
		debug("(parse_001) Nick desynch: %s != %s\n",newnick,current.nick);
#endif
		setbotnick(newnick);
	}
	if ((sp = find_aServer(current.server)))
	{
		if (sp->realname[0] == 0)
		{
			strncpy(sp->realname,from,NAMELEN-1);
			sp->realname[NAMELEN-1] = 0;
		}
	}
}

#ifdef PIPEUSER

void parse_002(char *from, char *rest)
{
	MyFree((char**)&current.saved_002);
	chop(&rest);
	set_mallocdoer(parse_002);
	mstrcpy(&current.saved_002,rest);
}

void parse_003(char *from, char *rest)
{
	MyFree((char**)&current.saved_003);
	chop(&rest);
	set_mallocdoer(parse_003);
	mstrcpy(&current.saved_003,rest);
}

void parse_004(char *from, char *rest)
{
	MyFree((char**)&current.saved_004);
	chop(&rest);
	set_mallocdoer(parse_004);
	mstrcpy(&current.saved_004,rest);
}

#endif /* PIPEUSER */

void parse_213(char *from, char *rest)
{
	aTime	*aVarX;

	chop(&rest);

	if (*rest == ':')
		rest++;

	for (aVarX = current.ParseList; aVarX; aVarX = aVarX->next)
	{
		if (aVarX->num == PA_STATS)
		{
			send_to_user(aVarX->name, "[\002%s\002] %s", from, rest);
		}
	}
}

void parse_219(char *from, char *rest)
{
	aTime	*aVarX,*aVarX2;

	aVarX = current.ParseList;
	while (aVarX)
	{
		if (aVarX->num == PA_STATS)
		{
			aVarX2 = aVarX->next;
			remove_time(&current.ParseList, aVarX);
			aVarX = aVarX2;
		}
		else
		{
			aVarX = aVarX->next;
		}
	}
}

void parse_243(char *from, char *rest)
{
	aTime	*aVarX;

	chop(&rest);
	for(aVarX=current.ParseList;aVarX;aVarX=aVarX->next)
	{
		if (aVarX->num == PA_STATS)
			send_to_user(aVarX->name,"[%s] %s",from,rest);
	}
}

void parse_251(char *from, char *rest)
{
	char	*users, *invusers, *servers;
	long	c_users = 0,
		c_invusers = 0,
		tot_users = 0,
		c_servers = 0;
	int	temp = 1;
	aTime	*aVarX;
	
	users = chop(&rest);
	while(temp && users && *users)
	{
		if (isdigit((int)*users))
		{
			c_users = atol(users);
			temp = 0;
		}
		else
		{
			users = chop(&rest);
		}
	}
	
	temp = 1;
	invusers = chop(&rest);
	while (temp && invusers && *invusers)
	{
		if (isdigit((int)*invusers))
		{
			c_invusers = atol(invusers);
			tot_users = c_users + c_invusers;
			temp = 0;
		}
		else
		{
			invusers = chop(&rest);
		}
	}
	
	temp = 1;
	servers = chop(&rest);
	while (temp && servers && *servers)
	{
		if (isdigit((int)*servers))
		{
			c_servers = atol(servers);
			temp = 0;
		}
		else
		{
			servers = chop(&rest);
		}
	}

	for (aVarX = current.ParseList; aVarX; aVarX = aVarX->next)
	{
		if (aVarX->num == PA_LUSERS)
		{
			send_to_user(aVarX->name,"[\037Lusers status\037]");
			send_to_user(aVarX->name,"Users: \002%li\002 Invisible: \002%li\002 Servers: \002%li\002",
				tot_users,c_invusers,c_servers);
		}
	}
}

void parse_252(char *from, char *rest)
{
	aTime	*aVarX;
	char	*opers;
	long	c_opers = 0;
	int	temp = 1;

	opers = chop(&rest);
	while(temp && opers && *opers)
	{
		if (isdigit((int)*opers))
		{
			c_opers = atol(opers);
			temp = 0;
		}
		else
		{
			opers = chop(&rest);
		}
	}

	for (aVarX = current.ParseList; aVarX; aVarX = aVarX->next)
	{
		if (aVarX->num == PA_LUSERS)
		{
			send_to_user(aVarX->name,"IRC Operators \002%li\002",c_opers);
		}
	}
}

void parse_253(char *from, char *rest)
{
	aTime	*aVarX;
	char	*ucons;
	long	c_ucons = 0;
	int	temp = 1;

	ucons = chop(&rest);
	while (temp && ucons && *ucons)
	{
		if (isdigit((int)*ucons))
		{
			c_ucons = atol(ucons);
			temp = 0;
		}
		else
		{
			ucons = chop(&rest);
		}
	}

	for (aVarX = current.ParseList; aVarX; aVarX = aVarX->next)
	{
		if (aVarX->num == 15)
		{
			send_to_user(aVarX->name, "Unknown Connections: \002%li\002", from, c_ucons);
		}
	}
}

void parse_254(char *from, char *rest)
{
	char	*chans;
	long	c_chans = 0;
	int	temp = 1;
	aTime	*aVarX;

	chans = chop(&rest);
	while(temp && chans && *chans)
	{
		if (isdigit((int)*chans))
		{
			c_chans = atol(chans);
			temp = 0;
		}
		else
		{
			chans = chop(&rest);
		}
	}

	for(aVarX=current.ParseList;aVarX;aVarX=aVarX->next)
	{
		if (aVarX->num == PA_LUSERS)
			send_to_user(aVarX->name,"Channels: \002%li\002",c_chans);
	}
}

void parse_255(char *from, char *rest)
{
	aTime	*aVarX,*aVarX2;
	char	*clients,*servers;
	int	c_clients,c_servers;
	int	temp;

	c_clients = c_servers = 0;

	temp = 1;
	clients = chop(&rest);
	while(temp && clients && *clients)
	{
		if (isdigit((int)*clients))
		{
			c_clients = atol(clients);
			temp = 0;
		}
		else
		{
			clients = chop(&rest);
		}
	}

	temp = 1;
	servers = chop(&rest);
	
	while(temp && servers && *servers)
	{
		if (isdigit((int)*servers))
		{
			c_servers = atol(servers);
			temp = 0;
		}
		else
		{
			servers = chop(&rest);
		}
	}

	for(aVarX=current.ParseList;aVarX;aVarX=aVarX->next)
	{
		if (aVarX->num == PA_LUSERS)
		{
			send_to_user(aVarX->name,"Clients: \002%li\002 Servers: \002%li\002",c_clients,c_servers);
			send_to_user(aVarX->name,"[\037End of Lusers\037]");
		}
	}
	
	aVarX = current.ParseList;
	while(aVarX)
	{
		if (aVarX->num == PA_LUSERS)
		{
			aVarX2 = aVarX->next;
			remove_time(&current.ParseList,aVarX);
			aVarX = aVarX2;
		}
		else
		{
			aVarX = aVarX->next;
		}
	}
}

void parse_301(char *from, char *rest)
{
	aTime	*aVarX;

	chop(&rest);
	get_token(&rest,":");

	for(aVarX=current.ParseList;aVarX;aVarX=aVarX->next)
	{
		if (aVarX->num == PA_WHOIS)
		{
			send_to_user(aVarX->name, "Away            - %s",rest);
		}
	}
}

void parse_311(char *from, char *rest)
{
	aTime	*aVarX,*aVarX2;
	char	*nick,*user,*host;

	chop(&rest);
	nick = chop(&rest);
	user = chop(&rest);
	host = chop(&rest);
	chop(&rest);

	if (rest && *rest == ':')
		rest++;

	if (!Strcasecmp(nick,current.nick))
		sprintf(current.userhost,"%s@%s",user,host);

	aVarX = current.ParseList;
	while(aVarX)
	{
		if (aVarX->num == PA_WHOIS)
		{
			send_to_user(aVarX->name, "Whois Info for  - %s",nick);
			send_to_user(aVarX->name, "Address         - %s@%s",user,host);
			send_to_user(aVarX->name, "IRCName         - %s",rest);
		}
		if (aVarX->num == PA_USERHOST)
		{
			send_to_user(aVarX->name,"Userhost: %s=%s@%s",nick,user,host);
			aVarX2 = aVarX->next;
			remove_time(&current.ParseList,aVarX);
			aVarX = aVarX2;
		}
		else
		{
			aVarX = aVarX->next;
		}
	}
}

void parse_312(char *from, char *rest)
{
	aTime	*aVarX;
	char	*nick,*server;

	chop(&rest);
	nick = chop(&rest);
	server = chop(&rest);

	if (rest && *rest == ':')
		rest++;

	for(aVarX=current.ParseList;aVarX;aVarX=aVarX->next)
	{
		if (aVarX->num == PA_WHOIS)
		{
			send_to_user(aVarX->name, "Server          - %s",server);
			send_to_user(aVarX->name, "                  (%s)",rest);
		}
	}
}

void parse_313(char *from, char *rest)
{
	char	*nick, *operinfo;
	char	tbuf[MAXLEN];
	aTime	*aVarX;

	chop(&rest);
	nick = chop(&rest);
	operinfo = get_token(&rest,":");

	strcpy(tbuf,operinfo);
	while ((operinfo = chop(&rest)))
	{
		sprintf(tbuf,"%s %s",tbuf,operinfo);
	}

	for(aVarX=current.ParseList;aVarX;aVarX=aVarX->next)
	{
		if (aVarX->num == PA_WHOIS)
		{
			send_to_user(aVarX->name, "IRCop           - %s %s",nick,tbuf);
		}
	}
}

void parse_315(char *from, char *rest)
{
	aChan	*Chan;
	char	*channel;

	chop(&rest);
	channel = chop(&rest);

	if ((Chan = find_channel(channel,CH_ACTIVE)) != NULL)
	{
		Chan->whois = TRUE;
		Chan->sync = TRUE;
	}
}

void parse_317(char *from, char *rest)
{
	char	*nick,*istr,*signon;
	long	c_idle,c_signon;
	int	min_idle;
	aTime	*aVarX;

	chop(&rest);
	nick = chop(&rest);
	istr = chop(&rest);
	signon = chop(&rest);

	c_idle = atol(istr);
	if (signon && *signon)
	{
		c_signon = atol(signon);
	}
	else
	{
		c_signon = 0;
	}

	min_idle = (c_idle / 60);

	for (aVarX = current.ParseList; aVarX; aVarX = aVarX->next)
	{
		if (aVarX->num == PA_WHOIS)
		{
			send_to_user(aVarX->name,"Idle            - %i minutes (%li seconds)",
				min_idle,c_idle);
			if (c_signon != 0)
			{
				send_to_user(aVarX->name,"Signed On       - %s",
					time2away(c_signon));
			}
		}
	}
}

void parse_318(char *from, char *rest)
{
	aTime	*aVarX, *aVarX2;

	aVarX = current.ParseList;
	while(aVarX)
	{
		if (aVarX->num == PA_WHOIS)
		{
			aVarX2 = aVarX->next;
			remove_time(&current.ParseList,aVarX);
			aVarX = aVarX2;
		}
		else
		{
			aVarX = aVarX->next;
		}
	}
}

void parse_319(char *from, char *rest)
{
	char	*nick;
	aTime	*aVarX;

	chop(&rest);
	nick = chop(&rest);
	if (!rest || !*rest)
		return;
	for(aVarX=current.ParseList;aVarX;aVarX=aVarX->next)
	{
		if (aVarX->num == PA_WHOIS)
		{
			if (*rest == ':')
				rest++;
			send_to_user(aVarX->name,"Channels        - %s",rest);
		}
	}
}

/*
 *  324 RPL_CHANNELMODEIS "<channel> <mode> <mode params>"
 */
void parse_324(char *from, char *rest)
{
	char	*channel;

	rest = strchr(rest,' ');
	rest++;
	channel = chop(&rest);
	if (channel && (*channel == ':'))
		channel++;
	on_mode(from,channel,rest);
}

/*
 *  352 RPL_WHOREPLY "<channel> <user> <host> <server> <nick> <H|G>[*][@|+] :<hopcount> <real name>"
 */
void parse_352(char *from, char *rest)
{
	aChanUser *User;
	char	*channel,*nick,*user,*host,*mode;
#ifdef SEEN
	char	nuh[MSGLEN];
#endif /* SEEN */

	chop(&rest);
	channel = chop(&rest);

	if ((CurrentChan = find_channel(channel,CH_ACTIVE)) == NULL)
		return;

	user = chop(&rest);
	host = chop(&rest);
	chop(&rest);
	nick = chop(&rest);
	mode = chop(&rest);

#ifdef SEEN
	sprintf(nuh,"%s!%s@%s",nick,user,host);
	make_seen(nick,nuh,"none",now,0,"no_info");
#endif /* SEEN */

	if (CurrentChan->whois == TRUE)
		return;

	User = make_chanuser(nick,user,host);

	while(*mode)
	{
		switch(*mode)
		{
		case '@':
			User->flags |= CU_CHANOP;
			User->flags &= ~CU_VOICE;
			if (is_me(nick))
				CurrentChan->bot_is_op = TRUE;
			break;
		case '+':
			User->flags |= CU_VOICE;
			User->flags &= ~CU_CHANOP;
			break;
		}
		mode++;
	}
}

void parse_367(char *from, char *rest )
{
	aChan	*Chan;
	char	*nick,*channel,*banstring,*banfrom,*bantime;

	nick = chop(&rest);
	channel = chop(&rest);

	if ((Chan = find_channel(channel,CH_ACTIVE)) == NULL)
		return;

	banstring = chop(&rest);
	banfrom = chop(&rest);
	bantime = chop(&rest);

	make_ban(&Chan->banned,banfrom,banstring,(bantime && *bantime) ? atol(bantime) : now);
}

/*
 *  401 ERR_NOSUCHNICK "<nickname> :No such nick/channel"
 */
void parse_401(char *from, char *rest)
{
	chop(&rest);
}

/*
 *  405 ERR_TOOMANYCHANNELS "<channel name> :You have joined too many channels"
 */
void parse_405(char *from, char *rest)
{
	aChan	*Chan;
	char	*channel;

	chop(&rest);
	channel = chop(&rest);

	if ((Chan = find_channel(channel,CH_ANY)) != NULL)
	{
		Chan->active = CH_OLD;
		Chan->sync = TRUE;
	}
}

void parse_433(char *from, char *rest) /* 431..436! */
/*
 * How to determine the new nick: 
 * - If it's possible to add a '_' to the nick, do it (done)
 * - else loop through the nick from begin to end and replace
 *   the first non-'_' with a '_'. i.e. __derServ -> ___erServ
 * - If the nick is 9 '_''s, try the original nick with something random
 */
{
	char	*s;

	if (current.connect == CN_ONLINE) return;
	
	if (strlen(current.nick) == 9)
	{
		for(s=current.nick; *s && (*s == '_'); s++)
			;
		if(*s)
			*s = '_';
		else
		{
			char	randomstr[5];
			
			strncpy(current.nick,current.wantnick,5);
			current.nick[5] = 0;
			sprintf(randomstr,"%d",(int)(rand() % 10000));
			Strcat(current.nick,randomstr);
		}
	}
	else
		Strcat(current.nick,"_");
	send_to_server("NICK %s", current.nick);
}

void parse_451(char *from, char *rest)
{
	send_to_server("NICK %s\nUSER %s . . :%s\n",current.wantnick,current.login,current.ircname);
}

void parse_471(char *from, char *rest)
{
	aChan	*Chan;
	char	*channel;

	chop(&rest);
	channel = chop(&rest);

	if ((Chan = find_channel(channel,CH_ANY)) != NULL)
	{
		Chan->active = CH_OLD;
		Chan->sync = TRUE;
	}
}

/*
 *  473 ERR_INVITEONLYCHAN "<channel> :Cannot join channel (+i)"
 */
void parse_473(char *from, char *rest)
{
#ifdef LINKING
	char	nuh[MSGLEN];
#endif /* LINKING */
	aChan	*Chan;
	char	*channel;

	chop(&rest);
	channel = chop(&rest);

	if ((Chan = find_channel(channel,CH_ANY)) != NULL)
	{
		Chan->active = CH_OLD;
		Chan->sync = TRUE;
	}
#ifdef LINKING
	if (current.userhost[0] == 0)
	{
		send_to_server("WHOIS %s",current.nick);
		return;
	}
	sprintf(nuh,"%s %s!%s",channel,current.nick,current.userhost);
	Link_needinvite(NULL,nuh);
#endif /* LINKING */
}

#define NEEDFROM	0x0001


/*
 *  Putting the most common first == less calls to Strcasecmp
 */
struct
{
	char	*name;
	int	flags;
	void	(*func)(char *, char *);

} pFuncsA[] =
{
	{ "PRIVMSG",	NEEDFROM,	parse_privmsg	},
	{ "JOIN",	NEEDFROM,	parse_join	},
	{ "PART",	NEEDFROM,	parse_part	},
	{ "MODE",	NEEDFROM,	parse_mode	},
	{ "NICK",	NEEDFROM,	on_nick		},
	{ "KICK",	NEEDFROM,	on_kick		},
	{ "PING",	0,		parse_ping	},
	{ "TOPIC",	NEEDFROM,	parse_topic	},
	{ "NOTICE",	NEEDFROM,	parse_notice	},
	{ "QUIT",	NEEDFROM,	parse_quit	},
	{ "INVITE",	NEEDFROM,	parse_invite	},
	{ "WALLOPS",	NEEDFROM,	parse_wallops	},
	{ "ERROR",	0,		parse_error	},
	{ NULL,		0,		NULL		}
};

/*
 *  Putting the most common first == less calls to Strcasecmp
 */
struct
{
	int	num;
	int	flags;
	void	(*func)(char *, char *);

} pFuncsN[] =
{
	{ 352,		NEEDFROM,	parse_352	},	/* RPL_WHOREPLY		*/
	{ 211,		NEEDFROM,	parse_213	},
	{ 213,		NEEDFROM,	parse_213	},
	{ 214,		NEEDFROM,	parse_213	},
	{ 215,		NEEDFROM,	parse_213	},
	{ 216,		NEEDFROM,	parse_213	},
	{ 217,		NEEDFROM,	parse_213	},
	{ 218,		NEEDFROM,	parse_213	},
	{ 219,		NEEDFROM,	parse_219	},
	{ 222,		NEEDFROM,	parse_213	},
	{ 223,		NEEDFROM,	parse_213	},
	{ 241,		NEEDFROM,	parse_213	},
	{ 242,		NEEDFROM,	parse_213	},
	{ 243,		NEEDFROM,	parse_243	},
	{ 244,		NEEDFROM,	parse_213	},
	{ 245,		NEEDFROM,	parse_213	},
	{ 251,		NEEDFROM,	parse_251	},
	{ 252,		NEEDFROM,	parse_252	},
	{ 253,		NEEDFROM,	parse_253	},
	{ 254,		NEEDFROM,	parse_254	},
	{ 255,		NEEDFROM,	parse_255	},
	{ 301,		NEEDFROM,	parse_301	},
	{ 311,		NEEDFROM,	parse_311	},
	{ 312,		NEEDFROM,	parse_312	},
	{ 313,		NEEDFROM,	parse_313	},
	{ 315,		NEEDFROM,	parse_315	},
	{ 317,		NEEDFROM,	parse_317	},
	{ 318,		NEEDFROM,	parse_318	},
	{ 319,		NEEDFROM,	parse_319	},
	{ 324,		NEEDFROM,	parse_324	},
	{ 367,		NEEDFROM,	parse_367	},
	{ 401,		NEEDFROM,	parse_401	},	/* ERR_NOSUCHNICK	*/
	{ 405,		NEEDFROM,	parse_405	},	/* ERR_TOOMANYCHANNELS	*/
	{ 433,		NEEDFROM,	parse_433	},
	{ 451,		NEEDFROM,	parse_451	},
	{ 471,		NEEDFROM,	parse_471	},
	{ 473,		NEEDFROM,	parse_473	},
	{ 474,		NEEDFROM,	parse_471	},
	{ 475,		NEEDFROM,	parse_471	},
	{ 476,		NEEDFROM,	parse_471	},
	/*
	 *  These only occur once for each server connect
	 */
	{ 001,		NEEDFROM,	parse_001	},
#ifdef PIPEUSER
	{ 002,		NEEDFROM,	parse_002	},
	{ 003,		NEEDFROM,	parse_003	},
	{ 004,		NEEDFROM,	parse_004	},
#endif /* PIPEUSER */
	{ 376,		NEEDFROM,	parse_EOMOTD	},	/* RPL_ENDOFMOTD	*/
	{ 422,		NEEDFROM,	parse_EOMOTD	},	/* ERR_NOMOTD		*/
	{ 0,		0,		NULL		}
};

void parseline(char *rest)
{
	char	*from,*command;
	int	i,num,*ip;

	from = chop(&rest);
	if (!from || !*from)
		return;
	if (*from == ':')
	{
		from++;
		command = chop(&rest);
	}
	else
	{
		command = from;
		from = NULL;
	}
	if (*rest == ':')
		rest++;
	if (*command > '0' && *command < '9')
	{
		ip = Atoi(command);
		if (!ip)
			return;
		num = *ip;
		for(i=0;pFuncsN[i].num;i++)
		{
			if (pFuncsN[i].num == num)
			{
				if ((pFuncsN[i].flags & NEEDFROM) && (!from))
					return;
				pFuncsN[i].func(from,rest);
				return;
			}
		}
	}
	else
	{
		for(i=0;pFuncsA[i].name;i++)
		{
			if (!Strcasecmp(pFuncsA[i].name,command))
			{
				if ((pFuncsA[i].flags & NEEDFROM) && (!from))
					return;
				pFuncsA[i].func(from,rest);
				return;
			}
		}
	}
}
