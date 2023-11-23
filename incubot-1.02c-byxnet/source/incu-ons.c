#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#ifndef SYSV
#include <strings.h>
#endif
#include <ctype.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>

#include "vladbot.h"
#include "channel.h"
#include "config.h"
#include "dcc.h"
#include "log.h"
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
#include "tb-ons.h"
#include "crc-ons.h"
#include "incu-ons.h"

extern	botinfo	*currentbot;
extern	int public_command;
extern	int addressed_to_me;
char		tempwork[WAYTOBIG];

void channel_is_invite(USERLVL_list **l_list, char *channel)
/*
 *	Bot tried to join a channel that is invite only.  Lets cycle through the
 * userlist and get a list of other bots to /msg BotNickList invite #channel
 */
{
	char invitenicks[512] = "";
	int numnicks = 0;
	USERLVL_list *dummy;
	extern botinfo *currentbot;

	for(dummy = *l_list; dummy; dummy = dummy->next)
		if(dummy->access == OTHER_BOT_LEVEL && strcmp(dummy->nick, currentbot->nick))
		{
			/* Create a comma-delimited list of nicks */
			add_to_commalist(dummy->nick, invitenicks);
			numnicks++;
			/* Some servers have a limit on the nicks a single message can be
				addressed to. Send message and clear nicklist if 10 nicks are
				in the list.
			 */
			if(numnicks > 9)
			{
				sendprivmsg(invitenicks, "invite %s", channel);
				printf("Sent invite request to %s for %s\n", invitenicks, channel);
				invitenicks[0] = '\0';
				numnicks = 0;
			}
		}
	if(numnicks)
		sendprivmsg(invitenicks, "invite %s", channel);
}


void add_to_commalist(char *nick, char *list)
/*
 *	Adds a nickname to a comma-delimited string of nicks so multiple
 * recipients can be sent the same message.
 */
{
	if(list[0] == '\0')
		strcpy(list, nick);
	else
	{
		strcat(list, ",");
		strcat(list, nick);
	}
}
void show_authed (char *from, char *to, char *rest)
{
	char	*nuh;

	if( rest == NULL )
	{
		send_to_user( from, "Please specify a nickname" );
		return;
	}
	if((nuh=username(rest))==NULL)
	{
		send_to_user(from, "%s is not on this channel!", rest);
		return;
	}

	if (userlevel(nuh)==0)
		send_to_user(from, "USERLIST: %s is not a user", rest);
	else
		if(checkauth(nuh))
			send_to_user(from, "%s is authenticated", rest);
		else
			send_to_user(from, "%s is not authenticated", rest);

}

void do_whoshitted(char *from, char *to, char *rest)
{
	USERLVL_list	*dummy;

	if (!rest)
	{
		send_to_user(from, "Please specify a mask to query shit info on.");
		return;
	}

	if((dummy = find_userhost(currentbot->lists->shitlist, rest, 0))!=NULL)
	{
		send_to_user(from, "%s was shitted by %s.", rest, dummy->mailaddr);
		return;
	}
	else
		send_to_user(from, "No shit information for %s stored", rest);
}

void do_whenshitted(char *from, char *to, char *rest)
{
	USERLVL_list	*dummy;

	if (!rest)
	{
		send_to_user(from, "Please specify a mask to query shit info on.");
		return;
	}
	if ((dummy = find_userhost(currentbot->lists->shitlist, rest, 0))!=NULL)
		if(dummy->nick)
			send_to_user(from, "%s was shitted on %s.", rest, dummy->nick);
		else
			send_to_user(from, "No shit information for %s stored", rest);
	else
		send_to_user(from, "%s is not shitted", rest);
}

void do_chshitlevel(char *from, char *to, char *rest)
{
	USERLVL_list *dummy;
	char	*newuser;
	char	*newlevel;
	int	already;

	if((newuser = get_token(&rest, " ")) == NULL)
	{
		send_to_user(from, "Which shitlist entry do you want to alter?");
		return;
	}

	if((newlevel = get_token(&rest, " ")) == NULL )
	{
		send_to_user(from, "What level should %s have?", newuser);
		return;
	}

	if ((dummy = find_userhost(currentbot->lists->shitlist, newuser, 0))==NULL)
	{
		send_to_user(from, "%s is not shitted", newuser);
		return;
	}

	already = dummy->access;

	if(already == atoi(newlevel))
	{
		send_to_user(from, "%s is already shitted at level %d", newuser, already);
		return;
	}

	if(atoi(newlevel) < 1)
	{
		send_to_user(from, "level should be >= 1!");
		return;
	}
	dummy->access = atoi(newlevel);
	send_to_user(from, "Changed the shitlevel for %s from %d to %d", newuser, already, dummy->access);
	add_list_event("%s changed the shitlevel for %s from %d to %d", from, newuser, already, dummy->access);
	globallog(2, DONELOG, "%s changed the shitlevel for %s to %d",
		from, newuser, atoi(newlevel));
}

void do_chwhenshitted(char *from, char *to, char *rest)
{
	USERLVL_list *dummy;
	char *user;
	char *newnick;
	char *old;

	if ((user=get_token(&rest," "))==NULL)
	{
		send_to_user(from, "Which shitlist entry do you want to change?");
		return;
	}

	if ((newnick=get_token(&rest," "))==NULL)
	{
		send_to_user(from, "What date do you want to add for %s?",user);
		return;
	}

	if (!check_date(newnick))
	{
		send_to_user(from, "Invalid date. Must be in mm/dd/yy format");
		return;
	}

	if ((dummy = find_userhost(currentbot->lists->shitlist, user, 0))==NULL)
	{
		send_to_user(from, "%s is not shitted", user);
		return;
	}

	old = dummy->nick;
	dummy->nick = strdup(newnick);
	send_to_user(from, "Changed shit date for %s from %s to %s", user, old, newnick);
	add_list_event("%s changed the shit date for %s from %s to %s", from, user, old, newnick);
	globallog(2, DONELOG, "%s changed the shit date for %s to %s",
		from, user, newnick);
	free(old);
	if(!write_lvllist(currentbot->lists->shitlist, currentbot->lists->shitfile))
	{
		send_to_user(from, "Shitlist could not be written to file %s",
			currentbot->lists->shitfile);

#ifdef BackedUp
		if (Abackedup)
			send_to_user(currentbot->backupnick,"!comm %s shitwrite",
				currentbot->backupbotpasswd);
#endif
	}
	else
		send_to_user(from, "Shitlist written to file %s", currentbot->lists->shitfile);
}

void do_chshitreason(char *from, char *to, char *rest)
{
	char	*user;

	if ((user=get_token(&rest," "))==NULL)
	{
		send_to_user(from,"Which shitlist entry do you want to change?");
		return;
	}

	if (!rest)
	{
		send_to_user(from,"What shit reason do you want to give %s?",user);
		return;
	}

	if (strlen(rest)<5)
	{
		send_to_user(from,"Get real dude, gimme a valid shit reason");
		return;
	}

	if(!add_to_passwdlist(currentbot->lists->shitlist2,user,rest))
	{
		send_to_user(from,"%s is not shitted",
			user);
		return;
	}

	send_to_user(from,"Shit reason for %s changed to %s",user,rest);

	if(!write_passwdlist(currentbot->lists->shitlist2,currentbot->lists->shitfile2))
		send_to_user(from, "Shitlist could not be written to file %s",currentbot->lists->shitfile2);
	else
		send_to_user(from, "Shitlist written to file %s",currentbot->lists->shitfile2);

	globallog(2, DONELOG, "%s changed the shit reason for %s to %s",from,user,rest);
	add_list_event("%s changed the shit reason for %s to %s", from, user, rest);
}

