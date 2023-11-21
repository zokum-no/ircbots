#ifndef PHRASE_H
#define PHRASE_H

typedef struct {
  char *NomStimulus;
  char *Stimulus;
  char *Auteur;
  int Present;
  int Actif;
} phr_tbl;

typedef struct 
{
  char *Reponse;
  char *NomStimulus;
  char *Auteur;
  char *Canal;
  int Active;
} rep_tbl;

extern phr_tbl **TableDesStimuli;
extern rep_tbl **TableDesReponses;
extern int TailleStim;
extern int TailleRep;

int SauveStimuli (char *nom_fichier);
int ChargeStimuli (char *nom_fichier);
int AjouteStimulus (char *from, char *channel, char *Stimulus, char *NomStimulus);
int SauveReponses (char *nom_fichier);
int ChargeReponses (char *nom_fichier);
int AjouteReponse (char *from, char *channel, char *Reponse, char *NomStimulus);
int SupprimeRep (int numero);
int SupprimeStim (int numero);
void LibereReponses ();

#endif
