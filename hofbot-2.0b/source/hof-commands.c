/*******************************************/
/*    File: hof-commands 		   */
/*  Author: HOF (jpoon@mathlab.sunysb.edu) */
/* Purpose: Commands for HOFbot            */
/* Version: 2.0B		           */
/*    Date: June, 1996			   */
/*******************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>
#include "config.h"
#include "fnmatch.h"
#include "hofbot.h"
#include "channel.h"
#include "userlist.h"
#include "dcc.h"
#include "function.h"
#include "commands.h"

extern botinfo *current;
extern char owneruserhost[MAXLEN];
extern int number_of_bots;


void do_botinfo(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	send_to_user(from, "\x2%10s: %s\x2", "Current nick", current->nick);
	send_to_user(from, "\x2%10s: %s:%d\x2", "Current server",
			current->serverlist[current->current_server].name,
		        current->serverlist[current->current_server].port);
	send_to_user(from, "\x2%10s: %c\x2", "Current prefix",
							current->cmdchar);
	show_channellist(from);
        send_to_user(from, "   ");
	send_to_user(from, "\x2%42s\x2", "=== Global settings ===");
	send_to_user(from, "\x2%17s: %3s %25s: %3s\x2", "GreetToggle", 
	 				 (current->greetoggle) ? "On" : "Off",
			   "NickToggle", (current->nicktoggle) ? "On" : "Off");
	send_to_user(from, "\x2%17s: %3s %25s: %3s\x2", "WarToggle", 
	 				 (current->warontoggle) ? "On" : "Off",
			   "NewsToggle", (current->newstoggle) ? "On" : "Off");
	send_to_user(from, "\x2%17s: %2d %25s: %2d\x2", "Nick change time",
	 			 current->nicklevel, "Enforce topic time",
				 current->enftopiclevel);
	send_to_user(from, "\x2%17s: %2d %25s: %2d\x2", "Random topic time",
	 			 current->rantopiclevel, "Random quote time",
				 current->ranquotelevel);
	send_to_user(from, "\x2%17s: %2d %25s: %2d\x2", "Flood level",
	 			 current->floodlevel, "Flood prot level",
				 current->floodprotlevel);
	send_to_user(from, "\x2%17s: %2d %25s: %2d\x2", "Mass prot level",
	 			 current->massprotlevel, "Mass deop level",
				 current->massdeoplevel);
	send_to_user(from, "\x2%17s: %2d %25s: %2d\x2", "Mass ban level",
	 			 current->massbanlevel, "Mass kick level",
				 current->masskicklevel);
	send_to_user(from, "\x2%17s: %2d %25s: %2d\x2", "Mass action level",
	 			 current->malevel, "Idle kick time",
				 current->idlelevel);
}

void do_setaway(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	if (!rest)
	{
		send_to_user(from, "\002%s\002", "I no longer have away status now");
		send_to_server("AWAY");
	}
	else
	{
		send_to_user(from, "\002Away msg change from \"%s\" to \"%s\"\002",
			current->awaymsg, rest);
		strcpy(current->awaymsg, rest);
		send_to_server("AWAY :\002%s\002", rest);
	}

}

void do_setgreet(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	if (!rest)
	{
		send_to_user(from, "\002%s\002", "USAGE: setgreet <text1,text2>");
	}
	else
	{
		strcpy(current->greetmsg, rest);
		send_to_user(from, "\002New greet msg is: %s\002", current->greetmsg);
	}

}

void do_savelevels(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	if (!write_levelfile("hofbot.levels"))
		send_to_user(from, "\002Levels could not be saved to %s\002",
			"hofbot.levels");
	else
		send_to_user(from, "\002Levels were written to %s\002",
			"hofbot.levels");
}

void do_send( char *from, char *to, char *rest, int cmdlevel)
{
  char  buf[50], buf2[MAXLEN], filename[MAXLEN], filelistfile[MAXLEN];
  FILE *in;
  FILE *ls_file;
  int ulevel, level;
  char lin[MAXLEN];
  char *ptr;

  if (!rest)
    {
      send_to_user(from, "\x2Please specify a filename or use \"%cfiles\"\x2",
		   current->cmdchar);
      return;
    }
  strcpy(filename, FILEUPLOADDIR);
  strcat(filename, rest); 
  ulevel = get_userlevel(from, "*");
  strcpy(filelistfile, FILEUPLOADDIR);
  strcat(filelistfile, FILELISTNAME);
  if( ( ls_file = fopen( filelistfile, "r" ) ) == NULL )
	{
		send_to_user(from, "\x2No files available\x2");
      		return;
	}
  while (freadln(ls_file, lin)) {
      ptr = lin;
      sscanf(ptr, "%s %i %s\n", buf, &level, buf2);
      if (!strcmp(rest, buf))
	{
	  if (level > ulevel)
	    {
	      send_to_user(from, "\x2Sorry, you need access level %i to get this file\x2", level);
	      return;
	    }
        }
   }
   fclose(ls_file);
   dcc_sendfile( from, filename );
   return;
}

void  do_flist( char *from, char *to, char *rest, int cmdlevel )
{
	FILE  *ls_file;
	char lin[MAXLEN];
        char filelistfile[MAXLEN];
	char *filename, *level, *info;
        char *ptr;
	int  showtitle = FALSE;

        strcpy(filelistfile, FILEUPLOADDIR);
        strcat(filelistfile, FILELISTNAME);
	if( ( ls_file = fopen( filelistfile, "r" ) ) == NULL )
	{
		send_to_user(from, "\x2No files available\x2");
		return;
	}
        while (freadln(ls_file, lin)) {
           ptr = lin; 
	   if (*ptr == '=') {
	   	ptr++;
		send_to_user(from, "\x2%s\x2", ptr);
		if (!showtitle) {
		  send_to_user(from, "\x2----- Filename -----+- Level -+-----------Information-----------\x2");
	          showtitle = TRUE;
		}
	   }
	   else {
		if (!showtitle) {
		  send_to_user(from, "\x2----- Filename -----+- Level -+-----------Information-----------\x2");
	          showtitle = TRUE;
		}
		filename = strtok(ptr, " ");
        	level = strtok(NULL, " ");
	        info = strtok(NULL, "\n");
		send_to_user(from, "%19s |     %3s |  %s", filename, level, info);
 	   }
	}
	send_to_user(from, "\x2--------------------+---------+---------------------------------\x2");
	send_to_user(from, "\x2To download a file: type %cget <filename>\x2", current->cmdchar);
	fclose( ls_file );
}

void  do_readnews( char *from, char *to, char *rest, int cmdlevel )
{
	FILE  *ls_file;
	char lin[MAXLEN];
	int level;
        char *ptr;

	if( ( ls_file = fopen(NEWSFILE, "r" ) ) == NULL )
	{
		send_to_user(from, "\x2No news available\x2");
		return;
	}
        while (freadln(ls_file, lin)) {
           ptr = lin; 
      	   send_to_user(from, "\x2%s\x2", ptr);
	}
	fclose( ls_file );
}

void do_refresh(char *from, char *to, char *rest, int cmdlevel)
{
        char *chan;
 
	chan = find_channel(to, &rest); 
        if (get_userlevel(from, chan) < cmdlevel)
        {
          no_access(from, chan);
          return;
        }
		if( !fork() )  {
		  sleep(5);
		  start_bots();
		}
		else {
		  globallog(ERRFILE, "SIGNING OFF: REFRESH");
   		  kill_all_bots("Going to take a break!");
		  exit( 0 );
		}
}

void do_addgreet(char *from, char *to, char *rest, int cmdlevel)
{
        char *chan, *ptr;
	char lin[MAXLEN];
 	FILE *ofp, *ifp;
	int  i=0;

	chan = find_channel(to, &rest); 
        if (get_userlevel(from, chan) < cmdlevel)
        {
          no_access(from, chan);
          return;
        }
	if (!rest)
	{
		send_to_user( from, "\002%s\002", "Usage: addgreet <greet part I>,<greet part II>");
		return;
	}
        if (!(ofp=fopen(GREETFILE, "a")))
          return;	
	fprintf(ofp, "%s\n", rest);
	fclose(ofp);
	if (!(ifp=fopen(GREETFILE, "r")))
          return;	
       	while (freadln(ifp, lin))
	  {
        	 ptr = lin;
	 	 ++i;  
	  }
       	send_to_user(from, "\002You have added greet #%i in the %s\002", i, GREETFILE);
	fclose(ifp);
	if (add_to_list(current->GreetList, rest))
		send_to_user(from, "\002For more info, use command GREETLIST\002");
}

void do_enftop(char *from, char *to, char *rest, int cmdlevel)
{
        char *chan;
        char topic[MAXLEN], temp[MAXLEN];
        int i, c = 0;

	chan = find_channel(to, &rest); 
        if (get_userlevel(from, chan) < cmdlevel)
        {
          no_access(from, chan);
          return;
        }
	if (!rest)
	{
		send_to_user( from, "\002%s\002", "What is the enforced topic?");
		return;
	}
        strcpy(temp, rest);
        for (i = 0; temp[c] != '\0'; ++i) {
          if ( i % 2 == 0)
            topic[i] = '\002';
          else
            topic[i] = temp[c++];
        }
        topic[i] = '\0';
        set_enftopic(chan, topic);
        send_to_user(from, "\x2%s topic is enforced on %s\x2", topic, chan);
   	send_to_server("TOPIC %s :\x2%s\x2", chan, topic);
}
void do_enftop2(char *from, char *to, char *rest, int cmdlevel)
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
		send_to_user( from, "\002%s\002", "What is the enforced topic?");
		return;
	}
        set_enftopic(chan, rest);
        send_to_user(from, "\x2%s topic is enforced on %s\x2", rest, chan);
   	send_to_server("TOPIC %s :\x2%s\x2", chan, rest);
} 

void greet_action(char *who, char *channel)
{
  LS *temp;
  FILE *in;
  char username[20], lin[MAXLEN], ptr[MAXLEN];
  char *msg1, *msg2;
  int i = 1, userlvl, greetlvl, luckynumber = 7;
 
  strcpy(username, getnick(who));
  if (!my_stricmp(username, current->nick))     /* never greet itself */
	return;
  srand((unsigned)time(NULL));		
  if ( RANDOM(1, 10) != luckynumber )          /* random greet! */
	return;
  greetlvl = get_greetlevel(who, channel);
  userlvl = get_userlevel(who, channel);
  if (!greetlvl && userlvl < 10) {
    strcpy(ptr, current->greetmsg);
    if (ptr && *ptr) {
	msg1 = strtok(ptr, ",");
	msg2 = strtok(NULL, ",");
        sendnotice(username, "\x2 %s %s, %s\x2", msg1, username, msg2);
    }
    else
        sendnotice(username, "Welcome %s, you are now in %s \002:)\002", username, channel); 
    return;
  }
  for(temp=*current->GreetList, i=1; temp; temp=temp->next, i++)
  {
	 if (greetlvl == i)
	 {
		strcpy(lin, temp->name);
	 	msg1=strtok(lin, ",");
                msg2=strtok(NULL, ",");
                sendprivmsg(channel, "\x2%s %s, %s\x2", msg1, getnick(who), msg2);
		return;
	 }
  }
}

