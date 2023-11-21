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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "phrase.h"
#include "config.h"
#include "misc.h"
#include "debug.h"

phr_tbl **TableDesStimuli = 0;
rep_tbl **TableDesReponses = 0;
int TailleStim = 0;
int TailleRep = 0;


int SauveStimuli (char *nom_fichier)
{
  int i;
  FILE *fichier;
  
  if (nom_fichier && strlen (nom_fichier)) 
	  if ((fichier = fopen (nom_fichier, "w"))) {
		  fprintf (fichier, "%d\n", TailleStim);
		  for (i = 0; i < TailleStim; i++) {
			  fprintf (fichier,"%s\t	%s\t%d\t%s\n",
					   TableDesStimuli[i]->NomStimulus,
					   TableDesStimuli[i]->Stimulus,
					   TableDesStimuli[i]->Actif,
					   TableDesStimuli[i]->Auteur);
		  }
		  fclose (fichier);
		  return (TRUE);
	  }
  
  return (FALSE);
  
}

int ChargeStimuli (char *nom_fichier)
{
  int i;
  FILE *fichier;
  char Buffer[BIG_BUFFER];
  char *Pointeur, *debut, *fin, *inter;

  if (nom_fichier && strlen (nom_fichier)) {
	  if ((fichier = fopen (nom_fichier, "r"))) {
		  if (TableDesStimuli && TailleStim) {
			  for (i = 0; i < TailleStim; i++) {
				  if (TableDesStimuli[i]->NomStimulus)
					  free (TableDesStimuli[i]->NomStimulus);
				  if (TableDesStimuli[i]->Stimulus)
					  free (TableDesStimuli[i]->Stimulus);
				  if (TableDesStimuli[i]->Auteur)
					  free (TableDesStimuli[i]->Auteur);
			  }
	
			  free (TableDesStimuli);
			  TailleStim = 0;
		  }
		  
		  fgets (Buffer, BIG_BUFFER, fichier);
		  KILLNEWLINE(Buffer);
		  
		  TailleStim = atoi (Buffer);

		  if (TailleStim > 0) {
			  TableDesStimuli = (phr_tbl **) malloc (TailleStim * sizeof (phr_tbl *));
		  }
	
		  for (i = 0; i < TailleStim && TableDesStimuli; i++) {
			  fgets (Buffer, BIG_BUFFER, fichier);
			  fin = strchr (Buffer, '\n');
			  if (fin)
				  *fin = '\0';
	
			  Pointeur = strdup (Buffer);
			  debut    = Pointeur;
			  
			  inter = strtok (Pointeur, "\t");

			  TableDesStimuli[i] = (phr_tbl *) malloc (sizeof (phr_tbl));
			  TableDesStimuli[i]->NomStimulus = strdup (inter?inter:"");
			  
			  inter = strtok (NULL, "\t");
			  TableDesStimuli[i]->Stimulus    = (inter?strdup (inter):strdup(""));

			  inter = strtok (NULL, "\t");
			  TableDesStimuli[i]->Actif       = (inter?atoi(inter):FALSE);

			  inter = strtok (NULL, "\t\n\0");
			  TableDesStimuli[i]->Auteur      = (inter?strdup (inter):strdup(""));

			  TableDesStimuli[i]->Present     = FALSE;


			  /* 	printf ("NomStimulus:\"%s\", Stimulus:\"%s\", Aut	eur:\"%s\"\n", */
			  /* 		TableDesStimuli[i]->NomStimulus, */
			  /* 		TableDesStimuli[i]->Stimulus, */
			  /* 		TableDesStimuli[i]->Auteur); */

			  free (debut);
		  }
		  fclose (fichier);
		  return (TRUE);
	  } else
		  return (FALSE);
  }
  else
	  return (FALSE);
}

