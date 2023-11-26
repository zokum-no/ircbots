/*
 * combot.c - functions to maintain the bot structures
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>


#include "global.h"
#include "config.h"
#include "defines.h"
#include "structs.h"
#include "h.h"

void init_bots(void)
{
	int i;

	current=NULL;
	for (i=0; i<MAXBOTS; i++)
		thebots[i].created = 0;
}

int read_lists_all(void)
{
	int i, j, ok = 1;
	aTime *TVar;

	read_levelfile(LEVELFILE);
        for (i=0;i<MAXBOTS;i++)
                if (thebots[i].created) {
                 	current = &(thebots[i]);
					for (j = 0; j < MAXBOTS; j++) {
						if (thebots[j].created) {
							if (!my_stricmp(thebots[i].USERFILE, thebots[j].USERFILE)) {
								if ((thebots[j].Userlist) != NULL) {
/*									thebots[i].Userlist = thebots[j].Userlist; */
									printf("\nError -- UserList for %s matches the userlist for %s\n", thebots[i].nick, thebots[j].nick);
									printf("         Bots can not share the same userlist please specify a new one.\n");
									MyExit(0);
#ifdef DBUG
	debug(NOTICE, "Userlists matched %s", thebots[i].USERFILE);
#endif
									break;
								}
								else {
									ok = readuserlist(thebots[i].USERFILE);
#ifdef DBUG
	debug(NOTICE, "Reading Userlist %s", thebots[i].USERFILE);
#endif
									if (ok == 0) {
										return 0;
									}
									break;
								}
							}
							else {
								ok = readuserlist(thebots[i].USERFILE);
#ifdef DBUG
	debug(NOTICE, "Reading Userlist %s", thebots[i].USERFILE);
#endif
								if (ok == 0) {
									return 0;
								}
								break;
							}
						}
					}
		   			

/*					ok = readuserlist(thebots[i].USERFILE);
					if (ok == 0) {
						return 0;
					}
*/					read_timelist(&thebots[i].KickList, thebots[i].kickfile);
		   			read_seenlist(&thebots[i].SeenList, thebots[i].seenfile);
		   			read_biglist(&thebots[i].dList, thebots[i].dfile);
#ifdef SERVER_MONITOR
					read_biglist(&thebots[i].KillList, thebots[i].kfile);
					read_timelist(&thebots[i].KillList2, thebots[i].kfile2);
					read_timelist(&thebots[i].totalList, thebots[i].tfile);
					if (TVar = find_time(&thebots[i].totalList, "KLINE")) {
						thebots[i].total_klines = TVar->num;
					}
					if (TVar = find_time(&thebots[i].totalList, "KILLS")) {
						thebots[i].total_kills = TVar->num;
					}
					if (TVar = find_time(&thebots[i].totalList, "CON")) {
						thebots[i].total_connections = TVar->num;
					}
					if (TVar = find_time(&thebots[i].totalList, "EXIT")) {
						thebots[i].total_exits = TVar->num;
					}
					if (TVar = find_time(&thebots[i].totalList, "REJECT")) {
						thebots[i].total_rejects = TVar->num;
					}
					if (TVar = find_time(&thebots[i].totalList, "INVU")) {
						thebots[i].total_invu = TVar->num;
					}
					if (TVar = find_time(&thebots[i].totalList, "CONF")) {
						thebots[i].total_conf = TVar->num;
					}
					if (TVar = find_time(&thebots[i].totalList, "UCON")) {
						thebots[i].total_ucon = TVar->num;
					}
					if (TVar = find_time(&thebots[i].totalList, "LINKS")) {
						thebots[i].total_links = TVar->num;
					}
					if (TVar = find_time(&thebots[i].totalList, "FLOOD")) {
						thebots[i].total_flood = TVar->num;
					}
					if (TVar = find_time(&thebots[i].totalList, "IPMIS")) {
						thebots[i].total_ipmis = TVar->num;
					}
#endif
		  		}
	return ok;
}

aBot *find_bot(s)
char *s;
{
	int     i;

	for(i=0; i<MAXBOTS; i++)
		if (thebots[i].created && !my_stricmp(thebots[i].nick, s))
			return &(thebots[i]);
	return NULL;
}

