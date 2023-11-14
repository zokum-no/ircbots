#ifndef _DCC_H_
#define _DCC_H_

#ifdef MSDOS
	#define fd_set char *
#endif

#ifdef AIX
#       include <sys/select.h>
#endif /* AIX */
#include <sys/types.h>
#include "config.h"

#define DCC_CHAT        ((unsigned) 0x0001)
#define DCC_FILEOFFER   ((unsigned) 0x0002)
#define DCC_FILEREAD    ((unsigned) 0x0003)
#define DCC_TALK        ((unsigned) 0x0004)
#define DCC_SUMMON      ((unsigned) 0x0005)
#define DCC_RAW_LISTEN  ((unsigned) 0x0006)
#define DCC_RAW         ((unsigned) 0x0007)
#define DCC_TYPES       ((unsigned) 0x000f)

#define DCC_WAIT        ((unsigned) 0x0010)
#define DCC_ACTIVE      ((unsigned) 0x0020)
#define DCC_OFFER       ((unsigned) 0x0040)
#define DCC_DELETE      ((unsigned) 0x0080)
#define DCC_TWOCLIENTS  ((unsigned) 0x0100)
#define DCC_STATES      ((unsigned) 0xfff0)
 

typedef struct DCC_struct
{
	unsigned        flags;
	int     read;
	int     write;
	int     file;
	int     filesize;               
	char    description[MAXLEN];    
	char    user[MAXLEN];
	char    ip_addr[20];            /* remote in ircII */
	int     port;                   /* remport */
	long    bytes_read;
	long    bytes_sent;
	long    lasttime;               /* can be used for idletime */
	long    starttime;
	struct  DCC_struct      *next;

} DCC_list;

DCC_list        *search_list(char *name, char *user, unsigned int type);
int             do_dcc( DCC_list *Client );
char            *reply_dcc( char *from, char *to, char *rest );
void            show_dcclist( char *from );
char            *dcc_time( long time );
int             add_client( DCC_list *Client );
int             delete_client( DCC_list *Client );
void            close_all_dcc();
void            register_dcc_offer( char *from, char *type, char *description, 
		char *inetaddr, char *port, char *size );
void            parse_dcc( fd_set *read_fds );
void            process_dcc( char *from, char *rest );
void            dcc_chat( char *from, char *rest );
int             send_chat( char *to, char *text );
void            dcc_sendfile( char *from, char *rest );

#endif /* DCC_H */





