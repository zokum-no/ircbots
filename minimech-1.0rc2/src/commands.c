/*

    Starglider Class EnergyMech, IRC bot software
    Parts Copyright (c) 1997-2000  proton

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/
#include "config.h"

void do_stats(char *from, char *to, char *rest, int cmdlevel)
{
	aStrp	*ump;
	aUser	*User,*Shit;
	char	*chan,*nuh;
	int	authd;

	chan = get_channel(to,&rest);
	if ((nuh = chop(&rest)))
	{
		User = find_handle(nuh);
		if (!User)
			nuh = nick2uh(from,nuh,0);
	}
	else
	{
		User = NULL;
		nuh = from;
	}
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

	if (!User)
		User = find_user(nuh,chan);

	Shit = find_shit(nuh,chan);

	if (User)
	{
		send_to_user(from,"[\037Immortality stats for %s on %s\037]",nuh,chan);
		send_to_user(from,"          Handle: %s",User->name);
		send_to_user(from,"      User level: %i%s",User->access,
			(User->access == BOTLEVEL) ? " [BOT]" : "");
		if ((ump = User->parm.umask))
		{
			send_to_user(from,"  Recognized u@h: %s%s",ump->p,(ump->next) ? "," : "");
			ump = ump->next;
			while(ump)
			{
				send_to_user(from,"                  %s%s",ump->p,(ump->next) ? "," : "");
				ump = ump->next;
			}
		}
		send_to_user(from,"        Verified: %s",
			(!authd) ? "No password set" : (authd == 1) ? "Yes" : "No");
		send_to_user(from," Protected level: %-3i%s",(User) ? User->prot : 0,
			get_int_var(chan,TOGPROT_VAR) ? "" : "   (Protection toggled off)");
		send_to_user(from,"      Auto Opped: %s%s",(User && User->aop) ? "Yes" : "No ",
			get_int_var(chan,TOGAOP_VAR) ? "" : "   (Auto-op is toggled off)");
		send_to_user(from,"  Partyline Echo: %s",(User->echo) ? "On" : "Off");
	}
	if (Shit)
	{
		if (!User)
			send_to_user(from,"[\037Shitlist stats for %s on %s\037]",nuh,chan);
		send_to_user(from,"   Shitlisted as: %s",Shit->parm.shitmask);
		send_to_user(from,"   Shitlisted by: %s",Shit->whoadd);
		send_to_user(from,"      Shitted on: %s",time2str(Shit->time));
		send_to_user(from,"Shitlisted level: %i",Shit->access);
		send_to_user(from,"Shitlist expires: %s",time2str(Shit->expire));
		send_to_user(from," Shitlist reason: %s",(Shit->reason) ? Shit->reason : "<No reason>");
	}
	send_to_user(from,"[\037End of UStats\037]");
}

void do_access(char *from, char *to, char *rest, int cmdlevel)
{
	char	*chan,*nuh;
	int	level;

	chan = get_channel(to,&rest);
	if ((nuh = chop(&rest)) != NULL)
	{
		if (*nuh == current.cmdchar)
		{
			nuh++; 
			level = level_needed(nuh);
			if (level < 0)
				return;
			else
				send_to_user(from,"The immortality level needed for that command is: %i", level);
			return;
		}
		if ((nuh = nick2uh(from,nuh,0)) == NULL)
			return;
	}
	else
		nuh = from;
	if (*nuh == '$')
	{
		while(*nuh && *nuh != '@')
			nuh++;
		if (*nuh == '@')
			nuh++;
	}
	send_to_user(from,"Immortality Level for %s",nuh);
	send_to_user(from,"Channel: %s  Access: %i",chan,get_userlevel(nuh,chan));
}

void do_auth(char *from, char *to, char *rest, int cmdlevel)
{
	aTime	*tmp;
#ifdef LINKING
	char	tms[256];
#endif
	char	*pass;
	int	pwn,v;

	pass = chop(&rest);
	pwn = password_needed(from);
	v = verified(from);

	if (pwn && v)
	{
		send_to_user(from,"You have already been authorized");
		return;
	}
	if (!pwn)
	{
		send_to_user(from,"You don't need a password to do commands");
		return;
	}
	if (!pass || !*pass)
	{
		if (pwn)
			send_to_user(from,"No password given, not verified");
		return;
	}
	if (strlen(pass) > MAXPASSCHARS)
		pass[MAXPASSCHARS] = 0;
	if (correct_password(from,pass))
	{
		send_to_user(from,"You are now officially immortal");
		if ((tmp = make_time(&current.Auths, from)) != NULL)
			tmp->time = now; 
		op_all_chan(CurrentNick);
#ifdef LINKING
		sprintf(tms,"%s %s",from,pass);
		Link_userverify(NULL,tms);
#endif /* LINKING */
	}
	else
		send_to_user(from,"Incorrect password, not authorized");
}

void do_time(char *from, char *to, char *rest, int cmdlevel)
{
	char	*ts;

	ts = time2away(now);
	if (ischannel(to))
	{
		sendprivmsg(to,"Current time is: %s",ts);
	}
	else
	{
		send_to_user(from,"Current time is: %s",ts);
	}
}

void do_uptime(char *from, char *to, char *rest, int cmdlevel)
{
	send_to_user(from, "Uptime: %s",idle2str(now - current.uptime,FALSE));
}

void do_ontime(char *from, char *to, char *rest, int cmdlevel)
{
	send_to_user(from, "Ontime: %s",idle2str(now - current.ontime,FALSE));
}

void do_cchan(char *from, char *to, char *rest, int cmdlevel)
{
	aChan	*Chan;
	char	*channel;

	if (rest && *rest)
	{
		channel = chop(&rest);
		if ((Chan = find_channel(channel,CH_ACTIVE)) != NULL)
		{
			current.CurrentChan = Chan;
			send_to_user(from,"Current channel set to %s",Chan->name);
		}
		else
			send_to_user(from,ERR_CHAN,channel);
		return;
	}
	send_to_user(from,"Current channel: %s",
		(current.CurrentChan) ? current.CurrentChan->name : "(none)");
}

void do_opme(char *from, char *to, char *rest, int cmdlevel)
{
	char	*channel;

	channel = get_channel(to,&rest);
	if (get_userlevel(from,channel) < cmdlevel)
		return;
	if (is_opped(current.nick,channel))
		sendmode(channel,"+o %s",CurrentNick);
}

