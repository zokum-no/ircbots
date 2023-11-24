#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"
#include "vladbot.h"
#include "misc.h"
#include "send.h"
#include "log.h"
#include "fnmatch.h"
#include "vlad-ons.h"
#include "tb-ons.h"
#include "shitlist.h"

extern botinfo	*currentbot;
extern int		public_command;
extern int		addressed_to_me;

char tempwork[WAYTOBIG];

shitlist_t	**init_shitlist(void)
{
	shitlist_t	**list;

	list = (shitlist_t **)malloc(sizeof(*list));
	*list=NULL;
	return(list);
}

void	delete_shitlist(shitlist_t **list)
{
	shitlist_t	*dummy;
	shitlist_t	*next;

	dummy = *list;
	while(dummy)
	{
		next = dummy->next;
		remove_from_shitlist(list, dummy->mask);
		dummy = next;
	}
}

shitlist_t *find_shitmask(shitlist_t **list, char *mask, int type)
{
/*
 * "type" is 0 for an exact match to "" or 1 for any match.
 */
	shitlist_t *entry;

	for(entry = *list; entry; entry = entry->next)
	{
		if (type==0)
		{
			if (STRCASEEQUAL(mask,entry->mask))
				return(entry);
		}
		else if (!mymatch(entry->mask, mask, FNM_CASEFOLD) || !mymatch(mask, entry->mask, FNM_CASEFOLD))
			return(entry);
	}
	return(NULL);
}

int add_to_shitlist(shitlist_t **list, char *mask, int level, char *whoshitted, char *whenshitted, char *whyshitted, long expire)
{
	shitlist_t	*new_entry;

	if((new_entry = find_shitmask(list, mask, 1)) != NULL )
	{
		/* If we didn't find an EXACT match, prevent dupes */
		if (!STRCASEEQUAL(new_entry->mask,mask))
			return FALSE;

		new_entry->level = level;
		free(new_entry->whyshitted);
		free(new_entry->whenshitted);
		free(new_entry->whoshitted);
		new_entry->whyshitted = strdup(whyshitted);
		new_entry->whenshitted = strdup(whenshitted);
		new_entry->whoshitted = strdup(whoshitted);
		new_entry->expire = expire;
		return TRUE;
	}

	if((new_entry = (shitlist_t*)malloc(sizeof(*new_entry))) == NULL)
		return FALSE;

	new_entry->mask = strdup(mask);
	new_entry->whyshitted = strdup(whyshitted);
	new_entry->whenshitted = strdup(whenshitted);
	new_entry->whoshitted = strdup(whoshitted);
	new_entry->level = level;
	new_entry->expire = expire;
	new_entry->next = *list;
	*list = new_entry;
	return TRUE;
}

void do_whoshitted(char *from, char *to, char *rest)
{
	shitlist_t *dummy;

	if (!rest)
	{
		send_to_user(from, "Please specify a mask to query shit info on.");
		return;
	}

	if((dummy = find_shitmask(currentbot->lists->shitlist, rest, 0))!=NULL)
	{
		send_to_user(from, "%s was shitted by %s.", rest, dummy->whoshitted);
		return;
	}
	else
		send_to_user(from, "No shit information for %s stored", rest);
}

void do_whenshitted(char *from, char *to, char *rest)
{
	shitlist_t *dummy;

	if (!rest)
	{
		send_to_user(from, "Please specify a mask to query shit info on.");
		return;
	}
	if ((dummy = find_shitmask(currentbot->lists->shitlist, rest, 0))!=NULL)
		send_to_user(from, "%s was shitted on %s.", rest, dummy->whenshitted);
	else
		send_to_user(from, "%s is not shitted", rest);
}

void do_chshitlevel(char *from, char *to, char *rest)
{
	shitlist_t *dummy;
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

	if ((dummy = find_shitmask(currentbot->lists->shitlist, newuser, 0))==NULL)
	{
		send_to_user(from, "%s is not shitted", newuser);
		return;
	}

	already = dummy->level;

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
	dummy->level = atoi(newlevel);
	send_to_user(from, "Changed the shitlevel for %s from %d to %d", newuser, already, dummy->level);
	add_list_event("%s changed the shitlevel for %s from %d to %d", from, newuser, already, dummy->level);
	globallog(2, DONELOG, "%s changed the shitlevel for %s to %d",	from, newuser, atoi(newlevel));
}

