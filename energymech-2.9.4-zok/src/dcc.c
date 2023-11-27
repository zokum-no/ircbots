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
#include "config.h"

aDCC *search_list(char *user, Ulong type)
{
	aDCC	*Client;

	for(Client=current->ClientList;Client;Client=Client->next)
	{
		if (((Client->flags & type) == type) && (!Strcasecmp(user,Client->user)))
			return(Client);
	}
	return(NULL);
}

void change_dccnick(char *olduser, char *newnuh)
{
	aDCC	*Client;

	for(Client=current->ClientList;Client;Client=Client->next)
	{
		if (!Strcasecmp(olduser,Client->user))
			strcpy(Client->user,newnuh);
	}
}

void add_client(aDCC *new)
{
	new->next = current->ClientList;
	current->ClientList = new;
}

void delete_client(aDCC **Client)
{
	aMsgList *pt;
	aDCC	*rem,*ptr;
	int	i,maxul;

#ifdef LINKING
	link_dccuser(getnick((*Client)->user),FALSE);
#endif /* LINKING */

	ptr = *Client;
	if (*Client == current->ClientList)
	{
		rem = *Client;
		current->ClientList = rem->next;
	}
	else
	{
		rem = current->ClientList;
		while(rem->next != *Client)
			rem = rem->next;
		rem->next = (*Client)->next;
	}

	pt = current->msglistBASE;
	for(i=0;i<current->msglistSIZE;i++)
	{
		if ((pt->used) && (!Strcasecmp(pt->who,ptr->user)))
			memset(pt,0,sizeof(aMsgList));
		pt++;
	}

	maxul = max_userlevel(ptr->user);
	send_global_statmsg(TEXT_DCCDISCONNECT,
		current->nick,getnick(ptr->user),maxul);

	close(ptr->sock);
	MyFree((char **)&ptr);
}

void close_all_dcc(void)
{
	while(current->ClientList)
	{
		delete_client(&current->ClientList);
	}
}

void process_incoming_chat(aDCC *Client)
{
	union 	sockaddr_union so;
	char	linebuf[MSGLEN];
	char	*ptr,*bp;
	int	maxul,size;
	int	s,oc;

	if (Client->flags & DCC_WAIT)
	{
#ifdef DEBUG
		debug("(process_incoming_chat) new user connecting\n");
#endif /* DEBUG */
		size = sizeof(union sockaddr_union);
		s = accept(Client->sock,&so.sa,&size);
		close(Client->sock);
		if (s == -1)
		{
			Client->flags = DCC_DELETE;
			return;
		}
		Client->sock = s;
		fcntl(s,F_SETFL,O_NONBLOCK | fcntl(s,F_GETFL));
		Client->flags &= ~DCC_WAIT;
		Client->flags |= DCC_ACTIVE;
		Client->lasttime = now;

		maxul = max_userlevel(Client->user);
#ifdef LINKING
		link_dccuser(getnick(Client->user),TRUE);
#endif /* LINKING */
		sprintf(linebuf,TEXT_DCCCONNECT,
			current->nick,getnick(Client->user),maxul);
		if ((send_to_socket(Client->sock,"[%s] %s\n",time2medium(now),linebuf)) < 0)
		{
			Client->flags = DCC_DELETE;
			return;
		}
		send_global_statmsg(linebuf);
		if (maxul == OWNERLEVEL)
		{
			do_statmsg(Client->user,current->nick,NULL,OWNERLEVEL);
		}
		return;
	}
	errno = EAGAIN;
	ptr = sockread(Client->sock,Client->sd,linebuf);
	oc = errno;
	if (ptr && *ptr)
	{
		s = now - Client->lasttime;
		if (s > 10)
		{
			Client->inputcount = strlen(ptr);
		}
		else
		{
			Client->inputcount += strlen(ptr);
			if ((Client->inputcount - (s * DCC_INPUT_DECAY)) > DCC_INPUT_LIMIT)
			{
				Client->flags = DCC_DELETE;
				return;
			}
		}
		Client->lasttime = now;
		CurrentUser = find_user(Client->user,NULL);
		CurrentShit = find_shit(Client->user,NULL);
		strcpy(CurrentNick,getnick(Client->user));
		if (*ptr == 1)
		{
			bp = ptr;
			chop(&bp);
			ptr = get_token(&bp,"\001");
		}
		on_msg(Client->user,current->nick,ptr);
		return;
	}
	switch(oc)
	{
	case EAGAIN:
	case EINTR:
		return;
	default:
		Client->flags = DCC_DELETE;
		return;
	}
}       