void do_deopme(char *from, char *to, char *rest, int cmdlevel)
{
	char	*channel;

	channel = get_channel(to,&rest);
	if (get_userlevel(from,channel) < cmdlevel)
		return;
	if (is_opped(current.nick,channel))
		sendmode(channel,"-o %s",CurrentNick);
}

void do_op(char *from, char *to, char *rest, int cmdlevel)
{
	char	*channel;

	channel = get_channel(to,&rest);
	if (get_userlevel(from,channel) < cmdlevel)
		return;
	if (!is_opped(current.nick,channel))
		return;

	if (rest)
	{
		channel_massmode(channel,rest,CU_CHANOP,'+','o');
		return;
	}
	sendmode(channel,"+o %s",CurrentNick);
}

void do_voice(char *from, char *to, char *rest, int cmdlevel)
{
	char	*channel;

	channel = get_channel(to,&rest);
	if (get_userlevel(from,channel) < cmdlevel)
		return;

	if (!is_opped(current.nick,channel))
		return;

	if (rest)
	{
		channel_massmode(channel,rest,MODE_FORCE,'+','v');
		return;
	}
	sendmode(channel,"+v %s",CurrentNick);
}

void do_deop(char *from, char *to, char *rest, int cmdlevel)
{
	char	*chan;

	chan = get_channel(to,&rest);
	if (get_userlevel(from,chan) < cmdlevel)
		return;
	if (!(usermode(chan,current.nick) & CU_CHANOP))
		return;
	if (rest)
	{
		channel_massmode(chan,rest,CU_CHANOP,'-','o');
		return;
	}
	usage(from,C_DEOP);
}

void do_unvoice(char *from, char *to, char *rest, int cmdlevel)
{
	char	*chan;

	chan = get_channel(to,&rest);
	if (get_userlevel(from,chan) < cmdlevel)
		return;
	if (!(usermode(chan,current.nick) & CU_CHANOP))
		return;
	if (rest)
	{
		channel_massmode(chan,rest,MODE_FORCE,'-','v');
		return;
	}
	usage(from,C_UNVOICE);
}

void do_invite(char *from, char *to, char *rest, int cmdlevel)
{
	char	*channel,*nick;

	channel = get_channel(to,&rest);
	if (get_userlevel(from,channel) < cmdlevel)
		return;

	nick = CurrentNick;
	if (rest && *rest)
		nick = rest;

	if ((find_channel(channel,CH_ACTIVE)) == NULL)
	{
		send_to_user(from,ERR_CHAN,channel);
		return;
	}

	while(nick && *nick)
		send_to_server("INVITE %s %s",chop(&nick),channel);

	send_to_user(from,"User(s) invited to %s",channel);
}
			
void do_chat(char *from, char *to, char *rest, int cmdlevel)
{
	if (search_list(from,DCC_CHAT))
	{
		send_to_user(from,"You are already DCC chatting me");
		return;
	}
	dcc_chat(from,rest);
}

void do_say(char *from, char *to, char *rest, int cmdlevel)
{
	char	*chan;

	chan = get_channel(to,&rest);
	if (get_userlevel(from,chan) < cmdlevel)
		return;
	if (!rest)
	{
		usage(from,C_SAY);
		return;
	}
	sendprivmsg(chan,"%s",rest);
}

void do_me(char *from, char *to, char *rest, int cmdlevel)
{
	char	*chan;

	chan = get_channel(to,&rest);
	if (get_userlevel(from,chan) < cmdlevel)
		return;
	if (!rest)
	{
		usage(from,C_ME);
		return;
	}
	send_ctcp(chan,"ACTION %s",rest);
}

void do_msg(char *from, char *to, char *rest, int cmdlevel)
{
	aDCC	*Client;
	char	*nick;

	nick = chop(&rest);
	if (!rest || !*rest)
	{
		usage(from,C_MSG);
		return;
	}
	if (ischannel(nick))
	{
		do_say(from,to,rest,cmdlevel);
		return;
	}
	if (!Strcasecmp(nick,current.nick))
	{
		send_to_user(from,"Nice try, but I don't think so");
		return;
	}
	if (*nick == '=')
	{
		nick++;
		if ((Client = search_list(nick,DCC_CHAT)) == NULL)
		{
			send_to_user(from,"I have no DCC connection to %s",nick);
			return;
		}
		send_chat(Client->user,rest);
	}
	else	
		sendprivmsg(nick,"%s",rest);
	send_to_user(from,"Message sent to %s",nick);
}


void do_do(char *from, char *to, char *rest, int cmdlevel)
{
	if (rest)
		send_to_server("%s",rest);
	else
		usage(from,C_DO);
}

void do_join(char *from, char *to, char *rest, int cmdlevel)
{
	char	*chan;

	chan = chop(&rest);
	if (!chan)
	{
		usage(from,C_JOIN);
		return;
	}
	if (!ischannel(chan))
	{
		send_to_user(from,"Invalid channel name");
		return;
	}
	if (get_userlevel(from,chan) < cmdlevel)
		return;
	send_to_user(from,"Attempting the join of %s",chan);
	join_channel(chan,rest,TRUE);
}

void do_part(char *from, char *to, char *rest, int cmdlevel)
{
	aChan	*Chan;
	char	*channel;

	channel = get_channel(to,&rest);
	if (get_userlevel(from,channel) < cmdlevel)
		return;
	if ((Chan = find_channel(channel,CH_ACTIVE)) == NULL)
	{
		send_to_user(from,ERR_CHAN,channel);
		return;
	}
	send_to_user(from,"Leaving %s",channel);
	Chan->rejoin = FALSE;
	Chan->active = CH_OLD;
	send_to_server("PART %s",channel);
	if (Chan == current.CurrentChan)
	{
		Chan = current.Channels;
		while(Chan)
		{
			if (Chan->active == CH_ACTIVE)
				break;
			Chan = Chan->next;
		}
		current.CurrentChan = Chan;
	}
}

void do_nick(char *from, char *to, char *rest, int cmdlevel)
{
	char *nick;

	nick = chop(&rest);
	if (nick && *nick)
	{
		if (!isnick(nick))
		{
			send_to_user(from,ERR_NICK,nick);
			return;
		}
		if (strlen(nick) > MAXNICKLEN)
		{
			send_to_user(from,"Nickname too long: %s",nick);
			return;
		}
		strcpy(current.wantnick,nick);
		send_to_server("NICK %s",current.wantnick);
	}
	else
		usage(from,C_NICK);
}

