/*
 * botlist.c - implementation of botlists
 * (c) 1997 H_I (parmenti@loria.fr)
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
#include <string.h>
#include <time.h>

#include "config.h"
#include "fnmatch.h"
#include "misc.h"
#include "send.h"
#include "botlist.h"

BOT_list *exist_bothost (BOT_list ** b_list, char *bothost)
{
  BOT_list *Bot;
  
  for ( Bot = *b_list; Bot; Bot = Bot->next )
    if (!fnmatch (Bot->bothost, bothost, FNM_CASEFOLD ))
      return (Bot);

  return (NULL);
}


BOT_list *find_bothost (BOT_list ** b_list, char *bothost) 
{
  BOT_list *Bot;
  
  for (Bot = *b_list; Bot; Bot = Bot->next)
    if (STRCASEEQUAL (bothost, Bot->bothost))
      return (Bot);

  return (NULL);
}

BOT_list **init_botlist () 
{
  BOT_list **b_list;

  b_list  = (BOT_list **) malloc (sizeof (*b_list));
  *b_list = NULL;
  return (b_list);
}

/* Adds a bot to the list... */
void add_to_botlist (BOT_list ** b_list, char *bothost, char *channel) 
{
  BOT_list *New_bot;
  
  if (is_bot (b_list, channel, bothost))
    return;

  if ((New_bot = (BOT_list*)malloc (sizeof (*New_bot))) == NULL)
    return;

  mstrcpy (&New_bot->bothost, bothost);
  mstrcpy (&New_bot->channel, channel);
  New_bot->next = *b_list;
  *b_list = New_bot;
}
  
int  remove_bot (BOT_list **b_list, int numero) 
{
  BOT_list **old;
  BOT_list *Bot;
  BOT_list *dummy = NULL;
  int i = 0;

  for (Bot = *b_list; Bot && !dummy; Bot = Bot->next) {
    i++;
    
    if (i == numero)
      dummy = Bot;
  }

  if (!dummy)
    return (FALSE);

  for (old = b_list; *old; old = &(*old)->next) {
    if (*old == dummy) {
      *old = dummy->next;
      
      free (dummy->bothost);
      free (dummy->channel);
      free (dummy);
      return (TRUE);
    }
  }

  return (FALSE);
}

/* Removes the first occurrence of bothost from b_list */
int  remove_from_botlist (BOT_list **b_list, char *bothost) 
{
  BOT_list **old;
  BOT_list *dummy;

  if ((dummy = find_bothost (b_list, bothost)) == NULL)
    return (FALSE);
  
  for (old = b_list; *old; old = &(*old)->next)
    if (*old == dummy) {
      *old = dummy->next;

      free (dummy->bothost);
      free (dummy->channel);
      free (dummy);
      return (TRUE);
    }

  return (FALSE);
}

void delete_botlist (BOT_list **b_list)
{
  BOT_list *dummy;
  BOT_list *next;
  
  if (b_list) {
    dummy = *b_list;
    while (dummy) {
      next = dummy->next;
      remove_from_botlist (b_list, dummy->bothost); /* Berk! Too slow */
      dummy = next;
    }
    free (b_list);
  } else {
    printf ("delete_botlist (): pointeur nul");
  }
}

int  write_botlist (BOT_list **b_list, char *filename) {
  BOT_list *dummy;
  time_t T;
  FILE *list_file;

  if (!b_list)
    return (FALSE);
  
  if ((list_file = fopen (filename, "w")) == NULL)
    return (FALSE);

  T = time ((time_t *) NULL);

  fprintf( list_file, "#############################################\n" );
  fprintf( list_file, "## %s\n", filename );
  fprintf( list_file, "## Created: %s", ctime( &T ) );
  fprintf( list_file, "## (c) 1997 H_I (parmenti@loria.fr)\n" );
  fprintf( list_file, "#############################################\n" );

  for (dummy = *b_list; dummy; dummy = dummy->next)
     fprintf( list_file,
	      " %40s %10s\n", dummy->bothost, dummy->channel );
  fprintf (list_file, "# end of %s\n", filename);
  fclose (list_file);
  return (TRUE);
}
  
int  is_bot (BOT_list ** b_list, char *channel, char *bothost) {
  BOT_list *Bot;

  for (Bot = *b_list; Bot; Bot = Bot->next)
    if (!fnmatch (Bot->bothost, bothost, FNM_CASEFOLD) &&
	!fnmatch (Bot->channel, channel, FNM_CASEFOLD))
      return (TRUE);

  return (FALSE);
}

int read_from_botfile (FILE *stream, char *bothost, char *channel)
{
  char s[MAXLEN];
  char *p;
  
  do
    p = fgets (s, MAXLEN, stream);
  while ((p != NULL) && (*s == '#'));

  if ( p == NULL )
    return (FALSE);

  sscanf (s, "%s %s", bothost, channel);
  return (TRUE);
}

int readbotdatabase (char *fname, BOT_list **botlist) {
  FILE *fp;
  char bothost[MAXLEN];
  char channel[MAXLEN];

  if ((fp = fopen (fname, "r")) == NULL) {
    printf ("File \"%s\" not found, aborting\n", fname);
    return (FALSE);
  }

  while (read_from_botfile (fp, bothost, channel))
    add_to_botlist (botlist, bothost, channel);
  fclose (fp);
  return (TRUE);
}

/* channel: masque de canaux a afficher
   from   : utilisateur demandeur
   to     : canal de l'utilisateur */
void show_botlist (BOT_list **botlist, char *channel, char *from, char *to) {
  BOT_list *Bot;
  char *channels;
  int i = 0;
  int n = 0;

  /* if the channel is not given, show all the bots */
  if (!channel || strlen (channel) == 0) {
    channels = strdup ("#*");
  } else {
    channels = strdup (channel);
  }

  for (Bot = *botlist; Bot; Bot = Bot->next) {
    i++; 

    if (!fnmatch (Bot->channel, channels, FNM_CASEFOLD) ||
	!fnmatch (channels, Bot->channel, FNM_CASEFOLD)) {
      n++;
      send_to_user (from, "%3d:%50s %10s", i, Bot->bothost, Bot->channel);
    }
  }
  if (!n)
    send_to_user (from, "La liste des bots du canal %s est vide!", channels);

  free (channels);
}
