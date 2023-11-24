#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef SYSV
#include <strings.h>
#endif
#include <ctype.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>

#include "channel.h"
#include "log.h"
#include "config.h"
#include "dcc.h"
#include "debug.h"
#include "ftext.h"
#include "ftp_dcc.h"
#include "misc.h"
#include "parsing.h"
#include "send.h"
#include "session.h"
#include "userlist.h"
#include "fnmatch.h"
#include "vladbot.h"
#include "vlad-ons.h"
#include "incu-ons.h"
#include "crc-ons.h"
#ifdef BOTNET
#include "botnet.h"
#endif
#include "tb-ons.h"

extern  botinfo *currentbot;
extern  int     number_of_bots;
extern  int     rehash;
extern  long    uptime;
extern  char    *botmaintainer;
extern command_tbl on_msg_commands[];
extern	int	 addressed_to_me;
extern	int	 public_command;

char	tempstring[MAXLEN];
#ifdef BackUp
int Query=0;
int Abackup=1;
#endif
#ifdef BackedUp
int Abackedup=1;
#endif


/*
char *seentype[]={	"%s joined at %s (%s)","%s left at %s (%s)",
					"%s signed off at %s (%s)",
 					"%s changed nicks at %s (%s)",
 					"%s was kicked by %s at %s (%s)"};
*/

/*        
struct seenstruct
{  
  char nick[70];
  char nick2[20];  
  char time[40];  
  char kicker[70]; 
  char soffnick[128];
  int type; 
  struct seenstruct *next;
};

typedef struct seenstruct *seenlist;
seenlist seen=NULL;
*/
extern int onchannel;

extern char		kicker[70];
char			*nicklist[100],*onefrom;
char			oldnick[20];
int				nickcount=0,curnick=-1;
extern int		NICK;
int				ALARM=1,ALARM2=1,ALARM3=0,alarms=30,ANICK=0,OKILL=0;
short			kick_check=0;

int lvlcheck(char *p1, char *p2)
{
  if (userlevel(p1)>userlevel(p2)) return 0;
  else if (userlevel(p1)==userlevel(p2) && protlevel(p1)>=protlevel(p2)) return 0;
  else return 1;
}

void checkalarms(void)
{
	if (ALARM)
	{
		ALARM=0;
		check_ops();
	}

	if (ALARM2)
	{
#ifdef BackUp
		if (Abackup)
		{
			Query=1;
			send_to_server("WHOIS %s",getnick(currentbot->backupmask));
		}
#endif
		currentbot->killed=0;
		OKILL=0;
		ALARM2=0;
		if (NICK) do_nickch2();
	}

	if (ALARM3>14)
	{
		do_nickch(currentbot->nick,currentbot->nick,"");
		ALARM3=0;
	}

	if (kick_check)
		check_for_kicks();

	if ( ((time(NULL) - currentbot->last_event_mail) > 43000) &&
		(strstr(time2str(time(NULL)),MAIL_EVENTS)!=NULL))
		mail_list_events();
}

void sig_alrm()
{
	ALARM=1;
	ALARM2++;

	if (ANICK)
		ALARM3++;

	signal(SIGALRM,sig_alrm2);
	alarm(alarms);
}
void sig_alrm2()
{
	ALARM2++;

	if (ANICK)
		ALARM3++;

	signal(SIGALRM,sig_alrm);
   alarm(alarms);
}

/*int addseen(char *nick, int type, char *soffnick)
{
	char		*te;
	int		i;
	seenlist	temp=seen,temp2;
	static int	count=0;

	te = strdup(nick);
  
	for(i=0;i<strlen(te);i++)
		te[i]=tolower(te[i]);

	if (count==0)
		seen=NULL;

	temp=seen;

	while(temp !=NULL && strcmp(te,temp->nick2)!=0)
		temp = temp -> next;

	if (temp == NULL)
	{
		if (count>=200)
		{
			temp=seen;
			seen=NULL;

			while(temp!=NULL)
			{
				temp2=temp;
				temp=temp->next;
				free(temp2);
			}

			count=0;
		}

		temp2 = (seenlist)malloc(sizeof(struct seenstruct));

		if (username(nick)==NULL)
			strcpy(temp2->nick,nick);
		else
			strcpy(temp2->nick,username(nick));

		strcpy(temp2->nick2,nick);
		temp2->type=type;

		for (i=0;i<strlen(temp2->nick2);i++) 
			temp2->nick2[i]=tolower(temp2->nick2[i]);

		strcpy(temp2->time,get_ctime(0));

		if (type==4)
			strcpy(temp2->kicker,kicker);

		strcpy(temp2->soffnick,soffnick);
		temp2->next=seen;
		seen=temp2;
		count++;
	}
	else
    {
		if (username(nick)==NULL)
			strcpy(temp->nick,nick);
		else
			strcpy(temp->nick,username(nick));

		strcpy(temp->time,get_ctime(0);

		if (type==4)
			strcpy(temp->kicker,kicker);

		strcpy(temp->soffnick, soffnick);
		temp->type=type;
    }

	return 1;
}
*/

void greet(char *who,int where)
{
  char words[40],w[40];
  int t=rand()%5;

  switch (rand() % 9) 
   {
    case 0:
	 case 1:
    case 2: strcpy(words,"Yo"); break;
    case 3:
    case 4:
	 case 5: strcpy(words,"Hiya"); break;
    case 6:
    case 7:
    case 8: strcpy(words,"Hey"); break;
    default: strcpy(words,"error"); break;
   }

  if (where)
	 {
	 if (t>3)
      {
	strcat(words," ");
	strcat(words,who);
      }
    send_to_user(who,words);
    }
  else
    {
    if (t<3)
      {
	strcpy(w,who);
	strcat(w,": ");
	strcat(w,words);
	strcpy(words,w);
      }
	 sendprivmsg(userchannel(who), words);
	 }
}

