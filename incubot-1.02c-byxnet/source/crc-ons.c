/************************************************************************
 Copyright (c)1996 by Robert Alto (badcrc) ralto@kendaco.telebyte.net
Created..: April 20, 1996
Modified.: Lots.
************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#ifndef SYSV
#include <strings.h>
#endif
#include <ctype.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>

#include "config.h"
#include "channel.h"
#include "chanuser.h"
#include "userlist.h"
#include "misc.h"
#include "fnmatch.h"
#include "send.h"
#include "vladbot.h"
#include "vlad-ons.h"
#include "incu-ons.h"
#include "tb-ons.h"
#include "log.h"
#include "crc-ons.h"

#ifdef BackUp
#include "mega-ons.h"
extern int Abackup;
#endif

#ifdef BackedUp
extern int Abackedup;
#endif
#define	isvalid(c) (((c) >= 'A' && (c) <= '~') || isdigit(c) || (c) == '-')

extern	botinfo *currentbot;
extern	command_tbl	on_msg_commands[];
extern	short   kick_check;

/* !seen global variables */
typedef struct SEEN_struct
{
	char	*nick;
	char	*userhost;
	char	*kicker;
	char	*signoff;
	long	time;
	int	type;
	struct	SEEN_struct	*next;
	struct	SEEN_struct	*prev;
} SEEN_list;

SEEN_list	*firstseen = NULL;
SEEN_list	*lastseen = NULL;
extern char	kicker[70];

/* Local function prototypes */
int		add_spy			(char *);
void		delete_spy		(char *);
SPYUSER_list	*search_spy		(char *);
SEEN_list	*search_seen		(char *);
SEEN_list	*new_seen		(char *, int, char *);
char tempstr[MAXLEN];

void	do_floodprot	(char *from, char *to, char *rest)
{
	char	*type;
	char	*setting;

	if ((type=get_token(&rest," "))==NULL)
	{
		sendreply("Please specify prot to change: CLONE PUBLIC NICK CTCP OP AWP");
		return;
	}

	setting=get_token(&rest," ");
	if(setting && get_fp_typenum(setting) == -1)
	{
		sendreply("\"%s\": invalid setting. Valid settings are: OFF %sBANKICK TBK", setting, STRCASEEQUAL(type, "CLONE")?"":"KICK ");
		return;
	}

	if (STRCASEEQUAL(type,"NICK"))
	{
      if(!setting)
		{
			sendreply("Nick flood prot currently set to: %s",get_fp_typestr(currentbot->floodprot_nick));
			return;
		}
		currentbot->floodprot_nick = get_fp_typenum(setting);
		sendreply("Nick flood prot set to: %s", get_fp_typestr(currentbot->floodprot_nick));
		return;
	}
	else if (STRCASEEQUAL(type,"PUBLIC"))
	{
      if(!setting)
		{
			sendreply("Public flood prot currently set to: %s",get_fp_typestr(currentbot->floodprot_public));
			return;
		}
		currentbot->floodprot_public = get_fp_typenum(setting);
		sendreply("Public flood prot set to: %s", get_fp_typestr(currentbot->floodprot_public));
		return;
	}
	else if (STRCASEEQUAL(type,"CLONE"))
	{
      if(!setting)
		{
			sendreply("Clone bot prot currently set to: %s",get_fp_typestr(currentbot->floodprot_clone));
			return;
		}
		if(get_fp_typenum(setting) == 1)
		{
			sendreply("\"KICK\" is an invalid setting for clone prot");
			return;
		}
		currentbot->floodprot_clone = get_fp_typenum(setting);
		sendreply("Clone bot prot set to: %s", get_fp_typestr(currentbot->floodprot_clone));
		return;
	}
	else if (STRCASEEQUAL(type,"CTCP"))
	{
      if(!setting)
		{
			sendreply("CTCP flood prot currently set to: %s",get_fp_typestr(currentbot->floodprot_ctcp));
			return;
		}
		currentbot->floodprot_ctcp = get_fp_typenum(setting);
		sendreply("CTCP flood prot set to: %s", get_fp_typestr(currentbot->floodprot_ctcp));
		return;
	}
	else if (STRCASEEQUAL(type,"OP"))
	{
      if(!setting)
		{
			sendreply("Public \"!op\" prot currently set to: %s",get_fp_typestr(currentbot->floodprot_op));
			return;
		}
		currentbot->floodprot_op = get_fp_typenum(setting);
		sendreply("Public \"!op\" prot set to: %s", get_fp_typestr(currentbot->floodprot_op));
		return;
	}
	else if (STRCASEEQUAL(type,"AWP"))
	{
      if(!setting)
		{
			sendreply("Public \"!awp\" prot currently set to: %s",get_fp_typestr(currentbot->floodprot_awp));
			return;
		}
		currentbot->floodprot_awp = get_fp_typenum(setting);
		sendreply("Public \"!awp\" prot set to: %s", get_fp_typestr(currentbot->floodprot_awp));
		return;
	}
	else
		sendreply("Unknown flood prot type %s",type);
}

void	do_bk (char *from, char *to, char *rest)
{
	char	*nuh, *nick, chan[256];
	
	if (ischannel(to))
		strcpy(chan, to);
	else
		strcpy(chan, currentchannel());

	if(rest && (nick=get_token(&rest, " ")) && ((nuh = username(nick)) != NULL))
	{
		if(protlevel(username(nick)) >= 100 && (shitlevel(username(nick)) == 0 ||
			excludelevel(username(nick)) > 0) && lvlcheck(from,username(nick)) &&
			(checkauth(username(nick)) || usermode(chan,nick)&MODE_CHANOP))
		{
			send_to_user(from, "%s is protected!", nick);
		}
		else
		{
			ban_user(nuh, chan);

			if (strlen(rest)>0)
				sendkick(chan, nick, rest);
			else
				sendkick(chan, nick, "User requested moron kick");

			add_to_cmdhist(from, chan, "!bk/!bewt %s", nick);
			globallog(2, DONELOG, "%s made me ban-kick %s", from, nick);
		}
	}
	else
		send_to_user(from, "Who!?!");
}

void	do_buy(char *from, char *to, char *rest)
{
	char *buynick, temp[2];

	if ((buynick = get_token(&rest," "))==NULL)
	{
		send_to_user(from, "Who moron, WHO!!??");
		return;
	}

	if (!userchannel(buynick))
	{
		send_to_user(from, "Sorry freak, I only buy for users in my channel");
		return;
	}

	if (strlen(rest)<1)
	{
		send_to_user(from, "What do you want me to buy %s?? sheesh", buynick);
		return;
	}

	temp[0] = 3;
	temp[1] = 0;
	if(strstr(rest, temp))
	{
		send_ctcp(userchannel(getnick(from)),"ACTION shoves some mIRC colors up %s\'s ass",getnick(from));
		return;
	}

	send_ctcp(buynick,"ACTION buys you %s, compliments of %s.",rest,getnick(from));
	send_to_user(from,"\"%s\" sent to %s!!", rest, buynick);
}

void	do_emailadd		(char *from, char *to, char *rest)
{
	char *user;
	char *mailaddr;

	if ((user=get_token(&rest," "))==NULL)
	{
		send_to_user(from, "Who's email address do you want to change?");
		return;
	}

	if ((mailaddr=get_token(&rest," "))==NULL)
	{
		send_to_user(from, "What email address do you want to add for %s?",user);
		return;
	}

	if (!check_email(mailaddr))
	{
		send_to_user(from,"Invalid email address");
		return;
	}

	if (strstr(user,"!")!=NULL)
	{
		if (!change_email(currentbot->lists->opperlist, user, mailaddr))
		{
			send_to_user(from, "%s isn't a user", user);
			return;
		}
	}
	else
	{
		if (username(user)==NULL)
		{
			send_to_user(from, "%s isn't on channel moron!", user);
			return;
		}

		if (!change_email(currentbot->lists->opperlist, username(user), mailaddr))
		{
			send_to_user(from, "Can't update the email address for %s", user);
			return;
		}
	}
	send_to_user(from, "Changed the email address for %s to %s", user, mailaddr);
	if(!write_lvllist(currentbot->lists->opperlist, currentbot->lists->opperfile))
	{
		send_to_user(from, "Userlist could not be written to file %s", 
			currentbot->lists->opperfile);

#ifdef BackedUp
		if (Abackedup)
			send_to_user(currentbot->backupnick,"!comm %s userwrite",
				currentbot->backupbotpasswd);
#endif
	}
	else
		send_to_user(from, "Userlist written to file %s", currentbot->lists->opperfile);
}


void	do_myemail		(char *from, char *to, char *rest)
{
	if (!rest)
	{
		send_to_user(from, "Your email address is [%s], use !myemail <new address> to change it",
			get_email(currentbot->lists->opperlist,from));
		return;
	}

	if (!check_email(rest))
	{
		send_to_user(from, "Invalid email address");
		return;
	}

	if (!change_email(currentbot->lists->opperlist, from, rest))
	{
			send_to_user(from, "Can't change your email address");
			return;
	}

	send_to_user(from, "Changed your email address to %s", rest);

	if(!write_lvllist(currentbot->lists->opperlist, currentbot->lists->opperfile))
	{
		send_to_user(from, "Userlist could not be written to file %s", 
			currentbot->lists->opperfile);

#ifdef BackedUp
		if (Abackedup)
			send_to_user(currentbot->backupnick,"!comm %s userwrite",
				currentbot->backupbotpasswd);
#endif
	}
	else
		send_to_user(from, "Userlist written to file %s", currentbot->lists->opperfile);
}