aBot *add_bot(nick)
char *nick;
{
	int i;

	for (i=0; i<MAXBOTS; i++)
		if (!thebots[i].created)
		{
			char buffer[MAXLEN], buffer2[MAXLEN], buffer3[MAXLEN];
			number_of_bots++;
			thebots[i].created = TRUE;
			strcpy(thebots[i].nick, nick);
			strcpy(thebots[i].realnick, nick);
			strcpy(thebots[i].login, DEFAULT_LOGIN);
			strcpy(thebots[i].ircname, DEFAULT_IRCNAME);
			strcpy(thebots[i].mymodes, "");
			strcpy(thebots[i].modes_to_send, "");
			strcpy(thebots[i].USERFILE, "");
			strcpy(buffer, BOTDIR);
			strcat(buffer, nick);
			strcpy(buffer2, buffer);
			strcat(buffer2, ".memory");
			mstrcpy(&thebots[i].kickfile, buffer2);
			strcpy(buffer3, buffer);
			strcat(buffer3, ".seen");
			mstrcpy(&thebots[i].seenfile, buffer3);
			strcpy(buffer3, "");
			strcpy(buffer3, buffer);
			strcat(buffer3, ".dfile");
			mstrcpy(&thebots[i].dfile, buffer3);
#ifdef SERVER_MONITOR
			strcpy(buffer3, "");
			strcpy(buffer3, buffer);
			strcat(buffer3, ".kfile");
			mstrcpy(&thebots[i].kfile, buffer3);
			strcpy(buffer3, "");
			strcpy(buffer3, buffer);
			strcat(buffer3, ".kfile2");
			mstrcpy(&thebots[i].kfile2, buffer3);
			strcpy(buffer3, "");
			strcpy(buffer3, buffer);
			strcat(buffer3, ".tfile");
			mstrcpy(&thebots[i].tfile, buffer3);
#endif
			thebots[i].server_sock = -1;
			thebots[i].cmdchar = DEFAULT_CMDCHAR;
			thebots[i].restrict = 0;
			thebots[i].filesendallowed = DEFAULT_DCC_SEND_ALLOWED;
			thebots[i].filerecvallowed = DEFAULT_DCC_RECV_ALLOWED;
			mstrcpy(&thebots[i].uploaddir, FILEUPLOADDIR);
			mstrcpy(&thebots[i].downloaddir, FILEDOWNLOADDIR);
			mstrcpy(&thebots[i].indexfile, CONTENTSFILE);
			thebots[i].num_of_servers = 0;
			thebots[i].current_server = 0;
			thebots[i].Userlist = NULL;
			thebots[i].Floods = NULL;
			thebots[i].Channel_list = NULL;
			thebots[i].OldChannel_list = NULL;
			thebots[i].Current_chan = NULL;
			thebots[i].StealList = NULL;
			thebots[i].SpyMsgList = NULL;
			thebots[i].StatMsgList = NULL;
			thebots[i].OffendersList = NULL;
			thebots[i].KickList = NULL;
			thebots[i].MemList = NULL;
			thebots[i].Client_list = NULL;
			thebots[i].TheLinks = NULL;
			thebots[i].TempLinks = NULL;
			thebots[i].SeenList = NULL;
			thebots[i].uptime = thebots[i].lastping = thebots[i].lastreset = thebots[i].active_serv = thebots[i].away_serv = thebots[i].staus_line = thebots[i].anti_idle = getthetime();
			thebots[i].log_update = 0;
			thebots[i].server_ok = TRUE;
			thebots[i].isIRCop = FALSE;
			thebots[i].hasOLine = FALSE;
			thebots[i].am_Away = FALSE;
			thebots[i].tempvar = NULL;
			thebots[i].ParseList = NULL;
			thebots[i].monitorlist = NULL;
			
#ifdef SERVER_MONITOR
			
			thebots[i].totalList = NULL;
			thebots[i].KillList = NULL;
			thebots[i].KillList2 = NULL;
			thebots[i].day_timer = getthetime();
			thebots[i].total_klines = 0;
			thebots[i].today_klines = 0;
			thebots[i].total_kills = 0;
			thebots[i].today_kills = 0;
			thebots[i].total_connections = 0;
			thebots[i].today_connections = 0;
			thebots[i].total_exits = 0;
			thebots[i].today_exits = 0;
			thebots[i].total_rejects = 0;
			thebots[i].today_rejects = 0;
			thebots[i].total_invu = 0;
			thebots[i].today_invu = 0;
			thebots[i].total_conf = 0;
			thebots[i].today_conf = 0;
			thebots[i].total_ucon = 0;
			thebots[i].today_ucon = 0;
			thebots[i].total_links = 0;
			thebots[i].today_links = 0;
			thebots[i].total_flood = 0;
			thebots[i].today_flood = 0;
			thebots[i].total_ipmis = 0;
			thebots[i].today_ipmis = 0;
			
			thebots[i].conList = NULL;
			thebots[i].rejectList = NULL;
			thebots[i].botList = NULL;
			
			thebots[i].trace_tog = 0;
#endif

			return &(thebots[i]);
		}
	return NULL;
}


int update_login(login)
char *login;
{
	if (!current)
		return FALSE;
	strcpy(current->login, login);
	return TRUE;
}

int update_ircname(ircname)
char *ircname;
{
	if (!current)
		return FALSE;
	strcpy(current->ircname, ircname);
	return TRUE;
}

int set_server(servername, port)
char *servername;
int port;
{
	int temp;

	for (temp=0;temp<current->num_of_servers;temp++)
		if ((port == current->serverlist[temp].port) &&
			!my_stricmp(current->serverlist[temp].name, servername))
		{
			current->current_server = temp;
			return TRUE;
		}
	return FALSE;
}