void do_chshitmask(char *from, char *to, char *rest)
{
	USERLVL_list	*dummy;
	EXPIRE_list		*expire;
	char	*user;
	char	*newuser;

	if ((user=get_token(&rest," "))==NULL)
	{
		send_to_user(from, "Which shitmask do you want to change?");
		return;
	}

	if ((newuser=get_token(&rest," "))==NULL)
	{
		send_to_user(from, "What do you want to change %s to?", user);
		return;
	}

	if(mymatch("*?!*?@*?",newuser, FNM_CASEFOLD))
	{
		send_to_user(from, "%s is an invalid mask.", newuser);
		return;
	}

	if (find_userhost(currentbot->lists->shitlist, newuser, 0)!=NULL)
	{
		send_to_user(from, "A shitlist entry matching %s already exists", newuser);
		return;
	}

	if ((dummy = find_userhost(currentbot->lists->shitlist, user, 0))==NULL)
	{
		send_to_user(from, "%s is not shitted", user);
		return;
	}

	free(dummy->userhost);
	dummy->userhost = strdup(newuser);

	if ((dummy = find_userhost(currentbot->lists->shitlist2, user, 0))==NULL)
	{
		send_to_user(from, "ERROR: %s not found in data structure shitlist2", user);
		return;
	}

	free(dummy->userhost);
	dummy->userhost = strdup(newuser);
	send_to_user(from, "Changed shitmask for %s to %s", user, newuser);
	globallog(2, DONELOG, "%s changed the shitmask for %s to %s",
		from, user, newuser);
	add_list_event("%s changed the shitmask for %s to %s", from, user, newuser);

	if(!write_lvllist(currentbot->lists->shitlist, currentbot->lists->shitfile))
	{
		send_to_user(from, "Shitlist could not be written to file %s",
			currentbot->lists->shitfile);
	}

	if(!write_passwdlist(currentbot->lists->shitlist2, currentbot->lists->shitfile2))
	{
		send_to_user(from, "Shitlist could not be written to file %s",
			currentbot->lists->shitfile2);

#ifdef BackedUp
		if (Abackedup)
			send_to_user(currentbot->backupnick,"!comm %s shitwrite",
				currentbot->backupbotpasswd);
#endif
	}
	else
	{
		send_to_user(from, "Shitlist written to file %s", currentbot->lists->shitfile);
		send_to_user(from, "Shitlist2 written to file %s", currentbot->lists->shitfile2);
	}
	if((expire = find_expire_userhost(currentbot->lists->expirelist, user, 0)) != NULL)
	{
		free(expire->userhost);
		expire->userhost = strdup(newuser);
		if(!write_expirelist(currentbot->lists->expirelist,currentbot->lists->expirefile))
			send_to_user(from, "Could not save expirelist");
	}
}

void do_chwhoshitted(char *from, char *to, char *rest)
{
	char	*user;
	char	*whoshitted;

	if ((user=get_token(&rest," "))==NULL)
	{
		send_to_user(from, "Which shitlist entry do you want to change?");
		return;
	}

	if ((whoshitted=get_token(&rest," "))==NULL)
	{
		send_to_user(from, "What whoshitted info do you want to add for %s?",user);
		return;
	}

	if (!change_email(currentbot->lists->shitlist, user, whoshitted))
	{
		send_to_user(from, "%s is not shitted", user);
		return;
	}
	send_to_user(from, "Changed the whoshitted info for %s to %s", user, whoshitted);
	add_list_event("%s changed the whoshitted info for %s to %s", from, user, whoshitted);
	if(!write_lvllist(currentbot->lists->shitlist, currentbot->lists->shitfile))
	{
		send_to_user(from, "Shitlist could not be written to file %s",
			currentbot->lists->shitfile);

#ifdef BackedUp
		if (Abackedup)
			send_to_user(currentbot->backupnick,"!comm %s shitwrite",
				currentbot->backupbotpasswd);
#endif
	}
	else
		send_to_user(from, "Shitlist written to file %s", currentbot->lists->shitfile);
}

int check_date(char *datestr)
{
	if(datestr[2] != '/' || datestr[5] != '/')
		return(0);
	if(!isdigit(datestr[0]) || !isdigit(datestr[1]) ||
		!isdigit(datestr[3]) || !isdigit(datestr[4]) ||
		!isdigit(datestr[6]) || !isdigit(datestr[7]))
		return(0);
	if(datestr[8] != '\0')
		return(0);
	return (1);
}

void show_userlist2(USERLVL_list **l_list, char *from, char *to, char *rest)
{
	USERLVL_list	*dummy;
	char	*arg1, *arg2;
	int temp = 0, level = 0;

	if ((arg1 = get_token(&rest," ")) == NULL)
	{
		send_to_user(from, "Invalid switch");
		return;
	}

	if(STRCASEEQUAL(arg1, "-mask"))
		temp = 1;
	if(STRCASEEQUAL(arg1, "-nick"))
		temp = 2;
	if(STRCASEEQUAL(arg1, "-email"))
		temp = 3;
	if(STRCASEEQUAL(arg1, "-level"))
		temp = 4;
	if(!temp)
	{
		send_to_user(from, "Invalid switch");
		return;
	}

	if ((arg2 = get_token(&rest," ")) == NULL)
	{
		switch(temp)
		{
			case 1:
				send_to_user(from, "Must specify a usermask");
				break;
			case 2:
				send_to_user(from, "Must specify a nick");
				break;
			case 3:
				send_to_user(from, "Must specify a email address");
				break;
			case 4:
				send_to_user(from, "Must specify a level");
				break;
		}
		return;
	}

	if(temp == 4)
		level = atoi(arg2);

	send_to_user(from, "%s User List", currentbot->nick);
	send_to_user(from, "-------+-----------------------------------------------+--------------");
	send_to_user(from, " Level | %-45s | Nickname", "nick!user@host");
	send_to_user(from, "-------+-----------------------------------------------+--------------");

	for( dummy = *l_list; dummy; dummy = dummy->next )
	{
		switch(temp)
		{
			case 1:
				if(!mymatch(arg2, dummy->userhost, FNM_CASEFOLD))
					send_to_user(from, "   %3d | %-45s | %s",
						dummy->access, dummy->userhost, dummy->nick);
				break;
			case 2:
				if(!mymatch(arg2, dummy->nick, FNM_CASEFOLD))
					send_to_user(from, "   %3d | %-45s | %s",
						dummy->access, dummy->userhost, dummy->nick);
				break;
			case 3:
				if(!mymatch(arg2, dummy->mailaddr, FNM_CASEFOLD))
					send_to_user(from, "   %3d | %-45s | %s",
						dummy->access, dummy->userhost, dummy->nick);
				break;
			case 4:
				if(dummy->access == level)
					send_to_user(from, "   %3d | %-45s | %s",
						dummy->access, dummy->userhost, dummy->nick);
				break;
		}
	}
	send_to_user(from, "End of User List");
}

void show_nuserinfo(char *from, char *to, char *rest)
{
	char	*nuh;
	char	*email;
	char	*nick;

	if( rest == NULL )
	{
		send_to_user( from, "Please specify a nickname" );
		return;
	}

	if((nuh=username(rest))==NULL)
	{
		send_to_user(from, "%s is not on this channel!", rest);
		return;
	}

	if (userlevel(nuh)==0)
	{
		send_to_user(from, "%s is not a user", rest);
		return;
	}

	email = get_email(currentbot->lists->opperlist, nuh);
	nick = get_userlist_nick(currentbot->lists->opperlist, nuh);
		send_to_user(from, "%s%s %s%s",
						email ? "Email: " : "No email address stored",
						email ? email     : "",
						nick  ? " Nick: " : "No nickname stored",
						nick  ? nick      : "");
}

int	num_userlist_matches(USERLVL_list **l_list, char *userhost, int searchtype)
/*
      searchtype:
                   0 - Nick
                   1 - Email
                   2 - Userhost
*/
{
	int				match=0;
	USERLVL_list	*dummy;

	for( dummy = *l_list; dummy; dummy = dummy->next )
	{
		switch(searchtype)
		{
			case 0:
				if(!mymatch(userhost, dummy->nick, FNM_CASEFOLD))
					match++;
				break;
			case 1:
				if(!mymatch(userhost, dummy->mailaddr, FNM_CASEFOLD))
					match++;
				break;
			case 2:
				if(!mymatch(userhost, dummy->userhost, FNM_CASEFOLD))
					match++;
				break;
		}
	}

	return match;
}

