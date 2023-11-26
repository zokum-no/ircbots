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
#include <stdarg.h>
#include <unistd.h>
 
#include "global.h"
#include "config.h"
#include "defines.h"
#include "structs.h"
#include "h.h"
#include "commands.h"
#include "fnmatch.h"
#include "ftext.h"

void do_nsl(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *serv, *nick, *uh, *t;
	char buf[BUFSIZ];
	char lilbuf[BUFSIZ];
	FILE *ptr;
	int abuf;
	
	if (!rest)
	{
		send_to_user(from, "\002No nick, domain or IP input\002");
	}
	
	chan = get_channel(to, &rest);
	nick = get_token(&rest, " ");
	
	if (!(uh = find_userhost(from, nick))) {
		serv = nick;
	}
	else {
		serv = gethost(uh);
	}
	
	t = strchr(serv, ';');
	if (t && *t) {
		*t = '\0';
	}
	
	sprintf(lilbuf, "nslookup %s", serv);

	if ((ptr = popen(lilbuf, "r")) != NULL)
		while (fgets(buf, BUFSIZ, ptr) != NULL) {
			abuf = strlen(buf);
			buf[(abuf - 1)] = '\0';
			if (ischannel(to)) {
				sendprivmsg(chan, "[\002%s\002] %s", serv, buf);
			}
			else {
				send_to_user(from, "[\002%s\002] %s", serv, buf);
			}
		}
	pclose (ptr);
}

void do_finger(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *serv, *nick, *uh, *t;
	char buf[BUFSIZ];
	char lilbuf[BUFSIZ];
	FILE *ptr;
	int abuf;
	
	if (!rest)
	{
		send_to_user(from, "\002Please Enter a u@h to Finger\002");
		return;
	}
	
	chan = get_channel(to, &rest);
	nick = get_token(&rest, " ");
	
	serv = nick;
	
	t = strchr(serv, ';');
	if (t && *t) {
		*t = '\0';
	}
	
	sprintf(lilbuf, "finger %s", serv);

	if ((ptr = popen(lilbuf, "r")) != NULL)
		while (fgets(buf, BUFSIZ, ptr) != NULL) {
			abuf = strlen(buf);
			buf[(abuf - 1)] = '\0';
			if (ischannel(to)) {
				sendprivmsg(chan, "[\002%s\002] %s", serv, buf);
			}
			else {
				send_to_user(from, "[\002%s\002] %s", serv, buf);
			}
		}
	pclose (ptr);
}

void do_whois(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *serv, *nick, *uh, *t;
	char buf[BUFSIZ];
	char lilbuf[BUFSIZ];
	FILE *ptr;
	int abuf;
	
	if (!rest)
	{
		send_to_user(from, "\002No domain or IP input\002");
	}
	
	chan = get_channel(to, &rest);
	serv = get_token(&rest, " ");

	t = strchr(serv, ';');
	if (t && *t) {
		*t = '\0';
	}
	
	sprintf(lilbuf, "whois %s", serv);

	if ((ptr = popen(lilbuf, "r")) != NULL)
		while (fgets(buf, BUFSIZ, ptr) != NULL) {
			abuf = strlen(buf);
			buf[(abuf - 1)] = '\0';
			send_to_user(from, "[\002%s\002] %s", serv, buf);
		}
	pclose (ptr);
}

void do_addnews(char *from, char *to, char *rest, int cmdlevel)
{
	FILE	*news;
	
	if (!rest || !*rest)  {
		send_to_user (from, "Usage: \002ADDNEWS <your news>\002");
		return;
	}
	
	if (!(news = fopen(NEWS_FILE, "at")))  {
		send_to_user (from, "\002Error: couldn't open the news file.\002");
		return;
	}
	
	fprintf(news, "* \002%s\002\n   [Added by %s] %s\n", rest, getnick(from), time2str(getthetime()));
	fclose(news);
	send_to_user (from, "\002Thanks, your news has been added.\002");
	return;
}