void greet2(char *who, int where)
{
  char words[40],w[40],words2[40];

  switch (rand()%2)
	{
     case 0:strcpy(words,"Pretty good"); break;
     case 1:strcpy(words,"Fine"); break;
   }
  switch (rand()%2)
   {
	  case 0:strcpy(words2,"And you?"); break;
     case 1:strcpy(words2,"How's things?"); break;
	}

  if (where)
   {
	send_to_user(who,words);
	if (rand()%5 < 3) send_to_user(who,words2);
   }
  else
   {
    if (rand()%5 < 3)
      {
	strcpy(w,who);
	strcat(w,": ");
	strcat(w,words);
	strcpy(words,w);
        strcpy(w,who);
	strcat(w,": ");
	strcat(w,words2);
	strcpy(words2,w);
      }
	 sendprivmsg(userchannel(who), words);
    if (rand()%5 < 3) sendprivmsg(userchannel(who), words2);
   }
}

void rgreet2(char *who, int where)
{
  char words[40],w[40],words2[40];

  switch (rand()%2)
   {
     case 0:strcpy(words,"Not much"); break;
	  case 1:strcpy(words,"Just chillin"); break;
	}
  switch (rand()%2)
   {
     case 0:strcpy(words2,"Wassup with you?"); break;
     case 1:strcpy(words2,"How's things?"); break;
	}

  if (where)
   {
	send_to_user(who,words);
	if (rand()%5 < 3) send_to_user(who,words2);
   }
  else
   {
    if (rand()%5 < 3)
      {
	strcpy(w,who);
	strcat(w,": ");
	strcat(w,words);
	strcpy(words,w);
	strcpy(w,who);
	strcat(w,": ");
	strcat(w,words2);
	strcpy(words2,w);
      }
    sendprivmsg(userchannel(who), words);
	 if (rand()%5 < 3) sendprivmsg(userchannel(who), words2);
	}
}


void    do_whyshitted(char *from, char *to, char *rest)
{
	shitlist_t *dummy;

	if (!rest)
	{
		send_to_user(from, "Please specify a mask to query shit info on.");
		return;
	}
	dummy = find_shitmask(currentbot->lists->shitlist, rest, 0);
	if (dummy)
	{
		if(dummy->whyshitted)
		{
			send_to_user(from, "%s %s by %s.", rest, dummy->whyshitted, dummy->whoshitted);
			return;
		}
	}
	send_to_user(from, "No shit information for %s stored", rest);
}

int Find_Lowest(userlist_t    **list)
{
  userlist_t    *p=*list;
  int low=4000;

  while (p!=NULL)
  {
    if (p->access<low) low=p->access;
    p=p->next;
  }
  return low;
}

void 	do_showaccess(char *from, char *to, char *rest)
{
	int i,level;
	int shitlvl, userlvl;

	shitlvl = shitlevel(from);
	userlvl = userlevel(from);
	if (rest)
	{
		level=atoi(rest);

		if (level<0)
		  send_to_user(from,"Invalid level");
		else
		{
			send_to_user(from,"Functions at level %d:",level);
			for(i = 0; on_msg_commands[i].name != NULL; i++)
			{
				if(level == currentbot->commandlvl[i] &&
					shitlvl <= on_msg_commands[i].shitlevel)
					send_to_user(from,"%s",on_msg_commands[i].name);
			}
			if(level == 25)
			{
				send_to_user(from,"Additional flags for level 25:");
				send_to_user(from,"NOBANKICK:    %s", currentbot->l_25_flags&FL_NOBANKICK?"ON":"OFF");
				send_to_user(from,"NOOPKICK:     %s", currentbot->l_25_flags&FL_NOOPKICK?"ON":"OFF");
				send_to_user(from,"NOPUBLICKICK: %s", currentbot->l_25_flags&FL_NOPUBLICKICK?"ON":"OFF");
				send_to_user(from,"NOCLONEKICK:  %s", currentbot->l_25_flags&FL_NOCLONEKICK?"ON":"OFF");
				send_to_user(from,"NOMASSNK:     %s", currentbot->l_25_flags&FL_NOMASSNK?"ON":"OFF");
				send_to_user(from,"CANHAVEOPS:   %s", currentbot->l_25_flags&FL_CANHAVEOPS?"ON":"OFF");
			}
		}
	}
	else
	{
		send_to_user(from,"             Command | Level");
		send_to_user(from,"---------------------+------");
		for(i = 0; on_msg_commands[i].name != NULL; i++)
		{
			if(userlvl >= currentbot->commandlvl[i] &&
				shitlvl <= on_msg_commands[i].shitlevel)
				send_to_user(from,"%20s | %d",
				on_msg_commands[i].name,currentbot->commandlvl[i]);
		}
	}
}

void do_action(char *from, char *to, char *rest)
{
	char temp[2];

	if(!rest)
	{
		send_to_user(from, "What do you want me to say?");
		return;
	}

	if (rest && (rest[0]==1 || rest[0]=='!'))
	{
		send_to_user(from,"Screw off man");
		send_ctcp(userchannel(getnick(from)),
			"ACTION grabs %s and stares in his eyes and says don\'t even think about it",
			getnick(from));
		return;
	}

	temp[0] = 3;
	temp[1] = 0;
	if(strstr(rest, temp))
	{
		send_ctcp(userchannel(getnick(from)),"ACTION shoves some mIRC colors up %s\'s ass",getnick(from));
		return;
	}

	send_ctcp(userchannel(getnick(from)),"ACTION %s",rest);
}

