/*
 * function.c - misc functions that were needed
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/time.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "global.h"
#include "config.h"
#include "defines.h"
#include "structs.h"
#include "h.h"

#define getrandom(min, max) ((rand() % (int)(((max)+1) - (min))) + (min))

static  char    usernick[MAXLEN];
static  char    normuserhost[MAXLEN];
static  char    timebuf[MAXLEN];
static  char    idlestr[MAXLEN];

static char *months[] =
{
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static char *days[] =
{
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

#ifdef NEED_STRSTR
char *strstr(s1, s2)
char *s1;
char *s2;
{
	register char *ptr = s1;
	int n = strlen(s2);

	while (*ptr)
	{
		if (!strncmp(ptr, s2, n))
			return ptr;
		ptr++;
	}
	return NULL;
}
#endif /* NEED_STRSTR */

char *inetntoa(in)
char *in;
{
        static char charbuf[16];
        register unsigned char  *s = (u_char *)in;
        register int a,b,c,d;

        a = (int)*s++;
        b = (int)*s++;
        c = (int)*s++;
        d = (int)*s++;
        sprintf(charbuf, "%d.%d.%d.%d", a,b,c,d );
        return charbuf;
}

char *formatgreet(stuff, insert)
char *stuff;
char *insert;
{
	static char stuffbuf[512];
	char keep[255];
	char *temp = stuffbuf;
	char *temp2 = keep;
	char *temp3 = keep;
	if (!stuff || !insert)
		return NULL;
	strcpy(temp, "");
	strcpy(keep, stuff);
	while(temp3 && *temp3)	
	{
		temp2 = strchr(temp3, '%');
		if (!temp2 || (*(temp2+1) != 'n'))
		{
			strcat(temp, temp3);
			temp3 = NULL;
		}
		else
		{
			*temp2 = '\0';
			strcat(temp, temp3);
			strcat(temp, insert);
			temp3 = temp2+2;
		}
	}
	return temp;
}

char *getmachinename()
{
        static char buf1[255];
#ifdef HAVE_GETDOMAINNAME
        char buf2[81];
#endif

        gethostname(buf1, 80);
#ifdef HAVE_GETDOMAINNAME
        getdomainname(buf2, 80);
        if (strstr(buf1, buf2) == NULL)
        {
                strcat(buf1, ".");
                strcat(buf1, buf2);
        }
#endif
        return buf1;
}

void update_auths()
{
	register aTime *cptr, *tmp;
	time_t now;

	now = getthetime();
	cptr = Auths;
	while (cptr)
	{
		tmp = cptr->next;
		if (now-cptr->time > AUTH_TIMEOUT)
		{
			if (is_present(getnick(cptr->name)))
				cptr->time = now;
			else
			{
#ifdef USE_CCC
				send_to_ccc("1 DAUTH %s %li", cptr->name,
						cptr->time);
#endif
				remove_time(&Auths, cptr);
			}
		}
		cptr = tmp;
	}
}

#ifdef USE_CCC

void ccc_connected()
{
	register aTime *tmp;
	int i;
 
	send_to_ccc("1 PASSWD %li %s!%s@%s %s!%s@%s",
		spec(mylocalhost), current->nick, current->login,
			mylocalhostname, current->nick, current->login,
			mylocalhost);
	for(tmp=Auths;tmp;tmp=tmp->next)
		send_to_ccc("1 AUTH %s %li", tmp->name, tmp->time);	
	for(i=0;i<MAXBOTS;i++)
		if (thebots[i].created)
			if (&(thebots[i]) != current)
			{
				send_to_ccc("1 BOT %s!%s@%s",
					thebots[i].nick,
					thebots[i].login,
					mylocalhostname);
				send_to_ccc("1 BOT %s!%s@%s",
					thebots[i].nick,
					thebots[i].login,
					mylocalhost);
			}
}

long int spec(string)
char *string;
{
        register int whee = 0;

        while (*string)
        {
                whee += (int) *string;
                string++;
        }
        return ((long)whee*93392)+20;
}

#endif /* USE_CCC */

#ifndef DOUGH_MALLOC

void *MyMalloc(HowBig)
int HowBig;
{
	void *tmp;

	if ((tmp = (void *) malloc(HowBig)) == NULL)
		MyExit(0); /* Out of memory */
	bzero(tmp, HowBig);
	return (void *) tmp;
}