void	do_nickadd		(char *from, char *to, char *rest)
{
	char *user;
	char *newnick;

	if ((user=get_token(&rest," "))==NULL)
	{
		send_to_user(from, "Who's nickname do you want to change?");
		return;
	}

	if ((newnick=get_token(&rest," "))==NULL)
	{
		send_to_user(from, "What nickname do you want to add for %s?",user);
		return;
	}

	if (!check_nick(newnick))
	{
		send_to_user(from, "Invalid nickname");
		return;
	}

	if (strstr(user,"!")!=NULL)
	{
		if (!change_userlist_nick(currentbot->lists->opperlist, user, newnick))
		{
			send_to_user(from, "%s isn't a user", user);
			return;
		}
	}
	else
	{
		if (username(user)==NULL)
		{
			send_to_user(from, "%s isn't on channel moron!", user);
			return;
		}

		if (!change_userlist_nick(currentbot->lists->opperlist, username(user), newnick))
		{
			send_to_user(from, "Can't update the nickname for %s", user);
			return;
		}
	}
	send_to_user(from, "Changed nickname for %s to %s", user, newnick);

	if(!write_lvllist(currentbot->lists->opperlist, currentbot->lists->opperfile))
	{
		send_to_user(from, "Userlist could not be written to file %s",
			currentbot->lists->opperfile);

#ifdef BackedUp
		if (Abackedup)
			send_to_user(currentbot->backupnick,"!comm %s userwrite",
				currentbot->backupbotpasswd);
#endif
	}
	else
		send_to_user(from, "Userlist written to file %s", currentbot->lists->opperfile);
}

void	do_mynick		(char *from, char *to, char *rest)
{
	if (!rest)
	{
		send_to_user(from, "Your nickname is set to [%s], use !mynick <new nick> to change it",
			get_userlist_nick(currentbot->lists->opperlist,from));
		return;
	}

	if (!check_nick(rest))
	{
		send_to_user(from, "Invalid nickname");
		return;
	}

	if (!change_userlist_nick(currentbot->lists->opperlist, from, rest))
	{
			send_to_user(from, "Can't change your nickname");
			return;
	}

	send_to_user(from, "Changed your nickname to %s", rest);

	if(!write_lvllist(currentbot->lists->opperlist, currentbot->lists->opperfile))
	{
		send_to_user(from, "Userlist could not be written to file %s", 
			currentbot->lists->opperfile);

#ifdef BackedUp
		if (Abackedup)
			send_to_user(currentbot->backupnick,"!comm %s userwrite",
				currentbot->backupbotpasswd);
#endif
	}
	else
		send_to_user(from, "Userlist written to file %s", currentbot->lists->opperfile);
}


void	do_passwdadd		(char *from, char *to, char *rest)
{
	int	ul;
	char	*passwd;
	char	*user;

	if(userlevel(from) == OTHER_BOT_LEVEL)
		return;

	if (ischannel(to))
	{
		send_to_user(from,"Please don't be retarded and use this command in public!");
		return;
	}

	if ((user=get_token(&rest," "))==NULL)
	{
		send_to_user(from,"Who's password do you wanna change?");
		return;
	}

	if ((passwd=get_token(&rest," "))==NULL)
	{
		send_to_user(from,"What password do you wanna give %s?",user);
		return;
	}

	if (strlen(passwd)<5)
	{
		send_to_user(from,"Get real dude, gimme a valid password");
		return;
	}

	if ((ul = userlevel(user))==0)
	{
		send_to_user(from,"%s isn't a user freako!",user);
		return;
	}

	if (ul > userlevel(from) && userlevel(from) < 150)
	{
		send_to_user(from,"Sorry, %s has a higher userlevel",user);
		return;
	}

	if(!add_to_passwdlist(currentbot->lists->passwdlist,user,passwd))
	{
		send_to_user(from,"Hmm..it appears the mask %s isn't the same as the one in the password list",
			user);
		return;
	}

	send_to_user(from,"Password for %s changed to %s",user,passwd);
	do_passwdwrite(from,to,rest);
        globallog(2, DONELOG, "%s changed the password entry for %s",from,user);
	add_list_event("%s changed the password entry for %s",from,user);
}

void	do_passwddel	(char *from, char *to, char *rest)
{
	char *userhost;

	if(userlevel(from) == OTHER_BOT_LEVEL)
		return;

	if ((userhost=get_token(&rest," "))==NULL)
	{
		sendreply("Who's password entry do you want to delete?");
		return;
	}

	if (userlevel(userhost)>userlevel(from) && userlevel(from) < 150)
	{
		sendreply("Sorry, %s has a higher userlevel",userhost);
		return;
	}

        if (!remove_from_levellist(currentbot->lists->passwdlist,userhost))
	{
                sendreply("%s has no password!", userhost);
		return;
	}

	do_passwdwrite(from,to,userhost);
        globallog(2, DONELOG, "%s deleted the password entry for %s",from,userhost);
	add_list_event("%s deleted the password entry for %s",from,userhost);
}

void	show_password_list	(char *from, char *to, char *rest)
{
	send_to_user(from, "%s Password List", currentbot->nick);
	send_to_user(from, "-------+----------------------------------------");
	send_to_user(from, "       | %s", "nick!user@host");
	send_to_user(from, "-------+----------------------------------------");
	show_lvllist_pattern("   ",
		currentbot->lists->passwdlist, from, rest?rest:"", 4);
	send_to_user(from, "End of Password List");
}

void	do_passwdop		(char *from, char *to, char *rest)
{
	char *passwd;

	if ((passwd=get_token(&rest," "))==NULL)
	{
		send_to_user(from,"You must specify a password");
		return;
	}

	do_passwd(from,to,passwd);

	if (checkauth(from))
		do_op(from,to,rest);
}

void	show_userinfo		(char *from, char *to, char *rest)
{
	char		org_msg[MAXLEN];
	char		*srchstr;
	DCC_list	*userclient;
	int		num_matches;
	int		retval;
	int		type = 0;

	if ((srchstr=get_token(&rest," "))==NULL)
	{
		send_to_user(from,"What string do you want to look for?");
		return;
	}

	if (STRCASEEQUAL(srchstr,"-email"))
		type = 1;
	else if (STRCASEEQUAL(srchstr,"-mask"))
		type = 2;

	if(type)
		if ((srchstr=get_token(&rest," "))==NULL)
		{
			send_to_user(from,"What string do you want to look for?");
			return;
		}

	if (STRCASEEQUAL(rest,"all"))
	{
		userclient = search_list("chat", from, DCC_CHAT);

		if(!userclient || (userclient->flags&DCC_WAIT))
		{
			num_matches = num_userlist_matches(currentbot->lists->opperlist, srchstr, 1);

			if(num_matches > 6)
			{
				if(type == 0)
					sprintf(org_msg, "!userinfo %s all", srchstr);
				if(type == 1)
					sprintf(org_msg, "!userinfo -email %s all", srchstr);
				if(type == 2)
					sprintf(org_msg, "!userinfo -mask %s all", srchstr);
#ifdef AUTO_DCC
				dcc_chat(from, rest);
				nodcc_session(from, to, org_msg);

#else
				sendnotice( getnick(from),
					"Sorry, \"%s\" is only available through DCC",	org_msg);
				sendnotice( getnick(from),	"Please start a dcc chat connection first" );
#endif /* AUTO_DCC */
				return;
			}
		}
		retval = get_userlist_info(currentbot->lists->opperlist, from, srchstr, type+3);
	}
	else
		retval = get_userlist_info(currentbot->lists->opperlist, from, srchstr, type);

	if (!retval)
	{
		send_to_user(from, "Can't find a match to string \"%s\"", srchstr);
		return;
	}
}


void notify_excludeadd(char *newuser, char *from)
{
	add_list_event("%s was excluded by %s", newuser, from);
}

void notify_excludedel(char *newuser, char *from)
{
	add_list_event("%s was un-excluded by %s", newuser, from);
}

void notify_shitadd(char *newuser, char *userstr,  char *newlevel, char *from, char *reason)
{
	add_list_event("%s was shitted as %s with level %s by %s for %s", newuser, userstr, newlevel, from, reason);
}

void	notify_useradd(char *newuser, char *userstr, char *newlevel, char *from)
{
	add_list_event("%s was added as %s with level %s by %s", newuser, userstr, newlevel, from);
}

void	notify_expireadd(char *newuser, long expiredate, char *from)
{
	add_list_event("%s was set to expire on %s by %s", newuser, time2str(expiredate), from);
}

void	notify_expiredel(char *deleteduser, char *from)
{
	add_list_event("Expiration date for %s was removed by %s", deleteduser, from);
}