void do_describe(char *from, char *to, char *rest) 
{
	char *nick, temp[2];

	if (rest==NULL)
	{
		send_to_user(from,"fool");
		return;
	}

	temp[0] = 3;
	temp[1] = 0;
	if(strstr(rest, temp))
	{
		send_ctcp(userchannel(getnick(from)),"ACTION shoves some mIRC colors up %s\'s ass",getnick(from));
		return;
	}

	nick=get_token(&rest," ");

	if (!userchannel(nick))
	{
		send_to_user(from, "Sorry freak, I only describe to users in my channel");
		return;
	}

	send_ctcp(nick,"ACTION %s",rest);
}

void do_talk(char *from, char *rest, int where)
{
  if (strstr(rest,"hi") || strstr(rest,"hey") || strstr(rest,"hello")
    || strstr(rest,"yo") || strstr(rest,"howdy") || strstr(rest,"re")
	 || strstr(rest,"rehi") || strstr(rest,"hola"))
	  greet(from,where);
  if (strstr(rest,"how's it going"))
	  greet2(from,where);
  if (strstr(rest,"wassup") || strstr(rest,"what's up"))
	  rgreet2(from,where);
}


int checkpasswd(char * from, char *passwd)
{
	char m[300];

	if (userpasswd(from)!=NULL && passwd!=NULL && !check_lock(currentbot->lists->passwdlist,from) &&
		strcmp(userpasswd(from),passwd)==0)
		return 1;
	else
		if (userpasswd(from)!=NULL && !check_lock(currentbot->lists->passwdlist,from) &&
			strcmp(userpasswd(from),currentbot->lockpass)!=0)
		{
			if (userpasswdfail(currentbot->lists->passwdlist,from)==currentbot->failurethreshold)
			{
				globallog(0,LOCKLOG,"%s password was locked from %s for too many failures",user_name(from),
				userpasswd(from));
				lock_passwd(currentbot->lists->passwdlist,from);
				change_passwdlist(currentbot->lists->passwdlist,from,currentbot->lockpass);
				sprintf(m, "Locked - %s: %s gave the wrong password too many times.", get_mask(currentbot->lists->opperlist, from), getnick(from));
				do_bwallop(currentbot->nick, NULL, m);
			}
			return 0;
		}
		else
			return 0;
}

int checkauth(char *from)
{
	CHAN_list	*dummy;
   USER_list	*dummy2;
	char			nick[16];

	if (strcmp(getnick(from),currentbot->nick)==0) return 1;
   if (onefrom && strcmp(from,onefrom)==0)
	{
		free(onefrom);
		onefrom=NULL;
		return 1;
	}

	strcpy(nick, getnick(from));
	if (!userchannel(nick))
	{
		if (currentbot->secure>0)
			return 0;
		else
			return 1;
	}

	dummy=search_chan(userchannel(nick));
   dummy2=search_user(&(dummy->users),nick);

   if (currentbot->secure>0 && dummy2==NULL) return 0;


/*   if ((currentbot->secure & dummy2->auth) | !currentbot->secure) */
   if (((currentbot->secure>0 ? 1 : 0) & dummy2->auth) | (currentbot->secure>0 ? 0 : 1))
	return 1;
   else
	return 0;
}

char *removewild(char *s)
{
  char *s2;
  int i,i2,c;

  s2=strdup(s);
  c=strlen(s2);
  for (i=0;i<c;i++)
    if (s2[i]=='?' || s2[i]=='*')
     {
      for (i2=i;i2<c;i2++)
	s2[i2]=s2[i2+1];
      c--;
      i--;
     }

  return s2;
}

int checkban(char *b)
{
	struct  USERLVL_struct  *p=*(currentbot->lists->protlist);
	char *s;

	while (p!=NULL)
	{
		s=removewild(p->userhost);
		if(p->access>=150 && !mymatch(b, s, FNM_CASEFOLD ) )
			return 0;
		free(s);
		p=p->next;
	}
	return 1;
}

void do_nickch2(void)
{
  int i=rand()%nickcount;
  FILE *f;

  if (curnick!=-1)
	 while (i==curnick) i=rand()%nickcount;

  curnick=i;
  strncpy(currentbot->nick, nicklist[curnick], NICKLEN);
  strncpy(currentbot->realnick, nicklist[curnick], NICKLEN);
  sendnick(currentbot->nick);
  f=fopen("cur.nick","w");
  fprintf(f,"%s\n",nicklist[curnick]);
  fclose(f);
}


void	do_nickch(char *from, char *to, char *rest)
{
	NICK=!NICK;

	if (NICK)
	{
		FILE *f;
		char s[20];

		f=fopen(currentbot->lists->nickfile,"rt");

		if (f==NULL)
		{
			fprintf(stderr,"You don't have an %s file for the nick list",
			currentbot->lists->nickfile);
			return;
		}

		while(fgets(s,20,f))
		{
			nicklist[nickcount]=strdup(s);
			nicklist[nickcount][strlen(nicklist[nickcount])-1]='\0';
			nickcount++;
		}

		fclose(f);
		strcpy(oldnick,currentbot->nick);
		do_nickch2();

		if (rest)
		{
			if (atoi(rest)>19)
				alarms=atoi(rest);
			else
				sendreply("Invalid cycle time using default");
		}

		if (ANICK)
			globallog(1,SYSTEMLOG,"Auto Nick Changes Activated!!!");
		else
		{
			sendreply("Random Nick Changes Activated with cycle time %d",alarms);
			globallog(1,SYSTEMLOG,"%s activated random nick changes with cycle %d",
				from,alarms);
		}
	}
	else
	{
		int i;

		ANICK=0;

		for (i=0;i<nickcount;i++) free(nicklist[i]);

		nickcount=0;
		curnick=-1;
		strncpy(currentbot->nick, oldnick, NICKLEN);
		strncpy(currentbot->realnick, oldnick, NICKLEN);
		sendnick(currentbot->nick);
		alarms=60;
		sendreply("Random Nick Changes Deactivated");
		globallog(1,SYSTEMLOG,"%s deactivated random nick changes",from);
	}
}

