/*
 * dcc.c - an effort to implement dcc (at least chat) in ComBot
 * (c) 1995 CoMSTuD (cbehrens@slavery.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Thanx to Richie_B for patching this for Ultrix.
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include "global.h"
#include "config.h"
#include "defines.h"
#include "structs.h"
#include "h.h"

struct 
{
	char    *name;
	void    (*function)();
} dcc_commands[] =
{
	{ "CHAT",       dcc_chat },
/*	{ "SEND",       dcc_sendfile }, */
	{ NULL,         (void(*)())(NULL) }
};

char *dcc_types[] =
{
	"<null>",
	"CHAT",
	"SEND",
	"GET",
	NULL
};      

void dccset_fds(rds, wds)
fd_set *rds;
fd_set *wds;
{
	register aDCC *Client;

	for (Client = current->Client_list; Client != NULL;
	    Client = Client->next)
		if (Client->read != -1)
			FD_SET(Client->read, rds);
}

aDCC *search_list(name, user, type)
char *name;
char *user;
unsigned int type;
{
	register aDCC *Client;

	for (Client = current->Client_list; Client; Client = Client->next)
	{
		if (((Client->flags&DCC_TYPES) == type) &&
			 (!my_stricmp(name, Client->description)) &&
			 (!my_stricmp(user, Client->user)))
			return Client;
	}
	return NULL;
}

void change_dccnick(olduser, newnuh)
char *olduser;
char *newnuh;
{
	register aDCC *Client;

	for (Client=current->Client_list;Client;Client=Client->next)
		if (!my_stricmp(olduser, Client->user))
			strcpy(Client->user, newnuh);
}

int do_dcc(Client)
aDCC *Client;
{
	struct sockaddr_in localaddr;
	struct     in_addr party, MyHostAddr;
	struct     hostent *hp;
	char               localhost[64];
	int                size;
	unsigned long      my_ip;
	unsigned long      TempLong;
	char               *Descr;
	char               *Type;
	char               filebuf[MAXLEN];

#ifdef DBUG
	debug(NOTICE, "Entered do_dcc( .. )");
#endif
	Type = dcc_types[Client->flags&DCC_TYPES];

	if (Client->flags & DCC_OFFER)
	{
		sscanf(Client->ip_addr, "%lu", &TempLong);
		party.s_addr = htonl(TempLong);

		if ( /* (Client->flags&DCC_FILEREAD) || */ ((Client->write = connect_by_number(Client->port,
			 inet_ntoa(party))) < 0))
		{
			delete_client(Client);
			return FAIL;
		}
		Client->read = Client->write;
		Client->flags &= ~DCC_OFFER;
		Client->flags |= DCC_ACTIVE;
		if(((Client->flags & DCC_TYPES) == DCC_FILEREAD) &&  
		(Descr = strrchr( Client->description, '/')))
			Descr++;
		else                                 
			Descr = Client->description;  
		if ((Client->flags & DCC_TYPES) == DCC_FILEREAD)
		{                                               
			sprintf(filebuf, "%s/%s", current->uploaddir,
				Descr);
			if((Client->file = open( filebuf, O_WRONLY |
			O_TRUNC | O_CREAT, 0644 ))==-1)
			{
				log_it(DCC_LOG_VAR, 0,
				"DCC upload: Unable to open %s from %s",
					Descr, Client->user);
				delete_client(Client);
				return FAIL;
			}                  
			log_it(DCC_LOG_VAR, 0, "DCC upload %s from %s", Descr,
			       Client->user);
		}
		Client->starttime = time(NULL);
		Client->lasttime = time(NULL);
		Client->bytes_read = Client->bytes_sent = 0;
		return 1;
	}
	else
	{
		Client->flags |= DCC_WAIT;
		if((Client->read = connect_by_number(0, "")) < 0)
		{       
			delete_client(Client);
			return FAIL;
		}
		size = sizeof(struct sockaddr_in);
		getsockname(Client->read, (struct sockaddr *)&localaddr, 
			    &size );
		gethostname(localhost, 64);
		if ((hp=gethostbyname(localhost)) != NULL)
			bcopy(hp->h_addr, (char*) &MyHostAddr, 
			      sizeof(MyHostAddr));              
		my_ip = (unsigned long)ntohl(MyHostAddr.s_addr);
		if(Client->flags & DCC_TWOCLIENTS)
		{
			if((Client->flags & DCC_FILEOFFER) &&
			  (Descr = strrchr(Client->description, '/')))
				Descr++;
			else
				Descr = Client->description;
			if (Client->filesize)
				send_ctcp(getnick( Client->user),
					  "DCC %s %s %lu %u %d", Type, Descr,
					  (unsigned long) my_ip,
					  (unsigned)ntohs(localaddr.sin_port),
					  Client->filesize);
			else
				send_ctcp(getnick( Client->user),
					  "DCC %s %s %lu %u", Type, Descr,
					  (unsigned long) my_ip,
					  (unsigned)ntohs(localaddr.sin_port));
		}
		Client->starttime = time(NULL);
		Client->lasttime = time(NULL);
		return 2;
	}
}                                       