void do_erasenews(char *from, char *to, char *rest, int cmdlevel)
{
	FILE *news;
	char buf[30];
	
	strcpy (buf, "rm -rf ");
	strcat (buf, NEWS_FILE);
	system (buf);
	if (!(news = fopen(NEWS_FILE, "a")))  {
		send_to_user (from, "\002Error: couldn't open the news file.\002");
		return;
	}

	fprintf(news, "@@@ \002%s's NEWS\002 @@@ Hot! Hot! Hot! @@@ Read now! @@@\n\n", current->nick);
	fclose(news);
	send_to_user(from, "\002All news has been erased.\002");
	return;
}

/* Internal proc that dumps the file given to the user specified.
   Kinda lame, but it works fine ;) */
   
void dump_file(char *from, char *filen, char *whatisit)
{
	FILE *file;
	char lin[MAXLEN];
	char *text;
	
	if ( !( file = fopen(filen, "r")) )  {
		send_to_user (from, "\002Couldn't open \"%s\"\002", whatisit);
		return;
	}
	while (freadln(file, lin))   {
		text = lin;
		send_to_user(from, "%s", text);
	}
	send_to_user (from, "\n\002### The End Of %s ###\002", whatisit);
	fclose ( file );
	return;
}

void do_news(char *from, char *to, char *rest, int cmdlevel)
{
	dump_file(from, NEWS_FILE, "News");
	return;
}

void do_addsite(char *from, char *to, char *rest, int cmdlevel)
{
	FILE	*sitez;
	char *site, *userpass;
	
	if (!rest || !*rest)  {
		send_to_user (from, "Usage: \002ADDSITE <address> <userid/pass> [<dir>]\002");
		return;
	}
	site = get_token (&rest, " ");

	if (!rest || !*rest)  {
		send_to_user (from, "Usage: \002ADDSITE <address> <userid/pass> [<dir>]\002");
		return;
	}
	userpass = get_token (&rest, " ");
	
	if (!(sitez = fopen(SITEZ_FILE, "at")))  {
		send_to_user (from, "\002Error: couldn't open the sitez file.\002");
		return;
	}
	
	fprintf(sitez, "\002%25s\002 | \002%15s\002 | %s\n", site, userpass, rest);
#ifndef ANON_SITEZ
	fprintf(sitez, " [Added by %s] %s\n", getnick(from), time2str(getthetime()));
#endif
	fclose(sitez);
	send_to_user (from, "\002Thanks, the site has been added\002");
	return;
}

void do_sitez(char *from, char *to, char *rest, int cmdlevel)
{
	dump_file(from, SITEZ_FILE, "Sitez");
	return;
}

void do_erasesitez(char *from, char *to, char *rest, int cmdlevel)
{
	FILE *sitez;
	
	if ( !(sitez = fopen(SITEZ_FILE, "w")) )  {
		send_to_user (from, "\002Error: couldn't open the sitez file.\002");
		return;
	}
	fprintf(sitez, "@@@ \002 %s's SiTEZ\002 @@@ Only Today! @@@ Only now! @@@\n",
					current->nick);
	fprintf(sitez, "@@@ Feel free to add your fav site! ADDSITE for details! @@@\n");
	fprintf(sitez, "\n--------------------------+-----------------+---------------------");
	fprintf(sitez, "\n      Site address        |  Login/Password |     Directory");
	fprintf(sitez, "\n--------------------------+-----------------+---------------------\n");
	fclose(sitez);
	send_to_user( from, "\002The sitez file has been erased.\002" );
	return;
}