void	do_bwallop(char *from, char *to, char *rest)
{
	CHAN_list *c;
	USER_list *u;
	char *channel,message[300],nicks[1024]="";
	int count = 0;

	if (rest==NULL)
		return;

	channel=get_token(&rest," ");
	if (!ischannel(channel))
	{
	  strcpy(message,channel);
	  strcat(message," ");
	  strcat(message,rest);
	  channel=strdup(currentchannel());
	}
	else
	  strcpy(message,rest);


	c=search_chan(channel);

	if (c==NULL)
		return;

	for(u=c->users; u; u=u->next)
	{
		if(usermode(channel,u->nick)&MODE_CHANOP)
		{
			if(count)
				strcat(nicks,",");

			strcat(nicks,u->nick);
			count++;

			if(count == 10)
			{
				send_to_server("NOTICE %s :[BWallOP:%s:%s] %s",nicks,channel,getnick(from),message);
				count = 0;
            nicks[0] = '\0';
			}
		}
	}
	if(count)
		send_to_server("NOTICE %s :[BWallOP:%s:%s] %s",nicks,channel,getnick(from),message);
}

void	do_wallop(char *from, char *to, char *rest)
{
	CHAN_list *c;
	USER_list *u;
	char *channel,message[300],nicks[1024]="";
	int count = 0;

	if (rest==NULL)
	{
		send_to_user(from,"What the hell do you want me to say?");
		return;
	}


	channel=get_token(&rest," ");

	if (!ischannel(channel))
	{
		strcpy(message,channel);
		strcat(message," ");
		strcat(message,rest);
		channel=strdup(currentchannel());
	}
	else
	{
	  strcpy(message,rest);
	}

	c=search_chan(channel);

	if (c==NULL)
	{
		send_to_user(from,"I'm not in that channel freako!");
		return;
	}

	for(u=c->users; u; u=u->next)
	{
		if(usermode(channel,u->nick)&MODE_CHANOP && strcmp(u->nick,getnick(from))!=0)
		{
			if(count)
				strcat(nicks,",");
			strcat(nicks,u->nick);
			count++;
		}

	   if (count == 10)
		{
			send_to_server("NOTICE %s :[WallOP:%s:%s] %s",nicks,channel,getnick(from),message);
			count = 0;
			nicks[0] = '\0';
		}
	}
	if(count)
		strcat(nicks,",");
	strcat(nicks,getnick(from));
	send_to_server("NOTICE %s :[WallOP:%s:%s] %s",nicks,channel,getnick(from),message);
}

void	do_ctalk(char *from, char *to, char *rest)
{
  int level;

	if (rest)
	{
		level=atoi(get_token(&rest," "));

		if (level<0 || level>MAXTALK)
		{
			send_to_user(from,"Invalid talk level");
			return;
		}

		currentbot->talk=level;

		if (currentbot->talk)
			send_to_user(from,"Talking enabled with level %d",currentbot->talk);
		else
			send_to_user(from,"Talking disabled");
	}
	else
	{
		send_to_user(from,"Talk is currently set to %d, valid levels are 0-%d",currentbot->talk,MAXTALK);
		return;
	}

/*	  currentbot->talk=!currentbot->talk; */
}

void	do_chaccess(char *from, char *to, char *rest)
{
	char *command,*level;
   int newlevel,i;
	int changed = 0;

	if ((command=get_token(&rest," "))==NULL)
	{
		send_to_user(from,"What command do you want to change?");
		return;
	}

	if (STRCASEEQUAL(command, "LEVEL25"))
	{
		change_l25_flags(from, to, rest);
		return;
	}

	if (*command==PREFIX_CHAR)
		command++;

	if ((level=get_token(&rest," "))==NULL)
	{
		send_to_user(from,"What do you want to change it to?");
		return;
	}

	newlevel=atoi(level);

#ifdef OLD_USERLEVELS
	if (newlevel<0 || newlevel>150)
#else
	if (newlevel<0 || newlevel>200)
#endif OLD_USERLEVELS
	{
		send_to_user(from,"Invalid level");
		return;
	}

	for(i = 0; on_msg_commands[i].name != NULL; i++)
	{
		if(STRCASEEQUAL(on_msg_commands[i].name, command))
		{
			if(shitlevel(from) > on_msg_commands[i].shitlevel)
			{
				send_to_user(from, "Shitlevel too high");
				return;
			}
			currentbot->commandlvl[i]=newlevel;
			changed = 1;

			send_to_user(from,"Access for %s changed to %d",command,newlevel);
			botlog(SYSTEMLOG,"Access for %s changed to %d by %s",command,newlevel,from);

		}
	}
	if(!changed)
		send_to_user(from, "Invalid command: %s", command);
}