void reply_dcc(from, to, rest)
char *from;
char *to;
char *rest;
{
	char    *type;
	char    *description;
	char    *inetaddr;
	char    *port;
	char    *size;

	if (ischannel(to))
		return;
	if (!get_int_varc(NULL, TOGCC_VAR))
		if (!max_userlevel(from))
			return;
	if (!(type = get_token(&rest, " ")) ||
	   !(description = get_token(&rest, " ")) ||
	   !(inetaddr = get_token(&rest, " ")) ||
	   !(port = get_token(&rest, " ")))
		return;
	size = get_token(&rest, " ");
	register_dcc_offer(from, type, description, inetaddr, port, size);
}

void show_dcclist(from)
char *from;
{
	register aDCC  *Client;
	unsigned  flags;

	if (current->Client_list == NULL)
	{
		send_to_user(from, "\002%s\002", "No connections made");
		return;
	}

	for (Client = current->Client_list; Client != NULL;
	    Client = Client->next )
	{
		flags = Client->flags;
		send_to_user( from, "Type: %-8.8s User: %s",
		      dcc_types[flags&DCC_TYPES], Client->user );
		send_to_user( from, "Status: %11s. Descr: %s", 
		      flags&DCC_DELETE ? "Closed": flags&DCC_ACTIVE ? "Active" :
		      flags&DCC_WAIT ? "Waiting" : flags&DCC_OFFER ? "Offered" :
		      "Unknown", Client->description );
		send_to_user( from, "Starttime = %-20.20s, Bytes sent: %d", 
		      time2str(Client->starttime), Client->bytes_sent );
		send_to_user( from, "Lasttime  = %-20.20s, Bytes read: %d", 
		      time2str(Client->lasttime), Client->bytes_read );
		send_to_user( from, "Idle: %s", 
				idle2str(getthetime()-Client->lasttime));
	}
}

void add_client(Client)
aDCC *Client;
{
	Client->next = current->Client_list;
	current->Client_list = Client;
}

int delete_client(Client)
aDCC *Client;
{
	register aDCC **lame;

	for (lame = &(current->Client_list); *lame; lame = &((*lame)->next))
		if (*lame == Client)
		{
		/* Can we use send_to_user here? It might be a send/get... */
		  sendnotice(getnick(Client->user), 
			     "\002Closing DCC %s[%s]-connection\002",
			     dcc_types[Client->flags&DCC_TYPES],
			     strrchr(Client->description, '/') ?
			     strrchr(Client->description, '/')+1:
			     Client->description);
			close(Client->read);
			close(Client->write);
			if(Client->file != -1) 
				close(Client->file);
			*lame = Client->next;
			MyFree((char **)&Client);
                           
			return TRUE;
		}
	return FALSE;
}                       

void close_all_dcc(void)
{
	register aDCC *Client;

	for(; (Client = current->Client_list) != NULL;)
		delete_client(Client);
}

void register_dcc_offer(from, type, description, inetaddr, port, size)
char *from;
char *type;
char *description;
char *inetaddr;
char *port;
char *size;
{
	aDCC *Client;

#ifdef DBUG
	debug(NOTICE, "Entered register_dcc_offer( %s, %s, %s, %s, %s, %s );",
		from, type, description, inetaddr, port, size );
#endif
	if ((Client = (aDCC *)MyMalloc(sizeof(aDCC))) == NULL)
		return;
	strcpy(Client->user, from);
	strcpy(Client->description, description);
	strcpy(Client->ip_addr, inetaddr);
	Client->read = Client->read = Client->file = -1;
	Client->flags = 0;
	Client->flags |= DCC_OFFER;
	Client->port = atoi(port);
	if (size)
		Client->filesize = atoi(size);
	if (!my_stricmp(type, "CHAT"))
		Client->flags |= DCC_CHAT;
	else if(!my_stricmp(type, "SEND"))
	{
		if (!current->filerecvallowed)
		{
			log_it(DCC_LOG_VAR, 0, 
				"DCC upload %s from %s rejected -- "
				"No receiving allowed", description, from);
			MyFree((char **)&Client);
			return;
		}
		Client->flags |= DCC_FILEREAD;
	}
	else 
		return;
	if (!search_list(description, from, Client->flags))
	{
		add_client(Client);
		do_dcc(Client);
	}
	else
		MyFree((char **)&Client);
#ifdef DBUG
	debug(NOTICE, "Exiting register_dcc_offer() correctly");
#endif
}