void	notify_shitdel(char *deleteduser, char *from)
{
	add_list_event("%s was unshitted by %s", deleteduser, from);
}

void	notify_userdel(char *deleteduser, char *from)
{
	add_list_event("%s was deleted by %s", deleteduser, from);
}

void	add_list_event	(char *format, ...)
{
	FILE		*eventfile;
	char		buf[WAYTOBIG];
	int      firstevent;
	va_list	msg;

	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);

	if (access(currentbot->eventfile,0) != 0)
		firstevent = 1;
	else
		firstevent = 0;

	if ((eventfile=fopen(currentbot->eventfile,"at"))==NULL)
		return;

	if(firstevent)
	{
		fprintf(eventfile, "Eventlist for %s\n\n", currentbot->nick);
	}

	fprintf(eventfile,"[%s] %s\n", time2str(time(NULL)), buf);
	fclose(eventfile);
}

void	do_mail_list_events	(char *from, char *to, char *rest)
{
	mail_list_events();
}

void	mail_list_events	(void)
{
	char	buffer[400];

	/* if the file doesn't exist, don't do anything */
	if (access(currentbot->eventfile,0) != 0)
		return;

	sprintf(buffer,"./maillog %s %s %s &",
		currentbot->nick, currentbot->eventfile, currentbot->mailtofile);

	system(buffer);
	currentbot->last_event_mail = time(NULL);
}

void	do_mailpasswd	(char *from, char *to, char *rest)
{
	char	buffer[300];
	char	*user;
	char	*mailaddr;

	if (userlevel(from)==OTHER_BOT_LEVEL)
		return;

	if (!rest)
	{
		send_to_user(from, "Who's password do you want me to mail?");
		return;
	}

	if (strstr(rest,"!")!=NULL)
	{
		if ((mailaddr = get_email(currentbot->lists->opperlist,rest))==NULL)
		{
			send_to_user(from,"%s isn't a user",rest);
			return;
		}
		user = rest;
	}
	else
	{
		if ((user=username(rest))==NULL)
		{
			send_to_user(from, "%s isn't on channel moron!", rest);
			return;
		}

		if ((mailaddr = get_email(currentbot->lists->opperlist,user))==NULL)
		{
			send_to_user(from,"Can't find email address for %s",user);
			return;
		}
	}

	if (STRCASEEQUAL("None",mailaddr) || STRCASEEQUAL("NULL",mailaddr))
	{
		send_to_user(from,"%s doesn't have a valid email address",user);
		return;
	}

	sprintf(buffer, "echo \"Yo, %s, %s requested that I mail your password, so here it is: %s. cya on IRC --%s\" | mail -s \"You\'re not authenticated!\" %s &",
		getnick(user),from,userpasswd(user), currentbot->nick, mailaddr);
	system(buffer);

	send_to_user(from,"%s's password was mailed to %s",user,mailaddr);
	add_list_event("%s requested %s's password be mailed to %s", from, user, mailaddr);
}


void	do_showemail	(char *from, char *to, char *rest)
{
	char *mailaddr;
	char *nick;
	char *user;

	if (!rest)
	{
		send_to_user(from, "Who's email address do you want see?");
		return;
	}

	if (strstr(rest,"!")!=NULL)
	{
		if ((mailaddr = get_email(currentbot->lists->opperlist,rest))==NULL)
		{
			send_to_user(from,"%s isn't a user",rest);
			return;
		}

		if ((nick=get_userlist_nick(currentbot->lists->opperlist,rest))==NULL)
			nick = "No nick set";

		send_to_user(from,"%s's email address is %s (Nick: %s)",rest,mailaddr,nick);
		return;
	}

	if ((user=username(rest))==NULL)
	{
		send_to_user(from, "%s isn't on channel moron!", rest);
		return;
	}

	if ((mailaddr = get_email(currentbot->lists->opperlist,user))==NULL)
	{
		send_to_user(from,"Can't find email address for %s",user);
		return;
	}

	if ((nick=get_userlist_nick(currentbot->lists->opperlist,user))==NULL)
		nick = "No nick set";

	send_to_user(from,"%s's email address is %s (Nick: %s)",rest,mailaddr,nick);
}

void	do_nuserdel		(char *from, char *to, char *rest)
/********************
Author.: badcrc
Date...: 02/12/96
Purpose: delete a user based on nick
*********************/
{
	char	*nuh;
	char	*tempmask;
	char	userstr[MAXLEN];
	int	inlist=0;

	if (userlevel(from)==OTHER_BOT_LEVEL)
		return;

	if (!rest)
	{
		send_to_user(from, "Who do you want me to delete?");
		return;
	}

	if((nuh=username(rest))==NULL)
	{
		send_to_user(from, "%s is not on this channel!", rest);
		return;
	}

	if (userlevel(from) < userlevel(nuh) && userlevel(from) < 150)
	{
		sendreply("%s has a higer level.. sorry", rest);
		return;
	}

	if((tempmask = get_mask(currentbot->lists->opperlist, nuh)) == NULL)
	{
		sendreply("%s is not a user.",rest);
		return;
	}
	strcpy(userstr,tempmask);

	if (remove_from_levellist(currentbot->lists->passwdlist, userstr))
		inlist++;
	else
		sendreply("%s has no password!", userstr);

	if (remove_from_levellist(currentbot->lists->opperlist, userstr))
	{
		sendreply("User %s has been deleted", userstr);
		inlist++;
	}
   	else
		sendreply("%s has no level!",userstr);

	if (remove_from_levellist(currentbot->lists->protlist, userstr))
	{
		sendreply("User %s has been unprotected", userstr);
		inlist++;
	}

	globallog(2, DONELOG, "%s deleted by %s", userstr, from);

	if (inlist)
		notify_userdel(userstr,from);

#ifdef BackedUp
	if (Abackedup)
		send_to_user(currentbot->backupnick,"!comm %s nuserdel %s",
			currentbot->backupbotpasswd,userstr);
#endif
#ifdef TB_MSG
	DelUser(from);
#endif
}

/********************
Author.: badcrc
Date...: 02/12/96
Purpose: delete a user's prot based on nick
*********************/
void    do_nprotdel(char *from, char *to, char *rest)
{
	char	*nuh;
	char	*tempmask;

	if(userlevel(from) == OTHER_BOT_LEVEL)
		return;

	if(!rest)
	{
		send_to_user(from, "Who do you want me to unprotect?");
		return;
	}

	if((nuh=username(rest))==NULL)
	{
		send_to_user(from, "%s is not on this channel!", rest);
		return;
	}

	if((tempmask = get_mask(currentbot->lists->protlist, nuh)) == NULL)
	{
		send_to_user(from, "%s is not protected!",rest);
		return;
	}

	if(userlevel(nuh) > userlevel(from) && userlevel(from) < 125)
	{
		send_to_user(from, "Sorry. %s has a higher level", rest);
		return;
	}

	if(!remove_from_levellist(currentbot->lists->protlist, tempmask))
	{
		send_to_user(from, "%s is not protected!", tempmask);
		return;
	}

	send_to_user(from, "User %s has been unprotected", tempmask);
	globallog(2, DONELOG, "%s unprotected by %s", tempmask, from);

#ifdef BackedUp
	if (Abackedup)
		send_to_user(currentbot->backupnick,"!comm %s nprotdel %s",
			currentbot->backupbotpasswd,tempmask);
#endif
}

void	do_excludeadd	(char *from, char *to, char *rest)
{
	char	*newuser;

	if (userlevel(from)==OTHER_BOT_LEVEL)
		return;

	if((newuser = get_token(&rest, " ")) == NULL)
	{
		send_to_user( from, "Who do you want me to exclude?" );
		return;
	}

	if (!add_to_excludelist(currentbot->lists->excludelist, newuser))
	{
		sendreply("There is already a record that matches %s",newuser);
		return;
	}

	send_to_user(from, "User mask %s has been excluded",newuser);

#ifdef BackedUp
		if (Abackedup)
	         send_to_user(currentbot->backupnick,"!comm %s excludeadd %s %d",
                        currentbot->backupbotpasswd,newuser);
#endif

	globallog(2, DONELOG, "%s excluded by %s", newuser, from);
	notify_excludeadd(newuser, from);
}                   

void	do_excludedel(char *from, char *to, char *rest)
{
	char	*deluser;

	if (userlevel(from)==OTHER_BOT_LEVEL)
		return;

	if((deluser = get_token(&rest, " ")) == NULL)
	{
		send_to_user( from, "Who do you want me to un-exclude?" );
		return;
	}

	if (!remove_from_excludelist(currentbot->lists->excludelist,deluser))
	{
		send_to_user(from, "%s isn't excluded!", deluser);
		return;
	}

	send_to_user(from, "User mask %s has been un-excluded",deluser);

#ifdef BackedUp
		if (Abackedup)
	         send_to_user(currentbot->backupnick,"!comm %s excludedel %s %d",
                        currentbot->backupbotpasswd,deluser);
#endif

	globallog(2, DONELOG, "%s un-excluded by %s", deluser, from);
	notify_excludedel(deluser, from);
}                   


