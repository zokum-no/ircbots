#ifndef FUNCTION_H
#define FUNCTION_H

#include <time.h>
#include <stdlib.h>

#define EXTRA_CHAR(q)   q==1?"":"s"
#define RANDOM( min, max ) ((rand() % (int)(((max)+1) - (min))) + (min))


char *random_str(int min, int max);
time_t getthetime(void);
char *stripl(char *string, char *chars);
char *terminate(char *string, char *chars);
char *get_token(char **src, char *token_sep);
int  my_stricmp(char *str1, char *str2);
int  ischannel(char *channel);
int  isnick(char *nick);
char *mstrcpy(char **dest, char *src);
char *mstrncpy(char **dest, char *src, int len);
char *getnick(char *nick_userhost);
char *gethost(char *nick_userhost);
char *time2str(time_t time);
char *time2small(time_t time);
char *idle2strsmall(time_t time);
char *idle2str(time_t time);
char *getuserhost(char *nick);
char *my_stristr(char *s1, char *s2);
char *find_userhost(char *from, char *nick);
void no_info(char *who, char *nick);
void not_on(char *who, char *channel);
void no_access(char *from, char *channel);
char *left(char *string, int i);
void shit_action(char *who, char *channel);
void prot_action(char *from, char *channel, char *protnuh);
void takeop(char *channel, char *nicks);
void giveop(char *channel, char *nicks);
void mode3(char *channel, char *mode, char *data);
int  i_am_op(char *channel);
char *find_channel(char *to, char **rest);
char *nick2uh(char *from, char *userhost, int type);
char *format_uh(char *userhost, int type);
void deop_ban(char *channel, char *nick, char *nuh);
void deop_siteban(char *channel, char *nick, char *nuh);
void deop_bannick(char *channel, char *nick);
void deop_sitebannick(char *channel, char *nick);
void deop_screwban(char *channel, char *nick);
void not_opped(char *from, char *channel);
int  toggle(char *from, int *bleah, char *what, ...);
void signoff(char *from, char *reason);
char *cluster(char *host);
/* int  numchar(char *string, char c); */
int numchar();
char *right(char *string, int num);
void no_chan(char *from);
int string_all_caps(char *string);
int num_words_all_caps(char *string);
int percent_caps(char *string);
char *encode(char *passwd);
int passmatch(char *plain, char *encoded);
char *lastuh(void);
int check_for_number(char *from, char *string);
void get_n_lists();

#endif /* FUNCTION_H */