void do_die(char *from, char *to, char *rest, int cmdlevel)
{
	char	*reason;
	
	if (get_userlevel(from,NULL) < cmdlevel)
		return;

#ifdef SESSIONS
	savesession();
#endif /* SESSIONS */

	if (!rest || !*rest)
	{
		if ((reason = randstring(SIGNOFFSFILE)) == NULL)
			reason = "I'll get you for this!!!";
	}
	else
	{
		reason = rest;
	}

	set_mallocdoer(do_die);
	mstrcpy(&current.signoff,reason);
	set_mallocdoer(do_die);
	mstrcpy(&current.from,from);

	current.connect = CN_BOTDIE;
}

void do_add(char *from, char *to, char *rest, int cmdlevel)
{
	aUser	*User;
	char	*handle,*chan,*nick,*level,*aopa,*prota,*uh,*pass;
	char	passwd[PASSBUF];
	int	axs,aop,prot,already,ulevel;
	int	*ip;

	handle = chop(&rest);
	chan = chop(&rest);

	if (!chan || !*chan || (!ischannel(chan) && Strcasecmp(chan,"*")))
		goto usage;

	if (*chan == '*')
		ulevel = get_userlevel(from,NULL);
	else
		ulevel = get_userlevel(from,chan);

	if (ulevel < cmdlevel)
		return;

	if (strlen(handle) > MAXNICKLEN)
	{
		send_to_user(from,"Handle must be between 1 and %i chars in length",MAXNICKLEN);
		return;
	}
	if ((User = find_handle(handle)) != NULL)
	{
		send_to_user(from,"Handle %s is already in use",User->name);
		return;
	}
	if ((nick = chop(&rest)) == NULL)
	{
		send_to_user(from,"No nick or userhost specified");
		return;
	}
	if ((level = chop(&rest)) == NULL)
	{
		send_to_user(from,"No level specified");
		return;
	}
	if ((aopa = chop(&rest)) == NULL)
		aopa = "0";
	if ((prota = chop(&rest)) == NULL)
		prota = "0";
	pass = chop(&rest);

	ip = Atoi(level);
	if (!ip)
		goto usage;
	axs = *ip;

	if ((axs != BOTLEVEL) && ((axs < 0) || (axs > OWNERLEVEL)))
	{
		send_to_user(from,"Valid levels are from 0 thru %i",OWNERLEVEL);
		return;
	}
	if ((ulevel != OWNERLEVEL) && (axs > ulevel))
	{
		send_to_user(from,"Level must be less than or equal to yours on %s",chan);
		return;
	}

	if (!(uh = nick2uh(from,nick,1)))
		return;
	if (!matches(uh,"!@"))
	{
		send_to_user(from,"Problem adding %s (global mask)",uh);
		return;
	}
	if (matches("*!*@*.*",uh))
	{
		send_to_user(from,"Problem adding %s (invalid mask)",uh);
		return;
	}

	ip = Atoi(prota);
	if (!ip || (*ip < 0) || (*ip > MAXPROTLEVEL))
		goto usage;
	prot = *ip;

	ip = Atoi(aopa);
	if (!ip || (*ip < 0) || (*ip > 1))
		goto usage;
	aop = *ip;

	already = is_user(uh,chan);
	if (already)
	{
		send_to_user(from,"%s %s on %s is already a user",nick,uh,chan);
		return;
	}
	if (axs == BOTLEVEL)
		pass = NULL;
	if (pass && *pass)
		makepass(passwd,pass);
	else
		passwd[0] = 0;

	if ((User = add_to_userlist(handle,axs,aop,prot,chan,(passwd[0]) ? passwd : NULL)) != NULL)
	{
		addmasktouser(User,uh);
		send_to_user(from,"%s has been added as %s on %s",handle,uh,chan);
		send_to_user(from,"Access level: %i  Auto-op: %s  Protect level: %i  %s %s",
			axs,(aop) ? "Yes" : "No",prot,(pass && *pass) ? "Password:" : "",
			(pass && *pass) ? pass : "");
#ifdef NEWUSER_SPAM
		if ((strchr(nick,'@') == NULL) && (axs != BOTLEVEL))
		{
			send_to_server("NOTICE %s :%s has blessed you with %i levels of immortality",
				nick,CurrentNick,axs);
			send_to_server("NOTICE %s :You are %sbeing auto-opped *smo0tch*",
				nick,(aop) ? "" : "not ");
			if (prot)
				send_to_server("NOTICE %s :You have been granted %i terajoules of EnergyMech Ego Shields",
					nick,prot);
			else
				send_to_server("NOTICE %s :EnergyMech force does not protect you",nick);
			send_to_server("NOTICE %s :My command character is %c",nick,current.cmdchar);
			send_to_server("NOTICE %s :Use \026%c%s\026 for command help",nick,current.cmdchar,C_HELP);
			if (pass && *pass)
			{
				send_to_server("NOTICE %s :Password necessary for doing commands: %s",nick,pass);
				send_to_server("NOTICE %s :If you do not like your password, use \026%c%s\026 to change it",
					nick,current.cmdchar,C_PASSWD);
			}
		}
#endif /* NEWUSER_SPAM */
	}
	else
		send_to_user(from,"Problem adding the user");
	return;
usage:
	usage(from,C_ADD);
}

void do_del(char *from, char *to, char *rest, int cmdlevel)
{
 	aUser	*User;
	char	*nick;
	int	ulevel;

	if (get_userlevel(from,NULL) < cmdlevel)
		return;
	nick = chop(&rest);
	if (!nick)
	{
		usage(from,C_DEL);
		return;
	}
	if ((User = find_handle(nick)) == NULL)
	{
		send_to_user(from,"Unknown handle");
		return;
	}
	ulevel = get_userlevel(from,User->chanp);
	if ((User->access == BOTLEVEL) && (ulevel == OWNERLEVEL))
	{
		send_to_user(from,"Deleting bot %s",nick);
	}
	else
	{
		if (ulevel < User->access)
		{
			send_to_user(from,"%s has a higher immortality level than you on %s",nick,User->chanp);
			return;
		}
	}
	remove_user(User);
	send_to_user(from,"User %s has been purged",nick);
}

#ifdef DYNCMDLEVELS

void do_savelevels(char *from, char *to, char *rest, int cmdlevel)
{
	if (!write_levelfile())
		send_to_user(from,"Levels could not be saved to %s",LEVELFILE);
	else
		send_to_user(from,"Levels were written to %s",LEVELFILE);
}

void do_loadlevels(char *from, char *to, char *rest, int cmdlevel)
{
	if (!read_levelfile())
		send_to_user(from,"Levels could not be read from %s",LEVELFILE);
	else
		send_to_user(from,"Levels were read from %s",LEVELFILE);
}