void check_files(void)
{
	FILE *tmp;
	
	if ( !(tmp = fopen(NEWS_FILE, "r")) )  {
		printf("No news file was found. Creating it...\n");
		tmp = fopen(NEWS_FILE, "w");
		fprintf(tmp, "@@@ \002%s's NEWS\002 @@@ Hot! Hot! Hot! @@@ Read now! @@@\n\n", current->nick);
		fclose(tmp);
	} else
	fclose(tmp);
	if ( !(tmp = fopen(SITEZ_FILE, "r")) )  {
		printf("No sitez file was found. Creating it... ");
#ifdef ANON_SITEZ
		printf("[Anonymous mode]");
#endif
		printf("\n");
		tmp = fopen(SITEZ_FILE, "w");
		fprintf(tmp, "@@@ \002 %s's SiTEZ\002 @@@ Only Today! @@@ Only now! @@@\n",
					current->nick);
		fprintf(tmp, "@@@ Feel free to add your fav site! ADDSITE for details! @@@\n");
	}
	fclose(tmp);
	return;
}

void do_cd(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	if (ischannel(to)) {
		return;
	}
	if (search_list("chat",from,DCC_CHAT))
		do_chdir(from, rest?rest:"/");
}

void do_flist(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	FILE	*ls_file;
	char	indexfile[MAXLEN];
	char	*s;
	char	*p;
	
	if (ischannel(to)) {
		return;
	}

	strcpy(indexfile, current->indexfile );
	if(rest)
	{
		for(p=rest; *p; p++) *p=tolower(*p);
		sprintf(indexfile, "%s.%s", current->indexfile, rest);
		if((ls_file = openindex(from, indexfile)) == NULL)
		{
			send_to_user(from, "No indexfile for %s", rest);
			return;
		}
	}
	else
		if((ls_file = openindex(from, current->indexfile)) == NULL)
		{
			send_to_user(from, "No indexfile");
			return;
		}

	while((s=get_ftext(ls_file)))
		send_to_user(from, s);
	fclose(ls_file);
}	

void do_send(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char	*pattern;
	
	if (ischannel(to)) {
		return;
	}
	
	if (rest && strstr(rest,"* * ")!=NULL)
	{
		log_it(DANGER_LOG_VAR, 0, "%s tried to flood me off with send", from);

		if (search_list("chat",from,DCC_CHAT))
			send_to_user(from, "Lame attempt");
		return;
	}

	if(rest)
		while((pattern = get_token(&rest, " ")))
			send_file(from, pattern);
	else if (search_list("chat",from,DCC_CHAT))
		send_to_user(from, "Please specify a filename (use !files)");
}

void show_dir(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char	*pattern;
	
	if (ischannel(to)) {
		return;
	}

	if(rest)
		while((pattern = get_token(&rest, " ")))
		{
			if(*rest != '-')
				do_ls(from, pattern);
		}
	else
		do_ls(from, "");
}

void show_cwd(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	if (ischannel(to)) {
		return;
	}
	
	if (search_list("chat",from,DCC_CHAT))
		pwd(from);
}

void show_queue(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	if (ischannel(to)) {
		return;
	}
	
	do_showqueue();
}
		
extern	int	send_file_from_list(char*, char *, char *);

SESSION_list	*currentsession;

SESSION_list	*find_session(char *user)
{
	SESSION_list	*session;

	for(session = current->session_list; session; session = session->next)
		if(!my_stricmp(user, session->user))
			return session;
	return NULL;
}

SESSION_list	*find_session2(char *user)
{
	SESSION_list	*session;

	for(session = current->session_list; session; session = session->next)
		if(!my_stricmp(user, getnick(session->user)))
			return session;
	return NULL;
}

SESSION_list	*create_session(char *user)
/*
 * Initializes a session_struct and places it in the list
 */
{
	SESSION_list	*new_session;

	if((new_session = (SESSION_list *) 
	                   malloc(sizeof(SESSION_list))) == NULL)
		return NULL;
	mstrcpy(&new_session->user, user);
	mstrcpy(&new_session->cwd, "/");
	new_session->nodcc_to = NULL;
	new_session->nodcc_rest = NULL;
	new_session->currentfile = 0;
	new_session->maxfile = 0;
	new_session->last_received = time(NULL);
	new_session->flood_start = time(NULL);
	new_session->flood_cnt = 1;			/* one msg */
	new_session->flooding = FALSE;
	new_session->next = current->session_list;
	current->session_list = new_session;
	return new_session;
}

