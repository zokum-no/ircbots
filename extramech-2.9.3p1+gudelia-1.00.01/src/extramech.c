/*

    ExtraMech, EnergyMech IRC bot software Extension
    Parts Copyright (c) 2001-2003 Obi-Wan

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

char *Lowerize( char *str )
{
   char *retstr = str; /* save string addr */
      while (*str != 0) { 
	if (isalpha(*str))
		*str = tolower(*str); /* Pasamos el nombre a minusculas */
	str++;	
   }

   return( retstr );
}

void do_defineadd(char *from, char *to,  char *rest, int cmdlevel)
{
	char  *chanshort, *chan, *parametro1, *showdefine, *definition, *aux;
	FILE *showfile;
	char file[MAXLEN];
	if (!rest)
	{
		usage(from,C_DEFINEADD);
		return;
	}
	else
	{
		parametro1 = chop(&rest);
		if (ischannel(parametro1))
		{
			chan = parametro1;
			aux = chop(&rest);
			definition = Lowerize(aux);
			if (!rest)
			{
				usage(from,C_DEFINEADD);
				return;
			}
			showdefine = rest;
		}
		else
		{
			chan = to;
			definition = Lowerize(parametro1);
			showdefine = rest;
			if (!rest)
			{
				usage(from,C_DEFINEADD);
				return;
			}
		}
	
		if (get_userlevel(from,chan) < cmdlevel)
			return;

		chanshort = chan;
		if ((*chanshort == '&') || (*chanshort == '+') || (*chanshort == '#')) 
			chanshort++; /* Nos cargamos el primer caracter del canal */
		chanshort = Lowerize(chanshort);
		sprintf(file,"defines/%s/%s", chanshort, definition);
		if ((showfile = fopen(file, "r")) != NULL) {
			send_to_user(from,TEXT_DEFINEALREADYEXIST, definition);			
			fclose(showfile);
			return;
		}
		else {
			showfile = NULL;
			if ((showfile = fopen(file, "w")) != NULL)
			{
 				fprintf(showfile,"%s: %s ",definition, showdefine);
 				fprintf(showfile,TEXT_DEFINEADDEDBY,CurrentNick);
				send_to_user(from,TEXT_DEFINESAVED, definition);	
				fclose(showfile);
			}
			else
			{
				send_to_user(from,TEXT_CANTSAVEDEFINE, definition);					
			}
		}
	}
}

void do_definedel(char *from, char *to,  char *rest, int cmdlevel)
{
	char  *chanshort, *chan, *parametro1, *parametro2, *showdefine;
	FILE *showfile;
	char file[MAXLEN];
	char *parametros = rest;
	if (!rest)
	{
		usage(from,C_DEFINEDEL);
		return;
	}
	else
	{
		parametro1 = chop(&rest);
		if (ischannel(parametro1))
		{
			chan = parametro1;
			showdefine = Lowerize(rest);
			parametro2 = chop(&rest);
			if (!parametro2)
			{
				usage(from,C_DEFINEDEL);
				return;
			}
		}
		else
		{
			chan = to;
			showdefine = Lowerize(parametros);
		}
	
		if (get_userlevel(from,chan) < cmdlevel)
			return;

		chanshort = chan;
		if ((*chanshort == '&') || (*chanshort == '+') || (*chanshort == '#')) 
			chanshort++; /* Nos cargamos el primer caracter del canal */
		chanshort = Lowerize(chanshort);
		sprintf(file,"defines/%s/%s", chanshort, showdefine);
		if ((showfile = fopen(file, "r")) == NULL) {
			send_to_user(from,TEXT_DEFINENOTEXIST, showdefine);			
			return;
		}
		else {
			fclose(showfile);
			showfile = NULL;
			if (remove(file) == 0)
			{
				send_to_user(from,TEXT_DEFINEREMOVED, showdefine);	
			}
			else
			{
				send_to_user(from,TEXT_CANTREMOVEDEFINE, showdefine);					
			}
		}
	}
}


void do_define(char *from, char *to,  char *rest, int cmdlevel)
{
	char  *chanshort, *chan, *parametro1, *parametro2, *showdefine;
	FILE *showfile;
   	char showline[MAXLEN];
	char file[MAXLEN];
	char *parametros = rest;
	chanshort = NULL;
	if (!rest)
	{
		usage(from,C_DEFINE);
		return;
	}
	else
	{
		parametro1 = chop(&rest);
		if (ischannel(parametro1))
		{
			chan = parametro1;
			showdefine = Lowerize(rest);
			parametro2 = chop(&rest);
			if (!parametro2)
			{
				usage(from,C_DEFINE);
				return;
			}
		}
		else
		{
			chan = to;
			showdefine = Lowerize(parametros);
		}
	
		if (get_userlevel(from,chan) < cmdlevel)
			return;

		chanshort = chan;
		if ((*chanshort == '&') || (*chanshort == '+') || (*chanshort == '#')) 
			chanshort++; /* Nos cargamos el primer caracter del canal */
		chanshort = Lowerize(chanshort);
		sprintf(file,"defines/%s/%s", chanshort, showdefine);
		showfile = fopen(file, "r");
		if (showfile) {
			while (fgets(showline, sizeof(showline), showfile)) {
				showline[strlen(showline)-1] = 0;
				sendprivmsg(chan,"%s",showline);
			}
			fclose(showfile);
			return;
		}
		else {
			sendprivmsg(chan,TEXT_NOSUCHDEFINEFILE, showdefine);
			return;
		}
	}
}



/* 24 ago 2002 - Obi-Wan 
   Command similar to phrase. First search channel´s phrase file.
   If the file does not exist, it users RANDPHRASEFILE
*/

