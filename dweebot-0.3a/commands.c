/*
 * commands.c - all the commands...
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <strings.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include "config.h"
#include "fnmatch.h"
#include "dweeb.h"
#include "channel.h"
#include "userlist.h"
#include "dcc.h"
/*
#include "ftext.h"
*/
#include "function.h"
#include "commands.h"

extern	botinfo	*current;
extern 	char	owneruserhost[MAXLEN];
extern 	int	number_of_bots;

void do_access(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nuh;
	int level;

	chan = find_channel(to, &rest);
	write_cmd_log(from, chan, getnick(from), "ACCESS", rest);
	if (nuh = get_token(&rest, " "))
        {
		if (*nuh == '+')
                {
			nuh++; 
			level = level_needed(nuh);
			if (level < 0)
				send_to_user(from, "\002No such command\002");
			else
				send_to_user(from, "\002You gotta have at least a %i level of DweeB0sity\002", level);
                	return;
		}
		nuh = nick2uh(from, nuh, 0);
	}
	else
	  nuh = from;
	if (!nuh)
	  return;
	send_to_user(from, "\002DweeB0sity for %s\002", nuh);
	send_to_user(from, "\002Channel: %s  DweeB0sity: %i\002", chan, get_userlevel(nuh, chan));
	if (current->restrict)
	  send_to_user(from, "\002BTW: Someone is holding back to %i DweeB0sity\002",
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
        char *chan;
        
	chan = find_channel(to, &rest);
	write_cmd_log(from, chan, getnick(from), "AUTH", rest);
	pass = get_token(&rest, " ");
	pwn = password_needed(from);
	v = verified(from);

	if (pwn && v)
	{
		send_to_user(from, "\002Yer already a DweeB\002");
		return;
        }
	if (!pass || !*pass)
	{
                if (pwn)
			send_to_user(from, "\002No password, No Dweebieness\002");
        	return;
	}
	if (!pwn)
	{
		send_to_user(from, "\002Yer a Major DweeB, you don't need no password\002");
		return;
        }
  	if (correct_password(from, pass))
	{
		send_to_user(from, "\002Yer have now been DweeBiniZed\002");
	   	set_mempassword(from, 1);
	   	op_all_chan(getnick(from));
	}
  	else
	 	send_to_user(from, "\002Who the HELL do you think yer kidding?\002");
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

	write_cmd_log(from, chan, getnick(from), "ENFMODES", rest);
	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!rest || !*rest)
	{
		set_enfmodes(chan, NULL);
		send_to_user(from, "\002I've stopped pushing yer desires on%s\002", chan);
		return;
	}
	rest[19] = '\0';
	for (; *rest; rest++)
	  if (strchr(ENFORCED_MODES, *rest))
		 *newptr++ = *rest;
	*newptr = '\0';
	if (!set_enfmodes(chan, newmodes))
		send_to_user(from, "\002Do I look lime I'm on %s ?\002", chan);
	else
		send_to_user(from, "\002Now pushing \"%s\" on %s\002",
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
  
  write_cmd_log(from, chan, getnick(from), "STATS", rest);
  
  if (nuh = get_token(&rest, " "))
	 nuh = nick2uh(from, nuh, 0);
  else
	 nuh = from;
  if (!nuh)
	 return;

  send_to_user( from, "Stats for %s on %s", nuh, chan);
  send_to_user( from, "--------------------------");
  send_to_user( from, "     DweeB level: %i", get_userlevel(nuh, chan));
  send_to_user( from, " Protected level: %i%s", get_protlevel(nuh, chan),
	 get_prottoggle(chan)? "" : "   \002(Protection toggled off)\002");
  send_to_user( from, "      Auto DweeB: %s%s", is_aop(nuh, chan) ? "Yes" : "No",
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
  write_cmd_log(from, "*", to, "VERSION", rest);
  send_to_user(from, "\002I'll never tell\002");
  return;
}


void do_info(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	write_cmd_log(from, "*", to, "INFO", rest);
	do_version(from, to, rest, cmdlevel);
	send_to_user(from, "\002DweeB'd at: %-20.20s\002", time2str(current->uptime));
	send_to_user(from, "\00No Sleep for: %s\002", idle2str(getthetime()-current->uptime));
	return;
}

void do_getch(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  char *chan;
  chan = find_channel(to, &rest);
  write_cmd_log(from, chan, getnick(from), "GETCH", rest);
  send_to_user( from, "\002I'm hanging out on: %s\002", currentchannel());
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
	write_cmd_log(from, chan, getnick(from), "OPME", rest);
	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}
	if (usermode(chan, getnick(from)) & MODE_CHANOP)
		send_to_user(from, "\002You're already a SuPiR DweeB on %s\002",
			chan);
	else
		giveop(chan, getnick(from));
}


void do_op(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;

	chan = find_channel(to, &rest);
	write_cmd_log(from, chan, getnick(from), "OP", rest);
	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (rest)
		mode3(chan, "+ooo", rest);
	else
		send_to_user( from, "\002%s\002", "Who shall I DweeBinize?");
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
	write_cmd_log(from, chan, getnick(from), "DEOP", rest);
	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (rest)
		mode3(chan, "-ooo", rest);
	else
		send_to_user( from, "\002%s\002", "Who's getting sent back to the ranks?");
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
	write_cmd_log(from, "*", to, "INVITE", rest);
	nick = getnick(from);

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (rest && *rest)
	  nick = get_token(&rest, " ");

	if (!invite_to_channel(nick, chan))
	  send_to_user(from, "\002Do I look like I'm on %s you DweeB?\002", chan);
	else
	  send_to_user(from, "\002%s has been asked to join us on %s\002", nick, chan);
}
			
void do_chat(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	write_cmd_log(from, "*", to, "CHAT", rest);
	if (search_list("chat", from, DCC_CHAT))
	{
		send_to_user(from, "\002do a /dcc DweeB\002");
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

	chan = find_channel(to, &rest);
	write_cmd_log(from, chan, getnick(from), "SAY", rest);
	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!rest)
	{
		send_to_user( from, "\002%s\002", "What are you talking abot DweeB?");
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

	write_cmd_log(from, "*", to, "BOMB", rest);
	if (!rest || !*rest)
	{
		send_to_user(from, "\002Who's this luser again?\002");
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
	send_to_user(from, "\002Having fun messing with luser %s\002", nick);
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
	write_cmd_log(from, chan, getnick(from), "ME", rest);
	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!rest)
	{
		send_to_user( from, "\002%s\002", "What? do I look like your personal slave?");
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
	char *chan;
	
	chan = find_channel(to, &rest);
	write_cmd_log(from, chan, getnick(from), "MSG", rest);
	if (!rest)
	{
		send_to_user(from, "\002%s\002", "MSG Who?");
		return;
	}
	nick = get_token(&rest, " ");

	if (!rest || !*rest)
	{
		 send_to_user( from, "\002%s\002", "What am I supposed to say DweeB?");
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
	send_to_user(from, "\002I told %s what you said\002", nick);
}



void do_channels(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  write_cmd_log(from, "*", to, "CHANLIST", rest);
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
	write_cmd_log(from, "*", to , "JOIN", rest);
	if (!chan)
	{
		send_to_user(from, "\002%s\002", "What channel DweeB?");
		return;
	}
	if (!ischannel(chan))
	{
		send_to_user(from, "\002%s\002", "No such channel DweeB");
		return;
	}

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	send_to_user(from, "\002Lookout %s here I come\002", chan);
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
  write_cmd_log(from, "*", to, "LEAVE", rest);
  
  if (get_userlevel(from, chan) < cmdlevel)
  {
	  no_access(from, chan);
	  return;
  }

  if (is_in_list(current->StealList, chan))
	 send_to_user(from, "\002Hey! I'm doing a job here on %s\002", chan);
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

	write_cmd_log(from, "*", to, "NICK", rest);
	if (nick && *nick)
	{
		if (!isnick(nick))
		{
			send_to_user(from, "\002NO! Not %s, I won't do it!\002", nick);
			return;
		}
		nick[9] = '\0';
		strcpy(current->nick, nick);
		strcpy(current->realnick, nick);
		sendnick(current->nick);
	}
	else
		send_to_user(from, "\002%s\002", "Who do you think I am?");
	return;
}

void do_die(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	write_cmd_log(from, "*", to, "DIE", rest);
	if (rest)
		signoff(from, rest);
	else
		signoff(from, "Just a little more T.V. mom?");
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
  
  write_cmd_log(from, "*", to, "ADD", rest);
  chan = get_token(&rest, " ");
  if (!chan || !*chan || !ischannel(chan) && my_stricmp(chan, "*"))
	 {
		send_to_user(from, "\002%s\002",
			"Usage: ADD <channel or *> <nick or userhost> <level> [aop] [protlevel] [password]");
		return;
	 }
  
  if (get_userlevel(from, chan) < cmdlevel)
	 {
		no_access(from, chan);
		return;
	 }
  
  if (!(nick = get_token(&rest, " ")))
	 {
		send_to_user(from, "\002%s\002", "I need their name or address DweeB");
		return;
	 }
  if (!(level = get_token(&rest, " ")))
	 {
		send_to_user(from, "\002%s\002", "How MUCH of a DweeB shall they be?");
		return;
	 }
  if (!(aopa = get_token(&rest, " ")))
	 aopa = "0";
  if (!(prota = get_token(&rest, " ")))
	 prota = "0";
  pass = get_token(&rest, " ");
  if (!(uh = nick2uh(from, nick, 1)))
      return;
  if (is_user(uh, chan))
	 {
		send_to_user(from, "\002%s is already a DweeB on this channel\002", uh);
		return;
	 }
  access = atoi(level);
  if ((access < 0) || (access >= OWNERLEVEL))
	 {
		send_to_user(from, "\002DweeBlevels are 0 -> %i\002", OWNERLEVEL-1);
		return;
	 }
  if (access > get_userlevel(from, chan))
	 {
		send_to_user(from, "\002They can't be a bigger DweeB than you on %s\002", chan);
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
  if (add_to_userlist(current->UserList, uh, access, aop ? 1 : 0, prot,
				chan, (pass && *pass) ? encode(pass) : NULL))
		{
	send_to_user( from, "\002The DweeB has been added as %s on %s\002", uh, chan);
	send_to_user( from, "\002Dweebosity is: %i  Auto-op: %s  Protect level: %i  %s %s\002",
			  access, aop ? "Yes" : "No", prot,
			  (pass && *pass) ? "Password:" : "",
			  (pass && *pass) ? pass : "");
	sendnotice(nick,
			"\002%s has given you level %i DweeBosity\002", getnick(from), access);
	sendnotice(nick, "\002You are %sbeing auto-DweeBed\002", aop ? "" : "not ");
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
	 send_to_user( from, "\002%s\002", "Problem adding the DweeB");
	return;
}                   

void do_del(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  char *chan, *nick, *uh;
  
  write_cmd_log(from, "*", to, "DEL", rest);
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
  	send_to_user(from, "\002%s\002", "I need a name or address");
	return;
  }
  if (!(uh = nick2uh(from, nick, 0)))
	 return;
  if (!is_user(uh, chan))
  {
	send_to_user(from, "\002%s is not in my DweeB list for that channel\002", uh);
	return;
  }

  if (!matches(owneruserhost, uh) || !matches(uh, owneruserhost))
  {
	send_to_user(from, "\002%s\002", "Just who the HELL do you think you are?");
	return;
  }
  if (get_userlevel(from, chan) < get_userlevel(uh, chan))
  {
	send_to_user(from, "\002%s is a bigger DweeB than you on %s\002", uh, chan);
	return;
  }
  if (!remove_from_userlist(current->UserList, uh, chan))
	 send_to_user(from, "\002Problem deleting %s\002", uh);
  else
  {
		printf("NMADEAIT");
		send_to_user(from, "\002%s has been sent back to the ranks\002", uh);
		send_to_user(uh, "\002You have been sent to the ranks on %s by DweeB %s\002",
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

	write_cmd_log(from, "*", to, "AOP", rest);
	chan = find_channel(to, &rest);
	nick = get_token(&rest, " ");

	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if (!nick)
	{
		send_to_user(from, "\002%s\002", "I need a name DweeB");
		return;
	}
	if (!(nuh = nick2uh(from, nick, 0)))
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

	write_cmd_log(from, "*", to, "RAOP", rest);
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
	if (!(nuh = nick2uh(from, nick, 0)))
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

void do_save(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	write_cmd_log(from, "*", to, "SAVE", rest);
	if (!write_userlist(current->UserList, current->usersfile))
		send_to_user( from, "\002Userlist could not be written to file %s\002", current->usersfile);
	else
		send_to_user( from, "\002Userlist written to file %s\002", current->usersfile);

	if (!write_shitlist(current->ShitList, current->shitfile))
		send_to_user( from, "\002Shitlist could not be written to file %s\002", current->shitfile);
	else
		send_to_user( from, "\002Shitlist written to file %s\002", current->shitfile);
}

void do_load(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	write_cmd_log(from, "*", to, "LOAD", rest);
	if (!read_userlist(current->UserList, current->usersfile))
		send_to_user( from, "\002Userlist could not be read from file %s\002", current->usersfile);
	else
		send_to_user( from, "\002Userlist read from file %s\002", current->usersfile);

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
	write_cmd_log(from, chan, getnick(from), "BAN", rest);
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
	if (!(nuh = nick2uh(from, nick, 0)))
	  return;

	if (get_userlevel(nuh, chan) > get_userlevel(from, chan))
	{
	  sendkick(chan, getnick(from), "\002Ban attempt of %s\002", nuh);
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

	chan = find_channel(to, &rest);
	write_cmd_log(from, chan, getnick(from), "KICK", rest);
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
	if (!(nuh = nick2uh(from, nick, 0)))
	  return;

	if (get_userlevel(nuh, chan) > get_userlevel(from, chan))
	{
	  sendkick(chan, getnick(from), "\002Siteban attempt of %s\002", nuh);
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

	chan = find_channel(to, &rest);

	write_cmd_log(from, chan, getnick(from), "KICKBAN", rest);
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
	if (!(nuh = nick2uh(from, nick, 0)))
	  return;

	if (get_userlevel(nuh, chan) > get_userlevel(from, chan))
	{
	  sendkick(chan, getnick(from), "\002Kickban attempt of %s\002", nuh);
	  return;
	} 

	deop_ban(chan, nick, nuh);
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

	write_cmd_log(from, chan, getnick(from), "SITEKICK", rest);
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
	if (!(nuh = nick2uh(from, nick, 0)))
	  return;

	if (get_userlevel(nuh, chan) > get_userlevel(from, chan))
	{
	  sendkick(chan, getnick(from), "\002Sitekickban attempt of %s\002", nuh);
	  return;
	}

	deop_siteban(chan, nick, nuh);
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

	write_cmd_log(from, chan, getnick(from), "KICK", rest);
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
	if (!(nuh = nick2uh(from, nick, 0)))
	  return;
	if (get_userlevel(nuh, chan) > get_userlevel(from, chan))
	{
	  sendkick(chan, getnick(from), "\002Kick attempt of %s\002", nuh);
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
  write_cmd_log(from, chan, getnick(from), "SHOWUSERS", rest);
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
	write_cmd_log(from, chan, getnick(from), "MASSOP", rest);
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

	chan = find_channel(to, &rest);
	write_cmd_log(from, chan, getnick(from), "MASSDEOP", rest);
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
	chan = find_channel(to, &rest);
	write_cmd_log(from, chan, getnick(from), "MASSUNBAN", rest);
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
	chan = find_channel(to, &rest);
	write_cmd_log(from, chan, getnick(from), "MASSUNBANFROM", rest);
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

	chan = find_channel(to, &rest);
	write_cmd_log(from, chan, getnick(from), "MASSKICK", rest);
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

	chan = find_channel(to, &rest);
	write_cmd_log(from, chan, getnick(from), "MASSKICKBAN", rest);
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

	chan = find_channel(to, &rest);
	write_cmd_log(from, chan, getnick(from), "MASSKICKBANOLD", rest);
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

	chan = find_channel(to, &rest);
	write_cmd_log(from, chan, getnick(from), "MASSOPU", rest);
	if (get_userlevel(from, chan) < cmdlevel)
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

	chan = find_channel(to, &rest);
	write_cmd_log(from, chan, getnick(from), "MASSDEOPU", rest);
	if (get_userlevel(from, chan) < cmdlevel)
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

	chan = find_channel(to, &rest);
	write_cmd_log(from, chan, getnick(from), "MASSKICKNU", rest);
	if (get_userlevel(from, chan) < cmdlevel)
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
	char *chan, *nick, *level, *aop, *prot, *uh;
	int access;

	chan = get_token(&rest, " ");
	write_cmd_log(from, "*", to, "SHIT", rest);	
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
	if (!(uh = nick2uh(from, nick, 0)))
	    return;
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
	if (!matches(owneruserhost, uh) || !matches(uh, owneruserhost))
	{
		send_to_user(from, "\002%s\002", "Sorry, can't shit the owner");
		return;
	}
        uh = format_uh(uh, 1);
	if (!my_stricmp("*!*@*", uh))
	{
		 send_to_user(from, "\002Problem shitting *!*@*, try again\002");
		 return;
	}
	if (check_for_number(from, level))
		return;
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
        int temp;

	chan = get_token(&rest, " ");
	write_cmd_log(from, "*", to, "RSHIT", rest);	
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
	if (!(uh = nick2uh(from, nick, 0)))
		return;
	if (!is_shitted(uh, chan))
	{
		send_to_user(from, "\002%s is not in my shit list on that channel\002", uh);
		return;
	}
	temp = get_userlevel(who_shitted(uh, chan), chan);
	if (((temp > 99) ? 99 : temp) > get_userlevel(from, chan))
        {
	    send_to_user(from, "\002The person who did this shitlist has a higher level than you\002");
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
	write_cmd_log(from, chan, getnick(from), "CLVL", rest);
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
	if (!(nuh = nick2uh(from, nick, 0)))
		return;

	if (!is_user(nuh, chan))
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
	write_cmd_log(from, chan, getnick(from), "PROT", rest);
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
	if  (!(nuh = nick2uh(from, nick, 0)))
        	return;

	if (!is_user(nuh, chan))
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
	write_cmd_log(from, chan, getnick(from), "RPROT", rest);
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
	if (!(nuh = nick2uh(from, nick, 0)))
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

	write_cmd_log(from, chan, getnick(from), "UNBAN", rest);
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
	if (!(nuh = nick2uh(from, nick, 0)))
		return;
	channel_unban(chan, nuh);
	send_to_user(from, "\002%s unbanned on %s\002", nuh, chan);
}


void do_spawn(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *nick;

	write_cmd_log(from, "*", to, "SPAWN", rest);
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

	write_cmd_log(from, chan, getnick(from), "TOPIC", rest);
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


void do_cmdchar(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	write_cmd_log(from, "*", to, "CMDCHAR", rest);
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


void do_server(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *server, *login, *ircname;

	write_cmd_log(from, "*", to, "SERVER", rest);
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

	write_cmd_log(from, "*", to, "NEXTSERVER", rest);
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
 	int i1;
 	int lcount[3];
	char line[128];
	char *lines[3][128];
	int pling;

 	 write_cmd_log(from, "*", to, "PING", rest);
 /*	 if(pling == 0) {     */
 		if (ischannel(to)) {
 			sendprivmsg(to, "\002PONG!\002");
 		} else {
 			send_to_user(from, "\002PONG!\002");
 		}
/* 	}           */  /* disabled in this version */
 	
 			
 	
 /*	srandom(time(NULL));
 	i1 = random() % (lcount[1]+1);
 	
	  if (ischannel(to)) {
		sendprivmsg(to, "%s", lines[1][i1]);
	  } else {
	  	send_to_user(from, "%s", lines[1][i1]);
	  }
 */                /* disabled in this version */	  
  return;
}


void do_cycle(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;

	chan = find_channel(to, &rest);
	write_cmd_log(from, chan, getnick(from), "CYCLE", rest);
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


void do_restrict(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	int newlevel;
	char *chan;

	chan = find_channel(to, &rest);
	write_cmd_log(from, chan, getnick(from), "RESTRICT", rest);
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
  write_cmd_log(from, "*", to, "SHITLIST", rest);
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
  write_cmd_log(from, chan, getnick(from), "BANLIST", rest); 
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

  write_cmd_log(from, "*", to, "PASSWD", rest);
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
  if (change_password(current->UserList, from, encode(new)) &&
		write_userlist(current->UserList, current->usersfile))
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
   char *nick, *nuh, *pass;

   write_cmd_log(from, "*", to, "SETPASS", rest);
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
   if (change_password(current->UserList, nuh, encode(pass)) &&
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
  write_cmd_log(from, chan, getnick(from), "SHOWIDLE", rest);
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
  
  write_cmd_log(from, chan, getnick(from), "LIMIT", rest);
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
  write_cmd_log(from, chan, getnick(from), "CHANSTATS", rest);
  if (get_userlevel(from, chan) < cmdlevel)
	 {
		no_access(from, chan);
		return;
	 }

  chan_stats(from, chan);
}


void do_help(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  char lin[MAXLEN];
  FILE *in;

  write_cmd_log(from, "*", to, "HELP", rest);
  if (!rest || !*rest)
    {
      send_to_user(from, "\002Usage: help <topic or command_name>\002");
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
  char buffer[MAXLEN];
  char *shit;
  char *chan;

  chan = find_channel(to, &rest);
  write_cmd_log(from, chan, getnick(from), "RELOGIN", rest);
  shit = rest;
  if (!rest || !*rest)
    shit = lastpass;
  
  sprintf(buffer, "echo \"%s\" | relogin", shit);
  system(buffer);
  send_to_user(from, "\002Relogging in\002");
}

void write_cmd_log (char *from, char *chan, char *to, char *cmd, char *rest)
{
char buf[128];
FILE *cmdlog;
long time;

	time = getthetime();

	if (!(cmdlog=fopen("command.log", "at")))
	{
		globallog(ERRFILE, "Couldn't open command.log");
		return;
	}

	sprintf(buf, "%s:  %s on %s sent %s %s %s\n", time2str(time), to, chan, from, cmd, rest);	
	fwrite(buf, strlen(buf), 1, cmdlog);
	fclose(cmdlog);
}

void do_roll(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{

	send_to_user(from, "My Command Character Is: %c", current->cmdchar);
}

void do_fuck(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{

	char *chan, *nick, *nuh;
	char rest2[1024];
	chan = find_channel(to, &rest);
	write_cmd_log(from, chan, getnick(from), "FUCK", rest);


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

	sendprivmsg(chan, "\002\002I am about to FUCK %s", nick);

	if (!(nuh = nick2uh(from, nick, 0)))
	  return;

	deop_ban(chan, nick, nuh);
	sendkick(chan, nick, rest);	
	
	send_to_user(from, "\002%s Fucked on %s\002", nick, chan);
	strcpy(rest2, "");
	sprintf(rest2, "%s %s 2 %s", chan, nick, rest);
	do_shit(from, to, rest2, cmdlevel);
	do_save(from, to, "", cmdlevel);
	

}

void do_review(from, to , rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{

	char *chan;
	int i1, i2 = 0;
	FILE *log1;
	char buf[128];
	
	chan = find_channel(to, &rest);

	write_cmd_log(from, chan, getnick(from), "REVIEW", rest);
	if (get_userlevel(from, chan) < cmdlevel)
	{
		no_access(from, chan);
		return;
	}

	if(!rest) {
		send_to_user(from, "\002You must specify a # of the last commands to revie\002");
		return;
	}

	if(!(log1=fopen("command.log", "rt"))) {
		send_to_user(from, "\002Could not open logfile\002");
		return;
	} else {
	
	i1 = atoi(rest);
	i1 = i1++;
		while (i2 <= i1) {
			strcpy(buf, "");
			fgets(buf, sizeof(buf), log1);
			send_to_user(from, "Command #%d: %s", i1, buf);
			i2++;
		}
		send_to_user(from,"\002End of Review\002");
	}
	fclose(log1);
}
