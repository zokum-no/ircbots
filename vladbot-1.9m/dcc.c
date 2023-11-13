/*
 * dcc.c - an effort to implement dcc (at least chat) in VladBot
 *
 * (c) 1993 VladDrac (irvdwijk@cs.vu.nl)
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

#include "dcc.h"
#include "misc.h"
#include "config.h"

extern	char	current_nick[MAXLEN];

DCC_list	*client_list = NULL;

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
	"TALK",			/* won't need the rest, but ok. */
	"SUMMON",
	"RAW_LISTEN",
	"RAW",
	NULL
};	


void	set_dcc_fds( rds, wds )
fd_set	*rds;
fd_set 	*wds;

{
	DCC_list	*Client;

	for( Client = client_list; Client != NULL; Client = Client->next )
		if( Client->read != -1 )
			FD_SET(Client->read, rds);
}

DCC_list	*search_list( name, user, type, flag )
char		*name;
char		*user;
unsigned	type;
int		flag;

{
	DCC_list	*Client;

	for( Client = client_list; Client; Client = Client->next )
	{
		if( ( (Client->flags&DCC_TYPES) == type ) &&
		( !strcasecmp( name, Client->description ) ) &&
		( !strcasecmp( user, Client->user ) ) )
			return Client;
	}
	return( NULL );
}

int	do_dcc( Client )
DCC_list	*Client;

{
	struct	sockaddr_in	localaddr,
				remaddr;
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
	printf( "Entered do_dcc( .. )\n" );
#endif
	Type = dcc_types[Client->flags&DCC_TYPES];

	if( Client->flags & DCC_OFFER )
	{
		sscanf(Client->ip_addr, "%lu", &TempLong);
		party.s_addr = htonl(TempLong);
		if( ( Client->write = connect_by_number( Client->port,
					       inet_ntoa( party ) ) ) < 0 )
		{
			delete_client( Client );
			return;
		}
		Client->read = Client->write;
		Client->flags &= ~DCC_OFFER;
		Client->flags |= DCC_ACTIVE;
		size = sizeof( remaddr );
		getpeername( Client->read, (struct sockaddr *) &remaddr, 
			     &size );
		if( ( (Client->flags & DCC_TYPES) == DCC_FILEREAD ) &&  
                (Descr = rindex( Client->description, '/' ) ) )
                        Descr++;
                else                                 
                        Descr = Client->description;  
                if( (Client->flags & DCC_TYPES) == DCC_FILEREAD )     
                {                                               
                        strcpy( filebuf, FILEUPLOADDIR );         
                        strcat( filebuf, Descr );              
                        if((Client->file = open( filebuf, O_WRONLY |
                        O_TRUNC | O_CREAT, 0644 ))==-1)
			{
				botlog( DCCLOGFILE, 
					"DCC upload: Unable to open %s", Descr );
				delete_client( Client );
				return( FAIL );
			}                  
			botlog( DCCLOGFILE, "DCC upload %s from %s", Descr,
				Client->user );
                }
		Client->starttime = time(NULL);
		Client->lasttime = time(NULL);
		Client->bytes_read = Client->bytes_sent = 0;
		return 1;
	}
	else
	{
		Client->flags |= DCC_WAIT;
		if( ( Client->read = connect_by_number( 0, "" ) ) < 0 )
		{	
			delete_client( Client );
			return( FAIL );
		}
		size = sizeof( struct sockaddr_in );
		getsockname( Client->read, (struct sockaddr *)&localaddr, 
			     &size );
		gethostname(localhost, 64);
		if(hp=gethostbyname(localhost))
			bcopy(hp->h_addr, (char*) &MyHostAddr, sizeof(MyHostAddr));		
		my_ip = (unsigned long)ntohl( MyHostAddr.s_addr );
		if( Client->flags & DCC_TWOCLIENTS )
		{
			if( ( Client->flags & DCC_FILEOFFER ) &&
    		          ( Descr = rindex( Client->description, '/' ) ) )
				Descr++;
			else
				Descr = Client->description;
			if( Client->filesize )
				send_ctcp( getnick( Client->user ),
					   "DCC %s %s %lu %u %d", Type, Descr,
					   (unsigned long) my_ip,
					   (unsigned)ntohs(localaddr.sin_port),
					   Client->filesize );
			else
				send_ctcp( getnick( Client->user ),
					   "DCC %s %s %lu %u", Type, Descr,
					   (unsigned long) my_ip,
					   (unsigned)ntohs(localaddr.sin_port));
		}
		Client->starttime = time(NULL);
		Client->lasttime = time(NULL);
		return 2;
	}
}					