void	do_accesswrite	(char *from, char *to, char *rest)
/*
 * This function makes the current access levels permanent
 * -badcrc 06.07.96
 */
{
	int	i, blah = 0;
	FILE	*af;

	if ((af=fopen("temp.list","wt"))==NULL)
	{
		if(to)
			send_to_user(from, "Cannot open access list for writing", currentbot->lists->accessfile);
		return;
	}

	blah += safe_fprintf(af,"#############################################\n");
	blah += safe_fprintf(af,"## IncuBot %s\n",currentbot->lists->accessfile);
	blah += safe_fprintf(af,"## Created: %s\n",get_ctime(0));
	blah += safe_fprintf(af,"## (c)1997 Incubus\n");
	blah += safe_fprintf(af,"## Format: Command Level\n");
	blah += safe_fprintf(af,"#############################################\n");

	for(i = 0; on_msg_commands[i].name != NULL; i++)
		blah += safe_fprintf(af,"%-15s %d\n",on_msg_commands[i].name,currentbot->commandlvl[i]);

	blah += safe_fprintf(af,"%-15s %d\n", "L25FLAGS", currentbot->l_25_flags);
	blah += safe_fprintf(af,"# End of %s\n",currentbot->lists->accessfile);
	fclose(af);

	if(blah)
	{
		if(to)
				send_to_user(from, "Could not save access list to %s", currentbot->lists->accessfile);
	}
	else
	{
		unlink(currentbot->lists->accessfile);
		if(!rename("temp.list", currentbot->lists->accessfile))
		{
			chmod(currentbot->lists->accessfile, S_IRUSR | S_IWUSR);
			if(to)
				send_to_user(from, "Accesslist written to file %s",currentbot->lists->accessfile);
		}
		else
			if(to)
				send_to_user(from, "Could not save access list to %s", currentbot->lists->accessfile);
	}
}

void	do_fchaccess(char *rest)
{
	char *command,*level;
	int newlevel,i;

	if ((command=get_token(&rest," "))==NULL)
	{
		return;
	}

	if (*command=='#')
		return;

	if ((level=get_token(&rest," "))==NULL)
	{
		return;
	}

	newlevel=atoi(level);

	if (newlevel<0)
	{
		return;
	}

	if(STRCASEEQUAL("L25FLAGS", command))
		currentbot->l_25_flags = newlevel;
	else
	{
		for(i = 0; on_msg_commands[i].name != NULL; i++)
			if(STRCASEEQUAL(on_msg_commands[i].name, command))
			{
				currentbot->commandlvl[i]=newlevel;
			}
	}
}

void do_readaccess(char *from, char *to, char *rest)
{
	FILE *f;
	char s[101];

	f=fopen(currentbot->lists->accessfile,"rt");

	if (f==NULL)
	{
		fprintf(stderr,"You don't have an %s file for the access list",
			currentbot->lists->accessfile);
		return;
	}

	while (!feof(f))
	{
		fgets(s,100,f);
		do_fchaccess(s);
	}

	if (strcmp(from,"bot")!=0) send_to_user(from,"Access list re-read");
		fclose(f);
}


void	do_showlocked(char *from, char *to, char *rest)
{
	struct locked
	{
		char	name[100];
		char	by[100];
		char	passwd[100];
		char	fr[30];
		int	lock;
		struct locked *next;
	};
	FILE *f;
	char s2[200],*s,*t;
	int i;
	struct locked *l=NULL, *n, *temp;
	userlist_t    *dummy=*(currentbot->lists->passwdlist);

	f=fopen(LOCKLOG,"rt");
	if (f==NULL)
	{
		send_to_user(from,"There are no locked users");
		return;
	}

	if (f!=NULL)
	{
		while (fgets(s2,199,f))
		{
			s=strstr(s2,"\n");

			if (s!=NULL)
				*s='\0';

			s=strdup(s2);

			for (i=0;i<6;i++)
				get_token(&s," ");

			t=get_token(&s," ");
			temp=l;

			while (temp!=NULL && strcmp(temp->name,t)!=0)
				temp=temp->next;

			if (strcmp(get_token(&s," "),"unlocked")==0)
			{
				if (temp!=NULL)
				{
					temp->lock=0;
					get_token(&s," ");
					strcpy(temp->by,get_token(&s," "));
				}
			}
			else
			{
				if (temp!=NULL)
					temp->lock=1;
				else
				{
					n=(struct locked *)malloc(sizeof(struct locked));
					strcpy(n->name,t);
					n->lock=1;
					n->next=l;
					l=n;

					if (strcmp(get_token(&s," "),"as")==0)
					{
						for (i=0;i<2;i++)
							get_token(&s," ");

						strcpy(n->by,get_token(&s," "));
						get_token(&s," ");
						strcpy(n->passwd,get_token(&s," "));
						strcpy(n->fr,"owner lock");
					}
					else
					{
						for (i=0;i<2;i++)
							get_token(&s," ");

						strcpy(n->passwd,get_token(&s," "));
						get_token(&s," ");

						if (strcmp(get_token(&s," "),"too")==0)
							strcpy(n->fr,"failed logins");
						else
							strcpy(n->fr,"public use");
					}
				}
			}
		}
		send_to_user(from,"Currently Locked out:");

		for(; dummy; dummy = dummy->next )
		{
			if (strcmp(dummy->passwd,currentbot->lockpass)==0)
				send_to_user(from,"%s is locked out",dummy->userhost);
		}

		send_to_user(from,"Current Lock Log:");

		temp=l;

		while (temp!=NULL)
		{
			l=temp;
			temp=temp->next;

			if (l->lock==0)
				sendreply("%s unlocked by %s",l->name,l->by);
			else
				sendreply("%s locked for %s from %s",l->name,l->fr,l->passwd);

			free( l);
		}
	}
	else
		send_to_user(from,"There is no locked record");

	fclose(f);
}

