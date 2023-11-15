/*  
 * misc.c
 *
 */

#include <stdio.h>		/* clean this up! */
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <ctype.h>
#include "misc.h"
#include "config.h"
#include "parse.h"
#include "channel.h"

static char usernick[MAXLEN];
static char timebuf[MAXLEN];
static char idlestr[MAXLEN];
extern CHAN_list *Channel_list;
extern int debugging;


void Debug(int Level, char *fmt, ...)
{
  va_list args;
  char buff[INBUFFSIZE*2];
  char *tmp;

  va_start(args, fmt);
  vsprintf(buff,fmt,args);
  va_end(args);

  for(tmp = buff;*tmp;tmp++)
  {
    if((*tmp == '\n') || (*tmp == '\r'))
       *tmp = '\0';
    else if(*tmp == '\001')
       *tmp = ' ';
  }

#ifdef DBUG
  if(debugging == 1)
    printf("DEBUG(%d): %s\n", Level, buff);
#endif
}

void safecopy(char *target, char *source, int num)
{
   target[--num] = '\0';
   for(;num>=0;num--)
   {
      target[num] = source[num];
   }
}

int BannedWordOk(int kslevel, int userlevel)
{
  switch(kslevel)
  {
     case 0: /* only peons */
             if(userlevel < ChanOp)
               return(FALSE);
             break;
     case 1: /* ChanOps and peons */
             if(userlevel < ChanHelper)
               return(FALSE);
             break;
     case 2: /* ChanHelpers, ops and peons */
             if(userlevel < ChanCoOwner)
               return(FALSE);
             break;
     case 3: /* CoOwners helpers ops peons */
             if(userlevel < ChanOwner)
               return(FALSE);
             break;
     case 4: /* Chan(co)Owners, helpers, ops, peons */
             if(userlevel < BotCoOwner)
               return(FALSE);
             break;
     case 5: /* All but owner */
             if(userlevel < BotOwner)
               return(FALSE);
             break;
     default: Debug(DBGERROR, "Unknown KSLEVEL %d", kslevel);
  }
  return(TRUE);
}

int AddyOk(char *Address)
{
  int i, len;
  int gotexcl = FALSE;
  int gotat = FALSE;

  len = strlen(Address);

  for (i = 0; i < len; i++)
  {
    if (iscntrl(Address[i]))
      return (FALSE);
    if ((isalnum(Address[i])) || (Address[i] == '*') || (Address[i] == '?'))
      continue;
    else if (Address[i] == '!')
    {
      if ((gotexcl) || (gotat))
	return (0);
      else
	gotexcl = TRUE;
    }
    else if (Address[i] == '@')
    {
      if (gotat)
	return (0);
      else
	gotat = TRUE;
    }
  }
  if (!gotat)
    return (FALSE);
  return (TRUE);
}

int ismask(char *mask)
{
  if (strchr(mask, '*') || strchr(mask, '?') || strchr(mask, '!') || strchr(mask, '@'))
    return (TRUE);
  else
    return (FALSE);

}

int IsAllDigits(char *string)
{
  while (*string)		/* assuming null terminated */
  {
    if (!isdigit(*string))
      return 0;			/* false */
    string++;
  }
  return 1;			/* true */
}

int KeyOk(char *Key)
{
  if (!Key)
    return (TRUE);
  {
    u_char *s1, *s2;

    for (s1 = s2 = (u_char *)Key; *s2; s2++)
      if ((*s1 = *s2 & 0x7f) > (u_char) 32 && *s1 != ':')
	s1++;
    *s1 = '\0';
  }
  if (!*Key)
    return (FALSE);
  return (TRUE);
}

char *UpCase(char *Str)
{
  char *UpCaseStr;
  int Len, i;

  Len = strlen(Str);
  UpCaseStr = (char *) malloc(Len + 1);
  for (i = 0; i < Len; i++)
    UpCaseStr[i] = toupper(Str[i]);
  UpCaseStr[i] = '\0';
  return (UpCaseStr);
}