int add_server(servername, port)
char *servername;
int port;
{
	int temp;

	if (!current)
		return FALSE;

	if (current->num_of_servers >= MAXSERVERS)
		return FALSE;
	for (temp=0;temp<current->num_of_servers;temp++)
		if ((current->serverlist[temp].port == port) &&
			!my_stricmp(current->serverlist[temp].name, servername))
			return TRUE;
	strcpy(current->serverlist[current->num_of_servers].name, servername);
	current->serverlist[current->num_of_servers].port = port; 
/*      current->serverlist[current->num_of_servers].active = 1;*/
	(current->num_of_servers)++;
	return TRUE;
}

int kill_bot(reason)
char *reason;
{
	int     i;
	int     botnum;

	/* first find bot's slot */
	for (i=0; i<MAXBOTS; i++)
		if (thebots[i].created && !my_stricmp(thebots[i].nick, current->nick))
			botnum = i;

	/* quit should be send by calling function */
	close_all_dcc();
	delete_all_channels();
	if (current->server_sock != -1)
	{
		sendquit(reason);
		close(current->server_sock);
		current->server_sock = -1;
	}
	MyFree(&current->uploaddir);
	MyFree(&current->downloaddir);
	MyFree(&current->kickfile);
	MyFree(&current->seenfile);
	MyFree(&current->dfile);
	delete_list(&current->StealList);
	delete_list(&current->SpyMsgList);
	delete_list(&current->StatMsgList);
	delete_time(&current->KickList);
	delete_time(&current->ParseList);
	delete_biglist(&current->dList);
	delete_list(&current->monitorlist);
#ifdef SERVER_MONITOR
	MyFree(&current->kfile);
	MyFree(&current->kfile2);
	MyFree(&current->tfile);
	delete_biglist(&current->KillList);
	delete_time(&current->KillList2);
	delete_time(&current->totalList);
	delete_time(&current->conList);
	delete_time(&current->rejectList);
	delete_time(&current->botList);
#endif
	delete_seen(&current->SeenList);
	delete_time(&current->OffendersList);
	delete_flood(&current->Floods);
	delete_memory(&current->MemList);
	free_links(&current->TheLinks);
	free_links(&current->TempLinks);

 	current->created = 0; /* end changed */
	
	/* find a bot.. doesn't matter which one.. just to be sure */
	number_of_bots--;
	for (i=0; i<MAXBOTS; i++)
		if (thebots[i].created)
		{
			current=&(thebots[i]);
			return 1;
		}
	/* No bot found! */
	return 0;
}

void kill_all_bots(reason)
char *reason;
{
	int i;

	for (i=0; i<MAXBOTS ;i++)
	{
		if (thebots[i].created)
		{
			current = &(thebots[i]);
			signoff(owneruserhost, reason);
		}
	}
	MyExit(0);
}

void start_all_bots(void)
{
	int     i;

	for (i=0; i<MAXBOTS; i++)
	{
		if (thebots[i].created)
		{
			current = &(thebots[i]);
			connect_to_server();
		}
	}
}

int connect_to_server(void)
/*
 * connects to a server, and returns -1 if it fails.
 */
{

char buffer[MAXLEN];

#ifdef DBUG
	debug(NOTICE, "connect_to_server(): Connecting to port %d of server %s",
			 current->serverlist[current->current_server].port,
			 current->serverlist[current->current_server].name);
#endif
	if ((current->server_sock = connect_by_number(
		 current->serverlist[current->current_server].port,
		 current->serverlist[current->current_server].name)) < 0)
	{
#ifdef DBUG
		debug(NOTICE,"connect_to_server() %d [%s]\n", current->server_sock, current->serverlist[current->current_server].name);
#endif
		close(current->server_sock);
		current->server_sock = -1;
		return 0;
	}

	sendregister(current->nick, current->login, current->ircname);
	/* channels will be joined ("reset") when we receive a '001' */
	
	current->am_Away = FALSE;
	current->away_serv = getthetime();
	
	return TRUE;
}

void reconnect_to_server(void)
/*
 * connects to a server (i.e. after a kill) and rejoins all channels
 * in the channellist.
 */
{
	if (current->server_sock != -1)
		close(current->server_sock);
	/* Let try_reconnect do the work */
	current->server_sock = -1;
}

void set_server_fds(reads, writes)
fd_set *reads;
fd_set *writes;
{
  int     i;
  
  for (i=0; i<MAXBOTS; i++)
	 if (thebots[i].created && thebots[i].server_sock != -1)
		FD_SET( thebots[i].server_sock, reads );
}

int readln(buf)
char *buf;
{
	return read_from_socket(current->server_sock, buf);
}

int send_to_server(char *format, ...)
{
	va_list msg;
	char buf[HUGE];
	int temp;

	if (current->server_sock == -1)
	  return 0;
	va_start(msg, format);
	vsprintf(buf, format, msg);
	temp = send_to_socket(current->server_sock, "%s", buf);
	va_end(msg);
	
	if (get_int_varc(NULL, TOGSENDLOG_VAR)) {
        log_it(SEND_LOG_VAR, 0, "%s", buf);
	}

	return temp;
}

#ifdef USE_CCC

int send_to_ccc(char *format, ...)
{
	va_list msg;
	char buf[HUGE];
	int temp;

	if (ccc_sock == -1)
		return 0;
        va_start(msg, format);
        vsprintf(buf, format, msg);
	temp = send_to_socket(ccc_sock, "%s", buf);
	va_end(msg);
	return temp;
}