int	delete_session(char *user)
/*
 * Removes session from list
 */
{
	SESSION_list	*p=current->session_list;
	SESSION_list	*old=NULL;
	int	i;

	while (p!=NULL && !!my_stricmp(p->user,user))
		{
		  old=p;
		  p=p->next;
		}

	if (p==NULL)
		return(FAIL);
	if (old==NULL)
		current->session_list = current->session_list->next;
	else
		old->next = p->next;

	free(p->user);
	free(p->cwd);
	if(p->maxfile != 0)
		for(i=0; i<p->maxfile; i++)
			{
				free(p->file_list[i].name);
				free(p->file_list[i].path);
			}
	free(p);
	return SUCCESS;
}

/*
 * So far for the basic stuff. Now some real work :)
 */

void	cleanup_sessions()
/*
 * Allows flooding users that have been quiet long enough
 * to talk again and removes old sessions
 */
{
	SESSION_list	*session;

	for(session = current->session_list; session; 
	    session = session->next)
	{
		if((time(NULL) - session->last_received) > SESSION_TIMEOUT)
			delete_session(session->user);
		else if(((time(NULL) - session->flood_start) > FLOOD_TALKAGAIN)
			&& session->flooding)
		{
			session->flooding = FALSE;
			session->flood_cnt = 0;

			if (max_userlevel(session->user)>0)
				send_to_user(session->user, "You may speak again");
		}
	}
}

int	check_session(char *user, char *to)
/*
 * Creates a new entry if non is found, checks for flooding
 * and gathers some information (has dcc etc.)
 */
{
	if((currentsession = find_session(user)) == NULL)
		currentsession = create_session(user);
	if(check_flood2(user,to))
		return IS_FLOODING;
	currentsession->last_received = time(NULL);
	return IS_OK;
}

int	check_flood2(char *user, char *to)
/*
 * Checks if this user is flooding (PUBLIC or PRIVATE. WALLOPS and NOTICES
 * are always ignored).
 * If so, this function takes appropriate action and returns TRUE so the
 * calling function knows it should ignore the text.
 *
 * Three possibilities:
 * - User is unknown to me, so he cannot be flooding -> create entry
 * - User is known to me and is already marked flooding -> ignore
 * - User is know to me and is not flooding -> look if flooding now
 */
{
	if((currentsession = find_session(user)) == NULL)
	{	
		currentsession = create_session(user);
		return FALSE;
	}
	else if(currentsession->flooding)
	{
		currentsession->last_received = time(NULL);
		return TRUE;
	}	
	else
	{
		char buff[150];

		currentsession->last_received = time(NULL);
		currentsession->flood_cnt++;
		if(time(NULL) - currentsession->flood_start <= FLOOD_INTERVAL)
		{	/* user is flooding */
			if(currentsession->flood_cnt < FLOOD_RATE)
				return FALSE;

			if (max_userlevel(user)>0)
				send_to_user(user, "I'm gonna ignore you for a while, LAMER!");

			currentsession->flooding = TRUE;

/*			if (current->talk>0)
			{
				sprintf(buff,"I'm ignoring %s cause the lamer triggered my flood prot.",user);
				do_bwallop(current->nick,to,buff);
			}
*/
			add_list_event("%s triggered my flood protection", user);
			return TRUE;
		}
		else
		{	/* reset values */
			currentsession->flood_start = time(NULL);
			currentsession->flood_cnt = 1;
			currentsession->flooding = FALSE;
			return FALSE;
		}
	}
}

/* 
 * some "ftp" commands 
 */
