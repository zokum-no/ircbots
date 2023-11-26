/*
 * powercore.c - the fusion reactor...
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

#define getrandom(min, max) ((rand() % (int)(((max)+1) - (min))) + (min))

void screwban_format(string)
char *string;
{
	char th[5] = "?*??";
	int i, ii, iii, iiii;

	i = getrandom(4, (strlen(string) - 5));
	for (ii = 0; ii < i; ii++)
	{
		iii = getrandom(0,(strlen(string) - 1));
		if (!strchr("!@*?",string[iii]))
		{
			iiii = getrandom(0,3);
			*(string+iii) = th[iiii];
		}
	}
	return;
}

void deop_screwban(channel, nick, nuh)
char *channel;
char  *nick;
char  *nuh;
{
	char temp[MAXLEN];
	char temp2[MAXLEN];

	strcpy(temp, format_uh(nuh,1));
	strcpy(temp2, format_uh(nuh,1));
	if(!channel || !nick || !nuh)
		return;
	screwban_format(temp);
	screwban_format(temp2);
	sendmode(channel,"-o+bb %s %s %s", nick, temp, temp2);
}

int channel_massrandkick(channel, pattern)
char *channel;
char *pattern;
{
	aChannel *Channel;
	register aChanUser *Dummy;
	char buffer[MAXLEN];
	int count;

	if ((Channel = find_channel(channel)) == NULL)
		return 0;
	count = 0;
	Dummy = Channel->users;
	while(Dummy)
	{
		if ((*pattern && !matches(pattern, get_username(Dummy)) &&
			(get_userlevel(get_username(Dummy), channel) <= get_int_varc(Channel, SETMAL_VAR))) ||
			(!*pattern && !is_user(get_username(Dummy), channel)))
		{
			sprintf(buffer, randstring(RANDKICKSFILE));
			sendkick(channel, get_usernick(Dummy), buffer);
			count++;
		}
		Dummy=Dummy->next;
	}
	return count;
}

void do_last(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *thenum;
	int i, num;
	chan = get_channel(to, &rest);

	if (get_userlevel(from, chan) < cmdlevel)
	{
		return;
	}
	if (!rest)
		num = 5;
	else
	{
		thenum = get_token(&rest, " ");
		num = atoi(thenum);
	}
	if ((num < 1) || (num > 20))
	{
		send_to_user(from, "\002last: Number of commands must be between 1 and 20\002");
	}
	else
	{
		send_to_user(from, "\002  Last %i Commands for Lame Ole Me: \002", num);
		send_to_user(from, "--------------------------------------------------------");
		for( i = 0; i < num; i++)
		send_to_user(from, "%2i) %s", i+1, lastcmds[i]);
		send_to_user(from, "------------ \002End Of List\002 -----------------------");
    }
}

void do_ping(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;
	char *nick;
	char buffer[MAXLEN];

	nick = get_token(&rest, " ");
	chan = get_channel(to, &rest);
	
	strcpy(buffer, randstring(PINGFILE));

	if(ischannel(to))
	{
		sendprivmsg(chan, "\002%s\002", buffer);
		return;
	}
	else
	{
		send_to_user(from, "\002%s\002", buffer);
		return;
	}
}

void do_randomkick(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *nuh;

	chan = get_channel(to, &rest);
	if (!(nick = get_token(&rest, " ")))
	{
		send_to_user(from, "\002%s\002", "Ambigous Command: Specify nick.");
		return;
	}
	if (!(nuh = nick2uh(from, nick, 0)))
	  return;
	if (get_userlevel(nuh, chan) > get_userlevel(from, chan))
	{
	  sendkick(chan, currentnick, "\002Random kick attempt of %s! pbht!\002", nick);
	  return;
	}

	sendkick(chan, nick, "%s", randstring(RANDKICKSFILE));
	send_to_user(from, "\002%s kicked from %s with a random reason\002", nick, chan);
}

void do_randomtopic(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan;

	chan = get_channel(to, &rest);
	if (i_am_op(chan))
	{
		sendtopic(chan, randstring(RANDTOPICSFILE));
		send_to_user(from, "\002Topic changed on %s\002", chan);
	}
	else
		not_opped(from, chan);
	return;
}

void do_randommasskick(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *pattern;

	chan = get_channel(to, &rest);
	pattern = get_token(&rest, " ");
	if (!pattern)
		pattern = "*!*@*";
	
	if (i_am_op(chan))
	{
		if (!channel_massrandkick(chan, pattern))
			send_to_user(from, "\002No match for masskick of %s on %s\002",
				pattern, chan);
	}
	else
		not_opped(from, chan);
}

void do_getbomb(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;

{
	char *nick, *nuh, *chan;
	int bing = 8;

	nick = get_token(&rest, " ");
	chan = get_channel(to, &rest);

	if (!rest)
	{
		send_to_user(from, "Usage: getbomb <nick or channel>");
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
		mysend_ctcp(nick, "DCC SEND EnergyMech_%d.zip 1444421 19 6667", bing);
		bing--;
	}
	send_to_user(from, "\002DCC send-bombed %s\002", nick);
 }

void do_swapnick(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char buffer[9];
	
	strcpy(buffer, randstring(NICKSFILE));

#ifdef USE_CCC
	send_to_ccc("1 NICK %s!%s@%s %s!%s@%s",
		current->nick, current->login, mylocalhostname,
		buffer, current->login, mylocalhostname);
	send_to_ccc("1 NICK %s!%s@%s %s!%s@%s",
		current->nick, current->login, mylocalhost,
		buffer, current->login, mylocalhost);
#endif

	strcpy(current->nick, buffer);
	strcpy(current->realnick, buffer);
	sendnick(current->nick);
	send_to_user(from, "My new nick is: \002%s\002", buffer);
	return;
}

void do_screwban(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *nick, *nuh;
	char buffer[MAXLEN];

	chan = get_channel(to, &rest);
	if (get_userlevel(from, chan) < cmdlevel)
		return;
	if (!(nick = get_token(&rest, " ")))
	{
		send_to_user(from,
			"Usage: screw [<channel>] <nick or userhost> [reason]");
		return;
	}
	if (!(nuh = nick2uh(from, nick, 0)))
		return;
		
	strcpy(buffer, randstring(RANDKICKSFILE));
	deop_screwban(chan, nick, nuh);
	sendkick(chan, nick, "%s", (rest && *rest) ? rest : buffer);
}

void do_rehash(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char tbuf[MAXLEN];
	
	strcpy(tbuf, randstring(SIGNOFFSFILE));
	close_all_dcc();
	system(executable);
	kill_all_bots(tbuf);
}

void do_search(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	if (!rest || !*rest)
	{
		send_to_user(from, "\002Usage: SEARCH <*pattern*>\002");
		return;
	}
	search_userlist(from, current->Userlist, rest);
}

void do_match(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	if(!rest || !*rest)
	{
		send_to_user(from, "\002Usage: MATCH <level>\002");
		return;
	}
	search_userlist2(from, current->Userlist, rest);
}

void do_cmatch(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	if(!rest || !*rest)
	{
		send_to_user(from, "\002Usage: CMATCH <level>\002");
		return;
	}
	search_userlist3(from, current->Userlist, rest);
}

void do_heart(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *sendto, *nick;
	int i;
	sendto = strtok(rest, " ");
 	nick = get_token(&rest, " ");

	if (sendto)
		nick = strtok(NULL,  " ");
	if (!sendto || !nick)
	{
		send_to_user(from, "\002Usage: heart <nick/channel> <nick>\002");
		return;
	}

	*(nick + 10) = '\0';
	for(i = 1; i < (10 - strlen(nick)) / 2; i++)
	strcat(nick, " ");
	if (ischannel(sendto))
	{
		send_to_user(from, "\002Sent a lovey-dovey heart to %s\002", sendto);
		sendprivmsg(sendto, "\002          ..     ..\002");
		sendprivmsg(sendto, "\002       .`    `.\'    \'.\002");
		sendprivmsg(sendto, "\002       . \002%9s   \002.\002", nick);
		sendprivmsg(sendto, "\002         .         .\002");
		sendprivmsg(sendto, "\002            .   .\002");
		sendprivmsg(sendto, "\002              .\002");
	}
	else
	{
		send_to_user(from, "\002Sent a lovey-dovey heart to %s\002", sendto);
		sendnotice(sendto, "\002          ..     ..\002");
		sendnotice(sendto, "\002       .`    `.\'    \'.\002");
		sendnotice(sendto, "\002       . \002%9s\002   .\002", nick);
		sendnotice(sendto, "\002         .         .\002");
		sendnotice(sendto, "\002            .   .\002");
		sendnotice(sendto, "\002              .\002");
	}
	return;
}

void do_addserver(from, to, rest, cmdlevel)
char *from, *to, *rest;
int cmdlevel;
{
	if (!rest || !*rest)
	{
		send_to_user(from,"\002Usage: addserver <new_server> [<port>]\002");
		return;
	}
	cfg_server(rest);
	send_to_user(from, "\002Attempting to add server %s to server list", rest);
}

void show_serverlist(from)
char *from;
{
   int temp;

   if (!current)
     return;

   send_to_user(from,"--------------------- \002Server List\002 -----------------------");
   for (temp=0;temp<current->num_of_servers;temp++)
     {
        send_to_user(from,"Server: \002%-23s\002   Port: \002%-5i\002  Active:\002%s\002",
                current->serverlist[temp].name, current->serverlist[temp].port,
                (current->serverlist[temp].active) ? "Yes":"No");
     }
   send_to_user(from,"----------------- \002End of server list\002 --------------------");
}

void do_serverlist(from, to, rest, cmdlevel)
char *from, *to, *rest;
int cmdlevel;
{
	show_serverlist(from); 
}

void do_kill(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *nick, *reason, *rtemp, *temp;
	char buffer[MAXLEN];
	
	nick = get_token(&rest, " ");
	reason = get_token(&rest, " ");
	
	if (current->isIRCop == FALSE) {
		send_to_user(from, "\002Permission Denied- I'm not an IRC operator\002");
		return;
	}
	
	if (!nick) {
		send_to_user(from, "\002You must enter a nick to kill\002");
		return;
	}
	if (!reason) {
		send_to_user(from, "\002Please enter a reason for the kill\002");
		return;
	}
	
	sprintf(buffer, reason);
	
	while (rtemp = get_token(&rest, " ")) {
		sprintf(buffer, "%s %s", buffer, rtemp);
	}
	
	sendkill(nick, "%s", buffer);
	
	if (current->tempvar) {
		MyFree((char **)&current->tempvar);
	}
	
	mstrcpy(&temp, from);
	current->tempvar = temp;
}

void do_kline(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *nick, *reason, *rtemp;
	char buffer[MAXLEN];
	
	nick = get_token(&rest, " ");
	reason = get_token(&rest, " ");
	
	if (current->isIRCop == FALSE) {
		send_to_user(from, "\002Permission Denied- I'm not an IRC operator\002");
		return;
	}
	
	if (!nick) {
		send_to_user(from, "\002You must enter a user@host to kline\002");
		return;
	}
	if (!reason) {
		send_to_user(from, "\002Please enter a reason for the kline\002");
		return;
	}
	
	sprintf(buffer, reason);
	
	while (rtemp = get_token(&rest, " ")) {
		sprintf(buffer, "%s %s", buffer, rtemp);
	}
	
	sendkline(nick, "%s %s", buffer, timetoserver(getthetime()));

	send_to_user(from, "\002K-Lined %s [%s]\002", nick, buffer);
}

void do_oper(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *nick, *pass, *temp;
	
	nick = get_token(&rest, " ");
	pass = get_token(&rest, " ");
	
	if (!nick)
	{
		send_to_user(from, "\002You must enter a username to oper\002");
		return;
	}
	if (!pass) {
		send_to_user(from, "\002Please enter a password to oper\002");
		return;
	}
	
	sendoper(nick, "%s", pass);	
	
	if (current->tempvar) {
		MyFree((char **)&current->tempvar);
	}
	
	mstrcpy(&temp, from);
	current->tempvar = temp;
}

void do_away(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	if (!rest)
	{
		send_to_server("AWAY");
		send_to_user(from, "\002No longer set /away\002");
		current->am_Away = FALSE;
		current->away_serv = getthetime();
		return;
	}
	send_to_server(AWAYFORM, rest, time2away(getthetime()));
	send_to_user(from, "\002Now set /away\002");
	current->am_Away = TRUE;
}

void do_display(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	FILE  *infile;
	char buffer[MAXLEN], filename[255];

	strcpy(filename, FILEUPLOADDIR);
	if (!rest)
	{
		send_to_user(from,
			 "\002Use \"%cdisplay <filename>\" to view one of these files:\002",
			 current->cmdchar);
		if ((infile = fopen(FILELISTNAME, "r")) == NULL)
		{
			send_to_user(from, "\002No files available for display\002");
			return;
		}
		send_to_user(from, "\002------ Filename ------\002");
		while (freadln(infile, buffer))
			send_to_user(from, "%s", buffer);
/*		while (!feof(infile))
		{
			if (fscanf(infile, "%s", buffer))
			send_to_user(from, "%s", buffer);
		}
*/		send_to_user(from, "\002----------------------\002");
		fclose(infile);
		return;
	}
	if ((infile = fopen(FILELISTNAME, "r")) == NULL)
	{
		send_to_user( from, "\002Problem opening %s\002", FILELISTNAME);
		return;
	}
	while (!feof(infile))
	{
		fscanf(infile, "%s\n", buffer);
		if (!strcmp(rest, buffer))
		{
			fclose(infile);
			strcat(filename, rest);
			if ((infile = fopen(filename, "r")) == NULL)
			{
				send_to_user(from, "\002Problem opening the file\002");
				return;
			}
			while (!feof(infile))
				if (freadln(infile, buffer))
					send_to_user(from, buffer);
			fclose(infile);
			return;
		}
	}
	send_to_user(from, "\002That file does not exist...use \"%cdisplay\"\002",
			 current->cmdchar);
	fclose(infile);
}