void process_incoming_chat(Client)
aDCC *Client;
{
	char            buf[HUGE];
	struct  sockaddr_in     remaddr;
	int             size;
	int             bytesread;      
	
	if (Client->flags&DCC_WAIT)
	{
		size = sizeof(struct sockaddr_in);
		Client->write = accept(Client->read, 
				(struct sockaddr *) &remaddr, &size);
		close(Client->read);
		Client->read = Client->write;
		Client->flags &= ~DCC_WAIT;
		Client->flags |= DCC_ACTIVE;
		Client->starttime = getthetime();
		Client->lasttime = getthetime();
		hasdcc_session(Client->user);
		return;   /* if there's data, we'll read it later */
	}
	switch ((bytesread = read_from_socket(Client->read, buf)))
	{
	case -1:
	case 0:         /* select said there is data! */
		if (Client->flags & DCC_ACTIVE)
		{
			Client->flags |= DCC_DELETE;
			return;         
		}
	default:
		terminate(buf, "\n");
		Client->lasttime = getthetime();
		Client->bytes_read += bytesread;
#ifdef DBUG
		debug(NOTICE, "=%s= %s", Client->user, buf);
#endif
		CurrentUser = find_user(&Userlist, Client->user, "*");
		CurrentShit = find_shit(&Userlist, Client->user, "*");
		strcpy(currentnick, getnick(Client->user));
		on_msg(Client->user, current->nick, buf);
		return;
	}
}       

void process_incoming_file(Client)
aDCC *Client;
{
	char            tmp[BIG_BUFFER];
	int             bytesread;
	int             bytestemp;

	switch((bytesread = read(Client->read, tmp, BIG_BUFFER)))
	{
	case -1:
	case 0:
		if (Client->flags & DCC_ACTIVE)
		{
			Client->flags |= DCC_DELETE;
			return;
		}
		break;
	default:
		write(Client->file, tmp, bytesread);
		Client->bytes_read += bytesread;
		/* To prevent ppl from sending huge files */
		if (!current->filerecvallowed)
			Client->flags |= DCC_DELETE;
		if (Client->bytes_read>(1000*maxuploadsize))
			Client->flags |= DCC_DELETE;
		bytestemp = htonl(Client->bytes_read);
		write(Client->write, &bytestemp, sizeof(int));
		Client->lasttime = getthetime();
		return;
	}
}

void process_outgoing_file(Client)
aDCC *Client;
{
	struct  sockaddr_in     remaddr;
	char    tmp[BIG_BUFFER+1];
	int     bytesrecvd;
	int     bytesread;
	int     size;

	if (Client->flags & DCC_WAIT)
	{
		size = sizeof(struct sockaddr_in);
		Client->write = accept(Client->read,
				(struct sockaddr *) &remaddr, &size);
		close(Client->read);
		Client->read = Client->write;
		Client->flags &= ~DCC_WAIT;
		Client->flags |= DCC_ACTIVE;
		Client->bytes_sent = 0L;
		Client->starttime = time(NULL);
		Client->lasttime = time(NULL);
		if ((Client->file = open(Client->description, O_RDONLY ))==-1)
		{
			log_it(DCC_LOG_VAR, 0,
			"Unable to open %s!", Client->description);
			Client->flags |= DCC_DELETE;
			return;
		}
		log_it(DCC_LOG_VAR, 0, 
			"DCC sending %s to %s", Client->description,
		       Client->user);

	}
	else
		if (read(Client->read, &bytesrecvd,
		sizeof(int)) < sizeof(int))
		{
			Client->flags |= DCC_DELETE;
			return;
		}
		else
			if (ntohl(bytesrecvd) != Client->bytes_sent)
				return;
	if ((bytesread = read(Client->file, tmp, BIG_BUFFER)) != 0)
	{
		write(Client->write, tmp, bytesread);
		Client->bytes_sent += bytesread;
		Client->lasttime = time(NULL);
	}
	else
		Client->flags |= DCC_DELETE;
}

