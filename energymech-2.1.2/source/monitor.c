#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/time.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>

#include "global.h"
#include "config.h"
#include "defines.h"
#include "structs.h"
#include "h.h"

#ifdef SERVER_MONITOR

struct
{
	char    *name;
	void    (*function)(char *from, char *rest);
} parse_servn[] =
{
	{ "*added K-Line*",				parse_serv_nkline	},
	{ "*Client connecting:*",		parse_serv_con		},
	{ "*Client exiting:*",			parse_serv_exit		},
	{ "*Rejecting idle*",			parse_serv_rejecti	},
	{ "*Rejecting*Bot:*",			parse_serv_rejectb	},
	{ "*Invalid username:*",		parse_serv_invu		},
	{ "*connect failure:*",			parse_serv_conf		},
	{ "*IP# Mismatch:*",			parse_serv_ipmis	},
	{ "*Kill line active*",			parse_serv_nkline	},
	{ "*unauthorized connection*",	parse_serv_ucon		},
	{ "*LINKS requested*",			parse_serv_links	},
	{ "*excess flood:*",			parse_serv_flood	},
	{ "*Received KILL message for*",parse_serv_kill		},
	{ "",         					(void(*)())(NULL) 	}
};
/* Colon removed from rest */
/* That sounds odd doesn't it */

void parse_server_notice(char *from, char *rest)
{
	int i;
	
#ifdef DBUG
				debug(NOTICE, "parse_server_notice(%s, %s)", from, rest);
#endif
	
	
/* If the server doesn't match the one were even on return */
	if (my_stricmp(from, current->serverlist[current->current_server].realname)) {
#ifdef DBUG
				debug(NOTICE, "exiting - parse_server_notice(...) [servers don't match]");
#endif
		return;
	}
	
/*  No numerics for checking what to do, just gonna have to use heavy duty
 *  matching techniques.
 */

	for (i = 0; parse_servn[i].name[0]; i++) {
		if (!matches(parse_servn[i].name, rest)) {
			parse_servn[i].function(from, rest);
			return;
		}
	}
#ifdef DBUG
				debug(NOTICE, "parse_server_notice(...) [No matches found...]");
#endif
}

void parse_serv_nkline(char *from, char *rest)
{
	char *aptr, *aptr2;
	char tbuf[MAXLEN];
	aTime *TVar;
	
	aptr = strcpy(tbuf, rest);
	aptr2 = get_token(&aptr, " ");
	
	if (!strcmp(aptr2, "***")) {
		get_token(&rest, " ");
		get_token(&rest, " ");
		get_token(&rest, " ");
	}
	
	log_it(MON_KLINE_LOG_VAR, 0, "%s", rest);
	
	current->total_klines++;
	current->today_klines++;
	
	if (TVar = find_time(&current->totalList, "KLINE")) {
		TVar->num++;
	}
	else {
		make_time(&current->totalList, "KLINE");
	}
}

void parse_serv_con(char *from, char *rest)
{
	char *aptr, *aptr2, *aptr3;
	char tbuf[MAXLEN];
	int varlen;
	aTime *aTimeVar, *TVar;
	
	current->total_connections++;
	current->today_connections++;
	
/* Throw away the "*** Notice --" junk */
	get_token(&rest, " ");
	get_token(&rest, " ");
	get_token(&rest, " ");
	
	log_it(MON_CON_LOG_VAR, 0, "%s", rest);
	
	get_token(&rest, ":");
	aptr = get_token(&rest, "(");
	aptr2 = get_token(&rest, " ");
	
#ifdef DBUG
				debug(NOTICE, "parse_serv_con(...) aptr = %s aptr2 = %s", aptr, aptr2);
#endif
	
	varlen = strlen(aptr2);
	
#ifdef DBUG
				debug(NOTICE, "parse_serv_con(...) varlen = %i", varlen);
#endif
	
	aptr2[(varlen - 1)] = '\0';
	
	sprintf(tbuf, "%s!%s", aptr, aptr2);
	
	aptr3 = format_uh(tbuf, 0);
	
#ifdef DBUG
				debug(NOTICE, "parse_serv_con(...) aptr3 = %s", aptr3);
#endif
	
	if (aTimeVar = find_time(&current->conList, aptr3)) {
		aTimeVar->num++;
	}
	else {
		aTimeVar = make_time(&current->conList, aptr3);
	}
	
	if (aTimeVar->num > 1) {
		send_monitormsg("[\002Clone\002] \002%s\002 connecting with \002%i\002 other clone(s)", tbuf, (aTimeVar->num - 1));
	}
#ifdef DBUG
				debug(NOTICE, "exiting - parse_serv_con(...)");
#endif

	if (TVar = find_time(&current->totalList, "CON")) {
		TVar->num++;
	}
	else {
		make_time(&current->totalList, "CON");
	}
}