void MyFree(FreeMe)
char **FreeMe;
{
	if ((FreeMe != NULL) && (*FreeMe != NULL))
	{
		free(*FreeMe);
		*FreeMe = NULL;
	}
}

#endif

char *host2ip(host)
char *host;
{
	struct hostent *temp=NULL;
	long temp1=0;

	temp=gethostbyname(host);
	if (temp)
		bcopy(temp->h_addr, (caddr_t)&temp1, temp->h_length);
	else
		return "";
	return inetntoa((char *)&temp1);
} 

void MyExit(val)
int val;
{
	delete_time(&Auths);
	delete_all_userlist(&(current->Userlist));
	free_strvars(DefaultVars);
	exit(val);
}

void free_links(thelink)
aLink **thelink;
{
	register aLink *hold;
	aLink *temp;
	
	temp = *thelink;
	while (temp)
	{
		hold = temp->next;
		if (temp->name)
			MyFree(&temp->name);
		if (temp->link)
			MyFree(&temp->link);
		MyFree((char **)&temp);
		temp = hold;
	}
	*thelink = (aLink *) 0;
}

void add_link(thelinks, serv1, serv2)
aLink **thelinks;
char *serv1;
char *serv2;
{
	aLink *serv;
  
	serv=(aLink *) MyMalloc(sizeof(aLink));
	serv->status = 0;
	serv->num = 0;
	serv->next=*thelinks;
	mstrcpy(&serv->name, serv1);
	mstrcpy(&serv->link, serv2);
	*thelinks = serv; 
}

int find_link(thelinks, serv1)
aLink *thelinks;
char *serv1;
{
	register aLink *temp;

	for (temp=thelinks;temp;temp=temp->next)
		if (!my_stricmp(temp->name, serv1))
			return TRUE;
	return FALSE;
}

char *random_str(min, max)
int min;
int max;
{
	 int i, ii;
	 
	 srand( (unsigned)time( NULL ) ); 
	 i = RANDOM(min, max);
	 for (ii=0;ii<i;ii++)
		 normuserhost[ii] = (char) RANDOM(97, 122);
	 normuserhost[ii]='\0';
	 return normuserhost;
}

time_t getthetime(void)
{
	time_t t;

	t = time( ( time_t *) NULL);
	return t;
}

char *stripl(string, chars)
char *string;
char *chars;
{
	if (!string || !chars)
	 return "";
	for (;*string && strchr(chars, *string); string++);
	return string;
}

char *fixusername(string)
char *string;
{
	if (string)
	{
		if (*string == '~')
			string++;
		else if (*string == '#')
			string++;
	}
	return string;
}

char *terminate(string, chars)  /* terminates a string if a char is in it */
char *string;
char *chars;
{
	char *ptr;

	if (!string || !chars)
	 return "";
	while (*chars)
	{
	if ((ptr = strrchr(string, *chars)) != NULL)
		*ptr = '\0';
	chars++;
	}
	return string;
}

char *get_token(src, token_sep)
char **src;
char *token_sep;
{
	char    *tok;

	if (!(src && *src && **src))
		return NULL;

	while(**src && strchr(token_sep, **src))
		(*src)++;

	if(**src)
		tok = *src;
	else
		return NULL;

	*src = strpbrk(*src, token_sep);
	if (*src)
	{
		**src = '\0';
		(*src)++;
		while(**src && strchr(token_sep, **src))
			(*src)++;
	}
	else
		*src = "";
	return tok;
}

int my_stricmp(str1, str2)
char *str1;
char *str2;
{
	if (!str1 && !str2)
		return 0;
	else if (!str1 || !str2)
		return 1;
	while (*str1 && *str2 && (toupper(*str1) == toupper(*str2)))
	{
		str1++;
		str2++;
	}
	if (*str1 || *str2)
		return 1;
	return 0;
}

int ischannel(channel)
char *channel;
{
	return (*channel == '#' || *channel == '&');
}

char *mstrcat(dest, src)
char **dest;
char *src;
{
	char *temp;

	if (!*dest)
		return mstrcpy(dest, src);
	if (src)
	{
		temp = (char *) MyMalloc(strlen(src)+strlen(*dest)+1);
		strcpy(temp, *dest);
		strcat(temp, src);
		MyFree((char **)dest);
		*dest = temp;
	}
	return *dest;
}