int	num_excludelist_matches (EXCLUDE_list **l_list, char *userhost)
{
	int				match=0;
	EXCLUDE_list	*dummy;

	for( dummy = *l_list; dummy; dummy = dummy->next )
	{
		if(!mymatch(userhost, dummy->userhost, FNM_CASEFOLD))
		{
			match++;
		}
	}

	return match;
}

int	num_expirelist_matches (EXPIRE_list **l_list, char *userhost)
{
	int			match=0;
	EXPIRE_list	*dummy;

	for( dummy = *l_list; dummy; dummy = dummy->next )
	{
		if(!mymatch(userhost, dummy->userhost, FNM_CASEFOLD))
		{
			match++;
		}
	}

	return match;
}

int checklevel(int level)
{
#ifdef OLD_USERLEVELS
	if(level != 25 && level != 50 && level != 75 && level != 100 && level != 105 && level != 115 && level != 125 && level != 150)
#else
	if(level != 25 && level != 50 && level != 75 && level != 100 && level != 105 && level != 115 && level != 125 && level != 150 && level != 200)
#endif
		return 0;

	return 1;
}

char	*get_add_mask	(char *usermask, int hosttype)
{
	char	*nuh;
	char	*nick;
	char	*user;
	char	*host;
	char	*domain;
	char	*ip2;
	char	*ip3;
	char	userstr[MAXLEN];

	nuh=&(tempwork[0]);
	strcpy(nuh, usermask);
	nick=get_token(&nuh, "!");
	user=get_token(&nuh,"@");

	if(*user == '~' || *user == '#')
		user++;

	host=get_token(&nuh, ".");

	if (check_ip(host))
	{
		if (hosttype)
		{
			domain=get_token(&nuh,"");
			sprintf(userstr, "*!*%s@%s.%s", user, host, domain);
		}
		else
		{
			ip2 = get_token(&nuh, ".");
			ip3 = get_token(&nuh, ".");
			sprintf(userstr, "*!*%s@%s.%s.%s.*", user, host, ip2, ip3);
		}
	}
	else
	{
		if(hosttype == 2)
			return (get_add_mask2(usermask));

		domain=get_token(&nuh,"");

		if(hosttype)
				sprintf(userstr, "*!*%s@%s.%s", user, host, domain);
		else if (domain && strstr(domain,".")==NULL)
			sprintf(userstr, "*!*%s@*%s.%s", user, host, domain);
		else
			sprintf(userstr, "*!*%s@*%s", user, domain?domain:host);
	}
	nuh = userstr;
	return (nuh);
}

char	*get_add_mask2	(char *usermask)
{
	char	*nuh;
	char	*nick;
	char	*user;
	char	*host;

	nuh=&(tempwork[0]);
	strcpy(nuh, usermask);
	nick=get_token(&nuh, "!");
	user=get_token(&nuh,"@");
	host=strrchr(nuh,'.');


	if(*user == '~' || *user == '#')
		user++;

	host--;

	while(*host != '.' && host > nuh)
	{
		host--;
	}

	sprintf(tempwork, "*!*%s@*%s",user, host);
	return(tempwork);
}

void	do_timer_bk (char *from, char *to, char *rest)
{
	char	*nuh, *nick, *temptime;
	int	Time;
	long	unban_time;

	if(!rest)
	{
		send_to_user(from, "Who do you want me to ban-kick?");
		return;
	}

	if((nick = get_token(&rest, " ")) == NULL)
	{
		send_to_user(from, "Who do you want me to ban-kick?");
		return;
	}

	if((nuh = username(nick)) == NULL)
	{
		send_to_user(from, "%s is not on this channel!", nick);
		return;
	}

	if((temptime = get_token(&rest, " ")) == NULL)
	{
		send_to_user(from, "How long do you want %s to be banned (1 - 30 minutes)?",nick);
		return;
	}

	Time = atoi(temptime);

	if(Time <= 0 || Time > 30)
	{
		send_to_user(from, "Give me a valid time moron! (1 - 30 minutes)");
		return;
	}

	Time *= 60;
	unban_time = time(NULL) + Time;

	if(protlevel(username(nick)) >= 100 && (shitlevel(username(nick)) == 0 ||
		excludelevel(username(nick)) > 0) && lvlcheck(from,username(nick)) &&
		(checkauth(username(nick)) || usermode(ischannel(to)?to:currentchannel(),nick)&MODE_CHANOP))
	{
		send_to_user(from, "%s is protected!", nick);
	}
	else
	{
		if (strlen(rest)>0)
			sprintf(tempwork, "[%s minute BK] %s", temptime, rest);
		else
			sprintf(tempwork, "[%s minute BK] User requested moron kick", temptime);

		if (ischannel(to))
		{
			ban_user_timed(nuh, to, unban_time);
			sendkick(to, nick, tempwork);
			add_to_cmdhist(from, to, "!tbk %s", nick);
		}
		else
		{
			ban_user_timed(nuh, currentchannel(), unban_time);
			sendkick(currentchannel(), nick, tempwork);
			add_to_cmdhist(from, currentchannel(), "!tbk %s %s", nick, temptime);
		}

		globallog(2, DONELOG, "%s made me ban-kick %s",from, nick);
	}
}

void	ban_user_timed(char *who, char *channel, long Time)
/*
 * Ban the user as nick and as user (two bans)
 */
{
	char			buf[MAXLEN];	/* make a copy, save original */
	char			banmask[MAXLEN];
	char			*usr = buf;
	char			*nick;
	char			*user;
	char    		*host;
	char    		*domain;
	char    		*ip2;
	char    		*ip3;
	CHAN_list	*chan;

	chan = search_chan(channel);
	strcpy(buf, who);
	nick = get_token(&usr, "!");
	user = get_token(&usr, "@");

	check_numbans(channel);
	if(*user == '~' || *user == '#')
		user++;

	host=get_token(&usr, ".");

	if (check_ip(host))
	{
		ip2 = get_token(&usr, ".");
		ip3 = get_token(&usr, ".");

		if (not(*user) || (*user =='*') || shitlevel(who) == 150)
		{
			sprintf(banmask, "*!*@%s.%s.%s.*", host, ip2, ip3);
			add_timed_ban(&(chan->banned), banmask, Time);
			sendmode(channel,"-o+b %s %s", nick, banmask);
		}
		else
		{
			sprintf(banmask, "*!*%s@%s.%s.%s.*", user, host, ip2, ip3);
			add_timed_ban(&(chan->banned), banmask, Time);
			sendmode(channel, "-o+b %s %s", nick, banmask);
		}
	}
	else
	{
		domain=get_token(&usr,"");

		if(not(*user) || (*user == '*') || shitlevel(who) == 150)/* faking login			*/
		{	/* we can't ban this user on his login,	*/
			/* and banning the nick isn't 'nuff, so	*/
			/* ban the entire site!			*/
			if (domain && strstr(domain,".")==NULL)
			{
				sprintf(banmask, "*!*@*%s.%s", host, domain);
				add_timed_ban(&(chan->banned), banmask, Time);
				sendmode(channel, "-o+b %s %s", nick, banmask);
			}
			else
			{
				sprintf(banmask, "*!*@*%s", domain ? domain : host);
				add_timed_ban(&(chan->banned), banmask, Time);
				sendmode(channel, "-o+b %s %s", nick, banmask);
			}

			return;
		}

		if (domain && strstr(domain,".")==NULL)
		{
			sprintf(banmask, "*!*%s@*%s.%s", user, host, domain);
			add_timed_ban(&(chan->banned), banmask, Time);
			sendmode(channel, "-o+b %s %s", nick, banmask);
		}
		else
		{
			sprintf(banmask, "*!*%s@*%s", user, domain?domain:host);
			add_timed_ban(&(chan->banned), banmask, Time);
			sendmode(channel, "-o+b %s %s", nick, banmask);
		}
	}
}


