/*
 * dcc.c - an effort to implement dcc (at least chat) in VladBot
 * (c) 1993 VladDrac (irvdwijk@cs.vu.nl)
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

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <strings.h>
#include <string.h>

#include "config.h"
#include "dcc.h"
#include "debug.h"
#include "log.h"
#include "misc.h"
#include "send.h"
#include "server.h"
#include "session.h"
#include "vladbot.h"
#ifdef NEXT
int	gethostname(char *, int);
int	bcopy(char *, char *, int);
#endif /* NEXT */

extern	void	on_msg(char *, char *, char *);

extern	botinfo	*currentbot;
extern  int     idletimeout;
extern  int     waittimeout;
extern  int     maxuploadsize;


struct 
{
	char	*name;
	void	(*function)();
} dcc_commands[] =
{
	{ "CHAT",	dcc_chat },
	{ "SEND",	dcc_sendfile },
	{ NULL,		null(void (*)) }
};

char *dcc_types[] =
{
	"<null>",
	"CHAT",
	"SEND",
	"GET",
	NULL
};	


void	dccset_fds(fd_set *rds, fd_set *wds)

{
	DCC_list	*Client;

	for(Client = currentbot->Client_list; Client != NULL; 
            Client = Client->next)
		if(Client->read != -1)
			FD_SET(Client->read, rds);
}

DCC_list	*search_list(char *name, char *user, unsigned int type)
{
	DCC_list	*Client;

	for(Client = currentbot->Client_list; Client; Client = Client->next)
	{
		if(((Client->flags&DCC_TYPES) == type) &&
		(STRCASEEQUAL( name, Client->description)) &&
		(STRCASEEQUAL( user, Client->user)))
			return Client;
	}
	return NULL;
}

int	do_dcc(DCC_list *Client)

