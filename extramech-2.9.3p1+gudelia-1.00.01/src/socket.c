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


int get_ip(char *hostname, union sockaddr_union *so) {
#ifdef IPV6
	struct addrinfo hints, *ai, *res;
	int error;
#else
	struct hostent *hp;
#endif

	memset(so, 0, sizeof(union sockaddr_union));

#ifdef IPV6
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_socktype = SOCK_STREAM;

	if (!hostname) {
		so->sin6.sin6_family = AF_INET6;
		so->sin6.sin6_addr = in6addr_any;
		return 0;
	}

	if ((error = getaddrinfo(hostname, NULL, &hints, &res)))
		return error;

	error = 1;
	for (ai = res; ai != NULL; ai = ai->ai_next) {
		if ((ai->ai_family == AF_INET6) || (ai->ai_family == AF_INET)) {
			memcpy(so, ai->ai_addr, sizeof(union sockaddr_union));
			error = 0;
			break;
		}
	}

	freeaddrinfo(res);
	return error;
#else
	if (!hostname) {
		so->sin.sin_family = AF_INET;
		so->sin.sin_addr.s_addr = INADDR_ANY;
		return 0;
	}

	if (!(hp = gethostbyname(hostname)))
		return -1;

	memcpy(&so->sin.sin_addr, hp->h_addr, 4);
	so->sin.sin_family = AF_INET;
	return 0;
#endif
}

int SockOpts(int family)
{
	int	s,pram;

	if ((s = socket(family,SOCK_STREAM,0)) < 0)
		return(-1);

	pram = 0;
	setsockopt(s,SOL_SOCKET,SO_LINGER,(char*)&pram,sizeof(int));
	pram = 1;
	setsockopt(s,SOL_SOCKET,SO_KEEPALIVE,(char*)&pram,sizeof(int));

	fcntl(s,F_SETFL,O_NONBLOCK | fcntl(s,F_GETFL));

	return(s);
}

int SockListener(int port)
{
	union sockaddr_union so;
	int	s;

#ifdef IPV6
	if ((s = SockOpts(AF_INET6)) < 0)
		return(-1);
	memset(&so,0,sizeof(union sockaddr_union));
	so.sin6.sin6_family = AF_INET6;
	so.sin6.sin6_addr = in6addr_any;
	so.sin6.sin6_port = htons(port);
#else
	if ((s = SockOpts(AF_INET)) < 0)
		return(-1);
	memset(&so,0,sizeof(union sockaddr_union));
	so.sin.sin_family = AF_INET;
	so.sin.sin_addr.s_addr = INADDR_ANY;
	so.sin.sin_port = htons(port);
#endif /* IPV6 */
	if ((bind(s,&so.sa,SIZEOF_SOCKADDR(so)) < 0) || (listen(s,1) < 0))
	{
		close(s);
		return(-1);
	}
	return(s);
}

int SockConnect(char *host, int port, int use_vhost)
{
	union  sockaddr_union so;
	int     s=-1,uv;

#ifdef DEBUG
	debug("[CbN] %s %i\n",nullstr(host),port);
#endif /* DEBUG */

	if (!host)
	{
#ifdef DEBUG
		debug("[CbN] Hostname missing!\n");
#endif /* DEBUG */
		close(s);
		return(-1);
	}
	if (use_vhost) {
		if (current->vhost[0] && current->vhost_type == VH_IPALIAS)
		{
			memset(&so,0,sizeof(so));
			if (!get_ip(current->vhost, &so))
			{
				if ((s = SockOpts(so.sa.sa_family)) < 0)
					return(-1);

				if (bind(s,&so.sa,SIZEOF_SOCKADDR(so)) >= 0)
				{
#ifdef DEBUG
					debug("[CbN] IP Alias virtual host bound OK\n");
#endif /* DEBUG */
				}
			}
			else
				current->vhost_type = VH_IPALIAS_FAIL;
		}
	}

#ifdef WINGATES
	/* bouncer connects */
	if (current->vhost[0] && current->vhost_type == VH_WINGATE)
	{
		memset(&so,0,sizeof(so));
		if (get_ip(current->vhost, &so))
		{
			current->vhost_type = VH_WINGATE_FAIL;
			return(-1);
		}

#ifdef IPV6
		if (so.sin.sin_family == AF_INET6)
			so.sin6.sin6_port = htons(current->vhost_port);
		else
#endif
			so.sin.sin_port = htons(current->vhost_port);

#ifdef DEBUG
		debug("[CbN] WINGATE host resolved OK\n");
#endif /* DEBUG */
	}
	/* Normal connect, no bounces... */
	else
#endif /* WINGATES */
	{
		memset(&so,0,sizeof(so));
		if (get_ip(host, &so))
			return(-1);
#ifdef IPV6
		if (so.sin.sin_family == AF_INET6)
			so.sin6.sin6_port = htons(port);
		else
#endif
			so.sin.sin_port = htons(port);

	}

	if (s < 0) {
		if ((s = SockOpts(so.sa.sa_family)) < 0)
			return(-1);
	}

	uv = connect(s,&so.sa,SIZEOF_SOCKADDR(so));

	if ((uv < 0) && (errno != EINPROGRESS))
	{
#ifdef DEBUG
		debug("[CbN] unable to connect. errno = %i\n",errno);
#endif /* DEBUG */
		close(s);
		return(-1);
	}

	return(s);
}