void do_chwhenshitted(char *from, char *to, char *rest)
{
	shitlist_t *dummy;
	char *user;
	char *newdate;
	char *old;

	if ((user=get_token(&rest," "))==NULL)
	{
		send_to_user(from, "Which shitlist entry do you want to change?");
		return;
	}

	if ((newdate=get_token(&rest," "))==NULL)
	{
		send_to_user(from, "What date do you want to add for %s?",user);
		return;
	}

	if (!check_date(newdate))
	{
		send_to_user(from, "Invalid date. Must be in mm/dd/yy format");
		return;
	}

	if ((dummy = find_shitmask(currentbot->lists->shitlist, user, 0))==NULL)
	{
		send_to_user(from, "%s is not shitted", user);
		return;
	}

	old = dummy->whenshitted;
	dummy->whenshitted = strdup(newdate);
	send_to_user(from, "Changed shit date for %s from %s to %s", user, old, newdate);
	add_list_event("%s changed the shit date for %s from %s to %s", from, user, old, newdate);
	globallog(2, DONELOG, "%s changed the shit date for %s to %s",	from, user, newdate);
	free(old);
	if(!write_shitlist(currentbot->lists->shitlist, currentbot->lists->shitfile))
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
	shitlist_t	*dummy;
	char			*user;

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

	if ((dummy = find_shitmask(currentbot->lists->shitlist, user, 0))==NULL)
	{
		send_to_user(from, "%s is not shitted", user);
		return;
	}

	free(dummy->whyshitted);
	dummy->whyshitted = strdup(rest);

	send_to_user(from,"Shit reason for %s changed to %s",user,rest);

	if(!write_shitlist(currentbot->lists->shitlist,currentbot->lists->shitfile))
		send_to_user(from, "Shitlist could not be written to file %s",currentbot->lists->shitfile);
	else
		send_to_user(from, "Shitlist written to file %s",currentbot->lists->shitfile);

	globallog(2, DONELOG, "%s changed the shit reason for %s to %s",from,user,rest);
	add_list_event("%s changed the shit reason for %s to %s", from, user, rest);
}

void do_chshitmask(char *from, char *to, char *rest)
{
	shitlist_t		*dummy;
	char				*user;
	char				*newuser;

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

	if (find_shitmask(currentbot->lists->shitlist, newuser, 0)!=NULL)
	{
		send_to_user(from, "A shitlist entry matching %s already exists", newuser);
		return;
	}

	if ((dummy = find_shitmask(currentbot->lists->shitlist, user, 0))==NULL)
	{
		send_to_user(from, "%s is not shitted", user);
		return;
	}

	free(dummy->mask);
	dummy->mask = strdup(newuser);

	send_to_user(from, "Changed shitmask for %s to %s", user, newuser);
	globallog(2, DONELOG, "%s changed the shitmask for %s to %s", from, user, newuser);
	add_list_event("%s changed the shitmask for %s to %s", from, user, newuser);

	if(!write_shitlist(currentbot->lists->shitlist, currentbot->lists->shitfile))
	{
		send_to_user(from, "Shitlist could not be written to file %s", currentbot->lists->shitfile);
	}
	else
		send_to_user(from, "Shitlist written to file %s", currentbot->lists->shitfile);
}

