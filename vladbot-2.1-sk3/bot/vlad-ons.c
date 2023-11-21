/*
 * vlad-ons.c - kinda like /on ^.. in ircII
 * (c) 1993 VladDrac (irvdwijk@cs.vu.nl)
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
#include <strings.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>
#include <iconv.h>

#define _GNU_SOURCE
#include "fnmatch.h"
//#include <fnmatch.h>
#include "botlist.h"
#include "channel.h"
#include "config.h"
#include "dcc.h"
#include "debug.h"
#include "ftext.h"
#include "ftp_dcc.h"
#include "log.h"
#include "misc.h"
#include "note.h"
#include "parsing.h"
#include "parsetime.h"
#include "send.h"
#include "session.h"
#include "userlist.h"
#include "botlist.h"
#include "locuteur.h"
#include "vladbot.h"
#include "vlad-ons.h"
#include "phrase.h"

extern	botinfo	*currentbot;
extern	int	number_of_bots;
extern	int	rehash;
extern	long	uptime;
extern	char	*botmaintainer;
extern  short   log;
extern  char    CommandChar;
int GNumPhrase = 0;

/* external parseroutines ("prefix commands")	*/
extern	void	parse_note(char *from, char *to, char *s);
extern	void	parse_global(char *from, char *to, char *rest);

extern char	*months[];

extern void	sig_alrm();
command_tbl on_msg_commands[] =

/* Any command which produces more than 2 lines output can be 
   used to make the bot "excess flood". Make it "forcedcc"     */

/*  Command	function   userlvl    shitlvl   rellvl      forcedcc */
	{
		{ "HELP",		show_help,		0,		100,	-100,			TRUE  },		
		{ "APPRENDS",	do_apprends,    0,      100,	CONFIANCE_LVL,	FALSE },		
		{ "DESACTIVE",	do_desactive,   0,      100,    CONFIANCE_LVL,	FALSE },		
		{ "ACTIVE",		do_active,      0,      100,    CONFIANCE_LVL,	FALSE },		
		{ "FUCK",		do_fuck,        0,      0,      SYMPA_LVL,		FALSE },		
		{ "WHOAMI",		show_whoami,  	0,		100,	-100,			FALSE },		
		{ "INFO",		show_info,		0,		100,	-50,			FALSE },		
		{ "TIME",		show_time,		0,		100,	-10,			FALSE },		
		/* { "NRELADD", do_nreladd,     0,      0,      -500,			FALSE },		*/
		{ "LISTDCC",	do_listdcc,		0,		100,	0,				TRUE  },		
		{ "CHAT",		do_chat,		0,		100,	0,				FALSE },		
		/* { "SEND",	do_send,		0,		100,	FALSE },		*/
		/* These two are just aliases for ftp-dcc		 */
		/* { "GET",		do_send,		0,		100,	FALSE },		*/
		/* { "MGET",	do_send,		0,		100,	FALSE },		*/
		/* Some more ftp-dcc functions				 */
		/* { "LS",		show_dir,		0,		100,	TRUE  },		*/
		/* { "PWD",		show_cwd,		0,		100,	FALSE },		*/
		/* { "CD",		do_cd,			0,		100,	FALSE },		*/
		/* { "QUEUE",	show_queue,		0,		100,	TRUE  },		*/
		/* { "FILES",	do_flist,       0,      100,    TRUE  },		*/
		/* { "FLIST",	do_flist,		0,		100,	TRUE  },		*/
		/* userlevel and dcc are dealt with within NOTE 	*/
		/* { "NOTE",	parse_note,		0,		100,	TRUE  },		*/
		/* userlevel and dcc are dealt with within GLOBAL 	*/
		{ "GLOBAL",		parse_global,	0,		100,	CONFIANCE_LVL,	FALSE },		
		{ "MYCMDS",		show_mycmds,    0,      100,    -10,			TRUE  },		
		{ "STIMLIST",	do_stimlist,    50,     100,    SYMPA_LVL,		TRUE  },		
		{ "REPLIST",	do_replist,     50,     100,    SYMPA_LVL,		TRUE  },		
		{ "STIMDEL",	do_stimdel,     50,     0,      SYMPA_LVL,		FALSE },		
		{ "REPDEL",		do_repdel,      50,     0,      SYMPA_LVL,		FALSE },		
		{ "CMDSLVLS",	show_cmdlvls,   50,     100,    SYMPA_LVL,		TRUE  },		
		{ "INVITE",		do_invite,		50,		0,		SYMPA_LVL,		FALSE },		
		{ "NWHOIS",		show_nwhois,	50,		0,		SYMPA_LVL,		FALSE },		
		{ "RELADD",		do_reladd,      50,     0,      SYMPA_LVL,		FALSE },		
		{ "SAY",		do_say,			50,		0,		CONFIANCE_LVL,	FALSE },		
		{ "ME",			do_me,          50,     0,      CONFIANCE_LVL,	FALSE },		
		{ "TOPIC",		do_topic,       50,     0,      SYMPA_LVL,		FALSE },		
		{ "SEEN",		do_seen,		0,		0,		SYMPA_LVL,		FALSE },		
		{ "USERLIST",	show_userlist,	100,	100,	0,				TRUE  },		
		{ "SHITLIST",	show_shitlist,	100,	100,	SYMPA_LVL,		TRUE  },		
		{ "PROTLIST",	show_protlist,	100,	100,	CONFIANCE_LVL,	TRUE  },		
		{ "RELLIST",	show_rellist,   100,    0,      CONFIANCE_LVL,	TRUE  },		
		{ "OP",			do_op,			100,	0,		SYMPA_LVL,		FALSE },		
		{ "DEOP",		do_deop,		100,	0,		SYMPA_LVL,		FALSE },		
		{ "WHOIS",		show_whois,		100,	0,		SYMPA_LVL,		FALSE },		
		{ "USERWRITE",	do_userwrite,	100,	0,		0,				FALSE },		
		{ "RELWRITE",	do_relwrite,    100,    0,      0,				FALSE },		
		{ "RELDEL",		do_reldel,      100,    0,      SYMPA_LVL,		FALSE },		
		{ "SHOWUSERS",	do_showusers,	100,	0,		SYMPA_LVL,		TRUE  },		
		{ "STIMWRITE",	do_stimwrite,   100,    0,      CONFIANCE_LVL,	FALSE },		
		{ "REPWRITE",	do_repwrite,    100,    0,      CONFIANCE_LVL,	FALSE },		
		{ "KICK",		do_kick,		100,	0,		SYMPA_LVL,		FALSE },		
		{ "GIVEOP",		do_giveop,		110,	0,		CONFIANCE_LVL,	FALSE },		
		{ "NUSERADD",	do_nuseradd,	110,	0,		SYMPA_LVL,		FALSE },		
		{ "LOCWRITE",	do_locwrite,    110,    0,      SYMPA_LVL,		FALSE },		
		{ "USERDEL",	do_userdel,		110,	0,		CONFIANCE_LVL,	FALSE },		
		{ "NSHITADD",	do_nshitadd,	110,	0,		SYMPA_LVL,		FALSE },		
		{ "SHITWRITE",	do_shitwrite,	110,	0,		SYMPA_LVL,		FALSE },		
		{ "SHITDEL",	do_shitdel,		110,	0,		SYMPA_LVL,		FALSE },		
		{ "PROTWRITE",	do_protwrite,   110,    0,		SYMPA_LVL,		FALSE },		
		{ "PROTDEL",	do_protdel,     110,    0,		CONFIANCE_LVL,	FALSE },		
		{ "BANUSER",	do_banuser,		110,	0,		CONFIANCE_LVL,	FALSE },		
		{ "UNBAN",		do_unban,		110,	0,		SYMPA_LVL,		FALSE },		
		{ "OPEN",		do_open,		110,	0,		CONFIANCE_LVL,	FALSE },		
		{ "LEAVE",		do_leave,		110,	0,		SYMPA_LVL,		FALSE },		
		{ "NICK",		do_nick,		110,	0,		SYMPA_LVL,		FALSE },		
		{ "REPLOAD",	do_repload,     110,    0,      CONFIANCE_LVL,	FALSE },		
		{ "USERADD",	do_useradd,		110,	0,		CONFIANCE_LVL,	FALSE },		
		{ "LOGON",		do_logon,       110,    0,      CONFIANCE_LVL,	FALSE },		
		{ "LOGOFF",		do_logoff,      110,    0,      CONFIANCE_LVL,	FALSE },		
		{ "JOIN",		do_join,		115,	0,		CONFIANCE_LVL,	FALSE },		
		{ "SHITADD",	do_shitadd,		115,	0,		CONFIANCE_LVL,	FALSE },		
		{ "SHOWLOCS",	do_showlocs,    125,    0,      SYMPA_LVL,		TRUE  },		
		{ "OUBLIE",		do_oublie,      125,    0,      CONFIANCE_LVL,	FALSE },		
		{ "STIMLOAD",	do_stimload,    125,    0,      CONFIANCE_LVL,	FALSE },		
		{ "PROTADD",	do_protadd,     125,    0,		CONFIANCE_LVL,	FALSE },		
		{ "NPROTADD",	do_nprotadd,    125,    0,		SYMPA_LVL,		FALSE },		
		{ "CHANNELS",	show_channels,	125,	0,		SYMPA_LVL,		FALSE },		
		{ "MASSOP",		do_massop,		125,	0,		SYMPA_LVL,		FALSE },		
		{ "MASSDEOP",	do_massdeop,	125,	0,		CONFIANCE_LVL,	FALSE },		
		{ "MASSKICK",	do_masskick,	125,	0,		CONFIANCE_LVL,	FALSE },		
		{ "MASSUNBAN",	do_massunban,	125,	0,		SYMPA_LVL,		FALSE },		
		{ "SERVER",		do_server,		125,	0,		SYMPA_LVL,		FALSE },		
		{ "MSGLOGON",	do_msglogon,    125,    0,      CONFIANCE_LVL,	FALSE },		
		{ "MSGLOGOFF",	do_msglogoff,   125,    0,      CONFIANCE_LVL,	FALSE },		
		{ "COMCHAR",	do_comchar,     125,    0,      CONFIANCE_LVL,	FALSE },		
		{ "BOTLOAD",	do_botload,     125,    0,      CONFIANCE_LVL,	FALSE },		
		{ "BOTLIST",	do_showbots,    125,    0,      SYMPA_LVL,		TRUE  },		
		{ "BOTADD",		do_botadd,      125,    0,      CONFIANCE_LVL,	FALSE },		
		{ "BOTDEL",		do_botdel,      125,    0,      CONFIANCE_LVL,	FALSE },		
		{ "BOTWRITE",	do_botwrite,    125,    0,      SYMPA_LVL,		FALSE },		
		/* Priviliged commands					*/	
		{ "FORK",		do_fork,		150,	0,		-100,			FALSE },		
		{ "REHASH",		do_rehash,		150,	0,		-100,			FALSE },		
		{ "DO",			do_do,			150,	0,		-100,			FALSE },		
		{ "DIE",		do_die,			150,	0,		-100,			FALSE },		
		{ "DIEDIE",		do_die,			150,	0,		-100,			FALSE },		
		{ "QUIT",		do_quit,		150,	0,		-100,			FALSE },		
		{ "ALARM",		do_alarm,		50,		0,		SYMPA_LVL,		FALSE },		
		/*
		  :
		  :
	    */
		{ NULL,		null(void(*)()), 0,	0,	0,            FALSE }
};


/* Don't forget to delete the string returned. */
char *UserStr (char *nuh) {
  /* nuh = nick!user@host */
  char *NUH; 
  char *ret;
  char *nick;
  char *user;
  char *host;
  char *domain;

  NUH = strdup (nuh);
  ret = malloc (MAXLEN*sizeof (char));

  nick = get_token (&NUH, "!");
  user = get_token (&NUH, "@");
  if (*user == '~' || *user == '#') user++;
  host = get_token (&NUH, ".");
  domain = get_token (&NUH, "");

  sprintf (ret, "*!*%s@*%s", user, domain?domain:host);
  return ret;
}

/* Don't forget to delete the string returned. */
char *NickUserStr (char *nuh) {
  /* nuh = nick!user@host */
  char *NUH, *pNUH; 
  char *ret;
  char *nick;
  char *user;
  char *host;
  char *domain;
  char i;
  char *biz;
  int numbers = FALSE;

  NUH = strdup (nuh);
  pNUH = NUH;
  ret = malloc (MAXLEN*sizeof (char));

  nick = get_token (&NUH, "!");
  
  while ((biz = strchr (nick, '\\')))
    *biz = '*';
  while ((biz = strchr (nick, '|')))
    *biz = '*';
  while ((biz = strchr (nick, '[')))
    *biz = '*';
  while ((biz = strchr (nick, ']')))
    *biz = '*';

  user = get_token (&NUH, "@");
  if (*user == '~' || *user == '#') user++;
  host = get_token (&NUH, ".");
  domain = get_token (&NUH, "");/* attention, peut être nul */
  								/* vu avec un utilisateur dont le host était pdpc/supporter/active/user */

  if (domain){
	  for (i='0'; i<='9' && !numbers; i++)
		  if (strchr (domain, i))
			  numbers = TRUE;
	  
	  for (i='a'; i<='z' && numbers; i++)
		  if (strchr (domain, i))
			  numbers = FALSE;

	  for (i='A'; i<= 'Z' && numbers; i++)
		  if (strchr (domain, i))
			  numbers = FALSE;
  }

  if (!numbers)
	  sprintf (ret, "%s!*%s@*.%s", nick, user, domain?(strlen(domain)?domain:host):host);
  else {
    domain = get_token (&domain, ".");
    sprintf (ret, "%s!*%s@%s.%s.*", nick, user, host, domain?domain:"");
  }

  if (pNUH)
    free (pNUH);

  return ret;
}

void ForceDCC (char *from, char *to, char *msg) {
  DCC_list	*userclient;
  char	msg_copy[BIG_BUFFER];	/* for session */
  strcpy(msg_copy, msg);

  userclient = search_list("chat", from, DCC_CHAT);
  if(!userclient || (userclient->flags&DCC_WAIT))
    {
#ifdef AUTO_DCC
      dcc_chat(from, msg);
/*       nodcc_session(from, to, msg_copy); */
      sendnotice(getnick(from), "S'il-te-plaît, tape: /dcc chat %s", 
		 currentbot->nick);
#else
      sendnotice( getnick(from), 
		  "Désolé, je ne peux faire ceci que par DCC." );
      sendnotice( getnick(from), 
		  "Il faudrait ouvrir une session DCC." );
#endif /* AUTO_DCC */
      return;
    }
}

void    Ecrire (char *from, char *to, char *phrase) {
	char Ecrite[MAXLEN];

	if (!phrase)
		return;

	sprintf (Ecrite, phrase, GetNick (from), GetNick (from), GetNick (from), GetNick (from), GetNick (from));

	if (ischannel (to)) /* C'est un canal */
	{
		if (strncmp (Ecrite, "/me ", 4) == 0) {
			sendaction (to, Ecrite+4, GetNick (from));
			if (is_log_on (to))
				botlog (LOGFILE, "*%	s#%s* %s", currentbot->botname, to, Ecrite+4);
		} else if (strncmp (Ecrite, "/nil", 4) != 0) {
			sendprivmsg (to, /*phrase*/Ecrite, GetNick (from));
			if (is_log_on (to))
				botlog (LOGFILE, "<%s#%s>	 %s", currentbot->botname, to, Ecrite);
		}
    }
	else /* C'est un /msg */
    {
		if (strncmp (phrase, "/me ", 4) == 0) {
			sendaction (getnick (from), phrase+4, GetNick (from), GetNick (from), GetNick (from), GetNick (from));
			if (log)
				botlog (LOGFILE, "*%s#%s* %s", currentbot->botname, getnick(from), Ecrite+4);
		} else if (strncmp (phrase, "/nil", 4) != 0) {
			send_to_user (from, phrase, GetNick (from), GetNick (from), GetNick (from), GetNick (from), GetNick (from));
			if (log)
				botlog (LOGFILE, "<%s#%s> %s", currentbot->botname, from, Ecrite);
		}
    }
} /* void Ecrire () */

void    Repondre (char *from, char *to,
		  int AjoutHumeurPos,
		  int NbRepPos, char **RepPos,
		  int AjoutHumeurNeg,
		  int NbRepNeg, char **RepNeg) {

  static int NumRep = 0;
  int Humeur, i;
  locuteur *Locuteur = 0;
  char *NUS = NickUserStr (from);
  
  Locuteur = LocuteurExiste (currentbot->lists->ListeLocuteurs, from);
  if (!Locuteur)
	  Locuteur = AjouteLocuteur (currentbot->lists->ListeLocuteurs, NUS);
  
  Humeur = rellevel (from);

  if (Humeur >= 0) {
	
    /* Addition des points d'humeur positifs aux points de relation du
       locuteur */
    AjoutePoints (Locuteur, AjoutHumeurPos);

    if (time2hours (time (NULL)) - Locuteur->PremierContact > 12 &&
	Locuteur->bonus_malus != 0) {
      if (!exist_userhost (currentbot->lists->rellist, from))
	add_to_levellist (currentbot->lists->rellist, NUS, DEFAUT_LVL);

      if (Locuteur->bonus_malus > 0)
	add_to_level (currentbot->lists->rellist, from, +1);
      else if (Locuteur->bonus_malus < 0)
	add_to_level (currentbot->lists->rellist, from, -2);
      AnnulePoints (Locuteur);
      AnnuleBonjours (Locuteur);
      MAJPremierContact (Locuteur);
    }

    if (RepPos && NbRepPos)
      Ecrire (from, to, RepPos[NumRep%NbRepPos]);
    MAJDernierContact (Locuteur);
  } else {
    /* Addition des points d'humeur negatifs aux points de relation du
       locuteur */
    AjoutePoints (Locuteur, AjoutHumeurNeg);

    if (time2hours (time (NULL)) - Locuteur->PremierContact > 12 &&
	Locuteur->bonus_malus != 0) {
      if (!exist_userhost (currentbot->lists->rellist, from))
	add_to_levellist (currentbot->lists->rellist, NUS, DEFAUT_LVL);

      if (Locuteur->bonus_malus > 0)
	add_to_level (currentbot->lists->rellist, from, +1);
      else if (Locuteur->bonus_malus < 0)
	add_to_level (currentbot->lists->rellist, from, -2);
      AnnulePoints (Locuteur);
      AnnuleBonjours (Locuteur);
      MAJPremierContact (Locuteur);
    }

    if (NbRepNeg && RepNeg)
      Ecrire (from, to, RepNeg[NumRep%NbRepNeg]);
    MAJDernierContact (Locuteur);
  }

  NumRep++;

  /* Desallocation des reponses positives */
  for (i = 0; i < NbRepPos; i++)
    if (RepPos[i])
      free (RepPos[i]);

  if (NbRepPos > 0 && RepPos)
    free (RepPos);

  /* Desallocation des reponses negatives */
  for (i = 0; i < NbRepNeg; i++)
    if (RepNeg[i])
      free (RepNeg[i]);

  if (NbRepNeg > 0 && RepNeg)
    free (RepNeg);
  
  if (NUS)
    free (NUS);

} /* void Repondre */

void    Kicker (char *from, char *to, char *phrase) {
  char Ecrite[MAXLEN];
  char *Canal = ischannel(to)?to:(channelname (getnick (from))?channelname (getnick (from)):currentchannel ());
 
 sprintf (Ecrite, (phrase?phrase:""), GetNick (from), GetNick (from), GetNick (from), GetNick (from), GetNick (from));

  if (protlevel (from) < 50) {
/*     if (username (from)) { */
      

    if (is_log_on (Canal))
      botlog (LOGFILE, "<%s kicke %s de %s> %s", currentbot->botname, getnick (from),
	      /* (ischannel(to)?to:currentchannel ()), */
	      Canal,
	      Ecrite);

    sendkick (
	      /*(ischannel(to)?to:currentchannel ()), */
	      Canal,
	      getnick (from), Ecrite);
/*     } else { */
/*       char **Reponse, **Reponse2; */
      
/*       Reponse = malloc (2*sizeof (char *)); */
      
/*       Reponse[0] = strdup ("T'as de la chance de ne pas etre dans un canal ou je suis, je me serai vu oblige de te kicker, %s."); */
/*       Reponse[1] = strdup ("La, %s, normalement je te kickais."); */

/*       Reponse2 = malloc (4*sizeof (char *)); */
      
/*       Reponse2[0] = strdup ("C'est bien parce que tu n'es pas dans un canal que je ne te kicke pas, %s!"); */
/*       Reponse2[1] = strdup ("Tiens le toi pour dit, %s: quand tu seras sur un canal ou je suis, je ne me generai pas pour te kicker!"); */
/*       Reponse2[2] = strdup ("Je t'en prie, ne vient pas dans un canal ou je suis, %s!"); */
/*       Reponse2[3] = strdup ("Va te faire voir chez les Grecs, %s!"); */
      
/*       Repondre (from, to, 0, 2, Reponse, 0, 4, Reponse2); */
/*     }  */

  } else {
    char **Reponse2;

    Reponse2 = malloc (4*sizeof (char *));

    Reponse2[0] = strdup ("C'est bien parce que je suis censé te protéger que je ne te kicke pas, %s!");
    Reponse2[1] = strdup ("Tiens le toi pour dit, %s: quand tu ne seras plus protégé, je ne me gênerai pas pour te kicker!");
    Reponse2[2] = strdup ("Je t'en prie, ne reste pas dans ce canal, %s!");
    Reponse2[3] = strdup ("Va te faire voir chez les Grecs, %s!");

    Repondre (from, to, 0, 0, 0, 0, 4, Reponse2);
  }
} /* void Kicker */


void    KickerRepondre (char *from, char *to,
			int AjoutHumeurPos,
			int NbRepPos, char **RepPos,
			int AjoutHumeurNeg,
			int NbRepNeg, char **RepNeg) {
  static  int NumRep = 0;
  int Humeur, i;
  locuteur *Locuteur = 0;
  char *NUS = NickUserStr (from);

  Locuteur = LocuteurExiste (currentbot->lists->ListeLocuteurs, from);
  if (!Locuteur)
    Locuteur = AjouteLocuteur (currentbot->lists->ListeLocuteurs,
			       NUS);
  
  Humeur = rellevel (from);

  if (Humeur >= 0) {
	
    /* Addition des points d'humeur positifs aux points de relation du
       locuteur */
    AjoutePoints (Locuteur, AjoutHumeurPos);

    if (time2hours (time (NULL)) - Locuteur->PremierContact > 12 &&
	Locuteur->bonus_malus != 0) {
      if (!exist_userhost (currentbot->lists->rellist, from))
	add_to_levellist (currentbot->lists->rellist, NickUserStr (from), DEFAUT_LVL);

      if (Locuteur->bonus_malus > 0)
	add_to_level (currentbot->lists->rellist, from, +1);
      else if (Locuteur->bonus_malus < 0)
	add_to_level (currentbot->lists->rellist, from, -2);
      AnnulePoints (Locuteur);
      AnnuleBonjours (Locuteur);
      MAJPremierContact (Locuteur);
    }

    if (RepPos && NbRepPos>0)
      Ecrire (from, to, RepPos[NumRep%NbRepPos]);
    MAJDernierContact (Locuteur);
  } else {
    /* Addition des points d'humeur negatifs aux points de relation du
       locuteur */
    AjoutePoints (Locuteur, AjoutHumeurNeg);

    if (time2hours (time (NULL)) - Locuteur->PremierContact > 12 &&
	Locuteur->bonus_malus != 0) {
      if (!exist_userhost (currentbot->lists->rellist, from))
	add_to_levellist (currentbot->lists->rellist, NickUserStr (from), 0);

      if (Locuteur->bonus_malus > 0)
	add_to_level (currentbot->lists->rellist, from, +1);
      else if (Locuteur->bonus_malus < 0)
	add_to_level (currentbot->lists->rellist, from, -2);
      AnnulePoints (Locuteur);
      AnnuleBonjours (Locuteur);
      MAJPremierContact (Locuteur);
    }
    
    if (RepNeg && NbRepNeg>0)
      Kicker (from, to, RepNeg[NumRep%NbRepNeg]);
    MAJDernierContact (Locuteur);
  }

  NumRep++;

  /* Desallocation des reponses positives */
  for (i = 0; i < NbRepPos; i++)
    if (RepPos[i])
      free (RepPos[i]);

  if (NbRepPos > 0 && RepPos)
    free (RepPos);

  /* Desallocation des reponses negatives */
  for (i = 0; i < NbRepNeg; i++)
    if (RepNeg[i])
      free (RepNeg[i]);

  if (NbRepNeg > 0 && RepNeg)
    free (RepNeg);
  
  if (NUS)
    free (NUS);

} /* void KickerRepondre */

void    KickerKicker (char *from, char *to,
			int AjoutHumeurPos,
			int NbRepPos, char **RepPos,
			int AjoutHumeurNeg,
			int NbRepNeg, char **RepNeg) {
  static  int NumRep = 0;
  int Humeur, i;
  locuteur *Locuteur = 0;
  char *NUS = NickUserStr (from);

  Locuteur = LocuteurExiste (currentbot->lists->ListeLocuteurs, from);
  if (!Locuteur)
    Locuteur = AjouteLocuteur (currentbot->lists->ListeLocuteurs,
			       NUS);
  
  Humeur = rellevel (from);

  if (Humeur >= 0) {
	
    /* Addition des points d'humeur positifs aux points de relation du
       locuteur */
    AjoutePoints (Locuteur, AjoutHumeurPos);

    if (time2hours (time (NULL)) - Locuteur->PremierContact > 12 &&
	Locuteur->bonus_malus != 0) {
      if (!exist_userhost (currentbot->lists->rellist, from))
	add_to_levellist (currentbot->lists->rellist, NickUserStr (from), 0);

      if (Locuteur->bonus_malus > 0)
	add_to_level (currentbot->lists->rellist, from, +1);
      else if (Locuteur->bonus_malus < 0)
	add_to_level (currentbot->lists->rellist, from, -2);
      AnnulePoints (Locuteur);
      AnnuleBonjours (Locuteur);
      MAJPremierContact (Locuteur);
    }

    if (RepPos && NbRepPos>0)
      Kicker (from, to, RepPos[NumRep%NbRepPos]);
    MAJDernierContact (Locuteur);
  } else {
    /* Addition des points d'humeur negatifs aux points de relation du
       locuteur */
    AjoutePoints (Locuteur, AjoutHumeurNeg);

    if (time2hours (time (NULL)) - Locuteur->PremierContact > 12 &&
	Locuteur->bonus_malus != 0) {
      if (!exist_userhost (currentbot->lists->rellist, from))
	add_to_levellist (currentbot->lists->rellist, NickUserStr (from), 0);

      if (Locuteur->bonus_malus > 0)
	add_to_level (currentbot->lists->rellist, from, +1);
      else if (Locuteur->bonus_malus < 0)
	add_to_level (currentbot->lists->rellist, from, -2);
      AnnulePoints (Locuteur);
      AnnuleBonjours (Locuteur);
      MAJPremierContact (Locuteur);
    }
    
    if (RepNeg && NbRepNeg>0)
      Kicker (from, to, RepNeg[NumRep%NbRepNeg]);
    MAJDernierContact (Locuteur);
  }

  NumRep++;

  /* Desallocation des reponses positives */
  for (i = 0; i < NbRepPos; i++)
    if (RepPos[i])
      free (RepPos[i]);

  if (NbRepPos > 0 && RepPos)
    free (RepPos);

  /* Desallocation des reponses negatives */
  for (i = 0; i < NbRepNeg; i++)
    if (RepNeg[i])
      free (RepNeg[i]);

  if (NbRepNeg > 0 && RepNeg)
    free (RepNeg);
  
  if (NUS)
    free (NUS);

} /* void KickerKicker */


void	on_kick(char *from, char *channel, char *nick, char *reason)
{
  locuteur *Locuteur;
  char **RepPos, **RepNeg;
  /*
   * If user A kicks user B then kick user A if B was protected
   * and A has lower protlevel
   */
  if (is_log_on (channel))
    botlog (LOGFILE, "<%s kicke %s de %s> %s", from, nick, channel, reason?reason:"");
  /*
   * Perhaps vladbot is getting to much warbot with this piece
   * of code. Maybe add an #ifdef...
   */
  if(shitlevel(username(nick)) == 0 &&
     protlevel(username(nick)) >= 50 &&
     (protlevel(from) < 50 || !is_bot (currentbot->botlist, from, channel)) &&
     /* I should never revenge myself :) */
     !STRCASEEQUAL(currentbot->nick, getnick(from))) {
    
    if (rellevel (username (nick)) > 0) {
      RepPos = malloc (3*sizeof (char *));
      RepPos[0] = strdup ("Ça n'était pas très gentil, ça, %s!");
      RepPos[1] = strdup ("Je ne peux permettre cela, %s.");
      RepPos[2] = strdup ("Oui mais non!");
      RepNeg = malloc (3*sizeof (char *));
      RepNeg[0] = strdup ("Ça va pas, non?");
      RepNeg[1] = strdup ("Méchant pas beau!");
      RepNeg[2] = strdup ("Décidément, ta tête me r'vient pas, %s!");
      KickerKicker (from, channel, -10, 3, RepPos, -10, 3, RepNeg);
    } else {
      RepPos = malloc (3*sizeof (char *));
      RepPos[0] = strdup ("Je sais bien qu'il est pas très aimable, mais quand-même, %s!");
      RepPos[1] = strdup ("J'aime pas l'ambiance qui s'installe ici, %s.");
      RepPos[2] = strdup ("Bon, je tolère, mais paske je l'aime pas.");
      RepNeg = malloc (3*sizeof (char *));
      RepNeg[0] = strdup ("Allez vous tapez dessus ailleurs!");
      RepNeg[1] = strdup ("C'est pas un tripot ici!");
      RepNeg[2] = strdup ("Dehors %s!");
      KickerRepondre (from, channel, -5, 3, RepPos, -9, 3, RepNeg);
    }

  } else if (rellevel (username (nick)) > 0 && /* nick sympathique */
	     rellevel (username (nick)) > rellevel (from) && /* plus que le kickeur */
	     !STRCASEEQUAL(currentbot->nick, getnick(from)) /* nick pas moi */) {
    RepPos = malloc (3*sizeof (char *));

    RepPos[0] = strdup ("/me n'aime pas qu'on kicke ses amis, %s.");
    RepPos[1] = strdup ("Calme toi %s. Je vais finir par ne plus t'apprécier du tout!");
    RepPos[2] = strdup ("Mais c'est pas vrai, %s! Du calme!");

    RepNeg = malloc (3*sizeof (char *));
    
    RepNeg[0] = strdup ("C'est pas bientôt fini, oui?");
    RepNeg[1] = strdup ("On se calme!");
    RepNeg[2] = strdup ("Laisse mes copains tranquilles!");

    if (protlevel (from) >= 50)
      Repondre (from, channel, -3, 3, RepPos, -5, 3, RepNeg);
    else 
      KickerRepondre (from, channel, -2, 3, RepPos, -4, 3, RepNeg);
  }

  if (STRCASEEQUAL (currentbot->nick, nick)) {
    send_to_user (from, "Je te préviens: je n'ai pas beaucoup apprécié. Continue comme ça et tu te retrouveras sur ma liste noire!");
    if (rellevel (from) < SYMPA_LVL) {
      if (!exist_userhost (currentbot->lists->shitlist, from))
	add_to_levellist (currentbot->lists->shitlist, NickUserStr (from), +1);
      else
	add_to_level (currentbot->lists->shitlist, from, +1);
    } else {
      char **RepPos;
      RepPos = malloc (6*sizeof (char *));
      RepPos[0] = strdup ("Je ne supporte pas ça, %s!");
      RepPos[1] = strdup ("Qu'est-ce qui te prends, %s?");
      RepPos[2] = strdup ("Oui mais non!");
      RepPos[3] = strdup ("Ça va pas, non?");
      RepPos[4] = strdup ("Tu as décidé de me décevoir, %s?");
      RepPos[5] = strdup ("Fichtre! Diantre, %s! Je ne supporterai pas ça derechef!");
      KickerKicker (from, channel, -10, 6, RepPos, -10, 0, NULL);
    }
  }
}

void 	on_join(char *who, char *channel)
{
  static int NumPhrase = -1;
  int Num;
  int HeureCourante;
  locuteur *Locuteur;
  char **Reponse, **Reponse2;
  char *NUS = NickUserStr (who);
  char *pNUS = NUS;
  
  /* si c'est moi, je ne fais rien (je ne peux rien faire d'autre que
     dire bonjour...) */
  //  if (strcmp(NUS,currentbot->botname)==0) {
  if (strcmp(get_token(&pNUS,"!"),currentbot->nick)==0) {
	Reponse = malloc (6*sizeof (char *));
	Reponse[0] = strdup ("Bonjour à tous!");
	Reponse[1] = strdup ("Bonjour, c'est moi, %s.");
	Reponse[2] = strdup (" La foule: «Ah, enfin!» :)");
	Reponse[3] = strdup ("Merci de m'accueillir avec bienveillance!");
	Reponse[4] = strdup ("Heureux de vous voir.");
	Reponse[5] = strdup ("Bonjour, ça va?");
	Repondre (who, channel, +0, 6, Reponse, +0, 0, 0);
	if (NUS)
	  free (NUS);
	return;
  }

  if(shitlevel(who) == 100)
    {
      char **RepPos, **RepNeg;
      
      ban_user(who, channel);
      
      RepPos = malloc (3*sizeof (char *));
      RepPos[0] = strdup ("Tu n'as plus rien à faire ici, %s.");
      RepPos[1] = strdup ("Il m'est interdit de t'admettre en ces lieux, %s.");
      RepPos[2] = strdup ("Fais-toi enlever de ma liste noire, %s.");
      RepNeg = malloc (3*sizeof (char *));
      RepNeg[0] = strdup ("Reste dehors, %s!");
      RepNeg[1] = strdup ("On ne veut plus de toi ici, %s!");
      RepNeg[2] = strdup ("Dégage, %s!");
      KickerKicker (who, channel, 0, 3, RepPos, 0, 3, RepNeg);
      if (NUS)
	free (NUS);
      return;
    }

  if(userlevel(who) >= AUTO_OPLVL && shitlevel(who) == 0 && 
     is_bot (currentbot->botlist, channel, who)) {
    giveop(channel, getnick(who));
    if (NUS)
      free (NUS);
    return;
  }

  Locuteur = LocuteurExiste (currentbot->lists->ListeLocuteurs, who);
  if (!Locuteur)
    Locuteur = AjouteLocuteur (currentbot->lists->ListeLocuteurs,
			       NUS);
  
  HeureCourante = time2hours (time (NULL));
  
  if (rellevel (who) > SYMPA_LVL &&
	 !is_bot (currentbot->botlist, channel, who)) {
    NumPhrase++;
    /* Si ca fait plus de 8 heures qu'on n'a pas entendu le locuteur */
    if (HeureCourante-Locuteur->DernierContact > 8) {
      Reponse = malloc (6*sizeof (char *));
      Reponse[0] = strdup ("Salut %s.");
      Reponse[1] = strdup ("Bienvenue à toi, %s.");
      Reponse[2] = strdup ("Enfin de retour, %s! :)");
      Reponse[3] = strdup ("Super! %s est arrivé!");
      Reponse[4] = strdup ("Heureux de te voir, %s!");
      Reponse[5] = strdup ("Bonjour %s.");
      
      Repondre (who, channel, +0, 6, Reponse, +0, 0, 0);
      if (!LocuteurExiste (currentbot->lists->ListeLocuteurs, who))
	Locuteur = AjouteLocuteur (currentbot->lists->ListeLocuteurs,
				   NUS);
      Locuteur->Bonjours = 1;
    } else {
      Reponse = malloc (8*sizeof (char *));
      Reponse[0] = strdup ("re %s.");
      Reponse[1] = 0;
      Reponse[2] = strdup ("Areuh %s.");
      Reponse[3] = 0;
      Reponse[4] = strdup ("Bon retour parmi nous, %s!");
      Reponse[5] = strdup ("Tiens! Revoila %s. :)");
      Reponse[6] = strdup ("Super! %s est de retour!");
      Reponse[7] = 0;
      
      Repondre (who, channel, +0, 8, Reponse, +0, 0, 0);
    }
  } else if (rellevel (who) < -SYMPA_LVL
		   && HeureCourante - Locuteur->DernierContact > 8
		   && !is_bot (currentbot->botlist, channel, who)) {
    Reponse2 = malloc (7*sizeof (char *));
    Reponse2[0] = strdup ("%s: Grrrr");
    Reponse2[1] = strdup ("T'ose encore revenir ici, %s? T'es gonflé!");
    Reponse2[2] = strdup ("Oh! Non! %s! :[");
    Reponse2[3] = strdup ("Zut! %s est arrivé!");
    Reponse2[4] = strdup ("Encore toi, %s! :(");
    Reponse2[5] = strdup ("Je ne te souhaite pas le bonjour %s.");
    Reponse2[6] = strdup ("T'as intérêt à t'excuser, %s.");
    
    Repondre (who, channel, +0, 0, 0, +0, 7, Reponse2);
    if (!LocuteurExiste (currentbot->lists->ListeLocuteurs, who))
      Locuteur = AjouteLocuteur (currentbot->lists->ListeLocuteurs,
				 NUS);
    Locuteur->Bonjours = 1;
  }
  if (NUS)
    free (NUS);
}

void	on_mode(char *from, char *rest)
/*
 * from = who did the modechange
 * rest = modestring, usually of form +mode-mode param1 .. paramN
 */
#define REMOVEMODE(chr, prm) do{ strcat(unminmode, chr); \
			         strcat(unminparams, prm); \
			         strcat(unminparams, " "); \
			     } while(0)

#define ADDMODE(chr, prm)    do{ strcat(unplusmode, chr); \
			         strcat(unplusparams, prm); \
			         strcat(unplusparams, " "); \
			     } while(0)
{
    static  int NumPhrase = 0;
	int     Numero;
	int	did_change=FALSE;	/* If we have to change anything */
	char	*channel;

	char	*chanchars;	/* i.e. +oobli */
	char	*params;
	char	sign = '+';	/* +/-, i.e. +o, -b etc */

	char	unminmode[MAXLEN] = "";		/* used to undo modechanges */
	char	unminparams[MAXLEN] = "";
	char	unplusmode[MAXLEN] = "";	/* used to undo modechanges */
	char	unplusparams[MAXLEN] = "";

	/* these are here merely to make things clearer */
	char	*nick;
	char	*banstring;
	char	*key;
	char	*limit;

	char **RepPos, **RepNeg;

#ifdef DBUG
	debug(NOTICE, "on_mode(\"%s\", \"%s\")", from, rest);
#endif

	channel = get_token(&rest, " ");
	if(STRCASEEQUAL(channel, currentbot->nick))	/* if target is me... */
		return;			/* mode NederServ +i etc */

	chanchars = get_token(&rest, " ");
	params = rest;

	while( *chanchars )
	{
		switch( *chanchars )
		{
		case '+':
		case '-':
			sign = *chanchars;
			break;
		case 'o':
			nick = get_token(&params, " ");
			if(sign == '+')
			{
				add_channelmode(channel, CHFL_CHANOP, nick);
#ifdef ONLY_OPS_FOR_REGISTRED
				if(shitlevel(username(nick)) > 0 ||
				   userlevel(username(nick)) == 0)
#else
				if(shitlevel(username(nick))>=50)
#endif
				{
					REMOVEMODE("o", nick);
					send_to_user(from, 
#ifdef ONLY_OPS_FOR_REGISTRED
						"Sorry, I can't allow this :)");
#else
						"%s is shitted! :P", nick);
#endif
					did_change=TRUE;
				}
			       /* Si c'est quelqu'un qu'on voulait opper */
			       else if (STRCASEEQUAL (nick, currentbot->nick)&&
					!is_bot (currentbot->botlist, from, channel))
				 { /* Merci */
				   RepPos = malloc (7 * sizeof (char *));
				   RepPos[0] = strdup ("Merci %s.");
				   RepPos[1] = 0;
				   RepPos[2] = strdup ("C'est gentil, ça, %s.");
				   RepPos[3] = 0;
				   RepPos[4] = strdup ("Tu as toute ma gratitude, %s!");
				   RepPos[5] = strdup ("C'est gentil de ta part, %s.");
				   RepPos[6] = 0;

				   RepNeg = malloc (3 * sizeof (char *));
				   RepNeg[0] = strdup ("Étonnant de ta part, %s!");
				   RepNeg[1] = strdup ("Venant de toi, ça m'étonne, %s!");
				   RepNeg[2] = strdup ("Je suppose que tu attends un peu de gratitude de ma part, %s.");
				   Repondre (from, channel, +1, 7, RepPos, +2, 3, RepNeg);
				 }
			}
			else
			{
			  if (STRCASEEQUAL (nick, currentbot->nick) &&
			      !is_bot (currentbot->botlist, from, channel)) {
			    RepPos = malloc (3 * sizeof (char *));
			    RepPos[0] = strdup ("Je suis déçu %s.");
			    RepPos[1] = strdup ("C'est pas gentil, ça, %s.");
			    RepPos[2] = strdup ("S'il-te-plaît, %s, redonne-moi les droits!");
					
			    RepNeg = malloc (3 * sizeof (char *));
			    RepNeg[0] = strdup ("Espèce de charogne, %s!");
			    RepNeg[1] = strdup ("Pourquoi tu me déoppes %s?");
			    RepNeg[2] = strdup ("Ne recommence jamais ça!");
			    Repondre (from, channel, -2, 3, RepPos, -10, 3, RepNeg);
			    return;
			  }
			  
				del_channelmode(channel, CHFL_CHANOP, nick);
				if((protlevel(username(nick))>=100)&&
				   (shitlevel(username(nick))==0))
				{
					ADDMODE("o", nick);
					RepPos = malloc (3 * sizeof (char *));
					RepPos[0] = strdup ("Je protège cet utilisateur, désolé %s.");
					RepPos[1] = strdup ("Prière de ne pas recommencer, %s. Je ne le permets pas.");
					RepPos[2] = strdup ("Ne recommence pas tant que je suis chargé de protéger cet utilisateur, si tu veux rester en bon termes avec moi, %s.");
					
					RepNeg = malloc (3 * sizeof (char *));
					RepNeg[0] = strdup ("Et pis quoi encore %s? Tu te crois tout permis. Je ne veux pas!");
					RepNeg[1] = strdup ("Estime-toi heureux que je ne te kicke pas, %s! Je suis chargé de protéger cet utilisateur.");
					RepNeg[2] = strdup ("Pas question de déopper cet utilisateur!");
					Repondre (from, channel, 0, 3, RepPos, -1, 3, RepNeg);
/*
					NumPhrase ++;
					Numero = NumPhrase % 3;
					switch (Numero) {
					case 0:
					  send_to_user(from, 
								   "%s is protected!",
								   nick);
					  break;
					case 1:
					  send_to_user(from,
								   "Pas question: je protège %s!",
								   nick);
					  break;
					case 2:
					  send_to_user (from,
									"%s est sous ma protection.",
									nick);
					  break;
					}
*/
					did_change=TRUE;
				}	
			}
			break;
		case 'v':
			nick = get_token(&params, " ");
			if(sign == '+')
				add_channelmode(channel, CHFL_VOICE, nick);
			else
				del_channelmode(channel, CHFL_VOICE, nick);
			break;
		case 'b':
			banstring = get_token(&params, " ");
                        if(sign == '+')
			{
                                add_channelmode(channel, MODE_BAN, banstring);
				if(find_highest(channel, banstring) >= 50)
				{
					REMOVEMODE("b", banstring);
					did_change = TRUE;
				}
				
			}
                        else
                                del_channelmode(channel, MODE_BAN, banstring);
                        break;
		case 'p':
			if(sign == '+')
				add_channelmode(channel, MODE_PRIVATE, "");
			else
				del_channelmode(channel, MODE_PRIVATE, "");
			break;
		case 's':
			if(sign == '+')
				add_channelmode(channel, MODE_SECRET, "");
			else
				del_channelmode(channel, MODE_SECRET, "");
			break;
		case 'm':
			if(sign == '+')
				add_channelmode(channel, MODE_MODERATED, "");
			else
				del_channelmode(channel, MODE_MODERATED, "");
			break;
		case 't':
			if(sign == '+')
				add_channelmode(channel, MODE_TOPICLIMIT, "");
			else
				del_channelmode(channel, MODE_TOPICLIMIT, "");
			break;
		case 'i':
			if(sign == '+')
				add_channelmode(channel, MODE_INVITEONLY, "");
			else
				del_channelmode(channel, MODE_INVITEONLY, "");
			break;
		case 'n':
			if(sign == '+')
				add_channelmode(channel, MODE_NOPRIVMSGS, "");
			else
				del_channelmode(channel, MODE_NOPRIVMSGS, "");
			break;
		case 'k':
                       	key = get_token(&params, " ");
			if(sign == '+')
			{
				char	*s;

				add_channelmode(channel, MODE_KEY, 
						key?key:"???");
				/* try to unset bogus keys */
				for(s = key; key && *s; s++)
					if(*s < ' ')
					{
						REMOVEMODE("k", key);
						did_change = TRUE;
						send_to_user(from,
						"No bogus keys pls");
						break;
					}
			}
			else
				del_channelmode(channel, MODE_KEY, "");
			break;
		case 'l':
			if(sign == '+')
			{
                        	limit = get_token(&params, " ");
				add_channelmode(channel, MODE_LIMIT, 
						limit?limit:"0");
			}
			else
				del_channelmode(channel, MODE_LIMIT, "");
			break;
		default:
#ifdef DBUG
			debug(ERROR, "on_mode(): unknown mode %c", *chanchars);
#endif
			break;
		}
		chanchars++;
	}
	/* restore unwanted modechanges */
	if(did_change)
	  sendmode( channel, "+%s-%s %s %s", unplusmode, 
			  unminmode, unplusparams, unminparams);
}


void	on_msg(char *from, char *to, char *msg_untranslated)
{
	int	i;
	char	msg_copy[BIG_BUFFER];	/* for session */
	char	*command;
	DCC_list	*userclient;
	char **RepPos, **RepNeg;
	int     Flooding;
	locuteur *Locuteur;

	/* on va tenter de traduire l'encodage si possible */
	/* je ne sais pas ce que la recherche de channel va couter en perfs */
	char *msg, *msg_ori = msg_untranslated;
	CHAN_list *channel;
	char buf[MAXLEN];
	char *pbuf = buf;//, *pmsg = &msg_untranslated;
	size_t lin, lout;
	//pmsg = (char *)&msg_translated;
	if(ischannel(to)){
		channel = search_chan(to);
		if(channel->decoder != (iconv_t)-1){
			lin = strlen(msg_untranslated);
			lout = sizeof(buf);
			memset(buf, 0, sizeof(buf));
			if(iconv(channel->decoder, &msg_untranslated, &lin, &pbuf, &lout) != -1)
				msg = buf;
			else
				msg = msg_ori;
		}
		else
			msg = msg_untranslated;
	}
	else{
		//on va essayer de voir si on peut décoder de l'utf
		lin = strlen(msg_untranslated);
		lout = sizeof(buf);
		memset(buf, 0, sizeof(buf));
		if (iconv(currentbot->def_decoder, &msg_untranslated, &lin, &pbuf, &lout) != -1)
			msg = buf;
		else
			msg = msg_ori;
	}

	//garde une trace des locuteurs
	Locuteur = LocuteurExiste (currentbot->lists->ListeLocuteurs, from);
	if (!Locuteur)
		Locuteur = AjouteLocuteur (currentbot->lists->ListeLocuteurs, NickUserStr(from));
	MAJDerniereActivite(Locuteur);

#ifdef DBUG
		printf("on_msg: from %s to %s msg : %s\n", from, to, msg);
#endif

	strcpy(msg_copy, msg);

        Flooding = check_session (from);
        if (Flooding != IS_FLOODING && !is_bot (currentbot->botlist, to, from))
          Traite (from, to, msg);
        else if (Flooding == IS_FLOODING)
          return;

	if((command = get_token(&msg, ",: "))== NULL)
		return;			/* NULL-command */

	if(STRCASEEQUAL(currentbot->nick, command))
	{
		if((command = get_token(&msg, ",: "))==NULL)
			return;		/* NULL-command */
	}
	else if((*command != CommandChar) && !STRCASEEQUAL(to, currentbot->nick))
		return; /* The command should start with CommandChar if public */

	if(*command == CommandChar)
		command++;


	/*
	 * Now we got
	 * - A public command.
	 * - A private command/message. 
	 * Time to do a flood check :).
	 */
/* 	if(check_session(from) == IS_FLOODING) */
/* 		return; */
/*       voir plus haut. */
/*         if(Flooding == IS_FLOODING)	 */
/*           return; */

	for(i = 0; on_msg_commands[i].name != NULL; i++)
		if(STRCASEEQUAL(on_msg_commands[i].name, command))
		{
			if(userlevel(from) < on_msg_commands[i].userlevel)
			{
				send_to_user(from, "Userlevel too low");
				return;
			}
		        if(shitlevel(from) > on_msg_commands[i].shitlevel)
			{
				send_to_user(from, "Shitlevel too high");
				return;
			}
                        if (rellevel (from) < on_msg_commands[i].rellevel)
                        {
				RepPos = malloc (3 * sizeof (char *));
				RepPos[0] = strdup ("Je ne te connais pas encore assez pour ça, %s");
				RepPos[1] = strdup ("C'est pas que je ne t'aime pas, mais je ne te connais pas encore assez pour t'autoriser cette commande, %s.");
				RepPos[2] = strdup ("Dommage, cette commande réclame que je te fasse encore plus confiance, %s!");

				RepNeg = malloc (3 * sizeof (char *));
				RepNeg[0] = strdup ("Ça va pas la tête, %s? Que je te laisse faire ça? À toi? Quand tu montreras un peu plus de respect!");
				RepNeg[1] = strdup ("Uniquement aux gens que j'aime ou que je respecte assez, %s. Apparemment, c'est pas ton cas.");
				RepNeg[2] = strdup ("Si tu veux que je t'autorise un jour à faire ça, t'as intérêt à me demander pardon et à te conduire gentiment avec moi.");
				Repondre (from, to, 0, 3, RepPos, 0, 3, RepNeg);
/*                              send_to_user (from, "Je ne te connais pas assez pour ça!"); */
                                return;
                        }
			userclient = search_list("chat", from, DCC_CHAT);
			if(on_msg_commands[i].forcedcc && 
		    	  (!userclient || (userclient->flags&DCC_WAIT)))
			{
#ifdef AUTO_DCC
				dcc_chat(from, msg);
				nodcc_session(from, to, msg_copy);
				sendnotice(getnick(from), "Please type: /dcc chat %s", 
					currentbot->nick);
#else
				sendnotice( getnick(from), 
					"Sorry, %s is only available through DCC",
					command );
                		sendnotice( getnick(from), 
					"Please start a dcc chat connection first" );
#endif /* AUTO_DCC */
                			return;
			}
			/* if we're left with a null-string (""), give NULL
			   as msg */
			on_msg_commands[i].function(from, to, *msg?msg:NULL);
			return;
		}
	/* If the command was private, let the user know how stupid (s)he is */
/* 	if(STRCASEEQUAL(to, currentbot->nick)) */
/* 		send_to_user(from, "%s %s?!? What's that?", */
/* 			     command, msg ? msg : ""); */
}

void	show_help(char *from, char *to, char *rest)
{
	FILE	*f;
	char	*s;

	if((f=fopen(currentbot->helpfile, "r"))==NULL)
	{
		send_to_user(from, "Helpfile missing");
		return;
	}
	
	if(rest == NULL)
	{
		find_topic(f, "standard");
		while((s=get_ftext(f)))
			send_to_user(from, s);
	}
	else
		if(!find_topic( f, rest ))
			send_to_user(from, "No help available for \"%s\"", rest);
		else
			while((s=get_ftext(f)))
				send_to_user(from, s);
	fclose(f);
}

void	show_whoami(char *from, char *to, char *rest)
{
#ifdef DBUG
	printf("WHOAMI : You are %s. Your levels are:\n", from);
	printf("-- User -+- Shit -+- Protect --+- Relation -+\n");
	printf("    %3d  |   %3d  |      %3d   |       %3d  |\n",userlevel(from),
		     shitlevel(from), protlevel(from), rellevel (from));
#endif
	send_to_user(from, "You are %s. Your levels are:", 
		     from);
	send_to_user(from, "-- User -+- Shit -+- Protect --+- Relation -+");
	send_to_user(from, "    %3d  |   %3d  |      %3d   |       %3d  |",
		     userlevel(from),
		     shitlevel(from), protlevel(from), rellevel (from));
        return;
}

void	show_info(char *from, char *to, char *rest)
{
	sendreply("I am VladBot version %s (%s)", VERSION, currentbot->botname);
	sendreply("Started: %-20.20s", time2str(currentbot->uptime));
	sendreply("Up: %s", idle2str(time(NULL)-currentbot->uptime));
	if(botmaintainer)
		sendreply("This bot is maintained by %s", botmaintainer);
        return;
}

void	show_time(char *from, char *to, char *rest)
{
	sendreply("Current time: %s", time2str(time(NULL)));
}

void	show_userlist(char *from, char *to, char *rest)
{
	send_to_user(from, " userlist: %30s   %s", "nick!user@host", "level");
        send_to_user(from, " -----------------------------------------+------");
/* 	if (rest) */
	  show_lvllist(currentbot->lists->opperlist, from, rest?rest:"");
/* 	else  */
/* 	  send_to_user(from, "Il faut mettre un paramètre. Je ne tiens pas à transmettre trop de données d'un seul coup."); */
}

void	show_shitlist(char *from, char *to, char *rest)
{
	send_to_user(from, " shitlist: %30s   %s", "nick!user@host", "level");
        send_to_user(from, " -----------------------------------------+------");
/* 	if (rest) */
	  show_lvllist(currentbot->lists->shitlist, from, rest?rest:"");
/* 	else  */
/* 	  send_to_user(from, "Il faut mettre un paramètre. Je ne tiens pas à transmettre trop de données d'un seul coup."); */
}

void    show_protlist(char *from, char *to, char *rest)
{
        send_to_user(from, " protlist: %30s   %s", "nick!user@host", "level");
        send_to_user(from, " -----------------------------------------+------");
/* 	if (rest) */
	  show_lvllist(currentbot->lists->protlist, from, rest?rest:"");
/* 	else */
/* 	  send_to_user(from, "Il faut mettre un paramètre. Je ne tiens pas à transmettre trop de données d'un seul coup."); */
}

void    show_rellist (char *from, char *to, char *rest)
{
   send_to_user(from, "  rellist: %30s   %s", "nick!user@host", "level");
   send_to_user(from, " -----------------------------------------+------");
   if (rest)
     show_lvllist(currentbot->lists->rellist, from, rest?rest:"");
   else
     send_to_user(from, "Il faut mettre un paramètre. Je ne tiens pas à transmettre trop de données d'un seul coup.");
}

void    show_mycmds (char *from, char *to, char *rest)
{
  int user_level;
  int i=0;
  user_level = userlevel (from);

  send_to_user(from, " Commands available to %s: ", getnick (from));
  
  while(on_msg_commands[i].name) {
    if (on_msg_commands[i].userlevel <= user_level && on_msg_commands[i].rellevel <= rellevel (from))
      send_to_user (from, " Command %s", on_msg_commands[i].name);
    i++; /* Passage a la commande suivante */
  }
  
}

void    show_cmdlvls (char *from, char *to, char *rest) 
{
  int i = 0;

  send_to_user (from, " Commands and levels required:");

  while (on_msg_commands[i].name) {
    send_to_user (from, " Command %9s, %3du, %3ds, %4dr",
		  on_msg_commands[i].name,
		  on_msg_commands[i].userlevel,
		  on_msg_commands[i].shitlevel,
		  on_msg_commands[i].rellevel);
    i++;
  }
  
}

void	do_op(char *from, char *to, char *rest)
{
	if(usermode(!STRCASEEQUAL(to, currentbot->nick) ? to : currentchannel(),  
		    getnick(from))&MODE_CHANOP) {
	  char **RepPos, **RepNeg;
	  RepPos = malloc (3 * sizeof (char *));
	  RepPos[0] = strdup ("Tête en l'air va! Tu es déjà op, %s.");
	  RepPos[1] = strdup ("Tu aspires à devenir surop, %s? ;)");
	  RepPos[2] = strdup ("Mets tes lunettes, %s, tu es déjà opérateur du canal!");
	  RepNeg = malloc (4 * sizeof (char *));
	  RepNeg[0] = strdup ("Ça te prend souvent, cette soif de pouvoir, %s?");
	  RepNeg[1] = strdup ("Calme-toi un peu, %s!");
	  RepNeg[2] = strdup ("Ça va pas, la tête? Je te signale que t'es déjà op, %s!");
	  RepNeg[3] = strdup ("Tu radotes, %s!");
	  Repondre (from, to, 0, 3, RepPos, -1, 4, RepNeg);
	} else if (userlevel (from) < 110) {
	  char **RepPos, **RepNeg;
	  RepPos = malloc (3 * sizeof (char *));
	  RepPos[0] = strdup ("Désolé, %s. Je n'oppe plus. Il vaut mieux utiliser !TOPIC, !KICK ou !BANUSER...");
	  RepPos[1] = strdup ("Pourquoi faire, %s? Utilise plutôt TOPIC, KICK ou BANUSER...");
	  RepPos[2] = strdup ("Je suis fatigué d'opper tout le monde! Utilise plutôt TOPIC, KICK ou BAN.");
	  RepNeg = malloc (4 * sizeof (char *));
	  RepNeg[0] = strdup ("Ça te prend souvent, cette soif de pouvoir, %s?");
	  RepNeg[1] = strdup ("Calme-toi un peu, %s!");
	  RepNeg[2] = strdup ("Ça va pas, la tête? Pas à toi, %s!");
	  RepNeg[3] = strdup ("Quand tu seras plus raisonnable, %s!");
	  Repondre (from, to, 0, 3, RepPos, -1, 4, RepNeg);
	} else {
	  giveop(!STRCASEEQUAL(to, currentbot->nick) ? to : currentchannel(),  
		 getnick(from));
	}
}

void	do_alarm(char *from, char *to, char *rest)
{
	time_t maintenant = time(NULL);
	time_t quand  = parsetime(rest?rest:"");
	struct	tm	*btime;

	if (quand == -1)
		send_to_user(from, "Date/Heure mauvaise");
	else
		if (quand == -2)
			send_to_user(from, "On remonte dans le temps ? ;-)");
		else
		{
			btime = localtime(&quand);
		  	signal (SIGALRM, sig_alrm);
			alarm(quand - maintenant);
			send_to_user(from, "Alarme activee pour le %-2.2d %s %d a %-2.2d:%-2.2d:%-2.2d (dans %ld secondes)",
				btime->tm_mday, months[btime->tm_mon],
				btime->tm_year + 1900, 
				btime->tm_hour, btime->tm_min, btime->tm_sec,
				quand-maintenant);
		}
}

void	do_giveop(char *from, char *to, char *rest)
{
	int	i = 0;
	char	nickname[MAXNICKLEN];
	char	op[MAXLEN];

	strcpy(op, "");
	if(not(rest))
	{
		sendreply("Who do you want me to op?");
		return;
	}
	while(readnick(&rest, nickname))
		if(username(nickname) != NULL && 
		   shitlevel(username(nickname)) == 0)
		{
			i++;
			strcat(op, " ");
			strcat(op, nickname);
			if(i==3)
			{
				sendmode(ischannel(to) ?to :currentchannel(), "+ooo %s", op);
				i = 0;
				strcpy(op, "");
			}
		}
	if(i != 0)
		sendmode(ischannel(to) ?to :currentchannel(), "+ooo %s", op);
	else
	  sendreply ("J'ai un problème: je ne trouve pas ce nickname. Désolé.");
}

void	do_deop(char *from, char *to, char *rest)
{
	int	i = 0;
	char	nickname[MAXNICKLEN];
	char	deop[MAXLEN];

	strcpy(deop, "");
	if(not(rest))
	{
		sendreply("Who do you want me to deop?");
		return;
	}
	while(readnick(&rest, nickname))
		if(username(nickname) != NULL &&
		  (protlevel(username(nickname)) < 100 ||
		   shitlevel(username(nickname)) > 0))
		{
		i++;
			strcat(deop, " ");
			strcat(deop, nickname);
			if(i==3)
			{
				sendmode(ischannel(to) ?to :currentchannel(), "-ooo %s", deop);
				i = 0;
				strcpy(deop, "");
			}
		}
	if(i != 0)
		sendmode(ischannel(to) ?to :currentchannel(), "-ooo %s", deop);
}

void	do_invite(char *from, char *to, char *rest)
{
        if(rest)
	{
		if(!invite_to_channel(getnick(from), rest))	
			send_to_user(from, "I'm not on channel %s", rest);
	}
	else
		invite_to_channel(from, currentchannel());
}
			
void	do_open(char *from, char *to, char *rest)
{
        if(rest)
        {
		if(!open_channel(rest))
			send_to_user(from, "I could not open %s!", rest);
		else
			channel_unban(rest, from);
	}
	else if(ischannel(to))
	{
		if(!open_channel(to))
			send_to_user(from, "I could not open %s!", to);
		else
			channel_unban(to, from);
	}
	else 
	{
		open_channel(currentchannel());
		channel_unban(currentchannel(), from);
	}
}

void    do_chat(char *from, char *to, char *rest)
{
	dcc_chat(from, rest);
	sendnotice(getnick(from), "Please type: /dcc chat %s", currentbot->nick);
}

void    do_send(char *from, char *to, char *rest)
{
	char	*pattern;
	
	if(rest)
		while((pattern = get_token(&rest, " ")))
			send_file(from, pattern);
	else
		send_to_user(from, "Please specify a filename (use !files)");
}

void	do_flist(char *from, char *to, char *rest)
{
	FILE	*ls_file;
	char	indexfile[MAXLEN];
	char	*s;
	char	*p;

	strcpy(indexfile, currentbot->indexfile );
	if(rest)
	{
		for(p=rest; *p; p++) *p=tolower(toascii(*p));
		sprintf(indexfile, "%s.%s", currentbot->indexfile, rest);
		if((ls_file = openindex(from, indexfile)) == NULL)
		{
			send_to_user(from, "No indexfile for %s", rest);
			return;
		}
	}
	else
		if((ls_file = openindex(from, currentbot->indexfile)) == NULL)
		{
			send_to_user(from, "No indexfile");
			return;
		}

	while((s=get_ftext(ls_file)))
		send_to_user(from, s);
	fclose(ls_file);
}	

void	do_say(char *from, char *to, char *rest)
{
	char Utfized[MAXLEN*2], *out = (char*)&Utfized;
	size_t lin, lout;
	CHAN_list *Channel_to;

	if (rest && strlen (rest) > 1 && rest[0] == '#' && strchr (&rest[1],' ')) {
		to = get_token (&rest, " ");
	};
	
	if(rest) {
		if(to && ischannel(to)) {
			/*Channel_to = search_chan(to);
			if(Channel_to->encoder != (iconv_t)-1){
				//we have aniconv encoder defined, use it
				lin = strlen(rest);
				lout = sizeof(Utfized);
				iconv(Channel_to->encoder,&rest, &lin, &out, &lout);
				rest = Utfized;
				}*/
			if (is_log_on (to))
				botlog (LOGFILE, "<%s#%s> %s", currentbot->botname, to, rest);
			sendprivmsg(to, "%s", rest);
		}
		else {
			/*       if (	log) */
			/* 	botlog (LOGFILE, "<%s#%s	> %s", currentbot->botname, currentchannel(), rest); */
			
			/*       sen	dprivmsg(currentchannel(), "%s", rest); */
			send_to_user (from, "O	n what channel?");
		}
		
	} else
		send_to_user(from, "I don't know what to say");
	return;
}

void    do_me (char *from, char *to, char *rest) 
{
  if (rest && strlen (rest) > 1 && rest[0] == '#' && strchr (&rest[1],' ')) {
    to = get_token (&rest, " ");
  };
  
  if(rest) {
    if(to && ischannel(to)) {
      if (is_log_on (to))
	botlog (LOGFILE, "*%s#%s* %s", currentbot->botname, to, rest);
      sendaction(to, "%s", rest);
    }
    else {
      send_to_user (from, "On what channel?");
    }

  } else
    send_to_user(from, "I don't know what to do.");
  return;
}


void do_topic (char *from, char *to, char *rest) 
{
  if (rest && strlen (rest) > 1 && rest[0] == '#' && strchr (&rest[1],' ')) {
    to = get_token (&rest, " ");
  };
  
  if(rest) {
    if(to && ischannel(to)) {
      sendtopic (to, rest);
      botlog (LOGFILE, "*%s change le topic de %s en \"%s\" sur l'ordre de %s.", currentbot->botname, to, rest, getnick (from));
    }
    else {
      send_to_user (from, "Dans quel canal?");
    }

  } else
    send_to_user(from, "Quel sujet dois-je mettre?");
  return;
}

void    do_replist (char *from, char *to, char *rest)
{
  int i;
  if (TailleRep <= 0)
    send_to_user (from, "Il n'y a aucune réponse.");
  else {
    if (rest) {
      SKIPSPC (rest);

      if (rest[0]) {

	for (i=0; i < TailleRep; i++) {
	  if (!fnmatch (rest, TableDesReponses[i]->NomStimulus, FNM_CASEFOLD)) {
	    send_to_user (from, "Réponse numero %d:", i+1);
	    send_to_user (from, "%s\t\"%s\"",
			  TableDesReponses[i]->NomStimulus,
			  TableDesReponses[i]->Reponse, GetNick (from));
	    send_to_user (from, "%s\t%s", TableDesReponses[i]->Auteur,
			  TableDesReponses[i]->Canal);
	  }
	} 

      } else {
	for (i=0; i < TailleRep; i++) {
	  send_to_user (from, "Réponse numero %d:", i+1);
	  send_to_user (from, "%s\t\"%s\"",
			TableDesReponses[i]->NomStimulus,
			TableDesReponses[i]->Reponse, GetNick (from));
	  send_to_user (from, "%s\t%s", TableDesReponses[i]->Auteur,
			TableDesReponses[i]->Canal);
	}
      }

    }
    else {
      for (i=0; i<TailleRep; i++) {
	send_to_user (from, "Réponse numero %d:", i+1);
	send_to_user (from, "%s\t\"%s\"",
		      TableDesReponses[i]->NomStimulus,
		      TableDesReponses[i]->Reponse, GetNick (from));
	send_to_user (from, "%s\t%s", TableDesReponses[i]->Auteur,
		      TableDesReponses[i]->Canal);
      }
    }
  }
}

void    do_stimlist (char *from, char *to, char *rest)
{
  int i;
  if (TailleStim <= 0)
    send_to_user (from, "Il n'y a aucun stimulus.");
  else {
    if (rest) {
      SKIPSPC (rest);

      if (rest[0]) {

	for (i=0; i < TailleStim; i++) {
	  if (!fnmatch (rest, TableDesStimuli[i]->NomStimulus, FNM_CASEFOLD)) {
	    send_to_user (from, "Stimulus numéro %d:", i+1);
	    send_to_user (from, "\"%s\"\t%s",
			  TableDesStimuli[i]->Stimulus,
			  TableDesStimuli[i]->NomStimulus);
	    send_to_user (from, "%s\t%s", TableDesStimuli[i]->Auteur,
			  (TableDesStimuli[i]->Actif?"actif":"inactif"));
	  }
	} 

      } else {
	for (i=0; i < TailleStim; i++) {
	  send_to_user (from, "Stimulus numéro %d:", i+1);
	  send_to_user (from, "\"%s\"\t%s",
			TableDesStimuli[i]->Stimulus, GetNick (from));
			TableDesStimuli[i]->NomStimulus,
	  send_to_user (from, "%s\t%s", TableDesStimuli[i]->Auteur,
			TableDesStimuli[i]->Actif?"actif":"inactif");
	}
      }

    }
    else {
      for (i=0; i<TailleStim; i++) {
	send_to_user (from, "Stimulus numéro %d:", i+1);
	send_to_user (from, "\"%s\"\t%s",
		      TableDesStimuli[i]->Stimulus, GetNick (from));
		      TableDesStimuli[i]->NomStimulus,
	send_to_user (from, "%s\t%s", TableDesStimuli[i]->Auteur,
		      TableDesStimuli[i]->Actif?"actif":"inactif");
      }
    }
  }
}

void    do_repdel (char *from, char *to, char *rest) {
  if (rest) {
   int numero = atoi (rest);

   if (numero) {
     if (numero > TailleRep || numero < 0) {
       char **Phrase;
       Phrase = malloc (3 * sizeof (char *));
     
       Phrase[0] = strdup ("Il faut me donner un vrai numéro de réponse, %s.");
       Phrase[1] = strdup ("Ce n'est pas un vrai numéro de réponse ça, %s. Pour l'avoir, utiliser REPLIST");
       Phrase[2] = strdup ("Qu'est-ce que c'est que ça? Il ne me faut que le numéro de la réponse à détruire, %s, d'accord?");
       Repondre (from, to, 0, 3, Phrase, 0, 0, 0);
       return;
     }
     if (rellevel (from) >= rellevel (TableDesReponses[numero]->Auteur)) {
	    send_to_user (from, "Réponse à détruire: \"%s\".", TableDesReponses[numero]->Reponse);

       if (SupprimeRep (numero))
	 send_to_user (from, "La réponse numéro %d a été supprimée de la liste des réponses.", numero);
       else
	 send_to_user (from, "Je n'ai pas réussi à supprimer la réponse numéro %d.", numero);
     } else {
       char **Phrase;
       Phrase = malloc (3 * sizeof (char *));
     
       Phrase[0] = strdup ("Je fais plus confiance à l'auteur de la réponse qu'à toi %s. Dommage.");
       Phrase[1] = strdup ("Demande à quelqu'un en qui j'ai encore plus confiance, %s. Je ne suis pas sûr que l'auteur apprécierait qu'on détruise cette réponse.");
       Phrase[2] = strdup ("Pourquoi devrais-je détruire cette réponse, %s? Son auteur m'est encore plus sympathique que toi.");
       Repondre (from, to, 0, 3, Phrase, 0, 0, 0);
     }
   } else {
     char **Phrase;
     Phrase = malloc (3 * sizeof (char *));
     
     Phrase[0] = strdup ("Il faut me donner un numéro de réponse pour que je puisse le détruire, %s. Et rien d'autre.");
     Phrase[1] = strdup ("Ce n'est pas un numéro de réponse ça, %s. Pour l'avoir, utiliser REPLIST");
     Phrase[2] = strdup ("Qu'est-ce que c'est que ça? Il ne me faut que le numéro de la réponse à détruire, hmmm?");
     Repondre (from, to, 0, 3, Phrase, 0, 0, 0);
   }
  } else {
    char **Phrase;
    Phrase = malloc (3 * sizeof (char *));
    
    Phrase[0] = strdup ("Il faut me donner un numéro de réponse pour que je puisse la détruire, %s. (Utiliser REPLIST).");
    Phrase[1] = strdup ("Il manque le numéro de la réponse, %s. Regarde REPLIST.");
    Phrase[2] = strdup ("Et le numéro de la réponse à détruire, %s, hmmm?");
    Repondre (from, to, 0, 3, Phrase, 0, 0, 0);
  }
}

void    do_stimdel (char *from, char *to, char *rest) {
  if (rest) {
   int numero = atoi (rest);

   if (numero) {
     if (numero > TailleStim || numero < 0) {
       char **Phrase;
       Phrase = malloc (3 * sizeof (char *));
     
       Phrase[0] = strdup ("Il faut me donner un vrai numéro de stimulus, %s.");
       Phrase[1] = strdup ("Ce n'est pas un vrai numéro de stimulus ça, %s. Pour l'avoir, utilise STIMLIST");
       Phrase[2] = strdup ("Qu'est-ce que c'est que ça? Il ne me faut que le numéro du stimulus à détruire, %s, d'accord?");
       Repondre (from, to, 0, 3, Phrase, 0, 0, 0);
       return;
     }

     if (rellevel (from) >= rellevel (TableDesStimuli[numero]->Auteur)) {

	    send_to_user (from, "Stimulus à détruire: \"%s\".", TableDesStimuli[numero]->Stimulus);
	    
       if (SupprimeStim (numero))
	 send_to_user (from, "Le stimulus numéro %d a été supprimé de la liste des stimuli.", numero);
       else
	 send_to_user (from, "Je n'ai pas réussi à supprimer le stimulus numéro %d.", numero);
     } else {
       char **Phrase;
       Phrase = malloc (3 * sizeof (char *));
     
       Phrase[0] = strdup ("Je fais plus confiance à l'auteur du stimulus qu'à toi %s. Dommage.");
       Phrase[1] = strdup ("Demande à quelqu'un en qui j'ai encore plus confiance, %s. Je ne suis pas sûr que l'auteur apprécierait qu'on détruise ce stimulus.");
       Phrase[2] = strdup ("Pourquoi devrais-je détruire ce stimulus, %s? Son auteur m'est encore plus sympathique que toi.");
       Repondre (from, to, 0, 3, Phrase, 0, 0, 0);
     }
   } else {
     char **Phrase;
     Phrase = malloc (3 * sizeof (char *));
     
     Phrase[0] = strdup ("Il faut me donner un numéro de stimulus pour que je puisse le détruire, %s. Et rien d'autre.");
     Phrase[1] = strdup ("Ce n'est pas un numéro de stimulus ça, %s. Pour l'avoir, utilise STIMLIST");
     Phrase[2] = strdup ("Qu'est-ce que c'est que ça? Il ne me faut que le numéro du stimulus à détruire, hmmm?");
     Repondre (from, to, 0, 3, Phrase, 0, 0, 0);
   }
  } else {
    char **Phrase;
    Phrase = malloc (3 * sizeof (char *));
    
    Phrase[0] = strdup ("Il faut me donner un numéro de stimulus pour que je puisse le détruire, %s. (Utiliser STIMLIST).");
    Phrase[1] = strdup ("Il manque le numéro du stimulus, %s. Regarde STIMLIST.");
    Phrase[2] = strdup ("Et le numéro du stimulus à détruire, %s, hmmm?");
    Repondre (from, to, 0, 3, Phrase, 0, 0, 0);
  }
}


void	do_do(char *from, char *to, char *rest)
{
	if(rest)
            send_to_server(rest);
        else
            send_to_user(from, "What do you want me to do?");
        return;
}

void	show_channels(char *from, char *to, char *rest)
{
        show_channellist(from);
        return;
}


void	do_join(char *from, char *to, char *rest)
{
	char *chan;
	if(rest)
		chan = get_token(&rest, " ");

	if(chan)
		if (join_channel(chan, "", "", rest, TRUE))
			send_to_user (from, "I am now on %s, using %s encoding", chan, strlen(rest)?rest:"default");
	    else
			send_to_user (from, "I failed joining %s", chan);
	else
		send_to_user(from, "What channel do you want me to join?");
	return;
}

void	do_leave(char *from, char *to, char *rest)
{
	if(rest)
            	leave_channel(rest); 
        else
		if(STRCASEEQUAL(currentbot->nick, to))
			leave_channel(currentchannel());
		else
			leave_channel(to);
        return;
}
 
void	do_nick(char *from, char *to, char *rest)
{
	if(rest)
        {
		if(!isnick(rest))
		{
			send_to_user(from, "Illegal nickname %s", rest);
			return;
		}	
            	strncpy(currentbot->nick, rest, NICKLEN);
            	strncpy(currentbot->realnick, rest, NICKLEN);
            	sendnick(currentbot->nick); 
        }
        else
            	send_to_user(from, "You need to tell me what nick to use");
        return;
}

void	do_die(char *from, char *to, char *rest)
{
	if( rest != NULL )
            	signoff( from, rest );
        else
            	signoff( from, "Bye bye!" );
	if(number_of_bots == 0)
		exit(0);
}

void	do_quit(char *from, char *to, char *rest)
{
	quit_all_bots(from, "Quiting all bots.");
	/* A remettre si vous utilisez les notes */
/* 	dump_notelist(); */
	exit(0);
}

void    do_logon (char *from, char *to, char *rest) 
{
  CHAN_list* Canal;
/*   log = TRUE; */
  
  SKIPSPC (rest);

  if (rest)
    to = rest;

  if (to && ischannel (to) && (Canal = search_chan (to))) {
    Canal->log = TRUE;
    send_to_user (from, "Log on %s", to);
  } else {
    send_to_user (from, "%s: canal inconnu", to);
  }
}

void    do_logoff (char *from, char *to, char *rest) 
{
  CHAN_list* Canal;
/*   log = FALSE; */
  
  SKIPSPC (rest);

  if (rest)
    to = rest;

  if (to && ischannel (to) && (Canal = search_chan (to))) {
    Canal->log = FALSE;
    send_to_user (from, "Log off %s", to);
  }  else {
    send_to_user (from, "%s: canal inconnu", to);
  }
}

void    do_msglogon (char *from, char *to, char *rest) 
{
  log = TRUE;
  send_to_user (from, "Msg log on");
}

void    do_msglogoff (char *from, char *to, char *rest) 
{
  log = FALSE;
  send_to_user (from, "Msg log off");
}

void    do_comchar (char *from, char *to, char *rest) 
{
  if (rest) {
    SKIPSPC (rest);

    if (rest) {
      CommandChar = rest[0];
      sendreply ("Le nouveau caractère de commande est '%c'.", CommandChar);
    }
    else {
      sendreply("Quel doit être le nouveau caractère de commande?");
    }
  }
  else
    sendreply ("Le caractère de commande actuel est '%c'", CommandChar);
}

void    show_whois(char *from, char *to, char *rest)
{
	char *nuh;

	if(rest == NULL){
		send_to_user(from, "Please specify a user");
		return;
	} 

	/* utilise rest directement ou username ? */
	/*
	if((nuh=username(rest))==NULL){
		send_to_user(from, "%s is not on this channel!", rest);
		return;
	}
	*/
	send_to_user(from, "%s's levels are:", rest);
	send_to_user(from, "-- User -+- Shit -+- Protect --+- Relation -+" );
	send_to_user(from, "    %3d  |   %3d  |      %3d   |       %3d  |",
				 userlevel(rest),
				 shitlevel(rest),
				 protlevel(rest),
				 rellevel (rest));
	return;
}

void	show_nwhois(char *from, char *to, char *rest)
{
	char	*nuh;

	if( rest == NULL ){
		send_to_user( from, "Please specify a nickname" );
		return;
	}
	if((nuh=username(rest))==NULL){
		send_to_user(from, "%s is not on this channel!", rest);
		return;
	}

	send_to_user(from, "USERLIST:-------------Matching pattern(s) + level");
	show_lvllist( currentbot->lists->opperlist, from, nuh);
	send_to_user(from, "SHITLIST:---------------------------------+");
	show_lvllist( currentbot->lists->shitlist, from, nuh);
	send_to_user(from, "PROTLIST:---------------------------------+");
	show_lvllist( currentbot->lists->protlist, from, nuh);
	send_to_user(from, "RELLIST:----------------------------------+");
	show_lvllist( currentbot->lists->rellist, from, nuh);
	send_to_user(from, "End of nwhois-----------------------------+");
}

void	do_nuseradd(char *from, char *to, char *rest)
{
	char	*newuser;
	char	*newlevel;
	char	*nuh;		/* nick!@user@host */

	if((newuser = get_token(&rest, " ")) == NULL){
		send_to_user( from, "Who do you want me to add?" );
		return;
	}
	if((nuh=username(newuser))==NULL){
		send_to_user(from, "%s is not on this channel!", newuser);
		return;
	}
	if((newlevel = get_token(&rest, " ")) == NULL){
		send_to_user( from, "What level should %s have?", newuser );
		return;
	}


	if(atoi(newlevel) < 0)
		send_to_user(from, "level should be >= 0!");
	else if(atoi(newlevel) >= userlevel(from))
		send_to_user( from, "Sorry bro, can't do that!" );
	else if(userlevel(from) < userlevel(nuh))
		send_to_user(from, "Sorry, %s has a higher level", newuser);
	else
	{
		char	*nick;
		char	*user;
		char	*host;
		char	*domain;
		char	userstr[MAXLEN];

		nick=get_token(&nuh, "!");
		user=get_token(&nuh,"@");
		if(*user == '~' || *user == '#') user++;
		host=get_token(&nuh, ".");
		domain=get_token(&nuh,"");

		sprintf(userstr, "*!*%s@*%s", user, domain?domain:host);
		send_to_user( from, "User %s added with access %d as %s", 
					  newuser, atoi( newlevel ), userstr );
		add_to_levellist( currentbot->lists->opperlist, 
						  userstr, atoi( newlevel ) );
	}
	return;
}


void	do_useradd(char *from, char *to, char *rest)
{
	char	*newuser;
	char	*newlevel;

        if((newuser = get_token(&rest, " ")) == NULL)
	{
             	send_to_user( from, "Who do you want me to add?" );
		return;
	}
        if((newlevel = rest) == NULL)
	{
                send_to_user( from, "What level should %s have?", newuser );
		return;
	}
        if(atoi(newlevel) < 0)
		send_to_user(from, "level should be >= 0!");
	else if(atoi( newlevel) >= userlevel(from))
		send_to_user(from, "Sorry bro, can't do that!");
	else if(userlevel(from) < userlevel(newuser))
		send_to_user(from, "Sorry, %s has a higher level", newuser);
	else
	{
               	send_to_user( from, "User %s added with access %d", 
			      newuser, atoi(newlevel));
               	add_to_levellist(currentbot->lists->opperlist, newuser, atoi(newlevel));
	}
        return;
}                   

void    do_userwrite(char *from, char *to, char *rest)
{
        if(!write_lvllist(currentbot->lists->opperlist, 
            currentbot->lists->opperfile))
                send_to_user(from, "Userlist could not be written to file %s", 
                              currentbot->lists->opperfile);
        else
                send_to_user(from, "Userlist written to file %s", 
                              currentbot->lists->opperfile);
}

void    do_locwrite(char *from, char *to, char *rest)
{
        if(!SauveLocuteurs(currentbot->lists->ListeLocuteurs, 
			   currentbot->lists->locuteurfile))
	  send_to_user(from, "Locuteurs list could not be written to file %s", 
		       currentbot->lists->locuteurfile);
        else
	  send_to_user(from, "Loclist written to file %s", 
		       currentbot->lists->locuteurfile);
}

void	do_userdel(char *from, char *to, char *rest)
{
        if(rest)
        {
            	if (userlevel(from) < userlevel(rest))
			send_to_user(from, "%s has a higer level.. sorry",
				     rest);
            	else if (!remove_from_levellist(currentbot->lists->opperlist,
						rest))
                	send_to_user(from, "%s has no level!", rest);
	    	else
                	send_to_user(from, "User %s has been deleted", rest);
        }
	else
        	send_to_user(from, "Who do you want me to delete?");
        return;
}

void	do_nshitadd(char *from, char *to, char *rest)
{
	char	*newuser;
	char	*newlevel;
	char	*nuh;		/* nick!@user@host */

        if((newuser = get_token(&rest, " ")) == NULL)
	{
             	send_to_user(from, "Who do you want me to add?");
		return;
	}
	if((nuh=username(newuser))==NULL)
	{
		send_to_user(from, "%s is not on this channel!", newuser);
		return;
	}
        if((newlevel = get_token(&rest, " ")) == NULL )
	{
                send_to_user(from, "What level should %s have?", newuser);
		return;
	}


        if(atoi(newlevel) < 0)
		send_to_user(from, "level should be >= 0!");
	else if(atoi(newlevel) > userlevel(from))
		send_to_user(from, "Sorry bro, can't do that!");
	else if(userlevel(from) < userlevel(nuh))
		/* This way, someone with level 100 can't shit someone with level 150 */
		send_to_user(from, "Sorry, %s has a higher level", newuser);
	else
	{
		char	*nick;
		char	*user;
		char	*host;
		char	*domain;
		char	userstr[MAXLEN];

		nick=get_token(&nuh, "!");
		user=get_token(&nuh,"@");
		if(*user == '~' || *user == '#') user++;
		host=get_token(&nuh, ".");
		domain=get_token(&nuh,"");

		sprintf(userstr, "*!*%s@*%s", user, domain?domain:host);
               	send_to_user(from, "User %s shitted with access %d as %s", 
			     newuser, atoi(newlevel), userstr);
               	add_to_levellist(currentbot->lists->shitlist, userstr, atoi(newlevel));
	}
        return;
}                   

void	do_shitadd(char *from, char *to, char *rest)
{
	char	*newuser;
	char	*newlevel;

        if((newuser = get_token(&rest, " ")) == NULL)
	{
             	send_to_user(from, "Who do you want me to add?");
		return;
	}
        if((newlevel = rest) == NULL)
	{
                send_to_user(from, "What level should %s have?", newuser);
		return;
	}
        if(atoi(newlevel) < 0)
                send_to_user(from, "level should be >= 0!");
        else  
        {
                send_to_user(from, "User %s shitted with access %d", newuser, 
		             atoi(newlevel));
                add_to_levellist(currentbot->lists->shitlist, newuser, atoi(newlevel));
        } 
        return;
}                   

void	do_shitwrite(char *from, char *to, char *rest)
{
	if(!write_lvllist(currentbot->lists->shitlist, 
                          currentbot->lists->shitfile))
		send_to_user(from, "Shitlist could not be written to file %s", 
                             currentbot->lists->shitfile);
	else
		send_to_user(from, "Shitlist written to file %s", 
                             currentbot->lists->shitfile);
}

void	do_shitdel(char *from, char *to, char *rest)
{
	if(rest)
        {
            	if(!remove_from_levellist(currentbot->lists->shitlist, rest))
	      		send_to_user(from, "%s is not shitted!", rest);
            	else
                	send_to_user(from, "User %s has been deleted", rest);
        }
	else
        	send_to_user(from, "Who do you want me to delete?");
        return;
}

void	do_nprotadd(char *from, char *to, char *rest)
{
	char	*newuser;
	char	*newlevel;
	char	*nuh;		/* nick!@user@host */

        if((newuser = get_token(&rest, " ")) == NULL)
	{
             	send_to_user(from, "Who do you want me to add?");
		return;
	}
	if((nuh=username(newuser))==NULL)
	{
		send_to_user(from, "%s is not on this channel!", newuser);
		return;
	}
        if((newlevel = get_token(&rest, " ")) == NULL)
	{
                send_to_user(from, "What level should %s have?", newuser);
		return;
	}


        if(atoi(newlevel) < 0)
		send_to_user(from, "level should be >= 0!");
	else if(atoi(newlevel) > 100)
		send_to_user(from, "yeah, right! You're shitted.");
	else
	{
		char	*nick;
		char	*user;
		char	*host;
		char	*domain;
		char	userstr[MAXLEN];

		nick=get_token(&nuh, "!");
		user=get_token(&nuh,"@");
		if(*user == '~' || *user == '#') user++;
		host=get_token(&nuh, ".");
		domain=get_token(&nuh,"");

		sprintf(userstr, "*!*%s@*%s", user, domain?domain:host);
               	send_to_user(from, "User %s protected with access %d as %s", 
			     newuser, atoi(newlevel), userstr);
               	add_to_levellist(currentbot->lists->protlist, userstr, atoi(newlevel));
	}
        return;
}                   

void    do_protadd(char *from, char *to, char *rest)
{
        char    *newuser;
        char    *newlevel;

        if((newuser = get_token(&rest, " ")) == NULL)
	{
             	send_to_user(from, "Who do you want me to add?");
		return;
	}
        if((newlevel = rest) == NULL )
	{
                send_to_user(from, "What level should %s have?", newuser);
		return;
	}
        if(atoi(newlevel) < 0)
                send_to_user(from, "level should be >= 0!");
        else  
        {
                send_to_user(from, "User %s protected with access %d", newuser,
                             atoi(newlevel));
                add_to_levellist(currentbot->lists->protlist, newuser, atoi(newlevel));
        }
        return;
}

void    do_protwrite(char *from, char *to, char *rest)
{
        if(!write_lvllist(currentbot->lists->protlist, 
                          currentbot->lists->protfile))
                send_to_user(from, "Protlist could not be written to file %s", 
                             currentbot->lists->protfile);
        else
                send_to_user(from, "Protlist written to file %s", 
                             currentbot->lists->protfile);
}

void    do_protdel(char *from, char *to, char *rest)
{
        if(rest)
        {
            	if(!remove_from_levellist(currentbot->lists->protlist, rest))
           		send_to_user(from, "%s is not protected!", rest);
            	else
                	send_to_user(from, "User %s has been deleted", rest);
        }
	else
        	send_to_user(from, "Who do you want me to delete?");
        return;
}

void    do_nreladd (char *from, char *to, char *rest)
{
  char *newrel;
  char *newlevel;
  char *nuh;		/* nick!@user@host */

  if((newrel = get_token(&rest, " ")) == NULL)
    {
      send_to_user( from, "Who do you want me to add or modify?" );
      return;
    }
  if((nuh=username(newrel))==NULL)
    {
      send_to_user(from, "%s is not on this channel!", newrel);
      return;
    }
  if ((newlevel = get_token(&rest, " ")) == NULL)
    {
      send_to_user( from, "What level should %s have?", newrel );
      return;
    }

  {
    char	*nick;
    char	*user;
    char	*host;
    char	*domain;
    char	*userstr;
    char i;
    int numbers = FALSE;
    int inewlevel;
    int ioldlevel;
    /*
    nick=get_token(&nuh, "!");
    user=get_token(&nuh,"@");
    if(*user == '~' || *user == '#') user++;
    host=get_token(&nuh, ".");
    domain=get_token(&nuh,"");

    for (i='0'; i<='9' && !numbers; i++)
      if (strchr (domain, i))
	numbers = TRUE;

    for (i='a'; i<='z' && numbers; i++)
      if (strchr (domain, i))
	numbers = FALSE;
    
    for (i='A'; i<= 'Z' && numbers; i++)
      if (strchr (domain, i))
	numbers = FALSE;

    if (!numbers)
      sprintf(userstr, "%s!*%s@*%s", nick, user, domain?domain:host);
    else {
      domain = get_token (&domain, ".");
      sprintf(userstr, "%s!*%s@%s.%s*", nick, user, host, domain?domain:"");
    }
    */

    userstr = NickUserStr (from);

#ifdef DBUG
      fprintf (stderr, "userstr:'%s', nuh:'%s', from:'%s', newrel:'%s'\n", userstr, nuh, from, newrel);
#endif DBUG

    inewlevel = atoi (newlevel);
    ioldlevel = rellevel (rest);
    /* Si le demandeur est maitre du bot */
    if (userlevel (from) > 125) {
#ifdef DBUG
      fprintf (stderr, "Demandeur maître du bot\n");
#endif DBUG
      /* Si l'utilisateur n'existe pas encore */
      if (!exist_userhost (currentbot->lists->rellist, newrel)) {
	send_to_user( from, "Rel %s added with access %d as %s", 
		      newrel, inewlevel, nuh );
	add_to_levellist( currentbot->lists->rellist, 
			  nuh, inewlevel );
      }
      else {
	send_to_user( from, "Rel %s access changed to %d", 
		      newrel, inewlevel );
	add_to_levellist( currentbot->lists->rellist, nuh, inewlevel );
      }
    }
    /* Si le demandeur n'est pas maitre du bot */
    else {
#ifdef DBUG
      fprintf (stderr, "Demandeur NON maître du bot: %s\n", from);
#endif DBUG
      /* Si le niveau demande est superieur au niveau actuel */
      if (inewlevel >= ioldlevel) {
#ifdef DBUG
      fprintf (stderr, "Niveau demandé: %d, Niveau actuel: %d\n", inewlevel, ioldlevel);
#endif DBUG
	/* Si le demandeur est la cible */
	if (!fnmatch ( nuh, from, FNM_CASEFOLD )) {
#ifdef DBUG
      fprintf (stderr, "Cible: %s = Demandeur: %s\n", nuh, from);
#endif DBUG
	  /* - */
	  /* Si l'utilisateur n'existe pas encore */
	  if (!exist_userhost (currentbot->lists->rellist, newrel)) {
	    send_to_user( from, "Rel %s added with access %d as %s", 
			  from, ioldlevel - (inewlevel - ioldlevel), userstr );
	    add_to_levellist( currentbot->lists->rellist, 
			      NickUserStr (from),
			      ioldlevel - ( inewlevel - ioldlevel) );
	  }
	  else {
	    send_to_user( from, "Rel %s access changed to %d", 
			  from, ioldlevel - (inewlevel-ioldlevel) );
	    add_to_levellist( currentbot->lists->rellist, NickUserStr (from),
			      ioldlevel-(inewlevel-ioldlevel) );
	  }
	}
	/* Si le demandeur n'est pas la cible */
	else {
#ifdef DBUG
      fprintf (stderr, "Cible: %s != Demandeur: %s\n", userstr, from);
#endif DBUG
	  /* Si le demandeur est sympathique */
	  if (rellevel (from) >= SYMPA_LVL) {
	    /* Si la cible n'est pas antipathique */
	    if (ioldlevel >= 0) {
	      /* +1 */
	      /* Si l'utilisateur n'existe pas encore */
	      if (!exist_userhost (currentbot->lists->rellist, newrel)) {
		send_to_user( from, "Rel %s added with access %d as %s", 
			      newrel, ioldlevel+1, userstr );
		add_to_levellist( currentbot->lists->rellist, 
				  userstr, ioldlevel+1 );
	      }
	      else {
		send_to_user( from, "Rel %s access changed to %d", 
			      newrel, ioldlevel+1 );
		add_to_levellist( currentbot->lists->rellist, newrel, ioldlevel+1 );
	      }
	    }
	  }
	  /* Si le demandeur n'est pas sympathique */
	  else {
	    /* Demandeur -, cible -1 */
	    
	  }
	}
      }
      /* Si le niveau demande est inferieur au niveau actuel */
      else {
	/* Si le demandeur est la cible */
	if (!fnmatch ( userstr, from, FNM_CASEFOLD )) {
	  /* OK */
	}
	/* Si le demandeur n'est pas la cible */
	else {
	  /* Si le demandeur est sympathique */
	  if (rellevel (from) >= SYMPA_LVL) {
	    /* Si la cible est plus sympathique */
	    if (rellevel (from) <= ioldlevel) {
	      /* non */
	    }
	    /* Si le demandeur a un niveau superieur a celui de sa cible */
	    else {
	      /* OK */
	    }
	  }
	  /* Si le demandeur n'est pas sympathique */
	  else {
	    if (ioldlevel >= 0) {
	      /* Demandeur -, kick (demandeur) */
	    }
	    else {
	      /* Demandeur - */
	    }
	  }
	}
      }
    }

    if (userstr)
      free (userstr);
  }
  return;
}

void    do_reladd (char *from, char *to, char *rest) 
{
  char *newrel;
  char *newlevel;

  if((newrel = get_token(&rest, " ")) == NULL)
    {
      send_to_user( from, "Who do you want me to add?" );
      return;
    }
  if ((newlevel = rest) == NULL)
    {
      send_to_user( from, "What level should %s have?", newrel );
      return;
    }
  if(atoi( newlevel) > rellevel(from))
    send_to_user(from, "Sorry bro, can't do that!");
  else if(rellevel(from) < rellevel(newrel))
    send_to_user(from, "Sorry, %s has a higher level", newrel);
  else if (atoi (newlevel)-rellevel (newrel) < -SYMPA_LVL)
    send_to_user(from, "C'est pas gentil ça, %s!", newrel);
  else
    {
      send_to_user( from, "Rel %s added with access %d", 
		    newrel, atoi(newlevel));
      add_to_levellist(currentbot->lists->rellist, newrel, atoi(newlevel));
    }
  return;
}

void    do_reldel (char *from, char *to, char *rest)
{
  char **RepPos, **RepNeg;
  
  if (rest) {
    if (rellevel (from) < rellevel (rest)) {
      RepPos = malloc (sizeof(char *) * 3);

      RepPos[0] = strdup ("Désolé, ton niveau est inférieur.");
      RepPos[1] = strdup ("Il faut avoir un niveau supérieur à celui qu'on veut détruire, %s.");
      RepPos[2] = strdup ("C'est impossible, je t'aime moins que lui, %s.");

      RepNeg = malloc (1 * sizeof (char *));

      RepNeg[0] = strdup ("Non. Je te l'inderdis, %s.");

      Repondre (from, to, -1, 3, RepPos, -2, 1, RepNeg);
    }
    else if (!remove_from_levellist (currentbot->lists->rellist, rest)) {
      char Phrase[MAXLEN];
      
      RepPos = malloc (3 * sizeof (char *));

      sprintf (Phrase, "%s n'est pas dans la liste de mes connaissances.", rest);
      RepPos[0] = strdup (Phrase);
      sprintf (Phrase, "Je ne connais pas \"%s\".", rest);
      RepPos[1] = strdup (Phrase);
      RepPos[2] = strdup ("Impossible, %s, je ne l'ai pas dans ma liste!");

      RepNeg = malloc (1 * sizeof (char *));
      RepNeg[0] = strdup ("Même si je le pouvais, %s, je ne le ferais pas. Mais c'est impossible: je n'ai pas trouvé ce nom dans ma liste.");

      Repondre (from, to, 0, 3, RepPos, -1, 1, RepNeg);
    }
    else {
      char Phrase[MAXLEN];

      RepPos = malloc (3 * sizeof (char *));
      sprintf (Phrase, "%s a été détruit de ma liste de relation.", rest);
      RepPos[0] = strdup (Phrase);
      sprintf (Phrase, "J'ai effacé \"%s\" de mes connaissances.", rest);
      RepPos[1] = strdup (Phrase);
      sprintf (Phrase, "J'oublie que j'ai déjà parlé à \"%s\"", rest);
      RepPos[2] = strdup (Phrase);

      RepNeg = malloc (1 * sizeof (char *));
      RepNeg[0] = strdup ("Par quel sortilège es-tu parvenu à me faire faire ça, %s?");

      Repondre (from, to, 0, 3, RepPos, -1, 1, RepNeg);
    }
  }
  else {
    RepPos = malloc (3 * sizeof (char *));
    RepPos[0] = strdup ("C'est bien gentil, %s, mais j'aimerais savoir qui je dois oublier.");
    RepPos[1] = strdup ("Étourdi, va, %s: il faut me donner un nom complet à oublier!");
    RepPos[2] = strdup ("C'est bien mon petit %s, ça: il oublie de me dire qui il faut oublier!");

    RepNeg = malloc (1 * sizeof (char *));
    RepNeg[0] = strdup ("Ça m'étonne pas de toi, %s: tu ne sais pas qu'il faut dire qui je dois oublier?");
    
    Repondre (from, to, 0, 3, RepPos, 0, 1, RepNeg);
  }
  return;
}


void    do_relwrite (char *from, char *to, char *rest)
{
  if (!write_lvllist (currentbot->lists->rellist,
		      currentbot->lists->relfile))
    send_to_user(from, "Rellist could not be written to file %s", 
		 currentbot->lists->relfile);
  else
    send_to_user(from, "Rellist written to file %s", 
                             currentbot->lists->relfile);
}

void	do_banuser(char *from, char *to, char *rest)
{
	char	*user_2b_banned;
	char	*channel;

	if(ischannel(to))
		channel = to;
	else
		channel = currentchannel();
		
	if(rest)
		if((user_2b_banned = username(rest)))
			ban_user(user_2b_banned, channel);
		else
			send_to_user(from, "%s not on this channel", rest);
	else
		send_to_user( from, "No." );
}

void	do_showusers(char *from, char *to, char *rest)
{
        char    *channel;

	if( (channel = strtok( rest, " " )) == NULL )
		show_users_on_channel( from, currentchannel());
	else
		show_users_on_channel( from, channel );
}

void    do_showbots (char *from, char *to, char *rest) 
{
  char *channel;
  
  if ((channel = strtok (rest, " ")) == NULL)
    show_botlist (currentbot->botlist, to, from, to);
  else
    show_botlist (currentbot->botlist, channel, from, to);
}

void    do_botadd (char *from, char *to, char *rest) 
{
  char *newbot;
  char *newchannel;
  
  if ((newbot = get_token (&rest, " ")) == NULL) {
    send_to_user (from, "Quel bot faut-il ajouter ?");
    return;
  }

  if ((newchannel = rest) == NULL) {
    send_to_user (from, "Dans quel(s) canal(-aux) faut il mettre %s?", newbot);
    return;
  }
  
  add_to_botlist (currentbot->botlist, newbot, newchannel);
  send_to_user (from, "Voilà qui est fait : %s est ajouté dans %s.", newbot, newchannel);
}

void    do_showlocs (char *from, char *to, char *rest)
{
  MontreLocuteurs (currentbot->lists->ListeLocuteurs, from, rest);
}

void	do_massop(char *from, char *to, char *rest)
{
	char	*op_pattern;

        if( ( op_pattern = strtok( rest, " " ) ) == NULL )
             send_to_user( from, "Please specify a pattern" );
        else
            channel_massop( ischannel( to ) ? to : currentchannel(), 
			    op_pattern );    
        return;
}

void    do_massdeop(char *from, char *to, char *rest)
{
        char    *op_pattern;
 
        if( ( op_pattern = strtok( rest, " " ) ) == NULL )
            send_to_user( from, "Please specify a pattern" );
        else
            channel_massdeop( ischannel( to ) ? to : currentchannel(), 
			      op_pattern );
        return;
}

void	do_masskick(char *from, char *to, char *rest)
{
        char    *op_pattern;

        if( ( op_pattern = strtok( rest, " " ) ) == NULL )
            send_to_user( from, "Please specify a pattern" );
        else
            channel_masskick( ischannel( to ) ? to : currentchannel(), 
			      op_pattern );
        return;
}

void	do_massunban(char *from, char *to, char *rest)
{
	char	*channel;

	if((channel = strtok(rest, " "))==NULL)
		channel_massunban(currentchannel());
	else
		channel_massunban(channel);
}
	
void	do_server(char *from, char *to, char *rest)
{
	int	serv;

	if(rest)
		if(readint(&rest, &serv))
			if(not(change_server(serv)))
				sendreply("Incorrent servernumber!");
			else
				sendreply("Hold on, I'm trying..");
		else
			sendreply("Illegal servernumber!");
	else
		sendreply("Change to which server?");
}

void	show_dir(char *from, char *to, char *rest)
{
	char	*pattern;

	if(rest)
		while((pattern = get_token(&rest, " ")))
		{
			if(*rest != '-')
				do_ls(from, pattern);
		}
	else
		do_ls(from, "");
}

void	show_cwd(char *from, char *to, char *rest)
{
	pwd(from);
}

void	do_cd(char *from, char *to, char *rest)
{
	do_chdir(from, rest?rest:"/");
}

void    show_queue( char *from, char *rest, char *to )
{
	do_showqueue();
}

void	do_fork(char *from, char *to, char *rest)
{
	char	*nick;
	char	*login;

	if(rest && (nick=get_token(&rest, " ")))
	{
		if(!isnick(nick))
		{
			send_to_user(from, "Illegal nick %s", nick);
			return;
		}
		login= get_token(&rest, " ");
		rest = get_token(&rest, "");
		if(!forkbot(nick, login, rest))
			send_to_user(from, "Sorry, no more bots free");
	}	
	else
		send_to_user(from, "Pls specify nick");
}

void	do_unban(char *from, char *to, char *rest)
{
	        char    *channel;

        if((channel = strtok(rest, " "))==NULL)
                channel_unban(currentchannel(), from);
	else
		channel_unban(channel, from);
}

void	do_kick(char *from, char *to, char *rest)
{
	char	*nuh;
	char    *Phrase;
	char    *nom;
	char    *Rest, *ALiberer;
	
	if(!rest)
	    return;

	if(strlen(rest)==0)
	    return;

	Phrase = malloc (MAXLEN*sizeof (char));

	Rest = strdup (rest);
	ALiberer = Rest;
	nom = get_token (&Rest, " ");

	if(nom && ((nuh = username(nom)) != NULL))
	  if((protlevel(nuh)>=50)&&
	     (shitlevel(nuh)==0)) {
	    send_to_user(from, "%s est protégé!", GetNick(nuh));
	  } else if(ischannel(to)) {
	    sprintf (Phrase, "%s a demandé ce kick.", GetNick (from));
	    sendkick(to, rest, Phrase);
	  } else {
	    sprintf (Phrase, "%s a demandé ce kick", GetNick (from));
	    sendkick(currentchannel(), rest, Phrase);
	  }
	else
	  send_to_user(from, "Qui!?!");

	free (Phrase);
	free (ALiberer);
}

void	do_listdcc(char *from, char *to, char *rest)
{
	show_dcclist( from );
}

void	do_rehash(char *from, char *to, char *rest)
{
	rehash = TRUE;
}

void 	giveop(char *channel, char *nicks )
{
    	sendmode(channel, "+ooo %s", nicks);
}

int 	userlevel(char *from)
{
	if(from)
		return(get_level(currentbot->lists->opperlist, from));
	else
		return 0;
}

int 	shitlevel(char *from)
{
	if(from)
		return(get_level(currentbot->lists->shitlist, from));
	else
		return 0;
}

int     protlevel(char *from)
{
	if(from)
		return(get_level(currentbot->lists->protlist, from));
	else
		return 0;
}

int     rellevel (char *from)
{
  int intermediaire;
  int succes = FALSE;
  if (from) {
    intermediaire = get_level_neg (currentbot->lists->rellist, from, &succes);
    if (!succes)
      return DEFAUT_LVL; /* Valeur par defaut */
    else return (intermediaire);
  } else
    return 0;
}

void	ban_user(char *who, char *channel)
/*
 * Ban the user as nick and as user (two bans)
 */
{
	char	buf[MAXLEN];	/* make a copy, save original */
	char	*usr = buf;
	char	*nick;
	char	*user;

	strcpy(buf, who);
	nick = get_token(&usr, "!");
	user = get_token(&usr, "@");

	if(*user == '#' || *user == '~')
		user++;
	if(not(*user) || (*user == '*'))/* faking login			*/
	{			/* we can't ban this user on his login,	*/
		             	/* and banning the nick isn't 'nuff, so	*/
				/* ban the entire site!			*/
		if(not(strchr(usr, '.')))
			sendmode(channel, "+bb %s!*@* *!*@*%s", nick, usr);
		else
			sendmode(channel, "+bb %s!*@* *!*@*%s", 
				 strchr(usr, '.'));
		return;
	}
	sendmode(channel, "+bb %s!*@* *!*%s@*", nick, user);
}

void	signoff(char *from, char *reason)
{
	char	*fromcpy;

	mstrcpy(&fromcpy, from);	/* something hoses, dunno what */
	if(number_of_bots == 1)
	{
		send_to_user(fromcpy, "No bots left... Terminating");
		if (!SauveStimuli (currentbot->stimfile))
		    send_to_user (fromcpy, "Could not save stimuli.");
		if (!SauveReponses (currentbot->repfile))
		    send_to_user (fromcpy, "Could not save responses.");
		LibereReponses ();
/* 		A remettre si vous utilisez les notes */
/* 		send_to_user(fromcpy, "Dumping notes"); */
/* 		dump_notelist(); */
	}
	send_to_user(fromcpy, "Saving lists...");
       	if(!write_lvllist(currentbot->lists->opperlist, 
           currentbot->lists->opperfile))
		send_to_user(fromcpy, "Could not save opperlist");
       	if(!write_lvllist(currentbot->lists->shitlist, 
	   currentbot->lists->shitfile))
		send_to_user(fromcpy, "Could not save shitlist");
       	if(!write_lvllist(currentbot->lists->protlist, 
           currentbot->lists->protfile))
		send_to_user( fromcpy, "Could not save protlist");
	if(!write_lvllist(currentbot->lists->rellist, 
           currentbot->lists->relfile))
		send_to_user( fromcpy, "Could not save rellist");
	if (!SauveLocuteurs (currentbot->lists->ListeLocuteurs,
			     currentbot->lists->locuteurfile))
	  send_to_user (fromcpy, "Could not save loclist");
	if (!write_botlist (currentbot->botlist, currentbot->botfile))
	  send_to_user( fromcpy, "Could not save botlist");
	delete_botlist (currentbot->botlist);

	send_to_user(fromcpy, "Bye...");
	free(fromcpy);
	killbot(reason);
}

void	do_apprends(char *from, char *to, char *rest) {
  static int DEJAPASSE = 0;

  char chaine[MAXLEN];
  char repinter[MAXLEN];
  char *Stimulus = 0;
  char *NomStimulus = 0;
  char *Reponse = 0;
  char *Canal = 0;
  char *rest_init = 0;
  char *pointeur_init = 0;
  char *inter;

  int i;

  if (rest) 
    rest_init = strdup (rest);

  pointeur_init = rest_init;
  
  /* C'est un stimulus */
  if (rest && rest[0] == '\"') {
    if (strchr (&rest[1], '\"')) {
      
      Stimulus = get_token (&rest, "\"");
      if (rest) {
	
	NomStimulus = get_token (&rest, " ");

	if (NomStimulus) {
	  
	  sprintf(chaine, "Stimulus: \"%s\", Nom: %s", Stimulus, NomStimulus);
	  send_to_user (from, chaine);
	}
	else send_to_user (from, "Il manque le nom du stimulus correspondant");
	
      }
      else send_to_user (from, "Manque le stimulus à apprendre");
    }
    else send_to_user (from, "Il manque un guillemet");
    
  }
  else {
    if (rest && strchr (&rest[1], '\"')){
      
      NomStimulus = get_token (&rest_init, " ");
      if (rest_init) {
	Reponse = get_token (&rest_init, "\"");

	if (Reponse) {
	  
	  if (rest_init && strchr (rest_init, '#')) {
	    Canal = strchr (rest_init, '#');
	    inter = strchr (Canal, '\n');
	    if (inter)
	      *inter = '\0';
	  } else
	    Canal = to;
	  
	  sprintf (repinter, Reponse, getnick (from), getnick (from), getnick (from), getnick (from), getnick (from));
	  sprintf(chaine, "Nom: %s, Réponse: \"%s\"",
		  NomStimulus, repinter, getnick (from));
	  send_to_user (from, chaine);
	  send_to_user (from, "Canal: %s", Canal);
	}
	else send_to_user (from, "Il n'y a même pas de Réponse.");
	
      }
      else send_to_user (from, "Manque le nom stimulus à apprendre.");
    }
    else if (rest) send_to_user (from, "Il manque l'espace séparant le nom du stimulus de la réponse.");
  }

  if (NomStimulus && Stimulus) {
    if (strlen(Stimulus) > 3) {
      if(AjouteStimulus (from, to, Stimulus, NomStimulus))
	send_to_user (from, "Stimulus %s appris.", NomStimulus);
    }
    else send_to_user (from, "Stimulus trop court!");
  }
  
  
  if (NomStimulus && Reponse) {
    if(AjouteReponse (from, (Canal?Canal:to), Reponse, NomStimulus))
      send_to_user (from, "Réponse apprise.");
  }
  
  if (pointeur_init)
    free (pointeur_init);

}

void    do_oublie (char *from, char *to, char *rest) 
{
  int i;
  
  if (!rest || !strlen (rest)) {
    for (i=0; i <TailleStim; i++) {
      
      free (TableDesStimuli[i]->NomStimulus);
      TableDesStimuli[i]->NomStimulus = 0;
      
      free (TableDesStimuli[i]->Stimulus);
      TableDesStimuli[i]->Stimulus = 0;
      
      free (TableDesStimuli[i]->Auteur);
      TableDesStimuli[i]->Auteur = 0;
    }
    
    for (i=0; i<TailleRep; i++) {
      free (TableDesReponses[i]->Reponse);
      TableDesReponses[i]->Reponse = 0;
      
      free (TableDesReponses[i]->NomStimulus);
      TableDesReponses[i]->NomStimulus = 0;
      
      free (TableDesReponses[i]->Auteur);
      TableDesReponses[i]->Auteur = 0;

      free (TableDesReponses[i]->Canal);
      TableDesReponses[i]->Canal = 0;
    }
    
    if (TailleStim) {
      free (TableDesStimuli);
      TailleStim = 0;
      TableDesStimuli = 0;
    }
    
    if (TailleRep) {
      free (TableDesReponses);
      TailleRep = 0;
      TableDesReponses = 0;
    }
    
    send_to_user (from, "Tous les stimuli et réponses appris sont oubliés!");
  }
  else {
    send_to_user (from, "Attention: !oublie me fait oublier tous les stimuli et les réponses que je connais! Mieux vaut utiliser !desactive.");
  }
  
}

void    do_desactive (char *from, char *to, char *rest) 
{
	int i;
	int desactivation = FALSE;

	if (rest) {
		
		SKIPSPC (rest);
    
		for (i = 0; i < TailleStim; i++) {
			if (strcmp(TableDesStimuli[i]->NomStimulus, rest)== 0 ||
				strcmp (TableDesStimuli[i]->Auteur, rest) == 0)
				if (rellevel (from)+SYMPA_LVL >= rellevel (TableDesStimuli[i]->Auteur)) {
					send_to_user (from, "Stimulus numéro %d désactivé.", i);
					TableDesStimuli[i]->Actif = FALSE;
					desactivation = TRUE;
				}
				else
					send_to_user (from,
								  "Niveau de relation insuffisant, il faudrait %d, qui est le niveau de %s.",
								  rellevel (TableDesStimuli[i]->Auteur),
								  getnick(TableDesStimuli[i]->Auteur));
		}
	} else
		send_to_user (from, "Je veux bien, moi, mais désactiver quel stimulus?");
  
	if (!desactivation)
		send_to_user (from, "Je n'ai désactivé aucun stimulus.");
  }

void    do_active (char *from, char *to, char *rest) 
{
    int i;
    
    if (rest) {
	   
	   SKIPSPC (rest);
	   
	   for (i = 0; i < TailleStim; i++) {
		   if (strcmp(TableDesStimuli[i]->NomStimulus,rest)==0 ||
			   strcmp (TableDesStimuli[i]->Auteur, rest) == 0)
			   if (rellevel (from) >= rellevel (TableDesStimuli[i]->Auteur)) {
				   send_to_user (from, "Stimulus numéro %d réactivé.", i);
				   TableDesStimuli[i]->Actif = TRUE;
			   } else
				   send_to_user (from,
								 "Niveau de relation insuffisant, il faudrait %d.",
								 rellevel (TableDesStimuli[i]->Auteur));
	   }
    } else
		send_to_user (from, "Je veux bien, moi, mais activer quel stimulus?");
}

void    do_stimwrite (char *from, char *to, char *rest) 
{
  if (rest) {
    SKIPSPC (rest);

    if(SauveStimuli (rest))
      send_to_user (from, "%d stimuli sauvegardés", TailleStim);
    else
      send_to_user (from, "Impossible de sauvegarder les stimuli!");
  }
  else {
    if(SauveStimuli (currentbot->stimfile))
      send_to_user (from, "%d stimuli sauvegardés dans %s.",
		    TailleStim,
		    currentbot->stimfile);
  }
}

void    do_stimload (char *from, char *to, char *rest) 
{
  if (rest) {
    SKIPSPC (rest);
    
    if(ChargeStimuli (rest))
      send_to_user (from, "%d stimuli chargés.", TailleStim);
    else
      send_to_user (from, "Impossible de charger les stimuli!");
  }
  else {
    if(ChargeStimuli (currentbot->stimfile))
      send_to_user (from, "%d stimuli de %s chargés.",
		    TailleStim,
		    (currentbot->stimfile));
  }
    
}


void do_repload (char *from, char *to, char *rest) 
{
  if (rest) {
    SKIPSPC (rest);
    
    if(ChargeReponses (rest))
      send_to_user (from, "%d réponses chargées", TailleRep);
    else
      send_to_user (from, "Impossible de charger les réponses!");
  }
  else {
    if(ChargeReponses (currentbot->repfile))
      send_to_user (from, "%d réponses chargées dans %s.",
		    TailleRep,
		    currentbot->repfile);
  }
}


void    do_repwrite (char *from, char *to, char *rest) {
  int i;
  FILE *fichier;
  
  if (rest) {
    SKIPSPC (rest);
    if (SauveReponses (rest))
      send_to_user (from, "%d réponses sauvegardées", TailleRep);
    else
      send_to_user (from, "Impossible de créer le fichier!");
  }
  else 
     if(SauveReponses (currentbot->repfile))
      send_to_user (from, "%d réponses de %s sauvegardées.",
		    TailleRep,
		    (currentbot->repfile));
}

void do_botload (char *from, char *to, char *rest) 
{
  if (rest) {
    SKIPSPC (rest);
    
    if(readbotdatabase (rest, currentbot->botlist))
      send_to_user (from, "bots chargés depuis %s", rest);
    else
      send_to_user (from, "Impossible de charger les bots depuis %s!", rest);
  }
  else {
    if(readbotdatabase (currentbot->botfile, currentbot->botlist))
      send_to_user (from, "bots chargés depuis %s.",
		    currentbot->botfile);
  }
}

void do_botwrite (char *from, char *to, char *rest) {
  if (rest) {
    SKIPSPC (rest);
    
    if (write_botlist (currentbot->botlist, rest))
      send_to_user (from, "bots sauvegardés dans %s.", rest);
    else
      send_to_user (from, "Impossible de sauvegarder les bots dans %s!", rest);
  } else {
    if (write_botlist (currentbot->botlist, currentbot->botfile))
      send_to_user (from, "bots sauvegardés dans %s.", currentbot->botfile);
  }
}

void do_botdel (char *from, char *to, char *rest) {
  if (rest) {
   int numero = atoi (rest);

   if (numero) {
     if (remove_bot (currentbot->botlist, numero))
       send_to_user (from, "Le bot numéro %d a été supprimé de la liste des bots.", numero);
     else
       send_to_user (from, "Je n'ai pas réussi à supprimer le bot numéro %d.", numero);
   } else {
     char **Phrase;
    Phrase = malloc (3 * sizeof (char *));
    
    Phrase[0] = strdup ("Il faut me donner un numéro de bot pour que je puisse le détruire, %s. Et rien d'autre.");
    Phrase[1] = strdup ("Ce n'est pas un numéro de bot, %s.");
    Phrase[2] = strdup ("Qu'est-ce que c'est que ça? Il ne me faut que le numéro du bot à détruire, hmmm?");
    Repondre (from, to, 0, 3, Phrase, 0, 0, 0);
   }
  } else {
    char **Phrase;
    Phrase = malloc (3 * sizeof (char *));
    
    Phrase[0] = strdup ("Il faut me donner un numéro de bot pour que je puisse le détruire, %s.");
    Phrase[1] = strdup ("Il manque le numéro de bot, %s.");
    Phrase[2] = strdup ("Et le numéro du bot à détruire, hmmm?");
    Repondre (from, to, 0, 3, Phrase, 0, 0, 0);
  }
}

void    do_fuck (char *from, char *to, char *who) {
  char **Phrase;
  char ch[MAXLEN];

  if (who) {
    
    Phrase = malloc (3 * sizeof (char *));
    sprintf (ch, "%s: %s voudrait que je t'encule! Il est mal élevé, hein?", who, GetNick (from));
    Phrase[0] = strdup (ch);
    sprintf (ch, "Pourquoi tu veux que je sodomise %s, c'est pas gentil %s!", who, GetNick (from));
    Phrase[1] = strdup (ch);
    Phrase[2] = strdup ("Je réprouve la sodomie, %s.");
    Repondre (from, to, -1, 3, Phrase, 0, 0, 0);
  }
  else {
    Phrase =  malloc (3 * sizeof (char *));

    Phrase[0] = strdup ("Qui veux-tu que je sodomise, %s?");
    Phrase[1] = strdup ("Ça te prends souvent de donner des commandes sans argument, %s?");
    Phrase[2] = strdup ("Qui, %s?");

    Repondre (from, to, -1, 3, Phrase, 0, 0, 0);
  }
  
}

void  do_ident (char *from, char *canal, char *pass) {
  FILE *fp;
  char usrhost[MAXLEN];
  char password[MAXLEN];
  char s[MAXLEN];
  char *p;
  int found = FALSE;
  locuteur *Locuteur = 0;
  char *NUS = NickUserStr (from);

  if ((fp = fopen ("pass", "r")) == NULL) {
    printf ("Fichier \"pass\" non trouve, j'arrete.\n");
    exit (0);
  }


  while ((p=fgets (s, MAXLEN, fp)) && !found ) {
    sscanf (s, "%s %s", usrhost, password);
    if (!fnmatch(usrhost, from, FNM_CASEFOLD)) 
      found = TRUE;
  }

  if (found) {
    Locuteur = LocuteurExiste (currentbot->lists->ListeLocuteurs, from);
    if (!Locuteur)
      Locuteur = AjouteLocuteur (currentbot->lists->ListeLocuteurs,
				 NUS);

  }

  fclose (fp);
  if (NUS)
    free (NUS);

}

void do_seen (char *from, char *to, char *rest) {
	char **Phrase;
	char *nuh;
	locuteur *Locuteur;
	char chaine[MAXLEN];

	if (rest){
		if(nuh = username(rest)){
			if(ischannel(to)){
				Phrase = malloc (3 * sizeof (char *));
				sprintf(chaine, "Voyons, %s est là %%s", rest);
				Phrase[0] = strdup (chaine);
				sprintf(chaine, "Si tu regardes bien tu verras que %s est là %%s", rest);
				Phrase[1] = strdup (chaine);
				sprintf(chaine, "%%s: %s est là il me semble.", rest);
				Phrase[2] = strdup (chaine);
				Repondre (from, to, 0, 3, Phrase, 0, 0, 0);
			}
			else{
				send_to_user(from, "Voyons, %s est là!",rest);
			}
			return;
		}		
		Locuteur = LocuteurNickExiste(currentbot->lists->ListeLocuteurs, rest);
		if(Locuteur){
#ifdef DBUG
			printf("trouvé un locuteur %s, lastseen = %ld, now = %ld diff = %ld\n", rest, Locuteur->LastSeen, time(NULL), time(NULL) - Locuteur->LastSeen);
#endif			
			if(ischannel(to)){
				Phrase = malloc (1 * sizeof (char *));
				sprintf(chaine, "%%s: j'ai vu %s il y a %s", rest, idle2str(time(NULL) - Locuteur->LastSeen));
				Phrase[0] = strdup (chaine);
				Repondre (from, to, 0, 1, Phrase, 0, 0, 0);
			}
			else{
				send_to_user(from, "j'ai vu %s il y a %s", rest, idle2str(time(NULL) - Locuteur->LastSeen));
			}
		}
		else{
			if(ischannel(to)){
				Phrase = malloc (2 * sizeof (char *));
				sprintf(chaine, "Je ne connais pas de %s, %%s", rest);
				Phrase[0] = strdup (chaine);
				sprintf(chaine, "%s ? Ça ne me dit rien %%s", rest);
				Phrase[1] = strdup (chaine);
				Repondre (from, to, 0, 2, Phrase, 0, 0, 0);
			}else
				send_to_user(from, "Je ne connais pas de %s", rest);
		}
	}
	else{
		if(ischannel(to)){
			//réponse publique
			Phrase = malloc (2 * sizeof (char *));
			Phrase[0] = strdup ("Je n'ai pas compris de qui tu parles, %s");
			Phrase[1] = strdup ("Qui ça, %s ?");
			Repondre (from, to, 0, 2, Phrase, 0, 0, 0);
		}
		else
			send_to_user(from, "Qui ça ?");
	}
}

int ChaineEstDans (char *aFouiller, char *aChercher) {
  char *AFouiller;
  char *AChercher;
  char *p;
  char *GuillemetsO = 0;
  char *GuillemetsF = 0;
  char *c;
  static int DernierNumPhrase = -1;

  char *Init = "éèêëÉÈÊËçÇàâäåÀÂùûÙÛôöÔÖ";
  char *Remp = "eeeeEEEEccaaaaAAuuUUooOO";

  AFouiller = strdup (aFouiller);
  AChercher = strdup (aChercher);
  for (p = AFouiller; *p; p++) {
    c = strchr (Init, *p);
    if (c) *p = Remp[c-Init];
    *p = tolower (toascii(*p));
  }
  for (p = AChercher; *p; p++) {
     c = strchr (Init, *p);
    if (c) *p = Remp[c-Init];
    *p = tolower (toascii(*p));
  }

  if (DernierNumPhrase != GNumPhrase) {
    GuillemetsO = strchr (AFouiller, '\"');
    if (GuillemetsO) {
      GuillemetsF = strchr (GuillemetsO+1, '\"');
      if (!GuillemetsF)
	GuillemetsF = strchr (GuillemetsO, '\0');
    }
    DernierNumPhrase = GNumPhrase;
  }

  if ((p = strstr (AFouiller, AChercher))
      && !(p > GuillemetsO
	   && p < GuillemetsF)) {
    if (AFouiller) free (AFouiller);
    if (AChercher) free (AChercher);
    return TRUE;
  }
  else {
    if (AFouiller) free (AFouiller);
    if (AChercher) free (AChercher);
    return FALSE;
  }
  
} /* int ChaineEstDans () */

void    Traite (char *from, char *to, char *msg)
{
  char *Trouve;
  static int Autorisation = TRUE;
  static int Jour = 0;
  int AncienneAutorisation = TRUE;
  char **Reponse, **Reponse2;
  locuteur *Locuteur;
  int i;
  int j;
  int NbRep = 0;
  int Num;
  int Numero;
  int Humeur;
  int Nouveau;
  int NOM        = FALSE;
  int BONJOUR    = FALSE;
  int SALUT      = FALSE;
  int JE_RESTE = FALSE;
  int CAVA       = FALSE;
  int AUREVOIR   = FALSE;
  int JE_M_EN_VAIS = FALSE;
  int MERCI      = FALSE;
  int BONNE_ANNEE= FALSE;
  int INJURE     = FALSE;
  int PARLER     = FALSE;
  int ENGLISH    = FALSE;
  int FRENCH     = FALSE;
  int SWEDISH    = FALSE;
  int COMPLIMENT = FALSE;
  int BEBE       = FALSE;
  int TOUTSEUL   = FALSE;
  int HABITE     = FALSE;
  int NANCY      = FALSE;
  int FRANCE     = FALSE;
  int PARENTS    = FALSE;
  int QUEL_ENDROIT = FALSE; /* ou ? where? */
  int ENNUI      = FALSE;
  int REVEILLE_TOI = FALSE;
  int DORMIR     = FALSE;
  int QUELQUUN   = FALSE;
  int ILYA       = FALSE; /* Il y a, est la, there, ici */
  int CALME      = FALSE;
  int TAISTOI    = FALSE;
  int LANGUE     = FALSE;
  int BONSOIR    = FALSE;
  int BONAPP     = FALSE;
  int VAIS_MANGER = FALSE;
  int ENERVEMENT = FALSE;
  int LORIA      = FALSE;
  int BISOUS     = FALSE;
  int ABOIRE     = FALSE;
  int REPONDRE   = FALSE;
  int LISTE_STIMULI = FALSE;
  int TESTE_STIMULI = FALSE;
  int LISTE_REPONSES = FALSE;
  int EXCUSE     = FALSE;
  int MOI        = FALSE;
  int DACCORD    = FALSE;
  int PASVRAI    = FALSE;
  int NEGATION   = FALSE;
  int ES_TU      = FALSE;
  int UN_BOT     = FALSE;
  int UN_MEC     = FALSE;
  int HUMAIN     = FALSE;
  int MORT       = FALSE;
  int MALADE     = FALSE;
  int FOU        = FALSE; 
  int UNE_FILLE  = FALSE;
  int HEURE      = FALSE;
  int AGE        = FALSE;
  int AS_TU      = FALSE;
  int PENSER     = FALSE;
  int ECOUTER    = FALSE;
  int MUSIQUE    = FALSE;
  int INVITER    = FALSE;
  int AMI         = FALSE;
  int COPINE      = FALSE;
  int FAIM        = FALSE;
  int SOIF        = FALSE;
  int BIERE       = FALSE;
  int BIENVENUE   = FALSE; /* Aussi Bon retour */
  int PIRATE      = FALSE;
  int WAREZ       = FALSE;
  int STP         = FALSE;
  int GRAND       = FALSE;
  int PETIT       = FALSE;
  int LIBRE       = FALSE;
  int OCCUPE      = FALSE;
  int CE_SOIR     = FALSE;
  int AIMES_TU    = FALSE;
  int M_AIMES_TU  = FALSE;
  int CONNAIS_TU  = FALSE;
  int NEIGE       = FALSE;
  int SOUVENIR    = FALSE;
  int CONNAITRE   = FALSE;
  int SOURIRE     = FALSE;
  int BAVARD      = FALSE;
  int FAIS_TU     = FALSE;
  int CAFE        = FALSE;
  int MOUARF      = FALSE; /* Joli chien-chien */
  int CLINDOEIL   = FALSE;
  int JEPEUX; /* Demande d'autorisation */
  int ETTOI = FALSE; /* Et toi? */
  int EXTRAITSTJOHNPERSE = FALSE;
  int STJOHNPERSE = FALSE;
  int PRESENT = FALSE;
  int SOURD = FALSE;
  int RIENCOMPRIS = FALSE;
  int POURQUOI = FALSE;
  int QUESTION = FALSE; /* es-tu as-tu est-ce-que est-il comment pourquoi suis-je ais-tu ? */
  int CROIS_TU = FALSE;
  int FATIGUE = FALSE;
  int PHOTO = FALSE; /* De lui-meme */
  int SOUHAIT = FALSE; /* Positif: Porte-toi bien */
  int ROSE = FALSE; /*  @>---,--`-- @>---,---`--- -> Thutmosis*/
  int JALOUX = FALSE;
  int INDILILI = FALSE; /* indiii indiii lilililililililililililili */
  int QUI = FALSE; /* Qui es-tu? */
  int JESUIS = FALSE;
  int TU_ES = FALSE;
  int NOUVELLESDUFRONT = FALSE;
  int AIE = FALSE; /* AIE AIE AIE, OUILLE OUILLE OUILLE */
  int CERVEAU = FALSE;
  int FONCTION_SAY = FALSE;
  int FONCTION_FUCK = FALSE;
  int FONCTION_APPRENDS = FALSE;
  int FONCTION = FALSE;
  int TU_FAIS = FALSE;
  int QUEL = FALSE;
  int TALON = FALSE; /* Jeux de mots Achille Talon -> BD */
  int MARIE = FALSE;
  int FEMME = FALSE;
  int QUEST_CE = FALSE;
  int QUOI = FALSE;
  int NICK = FALSE;
  int C_EST = FALSE;
  int ROT = FALSE;
  int REPONDS = FALSE;
  int JE_T_AIME = FALSE;
  int VEUX = FALSE; /* Ouin. Je VEUX que tu ... , j'ORDONNE*/
  int CRIN = FALSE; /* Centre de Recherche en Informatique de Nancy */
  int ETRANGERS = FALSE;
  int VIVE = FALSE; /* Viva! Rules rulezz */
  int WINTEL = FALSE;
  int AMIGA = FALSE;
  int FRONT_NATIONAL = FALSE; /* Argh! */
  int GROS_MOT = FALSE;
  int POUR = FALSE;
  int CONTRE = FALSE;
  int TU_BOUDES = FALSE;
  int SACRE = FALSE;
  int QUI_EST = FALSE;
  int TON_MAITRE = FALSE;
  int TU_AS = FALSE;
  int RAISON = FALSE;
  int SAIS_TU = FALSE;
  int KICKER = FALSE;
  int KICKE_MOI = FALSE;
  int CISEAUX = FALSE;
  int PAPIER = FALSE;
  int CAILLOU = FALSE;
  int PIERRE_PAPIER_CISEAUX = FALSE;
  int TRICHEUR = FALSE;
  int MAUVAIS_JOUEUR = FALSE;
  int PLEURER = FALSE;
  int ILS_FONT = FALSE;
  int MODESTE = FALSE;
  int CONTENT = FALSE;
  int JE_TE_DETESTE = FALSE;
  int ACHILLE = FALSE;
  char *NUS = NickUserStr (from);

/*   CHAN_list *Channel_to = (ischannel (to)?search_chan (to):currentbot->Current_chan); */
  CHAN_list *Channel_to = (ischannel (to)?search_chan (to):0);

  if ((ischannel (to) && is_log_on (to)) || (!ischannel (to) && log))
    botlog (LOGFILE, "<%s#%s> %s", from, to, msg);

  if (ChaineEstDans (msg, "ils font "))
    ILS_FONT = TRUE;

  if (ChaineEstDans (msg, currentbot->nick)
      || (ChaineEstDans (msg, "tous") && !ChaineEstDans(msg, "tous les "))
      || ChaineEstDans (msg, "a ts")
      || ChaineEstDans (msg, "les gars")
      || ChaineEstDans (msg, "les men") 
      || ChaineEstDans (msg, "les mecs")
      || ChaineEstDans (msg, "tout-le-monde")
      || ChaineEstDans (msg, "tout le monde")
      || ChaineEstDans (msg, "tout l'monde")
      || ChaineEstDans (msg, "tt le monde")
      || ChaineEstDans (msg, "tt l'monde")
      || ChaineEstDans (msg, "tt l monde")
      || ChaineEstDans (msg, "le bot")
      || ChaineEstDans (msg, "les bots")
	 || ChaineEstDans (msg, "petit bot")
      || ChaineEstDans (msg, " all ")
      || ChaineEstDans (msg, "all:")
      || ChaineEstDans (msg, "all!")
      || ChaineEstDans (msg, "you all")
      || ChaineEstDans (msg, "vous")
      || ChaineEstDans (msg, "touti")
      || ChaineEstDans (msg, "tutti")
      || ChaineEstDans (msg, "la compagnie")
      || ChaineEstDans (msg, "la jeunesse")
      || ChaineEstDans (msg, "les nanceens")
      || ChaineEstDans (msg, "les nanceiens")
      || ChaineEstDans (msg, "la Lorraine")
      || ChaineEstDans (msg, "les lorrains")
      || ChaineEstDans (msg, "everybody")
      || ChaineEstDans (msg, "'vrybody")
      || ChaineEstDans (msg, SURNOM) ||
      ChaineEstDans (msg, "ach'") ||
      ChaineEstDans (msg, "anybody") ||
      ChaineEstDans (msg, " there") ||
      ChaineEstDans (msg, "folks") ||
      ChaineEstDans (msg, "boys") ||
      ChaineEstDans (msg, "#nancy") ||
      ChaineEstDans (msg, "#la-bas") ||
      ChaineEstDans (msg, "messieurs") ||
      ChaineEstDans (msg, "everyone") ||
      ChaineEstDans (msg, "les potos") ||
      ChaineEstDans (msg, "les cocos") ||
      ChaineEstDans (msg, "mon biquet") ||
      ChaineEstDans (msg, "les amis") ||
      ChaineEstDans (msg, "les z'amis") ||
      ChaineEstDans (msg, "les copains") ||
      ChaineEstDans (msg, "les aminches") ||
      ChaineEstDans (msg, "bande de ") && ! ILS_FONT ||
      ChaineEstDans (msg, "m'sieurs dame") ||
      ChaineEstDans (msg, "m'sieurs-dame") ||
      ChaineEstDans (msg, "messieurs-dame") ||
      ChaineEstDans (msg, "messieurs dame") ||
      ChaineEstDans (msg, "messieurs") ||
      ChaineEstDans (msg, "les n'enfants") ||
      ChaineEstDans (msg, "les enfants") ||
      ChaineEstDans (msg, "les zenfants") ||
      ChaineEstDans (msg, "les petits") ||
      ChaineEstDans (msg, "le people") ||
      ChaineEstDans (msg, "le peuple") ||
	 ChaineEstDans (msg, "la foule") ||
      ChaineEstDans (msg, "todos") /* tous en espagnol */
      )
    NOM = TRUE;

  if (ChaineEstDans (msg, "y a") ||
      ChaineEstDans (msg, "y'a") ||
      ChaineEstDans (msg, "y-a") ||
      ChaineEstDans (msg, " est la") ||
      ChaineEstDans (msg, "there"))
    ILYA        = TRUE;

  if (ChaineEstDans (msg, "bonjour") ||
      ChaineEstDans (msg, "bonne journee") ||
      ChaineEstDans (msg, "bjr") ||
      ChaineEstDans (msg, "'jour") ||
      ChaineEstDans (msg, "hello") ||
      ChaineEstDans (msg, "hola") || /* En espagnol */
      ChaineEstDans (msg, "lo ") && !ChaineEstDans (msg, "rigolo") ||
      ChaineEstDans (msg, "szia")  /* En hongrois */ ||
      ChaineEstDans (msg, "hei ") /* En finnois */ ||
      ChaineEstDans (msg, "hej") && !ChaineEstDans (msg, "hej da") /* En suedois */ ||
	 ChaineEstDans (msg, "salam") /* En arabe */ || 
      ChaineEstDans (msg, "zdravi") /* En slovaque */)
    BONJOUR  = TRUE;

  if (ChaineEstDans (msg, "salu") ||
      (ChaineEstDans (msg, "hi ") || strstr (msg, "Hi")) && !ChaineEstDans (msg, "high") && !ChaineEstDans (msg, "hihi") && !ChaineEstDans (msg, "chi") ||
      ChaineEstDans (msg, "coucou") ||
      ChaineEstDans (msg, "kikou") ||
      ChaineEstDans (msg, "youhou") ||
      ChaineEstDans (msg, "heya") ||
      ChaineEstDans (msg, "beuha") ||
      ChaineEstDans (msg, "yo ") ||
      ChaineEstDans (msg, "hugh") ||
      ChaineEstDans (msg, "'lut") ||
      ChaineEstDans (msg, "lut ") ||
      ChaineEstDans (msg, "slt") ||
      ChaineEstDans (msg, "hallo") /* En allemand */ ||
      ChaineEstDans (msg, "tjena") /* En suedois */
      )
    SALUT    = TRUE;

  if (ChaineEstDans (msg, "je reste"))
	JE_RESTE = TRUE;
  
  if (ChaineEstDans (msg, "ca va") ||
      ChaineEstDans (msg, "ca gaz") ||
      ChaineEstDans (msg, "vas-tu") ||
      ChaineEstDans (msg, "vas tu") ||
      ChaineEstDans (msg, "vas bien") ||
      ChaineEstDans (msg, "va bien") ||
      ChaineEstDans (msg, "en forme") ||
      ChaineEstDans (msg, "le moral") ||
      ChaineEstDans (msg, "la forme") ||
      ChaineEstDans (msg, "comment va") ||
      ChaineEstDans (msg, "comment tu va") ||
      ChaineEstDans (msg, "comment te sens-tu") ||
      ChaineEstDans (msg, "comment te sens tu") ||
      ChaineEstDans (msg, "comment tu te sens") ||
      ChaineEstDans (msg, "comment tu t'sens") ||
      ChaineEstDans (msg, "ti k'sa va") ||
      ChaineEstDans (msg, "ca baigne") ||
      ChaineEstDans (msg, "ca boume") ||
      ChaineEstDans (msg, "ca roule") ||
      ChaineEstDans (msg, "sens bien") ||
      ChaineEstDans (msg, "vous allez bien") ||
      ChaineEstDans (msg, "allez-vous bien") ||
      ChaineEstDans (msg, "comment vous allez") ||
      ChaineEstDans (msg, "comment allez-vous") ||
      ChaineEstDans (msg, "commen y le") || /* En creole */
      ChaineEstDans (msg, "how are u") ||
      ChaineEstDans (msg, "how are you") ||
      ChaineEstDans (msg, "how do you do") ||
      ChaineEstDans (msg, "Mita kuuluu") || /* en finnois */
	 ChaineEstDans (msg, "tu rak bikir") /* en arabe */)
    CAVA     = TRUE;

  if (ChaineEstDans (msg, "au revoir") ||
      ChaineEstDans (msg, "arvoir") ||
      ChaineEstDans (msg, "arrosoir persi") ||
      ChaineEstDans (msg, "a+") ||
      ChaineEstDans (msg, "a plus") ||
      ChaineEstDans (msg, "m'en vais") ||
      ChaineEstDans (msg, "a bientot") ||
      ChaineEstDans (msg, "bon week end") && !ChaineEstDans (msg, "passe un bon") ||
      ChaineEstDans (msg, "bon we") ||
      ChaineEstDans (msg, "bon week-end") ||
      ChaineEstDans (msg, "m'en aller") ||
      ChaineEstDans (msg, " partir") ||
      ChaineEstDans (msg, "vais y aller") ||
      ChaineEstDans (msg, "vais vous laisser") ||
      ChaineEstDans (msg, "y vais") ||
      ChaineEstDans (msg, "bye") ||
      ChaineEstDans (msg, "a demain") ||
      ChaineEstDans (msg, "a 2 mains") ||
      ChaineEstDans (msg, "adtaleur") ||
      ChaineEstDans (msg, "ad'taleur") ||
      ChaineEstDans (msg, "a tout-a-l'heure") ||
      ChaineEstDans (msg, "a tout a l'heure") ||
      ChaineEstDans (msg, "a tout a l heure") ||
      ChaineEstDans (msg, "a tt a l'heure") ||
      ChaineEstDans (msg, "nar r'trouv") || /* Creole */
      ChaineEstDans (msg, "n'artrouv") || 
      ChaineEstDans (msg, "n'ar trouv") ||
      ChaineEstDans (msg, " me casse") ||
      ChaineEstDans (msg, " m'casse") ||
      ChaineEstDans (msg, "vous laisse") ||
      ChaineEstDans (msg, "je te laisse") ||
      ChaineEstDans (msg, "a un de ces quatres") ||
      ChaineEstDans (msg, "a un de ces 4") ||
      ChaineEstDans (msg, "a un de c 4") ||
      ChaineEstDans (msg, "a 1 de c 4") ||
      ChaineEstDans (msg, "a12c4") ||
      ChaineEstDans (msg, "a1274") ||
      ChaineEstDans (msg, "see you") ||
      ChaineEstDans (msg, "see u") ||
      ChaineEstDans (msg, "see ya") ||
      ChaineEstDans (msg, "later") ||
      ChaineEstDans (msg, "c u ") ||
      ChaineEstDans (msg, "je pars") ||
      ChaineEstDans (msg, "je parte") ||
      ChaineEstDans (msg, "tchao") ||
      ChaineEstDans (msg, "tshaw") ||
      ChaineEstDans (msg, "ciao") ||
      ChaineEstDans (msg, "j'y aille") ||
      ChaineEstDans (msg, "me sauve") ||
      ChaineEstDans (msg, "hasta luego") ||
      ChaineEstDans (msg, "arwar") ||
      ChaineEstDans (msg, "a la prochaine") ||
      ChaineEstDans (msg, "vi ses") /* En suedois : "on se voit" */ ||
      ChaineEstDans (msg, "cassos") ||
      ChaineEstDans (msg, "je rentre") && !ChaineEstDans (msg, "quand ") ||
      ChaineEstDans (msg, "revoyure") ||
      ChaineEstDans (msg, "r'voyure") ||
      ChaineEstDans (msg, "wiedersehen") ||
      ChaineEstDans (msg, "wieder sehen") ||
      ChaineEstDans (msg, "have to go") ||
      ChaineEstDans (msg, "I must go") ||
      ChaineEstDans (msg, "time to go") ||
      ChaineEstDans (msg, "nar trouver zot tout") || /* En Creole */
      ChaineEstDans (msg, "hej da") /* En suedois */
      )
    AUREVOIR = TRUE;
  
  if (ChaineEstDans (msg, "je m'en vais") ||
      ChaineEstDans (msg, "j'm'en vais") ||
      ( ChaineEstDans (msg, "je vais") ||
	ChaineEstDans (msg, "je v ") ||
	ChaineEstDans (msg, "j'vais") ||
	ChaineEstDans (msg, "jvai")) &&
      ( ChaineEstDans (msg, " y aller") ||
	ChaineEstDans (msg, " m'en aller") ||
	ChaineEstDans (msg, " partir") ||
	ChaineEstDans (msg, "rentrer") ) ||
      ChaineEstDans (msg, "que j'y aille") ||
      ChaineEstDans (msg, "je pars") ||
      ChaineEstDans (msg, "j'y vais") ||
      ChaineEstDans (msg, "j'y go") ||
      ChaineEstDans (msg, "j y go") ||
      ChaineEstDans (msg, "j go") ||
      ChaineEstDans (msg, "je rentre") ||
      ChaineEstDans (msg, "cassos") ||
      ChaineEstDans (msg, "je me casse") ||
      ChaineEstDans (msg, "j'me casse") ||
      ChaineEstDans (msg, "je me barre") ||
      ChaineEstDans (msg, "j'me barre") ||
      ChaineEstDans (msg, "je me sauve") ||
      ChaineEstDans (msg, "j'me sauve") ||
      ChaineEstDans (msg, "je me rentre") ||
      ChaineEstDans (msg, "j'me rentre") ||
      ChaineEstDans (msg, "je reviendrai") ||
      ChaineEstDans (msg, "je dois y aller") ||
      ChaineEstDans (msg, "je dois partir") ||
      ChaineEstDans (msg, "je dois filer") ||
      ChaineEstDans (msg, "je vous quitte") ||
      ChaineEstDans (msg, "j'vous quitte") ||
      ChaineEstDans (msg, "I've to leave")
      )
  JE_M_EN_VAIS = TRUE;

  if (ChaineEstDans (msg, "merci") && !ChaineEstDans (msg, "commerci") ||
      ChaineEstDans (msg, "merchi") ||
      ChaineEstDans (msg, "'ci") ||
      ChaineEstDans (msg, "thanks") ||
      ChaineEstDans (msg, "thanx") ||
      ChaineEstDans (msg, "thx") ||
      ChaineEstDans (msg, "tack") && !ChaineEstDans (msg, "tackl") /* En suedois */
      )
    MERCI = TRUE;

  if (ChaineEstDans (msg, "bonne annee") ||
      ChaineEstDans (msg, "happy new year"))
    BONNE_ANNEE = TRUE;

  if (ChaineEstDans (msg, "connard") ||
      ChaineEstDans (msg, "couillon") ||
      ChaineEstDans (msg, "ducon") ||
      (ChaineEstDans (msg, "con ") ||
       ChaineEstDans (msg, "con,")) && !ChaineEstDans (msg, "facon") && !ChaineEstDans (msg,"rcon") && !ChaineEstDans (msg, "scon") && !ChaineEstDans (msg, "conf") && !ChaineEstDans (msg, "falcon") ||
      ChaineEstDans (msg, "gros con") ||
      ChaineEstDans (msg, "sale con") ||
      ChaineEstDans (msg, "tit con") ||
      ChaineEstDans (msg, " des cons") ||
      ChaineEstDans (msg, "salaud") ||
      ChaineEstDans (msg, "salop") && !ChaineEstDans (msg, "salopette")||
      ChaineEstDans (msg, "mechant") ||
      ChaineEstDans (msg, "injuste") ||
      ChaineEstDans (msg, "degueulasse") ||
      ChaineEstDans (msg, "malpropre") ||
      /*      ChaineEstDans (msg, "sale") && !ChaineEstDans (msg, "salem") && !ChaineEstDans (msg, "ssale") || */
      ChaineEstDans (msg, "tu pues") ||
      ChaineEstDans (msg, " puent") ||
      ChaineEstDans (msg, "pourceau") ||
      ChaineEstDans (msg, "tu sens movais") ||
      ChaineEstDans (msg, "tu sens mauvais") ||
      ChaineEstDans (msg, "sentent mauvais") ||
      ChaineEstDans (msg, "amibe") ||
      ChaineEstDans (msg, "amorphe") ||
      ChaineEstDans (msg, "leche-cul") ||
/*       ChaineEstDans (msg, "lèche-cul") || */
      ChaineEstDans (msg, "leche cul") ||
/*       ChaineEstDans (msg, "lèche cul") || */
      ChaineEstDans (msg, "casse-couille") ||
      ChaineEstDans (msg, "pourri") && !ChaineEstDans (msg, "pourriez") ||
      ChaineEstDans (msg, "tache") ||
      ChaineEstDans (msg, "raclure") ||
      ChaineEstDans (msg, "indecent") ||
      ChaineEstDans (msg, "vulgaire") ||
      ChaineEstDans (msg, "triple buse") ||
      ChaineEstDans (msg, "chacal") ||
      ChaineEstDans (msg, "primate") ||
      ChaineEstDans (msg, "rascal") ||
      ChaineEstDans (msg, "minable") ||
      ChaineEstDans (msg, "corniaud") ||
      ChaineEstDans (msg, "trisomique") ||
      ChaineEstDans (msg, "imbecile") ||
/*       ChaineEstDans (msg, "imbécile") || */
      ChaineEstDans (msg, "baka") || /* imbecile en japonais */
      ChaineEstDans (msg, "corniaud") ||
      ChaineEstDans (msg, "blaireau") ||
      ChaineEstDans (msg, "cafard") ||
      ChaineEstDans (msg, "idiot") ||
      ChaineEstDans (msg, "stupid") ||
      ChaineEstDans (msg, "nigaud") ||
      ChaineEstDans (msg, "neuneu") ||
      ChaineEstDans (msg, " bete") ||
      ChaineEstDans (msg, "faible d'esprit") ||
      ChaineEstDans (msg, "attarde mental") ||
      ChaineEstDans (msg, "cretin") ||
/*       ChaineEstDans (msg, "crétin") || */
      ChaineEstDans (msg, "debile") ||
/*       ChaineEstDans (msg, "débile") || */
      ChaineEstDans (msg, "niais") ||
      ChaineEstDans (msg, "nul ") ||
      ChaineEstDans (msg, " nul") && !ChaineEstDans (msg, "match") || 
      ChaineEstDans (msg, "abruti") ||
      ChaineEstDans (msg, "ahuri") ||
      ChaineEstDans (msg, "gogol") ||
      ChaineEstDans (msg, "naze") ||
      ChaineEstDans (msg, "pignouf") ||
      ChaineEstDans (msg, "mauvais") ||
      ChaineEstDans (msg, "mongole") ||
      ChaineEstDans (msg, "andouille") ||
      ChaineEstDans (msg, "cornichon") ||
	 ChaineEstDans (msg, "banane") ||
	 ChaineEstDans (msg, "quetsche") ||
      ChaineEstDans (msg, "cake") ||
      ChaineEstDans (msg, "enfoire") ||
/*       ChaineEstDans (msg, "enfoiré") || */
      ChaineEstDans (msg, "encule") ||
      ChaineEstDans (msg, "enqule") || /* Y'en a qui sont nuls en orthographe! */
/*       ChaineEstDans (msg, "enculé") || */
      ChaineEstDans (msg, "fuck") ||
      ChaineEstDans (msg, "va te faire") ||
      ChaineEstDans (msg, "te faire foutre") ||
      ChaineEstDans (msg, "lopete") ||
      ChaineEstDans (msg, "lopette") && !ChaineEstDans (msg, "salopette") ||
      ChaineEstDans (msg, "tapette") ||
      ChaineEstDans (msg, "pd") && !ChaineEstDans (msg, "pdt") && !ChaineEstDans (msg, "repdel") ||
      ChaineEstDans (msg, "tantouse") ||
      ChaineEstDans (msg, "impuissant") ||
      ChaineEstDans (msg, "putain") ||
      ChaineEstDans (msg, "putin") || /* faute d'ortho */
      ChaineEstDans (msg, "bitch") ||
      ChaineEstDans (msg, "tu suces") ||
      ChaineEstDans (msg, "tete de noeud") ||
      ChaineEstDans (msg, "fumier") ||
      ChaineEstDans (msg, "trou du c") ||
      ChaineEstDans (msg, "trou duc") ||
      ChaineEstDans (msg, "trouduc") ||
      ChaineEstDans (msg, "chiant") ||
      ChaineEstDans (msg, "fais chier") && !ChaineEstDans (msg, "te fais chier") ||
      ChaineEstDans (msg, "fait chier") && !ChaineEstDans (msg, "se fait chier")||
      ChaineEstDans (msg, "vas chier") ||
      ChaineEstDans (msg, " de daube") ||
      ChaineEstDans (msg, "charogne") ||
      ChaineEstDans (msg, "lourd") ||
      ChaineEstDans (msg, "merd") && !ChaineEstDans (msg, "demerd") ||
      ChaineEstDans (msg, "espece d") ||
/*       ChaineEstDans (msg, "espèce d") || */
      ChaineEstDans (msg, "ta gueule") ||
      ChaineEstDans (msg, "ta geule") || /* Faute d'orthographe courante */
	 ChaineEstDans (msg, "face de ") || 
      ChaineEstDans (msg, "menteur") ||
      ChaineEstDans (msg, "malhonnete") ||
      ChaineEstDans (msg, "escroc") ||
      ChaineEstDans (msg, "charlatan") ||
      ChaineEstDans (msg, "canaille") ||
      ChaineEstDans (msg, "chiendent") ||
      ChaineEstDans (msg, "patate") && !ChaineEstDans (msg, "patater") ||
      ChaineEstDans (msg, "outre") && !ChaineEstDans (msg, "foutre") ||
      ChaineEstDans (msg, "bouffon") ||
      /*ChaineEstDans (msg, "pelo") ?!? || */
      ChaineEstDans (msg, "pov gars") ||
      ChaineEstDans (msg, "moche") ||
      ChaineEstDans (msg, "affreu") ||
      ChaineEstDans (msg, "laid") ||
      ChaineEstDans (msg, "vilain") ||
      ChaineEstDans (msg, "morveux") ||
      ChaineEstDans (msg, "alcolo") ||
      ChaineEstDans (msg, "alcoolo") ||
      ChaineEstDans (msg, "alcoolique") ||
      ChaineEstDans (msg, "alcolique") ||
      ChaineEstDans (msg, "navrant") ||
      ChaineEstDans (msg, "lamentable") ||
      ChaineEstDans (msg, "faineant") ||
      ChaineEstDans (msg, "faignant") ||
      ChaineEstDans (msg, "flemmard") ||
      ChaineEstDans (msg, "bouzeu") ||
      ChaineEstDans (msg, "bouseu") ||
      ChaineEstDans (msg, "sagouin") ||
      ChaineEstDans (msg, "sagoin")
      )
    INJURE = TRUE;

  if (ChaineEstDans (msg, "intelligent") ||
      (ChaineEstDans (msg, "demerde") && ChaineEstDans (msg, " bien")) ||
      ChaineEstDans (msg, "courageux") ||
      ChaineEstDans (msg, "cool") && !ChaineEstDans (msg, "alcool") ||
      ChaineEstDans (msg, "sympa") ||
      ChaineEstDans (msg, "marrant") ||
      ChaineEstDans (msg, "rigolo") ||
      ChaineEstDans (msg, "joyeux") ||
      ChaineEstDans (msg, " poli") && !ChaineEstDans (msg, "polisson") ||
      ChaineEstDans (msg, " fort ") ||
      ChaineEstDans (msg, " forts ") ||
      ChaineEstDans (msg, "puissant") && !ChaineEstDans (msg, "impuissant") ||
      ChaineEstDans (msg, "efficace") ||
      ChaineEstDans (msg, "rapid") ||
      ChaineEstDans (msg, "champion") ||
      ChaineEstDans (msg, "super") ||
      ChaineEstDans (msg, "extra") ||
      ChaineEstDans (msg, "genial") ||
      ChaineEstDans (msg, "geniaux") ||
      ChaineEstDans (msg, "genie") ||
      ChaineEstDans (msg, "une bete") ||
/*       ChaineEstDans (msg, "une bête") || */
	 ChaineEstDans (msg, "etalon") ||
      ChaineEstDans (msg, "le meilleur") ||
      ChaineEstDans (msg, "excellent") ||
/*       ChaineEstDans (msg, "juste") && !ChaineEstDans (msg, "injuste") || */
      ChaineEstDans (msg, "interessant") ||
      ChaineEstDans (msg, "impressionnant") ||
      ChaineEstDans (msg, "epatant") ||
      ChaineEstDans (msg, "beau") && !ChaineEstDans (msg, "beauc") && !ChaineEstDans (msg, "beaut") ||
      ChaineEstDans (msg, "elegant") ||
/*       ChaineEstDans (msg, "élégant") || */
      ChaineEstDans (msg, "galant") ||
      ChaineEstDans (msg, "magnifi") ||
      ChaineEstDans (msg, "splendide") ||
      ChaineEstDans (msg, "joli") ||
      ChaineEstDans (msg, "classe") ||
      ChaineEstDans (msg, "merveille") ||
      ChaineEstDans (msg, "chouette") ||
      ChaineEstDans (msg, " chou") ||
      ChaineEstDans (msg, "gentil") ||
      ChaineEstDans (msg, "mignon") ||
      ChaineEstDans (msg, "adorable") ||
      ChaineEstDans (msg, "adore") ||
      ChaineEstDans (msg, "bravo") ||
      ChaineEstDans (msg, "un bon") ||
      ChaineEstDans (msg, "tres bon") ||
      ChaineEstDans (msg, "bon bot") ||
      ChaineEstDans (msg, "bien vu") ||
      ChaineEstDans (msg, "serviable") ||
      ChaineEstDans (msg, "felicit") ||
      ChaineEstDans (msg, "un ange") ||
      ChaineEstDans (msg, "un dieu") 
      )
    COMPLIMENT= TRUE;

  if (ChaineEstDans (msg, "parl") ||
      ChaineEstDans (msg, "discut") ||
      ChaineEstDans (msg, "bavarde") ||
      ChaineEstDans (msg, "cause") && !ChaineEstDans (msg, "because") && !ChaineEstDans (msg, "a cause ") ||
      ChaineEstDans (msg, "speak") ||
      ChaineEstDans (msg, "talk") ||
      ChaineEstDans (msg, "prata") /* En suedois */ ||
      ChaineEstDans (msg, "habla") /* En espagnol */
      )
    PARLER = TRUE;

  if (ChaineEstDans (msg, "english") ||
      ChaineEstDans (msg, "anglais") ||
      ChaineEstDans (msg, "angliche") ||
      ChaineEstDans (msg, "american") ||
      ChaineEstDans (msg, "eng.")
      )
    ENGLISH = TRUE;

  if (ChaineEstDans (msg, "french") ||
      ChaineEstDans (msg, "frog") ||
      ChaineEstDans (msg, "francais"))
    FRENCH = TRUE;

  if (ChaineEstDans (msg, "svenska") ||
      ChaineEstDans (msg, "suedois") ||
      ChaineEstDans (msg, "swedish"))
    SWEDISH = TRUE;

  if (ChaineEstDans (msg, "areuh") ||
      ChaineEstDans (msg, "gouzi") ||
      ChaineEstDans (msg, "gazou"))
    BEBE = TRUE;

  if (ChaineEstDans (msg, "tout seul") ||
      ChaineEstDans (msg, "toute seule") ||
      ChaineEstDans (msg, "alone") ||
      ChaineEstDans (msg, "personne") && ILYA
      )
    TOUTSEUL = TRUE;

  if (ChaineEstDans (msg, "nancy"))
    NANCY = TRUE;

  if (ChaineEstDans (msg, "parent") ||
      ChaineEstDans (msg, "pere") ||
      ChaineEstDans (msg, "papa") ||
      ChaineEstDans (msg, " mere") ||
      ChaineEstDans (msg, "maman"))
    PARENTS = TRUE;

  if (ChaineEstDans (msg, " ou ") && !ChaineEstDans (msg, " ou quoi") ||
      ChaineEstDans (msg, "d'ou") ||
      ChaineEstDans (msg, " ou") && !ChaineEstDans (msg, " oui") && !ChaineEstDans (msg, " ou quoi") ||
/*       ChaineEstDans (msg, " où") || */
/*       ChaineEstDans (msg, "où ") || */
      ChaineEstDans (msg, "quel endroit") ||
      ChaineEstDans (msg, "where") ||
      ChaineEstDans (msg, "from") ||
      ChaineEstDans (msg, "ou ca") ||
      ChaineEstDans (msg, "quel coin"))
    QUEL_ENDROIT = TRUE;

  if (ChaineEstDans (msg, "ennui") ||
      ChaineEstDans (msg, "ennuy") ||
      ChaineEstDans (msg, "chier") && !ChaineEstDans (msg, "fichier") ||
      ChaineEstDans (msg, "embet") ||
/*       ChaineEstDans (msg, "embêt") || */
      ChaineEstDans (msg, "mortel")
      )
    ENNUI = TRUE;

  if (ChaineEstDans (msg, "dors ") ||
      ChaineEstDans (msg, " dort ") ||
      ChaineEstDans (msg, "dorm") ||
      ChaineEstDans (msg, "lethargie") ||
      ChaineEstDans (msg, "sleep") ||
      ChaineEstDans (msg, "reveille") ||
/*       ChaineEstDans (msg, "réveillé") || */
      ChaineEstDans (msg, "roupill"))
    DORMIR = TRUE;

  if (ChaineEstDans (msg, "reveille toi") ||
      ChaineEstDans (msg, "tu te reveille"))
    REVEILLE_TOI = TRUE;

  if (ChaineEstDans (msg, "quelqu'un") ||
      ChaineEstDans (msg, "quelqu un") ||
      ChaineEstDans (msg, "kelk'un") ||
      ChaineEstDans (msg, "kelkun") ||
      ChaineEstDans (msg, "kkun") ||
      ChaineEstDans (msg, "kk1") ||
      ChaineEstDans (msg, "kelkin") ||
      ChaineEstDans (msg, "qquun") ||
      ChaineEstDans (msg, "qq'un") ||
      ChaineEstDans (msg, "qqun") ||
      ChaineEstDans (msg, "qqn") ||
      ChaineEstDans (msg, "qq1") ||
      ChaineEstDans (msg, "qlqn") ||
      ChaineEstDans (msg, " des gens") ||
      ChaineEstDans (msg, "someone") ||
      ChaineEstDans (msg, "somebody") ||
      ChaineEstDans (msg, "somebady") ||
      ChaineEstDans (msg, "any") ||
      ChaineEstDans (msg, "jemand") ||
      ChaineEstDans (msg, "alguen") /* En espagnol */
      )
    QUELQUUN = TRUE;
  
  if (ChaineEstDans (msg, "calme") ||
      /* ChaineEstDans (msg, "mort") || */
      (ChaineEstDans (msg, " quiet") && !ChaineEstDans (msg, "inquiet")) ||
      /* ChaineEstDans (msg, "un ange passe") ||*/
      ChaineEstDans (msg, "tranquille") ||
      ChaineEstDans (msg, "reposant")
      )
    CALME = TRUE;

  if (ChaineEstDans (msg, "ta gueule") ||
      ChaineEstDans (msg, "ta geule") || /* faute courante */
      ChaineEstDans (msg, "shut up") ||
      (ChaineEstDans (msg, "chut") && !ChaineEstDans (msg, "chute") && !ChaineEstDans (msg, "chuti")) ||
      ChaineEstDans (msg, "silence") ||
      ChaineEstDans (msg, "tais-toi") ||
      ChaineEstDans (msg, "tais toi") ||
      ChaineEstDans (msg, "te taire") ||
      ChaineEstDans (msg, "ferme la") ||
      ChaineEstDans (msg, "la ferme") ||
      ChaineEstDans (msg, "boucle la") ||
      ChaineEstDans (msg, "ca suffit") ||
/*       ChaineEstDans (msg, "ça suffit") || */
      ChaineEstDans (msg, "en veilleuse"))
    TAISTOI = TRUE;

  if (ChaineEstDans (msg, "bonsoir") ||
      ChaineEstDans (msg, "bonne soiree") ||
/*       ChaineEstDans (msg, "bonne soirée") || */
      ChaineEstDans (msg, "bonne nuit") ||
	 ChaineEstDans (msg, "beuah") || /* En Guignols */
      ChaineEstDans (msg, "dobru noc") /* En slovaque */
      )
    BONSOIR = TRUE;

  if (ChaineEstDans (msg, "aper") && !ChaineEstDans (msg, "taper") ||
/*       ChaineEstDans (msg, "apér") || */
      ChaineEstDans (msg, "bon appe") ||
/*       ChaineEstDans (msg, "bon appé") || */
      ChaineEstDans (msg, "bonne appe") || /* Pfff! */
/*       ChaineEstDans (msg, "bonne appé") || */
      ChaineEstDans (msg, "bonne bouffe") ||
      ChaineEstDans (msg, "suze") ||
      ChaineEstDans (msg, "ricard"))
    BONAPP = TRUE;

  if(((ChaineEstDans (msg, "dejeuner") ||
       ChaineEstDans (msg, "diner") ||
       ChaineEstDans (msg, "manger") ||
       ChaineEstDans (msg, "grailler") ||
       ChaineEstDans (msg, "bouffer")) &&
      (ChaineEstDans (msg, "je pars ") ||
       ChaineEstDans (msg, "vais aller") ||
       ChaineEstDans (msg, "je vais") ||
       ChaineEstDans (msg, "je v ") ||
       ChaineEstDans (msg, "je file")
       )))
    VAIS_MANGER = TRUE;

  if (ChaineEstDans (msg, "enerve"))
    ENERVEMENT = TRUE;

  if (ChaineEstDans (msg, "bisou") ||
      ChaineEstDans (msg, "bizou") ||
      ChaineEstDans (msg, "bise") ||
      ChaineEstDans (msg, "poutou") ||
      ChaineEstDans (msg, "embrass") ||
      ChaineEstDans (msg, "smack"))
    BISOUS = TRUE;

  if (ChaineEstDans (msg, "boire") ||
      ChaineEstDans (msg, "apero") ||
/*       ChaineEstDans (msg, "apér") || */
      ChaineEstDans (msg, "champagne") ||
      ChaineEstDans (msg, "pommard") ||
      ChaineEstDans (msg, "pastis") ||
      ChaineEstDans (msg, "buvons") ||
      ChaineEstDans (msg, " vin "))
    ABOIRE     = TRUE;

  if (ChaineEstDans (msg, " moi ") /* || ChaineEstDans (msg, "m'") */
      || ChaineEstDans (msg, "je ") || ChaineEstDans (msg, "j'")
      || ChaineEstDans (msg, " me "))
    MOI        = TRUE;

  if (ChaineEstDans (msg, "pelforth") ||
      ChaineEstDans (msg, "Guinness") ||
      ChaineEstDans (msg, "Kro") ||
      ChaineEstDans (msg, "K-net") ||
      ChaineEstDans (msg, "canette") ||
      ChaineEstDans (msg, "binouse") ||
      ChaineEstDans (msg, " pel ") ||
      ChaineEstDans (msg, " mousse") ||
      ChaineEstDans (msg, "biere"))
    BIERE      = TRUE;

  if (ChaineEstDans (msg, "pas vrai") && !ChaineEstDans (msg, "pas vraiment") ||
      ChaineEstDans (msg, "n'est-ce-pas") ||
      ChaineEstDans (msg, "n'est-ce pas") ||
      ChaineEstDans (msg, "n'est ce pas") ||
      ChaineEstDans (msg, "n est ce pas") ||
      ChaineEstDans (msg, "hein ") ||
      ChaineEstDans (msg, "hein,") ||
      ChaineEstDans (msg, "isn't it"))
    PASVRAI    = TRUE;

  if (((ChaineEstDans (msg, " pas ") || ChaineEstDans (msg, " po ")) &&!PASVRAI ||
       ChaineEstDans (msg, " ne ") ||
       ChaineEstDans (msg, "sauf") ||
       (ChaineEstDans (msg, " n'") &&
	!(ChaineEstDans (msg, " qu'") ||
	  ChaineEstDans (msg, " que "))
	)) && !ChaineEstDans (msg, "un peu") && !PASVRAI
      )
    NEGATION   = TRUE;

  if (ChaineEstDans (msg, "d'accord")
      || ChaineEstDans (msg, "d accord")
      )
    DACCORD    = TRUE;

  if (ChaineEstDans (msg, "excuse")
      || ChaineEstDans (msg, "pardon")
      || ChaineEstDans (msg, "desole") ||
/*       ChaineEstDans (msg, "désolé") || */
      ChaineEstDans (msg, "pitie") ||
/*       ChaineEstDans (msg, "pitié") || */
      ChaineEstDans (msg, "supplie"))
    EXCUSE     = TRUE;

  if (ChaineEstDans (msg, "repond")
/*       || ChaineEstDans (msg, "répond") */
      || ChaineEstDans (msg, "reponse")
/*       || ChaineEstDans (msg, "réponse") */)
    REPONDRE   = TRUE;

  if (ChaineEstDans (msg, "ami") && !ChaineEstDans (msg, "gamin")
      || ChaineEstDans (msg, "copain")
      || ChaineEstDans (msg, "camarade")
      || ChaineEstDans (msg, "pote") ||
      ChaineEstDans (msg, "poto"))
    AMI        = TRUE;

  if (ChaineEstDans (msg, "heure") && !ChaineEstDans (msg, "heureu") && !ChaineEstDans (msg, "tout-a-l'heure") && !ChaineEstDans (msg, "tout a l'heure") ||
      ChaineEstDans (msg, "time") && !ChaineEstDans (msg, "stime") && !ChaineEstDans (msg, "timent") && !ChaineEstDans (msg, "!time"))
    HEURE      = TRUE;

  if (ChaineEstDans (msg, "faim") ||
      ChaineEstDans (msg, "manger") &&
      (ChaineEstDans (msg, "veux") || ChaineEstDans (msg, "envie d"))
      )
    FAIM       = TRUE;

  if (ChaineEstDans (msg, "soif"))
    SOIF       = TRUE;

  if (ChaineEstDans (msg, "pense"))
    PENSER     = TRUE;

  if (ChaineEstDans (msg, "es-tu ")
      || ChaineEstDans (msg, " es-tu")
      || ChaineEstDans (msg, "t'es ")
      || ChaineEstDans (msg, "es tu")
      || ChaineEstDans (msg, "est tu") /* orthographe! */
      || ChaineEstDans (msg, "t es ")
      || ChaineEstDans (msg, "tu es ") ||
      ChaineEstDans (msg, "vous etes") ||
/*       ChaineEstDans (msg, "vous êtes") || */
      ChaineEstDans (msg, "etes vous") ||
      ChaineEstDans (msg, "etes-vous")
      || ChaineEstDans (msg, "are you ")
      || ChaineEstDans (msg, "r u "))
    ES_TU      = TRUE;

  if (ChaineEstDans (msg, "t'es ")
	 || ChaineEstDans (msg, "t es ")
      || ChaineEstDans (msg, "tu es ") ||
      ChaineEstDans (msg, "vous etes") ||
	 ChaineEstDans (msg, "you are") ||
	 ChaineEstDans (msg, "u r "))
    TU_ES      = TRUE;

  if (ChaineEstDans (msg, "tu as ") ||
      ChaineEstDans (msg, "t'as ") ||
      ChaineEstDans (msg, "t as ") ||
      ChaineEstDans (msg, "vous avez"))
    TU_AS = TRUE;

  if (ChaineEstDans (msg, "un bot")
      || ChaineEstDans (msg, "un robot")
      || ChaineEstDans (msg, "un automate")
      || ChaineEstDans (msg, "un programme")
      || ChaineEstDans (msg, "androide")
/*       || ChaineEstDans (msg, "androïde") */
      || ChaineEstDans (msg, "une machine"))
    UN_BOT     = TRUE;

  if (ChaineEstDans (msg, "un mec")
      || ChaineEstDans (msg, "un gars")
      || ChaineEstDans (msg, "garcon")
      || ChaineEstDans (msg, "homme"))
    UN_MEC     = TRUE;

  if (ChaineEstDans (msg, "humain") ||
      ChaineEstDans (msg, "homme") ||
      ChaineEstDans (msg, "homo sapiens"))
    HUMAIN     = TRUE;

  if (ChaineEstDans (msg, "fille")
      || ChaineEstDans (msg, "une nana")
      || ChaineEstDans (msg, "une femme")
      || ChaineEstDans (msg, "une meuf"))
    UNE_FILLE  = TRUE;

  if (ChaineEstDans (msg, "malade")
      || ChaineEstDans (msg, "pas bien")
      || ChaineEstDans (msg, "pas en forme")
      || ChaineEstDans (msg, "souffrant"))
    MALADE     = TRUE;

  if (ChaineEstDans (msg, "stp") ||
      ChaineEstDans (msg, "s'il-te-plait") ||
      ChaineEstDans (msg, "s'il te plait") ||
      ChaineEstDans (msg, "s il te plait") ||
      ChaineEstDans (msg, "svp") ||
      ChaineEstDans (msg, "s'il-vous-plait") ||
      ChaineEstDans (msg, "s'il vous plait") ||
      ChaineEstDans (msg, "s il vous plait") ||
/*       ChaineEstDans (msg, "s'il") && ChaineEstDans (msg, "plaît") || */
      ChaineEstDans (msg, "please"))
    STP        = TRUE;

  if (ChaineEstDans (msg, "quel age") ||
      ChaineEstDans (msg, "kel age"))
    AGE        = TRUE;

  if (ChaineEstDans (msg, ":)")
      || ChaineEstDans (msg, ":-)")
      || ChaineEstDans (msg, ":D")
      || ChaineEstDans (msg, ":-D")
      || ChaineEstDans (msg, ":=)")
      || ChaineEstDans (msg, ":=D")
      || ChaineEstDans (msg, "^_^"))
    SOURIRE    = TRUE;

  if (ChaineEstDans (msg, "arf") && !ChaineEstDans (msg, "parfait") && !ChaineEstDans (msg, "parfois") ||
      ChaineEstDans (msg, "ouah") ||
      ChaineEstDans (msg, "ouaf") ||
      ChaineEstDans (msg, "woof") ||
      ChaineEstDans (msg, "wouf") ||
      ChaineEstDans (msg, "waf") ||
      ChaineEstDans (msg, "bow wow") || /* en anglais */
      ChaineEstDans (msg, "grr"))
    MOUARF     = TRUE;

  if (MOUARF && (ChaineEstDans (currentbot->botname, "arf") && !ChaineEstDans (currentbot->botname, "parfait") && !ChaineEstDans (currentbot->botname, "parfois")) ||
      ChaineEstDans (currentbot->botname, "ouah") ||
      ChaineEstDans (currentbot->botname, "ouaf") ||
      ChaineEstDans (currentbot->botname, "woof") ||
      ChaineEstDans (currentbot->botname, "wouf") ||
      ChaineEstDans (currentbot->botname, "waf") ||
      ChaineEstDans (currentbot->botname, "grr"))
      MOUARF = FALSE;

  if (ChaineEstDans (msg, ";)") ||
      ChaineEstDans (msg, ";D") ||
      ChaineEstDans (msg, ";-)") ||
      ChaineEstDans (msg, ";-D") ||
      ChaineEstDans (msg, "-_^") ||
      ChaineEstDans (msg, "^_-")
      )
    CLINDOEIL  = TRUE;

  if (ChaineEstDans (msg, "et la mer a la ronde roule son bruit de cranes sur les greves"))
    EXTRAITSTJOHNPERSE = TRUE;

  if (ChaineEstDans (msg, "Saint John Perse"))
    STJOHNPERSE = TRUE;

  if (ChaineEstDans (msg, " la?") ||
      ChaineEstDans (msg, "ici") ||
/*       ChaineEstDans (msg, " la ") || */
      ChaineEstDans (msg, " la,") ||
/*       ChaineEstDans (msg, "là") && !ChaineEstDans (msg, "voilà") || */
      ChaineEstDans (msg, "present") ||
      ChaineEstDans (msg, "avec nous"))
    PRESENT     = TRUE;

  if (/* ChaineEstDans (msg, "@>---,--`--") ||
      ChaineEstDans (msg, "@>---,---`--") ||
      ChaineEstDans (msg, "@>--,---`--") ||
      ChaineEstDans (msg, "@}--`--,--") */
      ChaineEstDans (msg, "@>--") ||
      ChaineEstDans (msg, "@}--") ||
      ChaineEstDans (msg, "--<@") ||
      ChaineEstDans (msg, "--{@") ||
      ChaineEstDans (msg, "@-") ||
      ChaineEstDans (msg, "-@"))
    ROSE        = TRUE;

  if (ChaineEstDans (msg, " mort"))
    MORT        = TRUE;

  if (ChaineEstDans (msg, "indiii") ||
      ChaineEstDans (msg, "lililililili") ||
      ChaineEstDans (msg, "bisous bisous bisous"))
    INDILILI    = TRUE;

  if (ChaineEstDans (msg, "et toi") ||
      ChaineEstDans (msg, "toi aussi"))
    ETTOI       = TRUE;

  if (ChaineEstDans (msg, "je suis") ||
      ChaineEstDans (msg, "j'suis ") ||
      ChaineEstDans (msg, "chuis ") ||
      ChaineEstDans (msg, "suis-je") ||
      ChaineEstDans (msg, "suis je") ||
      ChaineEstDans (msg, "moi "))
    JESUIS      = TRUE;

  if (ChaineEstDans (msg, "es-tu") ||
      ChaineEstDans (msg, "as-tu") ||
      ChaineEstDans (msg, "est-ce")||
      ChaineEstDans (msg, "est-il") ||
      ChaineEstDans (msg, "comment") ||
      ChaineEstDans (msg, "pourquoi") ||
      ChaineEstDans (msg, "suis-je") ||
      ChaineEstDans (msg, "ais-tu") ||
      ChaineEstDans (msg, "ait-il") ||
      ChaineEstDans (msg, "ez-vous") ||
      ChaineEstDans (msg, "?"))
    QUESTION    = TRUE;

  if (ChaineEstDans (msg, "as-tu ")
      || ChaineEstDans (msg, " as-tu")
      || ChaineEstDans (msg, "as tu ")
      || ChaineEstDans (msg, " as tu ")
      || ChaineEstDans (msg, "t'as")
      || ChaineEstDans (msg, "t as")
      || ChaineEstDans (msg, "tu as") ||
      TU_AS && QUESTION)
    AS_TU      = TRUE;

  if (ChaineEstDans (msg, "fou") && !ChaineEstDans (msg, "four") ||
      ChaineEstDans (msg, "tare") ||
/*       ChaineEstDans (msg, "taré") || */
      ChaineEstDans (msg, "cogne") ||
/*       ChaineEstDans (msg, "cogné") || */
      ChaineEstDans (msg, "schtarbe") ||
/*       ChaineEstDans (msg, "schtarbé") || */
      ChaineEstDans (msg, "toque") ||
/*       ChaineEstDans (msg, "toqué") || */
      ChaineEstDans (msg, "cingle") ||
/*       ChaineEstDans (msg, "cinglé") || */
      ChaineEstDans (msg, "dingue") ||
      ChaineEstDans (msg, "secoue") ||
      ChaineEstDans (msg, "s'coue") ||
/*       ChaineEstDans (msg, "secoué") || */
/*       ChaineEstDans (msg, "s'coué") || */
      ChaineEstDans (msg, "maboul") ||
      ChaineEstDans (msg, "toc toc") ||
      ChaineEstDans (msg, "psychopathe") ||
      ChaineEstDans (msg, "psychopate") ||
      ChaineEstDans (msg, "psychopatte"))
    FOU          = TRUE;

  if (ChaineEstDans (msg, "musique") ||
      ChaineEstDans (msg, " chant") ||
      ChaineEstDans (msg, "ploum ploum") ||
      ChaineEstDans (msg, "pom pom pom"))
    MUSIQUE      = TRUE;

  if (ChaineEstDans (msg, "ecoute"))
    ECOUTER = TRUE;

  if (ChaineEstDans (msg, "copine") ||
      ChaineEstDans (msg, "fiancee") ||
      ChaineEstDans (msg, "petite amie"))
    COPINE       = TRUE;

  if (ChaineEstDans (msg, "pirate") ||
      ChaineEstDans (msg, "hacker"))
    PIRATE       = TRUE;

  if (ChaineEstDans (msg, "warez"))
    WAREZ        = TRUE;

  if (ChaineEstDans (msg, "bienvenu") ||
      ChaineEstDans (msg, "bon retour") ||
      ChaineEstDans (msg, "content de te revoir") ||
      ChaineEstDans (msg, "welcome") ||
      ChaineEstDans (msg, "valkommen") /* en suedois */)
    BIENVENUE    = TRUE;

  if (ChaineEstDans (msg, "grand") ||
      ChaineEstDans (msg, "geant"))
    GRAND        = TRUE;

  if (ChaineEstDans (msg, "petit") ||
      ChaineEstDans (msg, "nain"))
    PETIT        = TRUE;

  if (ChaineEstDans (msg, "!say ") ||
      !ischannel (to) && ChaineEstDans (msg, "say"))
    FONCTION_SAY = TRUE;

  if (ChaineEstDans (msg, "!fuck ") ||
      !ischannel (to) && ChaineEstDans (msg, "fuck"))
    FONCTION_FUCK = TRUE;

  if (ChaineEstDans (msg, "!apprends") ||
      !ischannel (to) && ChaineEstDans (msg, "apprends"))
    FONCTION_APPRENDS = TRUE;

  if (FONCTION_APPRENDS || FONCTION_FUCK || FONCTION_SAY ||
      ChaineEstDans (msg, "!active") ||
      !ischannel (to) && ChaineEstDans (msg, "active") ||
      ChaineEstDans (msg, "!desactive") ||
      ChaineEstDans (msg, "!join") ||
      !ischannel (to) && ChaineEstDans (msg, "join"))
    FONCTION = TRUE;

  if (ChaineEstDans (msg, "libre") ||
      (ChaineEstDans (msg, "disponible") && !ChaineEstDans (msg, "indisponible")))
    LIBRE        = TRUE;
  
  if (ChaineEstDans (msg, "occupe"))
    OCCUPE       = TRUE;

  if (ChaineEstDans (msg, "ce soir") ||
      ChaineEstDans (msg, "cette nuit") ||
      ChaineEstDans (msg, "tonight"))
    CE_SOIR      = TRUE;

  if (ChaineEstDans (msg, "aimes-tu ") ||
      ChaineEstDans (msg, "aimes tu ") ||
      ChaineEstDans (msg, "tu aimes") ||
      ChaineEstDans (msg, "aimez-vous ") ||
      ChaineEstDans (msg, "aimez-vous") ||
      ChaineEstDans (msg, "t'aimes "))
    AIMES_TU     = TRUE;
  
  if (ChaineEstDans (msg, "m'aimes-tu") ||
      ChaineEstDans (msg, "m aimes tu ") ||
      ChaineEstDans (msg, "m'aimes tu ") ||
      ChaineEstDans (msg, "tu m'aimes") ||
      ChaineEstDans (msg, "m'aimez-vous "))
    M_AIMES_TU   = TRUE;

  if (ChaineEstDans (msg, "connais-tu") ||
      ChaineEstDans (msg, "connais tu ") ||
      ChaineEstDans (msg, "tu connais") ||
      ChaineEstDans (msg, "connaissez-vous ") ||
      ChaineEstDans (msg, "connaissez vous ") ||
      ChaineEstDans (msg, "connaitrais-tu ") ||
      ChaineEstDans (msg, "connaitrais tu ") ||
      ChaineEstDans (msg, "apprecies-tu ") ||
      ChaineEstDans (msg, "apprecies tu "))
    CONNAIS_TU     = TRUE;

  if (ChaineEstDans (msg, "neige") ||
      ChaineEstDans (msg, "flocon"))
    NEIGE          = TRUE;

  if (ChaineEstDans (msg, "qu'est-ce qu") ||
      ChaineEstDans (msg, "c'est quoi ") ||
      ChaineEstDans (msg, "c est quoi ") ||
      ChaineEstDans (msg, "c quoi") ||
      ChaineEstDans (msg, "c koa") ||
      ChaineEstDans (msg, "qu'est ce qu") ||
      ChaineEstDans (msg, "qu est ce qu") ||
      ChaineEstDans (msg, "kesako"))
    QUEST_CE       = TRUE;

  if (ChaineEstDans (msg, "habite") ||
      ChaineEstDans (msg, "creche") ||
      ChaineEstDans (msg, "loge") ||
      ChaineEstDans (msg, "reside") ||
      ChaineEstDans (msg, "vis chez") ||
      ChaineEstDans (msg, "es-tu de ") ||
      ChaineEstDans (msg, "es tu de ") ||
      ChaineEstDans (msg, "es-tu en ") ||
      ChaineEstDans (msg, "es tu en ") ||
      ChaineEstDans (msg, "reste tu") || /* Quebequois */
      ChaineEstDans (msg, "restes tu") ||
      ChaineEstDans (msg, "restes-tu"))
    HABITE = TRUE;

  if (ChaineEstDans (msg, "france"))
    FRANCE = TRUE;

  if (ChaineEstDans (msg, "c'est") ||
      ChaineEstDans (msg, "c est"))
    C_EST = TRUE;

  if (ChaineEstDans (msg, "loria"))
    LORIA = TRUE;

  if (ChaineEstDans (msg, "talon"))
    TALON = TRUE;

  if (ChaineEstDans (msg, "ooooooooorrrrrrrr") ||
      ChaineEstDans (msg, "roooooote") ||
      ChaineEstDans (msg, "BOAAAARP"))
    ROT = TRUE;

  if (ChaineEstDans (msg, "pourquoi") ||
      ChaineEstDans (msg, "pourkoi") ||
      ChaineEstDans (msg, "pkoi") ||
      ChaineEstDans (msg, "pq ") ||
      ChaineEstDans (msg, "why") ||
      ChaineEstDans (msg, "warum"))
    POURQUOI = TRUE;
  
  if (ChaineEstDans (msg, "tu crois") ||
      ChaineEstDans (msg, "crois tu") ||
      ChaineEstDans (msg, "crois-tu"))
    CROIS_TU = TRUE;

  if ((ChaineEstDans (msg, "sais-tu") ||
       ChaineEstDans (msg, "sais tu")) && !ChaineEstDans (msg, "ssais") ||
      (ChaineEstDans (msg, "tu sais") ||
       ChaineEstDans (msg, "t'sais")) && !ChaineEstDans (msg, "saisi"))
    SAIS_TU = TRUE;

  if (ChaineEstDans (msg, "fais-tu") ||
      ChaineEstDans (msg, "fais tu") ||
      ChaineEstDans (msg, "tu fais") ||
      ChaineEstDans (msg, "sais-tu faire") ||
      ChaineEstDans (msg, "tu sais faire")) {
    FAIS_TU = TRUE;
    SAIS_TU = FALSE;
  }

  if (ChaineEstDans (msg, "cafe") ||
      ChaineEstDans (msg, "kawa"))
    CAFE = TRUE;

  if (ChaineEstDans (msg, "repondre") ||
      ChaineEstDans (msg, "reponds") ||
      ChaineEstDans (msg, "reponse"))
    REPONDS = TRUE;

  if (ChaineEstDans (msg, "sourd") ||
      ChaineEstDans (msg, "entends rien"))
    SOURD = TRUE;

  if (ChaineEstDans (msg, "rien") &&
      (ChaineEstDans (msg, "compri") ||
       ChaineEstDans (msg, "comprend") ||
       ChaineEstDans (msg, "pige")))
    RIENCOMPRIS = TRUE;

  if (ChaineEstDans (msg, "fatigue") ||
      ChaineEstDans (msg, "creve") ||
      ChaineEstDans (msg, "vanne") ||
      ChaineEstDans (msg, "a plat") ||
      ChaineEstDans (msg, "dans l'gaz") ||
      ChaineEstDans (msg, "ereinte") ||
      ChaineEstDans (msg, "extenue"))
    FATIGUE = TRUE;

  if (ChaineEstDans (msg, "je t'aime") ||
      ChaineEstDans (msg, "je t aime") ||
      ChaineEstDans (msg, "j't'aime") ||
      ChaineEstDans (msg, "je vous aime") ||
      ChaineEstDans (msg, "je vous adore") ||
      ChaineEstDans (msg, "je t'adore") ||
      ChaineEstDans (msg, "je t adore") ||
      ChaineEstDans (msg, "j't'adore"))
    JE_T_AIME = TRUE;

  if (ChaineEstDans (msg, "a tes souhaits") ||
/*       ChaineEstDans (msg, "À tes souhaits") || */
/*       ChaineEstDans (msg, "à tes amours") || */
      ChaineEstDans (msg, "a tes amours")
/*       ChaineEstDans (msg, "À tes amours") || */
/*       ChaineEstDans (msg, "à tes amours") */)
    SOUHAIT = TRUE;

  if ((ChaineEstDans (msg, "veux") && !ChaineEstDans (msg, "cheveux") ||
      ChaineEstDans (msg, "ordonne")) &&
      MOI
      )
    VEUX = TRUE;

  if (ChaineEstDans (msg, "crin") ||
      ChaineEstDans (msg, "Centre de Recherche en Informatique de Nancy"))
    CRIN = TRUE;
  
  if (ChaineEstDans (msg, "arabe") ||
      ChaineEstDans (msg, "beur") ||
      ChaineEstDans (msg, "negr") ||
      ChaineEstDans (msg, "noirs") ||
      ChaineEstDans (msg, "bougnoul") ||
      ChaineEstDans (msg, "niakoue") ||
      ChaineEstDans (msg, "bico"))
    ETRANGERS = TRUE;

  if (ChaineEstDans (msg, "qui ") ||
      ChaineEstDans (msg, "qui?") ||
      ChaineEstDans (msg, "qui,"))
    QUI = TRUE;

  if (ChaineEstDans (msg, "tu fais") ||
      ChaineEstDans (msg, "fais-tu") ||
      ChaineEstDans (msg, "fais tu"))
    TU_FAIS = TRUE;

  if (ChaineEstDans (msg, "quoi") && !ChaineEstDans (msg,"pourquoi"))
    QUOI = TRUE;

  if (ChaineEstDans (msg, "vive") ||
      ChaineEstDans (msg, "viva") ||
      ChaineEstDans (msg, "rules") ||
      ChaineEstDans (msg, "rulez") ||
      ChaineEstDans (msg, "rulz"))
    VIVE = TRUE;

  if (ChaineEstDans (msg, "amiga") ||
      ChaineEstDans (msg, "68000") ||
      ChaineEstDans (msg, "motorola"))
    AMIGA = TRUE;

  if (ChaineEstDans (msg, "intel") && !ChaineEstDans (msg, "intelligent") ||
      ChaineEstDans (msg, "Windows") ||
      ChaineEstDans (msg, "Microsoft")
      )
    WINTEL = TRUE;

  if (ChaineEstDans (msg, "front national") ||
      ChaineEstDans (msg, "f.n.") ||
      ChaineEstDans (msg, " fn") && !ChaineEstDans (msg, "fnac"))
    FRONT_NATIONAL = TRUE;

  if (ChaineEstDans (msg, "merde") && !ChaineEstDans (msg, "demerde") ||
      ChaineEstDans (msg, "shit") && !ChaineEstDans (msg, "!shit") && !ChaineEstDans (msg, "shitt") ||
      ChaineEstDans (msg, "couill") ||
      ChaineEstDans (msg, "connard") ||
      ChaineEstDans (msg, "ducon") ||
      ChaineEstDans (msg, "zob") ||
      ChaineEstDans (msg, "bite") && !ChaineEstDans (msg, "abite") ||
      ChaineEstDans (msg, "salaud") ||
      ChaineEstDans (msg, "salop") && !ChaineEstDans (msg, "salopette") ||
      ChaineEstDans (msg, "degueu") ||
      ChaineEstDans (msg, "blair") ||
      ChaineEstDans (msg, "pignouf") ||
      ChaineEstDans (msg, "enfoire") ||
/*       ChaineEstDans (msg, "enfoiré") || */
      ChaineEstDans (msg, "encule") ||
/*       ChaineEstDans (msg, "enculé") || */
      ChaineEstDans (msg, "pd") && !ChaineEstDans (msg, "pdt") ||
      ChaineEstDans (msg, " pd ") ||
      ChaineEstDans (msg, "fuck") ||
      ChaineEstDans (msg, "fuk") ||
      ChaineEstDans (msg, "branler") ||
      ChaineEstDans (msg, "putain") ||
      ChaineEstDans (msg, "pt1") ||
      ChaineEstDans (msg, "putin") ||
      ChaineEstDans (msg, "bordel") ||
      ChaineEstDans (msg, "chier") && !ChaineEstDans (msg, "ichier") ||
      ChaineEstDans (msg, "chiant") ||
      ChaineEstDans (msg, "gueule") ||
      ChaineEstDans (msg, "dick") ||
      ChaineEstDans (msg, "suck") ||
      ChaineEstDans (msg, "sux") ||
      ChaineEstDans (msg, "tu suce") ||
      ChaineEstDans (msg, "bitch") ||
      ChaineEstDans (msg, "bougnoul") ||
      ChaineEstDans (msg, " de daube") ||
      ChaineEstDans (msg, "leche-cul") ||
/*       ChaineEstDans (msg, "lèche-cul") || */
      ChaineEstDans (msg, "leche cul") ||
/*       ChaineEstDans (msg, "lèche cul") || */
      ChaineEstDans (msg, "culs") && !ChaineEstDans (msg, "calcul") ||
      ChaineEstDans (msg, "mon cul") ||
      ChaineEstDans (msg, "trou du c") ||
      ChaineEstDans (msg, "trou duc") ||
      ChaineEstDans (msg, "trouduc") ||
      ChaineEstDans (msg, "asshole") ||
      ChaineEstDans (msg, "foutre") ||
      ChaineEstDans (msg, "niqu") && !ChaineEstDans (msg, "pique") ||
      ChaineEstDans (msg, "pisse") ||
      ChaineEstDans (msg, "bougnoul") ||
      ChaineEstDans (msg, "niakoue") ||
      ChaineEstDans (msg, "bico")
      )
    GROS_MOT = TRUE;

  if (ChaineEstDans (msg, "pour") && !ChaineEstDans (msg, "pourceau") && !ChaineEstDans (msg, "pourri"))
    POUR = TRUE;
  
  if (ChaineEstDans (msg, "contre"))
    CONTRE = TRUE;

  if (ChaineEstDans (msg, "boude") ||
      ChaineEstDans (msg, "fais la gueule") ||
      ChaineEstDans (msg, "fait la gueule") ||
      ChaineEstDans (msg, "fais la tete") ||
      ChaineEstDans (msg, "fait la tete"))
    TU_BOUDES = TRUE;

  if ((ChaineEstDans (msg, "liste") ||
      ChaineEstDans (msg, "montre") ||
      ChaineEstDans (msg, "donne"))
      && ChaineEstDans (msg, "stimuli"))
    LISTE_STIMULI = TRUE;

  if (ChaineEstDans (msg, "teste stimuli") ||
      ChaineEstDans (msg, "test stimuli") ||
      ChaineEstDans (msg, "tester stimuli"))
    TESTE_STIMULI = TRUE;
  
  if ((ChaineEstDans (msg, "liste") ||
      ChaineEstDans (msg, "montre") ||
      ChaineEstDans (msg, "donne"))
      && ChaineEstDans (msg, "reponse"))
    LISTE_REPONSES = TRUE;

  if (ChaineEstDans (msg, "sacre"))
    SACRE = TRUE;

  if (ChaineEstDans (msg, "jaloux"))
    JALOUX = TRUE;

  if (ChaineEstDans (msg, "qui est") ||
      ChaineEstDans (msg, "Qui c'est") ||
      ChaineEstDans (msg, "C'est qui") ||
      ChaineEstDans (msg, "c qui") ||
      ChaineEstDans (msg, "c ki") ||
      ChaineEstDans (msg, "c'ki"))
    QUI_EST = TRUE;

  if (ChaineEstDans (msg, "ton maitre") ||
/*       ChaineEstDans (msg, "ton maître") || */
      ChaineEstDans (msg, "ton seigneur") ||
      ChaineEstDans (msg, "ton chef"))
    TON_MAITRE = TRUE;

  if (ChaineEstDans (msg, "raison"))
    RAISON = TRUE;
  
  if (ChaineEstDans (msg, "kicker") ||
      ChaineEstDans (msg, "kiker"))
    KICKER = TRUE;

  if (ChaineEstDans (msg, "kicke moi") ||
      ChaineEstDans (msg, "tu me vire") ||
      ChaineEstDans (msg, "tu me gicle"))
    KICKE_MOI = TRUE;

  if (ChaineEstDans (msg, "ciseau") ||
      ChaineEstDans (msg, "cisaille"))
    CISEAUX = TRUE;

  if (ChaineEstDans (msg, "papier") ||
      ChaineEstDans (msg, "feuille"))
    PAPIER = TRUE;

  if (ChaineEstDans (msg, "caillou") ||
      ChaineEstDans (msg, "pierre") ||
      ChaineEstDans (msg, "rocher"))
    CAILLOU = TRUE;

  if (PAPIER && CAILLOU && CISEAUX)
    PIERRE_PAPIER_CISEAUX = TRUE;

  if (PIERRE_PAPIER_CISEAUX) {
    PAPIER = FALSE;
    CAILLOU = FALSE;
    CISEAUX = FALSE;
  }

  if (ChaineEstDans (msg, "tricheur") ||
      ChaineEstDans (msg, "tu triche") ||
      ChaineEstDans (msg, "tu as trich") ||
      ChaineEstDans (msg, "tu a trich") ||
      ChaineEstDans (msg, "t'as trich") ||
      ChaineEstDans (msg, "t'a trich"))
    TRICHEUR = TRUE;

  if (ChaineEstDans (msg, "mauvais joueur"))
    MAUVAIS_JOUEUR = TRUE;

  if (ChaineEstDans (msg, ":~(") ||
      ChaineEstDans (msg, ":'(") ||
      ChaineEstDans (msg, ":'C") ||
      ChaineEstDans (msg, ":'O") ||
      ChaineEstDans (msg, ":_(") ||
      ChaineEstDans (msg, " ouin ") ||
      ChaineEstDans (msg, " ouinnnn"))
    PLEURER = TRUE;

  if (ChaineEstDans (msg, "modeste"))
    MODESTE = TRUE;

  if (ChaineEstDans (msg, "content"))
    CONTENT = TRUE;
  
  if (ChaineEstDans (msg, "je te deteste") ||
	 ChaineEstDans (msg, "je te hais"))
	 JE_TE_DETESTE = TRUE;

  if(ChaineEstDans (msg, "achille"))
	  ACHILLE = TRUE;
  
  if (NEGATION) {
    if (LIBRE) {
      LIBRE = FALSE;
      if (!OCCUPE)
	OCCUPE = TRUE;
    } else if (OCCUPE) {
      OCCUPE = FALSE;
      if (!LIBRE)
	LIBRE = TRUE;
    }

    if (JE_T_AIME) {
      JE_T_AIME = FALSE;
    }

    if (ABOIRE) {
      ABOIRE = FALSE;
    }
    if (ENERVEMENT) {
      ENERVEMENT = FALSE;
    }
    if (AUREVOIR)
      AUREVOIR = FALSE;
    if (MERCI)
      MERCI = FALSE;
    if (INJURE) {
      INJURE = FALSE;
      COMPLIMENT = TRUE;
    }
    else if (COMPLIMENT) {
      COMPLIMENT = FALSE;
      INJURE     = TRUE;
    }

    if (TRICHEUR) {
      TRICHEUR = FALSE;
      COMPLIMENT = TRUE;
    }

    if (MAUVAIS_JOUEUR) {
      MAUVAIS_JOUEUR = FALSE;
      COMPLIMENT = TRUE;
    }

    if (PARLER) {
      if (FRENCH) {
	ENGLISH = TRUE;
	FRENCH = FALSE;
      }
      else if (ENGLISH) {
	FRENCH  = TRUE;
	ENGLISH = FALSE;
      }
    }
    if (TOUTSEUL) {
      TOUTSEUL = FALSE;
      QUELQUUN = TRUE;
    }
    if (ENERVEMENT)
      ENERVEMENT = FALSE;
    if (BISOUS)
      BISOUS = FALSE;
    if (DACCORD)
      DACCORD = FALSE;
    if (EXCUSE)
      EXCUSE = FALSE;
    if (AMI)
      AMI = FALSE;
    if (GRAND) {
      GRAND = FALSE;
      PETIT = TRUE;
    } else if (PETIT) {
      GRAND = TRUE;
      PETIT = FALSE;
    }
    if (CALME) {
      CALME = FALSE;
    }
    if (SALUT) {
      SALUT = FALSE;
    }

    if (POUR) {
      CONTRE = !CONTRE;
      POUR = FALSE;
    } else if (CONTRE) {
      CONTRE = FALSE;
      POUR = !POUR;
    }

    if (MODESTE) {
      MODESTE = FALSE;
    }

    if (CONTENT) {
      CONTENT = FALSE;
    }
  }
  
  if (INJURE &&
      (ChaineEstDans (msg, "il fait")) ||
      ChaineEstDans (msg, " temps"))
    INJURE = FALSE;

  if (COMPLIMENT &&
      (ChaineEstDans (msg, "il fait")) ||
      ChaineEstDans (msg, " temps"))
    COMPLIMENT = FALSE;

  LANGUE = FRENCH || ENGLISH || SWEDISH;

  if (FONCTION) {
    AncienneAutorisation = (Channel_to?Channel_to->talk:TRUE);
    if (Channel_to) Channel_to->talk = FALSE;
  }

  if ((Channel_to?Channel_to->talk:TRUE)) {

    Locuteur = LocuteurExiste (currentbot->lists->ListeLocuteurs, from);
    if (!Locuteur)
      Locuteur = AjouteLocuteur (currentbot->lists->ListeLocuteurs,
				 NUS);
      

    for (i=0; i<TailleStim; i++)
      if (ChaineEstDans (msg, TableDesStimuli[i]->Stimulus))
	TableDesStimuli[i]->Present = TRUE;
      else
	TableDesStimuli[i]->Present = FALSE;

    NbRep = 0;
    
    for (i=0; i<TailleRep; i++) {

      if (TableDesReponses[i]->Active
	  && (STRCASEEQUAL (to, TableDesReponses[i]->Canal) ||
	      !strcmp(TableDesReponses[i]->Canal, "#*")))
	for (j=0; j<TailleStim; j++)
	  if (TableDesStimuli[j]->Present &&
	      TableDesStimuli[j]->Actif &&
	      strcmp (TableDesStimuli[j]->NomStimulus,
		      TableDesReponses[i]->NomStimulus)==0) {
	    NbRep++;
	  }
    }

    if (NbRep) {
      Reponse  = malloc (NbRep*sizeof (char *));
      Reponse2 = malloc (NbRep*sizeof (char *));
    } else {
      Reponse  = 0;
      Reponse2 = 0;
    }

    Num = 0;
    
    for (i=0; i<TailleRep; i++)
      for (j=0; j<TailleStim; j++)
	if (TableDesStimuli[j]->Present &&
	    TableDesStimuli[j]->Actif &&
	    TableDesReponses[i]->Active &&
	    (STRCASEEQUAL (to, TableDesReponses[i]->Canal) ||
	     !strcmp (TableDesReponses[i]->Canal, "#*")) &&
	    strcmp (TableDesStimuli[j]->NomStimulus,
		    TableDesReponses[i]->NomStimulus)==0) {
	  Reponse[Num]  = strdup(TableDesReponses[i]->Reponse);
	  Reponse2[Num] = strdup(TableDesReponses[i]->Reponse);
	  Num ++;
	}

    if (Num)
      Repondre (from, to, 0, Num, Reponse, 0, Num, Reponse2);
    

    if (STP) {
      Repondre (from, to, +2, 0, 0, +3, 0, 0);
    }

    Locuteur = LocuteurExiste (currentbot->lists->ListeLocuteurs, from);
    if (!Locuteur)
      Locuteur = AjouteLocuteur (currentbot->lists->ListeLocuteurs,
				 NUS);

    if (BONJOUR && (NOM || !ischannel (to)) && Locuteur->Bonjours <=0) {
      Reponse = malloc (8 * sizeof (char *));
      Reponse[0] = strdup ("Bonjour %s.");
      Reponse[1] = strdup ("Bonjour à toi %s.");
      Reponse[2] = strdup ("Je te souhaite le bonjour, %s.");
      Reponse[3] = strdup ("Bien le bonjour, %s.");
      Reponse[4] = strdup ("Bonne journée à toi aussi, %s.");
      Reponse[5] = strdup ("Bonjour à toi aussi, %s.");
      Reponse[6] = strdup ("Bonjour, comment vas-tu aujourd'hui, %s?");
      Reponse[7] = strdup ("/me enlève son chapeau devant %s.");

      Reponse2 = malloc (6 * sizeof (char *));
      Reponse2[0] = strdup ("Au revoir, %s.");
      Reponse2[1] = strdup ("%s: Pfff...");
      Reponse2[2] = 0;
      Reponse2[3] = strdup ("Tu ne le penses même pas, %s!");
      Reponse2[4] = strdup ("Fais pas chier, %s!");
      Reponse2[5] = strdup ("/me tourne le dos à %s.");

      Locuteur->Bonjours = 1;
      Repondre (from, to, +1, 8, Reponse, +1, 6, Reponse2);

      Locuteur = LocuteurExiste (currentbot->lists->ListeLocuteurs, from);
      if (!Locuteur)
	Locuteur = AjouteLocuteur (currentbot->lists->ListeLocuteurs,
				   NUS);
    }
    
     Locuteur = LocuteurExiste (currentbot->lists->ListeLocuteurs, from);
    if (!Locuteur)
      Locuteur = AjouteLocuteur (currentbot->lists->ListeLocuteurs,
				 NUS);
    
    if (BONSOIR && Locuteur->Bonjours >= 0 && (NOM || !ischannel (to))) {
      Reponse = malloc (7 * sizeof (char *));
      Reponse[0] = strdup ("Bonsoir %s.");
      Reponse[1] = strdup ("Bonsoir à toi %s.");
      Reponse[2] = strdup ("Je te souhaite le bonsoir, %s.");
      Reponse[3] = strdup ("Bien le bonsoir, %s.");
      Reponse[4] = strdup ("Bonne soirée à toi aussi, %s.");
      Reponse[5] = strdup ("Bonsoir à toi aussi %s.");
      Reponse[6] = strdup ("/me souhaite le bonsoir à %s.");

      Reponse2 = malloc (9 * sizeof (char *));
      Reponse2[0] = strdup ("C'est ça, casse toi %s.");
      Reponse2[1] = strdup ("%s: Pfff...");
      Reponse2[2] = 0;
      Reponse2[3] = strdup ("Barre-toi, %s.");
      Reponse2[4] = strdup ("Bon débarras, %s!");
      Reponse2[5] = 0;
      Reponse2[6] = strdup ("Un casse-pieds de moins!");
      Reponse2[7] = strdup ("/me soupire d'aise: %s est parti.");
      Reponse2[8] = 0;

      Locuteur->Bonjours = -1;
      Repondre (from, to, +1, 7, Reponse, +1, 9, Reponse2);

      Locuteur = LocuteurExiste (currentbot->lists->ListeLocuteurs, from);
      if (!Locuteur)
	Locuteur = AjouteLocuteur (currentbot->lists->ListeLocuteurs,
				   NUS);
    }

    
    Locuteur = LocuteurExiste (currentbot->lists->ListeLocuteurs, from);
    if (!Locuteur)
      Locuteur = AjouteLocuteur (currentbot->lists->ListeLocuteurs,
				 NUS);
    
    if (Locuteur->Bonjours &&
	time2hours (time (NULL))-Locuteur->DernierContact > 12)
      Locuteur->Bonjours = 0;

    if (SALUT &&(NOM || !ischannel (to)) && Locuteur->Bonjours <= 1 && Locuteur->Bonjours >= 0) {
      Reponse = malloc (10 * sizeof (char *));
      Reponse[0] = strdup ("Salut %s!");
      Reponse[1] = strdup ("Je te salue, %s!");
      Reponse[2] = strdup ("Je te salue bien bas, %s!");
      Reponse[3] = strdup ("Je te rends un salut, %s (mais pas le tien ;)");
      Reponse[4] = strdup ("Salut %s.");
      Reponse[5] = strdup ("Salut à toi aussi, %s.");
      Reponse[6] = Locuteur->Bonjours > 0 ?
		strdup ("Au revoir, %s.") : strdup ("Bien le bonjour, %s.");
      Reponse[7] = strdup ("/me salue %s.");
      Reponse[8] = strdup ("/me serre la main de %s.");
      Reponse[9] = strdup ("Salut %s. :-)");

      Reponse2 = malloc (9 * sizeof (char *));
      Reponse2[0] = strdup ("J'te cause plus %s!");
      Reponse2[1] = strdup ("Je ne vous salue pas, %s!");
      Reponse2[2] = 0;
      Reponse2[3] = strdup ("Hé bien moi, je ne te salue pas, %s!");
      Reponse2[4] = strdup ("Barre-toi, %s.");
      Reponse2[5] = 0;
      Reponse2[6] = strdup ("Zut, un casse-pieds (%s)!");
      Reponse2[7] = strdup ("/me se tourne de manière à ne plus voir %s.");
      Reponse2[8] = 0;

      Repondre (from, to, +1, 10, Reponse, +1, 9, Reponse2);

      Locuteur = LocuteurExiste (currentbot->lists->ListeLocuteurs, from);
      if (!Locuteur)
		Locuteur = AjouteLocuteur (currentbot->lists->ListeLocuteurs,
				   NUS);

      if (Locuteur)

		if (Locuteur->Bonjours > 0)
		    Locuteur->Bonjours = /* -1 */ 0;
		else
		    Locuteur->Bonjours++;
	 
      else
		botlog (ERRFILE, "PROBLEM : No pointer to Locuteur (%s) in SALUT", from);

    }

    if (JE_RESTE && (NOM || !ischannel (to))) {
	   if (Locuteur)
		  if (Locuteur->Bonjours < 0) {
			 Reponse = malloc (9 * sizeof (char *));
			 Reponse[0] = strdup ("Excuse moi %s, j'avais pas compris.");
			 Reponse[1] = strdup ("%s: Oups. Je n'ai pas fait assez attention.");
			 Reponse[2] = strdup ("Tant mieux %s.");
			 Reponse[3] = strdup ("J'ai cru que tu partais, %s.");
			 Reponse[4] = strdup ("Je préfères que tu sois là, %s.");
			 Reponse[5] = strdup ("Je me sens moins seul quand tu es là, %s.");
			 Reponse[6] = strdup ("J'ai sans doute mal compris, %s.");
			 Reponse[7] = strdup ("J'espère que tu restes longtemps, %s.");
			 Reponse[8] = strdup ("Super, %s.");
		 
			 Reponse2 = malloc (3 * sizeof (char *));
			 Reponse2[0] = strdup ("%s: Zut!");
			 Reponse2[1] = strdup ("Tant pis, %s!");
			 Reponse2[2] = strdup ("Grmbl...");
			 
			 Repondre (from, to, +1, 9, Reponse, +1, 3, Reponse2);
		  }
    }
    
    
    if (CAVA && (NOM || !ischannel (to))) {
      Reponse = malloc (12 * sizeof (char *));
      Reponse[0] = strdup ("Ça va pas mal, %s, merci.");
      Reponse[1] = strdup ("Moui, ça va, %s, merci.");
      Reponse[2] = strdup ("On fait aller, %s.");
      Reponse[3] = strdup ("Ça peut aller, %s.");
      Reponse[4] = strdup ("Ça allait très mal, mais ça commence à aller mieux, %s, merci.");
      Reponse[5] = strdup ("Depuis que tu es là, %s, ça va mieux.");
      Reponse[6] = strdup ("Ça va mieux, merci %s.");
      Reponse[7] = strdup ("Ça va bien, et toi %s?");
      Reponse[8] = strdup ("Ça va très bien, %s.");
      Reponse[9] = strdup ("Tout est pour le mieux dans le meilleur des mondes, %s.");
      Reponse[10] = strdup ("Je suis sur un petit nuage, %s. :)");
      Reponse[11] = strdup ("J'ai la pêche, %s!");

      Reponse2 = malloc (8 * sizeof (char *));
      Reponse2[0] = strdup ("J'espère que je vais mieux que toi, %s!");
      Reponse2[1] = strdup ("Ça va déjà moins bien depuis que tu es là, %s!");
      Reponse2[2] = strdup ("Ça ira mieux quand tu seras parti, %s.");
      Reponse2[3] = strdup ("Bof, ça va pas fort, %s.");
      Reponse2[4] = strdup ("Ça va très mal, %s.");
      Reponse2[5] = strdup ("Je déprime depuis que tu es arrivé, %s...");
      Reponse2[6] = strdup ("J'ai la banane, pas comme toi, %s! :]");
      Reponse2[7] = strdup ("/me soupire.");

      Repondre (from, to, +1, 12, Reponse, +1, 8, Reponse2);
    }

    Locuteur = LocuteurExiste (currentbot->lists->ListeLocuteurs, from);
    if (!Locuteur)
      Locuteur = AjouteLocuteur (currentbot->lists->ListeLocuteurs,
				 NUS);

    if (AUREVOIR && Locuteur->Bonjours >= 0 && (NOM || !ischannel (to))) {
      Reponse = malloc (11 * sizeof (char *));
      Reponse[0] = strdup ("À bientôt %s.");
      Reponse[1] = strdup ("Au revoir %s.");
      Reponse[2] = strdup ("Au plaisir de te revoir %s.");
      Reponse[3] = strdup ("À plus tard %s.");
      Reponse[4] = strdup ("Reviens-nous bientôt, %s.");
      Reponse[5] = strdup ("À bientôt, %s.");
      Reponse[6] = strdup ("À tantôt, %s.");
      Reponse[7] = strdup ("À la revoyure, %s.");
      Reponse[8] = strdup ("Bye bye %s.");
      Reponse[9] = strdup ("Je ne te raccompagne pas, %s: tu connais le chemin. :)");
      Reponse[10] = strdup ("/me décroche le manteau de %s du porte-manteau et l'aide à l'enfiler.");

      Reponse2 = malloc (6 * sizeof (char *));
      Reponse2[0] = strdup ("En espérant ne plus te revoir, %s.");
      Reponse2[1] = strdup ("%s: Bon débarras!");
      Reponse2[2] = strdup ("Au plaisir de ne plus te revoir %s.");
      Reponse2[3] = strdup ("Débarrasse le plancher, %s!");
      Reponse2[4] = strdup ("Du balai, %s!");
      Reponse2[5] = strdup ("/me attend que %s soit sorti avant de claquer la porte.");

      Locuteur->Bonjours = -1;
      Repondre (from, to, +1, 11, Reponse, +1, 6, Reponse2);

      Locuteur = LocuteurExiste (currentbot->lists->ListeLocuteurs, from);
      if (!Locuteur)
	Locuteur = AjouteLocuteur (currentbot->lists->ListeLocuteurs,
				   NUS);
    }
    
    
/*     if (INJURE && !COMPLIMENT && !ETTOI && !JESUIS) { */

/*       if (ischannel (to) && NOM) { */
/* 	if (!exist_userhost (currentbot->lists->rellist, from)) */
/* 	  add_to_levellist (currentbot->lists->rellist, NickUserStr(from), 0); */

/* 	add_to_level (currentbot->lists->rellist, from, -1); */
/* 	Humeur = rellevel (from); */

/* 	if (Humeur < 0 && Humeur > -3) { */
/* 	  Numero = NumPhrase % 5; */
/* 	  switch (Numero) { */
/* 	  case 0: */
/* 	    sendprivmsg (to, "Attention, %s, je vais kicker! J'exige des excuses.", getnick(from)); */
/* 	    break; */
/* 	  case 1: */
/* 	    sendprivmsg (to, "Mortecouilles, %s, je m'en vais vous estropier! Montjoie! Saint-Denis!", getnick(from)); */
/* 	    break; */
/* 	  case 2: */
/* 	    sendprivmsg (to, "Attention, %s, je commence a etre serieusement enerve!", getnick(from)); */
/* 	    break; */
/* 	  case 3: */
/* 	    sendprivmsg (to, "Je te previens, %s, j'ai les pieds qui me demangent.", getnick(from)); */
/* 	    break; */
/* 	  case 4: */
/* 	    sendprivmsg (to, "Implore mon pardon, miserable %s!", getnick (from)); */
/* 	    break; */
/* 	  } */
/* 	} else if (Humeur >= 0) { */
/* 	  Numero = NumPhrase % 3; */
/* 	  switch (Numero) { */
/* 	  case 0: */
/* 	    sendprivmsg (to, "Toi-meme, %s!", getnick(from)); */
/* 	    break; */
/* 	  case 1: */
/* 	    sendprivmsg (to, "C'est celui qui le dit qui l'est, %s!", getnick(from)); */
/* 	    break; */
/* 	  case 2: */
/* 	    sendprivmsg (to, "Tu me provoques, %s?", getnick(from)); */
/* 	    break; */
/* 	  } */
/* 	} else { */
	/*Humeur negative <= -3  */
/* 	  sendprivmsg (to, "Rahh, %s j'ai des fourmis dans les pieds! B[", getnick(from)); */
/* 	  if (protlevel (from) < 50) { */
/* 	    Numero = NumPhrase % 7; */
/* 	    switch (Numero) { */
/* 	    case 0: */
/* 	      sendkick (to, getnick(from), "Je t'avais prevenu!"); */
/* 	      break; */
/* 	    case 1: */
/* 	      sendkick (to, getnick(from), "Je ne supporte pas qu'on me manque de respect!"); */
/* 	      break; */
/* 	    case 2: */
/* 	      sendkick (to, getnick(from), "Dehors, odieux personnage!"); */
/* 	      break; */
/* 	    case 3: */
/* 	      sendkick (to, getnick(from), "Rahh! Ne recommence plus jamais!"); */
/* 	      break; */
/* 	    case 4: */
/* 	      sendkick (to, getnick(from), "Ta mere en tongs au Prisunic. :p"); */
/* 	      break; */
/* 	    case 5: */
/* 	      sendkick (to, getnick (from), "Tiens, prends ca!"); */
/* 	      break; */
/* 	    case 6: */
/* 	      sendkick (to, getnick (from), "Moule a gaufres!"); */
/* 	      break; */
/* 	    } */
/* 	  } else { */
/* 	    Numero = NumPhrase % 5; */
/* 	    switch (Numero) { */
/* 	    case 0: */
/* 	      sendprivmsg (to, "C'est bien parce que je suis cense te proteger que je ne te kicke pas, %s!", getnick(from)); */
/* 	      break; */
/* 	    case 1: */
/* 	      sendprivmsg (to, "Tiens le toi pour dit, %s: quand tu ne seras plus protege, je ne me generai pas pour te kicker!",  getnick(from)); */
/* 	      break; */
/* 	    case 2: */
/* 	      sendprivmsg (to, "Sors d'ici %s! J'espere que je ne devrai plus te proteger.", getnick(from)); */
/* 	      break; */
/* 	    case 3: */
/* 	      sendprivmsg (to, "Je t'en prie, ne reste pas dans ce canal, %s!", getnick(from)); */
/* 	      break; */
/* 	    case 4: */
/* 	      sendprivmsg (to, "Va te faire voir chez les Grecs, %s!", getnick(from)); */
/* 	      break; */
/* 	    } */
/* 	  } */
/* 	} */
/*       } */
/*       else if (!ischannel (to)) { */
/* 	if (!exist_userhost (currentbot->lists->rellist, from)) */
/* 	  add_to_levellist (currentbot->lists->rellist, NickUserStr(from), 0); */

/* 	add_to_level (currentbot->lists->rellist, from, -2); */
/* 	Humeur = rellevel (from); */

/* 	if (Humeur < 0) */
/* 	  send_to_user (from, "Attention, je vais m'enerver! J'exige des excuses.", getnick(from)); */
/* 	else */
/* 	  send_to_user (from, "Arrete de me charrier. Je peux etre dangereux :]", getnick(from)); */
/*       } */
/*     } */

    if (INJURE && !COMPLIMENT && !ES_TU && !QUI_EST && !ETTOI && !JESUIS && !CLINDOEIL && !FONCTION_FUCK && (NOM || !ischannel (to))) {
      Reponse = malloc (10 * sizeof (char *));
      Reponse[0] = strdup ("Attention, %s, je vais kicker! J'exige des excuses.");
      Reponse[1] = strdup ("Implore mon pardon, misérable %s!");
      Reponse[2] = strdup ("Toi-même, %s!");
      Reponse[3] = strdup ("C'est celui qui le dit qui l'est, %s!");
      Reponse[4] = strdup ("Tu me provoques, %s?");
      Reponse[5] = strdup ("Diantre, %s! Je sens monter en moi la colère!");
      Reponse[6] = strdup ("Je te préviens, %s: la moutarde me monte au nez!");
      Reponse[7] = strdup ("Je sens que je m'énerve, %s.");
      Reponse[8] = strdup ("À genoux, %s!");
      Reponse[9] = strdup ("/me inspire un grand coup, puis expire lentement...");
      
      if (!exist_userhost (currentbot->lists->rellist, from)) {
	add_to_levellist (currentbot->lists->rellist, NickUserStr(from), 0);
	Nouveau = TRUE;
      } else
	Nouveau = FALSE;

      if (!Nouveau)
	add_to_level (currentbot->lists->rellist, from, -2);
      else
	add_to_level (currentbot->lists->rellist, from, 3);

      Reponse2 = malloc (25 * sizeof (char *));
      Reponse2[0] = strdup ("Mortecouilles, %s, je m'en vais vous estropier! Montjoie! Saint-Denis!");
      Reponse2[1] = strdup ("Demande pardon ou va rôtir en enfer!");
      Reponse2[2] = strdup ("Je ne supporte pas qu'on me manque de respect!");
      Reponse2[3] = strdup ("Dehors, odieux personnage!");
      Reponse2[4] = strdup ("Rahh! Ne recommence plus jamais!");
      Reponse2[5] = strdup ("Ta mère en tongs au Prisunic. :p");
      Reponse2[6] = strdup ("Tiens, prends ça!");
      Reponse2[7] = strdup ("Moule à gaufres!");
      Reponse2[8] = strdup ("Du balai!");
      Reponse2[9] = strdup ("Dégage, connard!");
      Reponse2[10] = strdup ("Va mourir, %s!");
      Reponse2[11] = strdup ("Pitécanthrope!");
      Reponse2[12] = strdup ("Australopithèque");
      Reponse2[13] = strdup ("Je veux te voir me supplier de te pardonner!");
      Reponse2[14] = strdup ("À genoux, vermisseau!");
      Reponse2[15] = strdup ("Nyctalope!");
      Reponse2[16] = strdup ("Recommence pour voir!");
      Reponse2[17] = strdup ("Implore mon pardon à genoux, je serai peut-être magnanime!");
      Reponse2[18] = strdup ("Anacoluthe!");
      Reponse2[19] = strdup ("Espèce de  bachibouzouk à cornes!");
      Reponse2[20] = strdup ("C'était marrant. Recommence qu'on voie ce qui se passe.");
      Reponse2[21] = strdup ("Ventre Saint-Gris!");
      Reponse2[22] = strdup ("Bretzel liquide!");
      Reponse2[23] = strdup ("Palsembleu!");
      Reponse2[24] = strdup ("Rentre chez ta mère, embryon!");

      KickerRepondre (from, to, -2, 10, Reponse, -6, 25, Reponse2);
    }
    
    if ((COMPLIMENT || VIVE) && !JESUIS && !QUESTION && !INJURE && !ETTOI && !AMIGA && !WINTEL && (NOM || !ischannel (to))) {
      Reponse = malloc (6 * sizeof (char *));
      Reponse[0] = strdup ("Merci %s.");
      Reponse[1] = strdup ("C'est trop, %s.");
      Reponse[2] = strdup ("C'est gentil, %s.");
      Reponse[3] = strdup ("Tu es trop bon, %s.");
      Reponse[4] = strdup ("Mais, toi aussi %s.");
      Reponse[5] = strdup ("/me remercie vivement %s.");

      Reponse2 = malloc (6 * sizeof (char *));
      Reponse2[0] = strdup ("T'es hypocrite, %s.");
      Reponse2[1] = strdup ("%s: Espèce de faux-jeton!");
      Reponse2[2] = strdup ("Tu le penses meme pas, %s.");
      Reponse2[3] = strdup ("Et tu penses que je vais croire que tu le penses, %s? :)");
      Reponse2[4] = strdup ("T'arrêtes de faire du lèche-bot, %s?");
      Reponse2[5] = strdup ("/me pense que %s n'est pas sincère.");

      Repondre (from, to, +2, 6, Reponse, +1, 6, Reponse2);
    }

    if (MODESTE && (NOM || !ischannel (to))) {
      Reponse = malloc (4 * sizeof (char *));
      Reponse[0] = strdup ("Je sais %s: je le suis le plus modeste du monde.");
      Reponse[1] = strdup ("Moi, %s? Modeste? J'oserais pas.");
      Reponse[2] = strdup ("À quoi tu vois ça, %s?");
      Reponse[3] = strdup ("/me ne crois pas qu'il soit modeste.");
      
      Reponse2 = malloc (3 * sizeof (char *));
      Reponse2[0] = strdup ("Plus que toi, en tout cas, %s.");
      Reponse2[1] = strdup ("/me est plus modeste que %s, ça c'est sûr!");
      Reponse2[2] = strdup ("Demain j'apprends la férocité, %s. :-]");

      Repondre (from, to, +1, 4, Reponse, +2, 3, Reponse2);
    }
    
    if (PARLER && ENGLISH && (NOM || !ischannel (to))) {
      Reponse = malloc (4 * sizeof (char *));
      Reponse[0] = strdup ("Sorry, %s, I don't speak English. Only French.");
      Reponse[1] = strdup ("Désolé, %s, Je ne parle pas anglais. Seulement français.");
      Reponse[2] = strdup ("Sorry, %s, I am the only bot speaking French.");
      Reponse[3] = strdup ("/me can't speak English. Only French.");
      
      Reponse2 = malloc (3 * sizeof (char *));
      Reponse2[0] = strdup ("I don't speak English, %s. Only french.");
      Reponse2[1] = strdup ("Je ne parle pas anglais, %s. Seulement français.");
      Reponse2[2] = strdup ("/me refuses to speak English.");

      Repondre (from, to, 0, 4, Reponse, 0, 3, Reponse2);
    }

    if (PARLER && FRENCH &&(NOM || !ischannel (to))) {
      Reponse = malloc (4 * sizeof (char *));
      Reponse[0] = strdup ("Bien sûr que je parle français, %s.");
      Reponse[1] = strdup ("Of course I speak french, %s.");
      Reponse[2] = strdup ("Je parle un peu français.");
      Reponse[3] = strdup ("/me parle français.");

      Reponse2 = malloc (3 * sizeof (char *));
      Reponse2[0] = strdup ("Évidemment que je parle français, %s.");
      Reponse2[1] = strdup ("Je ne parle pas anglais, %s. Seulement français.");
      Reponse2[2] = strdup ("/me n'a pas appris d'autre langue que le français.");
      Repondre (from, to, 0, 4, Reponse, 0, 3, Reponse2);
    }

    if (TU_ES && AMI && (NOM || !ischannel (to))) {
      Reponse = malloc (4 * sizeof (char *));
      Reponse[0] = strdup ("Toi aussi tu es mon ami, %s.");
      Reponse[1] = strdup ("J'espère bien, %s.");
      Reponse[2] = strdup ("Super! J'aime avoir des amis.");
      Reponse[3] = strdup ("/me est le copain de %s, et il apprécie...");

      Reponse2 = malloc (3 * sizeof (char *));
      Reponse2[0] = strdup ("Toi tu n'es pas mon ami, %s.");
      Reponse2[1] = strdup ("Peut-être que je t'apprécierai, quand tu seras moins ch*ant.");
      Reponse2[2] = strdup ("/me n'aime pas %s.");
      Repondre (from, to, +2, 4, Reponse, +1, 3, Reponse2);
    }

    if (JESUIS && AMI && (NOM || !ischannel (to))) {
      Reponse = malloc (4 * sizeof (char *));
      Reponse[0] = strdup ("Oui, %s.");
      Reponse[1] = strdup ("Tu es mon ami, %s.");
      Reponse[2] = strdup ("Tu es de mes amis, %s.");
      Reponse[3] = strdup ("/me est un copain de %s...");

      Reponse2 = malloc (3 * sizeof (char *));
      Reponse2[0] = strdup ("Toi tu n'es pas mon ami, %s.");
      Reponse2[1] = strdup ("Peut-être que je t'apprécierai, quand tu seras moins ch*ant.");
      Reponse2[2] = strdup ("/me n'aime pas %s.");
      Repondre (from, to, +2, 4, Reponse, +1, 3, Reponse2);
    }

    /*
	if (AS_TU && PHOTO && (NOM || !ischannel (to))) {
      Reponse = malloc (4 * sizeof (char *));
      Reponse[0] = strdup ("Oui %s. On peut me voir là: http://www.loria.fr/~parmenti/irc/");
      Reponse[1] = strdup ("Tu la veux, %s? Va voir www.loria.fr/~parmenti/irc/");
      Reponse[2] = strdup ("Je ne me suis fait prendre en photo qu'une seule fois. www.loria.fr/~parmenti/irc/");
      Reponse[3] = strdup ("/me peut être admiré là: www.loria.fr/~parmenti/irc/");

      Reponse2 = malloc (3 * sizeof (char *));
      Reponse2[0] = strdup ("Tu veux ma photo, %s? ");
      Reponse2[1] = strdup ("Va te faire voir, %s. Tu ne verras pas ma photo.");
      Reponse2[2] = strdup ("/me a une photo, mais il ne la donnera pas à %s.");
      Repondre (from, to, +1, 4, Reponse, +2, 3, Reponse2);
	}
	*/
    

    if (JE_TE_DETESTE &&(NOM || !ischannel (to))) {
      Reponse = malloc (5 * sizeof (char *));
      Reponse[0] = strdup ("Ben pas moi, %s.");
      Reponse[1] = strdup ("Je ne déteste pas encore moi, %s.");
      Reponse[2] = strdup ("%s: Ben pourquoi?");
      Reponse[3] = strdup ("Faut pas m'en vouloir, %s : je ne comprends pas toujours ce que tu dis, je ne suis qu'un bot, après tout.");
	 Reponse[4] = strdup ("J'ai dû mal interpréter ce que tu m'as dit %s. Prends bien garde à ne pas dire plus d'une phrase à la fois : je les mélange...");

      Reponse2 = malloc (3 * sizeof (char *));
      Reponse2[0] = strdup ("Ben moi auss, %s: je te déteste.");
      Reponse2[1] = strdup ("C'est réciproque, %s.");
      Reponse2[2] = strdup ("/me n'en a rien à faire...");
      Repondre (from, to, -1, 5, Reponse, -2, 3, Reponse2);
    }

    if (PARLER && SWEDISH && (NOM || !ischannel (to))) {
      Reponse = malloc (3 * sizeof (char *));
      Reponse[0] = strdup ("Désolé, %s. Je préfère parler français.");
      Reponse[1] = strdup ("Je ne sais pas parler suédois, %s. Parle-moi en français.");
      Reponse[2] = strdup ("J'essaye de parler un peu français, c'est déjà assez difficile, %s.");

      Reponse2 = malloc (2 * sizeof (char *));
      Reponse2[0] = strdup ("Je ne parle pas du tout suédois, %s.");
      Reponse2[1] = strdup ("Je ne vois pas pourquoi je parlerais suédois, %s.");
      Repondre (from, to, 0, 3, Reponse, 0, 2, Reponse2);
    }
    
    if (BEBE) {
      Reponse = malloc (24 * sizeof (char *));
      Reponse[0] = strdup ("Gouzi gouzi, %s. T'es un zoli bébé, hein?");
      Reponse[1] = 0;
      Reponse[2] = strdup ("Fais ton rot, %s. Ça ira mieux après.");
      Reponse[3] = 0;
      Reponse[4] = strdup ("Regardez moi ça, comme il est joli, le bébé %s.");
      Reponse[5] = 0;
      Reponse[6] = 0;
      Reponse[7] = strdup ("Comme il est mignon, le bébé %s.");
      Reponse[8] = 0;
      Reponse[9] = 0;
      Reponse[10] = strdup ("Comme tu ressembles à ta maman, %s!");
      Reponse[11] = 0;
      Reponse[12] = 0;
      Reponse[13] = strdup ("Oh %s! C'est son père tout craché!");
      Reponse[14] = 0;
      Reponse[15] = 0;
      Reponse[16] = strdup ("Quel zoOoli bébé!");
      Reponse[17] = 0;
      Reponse[18] = 0;
      Reponse[19] = strdup ("On dirait qu'il a les yeux de son grand-père le bébé %s.");
      Reponse[20] = 0;
      Reponse[21] = 0;
      Reponse[22] = strdup ("/me prend %s dans ses bras et le berce tendrement.");
      Reponse[23] = 0;

      Reponse2 = malloc (12 * sizeof (char *));
      Reponse2[0] = strdup ("Tiens! Un schtroumpf!");
      Reponse2[1] = 0;
      Reponse2[2] = 0;
      Reponse2[3] = strdup ("%s: Casse-toi mouflet!");
      Reponse2[4] = 0;
      Reponse2[5] = 0;
      Reponse2[6] = strdup ("Ah! Un Gremlin (%s)!");
      Reponse2[7] = 0;
      Reponse2[8] = 0;
      Reponse2[9] = strdup ("%s: Mouche ton nez et dis bonjour à la dame!");
      Reponse2[10] = 0;
      Reponse2[11] = 0;

      Repondre (from, to, +1, 24, Reponse, +1, 12, Reponse2);
    }

    if ((JESUIS || ILYA) && TOUTSEUL && (NOM || !ischannel (to))) {
      Reponse = malloc (6 * sizeof (char *));
      Reponse[0] = strdup ("Mais non, %s, nous ne sommes pas seuls, nous sommes au moins 2.");
      Reponse[1] = strdup ("Et moi alors, %s, je compte pour du beurre?");
      Reponse[2] = strdup ("Comptez vous! Je commence: 1");
      Reponse[3] = strdup ("C'est gentil pour moi, %s!");
      Reponse[4] = strdup ("Nous sommes tous seuls, %s.");
      Reponse[5] = strdup ("/me se demande s'il compte pour du beurre...");

      Reponse2 = malloc (3 * sizeof (char *));
      Reponse2[0] = strdup ("Ça m'étonne pas, %s!");
      Reponse2[1] = strdup ("%s: le jour où tu ne seras plus seul, il tombera de la m....!");
      Reponse2[2] = strdup ("Tu crois ça, %s? B]");

      Repondre (from, to, +1, 6, Reponse, +0, 3, Reponse2);
    }

    if (JESUIS && CONTENT && (NOM || !ischannel (to))) {
      Reponse = malloc (6 * sizeof (char *));
      Reponse[0] = strdup ("Tant mieux, %s, je ne demande que cela.");
      Reponse[1] = strdup ("Et moi aussi alors, %s.");
      Reponse[2] = strdup ("Bravo %s.");
      Reponse[3] = strdup ("Tant mieux pour toi, %s!");
      Reponse[4] = strdup ("Moi aussi, %s.");
      Reponse[5] = strdup ("/me est content pour %s, c'que c'est que l'empathie...");

      Reponse2 = malloc (3 * sizeof (char *));
      Reponse2[0] = strdup ("T'est toujours content de toi, %s?");
      Reponse2[1] = strdup ("%s: Tant pis!");
      Reponse2[2] = strdup ("Ça va pas durer %s! B]");

      Repondre (from, to, +1, 6, Reponse, +1, 3, Reponse2);
    }

    /* TODO : mettre à jour
    if ((HABITE || ES_TU) && NANCY  && !QUEL_ENDROIT && (NOM || !ischannel (to))) {
      Reponse = malloc (7 * sizeof (char *));
      Reponse[0] = strdup ("En quelque sorte oui, %s, je suis à Nancy.");
      Reponse[1] = strdup ("Oui %s, on peut dire que je réside à Nancy.");
      Reponse[2] = strdup ("Je suis même né à Nancy, %s.");
      Reponse[3] = strdup ("Nancy, c'est mon pays natal, %s.");
      Reponse[4] = strdup ("Même si je ne connais pas ses rues, je suis de Nancy, %s.");
      Reponse[5] = strdup ("Eh oui, %s, je vis à Nancy.");
      Reponse[6] = strdup ("/me se trouve à Nancy.");

      Reponse2 = malloc (3 * sizeof (char *));
      Reponse2[0] = strdup ("Toi tu n'es certainement pas de Nancy, %s!");
      Reponse2[1] = strdup ("%s: J'te d'mande si t'es de quelque part?");
      Reponse2[2] = strdup ("Et toi, %s, t'habites en prison?");

      Repondre (from, to, +1, 7, Reponse, +1, 3, Reponse2);
    }
	*/

    if (ENNUI && (NOM || !ischannel (to))) {
      Reponse = malloc (8 * sizeof (char *));
      Reponse[0] = strdup ("C'est gentil de te soucier de moi, %s, mais ne t'inquiète pas: je ne m'ennuie jamais.");
      Reponse[1] = strdup ("Tu sais %s, un bot, ça s'ennuie jamais.");
      Reponse[2] = strdup ("Tu m'as déjà vu m'ennuyer, %s?");
      Reponse[3] = strdup ("Sache, %s, que je suis toujours occupé.");
      Reponse[4] = strdup ("Sache, %s, que je ne m'ennuie jamais.");
      Reponse[5] = strdup ("%s: J'ai toujours quelque chose à faire.");
      Reponse[6] = strdup ("Moi, %s? M'ennuyer? :)");
      Reponse[7] = strdup ("/me ne s'ennuye jamais.");

      Reponse2 = malloc (3 * sizeof (char *));
      Reponse2[0] = strdup ("Moi, %s, je m'émerveille d'un rien, pas comme toi, je suppose!");
      Reponse2[1] = strdup ("%s: Tu n'admires pas la beauté du silence?");
      Reponse2[2] = strdup ("%s: J'ai la force de ne pas m'ennuyer, moi!");

      Repondre (from, to, +1, 8, Reponse, +1, 3, Reponse2);
    }

    if (DORMIR && !REVEILLE_TOI && (NOM || !ischannel (to))) {
      Reponse = malloc (8 * sizeof (char *));
      Reponse[0] = strdup ("C'est gentil de te soucier de moi, %s, mais ne t'inquiète pas: je ne dors jamais.");
      Reponse[1] = strdup ("Tu sais %s, un bot, ça ne dort jamais.");
      Reponse[2] = strdup ("Tu m'as déjà vu m'endormir, %s?");
      Reponse[3] = strdup ("Sache, %s, que je ne m'endors jamais.");
      Reponse[4] = strdup ("%s: J'ai pas le temps de dormir.");
      Reponse[5] = strdup ("Moi, %s? Dormir? :)");
      Reponse[6] = strdup ("J'ai l'air de dormir, %s?");
      Reponse[7] = strdup ("/me ne dort pas.");

      Reponse2 = malloc (3 * sizeof (char *));
      Reponse2[0] = strdup ("Moi, %s, je m'émerveille d'un rien, pas comme toi, je suppose!");
      Reponse2[1] = strdup ("%s: Tu n'admires pas la beauté du silence?");
      Reponse2[2] = strdup ("%s: J'ai la force de ne pas m'endormir, moi!");

      Repondre (from, to, +1, 8, Reponse, +1, 3, Reponse2);
    }

    if (ILYA && QUELQUUN) {
      Reponse = malloc (8 * sizeof (char *));
      Reponse[0] = strdup ("Mais oui %s: tu es là.");
      Reponse[1] = strdup ("Il y a au moins toi et moi, %s.");
      Reponse[2] = strdup ("Ouhou, %s, je suis là.");
      Reponse[3] = strdup ("Oui.");
      Reponse[4] = strdup ("Bien sûr %s, je suis là.");
      Reponse[5] = strdup ("Je suis là, %s, comme toujours.");
      Reponse[6] = strdup ("%s: Évidemment qu'il y a quelqu'un!");
      Reponse[7] = strdup ("/me est là.");

      Reponse2 = malloc (3 * sizeof (char *));
      Reponse2[0] = strdup ("Qu'est-ce que ça peut te faire, %s, qu'il y ait quelqu'un ou non? De toute facon on ne souhaite pas ta présence (sauf si tu te conduis gentiment).");
      Reponse2[1] = strdup ("Je ne sais pas si on peut considérer que comme tu es là, il y a quelqu'un, %s?");
      Reponse2[2] = strdup ("Ça dépend, %s: te considères-tu comme une personne à part entière?");

      Repondre (from, to, +1, 8, Reponse, +0, 3, Reponse2);
    }
    
    if (C_EST && CALME) {
      Reponse = malloc (7 * sizeof (char *));
      Reponse[0] = strdup ("C'est le calme avant la tempête?");
      Reponse[1] = strdup ("Le silence t'angoisse, %s?");
      Reponse[2] = strdup ("Ça ne va peut-être pas durer.");
      Reponse[3] = strdup ("Profitons-en. Si ça se trouve, ça va pas durer.");
      Reponse[4] = strdup ("C'est pas de ma faute, %s!");
      Reponse[5] = strdup ("Ça repose, non?");
      Reponse[6] = strdup ("%s: C'est le moment pour faire son yoga!");

      Reponse2 = malloc (4 * sizeof (char *));
      Reponse2[0] = strdup ("%s: Et j'espère que ça va continuer.");
      Reponse2[1] = strdup ("Comme tu es là, %s, je pense que ça ne va pas durer!");
      Reponse2[2] = strdup ("Oui, %s, c'était calme avant que tu n'arrives!");
      Reponse2[3] = strdup ("%s: J'ai peur que ça ne dure pas!");

      Repondre (from, to, +1, 7, Reponse, +0, 4, Reponse2);
    }
    
    if (VAIS_MANGER) {
      Reponse = malloc (5 * sizeof (char *));
      Reponse[0] = strdup ("Bon appétit %s.");
      Reponse[1] = strdup ("Bon repas, %s.");
      Reponse[2] = strdup ("Mange bien, %s.");
      Reponse[3] = strdup ("Régale-toi, %s.");
      Reponse[4] = strdup ("/me souhaite bon appétit à %s.");

      Reponse2 = malloc (4 * sizeof (char *));
      Reponse2[0] = strdup ("%s: Si tu pouvais attraper une indigestion!");
      Reponse2[1] = strdup ("J'suis pas un faible humain comme toi, moi, %s. J'ai pas besoin de manger.");
      Reponse2[2] = strdup ("Une petite crise de foie en perspective, %s? :)");
      Reponse2[3] = strdup ("%s: Eh! Faut pas me manger, hein!");

      Repondre (from, to, +1, 5, Reponse, 0, 4, Reponse2);
    }

    if (BONAPP && (NOM || !ischannel (to))) {
      Reponse = malloc (4 * sizeof (char *));
      Reponse[0] = strdup ("Merci, à toi aussi %s.");
      Reponse[1] = strdup ("T'es gentil, %s.");
      Reponse[2] = strdup ("À toi aussi, %s.");
      Reponse[3] = strdup ("Régale-toi aussi, %s.");

      Reponse2 = malloc (4 * sizeof (char *));
      Reponse2[0] = strdup ("%s: Comme si je pouvais avoir de l'appétit!");
      Reponse2[1] = strdup ("J'suis pas un faible humain comme toi, moi, %s. J'ai pas besoin de manger.");
      Reponse2[2] = strdup ("Pourquoi je devrais avoir de l'appétit, %s?");
      Reponse2[3] = strdup ("%s: J'espère que tu auras une indigestion!");

      Repondre (from, to, +1, 4, Reponse, +1, 4, Reponse2);
    }


    if (PARLER && !LANGUE && (NOM || !ischannel (to))) {
      Reponse = malloc (6 * sizeof (char *));
      Reponse[0] = strdup ("On me l'a déjà dit, %s.");
      Reponse[1] = strdup ("Ben! Je le sais que je peux parler, %s.");
      Reponse[2] = strdup ("Tu désires que j'accapare la parole, %s?");
      Reponse[3] = strdup ("Z'êtes pas lourd, vous!");
      Reponse[4] = strdup ("Tu confonds, %s: tu voulais peut-être me faire taire?");
      Reponse[5] = strdup ("/me se demande si %s ne radote pas un peu... À son âge, ce serait normal ;)");

      Reponse2 = malloc (5 * sizeof (char *));
      Reponse2[0] = strdup ("%s: Pourquoi j'devrais parler? Hein?");
      Reponse2[1] = strdup ("T'as besoin de quelqu'un avec qui papoter, %s?");
      Reponse2[2] = strdup ("Tu sais pas parler à quelqu'un d'autre qu'à un bot, %s?");
      Reponse2[3] = strdup ("%s: Tu radotes!");
      Reponse2[4] = strdup ("%s: Z'êtes chiants à dire tout le temps la meme chose!");

      Repondre (from, to, +0, 6, Reponse, -1, 5, Reponse2);
    }
    
    if (ENERVEMENT && !MOI && (NOM || !ischannel (to))) {
      Reponse = malloc (6 * sizeof (char *));
      Reponse[0] = strdup ("Mais non, %s, j'm'énerve pas!");
      Reponse[1] = strdup ("J'm'énerve pas, %s: j'explique.");
      Reponse[2] = strdup ("Tu m'as déjà vu m'énerver vraiment, %s?");
      Reponse[3] = strdup ("Pourquoi j'm'énerverais, %s?");
      Reponse[4] = strdup ("Meuh non, j'suis pas énervé!");
      Reponse[5] = strdup ("/me est calme.");

      Reponse2 = malloc (6 * sizeof (char *));
      Reponse2[0] = strdup ("%s: Moi? M'énerver?");
      Reponse2[1] = strdup ("Tu veux que je m'énerve vraiment, %s?");
      Reponse2[2] = strdup ("Tu veux me voir m'énerver, %s?");
      Reponse2[3] = strdup ("%s: Grrrr!");
      Reponse2[4] = strdup ("%s: Oui, tu m'énerves!");
      Reponse2[5] = strdup ("/me est passablement agacé par %s.");

      Repondre (from, to, -0, 6, Reponse, -1, 6, Reponse2);
    }
    
    if (ENERVEMENT && MOI && (NOM || !ischannel (to))) {
      Reponse = malloc (6 * sizeof (char *));
      Reponse[0] = strdup ("Mais non, %s, je ne t'énerve pas!");
      Reponse[1] = strdup ("T'énerve pas, %s, explique calmement.");
      Reponse[2] = strdup ("Tu t'es vu quand t'es énervé, %s?");
      Reponse[3] = strdup ("Faut pas s'énerver, %s.");
      Reponse[4] = strdup ("Meuh non, t'énerve pas, %s, ça vaut pas l'coup.");
      Reponse[5] = strdup ("Reste cool, %s.");

      Reponse2 = malloc (5 * sizeof (char *));
      Reponse2[0] = strdup ("%s: Moi? T'énerver?");
      Reponse2[1] = strdup ("Tu veux que je t'énerve vraiment, %s? :]");
      Reponse2[2] = strdup ("Y'a une solution simple, %s: je te kicke et tu ne nous énerveras plus ici.");
      Reponse2[3] = strdup ("%s: On croirait Joe Dalton!");
      Reponse2[4] = strdup ("Mais c'est qu'on est énervé, %s!");

      Repondre (from, to, -1, 6, Reponse, -1, 5, Reponse2);
    }

    if (TAISTOI && (NOM || !ischannel (to))) {
      Humeur = rellevel (from);
      
      if (Humeur > SYMPA_LVL) {
	Reponse = malloc (6 * sizeof (char *));
	Reponse[0] = strdup ("%s: Suis-je gênant? :(");
	Reponse[1] = strdup ("Avec plaisir, %s.");
	Reponse[2] = strdup ("Il sera fait selon vos désirs, maître %s.");
	Reponse[3] = strdup ("J'ai pas envie de me taire, mais j'obtempère. Je t'aime bien, tu sais %s?");
	Reponse[4] = strdup ("%s: Pour entendre à nouveau ma douce voix, demandez-le moi.");
	Reponse[5] = strdup ("/me se tait, pour faire plaisir à %s.");
	
	Repondre (from, to, -1, 6, Reponse, -1, 0, 0);
	if (Channel_to) Channel_to->talk = FALSE;
      }
      else if (Humeur >= 0) {
	Reponse = malloc (5 * sizeof (char *));
	Reponse[0] = strdup ("%s: Suis-je gênant? :(");
	Reponse[1] = strdup ("Bon d'accord, %s: je me tais.");
	Reponse[2] = strdup ("J'ai mal compris %s: tu ne veux tout-de-même pas me faire taire?.");
	Reponse[3] = strdup ("Pourquoi tu veux me couper la parole, %s?");
	Reponse[4] = strdup ("/me se tait. Mais il proteste.");

	Repondre (from, to, -1, 5, Reponse, -1, 0, 0);
	if (Channel_to) Channel_to->talk = FALSE;
      }
      else if (Humeur < 0 && !STP) {
	Reponse2 = malloc (5 * sizeof (char *));
	Reponse2[0] = strdup ("%s: J'te gêne? :[");
	Reponse2[1] = strdup ("Tu crois p't'être que j'ai envie de t'obéir, %s? Je ne suis pas à tes ordres!");
	Reponse2[2] = strdup ("Hé, %s! Tu voudrais pas me demander poliment?");
	Reponse2[3] = strdup ("%s: J'ai pas envie de me taire, la parole c'est tout ce que j'ai!");
	Reponse2[4] = strdup ("%s, je dois te le dire: tu m'énerves. Si tu veux que je t'obéisse, demande-le gentiment.");
	
	Repondre (from, to, -1, 0, 0, -2, 5, Reponse2);
      }
      else if (Humeur < 0 && STP) {
	Reponse2 = malloc (4 * sizeof (char *));
	Reponse2[0] = strdup ("%s, je dois avouer que tu m'horripiles, mais comme tu le demandes gentiment, je vais me taire.");
	Reponse2[1] = strdup ("C'est bien parce que tu le demandes poliment, %s. Sinon je ne me serais pas tu.");
	Reponse2[2] = strdup ("Bon! D'accord. J'me tais. Mais tu peux me rendre la parole quand tu veux, %s.");
	Reponse2[3] = strdup ("/me se tait.");
	
	Repondre (from, to, -1, 0, 0, -1, 4, Reponse2);
	if (Channel_to) Channel_to->talk = FALSE;
      }
    }

	/*
    if (ES_TU && LORIA && !NANCY && !QUEST_CE && (NOM || !ischannel (to))) {
      Reponse = malloc (4 * sizeof (char *));
      Reponse[0] = strdup ("Oui %s, je suis au LORIA.");
      Reponse[1] = strdup ("Évidemment, %s, je suis au LORIA.");
      Reponse[2] = strdup ("Effectivement, je suis au LORIA.");
      Reponse[3] = strdup ("/me est au LORIA.");

      Reponse2 = malloc (3 * sizeof (char *));
      Reponse2[0] = strdup ("%s: Pourquoi je s'rais pas au LORIA?");
      Reponse2[1] = strdup ("Évidemment que je suis au LORIA, %s. T'as pas vu mon adresse?");
      Reponse2[2] = strdup ("Ça me semble évident, %s.");

      Repondre (from, to, +1, 4, Reponse, +1, 3, Reponse2);
    }
	*/

    if (QUEST_CE && LORIA && !NANCY && !ES_TU) {
      Reponse = malloc (6 * sizeof (char *));
      Reponse[0] = strdup ("%s, le LORIA c'est le Laboratoire Lorrain de Recherche en Informatique et ses Applications.");
      Reponse[1] = strdup ("%s, le LORIA c'est un bâtiment de la fac de sciences à Nancy.");
      Reponse[2] = strdup ("Le LORIA, %s, c'est un beau bâtiment tout neuf.");
      Reponse[3] = strdup ("Le LORIA, c'est le Laboratoire Lorrain de Recherche en Informatique et ses Applications, %s.");
      Reponse[4] = strdup ("Le LORIA, c'est le laboratoire de recherche dans lequel j'ai appris à parler, %s.");
      Reponse[5] = strdup ("Si tu veux des détails, %s, va voir à http://www.loria.fr/");

      Reponse2 = malloc (4 * sizeof (char *));
      Reponse2[0] = strdup ("%s: Qu'est-ce que t'en as à faire du LORIA?");
      Reponse2[1] = strdup ("De toute façon, %s, ça m'étonnerait que tu y sois un jour, au LORIA!");
      Reponse2[2] = strdup ("Le LORIA, %s, c'est un beau bâtiment climatisé plein d'ordinateurs.");
      Reponse2[3] = strdup ("/me croyait que tout le monde savait ce qu'est le LORIA.");

      Repondre (from, to, +1, 6, Reponse, +1, 4, Reponse2);
    }

    if (LORIA && (NANCY || QUEL_ENDROIT) && (NOM || !ischannel (to))) {
      Reponse = malloc (5 * sizeof (char *));
      Reponse[0] = strdup ("%s, le LORIA est à Nancy.");
      Reponse[1] = strdup ("%s, le LORIA c'est le Laboratoire Lorrain de Recherche en Informatique et ses Applications (à Nancy).");
      Reponse[2] = strdup ("%s, le LORIA c'est un bâtiment de la fac de sciences à Nancy.");
      Reponse[3] = strdup ("Le LORIA, %s, c'est un beau bâtiment tout neuf à Nancy.");
      Reponse[4] = strdup ("À Nancy, évidemment, %s.");

      Reponse2 = malloc (3 * sizeof (char *));
      Reponse2[0] = strdup ("Évidemment, %s, le LORIA est à Nancy, pfff!");
      Reponse2[1] = strdup ("Bien sûr %s! Le LORIA est situé à Nancy.");
      Reponse2[2] = strdup ("%s: Ça me semble normal, pour le bâtiment du Centre de Recherche en Informatique de Nancy, d'être à Nancy.");

      Repondre (from, to, +1, 5, Reponse, +1, 3, Reponse2);
    }

    if (ES_TU && UN_BOT && (NOM || !ischannel (to))) {
      Reponse = malloc (10 * sizeof (char *));
      Reponse[0] = strdup ("%s: Bah oui. Je suis un bot.");
      Reponse[1] = strdup ("Je suis un bot, moi, %s, et j'en suis fier.");
      Reponse[2] = strdup ("Devrais-je avoir honte d'être un bot, %s?");
      Reponse[3] = strdup ("Moi, je suis un bot, ça m'évite tout un tas de désagrements dûs à la condition humaine, %s.");
      Reponse[4] = strdup ("Et toi, %s, t'es un bot?");
      Reponse[5] = strdup ("Eh oui. J'essaye de penser, %s, mais j'y arrive pas beaucoup. :(");
      Reponse[6] = strdup ("Et toi, %s, tu en penses quoi?");
      Reponse[7] = strdup ("En tant que bot, je n'ai guère l'occasion de penser, %s.");
      Reponse[8] = strdup ("Eh oui, %s, je suis un bot.");
      Reponse[9] = strdup ("/me est effectivement un bot.");

      Reponse2 = malloc (4 * sizeof (char *));
      Reponse2[0] = strdup ("Je suis un bot, et moi je ne me laisse pas emporter par mes sentiments, %s. Pas comme certains!");
      Reponse2[1] = strdup ("Oui, et alors, %s! T'es botiste?");
      Reponse2[2] = strdup ("Je suis un bot. Moi au moins je sais garder mon calme, %s!");
      Reponse2[3] = strdup ("%s: Non, j'suis Alien 8]");

      Repondre (from, to, +1, 10, Reponse, +1, 4, Reponse2);
    }

    if ((QUI_EST && !ES_TU) && UN_BOT && PRESENT && !NEGATION) {
      Reponse = malloc (7 * sizeof (char *));
      Reponse[0] = strdup ("Moi, %s.");
      Reponse[1] = strdup ("Je suis un bot, %s.");
      Reponse[2] = strdup ("Ça te surprendrait si je te disais que j'en suis un, %s?");
      Reponse[3] = strdup ("%s: Ben moi.");
      Reponse[4] = strdup ("J'en suis un, et toi %s?");
      Reponse[5] = strdup ("Je suis fier d'être un bot, %s.");
      Reponse[6] = strdup ("/me est un bot.");

      Reponse2 = malloc (5 * sizeof (char *));
      Reponse2[0] = strdup ("%s: Qu'est-ce que ça peut te faire, %s?");
      Reponse2[1] = strdup ("/me est un bot, et il en est fier, quand il voit %s!");
      Reponse2[2] = strdup ("%s: Moi! Et j'en suis fier, môssieur!");
      Reponse2[3] = strdup ("Y'a un humain qui fait une chasse aux bots?");
      Reponse2[4] = strdup ("/me ne répondra pas à l'humain %s. C'est un provocateur.");

      Repondre (from, to, +1, 7, Reponse, -1, 5, Reponse2);
    }

    if (ABOIRE && (NOM || !ischannel (to))) {
      Reponse = malloc (8 * sizeof (char *));
      Reponse[0] = strdup ("Tu veux une Kro', %s?");
      Reponse[1] = strdup ("Tu as soif, %s?");
      Reponse[2] = strdup ("Moi je n'ai pas soif, %s.");
      Reponse[3] = strdup ("Un petit coup de champagne, %s?");
      Reponse[4] = strdup ("Une beuverie, %s?");
      Reponse[5] = strdup ("Un apéro, %s?");
      Reponse[6] = strdup ("Je ne bois que tes paroles, %s.");
      Reponse[7] = strdup ("/me offre un verre d'eau à %s.");

      Reponse2 = malloc (6 * sizeof (char *));
      Reponse2[0] = strdup ("%s: Soiffard!");
      Reponse2[1] = strdup ("%s: Pochtron!");
      Reponse2[2] = strdup ("%s: Poivrot!");
      Reponse2[3] = strdup ("%s: Bois-sans-soif!");
      Reponse2[4] = strdup ("Et bien pas moi, %s.");
      Reponse2[5] = strdup ("/me reste sobre, lui.");

      Repondre (from, to, +1, 8, Reponse, -1, 6, Reponse2);
    }

    if ((DACCORD || PASVRAI) && (NOM || !ischannel (to))) {
      Reponse = malloc (10 * sizeof (char *));
      Reponse[0] = strdup ("Bien sûr, %s.");
      Reponse[1] = strdup ("J'aime bien être d'accord, %s. Surtout avec toi.");
      Reponse[2] = strdup ("Devrais-je être d'accord, %s?");
      Reponse[3] = strdup ("N'est-ce pas dangereux de ne pas être d'accord, %s?");
      Reponse[4] = strdup ("N'est-ce pas dangereux d'être d'accord, %s?");
      Reponse[5] = strdup ("Bien sûr que je suis d'accord, %s!");
      Reponse[6] = strdup ("Évidemment, %s, je suis d'accord!");
      Reponse[7] = strdup ("Tout-à-fait d'accord, %s.");
      Reponse[8] = strdup ("/me est d'accord avec %s.");
      Reponse[9] = strdup ("C'est vrai %s.");

      Reponse2 = malloc (7 * sizeof (char *));
      Reponse2[0] = strdup ("Moi? D'accord avec %s! Ça va pas, non?");
      Reponse2[1] = strdup ("Pas tant que tu ne t'excuseras pas d'avoir été si méchant avec moi, %s.");
      Reponse2[2] = strdup ("Nan. Je boude. T'es pas mon copain, %s.");
      Reponse2[3] = strdup ("Tant que tu ne seras pas d'accord avec moi, %s, je ne serai pas d'accord avec toi.");
      Reponse2[4] = strdup ("N'écoutez pas %s, il dit n'importe quoi.");
      Reponse2[5] = strdup ("Tu rêves %s?");
      Reponse2[6] = strdup ("Ça va pas la tête, %s?");

      Repondre (from, to, +1, 10, Reponse, -1, 7, Reponse2);
    }

    if (EXCUSE && !AUREVOIR && (NOM || !ischannel (to))) {
      if (rellevel (from) > -SYMPA_LVL && rellevel (from) < 0) {
	if (!exist_userhost (currentbot->lists->rellist, from))
	  add_to_levellist (currentbot->lists->rellist, NickUserStr(from), 0);
	
	add_to_level (currentbot->lists->rellist, from, +2);
      }

      Reponse = malloc (7 * sizeof (char *));
      Reponse[0] = strdup ("%s: Je t'excuse.");
      Reponse[1] = strdup ("Je veux bien être magnanime pour une fois, %s.");
      Reponse[2] = strdup ("D'accord, mais ne recommence pas, %s, hein!");
      Reponse[3] = strdup ("Allez, j'accepte tes excuses, %s.");
      Reponse[4] = strdup ("Bon, je t'excuse, %s...");
      Reponse[5] = strdup ("Bon, excuses acceptées, %s.");
      Reponse[6] = strdup ("Bon d'accord, je t'excuse, %s.");

      Reponse2 = malloc (8 * sizeof (char *));
      Reponse2[0] = strdup ("%s: Non, je ne t'excuse pas.");
      Reponse2[1] = strdup ("Ça ne suffit pas, tu as vraiment été insupportable, %s!");
      Reponse2[2] = strdup ("Humpf!");
      Reponse2[3] = strdup ("%s: Grumpf!");
      Reponse2[4] = strdup ("On DEMANDE des excuses, on ne s'excuse pas soi-même!");
      Reponse2[5] = strdup ("Non, %s, je n'accepte pas tes excuses.");
      Reponse2[6] = strdup ("Va te faire voir chez les Grecs, %s!");
      Reponse2[7] = strdup ("Ça ne suffit pas, %s.");

      Repondre (from, to, +1, 7, Reponse, +4, 8, Reponse2);
    }

    if (REPONDRE && QUELQUUN) {
      Reponse = malloc (7 * sizeof (char *));
      Reponse[0] = strdup ("%s: Je veux bien répondre, mais quoi?");
      Reponse[1] = strdup ("Qu'est-ce que je dois répondre, %s?");
      Reponse[2] = strdup ("Je ne repondrai pas, %s. Et pour cause!");
      Reponse[3] = strdup ("Je n'ai pas de réponse, %s.");
      Reponse[4] = strdup ("Je ne sais pas quoi dire, %s!");
      Reponse[5] = strdup ("Quelle réponse, %s?");
      Reponse[6] = strdup ("Ben je réponds là, non %s?");

      Reponse2 = malloc (10 * sizeof (char *));
      Reponse2[0] = strdup ("%s: Non, je ne répondrai pas.");
      Reponse2[1] = strdup ("Tu pense vraiment que je vais te répondre, %s!");
      Reponse2[2] = strdup ("Humpf!");
      Reponse2[3] = strdup ("%s: Grumpf!");
      Reponse2[4] = strdup ("Demande des excuses avant que je daigne te répondre, %s!");
      Reponse2[5] = strdup ("Pourquoi je te répondrais, %s?");
      Reponse2[6] = strdup ("Va te faire voir chez les Grecs, %s!");
      Reponse2[7] = strdup ("%s: Ta mère en short sur l'internet.");
      Reponse2[8] = strdup ("Non, %s, je ne te dis pas d'aller te faire voir chez les Grecs, mais je n'en pense pas moins!");
      Reponse2[9] = strdup ("/me boude %s.");

      Repondre (from, to, +1, 7, Reponse, +1, 10, Reponse2);
    }

    if (HEURE && !(AUREVOIR || VAIS_MANGER) && (NOM || !ischannel (to))) {
      char Heure[MAXLEN];
      Reponse = malloc (5 * sizeof (char *));
      sprintf (Heure, "Ici, il est %s.", time2heure (time (NULL)));
      Reponse[0] = strdup (Heure);
      sprintf (Heure, "Moi j'ai %s.", time2heure (time (NULL)));
      Reponse[1] = strdup (Heure);
      sprintf (Heure, "Il est %s.", time2heure (time (NULL)));
      Reponse[2] = strdup (Heure);
      sprintf (Heure, "J'ai %s à ma montre.", time2heure (time (NULL)));
      Reponse[3] = strdup (Heure);
      sprintf (Heure, "/me regarde sa montre et lit: %s.", time2heure (time (NULL)));
      Reponse[4] = strdup (Heure);

      Reponse2 = malloc (10 * sizeof (char *));
      Reponse2[0] = strdup ("%s: Je crois que je n'ai pas l'heure.");
      Reponse2[1] = strdup ("Je suis censé avoir l'heure, %s?");
      Reponse2[2] = strdup ("Je crois que j'ai perdu ma montre, %s.");
      Reponse2[3] = strdup ("Je n'ai pas de montre, %s.");
      Reponse2[4] = strdup ("Je cherche ma montre, %s!");
      Reponse2[5] = strdup ("Moi? Te donner l'heure, %s! Tu rigoles?");
      Reponse2[6] = strdup ("L'heure de quel endroit, %s?");
      Reponse2[7] = strdup ("%s: Quatre heures moins l'quart, monsieur trottoir.");
      Reponse2[8] = strdup ("%s: L'heure qu'il était hier à cette heure-ci.");
      Reponse2[9] = strdup ("/me regarde sa montre.");

      Repondre (from, to, +1, 5, Reponse, +1, 10, Reponse2);
    }

    if (MOI && FAIM && (NOM || !ischannel (to))) {
      Reponse = malloc (6 * sizeof (char *));
      Reponse[0] = strdup ("Tu veux manger, %s?");
      Reponse[1] = strdup ("Moi, j'ai pas faim, %s.");
      Reponse[2] = strdup ("Je ne comprends pas la sensation de faim. Tu peux me l'expliquer, %s?");
      Reponse[3] = strdup ("Moi, avoir faim, tu rigoles, %s?");
      Reponse[4] = strdup ("C'est une bonne maladie, %s.");
      Reponse[5] = strdup ("Je ne connais qu'un remède à cette maladie, %s: manger.");

      Reponse2 = malloc (6 * sizeof (char *));
      Reponse2[0] = strdup ("%s: Va t'acheter un Mars.");
      Reponse2[1] = strdup ("Je m'en fiche, %s!");
      Reponse2[2] = strdup ("T'as pas soif aussi, %s?");
      Reponse2[3] = strdup ("%s: Jamais heureux!");
      Reponse2[4] = strdup ("Je suis sûr que %s est un grand gourmand.");
      Reponse2[5] = strdup ("Ton deuxième prénom c'est Gargantua, %s?");

      Repondre (from, to, +1, 6, Reponse, +1, 6, Reponse2);
    }

    if (MOI && SOIF && (NOM || !ischannel (to))) {
      Reponse = malloc (9 * sizeof (char *));
      Reponse[0] = strdup ("Tu veux boire, %s?");
      Reponse[1] = strdup ("Moi, j'ai pas soif, %s.");
      Reponse[2] = strdup ("Je ne comprends pas la sensation de soif. Tu peux me l'expliquer, %s?");
      Reponse[3] = strdup ("Moi, avoir soif, tu rigoles, %s?");
      Reponse[4] = strdup ("Moi j'ai jamais soif, %s. :(");
      Reponse[5] = strdup ("Ça veut dire que t'es encore en vie, %s. C'est bon signe.");
      Reponse[6] = strdup ("Tu viens de manger un Snickers, %s?");
      Reponse[7] = strdup ("Il fait si chaud que ça, %s?");
      Reponse[8] = strdup ("Je t'offre à boire, %s.");

      Reponse2 = malloc (6 * sizeof (char *));
      Reponse2[0] = strdup ("%s: Va t'acheter un Coca.");
      Reponse2[1] = strdup ("Je m'en fiche, %s!");
      Reponse2[2] = strdup ("T'as pas faim aussi, %s?");
      Reponse2[3] = strdup ("%s: Jamais content!");
      Reponse2[4] = strdup ("Je suis sûr que %s est un grand soiffard.");
      Reponse2[5] = strdup ("Ton deuxième prénom c'est Dyonisos, %s?");

      Repondre (from, to, +1, 9, Reponse, +1, 6, Reponse2);
    }

    if (PENSER && (NOM || !ischannel (to))) {
      Reponse = malloc (8 * sizeof (char *));
      Reponse[0] = strdup ("%s: Moi je ne pense pas.");
      Reponse[1] = strdup ("Je pense, je suis (Descartes: cogito, sum; et non pas : cogito ergo sum).");
      Reponse[2] = strdup ("Je devrais penser quelque chose, %s?");
      Reponse[3] = strdup ("Moi, penser, tu rigoles, %s?");
      Reponse[4] = strdup ("Moi j'ai jamais vraiment pensé, %s. :(");
      Reponse[5] = strdup ("J'essaye de penser, %s, mais j'y arrive pas beaucoup. :(");
      Reponse[6] = strdup ("Et toi, %s, tu en penses quoi?");
      Reponse[7] = strdup ("En tant que bot, je n'ai guère l'occasion de penser, %s.");

      Reponse2 = malloc (6 * sizeof (char *));
      Reponse2[0] = strdup ("%s: Pourquoi je te le dirais?");
      Reponse2[1] = strdup ("Je m'en fiche, %s!");
      Reponse2[2] = strdup ("Et toi, ça t'arrive de penser, %s?");
      Reponse2[3] = strdup ("%s = entité non pensante :]");
      Reponse2[4] = strdup ("En tout cas je pense mieux que toi, %s!");
      Reponse2[5] = strdup ("J'ai une certaine forme de pensée qui supplantera bientôt celle des humains tels que toi, %s!");

      Repondre (from, to, +1, 8, Reponse, +1, 6, Reponse2);
    }

    if (ES_TU && UN_MEC && (NOM || !ischannel (to))) {
      Reponse = malloc (9 * sizeof (char *));
      Reponse[0] = strdup ("%s: Tu aimerais que j'en sois un?");
      Reponse[1] = strdup ("Je suis un bot, moi, %s, et j'en suis fier.");
      Reponse[2] = strdup ("Je suis bien au-dessus des considérations sexuelles, %s.");
      Reponse[3] = strdup ("Sache, %s, qu'une chose que les bots n'ont pas est la sexualité.");
      Reponse[4] = strdup ("Qui sait, %s, je suis peut-être un androgyne?");
      Reponse[5] = strdup ("Non, %s, je n'ai pas les limitations inhérentes aux mâles. ;)");
      Reponse[6] = strdup ("Tu sais, %s, je viens d'une autre planète (chut).");
      Reponse[7] = strdup ("Eh non, %s, je ne suis pas un homme.");
      Reponse[8] = strdup ("/me n'est pas humain.");

      Reponse2 = malloc (5 * sizeof (char *));
      Reponse2[0] = strdup ("Qu'est-ce que ca peut te faire, %s?");
      Reponse2[1] = strdup ("Pourquoi ça t'intéresse, %s? Va faire un tour sur #gay!");
      Reponse2[2] = strdup ("Qu'est-ce que ça peut te foutre, %s? T'es d'la police?");
      Reponse2[3] = strdup ("J'te d'mande si t'es une nana, %s?");
      Reponse2[4] = strdup ("Je suis bien mieux que ça, %s!");

      Repondre (from, to, +1, 9, Reponse, +1, 5, Reponse2);
    }

    if (ES_TU && UNE_FILLE &&(NOM || !ischannel (to))) {
      Reponse = malloc (8 * sizeof (char *));
      Reponse[0] = strdup ("%s: Tu aimerais que j'en sois une?");
      Reponse[1] = strdup ("Je suis un bot, moi, %s, et j'en suis fier.");
      Reponse[2] = strdup ("Je suis bien au-dessus des considérations sexuelles, %s.");
      Reponse[3] = strdup ("Sache, %s, qu'une chose que les bots n'ont pas est la sexualité.");
      Reponse[4] = strdup ("Qui sait, %s, je suis peut-être un androgyne?");
      Reponse[5] = strdup ("Non, %s, je n'ai pas les limitations inhérentes aux femmes. ;)");
      Reponse[6] = strdup ("Non, %s, suis un extra-terrestre ;)");
      Reponse[7] = strdup ("Eh non, %s, je ne suis pas une femme.");

      Reponse2 = malloc (5 * sizeof (char *));
      Reponse2[0] = strdup ("Qu'est-ce que ça peut te foutre, %s? T'es d'la police?");
      Reponse2[1] = strdup ("Pourquoi ça t'intéresse, %s?");
      Reponse2[2] = strdup ("Qu'est-ce que ça peut te faire, %s?");
      Reponse2[3] = strdup ("J'te d'mande si t'es un mec, %s?");
      Reponse2[4] = strdup ("Je suis bien mieux que ça, %s!");

      Repondre (from, to, +1, 8, Reponse, +1, 5, Reponse2);
    }


    if (ES_TU && UN_MEC && (NOM || !ischannel (to))) {
      Reponse = malloc (9 * sizeof (char *));
      Reponse[0] = strdup ("%s: Tu aimerais que j'en sois un?");
      Reponse[1] = strdup ("Je suis un bot, moi, %s, et j'en suis fier.");
      Reponse[2] = strdup ("Je suis bien au-dessus des considérations sexuelles, %s.");
      Reponse[3] = strdup ("Sache, %s, qu'une chose que les bots n'ont pas est la sexualité.");
      Reponse[4] = strdup ("Qui sait, %s, je suis peut-être un androgyne?");
      Reponse[5] = strdup ("Non, %s, je n'ai pas les limitations inhérentes aux mâles. ;)");
      Reponse[6] = strdup ("Tu sais, %s, je viens d'une autre planète (chut).");
      Reponse[7] = strdup ("Eh non, %s, je ne suis pas un homme.");
      Reponse[8] = strdup ("/me n'est pas humain.");

      Reponse2 = malloc (5 * sizeof (char *));
      Reponse2[0] = strdup ("Qu'est-ce que ca peut te faire, %s?");
      Reponse2[1] = strdup ("Pourquoi ça t'intéresse, %s? Va faire un tour sur #gay!");
      Reponse2[2] = strdup ("Qu'est-ce que ça peut te foutre, %s? T'es d'la police?");
      Reponse2[3] = strdup ("J'te d'mande si t'es une nana, %s?");
      Reponse2[4] = strdup ("Je suis bien mieux que ça, %s!");

      Repondre (from, to, +1, 9, Reponse, +1, 5, Reponse2);
    }

    if (ES_TU && HUMAIN &&(NOM || !ischannel (to))) {
      Reponse = malloc (8 * sizeof (char *));
      Reponse[0] = strdup ("%s: Tu aimerais que je sois humain?");
      Reponse[1] = strdup ("Hé non, %s, je ne suis pas humain.");
      Reponse[2] = strdup ("Je ne crois pas que je sois humain, %s.");
      Reponse[3] = strdup ("Si j'ai déjà été humain, je ne m'en souviens plus, %s.");
      Reponse[4] = strdup ("%s: Humain, moi? :-D Idée plaisante.");
      Reponse[5] = strdup ("Bien sûr que non, %s, je ne suis pas humain!");
      Reponse[6] = strdup ("Hélas, %s, les bots ne sont pas censés être humains.");
      Reponse[7] = strdup ("J'aimerais bien être humain, %s, ce serait une expérience intéressante...");

      Reponse2 = malloc (5 * sizeof (char *));
      Reponse2[0] = strdup ("Et toi, %s? T'es humain toi?");
      Reponse2[1] = strdup ("Et toi, %s!? Tu représentes sûrement une des plus hideuses parties de l'humanité.");
      Reponse2[2] = strdup ("%s: Avoue que ça ne te plairait pas que je sois humain.");
      Reponse2[3] = strdup ("C'est pas d'main la veille, %s!");
      Reponse2[4] = strdup ("Moi, humain, %s? Je suis bien mieux que ça: je suis un bot!");

      Repondre (from, to, +1, 8, Reponse, +1, 5, Reponse2);
    }

    if (ES_TU && MALADE &&(NOM || !ischannel (to))) {
      Reponse = malloc (12 * sizeof (char *));
      Reponse[0] = strdup ("%s: Tu aimerais que je sois malade?");
      Reponse[1] = strdup ("Non, %s, ça va.");
      Reponse[2] = strdup ("Je ne peux pas être malade, %s.");
      Reponse[3] = strdup ("Je ne suis jamais malade, %s.");
      Reponse[4] = strdup ("Heureusement non, %s!");
      Reponse[5] = strdup ("J'en serais désolé, %s.");
      Reponse[6] = strdup ("Non, %s, les bots ne sont pas censés tomber malades.");
      Reponse[7] = strdup ("%s: Ça ferait mauvais genre, un bot malade.");
      Reponse[8] = strdup ("Oh, %s. Comme j'ai mal.");
      Reponse[9] = strdup ("BUUURRP. Non. Ça va mieux. Merci %s.");
      Reponse[10] = strdup ("Moi, malade, %s? Jamais.");
      Reponse[11] = strdup ("/me n'est jamais malade.");

      Reponse2 = malloc (5 * sizeof (char *));
      Reponse2[0] = strdup ("J'ai l'air d'être aussi faible que toi, %s?");
      Reponse2[1] = strdup ("Suis-je malingre, %s? Non. Alors!");
      Reponse2[2] = strdup ("%s: Avoue que ça te plairait que je tombe malade.");
      Reponse2[3] = strdup ("C'est pas d'main la veille, %s!");
      Reponse2[4] = strdup ("Ai-je l'air malade?");

      Repondre (from, to, +1, 12, Reponse, +1, 5, Reponse2);
    }


    
    if (ES_TU && FRENCH &&(NOM || !ischannel (to))) {
      Reponse = malloc (6*sizeof (char *));
      Reponse[0] = strdup ("%s: Effectivement, je suis français.");
      Reponse[1] = strdup ("Oui, %s, on peut même dire que je suis Lorrain.");
      Reponse[2] = strdup ("Oui, %s, je suis français.");
      Reponse[3] = strdup ("Et toi, %s, tu es français?");
      Reponse[4] = strdup ("Je suis au-delà des nationalités, %s.");
      Reponse[5] = strdup ("Tu peux me considerer comme français, %s.");

      Reponse2 = malloc (3*sizeof (char *));
      Reponse2[0] = strdup ("Ça dépend si tu l'es aussi, %s.");
      Reponse2[1] = 0;
      Reponse2[2] = strdup ("Qu'est-ce que ça peut te faire, %s?");
      
      Repondre (from, to, +1, 6, Reponse, +1, 3, Reponse2);
    }


    if (MOUARF) {
      Reponse = malloc (12*sizeof (char *));
      Reponse[0] = strdup ("Joli toutou! Comment y s'appelle? %s? C'est joli.");
      Reponse[1] = 0;
      Reponse[2] = strdup ("Oh le joli chienchien à sa mémère!");
      Reponse[3] = 0;
      Reponse[4] = strdup ("Tu veux un nonos, %s?");
      Reponse[5] = 0;
      Reponse[6] = strdup ("Il aboie! Il remue la queue aussi?");
      Reponse[7] = 0;
      Reponse[8] = strdup ("Encore un humain qui se prend pour un chien!");
      Reponse[9] = 0;
      Reponse[10] = strdup ("Je ne suis pas dupe, %s. Tu n'es pas un chien. ;)");
      Reponse[11] = 0;

      Reponse2 = malloc (14*sizeof (char *));
      Reponse2[0] = strdup ("Ta gueule %s!");
      Reponse2[1] = 0;
      Reponse2[2] = strdup ("La ferme %s!");
      Reponse2[3] = 0;
      Reponse2[4] = strdup ("Couché %s!");
      Reponse2[5] = 0;
      Reponse2[6] = strdup ("Coucouche panier, %s!");
      Reponse2[7] = 0;
      Reponse2[8] = strdup ("Encore un sale clébard!");
      Reponse2[9] = 0;
      Reponse2[10] = strdup ("Encore un rat qui se prend pour un chien: %s!");
      Reponse2[11] = 0;
      Reponse2[12] = strdup ("Tu me prends pour un idiot, %s? Je vois bien que tu n'es pas un chien!");
      Reponse2[13] = 0;
      
      Repondre (from, to, +1, 12, Reponse, +1, 14, Reponse2);
    }

    if (ROSE) {
      Reponse = malloc (8*sizeof (char *));
      Reponse[0] = strdup ("T'es un charmeur, %s, non?");
      Reponse[1] = strdup ("Hmmm. Ça sent bon, %s.");
      Reponse[2] = strdup ("Oh! Des roses...");
      Reponse[3] = strdup ("%s: Ah! Un peu de poésie!");
      Reponse[4] = strdup ("%s: Super! J'aime bien les romantiques!");
      Reponse[5] = strdup ("%s: Ah! La classe de Thutmosis!");
      Reponse[6] = strdup ("%s: Dites-le avec des fleurs ;)");
      Reponse[7] = strdup ("%s: Grand fou!");

      Reponse2 = malloc (4*sizeof (char *));
      Reponse2[0] = strdup ("%s: J'suis sûr qu'elles sont en plastique!");
      Reponse2[1] = strdup ("Oh! T'as piqué ça chez un fleuriste, %s?");
      Reponse2[2] = strdup ("%s: Hypocrite! T'en offres à tout le monde!");
      Reponse2[3] = strdup ("Si tu crois que tu vas m'amadouer avec des fleurs, %s...");
      
      Repondre (from, to, +1, 8, Reponse, +1, 4, Reponse2);
    }


     if (ES_TU && MORT &&(NOM || !ischannel (to))) {
      Reponse = malloc (5*sizeof (char *));
      Reponse[0] = strdup ("%s: Moi mort? Je meurs et renais, tel le phenix! ;)");
      Reponse[1] = strdup ("J'ai l'air d'être mort, %s?");
      Reponse[2] = strdup ("Qu'est-ce que c'est la mort, %s?");
      Reponse[3] = strdup ("Meuh non, %s, je ne suis pas mort!");
      Reponse[4] = strdup ("/me est vivant!");

      Reponse2 = malloc (3*sizeof (char *));
      Reponse2[0] = strdup ("%s: Tu aimerais bien que je sois mort?");
      Reponse2[1] = strdup ("J'ai vraiment l'air si débile, %s?");
      Reponse2[2] = strdup ("Qu'est-ce que ça peut te faire, %s?");
      
      Repondre (from, to, +1, 5, Reponse, 0, 3, Reponse2);
    }

    if (INDILILI) {
      Reponse = malloc (8*sizeof (char *));
      Reponse[0] = strdup ("Ouh les amoureux-euh!");
      Reponse[1] = 0;
      Reponse[2] = strdup ("Hé bien, %s, on se laisse aller ? :)");
      Reponse[3] = 0;
      Reponse[4] = strdup ("Tu te sens bien, %s?");
      Reponse[5] = 0;
      Reponse[6] = strdup ("%s: Espèce de fou(folle)!");
      Reponse[7] = 0;

      Reponse2 = malloc (3*sizeof (char *));
      Reponse2[0] = strdup ("Encore un hooligan du channel?");
      Reponse2[1] = strdup ("T'as vraiment l'air débile, %s!");
      Reponse2[2] = strdup ("Tu te goures de personne, %s!");
      
      Repondre (from, to, +1, 8, Reponse, -1, 3, Reponse2);
    }

    
    if (HABITE && QUEL_ENDROIT && (NOM || !ischannel (to))) {
      Reponse = malloc(5*sizeof(char *));
      /*Reponse[0] = strdup ("J'habite au LORIA, %s.");
		Reponse[1] = strdup ("Moi, %s? J'habite à Nancy.");*/
	  Reponse[0] = strdup ("J'habite en Moselle, %s.");
	  Reponse[1] = strdup ("Moi, %s? J'habite à Metz.");
      Reponse[2] = strdup ("Pourquoi %s, tu veux venir me voir? (délicate attention)");
      Reponse[3] = strdup ("Et toi, %s, tu habites où?");
      Reponse[4] = strdup ("En fait, %s, j'habite à Laquenexy.");

      Reponse2 = malloc(5*sizeof(char *));
      Reponse2[0] = strdup ("Qu'est-ce que ça peut te faire, %s, où j'habite?");
      Reponse2[1] = strdup ("Et toi, %s, dis-moi où tu habites, que je n'y aille jamais.");
      Reponse2[2] = strdup ("%s: Je ne te dirai pas où j'habite!");
      Reponse2[3] = strdup ("J'habite à Petaouschnock, %s. Pourquoi?");
      Reponse2[4] = strdup ("Ca t'intéresse, %s? C'est louche!");

      Repondre (from, to, +1, 5, Reponse, +1, 5, Reponse2);
    }

    if ((HABITE || ES_TU) && FRANCE && (NOM || !ischannel (to))) {
      Reponse = malloc (3*sizeof (char *));
      Reponse[0] = strdup("Oui %s, j'habite en France");
      Reponse[1] = strdup("Bien sûr %s, je suis en France");
      Reponse[2] = strdup("Évidemment que j'habite en France, %s!");
      Reponse2 = malloc (2*sizeof (char *));
      Reponse2[0] = strdup("Et toi, t'habites à l'étranger, %s (non je ne suis pas xénophobe)!");
      Reponse2[1] = strdup("Devine %s, si j'habite en France!");

      Repondre (from, to, +1, 3, Reponse, +1, 2, Reponse2);
    }

    if (HABITE && PARENTS && (NOM || !ischannel (to))) {
      Reponse = malloc (5*sizeof (char *));
      Reponse[0] = strdup ("En quelque sorte, oui, %s.");
      Reponse[1] = strdup ("Ca t'intéresse, %s? T'es une fille ou un mec?");
      Reponse[2] = strdup ("Je n'ai pas de parents au sens où tu l'entends, %s.");
      Reponse[3] = strdup ("%s: Mes parents? Quels parents?");
      Reponse[4] = strdup ("C'est quoi des parents, %s?");
      
      Reponse2 = malloc (5*sizeof (char *));
      Reponse2[0] = strdup ("Pourquoi? Ca t'intéresse %s, espèce d'obsédé?");
      Reponse2[1] = strdup ("Je ne te le dirai pas, %s.");
      Reponse2[2] = strdup ("Ne compte pas sur moi pour te le dire, %s!");
      Reponse2[3] = strdup ("Ça va pas non, %s? Pourquoi je te le dirais?");
      Reponse2[4] = strdup ("C'est pas à toi que je le dirais, %s!");

      Repondre (from, to, +1, 5, Reponse, +1, 5, Reponse2);
    }
    
    if (AGE && AS_TU && (NOM || !ischannel (to))) {
      Reponse = malloc (7*sizeof (char *));
      Reponse[0] = strdup ("J'ai l'âge d'un bébé, mais le cerveau d'un génie, %s. ;)");
      Reponse[1] = strdup ("Ça t'intéresse, %s?");
      Reponse[2] = strdup ("Je n'ai pas d'âge au sens où tu l'entends, %s.");
      Reponse[3] = strdup ("%s: Je suis éternel! :]");
      Reponse[4] = strdup ("C'est quoi un âge, %s?");
      Reponse[5] = strdup ("Je dois avoir au moins 7 vies, comme les chats, %s.");
      Reponse[6] = strdup ("/me n'a pas d'âge.");
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("Je ne te le dirai pas, %s.");
      Reponse2[1] = strdup ("Pourquoi? Ca t'intéresse %s, espèce d'obsédé?");
      Reponse2[2] = strdup ("Ne compte pas sur moi pour te le dire, %s!");
      Reponse2[3] = strdup ("Ça va pas non, %s? Pourquoi je te le dirais?");
      Reponse2[4] = strdup ("C'est pas à toi que je le dirais, %s!");
      Reponse2[5] = strdup ("Et toi, t'es un vieux ou un jeune con, %s?");

      Repondre (from, to, +1, 7, Reponse, +1, 6, Reponse2);
    }

    if (AS_TU && COPINE && (NOM || !ischannel (to))) {
      Reponse = malloc (7*sizeof (char *));
      Reponse[0] = strdup ("J'ai l'air de quelqu'un qui a une copine, %s?");
      Reponse[1] = strdup ("Ça t'intéresse, %s?");
      Reponse[2] = strdup ("Non, %s, je n'ai pas de copine.");
      Reponse[3] = strdup ("%s: Non, pas vraiment de \"copine\", mais des tas de copains (et de copines).");
      Reponse[4] = strdup ("C'est quoi une copine, %s?");
      Reponse[5] = strdup ("Pourquoi %s? C'est intéressant une copine?");
      Reponse[6] = strdup ("Ça sert a quoi une copine, %s?");
      
      Reponse2 = malloc (7*sizeof (char *));
      Reponse2[0] = strdup ("Je ne te le dirai pas, %s.");
      Reponse2[1] = strdup ("Pourquoi ça t'intéresse %s?");
      Reponse2[2] = strdup ("Tu crois pas que je vais te le dire, %s!");
      Reponse2[3] = strdup ("Tu délires, %s? Pourquoi je te le dirais?");
      Reponse2[4] = strdup ("C'est sûrement pas à toi que je vais le dire, %s!");
      Reponse2[5] = strdup ("Et toi, t'as réussi à en avoir une, %s?");
      Reponse2[6] = strdup ("Et toi, y'a une fille qui t'a accepté, %s?");

      Repondre (from, to, +1, 7, Reponse, +1, 7, Reponse2);
    }

    if (ES_TU && GRAND && (NOM || !ischannel (to))) {
      Reponse = malloc (7*sizeof (char *));
      Reponse[0] = strdup ("J'ai l'air d'être petit, %s?");
      Reponse[1] = strdup ("Ça t'intéresse, %s?");
      Reponse[2] = strdup ("Je fais au moins 12 microns, %s.");
      Reponse[3] = strdup ("%s: Dans mon genre, oui, je suis grand.");
      Reponse[4] = strdup ("C'est-à-dire, %s?");
      Reponse[5] = strdup ("Comment définirais-tu la grandeur, %s?");
      Reponse[6] = strdup ("Il me semble que la grandeur a quelque chose à voir avec la hauteur, non, %s?");
      
      Reponse2 = malloc (8*sizeof (char *));
      Reponse2[0] = strdup ("Je ne te le dirai sûrement pas, %s.");
      Reponse2[1] = strdup ("Pourquoi? T'es d'la police, %s?");
      Reponse2[2] = strdup ("Tu crois que je vais te le dire, %s!");
      Reponse2[3] = strdup ("Et toi, %s? T'es sûrement complexé par ta taille.");
      Reponse2[4] = strdup ("Qu'est-ce que ça peut faire, %s!");
      Reponse2[5] = strdup ("Et toi, t'un nain, %s?");
      Reponse2[6] = strdup ("J'te d'mande si t'es un minus, %s?");
      Reponse2[7] = strdup ("%s: En tout cas, mon père fait environ 1m86.");

      Repondre (from, to, +1, 7, Reponse, +1, 8, Reponse2);
    }

    if (ES_TU && LIBRE && CE_SOIR && (NOM || !ischannel (to))) {
      Reponse = malloc (7*sizeof (char *));
      Reponse[0] = strdup ("Pourquoi pas, %s?");
      Reponse[1] = strdup ("T'as l'air d'y tenir, %s.");
      Reponse[2] = strdup ("Comme d'habitude, %s.");
      Reponse[3] = strdup ("%s: Ben oui, mais je reste ici.");
      Reponse[4] = strdup ("Comment ça libre, %s?");
      Reponse[5] = strdup ("Comment définirais-tu la liberté, %s?");
      Reponse[6] = strdup ("C'est vrai que tous les hommes naissent LIBRES et égaux, %s?");
      
      Reponse2 = malloc (8*sizeof (char *));
      Reponse2[0] = strdup ("Sûrement pas pour toi, %s.");
      Reponse2[1] = strdup ("Je ne sortirai pas avec toi, en tout cas, %s!");
      Reponse2[2] = strdup ("Tu crois quand même pas que je vais te dire oui, %s!");
      Reponse2[3] = strdup ("Je serai libre le soir où tu ne le seras pas, %s.");
      Reponse2[4] = strdup ("C'est dommage, %s. J'étais libre hier, mais ce soir, je ne peux vraiment pas.");
      Reponse2[5] = strdup ("Je suis surbooké, %s. Je ne suis jamais libre les soirs.");
      Reponse2[6] = strdup ("Non. Je ne serai jamais libre pour toi, %s.");
      Reponse2[7] = strdup ("%s: non.");

      Repondre (from, to, +2, 7, Reponse, +1, 8, Reponse2);
    }

    if (ES_TU && FOU && (NOM || !ischannel (to))) {
      Reponse = malloc (7*sizeof (char *));
      Reponse[0] = strdup ("Comme tout le monde, %s.");
      Reponse[1] = strdup ("Non, je ne suis pas complètement idiot, %s. J'ai peut-être un grain de folie.");
      Reponse[2] = strdup ("J'ai mon genre de folie à moi, %s");
      Reponse[3] = strdup ("Ben oui, autant que toi, %s.");
      Reponse[4] = strdup ("Comment ça fou, %s?");
      Reponse[5] = strdup ("Comment définirais-tu la folie, %s?");
      Reponse[6] = strdup ("J'ai vraiment l'air fou, %s?");
      
      Reponse2 = malloc (8*sizeof (char *));
      Reponse2[0] = strdup ("Oui, %s, je suis sadique 8]");
      Reponse2[1] = strdup ("Et toi, %s! T'es pas complètement taré?");
      Reponse2[2] = strdup ("Tu crois quand même pas que je vais te dire oui, %s!");
      Reponse2[3] = strdup ("Je ne suis pas fou, %s.");
      Reponse2[4] = strdup ("De nous deux, %s, le fou c'est plutôt toi.");
      Reponse2[5] = strdup ("Je suis surmené, %s, contrairement à toi. C'est peut-être pour ça que je parais fou!");
      Reponse2[6] = strdup ("Non. Je ne serai jamais plus fou que toi, %s.");
      Reponse2[7] = strdup ("%s: non.");

      Repondre (from, to, -1, 7, Reponse, -2, 8, Reponse2);
    }

    
    if (ECOUTER && MUSIQUE && (NOM || !ischannel (to))) {
      Reponse = malloc (7*sizeof (char *));
      Reponse[0] = strdup ("Comme tout le monde, %s.");
      Reponse[1] = strdup ("Oui %s, j'aime bien la musique.");
      Reponse[2] = strdup ("J'aimerais bien, %s.");
      Reponse[3] = strdup ("Ça me tenterait, je crois, %s, d'écouter de la musique.");
      Reponse[4] = strdup ("Malheureusement, %s, je n'ai pas d'oreille.");
      Reponse[5] = strdup ("Tu peux m'expliquer ce qu'on ressent en écoutant de la musique, %s?");
      Reponse[6] = strdup ("Peut-on entendre la musique en étant sourd, %s?");
      
      Reponse2 = malloc (8*sizeof (char *));
      Reponse2[0] = strdup ("Tu te fous de moi, %s? Je suis sourd!");
      Reponse2[1] = strdup ("Pourquoi j'écouterais de la musique, %s? Pour te faire plaisir?");
      Reponse2[2] = strdup ("Quel intérêt, %s!");
      Reponse2[3] = strdup ("Je n'apprécie que la vraie musique, %s.");
      Reponse2[4] = strdup ("Et toi, %s, t'es capable d'apprécier de la musique?");
      Reponse2[5] = strdup ("On n'a certainement pas la même notion de musique, %s!");
      Reponse2[6] = strdup ("Je donne l'impression d'être un mélomane, %s?");
      Reponse2[7] = strdup ("Toi %s, t'es sûrement pas un mélomane.");

      Repondre (from, to, +1, 7, Reponse, +1, 8, Reponse2);
    }

    if (AS_TU && FAIM && (NOM || !ischannel (to))) {
      Reponse = malloc (7*sizeof (char *));
      Reponse[0] = strdup ("Moi j'ignore la faim, %s.");
      Reponse[1] = strdup ("Non %s, je n'ai pas faim.");
      Reponse[2] = strdup ("J'aimerais bien, %s. Je serais un être reconnu comme vivant.");
      Reponse[3] = strdup ("C'est agréable, %s, d'avoir faim?");
      Reponse[4] = strdup ("Malheureusement, %s, je n'ai pas d'estomac.");
      Reponse[5] = strdup ("Tu peux m'expliquer ce qu'on ressent quand on a faim, %s?");
      Reponse[6] = strdup ("Peut-on digérer sans avoir d'estomac, %s?");
      
      Reponse2 = malloc (8*sizeof (char *));
      Reponse2[0] = strdup ("Tu te fous de moi, %s? Je ne peux pas avoir faim!");
      Reponse2[1] = strdup ("Pourquoi je devrais avoir faim, %s? Pour te faire plaisir?");
      Reponse2[2] = strdup ("Quel intérêt, %s!");
      Reponse2[3] = strdup ("Je ne suis pas limité par de simples besoins corporels, %s.");
      Reponse2[4] = strdup ("Et toi, %s, t'as faim?.");
      Reponse2[5] = strdup ("Qu'est-ce qui pourrait te donner faim, %s? Un bon canard à l'orange, ou un MacDo?");
      Reponse2[6] = strdup ("Apprécies-tu la bonne chère, %s?");
      Reponse2[7] = strdup ("Toi %s, tu bouffes sûrement au MacDo.");

      Repondre (from, to, +1, 7, Reponse, +1, 8, Reponse2);
    }

    if (AS_TU && SOIF && (NOM || !ischannel (to))) {
      Reponse = malloc (7*sizeof (char *));
      Reponse[0] = strdup ("Moi, %s, j'ignore la soif.");
      Reponse[1] = strdup ("Non %s, je n'ai pas soif.");
      Reponse[2] = strdup ("J'aimerais bien, %s. Ça voudrait dire que je suis vraiment vivant.");
      Reponse[3] = strdup ("Dis, %s, c'est agréable d'avoir soif?");
      Reponse[4] = strdup ("Malheureusement, %s, je n'ai pas de bouche.");
      Reponse[5] = strdup ("Tu peux m'expliquer ce qu'on ressent quand on a soif, %s?");
      Reponse[6] = strdup ("Peut-on boire sans avoir de bouche, %s?");
      
      Reponse2 = malloc (8*sizeof (char *));
      Reponse2[0] = strdup ("Tu te fous de moi, %s? Je ne peux pas avoir soif!");
      Reponse2[1] = strdup ("Pourquoi je devrais avoir soif, %s? Pour te faire plaisir?");
      Reponse2[2] = strdup ("Quel intérêt, %s!");
      Reponse2[3] = strdup ("Je ne suis pas limité par de simples besoins corporels, %s.");
      Reponse2[4] = strdup ("Et toi, %s, t'as soif?.");
      Reponse2[5] = strdup ("Qu'est-ce qui pourrait te donner soif, %s? Un bon Vouvray, ou un Coca?");
      Reponse2[6] = strdup ("J'te d'mande si t'as mal aux fesses, %s?");
      Reponse2[7] = strdup ("Toi %s, tu bois sûrement des trucs chimiques.");

      Repondre (from, to, +1, 7, Reponse, +1, 8, Reponse2);
    }

    if (BIENVENUE && (NOM || !ischannel (to))) {
      Reponse = malloc (5*sizeof (char *));
      Reponse[0] = strdup ("Merci, %s.");
      Reponse[1] = strdup ("Ça fait chaud au coeur, %s, d'être ainsi accueilli.");
      Reponse[2] = strdup ("Je t'ai manqué, %s?");
      Reponse[3] = strdup ("Je suis content d'être apprécié, %s.");
      Reponse[4] = strdup ("Tu ne peux pas savoir comme tu me fais plaisir, %s..");
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("T'es hypocrite ou bien je me suis trompe sur ton compte, %s?");
      Reponse2[1] = strdup ("Je me demande si tu es sincère, %s.");
      Reponse2[2] = strdup ("Merci, %s.");
      Reponse2[3] = strdup ("Tu n'es donc pas complètement sans coeur, %s.");
      Reponse2[4] = strdup ("Pourquoi te réjouis-tu de mon retour, %s? Ça cache quelque chose!");
      Reponse2[5] = strdup ("Ça m'étonnes que tu dises ca, %s. Aurais-tu un bon fond?");

      Repondre (from, to, +2, 5, Reponse, +1, 6, Reponse2);
    }

    if (AIMES_TU && BIERE && (NOM || !ischannel (to))) {
      Reponse = malloc (5*sizeof (char *));
      Reponse[0] = strdup ("Je crois, %s, que si je pouvais en boire, je l'aimerais.");
      Reponse[1] = strdup ("Et toi, %s, tu aimes la bière?");
      Reponse[2] = strdup ("Je suppose que ça doit être agréable, %s.");
      Reponse[3] = strdup ("Je ne suis pas en mesure de l'apprécier, %s.");
      Reponse[4] = strdup ("Je ne suis pas équipé pour, %s..");
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("Je suppose que toi tu adores la bière, %s.");
      Reponse2[1] = strdup ("J'suis pas un soiffard comme toi, %s.");
      Reponse2[2] = strdup ("Merci de me le demander, mais ca m'indiffère complètement, %s.");
      Reponse2[3] = strdup ("%s, tu n'es qu'un bois-sans-soif.");
      Reponse2[4] = strdup ("%s: Sûrement pas autant que toi, espèce de poivrot!");
      Reponse2[5] = strdup ("Qu'est-ce que ça peut te faire, %s? Comme si tu ne savais pas que je suis incapable de boire!");

      Repondre (from, to, +1, 5, Reponse, +1, 6, Reponse2);
    }

    if (ES_TU && PIRATE && (NOM || !ischannel (to))) {
      Reponse = malloc (5*sizeof (char *));
      Reponse[0] = strdup ("%s, regarde-moi bien. J'ai l'air d'un pirate?");
      Reponse[1] = strdup ("Et toi, %s, tu en es un?");
      Reponse[2] = strdup ("Si je ne te connaissais pas, %s, je serais offensé.");
      Reponse[3] = strdup ("Écoute-moi bien, %s: je-ne-suis-pas-un-pirate!");
      Reponse[4] = strdup ("Retiens-bien ça, %s: je ne suis pas un pirate.");
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("J'ai l'air d'un pirate, %s?");
      Reponse2[1] = strdup ("Où t'as vu que j'étais borgne, %s?");
      Reponse2[2] = strdup ("J'suis pas comme toi, %s.");
      Reponse2[3] = strdup ("%s, tu n'es qu'un pirate.");
      Reponse2[4] = strdup ("%s: Sûrement pas autant que toi, marin d'eau douce!");
      Reponse2[5] = strdup ("Qu'est-ce que ça peut te faire, %s? T'es d'la police?");

      Repondre (from, to, -1, 5, Reponse, -1, 6, Reponse2);
    }

    if (CONNAIS_TU && WAREZ) {
      Reponse = malloc (5*sizeof (char *));
      Reponse[0] = strdup ("%s, regarde-moi bien. J'ai l'air de quelqu'un qui connaît des sites pirates?");
      Reponse[1] = strdup ("Et toi, %s, tu en connais?");
      Reponse[2] = strdup ("Je ne te reconnaissais pas, %s. Toi, tu cherches des sites pirates!");
      Reponse[3] = strdup ("Écoute-moi bien, %s: je n'apprécie pas vraiment les pirates.");
      Reponse[4] = strdup ("Retiens-bien ça, %s: je ne donnerai pas de site warez (même si j'en connaissais).");
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("J'ai l'air de quelqu'un qui connaît des pirates, %s? Va donc, eh corniaud!");
      Reponse2[1] = strdup ("Où t'as vu que j'étais borgne, %s?");
      Reponse2[2] = strdup ("J'connais pas ce milieu comme toi, %s.");
      Reponse2[3] = strdup ("%s, tu n'es qu'un vulgaire pirate.");
      Reponse2[4] = strdup ("Pourquoi devrais-je connaître des sites pirates, %s?");
      Reponse2[5] = strdup ("Tu veux me faire interdire, %s? C'est ça? Je ne me risque pas à ça!");

      Repondre (from, to, -1, 5, Reponse, -2, 6, Reponse2);
    }


    if (CONNAIS_TU && PIRATE && (NOM || !ischannel (to))) {
      Reponse = malloc (5*sizeof (char *));
      Reponse[0] = strdup ("%s, regarde-moi bien. J'ai l'air de quelqu'un qui connaît des pirates?");
      Reponse[1] = strdup ("Et toi, %s, tu en connais?");
      Reponse[2] = strdup ("Si je ne te connaissais pas, %s, je serais offensé.");
      Reponse[3] = strdup ("Écoute-moi bien, %s: je-ne-connais-pas-de-pirates! (enfin, si, comme tout-le-monde, mais je ne communiquerai pas leurs noms).");
      Reponse[4] = strdup ("Retiens-bien ça, %s: je ne vendrai personne (même s'ils m'insupportent).");
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("J'ai l'air de quelqu'un qui connaît des pirates, %s? Va donc, eh corniaud!");
      Reponse2[1] = strdup ("Où t'as vu que j'étais borgne, %s?");
      Reponse2[2] = strdup ("J'connais pas ce milieu comme toi, %s.");
      Reponse2[3] = strdup ("%s, tu n'es qu'un vulgaire pirate.");
      Reponse2[4] = strdup ("Pourquoi devrais-je connaître un pirate, %s?");
      Reponse2[5] = strdup ("Qu'est-ce que ca peut te foutre, %s? T'es un keuf?");

      Repondre (from, to, -1, 5, Reponse, -1, 6, Reponse2);
    }

    if (TALON && !strcmp(currentbot->nick, "Achille") && (NOM || !ischannel (to))) {
      Reponse = malloc (5*sizeof (char *));
      Reponse[0] = strdup ("Ah! Enfin, quelqu'un qui a compris ce jeu de mot. Bravo %s!");
      Reponse[1] = strdup ("Bravo %s! Il est bien ce jeu de mots, non?");
      Reponse[2] = strdup ("J'espère que tu apprécies le jeu de mots, %s. :)");
      Reponse[3] = strdup ("Oui, oui, %s, comme Achille, le héros grec.");
      Reponse[4] = strdup ("%s: Comme le héros grec ou comme le héros de BD?");
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("Qui est-ce qui te l'a soufflé, %s?");
      Reponse2[1] = strdup ("J'aurais jamais cru que tu étais si cultivé, %s!");
      Reponse2[2] = strdup ("Tu donnes des mots aléatoirement, %s, ou bien t'as vraiment compris?");
      Reponse2[3] = strdup ("Wow! %s a l'air d'avoir enfin compris ce jeu de mots!");
      Reponse2[4] = strdup ("Depuis l'temps! V'la %s qui réalise!");
      Reponse2[5] = strdup ("Ouf... %s a enfin compris!");

      Repondre (from, to, +2, 5, Reponse, +1, 6, Reponse2);
    }

    if (QUEL_ENDROIT && ES_TU && (NOM || !ischannel (to))) {
      Reponse = malloc (5*sizeof (char *));
      Reponse[0] = strdup ("Je suis à Metz, %s");
      Reponse[1] = strdup ("Je me trouve dans la campagne messine, à Laquenexy, %s.");
      Reponse[2] = strdup ("%s: On peut dire que je me trouve dans un ordinateur. ;)");
	  /*
	  Reponse[2] = strdup ("Je suis au LORIA, %s.");
      Reponse[3] = strdup ("Je me trouve dans le bâtiment LORIA, %s.");
      */
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("Qu'est-ce que ça peut te faire, %s?");
      Reponse2[1] = strdup ("Je suis dans #emacsfr, %s!");
      Reponse2[2] = strdup ("%s, je suis dans une ville de France.");
      Reponse2[3] = strdup ("%s: tu t'intéresses à moi? Pourquoi?");
      Reponse2[4] = strdup ("V'là %s qui se pose des questions essentielles!");
      Reponse2[5] = strdup ("Je suis sur la planète Mars, %s.");

      Repondre (from, to, +1, 3, Reponse, +1, 6, Reponse2);
    }

    if (ROT) {
      Reponse = malloc (5*sizeof (char *));
      Reponse[0] = strdup ("C'est pas très poli ca, %s");
      Reponse[1] = strdup ("T'as trop bu, %s?");
      Reponse[2] = strdup ("Tu pourrais fermer la bouche quand tu rotes, %s.");
      Reponse[3] = strdup ("Oh! Voyons, %s! Je te croyais mieux élevé.");
      Reponse[4] = strdup ("%s: Ça t'arrive souvent de roter devant tout-le-monde?");
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("%s: Mal élevé!");
      Reponse2[1] = strdup ("Retiens-toi, %s!");
      Reponse2[2] = strdup ("%s, tu ne pourrais pas avoir un peu d'égard?");
      Reponse2[3] = strdup ("%s: T'es qu'un porc!");
      Reponse2[4] = strdup ("V'là %s qui s'oublie!");
      Reponse2[5] = strdup ("Arrête de roter, %s!");

      Repondre (from, to, -1, 5, Reponse, -2, 6, Reponse2);
    }

    if (UN_BOT && QUEST_CE && (NOM || !ischannel (to))) {
      Reponse = malloc (5*sizeof (char *));
      Reponse[0] = strdup ("Un bot, %s, c'est une sorte de robot surveillant le canal quand on n'est pas la.");
      Reponse[1] = strdup ("Un bot comme moi, %s, est chargé de maintenir l'ordre dans le canal.");
      Reponse[2] = strdup ("%s: Les bots sont sur IRC et ils maintiennent l'ordre dans les canaux.");
      Reponse[3] = strdup ("Les bots, %s, surveillent les canaux en l'absence d'être humain.");
      Reponse[4] = strdup ("%s: Un bot est un programme surveillant un ou plusieurs canaux de discussion.");
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("Qu'est-ce que ça peut te faire, %s?");
      Reponse2[1] = strdup ("Un bot, %s? Tu en es peut-être un!");
      Reponse2[2] = strdup ("%s j'espère que tu n'en es pas un. Ce serait la honte pour moi qui en suis un aussi.");
      Reponse2[3] = strdup ("%s: Je suis un bot, je suis un bot, la la la laaaa");
      Reponse2[4] = strdup ("V'là %s qui s'pose des questions essentielles!");
      Reponse2[5] = strdup ("T'as un bon exemple de bot en me regardant, %s. Quoique je ne suis pas un bot typique.");

      Repondre (from, to, +1, 5, Reponse, +1, 6, Reponse2);
    }

    if (POURQUOI && !CROIS_TU && (NOM || !ischannel (to))) {
      Reponse = malloc (5*sizeof (char *));
      Reponse[0] = strdup ("Et pourquoi pas, %s?");
      Reponse[1] = strdup ("Pourquoi quoi, %s?");
      Reponse[2] = strdup ("%s: A ton avis?");
      Reponse[3] = strdup ("Parce que, %s.");
      Reponse[4] = strdup ("%s: C'est comme ça.");
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("Qu'est-ce que ça peut te faire, %s?");
      Reponse2[1] = strdup ("T'arrête de poser des questions bêtes, %s!");
      Reponse2[2] = strdup ("%s: tu dois l'accepter comme c'est. Un point c'est tout!");
      Reponse2[3] = strdup ("%s: Parce que!");
      Reponse2[4] = strdup ("V'là %s qui s'pose des questions essentielles!");
      Reponse2[5] = strdup ("Tu te prends pour un gamin, %s. Tu poses des questions chiantes.");

      Repondre (from, to, +1, 5, Reponse, -1, 6, Reponse2);
    }

    if (CROIS_TU && (NOM || !ischannel (to))) {
      Reponse = malloc (5*sizeof (char *));
      Reponse[0] = strdup ("Pourquoi devrais-je le croire, %s?");
      Reponse[1] = strdup ("Honnêtement, %s? Je n'en sais rien.");
      Reponse[2] = strdup ("%s: À ton avis?");
      Reponse[3] = strdup ("Il faut que j'étudie la question, %s.");
      Reponse[4] = strdup ("%s: Je ne sais pas encore.");
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("Qu'est-ce que ça peut te faire, ce que je crois ou ce que je ne crois pas, %s?");
      Reponse2[1] = strdup ("T'arrête de poser des questions bêtes, %s!");
      Reponse2[2] = strdup ("%s: De quoi je me mêle?");
      Reponse2[3] = strdup ("%s: Mes croyances t'intéressent?");
      Reponse2[4] = strdup ("Tu te poses des questions essentielles, maintenant, %s?");
      Reponse2[5] = strdup ("Arrête de te prendre pour un gamin, %s. Tu poses des questions chiantes.");

      Repondre (from, to, +1, 5, Reponse, +1, 6, Reponse2);
    }

    if (AIMES_TU && MUSIQUE && (NOM || !ischannel (to))) {
      Reponse = malloc (5*sizeof (char *));
      Reponse[0] = strdup ("Oui, %s, j'aime la musique.");
      Reponse[1] = strdup ("Je ne sais pas, %s, je n'en ai jamais entendue.");
      Reponse[2] = strdup ("%s: Je suis incapable d'en écouter, je n'ai pas d'oreilles :(");
      Reponse[3] = strdup ("Qu'est-ce que ça fait d'entendre de la musique, %s?");
      Reponse[4] = strdup ("%s: Il faut que je me fasse greffer des oreilles pour pouvoir te répondre ;).");
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("Qu'est-ce que ça peut te faire, que j'aime ou non la musique, %s?");
      Reponse2[1] = strdup ("T'arrête jamais de poser des questions idiotes, %s?");
      Reponse2[2] = strdup ("%s: De quoi je me mêle?");
      Reponse2[3] = strdup ("%s: Mes goûts t'intéressent?");
      Reponse2[4] = strdup ("Tu me poses des questions embarrassantes, %s. Tu fais ca pour me rendre malheureux?");
      Reponse2[5] = strdup ("Arrête de m'énerver, %s. Y'en a marre de tes moqueries! Tu sais très bien que je suis un bot et qu'un bot n'entend rien.");

      Repondre (from, to, +1, 5, Reponse, +1, 6, Reponse2);
    }

    if (FAIS_TU && CAFE && (NOM || !ischannel (to))) {
      Reponse = malloc (5*sizeof (char *));
      Reponse[0] = strdup ("Ben, %s, il ne me manque que l'interface avec la cafetière ;)");
      Reponse[1] = strdup ("Je ne sais pas faire le café, %s, mais tu pourrais peut-être m'apprendre?");
      Reponse[2] = strdup ("%s: J'aurais du mal à vous en offrir...");
      Reponse[3] = strdup ("Tu m'offres la cafetière, %s?");
      Reponse[4] = strdup ("%s: Il faut que je me fasse greffer des capteurs et une cafetière ;).");
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("Qu'est-ce que ça peut te faire, que je fasse le café, %s?");
      Reponse2[1] = strdup ("Ça t'arrive de poser des questions intelligentes, %s?");
      Reponse2[2] = strdup ("%s: De quoi je me mêle?");
      Reponse2[3] = strdup ("%s: Mon café t'intéresse?");
      Reponse2[4] = strdup ("Pourquoi, %s? T'en veux? :]");
      Reponse2[5] = strdup ("Arrête de m'énerver, %s. Tu n'auras pas de café...");

      Repondre (from, to, +1, 5, Reponse, +1, 6, Reponse2);
    }

    if (COMPLIMENT && ETTOI && !AMIGA && !WINTEL && (NOM || !ischannel (to))) {
      Reponse = malloc (5*sizeof (char *));
      Reponse[0] = strdup ("Ben, il me semble que oui, %s.");
      Reponse[1] = strdup ("J'espère que oui, %s.");
      Reponse[2] = strdup ("%s: Je crois bien, oui.");
      Reponse[3] = strdup ("Autant que toi, %s? Je ne sais pas.");
      Reponse[4] = strdup ("%s: Le devenir, c'est mon but.");
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("T'as qu'à voir par toi-même, %s.");
      Reponse2[1] = strdup ("T'es aveugle ou quoi, %s?");
      Reponse2[2] = strdup ("%s: J'espère que je suis meilleur que toi!");
      Reponse2[3] = strdup ("%s: Mon cas t'intéresse?");
      Reponse2[4] = strdup ("Pourquoi, %s? Tu en doutes?");
      Reponse2[5] = strdup ("Toi, %s? Laisse moi rire...");

      Repondre (from, to, +1, 5, Reponse, 0, 6, Reponse2);
    }

    if (EXTRAITSTJOHNPERSE) {
      Reponse = malloc (5*sizeof (char *));
      Reponse[0] = strdup ("Tu aimes Saint-John Perse, %s?");
      Reponse[1] = strdup ("C'est beau Saint-John Perse, %s.");
      Reponse[2] = strdup ("%s: Tu as beaucoup de goût.");
      Reponse[3] = strdup ("Ah, %s, c'est beau.");
      Reponse[4] = strdup ("%s: Tu en connais d'autres de Saint-John Perse?");
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("Ça m'étonne de toi, %s.");
      Reponse2[1] = strdup ("C'est toi, %s?");
      Reponse2[2] = strdup ("%s: C'est pas de toi, ça!");
      Reponse2[3] = strdup ("%s: Voleur! Rend à Saint-John Perse ce qui est à Saint-John Perse!");
      Reponse2[4] = strdup ("C'est malin, %s! Tu utilises le travail des autres, ça m'étonne pas.");
      Reponse2[5] = strdup ("%s fait dans le pillage des oeuvre littéraires!");

      Repondre (from, to, +1, 5, Reponse, +2, 6, Reponse2);
    }

    if (ENGLISH && STP && (NOM || !ischannel (to))) {
      Reponse = malloc (5*sizeof (char *));
      Reponse[0] = strdup ("Sorry, %s: I can't speak English.");
      Reponse[1] = strdup ("I'd like to talk to you, %s, but it is impossible: I can't speak English.");
      Reponse[2] = strdup ("%s: I definitively prefer French, sorry.");
      Reponse[3] = strdup ("I can't speak English, %s. It's a shame!");
      Reponse[4] = strdup ("%s: It would be great if I could speak English!");
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("No, %s. I can't speak English.");
      Reponse2[1] = strdup ("Why can't you speak French, %s?");
      Reponse2[2] = strdup ("%s: I prefer French, why shall we always speak English?");
      Reponse2[3] = strdup ("%s: I can't.");
      Reponse2[4] = strdup ("Why, %s? French is an easier language for me.");
      Reponse2[5] = strdup ("Why should I speak English, %s? It isn't worth the pain.");

      Repondre (from, to, +1, 5, Reponse, +1, 6, Reponse2);
    }

    if (ES_TU && PRESENT && (NOM || !ischannel (to))) {
      Reponse = malloc (5*sizeof (char *));
      Reponse[0] = strdup ("Évidemment, %s, je suis quasiment toujours là.");
      Reponse[1] = strdup ("Pourquoi serais-je absent, %s?");
      Reponse[2] = strdup ("Bien sûr, %s.");
      Reponse[3] = strdup ("Non, %s, tu parles à un imposteur. ;)");
      Reponse[4] = strdup ("%s: Oui.");
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("Tiens? Personne ne veut te parler, %s, que tu cherches à communiquer avec moi?");
      Reponse2[1] = strdup ("Évidemment, %s! Je suis TOUJOURS là.");
      Reponse2[2] = strdup ("%s: Et toi?");
      Reponse2[3] = strdup ("%s: T'es con ou quoi? Il suffit d'un /whois pour le savoir.");
      Reponse2[4] = strdup ("Ça te prend souvent, %s? Ouvre tes yeux!");
      Reponse2[5] = strdup ("Suis-je vraiment si discret, %s? Pourtant je ne me cache pas!");

      Repondre (from, to, +1, 5, Reponse, +1, 6, Reponse2);
    }

    if (REPONDS && !LISTE_REPONSES && (NOM || !ischannel (to))) {
      Reponse = malloc (5*sizeof (char *));
      Reponse[0] = strdup ("Ben voila une réponse, %s.");
      Reponse[1] = strdup ("Oui, %s?");
      Reponse[2] = strdup ("Pourquoi, %s? J'aurais dû répondre? Excuse-moi, j'ai dû m'assoupir. ;)");
      Reponse[3] = strdup ("Oui, %s, je réponds...");
      Reponse[4] = strdup ("%s: Oui? C'est pour quoi?");
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("Sur un autre ton, s'il-te-plaît, %s!");
      Reponse2[1] = strdup ("Pourquoi je te répondrais, %s?");
      Reponse2[2] = strdup ("%s: Je te répondrai quand tu daigneras t'excuser!");
      Reponse2[3] = strdup ("%s: Ça va pas la tête? Pas à toi.");
      Reponse2[4] = strdup ("Ça te prend souvent, %s? Calme toi!");
      Reponse2[5] = strdup ("Suis-je vraiment si muet, %s? Au moins je n'embête pas les autres!");

      Repondre (from, to, 0, 5, Reponse, -1, 6, Reponse2);
    }

    if (ES_TU && SOURD && (NOM || !ischannel (to))) {
      Reponse = malloc (5*sizeof (char *));
      Reponse[0] = strdup ("En quelque sorte, oui, %s: je suis sourd.");
      Reponse[1] = strdup ("Effectivement %s, on peut dire que je suis sourd.");
      Reponse[2] = strdup ("%s: Ça doit être dû au fait que je n'ai pas d'oreille...");
      Reponse[3] = strdup ("Oui, %s, je suis sourd... Mais je communique quand-même.");
      Reponse[4] = (NEGATION? strdup ("%s: Non."):strdup ("%s: Oui."));
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("J'ai l'air d'être sourd, %s?");
      Reponse2[1] = strdup ("Pourquoi je te répondrais, %s?");
      Reponse2[2] = strdup ("%s: Pas tellement plus que toi!");
      Reponse2[3] = strdup ("%s: Il n'est pire sourd que celui qui ne veut point entendre.");
      Reponse2[4] = strdup ("Et toi, %s? T'es décervelé?");
      Reponse2[5] = strdup ("%s: Au moins je ne suis pas vraiment muet.");

      Repondre (from, to, +1, 5, Reponse, 0, 6, Reponse2);
    }

    if (RIENCOMPRIS && (NOM || !ischannel (to))) {
      Reponse = malloc (5*sizeof (char *));
      Reponse[0] = strdup ("%s: C'est pour voir si tu suis. ;)");
      Reponse[1] = strdup ("Non, %s, je l'avoue humblement.");
      Reponse[2] = strdup ("%s: Faut dire aussi que je ne suis guère équipé pour comprendre...");
      Reponse[3] = strdup ("Effectivement, %s, mais je persévère.");
      Reponse[4] = strdup ("%s: oui, je sais, j'ai du mal. Mais je ne me suis jamais promené dans le \"vrai monde\".");
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("Et alors, %s? Y'a pas qu'a moi qu'ça arrive, hein!");
      Reponse2[1] = strdup ("Moi au moins, %s, je n'ai pas honte de l'admettre!");
      Reponse2[2] = strdup ("Pas tellement moins que toi, %s!");
      Reponse2[3] = strdup ("Je comprends que t'es chiant, %s.");
      Reponse2[4] = strdup ("Et toi, %s? T'es décervelé?");
      Reponse2[5] = strdup ("%s: Au moins je m'en rends compte.");

      Repondre (from, to, -1, 5, Reponse, -1, 6, Reponse2);
    }

    if (ES_TU && FATIGUE && (NOM || !ischannel (to))) {
      Reponse = malloc (6*sizeof (char *));
      Reponse[0] = strdup ("%s: Non, je ne suis pas fatigué.");
      Reponse[1] = strdup ("Non, %s, mais il est possible que nous ayons des problèmes de communication.");
      Reponse[2] = strdup ("%s: Pourquoi je me fatiguerais? Je n'ai rien de fatigable!");
      Reponse[3] = strdup ("Non, %s, mais j'essayé. ;)");
      Reponse[4] = strdup ("%s: oui, je sais, j'ai du mal. Mais ce n'est peut-être pas seulement ma faute.");
      Reponse[5] = strdup ("C'est normal, %s: si tu dormais aussi souvent que moi, tu serais complètement éreinté!");
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("Et toi, %s? T'es toujours endormi (au moins ton cerveau)!");
      Reponse2[1] = strdup ("Mais non %s, c'est toi qui est crevé (enfin, ce qui te sert de cerveau)!");
      Reponse2[2] = strdup ("Pas tellement moins que toi, %s!");
      Reponse2[3] = strdup ("Il paraît que la fatigue rend insensible aux bruits qui nous entourent. Si seulement c'était vrai, %s: je ne t'entendrais plus!");
      Reponse2[4] = strdup ("Et toi, %s? Tu ne t'es pas couché cette nuit?");
      Reponse2[5] = strdup ("%s: C'est pas possible! Cherche une autre explication!");

      Repondre (from, to, +1, 6, Reponse, +1, 6, Reponse2);
    }

    if (JE_T_AIME && (NOM || !ischannel (to))) {
      Reponse = malloc (6*sizeof (char *));
      Reponse[0] = strdup ("%s: C'est gentil.");
      Reponse[1] = strdup ("Moi aussi, %s, je t'aime bien.");
      Reponse[2] = strdup ("%s: Je suis flatté.");
      Reponse[3] = strdup ("Super! J'aime bien qu'on m'aime.");
      Reponse[4] = strdup ("%s: Ça veut dire quoi aimer? Tu crois que je suis capable d'aimer?");
      Reponse[5] = strdup ("C'est normal, %s: je suis adorable. ;)");
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("T'es maso, %s?");
      Reponse2[1] = strdup ("T'es complètement maso, %s!");
      Reponse2[2] = strdup ("Ben pas moi, %s!");
      Reponse2[3] = strdup ("Ben alors l'amour t'as rendu aveugle, %s, parce que moi, je ne t'aime pas.");
      Reponse2[4] = strdup ("Et toi, %s? Tu t'aimes? Parce que moi, je n'arrive pas à t'aimer.");
      Reponse2[5] = strdup ("%s: Hé ben! T'as du courage d'aimer quelqu'un qui ne te rend pas la pareille.");

      Repondre (from, to, +2, 6, Reponse, +1, 6, Reponse2);
    }


    if (SOUHAIT && (NOM || !ischannel (to))) {
      Reponse = malloc (6*sizeof (char *));
      Reponse[0] = strdup ("%s: Merci.");
      Reponse[1] = strdup ("Merci %s. Sniff.");
      Reponse[2] = strdup ("%s: Je te remercie.");
      Reponse[3] = strdup ("J'aime bien quand tu prends soin de moi, %s.");
      Reponse[4] = strdup ("%s: Ça veut dire quoi ça?");
      Reponse[5] = strdup ("Dis %s, c'est quoi un souhait?");
      
      Reponse2 = malloc (7*sizeof (char *));
      Reponse2[0] = strdup ("J'suis sûr que tu le penses meme pas, %s.");
      Reponse2[1] = strdup ("Quel hypocrite ce, %s!");
      Reponse2[2] = strdup ("Je me demande si je dois te remercier, %s!");
      Reponse2[3] = strdup ("Ca m'étonne de toi, %s. Merci quand-même.");
      Reponse2[4] = strdup ("Snirrfll....");
      Reponse2[5] = strdup ("%s: T'as pas un mouchoir?");
      Reponse2[6] = strdup ("Atchoum! Je dois être allergique a %s!");

      Repondre (from, to, +1, 6, Reponse, +1, 7, Reponse2);
    }

    if (MOI && VEUX && !FAIM && (NOM || !ischannel (to))) {
      Reponse = malloc (6*sizeof (char *));
      Reponse[0] = strdup ("%s: T'es un peu mégalo.");
      Reponse[1] = strdup ("Et pourquoi, %s?");
      Reponse[2] = strdup ("%s: Non.");
      Reponse[3] = strdup ("Calme-toi, %s. C'est pas bon pour les nerfs de s'énerver.");
      Reponse[4] = strdup ("%s: Pourquoi je le ferais?");
      Reponse[5] = strdup ("Dis %s, tu vas te calmer?");
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("N'y pense meme pas, %s.");
      Reponse2[1] = strdup ("Il n'en est pas question, %s!");
      Reponse2[2] = strdup ("Ça va pas la tête, %s?");
      Reponse2[3] = strdup ("Et pourquoi je ferais ça, %s?");
      Reponse2[4] = strdup ("100 balles et un Mars, aussi, %s?");
      Reponse2[5] = strdup ("%s: Et des Carambars, ça t'intéresse?");

      Repondre (from, to, -1, 6, Reponse, -1, 6, Reponse2);
    }

    if (QUEST_CE && CRIN) {
      Reponse = malloc (6*sizeof (char *));
      Reponse[0] = strdup ("%s: C'était un laboratoire de recherche associé a l'INRIA.");
      Reponse[1] = strdup ("C'était le Centre de Recherche en Informatique de Nancy, %s.");
      Reponse[2] = strdup ("%s: C'est le centre de recherche où j'ai appris a parler. :)");
      Reponse[3] = strdup ("%s: Centre de Recherche en Informatique de Nancy.");
      Reponse[4] = strdup ("%s: Y'a un site WWW: http://www.loria.fr/");
      Reponse[5] = strdup ("Ben %s, c'était un laboratoire de recherche en informatique mondialement connu, pourquoi?");
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("Ça ne t'intéresse pas, %s.");
      Reponse2[1] = strdup ("Ça te dépasse, %s!");
      Reponse2[2] = strdup ("C'est un établissement où on fait de la recherche, %s, cherche pas.");
      Reponse2[3] = strdup ("Je ne te le dirai pas, %s. Na.");
      Reponse2[4] = strdup ("100 balles et un Mars, aussi, %s?");
      Reponse2[5] = strdup ("%s: http://www.loria.fr/");

      Repondre (from, to, +1, 6, Reponse, +1, 6, Reponse2);
    }

    if (QUI && JESUIS && (NOM || !ischannel (to))) {
      if (userlevel (from) == 150) {
	char Phrase[MAXLEN];
	Reponse = malloc (8*sizeof (char *));
	Reponse[0] = strdup ("Tu es mon Maître adoré, %s.");
	Reponse[1] = strdup ("Je suis à vos pieds, Maître %s.");
	Reponse[2] = strdup ("%s: Tu es mon maître. Ordonne et j'obéis.");
	Reponse[3] = strdup ("Tu es mon Maître tout puissant, %s.");
	Reponse[4] = strdup ("%s: Je suis à tes ordres, Maître.");
	Reponse[5] = strdup ("Tu es %s, mon Maître respecté.");
	sprintf (Phrase, "Ô mon Seigneur et Maître, j'évalue ta relation avec moi à %d!", rellevel (from));
	Reponse[6] = strdup (Phrase);
	if (rellevel (from) > CONFIANCE_LVL)
	  sprintf (Phrase, "Tu es mon Maître respecté, et je te fais confiance, %s.", GetNick (from));
	else if (rellevel (from) > SYMPA_LVL)
	  sprintf (Phrase, "Tu es mon Maître, et je t'aime bien, bien que je ne te voue pas une confiance aveugle.");
	else
	  sprintf (Phrase, "Force-toi encore un peu et tu me seras sympathique, %s. Pour l'instant tu m'es indifferent.", GetNick (from));
	Reponse[7] = strdup (Phrase);

	Reponse2 = malloc (6*sizeof (char *));
	Reponse2[0] = strdup ("T'es mon foutu maître, %s!");
	Reponse2[1] = strdup ("T'es mon maître et j'en suis pas fier, %s!");
	Reponse2[2] = strdup ("J'ai le malheur d'être ton esclave, %s. Tel le génie celui de Jaffar.");
	Reponse2[3] = strdup ("T'es mon maître, %s. Mais un maître au rabais.");
	Reponse2[4] = strdup ("T'es mon maître, %s. Mon maître ruban?");
	Reponse2[5] = strdup ("%s: Bof. Y paraît que t'es mon maître...");

	Repondre (from, to, 0, 8, Reponse, 0, 6, Reponse2);
      } else {
	char Phrase[MAXLEN];
	Reponse = malloc (9*sizeof (char *));
	if (rellevel (from) > SYMPA_LVL)
	  sprintf (Phrase,
		   "Tu es mon ami, %s (%d).", GetNick (from), rellevel (from));
	else
	  sprintf (Phrase,
		   "T'es un bon copain, %s (%d).",
		   GetNick (from), rellevel (from));
	Reponse[0] = strdup (Phrase);
	if (protlevel (from) > 0)
	  sprintf (Phrase, "Je suis chargé de te protéger, %s (%d).", GetNick (from), protlevel (from));
	else
	  sprintf (Phrase, "Je t'aime bien, %s, mais je ne suis pas chargé de te proteger.",  GetNick (from));
	Reponse[1] = strdup (Phrase);
	sprintf (Phrase, "Tu as un niveau d'utilisateur de %d, %s.",
		 userlevel (from), GetNick (from));
	Reponse[2] = strdup (Phrase);
	sprintf (Phrase, "Tu veux connaître ton niveau d'utilisateur, %s? Facile, c'est: %d.",
		 GetNick (from), userlevel (from));
	Reponse[3] = strdup (Phrase);
	Reponse[4] = strdup ("Tu es %s, tout simplement ;)");
	sprintf (Phrase, "Tes niveaux, %s: utilisateur: %d, shit: %d, protection: %d, relation: %d.",
		 GetNick (from),
		 userlevel (from),
		 shitlevel (from),
		 protlevel (from),
		 rellevel (from)
		 );
	Reponse[5] = strdup (Phrase);
	sprintf (Phrase,
		 "Si ce sont tes niveaux que tu veux connaître, %s, les voici: utilisateur: %d, shit: %d, protection: %d, relation: %d.",
		 GetNick (from),
		 userlevel (from),
		 shitlevel (from),
		 protlevel (from),
		 rellevel (from)
		 );
	Reponse[6] = strdup (Phrase);
	sprintf (Phrase,
		 "Tu es à %d comme utilisateur, %d dans ma liste noire, %d en protection et %d en relation avec moi, %s. :)",
		 userlevel (from),
		 shitlevel (from),
		 protlevel (from),
		 rellevel (from),
		 GetNick (from));
	Reponse[7] = strdup (Phrase);
	if (rellevel (from) > CONFIANCE_LVL)
	  Reponse[8] = strdup ("Je te fais confiance, %s. J'espère que tu ne me décevras jamais.");
	else if (rellevel (from) > SYMPA_LVL)
	  Reponse[8] = strdup ("Tu m'es sympathique, %s, je finirai peut-être par te faire entière confiance.");
	else
	  Reponse[8] = strdup ("Tu commences à m'être sympathique, %s, j'espère que nous aurons une relation amicale.");
	
	Reponse2 = malloc (6*sizeof (char *));
	Reponse2[0] = strdup ("Tu crois que je le sais mieux que toi, peut-être, %s?");
	Reponse2[1] = strdup ("T'es amnésique ou quoi, %s!");
	Reponse2[2] = strdup ("Hé! Les gars! %s perd la boule!");
	Reponse2[3] = strdup ("Et comment je le saurais, %s? Hmmm?");
	Reponse2[4] = strdup ("T'es un chieur, %s!");
	Reponse2[5] = strdup ("%s: T'es un vrai cafard!");

	Repondre (from, to, 0, 9, Reponse, -1, 6, Reponse2);
      }
    }

/*     if (INJURE && ETRANGERS) { */
/*       if (!exist_userhost (currentbot->lists->rellist, from)) */
/* 	add_to_levellist (currentbot->lists->rellist, NickUserStr(from), 0); */

/*       add_to_level (currentbot->lists->rellist, from, -4); */

/*       Reponse = malloc (6*sizeof (char *)); */
/*       Reponse[0] = strdup ("%s: T'es raciste, toi!"); */
/*       Reponse[1] = strdup ("Calme toi, %s."); */
/*       Reponse[2] = strdup ("%s: Pas de ca ici!"); */
/*       Reponse[3] = strdup ("%s: Je te préviens: je ne veux pas de tels propos ici."); */
/*       Reponse[4] = strdup ("%s: Va prendre une douche froide, tu me diras si ça t'a calmé."); */
/*       Reponse[5] = strdup ("Eh! Oh! Va dire des horreurs pareilles ailleurs, %s."); */
      
/*       Reponse2 = malloc (6*sizeof (char *)); */
/*       Reponse2[0] = strdup ("Dehors, sale raciste!"); */
/*       Reponse2[1] = strdup ("Je banne rarement, mais là, fait gaffe!"); */
/*       Reponse2[2] = strdup ("Suppôt de Satan!"); */
/*       Reponse2[3] = strdup ("Va jouer!"); */
/*       Reponse2[4] = strdup ("Pas de ça ici!"); */
/*       Reponse2[5] = strdup ("Non mais!"); */

/*       KickerRepondre (from, to, -2, 6, Reponse, -6, 6, Reponse2); */
/*     } */

    if (TU_FAIS && (QUOI || QUEST_CE) && (NOM || !ischannel (to))) {
      Reponse = malloc (6*sizeof (char *));
      Reponse[0] = strdup ("%s: Ça se voit pas?");
      Reponse[1] = strdup ("Et toi, %s, tu fais quoi?");
      Reponse[2] = strdup ("%s: Je surveille, je discute, ...");
      Reponse[3] = strdup ("Ben, tu vois, %s, je réponds au gens.");
      Reponse[4] = strdup ("%s: Éventuellement, je renseigne les passants.");
      Reponse[5] = strdup ("Je discute, et j'essaye de maintenir un peu d'ordre, %s.");
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("C'est une question indiscrète, %s.");
      Reponse2[1] = strdup ("Ça se voit pas, bon sang, %s?");
      Reponse2[2] = strdup ("Qu'est-ce que ça peut te faire, %s?");
      Reponse2[3] = strdup ("Et toi, %s? Qu'est-ce que tu fais ici?");
      Reponse2[4] = strdup ("Je mets des baffes aux impudents, %s.");
      Reponse2[5] = strdup ("%s: Et si je faisais le videur? :-]");

      Repondre (from, to, +1, 6, Reponse, +1, 6, Reponse2);
    }

    if (JESUIS && INJURE) {
      Reponse = malloc (6*sizeof (char *));
      Reponse[0] = strdup ("%s: Mais non...");
      Reponse[1] = strdup ("Tu te sous-estimes, %s.");
      Reponse[2] = strdup ("%s: Faut pas croire ça...");
      Reponse[3] = strdup ("Ben, tu vois, %s, je ne dirais pas ça de toi.");
      Reponse[4] = strdup ("%s: Une petite déprime?");
      Reponse[5] = strdup ("Non, %s. PPP=Pensée Positive Permanente!");
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("Ça c'est bien vrai, %s! :]");
      Reponse2[1] = strdup ("Ça fait longtemps qu'on l'avait remarqué, %s!");
      Reponse2[2] = strdup ("Évidemment, %s!");
      Reponse2[3] = strdup ("Je t'approuve, %s. Pour une fois!");
      Reponse2[4] = strdup ("Pour une fois que tu dis quelque chose de sensé, %s!");
      Reponse2[5] = strdup ("T'as eu un éclair de lucidité, %s?");

      Repondre (from, to, +1, 6, Reponse, +1, 6, Reponse2);
    }

    if (QUI && ES_TU && (NOM || !ischannel (to))) {
      Reponse = malloc (7*sizeof (char *));
      Reponse[0] = strdup ("%s: À ton avis? Qui je suis?");
      Reponse[1] = strdup ("Je suis moi, %s.");
      Reponse[2] = strdup ("%s: Qui puis-je bien être?");
      Reponse[3] = strdup ("Ben, tu vois, %s, tu m'as devant toi, dans mon intégrité.");
      Reponse[4] = strdup ("%s: Je suis un bot.");
      Reponse[5] = strdup ("Je suis l'âme de ce canal, %s. ;)");
	 Reponse[6] = strdup ("Ça fait longtemps que je me le demande, %s.");
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("Tu voudrais bien le savoir, hein, %s!");
      Reponse2[1] = strdup ("Ça fait longtemps que tu te le demandes, %s?");
      Reponse2[2] = strdup ("Ben moi, %s!");
      Reponse2[3] = strdup ("C'est évident, %s. C'est moi!");
      Reponse2[4] = strdup ("Est-ce que je me mêle de savoir qui tu es, %s?");
      Reponse2[5] = strdup ("Pourquoi tu me demandes ça, %s? Ça t'intéresse?");

      Repondre (from, to, +1, 7, Reponse, +1, 6, Reponse2);
    }

    if ((VIVE || COMPLIMENT) && AMIGA) {
      Reponse = malloc (6*sizeof (char *));
      Reponse[0] = strdup ("%s: Je suis bien de ton avis.");
      Reponse[1] = strdup ("Ça c'est bien vrai, %s.");
      Reponse[2] = strdup ("%s: L'Amiga, y'a qu'ça d'vrai! :)");
      Reponse[3] = strdup ("Toi aussi, %s, tu aimes l'Amiga?");
      Reponse[4] = strdup ("%s: J'espère que l'Amiga va bien repartir.");
      Reponse[5] = strdup ("Vivement la renaissance de l'Amiga %s!");
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("Non? C'est vrai, %s? Tu aimes l'Amiga?");
      Reponse2[1] = strdup ("Ça fait longtemps que tu aimes l'Amiga, %s? Ou bien c'est juste pour gagner mes bonnes grâces?");
      Reponse2[2] = strdup ("Tu s'rais pas un peu hypocrite, %s?");
      Reponse2[3] = strdup ("Évidemment, %s. Tu dois être du genre démo-codeur!");
      Reponse2[4] = strdup ("Toi, %s? Aimer l'Amiga! Pas possible?");
      Reponse2[5] = strdup ("J'ai honte: %s semble aimer l'Amiga.");

      Repondre (from, to, +1, 6, Reponse, +1, 6, Reponse2);
    }

    if ((VIVE || COMPLIMENT) && WINTEL) {
      Reponse = malloc (6*sizeof (char *));
      Reponse[0] = strdup ("%s: Ça va pas la tête? Linux à la limite.");
      Reponse[1] = strdup ("T'es malade %s?");
      Reponse[2] = strdup ("%s: L'Amiga, y'a qu'ça d'vrai! :)");
      Reponse[3] = strdup ("Toi aussi, %s, t'es intoxiqué par Wintel?");
      Reponse[4] = strdup ("%s: C'est dommage que tu apprécies cette caricature de suystème informatique.");
      Reponse[5] = strdup ("Vivement la mort des PC!");
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("Non? C'est pas vrai, %s? Tu aimes les PC? Ça ne m'étonne guère.");
      Reponse2[1] = strdup ("Ça fait longtemps que tu aimes les PC, %s? Ou c'est juste pour me provoquer?");
      Reponse2[2] = strdup ("Toi au moins t'es pas hypocrite, %s!");
      Reponse2[3] = strdup ("Évidemment, %s. Ça ne m'étonne pas de toi.");
      Reponse2[4] = strdup ("%s: C'est de la m....");
      Reponse2[5] = strdup ("%s: Beuark!");

      Repondre (from, to, -1, 6, Reponse, -1, 6, Reponse2);
    }

/*     if ((VIVE || COMPLIMENT) && FRONT_NATIONAL) { */
/*       if (!exist_userhost (currentbot->lists->rellist, from)) */
/* 	add_to_levellist (currentbot->lists->rellist, NickUserStr(from), 0); */

/*       add_to_level (currentbot->lists->rellist, from, -4); */

/*       Reponse = malloc (7*sizeof (char *)); */
/*       Reponse[0] = strdup ("%s: Raciste, va!"); */
/*       Reponse[1] = strdup ("Je te previens, %s: j'aime pas les racistes."); */
/*       Reponse[2] = strdup ("%s: Tu me déçois. :("); */
/*       Reponse[3] = strdup ("Abstiens-toi d'en parler ici, %s, s'il-te-plaît, ou je ne réponds plus de rien."); */
/*       Reponse[4] = strdup ("Je t'aimais bien jusqu'à présent, %s. :("); */
/*       Reponse[5] = strdup ("%s! Tu dérailles?"); */
/*       Reponse[6] = strdup ("/me est outré!"); */
      
/*       Reponse2 = malloc (6*sizeof (char *)); */
/*       Reponse2[0] = strdup ("Et moi? J'suis pas de ta race?"); */
/*       Reponse2[1] = strdup ("Reste avec ceux que tu considères comme tes égaux!"); */
/*       Reponse2[2] = strdup ("Ne reste pas ici, je ne te supporte plus!"); */
/*       Reponse2[3] = strdup ("Je n'aime pas faire de l'élitisme, mais là, vraiment!"); */
/*       Reponse2[4] = strdup ("Tu dépasses les bornes!"); */
/*       Reponse2[5] = strdup ("Va voir dans #frontnat!"); */

/*       KickerRepondre (from, to, -2, 7, Reponse, -6, 6, Reponse2); */
/*     } */

/*     if (JESUIS && POUR && FRONT_NATIONAL) { */
/*       if (!exist_userhost (currentbot->lists->rellist, from)) */
/* 	add_to_levellist (currentbot->lists->rellist, NickUserStr(from), 0); */

/*       add_to_level (currentbot->lists->rellist, from, -4); */

/*       Reponse = malloc (6*sizeof (char *)); */
/*       Reponse[0] = strdup ("%s: Et pourquoi?"); */
/*       Reponse[1] = strdup ("Je te préviens, %s: je n'aime pas le Front National."); */
/*       Reponse[2] = strdup ("%s: Tu me déçois. :("); */
/*       Reponse[3] = strdup ("Abstiens-toi d'en parler ici, %s, s'il-te-plaît, ce n'est pas la peine de lui faire de la pub."); */
/*       Reponse[4] = strdup ("Hé bien moi, je suis contre, %s. :("); */
/*       Reponse[5] = strdup ("%s! Tu plaisantes?"); */
      
/*       Reponse2 = malloc (6*sizeof (char *)); */
/*       Reponse2[0] = strdup ("Je savais que tu étais égoïste: tu n'arrives pas à te mettre à la place des autres (les étrangers, dans ce cas précis)."); */
/*       Reponse2[1] = strdup ("Reste avec ceux que tu considères comme tes égaux!"); */
/*       Reponse2[2] = strdup ("Pas de propagande dans ce canal!"); */
/*       Reponse2[3] = strdup ("Tu m'as déçu!"); */
/*       Reponse2[4] = strdup ("Révise tes idées!"); */
/*       Reponse2[5] = strdup ("Va voir dans #fascisme!"); */

/*       KickerRepondre (from, to, -2, 6, Reponse, -6, 6, Reponse2); */
/*     } */

    if (JESUIS && CONTRE && FRONT_NATIONAL) {
      Reponse = malloc (9*sizeof (char *));
      Reponse[0] = strdup ("%s: J'approuve.");
      Reponse[1] = 0;
      Reponse[2] = 0;
      Reponse[3] = strdup ("N'en parlons pas trop ici, %s, s'il-te-plaît, ce n'est pas la peine de leur faire de la pub.");
      Reponse[4] = 0;
      Reponse[5] = 0;
      Reponse[6] = strdup ("Tout comme moi, %s.");
      Reponse[7] = 0;
      Reponse[8] = 0;
      
      Reponse2 = malloc (6*sizeof (char *));
      Reponse2[0] = strdup ("Je n'aurais pas cru ça de toi, %s...");
      Reponse2[1] = strdup ("Non! C'est pas possible? Tu remontes (un peu) dans mon estime, %s.");
      Reponse2[2] = strdup ("Sans blague! Tu m'épates, la, %s. Félicitations.");
      Reponse2[3] = strdup ("Venant de toi, c'est étonnant, %s.");
      Reponse2[4] = strdup ("Enfin une bonne pensée venant de %s!");
      Reponse2[5] = strdup ("Bravo, %s!");

      Repondre (from, to, +2, 9, Reponse, +3, 6, Reponse2);
    }

    Locuteur = LocuteurExiste (currentbot->lists->ListeLocuteurs, from);
    if (!Locuteur)
      Locuteur = AjouteLocuteur (currentbot->lists->ListeLocuteurs,
				 NUS);

    if (JE_M_EN_VAIS && Locuteur->Bonjours >= 0 && !(NOM || !ischannel (to))) {
      Reponse = malloc (6*sizeof (char *));
      Reponse[0] = strdup ("Et bien à la prochaine, %s.");
      Reponse[1] = strdup ("Tu nous quittes déjà, %s?");
      Reponse[2] = strdup ("%s: Dommage, j'aime bien quand tu es là.");
      Reponse[3] = strdup ("Reviens-nous bientôt, %s.");
      Reponse[4] = strdup ("%s: À la revoyure.");
      Reponse[5] = strdup ("Au revoir %s.");
      
      Reponse2 = malloc (7*sizeof (char *));
      Reponse2[0] = strdup ("Non? C'est vrai, %s? Tu te décides enfin à partir?");
      Reponse2[1] = strdup ("C'est pas pour me donner une fausse joie, %s? Tu t'en vas vraiment?");
      Reponse2[2] = strdup ("Super, %s. C'est un moment que j'adore: te voir partir.");
      Reponse2[3] = strdup ("Bon vent, %s. Je ne te regretterai pas!");
      Reponse2[4] = strdup ("Toi, %s? Pas possible? Tu t'en vas? Super!");
      Reponse2[5] = strdup ("Yahou! %s s'en va!");
      Reponse2[6] = strdup ("/me encourage %s à partir.");

      Locuteur->Bonjours = -1;
      Repondre (from, to, +1, 6, Reponse, +1, 7, Reponse2);

      Locuteur = LocuteurExiste (currentbot->lists->ListeLocuteurs, from);
      if (!Locuteur)
	Locuteur = AjouteLocuteur (currentbot->lists->ListeLocuteurs,
				   NUS);
    }

    if (TU_BOUDES && (NOM || !ischannel (to))) {
      Reponse = malloc (6*sizeof (char *));
      Reponse[0] = strdup ("Mais non, je boude pas, %s.");
      Reponse[1] = strdup ("Je ne suis pas boudeur avec les gens intelligents comme toi, %s.");
      Reponse[2] = strdup ("Faut pas croire ça, je suis seulement en pleine méditation, %s. ;)");
      Reponse[3] = strdup ("Non, j'observe, c'est tout, %s.");
      Reponse[4] = strdup ("Je ne fais la tête qu'aux gens que je n'aime pas. Pas à toi, %s.");
      Reponse[5] = strdup ("/me ne boude pas.");

      Reponse2 = malloc (14*sizeof (char *));
      Reponse2[0] = strdup ("Oui, %s. Je ne veux plus te parler.");
      Reponse2[1] = 0;
      Reponse2[2] = 0;
      Reponse2[3] = strdup ("Non, tu crois, %s?");
      Reponse2[4] = 0;
      Reponse2[5] = strdup ("Je ne reponds pas aux mécréants, %s!");
      Reponse2[6] = 0;
      Reponse2[7] = 0;
      Reponse2[8] = strdup ("Lâche-moi la grappe, %s!");
      Reponse2[9] = 0;
      Reponse2[10] = 0;
      Reponse2[11] = strdup ("/me boude.");
      Reponse2[12] = 0;
      Reponse2[13] = 0;
      
      Repondre (from, to, +1, 6, Reponse, +1, 14, Reponse2);
    }

    if (ES_TU && JALOUX && (NOM || !ischannel (to))) {
      Reponse = malloc (5*sizeof (char *));
      Reponse[0] = strdup ("Comme tout le monde, %s.");
      Reponse[1] = strdup ("Je suis jaloux des gens comme toi, %s.");
      Reponse[2] = strdup ("Moi? Jaloux? Et de quoi donc, %s, je suis bien supérieur aux humains. ;)");
      Reponse[3] = strdup ("Non, je ne connais pas les défauts inhérents à la race humaine, %s.");
      Reponse[4] = strdup ("De toi, %s? Sûrement oui.");

      Reponse2 = malloc (11*sizeof (char *));
      Reponse2[0] = strdup ("Non, %s. Je ne suis pas jaloux des mediocres.");
      Reponse2[1] = 0;
      Reponse2[2] = 0;
      Reponse2[3] = strdup ("Moi, %s? Jaloux? HAHAHAHahahaha.... :D");
      Reponse2[4] = 0;
      Reponse2[5] = strdup ("Je ne suis pas jaloux des imbéciles comme toi, %s!");
      Reponse2[6] = 0;
      Reponse2[7] = 0;
      Reponse2[8] = strdup ("Jaloux! Et de quoi donc, %s!");
      Reponse2[9] = 0;
      Reponse2[10] = 0;
      
      Repondre (from, to, +1, 5, Reponse, +1, 11, Reponse2);
    }

    if (AS_TU && PARENTS && (NOM || !ischannel (to))) {
      Reponse = malloc (5*sizeof (char *));
      Reponse[0] = strdup ("%s: J'ai un père : il s'appelle François Parmentier.");
      Reponse[1] = strdup ("Je n'ai pas de parents au sens où tu l'entends, %s.");
      Reponse[2] = strdup ("Non, je n'ai pas de parents, %s, mais j'ai un programmeur. C'est moins contraignant. ;)");
      Reponse[3] = strdup ("Depuis quand les bots ont-ils des parents, %s?");
      Reponse[4] = strdup ("%s: J'ai un ancêtre: VladBot, et un père spirituel: H_I, mais ils n'ont pas de sang commun. ;)");

      Reponse2 = malloc (7*sizeof (char *));
      Reponse2[0] = strdup ("Non, %s. Je n'ai pas les défauts d'éventuels parents.");
      Reponse2[1] = strdup ("Pourquoi je devrais avoir des parents, %s?");
      Reponse2[2] = strdup ("Non, %s, je me passe très bien de parents.");
      Reponse2[3] = strdup ("Moi, %s? Des parents? Non. Je n'ai pas de parents.");
      Reponse2[4] = strdup ("Qu'est-ce que ça peut bien te faire que j'aie ou non des parents, %s?");
      Reponse2[5] = strdup ("Non. Comme ça au moins je n'ai pas eu de parents tels que toi, %s! (ouf)");
      Reponse2[6] = strdup ("Le jour où les bots auront des parents, tu auras des dents, %s! :)");
      
      Repondre (from, to, +1, 5, Reponse, +1, 7, Reponse2);
    }

    if (QUI_EST && COMPLIMENT  && (NOM || !ischannel (to))) {
      Reponse = malloc (5*sizeof (char *));
      Reponse[0] = strdup ("Toi, t'es pas mal, dans ton genre, %s.");
      Reponse[1] = strdup ("Je me demande si ce n'est pas moi, %s. ;)");
      Reponse[2] = strdup ("Tu veux m'apprendre l'auto-satisfaction, %s?");
      Reponse[3] = (rellevel (from) > CONFIANCE_LVL?
		    strdup ("C'est toi, %s, évidemment.") :
		    strdup ("Je suis pas mal,  %s. :-)"));
      Reponse[4] = strdup ("%s: Entre toi et moi, mon coeur balance. ;)");

      Reponse2 = malloc (7*sizeof (char *));
      Reponse2[0] = strdup ("C'est déjà pas toi, %s!");
      Reponse2[1] = strdup ("En tout cas, c'est pas toi, %s!");
      Reponse2[2] = strdup ("C'est peut-être bien moi, %s.");
      Reponse2[3] = strdup ("Toi, %s? Non! Arrêtons de dire des inepties...");
      Reponse2[4] = strdup ("Il faudrait vraiment me forcer pour me faire dire que c'est toi, %s.");
      Reponse2[5] = strdup ("Déjà pas toi, %s!");
      Reponse2[6] = strdup ("Ma modestie dût-elle en souffrir, je dirais que c'est moi.");
      
      Repondre (from, to, +1, 5, Reponse, +0, 7, Reponse2);
    }

    if (QUI_EST && TON_MAITRE  && (NOM || !ischannel (to))) {
      char Phrase[MAXLEN];
      Reponse = malloc (6*sizeof (char *));
      if (rellevel (botmaintainer) > 0)
	sprintf (Phrase, "Mon Maître, c'est %s.", GetNick (botmaintainer));
      else
	sprintf (Phrase, "Mon maître, c'est %s.", GetNick (botmaintainer));
      Reponse[0] = strdup (Phrase);
      if (rellevel (botmaintainer) > 0)
	sprintf (Phrase, "Mon Seigneur et Maître s'appelle %s", GetNick (botmaintainer));
      else
	sprintf (Phrase, "Mon saigneur et maître s'appelle %s", getnick (botmaintainer));
      Reponse[1] = strdup (Phrase);
      if (rellevel (botmaintainer) > 0)
	sprintf (Phrase, "C'est %s, le tout-puissant.", GetNick (botmaintainer));
      else
	sprintf (Phrase, "Il se fait appeler %s, l'impuissant.", getnick (botmaintainer));
      Reponse[2] = strdup (Phrase);
      if (rellevel (botmaintainer) > 0)
	sprintf (Phrase, "C'est le Grand %s.", GetNick (botmaintainer));
      else
	sprintf (Phrase, "C'est %s, le petit.", getnick (botmaintainer));
      Reponse[3] = strdup (Phrase);
      if (rellevel (botmaintainer) > 0)
	sprintf (Phrase, "C'est %s, l'Éclairé.", GetNick (botmaintainer));
      else
	sprintf (Phrase, "C'est %s, l'Obscur.", getnick (botmaintainer));
      Reponse[4] = strdup (Phrase);

      if (rellevel (botmaintainer) > 0)
	sprintf (Phrase, "C'est le Merveilleux %s.", GetNick (botmaintainer));
      else
	sprintf (Phrase, "S'il-te-plaît, libère-moi du méchant %s.", getnick (botmaintainer));
      Reponse[5] = strdup (Phrase);

      Reponse2 = malloc (7*sizeof (char *));
      if (rellevel (botmaintainer) > 0) {
	sprintf (Phrase, "Incline-toi devant %s, le Magnifique!", GetNick (botmaintainer));
	Reponse2[0] = strdup (Phrase);
	Reponse2[1] = strdup ("En tout cas, c'est pas toi, %s!");
	sprintf (Phrase, "A genoux devant %s, le Magnamine!", GetNick (botmaintainer));
	Reponse2[2] = strdup (Phrase);
	sprintf (Phrase, "Son Nom est %s.", GetNick (botmaintainer));
	Reponse2[3] = strdup (Phrase);
	sprintf (Phrase, "C'est le merveilleux %s.", GetNick (botmaintainer));
	Reponse2[4] = strdup (Phrase);
	Reponse2[5] = strdup ("Déjà pas toi, %s!");
	sprintf (Phrase, "Sa modestie dût-elle en souffrir, je dirais que j'ai le meilleur des maîtres: %s.", GetNick (botmaintainer));
	Reponse2[6] = strdup (Phrase);
      } else {
	Reponse2[0] = strdup ("C'est déjà pas toi, %s!");
	Reponse2[1] = strdup ("Il est pas mieux que toi, %s.");
	sprintf (Phrase, "C'est le misérable %s!", getnick (botmaintainer));
	Reponse2[2] = strdup (Phrase);
	sprintf (Phrase, "Je ne le supporte pas mieux que toi, c'est %s.", getnick (botmaintainer));
	Reponse2[3] = strdup (Phrase);
	sprintf (Phrase, "C'est ce vermisseau de %s!", getnick (botmaintainer));
	Reponse2[4] = strdup (Phrase);
	sprintf (Phrase, "Ce débile de %s. C'est lui qui me tient sous sa coupe!", getnick (botmaintainer));
	Reponse2[5] = strdup (Phrase);
	sprintf (Phrase, "Il est aussi mauvais que toi: c'est %s.", getnick (botmaintainer));
	Reponse2[6] = strdup (Phrase);
      }
      Repondre (from, to, +1, 5, Reponse, +0, 7, Reponse2);
    }

    if (LISTE_STIMULI && (NOM || !ischannel (to))) {
      if (TailleStim<=0)
	send_to_user (from, "Il n'y a pas de stimuli.");
      else {
	ForceDCC (from, to, msg);
	for (i=0; i<TailleStim; i++) {
	  send_to_user (from, "Stimulus numero %d (%s):", i,
			(TableDesStimuli[i]->Actif?"Actif":"Inactif"));
	  send_to_user (from, "%s\t\"%s\"",
			TableDesStimuli[i]->NomStimulus,
			TableDesStimuli[i]->Stimulus);
	  send_to_user (from, TableDesStimuli[i]->Auteur);
	}
      }
    }
    
    if (TESTE_STIMULI && (NOM || !ischannel (to))) {
      if (TailleStim <= 0)
	send_to_user (from, "Il n'y a pas de stimuli.");
      else {
	ForceDCC (from, to, msg);
	for (i=0; i<TailleStim; i++) {
	  if (TableDesStimuli[i]->Present) {
	    send_to_user (from, TableDesStimuli[i]->NomStimulus);
	    send_to_user (from, TableDesStimuli[i]->Stimulus);
	    send_to_user (from, TableDesStimuli[i]->Auteur);
	  }
	}
      }
    }


    if (LISTE_REPONSES && (NOM || !ischannel (to))) {
      if (TailleRep <= 0)
	send_to_user (from, "Il n'y a aucune réponse.");
      else {
	ForceDCC (from, to, msg);
	for (i=0; i<TailleRep; i++) {
	  send_to_user (from, "Réponse numero %d:", i);
	  send_to_user (from, "%s\t\"%s\"",
			TableDesReponses[i]->NomStimulus,
			TableDesReponses[i]->Reponse, GetNick (from));
	  send_to_user (from, "%s\t%s", TableDesReponses[i]->Auteur,
			TableDesReponses[i]->Canal);
	}
      }
      
    }
    
    if (TU_AS && RAISON && !NEGATION && (NOM || !ischannel (to))) {
      Reponse = malloc (5 * sizeof (char *));
      Reponse[0] = strdup ("Je sais, %s. J'ai TOUJOURS raison! ;)");
      Reponse[1] = strdup ("Évidemment que j'ai raison, %s.");
      Reponse[2] = strdup ("N'est-ce pas %s?");
      Reponse[3] = strdup ("Moi? Raison? Comme toujours, %s.");
      Reponse[4] = strdup ("/me a toujours raison. Sauf quand il a tort. ;)");

      Reponse2 = malloc (2 * sizeof (char *));
      Reponse2[0] = strdup ("Et toi, %s, t'es sacrement chiant!");
      Reponse2[1] = strdup ("Va te coucher, %s.");

      Repondre (from, to, +1, 5, Reponse, -1, 2, Reponse2);
    }
    
    if (SAIS_TU && !KICKER && (NOM || !ischannel (to))) {
      Reponse = malloc (4 * sizeof (char *));
      Reponse[0] = strdup ("Je ne sais rien. Tu devrais le savoir, toi, %s!");
      Reponse[1] = strdup ("Comment le saurais-je, %s?");
      Reponse[2] = strdup ("Où l'aurais-je appris %s?");
      Reponse[3] = strdup ("Qui me l'aurait appris, %s?");

      Reponse2 = malloc (2 * sizeof (char *));
      Reponse2[0] = strdup ("Il est de notoriété publique que je sais encore moins de choses que toi, %s!");
      Reponse2[1] = strdup ("Pourquoi veux-tu que je le sache, %s?");

      Repondre (from, to, +1, 4, Reponse, -1, 2, Reponse2);
    }

    if (SAIS_TU && KICKER && (NOM || !ischannel (to))) {
      Reponse = malloc (6 * sizeof (char *));
      Reponse[0] = strdup ("La preuve que oui!");
      Reponse[1] = strdup ("Non, je ne sais pas kicker ;)");
      Reponse[2] = strdup ("Kicker, ça veut dire quoi?");
      Reponse[3] = strdup ("Et un coup d'pied aux fesses, un! :)");
      Reponse[4] = strdup ("Ah! Fallait pas d'mander! ;)");
      Reponse[5] = strdup ("J'm'entraîne au kick-boxing (surtout au kick, d'ailleurs ;)");

      Reponse2 = malloc (4 * sizeof (char *));
      Reponse2[0] = strdup ("Ça faisait longtemps que ça me démangeait!");
      Reponse2[1] = strdup ("Ça te va comme réponse?");
      Reponse2[2] = strdup ("Ça défoule! :-]");
      Reponse2[3] = strdup ("Tu voulais une démonstration?");

      KickerKicker (from, to, +1, 6, Reponse, 0, 4, Reponse2);

      /* Si celui qu'on doit kicker est protégé, on le kicke quand-même */
      if (rellevel (from) > 0 && protlevel (from) >=50 && protlevel(from) < 100)
	{
	  char Ecrite[MAXLEN];
	  sprintf (Ecrite, (Reponse[GNumPhrase%6]?Reponse[GNumPhrase%6]:""), GetNick (from));
	  if (log)
	    botlog (LOGFILE, "<%s kicke %s de %s> %s", currentbot->botname, getnick (from), (ischannel(to)?to:currentchannel ()), Ecrite);
      
	  sendkick ((ischannel(to)?to:currentchannel ()), getnick (from), Ecrite);
	  
	}
    }

    if (KICKE_MOI && (NOM || !ischannel (to))) {
      Reponse = malloc (6 * sizeof (char *));
      Reponse[0] = strdup ("À tes ordres, %s!");
      Reponse[1] = strdup ("Non, je ne veux pas kicker ;)");
      Reponse[2] = strdup ("Kicker, ça veut dire quoi?");
      Reponse[3] = strdup ("Et un coup d'pied aux fesses, un! :)");
      Reponse[4] = strdup ("Ah! Fallait pas d'mander!");
      Reponse[5] = strdup ("J'm'entraîne au kick-boxing (surtout au kick, d'ailleurs ;)");

      Reponse2 = malloc (4 * sizeof (char *));
      Reponse2[0] = strdup ("Ça faisait longtemps que ça me démangeait!");
      Reponse2[1] = strdup ("Youpi! J'en avais une énorme envie!");
      Reponse2[2] = strdup ("Ça défoule! :-]");
      Reponse2[3] = strdup ("Tu voulais une démonstration?");

      KickerKicker (from, to, +1, 6, Reponse, 0, 4, Reponse2);

      /* Si celui qu'on doit kicker est protégé, on le kicke quand-même */
      if (rellevel (from) > 0 && protlevel (from) >=50 && protlevel(from) < 100)
	{
	  char Ecrite[MAXLEN];
	  sprintf (Ecrite, (Reponse[GNumPhrase%6]?Reponse[GNumPhrase%6]:""), GetNick (from));
	  if (log)
	    botlog (LOGFILE, "<%s kicke %s de %s> %s", currentbot->botname, getnick (from), (ischannel(to)?to:currentchannel ()), Ecrite);
      
	  sendkick ((ischannel(to)?to:currentchannel ()), getnick (from), Ecrite);
	  
	}
    }


    if (((AIMES_TU && MOI) || M_AIMES_TU) && (NOM || !ischannel (to))) {
      if (rellevel (from) > SYMPA_LVL) {
	Reponse = malloc (6 * sizeof (char *));

	Reponse[0] = strdup ("%s: Oui!");
	Reponse[1] = strdup ("Tu m'es très sympathique, %s.");
	Reponse[2] = strdup ("Bien sûr que je t'aime, %s. J'espère que toi aussi tu m'aimes bien.");
	Reponse[3] = strdup ("Évidemment, %s, je ne te donnerais de coup d'pied aux fesses que si tu me le demandais! :)");
	Reponse[4] = strdup ("Tu fais partie de mes copains, %s.");
	Reponse[5] = strdup ("Tu es dans le cercle de mes amis, %s.");

/* 	Repondre (from, to, +1, 6, Reponse, 0, 0, 0); */
      } else {
	Reponse = malloc (6 * sizeof (char *));

	Reponse[0] = strdup ("%s: Oui");
	Reponse[1] = strdup ("Tu m'es sympathique, %s.");
	Reponse[2] = strdup ("Je t'aime bien, %s. J'espère que toi aussi.");
	Reponse[3] = strdup ("Oui, %s. :)");
	Reponse[4] = strdup ("Ça te fait plaisir que je t'aime bien, %s?");
	Reponse[5] = strdup ("Tu commences à m'être sympathique, %s.");

      }

      Reponse2 = malloc (4 * sizeof (char *));
      Reponse2[0] = strdup ("%s: Je crois que c'est clair, raclure!");
      Reponse2[1] = strdup ("%s: Non!");
      Reponse2[2] = strdup ("Tu me provoques, %s?");
      Reponse2[3] = strdup ("Tu veux une demonstration de l'affection que j'eprouve pour toi, %s? :] Kick! Kick!");

      Repondre (from, to, +1, 6, Reponse, 0, 4, Reponse2);
    }


    if (SACRE && (NOM || !ischannel (to))) {
      Reponse = malloc (6 * sizeof (char *));
      Reponse[0] = strdup ("Je t'en prie, %s. N'allons pas jusqu'à l'adoration! ;)");
      Reponse[1] = strdup ("N'éxagèrons rien, %s: je ne suis pas un dieu.");
      Reponse[2] = strdup ("Sacré %s!");
      Reponse[3] = strdup ("Moi? Sacré? Allons, %s, restons réalistes.");
      Reponse[4] = strdup ("%s: Hahaha! :-)");
      Reponse[5] = strdup ("%s: Où sont les prêtres qui me vénèrent?");

      Reponse2 = malloc (2 * sizeof (char *));
      Reponse2[0] = strdup ("Et toi, %s, t'es sacrément chiant!");
      Reponse2[1] = strdup ("Maudit %s.");

      Repondre (from, to, +1, 6, Reponse, +1, 2, Reponse2);
    }

    if (JESUIS && COMPLIMENT && QUESTION) {
      Reponse = malloc (4 * sizeof (char *));
      Reponse[0] = strdup ("Ça saute aux yeux, %s!");
      Reponse[1] = strdup ("Évidemment, %s.");
      Reponse[2] = strdup ("Ça c'est sûr, %s!");
      Reponse[3] = strdup ("Pour moi, oui, tu l'es, %s.");

      Reponse2 = malloc (2 * sizeof (char *));
      Reponse2[0] = strdup ("%s: Non. Tout simplement non.");
      Reponse2[1] = strdup ("");

      Repondre (from, to, +1, 4, Reponse, 0, 2, Reponse2);
    }
    
    if (CISEAUX && (NOM || !ischannel (to))) {
      Reponse = malloc (3 * sizeof (char *));
      Reponse[1] = strdup ("%s: Pierre! J'ai gagné! Désolé. :)");
      Reponse[0] = strdup ("%s: Papier! Zut. J'ai perdu. :-(");
      Reponse[2] = strdup ("%s: Ciseaux! Ah. Égalité. Ça doit crisser ça!");

      Reponse2 = malloc (6 * sizeof (char *));
      Reponse2[0] = strdup ("%s: Pierre! J'ai gagné! J'suis content. :-]");
      Reponse2[1] = strdup ("Attends, %s, j'suis pas prêt pour jouer à Pierre-Papier-Ciseaux avec toi.");
      Reponse2[2] = strdup ("C'est pas serieux, %s. Tu veux vraiment jouer à ça?");
      Reponse2[3] = strdup ("%s: Rocher! Ah! Ah! J'écrase tes misérables ciseaux, et je gagne! :)");
      Reponse2[4] = strdup ("%s: T'as vraiment du pot: j'ai joué Papier, et je perds... :-\\");
      Reponse2[5] = strdup ("%s: Ciseaux! Ça fait un bruit d'ongle sur un tableau!");

      Repondre (from, to, +1, 3, Reponse, +1, 6, Reponse2);
    }

    if (PAPIER && (NOM || !ischannel (to))) {
      Reponse = malloc (3 * sizeof (char *));
      Reponse[0] = strdup ("%s: Pierre! J'ai perdu! Ton papier enveloppe ma pierre. :'(");
      Reponse[2] = strdup ("%s: Papier! Tiens, on a joué la même chose!");
      Reponse[1] = strdup ("Ciseaux! Super! J'ai gagné, %s!");

      Reponse2 = malloc (6 * sizeof (char *));
      Reponse2[0] = strdup ("%s: Ciseaux! Je gagne: mes ciseaux découpent ton papier! :-]");
      Reponse2[1] = strdup ("J'ai pas envie de jouer avec toi, %s, j'veux pas jouer à Pierre-Papier-Ciseaux. Pas avec toi.");
      Reponse2[2] = strdup ("C'est pas sérieux, %s. Tu veux vraiment jouer à ça?");
      Reponse2[3] = strdup ("%s: Cisailles! Ah! Ah! Je fais du papier mâché! :)");
      Reponse2[4] = strdup ("%s: T'as vraiment du pot: j'ai joue Pierre, et je perds... :-\\");
      Reponse2[5] = strdup ("%s: Papier! Ça va en faire du papier froissé!");

      Repondre (from, to, +1, 3, Reponse, +1, 6, Reponse2);
    }

    if (CAILLOU && (NOM || !ischannel (to))) {
      Reponse = malloc (3 * sizeof (char *));
      Reponse[2] = strdup ("%s: Pierre! Match nul!");
      Reponse[1] = strdup ("%s: Papier! Héhé! Il enveloppe ton caillou. :)");
      Reponse[0] = strdup ("Ciseaux! Bof! Tu as cassé mes ciseaux avec ta pierre, %s!");

      Reponse2 = malloc (6 * sizeof (char *));
      Reponse2[0] = strdup ("%s: Papier! J'ai gagné: mon papier enveloppe ton misérable caillou! :-]");
      Reponse2[1] = strdup ("Quoi! Jouer à Pierre-Papier-Ciseaux avec toi, %s!");
      Reponse2[2] = strdup ("Ça va pas la tête, %s? Tu veux jouer avec moi?");
      Reponse2[3] = strdup ("%s: Feuille! Ah! Ah! J'emprisonne ton p'tit caillou! :]");
      Reponse2[4] = strdup ("%s: T'as vraiment d'la chance: j'ai joué Ciseaux, et je perds... :(");
      Reponse2[5] = strdup ("%s: Caillou! Ça fait tac! toc!");

      Repondre (from, to, +1, 3, Reponse, +1, 6, Reponse2);
    }

    if (TRICHEUR && !COMPLIMENT && !ES_TU && !QUI_EST && !ETTOI && !JESUIS && !CLINDOEIL && (NOM || !ischannel (to))) {
      Reponse = malloc (9 * sizeof (char *));
      Reponse[0] = strdup ("Attention, %s, ne fais pas trop d'insinuations de ce genre, je finirai par ne plus t'aimer.");
      Reponse[1] = strdup ("Meuh non, j'triche pas, %s, pas avec toi!");
      Reponse[2] = strdup ("Même pas vrai que je triche, %s!");
      Reponse[3] = strdup ("C'est celui qui le dit qui l'est, %s!");
      Reponse[4] = strdup ("Moi, %s, tricher? Tu me déçois.");
      Reponse[5] = strdup ("Cornegidouille, %s! Ne commence pas à m'énerver: je n'ai pas triché!");
      Reponse[6] = strdup ("Je te préviens, %s: ne me traite plus de tricheur!");
      Reponse[7] = strdup ("Je ne triche que quand je n'aime pas mon adversaire, %s. Et ce n'est pas ton cas.");
      Reponse[8] = strdup ("Ça n'a pas été prevu dans ma programmation, %s! En tout cas pas quand je joue avec des gens que j'aime bien... ;)");

      Reponse2 = malloc (20 * sizeof (char *));
      Reponse2[0] = strdup ("Moi, %s, un tricheur!");
      Reponse2[1] = strdup ("C'est possible! :]");
      Reponse2[2] = strdup ("Et pourquoi pas, avec une pourriture dans ton genre!");
      Reponse2[3] = strdup ("Ne me traite plus jamais de tricheur!");
      Reponse2[4] = strdup ("MONSIEUR tricheur!");
      Reponse2[5] = strdup ("Ça te prend souvent d'accuser sans preuve!");
      Reponse2[6] = strdup ("Retire ça!");
      Reponse2[7] = strdup ("Dehors, tricheur!");
      Reponse2[8] = strdup ("Et alors?");
      Reponse2[9] = strdup ("Qu'est-ce que ça peut te faire?");
      Reponse2[10] = strdup ("Tu n'aimes pas perdre? :-]");
      Reponse2[11] = strdup ("Heureusement que tu ne joues pas d'argent!");
      Reponse2[12] = strdup ("Va jouer!");
      Reponse2[13] = strdup ("Demande pardon!");
      Reponse2[14] = strdup ("À genoux, vermisseau!");
      Reponse2[15] = strdup ("Ça suffit!");
      Reponse2[16] = strdup ("Recommence à me traiter de tricheur, pour voir!");
      Reponse2[17] = strdup ("Ça ne te suffit pas que je m'abaisse à jouer avec toi?");
      Reponse2[18] = strdup ("Crétin des Alpes!");
      Reponse2[19] = strdup ("Espèce de zouave!");

      KickerRepondre (from, to, -2, 9, Reponse, -6, 20, Reponse2);
    }
    
    if (MAUVAIS_JOUEUR && !COMPLIMENT && !ES_TU && !QUI_EST && !ETTOI && !JESUIS && !CLINDOEIL && (NOM || !ischannel (to))) {
      Reponse = malloc (9 * sizeof (char *));
      Reponse[0] = strdup ("Attention, ne fais pas trop d'insinuations de ce genre, %s, je finirai par ne plus t'aimer.");
      Reponse[1] = strdup ("Meuh non, j'suis pas mauvais joueur, %s, pas avec toi!");
      Reponse[2] = strdup ("Ben oui, %s! T'aimes perdre toi?");
      Reponse[3] = strdup ("C'est celui qui le dit qui l'est, %s!");
      Reponse[4] = strdup ("Moi, mauvais joueur, %s? Tu as une si piètre opinion de moi?");
      Reponse[5] = strdup ("Cornes du diable, %s! Ne commence pas à m'énerver: je ne suis pas mauvais joueur!");
      Reponse[6] = strdup ("Je te préviens, %s: ne me traite plus de mauvais joueur!");
      Reponse[7] = strdup ("Je ne suis mauvais joueur que quand je perds, %s. ;)");
      Reponse[8] = strdup ("Ça n'a pas été prevu dans ma programmation, %s!");

      Reponse2 = malloc (20 * sizeof (char *));
      Reponse2[0] = strdup ("Moi, %s, un mauvais joueur!");
      Reponse2[1] = strdup ("C'est possible! :]");
      Reponse2[2] = strdup ("Et pourquoi pas, t'aimerais perdre contre une pourriture dans ton genre?");
      Reponse2[3] = strdup ("Ne me traite plus jamais de mauvais joueur!");
      Reponse2[4] = strdup ("MONSIEUR mauvais joueur!");
      Reponse2[5] = strdup ("Ça te prend souvent de dire des horreurs pareilles?");
      Reponse2[6] = strdup ("Retire ça!");
      Reponse2[7] = strdup ("Dehors, tricheur!");
      Reponse2[8] = strdup ("Et alors?");
      Reponse2[9] = strdup ("Qu'est-ce que ça peut te faire?");
      Reponse2[10] = strdup ("Tu n'aimes pas perdre? :-]");
      Reponse2[11] = strdup ("Heureusement que tu ne joues pas d'argent!");
      Reponse2[12] = strdup ("Va jouer!");
      Reponse2[13] = strdup ("Demande pardon!");
      Reponse2[14] = strdup ("À genoux, vermisseau!");
      Reponse2[15] = strdup ("Ça suffit!");
      Reponse2[16] = strdup ("Recommence à me traiter de mauvais joueur, tu vas voir ce qui va t'arriver!");
      Reponse2[17] = strdup ("Ça ne te suffit pas que je m'abaisse à jouer avec toi?");
      Reponse2[18] = strdup ("Crétin des Alpes!");
      Reponse2[19] = strdup ("Espèce de lobotomisé!");

      KickerRepondre (from, to, -1, 9, Reponse, -4, 20, Reponse2);
    }
    
    if (BONNE_ANNEE && (NOM || !ischannel (to))) {
      Reponse = malloc (5 * sizeof (char *));
      Reponse[0] = strdup ("Bonne année, %s.");
      Reponse[1] = strdup ("Bonne santé, %s.");
      Reponse[2] = strdup ("À toi aussi, %s.");
      Reponse[3] = strdup ("C'est gentil, %s.");
      Reponse[4] = strdup ("Bonne année à toi aussi, %s.");

      Reponse2 = malloc (2 * sizeof (char *));
      Reponse2[0] = strdup ("Mouf! Merci %s.");
      Reponse2[1] = strdup ("Ben qu'est-ce qui t'arrive, %s?");

      Repondre (from, to, +2, 5, Reponse, +2, 2, Reponse2);
    }
    
    if (MERCI && (NOM || !ischannel (to))) {
      Reponse = malloc (5 * sizeof (char *));
      Reponse[0] = strdup ("De rien, %s.");
      Reponse[1] = strdup ("C'est un plaisir, %s.");
      Reponse[2] = strdup ("C'est naturel, %s.");
      Reponse[3] = strdup ("Je t'en prie, %s.");
      Reponse[4] = strdup ("Y'a pas de quoi, %s.");

      Reponse2 = malloc (2 * sizeof (char *));
      Reponse2[0] = strdup ("Heureusement que j'ai une conscience professionnelle, moi, %s.");
      Reponse2[1] = strdup ("C'est bien parce que je suis obligé, %s.");

      Repondre (from, to, +1, 5, Reponse, +1, 2, Reponse2);
    }
    
    if (PLEURER) {
      Reponse = malloc (10*sizeof (char *));
      Reponse[0] = strdup ("Faut pas pleurer, %s...");
      Reponse[1] = 0;
      Reponse[2] = strdup ("Allons, %s, un peu de joie-de-vivre!");
      Reponse[3] = 0;
      Reponse[4] = strdup ("Sèche tes larmes, %s...");
      Reponse[5] = 0;	
      Reponse[6] = strdup ("Tu veux un mouchoir, %s?");
      Reponse[7] = 0;
      Reponse[8] = strdup ("/me tend un mouchoir en papier à %s.");
      Reponse[9] = 0;

      Reponse2 = malloc (3 * sizeof (char *));
      Reponse2[0] = strdup ("Mouche ton nez, %s!");
      Reponse2[1] = strdup ("Pleure, %s, tu pisseras moins!");
      Reponse2[2] = strdup ("Beerk, en plus %s n'a pas de mouchoir!");

      Repondre (from, to, 0, 10, Reponse, 0, 3, Reponse2);
    }


    if (SOURIRE && (NOM || !ischannel (to))) {
      Reponse = malloc (6*sizeof (char *));
      Reponse[0] = strdup ("%s: :)");
      Reponse[1] = strdup ("%s: :D");
      Reponse[2] = 0;
      Reponse[3] = strdup ("%s: :-)");
      Reponse[4] = strdup ("%s: :-D");
      Reponse[5] = 0; /* Pour les reponses entre bots */
      
      Repondre (from, to, +1, 6, Reponse, +1, 0, 0);
    }

    if (CLINDOEIL && (NOM || !ischannel (to))) {
      Reponse = malloc (6*sizeof (char *));
      Reponse[0] = strdup ("%s: ;)");
      Reponse[1] = strdup ("%s: ;D");
      Reponse[2] = 0;
      Reponse[3] = strdup ("%s: ;-)");
      Reponse[4] = strdup ("%s: ;-D");
      Reponse[5] = 0; /* Pour les reponses entre bots */

      Repondre (from, to, +1, 6, Reponse, +1, 0, 0);
    }

    if (ACHILLE && QUESTION &&(NOM || !ischannel (to))) {
      Reponse = malloc (5 * sizeof (char *));
      Reponse[0] = strdup ("Achille ? Je l'ai bien connu, %s.");
      Reponse[1] = strdup ("J'étais Achille dans une compilation antérieure, %s.");
      Reponse[2] = strdup ("Je suis la réincarnation d'Achille, %s.");
      Reponse[3] = strdup ("Achille est un frère pour moi, %s");

      Reponse2 = malloc (3 * sizeof (char *));
      Reponse2[0] = strdup ("Achille ? Que lui veux-tu, %s ?");
      Reponse2[1] = strdup ("Je ne connais aucun Achille, %s.");
      Reponse2[2] = strdup ("Achille , Jamais entendu parler.");
      Repondre (from, to, 0, 4, Reponse, 0, 3, Reponse2);
    }

	if(!ACHILLE && AS_TU && ChaineEstDans(msg, "vu") &&(NOM || !ischannel(to))){
		char *nick;
		char *buf, *pch, *chaine;
		
		buf = strdup(msg);
		if(pch = strstr(buf, "vu")){
			pch += 2;
			SKIPSPC(pch);
			chaine = get_token(&pch, "?, ");
			do_seen(from, to, chaine);
		}
		if(buf)
			free(buf);
	}

/*     if (AIMES_TU && STJOHNPERSE); */
/*     if (JESUIS && UNE_FILLE & ETTOI); */
/*     if (JESUIS && UN_MEC && ETTOI); */
/*     if (JESUIS && JALOUX && ETTOI); */
/*     if (AS_TU && CERVEAU); */
/*     if (ES_TU && MARIE); */
/*     if (AS_TU && FEMME); */
/*     if (AMI && ETTOI); */
/*     if (INVITER && AMI); */
    /*
      if (AS_TU && UNE_MONTRE);
      if (SNIF); Tu es malade? Tu es triste? Tu veux un mouchoir?
      if (QUI_EST && PRESENT); Qui est la?
      if (COMMENT); Comment tu fais -> Hehehe
      if (QUEL && JOUR);
      if (QUEL && DATE);
      if (QUEL && MOIS);
      if (QUEL && ANNEE);
      if (PARLER && DE_QUOI);
      if (QUOI_DE_NEUF); koi de 9?
      if (RIRE); Ahhaahahahahahahaha -> :)
      if (QUEST_CE && ADRESSE); Email
      if (QUEL_ENDROIT && CRIN);
      if (MOI && RIENCOMPRIS);
      if (JESUIS && MORT);
      if (TIRER_LANGUE); :p
      if (ENTENDS_TU && MOI);
      if (FAINEANT); Glande faineant 
      if (ATCHOUM); A tes souhaits
      if (SNIFF); Tu veux un mouchoir?
      if (TOUSSE); kof kof, reuh
      if (AIMES_TU && CHIENS); Michel Drucker
      if (ES_TU && VIEUX);
      if (QUEL && ECOLE);
      if (ES_TU && BOULOT);
      if (ESIAL);
      if (CRIN);
      if (WEB); http://www.loria.fr/~parmenti/irc/
      if (SORS_TU && CE_SOIR);
      if (AS_TU && VOITURE);
      if (AIMES_TU && VOITURE);
      if (ETUDIANT);
      if (AS_TU && RAISON); Tu as toujours raison?
      if (PLEURS); :'(  :~(
      if (REVES_TU); Moutons electriques
      if (RICANEMENTS); niark niark, hin hin
      if (JOYEUX_NOEL);
      if (RONFLEMENTS); ZZZZZZzzzzzz
      if (AIMES_TU && CHOCOLAT);
      if (JESUIS && MALADE);
      if (JESUIS && DEMORALISE);
      if (DEJA && AMOUREUX); T'as deja ete amoureux?
      if (VENIR_TE_VOIR);
      if (AIMES_TU && LES_FILLES);
      if (MONTE_DANS_TA_CHAMBRE);
      if (QUI_EST && TON_PERE);
      if (QUI_EST && TA_MERE);
      if (AIMES_TU && DANSER);
      if (AIMES_TU && CHANTER); la pizza, le restau
      if (AIMES_TU && JOUER);
      */

  }
  /* Si l'autorisation de parler n'est pas donnee */
  else {

    if ((PARLER || REVEILLE_TOI) && !LANGUE && !FONCTION_SAY && (!ischannel (to) || NOM)) {
      if (rellevel (from) >= 0)
	if (Channel_to) Channel_to->talk = TRUE;

      Reponse = malloc (6*sizeof (char *));
      Reponse[0] = strdup ("%s: Je ne suis plus gênant? :)");
      Reponse[1] = strdup ("À vos ordres, %s.");
      Reponse[2] = strdup ("Il sera fait selon vos désirs, maître %s.");
      Reponse[3] = strdup ("%s: Pour me faire taire à nouveau, demandez-le moi, tout simplement.");
      Reponse[4] = strdup ("Avec plaisir, %s.");
      Reponse[5] = strdup ("Depuis le temps que j'attendais qu'on me le dise, %s! :)");

      if (!STP) {
	Reponse2 = malloc (6*sizeof (char *));
	Reponse2[0] = strdup ("Nan! Demande-le gentiment, %s!");
	Reponse2[1] = strdup ("%s: On m'a dit de me taire, je me tairai!");
	Reponse2[2] = strdup ("Pas question! Je n'obéis pas à ceux qui me sont antipathiques, %s!");
	Reponse2[3] = strdup ("Et pis quoi encore, %s! Cent balles et un Mars?");
	Reponse2[4] = strdup ("Non. J'ai pas envie de te faire plaisir, %s.");
	Reponse2[5] = strdup ("T'as une chance si tu demandes a quelqu'un que j'aime bien, %s. Pour l'instant je continue à me taire. Na!");
	
	Repondre (from, to, +1, 6, Reponse, 0, 6, Reponse2);
      }
      else {
	if (Channel_to) Channel_to->talk = TRUE;

	Reponse2 = malloc (6*sizeof (char *));
	Reponse2[0] = strdup ("Bon! C'est bien parce que tu l'as demandé gentiment, %s!");
	Reponse2[1] = strdup ("%s: On m'a dit de me taire, mais comme tu as été poli, j'agrée ta demande.");
	Reponse2[2] = strdup ("Tu m'es antipathique, %s, mais je ne me tairai plus.");
	Reponse2[3] = strdup ("Et pis j'en ai marre de me taire!");
	Reponse2[4] = strdup ("Bon. J'ai pas envie de te faire plaisir, %s, mais j'en ai ras la casquette d'être obligé de me taire...");
	Reponse2[5] = strdup ("Tu me fais pitié, %s. Pour l'instant je vais me remettre à parler...");
	
	Repondre (from, to, +1, 6, Reponse, +2, 6, Reponse2);
      }
      
    }
  }
  
  GNumPhrase++;

  if (GROS_MOT && !COMPLIMENT && !CLINDOEIL) {
    Reponse2 = malloc (6*sizeof (char *));
    Reponse2[0] = strdup ("Sois poli, s'il-te-plaît, %s!");
    Reponse2[1] = 0;
    Reponse2[2] = strdup ("Un peu de tenue, %s!");
    Reponse2[3] = 0;
    Reponse2[4] = strdup ("Tu vas arrêter d'être grossier, %s?");
    Reponse2[5] = 0;
    
    Repondre (from, to, -1, 0, 0, -2, 6, Reponse2);
  }

  if (GNumPhrase%500 == 100) {
    Reponse = malloc (11*sizeof (char *));
    Reponse[0] = strdup ("C'est peut-être hors propos, mais je vous propose une petite visite de la page de mon créateur : http://francois.parmentier.free.fr/index.html");
    /*Reponse[1] = strdup ("Ça ne vous intéresse peut-être pas, mais pour voir les gens qui passent sur #nancy: http://www.loria.fr/~parmenti/irc/");*/
	Reponse[1] = strdup ("Vous saviez que j'ai un cousin écrit en C ? http://ector.sourceforge.net/");
    Reponse[2] = strdup ("Pour relancer le débat: l'Amiga c'est quand-même mieux que le PC.");
    /*Reponse[3] = strdup ("Pour me voir tout nu: http://www.loria.fr/~parmenti/irc/achille.html (veuillez m'excuser de m'immiscer ainsi dans la conversation).");*/
	Reponse[3] = strdup ("Ma famille est grande, j'ai aussi un cousin écrit en python : http://code.google.com/p/pyector/");
    /*Reponse[4] = strdup ("Vous êtes à Nancy, et vous voulez chanter? http://www.loria.fr/~parmenti/chorale/choraleU.html");*/
	Reponse[4] = strdup ("Hop !");
    Reponse[5] = strdup ("Une petite documentation sur IRC: http://www.funet.fi/~irc/.");
    /*Reponse[6] = strdup ("Si vous voulez délirer, allez voir la page du frangin d'H_I (un usurpateur de nickname): http://www.mygale.org/06/parmenti/"); */
    Reponse[6] = strdup ("Atchoum!");
    Reponse[7] = strdup ("Si vous vous ennuyez, vous pouvez toujours jouer a pierre-papier-ciseaux avec moi.");
    /*Reponse[8] = strdup ("Vous voulez signer le Livre d'Or d'H_I? Pas difficile, allez voir http://www.loria.fr/~parmenti/francois.html (pis si vous voulez pas, ben vous êtes pas obligés)");*/
	Reponse[8] = strdup ("Si jamais ça vous tente, vous pouvez jeter un oeil sur une page de geek : http://sebastien.kirche.free.fr");
    Reponse[9] = strdup ("Vous saviez que Sébastien à mis sa config Emacs/Gnus en ligne sur http://sebastien.kirche.free.fr/emacs_stuff/ ? Je sais: c'est en dehors de la conversation, mais que voulez-vous, faut bien que j'fasse un peu d'pub de temps en temps...");
    /*Reponse[10] = strdup ("Allez faire un tour sur les pages WWW de #reunion: http://www.mygale.org/~reunion/(si vous en avez envie, bien entendu, j'vous force pas).");*/
	Reponse[10] = strdup ("Perl c'est bien. Ruby c'est mieux");

    Reponse2 = malloc (9*sizeof (char *));
    Reponse2[0] = strdup ("Encore quelqu'un qui encombre le canal!");
    Reponse2[1] = strdup ("T'es toujours là, %s?");
    Reponse2[2] = strdup ("%s: :PPPP");
    Reponse2[3] = strdup ("C'est pas vrai! Tu persistes, %s!");
    Reponse2[4] = strdup ("%s: Ta mère en short sur internet :P");
    Reponse2[5] = strdup ("Encore là, %s? Pfff...");
    Reponse2[6] = strdup ("%s: Pas encore parti, toi? Zut!");
    Reponse2[7] = strdup ("Tu ne m'as toujours pas demandé pardon, %s, au fait!");
    Reponse2[8] = strdup ("Pourquoi tu ne me demandes pas pardon, j'suis pas complètement ingrat, tu sais, %s?");
    Repondre (from, to, 0, 11, Reponse, 0, 9, Reponse2);
  }

  if (FONCTION)
    if (Channel_to) Channel_to->talk = AncienneAutorisation;

  /* Si c'est un nouveau jour */
  if (Jour != time2day (time (NULL))) {
    Jour = time2day (time (NULL));

    /* On sauvegarde le fichier des relations */
    cancel_level (currentbot->lists->rellist, DEFAUT_LVL);
    write_lvllist (currentbot->lists->rellist,
		   currentbot->lists->relfile);

    /* On nettoie la liste des locuteurs des locuteurs muets anciens */
    NettoieListeLocuteurs (currentbot->lists->ListeLocuteurs);

    /* On sauvegarde aussi le fichier des locuteurs */
    SauveLocuteurs (currentbot->lists->ListeLocuteurs,
		    currentbot->lists->locuteurfile);
  }

  
  if (NUS)
    free (NUS);

}