char *mstrcpy(dest, src)
char **dest;
char *src;
{
	if (src)
	{
		*dest = (char *) MyMalloc(strlen(src)+1);
		strcpy(*dest, src);
	}
	else
		*dest = NULL;
	return *dest;
}

char *mstrncpy(dest, src, len)
char **dest;
char *src;
int len;
{
	if (src)
	{
		*dest = (char *) MyMalloc(len+1);
		strncpy(*dest, src, len);
	}
	else
		*dest = NULL;
	return *dest;
}

char *getnick(nick_userhost)
char *nick_userhost;
{
	strcpy(usernick, nick_userhost);
	return(strtok( usernick, "!" ));
}

char *gethost(nick_userhost)
char *nick_userhost;
{
	char *temp;
	strcpy(usernick, nick_userhost);
	temp = usernick;
	(void) get_token(&temp, "@");
	return temp;
}

int num_min(time)
time_t time;
{
	struct tm *btime;

	btime = localtime(&time);
	if (time && (sprintf(timebuf, "%-2.2d", btime->tm_min)))
		return atoi(timebuf);
	return 0;
}

int num_hour(time)
time_t time;
{
	struct tm *btime;

	btime = localtime(&time);
	if (time && (sprintf(timebuf, "%-2.2d", btime->tm_hour)))
		return atoi(timebuf);
	return 0;
}

char *time2str(time)
time_t time;
{
	struct tm *btime;

	btime = localtime(&time);
	if (time && (sprintf(timebuf, "%-2.2d:%-2.2d:%-2.2d %s %-2.2d %d",
				btime->tm_hour, btime->tm_min, btime->tm_sec,
				months[btime->tm_mon], btime->tm_mday,
				btime->tm_year + 1900)))
		return timebuf;
	return NULL;
}

char *time2away(time)
time_t time;
{
	struct tm *btime;
	char tbuf[MAXLEN], tbuf2[MAXLEN], type[2];
	int hour;
	static char am[] = "am";
	static char pm[] = "pm";

	btime = localtime(&time);
	
	if (time) {
		sprintf(tbuf, "%-2.2d", btime->tm_hour);
		switch (atoi(tbuf)) {
		  case 0:
		    hour = 12;
		    strcpy(type, am);
		    break;
		  case 1:
		    hour = 1;
		    strcpy(type, am);
		    break;
		  case 2:
		  	hour = 2;
		  	strcpy(type, am);
		  	break;
		  case 3:
		  	hour = 3;
		  	strcpy(type, am);
		  	break;
		  case 4:
		  	hour = 4;
		  	strcpy(type, am);
		  	break;
		  case 5:
		  	hour = 5;
		  	strcpy(type, am);
		  	break;
		  case 6:
		  	hour = 6;
		  	strcpy(type, am);
		  	break;
		  case 7:
		  	hour = 7;
		  	strcpy(type, am);
		  	break;
		  case 8:
		  	hour = 8;
		  	strcpy(type, am);
		  	break;
		  case 9:
		  	hour = 9;
		  	strcpy(type, am);
		  	break;
		  case 10:
		  	hour = 10;
		  	strcpy(type, am);
		  	break;
		  case 11:
		  	hour = 11;
		  	strcpy(type, am);
		  	break;
		  case 12:
		  	hour = 12;
		  	strcpy(type, pm);
		  	break;
		  case 13:
		  	hour = 1;
		  	strcpy(type, pm);
		  	break;
		  case 14:
		  	hour = 2;
		  	strcpy(type, pm);
		  	break;
		  case 15:
		  	hour = 3;
		  	strcpy(type, pm);
		  	break;
		  case 16:
		  	hour = 4;
		  	strcpy(type, pm);
		  	break;
		  case 17:
		  	hour = 5;
		  	strcpy(type, pm);
		  	break;
		  case 18:
		  	hour = 6;
		  	strcpy(type, pm);
		  	break;
		  case 19:
		  	hour = 7;
		  	strcpy(type, pm);
		  	break;
		  case 20:
		  	hour = 8;
		  	strcpy(type, pm);
		  	break;
		  case 21:
		  	hour = 9;
		  	strcpy(type, pm);
		  	break;
		  case 22:
		  	hour = 10;
		  	strcpy(type, pm);
		  	break;
		  case 23:
		  	hour = 11;
		  	strcpy(type, pm);
		  	break;
		  default:
		  	hour = 0;
		  	strcpy(type, pm);
		  	break;
		}
	}
		  	
	if (time && (sprintf(timebuf, "%i:%-2.2d %s %s %s %d", hour, btime->tm_min, type, days[btime->tm_wday], months[btime->tm_mon], btime->tm_mday)))
	{
		return timebuf;
	}
	return NULL;
}