void do_chwhoshitted(char *from, char *to, char *rest)
{
	shitlist_t	*dummy;
	char			*user;
	char			*whoshitted;

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

	if ((dummy = find_shitmask(currentbot->lists->shitlist, user, 0))==NULL)
	{
		send_to_user(from, "%s is not shitted", user);
		return;
	}

	free(dummy->whoshitted);
	dummy->whoshitted = strdup(whoshitted);
	send_to_user(from, "Changed the whoshitted info for %s to %s", user, whoshitted);
	add_list_event("%s changed the whoshitted info for %s to %s", from, user, whoshitted);
	if(!write_shitlist(currentbot->lists->shitlist, currentbot->lists->shitfile))
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

void	do_expireadd	(char *from, char *to, char *rest)
{
	shitlist_t	*dummy;
	char			*newuser;
	char			*expirestr;
	long  		expiredate;
	int			rawtime = 0;

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

	if((dummy = find_shitmask(currentbot->lists->shitlist, newuser, 0))==NULL)
	{
		send_to_user(from, "%s is not shitted!", newuser);
		return;
	}

	dummy->expire = expiredate;

	send_to_user(from, "Shitlist mask %s set to expire on %s",newuser, time2str(expiredate));
	globallog(2, DONELOG, "Shitlist mask %s set to expire on %s by %s", newuser, time2str(expiredate), from);
	notify_expireadd(newuser, expiredate, from);
}

void	do_expiredel(char *from, char *to, char *rest)
{
	shitlist_t	*dummy;
	char			*deluser;

	if (userlevel(from)==OTHER_BOT_LEVEL)
		return;

	if((deluser = get_token(&rest, " ")) == NULL)
	{
		send_to_user( from, "Who do you want me to un-expire?" );
		return;
	}

	if((dummy = find_shitmask(currentbot->lists->shitlist, deluser, 0))==NULL)
	{
		send_to_user(from, "%s is not shitted!", deluser);
		return;
	}

	if (!dummy->expire)
	{
		send_to_user(from, "%s has no expiration date!", deluser);
		return;
	}

	dummy->expire = 0;
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
	show_expirelist_pattern("",currentbot->lists->shitlist, from, rest?rest:"");
	send_to_user(from, "End of Expire List");

}

int	show_expirelist_pattern (char *pretxt, shitlist_t **list, char *from, char *userhost)
{
	int				match=0;
	shitlist_t		*dummy;

	for( dummy = *list; dummy; dummy = dummy->next )
	{
		if(dummy->expire && (!mymatch(userhost, dummy->mask, FNM_CASEFOLD) || !*userhost))
		{
			if(!*userhost)
				send_to_user( from, "%s %25s | %s", pretxt, time2str(dummy->expire), dummy->mask);
			else
				send_to_user( from, "%s %s %s", pretxt, dummy->mask, time2str(dummy->expire));
			match++;
		}
	}

	return match;
}

int check_expirations(shitlist_t **list)
{
	shitlist_t	*dummy;
	char			*temp;

	for(dummy = *list; dummy; dummy = dummy->next)
		if(dummy->expire && dummy->expire <= time(NULL))
		{
			temp = strdup(dummy->mask);
			remove_from_shitlist(currentbot->lists->shitlist, dummy->mask);
			globallog(2, DONELOG, "Shitmask %s was removed because it expired.", temp);
			add_list_event("Shitmask %s was removed because it expired.", temp);
			write_shitlist(currentbot->lists->shitlist,currentbot->lists->shitfile);
			free(temp);
			return 1;
		}
	return 0;
}

void show_shitlist2(shitlist_t **list, char *from, char *to, char *rest)
{
	shitlist_t	*dummy=NULL;
	char			*arg1, *arg2;
	int 			temp = 0, level;

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
			break;
		case 2:
			send_to_user(from, " %-45s | Shit Level", "nick!user@host");
			break;
		case 3:
			send_to_user(from, " %-45s | Who Shitted", "nick!user@host");
			break;
		case 4:
			send_to_user(from, " %-45s | Shit Date", "nick!user@host");
			break;
		case 5:
			send_to_user(from, " %-45s", "nick!user@host");
			break;
	}
	send_to_user(from, "----------------------------------------------------------------------");

	for(dummy = *list; dummy; dummy = dummy->next )
	{
		switch(temp)
		{
			case 1:
				if(!mymatch(arg2, dummy->whyshitted, FNM_CASEFOLD))
					send_to_user(from, " %-45s | %s", dummy->mask, dummy->whyshitted);
				break;
			case 2:
				if(dummy->level == level)
					send_to_user(from, " %-45s | %d", dummy->mask, dummy->level);
				break;
			case 3:
				if(!mymatch(arg2, dummy->whoshitted, FNM_CASEFOLD))
					send_to_user(from, " %-45s | %s", dummy->mask, dummy->whoshitted);
				break;
			case 4:
				if(!mymatch(arg2, dummy->whenshitted, FNM_CASEFOLD))
					send_to_user(from, " %-45s | %s", dummy->mask, dummy->whenshitted);
				break;
			case 5:
				if(!mymatch(arg2, dummy->mask, FNM_CASEFOLD))
					send_to_user(from, " %-45s", dummy->mask);
				break;
		}
	}
	send_to_user(from, "End of Shit List");
}