void do_showbans(char *from, char *to, char *rest)
{
	CHAN_list *channel;
	BAN_list *ban;

	if(!rest)
	{
		channel = search_chan(currentchannel());
	}
	else
		if((channel = search_chan(rest)) == NULL)
		{
			send_to_user(from,"I'm not on that channel freako!");
			return;
		}

	send_to_user(from, "%s Ban List", channel->name);
	send_to_user(from, "---------------------------------------------");
	send_to_user(from, "%45s | Unban Time", "Banmask");
	send_to_user(from, "---------------------------------------------");
	for(ban = channel->banned; ban; ban = ban->next)
		if(ban->unban_time == -1)
			send_to_user(from,"%45s | None", ban->banstring);
		else
			send_to_user(from,"%45s | %s", ban->banstring, time2str(ban->unban_time));
	send_to_user(from, "End of Ban List");
}

EXPIRE_list	**init_expirelist( )
{
	EXPIRE_list	**l_list;

	l_list=(EXPIRE_list **)malloc(sizeof(*l_list));
	*l_list=NULL;
	return(l_list);
}

void	delete_expirelist(EXPIRE_list **l_list )
{
	EXPIRE_list	*dummy;
	EXPIRE_list	*next;

	dummy = *l_list;
	while(dummy)
	{
		next = dummy->next;
		remove_from_expirelist(l_list, dummy->userhost);
		dummy = next;
	}
}

int	remove_from_expirelist( l_list, userhost )
EXPIRE_list	**l_list;
char		*userhost;
/*
 * removes the first occurance of userhost from l_list
 */

{
	EXPIRE_list	*old=NULL,*p=*l_list;
	EXPIRE_list	*dummy;

	if( (dummy = find_expire_userhost(l_list, userhost, 0)) == NULL )
		return(FALSE);

	while (p!=NULL && p!=dummy)
		{
		  old=p;
		  p=p->next;
		}

	if (p==NULL)
		return(FALSE);
	if (old==NULL)
		*l_list = (*l_list)->next;
	else
		old->next = p->next;

	free(p->userhost);
	free(p);
	return(TRUE);
}

EXPIRE_list	*find_expire_userhost(EXPIRE_list **l_list, char *userhost, int type)
/*
 * "type" is 0 for an exact match to "userhost" or 1 for any match.
 *
 */
{
	EXPIRE_list	*User;

	for( User = *l_list; User; User = User->next )
	{
		if (type==0)
		{
			if (STRCASEEQUAL(userhost,User->userhost))
				return(User);
		}
		else if (!mymatch(User->userhost,userhost,FNM_CASEFOLD) ||
				!mymatch(userhost,User->userhost,FNM_CASEFOLD))
			return(User);
	}
	return(NULL);
}

int 	readexpirelistdatabase(char *fname, EXPIRE_list **lvllist)
{
	FILE 	*fp;
	char 	usrhost[MAXLEN];
	long 	expiredate;

	if((fp = fopen( fname, "r")) == NULL)
	{
		printf("File \"%s\" not found, aborting\n", fname);
		exit(0);
	}

	while(read_from_expirelistfile(fp, usrhost, &expiredate))
		add_to_expirelist(lvllist, usrhost, expiredate);

	fclose(fp);
	return(TRUE);
}

int	read_from_expirelistfile(FILE *stream, char *usrhost, long *expiredate)
{
	char	s[MAXLEN];
	char	*p;

	do
	{
		p = fgets(s, MAXLEN, stream);
	}while((p != NULL) && (*s == '#'));

	if( p == NULL )
		return( FALSE );

	sscanf(s, "%s %lu", usrhost, expiredate);
	return(TRUE);
}


int	add_to_expirelist(EXPIRE_list	**l_list, char *userhost, long expiredate)
{
	EXPIRE_list	*New_user;

	if( (New_user = find_expire_userhost(l_list, userhost, 1)) != NULL )
	{
		/* If we didn't find an EXACT match, prevent dupes */
		if (!STRCASEEQUAL(New_user->userhost,userhost))
			return FALSE;

		New_user->expire_date = expiredate;
		return TRUE;
	}

	if( (New_user = (EXPIRE_list*)malloc(sizeof(*New_user))) == NULL)
		return FALSE;

	mstrcpy(&New_user->userhost, userhost);
	New_user->expire_date = expiredate;
	New_user->next = *l_list;
	*l_list = New_user;
	return TRUE;
}

int	write_expirelist( l_list, filename )
EXPIRE_list	**l_list;
char		*filename;

{
	EXPIRE_list	*dummy;
	FILE	*list_file;

	if( ( list_file = fopen( filename, "w" ) ) == NULL )
		return( FALSE );

	fprintf( list_file, "#############################################\n" );
	fprintf( list_file, "## IncuBot %s\n", filename );
	fprintf( list_file, "## Created: %s\n", get_ctime(0));
	fprintf( list_file, "## (c) 1997 Incubus\n" );
	fprintf( list_file, "#############################################\n" );

	for( dummy = *l_list; dummy; dummy = dummy->next )
	{
		fprintf( list_file, " %40s %lu\n",dummy->userhost, dummy->expire_date);
	}
	fprintf( list_file, "# End of %s\n", filename );
	fclose( list_file );
	return( TRUE );
}

void	do_expireadd	(char *from, char *to, char *rest)
{
	char	*newuser;
	char	*expirestr;
	long  expiredate;
	int	rawtime = 0;

	if (userlevel(from)==OTHER_BOT_LEVEL)
		return;

	if((newuser = get_token(&rest, " ")) == NULL)
	{
		send_to_user( from, "Umm. I give me a shitmask freak!" );
		return;
	}

	if((expirestr = get_token(&rest, " ")) == NULL)
	{
		send_to_user( from, "How many days until it expires?" );
		return;
	}

	if(STRCASEEQUAL(expirestr, "-RAW"))
	{
		rawtime = 1;
		if((expirestr = get_token(&rest, " ")) == NULL)
		{
			send_to_user( from, "How many days until it expires?" );
			return;
		}
	}

	sscanf(expirestr, "%ld", &expiredate);

	if(expiredate < 0)
	{
		send_to_user(from, "Must be greater than 0");
		return;
	}


	if(!rawtime)
	{
		expiredate *= 86400;
		expiredate += time(NULL);
	}

	if(!find_userhost(currentbot->lists->shitlist, newuser, 0))
	{
		sendreply("%s is not shitted!",newuser);
		return;
	}

	if (!add_to_expirelist(currentbot->lists->expirelist, newuser, expiredate))
	{
		sendreply("There is already a record that matches %s",newuser);
		return;
	}

	send_to_user(from, "Shitlist mask %s will expire on %s",newuser, time2str(expiredate));
	globallog(2, DONELOG, "Shitlist mask %s set to expire on %s by %s", newuser, time2str(expiredate), from);
	notify_expireadd(newuser, expiredate, from);
}

void	do_expiredel(char *from, char *to, char *rest)
{
	char	*deluser;

	if (userlevel(from)==OTHER_BOT_LEVEL)
		return;

	if((deluser = get_token(&rest, " ")) == NULL)
	{
		send_to_user( from, "Who do you want me to un-expire?" );
		return;
	}

	if (!remove_from_expirelist(currentbot->lists->expirelist,deluser))
	{
		send_to_user(from, "%s has no expiration date!", deluser);
		return;
	}

	send_to_user(from, "Expiration date for %s has been removed",deluser);

	globallog(2, DONELOG, "Expiration date for %s removed by %s", deluser, from);
	notify_expiredel(deluser, from);
}