void changetopic(char *channel)
{
  FILE *infile;
  int number=0, i, topic;
  char buffer[MAXLEN];

  if (!(infile=fopen(TOPICFILE, "rt")))
    return;
/*
  while(!feof(infile)) {
    readline(infile, buffer);
    number++;
  }
  fclose(infile);
*/
  srand((unsigned)time(NULL));
  number = RANDOM(1, 900-1);	/* there are about 900 lines in quotes.bot */
  for(i=0;i<number;i++) /* readline(infile, buffer); */
	topic = freadln(infile, buffer);
  if (topic == TRUE)
	  send_to_server("TOPIC %s :\x2%s\x2", channel, buffer);
  fclose(infile);
}

void do_nextopic(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  FILE *infile;
  int number=0, i, topic;
  char buffer[MAXLEN];
  char *chan;

  chan = find_channel(to, &rest); 
      if (get_userlevel(from, chan) < cmdlevel)
        {
          no_access(from, chan);
          return;
        }
  if (!(infile=fopen(QUOTEFILE, "rt")))
    return;
/*
    while(!feof(infile)) {
    readline(infile, buffer);
    number++;
  }
  fclose(infile);
*/
  srand((unsigned)time(NULL));
  number = RANDOM(1, 900-1);
  for(i=0;i<number;i++) /* readline(infile, buffer); */
	topic = freadln(infile, buffer);
  if (topic == TRUE) {
	  if(ischannel(to))
	    send_to_server("TOPIC %s :\x2%s\x2", to, buffer);
	  else
	    send_to_server("TOPIC %s :\x2%s\x2", currentchannel(), buffer);
  }
  fclose(infile);
}