void	do_secure(char *from, char *to, char *rest)
{
	int	newsecure;

	if (rest == NULL)
	{
		sendreply("Security is currently set to %d, valid levels are 0-%d",currentbot->secure,MAX_SECURE);
		return;
	}

	newsecure = atoi(rest);

	if (newsecure < 0 || newsecure > MAX_SECURE)
	{
		sendreply("%d isn't a valid secure level!",newsecure);
		return;
	}

	currentbot->secure=newsecure;

	sendreply("Security level set to %d",newsecure);
}

void	do_chpasswd(char *from, char *to, char *rest)
{
	char *npasswd;
	char m[300];

	if (checkpasswd(from,get_token(&rest," ")))
	{
		if (!onchannel && !ischannel(to))
		{
			npasswd=get_token(&rest," ");
			if (npasswd)
			{
				change_passwdlist(currentbot->lists->passwdlist,from,npasswd);
				send_to_user(from,"Password successfully changed");
				do_passwdwrite(from,from,from);
			}
			else
				send_to_user(from,"You must specify a new password");
		}
		else
		{
			change_passwdlist(currentbot->lists->passwdlist,from,currentbot->lockpass);
			lock_passwd(currentbot->lists->passwdlist,from);
			send_to_user(from,"You EEEEDIOT! Your account is now LOCKED!");
			globallog(0,LOCKLOG,"%s password was locked from %s for doing it in public",user_name(from),rest);
			sprintf(m, "Locked - %s: %s said his password in public.", get_mask(currentbot->lists->opperlist, from), getnick(from));
			do_bwallop(currentbot->nick, to, m);
		}
	}
	else
		send_to_user(from,"Get lost, bozo!");
}

void	do_passwd(char *from, char *to, char *rest)
{
	char			m[300];	
	CHAN_list	*dummy;
	USER_list	*dummy2;
	int			i, auth = -1;

	if (!userchannel(getnick(from)))
	{
		send_to_user(from,"You're not on channel fool!");
		return;
	}

	if (checkpasswd(from,rest))
	{
		if (!onchannel && !ischannel(to))
		{
			send_to_user(from,"You've been authenticated");

			if ((i=get_userpasswdfail(currentbot->lists->passwdlist,from))>0)
			{
				send_to_user(from,"Your user password has failed %d time(s) clearing failures",i);
				clear_userpasswdfail(currentbot->lists->passwdlist,from);
			}

			auth=1;
		}
		else
		{
			change_passwdlist(currentbot->lists->passwdlist,from,currentbot->lockpass);
			lock_passwd(currentbot->lists->passwdlist,from);
			send_to_user(from,"You EEEEDIOT! Your account is now LOCKED!");
			globallog(0,LOCKLOG,"%s password was locked from %s for doing it in public",user_name(from),rest);
			sprintf(m, "Locked - %s: %s said his password in public.", get_mask(currentbot->lists->opperlist, from), getnick(from));
			do_bwallop(currentbot->nick, to, m);
		}
	}
	else
	{
		auth=0;
		send_to_user(from,"Bite me, you loser");
	}

	if(auth == 0 || auth == 1)
		for(dummy = currentbot->Channel_list; dummy; dummy = dummy->next)
			if((dummy2=search_user(&(dummy->users),getnick(from))) != NULL)
				dummy2->auth = auth;
}


/*void	do_showseen(char *from, char *to, char *rest)
{
  seenlist temp=seen;

  while(temp!=NULL) 
  {
	if (temp->type==4)
		send_to_user(from,seentype[temp->type],temp->nick,temp->kicker,temp->time, temp->soffnick);
	else
		send_to_user(from,seentype[temp->type],temp->nick,temp->time,temp->soffnick);
 	temp=temp->next;
  }
}

void	do_seen(char *from, char *to, char *rest)
{
  seenlist temp;
  char ch[20];
  char *t;
  int i;

  temp=seen;

  if (rest!=NULL)
  {
  t=strdup(rest);
  i=0;
  while (t[i]!=' ' && i<19) i++;
  t[i]='\0';
	for (i=0;i<strlen(t);i++) t[i]=tolower(t[i]);
  while (temp!=NULL && strcmp(t,temp->nick2)!=0) 
	temp=temp->next;

  if (temp==NULL)
  {
    switch(rand()%7)
    { case 1: strcpy(ch,"#gaysm"); break;
      case 2: strcpy(ch,"#lesbian"); break;
		case 3: strcpy(ch,"#gaychub"); break;
      case 4: strcpy(ch,"#dingolovers"); break;
      case 5: strcpy(ch,"#dolphinsex"); break;
      case 6: strcpy(ch,"#doomroom"); break;
      default: strcpy(ch,"#polska"); break;
    }
    send_to_user(from,"Sorry dude, haven't seen %s, check in %s.", t, ch);
  }
  else
    if (temp->type==0)
    send_to_user(from,"Dumb question, %s is right here moron!",temp->nick);
    else
	if (temp->type==4)
		send_to_user(from,seentype[temp->type],temp->nick,temp->kicker,temp->time,temp->soffnick);
	else
		send_to_user(from,seentype[temp->type],temp->nick,temp->time,temp->soffnick);
  }
}
*/

void	do_cycle(char *from, char *to, char *rest)
{
	char *chan;

	if(public_command == TRUE && addressed_to_me == FALSE)
	{
		send_to_user(from, "That command must be addressed to me if used in public.");
		return;
	}
	if (rest==NULL)
	{
		if (userchannel(getnick(from)) != NULL)
			chan = strdup(userchannel(getnick(from)));
		else
			chan = strdup(currentchannel());
	}
	else
	{
		chan = strdup(rest);
	}
	leave_channel(chan);
	join_channel(chan, "", "", TRUE);
	free(chan);
	globallog(0, SYSTEMLOG, "%s made me cycle in %s", from, chan);
}