void parse_dcc_input(void)
{
	aDCC	*Client;
	char	tmp[MSGLEN];
	int	maxul;

	for(Client=current->ClientList;Client;Client=Client->next)
	{
		if ((Client->flags & DCC_ASYNC) && (Client->sock != -1) &&
		    (FD_ISSET(Client->sock,&write_fds)))
		{
#ifdef DEBUG
			debug("(parse_dcc_input) chat connected [ASYNC]\n");
#endif /* DEBUG */
			Client->flags &= ~DCC_ASYNC;
			Client->flags &= ~DCC_WAIT;
			Client->flags |= DCC_ACTIVE;
			Client->lasttime = now;

			maxul = max_userlevel(Client->user);
#ifdef LINKING
			link_dccuser(getnick(Client->user),TRUE);
#endif /* LINKING */
			sprintf(tmp,TEXT_DCCCONNECT,
				current->nick,getnick(Client->user),maxul);
			if ((send_to_socket(Client->sock,"[%s] %s\n",time2medium(now),tmp)) < 0)
			{
				Client->flags = DCC_DELETE;
				return;
			}
			send_global_statmsg(tmp);
			if (maxul == OWNERLEVEL)
			{
				do_statmsg(Client->user,current->nick,NULL,OWNERLEVEL);
			}
		}
		if ((Client->sock != -1) && FD_ISSET(Client->sock,&read_fds))
		{
			if ((Client->flags & DCC_CHAT) == DCC_CHAT)
			{
				process_incoming_chat(Client);
				if (*Client->rest)
					{
						on_msg(Client->user,"-",Client->rest);
						*Client->rest = 0;
					}
			}
		}
		if ((Client->flags & DCC_WAIT) && ((now - Client->lasttime) >= WAITTIMEOUT))
		{
#ifdef DEBUG
			debug("(parse_dcc_input) connection timed out (%s)\n",Client->user);
#endif /* DEBUG */
			Client->flags = DCC_DELETE;
			return;
		}
	}
}

void dcc_connect(char *from, char *rest)
{
	union sockaddr_union so;
	aDCC	*Client;
	char	*sip,*port,ip_addr[30];
	Ulong	ipl;
	int	iport;

	if (search_list(from,DCC_CHAT))
		return;
	if (password_needed(from) && !verified(from))
	{
		send_to_user(from,TEXT_USEVERIFY);
		return;
	}
	chop(&rest);
	sip = chop(&rest);
	port = chop(&rest);
	if (!port || !*port)
		return;
	iport = atoi(port);
	if ((iport < 1024) || (iport > 65535))
		return;

#ifdef IPV6
	if (sip[0]=='[') {				/* some ipv6 clients send [ip]	*/
		sip++;					/* ... so we strip it out       */
		sip[strlen(sip)]='\0';
	}

	if (!strchr(sip,':'))
#endif /* IPV6 */
	{
		if (sscanf(sip,"%lu",&ipl) != 1)
			return;
		so.sin.sin_addr.s_addr = htonl(ipl);
		strcpy(ip_addr,inet_ntoa(so.sin.sin_addr));
		sip = ip_addr;
	}

#ifdef DEBUG
	debug("(dcc_connect) %s [%s,%s]\n",from,inet_ntoa(so.sin.sin_addr),port);
#endif /* DEBUG */
	set_mallocdoer(dcc_connect);
	Client = (aDCC*)MyMalloc(sizeof(aDCC));
	strcpy(Client->user,from);
	Client->sock = -1;
	Client->flags = DCC_CHAT|DCC_WAIT|DCC_ASYNC;
	Client->lasttime = now;
	add_client(Client);
	Client->sock = SockConnect(sip,iport,FALSE);
	if (Client->sock < 0)
		Client->flags = DCC_DELETE;
}

void dcc_chat(char *from, char *rest)
{
	union sockaddr_union so;
	aDCC	*Client;
	int	sz;

	if (search_list(from,DCC_CHAT))
		return;

	set_mallocdoer(dcc_chat);
	Client = (aDCC*)MyMalloc(sizeof(aDCC));
	strcpy(Client->user,from);
	if (rest)
		strcpy(Client->rest,rest);
	else
		*Client->rest = 0;
	Client->sock = -1;
	Client->flags = DCC_CHAT|DCC_WAIT;
	Client->lasttime = now;

	add_client(Client);

	Client->flags |= DCC_WAIT;
	if ((Client->sock = SockListener(0)) < 0)
	{       
		Client->flags = DCC_DELETE;
		return;
	}
	sz = sizeof(union sockaddr_union);
	if (getsockname(Client->sock,&so.sa,&sz) < 0)
	{
		Client->flags = DCC_DELETE;
		return;
	}

#ifdef IPV6
	if (current->ip.sa.sa_family == AF_INET6) {
		send_ctcp(getnick(Client->user),"DCC CHAT CHAT %s %i",
			ip2string(so.sin6.sin6_addr),ntohs(so.sin6.sin6_port));
	} else
#endif /* IPV6 */
	{
		send_ctcp(getnick(Client->user),"DCC CHAT CHAT %lu %i",
			htonl(current->ip.sin.sin_addr.s_addr),ntohs(so.sin.sin_port));
	}
}