void do_phrase(char *from, char *to, char *rest, int cmdlevel)
{

	char	*chan,*nick,*phrase,*chanshort;
	char file[MAXLEN];
	nick = chop(&rest);
	chan = get_channel(to, &rest);
	chanshort = NULL;
	if (rest)
	{
		if (ischannel(nick)) {
			chanshort = nick;
			if ((*chanshort == '&') || (*chanshort == '+') || (*chanshort == '#')) 
				chanshort++; /* Nos cargamos el primer caracter del canal */
			chanshort = Lowerize(chanshort);
			sprintf(file,"%sphrases/%s.e", RANDDIR, chanshort);
			if ((phrase = randstring(file)) == NULL)
			{
				send_to_user(from,ERR_FILEOPEN,file);
				if ((phrase = randstring(RANDPHRASEFILE)) == NULL)
				{
					send_to_user(from,ERR_FILEOPEN,RANDPHRASEFILE);
					return;
				}
			}
		}
		else {
			if ((phrase = randstring(RANDPHRASEFILE)) == NULL)
			{
				send_to_user(from,ERR_FILEOPEN,RANDPHRASEFILE);
				return;
			}
		}
		to_server("PRIVMSG %s :%s",nick,phrase);
		send_to_user(from,"(%s) %s",nick,phrase);
		return;
	}
	if (ischannel(to))
	{
		chanshort = to;
		if ((*chanshort == '&') || (*chanshort == '+') || (*chanshort == '#')) 
			chanshort++; /* Nos cargamos el primer caracter del canal */
		chanshort = Lowerize(chanshort);
		sprintf(file,"%sphrases/%s.e", RANDDIR, chanshort);
		if ((phrase = randstring(file)) == NULL)
		{
			send_to_user(from,ERR_FILEOPEN,file);
			if ((phrase = randstring(RANDPHRASEFILE)) == NULL)
			{
				send_to_user(from,ERR_FILEOPEN,RANDPHRASEFILE);
				return;
			}
		}
		sendprivmsg(chan,"%s",phrase);
		return;
	}
	usage(from,C_PHRASE);
}

/* Shows URL for channel stats if CHANLOG is active ;) 24 ago 2002 - Obi-Wan */

void do_cstats(char *from, char *to,  char *rest, int cmdlevel)
{
	char  *chanshort,*chan;
	aChan	*Chan;
	chan = get_channel(to, &rest);
	if (get_userlevel(from,chan) < cmdlevel)
		return;
	if ((Chan = find_channel(chan,CH_ACTIVE)) == NULL)
	{
		send_to_user(from,ERR_CHAN,chan);
		return;
	}
	chanshort = Chan->name;
   	if ((*chanshort == '&') || (*chanshort == '+') || (*chanshort == '#')) 
		chanshort++; /* Nos cargamos el primer caracter del canal */
	chanshort = Lowerize(chanshort);
	if (get_int_var(chan,TOGCHANLOG_VAR)) {
		sendprivmsg(chan,TEXT_WEBSTATS1,chanshort);
		sendprivmsg(chan,TEXT_WEBSTATS2);
	}
	else {
		sendprivmsg(chan,TEXT_NOWEBSTATS,chan);
	}
}

/* Shows credits of energymech ;) 26 dic 2002 - Obi-Wan */

void do_credits(char *from, char *to, char *rest, int cmdlevel) {
  const char *creditstext[] = {
    "",
    "Original EnergyMech Design:",
    "",
    "polygons <http://www.fractal.net>",
    "",
    "",
    "Starglider Class EnergyMech main developer:", 
    "",
    "proton <proton@energymech.net>",
    "",
    "",
    "Other Starglider Class EnergyMech developers",
    "",
    "endorphin  <endorphin@energymech.net>",
    "DaRkThInG  <darkthing@energymech.net>",
    "Dark-Lord  <darklord@energymech.net>",
    "",
    "",
    "Desarrollador principal ExtraMech y traductor:",
    "",
    "Obi-Wan (Julio Gonzalez)     <Obi-Wan@irc-world.org>",
    "",
    "",
    "All information about Starglider Class EnergyMech is",
    "avaible at http://www.energymech.net",
    "",
    "Win32 versions avaible at http://winmech.tripod.com/",
    "",
    "Toda la información sobre esta traducción de",
    "Starglider Class EnergyMech al castellano, asi como",
    "los diferentes parches, puede ser obtenida en la",
    "direccion web http://www.irc-world.org/energymech/",
    "",
    "",
    0,
};

  const char **creditsline = creditstext;

	send_to_user(from,TEXT_STARTCREDITS,current->nick);
	send_to_user(from," ");
      while (*creditsline) {
		send_to_user(from,"%s",*creditsline++);
	}
	send_to_user(from," ");
	send_to_user(from,TEXT_ENDCREDITS);
}

/* Shows motd for energymech ;) 24 ago 2002 - Obi-Wan */
void do_motd(char *from, char *to, char *rest, int cmdlevel) {
	FILE *motd;
   	char motdline[MAXLEN];
	motd = fopen(MOTDFILE, "r");
   	if (motd) {
		send_to_user(from,TEXT_STARTMOTD,current->nick);
		send_to_user(from," ");
		while (fgets(motdline, sizeof(motdline), motd)) {
	    		motdline[strlen(motdline)-1] = 0;
			send_to_user(from,"%s",motdline);
		}
		fclose(motd);
		send_to_user(from," ");
		send_to_user(from,TEXT_ENDMOTD);
    	}
	else {
		send_to_user(from,TEXT_NOMOTD);
    	}
}


