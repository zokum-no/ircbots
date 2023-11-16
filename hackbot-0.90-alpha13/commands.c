/*
 * commands.c - all the commands...
 * (c) 1994 CoMSTuD (cbehrens@iastate.edu)
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
#ifndef MSDOS
	#include <strings.h>
#endif
#include <time.h>
#include <signal.h>
#include "config.h"
#include "fnmatch.h"
#include "hackbot.h"
#include "channel.h"
#include "userlist.h"
#include "dcc.h"
/*
#include "ftext.h"
*/
#include "function.h"
#include "commands.h"

extern botinfo *current;
extern char owneruserhost[MAXLEN];
extern int number_of_bots;

void do_access(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nuh;

	chan = find_channel(to, &rest);
	nuh = from;
	if (nuh = get_token(&rest, " "))
	  nuh = find_userhost(from, nuh);
	else
	  nuh = from;
	if (!nuh)
	  return;
	send_to_user(from, "\002Access for %s\002", nuh);
	send_to_user(from, "\002Channel: %s  Access: %i\002", chan, get_userlevel(nuh, chan));
	if (current->restrict)
	  send_to_user(from, "\002Note: I'm being restricted to level %i\002",
				 current->restrict);
}

void do_auth(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  char *pass;
  
  pass = get_token(&rest, " ");

  if (!pass || !*pass)
	 send_to_user(from, "\002No password given, not verified\002");
  else if (correct_password(from, pass))
	 {
		send_to_user(from, "\002You have now been authorized\002");
		set_mempassword(from, 1);
	 }
  else
	 send_to_user(from, "\002Incorrect password, not authorized\002");
}

void do_enfmodes(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char newmodes[20];
	char *chan, *newptr;

	newptr = newmodes;
	chan = find_channel(to, &rest);

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!rest || !*rest)
	{
		set_enfmodes(chan, NULL);
		send_to_user(from, "\002No longer enforcing modes on %s\002", chan);
		return;
	}
	rest[19] = '\0';
	for (; *rest; rest++)
	  if (strchr(ENFORCED_MODES, *rest))
		 *newptr++ = *rest;
	*newptr = '\0';
	if (!set_enfmodes(chan, newmodes))
		send_to_user(from, "\002I'm not on %s\002", chan);
	else
		send_to_user(from, "\002Now enforcing \"%s\" on %s\002",
			newmodes, chan);
}

void do_stats(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  char *chan, *nuh;

  chan = find_channel(to, &rest);
  nuh = from;

  if (nuh = get_token(&rest, " "))
	 nuh = find_userhost(from, nuh);
  else
	 nuh = from;
  if (!nuh)
	 return;

  send_to_user( from, "Stats for %s on %s", nuh, chan);
  send_to_user( from, "--------------------------");
  send_to_user( from, "      User level: %i", get_userlevel(nuh, chan));
  send_to_user( from, " Protected level: %i%s", get_protlevel(nuh, chan),
	 get_prottoggle(chan)? "" : "   \002(Protection toggled off)\002");
  send_to_user( from, "      Auto Opped: %s%s", is_aop(nuh, chan) ? "Yes" : "No",
	 get_aoptoggle(chan)? "" : "   \002(Auto-op is toggled off)\002");
  send_to_user( from, "Shitlisted level: %i", get_shitlevel(nuh, chan));
  return;
}

void do_version(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  send_to_user(from, "\002HackBot Version %s (C) 1994 CoMSTuD, Released %s\002",
		  VERSION, BOTDATE);
  send_to_user(from, "\002%s\002", "Written by: CoMSTuD!cbehrens@iastate.edu");
  return;
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
	return;
}

void do_getch(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  send_to_user( from, "\002Current channel: %s\002", currentchannel());
  return;
}

void do_opme(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;

	chan = find_channel(to, &rest);

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}
	if (usermode(chan, getnick(from)) & MODE_CHANOP)
		send_to_user(from, "\002You're already channel operator on %s\002",
			chan);
	else
		giveop(chan, getnick(from));
}

void do_deopme(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;

	chan = find_channel(to, &rest);

	if (usermode(chan, getnick(from)) & MODE_CHANOP)
		takeop(chan, getnick(from));
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

	chan = find_channel(to, &rest);

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (rest)
		mode3(chan, "+ooo", rest);
	else
		send_to_user( from, "\002%s\002", "Who do you want me to op, you schmuck?");
	return;
}

