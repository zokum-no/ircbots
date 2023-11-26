/*
 * commands.c - all the commands...
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "global.h"
#include "config.h"
#include "defines.h"
#include "structs.h"
#include "h.h"
#include "commands.h"

void do_stats(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nuh;
	int authd;
	aUser *User, *Shit, *ProtNick, *Greet;

	chan = get_channel(to, &rest);
	if ((nuh = get_token(&rest, " ")))
		nuh = nick2uh(from, nuh, 0);
	else
		nuh = from;
	if (!nuh)
		return;
	if (password_needed(nuh))
	{
		if (verified(nuh))
			authd = 1;
		else
			authd = 2;
	}
	else
		authd = 0;

	User = find_user(&(current->Userlist), nuh, chan); 
	Shit = find_shit(&(current->Userlist), nuh, chan);
	ProtNick = find_protnick(&(current->Userlist), nuh);
	Greet = find_greet(&(current->Userlist), nuh);
	if (User)
	{
	send_to_user(from, "Immortality stats for %s on %s", nuh, chan);
	send_to_user(from, "---------------------------------------------------------");
	send_to_user(from, "      User level: %i", User->access);
	send_to_user(from, "  Recognized u@h: %s", User->userhost);
	send_to_user(from, "        Added by: %s", User->whoadd);
	send_to_user(from, "        Added on: %s", time2str(User->time));
	send_to_user(from, "        Verified: %s",
		(!authd) ? "No password needed" : (authd==1) ? "Yes" : "No");
	send_to_user(from, " Protected level: %i%s",
		User ? User->prot : 0,
		get_int_var(chan, TOGPROT_VAR) ? "" :
		"   \002(Protection toggled off)\002");
	send_to_user(from,"      Auto Opped: %s%s",
		(User && User->aop) ? "Yes" : "No",
		get_int_var(chan, TOGAOP_VAR) ? "" :
		"   \002(Auto-op is toggled off)\002");
	}
	if (Shit)
	{
                send_to_user(from, "   ---------");
		send_to_user(from, "   Shitlisted as: %s", Shit->userhost);
		send_to_user(from, "   Shitlisted by: %s",
			Shit->whoadd ? Shit->whoadd : "<Anonymous>");
		send_to_user(from, "      Shitted on: %s", time2str(Shit->time));
		send_to_user(from, "Shitlisted level: %i",
		Shit ? Shit->access : 0);
		send_to_user(from, "Shitlist expires: %s",
			time2str(Shit->expire));
		send_to_user(from, " Shitlist reason: %s",
			Shit->reason ? Shit->reason : "<No reason>");	
 	} 
        if (ProtNick)
        {
                send_to_user(from, "   ---------");
                send_to_user(from, "  Protected Nick: %s", ProtNick->userhost);
                send_to_user(from, "    Level needed: %i", ProtNick->access);
        }
        if (Greet)
        {
                send_to_user(from, "   ---------");
                send_to_user(from, "    Greeting u@h: %s", Greet->userhost);
                send_to_user(from, "      Greeting: %s", Greet->reason);
        }
}

void do_access(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nuh;
	int level;

	chan = get_channel(to, &rest);
	if ((nuh = get_token(&rest, " ")) != NULL)
        {
		if (*nuh == current->cmdchar)
                {
			nuh++; 
			level = level_needed(nuh);
			if (level < 0)

			/* keepin em lamers in the dark :)  */
			/*
				send_to_user(from, "\002No such command\002");
			*/
				return;
			else
				send_to_user(from, "\002The immortality level needed for that command is: %i\002", level);
                	return;
		}
		nuh = nick2uh(from, nuh, 0);
	}
	else
		nuh = from;
	if (!nuh)
	{
		return;
	}
	else
	{
	send_to_user(from, "\002Immortality Level for %s\002", nuh);
	send_to_user(from, "\002Channel: %s  Access: %i\002", chan, get_userlevel(nuh, chan));

		if (get_userlevel(nuh, chan) == 100)
		{
			send_to_user(from, "\002%s is a God\002", nuh);
		}
	}
	if (current->restrict)
		send_to_user(from,
			"\002Currently being restricted to %i\002",
			 current->restrict);
}

void do_auth(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *pass;
	int pwn, v;  

	pass = get_token(&rest, " ");
	pwn = password_needed(from);
	v = verified(from);

	if (pwn && v)
	{
		send_to_user(from, "\002You have already been authorized\002");
		return;
        }
	if (!pwn)
	{
		send_to_user(from, "\002You don't need a password to do commands\002");
		return;
	}
	if (!pass || !*pass)
	{
                if (pwn)
			send_to_user(from, "\002No password given, not verified\002");
        	return;
	}
  	if (correct_password(from, pass))
	{
		aTime *tmp;
		send_to_user(from, "\002You are now officially immortal\002");
		if ((tmp = make_time(&Auths, from)) != NULL)
			tmp->time = getthetime(); 
#ifdef USE_CCC
		send_to_ccc("1 AUTH %s %li", from, getthetime());
#endif
	   	op_all_chan(currentnick);
	}
  	else
	 	send_to_user(from, "\002Incorrect password, not authorized\002");
}

void do_version(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	send_to_user(from, "\002%s Class EnergyMech\002", CLASS);
	send_to_user(from, "\002Compiled on %s %s\002", __DATE__, __TIME__);
	send_to_user(from, "\002Created on %s\002", BOTDATE);
	send_to_user(from, "\002Version %s %s\002", VERSION, REVISION);
	send_to_user(from, "\002Current Version by, Polygon\002");
	send_to_user(from, "\002Distribution: %s\002", POLYGON);
}

void do_time(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	if (ischannel(to)) {
		sendprivmsg(to, "\002Current time in %s is: %s\002", PHYSICAL_LOCATION, time2away(getthetime()));
	}
	else {
	send_to_user(from, "\002Current time in %s is: %s\002",
		PHYSICAL_LOCATION, time2away(getthetime()));
	}
}

void do_info(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	do_version(from, to, rest, cmdlevel);
	send_to_user(from, "\002Started: %-20.20s\002", time2str(current->uptime));
	send_to_user(from, "\002Uptime : %s\002", idle2str(getthetime()-current->uptime));
	send_to_user(from, "\002Cmdchar: %c\002", current->cmdchar);
}

void do_uptime(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	send_to_user(from, "\002Started: %-20.20s\002", time2str(current->uptime));
	send_to_user(from, "\002Uptime : %s\002", idle2str(getthetime()-current->uptime));
}

void do_getch(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	send_to_user( from, "\002Current channel: %s\002", currentchannel());
}

void do_opme(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	

	chan = get_channel(to, &rest);
	
	if (get_userlevel(from, chan) < cmdlevel) {
		return;
	}
	
	if (usermode(chan, currentnick) & MODE_CHANOP) {
		send_to_user(from, "\002You're already channel operator on %s\002", chan);
	}
	else if (get_token(&rest, " ") != NULL) {
		return;
	}
	else {
		giveop(chan, currentnick);
	}
}

void do_deopme(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;

	chan = get_channel(to, &rest);
	
	if (get_userlevel(from, chan) < cmdlevel) {
		return;
	}
	
	if (get_token(&rest, " ") != NULL) {
		return;
	}
	else if (usermode(chan, currentnick) & MODE_CHANOP) {
		takeop(chan, currentnick);
	}
	else {
		send_to_user(from, "\002You're already not a channel operator on %s\002", chan);
	}
}

void do_op(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	

	chan = get_channel(to, &rest);
	
	if (get_userlevel(from, chan) < cmdlevel) {
		return;
	}
	
	if (rest) {
		mode3(chan, "+oooo", rest);
	}
	else {
		giveop(chan, currentnick);
	}
}

void do_deop(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	
	if (get_userlevel(from, chan) < cmdlevel) {
		return;
	}
	
	chan = get_channel(to, &rest);
	
	if (rest) {
		mode3(chan, "-oooo", rest);
	}
	else {
		send_to_user(from, "\002%s\002", "Who do you want me to deop, you schmuck?");
	}
}

void do_invite(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick;
	

	chan = get_channel(to, &rest);
	nick = currentnick;
	
	if (get_userlevel(from, chan) < cmdlevel) {
		return;
	}

	if (rest && *rest)
	  nick = rest;

	if (!invite_to_channel(nick, chan)) {
		send_to_user(from, "\002I'm not on channel %s, you dickhead\002", chan);
	}
	else {
		send_to_user(from, "\002User(s) invited to %s\002", chan);
	}
}
			
void do_chat(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	if (search_list("chat", from, DCC_CHAT))
	{
		send_to_user(from, "\002You are already DCC chatting me\002");
		return;
	}
	dcc_chat(from, rest);
}

void do_say(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	

	chan = get_channel(to, &rest);
	
	if (get_userlevel(from, chan) < cmdlevel) {
		return;
	}
	
	if (!rest)
	{
		send_to_user(from,
			"\002What am I suppposed to say, lamer?\002");
		return;
	}
	sendprivmsg(chan, "%s", rest);
}

void do_me(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	char text[HUGE];
	

	strcpy(text, "ACTION ");
	chan = get_channel(to, &rest);

	if (get_userlevel(from, chan) < cmdlevel) {
		return;
	}
	
	if (!rest)
	{
		send_to_user( from,
			"\002What am I suppposed to do, dorkus? :P\002");
		return;
	}
	strcat(text, rest);

	send_ctcp(chan, "%s", text);
}