void	show_shitlist(char *from, char *to, char *rest)
{
	if(!rest)
	{
		send_to_user(from, "%s Shit List", currentbot->nick);
		send_to_user(from, "-------+--------------------------------------------------------------");
		send_to_user(from, " Level | %s", "nick!user@host");
		send_to_user(from, "-------+--------------------------------------------------------------");
		show_shit_list("   ", currentbot->lists->shitlist, from, rest?rest:"",2, 0);
		send_to_user(from, "End of Shit List");
	}
	else
		show_shitlist2(currentbot->lists->shitlist, from, to, rest);
}

void	show_shit_list(char *pretxt, shitlist_t **list, char *from, char *mask, int listtype, int authed)
/*
	List types:
		2 - !shitlist
*/
{
	shitlist_t	*dummy;

	for(dummy = *list; dummy; dummy = dummy->next )
		if(!mymatch(dummy->mask, mask, FNM_CASEFOLD) || !*mask)
		{
			send_to_user(from, "%s%3d | %s", pretxt, dummy->level, dummy->mask);
		}
}

void	do_nshitadd(char *from, char *to, char *rest)
{
	char	*newuser;
	char	*newlevel;
	char	*nuh;		/* nick!@user@host */
	char	userstr[MAXLEN];
	char  m[300],m2[100];
	char	*expirestr;
	char	temp[2];
	int	level;
	int	doexpire = 0;
	long	expiretime = 0;

	if (userlevel(from)==OTHER_BOT_LEVEL) return;

	if((newuser = get_token(&rest, " ")) == NULL)
	{
		sendreply("Who do you want me to add?");
		return;
	}

	if(STRCASEEQUAL(newuser, "-expire"))
	{
		doexpire = 1;

		if((expirestr = get_token(&rest, " ")) == NULL)
		{
			sendreply("How long should they be shitted?");
			return;
		}

		if(!isnumeric(expirestr))
		{
			send_to_user(from, "%s is an invalid expire time", expirestr);
			return;
		}

		sscanf(expirestr, "%ld", &expiretime);

		if(!expiretime)
		{
			sendreply("Expire time should be > 0" );
			return;
		}

		expiretime *= 86400;
		expiretime += time(NULL);

		if((newuser = get_token(&rest, " ")) == NULL)
		{
			sendreply("Who do you want me to add?");
			return;
		}
	}

	if((nuh=username(newuser))==NULL)
	{
		sendreply("%s is not on this channel!", newuser);
		return;
	}

	if((newlevel = get_token(&rest, " ")) == NULL )
	{
		sendreply("What level should %s have?", newuser);
		return;
	}

	if(!isnumeric(newlevel))
	{
		send_to_user(from, "%s is an invalid shitlevel", newlevel);
		return;
	}

	sscanf(newlevel, "%d", &level);

	if(level < 0)
	{
		sendreply("level should be >= 0!");
		return;
	}

	if(userlevel(from) < userlevel(nuh) && userlevel(from) < 150)
	{
		/* This way, someone with level 100 can't shit someone with level 150 */
		sendreply("Sorry, %s has a higher level", newuser);
		return;
	}


	strcpy(userstr,get_add_mask(nuh,0));

	if (strlen(rest)==0)
		strcpy(m2,"Blow Us");
	else
	{
		strncpy(m2,rest,99);
		m2[99] = '\0';
	}

	temp[0] = 3;
	temp[1] = 0;
	if(strstr(m2, temp))
	{
		send_ctcp(userchannel(getnick(from)),"ACTION shoves some mIRC colors up %s\'s ass",getnick(from));
		return;
	}

	sprintf(m,"Shitted: %s",m2);

	if (!add_to_shitlist(currentbot->lists->shitlist, userstr, level, from, get_gmtime(), m, expiretime))
	{
		sendreply("There is already a record that matches %s",userstr);
		return;
	}

	sendreply("User %s shitted with access %d as %s", newuser, level, userstr);

	notify_shitadd(newuser,userstr,newlevel,from,m2);

	sprintf(m,"%s shitted as %s with level %d by %s for %s",newuser,
		userstr, level, from,m2);

	if (currentbot->talk>0)
		do_bwallop(currentbot->nick,to,m);

	globallog(2, DONELOG, "%s shitted as %s with level %d by %s",
		newuser, userstr, level, from);

#ifdef BackedUp
	if (Abackedup)
		send_to_user(currentbot->backupnick,"!comm %s shitadd %s %d %s",
			currentbot->backupbotpasswd,userstr, level, m);
#endif
}