void do_deop(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;

	chan = find_channel(to, &rest);

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (rest)
		mode3(chan, "-ooo", rest);
	else
		send_to_user( from, "\002%s\002", "Who do you want me to op, you schmuck?");
	return;
}

void do_invite(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick;

	chan = find_channel(to, &rest);
	nick = getnick(from);

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (rest && *rest)
	  nick = get_token(&rest, " ");

	if (!invite_to_channel(nick, chan))
	  send_to_user(from, "\002I'm not on channel %s, you dickhead\002", chan);
	else
	  send_to_user(from, "\002%s invited to %s\002", nick, chan);
}
			
void do_chat(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	dcc_chat(from, rest);
	sendnotice(getnick(from), "\002Please type: /dcc chat %s\002", current->nick);
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

	chan = find_channel(to, &rest);

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!rest)
	{
		send_to_user( from, "\002%s\002", "What am I suppposed to say, dilweed?");
		return;
	}
	sendprivmsg(chan, "\002\002%s", rest);
	return;
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
	return;
}

void do_me(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	char text[HUGE];

	strcpy(text, "ACTION \002\002");
	chan = find_channel(to, &rest);

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!rest)
	{
		send_to_user( from, "\002%s\002", "What am I suppposed to action, dilweed?");
		return;
	}
	strcat(text, rest);

	send_ctcp(chan, "%s", text);
	return;
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
		send_to_user(from, "\002%s\002", "No nick given");
		return;
	}
	nick = get_token(&rest, " ");

	if (!rest || !*rest)
	{
		 send_to_user( from, "\002%s\002", "What's the message?");
		 return;
	}

	if (ischannel(nick))
	{
		send_to_user(from, "\002Use \"%csay\" instead\002", current->cmdchar);
		return;
	}

	if (!my_stricmp(nick,current->nick))
	{
		 send_to_user(from, "\002%s\002", "Nice try, but I don't think so");
		 return;
	}

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
  return;
}

void do_channels(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  show_channellist(from);
  return;
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
		send_to_user(from, "\002%s\002", "No channel name specified");
		return;
	}
	if (!ischannel(chan))
	{
		send_to_user(from, "\002%s\002", "Invalid channel name");
		return;
	}

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	send_to_user(from, "\002Attempting the join of %s\002", chan);
	join_channel(chan, rest ? rest : "", TRUE);
}

void do_leave(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  char *chan;

  chan = find_channel(to, &rest);

  if (get_userlevel(from, chan) < cmdlevel)
  {
	  no_access(from, chan);
	  return;
  }

  if (is_in_list(current->StealList, chan))
	 send_to_user(from, "\002Sorry, I'm trying to steal %s\002", chan);
  else
	 leave_channel(chan);
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
		exit(0);
}