void do_msg(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *nick;

	if (!rest)
	{
		send_to_user(from, "\002No nick given!@#\002");
		return;
	}
	nick = get_token(&rest, " ");

	if (!rest || !*rest)
	{
		 send_to_user( from, "\002What's the message?\002");
		 return;
	}

	if (ischannel(nick))
	{
		do_say(from, to, rest, cmdlevel);
		return;
	}

	if (!my_stricmp(nick,current->nick))
	{
		 send_to_user(from, "\002Nice try, but I don't think so\002");
		 return;
	}

	if (*nick == '=')
	{
		register aDCC *Client;
		int ok = 0;

		nick++;
		for (Client=current->Client_list;Client;Client=Client->next)
		{
                	if (((Client->flags&DCC_TYPES) == DCC_CHAT) &&
                         (!my_stricmp("chat", Client->description)) &&
                         (!my_stricmp(nick, getnick(Client->user))))
			{
				ok = 1;
				send_chat(Client->user, rest);
			}
		}
		if (!ok)
		{
			send_to_user(from, "\002I have no DCC connection to %s\002", nick);
			return;
		}
	}
	else	
		sendprivmsg(nick, "%s", rest);
	send_to_user(from, "\002Message sent to %s\002", nick);
}


void do_do(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  char *temp;

  if (rest)
  {
	 temp = strchr(rest, ':');
	 if (temp)
		*temp++ = '\0';
	 if (temp && *temp)
		send_to_server( "%s:%s", rest, temp);
	 else
		send_to_server( "%s", rest);
  }
  else
	 send_to_user(from, "\002%s\002", "What do you want me to do?");
}

void do_channels(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	show_channel_list(from);
}

void do_join(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;

	chan = get_token(&rest, " ");
	if (!chan)
	{
		send_to_user(from, "\002No channel name specified\002");
		return;
	}
	if (!ischannel(chan))
	{
		send_to_user(from, "\002Invalid channel name\002");
		return;
	}

	if (get_userlevel(from, chan) < cmdlevel)
	{
		return;
	}

	send_to_user(from, "\002Attempting the join of %s\002", chan);
	join_channel(chan, rest, TRUE);
}

void do_leave(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	

	chan = get_channel(to, &rest);
	
	if (get_userlevel(from, chan) < cmdlevel) {
		return;
	}
	
	if (find_list(&current->StealList, chan))
	{
	do_rsteal(from, to, rest, cmdlevel);
		if (find_list(&current->StealList, chan))
		{
		send_to_user(from, "\002Currently trying to steal %s\002", chan);
		return;
		}
	send_to_user(from, "\002Attempting to leave %s\002", chan);
	}
	else
	{
	send_to_user(from, "\002Attempting to leave %s\002", chan);
	leave_channel(chan);
	}
}

void do_nick(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *nick;

	nick = get_token(&rest, " ");

	if (nick && *nick)
	{
		if (!isnick(nick))
		{
			send_to_user(from, "\002Illegal nickname %s\002", nick);
			return;
		}
		nick[9] = '\0';
#ifdef USE_CCC
		send_to_ccc("1 NICK %s!%s@%s %s!%s@%s",
			current->nick, current->login, mylocalhostname,
			nick, current->login, mylocalhostname);
		send_to_ccc("1 NICK %s!%s@%s %s!%s@%s",
			current->nick, current->login, mylocalhost,
			nick, current->login, mylocalhost);
#endif
		strcpy(current->nick, nick);
		strcpy(current->realnick, nick);
		sendnick(current->nick);
	}
	else
		send_to_user(from, "\002%s\002", "No nickname specified");
	return;
}

void do_die(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	int templevel;
	
	templevel = get_userlevel(from, "#Onefuckedupchannel!#O!@I#");
	
	if (templevel < cmdlevel) {
		return;
	}
	
	if (rest) {
		signoff(from, rest);
	}
	else {
		signoff(from, randstring(SIGNOFFSFILE));
	}
	if (number_of_bots == 0) {
		MyExit(0);
	}
}

void do_add(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *level, *aopa, *prota, *uh, *pass;
	int access, aop, prot, already, templevel;
	aUser *tmp;  

	chan = get_token(&rest, " ");
  if (!chan || !*chan || (!ischannel(chan) && my_stricmp(chan, "*")))
	 {
		send_to_user(from, "\002%s\002",
			"Usage: ADD <channel or *> <nick or userhost> <level> [aop] [protlevel] [password]");
		return;
	 }
  if (*chan == '*')  /* Nice kludge on next line =) */
	templevel = get_userlevel(from, "#Onefuckedupchannel!#O!@I#");
  else
	templevel = get_userlevel(from, chan);
  if (templevel < cmdlevel)
	 {
		return;
	 }
  
  if (!(nick = get_token(&rest, " ")))
	 {
		send_to_user(from, "\002No nick or userhost specified\002");
		return;
	 }
  if (!(level = get_token(&rest, " ")))
	 {
		send_to_user(from, "\002%s\002", "No level specified");
		return;
	 }
  if (!(aopa = get_token(&rest, " ")))
	 aopa = "0";
  if (!(prota = get_token(&rest, " ")))
	 prota = "0";
  pass = get_token(&rest, " ");
  if (!(uh = nick2uh(from, nick, 1)))
      return;
      
/* SuperUser */

	if (!strcmp(level, "~1")) {
		access = SUPERUSER;
	}
	else {
  		access = atoi(level);
  		if ((access < 0) || (access >= OWNERLEVEL+1))
  		{
			send_to_user(from, "\002Valid levels are from 0 thru %i\002", OWNERLEVEL);
			return;
  		}
  		if (access > templevel)
  		{
			send_to_user(from, "\002Level must be <= to yours on %s\002", chan);
			return;
  		}
  	}
  	
/* End SuperUser */

  if (!my_stricmp("*!*@*", uh))
  {
	send_to_user(from, "\002Problem adding *!*@*, try again\002");
	return;
  }
  prot = atoi(prota);
  aop = atoi(aopa);
  if (check_for_number(from, prota) || check_for_number(from, aopa))
	return;
  prot = (prot < 0) ? 0 : (prot > MAXPROTLEVEL) ? MAXPROTLEVEL : prot;
  already = is_user(uh, chan);
	if (already)
	{
		send_to_user(from, "\002%s %s on %s is already a user\002", nick, uh, chan);
		return;
	}
  if ((tmp=add_to_userlist(&(current->Userlist), uh, access, aop ? 1 : 0, prot,
		chan, (pass && *pass) ? encode(pass) : NULL)) != NULL)
  {
	if (tmp->whoadd)
		MyFree(&tmp->whoadd);
	mstrcpy(&tmp->whoadd, from);
#ifdef USE_CCC
	send_to_ccc("1 ADD %s %s %s %i %i %i %s",
		from, chan, uh, access, aop ? 1 : 0, prot,
		(pass && *pass) ? encode(pass) : "");
#endif
	send_to_user(from, "\002%s has been added as %s on %s\002", nick, uh, chan);
	send_to_user(from,
				"\002Access level: %i  Auto-op: %s  Protect level: %i  %s %s\002",
			  access, aop ? "Yes" : "No", prot,
			  (pass && *pass) ? "Password:" : "",
			  (pass && *pass) ? pass : "");
	sendnotice(nick, "\002%s has blessed you with %i levels of immortality\002",
				currentnick, access);
	sendnotice(nick, "\002You are %sbeing auto-opped *smo0tch*\002",
		aop ? "" : "not ");
	if (prot)
	  sendnotice(nick, "\002You have been granted %i terajoules of EnergyMech Ego Shields\002",
			  prot);
	else
	  sendnotice(nick, "\002%s\002", "EnergyMech force does not protect you");
	sendnotice(nick, "\002My command character is %c\002", current->cmdchar);
	sendnotice(nick, "\002Use \002%chelp topics\002 and \002%chelpfull\002 commands for command help\002", current->cmdchar, current->cmdchar);
	if (pass && *pass)
	{
	  sendnotice(nick, "\002Password necessary for doing commands: %s\002",
			  pass);
	  sendnotice(nick, "\002If you do not like your password, use \"passwd\" to change it\002");
	}
		}
  else
	send_to_user( from, "\002Problem adding the user\002");
	return;
}                   

void do_del(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *uh;
 	aUser *tmp;
	int templevel;
	
	chan = get_channel(to, &rest);	
	
	if (*chan == '*') {
		templevel = get_userlevel(from, "#Onefuckedupchannel!#O!@I#");
	
		if (templevel < cmdlevel) {
			return;
		}
	}
	
	if (get_userlevel(from, chan) < cmdlevel) {
		return;
	}
  
	if (!chan || !*chan || (!ischannel(chan) && my_stricmp(chan, "*")))
	{
		send_to_user(from, "\002%s\002",
			"Usage: DEL <channel or *> <nick or userhost>");
		return;
	}
	nick = get_token(&rest, " ");
	if (!nick)
	{ 
  		send_to_user(from, "\002No nick or userhost specified\002");
		return;
	}
	if (!(uh = nick2uh(from, nick, 0)))
		return;
	if ((tmp=find_user(&(current->Userlist), uh, chan)) == NULL)
	{
		send_to_user(from,
			"\002%s is not in my users list for that channel\002",
			uh);
		return;
	}
	if (!matches(owneruserhost, uh) || !matches(uh, owneruserhost))
	{
		send_to_user(from,
			"\002Sorry, %s is god and can not be deleted\002", nick);
		return;
	}
	if (!matches(DEFAULT_OWNERUH, uh) || !matches(uh, DEFAULT_OWNERUH))
	{
		send_to_user(from,
			"\002Sorry, %s is god and can not be deleted\002", nick);
		return;
	}
	
/* SuperUser */

	if ((tmp->access == SUPERUSER) && (get_userlevel(from, chan) == OWNERLEVEL)) {
		send_to_user(from, "\002Deleting SuperUser %s\002", uh);
	}
	else {
		if (get_userlevel(from, chan) < tmp->access)
		{
			send_to_user(from, "\002%s has a higher immortality level than you on %s\002", uh, chan);
			return;
		}
	}
	
/* End SuperUser */

#ifdef USE_CCC
        send_to_ccc("1 DEL %s %s %s", from, tmp->channel, tmp->userhost);
#endif
	remove_user(&(current->Userlist), tmp);
	send_to_user(from, "\002User %s has been purged\002", uh);
}