char    *reply_dcc(from, to, rest )
char    *from,
        *to,
        *rest;
{
        char    *type;
        char    *description;
        char    *inetaddr;
        char    *port;
        char    *size;

	if( ischannel( to ) )
                return null(char *);

        if ( !( type = strtok( rest, " " ) ) ||
                        !( description = strtok( NULL, " " ) ) ||
                        !( inetaddr = strtok( NULL, " " ) ) ||
                        !( port = strtok( NULL, " " )))
                return NULL;
        size = strtok( NULL, " " );
        register_dcc_offer(from, type, description, inetaddr, port, size);
}


void	show_dcclist( from )
char	*from;

{
	DCC_list        *Client;
	unsigned	flags;

	if( client_list == NULL )
	{
		send_to_user( from, "No connections made" );
		return;
	}

        for( Client = client_list; Client != NULL; Client = Client->next )
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
		send_to_user( from, "Idle: %s", idle2str(time(NULL)-Client->lasttime));
	}
}

int	add_client( Client )
DCC_list	*Client;

{
	Client->next = client_list;
	client_list = Client;
}

int	delete_client( Client )
DCC_list        *Client;
 
{
	DCC_list	**lame;

	for( lame = &client_list; *lame; lame = &((*lame)->next) )
		if( *lame == Client )
		{
		/* Can we use send_to_user here? It might be a send/get... */
			sendnotice( getnick( Client->user ), 
				    "Closing DCC %s-connection",
				    dcc_types[Client->flags&DCC_TYPES] );
	        	close( Client->read );
			close( Client->write );
			if( Client->file != -1) 
				close( Client->file );
			*lame = Client->next;
			free( Client );	
			return( TRUE );
		}
	return( FALSE );
}			

void	register_dcc_offer(from, type, description, inetaddr, port, size)
char	*from;
char	*type;
char	*description;
char	*inetaddr;
char	*port;
char	*size;

{
	struct	in_addr	shitty, remaddr;
	int	tmp_sock;
	int	rl;
	DCC_list	*Client;

#ifdef DBUG
        printf( "ENTERED register_dcc_offer( %s, %s, %s, %s, %s, %s );\n",
                from, type, description, inetaddr, port, size );
#endif
	if( ( Client = (DCC_list *)malloc( sizeof( *Client ))) == NULL )
		return;
	strcpy( Client->user, from );
	strcpy( Client->description, description );
	strcpy( Client->ip_addr, inetaddr );
	Client->read = Client->read = Client->file = -1;
	Client->flags = 0;
	Client->flags |= DCC_OFFER;
	Client->port = atoi( port );
	if( size )
		Client->filesize = atoi( size );
	if( !strcasecmp( type, "CHAT" ) )
		Client->flags |= DCC_CHAT;
	else if( !strcasecmp( type, "SEND" ) )
		Client->flags |= DCC_FILEREAD;
	else 
		return;
	if( !search_list( description, from, Client->flags, 1 ) )
	{
		add_client( Client );
		do_dcc( Client );
	}
	else
		free( Client );
#ifdef DBUG
	printf( "Exiting register_dcc_offer() correctly\n" );
#endif
}

void	process_incoming_chat( Client )
DCC_list	*Client;