char *timetoserver(time)
time_t time;
{
	struct tm *btime;

	btime = localtime(&time);
	if (time && (sprintf(timebuf, "%s_%-2.2d_%d", months[btime->tm_mon], btime->tm_mday, btime->tm_year + 1900)))
		return timebuf;
	return NULL;
}

char *time2medium(time)
time_t time;
{
	struct tm *btime;

	btime = localtime(&time);
	if (time && (sprintf(timebuf, "%-2.2d:%-2.2d",
		btime->tm_hour, btime->tm_min)))
		return timebuf;
	return NULL;
}

char *time2small(time)
time_t time;
{
	struct tm *btime;

	btime = localtime( &time );
	if (time && (sprintf(timebuf, "%s %-2.2d",
				months[btime->tm_mon], btime->tm_mday)))
		/*		  btime->tm_year+1900))) */
		return timebuf;
	return(NULL);
}

char *idle2str(time)
time_t time;
{
	int days,
		 hours,
		 mins,
		 secs;

	days  = (time/86400);
	hours = ((time - (days*86400))/3600);
	mins  = ((time - (days*86400) - (hours*3600))/60);
	secs  = (time - (days*86400) - (hours*3600) - (mins*60));

	sprintf(idlestr, "%d day%s %-2.2d:%-2.2d:%-2.2d",
		days, EXTRA_CHAR(days), hours,
		mins, secs);
	return idlestr;
}

char *idle2strsmall(time)
time_t time;
{
        int days,
                 hours,
                 mins,
                 secs;

        days  = (time/86400);
        hours = ((time - (days*86400))/3600);
        mins  = ((time - (days*86400) - (hours*3600))/60);
        secs  = (time - (days*86400) - (hours*3600) - (mins*60));

        sprintf(idlestr, "%d d, %d h, %d m, %d s",
                days, hours, mins, secs);
        return idlestr;
}

char *getuserhost(nick)
char *nick;
{
  static char line[HUGE];
  char unknown[] = "<UNKNOWN>@<UNKNOWN>";
  char *ptr, *ptr2;

/*
  if (nick && *nick)
	 strcpy(normuserhost, nick);
  else
	 strcpy(normuserhost, "*");
  strcat(normuserhost, "!");
*/
  strcpy(normuserhost, unknown);

  if (!nick || !*nick)
	 return normuserhost;

  senduserhost(nick);

	while (1==1)
	{
		while (readln(line) <= 0);
		debug(NOTICE, "getuserhost: line = %s", line);
		ptr2 = strstr(line, "302");
		if (ptr2 && my_stristr(line, nick))
		{
		  if (!(ptr = strchr(ptr2, '+')))
			 ptr = strchr(ptr2, '-');
		  if (ptr)
			 ptr++;
		  if (!ptr || !*ptr)
			 return normuserhost;
		  terminate(ptr, "\r\n");
		  return ptr;
		}
		else if (ptr2)
			return normuserhost;
		parseline(line);
	}
}

char *my_stristr(s1, s2)
char *s1;
char *s2;
{
	char n1[HUGE], n2[HUGE];
	char *temp, *ptr1, *ptr2;
	char *save = s1;

	ptr1 = n1;
	ptr2 = n2;
	if (!s1 || !s2)
	  return NULL;
	while (*s1)
	  *(ptr1++) = toupper(*(s1++));
	while (*s2)
	  *(ptr2++) = toupper(*(s2++));
	*ptr1 = '\0';
	*ptr2 = '\0';
	temp = strstr(n1, n2);
	if (temp)
	  return (save + (temp-n1));
	return NULL;
}