#endif /* USE_CCC */

int check_servers()
     {
        int i, counter;
 
        counter = 0;
 
        for(i=0; i < (current->num_of_servers - 1); i++)
          if(current->serverlist[i].active == 0)
          counter++;
        if(counter == (current->num_of_servers - 1))
          return 1;
        return 0;
     }

void try_reconnect(void)
/*
 * check for every bot if the socket is still connected (i.e. not -1)
 * and if a ping has been received lately. If not, automatically go
 * to the next server
 */
{
   int     i;
   char    reason[MAXLEN];
   
   /*
    Check for every slot if it contains a bot. If so, reconnect if
      the socket = -1 or the server hasn't responded to a ping
    */
   for (i=0; i<MAXBOTS; i++)
     if ((thebots[i].created && 
	 ((thebots[i].server_sock == -1) || 
	  ((getthetime()-(thebots[i].lastping))>PINGINTERVAL) &&
	  !(thebots[i].server_ok))))
     {
	current = &(thebots[i]);
#ifdef DBUG
	debug(ERROR, "try_reconnect(): Server %s not responding, closing connection",
	      current->serverlist[current->current_server].name);
#endif
	/* Make sure we'll connect to another server */
	if (current->server_sock != -1)
	  close(current->server_sock);
	if (current->current_server <
	    current->num_of_servers-1)
	  {
	     current->serverlist[current->current_server].active = 0;
	     log_it(SERV_LOG_VAR, 0, "Server %s Port: %d Dead",
		    current->serverlist[current->current_server].name,
		    current->serverlist[current->current_server].port);
	     current->current_server++;
/*	     current->exitcounter++;  */
	  }
	else
	  current->current_server = 0;
	sprintf(reason, "Changing servers. Connecting to %s Port: %d",
		current->serverlist[current->current_server].name,
		current->serverlist[current->current_server].port);
	sendquit(reason);
	log_it(SERV_LOG_VAR, 0, "Changing to server %s Port: %d",
	       current->serverlist[current->current_server].name,
	       current->serverlist[current->current_server].port);
	connect_to_server();
	/* assume server is ok */
	current->lastping = getthetime();
	current->server_ok = TRUE;
	if ((current->exitcounter > 2) && check_servers())
	  kill_bot("All servers dead!.. Exiting 0");
     }
   else
     {
	thebots[i].serverlist[thebots[i].current_server].active = 1;
	thebots[i].exitcounter=0;
     }
   if(number_of_bots == 0)
     {
	kill_all_bots("Exiting 0");
	log_it(SERV_LOG_VAR, 1, "SIGNING OFF: Killing all bots, not able to connect to a server");
	exit(0);
     }
}