void do_add(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  char *chan, *nick, *level, *aopa, *prota, *uh, *pass;
  int access, aop, prot;
  
  chan = get_token(&rest, " ");
  if (!chan || !*chan || !ischannel(chan) && my_stricmp(chan, "*"))
	 {
		send_to_user(from, "\002%s\002",
			"Usage: ADD <channel or *> <nick or userhost> <level> [aop] [protlevel]");
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
  if (!(aopa = get_token(&rest, " ")))
	 aopa = "0";
  if (!(prota = get_token(&rest, " ")))
	 prota = "0";
  pass = get_token(&rest, " ");
  uh = nick2uh(nick, 1);
  if (!uh)
	 return;
  
  if (is_user(uh, chan))
	 {
		send_to_user(from, "\002%s is in my userlist already for this channel\002", uh);
		return;
	 }
  access = atoi(level);
  if ((access < 0) || (access >= OWNERLEVEL))
	 {
		send_to_user(from, "\002Valid levels are from 0 thru %i\002", OWNERLEVEL-1);
		return;
	 }
  if (access > get_userlevel(from, chan))
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
  prot = (prot < 0) ? 0 : (prot > MAXPROTLEVEL) ? MAXPROTLEVEL : prot;
  if (add_to_userlist(current->UserList, uh, access, aop ? 1 : 0, prot,
				chan, pass))
		{
	send_to_user( from, "\002The user has been added as %s on %s\002", uh, chan);
	send_to_user( from, "\002Access level: %i  Auto-op: %s  Protect level: %i  %s %s\002",
			  access, aop ? "Yes" : "No", prot,
			  (pass && *pass) ? "Password:" : "",
			  (pass && *pass) ? pass : "");
	sendnotice(nick,
			"\002%s has given you level %i clearance\002", getnick(from), access);
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
	 send_to_user( from, "\002%s\002", "Problem adding the user");
	return;
}                   

void do_del(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  char *chan, *nick, *uh;
  
  chan = get_token(&rest, " ");
  
  if (!chan || !*chan || !ischannel(chan) && my_stricmp(chan, "*"))
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
		send_to_user(from, "\002%s\002", "No nick or userhost specified");
		return;
	 }
  uh = nick;
  if (!strchr(nick, '!') && !strchr(nick, '@'))
	 uh = find_userhost(from, nick);
  if (!uh)
	 return;

  if (!is_user(uh, chan))
	 {
		send_to_user(from, "\002%s is not in my users list for that channel\002", uh);
		return;
	 }

  if (!matches(owneruserhost, uh))
	 {
		send_to_user(from, "\002%s\002", "Sorry, you may not delete the owner");
		return;
	 }
  if (get_userlevel(from, chan) < get_userlevel(uh, chan))
	 {
		send_to_user(from, "\002%s has a higher access level than you on %s\002", uh, chan);
		return;
	 }
  if (!remove_from_userlist(current->UserList, uh, chan))
	 send_to_user(from, "\002Problem deleting %s\002", uh);
  else
	 {
		printf("NMADEAIT");
		send_to_user(from, "\002User %s has been deleted\002", uh);
		send_to_user(uh, "\002Your access on %s has been deleted by %s\002",
			chan, getnick(from));
	 }
}

void do_aop(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *nuh;

	chan = find_channel(to, &rest);
	nick = get_token(&rest, " ");

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!nick)
	{
		send_to_user(from, "\002%s\002", "Specify someone to auto-op");
		return;
	}
	nuh = find_userhost(from, nick);
	if (!nuh)
		return;

	if (!is_user(nuh, chan))
	{
		send_to_user( from, "\002%s is not in my users list on %s\002", nuh, chan);
		return;
	}
	if (is_aop(nuh, chan))
	{
		send_to_user( from, "\002%s is already being auto-opped on %s\002", nuh, chan);
		return;
	}
	if (change_userlist(nuh, chan, -1, 1, -1))
		send_to_user(from, "\002%s is now being auto-opped on %s\002", nuh, chan);
	else
		send_to_user(from, "\002%s\002", "Problem updating the userlist");
	return;
}

void do_raop(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *nuh;

	chan = find_channel(to, &rest);
	nick = get_token(&rest, " ");

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!nick)
	{
		send_to_user(from, "\002%s\002", "Specify someone to not auto-op");
		return;
	}
	nuh = find_userhost(from, nick);
	if (!nuh)
		return;

	if (!is_user(nuh, chan))
	{
		send_to_user( from, "\002%s is not in my users list on %s\002", nuh, chan);
		return;
	}
	if (!is_aop(nuh, chan))
	{
		send_to_user( from, "\002%s is already not being auto-opped on %s\002", nuh, chan);
		return;
	}
	if (change_userlist(nuh, chan, -1, 0, -1))
		send_to_user(from, "\002%s is no longer being auto-opped on %s\002", nuh, chan);
	else
		send_to_user(from, "\002%s\002", "Problem updating the userlist");
	return;
}

void do_saveusers(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	if (!write_userlist(current->UserList, current->usersfile))
		send_to_user( from, "\002Userlist could not be written to file %s\002", current->usersfile);
	else
		send_to_user( from, "\002Userlist written to file %s\002", current->usersfile);
}

void do_saveshit(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	if (!write_shitlist(current->ShitList, current->shitfile))
		send_to_user( from, "\002Shitlist could not be written to file %s\002", current->shitfile);
	else
		send_to_user( from, "\002Shitlist written to file %s\002", current->shitfile);
}

void do_loadusers(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	if (!read_userlist(current->UserList, current->usersfile))
		send_to_user( from, "\002Userlist could not be read from file %s\002", current->usersfile);
	else
		send_to_user( from, "\002Userlist read from file %s\002", current->usersfile);
}