void do_nextquote(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
  FILE *infile;
  int number=0, i, quote;
  char buffer[MAXLEN];
  char *chan;

  chan = find_channel(to, &rest); 
      if (get_userlevel(from, chan) < cmdlevel)
        {
          no_access(from, chan);
          return;
        }
  if (!(infile=fopen(QUOTEFILE, "rt")))
    return;
/*
    while(!feof(infile)) {
    readline(infile, buffer);
    number++;
  }
  fclose(infile);
*/
  srand((unsigned)time(NULL));
  number = RANDOM(1, 900-1);
  for(i=0;i<number;i++) /* readline(infile, buffer); */
	quote = freadln(infile, buffer);
  if (quote == TRUE) {
	  if(ischannel(to))
	    sendnotice(to, "\x2%s\x2", buffer);
	  else
	    sendnotice(currentchannel(), "\x2%s\x2", buffer);
  }
  fclose(infile);
}

void do_nextnick(from, to, rest, cmdlevel)
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
  changenick();
}

void do_heart(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
        char *chan;
        char *nick;

	chan = find_channel(to, &rest); 
        if (get_userlevel(from, chan) < cmdlevel)
        {
          no_access(from, chan);
          return;
        }
	if (!rest)
	{
		send_to_user(from, "\002%s\002", "No nick given");
		return;
	}
	nick = get_token(&rest, " ");

	if (!my_stricmp(nick,current->nick))
	{
		 send_to_user(from, "\002%s\002", "Nice try, but I don't think so"); 
		 return;
	}
        
        send_to_user(nick, "\002%s has requested me to deliver you a lovely item!\002", from);
send_to_user(nick, "             @@@         @@@         @@@");
send_to_user(nick, "          @@@   @@@   @@@   @@@   @@     @@@");
send_to_user(nick, "        @@         @@@@        @@           @@    ");
send_to_user(nick, "       @                         @            @");
send_to_user(nick, "       @                         @            @");
send_to_user(nick, "  <================             @            @====== <<<<<<<<<<");
send_to_user(nick, "          @                   @            @             ");
send_to_user(nick, "            @               @            @");
send_to_user(nick, "              @@         @@ @         @@");
send_to_user(nick, "                @@     @@    @@     @@");
send_to_user(nick, "                   @@@          @@@");
        send_to_user(from, "\002Love sent to %s\002", nick);
}