void	show_expire_list(char *from, char *to, char *rest)
{
	send_to_user(from, "%s Expire List", currentbot->nick);
	send_to_user(from, "------------------------------------------------");
	send_to_user(from, " %-25s | %s", "Expiration date", "nick!user@host");
	send_to_user(from, "------------------------------------------------");
	show_expirelist_pattern("",currentbot->lists->expirelist, from, rest?rest:"");
	send_to_user(from, "End of Expire List");

}

int	show_expirelist_pattern (char *pretxt, EXPIRE_list **l_list, char *from, char *userhost)
{
	int				match=0;
	EXPIRE_list		*dummy;

	for( dummy = *l_list; dummy; dummy = dummy->next )
	{
		if(!mymatch(userhost, dummy->userhost, FNM_CASEFOLD) || !*userhost)
		{
			if(!*userhost)
				send_to_user( from, "%s %25s | %s", pretxt, time2str(dummy->expire_date), dummy->userhost);
			else
				send_to_user( from, "%s %s %s", pretxt, dummy->userhost, time2str(dummy->expire_date));
			match++;
		}
	}

	return match;
}

void    do_expirewrite(char *from, char *to, char *rest)
{
	if(!write_expirelist(currentbot->lists->expirelist, currentbot->lists->expirefile))
	{
		send_to_user(from, "Expirelist could not be written to file %s",
			currentbot->lists->expirefile);

	}
	else
		send_to_user(from, "Expirelist written to file %s", currentbot->lists->expirefile);
}

int check_expirations(EXPIRE_list **e_list)
{
	EXPIRE_list	*dummy;

	for(dummy = *e_list; dummy; dummy = dummy->next)
		if(dummy->expire_date <= time(NULL))
		{
			remove_from_levellist(currentbot->lists->shitlist, dummy->userhost);
			remove_from_levellist(currentbot->lists->shitlist2, dummy->userhost);
			globallog(2, DONELOG, "Shitmask %s was removed because it expired.", dummy->userhost);
			add_list_event("Shitmask %s was removed because it expired.", dummy->userhost);
			remove_from_expirelist(e_list, dummy->userhost);
			write_expirelist(currentbot->lists->expirelist, currentbot->lists->expirefile);
			write_lvllist(currentbot->lists->shitlist,currentbot->lists->shitfile);
			write_passwdlist(currentbot->lists->shitlist2,currentbot->lists->shitfile2);
			return 1;
		}
	return 0;
}

void show_shitlist2(USERLVL_list **l_list1, USERLVL_list **l_list2, char *from, char *to, char *rest)
{
	USERLVL_list	*dummy=NULL;
	char	*arg1, *arg2;
	int temp = 0, level;

	if ((arg1 = get_token(&rest," ")) == NULL)
	{
		send_to_user(from, "Invalid switch");
		return;
	}

	if(STRCASEEQUAL(arg1, "-reason"))
		temp = 1;
	if(STRCASEEQUAL(arg1, "-level"))
		temp = 2;
	if(STRCASEEQUAL(arg1, "-whoshitted") || STRCASEEQUAL(arg1, "-who"))
		temp = 3;
	if(STRCASEEQUAL(arg1, "-whenshitted") || STRCASEEQUAL(arg1, "-date"))
		temp = 4;
	if(STRCASEEQUAL(arg1, "-mask"))
		temp = 5;

	if(!temp)
	{
		send_to_user(from, "Invalid switch");
		return;
	}

	if ((arg2 = get_token(&rest," ")) == NULL)
	{
		send_to_user(from, "Must specify a %s", temp == 2? "level":"pattern");
		return;
	}

	if(temp == 2)
		sscanf(arg2, "%d", &level);

	send_to_user(from, "%s Shit List", currentbot->nick);
	send_to_user(from, "----------------------------------------------------------------------");
	switch(temp)
	{
		case 1:
			send_to_user(from, " %-45s | Shit Reason", "nick!user@host");
			dummy = *l_list2;
			break;
		case 2:
			send_to_user(from, " %-45s | Shit Level", "nick!user@host");
			dummy = *l_list1;
			break;
		case 3:
			send_to_user(from, " %-45s | Who Shitted", "nick!user@host");
			dummy = *l_list1;
			break;
		case 4:
			send_to_user(from, " %-45s | Shit Date", "nick!user@host");
			dummy = *l_list1;
			break;
		case 5:
			send_to_user(from, " %-45s", "nick!user@host");
			dummy = *l_list1;
			break;
	}
	send_to_user(from, "----------------------------------------------------------------------");

	for(; dummy; dummy = dummy->next )
	{
		switch(temp)
		{
			case 1:
				if(!mymatch(arg2, dummy->passwd, FNM_CASEFOLD))
					send_to_user(from, " %-45s | %s", dummy->userhost, dummy->passwd);
				break;
			case 2:
				if(dummy->access == level)
					send_to_user(from, " %-45s | %d", dummy->userhost, dummy->access);
				break;
			case 3:
				if(!mymatch(arg2, dummy->mailaddr, FNM_CASEFOLD))
					send_to_user(from, " %-45s | %s", dummy->userhost, dummy->mailaddr);
				break;
			case 4:
				if(!mymatch(arg2, dummy->nick, FNM_CASEFOLD))
					send_to_user(from, " %-45s | %s", dummy->userhost, dummy->nick);
				break;
			case 5:
				if(!mymatch(arg2, dummy->userhost, FNM_CASEFOLD))
					send_to_user(from, " %-45s", dummy->userhost);
				break;
		}
	}
	send_to_user(from, "End of Shit List");
}
char *get_fp_typestr(int setting)
{
	switch(setting)
	{
		case 0:
				strcpy(tempwork, "OFF");
				break;
		case 1:
				strcpy(tempwork, "KICK");
				break;
		case 2:
				strcpy(tempwork, "BANKICK");
				break;
		case 3:
				strcpy(tempwork, "TBK");
				break;
	}
	return tempwork;
}
int get_fp_typenum(char *setting)
{
		if(STRCASEEQUAL(setting, "OFF"))
		{
			return 0;
		}
		if(STRCASEEQUAL(setting, "KICK"))
		{
			return 1;
		}
		if(STRCASEEQUAL(setting, "BANKICK"))
		{
			return 2;
		}
		if(STRCASEEQUAL(setting, "TBK"))
		{
			return 3;
		}
		return -1; /* Error */
}
void floodevent(char *channel, char *from, int type, char *kicktext)
{
	long	unban_time;
   char	kicktext2[128];

   switch(type)
   {
   	case 1:
			sendkick(channel,getnick(from),kicktext);
      	break;
   	case 2:
			check_numbans(channel);
			sprintf(kicktext2,"%s [BK]",kicktext);
			ban_user(from,channel);
			sendkick(channel,getnick(from),kicktext2);
      	break;
   	case 3:
			check_numbans(channel);
			unban_time = time(NULL) + 300;
			sprintf(kicktext2,"%s [5 minute BK]",kicktext);
			ban_user_timed(from,channel,unban_time);
			sendkick(channel,getnick(from),kicktext2);
      	break;
   }
}
void do_chanst(char *from, char *to, char *rest)
{
	CHAN_list	*Channel;
	USER_list	*User;
	char			chan[128];
	int			opnum = 0;
	int			nopnum = 0;
	int			voicenum = 0;

   if(rest)
	{
		if(ischannel(rest))
			strcpy(chan,rest);
	}
	else if(ischannel(to))
		strcpy(chan,to);
	else
		strcpy(chan,currentchannel());


	if ((Channel=search_chan(chan))==NULL)
	{
		send_to_user(from,"I'm not on %s", chan);
		return;
	}

	for (User=Channel->users; User; User=User->next)
	{
		if(usermode(chan, User->nick)&MODE_CHANOP)
			opnum++;
		else
		{
			nopnum++;
			if(usermode(chan, User->nick)&MODE_VOICE)
				voicenum++;
		}
	}

	send_to_user(from,"Channel Statistics for %s:", Channel->name);
	send_to_user(from,"Ops: %2d Non-ops: %2d Voices: %2d Total: %2d", opnum, nopnum, voicenum, opnum+nopnum);
}