#endif /* DYNCMDLEVELS */

void do_savelists(char *from, char *to, char *rest, int cmdlevel)
{
	send_to_user(from,"Lists%s saved to file %s",
 		(saveuserlist()) ? "" : " could not be",current.Userfile);
}

void do_loadlists(char *from, char *to, char *rest, int cmdlevel)
{
	send_to_user(from,"Lists%s read from file %s",
		(readuserlist()) ? "" : "could not be",current.Userfile);
}

void do_ban(char *from, char *to, char *rest, int cmdlevel)
{
	char	*chan,*nick,*nuh;
	int	ul;

	chan = get_channel(to,&rest);
	if ((ul = get_userlevel(from,chan)) < cmdlevel)
		return;
	if (!(nick = chop(&rest)))
	{
		send_to_user(from,"No nick specified");
		return;
	}
	if (!(nuh = nick2uh(from,nick,0)))
		return;
	if (get_userlevel(nuh,chan) > ul)
	{
		sendkick(chan,CurrentNick,"Ban attempt of %s",nuh);
		return;
	}
	deop_ban(chan,nick,nuh);
	send_to_user(from,"%s banned on %s",nick,chan);
}
		
void do_siteban(char *from, char *to, char *rest, int cmdlevel)
{
	char	*chan,*nick,*nuh;

	chan = get_channel(to,&rest);
	if (get_userlevel(from,chan) < cmdlevel)
		return;
	if (!(nick = chop(&rest)))
	{
		send_to_user(from,"No nick specified");
		return;
	}
	if (!(nuh = nick2uh(from,nick,0)))
		return;
	if (get_userlevel(nuh,chan) > get_userlevel(from,chan))
	{
		sendkick(chan,CurrentNick,"Siteban attempt of %s",nuh);
		return;
	}
	deop_siteban(chan,nick,nuh);
	send_to_user(from,"%s sitebanned on %s",nick,chan);
}
		
void do_kickban(char *from, char *to, char *rest, int cmdlevel)
{
	char	*chan,*nick,*nuh;

	chan = get_channel(to,&rest);
	if (get_userlevel(from,chan) < cmdlevel)
		return;
	
	if (!(nick = chop(&rest)))
	{
		send_to_user(from,"No nick specified");
		return;
	}
	if (!(nuh = nick2uh(from,nick,0)))
		return;
	if (get_userlevel(nuh,chan) > get_userlevel(from,chan))
	{
		sendkick(chan,CurrentNick,"Kickban attempt of %s",nuh);
		return;
	}
	deop_ban(chan,nick,nuh);
	sendkick(chan,nick,"%s",(rest) ? rest : "Requested Kick" );
	send_to_user(from,"%s kickbanned on %s",nick,chan);
}

void do_sitekickban(char *from, char *to, char *rest, int cmdlevel)
{
	char	*chan,*nick,*nuh;

	chan = get_channel(to,&rest);
	if (get_userlevel(from,chan) < cmdlevel)
		return;
	if (!(nick = chop(&rest)))
	{
		send_to_user(from,"No nick specified");
		return;
	}
	if (!(nuh = nick2uh(from,nick,0)))
		return;
	if (get_userlevel(nuh,chan) > get_userlevel(from,chan))
	{
		sendkick(chan,CurrentNick,"Sitekickban attempt of %s",nuh);
		return;
	}
	deop_siteban(chan,nick,nuh);
	sendkick(chan,nick,"%s",(rest) ? rest : "Requested Kick");
	send_to_user(from,"%s sitekickbanned on %s",nick,chan);
}

void do_kick(char *from, char *to, char *rest, int cmdlevel)
{
	char	*chan,*nick,*nuh;

	chan = get_channel(to,&rest);
	if (get_userlevel(from,chan) < cmdlevel)
		return;
	if (!(nick = chop(&rest)))
	{
		send_to_user(from,"No nick specified");
		return;
	}
	if (!(nuh = nick2uh(from,nick,0)))
		return;
	if (get_userlevel(nuh,chan) > get_userlevel(from,chan))
	{
		sendkick(chan,CurrentNick,"Kick attempt of %s",nuh);
		return;
	}
	sendkick(chan,nick,"%s",(rest) ? rest : randstring(RANDKICKSFILE));
	send_to_user(from,"%s kicked from %s",nick,chan);
}

void do_showusers(char *from, char *to, char *rest, int cmdlevel)
{
	aChanUser *User;
	aChan	*Chan;
	char	*channel,*pattern,*nuh;
	char	modechar,thechar;
	int	umode,ul,flags,ct;

	channel = get_channel(to,&rest);
	if (get_userlevel(from,channel) < cmdlevel)
		return;

	nuh = NULL;
	pattern = chop(&rest);
	if (pattern && (*pattern == '-'))
	{
		nuh = pattern;
		pattern = chop(&rest);
	}
	else
	if (rest && (*rest == '-'))
		nuh = chop(&rest);

	ct = flags = 0;
	if (nuh && !Strcasecmp(nuh,"-ops"))
		flags = 1;
	if (nuh && !Strcasecmp(nuh,"-nonops"))
		flags = 2;

	if ((Chan = find_channel(channel,CH_ANY)) == NULL)
	{
		send_to_user(from,"I have no information on %s",channel);
		return;
	}
	send_to_user(from,"[\037Users on %s%s\037]\n",channel,
		(Chan->active == CH_OLD) ? " (from memory)" : "");

	if (Chan->users)
	{
		for(User=Chan->users;User;User=User->next)
		{
			nuh = get_nuh(User);
			umode = get_usermode(User);

			if ((flags == 1) && !(umode & CU_CHANOP))
				continue;
			if ((flags == 2) && (umode & CU_CHANOP))
				continue;
			if (pattern && *pattern && matches(pattern,nuh))
				continue;

			modechar = ' ';
			if (umode & CU_CHANOP)
				modechar = '@';
			else
			if (umode & CU_VOICE)
				modechar = '+';

			thechar = 'u';
			if ((ul = get_userlevel(nuh,channel)) == 0)
				if ((ul = get_shitlevel(nuh,channel)) != 0)
					thechar = 's';
			if (ul == 200)
				thechar = 'b';

			send_to_user(from,"%4i%c   %c %9s %s",ul,thechar,modechar,
				getnick(nuh),strchr(nuh,'!')+1);
			ct++;
		}
		if (!ct)
			send_to_user(from,"No matching users found");
	}
	else
		send_to_user(from,"No users found");
	send_to_user(from,"\n[\037End of Userlist\037]");
}

