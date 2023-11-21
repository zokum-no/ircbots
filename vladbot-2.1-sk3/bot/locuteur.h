#ifndef LOCUTEUR_H
#define LOCUTEUR_H

typedef struct locuteur_struct {
	char *nuh;                /* H_I!irc@blah */
	int bonus_malus;          /* Cumul points intermediaires */
	int PremierContact;       /* heure du premier contact */
	int DernierContact;       /* Heure du dernier contact */
	int Bonjours;             /* Nombre de bonjours */
	int Veridique;            /* Booleen disant si le locuteur s'est identifie */
	long LastSeen;			  /* dernier signe de vie du locuteur */
  struct locuteur_struct *suivant; /* locuteur suivant */
} locuteur;

locuteur **InitLocuteurListe ();
locuteur *LocuteurExiste (locuteur **, char *);
locuteur *LocuteurNickExiste (locuteur **, char *);
locuteur *AjouteLocuteur (locuteur **, char *);
void      AjoutePoints (locuteur *, int);
void      AnnulePoints (locuteur *);
void      AnnuleBonjours (locuteur *);
void      MAJPremierContact (locuteur *);
void      MAJDernierContact (locuteur *);
void	  MAJDerniereActivite (locuteur *);
void      EnleveLocuteur (locuteur **, locuteur *);
void      MontreLocuteurs (locuteur **, char *, char *);
int       SauveLocuteurs (locuteur **, char *);
void      DetruitListe (locuteur **);
int       ChargeLocuteurs (char *, locuteur **);
void      NettoieListeLocuteurs (locuteur **);
int       loclevel (char *);
int       loctrue (char *);
#endif