void do_chuserlevel(char *from, char *to, char *rest)
{
	USERLVL_list *dummy;
	char *newuser;
	char *newlevel;
	int	already;

	if((newuser = get_token(&rest, " ")) == NULL)
	{
		send_to_user(from, "Which shitlist entry do you want to alter?");
		return;
	}

	if((newlevel = get_token(&rest, " ")) == NULL )
	{
		send_to_user(from, "What level should %s have?", newuser);
		return;
	}

	if(!checklevel(atoi(newlevel)))
	{
		send_to_user(from, "Invalid level: %s", newlevel);
		return;
	}

	if(atoi(newlevel) > userlevel(from) && userlevel(from) < 150)
	{
		send_to_user(from, "Sorry bro, can't do that!");
		return;
	}

	if(userlevel(from) < userlevel(newuser) && userlevel(from) < 150)
	{
		send_to_user(from, "Sorry, %s has a higher level", newuser);
		return;
	}

	if ((dummy = find_userhost(currentbot->lists->opperlist, newuser, 0))==NULL)
	{
		send_to_user(from, "%s is not u user", newuser);
		return;
	}

   already = dummy->access;

	if(already == atoi(newlevel))
	{
		send_to_user(from, "%s is already level %d", newuser, already);
		return;
	}

	dummy->access = atoi(newlevel);
	send_to_user(from, "Changed the userlevel for %s from %d to %d", newuser, already, dummy->access);
	add_list_event("%s changed the userlevel for %s from %d to %d", from, newuser, already, dummy->access);
	globallog(2, DONELOG, "%s changed the userlevel for %s to %d",
		from, newuser, atoi(newlevel));

}

void do_nchuserlevel(char *from, char *to, char *rest)
{
	USERLVL_list	*dummy;
	char				*newuser;
	char				*newlevel;
	char				*nuh;
	int				already;

	if((newuser = get_token(&rest, " ")) == NULL)
	{
		send_to_user(from, "Which shitlist entry do you want to alter?");
		return;
	}

	if((nuh=username(newuser))==NULL)
	{
		sendreply("%s is not on this channel!", newuser);
		return;
	}

	if((newlevel = get_token(&rest, " ")) == NULL )
	{
		send_to_user(from, "What level should %s have?", newuser);
		return;
	}

	if(!checklevel(atoi(newlevel)))
	{
		send_to_user(from, "Invalid level: %s", newlevel);
		return;
	}
	if(atoi(newlevel) > userlevel(from) && userlevel(from) < 150)
	{
		send_to_user(from, "Sorry bro, can't do that!");
		return;
	}

	if(userlevel(from) < userlevel(nuh) && userlevel(from) < 150)
	{
		sendreply("Sorry, %s has a higher level", newuser);
		return;
	}

	if ((dummy = find_userhost(currentbot->lists->opperlist, nuh, 1))==NULL)
	{
		send_to_user(from, "%s is not a user", newuser);
		return;
	}

	already = dummy->access;

	if(already == atoi(newlevel))
	{
		send_to_user(from, "%s is already level %d", newuser, already);
		return;
	}

	dummy->access = atoi(newlevel);
	send_to_user(from, "Changed the userlevel for %s from %d to %d", dummy->userhost, already, dummy->access);
	add_list_event("%s changed the userlevel for %s from %d to %d", from, dummy->userhost, already, dummy->access);
	globallog(2, DONELOG, "%s changed the userlevel for %s to %d",
		from, dummy->userhost, dummy->access);
}

void    do_send_list(char *from, char *to, char *rest)
/*
 *		DCC sends one or more of the bot's list files.
 *
 *		List files:
 *			user.list
 *			passwd.list
 *			shit.list
 *			shit.list2
 *			exclude.list
 *			expire.list
 *			access.list
 *			prot.list
 */
{
	if(!rest)
	{
		send_to_user(from, "Please specify a file. Valid files are: USER, PASSWD, SHIT, EXCLUDE, EXPIRE, PROT, ACCESS");
		return;
	}

	if(STRCASEEQUAL(rest, "USER"))
	{
		send_list_file(from, currentbot->lists->opperfile, (char *)NULL);
		return;
	}

	if(STRCASEEQUAL(rest, "SHIT"))
	{
		send_list_file(from, currentbot->lists->shitfile, currentbot->lists->shitfile2);
		return;
	}

	if(STRCASEEQUAL(rest, "PASSWD"))
	{
		send_list_file(from, currentbot->lists->passwdfile, (char *)NULL);
		return;
	}

	if(STRCASEEQUAL(rest, "EXCLUDE"))
	{
		send_list_file(from, currentbot->lists->excludefile, (char *)NULL);
		return;
   }

	if(STRCASEEQUAL(rest, "EXPIRE"))
	{
		send_list_file(from, currentbot->lists->expirefile, (char *)NULL);
		return;
	}

	if(STRCASEEQUAL(rest, "PROT"))
	{
		send_list_file(from, currentbot->lists->protfile, (char *)NULL);
		return;
	}

	if(STRCASEEQUAL(rest, "ACCESS"))
	{
		send_list_file(from, currentbot->lists->accessfile, (char *)NULL);
		return;
	}

	send_to_user(from, "Invalid file. Valid files are: USER, PASSWD, SHIT, EXCLUDE, EXPIRE, PROT, ACCESS");
}

void	send_list_file(char *from, char *name1, char *name2)
{
	char	*dir_list[128];
	int	n = 1, i;
	
	botlog(DCCLOGFILE, "DCC get from %s: %s %s received", from, name1, name2?name2:"");

	dir_list[0] = strdup(name1);

	if(name2)
		dir_list[n++] = strdup(name2);

	dcc_sendlist(from, "\010 \010" , dir_list, n);

	for(i=0; i<n; i++)
		free(dir_list[i]);
}
void	do_nhuseradd(char *from, char *to, char *rest)
{
	char	str[WAYTOBIG];

	sprintf(str, "-fullhost %s", rest);
	do_nuseradd(from, to, str);
}
void add_to_cmdhist(char *userhost, char *channel, char *format, ...)
{
	char 	buf[WAYTOBIG];
	va_list	msg;
	HISTORY_list	*temp1;
	HISTORY_list	*temp2;

	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);

	if((temp1 = (HISTORY_list *)malloc(sizeof(HISTORY_list))) == NULL)
		return;

	temp1->userhost = strdup(userhost);
	temp1->command = strdup(buf);
	temp1->channel = strdup(channel);
	temp1->next = NULL;

	if (currentbot->cmdhistory == NULL)
	{
		currentbot->cmdhistory = temp1;
		currentbot->num_cmdhist++;
	}
	else
	{
		if(currentbot->num_cmdhist > 19)
		{
			temp2 = currentbot->cmdhistory;
			currentbot->cmdhistory = temp2->next;
			free(temp2);
		}
		else
			currentbot->num_cmdhist++;

		for(temp2 = currentbot->cmdhistory; temp2->next != NULL; temp2 = temp2->next);

		temp2->next = temp1;
	}	
}
void show_cmdhistory(char *from, char *to, char *rest)
{
	HISTORY_list	*dummy;

	send_to_user(from, "----------------------------------------------------------------------");
	send_to_user(from, " %-45s | %15s | %s", "Userhost", "Channel", "Command");
	send_to_user(from, "----------------------------------------------------------------------");
	for(dummy = currentbot->cmdhistory; dummy; dummy = dummy->next)
	{
		send_to_user(from, " %-45s | %15s | %s", dummy->userhost, dummy->channel, dummy->command);
	}
	send_to_user(from, "End of Command History");
}
void delete_cmdhistory(void)
{
	HISTORY_list	*temp;

	while(currentbot->cmdhistory)
	{
		temp = currentbot->cmdhistory->next;
		free(currentbot->cmdhistory->userhost);
		free(currentbot->cmdhistory->channel);
		free(currentbot->cmdhistory->command);
		free(currentbot->cmdhistory);
		currentbot->cmdhistory = temp;
	}
}