char	*get_cwd(char *user)
{
	if((currentsession = find_session(user)) == NULL)
		currentsession = create_session(user);
	currentsession->last_received = time(NULL);
	return currentsession->cwd;
}

void	change_dir(char *user, char *new_dir)
{
	if((currentsession = find_session(user)) == NULL)
		currentsession = create_session(user);
	free(currentsession->cwd);
	mstrcpy(&currentsession->cwd, new_dir);
	currentsession->last_received = time(NULL);
}

void	dcc_sendnext2(char *user)
{
        if((currentsession = find_session(user)) == NULL)
		return;

	currentsession->last_received = time(NULL);
	if(currentsession->maxfile == 0)
		return;

	if(currentsession->currentfile == currentsession->maxfile)
	{
		int	i;

		for(i=0; i<currentsession->maxfile; i++)
		{
			free(currentsession->file_list[i].name);
			free(currentsession->file_list[i].path);
		}
		currentsession->maxfile = 0;
		currentsession->currentfile = 0;
		return;
	}

	while((currentsession->currentfile < currentsession->maxfile) &&
	       !send_file_from_list(user, 
	       currentsession->file_list[currentsession->currentfile].path, 
	       currentsession->file_list[currentsession->currentfile].name))
		currentsession->currentfile++;

	currentsession->currentfile++;
	
}

int	dcc_sendlist(char *user, char *path, char *file_list[], int n)
/*
 * Sends all files in file_list[] (located in path) which contains n 
 * entries to user "user" one-by-one!
 * If internally the maxfile != 0, it means there are already files
 * being queued and the function will return 0, else 1
 *
 */
{
	int	i,j;
	int	files_to_copy;

	if((currentsession = find_session(user)) == NULL)
	                currentsession = create_session(user);

	currentsession->last_received = time(NULL);
	files_to_copy = (currentsession->maxfile+n>LISTSIZE)?
	                (LISTSIZE-currentsession->maxfile):
			n;
	sendreply("%d File%s added to filequeue", files_to_copy,
		  files_to_copy==1?"":"s");

	/* copy the filelist */
	for(i=0, j=currentsession->maxfile; i<files_to_copy; i++, j++)
	{
		mstrcpy(&(currentsession->file_list[j].path), path);
		mstrcpy(&(currentsession->file_list[j].name), file_list[i]);
	}
	currentsession->maxfile += files_to_copy;
	if(currentsession->currentfile == 0)
		dcc_sendnext2(user);
	return TRUE;
}

void	do_showqueue()
{
	int	i;

	sendreply("Maxfile = %d, currentfile = %d", currentsession->maxfile,
		   currentsession->currentfile);
	sendreply("Filequeue:");
	for(i=0; i<currentsession->maxfile; i++)
		sendreply("path: %s, file %s", currentsession->file_list[i].path,
					       currentsession->file_list[i].name);
}

void do_clearqueue(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	int	i;

	if (currentsession->maxfile != 0) {
		for (i=0; i < currentsession->maxfile; i++) {
				free(currentsession->file_list[i].name);
				free(currentsession->file_list[i].path);
		}
		currentsession->maxfile = 0;
		currentsession->currentfile = 0;
	}
}

/*
 * additional functions
 */
void	sendreply(char *s, ...)
{
	va_list	msg;
	char	buf[MAXLEN];

	va_start(msg, s);
	vsprintf(buf, s, msg);
	send_to_user(currentsession->user, buf);
	va_end(msg);
}

void	add_list_event	(char *format, ...)
{
	FILE	*eventfile;
   	char	buf[MAXLEN];
	va_list	msg;

	va_start(msg, format);
	vsprintf(buf, format, msg);
	va_end(msg);

	if ((eventfile=fopen(EVENTLOG,"at"))==NULL)
		return;

	fprintf(eventfile,"[%s] %s\n", time2str(time(NULL)), buf);
	fclose(eventfile);
}