void	do_shitadd(char *from, char *to, char *rest)
{
	char	m[300], m2[100];
	char	*newuser;
	char	*newlevel;
	char	*expirestr;
	char	temp[2];
	int	level;
	int	doexpire = 0;
	long	expiretime = 0;

	if (userlevel(from)==OTHER_BOT_LEVEL)
		return;

	if((newuser = get_token(&rest, " ")) == NULL)
	{
		send_to_user(from, "Who do you want me to add?");
		return;
	}

	if(STRCASEEQUAL(newuser, "-expire"))
	{
		doexpire = 1;

		if((expirestr = get_token(&rest, " ")) == NULL)
		{
			sendreply("How long should they be shitted?");
			return;
		}

		if(!isnumeric(expirestr))
		{
			send_to_user(from, "%s is an invalid expire time", expirestr);
			return;
		}

		sscanf(expirestr, "%ld", &expiretime);

		if(!expiretime)
		{
			sendreply("Expire time should be > 0" );
			return;
		}

		expiretime *= 86400;
		expiretime += time(NULL);

		if((newuser = get_token(&rest, " ")) == NULL)
		{
			sendreply("Who do you want me to add?");
			return;
		}
	}

	if(mymatch("*?!*?@*?",newuser, FNM_CASEFOLD))
	{
		send_to_user(from, "%s is an invalid mask.", newuser);
		return;
	}

	if((newlevel = get_token(&rest, " ")) == NULL )
	{
		send_to_user(from, "What level should %s have?", newuser);
		return;
	}

	if(!isnumeric(newlevel))
	{
		send_to_user(from, "%s is an invalid shitlevel", newlevel);
		return;
	}

	sscanf(newlevel, "%d", &level);

	if(level < 0)
	{
		send_to_user(from, "level should be >= 0!");
		return;
	}

	if(userlevel(from) < userlevel(newuser) && userlevel(from) < 150)
	{
		/* This way, someone with level 100 can't shit someone with level 150 */
		sendreply("Sorry, %s has a higher level", newuser);
		return;
	}

	temp[0] = 3;
	temp[1] = 0;
	if(strstr(rest, temp))
	{
		send_ctcp(userchannel(getnick(from)),"ACTION shoves some mIRC colors up %s\'s ass",getnick(from));
		return;
	}

	if (strlen(rest) == 0)
		strcpy(m2, "Blow Us");
	else
	{
		strncpy(m2, rest, 99);
		m2[99] = '\0';
	}

	sprintf(m,"Shitted: %s", m2);

	if (!add_to_shitlist(currentbot->lists->shitlist, newuser, level, from, get_gmtime(), m, expiretime))
	{
		sendreply("There is already a record that matches %s",newuser);
		return;
	}

	send_to_user(from, "User %s shitted with access %d", newuser, level);

	notify_shitadd(newuser,newuser,newlevel,from,m2);
	sprintf(m,"%s shitted with level %d by %s for %s", newuser, level, from,m2);

	if (currentbot->talk>0)
		do_bwallop(currentbot->nick,to,m);

	globallog(2, DONELOG, "%s shitted with level %d by %s", newuser, level, from);
#ifdef BackedUp
	if (Abackedup)
		send_to_user(currentbot->backupnick,"!comm %s shitadd %s %d %s", currentbot->backupbotpasswd,newuser, level, m);
#endif
}