void do_aop(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *nuh;
	aUser *tmp;
	

	chan = get_channel(to, &rest);
	nick = get_token(&rest, " ");

	if (!nick)
	{
		send_to_user(from, "\002%s\002", "Specify someone to auto-op");
		return;
	}
	if (!(nuh = nick2uh(from, nick, 0)))
		return;

	if ((tmp=find_user(&(current->Userlist), nuh, chan)) == NULL)
	{
		send_to_user( from, "\002%s is not in my users list on %s\002", nuh, chan);
		return;
	}
	if (tmp->aop)
	{
		send_to_user( from, "\002%s is already being auto-opped on %s\002", nuh, chan);
		return;
	}
	tmp->aop = 1;
	send_to_user(from,"\002%s is now being auto-opped on %s\002",nuh,chan);
#ifdef USE_CCC
	send_to_ccc("1 CUSER %s %s %s -1 1 -1", from, chan, nuh);
#endif
}

void do_raop(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *nuh;
	aUser *tmp;
	

	chan = get_channel(to, &rest);
	nick = get_token(&rest, " ");

	if (!nick)
	{
		send_to_user(from, "\002%s\002", "Specify someone to not auto-op");
		return;
	}
	if (!(nuh = nick2uh(from, nick, 0)))
        	return;
	if ((tmp=find_user(&(current->Userlist), nuh, chan)) == NULL)
	{
		send_to_user(from, "\002%s is not in my users list on %s\002",
			nuh, chan);
		return;
	}
	if (!tmp->aop)
	{
		send_to_user(from,
			"\002%s is already not being auto-opped on %s\002",
			nuh, chan);
		return;
	}
	tmp->aop = 0;
	send_to_user(from,
		"\002%s is no longer being auto-opped on %s\002", nuh, chan);
#ifdef USE_CCC
	send_to_ccc("1 CUSER %s %s %s -1 0 -1", from, chan, nuh);
#endif
}

void do_savelevels(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	if (!write_levelfile(LEVELFILE))
		send_to_user(from, "\002Levels could not be saved to %s\002",
			LEVELFILE);
	else
		send_to_user(from, "\002Levels were written to %s\002",
			LEVELFILE);
}

void do_loadlevels(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	if (!read_levelfile(LEVELFILE))
		send_to_user(from, "\002Levels could not be read from %s\002", LEVELFILE);
	else
		send_to_user(from, "\002Levels were read from %s\002", LEVELFILE);
}

void do_savelists(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
        if (!saveuserlist(&(current->Userlist), current->USERFILE))
        	send_to_user( from, "\002Lists could not be saved to file %s\002", current->USERFILE);
        else
            send_to_user( from, "\002Lists saved to file %s\002", current->USERFILE);
}

void do_savekicks(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
        if (!write_timelist(&current->KickList, current->kickfile))
                send_to_user( from, "\002Kicklist could not be written to file"
			" %s\002", current->kickfile);
        else
                send_to_user( from, "\002Kicklist written to file %s\002",
			 current->kickfile);
}

void do_loadkicks(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
        if (!read_timelist(&current->KickList, current->kickfile))
                send_to_user( from, "\002Kicklist could not be read from file"
                        " %s\002", current->kickfile);
        else
                send_to_user( from, "\002Kicklist read from file %s\002",
                         current->kickfile);
}

void do_loadlists(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	if (!(readuserlist(current->USERFILE)))
	send_to_user( from, "\002Lists could not be read from file %s\002", current->USERFILE);
	else
		send_to_user( from, "\002Lists read from file %s\002", current->USERFILE);
}

void do_ban(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *nuh;
	

	chan = get_channel(to, &rest);
		
	if (!(nick = get_token(&rest, " ")))
	{
		send_to_user(from, "\002No nick specified\002");
		return;
	}
	if (!(nuh = nick2uh(from, nick, 0)))
	  return;

	if (get_userlevel(nuh, chan) > get_userlevel(from, chan))
	{
	  sendkick(chan, currentnick, "\002Ban attempt of %s\002", nuh);
	  return;
	}
	deop_ban(chan, nick, nuh);
	send_to_user(from, "\002%s banned on %s\002", nick, chan);
}
		
void do_siteban(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *nuh;
	

	chan = get_channel(to, &rest);
	
	if (get_userlevel(from, chan) < cmdlevel) {
		return;
	}

	if (!(nick = get_token(&rest, " ")))
	{
		send_to_user(from, "\002No nick specified\002");
		return;
	}
	if (!(nuh = nick2uh(from, nick, 0)))
	  return;

	if (get_userlevel(nuh, chan) > get_userlevel(from, chan))
	{
	  sendkick(chan, currentnick, "\002Siteban attempt of %s\002", nuh);
	  return;
	}
	deop_siteban(chan, nick, nuh);
	send_to_user(from, "\002%s sitebanned on %s\002", nick, chan);
}
		
void do_kickban(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *nuh;
	

	chan = get_channel(to, &rest);
	
	if (get_userlevel(from, chan) < cmdlevel) {
		return;
	}
	
	if (!(nick = get_token(&rest, " ")))
	{
		send_to_user(from, "\002%s\002", "No nick specified");
		return;
	}
	if (!(nuh = nick2uh(from, nick, 0)))
	  return;

	if (get_userlevel(nuh, chan) > get_userlevel(from, chan))
	{
	  sendkick(chan, currentnick, "\002Kickban attempt of %s\002", nuh);
	  return;
	}

	deop_ban(chan, nick, nuh);
	sendkick(chan, nick, "%s", rest ? rest : "Requested Kick" );
	send_to_user(from, "\002%s kickbanned on %s\002", nick, chan);
}

void do_sitekickban(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *nuh;
	

	chan = get_channel(to, &rest);
	
	if (get_userlevel(from, chan) < cmdlevel) {
		return;
	}
	
	if (!(nick = get_token(&rest, " ")))
	{
		send_to_user(from, "\002%s\002", "No nick specified");
		return;
	}
	if (!(nuh = nick2uh(from, nick, 0)))
	  return;

	if (get_userlevel(nuh, chan) > get_userlevel(from, chan))
	{
	  sendkick(chan, currentnick, "\002Sitekickban attempt of %s\002", nuh);
	  return;
	}
	deop_siteban(chan, nick, nuh);
	sendkick(chan, nick, "%s", rest ? rest : "Requested Kick");
	send_to_user(from, "\002%s sitekickbanned on %s\002", nick, chan);
}

void do_kick(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *nuh;
	

	chan = get_channel(to, &rest);
	
	if (get_userlevel(from, chan) < cmdlevel) {
		return;
	}
	
	if (!(nick = get_token(&rest, " ")))
	{
		send_to_user(from, "\002%s\002", "No nick specified");
		return;
	}
	if (!(nuh = nick2uh(from, nick, 0)))
		return;
	if (get_userlevel(nuh, chan) > get_userlevel(from, chan))
	{
		sendkick(chan, currentnick, "\002%s is immortal\002", nick);
		return;
	}
	sendkick(chan, nick, "%s", rest ? rest : randstring(RANDKICKSFILE));
	send_to_user(from, "\002%s kicked from %s\002", nick, chan);
}

void do_showusers(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *pattern, *flag;
	int flags;

	flag = (char *) 0;

	chan = get_channel(to, &rest);
	pattern = get_token(&rest, " ");
	if (pattern && (*pattern == '-'))
	{
		flag = pattern;
		pattern = get_token(&rest, " ");
	}
	else if (rest && (*rest == '-'))
		flag = get_token(&rest, " ");
	flags = 0;
	if (flag && !my_stricmp(flag, "-ops"))
		flags = 1;
	if (flag && !my_stricmp(flag, "-nonops"))
		flags = 2;
	do_who(chan);
	show_users_on_channel(from, chan, pattern, flags);
}

void do_massop(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *pattern;
	

	chan = get_channel(to, &rest);
	
	if (get_userlevel(from, chan) < cmdlevel) {
		return;
	}
	
	pattern = get_token(&rest, " ");
	if (!pattern)
		pattern = "*!*@*";

	if (i_am_op(chan))
	{
		if (!channel_massop(chan, pattern))
			send_to_user(from, "\002No match for massop of %s on %s\002", pattern, chan);
	}
	else
		not_opped(from, chan);
}

void do_massdeop(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *pattern;

	chan = get_channel(to, &rest);
	
	if (get_userlevel(from, chan) < cmdlevel) {
		return;
	}
	
	pattern = get_token(&rest, " ");
	if (!pattern)
		pattern = "*!*@*";

	if (i_am_op(chan))
	{
		if (!channel_massdeop(chan, pattern))
			send_to_user(from, "\002No match for massdeop of %s on %s\002", pattern, chan);
	}
	else
		not_opped(from, chan);
}

void do_massunban(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *pattern, *temp;
	time_t minutes;
	

	minutes = 0;
	chan = get_channel(to, &rest);
	
	if (get_userlevel(from, chan) < cmdlevel) {
		return;
	}
	
	pattern = get_token(&rest, " ");
	temp = get_token(&rest, " ");
	if (!pattern)
 		pattern = "*!*@*";
	if (*pattern == '+')
        {
		 temp = pattern;
		 pattern = "*!*@*";
	}
	if (temp && *temp)
	{
		 if (*temp == '+')
			temp++;
		 minutes = atol(temp);
	}
	if (!i_am_op(chan))
        {
		not_opped(from, chan);
		return;
        }
	if (!channel_massunban(chan, pattern, minutes*60))
		send_to_user(from, "\002No match for massunban of %s on %s\002", pattern, chan);
}

void do_massunbanfrom(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *pattern, *temp;
	time_t minutes;
	

	minutes = 0;
	chan = get_channel(to, &rest);
	
	if (get_userlevel(from, chan) < cmdlevel) {
		return;
	}
	
	pattern = get_token(&rest, " ");
	temp = get_token(&rest, " ");
	if (!pattern)
		pattern = "*!*@*";
	if (*pattern == '+')
	{
		 temp = pattern;
		 pattern = "*!*@*";
	}
	if (temp && *temp)
	{
		 if (*temp == '+')
			temp++;
		 minutes = atol(temp);
	}
	if (!i_am_op(chan))
        {
		not_opped(from, chan);
		return;
        }
	if (!channel_massunbanfrom(chan, pattern, minutes*60))
		send_to_user(from, "\002No match for massunban of %s on %s\002", pattern, chan);
}