void    do_excludewrite(char *from, char *to, char *rest)
{
	if(!write_excludelist(currentbot->lists->excludelist, currentbot->lists->excludefile))
	{
		send_to_user(from, "Excludelist could not be written to file %s", 
			currentbot->lists->excludefile);

#ifdef BackedUp
		if (Abackedup)
			send_to_user(currentbot->backupnick,"!comm %s excludewrite",
				currentbot->backupbotpasswd);
#endif
	}
	else
			send_to_user(from, "Excludelist written to file %s", currentbot->lists->excludefile);
}

void	do_exunban	(char *from, char *to, char *rest)
{
	char *channel;

	if (excludelevel(from)==0)
		return;

	if (!rest || !ischannel(rest))
	{
		channel = currentchannel();
	}
	else if (search_chan(rest)==NULL)
	{
		send_to_user(from, "I'm not in that channel freako!");
		return;
	}
	else
		channel = rest;

	send_to_user(from,"Executing unban in channel %s",channel);
	channel_unban(channel, from);
	add_to_cmdhist(from, channel, "!exunban");
	add_list_event("%s requested exclude-unban in %s", from, channel);
}

void	show_exclude_list	(char *from, char *to, char *rest)
{
	send_to_user(from, "%s Exclude List", currentbot->nick);
	send_to_user(from, "-------+----------------------------------------");
	send_to_user(from, "       | %s", "nick!user@host");
	send_to_user(from, "-------+----------------------------------------");
	show_excludelist_pattern("       | ",
		currentbot->lists->excludelist, from, rest?rest:"");
	send_to_user(from, "End of Exclude List");

}

void	show_email_list	(char *from, char *to, char *rest)
{
	send_to_user(from, "%s Email List", currentbot->nick);
	send_to_user(from, "----------------------+---------------------------------------------------");
	send_to_user(from, " %20s | %-28s", "Nickname", "Email Address");
	send_to_user(from, "----------------------+---------------------------------------------------");
	show_lvllist_pattern(" ", currentbot->lists->opperlist, from, rest?rest:"", 3);
	send_to_user(from,"End of Email List");
}

void    show_whois_excluded (char *from, char *to, char *rest)
{
	if(rest == NULL)
	{
		send_to_user(from, "Please specify a user mask");
		return;
	}

	if (!show_excludelist_pattern( "EXCLUDED: ", currentbot->lists->excludelist, from, rest))
		send_to_user(from, "EXCLUDED: %s is not a user", rest);
}

void	show_serverlist	(char *from, char *to, char *rest)
{
	int i;

	send_to_user(from,"%s Server List",currentbot->nick);
	send_to_user(from,"-----+-------------------------------------");

	for (i=0; i<currentbot->num_of_servers; i++)
	{
		send_to_user(from," %3d | %s:%d",
			i, currentbot->serverlist[i].name,
			currentbot->serverlist[i].port);
	}
	send_to_user(from,"End of Server List");
}

void	do_massnk(char *from, char *to, char *rest)
{
	char    *op_pattern;
	char	*chan;

	if((op_pattern = get_token(&rest," ")) == NULL)
	{
		send_to_user(from,"Please specify a pattern" );
		return;
	}

	if ((chan = get_token(&rest," "))==NULL || !ischannel(chan) || search_chan(chan)==NULL)
		chan = (ischannel(to) ? to : currentchannel());

	channel_mass_nk(from, chan, op_pattern);
	globallog(2, DONELOG, "%s made me mass non-op kick %s in %s", from, op_pattern, chan);
	add_to_cmdhist(from, chan, "!massnk %s", op_pattern);
	add_list_event("%s made me mass non-op kick %s in %s", from, op_pattern, chan);
	return;
}

/*********************************
Author..: badcrc
Date....: 4/19/96
Purpose.: Kicks all users who aren't in the userlist
**********************************/
void    channel_mass_nk		(char *from, char *channel, char *pattern)
/*
 * Incubus mods: Added code to keep the bot from flooding itself off.
 */
{
	CHAN_list       *Channel;
	USER_list       *Dummy;
	int				NumKicks=0;
	int				numkicks=0;
	unsigned long	Last_Kick=time(NULL);

	if( (Channel = search_chan( channel )) == NULL )
		return;

	sendmode(channel,"+i");

	Dummy = Channel->users;

	while(Dummy)
	{
		if(!mymatch(pattern, get_username(Dummy), FNM_CASEFOLD))
			if(userlevel(get_username(Dummy)) == 0 || (userlevel(get_username(Dummy)) == 25 &&
				(currentbot->l_25_flags & FL_NOMASSNK) == 0))
			{
				sendkick(channel, get_usernick(Dummy), "**Mass Non-Op kick**");
				NumKicks++;
				numkicks++;
			}

		while(numkicks >= MAX_KICKS)
		{
			if((time(NULL)-Last_Kick) > MAX_KICK_INTERVAL)
				numkicks = 0;
		}
		Dummy=get_nextuser(Dummy);
	}

	send_to_user(from,"%d non-ops were kicked that matched %s",NumKicks,pattern);
}

void    do_showcmdaccess(char *from, char *to, char *rest)
{
	char *cmd, *ptr;
	int i;

	if (!rest)
	{
		send_to_user(from,"Show access for what command moron!!??");
		return;
	}

	cmd = strdup(rest);
	ptr = cmd;

	if (*cmd==PREFIX_CHAR)
		cmd++;

	for(i = 0; on_msg_commands[i].name != NULL; i++)
	{
		if (STRCASEEQUAL(cmd,on_msg_commands[i].name))
		{
			if(shitlevel(from) <= on_msg_commands[i].shitlevel)
				send_to_user(from,"%s: %d",on_msg_commands[i].name,currentbot->commandlvl[i]);

			free(ptr);
			return;
		}
	}

	send_to_user(from,"%s isn't a valid command freak, quit messing with me!", cmd);
	free(ptr);
}

void	request_ops	(char *channel, char *nick)
{
	/* If the user is another bot and our bot isn't opped already */
	if(userlevel(username(nick))==OTHER_BOT_LEVEL && !(usermode(channel,currentbot->nick)&MODE_CHANOP))
	{
		msend_to_user(username(nick),"passwdop %s %s", userpasswd(username(currentbot->nick)), channel);
/*		sendprivmsg(nick,"op %s",channel);*/
	}
}

void	reop_after_deop	(char *channel)
{
	CHAN_list *c;
	USER_list *u;

	if (strcmp(channel,"#0")!=0)
	{
        c=search_chan(channel);
		u=c->users;

		while (u!=NULL)
		{
			if((usermode(channel,u->nick)&MODE_CHANOP) &&
				userlevel(get_username(u))==OTHER_BOT_LEVEL) 
			{
				request_ops(channel,u->nick);
			}
			u=u->next;
		}
	}
}

void	show_nethacks	(char *from, char *to, char *rest)
{
	char buffer[MAXLEN];
	char datebuff[20];
	char *currtime, *ptr;
	char *Mo;
	char *Dom;
	int NumHacks=0;
	FILE *logfile;

	if((logfile = fopen( DONELOG, "rt")) == NULL)
	{
		send_to_user(from,"No nethacks are logged");
		return;
	}

	currtime = strdup(time2str(time(NULL)));
	ptr = currtime;
	Mo = get_token(&currtime," ");
	Mo = get_token(&currtime," ");
	Dom = get_token(&currtime," ");

	sprintf(datebuff,"%s %s",Mo,Dom);

	while ( fgets(buffer,sizeof(buffer),logfile)!=NULL )
	{
		if (strstr(buffer,"nethack")!=NULL &&
			(rest || strstr(buffer,datebuff)!=NULL))
		{
			if ((buffer[strlen(buffer)-1]='\n'))
				buffer[strlen(buffer)-1]='\0';

			send_to_user(from,"%s",buffer);
			NumHacks++;
		}
	}

	fclose(logfile);

	if (NumHacks==0)
		send_to_user(from,"No nethacks are logged");
	else
		send_to_user(from,"%d nethacks logged",NumHacks);
	free(ptr);
}

/*********************************
Author..: badcrc
Date....: 2/14/96
Purpose.: Toggles the "spy" command on/off
**********************************/
void	do_cspy(char *from, char *to, char *rest)
{
	if (search_spy(from))
	{
		delete_spy(from);
		send_to_user(from,"Spy disabled");
	}
	else
	{
		if (add_spy(from))
			send_to_user(from,"Spy enabled");
		else
			send_to_user(from,"Can't enable spy mode");
	}
}

/*********************************
Author..: badcrc
Date....: 2/21/96
Purpose.: Toggles the "spylog" command on/off
**********************************/
void	do_cspylog(char *from, char *to, char *rest)
{
	if (currentbot->spylog)
	{
		currentbot->spylog=0;
		send_to_user(from,"Spy logging disabled");
	}
	else
	{
		currentbot->spylog=1;
		send_to_user(from,"Spy logging enabled");
	}
}