void	do_shitwrite(char *from, char *to, char *rest)
{
	if(!write_shitlist(currentbot->lists->shitlist, currentbot->lists->shitfile))
		send_to_user(from, "Shitlist could not be written to file %s", currentbot->lists->shitfile);
	else
	{
		send_to_user(from, "Shitlist written to file %s", currentbot->lists->shitfile);
#ifdef BackedUp
				if (Abackedup)
				send_to_user(currentbot->backupnick,"!comm %s shitwrite", currentbot->backupbotpasswd);
#endif
	}
}

void	do_shitdel(char *from, char *to, char *rest)
{
	int	exlist_matches;

	if(!rest)
	{
		send_to_user(from, "Who do you want me to delete?");
		return;
	}

	if(!remove_from_shitlist(currentbot->lists->shitlist, rest))
	{
		send_to_user(from, "%s is not shitted!", rest);
		return;
	}

	send_to_user(from, "User %s has been deleted", rest);
	if((exlist_matches=num_excludelist_matches(currentbot->lists->excludelist, rest)) > 0)
		send_to_user(from, "There %d exlist entries that match %s", exlist_matches, rest);

	globallog(2, DONELOG, "%s unshitted by %s", rest, from);
	notify_shitdel(rest,from);

#ifdef BackedUp
	if (Abackedup)
		send_to_user(currentbot->backupnick,"!comm %s shitdel %s",
		currentbot->backupbotpasswd,rest);
#endif
}

void	do_nshitdel(char *from, char *to, char *rest)
{
	char	*who;
	char	*mask;

	if (userlevel(from)==OTHER_BOT_LEVEL) return;

	if(!rest)
	{
		send_to_user(from, "Who do you want me to delete?");
		return;
	}

	if((who=username(rest))==NULL)
	{
		sendreply("%s is not on this channel!", rest);
		return;
	}

	if((mask = shitmask(who)) == NULL)
	{
		send_to_user(from, "%s is not shitted!", rest);
		return;
	}

	if(excludelevel(who))
	{
		send_to_user(from, "%s is already excluded!", rest);
		return;
	}

	if(!remove_from_shitlist(currentbot->lists->shitlist, mask))
	{
		send_to_user(from, "Could not remove %s from shitlist!", mask);
		return;
	}

	send_to_user(from, "User %s has been unshitted", rest);

	globallog(2, DONELOG, "%s unshitted by %s", rest, from);
	notify_shitdel(rest,from);

#ifdef BackedUp
	if (Abackedup)
		send_to_user(currentbot->backupnick,"!comm %s shitdel %s",
		currentbot->backupbotpasswd,rest);
#endif
}

int remove_from_shitlist(shitlist_t **list, char *mask)
{
	shitlist_t	*old = NULL, *p = *list;
	shitlist_t	*dummy;

	if((dummy = find_shitmask(list, mask, 0)) == NULL)
		return(FALSE);

	for(p = *list; p && p!=dummy; p = p->next)
		old=p;

	if (p==NULL)
		return(FALSE);

	if (old==NULL)
		*list = (*list)->next;
	else
		old->next = p->next;

	free(p->mask);
	free(p->whyshitted);
	free(p->whoshitted);
	free(p->whenshitted);
	free(p);
	return(TRUE);
}

int shitlevel(char *from)
{
	shitlist_t	*dummy;

	if(from)
	{
		if((dummy = find_shitmask(currentbot->lists->shitlist, from, 1))==NULL)
			return 0;
		else
			return(dummy->level);
	}
	else
		return 0;
}
char *shitmask(char *from)
{
	shitlist_t	*dummy;

	if(from)
	{
		if((dummy = find_shitmask(currentbot->lists->shitlist, from, 1))==NULL)
			return NULL;
		else
			return(dummy->mask);
	}
	else
		return NULL;
}