void parse_serv_exit(char *from, char *rest)
{
	char *aptr, *aptr2, *aptr3;
	char tbuf[MAXLEN];
	int varlen;
	aTime *aTimeVar, *TVar;
	
	current->total_exits++;
	current->today_exits++;
	
/* Throw away the "*** Notice --" junk */
	get_token(&rest, " ");
	get_token(&rest, " ");
	get_token(&rest, " ");
	
	log_it(MON_EXIT_LOG_VAR, 0, "%s", rest);
	
	get_token(&rest, ":");
	aptr = get_token(&rest, "(");
	aptr2 = get_token(&rest, " ");
	
	varlen = strlen(aptr2);
	
	aptr2[(varlen - 1)] = '\0';
	
	sprintf(tbuf, "%s!%s", aptr, aptr2);
	
	aptr3 = format_uh(tbuf, 0);
	
	if (aTimeVar = find_time(&current->conList, aptr3)) {
		aTimeVar->num--;
		if (aTimeVar->num == 0) {
			remove_time(&current->conList, aTimeVar);
		}
	}
	
	if (TVar = find_time(&current->totalList, "EXIT")) {
		TVar->num++;
	}
	else {
		make_time(&current->totalList, "EXIT");
	}
}

void parse_serv_rejecti(char *from, char *rest)
{
	char *aptr, *aptr2, *aptr3;
	char tbuf[MAXLEN], abuf[MAXLEN];
	int varlen;
	aTime *aTimeVar, *TVar;
	
	current->total_rejects++;
	current->today_rejects++;
	
/* Throw away the "*** Notice --" junk */
	get_token(&rest, " ");
	get_token(&rest, " ");
	get_token(&rest, " ");
	
	log_it(MON_REJECT_LOG_VAR, 0, "%s", rest);
	
	aptr = get_token(&rest, " ");
	aptr2 = get_token(&rest, " ");
	
	while (!strchr(aptr2, '@')) {
		aptr = aptr2;
		aptr2 = get_token(&rest, " ");
	}
	
	
	varlen = strlen(aptr2);
	
	aptr2[(varlen - 1)] = '\0';
	aptr2++;
	
	sprintf(tbuf, "%s!%s", aptr, aptr2);
	
	aptr3 = format_uh(tbuf, 0);
	
	if (aTimeVar = find_time(&current->rejectList, aptr3)) {
		aTimeVar->num++;
		if (aTimeVar->num == 5) {
			send_monitormsg("[\002Idle Reject\002] Possible bot \002%s\002, \002%i\002 connect attempts in %li seconds", tbuf, aTimeVar->num, (getthetime() - aTimeVar->time));
			log_it(MON_RECKLINE_LOG_VAR, 0, "[Idle Reject] Possible bot %s, %i connect attempts in %li seconds", tbuf, aTimeVar->num, (getthetime() - aTimeVar->time));
		}
		else {
			if (aTimeVar->num > 5) {
				send_monitormsg("[\002Idle Reject\002] \002%s\002 rejected by server, \002%i\002 connect attempts in %li seconds", tbuf, aTimeVar->num, (getthetime() - aTimeVar->time));
			}
		}
	}
	else {
		aTimeVar = make_time(&current->rejectList, aptr3);
	}
	
	if (TVar = find_time(&current->totalList, "REJECT")) {
		TVar->num++;
	}
	else {
		make_time(&current->totalList, "REJECT");
	}
}