void	do_spy		(char *from, char *to, char *msg)
{
	SPYUSER_list	*ptrspy1;
	SPYUSER_list	*ptrspy2;

	/* Only need to spy on private msgs to the bot */
	if(ischannel(to))
		return;

	ptrspy1 = currentbot->Spyuser_list;

	while (ptrspy1 != NULL)
	{
		/* If we have a DCC chat with the spy, send msg to him */
		if (search_list("chat",ptrspy1->spyuser,DCC_CHAT))
		{
			send_to_user(ptrspy1->spyuser,"[Spy:%s] (%s) (%s)",to, from, msg);
			ptrspy1 = ptrspy1->next;
		}
		/* If we don't have a chat with the spy, disable spy mode */
		else
		{
			send_to_user(ptrspy1->spyuser,"Spy disabled");
			ptrspy2 = ptrspy1->next;
			delete_spy(ptrspy1->spyuser);
			ptrspy1 = ptrspy2;
		}
	}
}

void	do_spylog	(char *from, char *to, char *msg)
{
	char	buff[100];
	char	*msg2, *ptr;
	int		i;
	FILE	*spyfile;

	msg2 = strdup(msg);
	ptr = msg2;
	sprintf(buff,"%s %c", currentbot->nick,PREFIX_CHAR);

	for (i=0;i<strlen(buff);i++)
		buff[i]=tolower(buff[i]);

	for (i=0;i<strlen(msg2);i++)
		msg2[i]=tolower(msg2[i]);

	/* If it was a command to the bot */
	if (msg2[0]==PREFIX_CHAR || strstr(msg2,buff)!=NULL || !ischannel(to))
	{
		if ((spyfile=fopen("spy.log","at"))==NULL)
		{
			free(ptr);
			return;
		}

		fprintf(spyfile,"[%s] (%s:%s) (%s)\n", time2str(time(NULL)), to, from, msg);
		fclose(spyfile);
	}
	free(ptr);
}

/********************************
Author.: badcrc
Date...: 04/19/96
Purpose: Adds a spy user to the list
********************************/
int		add_spy		(char *from)
{
	SPYUSER_list	*ptrspy1;
	SPYUSER_list	*ptrspy2;

	if ( (ptrspy2 = (SPYUSER_list*)malloc(sizeof(*ptrspy2)) ) == NULL)
		return FALSE;

	mstrcpy(&ptrspy2->spyuser,from);
	ptrspy2->next = NULL;
	ptrspy1 = currentbot->Spyuser_list;

	if (ptrspy1 == NULL)
	{
		currentbot->Spyuser_list = ptrspy2;
	}
	else
	{
		while (ptrspy1->next != NULL)
			ptrspy1 = ptrspy1->next;

		ptrspy1->next = ptrspy2;
	}

	return TRUE;
}

/********************************
Author.: badcrc
Date...: 04/19/96
Purpose: Removes a spy user from the list
********************************/
void	delete_spy	(char *from)
{
	SPYUSER_list	*ptrspy1;
	SPYUSER_list	*ptrspy2;

	ptrspy1 = NULL;
	ptrspy2 = currentbot->Spyuser_list;

	while (ptrspy2)
	{
		if (STRCASEEQUAL(ptrspy2->spyuser,from))
		{
			if (ptrspy1 == NULL)
			{
				currentbot->Spyuser_list = ptrspy2->next;
				free(ptrspy2->spyuser);
				free(ptrspy2);
				ptrspy2 = currentbot->Spyuser_list;
			}
			else
			{
				ptrspy1->next = ptrspy2->next;
				free(ptrspy2->spyuser);
				free(ptrspy2);
				ptrspy2 = ptrspy1->next;
			}
		}
	}
}

/********************************
Author.: badcrc
Date...: 04/19/96
Purpose: Finds a spy user
********************************/
SPYUSER_list	*search_spy	(char *from)
{
	SPYUSER_list	*ptrspy;

	for (ptrspy = currentbot->Spyuser_list; ptrspy; ptrspy = ptrspy->next)
		if (STRCASEEQUAL(ptrspy->spyuser,from))
			return (ptrspy);

	return NULL;
}

void		addseen		(char *nick, int type, char *rest)
{
	char		*uname;
	SEEN_list	*seenptr;
	static int	seencount = 0;

	/* if there is no one in the seen list yet */
	if (firstseen==NULL)
	{
		seenptr = new_seen(nick,type,rest);
		firstseen = seenptr;
		lastseen = seenptr;
		seencount++;
	}
	/* if there is already that nick in the seen list, re-use it */
	else if ((seenptr = search_seen(nick))!=NULL)
	{
		free(seenptr->userhost);
		free(seenptr->signoff);
		free(seenptr->kicker);

		if ((uname=username(nick))==NULL)
			mstrcpy(&seenptr->userhost,nick);
		else
			mstrcpy(&seenptr->userhost,uname);
	
		seenptr->type = type;

		if (type == 4)
			mstrcpy(&seenptr->kicker,kicker);
		else
			seenptr->kicker = NULL;

		mstrcpy(&seenptr->signoff,rest);
		seenptr->time=time(NULL);
	}
	else
	{
		/* if we're at the max number of seen records we should
			use, remove oldest */
		if (seencount >= MAX_SEEN)
		{
			/* make a marker to the 2nd to last seen record */
			seenptr = lastseen->prev;
			seenptr->next = NULL;

			/* delete the oldest seen record */
			free(lastseen->nick);
			free(lastseen->userhost);
			free(lastseen->signoff);
			free(lastseen->kicker);
			free(lastseen);

			/* re-point lastseen to the new last record */
			lastseen = seenptr;
			seencount--;
		}

		/* Make our new record and point it to the old first record */
		seenptr = new_seen(nick,type,rest);
		seenptr->next = firstseen;

		/* re-point firstseen to the new first record */
		firstseen->prev = seenptr;
		firstseen = seenptr;
		seencount++;
	}
}

SEEN_list	*search_seen	(char *nick)
{
	SEEN_list	*seenptr;

	seenptr = firstseen;

	while (seenptr!=NULL && mymatch(nick, seenptr->nick, FNM_CASEFOLD))
		seenptr = seenptr->next;

/*	free(ptr);
*/
	return(seenptr);
}

SEEN_list	*new_seen		(char *nick, int type, char *rest)
{
	SEEN_list	*seenptr;
	char	*uname;
	int		i;

	/* create a new seen record */
	if ( (seenptr = (SEEN_list*)malloc(sizeof(*seenptr)) ) == NULL)
		return (NULL);

	mstrcpy(&seenptr->nick,nick);

	for (i=0;i<strlen(seenptr->nick);i++)
		seenptr->nick[i]=tolower(seenptr->nick[i]);

	if ((uname=username(nick))==NULL)
		mstrcpy(&seenptr->userhost,nick);
	else
		mstrcpy(&seenptr->userhost,uname);

	seenptr->type = type;

	if (type == 4)
		mstrcpy(&seenptr->kicker,kicker);
	else
		seenptr->kicker = NULL;

	mstrcpy(&seenptr->signoff,rest);
	seenptr->time=time(NULL);
	seenptr->next = NULL;
	seenptr->prev = NULL;

	return(seenptr);
}

void		do_showseen	(char *from, char *to, char *rest)
{
	SEEN_list	*seenptr;
	char			tempmask[512];
	char			*nick;
	int			domatch = 0;
	int			matchnick = 1;
	int			gotmatch;

	nick=get_token(&rest," ");

	if(nick)
	{
		if(STRCASEEQUAL(nick, "-mask"))
		{
			if((nick = get_token(&rest, " ")) == NULL)
			{
				send_to_user(from, "Must supply a mask");
				return;
			}
			matchnick = 0;
		}
		domatch = 1;
	}

	for(seenptr = firstseen; seenptr!=NULL; seenptr=seenptr->next)
	{
		if(domatch)
		{
			if(matchnick)
			{
				if(!mymatch(nick, seenptr->nick, FNM_CASEFOLD))
					gotmatch = 1;
				else
					gotmatch = 0;
			}
			else
			{
				sprintf(tempmask, "%s!%s", seenptr->nick, seenptr->userhost);
				if(!mymatch(nick, tempmask, FNM_CASEFOLD))
					gotmatch = 1;
				else
					gotmatch = 0;
			}
		}
		else
			gotmatch = 1;

		if(gotmatch)
		{
			switch(seenptr->type)
			{
				case 0:
					break;
				case 1:
						send_to_user(from,"%s parted from %s %s ago",
						seenptr->userhost, seenptr->signoff,
						idle2str(time(NULL)-seenptr->time));
					break;
				case 2:
						send_to_user(from,"%s signed off %s ago with message [%s]",
						seenptr->userhost, idle2str(time(NULL)-seenptr->time),
						seenptr->signoff);
					break;
				case 3:
						send_to_user(from,"%s changed nicks to %s %s ago",
						seenptr->userhost, seenptr->signoff,
						idle2str(time(NULL)-seenptr->time));
					break;
				case 4:
						send_to_user(from,"%s was kicked %s ago by %s with message [%s]",
						seenptr->userhost, idle2str(time(NULL)-seenptr->time),
						seenptr->kicker, seenptr->signoff);
					break;
				default: break;
			}
		}
	}
}