void do_rose(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *nick, *msg, *color;
	char sender[10];
        char *chan;

	strcpy(sender, getnick(from));
	chan = find_channel(to, &rest); 
        if (get_userlevel(from, chan) < cmdlevel)
        {
          no_access(from, chan);
          return;
        }
  	if (!(nick = get_token(&rest, " ")))
	{
		send_to_user(from, "\002%s\002", "Usage: sendrose <nick> <color> <msg>");
		return;
	}
  	if (!(color = get_token(&rest, " ")))
	{
		send_to_user(from, "\002%s\002", "Usage: sendrose <nick> <color> <msg>");
		return;
	}
  	if (!rest)
	{
		send_to_user(from, "\002%s\002", "Usage: sendrose <nick> <color> <msg>");
		return;
	}
	if (!my_stricmp(nick,current->nick))
	{
		 send_to_user(from, "\002%s\002", "Nice try, but I don't think so");
		 return;
	}
        send_to_user(nick, "\002%s has requested me to send you something.\002", from);
	send_to_user(nick, "\002Are u ready for the surprise?!? ;)\002");
	send_to_user(nick, "\002Here you go..here you go...here you go!\002");
	send_to_user(nick, "\002One dozen fresh %s roses for you!\002", color);
	send_to_user(nick, "\002@)-'-,--- @>-'-,--- @>-'-,--- @>-'-,---\002");
	send_to_user(nick, "\002@)-'-,--- @>-'-,--- @>-'-,--- @>-'-,---\002");
	send_to_user(nick, "\002@)-'-,--- @>-'-,--- @>-'-,--- @>-'-,---\002");
        send_to_user(nick, "\002There is a card says: \n");
	send_to_user(nick, "\002\"%s\"\002", rest);
	send_to_user(nick, "\002			-%s\002", sender);
	send_to_user(nick, "\002So how much is my tips? :)~\002");
        send_to_user(from, "\002roses sent to %s\002", nick);
}


