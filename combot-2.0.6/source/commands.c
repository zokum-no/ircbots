/*
 * commands.c - all the commands...
 * (c) 1995 CoMSTuD (cbehrens@slavery.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>

#include "global.h"
#include "config.h"
#include "defines.h"
#include "structs.h"
#include "h.h"
#include "commands.h"

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
				send_to_user(from, "\002No such command\002");
			else
				send_to_user(from, "\002The level needed for that command is: %i\002", level);
                	return;
		}
		nuh = nick2uh(from, nuh, 0);
	}
	else
		nuh = from;
	if (!nuh)
		return;
	send_to_user(from, "\002Access for %s\002", nuh);
	send_to_user(from, "\002Channel: %s  Access: %i\002", chan,
		get_userlevel(nuh, chan));
	if (current->restrict)
		send_to_user(from,
			"\002Note: I'm being restricted to level %i\002",
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
		send_to_user(from, "\002You have now been authorized\002");
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
	if ((nuh = get_token(&rest, " ")) != NULL)
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

	User = find_user(&Userlist, nuh, chan); 
	Shit = find_shit(&Userlist, nuh, chan);
	ProtNick = find_protnick(&Userlist, nuh);
	Greet = find_greet(&Userlist, nuh);
	send_to_user(from, "Stats for %s on %s", nuh, chan);
	send_to_user(from, "--------------------------");
	send_to_user(from, "      User level: %i", User ? User->access : 0);
	if (User)
		send_to_user(from, "  Recognized u@h: %s", User->userhost);
	send_to_user(from,  "        Verified: %s",
		(!authd) ? "No password needed" : (authd==1) ? "Yes" : "No");
	send_to_user(from, " Protected level: %i%s",
		User ? User->prot : 0,
		get_int_var(chan, TOGPROT_VAR) ? "" :
		"   \002(Protection toggled off)\002");
	send_to_user(from,"      Auto Opped: %s%s",
		(User && User->aop) ? "Yes" : "No",
		get_int_var(chan, TOGAOP_VAR) ? "" :
		"   \002(Auto-op is toggled off)\002");
	send_to_user(from, "Shitlisted level: %i",
		Shit ? Shit->access : 0);
	if (Shit)
	{
		send_to_user(from, "   Shitlisted as: %s", Shit->userhost);
		send_to_user(from, "   Shitlisted by: %s",
			Shit->whoadd ? Shit->whoadd : "<Anonymous>");
		send_to_user(from, "Shitlist expires: %s",
			time2str(Shit->expire));
		send_to_user(from, " Shitlist reason: %s",
			Shit->reason ? Shit->reason : "<No reason>");	
 	} 
        if (ProtNick)
        {
                send_to_user(from, "                    ---");
                send_to_user(from, " Protected Nick: %s", ProtNick->userhost);
                send_to_user(from, "   Level needed: %i", ProtNick->access);
        }
        if (Greet)
        {
                send_to_user(from, "                    ---");
                send_to_user(from, "  Greeting u@h: %s", Greet->userhost);
                send_to_user(from, "      Greeting: %s", Greet->reason);
        }
}

void do_version(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	send_to_user(from, "\002ComBot Version %s (C) 1995 CoMSTuD, Released %s\002",
		  VERSION, BOTDATE);
	send_to_user(from, "\002Written by: CoMSTuD!cbehrens@slavery.com\002");
}

void do_time(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	send_to_user(from, "\002Current time: %s\002", time2str(getthetime()));
}

void do_info(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	do_version(from, to, rest, cmdlevel);
	send_to_user(from, "\002Started: %-20.20s\002", time2str(current->uptime));
	send_to_user(from, "\002Up: %s\002", idle2str(getthetime()-current->uptime));
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
	int ulevel;

	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
		return;
	}
	if (usermode(chan, currentnick) & MODE_CHANOP)
		send_to_user(from, "\002You're already channel operator on %s\002",
			chan);
	else
		giveop(chan, currentnick);
}

void do_deopme(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;

	chan = get_channel(to, &rest);

	if (usermode(chan, currentnick) & MODE_CHANOP)
		takeop(chan, currentnick);
	else
		send_to_user(from, "\002You're already not a channel operator on %s\002",
			chan);
}

void do_op(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	int ulevel;

	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
		return;
	}
	if (rest)
		mode3(chan, "+ooo", rest);
	else
		send_to_user( from, "\002%s\002",
			"Who do you want me to op, you schmuck?");
}

void do_deop(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	int ulevel;

	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (rest)
		mode3(chan, "-ooo", rest);
	else
		send_to_user( from, "\002%s\002",
			"Who do you want me to deop, you schmuck?");
}

void do_invite(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick;
	int ulevel;

	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	nick = currentnick;

	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (rest && *rest)
	  nick = rest;

	if (!invite_to_channel(nick, chan))
	  send_to_user(from, "\002I'm not on channel %s, you dickhead\002", chan);
	else
	  send_to_user(from, "\002User(s) invited to %s\002", chan);
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

void do_send(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	send_to_user(from, "\002%s\002", "Sorry, \"send\" is disabled in this version");
}

void do_flist(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	send_to_user(from, "\002%s\002", "Sorry, \"files\" is disabled in this version");
}

void do_say(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	int ulevel;

	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!rest)
	{
		send_to_user(from,
			"\002What am I suppposed to say, dilweed?\002");
		return;
	}
	sendprivmsg(chan, "\002\002%s", rest);
}

void do_bomb(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *nick;

	if (!rest || !*rest)
	{
		send_to_user(from, "\002Who am I supposed to bomb?\002");
		return;
	}
	nick = get_token(&rest, " ");

	if (nick && ischannel(nick))
	{
		if (get_userlevel(from, nick) < cmdlevel)
		{
			no_access(from, nick);
			return;
		}
	}

	if (nick)
		send_ctcp_reply(nick, "\002\002%s", rest ? rest : "");
	send_to_user(from, "\002Bombed %s\002", nick);
}

void do_me(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	char text[HUGE];
	int ulevel;

	strcpy(text, "ACTION \002\002");
	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!rest)
	{
		send_to_user( from,
			"\002What am I suppposed to action, dilweed?\002");
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
		send_to_user(from, "\002No nick given\002");
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
		send_to_user(from, "\002Use \"%csay\" instead\002",
			current->cmdchar);
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
		sendprivmsg(nick, "\002\002%s", rest);
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
		send_to_server( "%s:\002\002%s", rest, temp);
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
		no_access(from, chan);
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
	int ulevel;

	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
		return;
	}
	if (find_list(&current->StealList, chan))
		send_to_user(from, "\002Sorry, I'm trying to steal %s\002",
			chan);
  else
  {
	send_to_user(from, "\002Attempting the part of %s\002", chan);
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
	if (rest)
		signoff(from, rest);
	else
		signoff(from, "Later...");
	if (number_of_bots == 0)
		MyExit(0);
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
		no_access(from, chan);
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
  access = atoi(level);
  if ((access < 0) || (access >= OWNERLEVEL))
  {
		send_to_user(from, "\002Valid levels are from 0 thru %i\002", OWNERLEVEL-1);
		return;
  }
  if (access > templevel)
  {
		send_to_user(from, "\002Level must be <= to yours on %s\002", chan);
		return;
  }
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
  if ((tmp=add_to_userlist(&Userlist, uh, access, aop ? 1 : 0, prot,
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
	if (already)
		send_to_user(from, "\002User added *again* as %s on %s\002", uh, chan);
        else
		send_to_user( from, "\002The user has been added as %s on %s\002", uh, chan);
	send_to_user( from, "\002Access level: %i  Auto-op: %s  Protect level: %i  %s %s\002",
			  access, aop ? "Yes" : "No", prot,
			  (pass && *pass) ? "Password:" : "",
			  (pass && *pass) ? pass : "");
	sendnotice(nick,
			"\002%s has given you level %i clearance\002", currentnick, access);
	sendnotice(nick, "\002You are %sbeing auto-opped\002", aop ? "" : "not ");
	if (prot)
	  sendnotice(nick, "\002You are being protected with level %i\002",
			  prot);
	else
	  sendnotice(nick, "\002%s\002", "You are not being protected");
	sendnotice(nick, "\002My command character is %c\002", current->cmdchar);
	if (pass && *pass)
	{
	  sendnotice(nick, "\002Password necessary for doing commands: %s\002",
			  pass);
	  sendnotice(nick, "\002If you don't like the password, use \"passwd\" to change it\002");
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
	chan = get_token(&rest, " ");
  
	if (!chan || !*chan || (!ischannel(chan) && my_stricmp(chan, "*")))
	{
		send_to_user(from, "\002%s\002",
			"Usage: DEL <channel or *> <nick or userhost>");
		return;
	}
	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
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
	if ((tmp=find_user(&Userlist, uh, chan)) == NULL)
	{
		send_to_user(from,
			"\002%s is not in my users list for that channel\002",
			uh);
		return;
	}
	if (!matches(owneruserhost, uh) || !matches(uh, owneruserhost))
	{
		send_to_user(from,
			"\002Sorry, you may not delete the owner\002");
		return;
	}
	if (get_userlevel(from, chan) < tmp->access)
	{
		send_to_user(from,
			"\002%s has a higher access level than you on %s\002",
			uh, chan);
		return;
	}
#ifdef USE_CCC
        send_to_ccc("1 DEL %s %s %s", from, tmp->channel, tmp->userhost);
#endif
	remove_user(&Userlist, tmp);
	send_to_user(from, "\002User %s has been deleted\002", uh);
}

void do_aop(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *nuh;
	aUser *tmp;
	int ulevel;

	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	nick = get_token(&rest, " ");

	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!nick)
	{
		send_to_user(from, "\002%s\002", "Specify someone to auto-op");
		return;
	}
	if (!(nuh = nick2uh(from, nick, 0)))
		return;

	if ((tmp=find_user(&Userlist, nuh, chan)) == NULL)
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
	int ulevel;

	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	nick = get_token(&rest, " ");

	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!nick)
	{
		send_to_user(from, "\002%s\002", "Specify someone to not auto-op");
		return;
	}
	if (!(nuh = nick2uh(from, nick, 0)))
        	return;
	if ((tmp=find_user(&Userlist, nuh, chan)) == NULL)
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
		send_to_user(from, "\002Levels could not be read from %s\002",
			LEVELFILE);
	else
		send_to_user(from, "\002Levels were read from %s\002",
			LEVELFILE);
}

void do_savelists(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
        if (!saveuserlist(&Userlist, USERFILE))
        send_to_user( from, "\002Lists could not be saved to file %s\002",
                USERFILE);
        else
                send_to_user( from, "\002Lists saved to file %s\002",
                        USERFILE);
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
	if (!(readuserlist(USERFILE)))
	send_to_user( from, "\002Lists could not be read from file %s\002",
		USERFILE);
	else
		send_to_user( from, "\002Lists read from file %s\002",
			USERFILE);
}

void do_ban(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *nuh;
	int ulevel;

	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);

	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
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
	int ulevel;

	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
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
	int ulevel;

	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
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
	int ulevel;

	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
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
	int ulevel;

	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
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
	  sendkick(chan, currentnick, "\002Kick attempt of %s\002", nuh);
	  return;
	}

	sendkick(chan, nick, "%s", rest ? rest : "Requested Kick");
	send_to_user(from, "\002%s kicked from %s\002", nick, chan);
}

void do_showusers(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *pattern, *flag;
	int flags, ulevel;

	flag = (char *) 0;

	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
		return;
	}
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
	show_users_on_channel(from, chan, pattern, flags);
}

void do_massop(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *pattern;
	int ulevel;

	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
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
	int ulevel;

	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
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
	int ulevel;

	minutes = 0;
	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
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
	int ulevel;

	minutes = 0;
	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
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
	int ulevel;

	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
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
	int ulevel;

	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
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
	int ulevel;

	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
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
	int ulevel;
	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
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
	int ulevel;

	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
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
	int ulevel;

	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
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
		no_access(from, chan);
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
		send_to_user(from, "\002%s has a higher access level on %s than you\002", uh2, chan);
		return;
	}
	if (!matches(owneruserhost, uh2) || !matches(uh2, owneruserhost))
	{
		send_to_user(from, "\002%s\002", "Sorry, can't shit the owner");
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

	if (add_to_shitlist(&Userlist, uh, access, chan, whoshit,
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
		no_access(from, chan);
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
	if ((tmp=find_shit(&Userlist, uh, chan)) == NULL)
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
	remove_user(&Userlist, tmp);
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
	int oldlevel, level, ulevel;
	aUser *tmp;

	oldlevel = level = 0;
	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	nick = get_token(&rest, " ");

	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!nick)
	{
		send_to_user(from, "\002%s\002", "Specify a user to change");
		return;
	}
	if (!(nuh = nick2uh(from, nick, 0)))
		return;

	if ((tmp=find_user(&Userlist, nuh, chan)) == NULL)
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

	if ((level < 0) || (level >= OWNERLEVEL))
	{
		send_to_user(from, "\002Valid levels are 0 thru %i\002", OWNERLEVEL-1);
		return;
	}

	oldlevel = tmp->access;
	
	if (level > get_userlevel(from, chan))
	{
		send_to_user(from, "\002%s\002",
			"Can't change the level to higher than yours");
		return;
	}
	if (oldlevel > get_userlevel(from, chan))
	{
		send_to_user(from, "\002%s has a higher level than you\002", nuh);
		return;
	}

	if (!matches(owneruserhost, nuh))
	{
		send_to_user(from, "\002%s\002", "Sorry, can't change owner's level");
		return;
	}

	tmp->access = level;
	send_to_user(from, "\002Access level changed from %i to %i\002",
		oldlevel, level);
	send_to_user(nuh, "\002%s has changed your access level on %s to %i\002",
		currentnick, chan, level);
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
	int oldlevel, level, ulevel;
	aUser *tmp;

	oldlevel = level = 0;
	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	nick = get_token(&rest, " ");
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!nick)
	{
		send_to_user(from, "\002%s\002", "Specify someone to protect");
		return;
	}
	if  (!(nuh = nick2uh(from, nick, 0)))
        	return;

	if ((tmp = find_user(&Userlist, nuh, chan)) == NULL)
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
	int ulevel;

	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	nick = get_token(&rest, " ");
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!nick)
	{
		send_to_user(from, "\002%s\002", "Specify someone to not protect");
		return;
	}
	if (!(nuh = nick2uh(from, nick, 0)))
		return;

	if ((tmp = find_user(&Userlist, nuh, chan)) == NULL)
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
	int ulevel;

	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
		return;
	}
	nick = get_token(&rest, " ");
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

	if (!(nick = get_token(&rest, " ")))
	{
		send_to_user(from, "\002%s\002", "No nickname specified");
		return;
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
	int ulevel;

	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
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

void do_ping(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  if (ischannel(to))
	 sendprivmsg(to, "\002\002\002Pong\002");
  else
	 send_to_user(from, "\002\002\002Pong\002");
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
		no_access(from, chan);
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
	  no_access(from, chan);
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
	int ulevel;

	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
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
		no_access(from, chan);
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
		no_access(from, chan);
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
		no_access(from, chan);
		return;
	}

	send_to_user(from, "\002--- Spylist for %s\002", chan);
	show_spy_list(from, chan);
	send_to_user(from, "\002%s\002", "--- end of list ---");
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
	show_userlist(from, Userlist);
}

void do_shitlist(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	show_shitlist(from, Userlist);
}

void do_greetlist(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
        register aUser *dummy;
        send_to_user(from, "\002%s\002",
                "------ Userhost ------- Greeting -----");
        for (dummy = Userlist; dummy; dummy = dummy->next )
		if (IsGreet(dummy))
	                send_to_user( from, " %15s     %s",
       	                 dummy->userhost, dummy->reason);
        send_to_user(from, "\002--- end of greetlist ---\002");
}

void do_kicksaylist(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
        register aUser *dummy;
        send_to_user(from, "\002%s\002",
                "--- Channel ------- String -----");
        for (dummy = Userlist; dummy; dummy = dummy->next )
                if (IsKickSay(dummy))
                        send_to_user(from, "%15s %s",
                        	dummy->channel, dummy->userhost);
        send_to_user(from, "\002--- end of kicksaylist ---\002");
}

void do_saysaylist(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
        register aUser *dummy;
        send_to_user(from, "\002%s\002",
                "--- Channel ------- String ---------- What to say ---------");
        for (dummy = Userlist; dummy; dummy = dummy->next )
                if (IsSaySay(dummy))
                        send_to_user(from, "%15s %s     %s",
                        	dummy->channel, dummy->userhost,
				dummy->reason);
        send_to_user(from, "\002--- end of saysaylist ---\002");
}

void do_nicklist(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
       register aUser *dummy;
       send_to_user(from,"\002------ Userhost --- Level ---- Reason -----\002");
       for (dummy = Userlist; dummy; dummy = dummy->next )
		if (IsProtNick(dummy))
	               send_to_user( from, " %15s     %3d    %s",
                       dummy->userhost, dummy->access, dummy->reason);
       send_to_user(from, "\002--- end of nicklist ---\002");
}

void do_banlist(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	int ulevel;  

	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
		return;
	}
	send_to_user(from, "\002Banlist on %s:\002", chan);
	show_banlist(from, chan);
	send_to_user(from, "\002--- end of list ---\002");
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
	if ((tmp = find_user(&Userlist, from, NULL)) == NULL)
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
	if (change_password(&Userlist, from, newpass) &&
		saveuserlist(&Userlist, USERFILE))
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
   if (change_password(&Userlist, nuh, newpass) &&
		saveuserlist(&Userlist, USERFILE))
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
	int ulevel;
  
	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
		return;
	}
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
	int oldlimit, limit, ulevel;
  
	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
		return;
	}
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
	int ulevel;
  
	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
		return;
	}
	chan_stats(from, chan);
}

void do_exec(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char line[1024], small[3], c;
	char *command, *where;
	FILE *bleah;
	int numlines;

	numlines = 0;
	if (!(where=get_token(&rest, " ")) || !*where || !rest || !*rest)
	{
		send_to_user(from, "\002Usage: exec <where> <command>\002");
		return;
	}
	command=get_token(&rest, " ");
	if (my_stricmp(command, "uptime"))
	{
		send_to_user(from, "\002That command not allowed\002");
		return;
	}
	bleah = popen(command, "r");
	if (!bleah)
	{
		send_to_user(from, "\002Problem executing the command\002");
		return;
	}
	strcpy(line, "");
	small[1] = '\0';
	while (!feof(bleah))
	{
		fread(&c, 1, 1, bleah);
		if (c == 10)
		{
			numlines++;
			sendprivmsg(where, "%s", line);
			strcpy(line, "");
			if (numlines == 7)
			{
				sendprivmsg(where, "\002[-- Ending exec (possible excess flood) --]\002");
				pclose(bleah);
				return;
			}
		}
		else
		{
			*small = c;
			strcat(line, small);
		}
	}
	if (!line)
		sendprivmsg(where, "%s", line);
	pclose(bleah);
}

void do_help(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  char lin[MAXLEN];
  FILE *in;

  if (!rest || !*rest)
    {
      send_to_user(from, "\002Usage: help <topics or command_name>\002");
      return;
    }

  if (!(in = fopen(HELPFILE, "rt")))
    {
      send_to_user(from, "\002Couldn't open the helpfile (%s)\002",
		   HELPFILE);
      return;
    }
  while (find_topic(in, lin) && !feof(in))
    if (!my_stricmp((char *) lin+1, rest))
      {
	int level;
	
	if (strchr(rest, ' '))
	  {
	    int num;
	    char buffer[MAXLEN];
	    strcpy(buffer, "");
	    num = 0;
	    while (freadln(in, lin) && !feof(in) && my_stricmp(lin, ":endtopic"))
	      if (lin && (*lin == ':'))
		{
		  num++;
		  if (num != 1)
		    strcat(buffer, " ");
		  strcat(buffer, (char *) lin+1);
		  if (num == 4)
		    {
		      send_to_user(from, "%s", buffer);
		      num = 0;
		      strcpy(buffer, "");
		    }
		}
	    if (num)
	      send_to_user(from, "%s", buffer);
	    return;
	  }
	send_to_user(from, "HELP on %s", rest);
	level = level_needed(rest);
	if (level >= 0)
	  send_to_user(from, "Level needed: %i", level);
	while (freadln(in, lin) && lin && (*lin != ':') && !feof(in))
	  send_to_user(from, "%s", lin);
	return;
      }
  return;    /* don't reply */
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
        if (!strncmp(Buf, "Auth", 4))
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
                no_access(from, chan);
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
        if ((tmp=find_shit(&Userlist, uh, chan)) == NULL)
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
                no_access(from, chan);
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
	if ((tmp=find_shit(&Userlist, uh, chan)) == NULL)
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
	if ((tmp=find_protnick(&Userlist, thenick)) != NULL)
	{
		send_to_user(from, "\002That nick is already being protected\002");
		return;
	}
	tmp = make_user(&Userlist);
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
	if ((tmp=find_protnick(&Userlist, thenick)) != NULL)
	{
		remove_user(&Userlist, tmp);
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
	int ulevel;

	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	if (ulevel < cmdlevel)
	{
		no_access(from, chan);
		return;
	}
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
                no_access(from, chan);
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

void do_fuck(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *nuh;
	char *temp;
	char bigbuffer[HUGE];
	int ulevel;

	chan = get_channel_and_userlevel(to, &rest, from, &ulevel);
	strcpy(bigbuffer, "fuck ");
	if (rest)
		strcat(bigbuffer, rest);
	if (ulevel < cmdlevel)
	{
		on_public(from, to, bigbuffer);
                return;
	}
	if (!(nick = get_token(&rest, " ")))
	{
		on_public(from, to, bigbuffer);
                return;
	}
	if (!my_stricmp(nick, "you"))
		if (!(nick = get_token(&rest, " ")))
		{
			on_public(from, to, bigbuffer);
			return;
		}
	temp = right(nick, 1);
	if (temp && (strchr(",;", *temp)))
		*temp = '\0';
        if (!(nuh = nick2uh(NULL, nick, 0)))
          return;

        if (get_userlevel(nuh, chan) > get_userlevel(from, chan))
        {
          sendkick(chan, currentnick, "\002FUCK attempt of %s\002", nuh);
          return;
        }

        deop_ban(chan, nick, nuh);
        sendkick(chan, nick, (rest && *rest) ? "\002%s\002" :
		"\002FUCK requested by: %s\002", (rest && *rest) ? rest :
				currentnick);
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
	tmp = find_greet(&Userlist, uh);
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
		tmp=make_user(&Userlist);
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
        tmp = find_greet(&Userlist, uh);
        if (!tmp)
                send_to_user(from, "\002I'm already not greeting %s\002",
                        uh);
        else
        {
		remove_user(&Userlist, tmp);
                send_to_user(from, "\002I'm no longer greeting %s\002", uh);
	}
}

void do_kicksay(from, to, rest, cmdlevel)
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

	tmp = find_kicksay(&Userlist, rest, chan);
	if (tmp)
	{
		send_to_user(from, "\002I'm already kicking on \"%s\"\002",
			tmp->userhost);
		return;
	}
        tmp=make_user(&Userlist);
        tmp->type |= USERLIST_KICKSAY;
	if (*rest != '*')
		mstrcpy(&tmp->userhost, "*");
        mstrcat(&tmp->userhost, rest);
	if (*(rest+strlen(rest)-1) != '*')
		mstrcat(&tmp->userhost, "*");
        mstrcpy(&tmp->whoadd, from);
        mstrcpy(&tmp->channel, chan);
	send_to_user(from, "\002Now kicking on the say of \"%s\" on %s\002",
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

        tmp = find_kicksay(&Userlist, rest, chan);
        if (!tmp)
        {
                send_to_user(from, "\002I'm not kicking on that\002");
                return;
        }
	send_to_user(from, "\002No longer kicking on the say of %s\002",
		tmp->userhost);
	remove_user(&Userlist, tmp);
}

void saysay_usage(to)
char *to;
{
	send_to_user(to, "\002Usage: ss <channel or *> <\"String to respond to\"> <\"What to say\">\002");
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
        tmp = find_saysay(&Userlist, string, chan);
        if (tmp)
        {
                send_to_user(from, "\002I'm already talking on \"%s\"\002",
                        tmp->userhost);
                return;
        }
        tmp=make_user(&Userlist);
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
        tmp = find_saysay(&Userlist, rest, chan);
        if (!tmp)
        {
                send_to_user(from, "\002I'm not responding to that\002");
                return;
        }
	send_to_user(from, "\002No longer responding to %s\002",
		tmp->userhost);
	remove_user(&Userlist, tmp);
}