{
	struct	sockaddr_in	localaddr;
	struct	in_addr		party,
				MyHostAddr;
	struct	hostent		*hp;
	char			localhost[64];
	int			size;
	unsigned long		my_ip;
	unsigned long		TempLong;
	char			*Descr;
	char			*Type;
	char			filebuf[MAXLEN];

#ifdef DBUG
	debug(NOTICE, "Entered do_dcc( .. )");
#endif
	Type = dcc_types[Client->flags&DCC_TYPES];

	if(Client->flags & DCC_OFFER)
	{
		sscanf(Client->ip_addr, "%lu", &TempLong);
		party.s_addr = htonl(TempLong);
		if((Client->write = connect_by_number(Client->port,
		    inet_ntoa( party))) < 0)
		{
			delete_client(Client);
			return FALSE;
		}
		Client->read = Client->write;
		Client->flags &= ~DCC_OFFER;
		Client->flags |= DCC_ACTIVE;
		if(((Client->flags & DCC_TYPES) == DCC_FILEREAD) &&  
                (Descr = strrchr( Client->description, '/')))
                        Descr++;
                else                                 
                        Descr = Client->description;  
                if((Client->flags & DCC_TYPES) == DCC_FILEREAD)     
                {                                               
			sprintf(filebuf, "%s/%s", currentbot->uploaddir,
				Descr);
                        if((Client->file = open( filebuf, O_WRONLY |
                        O_TRUNC | O_CREAT, 0644 ))==-1)
			{
				botlog(DCCLOGFILE, 
				"DCC upload: Unable to open %s", Descr);
				delete_client(Client);
				return FAIL;
			}                  
			botlog(DCCLOGFILE, "DCC upload %s from %s", Descr,
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
		if((hp=gethostbyname(localhost)))
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
			if(Client->filesize)
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


void	reply_dcc(char *from, char *to, char *rest)
{
        char    *type;
        char    *description;
        char    *inetaddr;
        char    *port;
        char    *size;

	if(ischannel(to))
                return;

        if(!(type = get_token(&rest, " ")) ||
           !(description = get_token(&rest, " ")) ||
           !(inetaddr = get_token(&rest, " ")) ||
           !(port = get_token(&rest, " ")))
                return;
        size = get_token(&rest, " ");
        register_dcc_offer(from, type, description, inetaddr, port, size);
}


void	show_dcclist(char *from)

{
	DCC_list        *Client;
	unsigned	flags;

	if(currentbot->Client_list == NULL)
	{
		send_to_user(from, "No connections made");
		return;
	}

        for(Client = currentbot->Client_list; Client != NULL; 
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
		      idle2str(time(NULL)-Client->lasttime));
	}
}

void	add_client(DCC_list *Client)

{
	Client->next = currentbot->Client_list;
	currentbot->Client_list = Client;
}

int	delete_client(DCC_list *Client)
 
{
	DCC_list	**lame;

	for(lame = &(currentbot->Client_list); *lame; lame = &((*lame)->next))
		if(*lame == Client)
		{
		/* Can we use send_to_user here? It might be a send/get... */
			sendnotice(getnick(Client->user), 
				   "Closing DCC %s[%s]-connection",
				   dcc_types[Client->flags&DCC_TYPES],
				   strrchr(Client->description, '/') ?
				   strrchr(Client->description, '/')+1:
				   Client->description);
	        	close(Client->read);
			close(Client->write);
			if(Client->file != -1) 
				close(Client->file);
			*lame = Client->next;
			free(Client);	
			return TRUE;
		}
	return FALSE;
}			

void	close_all_dcc()
{
	DCC_list	*Client;

	for(;(Client = currentbot->Client_list);)
		delete_client(Client);
}

void	register_dcc_offer(char *from, char *type, char *description, 
                           char *inetaddr, char *port, char *size)

{
	DCC_list	*Client;

#ifdef DBUG
        debug(NOTICE, "Entered register_dcc_offer( %s, %s, %s, %s, %s, %s );",
                from, type, description, inetaddr, port, size );
#endif
	if((Client = (DCC_list *)malloc(sizeof(*Client))) == NULL)
		return;
	strcpy(Client->user, from);
	strcpy(Client->description, description);
	strcpy(Client->ip_addr, inetaddr);
	Client->read = Client->read = Client->file = -1;
	Client->flags = 0;
	Client->flags |= DCC_OFFER;
	Client->port = atoi(port);
	if(size)
		Client->filesize = atoi(size);
	if(STRCASEEQUAL(type, "CHAT"))
		Client->flags |= DCC_CHAT;
	else if(STRCASEEQUAL(type, "SEND"))
		Client->flags |= DCC_FILEREAD;
	else 
		return;
	if(!search_list(description, from, Client->flags))
	{
		add_client(Client);
		do_dcc(Client);
	}
	else
		free(Client);
#ifdef DBUG
	debug(NOTICE, "Exiting register_dcc_offer() correctly");
#endif
}

void	process_incoming_chat(DCC_list *Client)

{
	char		buf[MAXLEN];
	struct  sockaddr_in     remaddr;
	int		size;
	int		bytesread;	
	
	if(Client->flags&DCC_WAIT) 
	{
		size = sizeof(struct sockaddr_in);
		Client->write = accept(Client->read, 
				(struct sockaddr *) &remaddr, &size);
		close(Client->read);
		Client->read = Client->write;
		Client->flags &= ~DCC_WAIT;
		Client->flags |= DCC_ACTIVE;
		Client->starttime = time(NULL);
		Client->lasttime = time(NULL);
		hasdcc_session(Client->user);
		return;	  /* if there's data, we'll read it later */
	}
	switch((bytesread = read_from_socket(Client->read, buf)))
	{
	case -1:
	case 0:		/* select said there is data! */
		if( Client->flags&DCC_ACTIVE )
		{
			Client->flags |= DCC_DELETE;
			return;		
		}
	default:
		KILLNEWLINE(buf);
		Client->lasttime = time(NULL);
		Client->bytes_read += bytesread;
#ifdef DBUG
		debug(NOTICE, "=%s= %s", Client->user, buf);
#endif
		on_msg(Client->user, currentbot->nick, buf);	
		return;
	}
}	

void    process_incoming_file(DCC_list *Client)

{
	char		tmp[BIG_BUFFER];
	int		bytesread;
	int		bytestemp;

        switch((bytesread = read(Client->read, tmp, BIG_BUFFER)))
        {
        case -1:
        case 0:
                if(Client->flags&DCC_ACTIVE)
                {
                        Client->flags |= DCC_DELETE;
                        return;
                }
		break;
        default:
		write(Client->file, tmp, bytesread);
		Client->bytes_read += bytesread;
		/* To prevent ppl from sending huge files */
		if(Client->bytes_read>(1000*maxuploadsize))
			Client->flags |= DCC_DELETE;
		bytestemp = htonl(Client->bytes_read);
		write(Client->write, (char *)&bytestemp, sizeof(int));
		Client->lasttime = time(NULL);
                return;
        }
}

void	process_outgoing_file(DCC_list *Client)

{
	struct	sockaddr_in	remaddr;
	char	tmp[BIG_BUFFER+1];
	int	bytesrecvd;
	int	bytesread;
	int	size;

	if(Client->flags & DCC_WAIT)
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
		if((Client->file = open(Client->description, O_RDONLY ))==-1)
		{
			botlog(DCCLOGFILE, 
			"Unable to open %s!", Client->description);
			Client->flags |= DCC_DELETE;
			return;
		}
	        botlog(DCCLOGFILE, "DCC sending %s to %s", Client->description,
		       Client->user);

	}
	else
		if(read(Client->read, (char *)&bytesrecvd, 
		sizeof(int)) < sizeof(int))
		{
			Client->flags |= DCC_DELETE;
			return;
		}
		else
			if(ntohl(bytesrecvd) != Client->bytes_sent)
				return;
	if((bytesread = read(Client->file, tmp, BIG_BUFFER)))
	{
		write(Client->write, tmp, bytesread);
		Client->bytes_sent += bytesread;
		Client->lasttime = time(NULL);
	}
	else
		Client->flags |= DCC_DELETE;
}

void	parse_dcc(fd_set *read_fds)
/*
 * Look at every client for in/output and check idletime 
 */
{
        DCC_list        **Client;

	Client = &(currentbot->Client_list);
	while(*Client)
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
		if( ((*Client)->flags & DCC_DELETE) ||
		    ((time(NULL)-(*Client)->lasttime) >= idletimeout) ||
 		    (((*Client)->flags & DCC_WAIT) && 
		    ((time(NULL)-(*Client)->lasttime) >= waittimeout))) 
		{
			int	is_offer;
			char	user[MAXLEN];

			/* Ugly kludge, but it works :) */

			is_offer = (*Client)->flags&DCC_TYPES;
			strcpy(user, (*Client)->user);
			delete_client(*Client);
			if(is_offer)
				dcc_sendnext(user);
		}
		else
			Client = &((*Client)->next);
	}
} 

void	process_dcc(char *from, char *rest)

{
	char	*command;
	int	i;

	if((command = get_token(&rest, " ")) == NULL)
		return;

	for(i = 0; dcc_commands[i].name != NULL; i++)
		if(STRCASEEQUAL(dcc_commands[i].name, command))
		{
			dcc_commands[i].function(from, rest);
			return;
		}
}

void	dcc_chat(char *from, char *rest)

{
	DCC_list	*Client;

	if((Client = (DCC_list *)malloc(sizeof(*Client))) == NULL)
		return;
	strcpy(Client->user, from);
	strcpy(Client->description, "chat");
	strcpy(Client->ip_addr, "N/A");
	Client->read = Client->write = Client->file = -1;
	Client->flags = 0;
	Client->flags |= DCC_WAIT;
	Client->flags |= DCC_CHAT;
	Client->flags |= DCC_TWOCLIENTS;
	Client->starttime = time(NULL);
	Client->lasttime = time(NULL);
	Client->bytes_read = Client->bytes_sent = 0;
	Client->filesize = 0;
	if(!search_list( "chat", from, DCC_CHAT))
	{
		add_client(Client);
		do_dcc(Client);
	}
	else
		free(Client);
}

int	send_chat(char *to, char *text)

{
	int		bytessend;
	DCC_list        *has_dcc;
 
	if(!(has_dcc = search_list("chat", to, DCC_CHAT)))
		return(FALSE);
 
	if((has_dcc->flags&DCC_ACTIVE))
	{
		switch(bytessend = send_to_socket(has_dcc->write, "%s",
		     text))
		{
		case -1:
#ifdef DBUG
			debug(ERROR, "DCC write failed!");
#endif
			delete_client(has_dcc);
			return(FALSE);
		case 0:
#ifdef DBUG
			debug(NOTICE, "DCC write = 0");
#endif
			break;
		default:
			has_dcc->bytes_sent += bytessend;
			has_dcc->lasttime = time(NULL);
		}
		return TRUE;
	}
	return FALSE ;
}


void	dcc_sendfile(char *from, char *rest)
/* No checks are done (anymore). file should exist, be a file and be readable! */
{
	struct	stat	stat_buf;
	DCC_list        *Client;

	if(rest == NULL)
		return;

	if(stat(rest, &stat_buf))
		return;			/* FAILURE! */

	if((Client = (DCC_list *)malloc(sizeof(DCC_list))) == NULL)
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
        Client->starttime = time(NULL);
	Client->lasttime = time(NULL);
	Client->bytes_read = Client->bytes_sent = 0;
        if(!search_list(Client->description, from, DCC_FILEOFFER))
	{
		add_client(Client);
                do_dcc(Client);
        }
	else
		free( Client );
}