int	read_from_shitlistfile(FILE *stream, char **mask, int *level, char **whoshitted, char **whenshitted, char **whyshitted, long *expire)
{
	char	*p;
	char	*lvl;
	char	*exp;
	int	i;

	do
	{
		p = fgets(tempwork, WAYTOBIG, stream);
	}while((p != NULL) && (*tempwork == '#'));

	if(!p)
		return(FALSE);

/*	mask level expire whoshitted whenshitted whyshitted */

	*mask = get_token(&p, " ");
	lvl = get_token(&p, " ");
	exp = get_token(&p, " ");
	*whoshitted = get_token(&p, " ");
	*whenshitted = get_token(&p, " ");

	if(!p)
		*whyshitted = "Shitted: Blow us";
	else
	{
		*whyshitted = p;

		for(i = 0; p[i]; i++)
		{
			if(p[i] == 10 || p[i] == 13)
				p[i] = '\0';
		}
	}

	*level = atoi(lvl);
	*expire = atol(exp);

	return(TRUE);
}

int 	readshitlistdatabase(char *fname, shitlist_t **shitlist)
{
	FILE 	*fp;
	char 	*mask = NULL;
	char	*whoshitted = NULL;
	char	*whenshitted = NULL;
	char	*whyshitted = NULL;
	int  	level;
	long	expire;

	if((fp = fopen( fname, "r")) == NULL)
	{
		printf("File \"%s\" not found, aborting\n", fname);
		exit(0);
	}

	while(read_from_shitlistfile(fp, &mask, &level, &whoshitted, &whenshitted, &whyshitted, &expire))
		add_to_shitlist(shitlist, mask, level, whoshitted, whenshitted, whyshitted, expire);

	fclose(fp);
	return(TRUE);
}

int	num_shitlist_matches(shitlist_t **list, char *mask, int searchtype)
/*
      searchtype:
                   0 - Whenshitted
                   1 - Whoshitted
                   2 - Userhost
*/
{
	int			match=0;
	shitlist_t	*dummy;

	for( dummy = *list; dummy; dummy = dummy->next )
	{
		switch(searchtype)
		{
			case 0:
				if(!mymatch(mask, dummy->whenshitted, FNM_CASEFOLD))
					match++;
				break;
			case 1:
				if(!mymatch(mask, dummy->whoshitted, FNM_CASEFOLD))
					match++;
				break;
			case 2:
				if(!mymatch(mask, dummy->mask, FNM_CASEFOLD))
					match++;
				break;
		}
	}

	return match;
}

int	show_shitlist_pattern(char *pretxt, shitlist_t **list, char *from, char *mask, int listtype)
{
	int				match=0;
	shitlist_t		*dummy;

	for(dummy = *list; dummy; dummy = dummy->next)
	{
		if(!mymatch(mask, dummy->mask, FNM_CASEFOLD) || !*mask)
		{
			if (listtype==2)
				send_to_user(from, "%s%3d | %s", pretxt, dummy->level, dummy->mask);
			match++;
		}
	}

	return match;
}

int write_shitlist(shitlist_t **list, char *filename)
{
	shitlist_t	*dummy;
	FILE			*f;
	int			blah = 0;

	if((f = fopen("temp.list", "w")) == NULL)
		return 0;

	blah += safe_fprintf(f,"#############################################\n");
	blah += safe_fprintf(f,"## IncuBot %s\n",currentbot->lists->shitfile);
	blah += safe_fprintf(f,"## Created: %s\n",get_ctime(0));
	blah += safe_fprintf(f,"## Format: mask level expire whoshitted whenshitted whyshitted\n");
	blah += safe_fprintf(f,"#############################################\n");

	for(dummy = *list; dummy; dummy = dummy->next)
		blah += safe_fprintf(f, "%s %d %lu %s %s %s\n", dummy->mask, dummy->level, dummy->expire, dummy->whoshitted, dummy->whenshitted, dummy->whyshitted);

	fclose(f);

	if(blah)
		return 0;

	unlink(filename);
	if(rename("temp.list", filename))
		return 0;

	chmod(filename, S_IRUSR | S_IWUSR);
	return 1;
}

