/*
 * publics.c - public setting up/handling ops
 */
#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "note.h"
#include "fnmatch.h"
#include "misc.h"
#include "parse.h"
#include "vladbot.h"
#include "publics.h"

#define PUBLICSFILE "publics.bot"

extern botinfo *currentbot;
botinfo *tempbot;
public_list *publics, *last;
int current1, current2;

char	*preadline(FILE *f, char *s)
{
	if(fgets(s,255,f))
		return s;
	return NULL;
}

void parse_public(char *s)
{
  KILLNEWLINE(s);
  skipspc(&s);
  switch(*s) {
  case '%':
    parsecmd(s);
    break;
  case '#':
    break;
  default:
    add_response(s);
    break;
  }
}

void parsecmd(char *s)
{
  s++;
  switch(*s) {
  case 'n':
    s+=2;
    do_newpublic(s);
    break;
  case 'r':
    s+=2;
    do_response(s);
    break;
  default:
    break;
  }
}

void do_newpublic(char *s)
{
  char pat[255], num[4], *flag;
  int i, j;

  if(tempbot->publics==null(public_list *)) {
    tempbot->publics=(public_list *)malloc(sizeof(public_list));
    last=tempbot->publics;
  }
  else {
    last->next=(public_list *)malloc(sizeof(public_list));
    last=last->next;
  }
  last->next=null(public_list *);
  for(i=0,j=1;s[j]!='"';i++, j++) {
    pat[i]=s[j]; pat[i+1]='\0';
  }
  j+=2;
  for(i=0;s[j]!=' ';i++, j++) {
    num[i]=s[j];num[i+1]='\0';
  }
  j++;
  flag=&s[j];
  last->pattern=(char *)malloc(strlen(pat)*sizeof(char));
  strcpy(last->pattern, pat);
  last->no_responses=atoi(num);
  last->contains=atoi(flag);
  last->responses=(char ***)malloc(last->no_responses*sizeof(char **));
  last->next=null(public_list *);
  current1=0;
}

void do_response(char *s)
{
  char num[5];
  int i;

  for(i=0;*s!=null(char);i++,s++) 
    num[i]=*s;
  last->responses[current1]=(char **)malloc(atoi(num)*sizeof(char **));
  current1++;
  current2=0;
}

void add_response(char *s)
{
  last->responses[current1-1][current2]=(char *)malloc(strlen(s)*sizeof(char));
  strcpy(last->responses[current1-1][current2], s);
  last->responses[current1-1][current2+1]=null(char *);
  current2++;
}

void read_public(botinfo *bot)
{
  FILE *infile;
  char	buf[255];
  int linenum = 0;
  tempbot = bot;
  if((infile = fopen(bot->publicfile, "r")) == NULL)
  {
      printf("Publics data file '%s' could not be read!\n",
	     bot->publicfile);
      exit(1);
  }
  while(preadline(infile, buf))
  {
      linenum++;
      parse_public(buf);
  }
  fclose(infile);
}

void handle_publics(char *from, char *to, char *s)
{
  public_list *current;
  int i, j, k, l, m;
  char *t;
  
  for(current=currentbot->publics;current!=null(public_list *);current=current->next) {
    if((!fnmatch(current->pattern, s, FNM_CASEFOLD))&&
       ((!current->contains)||(scontains(s, currentbot->initials)))) {
      i=(random()%current->no_responses);
      for(j=0;current->responses[i][j]!=null(char *);j++) {
	t=(char *)malloc((strlen(current->responses[i][j])+strlen(from))*sizeof(char));
	for(k=0,l=0;current->responses[i][j][k]!='\0';k++) {
	  if((current->responses[i][j][k]=='\\')&&
	     (current->responses[i][j][k+1]=='n')) {
	    k++;
	    for(m=0;m<strlen(getnick(from));m++,l++) {
	      t[l]=getnick(from)[m];
	      t[l+1]='\0';
	    }
	  }
	  else {
	    t[l]=current->responses[i][j][k];
	    t[l+1]='\0';
	    l++;
	  }
	}
	if(*t=='s') {
	  t++;
	  t++;
	  sendprivmsg(to, "%s", t);
	}
	else if(*t=='a') {
	  t++;
	  t++;
	  send_ctcp(to, "ACTION %s", t);
	}
      }
      return;
    }
  }
}

void re_read_publics(char *from, char *to, char *rest)
{
  public_list *current, *next;
  int i1, i2;

  for(current=currentbot->publics;current!=NULL;current=current->next) {
    free(current->pattern);
    for(i2=0;current->responses[i2]!=NULL;i2++) {
      free(current->responses[i2]);
    }
    if(current->next!=NULL) {
      next=current->next;
      free(current);
      current=next;
    }
  }
  read_public(currentbot);
  sendreply("Publics data reread.");
}

int scontains(char *s, char *match)
{
  char temp[255];

  sprintf(temp, "*%s*", match);
  return(!fnmatch(temp, s, FNM_CASEFOLD));
}

