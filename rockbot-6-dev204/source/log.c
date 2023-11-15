/*
 *
 */

#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include "dcc.h"
#include "config.h"
#include "function.h"
#include "misc.h"
#include "userlist.h"

extern char botlogfile[FNLEN];
extern char bot_dir[FNLEN], current_nick[MAXNICKLEN];

void botlog(char *logfile, char *logfmt, ...)
{
  va_list args;
  char entiremessage[WAYTOBIG];
  FILE *flog;
  time_t Tme;
  char *time_string;

  va_start(args, logfmt);
  vsnprintf(entiremessage, WAYTOBIG-1, logfmt, args);
  va_end(args);
  if (!Globals->Settings[LOGGING] && !strcasecmp(logfile, botlogfile))
    return;
  if ((Tme = time((time_t *) NULL)) != -1)
  {
    time_string = ctime(&Tme);
    *(time_string + strlen(time_string) - 1) = '\0';
  }
  if ((flog = fopen(logfile, "a")) == NULL)
    return;
  fprintf(flog, "%s: %s\n", time_string, entiremessage);
  fclose(flog);
}

void logchat(char *channel, char *msg)
{
  char *temp, filename[FNLEN];
  char buffer[MAXLEN];
  FILE *out;
  time_t currenttime;
  extern char instancename[MAXLEN];

  strcpy(buffer, strip(channel, '#'));
  strlwr(buffer);
  sprintf(filename, "%s/%s/%s.logchat", bot_dir, instancename, buffer);
  currenttime = getthetime();
  temp = ctime(&currenttime);
  *(temp + strlen(temp) - 1) = '\0';
  sprintf(buffer, "%s: ", temp);
  strcat(buffer, msg);

  if ((out = fopen(filename, "a")) == NULL)
    return;
  fprintf(out, "%s\n", buffer);
  fclose(out);
}