void do_poem(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *nick;
        char *chan;

	chan = find_channel(to, &rest); 
        if (get_userlevel(from, chan) < cmdlevel)
        {
          no_access(from, chan);
          return;
        }

	if (!rest)
	{
		send_to_user(from, "\002%s\002", "No nick given");
		return;
	}
	nick = get_token(&rest, " ");

	if (!my_stricmp(nick,current->nick))
	{
		 send_to_user(from, "\002%s\002", "Nice try, but I don't think so");
		 return;
	}
	send_to_user(nick, "\002%s has requested me to send you a poem\002", from);
        send_to_user(nick, "\002        LOVE'S OMNIPRESENCE\002");
        send_to_user(nick, "\002  Were I as base as is the lowly plain,\002");
        send_to_user(nick, "\002    And you, my Love, as high as heaven above,\002");
        send_to_user(nick, "\002  Yet should the thoughts of me your humble swain\002");
        send_to_user(nick, "\002    Ascend to heaven,in honor of my Love.\002");
        send_to_user(nick, "\002  Were I as high as heaven above the plain,\002");
        send_to_user(nick, "\002    And you,my Love,as humble and as low\002");
        send_to_user(nick, "\002  As are the deepest bottoms of the main,\002");
        send_to_user(nick, "\002    Whereso'er you were,with you my love should go.\002");
        send_to_user(nick, "\002  Were you the earth,dear Love, and I the skies,\002");
        send_to_user(nick, "\002    My love should shine on you like to the sun,\002");
        send_to_user(nick, "\002  And look upon you with ten thousand eyes\002");
        send_to_user(nick, "\002    Till heaven wax'd blind,and till the world were done.\002");
        send_to_user(nick, "\002  Whereso'er I am, below, or else above you,\002");
        send_to_user(nick, "\002  Whereso'er you are, my heart shall truly love you.\002");
        send_to_user(from, "\002poem sent to %s\002", nick);
}

void do_news(char *from, char *to, char *rest, int cmdlevel)
{
	CHAN_list *Channel;
        char *chan;

	chan = find_channel(to, &rest); 
        if (get_userlevel(from, chan) < cmdlevel)
        {
          no_access(from, chan);
          return;
        }
	if (!rest)
	{
		send_to_user( from, "\002%s\002", "What is the news?");
		return;
	}
       for (Channel = current->Channel_list; Channel; Channel = Channel->next) {
          sendnotice(Channel->name, "\002    Broadcast from %s [%-20.20s]\002", getnick(from), time2str(getthetime()));
          sendnotice(Channel->name, "\002%s\002", rest);
      }	 
}

void do_prefix(char *from, char *to, char *rest, int cmdlevel)
{
        char *chan;
 
	chan = find_channel(to, &rest); 
        if (get_userlevel(from, chan) < cmdlevel)
        {
          no_access(from, chan);
          return;
        }
	send_to_user( from, "\002The hofkey you need is %c\002", current->cmdchar);
}

void do_listgreet(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	LS *temp;
	int i;

        send_to_user(from, "\002%s\002",
	 "========================= GreetList =========================");
	for(temp=*current->GreetList, i=1; temp; temp=temp->next, i++)
	{
 	       	send_to_user(from, "\002%d. %s\002", i, temp->name);
	}
       send_to_user(from, "\002%s\002",
	 "=======================End of GreetList=======================");
	return;
}

/**************************** partyline commands ****************************/
void do_addparty(from, to, rest, cmdlevel)
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
	dcc_chat(uh, 1);
	sendnotice(nick, "\002You are invited to join the HOFBOT dcc party 
			chat.\002");
	sendnotice(nick, "\002Please type: /dcc chat %s\002", current->nick);
        send_to_user(from, "\002%s has been invited.\002", nick);
	return;
}