char *find_userhost(from, nick)
char *from;
char *nick;
{
	static char userhost[MAXLEN];
	char buffer[MAXLEN];
	char *temp;

	if (!nick)
		return NULL;
	strcpy(userhost, nick);
	if (strchr(nick, '!'))
	  return nick;
	if (strchr(nick, '@'))
	{
		strcpy(userhost, "*!");
		if (nick[0] != '*')
			strcat(userhost, "*");
		strcat(userhost, nick);
		return userhost;
	}
	temp = username(nick);
	if (!temp)
	  {
		 temp = getuserhost(nick);
		 if (temp && !my_stricmp(temp, "<UNKNOWN>@<UNKNOWN>"))
			temp = NULL;
		 else if (temp)
		 {
			strcpy(buffer, nick);
			strcat(buffer, "!");
			strcat(buffer, temp);
			temp = buffer;
	  	 }
	  }
	if (!temp)
	  temp = getuserhost_frommem(nick);
	if (strstr(temp, "<UNKNOWN>@<UNKNOWN>"))
	  temp = NULL;
	if (!temp)
	{
                if (from)
			no_info(from, nick);
		return NULL;
	}
	strcpy(userhost, temp);
	return userhost;
}

void no_info(who, nick)
char *who;
char *nick;
{
	send_to_user(who, "\002No information found for %s\002", nick);
}

void not_on(who, channel)
char *who;
char *channel;
{
	send_to_user(who, "\002I'm not on %s\002", channel);
}

void no_access(from, channel)
char *from;
char *channel;
{
	send_to_user(from, "\002You're not immortal enough on %s\002", channel);
}

char *left(string, i)
char *string;
int i;
{
  static char temp[255];
  strcpy(temp, string);
  temp[i] = '\0';
  return temp;
}

void mass_action(who, channel)
char *who;
char *channel;
{
	char *temp;
	int mpl;

	if (!get_int_var(channel, TOGMASS_VAR))
		return;

	temp = format_uh(who, 1);
	if (((mpl=get_int_var(channel, SETMPL_VAR)) >= 3) &&
		get_int_var(channel, TOGAS_VAR))
	{
		if (!is_user(who, channel) &&
			!(usermode(channel, getnick(who))&MODE_CHANOP))
		{
			add_to_shitlist(&(current->Userlist), temp, 2, channel, "Auto-Shit",
			  "Quit the damn mass modes", getthetime(), getthetime() + (long) 86400);
#ifdef USE_CCC
			send_to_ccc("1 SHIT Auto-Shit %s %s 2 %li %li %s",
                        channel, temp, getthetime(),
                        getthetime()+86400, "Quit the damn mass modes");
#endif
		}
	}
	if (mpl >= 2)
		deop_ban(channel, getnick(who), temp);
	if (mpl >= 1)
		sendkick(channel, getnick(who),
			"\002Get the fuck out mass moding lamer");
}

void shit_action(who, channel)
char *who;
char *channel;
{
	char *shitteduh;
	aUser *tmp;
	
	if (!get_int_var(channel, TOGSHIT_VAR))
		return;
	if (!i_am_op(channel))
		return;
	if (is_user(who, channel))  /* bypass users */
		return;
	if ((tmp=find_shit(&(current->Userlist), who, channel)) == NULL)
		return;
	shitteduh = tmp->userhost ? tmp->userhost : "";
	if (!*shitteduh)
		return;
	if (tmp->access > 1)
	{
		char *shitr;
		deop_ban(channel, getnick(who), shitteduh);
		shitr = get_shitreason(tmp);
		sendkick(channel, getnick(who), "%s", shitr);
	}
	else if ((tmp->access == 1) && is_opped(getnick(who), channel))
		takeop(channel, getnick(who));
}

void prot_action(from, channel, protnuh)
char *from;
char *channel;
char *protnuh;
{
	int level;
	char saveprotnuh[MAXLEN];
	aUser *Dummy, *tmp;
	
	if (!my_stricmp(getnick(from), current->nick)) {
		return;
	}

	if (!get_int_var(channel, TOGPROT_VAR))
		return;

	if (protnuh)
		strcpy(saveprotnuh, protnuh);
	else
		return;
	if (!i_am_op(channel))
		return;
	Dummy = find_user(&(current->Userlist), protnuh, channel);
	level = get_protlevel_matches(protnuh, channel);
	if (Dummy && (Dummy->prot > level))
	{
		level = Dummy->prot;	
		strcpy(saveprotnuh, Dummy->userhost);
	}
	if (!level)
		return;
	tmp=find_user(&(current->Userlist), from, channel);
	if (!tmp || !(tmp->prot || (tmp->access < ASSTLEVEL)))
	{
	  char *banuh;
	  banuh = format_uh(from, 1);
	  if (level >= 4)
		 deop_ban(channel, getnick(from), banuh);
	  if (level >= 3)
		 sendkick(channel, getnick(from),
			"\002%s is Protected\002", saveprotnuh);
	  if (level == 2)
		 takeop(channel, getnick(from));
	}
}