void	check_ops(void)
{
	CHAN_list	*Channel;
	USER_list	*u;
	char			uname[MAXLEN];
	int 			userlvl;

#ifdef BOTNET
	short			check_botnet=0;
	static long	last_botnet_checktime = 0;

  if ( (time(NULL)-last_botnet_checktime)>BOTNET_CHECK_INTERVAL )
  {
	 last_botnet_checktime = time(NULL);
	 check_botnet=1;
  }
  else
	 check_botnet=0;
#endif

	for (Channel = currentbot->Channel_list; Channel;Channel = Channel->next)
	{
		if (strcmp(Channel->name,"#0")!=0)
		{
			for (u=Channel->users; u!=NULL; u=u->next)
			{
				strcpy(uname,get_username(u));

				if(usermode(Channel->name,u->nick)&MODE_CHANOP)
				{
					userlvl = userlevel(uname);
#ifdef BOTNET
					if (userlvl==OTHER_BOT_LEVEL)
					{
						request_ops(Channel->name,u->nick);

						if (check_botnet)
							check_botnet_server(uname);

						continue;
					}
#endif
					if (currentbot->secure==1)
						continue;
					if(userlvl >= MIN_USERLEVEL_FOR_OPS)
						continue;
					if(userlvl==25 && currentbot->l_25_flags&FL_CANHAVEOPS)
						continue;
					if(STRCASEEQUAL(u->nick,currentbot->nick))
						continue;
					sendmode(Channel->name, "-o %s", u->nick);
				}
			}

			check_channel_bans(Channel->name);
		}
	}
}

void	do_nshowpasswd(char *from, char *to, char *rest)
{
	char	*newuser;
	char	*nuh;		/* nick!@user@host */

	if(userlevel(from) == OTHER_BOT_LEVEL)
		return;

	if((newuser = get_token(&rest, " ")) == NULL)
	{
		send_to_user( from, "Who do you want me to check?" );
		return;
	}

	if((nuh=username(newuser))==NULL)
	{
		send_to_user(from, "%s is not on this channel!", newuser);
		return;
	}


	if(userlevel(from) < userlevel(nuh) && userlevel(from) <= 100)
	{
		send_to_user(from, "Sorry. %s has a higher level", newuser);
		return;
	}

	if (userpasswd(nuh)!=NULL)
	{
		if (strcmp(userpasswd(nuh),currentbot->lockpass)==0)
			send_to_user(from,"%s's account is locked",newuser);
		else
			send_to_user(from,"%s's password is %s",newuser,userpasswd(nuh));
		globallog(2, DONELOG, "%s got %s's password",from,nuh);
	}
	else send_to_user(from,"%s is not a user",newuser);
	  return;
}


void	do_showpasswd(char *from, char *to, char *rest)
{
	char	*newuser;
	userlist_t	*dummy;

	if(userlevel(from) == OTHER_BOT_LEVEL)
		return;

	if((newuser = get_token(&rest, " ")) == NULL)
	{
		send_to_user( from, "Who do you want me to check?" );
		return;
	}

	if(userlevel(from) < userlevel(newuser) && userlevel(from) <= 100)
	{
		send_to_user(from, "Sorry. %s has a higher level", newuser);
		return;
	}

	for( dummy = *currentbot->lists->passwdlist; dummy; dummy = dummy->next )
	{
		if( !mymatch( newuser, dummy->userhost, FNM_CASEFOLD ) )
		{
			if (strcmp(dummy->passwd,currentbot->lockpass)==0)
				send_to_user(from,"%s's account is locked",dummy->userhost);
			else
				send_to_user(from,"%s's password is %s",dummy->userhost,dummy->passwd);

			globallog(2, DONELOG, "%s got %s's password",from,dummy->userhost);
		}
	}
	return;
}


void	do_nlock(char *from, char *to, char *rest)
{
	char	*newuser;
	char	*nuh;		/* nick!@user@host */

	if((newuser = get_token(&rest, " ")) == NULL)
	{
		send_to_user( from, "Who do you want me to lock?" );
		return;
	}
	if((nuh=username(newuser))==NULL)
	{
		send_to_user(from, "%s is not on this channel!", newuser);
		return;
	}
	if (userlevel(nuh)>userlevel(from))
	{
		send_to_user(from,"I don't think so dookie");
		return;
	}

	{
		char	*nick;
		char	*user;
		char	*host;
		char	*domain;
		char	userstr[MAXLEN];

		nick=get_token(&nuh, "!");
		user=get_token(&nuh,"@");
		if(*user == '~' || *user == '#') user++;
		host=get_token(&nuh, ".");
		domain=get_token(&nuh,"");


		sprintf(userstr, "*!*%s@*%s", user, domain?domain:host);
		if (lock_passwd( currentbot->lists->passwdlist,userstr))
		{
						send_to_user( from, "User %s locked as %s",
					newuser, userstr );
					 globallog(0, LOCKLOG, "%s locked as %s by %s from %s",
								  userstr,newuser, from,userpasswd(userstr));
		change_passwdlist(currentbot->lists->passwdlist,userstr,currentbot->lockpass);
		}
		else
		send_to_user(from,"%s is not in the userlist",nick);

	}
		  return;
}


void    do_passwdwrite(char *from, char *to, char *rest)
{
	if(!write_passwdlist(currentbot->lists->passwdlist,currentbot->lists->passwdfile))
		send_to_user(from, "Passwdlist could not be written to file %s",currentbot->lists->passwdfile);
	else
		send_to_user(from, "Passwdlist written to file %s",currentbot->lists->passwdfile);
}