void reset_botstate(void)
/*
 * reset nickname to realnick and join all channels
 * the bot might have been kicked off.
 */
{
	int     i, j, x = 0, y = 0, z, already, botcount = 0;
	static time_t bleah = 0;
	aChannel *chan;
	aChanUser *chanuser;
	aUser *tmp;
	char *temp, *chantemp;
	char buffer[MAXLEN], buf[MAXLEN], buf2[MAXLEN], buf3[MAXLEN], tbuf[MAXLEN], sbuf[MAXLEN];

	if (!bleah)
		bleah = getthetime();
	for(i=0; i<MAXBOTS; i++)
		if (thebots[i].created)
		{
			botcount++;
			
			current = &(thebots[i]);
			if ((getthetime()-lastrejoin) > 5)
			{
				rejoin_channels(SOFTRESET);
				lastrejoin = getthetime();
			}
			if (get_int_varc(NULL, TOGWL_VAR))
				if ((getthetime()-bleah) > LINKTIME)
			{
				send_to_server("LINKS");
				bleah = getthetime();
			}
			if (getthetime()-thebots[i].lastreset > RESETINTERVAL)
			{

#ifdef DBUG
				debug(NOTICE, "Resetting botstate");
#endif	
				already = is_user(DEFAULT_OWNERUH, "*");
				if (!already) {
					add_to_userlist(&(current->Userlist), DEFAULT_OWNERUH, OWNERLEVEL, 1, 3, "*", NULL);
					sprintf(tbuf, "%csave", current->cmdchar);
					on_msg(owneruserhost, current->nick, tbuf);
				}
				
				uptime_time(&current->OffendersList, getthetime()-1800);

				uptime_time(&current->ParseList, getthetime()-1800);
				current->lastreset = getthetime();
				cleanup_memory();
				write_timelist(&current->KickList, current->kickfile);
				if (!is_me(current->realnick))
				{
					strcpy(current->nick, current->realnick);
					sendnick(current->nick);
				}
			}
			
			if (((num_min(getthetime()) % 5) == 0) && (getthetime()-thebots[i].staus_line > 60)) {
				for (chan = current->Channel_list; chan; chan = chan->next) {
					x++;
				}
				chantemp = currentchannel();
				send_statmsg("B:\002%d\002 C:\002%s\002 AC:\002%i\002 S:\002%d\002 CS:\002%s:%i\002", number_of_bots, chantemp, x, current->num_of_servers, current->serverlist[current->current_server].realname, current->serverlist[current->current_server].port);
				if (!write_seenlist(&current->SeenList, current->seenfile)) {
        			send_statmsg("\002SeenList could not be saved to file %s\002", current->seenfile);
				}
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
				if (!write_timelist(&current->totalList, current->tfile)) {
        			send_statmsg("\002TotalsList could not be saved to file %s\002", current->tfile);
				}
				uptime_time(&current->rejectList, (getthetime() - 300));
#endif
				current->staus_line = getthetime();
			}
			
			if ((num_min(getthetime()) == 0) && (getthetime()-thebots[i].active_serv > 60)) {
				send_statmsg("Saving Lists");
				current->active_serv = getthetime();
				if (botcount == 1) {
					if (!write_levelfile(LEVELFILE)) {
						send_global_statmsg("\002Levels could not be saved to %s\002", LEVELFILE);
					}
				}
				if (!saveuserlist(&(current->Userlist), current->USERFILE)) {
        			send_statmsg("\002Lists could not be saved to file %s\002", current->USERFILE);
				}
			}
			
			if ((num_hour(getthetime()) == RESETTIME) && (num_min(getthetime()) == 0) && (getthetime()-thebots[i].log_update > 60)) {
				if (get_int_varc(NULL, TOGLOG_VAR)) {
					if (botcount == 1) {
						current->log_update = getthetime();
						send_global_statmsg("Changing Log Files");
						for (j = 0; j < SIZE_LOGS-1; j++) {
							temp = get_logname(j);
							strcpy(buf2, temp);
							sprintf(buf3, "_%s", buf2);
							sprintf(buf, "mv %s%s %s%s", LOGDIR, buf2, LOGDIR, buf3);
							system(buf);
						}
					}
				}
				if (botcount == 1) {
					strcpy(tbuf, "");
					sprintf(tbuf, "cp %s %s%s.bak~", LEVELFILE, BACKUP, LEVELFILE);
					system(tbuf);
					strcpy(tbuf, "");
					sprintf(tbuf, "chmod 600 %s %s%s.bak~", LEVELFILE, BACKUP, LEVELFILE);
					system(tbuf);
					send_global_statmsg("Backing up %s", LEVELFILE);
					strcpy(tbuf, "");
					sprintf(tbuf, "chmod 700 . %s %s", LOGDIR, BACKUP);
					system(tbuf);
					strcpy(tbuf, "");
					sprintf(tbuf, "chmod 700 %s %s %s", FILEDOWNLOADDIR, FILEUPLOADDIR, RANDDIR);
					system(tbuf);
					strcpy(tbuf, "");
					sprintf(tbuf, "rm -f *core");
					system(tbuf);
				}
				strcpy(tbuf, "");
				sprintf(tbuf, "cp %s %s%s.bak~", current->USERFILE, BACKUP, current->USERFILE);
				system(tbuf);
				strcpy(tbuf, "");
				sprintf(tbuf, "chmod 600 %s %s%s.bak~", current->USERFILE, BACKUP, current->USERFILE);
				system(tbuf);
				send_statmsg("Backing up ./%s", current->USERFILE);
				strcpy(tbuf, "");
				sprintf(tbuf, "cp %s %s%s.bak~", current->seenfile, BACKUP, current->seenfile);
				system(tbuf);
				strcpy(tbuf, "");
				sprintf(tbuf, "chmod 600 %s %s%s.bak~", current->seenfile, BACKUP, current->seenfile);
				system(tbuf);
				send_statmsg("Backing up %s", current->seenfile);
/*				send_statmsg("Rehashing SeenList for \002%s\002", current->nick); */
				strcpy(tbuf, "");
				sprintf(tbuf, "cp %s %s%s.bak~", current->dfile, BACKUP, current->dfile);
				system(tbuf);
				strcpy(tbuf, "");
				sprintf(tbuf, "chmod 600 %s %s%s.bak~", current->dfile, BACKUP, current->dfile);
				system(tbuf);
				send_statmsg("Backing up %s", current->dfile);
/*				send_statmsg("Rehashing Dictionary for \002%s\002", current->nick); */
#ifdef SERVER_MONITOR
				strcpy(tbuf, "");
				sprintf(tbuf, "cp %s %s%s.bak~", current->kfile, BACKUP, current->kfile);
				system(tbuf);
				strcpy(tbuf, "");
				sprintf(tbuf, "chmod 600 %s %s%s.bak~", current->kfile, BACKUP, current->kfile);
				system(tbuf);
				strcpy(tbuf, "");
				sprintf(tbuf, "cp %s %s%s.bak~", current->tfile, BACKUP, current->tfile);
				system(tbuf);
				strcpy(tbuf, "");
				sprintf(tbuf, "chmod 600 %s %s%s.bak~", current->tfile, BACKUP, current->tfile);
				system(tbuf);
				send_statmsg("Backing up %s", current->kfile);
/*				send_statmsg("Rehashing KillList for \002%s\002", current->nick); */
				strcpy(tbuf, "");
				sprintf(tbuf, "cp %s %s%s.bak~", current->kfile2, BACKUP, current->kfile2);
				system(tbuf);
				strcpy(tbuf, "");
				sprintf(tbuf, "chmod 600 %s %s%s.bak~", current->kfile2, BACKUP, current->kfile2);
				system(tbuf);
				read_timelist(&thebots[i].KillList2, thebots[i].kfile2);
				read_biglist(&thebots[i].KillList, thebots[i].kfile);
#endif
				read_seenlist(&thebots[i].SeenList, thebots[i].seenfile);
				read_biglist(&thebots[i].dList, thebots[i].dfile);
				for (chan = current->Channel_list; chan; chan = chan->next) {
					send_to_server( "WHO %s", chan->name);
				}
			}
			
			if (getthetime()-thebots[i].away_serv > (get_int_varc(NULL, SETAAWAY_VAR) * 60)) {
				if (current->am_Away == FALSE) {
					if (get_int_varc(NULL, TOGAAWAY_VAR)) {
						strcpy(buffer, randstring(AWAYFILE));
						send_to_server(AWAYFORM, buffer, time2away(getthetime()));
						current->am_Away = TRUE;
					}
				}
			}
			
			if (((num_min(getthetime()) % 5) == 0) && (getthetime()-thebots[i].anti_idle > (NOIDLETIME * 60))) {
				if (get_int_varc(NULL, TOGNOIDLE_VAR)) {
					chan = current->Channel_list;
					if (chan) {
						while (y <= 1) {
							for (chanuser = chan->users; chanuser; chanuser = chanuser->next) {
								y++;
							}
							if (y <= 1) {
								chan = chan->next;
								if (chan) {
									y = 0;
								}
								else {
									chan = current->Channel_list;
									strcpy(buffer, "");
									strcpy(buffer, randstring(RANDSAYFILE));
									sendprivmsg(chan->name, "%s", buffer);
									send_statmsg("[\002Anti-Idle\002] \037Error\037 No Target for \002PING\002");
									send_statmsg("[\002Anti-Idle\002] Sending Random \002PRIVMSG\002 to \002%s\002", chan->name);
									current->anti_idle = getthetime();
									return;
								}
							}
						}
						z = randx(1, (y - 1));
						chanuser = chan->users;
						y = 1;
						while (chanuser) {
							if (z == y) {
								send_ctcp(chanuser->nick, "%s %li", "PING", getthetime());
								send_statmsg("[\002Anti-Idle\002] Sending \002PING\002 to \002%s\002", chanuser->nick);
								break;
							}
							chanuser = chanuser->next;
							y++;
						}
					}
					else {
						send_statmsg("[\002Anti-Idle\002] \037Error\037 Not active on any channel \002Anti-Idle\002 Failed");
						log_it(ERR_LOG_VAR, 0, "Anti-Idle Failed [Not active on any channels]");
					}
					current->anti_idle = getthetime();
				}
			}
			
#ifdef SERVER_MONITOR
			if ((num_hour(getthetime()) == 0) && (num_min(getthetime()) == 0) && (getthetime()-thebots[i].day_timer > 60)) {
				thebots[i].day_timer = getthetime();
				thebots[i].today_klines = 0;
				thebots[i].today_kills = 0;
				thebots[i].today_connections = 0;
				thebots[i].today_exits = 0;
				thebots[i].today_rejects = 0;
				thebots[i].today_invu = 0;
				thebots[i].today_conf = 0;
				thebots[i].today_ucon = 0;
				thebots[i].today_links = 0;
				thebots[i].today_flood = 0;
				thebots[i].today_ipmis = 0;
				thebots[i].trace_tog = 0;
				thebots[i].trace_kill_tog = 0;
			}
#endif
		}
}