void takeop(channel, nicks)
char *channel;
char *nicks;
{
	if (i_am_op(channel))
		mode3(channel, "-oooo", nicks);
}

void giveop(channel, nicks)
char *channel;
char *nicks;
{
	if (i_am_op(channel))
		mode3(channel, "+oooo", nicks);
}

void mode3(channel, mode, data)
char *channel;
char *mode;
char *data;
{
	char *temp, *start;

	temp = data;
	temp = stripl(temp, " ");
	start = temp;
	temp = strchr(temp, ' ');
	if (temp)
	{
		temp = stripl(temp, " ");
		temp = strchr(temp, ' ');
	}
	if (temp)
	{
		temp = stripl(temp, " ");
		temp = strchr(temp, ' ');
	}
	if (temp)
	{
		temp = stripl(temp, " ");
		temp = strchr(temp, ' ');
	}
	if (temp)
	{
		mode3(channel, mode, temp);
		*temp = '\0';
	}
#ifdef DBUG
	debug(NOTICE, "Sending mode: %s %s %s\n", channel, mode, start);
#endif
	sendmode(channel, "%s %s", mode, start);
}

int is_me(nick)
char *nick;
{
	return (!my_stricmp(current->nick, nick));
}

int i_am_op(channel)
char *channel;
{
	return is_opped(current->nick, channel);
}

int is_a_bot(nuh)
char *nuh;
{
	register int i;
	register aList *tmp;
	char temp[MAXLEN], temp2[MAXLEN];
	char *n, *u, *h, *nuh2;

	if (!nuh)
		return FALSE; /* just in case */
	strcpy(temp, nuh);
	nuh2 = temp;
	n = get_token(&nuh2, "!");
	u = get_token(&nuh2, "@");
	h = nuh2;	
	if (!n || !u || !h)
		return FALSE;
	u = fixusername(u);
	sprintf(temp2, "%s!%s@%s", n, u, h);	

        for (i=0; i<MAXBOTS; i++)
                if (thebots[i].created)
			if (!my_stricmp(n, thebots[i].nick))
				return TRUE;
	for (tmp=Bots;tmp;tmp=tmp->next)
		if (!my_stricmp(temp2, tmp->name))
			return TRUE;	
	return FALSE;
}

int bot_matches(nuh)
char *nuh;
{
        register aList *tmp;

        if (!nuh)
                return FALSE; /* just in case */
        for (tmp=Bots;tmp;tmp=tmp->next)
                if (!matches(nuh, tmp->name))
                        return TRUE;
        return FALSE;
}

char *get_channel_and_userlevel(to, rest, from, level)
char *to;
char **rest;
char *from;
int *level;
{
        static char chan[MAXLEN];
		
        strcpy(chan, to);
    
	if (level)
		*level = CurrentUser ? CurrentUser->access : 0;
        if (!ischannel(to))
		strcpy(chan, currentchannel());
	if (*rest && **rest)
		if (ischannel(*rest))
		{
			strcpy(chan, get_token(rest, " "));
			if (level)
				*level = get_userlevel(from, chan);
		}
	return chan;
}

char *get_channel(to, rest)
char *to;
char **rest;
{
	static char chan[MAXLEN];

	strcpy(chan, to);

	if (!ischannel(to))
		strcpy(chan, currentchannel());
	if (*rest && **rest)
	  if (ischannel(*rest))
	    strcpy(chan, get_token(rest, " "));
	return chan;
}

char *get_channel2(to, rest)
char *to;
char **rest;
{
	static char chan[MAXLEN];

	strcpy(chan, to);

	if (!ischannel(to))
		strcpy(chan, currentchannel());
	if (*rest && **rest)
		if (ischannel(*rest) || (**rest == '*' &&
			(*(*rest+1) == ' ' || !(*(*rest+1)))))
			strcpy(chan, get_token(rest, " "));
	return chan;
}