void parse_serv_rejectb(char *from, char *rest)
{
	char *aptr, *aptr2, *aptr3;
	char tbuf[MAXLEN], abuf[MAXLEN];
	int varlen, varlen2;
	aTime *aTimeVar, *TVar;
	
	current->total_exits++;
	current->today_exits++;
	
/* Throw away the "*** Notice --" junk */
	get_token(&rest, " ");
	get_token(&rest, " ");
	get_token(&rest, " ");
	
	log_it(MON_REJECT_LOG_VAR, 0, "%s", rest);
	
	get_token(&rest, ":");
	aptr = get_token(&rest, " ");
		
	if (aptr2 = strrchr(aptr, '[')) {
		varlen = strlen(aptr);
		varlen2 = strlen(aptr2);
		aptr2[varlen2 - 1] = '\0';
		aptr2 = strcpy(abuf, aptr2);
		aptr2++;
		aptr[varlen - varlen2] = '\0';
		varlen = strlen(aptr);
		if (isspace(aptr[varlen])) {
			aptr[varlen - 1] = '\0';
		}
	}
	else {
		return;
	}
	
	sprintf(tbuf, "%s!%s", aptr, aptr2);
	
	aptr3 = format_uh(tbuf, 0);
	
	if (aTimeVar = find_time(&current->rejectList, aptr3)) {
		aTimeVar->num++;
		send_monitormsg("[\002Bot Reject\002] \002%s\002 rejected by server, \002%i\002 connect attempts in %li seconds", tbuf, aTimeVar->num, (getthetime() - aTimeVar->time));
	}
	else {
		aTimeVar = make_time(&current->rejectList, aptr3);
	}
	
	if (TVar = find_time(&current->totalList, "REJECT")) {
		TVar->num++;
	}
	else {
		make_time(&current->totalList, "REJECT");
	}
}

void parse_serv_invu(char *from, char *rest)
{
	char *aptr, *aptr2, *aptr3;
	char tbuf[MAXLEN];
	int varlen;
	aTime *aTimeVar, *TVar;
	
	current->total_invu++;
	current->today_invu++;
	
/* Throw away the "*** Notice --" junk */
	get_token(&rest, " ");
	get_token(&rest, " ");
	get_token(&rest, " ");
	
	log_it(MON_INVU_LOG_VAR, 0, "%s", rest);
	
	get_token(&rest, ":");
	aptr = get_token(&rest, " ");
	aptr2 = get_token(&rest, " ");
	aptr2++;
	
	varlen = strlen(aptr2);
	
	aptr2[(varlen - 1)] = '\0';
	
	sprintf(tbuf, "%s!%s", aptr, aptr2);
	
	aptr3 = format_uh(tbuf, 0);
	
	if (aTimeVar = find_time(&current->rejectList, aptr3)) {
		aTimeVar->num++;
		if (aTimeVar->num == 5) {
			send_monitormsg("[\002Invalid Username\002] Possible bot \002%s\002, \002%i\002 connect attempts in %li seconds", aptr2, aTimeVar->num, (getthetime() - aTimeVar->time));
			log_it(MON_RECKLINE_LOG_VAR, 0, "[Invalid Username] Possible bot %s, %i connect attempts in %li seconds", aptr2, aTimeVar->num, (getthetime() - aTimeVar->time));
		}
		else {
			send_monitormsg("[\002Invalid Username\002] \002%s\002 rejected by server, \002%i\002 connect attempts in %li seconds", tbuf, aTimeVar->num, (getthetime() - aTimeVar->time));
		}
	}
	else {
		aTimeVar = make_time(&current->rejectList, aptr3);
	}

	if (TVar = find_time(&current->totalList, "INVU")) {
		TVar->num++;
	}
	else {
		make_time(&current->totalList, "INVU");
	}
}