void do_loadshit(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	if (!read_shitlist(current->ShitList, current->shitfile))
		send_to_user( from, "\002Shitlist could not be read from file %s\002", current->shitfile);
	else
		send_to_user( from, "\002Shitlist read from file %s\002", current->shitfile);
}

void do_ban(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *nuh;

	chan = find_channel(to, &rest);

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}
		
	if (!(nick = get_token(&rest, " ")))
	{
		send_to_user(from, "\002%s\002", "No nick specified");
		return;
	}
	if (!(nuh = nick2uh(nick, 0)))
	  return;

	if (get_userlevel(nuh, chan) > get_userlevel(from, chan))
	{
	  sendkick(chan, getnick(from), "\002%s\002", "buwahahaha, nice try");
	  return;
	}
	deop_ban(chan, nick);
	send_to_user(from, "\002%s banned on %s\002", nick, chan);
}
		
void do_siteban(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *nuh;

	chan = find_channel(to, &rest);

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!(nick = get_token(&rest, " ")))
	{
		send_to_user(from, "\002%s\002", "No nick specified");
		return;
	}
	if (!(nuh = nick2uh(nick, 0)))
	  return;

	if (get_userlevel(nuh, chan) > get_userlevel(from, chan))
	{
	  sendkick(chan, getnick(from), "\002%s\002", "buwahahaha, nice try");
	  return;
	}

	deop_siteban(chan, nick);
	send_to_user(from, "\002%s sitebanned on %s\002", nick, chan);
}
		
void do_kickban(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *nuh;

	chan = find_channel(to, &rest);

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!(nick = get_token(&rest, " ")))
	{
		send_to_user(from, "\002%s\002", "No nick specified");
		return;
	}
	if (!(nuh = nick2uh(nick, 0)))
	  return;

	if (get_userlevel(nuh, chan) > get_userlevel(from, chan))
	{
	  sendkick(chan, getnick(from), "\002%s\002", "buwahahaha, nice try");
	  return;
	} 

	deop_ban(chan, nick);
	sendkick(chan, nick, rest);
	send_to_user(from, "\002%s kickbanned on %s\002", nick, chan);
}

void do_sitekickban(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *nuh;

	chan = find_channel(to, &rest);

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!(nick = get_token(&rest, " ")))
	{
		send_to_user(from, "\002%s\002", "No nick specified");
		return;
	}
	if (!(nuh = nick2uh(nick, 0)))
	  return;

	if (get_userlevel(nuh, chan) > get_userlevel(from, chan))
	{
	  sendkick(chan, getnick(from), "\002%s\002", "buwahahaha, nice try");
	  return;
	}

	deop_siteban(chan, nick);
	sendkick(chan, nick, rest);
	send_to_user(from, "\002%s sitekickbanned on %s\002", nick, chan);
}

void do_kick(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *nuh;

	chan = find_channel(to, &rest);

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!(nick = get_token(&rest, " ")))
	{
		send_to_user(from, "\002%s\002", "No nick specified");
		return;
	}
	if (!(nuh = nick2uh(nick, 0)))
	  return;
	if (get_userlevel(nuh, chan) > get_userlevel(from, chan))
	{
	  sendkick(chan, getnick(from), "\002%s\002", "buwahahaha, nice try");
	  return;
	}

	sendkick(chan, nick, rest);
	send_to_user(from, "\002%s kicked from %s\002", nick, chan);
}

void do_showusers(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  char *chan;

  chan = find_channel(to, &rest);

  show_users_on_channel(from, chan);
}

void do_massop(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *pattern;

	chan = find_channel(to, &rest);
	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}
	pattern = get_token(&rest, " ");
	if (!pattern)
		pattern = "*!*@*";

	if (i_am_op(chan))
	{
		channel_massop(chan, pattern);
		send_to_user(from, "\002%s massopped on %s\002", pattern, chan);
	}
	else
		not_opped(from, chan);
	return;
}

void do_massdeop(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *pattern;

	chan = find_channel(to, &rest);
	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}
	pattern = get_token(&rest, " ");
	if (!pattern)
		pattern = "*!*@*";

	if (i_am_op(chan))
	{
		channel_massdeop(chan, pattern);
		send_to_user(from, "\002%s massdeopped on %s\002", pattern, chan);
	}
	else
		not_opped(from, chan);
	return;
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
	chan = find_channel(to, &rest);
	if (get_userlevel(from, chan) < cmdlevel)
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
	channel_massunban(chan, pattern, minutes*60);
	return;
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
	chan = find_channel(to, &rest);
	if (get_userlevel(from, chan) < cmdlevel)
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
	channel_massunbanfrom(chan, pattern, minutes*60);
	return;
}