char *nick2uh(from, userhost, type)
char *from;
char *userhost;
int type;
{
	static char buffer[HUGE];
	char *uh;

	if (!userhost || !*userhost)
		return NULL;
	uh = buffer;
	if (strchr(userhost, '!') && strchr(userhost, '@'))
		return userhost;

	strcpy(uh, userhost);
	if (!strchr(userhost, '!') && !strchr(userhost, '@'))
	{
		char *temp;
		temp = find_userhost(from, userhost);
		if (!temp)
			return NULL;
		if (type)
			temp = format_uh(temp, type);
		strcpy(buffer, temp);
	}
	else
	{
		strcpy(buffer, "*!");
		if (!strchr(userhost, '@'))
			strcat(buffer, "*");
		strcat(buffer, userhost);
	}
	return buffer;
}

char *format_nuh(userhost)
char *userhost;
{
	char *n, *u, *h, *ptr;
	static char buffer[MAXLEN];
	char temp[MAXLEN];

	if (!userhost)
		return NULL;
	ptr = temp;
	strcpy(ptr, userhost);
	n = get_token(&ptr, "!");
	u = get_token(&ptr, "@");
	h = cluster(ptr);
	u = fixusername(u);
	sprintf(buffer, "%s!%s@%s", n, u, h);
	return buffer;
}

char *format_nuh2(userhost)
char *userhost;
{
	char *n, *u, *h, *ptr;
	static char buffer[MAXLEN];
	char temp[MAXLEN];

	if (!userhost)
		return NULL;
	ptr = temp;
	strcpy(ptr, userhost);
	n = get_token(&ptr, "!");
	u = get_token(&ptr, "@");
	h = cluster(ptr);
	u = fixusername(u);
	sprintf(buffer, "%s!%s@%s", n, u, h);
	return h;
}
	
char *format_uh(userhost, type)
char *userhost;
int type;
{
	char *n, *u, *h, *ptr;
	static char buffer[MAXLEN];
	char temp[MAXLEN];

	ptr = temp;
	strcpy(ptr, userhost);
	if (strchr(ptr, '*'))
	{
		strcpy(buffer, ptr);
		return buffer;
	}
	n = get_token(&ptr, "!");
	u = get_token(&ptr, "@");
	h = ptr;
	u = fixusername(u);
	if (!u || !*u)
	  return NULL;
	if (!h || !*h)
	  return NULL;

/* to catch nick bans also (kludgy though) */
/* disabled for now
	strcat(buffer, "*!");
	if ((type == 0) || (type == 1))
	{
		if(*u != '*')
		  strcat(buffer, "*");
		  strcat(buffer, right(u, 9));
		  strcat(buffer, "@");
		  strcat(buffer, cluster(h));
	}
	else if (type == 2)
	{
		strcat(buffer, "*@");
		strcat(buffer, cluster(h));
	}
	else
	{
		if(*u != '*')
		  strcat(buffer, "*");
		  strcat(buffer, right(u, 9));
		  strcat(buffer, "@*");
	}

*/
	if ((type == 0) || (type == 1))
		sprintf(buffer, "*!*%s@%s", right(u, 9), cluster(h));
	else
		sprintf(buffer, "*!*@%s", cluster(h));
	return buffer;
}

void deop_ban(channel, nick, nuh)
char *channel;
char *nick;
char *nuh;
{
	if (!channel || !nick || !nuh)
		return;
	sendmode(channel, "-o+b %s %s", nick, format_uh(nuh, 1));
}

void deop_bannick(channel, nick)
char *channel;
char *nick;
{
	char *uh;

	uh = nick2uh(NULL, nick, 1);
	sendmode(channel, "-o+b %s %s", nick, uh);
}

void deop_siteban(channel, nick, nuh)
char *channel;
char *nick;
char *nuh;
{
	if (!channel || !nick || !nuh)
		return;
	if (strchr(nuh, '*'))
        	sendmode(channel, "-o+b %s %s", nick, nuh);
        else
		sendmode(channel, "-o+b %s %s", nick, format_uh(nuh, 2));
}

void deop_sitebannick(channel, nick)
char *channel;
char *nick;
{
	char *uh;

	uh = nick2uh(NULL, nick, 2);
	sendmode(channel, "-o+b %s %s", nick, uh);
}

void not_opped(from, channel)
char *from;
char *channel;
{
	send_to_user(from, "\002I am not opped on %s\002", channel);
}