void parse_serv_conf(char *from, char *rest)
{
	char *aptr, *aptr2, *aptr3;
	char tbuf[MAXLEN], abuf[MAXLEN];
	int varlen, varlen2;
	aTime *aTimeVar, *TVar;
	
	current->total_conf++;
	current->today_conf++;
	
/* Throw away the "*** Notice --" junk */
	get_token(&rest, " ");
	get_token(&rest, " ");
	get_token(&rest, " ");
	
	log_it(MON_CONF_LOG_VAR, 0, "%s", rest);

/* Nicks could have the '[' or ']' char in them and there is no space between them :( */
	get_token(&rest, ":");
	aptr = get_token(&rest, " ");
		
	if (aptr2 = strrchr(aptr, '[')) {
		varlen = strlen(aptr);
		varlen2 = strlen(aptr2);
		aptr2[varlen2 - 1] = '\0';
		aptr2 = strcpy(abuf, aptr2);
		aptr2++;
		aptr[varlen - varlen2] = '\0';
	}
	else {
		return;
	}
	
	sprintf(tbuf, "%s!%s", aptr, aptr2);
	
	aptr3 = format_uh(tbuf, 0);
	
	if (aTimeVar = find_time(&current->rejectList, aptr3)) {
		aTimeVar->num++;
		if (aTimeVar->num == 5) {
			send_monitormsg("[\002Connection Failure\002] Possible bot \002%s\002, \002%i\002 connect attempts in %li seconds", aptr2, aTimeVar->num, (getthetime() - aTimeVar->time));
			log_it(MON_RECKLINE_LOG_VAR, 0, "[Connection Failure] Possible bot %s, %i connect attempts in %li seconds", aptr2, aTimeVar->num, (getthetime() - aTimeVar->time));
		}
		else {
			send_monitormsg("[\002Connection Failure\002] \002%s\002 rejected by server, \002%i\002 connect attempts in %li seconds", tbuf, aTimeVar->num, (getthetime() - aTimeVar->time));
		}
	}
	else {
		aTimeVar = make_time(&current->rejectList, aptr3);
	}
	
	if (TVar = find_time(&current->totalList, "CONF")) {
		TVar->num++;
	}
	else {
		make_time(&current->totalList, "CONF");
	}
}

void parse_serv_ipmis(char *from, char *rest)
{
	char *aptr, *aptr2;
	aTime *TVar;
	
	current->total_ipmis++;
	current->today_ipmis++;
	
/* Throw away the "*** Notice --" junk */
	get_token(&rest, " ");
	get_token(&rest, " ");
	get_token(&rest, " ");
	
	log_it(MON_IPMIS_LOG_VAR, 0, "%s", rest);
	
	if (TVar = find_time(&current->totalList, "IPMIS")) {
		TVar->num++;
	}
	else {
		make_time(&current->totalList, "IPMIS");
	}
}

void parse_serv_ucon(char *from, char *rest)
{
	aTime *TVar;
	
	current->total_ucon++;
	current->today_ucon++;
	
/* Throw away the "*** Notice --" junk */
	get_token(&rest, " ");
	get_token(&rest, " ");
	get_token(&rest, " ");
	
	log_it(MON_UCON_LOG_VAR, 0, "%s", rest);
	
	if (TVar = find_time(&current->totalList, "UCON")) {
		TVar->num++;
	}
	else {
		make_time(&current->totalList, "UCON");
	}
}

void parse_serv_links(char *from, char *rest)
{
	aTime *TVar;
	
	current->total_links++;
	current->today_links++;
	
/* Throw away the "*** Notice --" junk */
	get_token(&rest, " ");
	get_token(&rest, " ");
	get_token(&rest, " ");
	
	log_it(MON_LINKS_LOG_VAR, 0, "%s", rest);
	
	if (TVar = find_time(&current->totalList, "LINKS")) {
		TVar->num++;
	}
	else {
		make_time(&current->totalList, "LINKS");
	}
}