void do_masskick(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *pattern;
	

	chan = get_channel(to, &rest);
	
	if (get_userlevel(from, chan) < cmdlevel) {
		return;
	}
	
	pattern = get_token(&rest, " ");
	if (!pattern)
		pattern = "*!*@*";
	
	if (i_am_op(chan))
	{
		if (!channel_masskick(chan, pattern, rest))
			send_to_user(from, "\002No match for masskick of %s on %s\002", pattern, chan);
	}
	else
		not_opped(from, chan);
}

void do_masskickban(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *pattern;
	

	chan = get_channel(to, &rest);

	if (get_userlevel(from, chan) < cmdlevel) {
		return;
	}
	
	pattern = get_token(&rest, " ");
	if (!pattern)
		pattern = "*!*@*";

	if (i_am_op(chan))
	{
		if (!channel_masskickban(chan, pattern, rest))
			send_to_user(from, "\002No match for masskickban of %s on %s\002", pattern, chan);
	}
	else
		not_opped(from, chan);
}

void do_masskickbanold(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *pattern;
	

	chan = get_channel(to, &rest);
	
	if (get_userlevel(from, chan) < cmdlevel) {
		return;
	}
	
	pattern = get_token(&rest, " ");
	if (!pattern)
		pattern = "*!*@*";

	if (i_am_op(chan))
	{
		if (!channel_masskickbanold(chan, pattern, rest))
			send_to_user(from, "\002No match for masskickban of %s on %s\002", pattern, chan);
	}
	else
		not_opped(from, chan);
}

void do_massopu(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	
	chan = get_channel(to, &rest);
	
	if (get_userlevel(from, chan) < cmdlevel) {
		return;
	}

	if (i_am_op(chan))
	{
		if (!channel_massop(chan, ""))
			send_to_user(from, "\002No match for massop of users on %s\002", chan);
	}
	else
		not_opped(from, chan);
}

void do_massdeopnu(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	

	chan = get_channel(to, &rest);
	
	if (get_userlevel(from, chan) < cmdlevel) {
		return;
	}

	if (i_am_op(chan))
	{
		if (!channel_massdeop(chan, ""))
			send_to_user(from, "\002No match for massdeop of non-users on %s\002", chan);
	}
	else
		not_opped(from, chan);
}

void do_masskicknu(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	

	chan = get_channel(to, &rest);
	
	if (get_userlevel(from, chan) < cmdlevel) {
		return;
	}
	
	if (i_am_op(chan))
	{
		if (!channel_masskick(chan, "", rest))
			send_to_user(from, "\002No match for masskick of non-users on %s\002", chan);
	}
	else
		not_opped(from, chan);
}

void do_shit(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *level, *uh, *aday, *uh2;
	int access;
        long int days;
	char whoshit[255];

	aday = (char *) 0;
	chan = get_token(&rest, " ");
	if (!chan || !*chan || (!ischannel(chan) && my_stricmp(chan, "*")))
	{
		send_to_user(from, "\002%s\002",
			"Usage: SHIT <channel or *> <nick or userhost> <level>");
		return;
	}

	if (get_userlevel(from, chan) < cmdlevel)
	{
		return;
	}

	if (!(nick = get_token(&rest, " ")))
	{
		send_to_user(from, "\002%s\002", "No nick or userhost specified");
		return;
	}
	if (!(level = get_token(&rest, " ")))
	{
		send_to_user(from, "\002%s\002", "No level specified");
		return;
	}
	if (rest && isdigit(*rest))
		aday = get_token(&rest, " ");
	if (!rest || !*rest)
	{
		 send_to_user(from, "\002%s\002", "No reason specified");
		 return;
	}
	if (!(uh2 = nick2uh(from, nick, 0)))
	    return;
	if (is_shitted(uh2, chan))
	{
		send_to_user(from, "\002%s is in my shitlist already for this channel\002", uh2);
		return;
	}
	if (get_userlevel(uh2, chan) > get_userlevel(from, chan))
	{
		send_to_user(from, "\002%s is a God compared to you on %s\002", uh2, chan);
		return;
	}
	if (!matches(owneruserhost, uh2) || !matches(uh2, owneruserhost))
	{
		send_to_user(from, "\002%s is God and can not be shitted\002", uh2);
		return;
	}
        uh = format_uh(uh2, 1);
	if (!uh)
	{
		send_to_user(from, "\002A problem has occured, give comstud the following few lines:\002");
		send_to_user(from, "\002From = %s\002", from);
		send_to_user(from, "\002To   = %s\002", to);
		send_to_user(from, "\002Nick = %s\002", nick);
		send_to_user(from, "\002UH2  = %s\002", uh2);
		return;
	}
	if (!my_stricmp("*!*@*", uh))
	{
		 send_to_user(from, "\002Problem shitting *!*@*, try again\002");
		 return;
	}
	if (check_for_number(from, level))
		return;
	access = atoi(level);
	if ((access <= 0) || (access > MAXSHITLEVEL))
	{
		send_to_user(from, "\002Valid levels are from 1 thru %i\002", MAXSHITLEVEL);
		return;
	}
	days = aday ? atol(aday) : 30;
	strcpy(whoshit, from);
	if (rest && (*rest == '?'))
	{
		char *ptr;

		rest++;
		ptr = strchr(from, '!');
		strcpy(whoshit, "<Anonymous>");
		strcat(whoshit, ptr ? ptr : "");
	}

	if (add_to_shitlist(&(current->Userlist), uh, access, chan, whoshit,
		(rest && *rest) ? rest : "" , getthetime(),
		getthetime()+(86400 * days)))
	{
#ifdef USE_CCC
		send_to_ccc("1 SHIT %s %s %s %i %li %li %s",
			whoshit, chan, uh, access, getthetime(),
			getthetime()+(86400 * days), rest ? rest : "");		
#endif
		send_to_user( from, "\002The user has been shitted as %s on %s\002", uh, chan);
		send_to_user(from, "\002The shitlist will expire: %s\002", time2str(getthetime()+ (86400 * (long)days)));
		check_shit();
	}
	else
	  send_to_user( from, "\002%s\002", "Problem shitting the user");
	return;
}

void do_rshit(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *uh;
        int temp;
	aUser *tmp;

	chan = get_token(&rest, " ");
	if (!chan || !*chan || (!ischannel(chan) && my_stricmp(chan, "*")))
	{
		send_to_user(from, "\002%s\002",
			"Usage: RSHIT <channel or *> <nick or userhost>");
		return;
	}

	if (get_userlevel(from, chan) < cmdlevel)
	{
		return;
	}

	nick = get_token(&rest, " ");
	if (!nick)
	{
		send_to_user(from, "\002No nick or userhost specified\002");
		return;
	}
	if (!(uh = nick2uh(from, nick, 0)))
		return;
	if ((tmp=find_shit(&(current->Userlist), uh, chan)) == NULL)
	{
		send_to_user(from, "\002%s is not in my shit list on that channel\002", uh);
		return;
	}
	temp = get_userlevel(tmp->whoadd, chan);
	if (((temp > 99) ? 99 : temp) > get_userlevel(from, chan))
        {
	    send_to_user(from, "\002The person who did this shitlist has a higher level than you\002");
	    return;
	}
#ifdef USE_CCC
        send_to_ccc("1 RSHIT %s %s %s", from, tmp->channel, tmp->userhost);
#endif
	remove_user(&(current->Userlist), tmp);
	send_to_user(from, "\002User %s is no longer being shitted on %s\002",
		uh, chan);
}

void do_clvl(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *nuh, *bleah;
	int oldlevel, level, xlevel;
	aUser *tmp;

	oldlevel = level = 0;
	chan = get_channel(to, &rest);
	nick = get_token(&rest, " ");

	if (!nick)
	{
		send_to_user(from, "\002%s\002", "Specify a user to change");
		return;
	}
	if (!(nuh = nick2uh(from, nick, 0)))
		return;

	if ((tmp=find_user(&(current->Userlist), nuh, chan)) == NULL)
	{
		send_to_user( from, "\002%s is not in my users list on %s\002", nuh, chan);
		return;
	}

	if (!(bleah = get_token(&rest, " ")))
	{
		send_to_user(from, "\002%s\002", "No level specified");
		return;
    }

	if (check_for_number(from, bleah))
		return;
	level = atoi(bleah);

	if ((level < 0) || (level >= OWNERLEVEL+1))
	{
		send_to_user(from, "\002Valid levels are 0 thru %i\002", OWNERLEVEL);
		return;
	}

	oldlevel = tmp->access;
	
	if (level > get_userlevel(from, chan))
	{
		send_to_user(from, "\002%s\002",
			"I can not change the level higher than your own");
		return;
	}
	if (oldlevel > get_userlevel(from, chan))
	{
		send_to_user(from, "\002%s practices his immortality more than you!\002", nuh);
		return;
	}
	if (!matches(owneruserhost, nuh))
	{
		send_to_user(from, "\002%s is God and can not be altered\002", nuh);
		return;
	}

	xlevel = level - oldlevel;
	tmp->access = level;
	send_to_user(from, "\002Immortality level changed from %i to %i\002",
		oldlevel, level);
	if (level == OWNERLEVEL)
	{
	send_to_user(nuh, "\002%s has blessed you with God immortality on %s\002", currentnick, chan);
	send_to_user(nuh, "\002Your new immortality level is %i\002", level);
	send_to_user(nuh, "\002You have gained %i immortality levels\002", xlevel);
	}
	else
	{
		if (oldlevel > level) {
			return;
		}
		else {
			send_to_user(nuh, "\002%s has granted your immortality on %s to %i\002", currentnick, chan, level);
		}
	}
#ifdef USE_CCC
		send_to_ccc("1 CUSER %s %s %s %i -1 -1",
			from, chan, nuh, level);
#endif
}