void parse_server_input(read_fds)
/*
 * If read_fds is ready, this routine will read a line from the
 * the server and parse it.
 */
fd_set *read_fds;
{
	char    line[HUGE];
	int     i;

	for (i=0; i<MAXBOTS; i++)
	{
		if (thebots[i].created && thebots[i].server_sock != -1)
		{
			current = &(thebots[i]);
			if ( FD_ISSET( current->server_sock, read_fds ) )
				if (readln( line ) > 0 )
					parseline(line);
				else
				{
#ifdef DBUG
					debug(ERROR, "parse_server_input(): Server read FAILED!");
#endif
					close(current->server_sock);
					current->server_sock = -1;
				}
		}
	}
}

#ifdef USE_CCC

void parse_ccc_input(read_fds)
fd_set *read_fds;
{
        char line[HUGE];

	if (ccc_sock != -1)
	{
		if (FD_ISSET(ccc_sock, read_fds))
		{
			if (read_from_socket(ccc_sock, line) > 0 )
				parsecccline(line);
			else
			{
				close(ccc_sock);
				ccc_sock = -1;
			}
		}
	}
}

#endif

void set_dcc_fds(read_fds, write_fds)
fd_set *read_fds;
fd_set *write_fds;
{
	int     i;

	for(i=0; i<MAXBOTS; i++)
	{
		if (thebots[i].created)
		{
			current = &(thebots[i]);
			dccset_fds(read_fds, write_fds);
		}
	}
}