void do_masskick(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *pattern;

	chan = find_channel(to, &rest);
	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}
	pattern = get_token(&rest, " ");
	if (!pattern)
		pattern = "*!*@*";

	if (i_am_op(chan))
	{
		channel_masskick(chan, pattern);
		send_to_user(from, "\002%s masskicked on %s\002", pattern, chan);
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

	chan = find_channel(to, &rest);
	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}
	pattern = get_token(&rest, " ");
	if (!pattern)
		pattern = "*!*@*";

	if (i_am_op(chan))
	{
		channel_masskickban(chan, pattern);
		send_to_user(from, "\002%s masskickbanned on %s\002", pattern, chan);
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

	chan = find_channel(to, &rest);
	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (i_am_op(chan))
	{
		channel_massop(chan, "");
		send_to_user(from, "\002Users massopped on %s\002", chan);
	}
	else
		not_opped(from, chan);
	return;
}

void do_massdeopnu(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;

	chan = find_channel(to, &rest);
	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (i_am_op(chan))
	{
		channel_massdeop(chan, "");
		send_to_user(from, "\002Non-users massdeopped on %s\002", chan);
	}
	else
		not_opped(from, chan);
	return;
}

void do_masskicknu(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;

	chan = find_channel(to, &rest);
	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}
	if (i_am_op(chan))
	{
		channel_masskick(chan, "");
		send_to_user(from, "\002Non-users masskicked on %s\002", chan);
	}
	else
		not_opped(from, chan);
	return;
}

void do_shit(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *level, *aop, *prot, *uh;
	int access;

	chan = get_token(&rest, " ");
	
	if (!chan || !*chan || !ischannel(chan) && my_stricmp(chan, "*"))
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
	if (!rest || !*rest)
	  {
		 send_to_user(from, "\002%s\002", "No reason specified");
		 return;
	  }
	uh = nick2uh(nick, 0);
	if (!uh)
	  {
		 send_to_user(from, "No info found");
		  return;
	  }
	if (is_shitted(uh, chan))
	{
		send_to_user(from, "\002%s is in my shitlist already for this channel\002", uh);
		return;
	}
	if (get_userlevel(uh, chan) > get_userlevel(from, chan))
	{
		send_to_user(from, "\002%s has a higher access level on %s than you\002", uh, chan);
		return;
	}
	if (!matches(owneruserhost, uh))
	{
		send_to_user(from, "\002%s\002", "Sorry, can't shit the owner");
		return;
	}
	uh = nick2uh(nick, 1);
	if (!my_stricmp("*!*@*", uh))
	  {
		 send_to_user(from, "\002Problem shitting *!*@*, try again\002");
		 return;
	  }
	access = atoi(level);
	if ((access < 0) || (access > MAXSHITLEVEL))
	{
		send_to_user(from, "\002Valid levels are from 0 thru %i\002", MAXSHITLEVEL);
		return;
	}
	if (add_to_shitlist(current->ShitList, uh, access, chan, from, (rest && *rest) ? rest : "" , getthetime()))
	{
		send_to_user( from, "\002The user has been shitted as %s on %s\002", uh, chan);
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

	chan = get_token(&rest, " ");
	
	if (!chan || !*chan || !ischannel(chan) && my_stricmp(chan, "*"))
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
		send_to_user(from, "\002%s\002", "No nick or userhost specified");
		return;
	}
	uh = nick;
	if (!strchr(nick, '!') && !strchr(nick, '@'))
		uh = find_userhost(from, nick);
	if (!uh)
		return;

	if (!is_shitted(uh, chan))
	{
		send_to_user(from, "\002%s is not in my shit list on that channel\002", uh);
		return;
	}

	if (!remove_from_shitlist(current->ShitList, uh, chan))
		send_to_user( from, "\002Problem deleting %s\002", uh);
	else
		send_to_user(from, "\002User %s is no longer being shitted on %s\002", uh, chan);
	return;
}

