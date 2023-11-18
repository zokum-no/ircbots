#ifndef USERLIST_H
#define USERLIST_H

#include <time.h>
#include "config.h"

typedef struct ListType
{
	char name[MAXLEN];
	struct ListType *next;
} ListStruct;

typedef struct FloodType
{
  char host[MAXLEN];
  int num;
  time_t thetime;
  struct FloodType *next;
} FloodStruct;

typedef struct UserType
{
	char userhost[MAXLEN];          /* Nick!User@Host */
	int  level;                     /* Access level or ShitLevel */
	int  isaop;                     /* Is Auto-Op'd */
	int  protlevel;                 /* Protect Level */
	struct UserType *next;    /* next user */
} UserStruct;

typedef struct ShitType
{
	char userhost[MAXLEN];          /* Nick!User@Host */
	int  level;                     /* Access level or ShitLevel */
	struct ListType *channels;
	struct ShitType *next;    /* next user */
} ShitStruct;

typedef struct MessagesType
{
  char from[MAXLEN];
  char text[MAXLEN];
  time_t time;
  int num;
  struct MessagesType *next;
} MessagesStruct;

typedef struct UserHostType
{
	char name[100];
	char channel[100];
	time_t time;
	time_t kicktime, bantime, deoptime, killtime;
	int bannum, deopnum, kicknum;
	int totmsg, msgnum;
	int totop, totban, totdeop, totkick, totkill;
	struct MessagesType *messages;
	struct UserHostType *next;
} UserHostStruct;

FloodStruct *init_flood(void);
FloodStruct *find_flood(char *userhost);
int is_flooding(char *userhost);
int remove_floodstruct(FloodStruct *dummy);
void remove_oldfloods(void);
int add_flood(char *userhost);
char *shitchan( char *userhost);
ShitStruct *is_in_list2( char *thename);
UserStruct *find_user( char *userhost);
ShitStruct *find_shit( char *userhost);
ShitStruct *find_shitbm( char *userhost);
UserStruct *init_userlist( );
ShitStruct *init_shitlist( );
MessagesStruct *init_messages();
MessagesStruct *get_message(MessagesStruct *mess, int num);
int add_message(MessagesStruct **mess, char *from, char *message, int num);
time_t getthetime( void );
int getseconds( time_t seconds);
int remove_all_from_userhostlist( void );
int update_userhostlist( char *thename, char *thechannel, time_t thetime);
UserHostStruct *init_userhostlist( void );
time_t getlasttime(char *fullname);
char *getlastchan(char *fullname);
char *getuserhost_fromlist(char *nick);
UserHostStruct *find_userhost(char *fullname);
int num_userhost(char *nick);
UserHostStruct *find_userhostfromnick(UserHostStruct *begin, char *nick);
int shitlevelbm( char *userhost);
int protlevelbm( char *userhost);
int remove_userhost( char *thename);
ListStruct *find_list(ListStruct *WhichList, char *thename);
int is_in_list( ListStruct *WhichList, char *thename);
ListStruct *init_list( void );
int add_to_list( ListStruct **WhichList, char *thename);
int remove_from_list( ListStruct **WhichList, char *thename );
int remove_all_from_list( ListStruct **WhichList );
int isuser( char *userhost );
int change_userlist( char *userhost, int level, int aop, int prot);
int change_shitlist( char *userhost, int level);
int change_a_list( char *userhost, int level, int aop, int prot);
int add_to_userlist( char *userhost, int level, int aop, int prot );
int add_to_shitlist( char *userhost, int level, char *channels);
int remove_from_userlist( char *userhost );
int remove_from_shitlist( char *userhost );
void show_shitlist( char *from, char *userhost );
void show_userlist( char *from, char *userhost );
void show_protlist( char *from, char *userhost );
int userlevel( char *userhost );
int isaop( char *userhost );
int protlevel( char *userhost );
int shitlevel( char *userhost );
int write_userlist( char *filename );
int write_shitlist( char *filename );

#endif /* USERLIST_H */