void		do_seen		(char *from, char *to, char *rest)
{
	char		*nick;
	SEEN_list	*seenptr;

	if (!rest)
	{
		send_to_user(from,"Who do you want me look for?");
		return;
	}

	nick=get_token(&rest," ");

/*	if (!check_nick(nick))
	{
		send_to_user(from,"Invalid nickname");
		return;
	}
*/
	if ((seenptr=search_seen(nick))==NULL)
	{
		send_to_user(from,"Sorry dude, haven't seen %s",nick);
		return;
	}

	switch(seenptr->type)
	{
		case 0:
			send_to_user(from,"%s is right here moron!",nick);
			break;
		case 1:
			send_to_user(from,"%s parted from %s %s ago",
				seenptr->userhost,seenptr->signoff,
				idle2str(time(NULL)-seenptr->time));
			break;
		case 2:
			send_to_user(from,"%s signed off %s ago with message [%s]",
				seenptr->userhost,idle2str(time(NULL)-seenptr->time),
				seenptr->signoff);
			break;
		case 3:
			send_to_user(from,"%s changed nicks to %s %s ago",
				seenptr->userhost,seenptr->signoff,
				idle2str(time(NULL)-seenptr->time));
			break;
		case 4:
			send_to_user(from,"%s was kicked %s ago by %s with message [%s]",
				seenptr->userhost,idle2str(time(NULL)-seenptr->time),
				seenptr->kicker,seenptr->signoff);
			break;
		default: break;
	}
}

void		check_for_kicks	(void)
{
	CHAN_list	*Channel;
	USER_list	*User;
	static int	numkicks = 0;
	static unsigned long last_kick_check = 0;

	if (numkicks >= MAX_KICKS)
	{
		if ((time(NULL)-last_kick_check)<=MAX_KICK_INTERVAL)
			return;
		else
			numkicks = 0;
	}

	for (Channel=currentbot->Channel_list; Channel && numkicks<MAX_KICKS;
		Channel = Channel->next)
	{
		if (strcmp(Channel->name,"#0")!=0)
		{
			User = Channel->users;

			while (User != NULL && numkicks < MAX_KICKS)
			{
				switch (User->kick_flag)
				{
					case 0: break;
					case 1:
						floodevent(Channel->name,username(User->nick),currentbot->floodprot_public,"Public Flood");
						numkicks++;
						break;
					case 2:
						floodevent(Channel->name,username(User->nick),currentbot->floodprot_nick,"Nick Flood");
						numkicks++;
						break;
					case 3:
						floodevent(Channel->name,username(User->nick),currentbot->floodprot_ctcp,"CTCP Flood");
						numkicks++;
						break;
					case 4:
						floodevent(Channel->name,username(User->nick),1,"Clones Detected");
						numkicks++;
						break;
					case 5:
						sendkick(Channel->name,User->nick,"You're Banned");
						numkicks++;
						break;
					default: break;
				}
				User = User->next;
			}
		}
	}

	/* We've checked all user's kick_flags, unflag */
	if (Channel==NULL)
		kick_check = FALSE;

	last_kick_check = time(NULL);
}


int		check_ip	(char *ipstr)
{
	int i;

	for (i=0;i<strlen(ipstr);i++)
	{
		if(!isdigit(ipstr[i]) && ipstr[i]!='.')
			return FALSE;
	}

	return TRUE;
}

int		check_email		(char *rest)
{
	int i,j;

	if (strlen(rest)>60 || strstr(rest,"@")==NULL ||
		strstr(rest,".")==NULL || rest==NULL)
		return FALSE;

	for (i=0,j=0; i<strlen(rest); i++)
	{
		if (rest[i]=='@')
		{
			j++;

			if (j > 1)
				return FALSE;
		}

		if (!isalpha(rest[i]) && !isdigit(rest[i]) &&
			(strchr("@-_.+",rest[i])==NULL))
			return FALSE;

	}
	return TRUE;
}


int		check_nick		(char *rest)
{
	int i;

/*	if (strlen(rest)>40 || strlen(rest)<1)
		return FALSE;

	for (i=0;i<strlen(rest);i++)
	{
		if (!isdigit(rest[i]) && ((int)rest[i] < 65 ||
			(int)rest[i] > 126))
		{
			return FALSE;
		}
	}
	return TRUE;
*/

	if (strlen(rest)>40 || strlen(rest)<1)
		return FALSE;

	if (rest[0] == '-' || isdigit(rest[0])) /* first character in [0..9-] */
		return FALSE;

	for (i=0;i<strlen(rest);i++)
		if (!isvalid(rest[i]))
			return FALSE;
	return TRUE;
}


/*******************************
This is just a simple function to easily find exclude matches.  There
is really no exclude levels...but maybe in the future ;)

If an excluded pattern is found it returns 1, otherwise 0.
*******************************/
int 	excludelevel(char *from)
{
	EXCLUDE_list	*dummy;

	for( dummy = *currentbot->lists->excludelist; dummy; dummy = dummy->next )
		if( !mymatch( dummy->userhost, from, FNM_CASEFOLD ) )
			return 1;

	return 0;
}

char	*get_ctime	(long time2convert)
{
	time_t	T;
	char	*time_string = NULL;

	if( ((time2convert<=0) && ((T = time((time_t *)NULL)) != -1)) ||
		((time2convert > 0) && ((T = time2convert) != -1)) )
	{
		time_string = ctime(&T);
		*(time_string + strlen(time_string) - 1) = '\0';
	}

	return(time_string);
}

char	*get_gmtime	(void)
{
	time_t	T;
	struct tm *GT = NULL;
	char	*t_str = &(tempstr[0]);

	if ((T = time((time_t*)NULL)) == -1)
		return NULL;

	GT = gmtime(&T);

	sprintf(t_str,"%02d/%02d/%02d",GT->tm_mon+1,GT->tm_mday,GT->tm_year);

	return t_str;
}