void do_clvl(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *nuh, *bleah;
	int oldlevel, level;

	oldlevel = level = 0;
	chan = find_channel(to, &rest);
	nick = get_token(&rest, " ");

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!nick)
	{
		send_to_user(from, "\002%s\002", "Specify a user to change");
		return;
	}
	nuh = find_userhost(from, nick);
	if (!nuh)
		return;

	if (!is_user(nuh, chan))
	{
		send_to_user( from, "\002%s is not in my users list on %s\002", nuh, chan);
		return;
	}

	bleah = get_token(&rest, " ");
	if (bleah)
		level = atoi(bleah);

	if ((level < 0) || (level >= OWNERLEVEL))
	{
		send_to_user(from, "\002Valid levels are 0 thru %i\002", OWNERLEVEL);
		return;
	}

	oldlevel = get_userlevel(nuh, chan);

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

	if (!change_userlist(nuh, chan, level, -1, -1))
		send_to_user(from, "\002%s\002", "Problem updating the userlist");
	else
	{
		send_to_user( from, "\002Access level changed from %i to %i\002",
			oldlevel, level);
		send_to_user(nuh, "\002%s has changed your access level on %s to %i\x2", getnick(from), chan, level);
	}
	return;
}

void do_prot(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *nuh, *prot;
	int oldlevel, level;

	oldlevel = level = 0;
	chan = find_channel(to, &rest);
	nick = get_token(&rest, " ");

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!nick)
	{
		send_to_user(from, "\002%s\002", "Specify someone to protect");
		return;
	}
	nuh = find_userhost(from, nick);
	if (!nuh)
		return;

	if (!is_user(nuh, chan))
	{
		send_to_user( from, "\002%s is not in my users list on %s\002", nuh, chan);
		return;
	}

	prot = get_token(&rest, " ");
	if (prot)
		level = atoi(prot);

	if (!level || (level > MAXPROTLEVEL))
	{
		send_to_user(from, "\002Valid protect levels are 1 thru %i\002",
			MAXPROTLEVEL);
		return;
	}

	oldlevel = get_protlevel(nuh, chan);

	if (!change_userlist(nuh, chan, -1, -1, level))
		send_to_user(from, "\002%s\002", "Problem updating the userlist");
	else if (oldlevel)
		send_to_user( from, "\002Protect level changed from %i to %i\002",
			oldlevel, level);
	else
		send_to_user( from, "\002User now protected with level %i\002",
			level);
	return;
}

void do_rprot(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *nuh;

	chan = find_channel(to, &rest);
	nick = get_token(&rest, " ");

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!nick)
	{
		send_to_user(from, "\002%s\002", "Specify someone to not protect");
		return;
	}
	nuh = find_userhost(from, nick);
	if (!nuh)
		return;

	if (!is_user(nuh, chan))
	{
		send_to_user( from, "\002%s is not in my users list on %s\002", nuh, chan);
		return;
	}
	if (!get_protlevel(nuh, chan))
	{
		send_to_user( from, "\002%s is already not being protected on %s\002", nuh, chan);
		return;
	}
	if (change_userlist(nuh, chan, -1, -1, 0))
		send_to_user(from, "\002%s is no longer being protected on %s\002", nuh, chan);
	else
		send_to_user(from, "\002%s\002", "Problem updating the userlist");
	return;
}