void	do_unlock(char *from, char *to, char *rest)
{
	char	*user;
	char	*p;
	char	passwd[6];
	int i;

	if((user=get_token(&rest," "))==NULL)
	{
 		send_to_user(from, "Who do you want me to unlock?");
		return;
	}

	/* If it doesn't look like an address, find address for a nick */
	if (strstr(user,"!")==NULL)
	{
		char	*userhost;

		if ((userhost=username(user))==NULL)
		{
			sendreply("%s isn't on this channel!",user);
			return;
		}

		user = strdup(get_add_mask(userhost,0));
	}

	if ((p=get_token(&rest," "))==NULL)
	{
		for (i=0;i<5;i++)
			passwd[i]=rand()%26+65;
		passwd[5]='\0';
		p = passwd;
	}
	else if (onchannel)
	{
		send_to_user(from,"Can't unlock account this way in public!");
		return;
	}

	if(unlock_passwd(currentbot->lists->passwdlist, user))
	{
		send_to_user(from, "User %s has been unlocked", user);
		globallog(0, LOCKLOG, "%s unlocked by %s", user, from);
		send_to_user(from,"Password is set to: %s",p);
		change_passwdlist(currentbot->lists->passwdlist,user,p);
	}
	else
		send_to_user(from,"%s is not a user",user);
}

void    do_loglevel(char *from, char *to, char *rest)
{
	int level;

	if(rest)
	{
		level = atoi(rest);

		if(set_loglevel(level))
		{
			send_to_user(from, "Loglevel set to %d", level);
			globallog(0, SYSTEMLOG, "%s set the loglevel to %d", from, level);
		}
		else
			send_to_user(from, "Illegal loglevel %d", level);

	}
	else
		send_to_user(from, "The current loglevel is %d", get_loglevel());
}

char	*userpasswd(char *from)
{
	if(from)
		return(get_passwd(currentbot->lists->passwdlist, from));
	else
		return NULL;
}

char	*usershitreason(char *from)
{
	shitlist_t	*dummy;

	dummy = find_shitmask(currentbot->lists->shitlist, from, 1);
	if(from)
		if(dummy)
		{
			sprintf(tempstring, "%s - %s", dummy->whyshitted, dummy->whenshitted);
			return(tempstring);
		}
	return NULL;
}

char 	*user_name(char *from)
{
	if(from)
		return(get_name(currentbot->lists->opperlist, from));
	else
		return NULL;
}



void ReadLock(userlist_t *user)
{
	userlist_t *p=user;

	while (p)
	{
		if (strcmp(p->passwd,currentbot->lockpass)==0)
			p->lock=1;
		p=p->next;
	}
}

void do_one(char *from, char *to, char *rest)
{
	char *command,*passwd,*s1,*s2,*s3;
	int i;
	DCC_list        *userclient;

	if (rest)
	{
		if ((passwd=get_token(&rest," "))!=NULL)
		{
			if (checkpasswd(from,passwd))
			{
				if (!onchannel)
				{
					send_to_user(from,"You've been authenticated");
					if ((i=get_userpasswdfail(currentbot->lists->passwdlist, from))>0)
					{
						send_to_user(from,"Your user password has failed %d time(s) clearing failures",i);
						clear_userpasswdfail(currentbot->lists->passwdlist,from);
					}
				}
				else
				{
					change_passwdlist(currentbot->lists->passwdlist,from,currentbot->lockpass);
					lock_passwd(currentbot->lists->passwdlist,from);
					send_to_user(from,"You EEEEDIOT! Your account is now LOCKED!");
					globallog(0,LOCKLOG,"%s password was locked from %s for doing it in public",
					user_name(from),passwd);
					return;
				}
			}
			else
			{
				send_to_user(from,"Bite me, you loser");
				return;
			}
		}
		else
		{
			send_to_user(from,"You didn't specify a password");
			return;
		}
	}
	else
	{
		send_to_user(from,"You didn't specify a password");
		return;
	}

	if (rest)
	{
		if ((command=get_token(&rest," "))!=NULL)
		{
			for(i = 0; on_msg_commands[i].name != NULL; i++)
			{
				if(STRCASEEQUAL(on_msg_commands[i].name, command))
				{
					if(userlevel(from) < currentbot->commandlvl[i])
					{
						/*send_to_user(from, "Userlevel too low");*/
						return;
					}
					if(shitlevel(from) > on_msg_commands[i].shitlevel)
					{
						send_to_user(from, "Shitlevel too high");
						return;
					}

					userclient = search_list("chat", from, DCC_CHAT);
					if(on_msg_commands[i].forcedcc && (!userclient || (userclient->flags&DCC_WAIT)))
					{
#ifdef AUTO_DCC
						dcc_chat(from, rest);
						nodcc_session(from, to, rest);
						sendnotice(getnick(from),"Please type: /dcc chat %s",currentbot->nick);
#else
						sendnotice( getnick(from),"Sorry, %s is only available through DCC",command );
						sendnotice( getnick(from),"Please start a dcc chat connection first" );
#endif /* AUTO_DCC */
						return;
					}
					/* if we're left with a null-string (""), give NULL as msg */
					send_to_user(from,"Executing %s %s",command,rest);
					s1=strdup(from);
					s2=strdup(to);
					s3=*rest?strdup(rest):NULL;
					onefrom=strdup(from);
					on_msg_commands[i].function(s1, s2,s3);
					free(s1);
					free(s2);
					free(s3);
					free(onefrom);
					onefrom=NULL;
					return;
				}
			}
		}
		else
		{
			send_to_user(from,"You didn't specify a command to do");
			return;
		}
	}
	else
	{
		send_to_user(from,"You didn't specify a command to do");
		return;
	}
	send_to_user(from,"What the hell command is that...MORON!");
}