int GetModeNum(char *Mode)
{
  int Len, i;
  int ModeNum = 0;

  Len = strlen(Mode);
  for (i = 0; i < Len; i++)
  {
    switch (Mode[i])
    {
    case 'n':
      ModeNum = ModeNum | CMODEn;
      break;
    case 't':
      ModeNum = ModeNum | CMODEt;
      break;
    case 's':
      ModeNum = ModeNum | CMODEs;
      break;
    case 'p':
      ModeNum = ModeNum | CMODEp;
      break;
    case 'i':
      ModeNum = ModeNum | CMODEi;
      break;
    case 'm':
      ModeNum = ModeNum | CMODEm;
      break;
    default:
      return (-1);
    }
  }
  return (ModeNum);

}

void ConvertModeString(char *ModeString, unsigned int *PlusModes, unsigned int *MinModes,
		       char **Key, char **limit)
{
  unsigned int *modeptr, *optr;
  char *modes;
  char *tmpstr;

  modes = (char *) strtok(ModeString, " ");
  modeptr = PlusModes;
  optr = MinModes;
  *PlusModes = (unsigned int) 0;
  *MinModes = (unsigned int) 0;
  if(!modes)
     return;
  if (*Key)
  {
    free(*Key);
    *Key = NULL;
  }
  if (*limit)
  {
    free(*limit);
    *limit = NULL;
  }
  while (*modes)
  {
    switch (*modes)
    {
    case '+':
      modeptr = PlusModes;
      optr = MinModes;
      break;
    case '-':
      modeptr = MinModes;
      optr = PlusModes;
      break;
    case 'n':
      *modeptr = *modeptr | CMODEn;
      *optr = *optr & ~CMODEn;
      break;
    case 't':
      *modeptr = *modeptr | CMODEt;
      *optr = *optr & ~CMODEt;
      break;
    case 's':
      *modeptr = *modeptr | CMODEs;
      *optr = *optr & ~CMODEs;
      break;
    case 'p':
      *modeptr = *modeptr | CMODEp;
      *optr = *optr & ~CMODEp;
      break;
    case 'i':
      *modeptr = *modeptr | CMODEi;
      *optr = *optr & ~CMODEi;
      break;
    case 'm':
      *modeptr = *modeptr | CMODEm;
      *optr = *optr & ~CMODEm;
      break;
    case 'k':
      if (modeptr == MinModes)
      {
	*modeptr = *modeptr | CMODEk;
	*optr = *optr & ~CMODEk;
	break;
      }
      if((tmpstr = (char *) strtok(NULL, " ")))
      {
	if (KeyOk(tmpstr))
	{
	  if (*Key)
	    break;		/* already got first one.. */
	  *Key = malloc(strlen(tmpstr) + 2);
	  if (!(*Key))
	    return;		/* out of memory! */
	  strcpy(*Key, tmpstr);
	  *modeptr = *modeptr | CMODEk;
	}
      }
      break;
    case 'l':
      if (modeptr == MinModes)
      {
	*modeptr = *modeptr | CMODEl;
	*optr = *optr & ~CMODEl;
	break;
      }
      if((tmpstr = (char *) strtok(NULL, " ")))
      {
	if (IsAllDigits(tmpstr) && (atol(tmpstr) != 0))
	{
	  if (*limit)
	    break;		/* already got firstone */
	  *limit = malloc(strlen(tmpstr) + 2);
	  if (!(*limit))
	    return;		/* out of memory! */
	  strcpy(*limit, tmpstr);
	  *modeptr = *modeptr | CMODEl;
	}
      }
      break;
    default:
      break;
    }
    modes++;
  }
}

char *strlwr(char *string)
{
  int i;

  for (i = 0; i < strlen(string); i++)
    string[i] = tolower(string[i]);
  return string;
}

#ifdef NOTDEF
int ison(char *nick)
{
  char ison_line[MAXLEN];
  char *line;

  line = ison_line;

  sendison(nick);
  while (readln(line) <= 0)
  {
  };

  line = strrchr(line, ':') + 1;
  line = strtok(line, " ");
  if (!strcasecmp(line, nick))
    return (TRUE);
  return (FALSE);
}
#endif