{
	char		buf[MAXLEN];
	struct  sockaddr_in     remaddr;
	int		size;
	int		bytesread;	
	
	if( Client->flags&DCC_WAIT ) 
	{
		size = sizeof( struct sockaddr_in );
		Client->write = accept( Client->read, 
				(struct sockaddr *) &remaddr, &size );
		close( Client->read );
		Client->read = Client->write;
		Client->flags &= ~DCC_WAIT;
		Client->flags |= DCC_ACTIVE;
		Client->starttime = time(NULL);
		Client->lasttime = time(NULL);
		return;	  /* if there's data, we'll read it later */
	}
	switch( ( bytesread = read_from_socket( Client->read, buf ) ) )
	{
	case -1:
	case 0:		/* select said there is data! */
		if( Client->flags&DCC_ACTIVE )
		{
			Client->flags |= DCC_DELETE;
			return;		
		}
	default:
		if( *( buf + strlen( buf ) - 1 ) == '\n' )
			*( buf + strlen( buf ) - 1 ) = '\0';
		Client->lasttime = time(NULL);
		Client->bytes_read += bytesread;
		on_msg( Client->user, current_nick, buf );	
		return;
	}
}	

void    process_incoming_file( Client )
DCC_list        *Client;

{
	char		tmp[BIG_BUFFER];
	int		bytesread;
        struct  sockaddr_in     remaddr;
        int             size;
	int		bytestemp;

        switch( ( bytesread = read( Client->read, tmp, BIG_BUFFER ) ) )
        {
        case -1:
        case 0:
                if( Client->flags&DCC_ACTIVE )
                {
                        Client->flags |= DCC_DELETE;
                        return;
                }
		break;
        default:
		write( Client->file, tmp, bytesread );
		Client->bytes_read += bytesread;
		/* To prevent ppl from sending huge files */
		if(Client->bytes_read>DCC_MAXFILESIZE)
			Client->flags |= DCC_DELETE;
		bytestemp = htonl(Client->bytes_read);
		write(Client->write, &bytestemp, sizeof(int));
		Client->lasttime = time(NULL);
                return;
        }
}

void	process_outgoing_file( Client )
DCC_list	*Client;

{
	struct	sockaddr_in	remaddr;
	char	tmp[BIG_BUFFER+1];
	int	bytesrecvd;
	int	bytesread;
	int	size;

	if( Client->flags & DCC_WAIT )
	{
		size = sizeof( struct sockaddr_in );
		Client->write = accept( Client->read,
				(struct sockaddr *) &remaddr, &size );
		close( Client->read );
		Client->read = Client->write;
		Client->flags &= ~DCC_WAIT;
		Client->flags |= DCC_ACTIVE;
		Client->bytes_sent = 0L;
		Client->starttime = time(NULL);
		Client->lasttime = time(NULL);
		if((Client->file = open( Client->description, O_RDONLY ))==-1)
		{
			botlog( DCCLOGFILE, "Unable to open %s!", Client->description );
			Client->flags |= DCC_DELETE;
			return;
		}
	        botlog( DCCLOGFILE, "DCC sending %s to %s", Client->description,
			Client->user );

	}
	else
		if( read( Client->read, &bytesrecvd, 
		sizeof( int ) ) < sizeof( int ) )
		{
			Client->flags |= DCC_DELETE;
			return;
		}
		else
			if( ntohl(bytesrecvd) != Client->bytes_sent)
				return;
	if( bytesread = read( Client->file, tmp, BIG_BUFFER ) )
	{
		write( Client->write, tmp, bytesread );
		Client->bytes_sent += bytesread;
		Client->lasttime = time(NULL);
	}
	else
		Client->flags |= DCC_DELETE;
}