void signoff(from, reason)
char *from;
char *reason;
{
	if (from)
		send_to_user(from, "\002Saving the lists...\002");
	if (!saveuserlist(&(current->Userlist), current->USERFILE))
		send_to_user(from, "\002Could not save user/shitlists\002");
	if (!write_timelist(&current->KickList, current->kickfile) && from)
		send_to_user(from, "\002%s\002", "Could not save kicklist");
	if (!write_seenlist(&current->SeenList, current->seenfile)) {
        send_to_user("\002SeenList could not be saved to file %s\002", current->seenfile);
	}
	if (!write_levelfile(LEVELFILE) && from)
		send_to_user(from, "\002%s\002", "Could not save levels");
	if (from)
		send_to_user(from, "\002%s\002", "ShutDown Complete");
	if (number_of_bots == 1)
	{
		if (from)
			send_to_user(from, "\002%s\002",
				 "No Mechs Left");
		free_logs();
	}
	if (!kill_bot(reason))
		MyExit(0);
}

int isnick(nick)
char *nick;
{
  int i;

  for (i=0; nick[i]; i++)
	 if((i==0 && !isalpha(nick[i])) || !isalnum(nick[i]))
		if(!strrchr("|_-[]\\{}^", nick[i]))
			return FALSE;
  return TRUE;
}

char *cluster(hostname)
char *hostname;
{
	static char result[1024];
	char	    temphost[255];
	char	    *host;

	if (!hostname)
                return (char *) 0;
	host = temphost;
	strcpy(result, "");
	if (strchr(hostname, '@'))
        {
            strcpy(result, hostname);
            *strchr(result, '@') = '\0';
            strcat(result, "@");
	    hostname = strchr(hostname, '@');
            hostname++;
        }
	strcpy(host, hostname);
	if (*host && isdigit(*(host+strlen(host)-1)))
	{
		int i;
		char *tmp;
                tmp = host;
		for (i=0;i<2;i++)
			tmp = strchr(tmp, '.')+1;
		*tmp = '\0';
		strcat(result, host);
		strcat(result, "*.*");
	}
	else
	{
                char	*tmp;
                int	num;

		num = 1;
		tmp = right(host, 3);
		if (my_stricmp(tmp, "com") &&
			my_stricmp(tmp, "edu") &&
			(my_stristr(host, "com") ||
				my_stristr(host, "edu")))
          		num = 2;
		while (host && *host && (numchar(host, '.') > num))
                {
			if ((host = strchr(host, '.')) != NULL)
				host++;
                        else
                                return (char *) NULL;
		}
		strcat(result, "*");
		if (my_stricmp(host, temphost))
			strcat(result, ".");
                strcat(result, host);
	}
	return result;
}

int numchar(string, ch)
char *string;
int ch;
{
	int num = 0;
	char c;

	c = (char) ch;
	while (*string)
	{
		if (tolower(*string) == tolower(c))
			num++;
		string++;
	}
	return num;
}

char *right(string, num)
char *string;
int num;
{
	if (strlen(string) < num)
		return string;
	return (string+strlen(string)-num);
}

void no_chan(from)
char *from;
{
  send_to_user(from, "\002%s\002", "No channel given");
}

int percent_caps(string)
char *string;
{
    int len, tot;

    
    tot = 0;
    if (!string || !*string)
       return 0;
    len = strlen(string);
    while (*string)
    {
      if (isalpha(*string) && isupper(*string))
         tot++;
      string++;
    }
    return (100*tot)/len;
}


char *encode(passwd)
char *passwd;
{
  static char saltChars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";
  char salt[3];

  srand(time(NULL));
  salt[0] = saltChars[rand() % 64];
  salt[1] = saltChars[rand() % 64];
  salt[2] = '\0';
  
  return (char *) crypt(passwd, salt);
}

int passmatch(plain, encoded)
char *plain;
char *encoded;
{
  char salt[3];

  salt[0] = encoded[0];
  salt[1] = encoded[1];
  salt[2] = '\0';
  return (!my_stricmp((char *) crypt(plain, salt), encoded));
}

int check_for_number(from, string)
char *from;
char *string;
{
	if (!string || !isdigit(*string))
	{
		send_to_user(from, "\002%s\002", "A number is expected instead of a string");
		return 1;
	}
	return 0;
}

void do_who(chan)
char *chan;
{
	send_to_server( "WHO %s", chan);
}