void parse_serv_flood(char *from, char *rest)
{
	aTime *TVar;
	
	current->total_flood++;
	current->today_flood++;
	
/* Throw away the "*** Notice --" junk */
	get_token(&rest, " ");
	get_token(&rest, " ");
	get_token(&rest, " ");
	
	log_it(MON_FLOOD_LOG_VAR, 0, "%s", rest);
	
	if (TVar = find_time(&current->totalList, "FLOOD")) {
		TVar->num++;
	}
	else {
		make_time(&current->totalList, "FLOOD");
	}
}

void parse_serv_kill(char *from, char *rest)
{
	aTime *TVar;
	
	char *prey, *predator, *killmsg;
	char tbuf[MAXLEN];
	aTime *aTimeVar;
	
	strcpy(tbuf, "");
	
	current->total_kills++;
	current->today_kills++;
	
/* Throw away the "*** Notice --" junk */
	get_token(&rest, " ");
	get_token(&rest, " ");
	get_token(&rest, " ");
	
	get_token(&rest, " "); /* Received */
	get_token(&rest, " "); /* KILL */
	get_token(&rest, " "); /* message */
	get_token(&rest, " "); /* for */
	
	prey = get_token(&rest, ".");
	
	get_token(&rest, " "); /* From */
	
	predator = get_token(&rest, " ");
	
	get_token(&rest, " "); /* Path: */
	get_token(&rest, " "); /* the actual path */
	
	killmsg = get_token(&rest, " ");
	
	while (killmsg) {
		sprintf(tbuf, "%s %s", tbuf, killmsg);
		killmsg = get_token(&rest, " ");
	}
	
	if (!matches("*.*", predator)) {
		make_biglist(&current->KillList, predator, getthetime(), 1, prey, tbuf);	
	}
	else {
		make_biglist(&current->KillList, predator, getthetime(), -20, prey, tbuf);
	}
	
	if (aTimeVar = find_time(&current->KillList2, predator)) {
		aTimeVar->num++;
	}
	else {
		make_time(&current->KillList2, predator);
	}
	
	if (TVar = find_time(&current->totalList, "KILLS")) {
		TVar->num++;
	}
	else {
		make_time(&current->totalList, "KILLS");
	}
}

struct
{
	char    *name;
	void    (*function)(char *from, char *to, char *rest, int cmdlevel);
} parse_monitor[] =
{
	{ "SERVKILLS",			do_monitor_servkills},
	{ "KILLSFOR",			do_monitor_killsfor	},
	{ "KILLSFROM",			do_monitor_killsfrom},
	{ "TOPKILLS",			do_monitor_topkills	},
	{ "CLONES",				do_monitor_clones	},
	{ "STATS",				do_monitor_stats	},
	{ "BOTS",				do_monitor_bots		},
	{ "HELP",				do_monitor_help		},
	{ "KLINECLONES",		do_monitor_klinec	},
	{ "KILLCLONES",			do_monitor_killc	},
	{ "REHASH",				do_monitor_rehash	},
	{ "KILLBOTS",			do_monitor_killb	},
	{ "KLINEBOTS",			do_monitor_klineb	},
	{ "",         			(void(*)())(NULL) 	}
};

void do_monitor(char *from, char *to, char *rest, int cmdlevel)
{
	char *command;
	int i;
	
	if (!rest) {
		do_monitor_help(from, to, rest, cmdlevel);
		return;
	}
		
	command = get_token(&rest, " ");
	
	for (i = 0; parse_monitor[i].name[0]; i++) {
		if (!my_stricmp(parse_monitor[i].name, command)) {
			parse_monitor[i].function(from, to, rest, cmdlevel);
			return;
		}
	}
	
	send_to_user(from, "Unknown Monitor Command \002%s\002", command);
}

void do_monitor_servkills(char *from, char *to, char *rest, int cmdlevel)
{
	aBigList *aVar;
	
	for (aVar = current->KillList; aVar; aVar = aVar->next) {
		if (!matches("*.*", aVar->user)) {
			send_to_user(from, "[\002Server Kills\002] %30s: %7li", aVar->user, aVar->num);
		}
	}
}