void do_prot(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *nuh, *prot;
	int oldlevel, level;
	aUser *tmp;

	oldlevel = level = 0;
	chan = get_channel(to, &rest);
	nick = get_token(&rest, " ");

	if (!nick)
	{
		send_to_user(from, "\002%s\002", "Specify someone to protect");
		return;
	}
	if  (!(nuh = nick2uh(from, nick, 0)))
        	return;

	if ((tmp = find_user(&(current->Userlist), nuh, chan)) == NULL)
	{
		send_to_user( from, "\002%s is not in my users list on %s\002", nuh, chan);
		return;
	}

	prot = get_token(&rest, " ");
	if (prot)
        {
		if (check_for_number(from, prot))
                	return;
		level = atoi(prot);
	}
	if (!level || (level > MAXPROTLEVEL))
	{
		send_to_user(from, "\002Valid protect levels are 1 thru %i\002",
			MAXPROTLEVEL);
		return;
	}
	oldlevel = tmp->prot;
	tmp->prot = level;
	if (oldlevel)
		send_to_user( from, "\002Protect level changed from %i to %i\002",
			oldlevel, level);
	else
		send_to_user( from, "\002User now protected with level %i\002",
			level);
#ifdef USE_CCC
	send_to_ccc("1 CUSER %s %s %s -1 -1 %i", from, chan, nuh, level);
#endif 
}

void do_rprot(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *nuh;
	aUser *tmp;
	

	chan = get_channel(to, &rest);
	nick = get_token(&rest, " ");

	if (!nick)
	{
		send_to_user(from, "\002%s\002", "Specify someone to not protect");
		return;
	}
	if (!(nuh = nick2uh(from, nick, 0)))
		return;

	if ((tmp = find_user(&(current->Userlist), nuh, chan)) == NULL)
	{
		send_to_user( from, "\002%s is not in my users list on %s\002", nuh, chan);
		return;
	}
	if (!tmp->prot)
	{
		send_to_user( from, "\002%s is already not being protected on %s\002", nuh, chan);
		return;
	}
	tmp->prot = 0;
	send_to_user(from, "\002%s is no longer being protected on %s\002", nuh, chan);
#ifdef USE_CCC
	send_to_ccc(from, "1 CUSER %s %s %s -1 -1 0", from, chan, nuh);
#endif
}

void do_unban(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *nuh;
	

	chan = get_channel(to, &rest);
	nick = get_token(&rest, " ");

	if (get_userlevel(from, chan) < cmdlevel) {
		return;
	}
	
	if (!i_am_op(chan))
	{
		not_opped(from, chan);
		return;
	}
	if (!nick)
	{
		channel_unban(chan, from);
		send_to_user(from, "\002You have been unbanned on %s\002", chan);
	}
	if (!(nuh = nick2uh(from, nick, 0)))
		return;
	channel_unban(chan, nuh);
	send_to_user(from, "\002%s unbanned on %s\002", nuh, chan);
}

void do_listdcc(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	show_dcclist(from);
}

void do_spawn(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *nick;
	char buffer[MAXLEN];
	
	int templevel;
	
	templevel = get_userlevel(from, "#Onefuckedupchannel!#O!@I#");
	
	if (templevel < cmdlevel) {
		return;
	}
	
	if (!(nick = get_token(&rest, " ")))
	{
	strcpy(buffer, randstring(NICKSFILE));
	nick = buffer;
	}
	if (!isnick(nick))
	{
		send_to_user(from, "\002Invalid nickname: %s\002", nick);
		return;
	}
	if (!forkbot(from, nick, rest))
		send_to_user(from, "\002%s\002", "Couldn't spawn the bot");
}

void do_topic(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	

	chan = get_channel(to, &rest);
	
	if (get_userlevel(from, chan) < cmdlevel) {
		return;
	}

	if (!rest)
	{
		send_to_user(from, "\002%s\002", "No topic specified");
		return;
	}
	if (i_am_op(chan))
	{
		sendtopic(chan, rest);
		send_to_user(from, "\002Topic changed on %s\002", chan);
	}
	else
		not_opped(from, chan);
	return;
}

void do_spymsg(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	if (find_list(&current->SpyMsgList, from) != NULL)
	{
		send_to_user(from, "\002%s\002", "Already redirecting messages to you");
		return;
	}
	if (make_list(&current->SpyMsgList, from) != NULL)
		send_to_user(from, "\002%s\002", "Now redirecting messages to you");
	else
		send_to_user(from, "\002%s\002", "Can't redirect you the messages");
	return;
}

void do_statmsg(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	if (find_list(&current->StatMsgList, from) != NULL)
	{
		return;
	}
	if (make_list(&current->StatMsgList, from) != NULL)
		send_statmsg("(\002%s\002) Added to Mech Core", getnick(from));
	else
		send_to_user(from, "\002%s\002", "Can not add you to Mech Core due to an error");
	return;
}

void do_rspymsg(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char buffer[255];
	aList *tmp;

	strcpy(buffer, currentnick);
	tmp = find_list2(&current->SpyMsgList, buffer); 
	if (tmp)
	{
		remove_list(&current->SpyMsgList, tmp);
		send_to_user( from, "\002%s\002", "No longer redirecting messages to you");
	}
	else
		send_to_user(from, "\002%s\002", "I'm not redirecting messages to you");
	return;
}

void do_rstatmsg(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char buffer[255];
	aList *tmp;

	strcpy(buffer, currentnick);
	tmp = find_list2(&current->StatMsgList, buffer); 
	if (tmp)
	{
		remove_list(&current->StatMsgList, tmp);
	}

	return;
}

void do_cmdchar(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	if (!rest || !*rest)
	{
		send_to_user(from, "\002%s\002", "No command character specified");
		return;
	}

	if (isalnum(*rest))
		send_to_user(from, "\002%s\002", "Invalid command character");
	else
	{
		current->cmdchar = *rest;
		send_to_user(from, "\002My command character is now \"%c\"\002",
			current->cmdchar);
	}
	return;
}

void do_userhost(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *nick, *uh;

	if (!(nick = get_token(&rest, " ")))
	  {
		 send_to_user(from, "\002%s\002", "No nick specified");
		 return;
	  }
	
	uh = find_userhost(from, nick);
	if (!uh)
	  return;
	send_to_user(from, "\002Possible userhost: %s\002", uh);
	return;
}

void do_last10(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
/*
	int i;
	send_to_user(from, "\002%s\002", "Last 10 commands:");
	send_to_user(from, "\002%s\002", "-------------------------");
	for(i=0;i<10;i++)
		send_to_user(from, "\002%s: %10s by %s\002", time2str(current->lastcmds[i].time), current->lastcmds[i].name, current->lastcmds[i].from);
	send_to_user(from, "\002%s\002", "------ end of list ------");
*/
	return;
}

void do_server(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *server, *login, *ircname, *porta;
	int port;

	server = get_token(&rest, " ");
	if (!server)
	{
		send_to_user(from, "\002%s\002", "Usage: server <servername> [login] [ircname]");
		return;
	}
	login = get_token(&rest, " ");
	port = 6667;
	if (login && isdigit(*login))
	{
		porta = login;
		port = atoi(porta);
		if (!port)
			port = 6667;
		login = get_token(&rest, " ");
	}
	ircname = rest;
	if (login && *login)
		  strcpy(current->login, login);
	if (ircname && *ircname)
		 strcpy(current->ircname, ircname);
	send_to_user(from, "\002Trying new server: %s on port %i\002", server,
		port);
	sendquit("brb");

	if (!add_server(server, port) || !set_server(server, port))
	{
		send_to_user(from, "\002%s\002", "Problem switching servers");
		return;
	}

	connect_to_server();
	return;
}

void do_nextserver(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *login, *ircname;

	login = get_token(&rest, " ");
	ircname = rest;

	if (login && *login)
	  strcpy(current->login, login);
	 
	if (ircname && *ircname)
		strcpy(current->ircname, ircname);
	send_to_user(from, "\002%s\002", "Jumping to next server...");
	reconnect_to_server();
	return;
}

void do_steal(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	aList *temp;

	chan = get_token(&rest, " ");
	
	if (!chan || !*chan || !ischannel(chan))
	{
		send_to_user(from, "\002%s\002", "Trying to steal the following channels:");
		send_to_user(from, "\002%s\002", "---------------------------------------");
		for (temp=current->StealList;temp;temp=temp->next)
			send_to_user(from, "\002%s\002", temp->name);
		send_to_user(from, "\002%s\002", "-------------- end of list ------------");
		return;
	}

	if (get_userlevel(from, chan) < cmdlevel)
	{
		return;
	}

	if ((temp=find_list(&current->StealList, chan)) != NULL)
	{
		send_to_user(from, "\002%s\002", "Already stealing that channel");
		return;
	}
	if ((temp=make_list(&current->StealList, chan)) != NULL)
	{
		send_to_user(from, "\002Now trying to steal %s\002", chan);

			join_channel(chan, NULL, TRUE);
	}
	else
		send_to_user( from, "\002%s\002", "Problem adding the channel");
}

void do_rsteal(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	aList *tmp;

	chan = get_token(&rest, " ");

	if (!chan || !*chan || !ischannel(chan))
	{
		send_to_user(from, "\002%s\002", "No channel or invalid channel specified");
		return;
	}

	if (get_userlevel(from, chan) < cmdlevel)
	{
	  return;
	}
	
	if ((tmp = find_list(&current->StealList, chan)) == NULL)
	{
		send_to_user( from, "\002I'm not trying to steal %s\002", chan);
		return;
	}
	remove_list(&current->StealList, tmp);
	send_to_user(from, "\002No longer stealing %s\002", chan);
	leave_channel(chan);
}