void do_unban(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *nuh;

	chan = find_channel(to, &rest);

	if (get_userlevel(from, chan) < cmdlevel)
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
	nuh = find_userhost(from, nick);
	if (!nuh)
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

	chan = find_channel(to, &rest);

	if (get_userlevel(from, chan) < cmdlevel)
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
	if (is_in_list(current->SpyMsgList, from))
	{
		send_to_user(from, "\002%s\002", "Already redirecting messages to you");
		return;
	}
	if (add_to_list(current->SpyMsgList, from))
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

	strcpy(buffer, getnick(from));
	if (!is_in_list2(current->SpyMsgList, buffer))
	{
		send_to_user(from, "\002%s\002", "I'm not redirecting messages to you");
		return;
	}
	if (remove_from_list2(current->SpyMsgList, buffer))
		send_to_user( from, "\002%s\002", "No longer redirecting messages to you");
	else
		send_to_user(from, "\002%s\002", "Can't stop redirecting you the messages");
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
	int i;
/*
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
	char *server, *login, *ircname;

	server = get_token(&rest, " ");
	if (!server)
	{
		send_to_user(from, "\002%s\002", "Usage: server <servername> [login] [ircname]");
		return;
	}
	login = get_token(&rest, " ");
	ircname = rest;
	if (login && *login)
		  strcpy(current->login, login);
	if (ircname && *ircname)
		 strcpy(current->ircname, ircname);
	send_to_user(from, "\002Trying new server: %s\002", server);
	sendquit("brb");

	if (!add_server(server) || !set_server(server))
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
	LS *temp;

	chan = get_token(&rest, " ");
	
	if (!chan || !*chan || !ischannel(chan))
	{
		send_to_user(from, "\002%s\002", "Trying to steal the following channels:");
		send_to_user(from, "\002%s\002", "---------------------------------------");
		for(temp=*current->StealList;temp;temp=temp->next)
			send_to_user(from, "\002%s\002", temp->name);
		send_to_user(from, "\002%s\002", "-------------- end of list ------------");
		return;
	}

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (is_in_list(current->StealList, chan))
	{
		send_to_user(from, "\002%s\002", "Already stealing that channel");
		return;
	}
	if (add_to_list(current->StealList, chan))
	{
		send_to_user(from, "\002Now trying to steal %s\002", chan);
		join_channel(chan, "", TRUE);
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
	
	if (!is_in_list(current->StealList, chan))
	{
		send_to_user( from, "\002I'm not trying to steal %s\002", chan);
		return;
	}
	if (remove_from_list(current->StealList, chan))
		send_to_user(from, "\002No longer stealing %s\002", chan);
	else
		send_to_user(from, "\002%s\002", "Problem deleting the channel");
	leave_channel(chan);
}

void do_cycle(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;

	chan = find_channel(to, &rest);

	if (get_userlevel(from, chan) < cmdlevel)
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
	LS *Temp;
	CHAN_list *Channel;
	
	chan = get_token(&rest, " ");
	strcpy(buffer, getnick(from));
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
	
	if (!(Channel = search_chan(chan)))
	{
		not_on(from, chan);
		return;
	}
	Temp = is_in_list2(Channel->SpyList, buffer);   
	if (!Temp)
	{
		send_to_user(from, "\002You are not spying on %s\002", chan);
		return;
	}
	if (get_userlevel(Temp->name, chan) > get_userlevel(from, chan))
	{
		send_to_user(from, "\002Can't remove someone with a higher access level than you\002");
		return;
	}
	if (remove_from_list(Channel->SpyList, Temp->name))
		send_to_user(from, "\002No longer spying on %s for you\002", chan);
	else
		send_to_user(from, "\002You are not spying on %s\002", chan);
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
	show_spy_list(from, rest);
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
  show_userlist(from, current->UserList);
}

void do_shitlist(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  show_shitlist(from, current->ShitList);
}

void do_banlist(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  char *chan;

  chan = find_channel(to, &rest);
  
  if (get_userlevel(from, chan) < cmdlevel)
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
  char *old, *new;

  old = get_token(&rest, " ");
  new = get_token(&rest, " ");

  if (!old || !new || !*old || !*new)
	 {
		send_to_user(from, "\002Usage: passwd <oldpasswd> <newpasswd>\002");
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
  if (change_password(current->UserList, from, new) &&
		write_userlist(current->UserList, current->usersfile))
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
  
  chan = find_channel(to, &rest);
  
  if (get_userlevel(from, chan) < cmdlevel)
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
  int oldlimit, limit;
  
  chan = find_channel(to, &rest);
  
  if (get_userlevel(from, chan) < cmdlevel)
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
  
  chan = find_channel(to, &rest);
  
  if (get_userlevel(from, chan) < cmdlevel)
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
	char *where;
	FILE *bleah;
	int numlines;

	numlines = 0;
	if (!(where=get_token(&rest, " ")) || !*where || !rest || !*rest)
	{
		send_to_user(from, "\002Usage: exec <where> <command>\002");
		return;
	}
	if (my_stristr(rest, "rm ") || my_stristr(rest, "cp ") || my_stristr(rest, "mv ") ||
		my_stristr(rest, "kill ") || strchr(rest, '>') || strchr(rest, '<') ||
		strchr(rest, '|'))
	{
		send_to_user(from, "\002Nice fucking try\002");
		return;
	}
	bleah = popen(rest, "r");
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