void do_chusermask(char *from, char *to, char *rest)
{
	USERLVL_list	*dummy;
	char	*user;
	char	*newuser;
	char	writtendesc[80] = "", writtenfile[80] = "";
	char	unwrittendesc[512] = "", unwrittenfile[512] = "";
	int	foundpasswd = 0, foundprot = 0;
	int	wroteuser = 0, wrotepasswd = 0, wroteprot = 0;
	int	numpassed = 0, numfailed = 0;

	if ((user=get_token(&rest," "))==NULL)
	{
		send_to_user(from, "Which usermask do you want to change?");
		return;
	}

	if ((newuser=get_token(&rest," "))==NULL)
	{
		send_to_user(from, "What do you want to change %s to?", user);
		return;
	}

	if (find_userhost(currentbot->lists->opperlist, newuser, 0)!=NULL)
	{
		send_to_user(from, "A userlist entry matching %s already exists", newuser);
		return;
	}

	if ((dummy = find_userhost(currentbot->lists->opperlist, user, 0))==NULL)
	{
		send_to_user(from, "%s is not a user", user);
		return;
	}

	if (dummy->access > userlevel(from) && userlevel(from) < 150)
	{
		send_to_user(from, "Sorry, %s has a higher level.", user);
		return;
	}

	free(dummy->userhost);
	dummy->userhost = strdup(newuser);

	if ((dummy = find_userhost(currentbot->lists->passwdlist, user, 0))!=NULL)
	{
		foundpasswd = 1;
		free(dummy->userhost);
		dummy->userhost = strdup(newuser);
	}
	if ((dummy = find_userhost(currentbot->lists->protlist, user, 0))!=NULL)
	{
		foundprot = 1;
		free(dummy->userhost);
		dummy->userhost = strdup(newuser);
	}
	send_to_user(from, "Changed usermask for %s to %s", user, newuser);
	globallog(2, DONELOG, "%s changed the usermask for %s to %s",
		from, user, newuser);
	add_list_event("%s changed the usermask for %s to %s", from, user, newuser);

/*
   This is a mess, but it keeps the bot from sending 3 additional lines to
   the user. If all files are written successfully, only one line is sent.
   If some are written and some are not, two lines ar sent. If all files
   cannot be written, one line is sent.
*/
	if(write_lvllist(currentbot->lists->opperlist, currentbot->lists->opperfile))
	{
		wroteuser = 1;
		numpassed++;
	}
	else
		numfailed++;
	if(foundpasswd)
	{
		if(write_passwdlist(currentbot->lists->passwdlist, currentbot->lists->passwdfile))
		{
			wrotepasswd = 1;
			numpassed++;
		}
		else
			numfailed++;
	}

	if(foundprot)
	{
		if(write_lvllist(currentbot->lists->protlist, currentbot->lists->protfile))
		{
			wroteprot = 1;
			numpassed++;
		}
		else
			numfailed++;
	}

	if(wroteuser)
	{
		strcat(writtendesc, "Userlist");
		strcat(writtenfile, currentbot->lists->opperfile);
		if(numpassed == 3)
		{
			strcat(writtendesc, ", ");
			strcat(writtenfile, ", ");
		}
		else
			if(numpassed == 2)
			{
				strcat(writtendesc, " and ");
				strcat(writtenfile, " and ");
			}
	}
	else
	{
		strcat(unwrittendesc, "Userlist");
		strcat(unwrittenfile, currentbot->lists->opperfile);
		if(numfailed == 3)
		{
			strcat(unwrittendesc, ", ");
			strcat(unwrittenfile, ", ");
		}
		else
			if(numfailed == 2)
			{
				strcat(unwrittendesc, " and ");
				strcat(unwrittenfile, " and ");
			}
	}
	if(foundpasswd)
	{
		if(wrotepasswd)
		{
			if(strlen(writtendesc))
			{
				strcat(writtendesc, "passwdlist");
				strcat(writtenfile, currentbot->lists->passwdfile);
			}
			else
			{
				strcat(writtendesc, "Passwdlist");
				strcat(writtenfile, currentbot->lists->passwdfile);
			}
			if(numpassed == 3)
			{
				strcat(writtendesc, ", and ");
				strcat(writtenfile, ", and ");
			}
			else
				if(numpassed == 2)
				{
					strcat(writtendesc, " and ");
					strcat(writtenfile, " and ");
				}
		}
		else
		{
			if(strlen(unwrittendesc))
			{
				strcat(unwrittendesc, "passwdlist");
				strcat(unwrittenfile, currentbot->lists->passwdfile);
			}
			else
			{
				strcat(unwrittendesc, "Passwdlist");
				strcat(unwrittenfile, currentbot->lists->passwdfile);
			}
			if(numfailed == 3)
			{
				strcat(unwrittenfile, ", and ");
				strcat(unwrittenfile, ", and ");
			}
			else
				if(numfailed == 2)
				{
					strcat(unwrittenfile, " and ");
					strcat(unwrittenfile, " and ");
				}
		}
	}
	if(foundprot)
	{
		if(wroteprot)
		{
			if(strlen(writtendesc))
			{
				strcat(writtendesc, "protlist");
				strcat(writtenfile, currentbot->lists->protfile);
			}
			else
			{
				strcat(writtendesc, "Protlist");
				strcat(writtenfile, currentbot->lists->protfile);
			}
		}
		else
		{
			if(strlen(unwrittendesc))
			{
				strcat(unwrittendesc, "protlist");
				strcat(unwrittenfile, currentbot->lists->protfile);
			}
			else
			{
				strcat(unwrittendesc, "Protlist");
				strcat(unwrittenfile, currentbot->lists->protfile);
			}
		}
	}

	if(numpassed)
		send_to_user(from, "%s written to file%s %s", writtendesc, numpassed>1?"s":"", writtenfile);
	if(numfailed)
		send_to_user(from, "%s written to file%s %s", unwrittendesc, numfailed>1?"s":"", unwrittenfile);
	if(!foundpasswd)
		send_to_user(from, "Warning: Could not find an exact match to %s in the passwdlist.", user);
}

int isvalidchan(char *chan)
{
	if(*chan != '#' && *chan != '&')
		return 0;
	else
		chan++;

	if(*chan == '\0')
		return 0;

	return 1;
}

void do_readfp(char *from, char *to, char *rest)
{
	FILE *f;
	char s[101];

	f=fopen(currentbot->lists->fpfile,"rt");

	if (f==NULL)
	{
		fprintf(stderr,"You don't have a %s file for the floodprot settings",
			currentbot->lists->fpfile);
		return;
	}

	while (!feof(f))
	{
		fgets(s,100,f);
		do_fchfp(s);
	}

	if (strcmp(from,"bot")!=0) send_to_user(from,"Floodprot settings re-read");
		fclose(f);
}

void	do_fchfp(char *rest)
{
	char	*type,*setting;
	int	newsetting;

	if ((type=get_token(&rest," "))==NULL)
	{
		return;
	}

	if (*type=='#')
		return;

	if ((setting=get_token(&rest," "))==NULL)
	{
		return;
	}

	newsetting=atoi(setting);

	if (newsetting<0)
	{
		return;
	}

	if (STRCASEEQUAL(type,"NICK"))
	{
		currentbot->floodprot_nick = newsetting;
	}
	else if (STRCASEEQUAL(type,"PUBLIC"))
	{
		currentbot->floodprot_public = newsetting;
	}
	else if (STRCASEEQUAL(type,"CLONE"))
	{
		currentbot->floodprot_clone = newsetting;
	}
	else if (STRCASEEQUAL(type,"CTCP"))
	{
		currentbot->floodprot_ctcp = newsetting;
	}
	else if (STRCASEEQUAL(type,"OP"))
	{
		currentbot->floodprot_op = newsetting;
	}
	else if (STRCASEEQUAL(type,"AWP"))
	{
		currentbot->floodprot_awp = newsetting;
	}
}