int AjouteStimulus (char *from, char *channel, char *Stimulus, char *NomStimulus) 
{
  int i;
  phr_tbl **TableIntermediaireP;

  if (TailleStim >= 0) {
    TableIntermediaireP = (phr_tbl **) malloc (sizeof (phr_tbl *)*(TailleStim+1));
    TailleStim++;
    
    for (i = 0; i < TailleStim -1; i++)
      TableIntermediaireP[i] = TableDesStimuli[i];
    
    if (TableDesStimuli)
      free (TableDesStimuli);

    TableDesStimuli = TableIntermediaireP;
    
    TableDesStimuli[TailleStim-1] = (phr_tbl *) malloc (sizeof (phr_tbl));
    TableDesStimuli[TailleStim-1]->NomStimulus = strdup(NomStimulus);
    TableDesStimuli[TailleStim-1]->Stimulus    = strdup(Stimulus);
    TableDesStimuli[TailleStim-1]->Auteur      = strdup(from);
    TableDesStimuli[TailleStim-1]->Actif       = TRUE;
    return TRUE;
  } else
    return FALSE;
}


int SauveReponses (char *nom_fichier)
{
  int i;
  FILE *fichier;

  if (TableDesReponses[0])
  {
	  if (nom_fichier && strlen (nom_fichier)) {
		  if ((fichier = fopen (nom_fichier, "w"))) {
			  assert(fichier != NULL);
			  fprintf (fichier, "%d\n", TailleRep);
			  for (i = 0; i < TailleRep; i++) {
				  fprintf (fichier,"%s\t%s\t%s\t%s\n",
						   TableDesReponses[i]->NomStimulus,
						   TableDesReponses[i]->Reponse,
						   TableDesReponses[i]->Auteur,
						   TableDesReponses[i]->Canal);
			  }
			  fclose (fichier);
			  return (TRUE);
		  }
	  }
  }
  else
	  fprintf(stderr,"Table des reponses vide?!?\n");
  
  return (FALSE);
}

int ChargeReponses (char *nom_fichier)
{
  int i;
  FILE *fichier;
  char Buffer[BIG_BUFFER];
  char *Pointeur, *debut, *fin;

  if (nom_fichier && strlen (nom_fichier)) {
	  if ((fichier = fopen (nom_fichier, "r"))) {
		  if (TableDesReponses && TailleRep) {
			  for (i = 0; i < TailleRep; i++) {
				  if (TableDesReponses[i]->Reponse)
					  free (TableDesReponses[i]->Reponse);
				  if (TableDesReponses[i]->NomStimulus)
					  free (TableDesReponses[i]->NomStimulus);
				  if (TableDesReponses[i]->Auteur)
					  free (TableDesReponses[i]->Auteur);
				  if (TableDesReponses[i]->Canal)
					  free (TableDesReponses[i]->Canal);
			  }
	
			  free (TableDesReponses);
			  TableDesReponses = NULL;
			  TailleRep = 0;
		  }
		  
		  
		  fgets (Buffer, BIG_BUFFER, fichier);
		  fin = strchr (Buffer, '\n');
		  if (fin)
			  *fin = '\0';
		  
		  TailleRep = atoi (Buffer);

		  if (TailleRep > 0) 
			  TableDesReponses = (rep_tbl **) malloc (TailleRep * sizeof (rep_tbl *));
		  for (i = 0; i < TailleRep; i++) {
			  fgets (Buffer, BIG_BUFFER, fichier);
			  fin = strchr (Buffer, '\n');
			  if (fin)
				  *fin = '\0';
			  Pointeur = strdup (Buffer);
			  debut    = Pointeur;
			  
			  TableDesReponses[i] = (rep_tbl*) malloc (sizeof (rep_tbl));
			  TableDesReponses[i]->NomStimulus = strdup (get_token (&Pointeur, "\t"));
			  TableDesReponses[i]->Reponse     = strdup (get_token (&Pointeur, "\t"));
			  TableDesReponses[i]->Auteur      = strdup (get_token (&Pointeur, "\t"));
			  TableDesReponses[i]->Canal       = strdup (get_token (&Pointeur, "\t\n"));
			  TableDesReponses[i]->Active      = TRUE;

			  free (debut);
		  }
		  fclose (fichier);
		  return (TRUE);
	  } else
		  return (FALSE);
  }
  else
	  return (FALSE);
	
}