void do_servstats(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *line, *serv, *temp;
	aTime *aVarX;
	
	line = get_token(&rest, " ");
	serv = get_token(&rest, " ");
	
	if (!line) {
		send_to_user(from, "\002What type of status line do you want to check?\002");
		return;
	}
	if (!serv) {
		send_to_server("STATS %s", line);
	}
	else {
		send_to_server("STATS %s %s", line, serv);
	}
	
	aVarX = make_time(&current->ParseList, from);
	aVarX->num = 20;
}

void do_deoper(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{	
	send_to_server("MODE %s -o", current->nick);
	current->isIRCop = FALSE;
}

void do_trace(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	aTime *aVarX;
	char *thing, *temp;
	char tbuf[MAXLEN];
	
	thing = get_token(&rest, " ");
	
	if (!thing) {
		send_to_server("TRACE");
	}
	else {
		send_to_server("TRACE %s", thing);
	}
	
	aVarX = make_time(&current->ParseList, from);
	aVarX->num = 10;
}

void do_whois_irc(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	aTime *aVarX;
	char *nick, *nick2, *temp;
	char tbuf[MAXLEN];
	
	nick = get_token(&rest, " ");
	nick2 = get_token(&rest, " ");
	
	if (!nick) {
		send_to_user(from, "\002Please specify a nick\002");
	}
	else if (!nick2) {
		send_to_server("WHOIS %s", nick);
	}
	else {
		send_to_server("WHOIS %s %s", nick, nick2);
	}
	
	aVarX = make_time(&current->ParseList, from);
	aVarX->num = 5;
}

void do_lusers(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *temp;
	aTime *aVarX;
	
	send_to_server("LUSERS");

	aVarX = make_time(&current->ParseList, from);
	aVarX->num = 15;		
}