void do_cycle(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	

	chan = get_channel(to, &rest);
	
	if (get_userlevel(from, chan) < cmdlevel) {
		return;
	}

	if (!mychannel(chan))
		send_to_user(from, "\002I'm not on %s\002", chan);
	else
	{
		send_to_user(from, "\002Cycling channel %s\002", chan);
		cycle_channel(chan);
	}
}

void do_spy(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	char *where;

	chan = get_token(&rest, " ");
	where = get_token(&rest, " ");

	if (get_userlevel(from, chan) < cmdlevel) {
		return;
	}
	
	if (where && !*where)
		where = NULL;
	if (!chan)
	{
		send_to_user(from, "\002%s\002", "No channel specified");
		return;
	}
	if (!mychannel(chan))
	{
		send_to_user(from, "\002I'm not on %s\002", chan);
		return;
	}
	if (get_userlevel(from, chan) < cmdlevel)
	{
		return;
	}
	if (where && !ischannel(where))
	{
		send_to_user(from, "\002Object must be a channel\002");
		return;
	}
	if (where && !mychannel(where))
	{
		send_to_user(from, "\002I'm not on %s\002", where);
		return;
	}
	if (add_spy(chan, where ? where : from))
		send_to_user(from, "\002Now spying on %s for you\002", chan);
	else
		send_to_user(from, "\002You are already spying on %s\002", chan);
	return;
}

void do_rspy(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	char buffer[255];
	aList *Temp;
	aChannel *Channel;
	
	chan = get_token(&rest, " ");
	strcpy(buffer, currentnick);
	if (rest && *rest)
		strcpy(buffer, rest);
	if (!chan)
	{
		send_to_user(from, "\002%s\002", "No channel specified");
		return;
	}
	if (!mychannel(chan))
	{
		send_to_user(from, "\002I'm not on %s\002", chan);
		return;
	}
	if (get_userlevel(from, chan) < cmdlevel)
	{
		return;
	}
	
	if (!(Channel = find_channel(chan)))
	{
		not_on(from, chan);
		return;
	}
	if ((Temp = find_list2(&Channel->SpyList, buffer)) == NULL)
	{
		send_to_user(from, "\002You are not spying on %s\002", chan);
		return;
	}
	if (get_userlevel(Temp->name, chan) > get_userlevel(from, chan))
	{
		send_to_user(from, "\002Can't remove someone with a higher access level than you\002");
		return;
	}
	remove_list(&Channel->SpyList, Temp);
	send_to_user(from, "\002No longer spying on %s for you\002", chan);
	return;
}

void do_spylist(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;

	chan = get_token(&rest, " ");
	if (!chan)
	{
		send_to_user(from, "\002--- Spy Message List\002");
		show_spymsg_list(from);
		send_to_user(from, "\002--- Stat Message List\002");
		show_statmsg_list(from);
		send_to_user(from, "\002%s\002", "--- End Spy Message List");
		return;
	}
	if (!mychannel(chan))
	{
		send_to_user(from, "\002I'm not on %s\002", chan);
		return;
	}

	if (get_userlevel(from, chan) < cmdlevel)
	{
		return;
	}

	send_to_user(from, "\002--- Spy List for %s\002", chan);
	show_spy_list(from, chan);
	send_to_user(from, "\002%s\002", "--- End of Spy List ---");
}


void do_restrict(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	int newlevel;

	if (!rest)
	{
		if (current->restrict)
			send_to_user(from, "\002I'm restricted to level %i\002",
					  current->restrict);
		else
			send_to_user(from, "\002%s\002", "I'm not being restricted to any level");
		return;
	}
	newlevel = atoi(rest);
	if (newlevel < 0)
	 send_to_user(from, "\002%s\002", "Restrict level must be >= 0");
	else if (newlevel > OWNERLEVEL)
	 send_to_user(from, "\002Restrict level must be <= %i\002", OWNERLEVEL);
	else if (newlevel > max_userlevel(from))
	 send_to_user(from, "\002%s\002", "I can't restrict to a higher level than your user level");
	else
	{
	 if (newlevel)
		 send_to_user(from, "\002Now restricted at level %i\002", newlevel);
	 else
		 send_to_user(from, "\002%s\002", "Not restricting access");
	 current->restrict = newlevel;
	}
	return;
}

void do_userlist(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	show_userlist(from, current->Userlist);
}

void do_shitlist(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	show_shitlist(from, current->Userlist);
}

void do_greetlist(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	show_greetlist(from, current->Userlist);
}

void do_kicksaylist(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	show_kicksaylist(from, current->Userlist);
}

void do_saysaylist(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	show_saysaylist(from, current->Userlist);
}

void do_nicklist(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
       register aUser *dummy;

       send_to_user(from, "------ \002Userhost\002 --- \002Level\002 ---- \002Reason\002 -----");
       for (dummy = current->Userlist; dummy; dummy = dummy->next )
	{
		if (IsProtNick(dummy))
	               send_to_user( from, " %15s     %3d    %s",
                       dummy->userhost, dummy->access, dummy->reason);
	}
        send_to_user(from, "--- End of nick list ---");
}

void do_banlist(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	  

	chan = get_channel(to, &rest);
	send_to_user(from, "\002Banlist on %s:\002", chan);
	send_to_user(from, "----------------------------");
	show_banlist(from, chan);
	send_to_user(from, "--- \002End of %s banlist\002 ---", chan);
}

void do_passwd(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *old, *new, *newpass;
	aUser *tmp;

	old = get_token(&rest, " ");
	new = get_token(&rest, " ");

	if (!old || !new || !*old || !*new)
	{
		send_to_user(from, "\002Usage: passwd <oldpasswd> <newpasswd>\002");
		return;
	}
	if ((tmp = find_user(&(current->Userlist), from, NULL)) == NULL)
	{
		send_to_user(from, "\002You aren't in my userlist\002");
		return;
	}
	if (!correct_password(from, old))
	{
		send_to_user(from, "\002Old password is not correct\002");
		return;
	}
	if ((strlen(new) < 3) || (strlen(new) > 15))
	{
		send_to_user(from, "\002Password must be between 3 and 15 characters\002");
		return;
	 }
	newpass = encode(new);
#ifdef USECCC
	send_to_ccc("1 CUSER %s * %s -1 -1 -1 %s", from, from, newpass); 
#endif
	if (change_password(&(current->Userlist), from, newpass) &&
		saveuserlist(&(current->Userlist), current->USERFILE))
		send_to_user(from, "\002New password saved\002");
	else
		send_to_user(from, "\002Problem saving the password\002");
}

void do_setpass(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
   char *nick, *nuh, *pass, *newpass;

   nick = get_token(&rest, " ");
   pass = get_token(&rest, " ");

   if (!nick || !pass || !*nick || !*pass)
   {
	send_to_user(from, "\002Usage: setpass <nick or userhost> <password>\002");
	return;
   }
   if ((strlen(pass) < 3) || (strlen(pass) > 15))
   {
	send_to_user(from, "\002Password must be between 3 and 15 characters\002");
	return;
   }
   if (!(nuh = nick2uh(from, nick, 0)))
   	return;
   newpass = encode(pass);
#ifdef USE_CCC
   send_to_ccc("1 CUSER %s * %s -1 -1 -1 %s", from, nuh, newpass);
#endif
   if (change_password(&(current->Userlist), nuh, newpass) &&
		saveuserlist(&(current->Userlist), current->USERFILE))
	 send_to_user(from, "\002New password saved\002");
   else
	 send_to_user(from, "\002Problem saving the password\002");
}

void do_showidle(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *seconds;
	
  
	chan = get_channel(to, &rest);
	seconds = get_token(&rest, " ");
	if (!seconds || !*seconds)
		seconds = "5";
	show_idletimes(from, chan, atoi(seconds));
}

void do_limit(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	int oldlimit, limit;
  
	chan = get_channel(to, &rest);
	oldlimit = get_limit(chan);
	if (!rest || !*rest)
	{
		if (oldlimit)
	send_to_user(from, "\002Current user limit on %s is %i\002",
			  chan, oldlimit);
		else
	send_to_user(from, "\002There is no limit on %s\002", chan);
		return;
	}
	limit = atoi(rest);
	if (limit < 0)
	{
		send_to_user(from, "\002Limit can be 0 for no limit, but not less than 0\002");
		return;
	 }
	if (set_limit(chan, limit))
	{
		if (limit == oldlimit)
	send_to_user(from, "\002Limit on %s was not changed from %i\002",
			  chan, limit);
		else
	send_to_user(from, "\002Limit on %s was changed from %i to %i\002",
			  chan, oldlimit, limit);
	}
	else
		send_to_user(from, "\002Problem changing the limit\002");
	check_limit(chan);
}

void do_chanstats(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	
	chan = get_channel(to, &rest);
	do_who(chan);
	chan_stats(from, chan);
}

void do_relogin(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  static char lastpass[100] = "";
  char buffer[MAXLEN], Buf[MAXLEN];
  char *shit;
  FILE *fp;
  int right_pword = 0;

  shit = rest;
  if (!rest || !*rest)
    shit = lastpass;

  strcpy(lastpass, shit);
  sprintf(buffer, "echo \"%s\" | relogin", shit);
  if ((fp = popen(buffer, "r")) == NULL)
     send_to_user(from, "\002Execution of relogin command failed\002");
  else
  {
     while (fgets(Buf, MAXLEN, fp))
     {
        Buf[strlen(Buf)-1] = '\0';
        send_to_user(from, "\002%s\002", Buf);
        if (!strncmp(Buf, "Verify", 4))
           right_pword = 1;
     }

     pclose(fp);
     if (!right_pword)
	send_to_user(from, "\002Relogin failed, incorrect password\002");
     else
        send_to_user(from, "\002Relogin successful\002");
  }
}