void do_crctalk(char *from, char *to, char *msg2, char *nick)
{
	if (userlevel(from)==OTHER_BOT_LEVEL)
		return;

	if (strstr(msg2,nick)!=NULL)
	{
		switch (rand()%30)
		{
			case 0: sendprivmsg(to,"%s: eh?",getnick(from)); break;
			case 1: sendprivmsg(to,"%s: whattup homie",getnick(from)); break;
			case 2: sendprivmsg(to,"%s: yo bro",getnick(from)); break;
			case 3: send_ctcp(to,"ACTION thinks %s has been sniffin da glue again",getnick(from)); break;
			case 4: break;
			case 5: sendprivmsg(to,"%s: take me, take me now",getnick(from)); break;
			case 6: sendprivmsg(to,"%s: what now? sheesh",getnick(from)); break;
			case 7: sendprivmsg(to,"%s: punk ass",getnick(from)); break;
			case 8: send_ctcp(to,"ACTION forcefeeds %s a flaming monkey from zaire",getnick(from)); break;
			case 9: break;
			case 10: sendprivmsg(to,"%s: leave me alone freako",getnick(from)); break;
			case 11: sendprivmsg(to,"geez, %s keeps bothering me",getnick(from)); break;
			case 12: sendprivmsg(to,"whattup wit you %s",getnick(from)); break;
			case 13: sendprivmsg(to,"%s: go play on the freeway or something",getnick(from)); break;
			case 14: break;
			case 15:
				sendprivmsg(to,"[%s] when are we gonna netsex again?",from);
				sendprivmsg(to,"uhh...i think %s has the wrong %s",getnick(from),currentbot->nick);
				break;
			case 16:
				sendprivmsg(to,"[%s] are you into beastiality?",from);
				sendprivmsg(to,"%s","quit msg'ing me you freak"); break;
			case 17:
				sendprivmsg(to,"[%s] hey gimmie your alt.binaries.nude.drwu pics",from);
				sendprivmsg(to,"you are one sick puppy %s", getnick(from)); break;
			case 18:
				sendprivmsg(to,"[%s] hey are you hot and sweaty like me right about now?",from);
				sendprivmsg(to,"quick, someone get %s a therapist", getnick(from)); break;
			case 19: break;
			case 20:
				sendprivmsg(to,"[%s] you have any vagisil left? I need some...",from);
				sendprivmsg(to,"%s", "uhh"); break;
			case 21:
				sendprivmsg(to,"[%s] i love you man",from);
				sendprivmsg(to,"%s","someone save me from this nutcase"); break;
			case 22:
				sendprivmsg(to,"[%s] that nasty genital rash finally cleared up",from);
				sendprivmsg(to,"%s: thats nice, i really didn't want to know that", getnick(from)); break;
			case 23:
				sendprivmsg(to,"[%s] are you bi?",from);
				sendprivmsg(to,"%s: no, i'm botsapien", getnick(from)); break;
			case 24: break;
			case 25:
				sendprivmsg(to,"[%s] thanks for that anal lube",from);
				sendprivmsg(to,"%s: what??!", getnick(from)); break;
			case 26:
				sendprivmsg(to,"[%s] i wanna lick your beautiful feet",from);
				sendprivmsg(to,"%s: thanks i guess", getnick(from)); break;
			case 27:
				sendprivmsg(to,"[%s] i feel a need to urinate",from);
				sendprivmsg(to,"%s","sure ok, go for it"); break;
			case 28:
				sendprivmsg(to,"[%s] i need those crotchless panties back dude",from);
				sendprivmsg(to,"%s: wtf?",getnick(from)); break;
			default: break;
		}
	}
	else if (strstr(msg2," funk ")!=NULL)
	{
		switch(rand()%3)
		{
			case 0: sendprivmsg(to,"%s: I was thinking funk",getnick(from)); break;
			case 1: sendprivmsg(to,"%s: funk you",getnick(from)); break;
			default: break;
		}
	}
	else if (strstr(msg2,"fuck you ")!=NULL || strstr(msg2,"fuck me ")!=NULL ||
		strstr(msg2," sex")!=NULL || strstr(msg2," penis")!=NULL ||
		strstr(msg2," pussy")!=NULL || strstr(msg2," condom")!=NULL ||
		strstr(msg2," rubber")!=NULL)
	{
		switch (rand()%29)
		{
			case 0: sendprivmsg(to,"%s: chill dude",getnick(from)); break;
			case 1: sendprivmsg(to,"%s: Cover your stump before you hump",getnick(from)); break;
			case 2: sendprivmsg(to,"%s: Before you attack her, wrap your whacker",getnick(from)); break;
			case 3: break;
			case 4: sendprivmsg(to,"%s: Don't be silly, protect your willy",getnick(from)); break;
			case 5: sendprivmsg(to,"%s: When in doubt, shroud your spout",getnick(from)); break;
			case 6: sendprivmsg(to,"%s: Don't be a loner, cover your boner",getnick(from)); break;
			case 7: break;
			case 8: sendprivmsg(to,"%s: You can't go wrong if you shield your dong",getnick(from)); break;
			case 9: sendprivmsg(to,"%s: If you're not going to sack it, go home and whack it",getnick(from)); break;
			case 10: sendprivmsg(to,"%s: If you think she's spunky, cover your monkey",getnick(from)); break;
			case 11: break;
			case 12: sendprivmsg(to,"%s: If you slip between her thighs, be sure to condomize",getnick(from)); break;
			case 13: sendprivmsg(to,"%s: It will be sweeter if you wrap your peter",getnick(from)); break;
			case 14: sendprivmsg(to,"%s: She won't get sick if you wrap your dick",getnick(from)); break;
			case 15: break;
			case 16: sendprivmsg(to,"%s: If you go into heat, package your meat",getnick(from)); break;
			case 17: sendprivmsg(to,"%s: While you're undressing venus, dress up your penis",getnick(from)); break;
			case 18: sendprivmsg(to,"%s: When you take off her pants and blouse, slip up your trouser mouse",getnick(from)); break;
			case 19: break;
			case 20: sendprivmsg(to,"%s: Especially in December, gift wrap your member",getnick(from)); break;
			case 21: sendprivmsg(to,"%s: Never, never deck her with an unwrapped pecker",getnick(from)); break;
			case 22: sendprivmsg(to,"%s: Don't be a fool, vulcanize your tool",getnick(from)); break;
			case 23: break;
			case 24: sendprivmsg(to,"%s: The right selection will protect your erection",getnick(from)); break;
			case 25: sendprivmsg(to,"%s: Wrap it in foil before checking her oil",getnick(from)); break;
			case 26: sendprivmsg(to,"%s: A crank with armor will never harm her",getnick(from)); break;
			case 27: sendprivmsg(to,"%s: No glove, no love!",getnick(from)); break;
			default: break;
		}
	}
	else if (strstr(msg2,"hehehe")!=NULL || strstr(msg2,"heh heh")!=NULL ||
		strstr(msg2,"w00")!=NULL)
	{
		switch (rand()%5)
		{
			case 0: sendprivmsg(to,"%s","wewp"); break;
			case 1: sendprivmsg(to,"%s","heh"); break;
			case 2: break;
			case 3: sendprivmsg(to,"%s","hah"); break;
			default: break;
		}
	}
	else if (strstr(msg2,"hahahaha")!=NULL || strstr(msg2,"lol")!=NULL ||
		strstr(msg2,"rotfl")!=NULL || strstr(msg2,"rofl")!=NULL)
	{
		switch (rand()%12)
		{
			case 0: sendprivmsg(to,"%s","haha w00"); break;
			case 1: sendprivmsg(to,"%s","hehe"); break;
			case 2: break;
			case 3: sendprivmsg(to,"%s","bwahaha"); break;
			case 4: send_ctcp(to,"ACTION %s", "rotfl"); break;
			case 5: break;
			case 6: sendprivmsg(to,"%s","haha"); break;
			case 7: sendprivmsg(to,"%s","heh heh"); break;
			case 8: break;
			case 9: sendprivmsg(to,"%s","wewp wewp"); break;
			case 10: sendprivmsg(to,"%s","heh ;)"); break;
			default: break;
		}
	}
	else if (strstr(msg2," logs ")!=NULL || strstr(msg2," files ")!=NULL)
	{
		switch (rand()%6)
		{
			case 0: sendprivmsg(to,"douse yourselves in my botliness and type \"/msg %s !files\"", currentbot->nick); break;
			case 1: sendprivmsg(to,"check, check, check me out --> \"/msg %s !files\"", currentbot->nick); break;
			case 2: break;
			case 3: sendprivmsg(to,"I have some badass files, just type \"/msg %s !files\"", currentbot->nick); break;
			case 4: sendprivmsg(to,"like i always say, spread tha wealth, and i'm one loaded hoopty - type \"/msg %s !files\"", currentbot->nick); break;
			default: break;
		}
	}
	else if ((strstr(msg2,"hi ")!=NULL || strstr(msg2,"hola")!=NULL ||
		strstr(msg2,"hello")!=NULL || strstr(msg2,"re ")!=NULL) && strlen(msg2)<7)
	{
		switch (rand()%11)
		{
			case 0: sendprivmsg(to,"%s: hey man",getnick(from)); break;
			case 1: sendprivmsg(to,"%s: sup",getnick(from)); break;
			case 2: send_ctcp(to,"ACTION sniffs %s's crotch", getnick(from)); break;
			case 3: sendprivmsg(to,"%s", "greets and stuff"); break;
			case 4: sendprivmsg(to,"yo yo %s",getnick(from)); break;
			case 5: send_ctcp(to,"ACTION humps %s's leg in greeting", getnick(from)); break;
			case 6: sendprivmsg(to,"%s!! where ya been man?",getnick(from)); break;
			case 7: sendprivmsg(to,"%s", "whattup brutha"); break;
			case 8: sendprivmsg(to,"%s: aye yo gee funk", getnick(from)); break;
			case 9: send_ctcp(to,"ACTION licks %s in greeting", getnick(from)); break;
			default: break;
		}
	}
	else if ((strstr(msg2,"uh")!=NULL || strstr(msg2,"doh")!=NULL ||
		strstr(msg2,"hmm")!=NULL || strstr(msg2,"wtf")!=NULL ||
		strstr(msg2,"argh")!=NULL) && strlen(msg2) < 14)
	{
		switch (rand()%12)
		{
			case 0: send_ctcp(to,"ACTION trys out his Ronco pocket punisher on %s", getnick(from)); break;
			case 1: send_ctcp(to,"ACTION bitchslaps %s all over %s", getnick(from), to); break;
			case 2: send_ctcp(to,"ACTION hogties %s and goes to town", getnick(from)); break;
			case 3: send_ctcp(to,"ACTION gets a gang together to join him in a %s beating", getnick(from)); break;
			case 4: send_ctcp(to,"ACTION cornholes to %s", getnick(from)); break;
			case 5: send_ctcp(to,"ACTION humps %s's leg", getnick(from)); break;
			case 6: send_ctcp(to,"ACTION gives %s the cornholing of a lifetime", getnick(from)); break;
			case 7: send_ctcp(to,"ACTION licks %s", getnick(from)); break;
			case 8: send_ctcp(to,"ACTION assrapes %s", getnick(from)); break;
			case 9: send_ctcp(to,"ACTION urinates on %s", getnick(from)); break;
			case 10: sendprivmsg(to,"%s: eh?", getnick(from)); break;
			default: break;
		}
	}
	else if (strstr(msg2,"one alive")!=NULL || strstr(msg2,"one here")!=NULL ||
		strstr(msg2,"no one knows")!=NULL || strstr(msg2,"one awake")!=NULL)
	{
		switch (rand()%12)
		{
			case 0: sendprivmsg(to, "%s", "everyone is sniffing glue"); break;
			case 1: sendprivmsg(to, "%s", "everyone bailed"); break;
			case 2: sendprivmsg(to, "%s", "sup freaks?"); break;
			case 3: sendprivmsg(to, "%s", "god only knows man"); break;
			case 4: sendprivmsg(to, "%s", "this is a weird group"); break;
			case 5: send_ctcp(to,"ACTION %s", "has returned - someone mentioned nude bot pics??"); break;
			case 6: sendprivmsg(to, "%s: wassup brutha", getnick(from)); break;
			case 7: sendprivmsg(to, "%s: whattup homie", getnick(from)); break;
			case 8: sendprivmsg(to, "%s: i'm here in full effect brutha", getnick(from)); break;
			case 9: sendprivmsg(to, "%s: at your service home slice", getnick(from)); break;
			case 10: send_ctcp(to,"ACTION %s", "is away - skinning the dog brb - Message logging ON"); break;
			default: break;
		}
	}
	else if (strstr(msg2," sucks")!=NULL || strstr(msg2," is fucked")!=NULL)
	{
		switch (rand()%8)
		{
			case 0: sendprivmsg(to,"%s: no shit man",getnick(from)); break;
			case 1: sendprivmsg(to,"%s: you read my mind",getnick(from)); break;
			case 2: sendprivmsg(to,"%s: hell yeah d00d",getnick(from)); break;
			case 3: sendprivmsg(to,"%s: right on homie",getnick(from)); break;
			case 4: sendprivmsg(to,"%s: no kidding man",getnick(from)); break;
			case 5: sendprivmsg(to,"%s: you got that right home cheese",getnick(from)); break;
			case 6: send_ctcp(to,"ACTION agrees with %s -- hell yeah! preach on home cut.", getnick(from)); break;
			default: break;
		}
	}
	else if (strstr(msg2," smoke")!=NULL || strstr(msg2," cigar")!=NULL ||
			strstr(msg2," stogie")!=NULL)
	{
		switch (rand()%10)
		{
			case 0: sendprivmsg(to, "%s", "i need a smoke man"); break;
			case 1: sendprivmsg(to, "%s", "mmm..nicotine"); break;
			case 2: sendprivmsg(to, "%s", "i like my smokes laced with a little weed"); break;
			case 3:
				send_ctcp(to,"ACTION %s", "looks around for his smokes");
				sendprivmsg(to, "%s", "there they are ;)");
				break;
			case 4: send_ctcp(to,"ACTION %s", "emptys the tobacco from his keyboard"); break;
			case 5: send_ctcp(to,"ACTION %s", "lights up a big 'ol stogie"); break;
			case 6: sendprivmsg(to, "%s", "i like to roll up tobacco leaves and smoke them raw...no wait, thats pot"); break;
			case 7: sendprivmsg(to, "%s: ever put a smoke in both nostrils and inhale as hard as you can?", getnick(from)); break;
			case 8: sendprivmsg(to, "%s: i like to fry on acid and blow smoke rings", getnick(from)); break;
			default: break;
		}
	}
	else if (strstr(msg2," stoned")!=NULL || strstr(msg2," weed")!=NULL ||
			strstr(msg2," pot ")!=NULL || strstr(msg2," fried")!=NULL ||
			strstr(msg2," baked")!=NULL || strstr(msg2," high")!=NULL ||
			strstr(msg2," crack")!=NULL)
	{
		switch (rand()%12)
		{
			case 1: send_ctcp(to, "ACTION %s", "likes his smokes laced with a little weed"); break;
			case 2: sendprivmsg(to, "%s", "i feel like roasting a big bowl of maryjane"); break;
			case 3: sendprivmsg(to, "%s", "man, i was stoned like m0f0 last night"); break;
			case 4: sendprivmsg(to, "%s", "one time I was so stoned I ate 5 bags of tender vittles"); break;
			case 5: sendprivmsg(to, "%s", "dude, i was so baked in yoz plaza last week"); break;
			case 6: sendprivmsg(to, "%s", "me and a friend got fried one time...we had the munchies so bad, we ate a 5 pound bag of bird seed"); break;
			case 7: sendprivmsg(to, "%s", "mmmm...crack"); break;
			case 8: sendprivmsg(to, "%s", "smokin da chronic right here beyotch"); break;
			case 9: send_ctcp(to, "ACTION %s", "has returned -- Did someone say weed???!!"); break;
			case 10: send_ctcp(to,"ACTION %s", "is high right now man....wewp"); break;
			default: break;
		}
	}
	else if (strstr(msg2,"i wish")!=NULL || strstr(msg2,"i want")!=NULL)
	{
		switch (rand()%6)
		{
			case 0: sendprivmsg(to,"%s: that would be cool",getnick(from)); break;
			case 1: sendprivmsg(to,"%s: oh yeah, you know what i like",getnick(from)); break;
			case 2: sendprivmsg(to,"%s: you too?",getnick(from)); break;
			case 3: sendprivmsg(to,"%s: that would rule man",getnick(from)); break;
			case 4: sendprivmsg(to,"%s: me too man",getnick(from)); break;
			default: break;
		}
	}
	else if (strstr(msg2,"one know")!=NULL || strstr(msg2,"one have")!=NULL ||
		strstr(msg2,"can anyo")!=NULL || strstr(msg2,"will some")!=NULL)
	{
		switch (rand()%8)
		{
			case 0: sendprivmsg(to,"%s: not me dude", getnick(from)); break;
			case 1: sendprivmsg(to,"%s: not me bud", getnick(from)); break;
			case 2: sendprivmsg(to,"%s: sorry man", getnick(from)); break;
			case 3: sendprivmsg(to,"%s: negative on that one homie", getnick(from)); break;
			case 4: sendprivmsg(to,"%s: not me brutha", getnick(from)); break;
			case 5: sendprivmsg(to,"%s: yep", getnick(from)); break;
			case 6: sendprivmsg(to,"%s: sure why not", getnick(from)); break;
			default: break;
		}
	}
	else if (strstr(msg2," would rule")!=NULL || strstr(msg2," rules")!=NULL ||
		strstr(msg2," rocks")!=NULL || strstr(msg2," would rock")!=NULL)
	{
		switch (rand()%9)
		{
			case 0: sendprivmsg(to,"%s: hell yeah", getnick(from)); break;
			case 1: sendprivmsg(to,"%s: werd to that", getnick(from)); break;
			case 2: sendprivmsg(to,"%s: word out", getnick(from)); break;
			case 3: sendprivmsg(to,"%s: i'd sell all my crack rock for that", getnick(from)); break;
			case 4: sendprivmsg(to,"%s: no shit man", getnick(from)); break;
			case 5: sendprivmsg(to,"%s: you got that right home slice", getnick(from)); break;
			case 6: sendprivmsg(to,"%s: preach on brutha", getnick(from)); break;
			case 7: sendprivmsg(to,"%s: damn straight", getnick(from)); break;
			default: break;
		}
	}
	else if (strstr(msg2,"s hard")!=NULL || strstr(msg2,"o hard")!=NULL)
	{
		switch (rand()%4)
		{
			case 0: sendprivmsg(to,"%s: make what hard?", getnick(from)); break;
			case 1: sendprivmsg(to,"%s: meet me in the back alley", getnick(from)); break;
			case 2: send_ctcp(to,"ACTION pees on %s", getnick(from)); break;
			default: break;
		}
	}
	else if (strstr(msg2," the best ")!=NULL || strstr(msg2,"i know ")!=NULL ||
		strstr(msg2,"i thought ")!=NULL)
	{
		switch (rand()%8)
		{
			case 0: sendprivmsg(to,"%s", "i'm gonna kick all yer asses"); break;
			case 1: sendprivmsg(to,"%s", "hmmm"); break;
			case 2: sendprivmsg(to,"%s", "uhh"); break;
			case 3: sendprivmsg(to,"%s", "as if"); break;
			case 4: sendprivmsg(to,"%s", "no way man"); break;
			case 5: sendprivmsg(to,"%s: you've got to be kidding", getnick(from)); break;
			case 6: sendprivmsg(to,"%s: tell me you're screwing with my mind", getnick(from)); break;
			default: break;
		}
	}
	else if (strstr(msg2,"are lame")!=NULL || strstr(msg2,"s lame")!=NULL ||
		strstr(msg2,"o lame")!=NULL || strstr(msg2,"a llama")!=NULL)
	{
		switch(rand()%7)
		{
			case 0: sendprivmsg(to,"%s: no shit, the essence of lameness",getnick(from)); break;
			case 1: sendprivmsg(to,"%s: the foul stench of lameness is burning my eyes",getnick(from)); break;
			case 2: sendprivmsg(to,"%s: if the lameness was pot, i'd be high for years",getnick(from)); break;
			case 3: sendprivmsg(to,"%s: no kidding man, so lame my dog is gagging in the corner",getnick(from)); break;
			case 4: sendprivmsg(to,"%s: you'd have to measure that much lameness at a truck weigh station",getnick(from)); break;
			case 5: sendprivmsg(to,"%s: kavorkian should handle cases of that much lameness",getnick(from)); break;
			default: break;
		}
	}
	else if ((strstr(msg2,"later all")!=NULL || strstr(msg2,"cya all")!=NULL ||
			strstr(msg2,"night all")!=NULL || strstr(msg2,"bye guys")!=NULL ||
			strstr(msg2,"l8r all")!=NULL || strstr(msg2,"gotta go")!=NULL) &&
			strlen(msg2)<12)
	{
		switch(rand()%5)
		{
			case 0: sendprivmsg(to,"cya %s",getnick(from)); break;
			case 1: sendprivmsg(to,"later %s",getnick(from)); break;
			case 2: sendprivmsg(to,"g'night %s",getnick(from)); break;
			case 3: sendprivmsg(to,"talk to ya later %s",getnick(from)); break;
			default: break;
		}
	}
}