void	do_fpwrite	(char *from, char *to, char *rest)
{
	FILE	*af;

	if ((af=fopen(currentbot->lists->fpfile,"wt"))==NULL)
	{
		send_to_user(from, "Could not save flood prot settings to %s",
			currentbot->lists->fpfile);
		return;
	}

	fprintf(af,"#############################################\n");
	fprintf(af,"## IncuBot %s\n",currentbot->lists->fpfile);
	fprintf(af,"## Created: %s\n",get_ctime(0));
	fprintf(af,"## (c)1997 Incubus\n");
	fprintf(af,"## Format: Type Setting\n");
	fprintf(af,"#############################################\n");

	fprintf(af,"%-15s %d\n", "NICK", currentbot->floodprot_nick);
	fprintf(af,"%-15s %d\n", "PUBLIC",  currentbot->floodprot_public);
	fprintf(af,"%-15s %d\n", "CLONE",  currentbot->floodprot_clone);
	fprintf(af,"%-15s %d\n", "CTCP",  currentbot->floodprot_ctcp);
	fprintf(af,"%-15s %d\n", "OP",  currentbot->floodprot_op);
	fprintf(af,"%-15s %d\n", "AWP",  currentbot->floodprot_awp);

	fprintf(af,"# End of %s\n",currentbot->lists->fpfile);
	fclose(af);
	if(to)
		sendreply("Floodprot settings written to file %s",currentbot->lists->fpfile);
}
void do_voice(char *from, char *to, char *rest)
{
	char	*voicechan;
	int   allocated = 0;

	if (!userchannel(getnick(from)))
	{
		send_to_user(from, "You're not on channel fool!");
		return;
	}
	if(public_command == TRUE && addressed_to_me == FALSE)
	{
		send_to_user(from, "That command must be addressed to me if used in public.");
		return;
	}
	if ((voicechan=get_token(&rest," "))==NULL)
	{
		if (STRCASEEQUAL(to, currentbot->nick))
			voicechan = userchannel(getnick(from));
		else
		{
			voicechan = strdup(to);
			allocated = 1;
		}
	}
	if (search_chan(voicechan)==NULL)
	{
		send_to_user(from, "I'm not in that channel freako!");
		if(allocated)
			free(voicechan);
		return;
	}

	if(!(usermode(voicechan, currentbot->nick)&MODE_CHANOP))
		send_to_user(from, "I'm not opped on %s!", voicechan);
	else
		if(!(usermode(voicechan, getnick(from))&MODE_VOICE))
			sendmode(voicechan, "+v %s", getnick(from));

	if(allocated)
		free(voicechan);
}
int isnumeric(char *num)
{
	int i;

	for(i = 0; num[i] != '\0'; i++)
	{
		if(num[i] != '0' && num[i] != '1' && num[i] != '2' && num[i] != '3' && num[i] != '4' && num[i] != '5' && num[i] != '6' && num[i] != '7' && num[i] != '8' && num[i] != '9')
			return 0;
	}

	return 1;
}
void do_devoice(char *from, char *to, char *rest)
{
	char	*voicechan;
	int   allocated = 0;

	if (!userchannel(getnick(from)))
	{
		send_to_user(from, "You're not on channel fool!");
		return;
	}
	if(public_command == TRUE && addressed_to_me == FALSE)
	{
		send_to_user(from, "That command must be addressed to me if used in public.");
		return;
	}
	if ((voicechan=get_token(&rest," "))==NULL)
	{
		if (STRCASEEQUAL(to, currentbot->nick))
			voicechan = userchannel(getnick(from));
		else
		{
			voicechan = strdup(to);
			allocated = 1;
		}
	}
	if (search_chan(voicechan)==NULL)
	{
		send_to_user(from, "I'm not in that channel freako!");
		if(allocated)
			free(voicechan);
		return;
	}

	if(!(usermode(voicechan, currentbot->nick)&MODE_CHANOP))
		send_to_user(from, "I'm not opped on %s!", voicechan);
	else
		if(usermode(voicechan, getnick(from))&MODE_VOICE)
			sendmode(voicechan, "-v %s", getnick(from));

	if(allocated)
		free(voicechan);
}
void set_l25_flag(int flag, int sign)
{
	if(sign)
		currentbot->l_25_flags = currentbot->l_25_flags | flag;
	else
		if(currentbot->l_25_flags & flag)
			currentbot->l_25_flags = currentbot->l_25_flags - flag;
}
void change_l25_flags(char *from, char *to, char *rest)
{
	char *flagstr;
	char badsigns[128] = "";
	char signerrstr[512] = "";
	char flagerrstr[512] = "";
	int sign;
	int badsignlen = 0;
	int numsignerrs = 0;
	int numflagerrs = 0;
	int numsuccess = 0;

	if(!rest)
	{
		send_to_user(from, "What permissions do you want me to set?");
		return;
	}

	while((flagstr = get_token(&rest, " ")) != NULL)
	{
		if(*flagstr != '+' && *flagstr != '-')
		{
				badsigns[badsignlen] = ' ';
				badsigns[badsignlen+1] = *flagstr;
				badsigns[badsignlen+2] = '\0';
				strcat(signerrstr, " ");
				strcat(signerrstr, flagstr);
				badsignlen += 2;
				numflagerrs++;
		}
		else
		{
			if(*flagstr == '+')
				sign = 1;
			else
				sign = 0;

			flagstr++;
			numsuccess++;

			if(STRCASEEQUAL("NOBANKICK", flagstr))
				set_l25_flag(FL_NOBANKICK, sign);
			else if(STRCASEEQUAL("NOOPKICK", flagstr))
				set_l25_flag(FL_NOOPKICK, sign);
			else if(STRCASEEQUAL("NOPUBLICKICK", flagstr))
				set_l25_flag(FL_NOPUBLICKICK, sign);
			else if(STRCASEEQUAL("NOCLONEKICK", flagstr))
				set_l25_flag(FL_NOCLONEKICK, sign);
			else if(STRCASEEQUAL("NOMASSNK", flagstr))
				set_l25_flag(FL_NOMASSNK, sign);
			else if(STRCASEEQUAL("CANHAVEOPS", flagstr))
				set_l25_flag(FL_CANHAVEOPS, sign);
			else
			{
				strcat(flagerrstr, " ");
				strcat(flagerrstr, flagstr);
				numflagerrs++;
				numsuccess--;
			}
		}
	}
	if(numsignerrs)
		send_to_user(from, "Bad sign%s:%s. Ignoring:%s", numsignerrs==1?"":"s", badsigns, signerrstr);
	if(numflagerrs)
		send_to_user(from, "Unknown flag%s:%s", numflagerrs>1?"s":"", flagerrstr);
	if(numsuccess)
	{
		sign = currentbot->l_25_flags;
		send_to_user(from, "Level 25 flags now set to:");
		send_to_user(from, "%12s = %-3s. %12s = %-3s. %12s = %-3s.", "NOBANKICK",
		                                                          sign&FL_NOBANKICK?"ON":"OFF",
		                                                          "NOOPKICK",
		                                                          sign&FL_NOOPKICK?"ON":"OFF",
		                                                          "NOPUBLICKICK",
		                                                          sign&FL_NOPUBLICKICK?"ON":"OFF");
		send_to_user(from, "%12s = %-3s. %12s = %-3s. %12s = %-3s.", "NOCLONEKICK",
		                                                          sign&FL_NOCLONEKICK?"ON":"OFF",
		                                                          "NOMASSNK",
		                                                          sign&FL_NOMASSNK?"ON":"OFF",
		                                                          "CANHAVEOPS",
		                                                          sign&FL_CANHAVEOPS?"ON":"OFF");
	}
}