/*
 *  Format text and send to a socket or file descriptor
 */
int send_to_socket(int sock, char *format, ...)
{
	char	bigbuf[HUGE];
	va_list msg;
#ifdef DEBUG
	char	*pt,*p;
	int	i;
#endif /* DEBUG */

	if (sock == -1)
		return(-1);
	va_start(msg,format);
	vsprintf(bigbuf,format,msg);
	va_end(msg);

#ifdef DEBUG
	i = write(sock,bigbuf,strlen(bigbuf));
	pt = bigbuf;
	while(pt && *pt)
	{
		p = get_token(&pt,"\r\n");
		debug("[SSo] {%i} %s\n",sock,nullstr(p));
	}
	if (i < 0)
		debug("[SSo] {%i} errno = %i\n",sock,errno);
	return(i);
#else /* DEBUG */
	return(write(sock,bigbuf,strlen(bigbuf)));
#endif /* DEBUG */
}

/*
 *  Format a PRIVMSG and send to current bots server
 */
void sendprivmsg(char *to, char *format, ...)
{
	char	buf[HUGE];
	va_list msg;

	va_start(msg,format);
	vsprintf(buf,format,msg);
	va_end(msg);

	current->activity = now;

	to_server("PRIVMSG %s :%s",to,buf);
}

/*
 *  Format a MODE and send to current bots server
 */
void sendmode(char *to, char *format, ...)
{
	char	buf[HUGE];
	va_list msg;

	va_start(msg,format);
	vsprintf(buf,format,msg);
	va_end(msg);

	to_server("MODE %s %s",to,buf);
}

/*
 *  Format a KICK and send to current bots server
 */
void sendkick(char *channel, char *nick, char *format, ...)
{       
	char	buf[HUGE];
	va_list msg;

	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);

	to_server("KICK %s %s :%s", channel, nick, buf);
}

/*
 *  Format a CTCP reply and send to current bots server
 */
void send_ctcp_reply(char *to, char *format, ...)
{
	char	buf[HUGE];
	va_list msg;

	if ((now - current->lastctcp) < 5)
		return;
	current->lastctcp = now;

	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);
	to_server("NOTICE %s :\001%s\001", to, buf);
}

/*
 *  Format a CTCP and send to current bots server
 */
void send_ctcp(char *to, char *format, ...)
{
	char	buf[HUGE];
	va_list msg;

	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);

	to_server("PRIVMSG %s :\001%s\001", to, buf);
}

/*
 *  Format a message and send it either through DCC if the user is
 *  connected to the partyline, or send it as a NOTICE if he's not
 */
void send_to_user(char *to, char *format, ...)
{
#ifdef DEBUG
	char	*pt,*copy;
#endif /* DEBUG */
#ifdef LINKING
	aBot	*bk;
	char	tmp[HUGE];
#endif /* LINKING */
	va_list msg;
	char	buf[HUGE];

	if (!to)
		return;

	va_start(msg,format);
	vsprintf(buf,format,msg);
	va_end(msg);

#ifdef DEBUG
	set_mallocdoer(send_to_user);
	mstrcpy(&copy,buf);
	pt = copy;
	while(pt && *pt)
		debug("[StU] {%s} %s\n",getnick(to),get_token(&pt,"\n"));
	MyFree(&copy);
#endif /* DEBUG */
	if (to && *to)
	{
#ifdef LINKING
		if (*to == '$')
		{
#ifdef DEBUG
			debug("(send_to_user) ==> %s\n",to);
#endif /* DEBUG */
			bk = current;
			sprintf(tmp,"%s %s %s",current->nick,to,escape_string(buf));
			Link_reply(NULL,tmp);
			current = bk;
			return;
		}
#endif /* LINKING */
		if (send_chat(to,buf))
			return;
		if (need_dcc || shell_cmd)
			return;
		if (*to == '*')
			return;
		to_server("NOTICE %s :%s",getnick(to),buf);
	}
}

/*
 *  Format a message and send it to the current bots server
 */
void to_server(char *format, ...)
{
	va_list msg;
	char	buf[HUGE];
	int	temp;

	if (current->sock == -1)
		return;
	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);
	Strcat(buf,"\n");
	if ((temp = write(current->sock,buf,strlen(buf))) < 0)
	{
#ifdef DEBUG
		debug("[StS] {%i} errno = %i\n",current->sock,errno);
#endif /* DEBUG */
		close(current->sock);
		current->sock = -1;
		current->connect = CN_NOSOCK;
		return;
	}
