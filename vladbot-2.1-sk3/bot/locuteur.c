/*
 * locuteur.c - implementation of userlists
 * (c) 1996 H_I (parmenti@loria.fr)
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

#include <stddef.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <time.h>

#include "config.h"
#define _GNU_SOURCE
#include "fnmatch.h"
#include "misc.h"
#include "send.h"
#include "locuteur.h"
#include "file.h"
#include "vladbot.h"

extern botinfo *currentbot;

locuteur **InitLocuteurListe () {
	locuteur **Liste = malloc (sizeof (locuteur *));
  
	*Liste = NULL;
	return (Liste);
}

locuteur *LocuteurExiste (locuteur **Liste, char *userhost) {
	locuteur *User;

	for (User = *Liste; User; User = User->suivant)
		if (!fnmatch (User->nuh, userhost, FNM_CASEFOLD))
			return User;

	return NULL;
} /* locuteur LocuteurExiste () */

locuteur *LocuteurNickExiste (locuteur **Liste, char *nick) {
	locuteur *User;
#ifdef DBUG
	printf("LocuteurNickExiste : cherche %s\n", nick);
#endif
	for (User = *Liste; User; User = User->suivant){
#ifdef DBUG
		printf("Locuteur courant : %s, nick %s\n", User->nuh, GetNick(User->nuh));
#endif
		if (!fnmatch (GetNick(User->nuh), nick, FNM_CASEFOLD))
			return User;
	}

	return NULL;
} /* locuteur LocuteurNickExiste () */

locuteur *AjouteLocuteur (locuteur **Liste, char *userhost) {

	if (Liste) {
		locuteur *AAjouter = malloc (sizeof (locuteur));
    
		AAjouter->nuh = strdup(userhost);
		AAjouter->bonus_malus = 0;
		AAjouter->PremierContact = 0 /* time2hours (time (NULL))*/;
		AAjouter->DernierContact = 0 /* time2hours (time (NULL))*/;
		AAjouter->Bonjours = 0;
		AAjouter->Veridique = TRUE;
		AAjouter->LastSeen = time(NULL);
		AAjouter->suivant = *Liste;
    
		*Liste = AAjouter;
    
		return AAjouter;
	}
	else
		return NULL;
} /* void AjouteLocuteur () */

void AjoutePoints (locuteur *Locuteur, int Points) {
	if (Locuteur)
		Locuteur->bonus_malus += Points;
} /* void AjoutePoints () */

void AnnulePoints (locuteur *Locuteur) {
	if (Locuteur)
		Locuteur->bonus_malus = 0;
}

void MAJPremierContact (locuteur *Locuteur) {
	if (Locuteur) {
		Locuteur->PremierContact = time2hours (time (NULL));
		Locuteur->Veridique = FALSE;
		/* PAS FINI ? */
	}
}

/* dernier contact du bot -> locuteur */
void MAJDernierContact (locuteur *Locuteur) { 
	if (Locuteur) {
		if (Locuteur->DernierContact - Locuteur->PremierContact > 12) {
			MAJPremierContact (Locuteur);
			AnnuleBonjours (Locuteur);
		}
		Locuteur->LastSeen = time(NULL);
		Locuteur->DernierContact = time2hours (time (NULL));
	}
}

/* dernier signe de vie du locuteur */
void MAJDerniereActivite (locuteur *Locuteur) {
	if(Locuteur)
		Locuteur->LastSeen = time(NULL);
}

void AnnuleBonjours (locuteur *Locuteur) {
	if (Locuteur)
		Locuteur->Bonjours = 0;
}

void EnleveLocuteur (locuteur **Liste, locuteur *AOter) {
	locuteur *courant, *prec;

	if (Liste)
		if (*Liste) {
			if (AOter) {
				prec = NULL;
				for (courant = *Liste; courant; courant = courant->suivant) {
					if (courant == AOter) {
						if (prec)
							prec->suivant = courant->suivant;
						else
							*Liste = courant->suivant;
					}
					prec = courant;
				}
				if (AOter->nuh)
					free (AOter->nuh);
				free (AOter);
			}
		}
} /* void EnleveLocuteur () */

void MontreLocuteurs (locuteur **Liste, char *from, char *userhost) {
	locuteur *Locuteur;

	for (Locuteur = *Liste; Locuteur; Locuteur = Locuteur->suivant)
		if ((userhost && !fnmatch (userhost, Locuteur->nuh, FNM_CASEFOLD))
			|| !userhost)
			send_to_user( from, " %40s | %4d | %6d | %-2d", 
						  Locuteur->nuh, Locuteur->bonus_malus,
						  Locuteur->PremierContact, Locuteur->Bonjours);
} /* void MontreLocuteurs () */