void	parse_dcc_input( read_fds )
fd_set	*read_fds;
/*
 * Look at every client for in/output and check idletime 
 */
{
        DCC_list        **Client;


	Client = &client_list;	
	while( *Client )
	{
		if( ((*Client)->read != -1) && FD_ISSET((*Client)->read,
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
		    ((time(NULL)-(*Client)->lasttime) >= DCC_IDLETIMEOUT) ||
 		    (((*Client)->flags & DCC_WAIT) && 
		    ((time(NULL)-(*Client)->lasttime) >= DCC_WAITTIMEOUT))) 
			delete_client( *Client );
		else
			Client = &((*Client)->next);
	}
} 

void	process_dcc( from, rest )
char	*from;
char	*rest;

{
	char	*command;
	char	*dcc_rest;
	int	i;

	dcc_rest = strchr( rest, ' ' ) + 1;

	if( ( command = strtok( rest, " " ) ) == NULL )
		return;

	for( i = 0; dcc_commands[i].name != NULL; i++ )
		if( !strcasecmp( dcc_commands[i].name, command ) )
		{
			dcc_commands[i].function( from, dcc_rest );
			return;
		}
}

void	dcc_chat( from, rest )
char 	*from;
char 	*rest;

{
	DCC_list	*Client;

	if( (Client = (DCC_list *)malloc( sizeof( *Client ) ) ) == NULL )
		return;
	strcpy( Client->user, from );
	strcpy( Client->description, "chat" );
	strcpy( Client->ip_addr, "N/A" );
	Client->read = Client->write = Client->file = -1;
	Client->flags = 0;
	Client->flags |= DCC_WAIT;
	Client->flags |= DCC_CHAT;
	Client->flags |= DCC_TWOCLIENTS;
	Client->starttime = time(NULL);
	Client->lasttime = time(NULL);
	Client->bytes_read = Client->bytes_sent = 0;
	Client->filesize = 0;
	if( !search_list( "chat", from, DCC_CHAT, 1 ) )
	{
		add_client( Client );
		do_dcc( Client );
	}
	else
		free( Client );
}

int	send_chat( to, text )
char	*to;
char	*text;

{
	int		bytessend;
	DCC_list        *has_dcc;
 
	if(!(has_dcc = search_list( "chat", to, DCC_CHAT, 1 )))
		return(FALSE);
 
	if((has_dcc->flags&DCC_ACTIVE))
	{
		switch(bytessend = send_to_socket( has_dcc->write, "%s",
		     text))
		{
		case -1:
#ifdef DBUG
			printf("DCC write failed!\n");
#endif
			delete_client( has_dcc );
			return(FALSE);
		case 0:
#ifdef DBUG
			printf("DCC write = 0\n");
#endif
			break;
		default:
			has_dcc->bytes_sent += bytessend;
			has_dcc->lasttime = time(NULL);
		}
	}
        else
		return(FALSE);
}


void	dcc_sendfile( from, rest )
char	*from;
char	*rest;

{
	struct	stat	stat_buf;
	DCC_list        *Client;
	char		*file_to_send;
	char		filebuf[MAXLEN];
	char		*Descr;

	if( ( file_to_send = strtok( rest, " \n\0" ) ) == NULL )
		return;

	botlog( DCCLOGFILE, "DCC get from %s: %s received", 
                from, file_to_send );

/* do some checking on the filename */
/* our file should be located in FILEROOTDIR.
   leading / should be deleted */

	if( *file_to_send == '/' )
		file_to_send++;
	strcpy( filebuf, FILEROOTDIR );
	strcat( filebuf, file_to_send );

	if( strstr( filebuf, ".." ) )
	{
		send_to_user( from, "\'..\' are not allowed in filenames!" );
		return;
	}
	if( stat( filebuf, &stat_buf ) )
	{
		send_to_user( from, "Could not stat %s!", filebuf );
		return;			/* FAILURE! */
	}
	if( access( filebuf, R_OK ) )
	{
		send_to_user( from, "No readpermission on %s!", filebuf );
		return;			/* Not permitted to read */
	}
	if( stat_buf.st_mode & S_IFDIR )
	{
		send_to_user( from, "Sorry, %s is a directory!", filebuf );
		return;			/* Directory */
	}

	if( (Client = (DCC_list *)malloc( sizeof( *Client ) ) ) == NULL )
                return;

        strcpy( Client->user, from );
	strcpy( Client->description, filebuf );
	strcpy( Client->ip_addr, "N/A" );
	Client->read = Client->write = Client->file = -1;
        Client->flags = 0;
	Client->flags |= DCC_WAIT;
        Client->flags |= DCC_FILEOFFER;
        Client->flags |= DCC_TWOCLIENTS;
	Client->filesize = stat_buf.st_size;
        Client->starttime = time(NULL);
	Client->lasttime = time(NULL);
	Client->bytes_read = Client->bytes_sent = 0;
        if( !search_list( Client->description, from, DCC_FILEOFFER, 1 ) )
	{
		add_client( Client );
                do_dcc( Client );
		Descr = strrchr( Client->description, '/' ) ?
			strrchr( Client->description, '/' ) + 1 :
			Client->description;
		send_to_user( from, "Now please type: /dcc get %s %s",
                            current_nick, Descr);
        }
	else
		free( Client );
}