void do_monitor_killsfor(char *from, char *to, char *rest, int cmdlevel)
{
	aBigList *aVar;
	int i = 0, j = - 1, k = 1;
	char *pattern, *num;
	
	if (!rest) {
		send_to_user(from, "\002Enter a pattern to search for\002");
	}
	
	pattern = get_token(&rest, " ");
	if (num = get_token(&rest, " ")) {
		if (isdigit(*num)) {
			j = atoi(num);
		}
		else if (!my_stricmp(num, "ALL")) {
			k = 0;
		}
	}
	
	for (aVar = current->KillList; aVar; aVar = aVar->next) {
		if (!matches(pattern, aVar->data1)) {
			i++;
			if (k) {
				send_to_user(from, "[\002%s\002] (\002%s\002) Killed \002%s\002 %s", time2small(aVar->time), aVar->user, aVar->data1, aVar->data2);
				if ((i > j) && (j != -1)) {
					send_to_user(from, "-- \002%i\002 Kill Limit reached", j);
				}
			}
		}
	}
	send_to_user(from, "-- Killed \002%i\002 times", i);
}

void do_monitor_killsfrom(char *from, char *to, char *rest, int cmdlevel)
{
	aBigList *aVar;
	int i = 0, j = - 1, k = 1;
	char *pattern, *num;
	
	if (!rest) {
		send_to_user(from, "\002Enter a pattern to search for\002");
	}
	
	pattern = get_token(&rest, " ");
	if (num = get_token(&rest, " ")) {
		if (isdigit(*num)) {
			j = atoi(num);
		}
		else if (!my_stricmp(num, "ALL")) {
			k = 0;
		}
	}
	
	for (aVar = current->KillList; aVar; aVar = aVar->next) {
		if (!matches(pattern, aVar->user)) {
			i++;
			if (k) {
				send_to_user(from, "[\002%s\002] (\002%s\002) Killed \002%s\002 %s", time2small(aVar->time), aVar->user, aVar->data1, aVar->data2);
				if ((i > j) && (j != -1)) {
					send_to_user(from, "-- \002%i\002 Kill Limit reached", j);
				}
			}
		}
	}
	send_to_user(from, "-- Killed \002%i\002 times", i);
}

void do_monitor_topkills(char *from, char *to, char *rest, int cmdlevel)
{
	char *Nicks[11];
	time_t Times[11];
	int Num[11];
	int count, done, i, ii;
	aTime *aVar;
	
	for (i=0;i<10;i++)
	{
		Nicks[i] = NULL;
		Num[i] = 0;
		Times[i] = 0;
	}
	count = 0;
	for (aVar = current->KillList2; aVar; aVar = aVar->next)
	{
		if (matches("*.*", aVar->name)) {
			done=0;
			for(i=0;i<10;i++)
			{
				if (!done && (aVar->num > Num[i]))
				{
					count++;
					for(ii=9;ii>i;ii--)
					{
						Nicks[ii] = Nicks[ii-1];
						Num[ii] = Num[ii-1];
						Times[ii] = Times[ii-1];
					}
					done++;
					Nicks[i] = aVar->name;
					Num[i] = aVar->num;
					Times[i] = aVar->time;
				}
			}
		}
	}
	if (count > 10)
		count = 10;
	for (i=0;i<count;i++)
		send_to_user(from, "\002%2i) %5i: %s\002",
			i+1, Num[i], Nicks[i]);
	if (!count)
		send_to_user(from, "\002No one has been killed\002");
}

void do_monitor_clones(char *from, char *to, char *rest, int cmdlevel)
{
	aTime *aTimeVar;
	
	for (aTimeVar = current->conList; aTimeVar; aTimeVar = aTimeVar->next) {
		if (aTimeVar->num > 1) {
			send_to_user(from, "[\002Clones\002] %s with %i total connections", aTimeVar->name, aTimeVar->num);
		}
	}
}