#ifdef DEBUG
	{
		char	*pop,*args;   
 
		args = buf;
		pop = get_token(&args,"\n");
		while(pop && *pop)
		{
			debug("[StS] {%i} %s\n",current->sock,pop);
			pop = get_token(&args,"\n");
		}
	}
#endif /* DEBUG */
}

/*
 *  Read any data waiting on a socket or file descriptor
 *  and return any complete lines to the caller
 */
char *sockread(int s, char *rest, char *line)
{
	char	tmp[MSGLEN];
	char	*pt,*tp;
	int	l;

	pt = rest;
	while(*pt)
	{
		if ((*pt == '\n') || (*pt == '\r'))
		{
			*pt = 0;
			strcpy(line,rest);
			pt++;
			while((*pt == '\n') || (*pt == '\r'))
				pt++;
			strcpy(rest,pt);
#ifdef DEBUG
			debug("[SoR] {%i} %s\n",s,line);
#endif /* DEBUG */
			errno = EAGAIN;
			return((*line) ? line : NULL);
		}
		pt++;
	}
	l = pt - rest;
	tp = pt;
	memset(tmp,0,sizeof(tmp));
	switch(read(s,tmp,MSGLEN-2))
	{
	case -1:
		return(NULL);
	case 0:
		errno = EPIPE;
		return(NULL);
	}
	pt = tmp;
	while(*pt)
	{
		if ((*pt == '\r') || (*pt == '\n'))
		{
			*tp = *pt = 0;
			strcpy(line,rest);
			pt++;
			while((*pt == '\n') || (*pt == '\r'))
				pt++;
			strcpy(rest,pt);
#ifdef DEBUG
			debug("[SoR] {%i} %s\n",s,line);
#endif /* DEBUG */
			errno = EAGAIN;
			return((*line) ? line : NULL);
		}
		if (l >= MSGLEN-2)
		{
			pt++;
			continue;
		}
		*(tp++) = *(pt++);
		l++;
	}
	*tp = 0;
	return(NULL);
}

void readline(int s, int (*callback)(char *))
{
	char	linebuf[MSGLEN],readbuf[MSGLEN];
	char	*ptr;
	int	oc;

	memset(readbuf,0,sizeof(readbuf));
	while(TRUE)
	{
		ptr = sockread(s,readbuf,linebuf);
		oc = errno;
		if (ptr && *ptr)
		{
			if (callback(ptr) == TRUE)
				oc = EPIPE;
		}
		if (oc != EAGAIN)
			break;
	}
#ifdef DEBUG
	debug("(readline) done reading lines\n");
#endif /* DEBUG */
}

int killsock(int sock)
{
	aKillSock *ks,*ksnext,*ksp;
	struct	timeval t;
	fd_set	rd,wd;
	char	bitbucket[MSGLEN];
	int	hisock,n;

	if (sock >= 0)
	{
		set_mallocdoer(killsock);
		ks = (aKillSock*)MyMalloc(sizeof(aKillSock));
		ks->time = now;
		ks->sock = sock;
		ks->next = killsocks;
		killsocks = ks;
#ifdef DEBUG
		debug("(killsock) {%i} added killsocket\n",ks->sock);
#endif /* DEBUG */
		return(TRUE);
	}

	if (killsocks == NULL)
		return(FALSE);

	if (sock == -1)
		t.tv_sec = 0;
	else
		t.tv_sec = 1;
	t.tv_usec = 0;

	FD_ZERO(&rd);
	FD_ZERO(&wd);

	hisock = -1;
	for(ks=killsocks;ks;ks=ks->next)
	{
		if (ks->sock > hisock)
			hisock = ks->sock;
		FD_SET(ks->sock,&rd);
		FD_SET(ks->sock,&wd);
	}

	if (select(++hisock,&rd,&wd,NULL,&t) == -1)
	{
		switch(errno)
		{
		case EINTR:
			/* have to redo the select */
			return(TRUE);
		case ENOMEM:
			/* should be very rare, but still */
			mechexit(0);
		}
	}

	ks = killsocks;
	while(ks)
	{
		ksnext = ks->next;
		if (FD_ISSET(ks->sock,&rd))
		{
			n = read(ks->sock,&bitbucket,MSGLEN);
			if ((n == 0) || ((n == -1) && (errno != EAGAIN)))
			{
				/*
				 *  EOF or error; remove it.
				 */
				close(ks->sock);
				if (ks == killsocks)
				{
					killsocks = ksnext;
				}
				else
				{
					ksp = killsocks;
					while(ksp && (ksp->next != ks))
						ksp = ksp->next;
					if (ksp)
					{
						ksp->next = ksnext;
					}
				}
#ifdef DEBUG
				debug("(killsock) {%i} removing killsocket\n",ks->sock);
#endif /* DEBUG */
				MyFree((char**)&ks);
			}
		}
		ks = ksnext;
	}
	return(TRUE);
}