void do_shit(char *from, char *to, char *rest, int cmdlevel)
{
	char	*chan,*nick,*level,*uh,*aday,*uh2;
	char	whoshit[255];
	int	axs;
	int	days,uaxs,shaxs; 

	aday = NULL;
	chan = chop(&rest);
	if (!chan || !*chan || (!ischannel(chan) && Strcasecmp(chan,"*")))
	{
		usage(from,C_SHIT);
		return;
	}

	if ((uaxs = get_userlevel(from,chan)) < cmdlevel)
		return;

	if ((nick = chop(&rest)) == NULL)
	{
		send_to_user(from,"No nick or userhost specified");
		return;
	}
	if ((level = chop(&rest)) == NULL)
	{
		send_to_user(from,"No level specified");
		return;
	}
	if (rest && isdigit((int)*rest))
		aday = chop(&rest);
	if (!rest || !*rest)
	{
		send_to_user(from,"No reason specified");
		return;
	}
	if (!(uh2 = nick2uh(from,nick,0)))
		return;
	if (is_shitted(uh2,chan))
	{
		send_to_user(from,"%s is in my shitlist already for this channel",uh2);
		return;
	}
	shaxs = get_userlevel(uh2,chan);
	if ((shaxs > uaxs) && (uaxs != OWNERLEVEL))
	{
		send_to_user(from,"%s is a God compared to you on %s",uh2,chan);
		return;
	}
	if ((uh = format_uh(uh2,1)) == NULL)
		return;
	if (!Strcasecmp("*!*@*",uh))
	{
		send_to_user(from,"Problem shitting *!*@*, try again");
		return;
	}
	if (check_for_number(from,level))
		return;
	axs = atoi(level);
	if ((axs <= 0) || (axs > MAXSHITLEVEL))
	{
		send_to_user(from,"Valid levels are from 1 thru %i",MAXSHITLEVEL);
		return;
	}
	days = aday ? atol(aday) : 30;
	strcpy(whoshit,from);

	if (add_to_shitlist(uh,axs,chan,whoshit,(rest && *rest) ? rest : "",now,now + (86400 * days)))
	{
		send_to_user(from,"The user has been shitted as %s on %s",uh,chan);
		send_to_user(from,"The shitlist will expire: %s",time2str(now + (86400 * days)));
		check_shit();
	}
	else
		send_to_user(from,"Problem shitting the user");
}

void do_rshit(char *from, char *to, char *rest, int cmdlevel)
{
	aUser	*User;
	char	*channel,*nick,*uh;
	int	ul;

	channel = chop(&rest);
	if (!channel || !*channel || (!ischannel(channel) && *channel != '*'))
	{
		usage(from,C_RSHIT);
		return;
	}

	if ((ul = get_userlevel(from,channel)) < cmdlevel)
		return;

	if ((nick = chop(&rest)) == NULL)
	{
		send_to_user(from,"No nick or userhost specified");
		return;
	}
	if ((uh = nick2uh(from,nick,0)) == NULL)
		return;
	if ((User = find_shit(uh,channel)) == NULL)
	{
		send_to_user(from,"%s is not in my shit list on that channel",uh);
		return;
	}
	if ((get_userlevel(User->whoadd,channel)) > ul)
	{
		send_to_user(from,"The person who did this shitlist has a higher level than you");
		return;
	}
	remove_user(User);
	send_to_user(from,"User %s is no longer being shitted on %s",uh,channel);
}

void do_clvl(char *from, char *to, char *rest, int cmdlevel)
{
	char	*chan,*nick,*clev;
	int	level,flvl;
	aUser	*tmp;

	level = 0;
	chan = get_channel(to,&rest);
	nick = chop(&rest);
	clev = chop(&rest);

	if ((!nick) || (!clev))
	{
		usage(from,C_CLVL);
		return;
	}
	if ((tmp = find_handle(nick)) == NULL)
	{
		send_to_user(from,"Unknown user: %s",nick);
		return;
	}
	flvl = get_userlevel(from,chan);
	if (flvl < tmp->access)
	{
		send_to_user(from,"access denied");
		return;
	}
	if (check_for_number(from,clev)) return;
	level = atoi(clev);
	if (flvl < level)
	{
		send_to_user(from,"access denied");
		return;
	}
	if ((level < 0) || (level >= OWNERLEVEL+1))
	{
		send_to_user(from,"Valid levels are 0 thru %i",OWNERLEVEL);
		return;
	}

	send_to_user(from,"Level changed from %i to %i",tmp->access,level);
	tmp->access = level;
}

void do_unban(char *from, char *to, char *rest, int cmdlevel)
{
	char	*chan,*nick,*nuh;

	chan = get_channel(to,&rest);
	nick = chop(&rest);
	if (!is_opped(current.nick,chan))
		return;

	if (!nick)
	{
		channel_unban(chan,from);
		send_to_user(from,"You have been unbanned on %s",chan);
		return;
	}
	if (strchr(nick,'*'))
	{
		channel_massunban(chan,nick,0);
		return;
	}
	if (!(nuh = nick2uh(from,nick,0)))
		return;
	channel_unban(chan,nuh);
	send_to_user(from,"%s unbanned on %s",nuh,chan);
}

void do_topic(char *from, char *to, char *rest, int cmdlevel)
{
	aChan	*Chan;
	char	*channel;

	channel = get_channel(to,&rest);
	if (get_userlevel(from,channel) < cmdlevel)
		return;
	if (!rest)
	{
		usage(from,C_TOPIC);
		return;
	}
	if ((Chan = find_channel(channel,CH_ACTIVE)) == NULL)
	{
		send_to_user(from,ERR_CHAN,channel);
		return;
	}
	if ((Chan->topprot == FALSE) || (is_opped(current.nick,channel)))
	{
		send_to_server("TOPIC %s :%s",channel,rest);
		send_to_user(from,"Topic changed on %s",channel);
		return;
	}
	send_to_user(from,ERR_NOTOPPED,channel);
}

void do_spymsg(char *from, char *to, char *rest, int cmdlevel)
{
	aMsgList *pt,new;
	int	i;

	if (*from == '$')
		return;

	pt = current.msglistBASE;
	for(i=0;i<current.msglistSIZE;i++)
	{
		if (pt->used)
			if ((!strcmp(pt->type,"spymsg")) && (!Strcasecmp(pt->who,from)))
			{
				send_to_user(from,"Already redirecting messages to you");
				return;
			}
		pt++;
	}
	memset(&new,0,sizeof(aMsgList));
	new.used = TRUE;
	strcpy(new.who,from);
	strcpy(new.type,"spymsg");
	add_aMsgList(&current.msglistBASE,&new,&current.msglistSIZE);
	send_to_user(from,"Now redirecting messages to you");
}