void do_monitor_bots(char *from, char *to, char *rest, int cmdlevel)
{
	aTime *aTimeVar;
	
	for (aTimeVar = current->botList; aTimeVar; aTimeVar = aTimeVar->next) {
		send_to_user(from, "[\002Bots\002] %s with %i total connections", aTimeVar->name, aTimeVar->num);
	}
}

void do_monitor_stats(char *from, char *to, char *rest, int cmdlevel)
{
	aTime *aTimeVar;
	int i = 0, j = 0, debug = 0;
	
	for (aTimeVar = current->conList; aTimeVar; aTimeVar = aTimeVar->next) {
/* Debug */		
/*		send_to_user(from, "[\002Debug Con List\002] %s %i", aTimeVar->name, aTimeVar->num); */
		if (aTimeVar->num > 1) {
			i += aTimeVar->num;
		}
	}
	
	for (aTimeVar = current->botList; aTimeVar; aTimeVar = aTimeVar->next) {
		j++;
	}
	
	send_to_user(from, "[\002Stats\002] Monitor Statistics as of \037%s\037", time2str(getthetime()));
	send_to_user(from, "[\002Stats\002] Today Stats Record Started at \037%s\037", time2str(current->day_timer));
	send_to_user(from, "[\002Stats\002] Clones: \002%-3i\002 Bots: \002%-3i\002", i, j);
	send_to_user(from, "[\002Stats\002] Total K-Lines:     \002%6i\002 K-Lines Today:     \002%6i\002", current->total_klines, current->today_klines);
	send_to_user(from, "[\002Stats\002] Total Kills:       \002%6i\002 Kills Today:       \002%6i\002", current->total_kills, current->today_kills);
	send_to_user(from, "[\002Stats\002] Total Connections: \002%6i\002 Connections Today: \002%6i\002", current->total_connections, current->today_connections);
	send_to_user(from, "[\002Stats\002] Total Exits:       \002%6i\002 Exits Today:       \002%6i\002", current->total_exits, current->today_exits);
	send_to_user(from, "[\002Stats\002] Total Rejects:     \002%6i\002 Rejects Today:     \002%6i\002", current->total_rejects, current->today_rejects);
	send_to_user(from, "[\002Stats\002] Total Inv u@h:     \002%6i\002 Inv u@h Today:     \002%6i\002", current->total_invu, current->today_invu);
	send_to_user(from, "[\002Stats\002] Total Connect Fail:\002%6i\002 Connect Fail Today:\002%6i\002", current->total_conf, current->today_conf);
	send_to_user(from, "[\002Stats\002] Total Unauth Cons: \002%6i\002 Unauth Cons Today: \002%6i\002", current->total_ucon, current->today_ucon);
	send_to_user(from, "[\002Stats\002] Total Floods:      \002%6i\002 Floods Today:      \002%6i\002", current->total_flood, current->today_flood);
	send_to_user(from, "[\002Stats\002] Total IP Mismatchs:\002%6i\002 IP Mismatchs Today:\002%6i\002", current->total_ipmis, current->today_ipmis);
}

void do_monitor_help(char *from, char *to, char *rest, int cmdlevel)
{
	send_to_user(from, "[\002Help\002] __Monitor_Commands___________________________");
	send_to_user(from, "[\002Help\002] \002SERVKILLS\002 - Shows server kill statistics");
	send_to_user(from, "[\002Help\002] \002KILLSFOR\002  - Shows kills on a particular nick");
	send_to_user(from, "[\002Help\002] \002KILLSFROM\002 - Shows kills from an oper");
	send_to_user(from, "[\002Help\002] \002TOPKILLS\002  - Shows the top kills");
	send_to_user(from, "[\002Help\002] \002CLONES\002    - Lists current userhosts that are clones");
	send_to_user(from, "[\002Help\002] \002BOTS\002      - Lists current possible bots connected");
	send_to_user(from, "[\002Help\002] \002STATS\002     - Statistics for the server");
	send_to_user(from, "[\002Help\002] \002KLINEC\002    - K-Lines all userhosts that are clones");
	send_to_user(from, "[\002Help\002] \002KILLC\002     - Kills all userhosts that are clones");
	send_to_user(from, "[\002Help\002] \002KLINEB\002    - K-Lines all userhosts that are bots");
	send_to_user(from, "[\002Help\002] \002KILLB\002     - Kills are userhosts that are bots");
	send_to_user(from, "[\002Help\002] \002REHASH\002    - Rehashes all connection lists using TRACE");
	send_to_user(from, "[\002Help\002] \002HELP\002      - Your looking at it");
	send_to_user(from, "[\002Help\002] _____________________________________________");
}

