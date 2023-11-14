/*
 * function.c - misc functions that were needed
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#ifndef MSDOS
  #include <sys/time.h>
#endif
#include <time.h>
#include <ctype.h>
#include "config.h"
#include "function.h"
#include "userlist.h"
#include "channel.h"
#include "chanuser.h"
#include "dweeb.h"
#include "debug.h"

extern char *crypt(char *plaintext, char *salt);

static  char    usernick[MAXLEN];
static  char    normuserhost[MAXLEN];
static  char    timebuf[MAXLEN];
static  char    idlestr[MAXLEN];
char		lastuserhost[MAXLEN];

extern botinfo *current;
extern int number_of_bots;


static  char    *months[] =
{
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

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

char *terminate(string, chars)  /* terminates a string if a char is in it */
char *string;
char *chars;
{
	char *ptr;

	if (!string || !chars)
	 return "";
	while (*chars)
	{
	if (ptr = strrchr(string, *chars))
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

char *mstrcpy(dest, src)
char **dest;
char *src;
{
	if (src)
	{
		*dest = (char *) malloc(strlen(src)+1);
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
		*dest = (char *) malloc(len+1);
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
	char *temp, *ptr;
	strcpy(usernick, nick_userhost);
	temp = usernick;
	ptr = get_token(&temp, "@");
	if (!temp || !*temp)
		return "";
	return (cluster(temp));
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

	sprintf(idlestr, "%d day%s, %d hour%s, %d minute%s and %d second%s",
		days, EXTRA_CHAR(days), hours, EXTRA_CHAR(hours),
		mins, EXTRA_CHAR(mins), secs, EXTRA_CHAR(secs));
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
  char line[HUGE];
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
	  return (s1 + (temp-n1));
	return NULL;
}

char *find_userhost(from, nick)
char *from;
char *nick;
{
	static char userhost[MAXLEN];
	char buffer[MAXLEN];
	char *temp;

	strcpy(lastuserhost, "");
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
		 else
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
	strcpy(lastuserhost, temp);
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
	send_to_user(from, "\002You don't have enough access on %s\002", channel);
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

  if (!get_masstoggle(channel))
	 return;

  temp = format_uh(who, 1);
  if (current->massprotlevel >= 3)
	 {
		if (!is_user(who, channel) &&
			!(usermode(channel, getnick(who))&MODE_CHANOP))
		{
			add_to_shitlist(current->ShitList, temp, 2, channel, "Auto-Shit",
			  "Quit the damn mass modes", getthetime());
			sendnotice(channel, "\002%s shitlisted for mass moding\002", getnick(who));
		}
	 }
  if (current->massprotlevel >= 2)
	 deop_ban(channel, getnick(who), temp);
  if (current->massprotlevel >= 1)
	 sendkick(channel, getnick(who), "\002%s\002", "Get the fuck out mass moding lamer");
}

void shit_action(who, channel)
char *who;
char *channel;
{
	int level;
	char *shitteduh;
	SLS *dummy;

debug(NOTICE, "ShitAction: %s, %s", who, channel);
	
	if (!get_shittoggle(channel))
	  return;

	if (get_userlevel(who, channel))  /* bypass users */
			 return;
	 
	if (!(level = get_shitlevel(who, channel)))
		return;

	if (!i_am_op(channel))
		return;

	if (!(dummy=is_in_shitlist(current->ShitList, who, channel)))
		return;

	shitteduh = get_shituh(dummy);   /* Note that get_shituh and
						 get_shitreason use the
						 same static buffer
					 */
	if (!*shitteduh)
		return;
	if (level >= 2)
	{
	  char *shitr;
	  if (!my_stricmp(getnick(shitteduh), "*") ||
			!my_stricmp(getnick(shitteduh), ""))
		 ;
	  else
		 if (get_userlevel(who, channel) > 50)
			return;
	  deop_ban(channel, getnick(who), shitteduh);

/*        else
		 deop_ban(channel, who); */

	  shitr = get_shitreason(dummy);
	  sendkick(channel, getnick(who), "%s", shitr);
	  return;
	}
	else
		takeop(channel, getnick(who));
}

void prot_action(from, channel, protnuh)
char *from;
char *channel;
char *protnuh;
{
	int level;
	char buf[MAXLEN];
	char saveprotnuh[MAXLEN];
	ULS *Dummy;

	if (!get_prottoggle(channel))
		return;

	if (protnuh)
	  strcpy(saveprotnuh, protnuh);
	else
	  strcpy(saveprotnuh, "");
	if (Dummy=is_in_userlist(current->UserList, saveprotnuh, channel))
	  {
		 if (Dummy->prot)
			strcpy(saveprotnuh, Dummy->userhost);
	  }
	level = get_protlevel2(saveprotnuh, channel);
	if (!level || !i_am_op(channel))
		return;

	if (!(get_protlevel(from, channel) || (get_userlevel(from, channel) >= ASSTLEVEL)))
	{
	  char *banuh;
	  banuh = format_uh(from, 1);
	  if (level >= 4)
		 deop_ban(channel, getnick(from), from);
	  if (level >= 3)
		 sendkick(channel, getnick(from), "\002%s is Protected\002", saveprotnuh);
	  if (level == 2)
		 takeop(channel, getnick(from));
	}
}

void takeop(channel, nicks)
char *channel;
char *nicks;
{
	if (i_am_op(channel))
		mode3(channel, "-ooo", nicks);
}

void giveop(channel, nicks)
char *channel;
char *nicks;
{
	if (i_am_op(channel))
		mode3(channel, "+ooo", nicks);
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
		mode3(channel, mode, temp);
		*temp = '\0';
	}
#ifdef DBUG
	debug(NOTICE, "Sending mode: %s %s %s\n", channel, mode, start);
#endif
	sendmode(channel, "%s %s", mode, start);
}

int i_am_op(channel)
char *channel;
{
	return is_opped(current->nick, channel);
}

char *find_channel(to, rest)
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
	debug(NOTICE, "find_channel: CHANNEL: %s", chan);
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
		char *temp, *n, *u, *h;
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

char *format_uh(userhost, type)
char *userhost;
int type;
{
	char *n, *u, *h;
	static char buffer[MAXLEN];

	if (strchr(userhost, '*'))
		return userhost;
	n = get_token(&userhost, "!");
	u = get_token(&userhost, "@");
	h = userhost;
	u = stripl(u, "~#");
	if (!u || !*u)
	  return NULL;
	if (!h || !*h)
	  return NULL;
	if ((type == 0) || (type == 1))
		sprintf(buffer, "*!*%s@%s", u, cluster(h));
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

int toggle(char *from, int *bleah, char *what, ...)
{
	char buf[HUGE];
	va_list msg;

	va_start(msg, what);
	vsprintf(buf, what, msg);
	va_end(msg);
	if (*bleah)
	{
		send_to_user(from, "\002%s now disabled\002", buf);
		*bleah = 0;
	}
	else
	{
		send_to_user(from, "\002%s now enabled\002", buf);
		*bleah = 1;
	}
}

void signoff(from, reason)
char *from;
char *reason;
{
  char    *fromcpy;
  
  mstrcpy(&fromcpy, from);                /* something hoses */
  send_to_user(fromcpy, "\002Saving lists...\002");
  if (!write_userlist(current->UserList, current->usersfile))
	 send_to_user(fromcpy, "\002Could not save userlist\002");
  if (!write_shitlist(current->ShitList, current->shitfile))
	 send_to_user(fromcpy, "\002%s\002", "Could not save shitlist");
  send_to_user(fromcpy, "\002%s\002", "Owwwww...I'm dead!");
  if (number_of_bots == 1)
	 send_to_user(fromcpy, "\002%s\002", "No bots left... Terminating");
  free(fromcpy);
  kill_bot(reason);
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
	    if (!(hostname = strchr(hostname, '@')))
            	return (char *) 0;
            hostname++;
        }
	strcpy(host, hostname);

	if (isdigit(*host))
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

int numchar(string, c)
char *string;
char c;
{
	int num = 0;
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

void steal_channels(void)
{
	LS *temp;
	int number;

	for (temp=*(current->StealList);temp;temp=temp->next)
	{
	  number = tot_num_on_channel(temp->name);
	  debug(NOTICE, "Steal: %i users on %s", number, temp->name);
	  if (number == 1)
		 cycle_channel(temp->name);
	}
}

int string_all_caps(string)
char *string;
{
	if (!string)
		return FALSE;
	if (strlen(string) < 2)
	  return FALSE;
	while (*string)
	{
		if (toupper(*string) != *string)
			return FALSE;
		string++;
	}
	return TRUE;
}

int num_words_all_caps(string)
char *string;
{
	char *temp, *temp2;
	int num;

	num = 0;
	if (!string)
		return num;
	mstrcpy(&temp, string);
	while (temp && *temp)
	{
		temp2 = get_token(&temp, ",.;: ");
		if (temp2 && string_all_caps(temp2))
			num++;
	}
	return num;
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

  srandom(time(NULL));
  salt[0] = saltChars[random() % 64];
  salt[1] = saltChars[random() % 64];
  salt[2] = '\0';
  
  return crypt(passwd, salt);
}

int passmatch(plain, encoded)
char *plain;
char *encoded;
{
  char salt[3];

  salt[0] = encoded[0];
  salt[1] = encoded[1];
  salt[2] = '\0';
  printf("HEREEREERER: %s %s\n", crypt(plain, salt), encoded);
  return (!my_stricmp(crypt(plain, salt), encoded));
}

char *get_lastuh(void)
{
	return lastuserhost;
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


void get_n_lists()
{

FILE *pl;
FILE *gl;
int lcount[3];
char line[128];
char *lines[3][128];
int pling;
int gling;

	int i=0;
	void *pos;

	if(!(pl=fopen("PING.list", "r"))) {
		globallog(ERRFILE, "could not open PING.list");
			pling = 0;
	}

	while((fgets(line, sizeof(line), pl))!=NULL) {
		lines[1][i] = strdup(line);
		if ((pos==index(lines[1][i], "\n"))!=NULL) {
			i = i+1;
		}
	}

	fclose(pl);
	lcount[1] = i;
	pling = 1;
	
	if(!(gl=fopen("GREET.list", "r"))) {
		globallog(ERRFILE, "could not open GREET.list");
			gling = 0;
	}

	while((fgets(line, sizeof(line), gl))!=NULL) {
		lines[2][i] = strdup(line);
		if ((pos==index(lines[2][i], "\n"))!=NULL) {
			i = i+1;
		}
	}

	fclose(gl);
	lcount[2] = i;
	gling = 1;

	return;
		
}