void do_statmsg(char *from, char *to, char *rest, int cmdlevel)
{
	aMsgList *pt,new;
	int	i;

	if (*from == '$') return;
	pt = current.msglistBASE;
	for(i=0;i<current.msglistSIZE;i++)
	{
		if (pt->used)
			if ((!strcmp(pt->type,"statmsg")) && (!Strcasecmp(pt->who,from)))
				return;
		pt++;
	}
	memset(&new,0,sizeof(aMsgList));
	new.used = TRUE;
	strcpy(new.who,from);
	strcpy(new.type,"statmsg");
	add_aMsgList(&current.msglistBASE,&new,&current.msglistSIZE);
	send_statmsg("(%s) Added to mech core",getnick(from));
}

void do_rspymsg(char *from, char *to, char *rest, int cmdlevel)
{
	aMsgList *pt;
	int	i;

	if (*from == '$')
		return;
	pt = current.msglistBASE;
	for(i=0;i<current.msglistSIZE;i++)
	{
		if (pt->used)
			if ((!strcmp(pt->type,"spymsg")) && (!Strcasecmp(pt->who,from)))
			{
				memset(pt,0,sizeof(aMsgList));
				send_to_user(from,"No longer redirecting messages to you");
				return;
			}
		pt++;
	}
}

void do_rstatmsg(char *from, char *to, char *rest, int cmdlevel)
{
	aMsgList *pt;
	int	i;

	if (*from == '$')
		return;
	pt = current.msglistBASE;
	for(i=0;i<current.msglistSIZE;i++)
	{
		if (pt->used)
			if ((!strcmp(pt->type,"statmsg")) && (!Strcasecmp(pt->who,from)))
			{
				send_statmsg("(%s) Removed from mech core",getnick(from));
				memset(pt,0,sizeof(aMsgList));
				return;
			}
		pt++;
	}
}

void do_server(char *from, char *to, char *rest, int cmdlevel)
{
	aServer *sp;
	char	*server,*login,*ircname,*porta;
	int	port;

	server = chop(&rest);
	if (!server || !*server)
	{
		usage(from,C_SERVER);
		return;
	}
	if (strlen(server) >= MAXHOSTLEN)
	{
		send_to_user(from,"Hostname exceeds maximum length");
		return;
	}
	login = chop(&rest);
	port = 6667;
	if (login && isdigit((int)*login))
	{
		porta = login;
		port = atoi(porta);
		if (!port)
			port = 6667;
		login = chop(&rest);
	}
	ircname = rest;
	if (login && *login)
		strcpy(current.login,login);
	if (ircname && *ircname)
		strcpy(current.ircname,ircname);
	if ((sp = add_server(server,port,NULL)) == NULL)
	{
		send_to_user(from,"Problem switching servers");
		return;
	}
	current.nextserver = sp->ident;
	send_to_user(from,"Trying new server: %s on port %i",server,port);
	switch(current.connect)
	{
	case CN_CONNECTED:
	case CN_ONLINE:
		send_to_server("QUIT :Trying new server, brb...");
		killsock(current.sock);
		break;
	default:
		if (current.sock != -1)
			close(current.sock);
	}
	current.sock = -1;
}

void do_nextserver(char *from, char *to, char *rest, int cmdlevel)
{
	char	*login;

	login = chop(&rest);
	if (login && *login)
		strcpy(current.login,login);
	if (rest && *rest)
		strcpy(current.ircname,rest);

	send_to_user(from,"Switching servers...");
	switch(current.connect)
	{
	case CN_CONNECTED:
	case CN_ONLINE:
		send_to_server("QUIT :Switching servers...");
		killsock(current.sock);
		break;
	default:
		if (current.sock != -1)
			close(current.sock);
	}
	current.sock = -1;
}

#ifdef WARRING

void do_steal(char *from, char *to, char *rest, int cmdlevel)
{
	aChan	*Chan;
	char	*channel;
	int	i;

	channel = chop(&rest);

	if (!channel || !*channel)
	{
		send_to_user(from,"[\037Channels to steal\037]\n");
		i = 0;

		for(Chan=current.Channels;Chan;Chan=Chan->next)
		{
			if (Chan->steal)
			{
				send_to_user(from,"%s",Chan->name);
				i++;
			}
		}

		if (i == 0)
			send_to_user(from,"(none)");

		send_to_user(from,"\n[\037End of List\037]");
		return;
	}

	if (!ischannel(channel))
	{
		usage(from,C_STEAL);
		return;
	}

	if ((Chan = find_channel(channel,CH_ANY)) == NULL)
	{
		join_channel(channel,NULL,TRUE);
		if ((Chan = find_channel(channel,CH_ANY)) == NULL)
		{
			send_to_user(from,"Problem adding the channel");
			return;
		}
	}
	if (Chan->steal)
	{
		send_to_user(from,"Already trying to steal that channel!");
		return;
	}
#ifdef DEBUG
	debug("(do_steal) stealing: %s\n",Chan->name);
#endif
	Chan->steal = TRUE;
	send_to_user(from,"Now trying to steal %s",channel);
}

void do_rsteal(char *from, char *to, char *rest, int cmdlevel)
{
	aChan	*Chan;
	char	*channel;

	channel = chop(&rest);
	if (!channel || !*channel || !ischannel(channel))
	{
		usage(from,C_RSTEAL);
		return;
	}

	if ((Chan = find_channel(channel,CH_ACTIVE)) == NULL)
	{
		send_to_user(from,ERR_CHAN,channel);
		return;
	}

	if (Chan->steal == FALSE)
	{
		send_to_user(from,"I'm not trying to steal %s",channel);
		return;
	}
	Chan->steal = FALSE;
	send_to_user(from,"No longer trying to steal %s",channel);
}

#endif

void do_cycle(char *from, char *to, char *rest, int cmdlevel)
{
	char	*channel;

	channel = get_channel(to,&rest);
	if (get_userlevel(from,channel) < cmdlevel)
		return;

	if ((find_channel(channel,CH_ACTIVE)) == NULL)
		send_to_user(from,ERR_CHAN,channel);
	else
	{
		send_to_user(from,"Cycling channel %s",channel);
		cycle_channel(channel);
	}
}