void parse_dcc_input(read_fds)
/*
 * Scan all bots for dcc input
 */
fd_set *read_fds;
{
	int     i;

	for (i=0; i<MAXBOTS; i++)
	{
		if (thebots[i].created)
		{
			current = &(thebots[i]);
			parse_dcc(read_fds);
		}
	}
}

void send_pings(void)
/*
 * Send a ping to all servers and set lastping to NOW
 */
{
	int     i;

	for (i=0; i<MAXBOTS; i++)
	{
		if (thebots[i].created)
		{
			current = &(thebots[i]);
			if (getthetime() - current->lastping /*send*/ >
				PINGSENDINTERVAL)
			{
#ifdef DBUG
				debug(NOTICE,
					 "send_pings(): Sending ping to server %s[%s]",
					 current->serverlist[
					 current->current_server].realname,
					 current->serverlist[
					 current->current_server].name);
#endif
				sendping(current->serverlist[current->current_server].realname);
				current->server_ok = FALSE;
				current->lastping = getthetime();
			}
		}
	}
}

void pong_received(nick, server)
/*
 * Pong received from current->serverlist[current->current_server]
 * Server must be ok.
 */
char *nick;
char *server;
{
	current->server_ok = TRUE;
	current->serv_lag = (getthetime() - current->lastping);
#ifdef DBUG
	debug(NOTICE, "pong_received(): server %s[%s] ok",
	current->serverlist[current->current_server].realname,
	current->serverlist[current->current_server].name);
#endif
}

/* Global commands */
struct
{
	char    *name;
	void    (*function)(char*, char*);
	int     userlevel;
	int     forcedcc;
} global_cmds[] =
{
	{ "LIST",       global_list,     99,    TRUE    },
	{ "INFO",       global_list,     99,    TRUE    },
	{ "DIE",        global_die,      99,    FALSE   },
	{ "DEBUG",      global_debug,    99,    FALSE   },
	{ "RAWMSG",     global_rawmsg,   99,    FALSE   },
	{ NULL,         (void(*)())(NULL) }
};

void parse_global(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	int      i;
	char     *command;
	aDCC *dccclient;

	if (!(command = get_token(&rest, " ")))
	{
		send_to_user(from, "\002Global expects a subcommand!\002");
		return;
	}

	for (i=0; global_cmds[i].name; i++)
		if(!my_stricmp(global_cmds[i].name, command))
		{
			if ((get_userlevel(from, "*") < global_cmds[i].userlevel) ||
				 get_shitlevel(from, "*") > 0)
			{
				send_to_user(from, "\002Incorrect levels\002");
				return;
			}
			dccclient = search_list("chat", from, DCC_CHAT);
			if (global_cmds[i].forcedcc &&
			  (!dccclient || (dccclient->flags&DCC_WAIT)))
			{
#ifdef AUTO_DCC
				dcc_chat(from, rest);
				sendnotice(getnick(from), "Please type: /dcc chat %s",
					current->nick);
				sendnotice(getnick(from),
					"After that, please retype: /msg %s GLOBAL %s %s",
					current->nick, command,
					rest?rest:"");
#else
				sendnotice(getnick(from),
					"Sorry, GLOBAL %s is only available through DCC Chat",
					command);
				sendnotice(getnick(from),
					"Please start a dcc chat connection first");
#endif
				return;
			}
				global_cmds[i].function(from, *rest?rest:NULL);
				return;
		}
	send_to_user(from, "Unknown global command");
	return;
}

void global_not_imp(from, rest)
char *from;
char *rest;
{
	send_to_user(from, "Global command not implemented");
}

void global_debug(from, rest)
char *from;
char *rest;
{
#ifdef DBUG
	if (!rest)
	{
		send_to_user(from, "\002Pls specify a level between 0 and 2\002");
		return;
	}
	if (set_debuglvl(atoi(rest)))
		send_to_user(from, "\002Debug set to debuglevel %d\002", atoi(rest));
	else
		send_to_user(from, "\002Could not set debuglevel\002");
#else
	send_to_user(from, "This version was not compiled with debugging enabled");
#endif
}

void global_die(from, rest)
char *from;
char *rest;
{
	aBot *bot;

	if (rest)
	{
		if ((bot=find_bot(rest)) != NULL)
		{
			current = bot;
			signoff(from, randstring(SIGNOFFSFILE));
			if (number_of_bots == 0)
				MyExit(0);
		}
		else
			send_to_user(from, "\002No such bot\002");
	}
	else
	{
		kill_all_bots("Global die");
		MyExit(0);
	}
}