int send_chat(char *to, char *text)
{
	aDCC	*Client;
	int	bytessend;

	if ((Client = search_list(to,DCC_CHAT)) == NULL)
		return(FALSE);
 
	if (!(Client->flags & DCC_ACTIVE))
		return(FALSE);

	switch(bytessend = send_to_socket(Client->sock,"%s\n",text))
	{
	case -1:
		Client->flags = DCC_DELETE;
		return(FALSE);
	case 0:
		break;
	default:
		Client->lasttime = now;
	}
	return(TRUE);
}

void ctcp_version(char *from, char *to, char *rest)
{
	char	*st;
	int	maxul;

	maxul = max_userlevel(from);
	send_statmsg(TEXT_CTCPVERREQ,CurrentNick,maxul);

	if (!get_int_varc(NULL,TOGCLOAK_VAR))
	{
		if (get_int_varc(NULL,TOGRV_VAR))
		{
			st = randstring(VERSIONFILE);
			if (st != NULL)
			{
				send_ctcp_reply(CurrentNick,"VERSION %s",st);
				return;
			}
		}
		send_ctcp_reply(CurrentNick,"VERSION %s",EXVERSION);
	}
}

void ctcp_finger(char *from, char *to, char *rest)
{
	char	*st;
	int	maxul;

	maxul = max_userlevel(from);
	send_statmsg(TEXT_CTCPFINGERREQ,CurrentNick,maxul);

	if (!get_int_varc(NULL,TOGCLOAK_VAR))
	{
		if (get_int_varc(NULL,TOGRF_VAR))
		{
			st = randstring(VERSIONFILE);
			if (st != NULL)
			{
				send_ctcp_reply(CurrentNick,"FINGER %s",st);
				return;
			}
		}
		send_ctcp_reply(CurrentNick,"FINGER %s",EXFINGER);
	}
}

void ctcp_dcc(char *from, char *to, char *rest)
{
	char	*type,*p;
	int	maxul;

	if ((!rest) || (!*rest))
		return;

	type = chop(&rest);
	p = type;
	while(*p && (*p = toupper(*p)))
		p++;
	maxul = max_userlevel(from);
	send_statmsg(TEXT_DCCREQ,CurrentNick,maxul,type,nullstr(rest));

	if ((maxul >= MINUSERLEVEL) && (maxul != BOTLEVEL) &&
	    (!Strcasecmp(type,"CHAT")))
	{
		dcc_connect(from,rest);
		return;
	}
}

void ctcp_ping(char *from, char *to, char *rest)
{
	int	maxul;

	if ((!rest) || (!*rest) || (strlen(rest) > 100))
		return;
	maxul = max_userlevel(from);
	send_statmsg("[CTCP] :%s[%i]: Requested Ping Info",CurrentNick,maxul);
	if ((maxul >= ASSTLEVEL) || (!get_int_varc(NULL,TOGCLOAK_VAR)))
	{
		send_ctcp_reply(CurrentNick,"PING %s",(rest) ? rest : "");
	}
}

struct
{
	char    *name;
	void    (*func)(char *, char *, char *);

} ctcp_commands[] =
{
	{ "VERSION",		ctcp_version	},
	{ "FINGER",		ctcp_finger	},
	{ "ACTION",		on_actionmsg	},
	{ "PING",		ctcp_ping	},
	{ "DCC",		ctcp_dcc	},
	{ NULL,			NULL		}
};

void on_ctcp(char *from, char *to, char *rest)
{
	char    *command;
	int     i,mul;

	if ((command = chop(&rest)) == NULL)
		return;
	for(i=0;ctcp_commands[i].name;i++)
	{
		if (!Strcasecmp(ctcp_commands[i].name,command))
		{
			ctcp_commands[i].func(from,to,rest);
			return;
		}
	}
	mul = max_userlevel(from);
	send_statmsg(TEXT_CTCPUNKNOWN,CurrentNick,mul,command);
}