int SauveLocuteurs (locuteur **Liste, char *NomFichier) {
	locuteur *Locuteur;
	time_t   T;
	FILE     *fichier;

	if ((fichier = fopen (NomFichier, "w")) == NULL)
		return (FALSE);

	T = time ((time_t *) NULL);

	fprintf( fichier, "#############################################\n" );
	fprintf( fichier, "## %s\n", NomFichier );
	fprintf( fichier, "## Created: %s", ctime( &T ) );
	fprintf( fichier, "## (c) 1996 H_I (parmenti@loria.fr)\n" );
	fprintf( fichier, "#############################################\n" );
	
	for (Locuteur = *Liste; Locuteur; Locuteur = Locuteur->suivant)
		fprintf (fichier,
				 "%40s %d %d %d %ld\n",
				 Locuteur->nuh, Locuteur->bonus_malus,
				 Locuteur->PremierContact,
				 Locuteur->DernierContact,
				 Locuteur->LastSeen
				 );
  
	fprintf (fichier, "# End of %s\n", NomFichier );
	fclose (fichier);
	return (TRUE);
} /* int SauveLocuteurs () */

void DetruitListe (locuteur **Liste) {
	locuteur *ADetruire;

	if (Liste)
		while (*Liste) {
			ADetruire = *Liste;
			*Liste = (*Liste)->suivant;
			if (ADetruire->nuh) {
				free (ADetruire->nuh);
			}
			free (ADetruire);
		}

} /* void DetruitListe () */


int Lit_Fichier_Locuteur (FILE *stream, char *usrhost, int *points, int *heure, int *dernier, long *seen) {
	char s[MAXLEN];
	char *p;
	
	do {
		p = fgets (s, MAXLEN, stream);
	} while ((p != NULL) && (*s == '#'));
	
	if ( p == NULL )
		return (FALSE);
	
	sscanf (s, "%s %d %d %d %ld", usrhost, points, heure, dernier, seen);
	
	return (TRUE);
} /* int Lit_Fichier_Locuteur () */

int       ChargeLocuteurs (char *NomFichier, locuteur **Liste) {
	FILE *fp;
	char usrhost[MAXLEN];
	int  points;
	int  pc; 	/* heures du premier contact */
	int  dc; 	/* 	heures du dernier contact */
	long seen;	/* dernière activité vue */
	locuteur *Loc;

	if ((fp = fopen (NomFichier, "r")) == NULL) {
		printf ("Fichier \"%s\" non trouve, j'arrete.\n", NomFichier);
		exit (0);
	}

	while (Lit_Fichier_Locuteur (fp, usrhost, &points, &pc, &dc, &seen)) {
		Loc = AjouteLocuteur (Liste, usrhost);
		Loc->bonus_malus = points;
		Loc->PremierContact = pc;
		Loc->DernierContact = dc;
		Loc->LastSeen = seen;
	}
	
	fclose (fp);
	return (TRUE);
	
} /* int ChargeLocuteurs () */

/* Enleve tous les locuteurs qui ont un DernierContact ancien, un
   DernierContact - PremierContact petit et un bonus nul */
void NettoieListeLocuteurs (locuteur **Liste) {
	locuteur *ListeP, *Suivant;
	int Premier, Dernier, Bonus, Actuel;
	
	if (*Liste) {
		Actuel = time2hours (time (NULL));
		ListeP = *Liste;
		while (ListeP) {
			Premier = ListeP->PremierContact;
			Dernier = ListeP->DernierContact;
			Bonus   = ListeP->bonus_malus;
			Suivant = ListeP->suivant;
			if ((!Bonus && Dernier-Premier <= 2 && Actuel-Dernier > 24*7) ||
				((Bonus < 5 && Bonus > -5) && Actuel-Dernier > 24*7*4) ||
				(Actuel-Dernier > 365 * 24 * 7 )) {
				EnleveLocuteur (Liste, ListeP);
			}
			ListeP = Suivant;
		}
	}
}

/* Renvoie le score provisoire du locuteur */
int loclevel (char *from) {
	locuteur *Locuteur = 0;
	Locuteur = LocuteurExiste (currentbot->lists->ListeLocuteurs, from);
	if (Locuteur)
		return (Locuteur->bonus_malus);
	else
		return (0);
}

/* Renvoie la valeur du champ Veridique du locuteur correspondant au nom passe en parametre */
int loctrue (char *from) {
	locuteur *Locuteur = 0;
	Locuteur = LocuteurExiste (currentbot->lists->ListeLocuteurs, from);
	if (Locuteur)
		return (Locuteur->Veridique);
	else
		return (FALSE);
 }