int AjouteReponse (char *from, char *channel, char *Reponse, char *NomStimulus) 
{
  rep_tbl** TableIntermediaireR;
  int i;

  if (TailleRep >= 0) {
	  TableIntermediaireR = (rep_tbl**) malloc (sizeof (rep_tbl *)*(TailleRep+1));
	  TailleRep++;
    
	  for (i = 0; i < TailleRep -1; i++)
		  TableIntermediaireR[i] = TableDesReponses[i];
    
	  if (TableDesReponses)
		  free (TableDesReponses);
    
	  TableDesReponses = TableIntermediaireR;
	  
	  TableDesReponses[TailleRep-1] = (rep_tbl*) malloc (sizeof (rep_tbl));
	  TableDesReponses[TailleRep-1]->Reponse     = strdup(Reponse);
	  TableDesReponses[TailleRep-1]->NomStimulus = strdup(NomStimulus);
	  TableDesReponses[TailleRep-1]->Auteur      = strdup(from);
	  TableDesReponses[TailleRep-1]->Canal       = strdup(channel);
	  TableDesReponses[TailleRep-1]->Active      = TRUE;
	  
	  return TRUE;
  } else
	  return FALSE;
}

int SupprimeRep (int numero) 
{
  int i;

  numero--; /* on demarre a 1 */
  if (numero < TailleRep && numero >= 0) {
	  free (TableDesReponses[numero]->Reponse);
	  free (TableDesReponses[numero]->NomStimulus);
	  free (TableDesReponses[numero]->Auteur);
	  free (TableDesReponses[numero]->Canal);

	  for (i = numero; i < TailleRep-1; i++) {
		  TableDesReponses[i]->Reponse = TableDesReponses[i+1]->Reponse;
		  TableDesReponses[i]->NomStimulus = TableDesReponses[i+1]->NomStimulus;
		  TableDesReponses[i]->Auteur = TableDesReponses[i+1]->Auteur;
		  TableDesReponses[i]->Canal = TableDesReponses[i+1]->Canal;
		  TableDesReponses[i]->Active = TableDesReponses[i+1]->Active;
	  }
	  TailleRep--;
	  return TRUE;
  }
  
  return FALSE;
}

int SupprimeStim (int numero) 
{
  int i;

  numero--; /* On demarre a 1 */
  if (numero < TailleStim && numero >= 0) {
	  free (TableDesStimuli[numero]->NomStimulus);
	  free (TableDesStimuli[numero]->Stimulus);
	  free (TableDesStimuli[numero]->Auteur);

	  for (i = numero; i < TailleStim-1; i++) {
		  TableDesStimuli[i]->Stimulus = TableDesStimuli[i+1]->Stimulus;
		  TableDesStimuli[i]->NomStimulus = TableDesStimuli[i+1]->NomStimulus;
		  TableDesStimuli[i]->Auteur = TableDesStimuli[i+1]->Auteur;
		  TableDesStimuli[i]->Present = TableDesStimuli[i+1]->Present;
		  TableDesStimuli[i]->Actif = TableDesStimuli[i+1]->Actif;
	  }
	  TailleStim--;
	  return TRUE;
  }
  
  return FALSE;
}


void LibereReponses () 
{
  rep_tbl *Courant, *Suivant;
  int i;

  for (i = 0; i < TailleRep; i++) 
	  if (TableDesReponses[i]) {
		  free (TableDesReponses[i]->Reponse);
		  free (TableDesReponses[i]->NomStimulus);
		  free (TableDesReponses[i]->Auteur);
		  free (TableDesReponses[i]->Canal);
		  
		  free (TableDesReponses[i]);
	  }

  if (TailleRep && TableDesReponses){
	  free (TableDesReponses);
	  TableDesReponses = NULL;
  }
}