USER_list *finduserbynick(char *nick)
{				/* lets do this right */
  CHAN_list *ChanPtr;
  USER_list *UserPtr;

  /* For each channel, check all ppl in it to see if we know the
   * nick.
   */
  for (ChanPtr = Channel_list; ChanPtr; ChanPtr = ChanPtr->next)
  {
    for (UserPtr = ChanPtr->users; UserPtr; UserPtr = UserPtr->next)
    {
      if (!strcasecmp(UserPtr->nick, nick))
      {				/* We found him */
	return (UserPtr);
      }
    }
  }
  return (NULL);		/* can't find that user match */
}

char *getuserhost(char *nick)
{
  USER_list *UserPtr;
  static char nuh[MAXLEN];

  UserPtr = finduserbynick(nick);
  if (UserPtr)
  {
    sprintf(nuh, "%s!%s@%s", UserPtr->nick, UserPtr->user, UserPtr->host);
    return (nuh);
  }
  else
  {
    return ("");
  }
}

#ifdef NOTDEF
{
  if (!nick || !*nick)
    return "";

  senduserhost(nick);

  do
  {
    while (readln(line) <= 0) ;
    ptr2 = strcasestr(line, "302");
    if (ptr2 && strcasestr(line, nick))
    {
/* this part is fucked. what if servername has - in it??? 
 *      if (!(ptr = strrchr(line, '+')))
 *          ptr = strrchr(line, '-');
 */
      ptr = strchr(line, '=');
      if (*ptr == '=')
	*ptr++;
      /* *ptr++; */
      if (!ptr)
	return "";
      ptr++;
      if (!ptr)
	return "";
      if (strchr(ptr, '\n'))
	*strchr(ptr, '\n') = '\0';
      if (strchr(ptr, '\r'))
	*strchr(ptr, '\r') = '\0';
      return (ptr);
    }
    if (!ptr2)
      parseline(line);
  }
  while (!ptr2);
  return "";
}
#endif

int ischannel(char *channel)
{
  return (*channel == '#' || *channel == '&');
}

char *strcasestr(s1, s2)
     char *s1;
     char *s2;
{
  char n1[256], n2[256];
  char *temp;
  int i;

  for (i = 0; s1[i] != '\0'; i++)
    n1[i] = toupper(s1[i]);
  n1[i] = '\0';

  for (i = 0; s2[i] != '\0'; i++)
    n2[i] = toupper(s2[i]);
  n2[i] = '\0';
  temp = strstr(n1, n2);
  if (temp != NULL)
    return (s1 + (temp - n1));
  return NULL;

}

char *getnick(char *nick_userhost)
{
   char *tmp;

   strcpy(usernick, nick_userhost);
   tmp = strtok(usernick, "!");
   if(!tmp)
     return("");
   return (tmp);
}

char *gethost(char *nick_userhost)
{
  char *temp;

  temp = strchr(nick_userhost, '@');
  if (temp)
    temp++;
  if (temp)
    temp = strtok(temp, "\r\n\0");
  if (temp)
    strcpy(usernick, temp);
  else
    strcpy(usernick, "");
  return usernick;
}

char *time2str(time_t time)
{
  struct tm *btime;
  static char *months[] =
  {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };

  btime = localtime((time_t *) & time);
  if (time && (sprintf(timebuf, "%-2.2d:%-2.2d:%-2.2d %s %-2.2d %d",
		       btime->tm_hour, btime->tm_min, btime->tm_sec,
		       months[btime->tm_mon], btime->tm_mday,
		       btime->tm_year + 1900)))
    return timebuf;
  return (NULL);
}

char *idle2str(time)
     long time;

{
  int days, hours, mins, secs;

  days = (time / 86400);
  hours = ((time - (days * 86400)) / 3600);
  mins = ((time - (days * 86400) - (hours * 3600)) / 60);
  secs = (time - (days * 86400) - (hours * 3600) - (mins * 60));

  sprintf(idlestr, "%d day%s, %d hour%s, %d minute%s and %d second%s",
	  days, EXTRA_CHAR(days), hours, EXTRA_CHAR(hours),
	  mins, EXTRA_CHAR(mins), secs, EXTRA_CHAR(secs));
  return idlestr;
}


char *strip(char *temp, char c)
{
  if (temp)
    for (; (temp != NULL) && (*temp == c); temp++) ;
  return temp;
}

char *fixuser(char *temp)
{
  temp = strip(temp, '~');
  temp = strip(temp, '#');
  return(temp);
}
