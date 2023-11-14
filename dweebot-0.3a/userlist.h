#ifndef USERLIST_H
#define USERLIST_H

#include "config.h"
#include <time.h>

typedef struct UserListStruct
{
	char  *userhost;
	int   access;
	int   aop;
	int   prot;
	char  *channel;
	char  *password;
	struct UserListStruct *next;   /* next entry */
} ULS;

typedef struct ShitListStruct
{
	char  *userhost;
	int   access;
	char  *channel;
        char  *shitby;
	char  *reason;
	long  time;
	struct ShitListStruct *next;   /* next entry */
} SLS;

typedef struct ListStruct
{
	char  *name;
	struct ListStruct *next;   /* next entry */
} LS;

typedef struct TimeStruct
{
	char   *name;
	time_t time;
	int    num;
	struct TimeStruct *next;
} TS;

int verified(char *userhost);
int change_password(ULS **l_list, char *userhost, char *password);
ULS *is_in_userlist(ULS **l_list, char *userhost, char *channel);
int is_user(char *userhost, char *channel);
ULS **init_userlist(void);
int add_to_userlist(ULS **l_list, char *h, int level, int aop,
	 int prot, char *chan, char *password);
void show_userlist(char *from, ULS **l_list);
int remove_from_userlist(ULS **l_list, char *userhost, char *chan);
void delete_userlist(ULS **l_list);
int password_needed(char *userhost);
int correct_password(char *userhost, char *password);
int max_userlevel(char *userhost);
int get_userlevel(char *userhost, char *chan);
int get_protlevel(char *userhost, char *chan);
int is_aop(char *userhost, char *chan);
int write_userlist(ULS **l_list, char *filename);
int read_userlist(ULS **l_list, char *filename);
SLS *is_in_shitlist(SLS **l_list, char *userhost, char *channel);
SLS **init_shitlist(void);
int add_to_shitlist(SLS **l_list, char *uh, int level, char *chan, char *from, char *reason, long int time);
void show_shitlist(char *from, SLS **l_list);
int remove_from_shitlist(SLS **l_list, char *userhost, char *chan);
void delete_shitlist(SLS **l_list);
int is_shitted(char *userhost, char *channel);
char *who_shitted(char *userhost, char *channel);
int get_shitlevel(char *userhost, char *chan);
char *get_shituh(SLS *dummy);
char *get_shitreason(SLS *dummy);
int read_shitlist(SLS **l_list, char *filename);
int write_shitlist(SLS **l_list, char *filename);
LS *is_in_list(LS **l_list, char *thing);
LS *is_in_list2(LS **l_list, char *thing);
LS **init_list(void);
int add_to_list(LS **l_list, char *thing);
int remove_from_list(LS **l_list, char *thing);
int remove_from_list2(LS **l_list, char *thing);
void remove_all_from_list(LS **l_list);
void delete_list(LS **l_list);
TS *is_in_timelist(TS **l_list, char *name);
TS **init_timelist(void);
int add_to_timelist(TS **l_list, char *name);
int remove_from_timelist(TS **l_list, char *name);
void delete_timelist(TS **l_list, time_t time);
void remove_all_from_timelist(TS **l_list);

#endif /* USERLIST_H */