void do_setshitreason(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
        char *chan, *nick, *uh;
        int temp;
	aUser *tmp;

        chan = get_token(&rest, " ");
        if (!chan || !*chan || (!ischannel(chan) && my_stricmp(chan, "*")))
        {
                send_to_user(from, "\002%s\002",
                        "Usage: SETSHITREASON <channel or *> <nick or userhost> <newreason>");
                return;
        }
        if (get_userlevel(from, chan) < cmdlevel)
        {
                return;
        }
        nick = get_token(&rest, " ");
        if (!nick)
        {
                send_to_user(from, "\002No nick or userhost specified\002"
);
                return;
        }
	if (!rest || !*rest)
	{
		send_to_user(from, "\002No reason given\002");
		return;
	}
        if (!(uh = nick2uh(from, nick, 0)))
                return;
        if ((tmp=find_shit(&(current->Userlist), uh, chan)) == NULL)
        {
                send_to_user(from, "\002%s is not in my shit list on that channe
l\002", uh);
                return;
        }
        temp = get_userlevel(tmp->whoadd, chan);
        if (((temp > 99) ? 99 : temp) > get_userlevel(from, chan))
        {
            send_to_user(from, "\002The person who did this shitlist has a highe
r level than you\002");
            return;
        }
	MyFree(&tmp->whoadd);
	MyFree(&tmp->reason);
	mstrcpy(&tmp->whoadd, from);
	mstrcpy(&tmp->reason, rest);
	send_to_user(from, "\002The reason has been set\002");
}

void do_cslvl(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
        char *chan, *nick, *uh;
        int temp, newlevel;
	aUser *tmp;

        chan = get_token(&rest, " ");

        if (!chan || !*chan || (!ischannel(chan) && my_stricmp(chan, "*")))
        {
                send_to_user(from, "\002%s\002",
                        "Usage: CSLVL <channel or *> <nick or userhost> <newlevel>");
                return;
        }

        if (get_userlevel(from, chan) < cmdlevel)
        {
                return;
        }

        nick = get_token(&rest, " ");
        if (!nick)
        {
                send_to_user(from, "\002%s\002", "No nick or userhost specified"
);
                return;
        }
	if (!(uh = nick2uh(from, nick, 0)))
		return;
	if ((tmp=find_shit(&(current->Userlist), uh, chan)) == NULL)
	{
		send_to_user(from,
		    "\002%s is not in my shit list on that channel\002", uh);
                return;
	}
        if (!rest || !*rest)
        {
                send_to_user(from, "\002%s\002", "No level given");
                return;
        }
	if (check_for_number(from, rest))
		return;
	newlevel = atoi(rest);
	if ((newlevel <= 0) || (newlevel > MAXSHITLEVEL))
        {
                send_to_user(from, "\002Valid levels are from 1 thru %i\002",
			MAXSHITLEVEL);
                return;
        }
        temp = get_userlevel(tmp->whoadd, chan);
        if (((temp > 99) ? 99 : temp) > get_userlevel(from, chan))
        {
            send_to_user(from, "\002The person who did this shitlist has a highe
r level than you\002");
            return;
        }
	tmp->access = newlevel;
	send_to_user(from, "\002The level has been changed\002");
}

void do_idle(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	if (!rest || !*rest)
	{
		send_to_user(from, "\002No nickname given\002");
		return;
	}
	if (!is_present(rest))
	{
		send_to_user(from, "\002That user is not on any of my channels\002");
		return;
	}
	send_to_user(from, "\002%s has been idle for %s\002",
		rest, idle2str(getthetime()-get_idletime(rest)));
}

void do_protnick(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char thenick[20];
	char *nick, *level, *reason;
	int thelevel;
	aUser *tmp;

	if (!(nick = get_token(&rest, " ")))
	{
		send_to_user(from, "\002%s\002", "No nickname spec given");
		return;
	}
	if (!(level = get_token(&rest, " ")))
	{
		send_to_user(from, "\002%s\002", "No level given");
		return;
	}
	reason = (rest && *rest) ? rest : "This nick is reserved";
	if (check_for_number(from, level))
		return;
	thelevel = atoi(level);
	if ((thelevel <= 0) || (thelevel > OWNERLEVEL))
	{
		send_to_user(from, "\002The level must be between 1 and %i\002",
			OWNERLEVEL);
		return;
	}
	if (strchr(nick, '!'))
		*strchr(nick, '!') = '\0';
	strncpy(thenick, nick, 15);
	thenick[15] = '\0'; /* just in case nick happens to be longer than 15 */
	strcat(thenick, "!*@*");
	if ((tmp=find_protnick(&(current->Userlist), thenick)) != NULL)
	{
		send_to_user(from, "\002That nick is already being protected\002");
		return;
	}
	tmp = make_user(&(current->Userlist));
	mstrcpy(&tmp->userhost, thenick);
	mstrcpy(&tmp->channel, "*");
	tmp->access = thelevel;
	mstrcpy(&tmp->whoadd, from);
	mstrcpy(&tmp->reason, reason);
	tmp->type |= USERLIST_PROTNICK;
	send_to_user(from, "\002%s added to the nick protection list\002",
		thenick);
	check_protnicks();
}

void do_rprotnick(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char thenick[20];
	char *nick;
	aUser *tmp;

	if (!(nick = get_token(&rest, " ")))
	{
		send_to_user(from, "\002No nickname spec given to "
			"unprotect\002");
		return;
	}
	if (strchr(nick, '!'))
		*strchr(nick, '!') = '\0';
	strncpy(thenick, nick, 15);
	thenick[15] = '\0'; /* just in case nick happens to be longer than 15 */
	strcat(thenick, "!*@*");
	if ((tmp=find_protnick(&(current->Userlist), thenick)) != NULL)
	{
		remove_user(&(current->Userlist), tmp);
		send_to_user(from, "\002%s removed from the nick protection "
			"list\002", thenick);
	}
	else
		send_to_user(from, "\002That nick isn't being protected\002");
}

void do_kicks(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *nuh, *temp;
	aTime *Kicks;

	if ((nuh = get_token(&rest, " ")) != NULL)
		nuh = nick2uh(from, nuh, 0);
	else
		nuh = from;
	if (!nuh)
		return;
	temp = format_nuh(nuh);
	Kicks = find_time(&current->KickList, temp);
	send_to_user(from, "\002Kicks for %s: %i\002", temp,
		Kicks ? Kicks->num : 0);
	if (Kicks)
		send_to_user(from, "\002Last kick at %s\002",
			time2str(Kicks->time));
}

void do_topkicks(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *Nicks[11];
	time_t Times[11];
	int Num[11];
	int count, done, i, ii;
	aTime *Kicks;

	for (i=0;i<10;i++)
	{
		Nicks[i] = NULL;
		Num[i] = 0;
		Times[i] = 0;
	}
	count = 0;
	for (Kicks=current->KickList;Kicks;Kicks=Kicks->next)
	{
		done=0;
		for(i=0;i<10;i++)
		{
			if (!done && (Kicks->num > Num[i]))
			{
				count++;
				for(ii=9;ii>i;ii--)
				{
					Nicks[ii] = Nicks[ii-1];
					Num[ii] = Num[ii-1];
					Times[ii] = Times[ii-1];
				}
				done++;
				Nicks[i] = Kicks->name;
				Num[i] = Kicks->num;
				Times[i] = Kicks->time;
			}
		}
	}
	if (count > 10)
		count = 10;
	for (i=0;i<count;i++)
		send_to_user(from, "\002%2i) %4i: %s\002",
			i+1, Num[i], Nicks[i]);
	if (!count)
		send_to_user(from, "\002No one has kicked\002");
}

void do_rollcall(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	send_to_user(from, "\002My command character is %c\002",
		current->cmdchar);
}

void do_saveall(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	do_savelists(from, to, rest, cmdlevel);
	do_savelevels(from, to, rest, cmdlevel);
	do_savekicks(from, to, rest, cmdlevel);
	if (!write_biglist(&current->dList, current->dfile)) {
        send_statmsg("\002Dictionary could not be saved to file %s\002", current->dfile);
	}
#ifdef SERVER_MONITOR
	if (!write_biglist(&current->KillList, current->kfile)) {
        send_statmsg("\002KillList could not be saved to file %s\002", current->kfile);
	}
	if (!write_timelist(&current->KillList2, current->kfile2)) {
        send_statmsg("\002KillList2 could not be saved to file %s\002", current->kfile2);
	}
#endif
	if (!write_seenlist(&current->SeenList, current->seenfile)) {
        send_statmsg("\002SeenList could not be saved to file %s\002", current->seenfile);
	}
}

void do_loadall(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	do_loadlists(from, to, rest, cmdlevel);
	do_loadlevels(from, to, rest, cmdlevel);
	do_loadkicks(from, to, rest, cmdlevel);
}

void do_names(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	

	chan = get_channel(to, &rest);
	show_names_on_channel(from, chan);
}

void do_collect(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	aChannel *Chan;
        char *chan;

        chan = get_token(&rest, " ");
        if (!chan)
        {
                send_to_user(from, "\002No channel name specified\002");
                return;
        }
        if (!ischannel(chan))
        {
                send_to_user(from, "\002Invalid channel name\002");
                return;
        }

        if (get_userlevel(from, chan) < cmdlevel)
        {
                return;
        }
	if ((Chan = find_channel(chan)) != NULL)
	{
		send_to_user(from, "\002I'm on that channel dipshit\002");
		return;
	}
        send_to_user(from, "\002Attempting to collect data from %s\002", chan);
        join_channel(chan, rest, TRUE);
	if (!(Chan = find_oldchannel(chan)))
		leave_channel(chan);
	else
	{
		char *temp;
		mstrcpy(&temp, from);
		Chan->flags = COLLECT_TYPE;
		Chan->collectby = temp;
	}
	leave_channel(chan);
}

void do_forget(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	aChannel *Chan;

	if (!(chan = get_token(&rest, " ")))
	{
		send_to_user(from, "\002No channel specified to forget\002");
		return;
	}
	if (!(Chan = find_oldchannel(chan)))
	{
		send_to_user(from, "\002Channel %s is not in memory\002",
			chan);
		return;
	}
	if (delete_channel(&current->OldChannel_list, Chan))
		send_to_user(from, "\002Channel %s is now forgotten\002",
			chan);
	else
		send_to_user(from, "\002Channel %s is not in memory\002",
			chan);
}

void do_splits(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	aLink *temp;
	register int count;

	if (!get_int_var(NULL, TOGWL_VAR))
	{
		send_to_user(from,
			"\002Link watching is currently toggled off\002");
		return;
	}
	count = 0;
	for (temp=current->TheLinks;temp;temp=temp->next)
		if (temp->status & SPLIT)
		{
			count++;
			send_to_user(from, "\002%s [%s]\002", temp->name,
					temp->link);
		}
	if (!count)
		send_to_user(from, "\002I find no split servers\002");
}

void do_links(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	register aLink *temp;
	register int count;

	if (!get_int_var(NULL, TOGWL_VAR))
	{
		send_to_user(from,
			"\002Link watching is currently toggled off\002");
		return;
	}
	count = 0;
	for (temp=current->TheLinks;temp;temp=temp->next)
	{
		count++;
		send_to_user(from, "\002%s [%s]\002", temp->name,
			temp->link);
	}
	send_to_user(from, "\002Total number of servers: %i\002", count);
}

void do_topsplits(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
        char *Server[11];
        int Num[11];
        int count, done, i, ii;
        aLink *Temp;

	for (i=0;i<10;i++)
	{
		Server[i] = (char *) 0;
		Num[i] = 0;
	}
        count = 0;
        for (Temp=current->TheLinks;Temp;Temp=Temp->next)
        {
                done=0;
                for(i=0;i<10;i++)
                {
                        if (!done && (Temp->num > Num[i]))
                        {
                                count++;
                                for(ii=9;ii>i;ii--)
                                {
                                        Server[ii] = Server[ii-1];
                                        Num[ii] = Num[ii-1];
                                }
                                done++;
                                Server[i] = Temp->name;
                                Num[i] = Temp->num;
                        }
                }
	}
        if (count > 10)
                count = 10;
        for (i=0;i<count;i++)
                send_to_user(from, "\002%2i) %4i: %s\002",
                        i+1, Num[i], Server[i]);
        if (!count)
                send_to_user(from, "\002No servers have split\002");
}

void do_greet(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
        char *nick, *uh;
        aUser *tmp;

	if (!(nick = get_token(&rest, " ")))
         {
                send_to_user(from, "\002No nick or userhost specified\002");
                return;
         }
	if (!(uh = nick2uh(from, nick, 0)))
		return;
	tmp = find_greet(&(current->Userlist), uh);
	if (tmp)
	{
		if (!rest || !*rest)
		{
			send_to_user(from,
		"\002The current greeting for %s is:\002", tmp->userhost);
			send_to_user(from, "\002%s\002", tmp->reason);
			return;
		}
		MyFree(&tmp->reason);
		mstrcpy(&tmp->reason, rest);
	send_to_user(from,"\002The greeting for %s has been changed\002",
			tmp->userhost);
	}
	else
	{
		char *uh2;

		if (!rest || !*rest)
		{
			send_to_user(from, "\002I'm not greeting %s\002",
				uh);
			return;
		}
		uh2 = format_uh(uh, 1);
		tmp=make_user(&(current->Userlist));
		tmp->type |= USERLIST_GREET;
		mstrcpy(&tmp->userhost, uh2);
		mstrcpy(&tmp->whoadd, from);
		mstrcpy(&tmp->reason, rest);
		send_to_user(from, "\002I'm now greeting %s\002", uh2);
	}
}

void do_rgreet(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
        char *nick, *uh;
        aUser *tmp;

        if (!(nick = get_token(&rest, " ")))
         {
                send_to_user(from, "\002No nick or userhost specified\002");
                return;
         }
        if (!(uh = nick2uh(from, nick, 0)))
                return;
        tmp = find_greet(&(current->Userlist), uh);
        if (!tmp)
                send_to_user(from, "\002I'm already not greeting %s\002",
                        uh);
        else
        {
		remove_user(&(current->Userlist), tmp);
                send_to_user(from, "\002I'm no longer greeting %s\002", uh);
	}
}

void saysay_usage(to)
char *to;
{
        send_to_user(to, "\002Usage: ss <channel or *> <\"String to respond to\"> <\"What to say\">\002");
}

void kicksay_usage(to)
char *to;
{
        send_to_user(to, "\002Usage: ks <channel or *> <\"String to kick on\"> <\"kick reason\">\002");
}

void do_kicksay(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
        char *chan, *string, *what;
        aUser *tmp;

        chan = get_token(&rest, " ");
        if (!chan || !*chan)
        {
		kicksay_usage(from);
                return;
        }
        if (!ischannel(chan) && strcmp(chan, "*"))
        {
		kicksay_usage(from);
                return;
        }
        if (!rest || !*rest)
        {
		kicksay_usage(from);
                return;
        }
	if (*rest != '"')
	{
		send_to_user(from, "\002Please put quotes (\"'s) around the arguements\002");
		send_to_user(from, "\002Example: ks * \"Fuck you\" \"No, fuck you %%n\"\002");
		return;
	} 
	rest++;
	string = get_token(&rest, "\"");
	if (!*string)
	{
		kicksay_usage(from);
		return;
	}
	if (rest && *rest == ' ')
		rest++;
	if (rest && *rest != '"')
	{
		kicksay_usage(from);
		return;
	}
	if (rest)
		rest++;
	if ((what=rest) != NULL)
		what = get_token(&rest, "\"");
	if (!what || !*what)		
	{
		kicksay_usage(from);
		return;
	}
        tmp = find_kicksay(&(current->Userlist), string, chan);
        if (tmp)
        {
                send_to_user(from, "\002I'm already kicking on \"%s\"\002",
                        tmp->userhost);
                return;
        }
        tmp=make_user(&(current->Userlist));
        tmp->type |= USERLIST_KICKSAY;
        if (*string != '*')
                mstrcpy(&tmp->userhost, "*");
        mstrcat(&tmp->userhost, string);
        if (*(string+strlen(string)-1) != '*')
                mstrcat(&tmp->userhost, "*");
        mstrcpy(&tmp->whoadd, from);
        mstrcpy(&tmp->channel, chan);
	mstrcpy(&tmp->reason, what);
	send_to_user(from, "\002Now kicking on \"%s\" on %s\002",
		tmp->userhost, chan);
}

void do_rkicksay(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
        char *chan;
        aUser *tmp;

        chan = get_token(&rest, " ");
        if (!chan || !*chan)
        {
                send_to_user(from, "\002No channel specified\002");
                return;
        }
        if (!ischannel(chan) && strcmp(chan, "*"))
        {
                send_to_user(from, "\002Specify a channel name or '*' for all\002");
                return;
        }
        if (!rest || !*rest)
        {
                send_to_user(from, "\002No kicksay string specified\002");
                return;
        }
        tmp = find_kicksay(&(current->Userlist), rest, chan);
        if (!tmp)
        {
                send_to_user(from, "\002I'm not kicking on that\002");
                return;
        }
	send_to_user(from, "\002No longer kicking on %s\002",
		tmp->userhost);
	remove_user(&(current->Userlist), tmp);
}


void do_saysay(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
        char *chan, *string, *what;
        aUser *tmp;

        chan = get_token(&rest, " ");
        if (!chan || !*chan)
        {
		saysay_usage(from);
                return;
        }
        if (!ischannel(chan) && strcmp(chan, "*"))
        {
		saysay_usage(from);
                return;
        }
        if (!rest || !*rest)
        {
		saysay_usage(from);
                return;
        }
	if (*rest != '"')
	{
		send_to_user(from, "\002Please put quotes (\"'s) around the arguements\002");
		send_to_user(from, "\002Example: ss * \"Fuck you\" \"No, fuck you %%n\"\002");
		return;
	} 
	rest++;
	string = get_token(&rest, "\"");
	if (!*string)
	{
		saysay_usage(from);
		return;
	}
	if (rest && *rest == ' ')
		rest++;
	if (rest && *rest != '"')
	{
		saysay_usage(from);
		return;
	}
	if (rest)
		rest++;
	if ((what=rest) != NULL)
		what = get_token(&rest, "\"");
	if (!what || !*what)		
	{
		saysay_usage(from);
		return;
	}
        tmp = find_saysay(&(current->Userlist), string, chan);
        if (tmp)
        {
                send_to_user(from, "\002I'm already talking on \"%s\"\002",
                        tmp->userhost);
                return;
        }
        tmp=make_user(&(current->Userlist));
        tmp->type |= USERLIST_SAYSAY;
        if (*string != '*')
                mstrcpy(&tmp->userhost, "*");
        mstrcat(&tmp->userhost, string);
        if (*(string+strlen(string)-1) != '*')
                mstrcat(&tmp->userhost, "*");
        mstrcpy(&tmp->whoadd, from);
        mstrcpy(&tmp->channel, chan);
	mstrcpy(&tmp->reason, what);
	send_to_user(from, "\002Now responding to \"%s\" on %s\002",
		tmp->userhost, chan);
}

void do_rsaysay(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
        char *chan;
        aUser *tmp;

        chan = get_token(&rest, " ");
        if (!chan || !*chan)
        {
                send_to_user(from, "\002No channel specified\002");
                return;
        }
        if (!ischannel(chan) && strcmp(chan, "*"))
        {
                send_to_user(from, "\002Specify a channel name or '*' for all\002");
                return;
        }
        if (!rest || !*rest)
        {
                send_to_user(from, "\002No saysay string specified\002");
                return;
        }
        tmp = find_saysay(&(current->Userlist), rest, chan);
        if (!tmp)
        {
                send_to_user(from, "\002I'm not responding to that\002");
                return;
        }
	send_to_user(from, "\002No longer responding to %s\002",
		tmp->userhost);
	remove_user(&(current->Userlist), tmp);
}