void parse_dcc(read_fds)
fd_set *read_fds;
/*
 * Look at every client for in/output and check idletime 
 */
{
	register aDCC **Client;

	Client = &(current->Client_list);
	while (*Client)
	{
		if(((*Client)->read != -1) && FD_ISSET((*Client)->read,
		    read_fds))
		{
			/*
			 * Next line is VERY important. the same fd
			 * may get parsed twice (when a client is
			 * added within process_incoming_chat 
			 * (i.e. /msg =bot !send file). This way,
			 * it will get only parsed once a time
			 */
			FD_CLR((*Client)->read, read_fds);
			switch( (*Client)->flags&DCC_TYPES )
			{
			case DCC_CHAT:
				process_incoming_chat( *Client );
				break;
			case DCC_FILEOFFER:
				process_outgoing_file( *Client );
				break;
			case DCC_FILEREAD:
				process_incoming_file( *Client );
				break;
			default:
				break;
			}
		}
		if ( ((*Client)->flags & DCC_DELETE) ||

		   (!((*Client)->flags & DCC_CHAT) &&
			(getthetime()-(*Client)->lasttime) >= idletimeout) ||

		    (((*Client)->flags & DCC_WAIT) && 
			 ((getthetime()-(*Client)->lasttime) >= waittimeout)))
		{
			int     is_offer;
			char    user[MAXLEN];

			/* Ugly kludge, but it works :) */

			is_offer = (*Client)->flags&DCC_TYPES;
			strcpy(user, (*Client)->user);
			delete_client(*Client);
/*			if (is_offer)
				dcc_sendnext(user);
*/		}
		else
			Client = &((*Client)->next);
	}
} 

void process_dcc(from, rest)
char *from;
char *rest;
{
	char    *command;
	int     i;

	if ((command = get_token(&rest, " ")) == NULL)
		return;

	for (i = 0; dcc_commands[i].name != NULL; i++)
		if (!my_stricmp(dcc_commands[i].name, command))
		{
			dcc_commands[i].function(from, rest);
			return;
		}
}

void dcc_chat(from, rest)
char *from;
char *rest;
{
	aDCC *Client;

	if ((Client = (aDCC *)MyMalloc(sizeof(aDCC))) == NULL)
		return;
	strcpy(Client->user, from);
	strcpy(Client->description, "chat");
	strcpy(Client->ip_addr, "N/A");
	Client->read = Client->write = Client->file = -1;
	Client->flags = 0;
	Client->flags |= DCC_WAIT;
	Client->flags |= DCC_CHAT;
	Client->flags |= DCC_TWOCLIENTS;
	Client->starttime = getthetime();
	Client->lasttime = getthetime();
	Client->bytes_read = Client->bytes_sent = 0;
	Client->filesize = 0;
	if (!search_list( "chat", from, DCC_CHAT))
	{
		add_client(Client);
		do_dcc(Client);
	}
	else
		MyFree((char **)&Client);
}

int send_chat(to, text)
char *to;
char *text;
{
	int bytessend;
	aDCC *has_dcc;

	if (!(has_dcc = search_list("chat", to, DCC_CHAT)))
		return(FALSE);
 
	if ((has_dcc->flags&DCC_ACTIVE))
	{
		in_dcc = 1;
		switch(bytessend = send_to_socket(has_dcc->write, "%s",
		     text))
		{
		case -1:
#ifdef DBUG
			debug(ERROR, "DCC write failed!");
#endif
			delete_client(has_dcc);
			in_dcc = 0;
			return FALSE;
		case 0:
#ifdef DBUG
			debug(NOTICE, "DCC write = 0");
#endif
			break;
		default:
			has_dcc->bytes_sent += bytessend;
			has_dcc->lasttime = getthetime();
		}
		in_dcc = 0;
	}
	else
		return FALSE;
	return TRUE;
}

void dcc_sendfile(from, rest)
/* No checks are done (anymore). file should exist, be a file and be readable! */
char *from;
char *rest;
{
/*
	struct stat stat_buf;
	aDCC *Client;
	char *Descr;

	if (!current->filesendallowed)
		return;
	if (rest == NULL)
		return;

	if (stat(rest, &stat_buf))
		return;

	if ((Client = (aDCC *)MyMalloc(sizeof(aDCC))) == NULL)
		return;

	strcpy(Client->user, from);
	strcpy(Client->description, rest);
	strcpy(Client->ip_addr, "N/A");
	Client->read = Client->write = Client->file = -1;
	Client->flags = 0;
	Client->flags |= DCC_WAIT;
	Client->flags |= DCC_FILEOFFER;
	Client->flags |= DCC_TWOCLIENTS;
	Client->filesize = stat_buf.st_size;
	Client->starttime = getthetime();
	Client->lasttime = getthetime();
	Client->bytes_read = Client->bytes_sent = 0;
	if (!search_list(Client->description, from, DCC_FILEOFFER))
	{
		add_client(Client);
		do_dcc(Client);
	}
	else
		MyFree((char **)&Client);
*/
}