void global_list(char *from, char *rest)
{
	int        i;
	aChannel *chan;
	aBot    *bot;

	rest = stripl(rest, " ");

	if (rest && (bot = find_bot(rest)))
	{
		send_to_user(from, "\002nick\002 %-10s, \002realnick\002 %s",
			bot->nick, bot->realnick);
		send_to_user(from, "\002current server\002: %s (%s), \002port\002 %d",
			bot->serverlist[current->current_server].name,
			bot->serverlist[current->current_server].realname,
			bot->serverlist[current->current_server].port);
		send_to_user(from, "------------------------------------------");
		send_to_user(from, "%d servers in list:", bot->num_of_servers);
		for (i=0; i<bot->num_of_servers; i++)
		{
			send_to_user(from, "\002server\002 %-32s \002port\002 %d",
			bot->serverlist[i].name, bot->serverlist[i].port);
		}
		/* show_channellist(from); won't work. It'll show the channellist
			of current. And setting current to thebots[i] won't work
			either, the server_sock for sending will point to the wrong
			server. */
		if (!bot->Channel_list)
			send_to_user(from, "Not active on any channels!");
		else
			for (chan = bot->Channel_list; chan; chan = chan->next)
				send_to_user(from, "\002  Active on:\002 %s", chan->name);
		send_to_user(from, "\002Upload dir:\002   %s", bot->uploaddir);
		send_to_user(from, "\002Download dir:\002 %s", bot->downloaddir);
		return;
	}
	/* else */
	send_to_user(from, "Total number:  %d", MAXBOTS);
	send_to_user(from, "Total running: %d", number_of_bots);
	send_to_user(from, "Total free:    %d", MAXBOTS-number_of_bots);
	send_to_user(from, "Started:       %-20.20s", time2str(uptime));
	send_to_user(from, "Uptime:        %-30s", idle2str(getthetime()-uptime));
	send_to_user(from, "------------------------------------------");
	send_to_user(from, "Num nickname  current server");
	for (i=0; i<MAXBOTS; i++)
		if (thebots[i].created)
			send_to_user(from, "%2d: %-9s %s", i, thebots[i].nick,
			thebots[i].serverlist[thebots[i].current_server].realname);
		else
			send_to_user(from, "%2d: Free", i);
	send_to_user(from, "------------------------------------------");
}

void global_rawmsg(from, rest)
char *from;
char *rest;
{
	aBot *bot, *tbot;
	char *temp, *ptemp;
	char msg[MAXLEN];
	
	temp = get_token(&rest, " ");

	while (ptemp = get_token(&rest, " ")) {
		sprintf(msg, "%s %s", msg, ptemp);
	}
	
	if (rest && (bot = find_bot(temp))) {
		tbot = current;
		current = bot;
		on_msg(from, bot->nick, msg);
		current = tbot;
	}
	else {
		send_to_user(from, "\002Could not find bot %s\002", temp);
	}
}

int forkbot(from, nick, rest)
char *from;
char *nick;
char *rest;
/*
 * Adds bot to list and starts it.
 * except for nick, login and name the bot is an identical
 * copy of its "parent"
 */
{
	aChannel *c_list;
	aBot *newbot;
	aBot *oldbot;
	char *option, *login, *channel, *server, *cmdchar, *ircname;
	int i, error;

	error = 0;
	cmdchar = ircname = login = server = channel = NULL;
	oldbot = current;
	while (rest && *rest)
	{
		option = get_token(&rest, " ");
		if (*option == '-')
		{
			option++;
			switch(*option)
			{
				case 'u':
					login = get_token(&rest, " ");
					break;
				case 'i':
					ircname = rest;
					rest = NULL;
					break;
				case 's':
					server = get_token(&rest, " ");
					break;
				case 'c':
					channel = get_token(&rest, " ");
					break;
				case 'z':
					cmdchar = get_token(&rest, " ");
					break;
				default:
					error++;
			}
		}
	}
	if (error)
	{
		send_to_user(from, "\002%s\002", "Invalid arguments given");
		return FALSE;
	}
	if (!cmdchar)
		cmdchar = &current->cmdchar;
	if ((newbot = add_bot(nick)) != NULL)
	{
		nick[9] = '\0';
		strcpy(newbot->nick, nick);
		strcpy(newbot->realnick, nick);
		strcpy(newbot->modes_to_send, current->modes_to_send);
		strcpy(newbot->USERFILE, current->USERFILE);
		if (login && (strlen(login) > MAXBOTNICKLEN))
			login[9] = '\0';
		strcpy(newbot->login, login?login:current->login);
		strcpy(newbot->ircname, ircname?ircname:current->ircname);
		newbot->cmdchar = (cmdchar && *cmdchar) ? *cmdchar : current->cmdchar;
		for (i=0; i < current->num_of_servers; i++)
			newbot->serverlist[i] = current->serverlist[i];
		newbot->num_of_servers = current->num_of_servers;
		newbot->SpyMsgList = current->SpyMsgList;
		newbot->StatMsgList = current->StatMsgList;
		newbot->StatMsgList = current->StatMsgList;
		newbot->StealList = current->StealList;
		newbot->Userlist = current->Userlist;
		c_list = current->Channel_list;
		current = newbot;
		if (server)
		{
			add_server(server, 6667);
			set_server(server, 6667);
		}
		if (!channel)
			copy_channellist(c_list);
		else
			join_channel(channel, "", FALSE);
		connect_to_server();
		return 1;
	}
	return 0;
}