void do_monitor_klinec(char *from, char *to, char *rest, int cmdlevel)
{
	aTime *aTimeVar;
	char abuf[MAXLEN];
	
	if (!current->isIRCop) {
		send_to_user(from, "\002Permission Denied- I'm not an IRC operator\002");
		return;
	}
	
	for (aTimeVar = current->conList; aTimeVar; aTimeVar = aTimeVar->next) {
		if (aTimeVar->num > 1) {
			sprintf(abuf, "%s Clones", aTimeVar->name);
			do_kline(from, to, abuf, cmdlevel);
		}
	}
}

void do_monitor_killc(char *from, char *to, char *rest, int cmdlevel)
{
	aTime *aTimeVar;
	char abuf[MAXLEN];
	
	if (!current->isIRCop) {
		send_to_user(from, "\002Permission Denied- I'm not an IRC operator\002");
		return;
	}
	
	current->trace_kill_tog = 1;
	current->trace_tog = 1;
	
	send_to_server("TRACE");
}

void do_monitor_klineb(char *from, char *to, char *rest, int cmdlevel)
{
	aTime *aTimeVar;
	char abuf[MAXLEN];
	
	if (!current->isIRCop) {
		send_to_user(from, "\002Permission Denied- I'm not an IRC operator\002");
		return;
	}
	
	for (aTimeVar = current->botList; aTimeVar; aTimeVar = aTimeVar->next) {
		if (aTimeVar->num > 1) {
			sprintf(abuf, "%s Bot", aTimeVar->name);
			do_kline(from, to, abuf, cmdlevel);
		}
	}
}

void do_monitor_killb(char *from, char *to, char *rest, int cmdlevel)
{
	aTime *aTimeVar;
	char abuf[MAXLEN];
	
	if (!current->isIRCop) {
		send_to_user(from, "\002Permission Denied- I'm not an IRC operator\002");
		return;
	}
	
	current->trace_kill_tog = 2;
	current->trace_tog = 1;
	
	send_to_server("TRACE");
}

void do_monitor_rehash(char *from, char *to, char *rest, int cmdlevel)
{
	aTime *aTimeVar;
	char abuf[MAXLEN];
	
	delete_time(&current->conList);
	current->conList = NULL;
	uptime_time(&current->rejectList, (getthetime() - 300));
	send_to_user(from, "[\002Monitor Rehash\002] Rehashing Connection List");
	current->trace_tog = 1;
	send_to_server("TRACE");
}

#endif

void send_monitormsg(char *format, ...)
{
	char buf[HUGE];
  	va_list msg;
  	aList *temp;
  
  	va_start(msg, format);
  	vsprintf(buf, format, msg);
  	va_end(msg);

  	for (temp = current->monitorlist; temp; temp = temp->next) {
	 	send_to_user(temp->name, "[\002%s\002] %s", time2medium(getthetime()), buf);
	}
}

void do_cumonitor(char *from, char *to, char *rest, int cmdlevel)
{
	if (find_list(&current->monitorlist, from) != NULL) {
		return;
	}
	if (make_list(&current->monitorlist, from) != NULL)
		send_monitormsg("(\002%s\002) Added to Mech Monitor List", getnick(from));
	else
		send_to_user(from, "\002%s\002", "Can not add you to Mech Core due to an error");
}

void do_rcumonitor(char *from, char *to, char *rest, int cmdlevel)
{
	aList *tmp;

	tmp = find_list(&current->monitorlist, from); 
	if (tmp)
	{
		remove_list(&current->monitorlist, tmp);
	}
}