void do_addbot(from, to, rest, cmdlevel)
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
	dcc_chat(uh, 2);
        send_to_user(from, "\002%s has been invited\002", nick);
	return;
}

void do_joinparty(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	dcc_chat(from, 1);
	return;
}

void do_listparty(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{

	show_party(from);
	return;
}

void do_exit(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{

	exit_party(from);
	return;
}

void do_awaymsg(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	setaway_party(from, rest);
	return;
}

void do_whois(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *nick;

	if (!(nick = get_token(&rest, " ")))
	  {
		 send_to_user(from, "\002%s\002", "No nick specified");
		 return;
	  }
	whois_party(from, nick);
	return;
}

void do_partynick(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *nick;

	if (!(nick = get_token(&rest, " ")))
	  {
		 send_to_user(from, "\002%s\002", "No nick specified");
		 return;
	  }
	newnick_party(from, nick);
	return;
}

void do_kickparty(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *nick;

	if (!(nick = get_token(&rest, " ")))
	  {
		 send_to_user(from, "\002%s\002", "No nick specified");
		 return;
	  }
	kick_party(from, nick, rest);
	return;
}

void do_shutdown(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	shutdown_party(from);
}

void do_helpme(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	send_to_user(from, "\002All partyline commands should begin with a '.' \002");
	send_to_user(from, "There are only 10 commands dedicated to partyline");
	send_to_user(from, "for just now.  More commands will be added soon.");
	send_to_user(from, "Enjoy the new lag free chat fun! =)");
	send_to_user(from, "\002.ADDBOT:\002    Link a bot to partyline");
	send_to_user(from, "\002.ADDPARTY:\002  Add someone to partyline");
	send_to_user(from, "\002.AWAY:\002      Set/Unset away");
	send_to_user(from, "\002.EXIT:\002      Exit partyline");
	send_to_user(from, "\002.HELPME:\002    Help on partyline");
	send_to_user(from, "\002.KICK:\002      Kick someone off partyline");
	send_to_user(from, "\002.PARTYLIST:\002 Show users currently on partyline");
	send_to_user(from, "\002.PARTYNICK:\002 Change your nick in partyline");
	send_to_user(from, "\002.WHOIS:\002     Get a user information on partyline");
	send_to_user(from, "\002.SHUTDOWN:\002     Shut down a partyline.");
	send_to_user(from, "\002+ ALL 139 regular commands like .OP, .PING, etc...\002");
	send_to_user(from, "\002Use +OP, +PING, ... to control all the linked hofbots\002");
	return;
}

void do_dccparty(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *levelstr;
	int  level;

	if (!rest) {
		send_to_user(from, "\x2%s\x2", "usage: dccparty [0|1]");
		send_to_user(from, "\x2%s\x2", "1	change current status to dcc chat");
		send_to_user(from, "\x2%s\x2", "2	change current status to partyline");
		return;
	}
	level = atoi(rest);
	if (level >= 0 && level < 3)
		change_party(from, level);
	else
		send_to_user(from, "\x2%s\x2", "invalid level!");

}

void do_flood(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *nick, *uh, *floodnumstr;
	char bold = '\x2', beep = '\007';
	int floodnum, i;

	if (!(nick = get_token(&rest, " ")))
	  {
		 send_to_user(from, "\002%s\002", "No nick specified"); 
		 return;
	  }

	uh = find_userhost(from, nick);
	if (!uh)
	  return;

	if (!(floodnumstr = get_token(&rest, " ")))
		floodnum = 3;
	else
		floodnum = atoi(floodnumstr);

	for ( i = 1; i <= floodnum; i++)
	   {
/*		send_ctcp( nick, "PING");	*/
		if (rest && *rest)
			sendnotice(nick, "%s", rest);
		else /* default flood txt */
			sendnotice(nick, "%s %c%s %c%s %c%s",
				  "ks?<>{}:E$%{^",
				  bold,
				  "6??^&&*Y5",
				  bold,
				  "dskr#$^G~!{}<>",
				  bold,
				  "#%$*^&&#%$*&@%");
	   }
	send_to_user(from, "\002%s\002", "Finish flooding!"); 
}