void do_spy(char *from, char *to, char *rest, int cmdlevel)
{
	aMsgList *pt,new;
	char	*chan,*where;
	int	i;

	chan = chop(&rest);
	where = chop(&rest);

	if (get_userlevel(from,chan) < cmdlevel)
		return;
	if (where && !*where)
		where = NULL;
	if (!chan)
	{
		usage(from,C_SPY);
		return;
	}
	if ((find_channel(chan,CH_ACTIVE)) == NULL)
	{
		send_to_user(from,ERR_CHAN,chan);
		return;
	}
	if (where)
	{
		if (!ischannel(where))
		{
			send_to_user(from,"Object must be a channel");
			return;
		}
		if (where && ((find_channel(where,CH_ACTIVE)) == NULL))
		{
			send_to_user(from,ERR_CHAN,where);
			return;
		}
		if (get_userlevel(from,where) < cmdlevel)
		{
			send_to_user(from,"You don't have enough access on %s",where);
			return;
		}
	}
	if (!where || !*where)
		where = from;

	pt = current.msglistBASE;
	for(i=0;i<current.msglistSIZE;i++)
	{
		if (pt->used)
			if ((!Strcasecmp(pt->type,chan)) && (!Strcasecmp(pt->who,where)))
			{
				send_to_user(from,"You are already spying on %s",chan);
				return;
			}
		pt++;
	}
	memset(&new,0,sizeof(new));
	new.used = TRUE;
	strcpy(new.type,chan);
	strcpy(new.who,where);
	add_aMsgList(&current.msglistBASE,&new,&current.msglistSIZE);
	send_to_user(from,"Now spying on %s",chan);
}

void do_rspy(char *from, char *to, char *rest, int cmdlevel)
{
	aMsgList	*pt;
	char		*chan;
	char		*who;
	int		i;

	chan = chop(&rest);
	if (!chan || !ischannel(chan))
	{
		usage(from,C_RSPY);
		return;
	}
	if (get_userlevel(from,chan) < cmdlevel)
		return;

	who = chop(&rest);
	if (!who)
		who = from;

#ifdef DEBUG
	debug("(do_rspy) %s -%s->%s\n",from,chan,who);
#endif

	pt = current.msglistBASE;
	for(i=0;i<current.msglistSIZE;i++)
	{
#ifdef DEBUG
		if (pt->used)
			debug("[RSPY] type=%s who=%s\n",pt->type,pt->who);
#endif
		if ((pt->used) && (!Strcasecmp(pt->type,chan)))
		{
			if ((!Strcasecmp(pt->who,who)) || (!Strcasecmp(getnick(pt->who),who)))
			{
				if (!Strcasecmp(from,who))
					send_to_user(from,"No longer spying on %s for you",chan);
				else
					send_to_user(from,"No longer spying on %s for %s",chan,who);
				memset(pt,0,sizeof(aMsgList));
				return;
			}
		}
		pt++;
	}
	send_to_user(from,"You are not spying on %s",chan);
}

void do_spylist(char *from, char *to, char *rest, int cmdlevel)
{
	aMsgList *pt;
	int	i;

	send_to_user(from,"[\037Spy List\037]\n");
	pt = current.msglistBASE;
	for(i=0;i<current.msglistSIZE;i++)
	{
		if (pt->used)
			send_to_user(from,"%-12s %s",pt->type,pt->who);
		pt++;
	}
	send_to_user(from,"\n[\037End of List\037]");
}

void do_banlist(char *from, char *to, char *rest, int cmdlevel)
{
	aChan	*Chan;
	aBan	*Ban;
	char	*channel;

	channel = get_channel(to,&rest);
	if ((get_userlevel(from,channel)) < cmdlevel)
		return;

	if ((Chan = find_channel(channel,CH_ACTIVE)) == NULL)
	{
		send_to_user(from,ERR_CHAN,channel);
		return; 
	}

	if (Chan->banned)
	{
		send_to_user(from,"[\037Banlist on %s\037]\n\n\037Ban pattern\037%30s",
			channel,"\037Set by\037");
		for(Ban=Chan->banned;Ban;Ban=Ban->next)
			send_to_user(from,"%-30s   %s",Ban->banstring,Ban->bannedby);
		send_to_user(from,"\n[\037End of Banlist\037]");
	}
	else
		send_to_user(from,"There are no active bans on %s",channel);
}

void do_passwd(char *from, char *to, char *rest, int cmdlevel)
{
	aUser	*User;
	char	*pass,*oldpass;

	oldpass = NULL;

	pass = chop(&rest);
	if (!pass || !*pass)
	{
		usage(from,C_PASSWD);
		return;
	}
	if ((User = find_user(from,NULL)) == NULL)
		return;
	if (User->pass[0])
	{
		oldpass = pass;
		pass = chop(&rest);
		if (!pass || !*pass)
		{
			usage(from,C_PASSWD);
			return;
		}
	}
	if (strlen(pass) < MINPASSCHARS)
	{
		send_to_user(from,"Password must be at least %i characters long",MINPASSCHARS);
		return;
	}
	if (strlen(pass) > MAXPASSCHARS)
		pass[MAXPASSCHARS] = 0;
	if (User->pass[0] && !passmatch(oldpass,User->pass))
	{
		send_to_user(from,"Old password is not correct");
		return;
	}
	makepass(User->pass,pass);
	send_to_user(from,"New password has been set");
}

void do_setpass(char *from, char *to, char *rest, int cmdlevel)
{
	aUser	*User;
	char	*nick,*pass;
	int	ul;

	nick = chop(&rest);
	pass = chop(&rest);

	if (!nick || !*nick || !pass || !*pass)
	{
		usage(from,C_SETPASS);
		return;
	}
	if (strlen(pass) <= MINPASSCHARS)
	{
		send_to_user(from,"Password must be at least %i characters long",MINPASSCHARS);
		return;
	}
	if (strlen(pass) >= MAXPASSCHARS)
		pass[MAXPASSCHARS] = 0;
	if ((User = find_handle(nick)) == NULL)
	{
		send_to_user(from,"Unknown handle");
		return;
	}
	ul = get_userlevel(from,User->chanp);
	if (ul < User->access)
	{
		send_to_user(from,"Access denied");
		return;
	}
	if (!Strcasecmp(pass,"none"))
	{
		if (ul < ASSTLEVEL)
		{
			send_to_user(from,"Access denied");
			return;
		}
		memset(User->pass,0,sizeof(User->pass));
		send_to_user(from,"Password for %s has been removed",User->name);
		return;
	}
	makepass(User->pass,pass);
	send_to_user(from,"New password for %s has been set",User->name);
}

