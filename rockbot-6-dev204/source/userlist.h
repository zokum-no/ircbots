#include "config.h"
/* userlist.c */
time_t getthetime(void);
int getseconds(time_t seconds);
UserHostStruct *init_userhostlist(void);
UserHostStruct *find_userhostfromnick(UserHostStruct *begin, char *nick);
UserHostStruct *find_userhost(char *fullname);
int num_userhost(char *nick);
char *getuserhost_fromlist(char *nick);
char *getlastchan(char *fullname);
time_t getlasttime(char *fullname);
int remove_userhost(char *thename);
int update_userhostlist(char *thename2, char *thechannel, time_t thetime);
int remove_all_from_userhostlist(void);
ListStruct *find_list(ListStruct *WhichList, char *thename);
int is_in_list(ListStruct *WhichList, char *thename);
ListStruct *init_list(void);
int add_to_list(ListStruct **WhichList, char *thename);
int remove_from_list(ListStruct **WhichList, char *thename);
int remove_all_from_list(ListStruct **WhichList);
MessagesStruct *init_messages(void);
int add_message(MessagesStruct **mess, char *from, char *message, int num);
MessagesStruct *get_message(MessagesStruct *mess, int num);
FloodStruct *init_flood(void);
FloodStruct *find_flood(char *hostname);
int add_flood(char *userhost);
int is_flooding(char *userhost);
int remove_floodstruct(FloodStruct *dummy);
void remove_oldfloods(void);
UserStruct *init_userlist(void);
UserStruct *FindOpByHandle(CHAN_list *ChanPtr, char *handle);
UserStruct *find_user(char *userhost, CHAN_list *ChanPtr);
char *gethandle(char *userhost, CHAN_list *ChanPtr);
int isuser(char *userhost, CHAN_list *ChanPtr);
void DelChanListUser(CHAN_list *ChanPtr, UserStruct *OpTarget);
int AddChanListAdditionalHost(CHAN_list *ChanPtr, UserStruct *UserPtr, char *userhost, int security);
UserStruct *AddChanListUser(CHAN_list *ChanPtr, char *userhost, char *handle, char *password, int level, int prot, int aop, int security, char *info);
int remove_from_userlist(char *userhost, CHAN_list *ChanPtr);
ShitStruct *init_shitlist(void);
ShitStruct *find_shit(char *userhost, CHAN_list *ChanPtr);
ShitStruct *find_shitbm(char *userhost);
void AddChanListBan(CHAN_list *ChanPtr, char *hostmask, int level, char *bywho, time_t timestamp, time_t expiretime, char *reason);
int add_to_userlist(char *userhost, int level, int aop, int prot);
int add_to_shitlist(char *userhost, int level, char *channels);
int remove_from_shitlist(char *userhost, CHAN_list *ChanPtr);
int change_userlist(char *userhost, int level, int aop, int prot, CHAN_list *ChanPtr);
void DelAuth(char *rest);
int isauth(char *userhost, CHAN_list *ChanPtr);
int userlevel(char *userhost, CHAN_list *ChanPtr);
int isowner(char *userhost);
int isaop(char *userhost, CHAN_list *ChanPtr);
int protlevel(char *userhost, CHAN_list *ChanPtr);
int protlevelbm(char *userhost, CHAN_list *ChanPtr);
char *shitchan(char *userhost);
int shitlevel(char *userhost, CHAN_list *ChanPtr);
int shitlevelbm(char *userhost, CHAN_list *ChanPtr);
void show_shitlist(char *from, CHAN_list *ChanPtr, char *userhost);
void show_userlist(char *from, CHAN_list *ChanPtr, char *userhost);
void show_protlist(char *from, CHAN_list *ChanPtr, char *userhost);
int read_userhost(char *InstanceName);
int write_userhost(char *unused);
int readln_from_a_file(FILE *stream, char *lin);
int read_shitlist(char *filename);
int read_userlist(char *filename);
