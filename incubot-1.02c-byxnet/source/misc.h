#ifndef MISC_H
#define MISC_H

#include <stdio.h>
#include <stdlib.h>

#define EXTRA_CHAR(q)	q==1?"":"s"
#define SKIPSPC(s)	for(;s&&(*s==' '||*s=='\t'); s++)
#define not(expr)	(!(expr))

/* The else is needed or things might get wrong in if-constructions */
#define KILLNEWLINE(q)	if(*(q+strlen(q)-1)=='\n') *(q+strlen(q)-1)='\0'; else {};
#define KILLRETURN(q)	if(*(q+strlen(q)-1)=='\r') *(q+strlen(q)-1)='\0'; else {};

/* str(case)cmp's returnvalues are kinda misleading */
#define STRCASEEQUAL(s1, s2)	(strcasecmp((s1), (s2)) == 0)
#define STREQUAL(s1, s2)	(strcmp((s1), (s2)) == 0)

int 	ischannel(char *channel );
int	isnick(char *nick);
char	*getnick(char *nick_userhost);
char	*time2str(long time);
char	*time2small(long time);
char	*idle2str(long time);
char	*mstrcpy(char **dest, char *src);
char	*mstrncpy(char **dest, char *src, int len);
char	*get_token(char **src, char *token_sep);
char	*get_string(char **src);
char	*expand_twiddle(char *s);
char	*strcasestr(char *s1, char *s2);
#ifdef ULTRIX
char	*strdup(char *s);
#endif

#endif /* MISC_H */