void do_showidle(char *from, char *to, char *rest, int cmdlevel)
{
	char *chan, *seconds;

	chan = get_channel(to, &rest);
	seconds = chop(&rest);
	if (!seconds || !*seconds) seconds = "5";
	show_idletimes(from, chan, atoi(seconds));
}

void do_cslvl(char *from, char *to, char *rest, int cmdlevel)
{
	char	*chan, *nick, *uh;
	int	temp, newlevel;
	aUser	*tmp;

	chan = chop(&rest);

	if (!chan || !*chan || (!ischannel(chan) && Strcasecmp(chan,"*")))
	{
		usage(from,C_SHITLVL);
		return;
	}
	if (get_userlevel(from,chan) < cmdlevel)
		return;

	nick = chop(&rest);
	if (!nick)
	{
		send_to_user(from,"No nick or userhost specified");
		return;
	}
	if (!(uh = nick2uh(from,nick,0))) return;
	if ((tmp = find_shit(uh,chan)) == NULL)
	{
		send_to_user(from,"%s is not in my shit list on that channel",uh);
		return;
	}
	if (!rest || !*rest)
	{
		send_to_user(from,"No level given");
		return;
	}
	if (check_for_number(from,rest))
		return;
	newlevel = atoi(rest);
	if ((newlevel <= 0) || (newlevel > MAXSHITLEVEL))
	{
		send_to_user(from,"Valid levels are from 1 thru %i",MAXSHITLEVEL);
		return;
	}
	temp = get_userlevel(tmp->whoadd,chan);
	if (((temp > 99) ? 99 : temp) > get_userlevel(from,chan))
	{
		send_to_user(from,"The person who did this shitlist has a higher level than you");
		return;
	}
	tmp->access = newlevel;
	send_to_user(from,"The level has been changed");
}

void do_idle(char *from, char *to, char *rest, int cmdlevel)
{
	if (!is_present(rest))
	{
		send_to_user(from,"That user is not on any of my channels");
		return;
	}
	send_to_user(from,"%s has been idle for %s",
		rest,idle2str(now - get_idletime(rest),FALSE));
}

void do_saveall(char *from, char *to, char *rest, int cmdlevel)
{
	do_savelists(from, to, rest, cmdlevel);
#ifdef DYNCMDLEVELS
	do_savelevels(from, to, rest, cmdlevel);
#endif /* DYNCMDLEVELS */
#ifdef SEEN
	if (!write_seenlist(&current.SeenList,current.seenfile))
	{
		send_to_user("SeenList could not be saved to file %s",current.seenfile);
	}
#endif /* SEEN */
}

void do_loadall(char *from, char *to, char *rest, int cmdlevel)
{
	do_loadlists(from, to, rest, cmdlevel);
#ifdef DYNCMDLEVELS
	do_loadlevels(from, to, rest, cmdlevel);
#endif /* DYNCMDLEVELS */
#ifdef SEEN
	if (!read_seenlist())
	{
		send_to_user("SeenList could not be loaded from file %s",current.seenfile);
	}
#endif /* SEEN */
}

void do_names(char *from, char *to, char *rest, int cmdlevel)
{
	char	*chan;

	chan = get_channel(to, &rest);
	show_names_on_channel(from, chan);
}

void do_forget(char *from, char *to, char *rest, int cmdlevel)
{
	aChan	*Chan;
	char	*channel;

	if ((channel = chop(&rest)) == NULL)
	{
		usage(from,C_FORGET);
		return;
	}
	if ((Chan = find_channel(channel,CH_OLD)) == NULL)
	{
		send_to_user(from,"Channel %s is not in memory",channel);
		return;
	}
	if (delete_channel(&current.Channels,Chan))
		send_to_user(from,"Channel %s is now forgotten",channel);
	else
		send_to_user(from,"Channel %s couldn't be forgotten",channel);
}

void do_kicksay(char *from, char *to, char *rest, int cmdlevel)
{
	aUser	*User,new;
	char	*chan,*string,*what;

	chan = chop(&rest);
	if (!chan || !*chan)
	{
		goto usage;
	}
	if (!ischannel(chan) && strcmp(chan,"*"))
	{
		goto usage;
	}
	if (!rest || !*rest)
	{
		goto usage;
	}
	if (*rest != '"')
	{
		send_to_user(from,"Please put quotes (\"'s) around the arguements");
		send_to_user(from,"Example: KS * \"Fuck you\" \"No, fuck you %%n\"");
		return;
	} 
	rest++;
	string = get_token(&rest,"\"");
	if (!*string)
	{
		goto usage;
	}
	if (rest && *rest == ' ')
		rest++;
	if (rest && *rest != '"')
	{
		goto usage;
	}
	if (rest)
		rest++;
	if ((what = rest) != NULL)
		what = get_token(&rest,"\"");
	if (!what || !*what)		
	{
		goto usage;
	}
	if ((User = find_kicksay(string,chan)) != NULL)
	{
		send_to_user(from,"I'm already kicking on \"%s\"",User->parm.kicksay);
		return;
	}

	memset(&new,0,sizeof(aUser));
	new.used = TRUE;
	new.ident = userident++;
	new.type = USERLIST_KICKSAY;
	set_mallocdoer(do_kicksay);
	new.parm.kicksay = (char*)MyMalloc(strlen(string)+3);
	sprintf(new.parm.kicksay,"*%s*",string);
	set_mallocdoer(do_kicksay);
	mstrcpy(&new.whoadd,from);
	set_mallocdoer(do_kicksay);
	mstrcpy(&new.reason,what);
	set_mallocdoer(do_kicksay);
	mstrcpy(&new.chanp,chan);
	add_aUser(&new);
	send_to_user(from,"Now kicking on \"%s\" on %s",new.parm.kicksay,chan);
	return;
usage:
	usage(from,C_KS);
}

void do_rkicksay(char *from, char *to, char *rest, int cmdlevel)
{
	aUser	*User;
	char	*chan;

	chan = chop(&rest);
	if (!chan || !*chan)
	{
		usage(from,C_RKS);
		return;
	}
	if (!ischannel(chan) && strcmp(chan,"*"))
	{
		send_to_user(from,"Specify a channel name or '*' for all");
		return;
	}
	if (!rest || !*rest)
	{
		send_to_user(from,"No kicksay string specified");
		return;
	}
	User = find_kicksay(rest,chan);
	if (!User)
	{
		send_to_user(from,"I'm not kicking on that");
		return;
	}
	send_to_user(from,"No longer kicking on %s",User->parm.kicksay);
	remove_user(User);
}

