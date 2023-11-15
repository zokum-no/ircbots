#include "config.h"
/* chanuser.c */
USER_list *search_user(USER_list **u_list, char *nick);
void add_user(USER_list **u_list, char *nick, char *user, char *host);
void fit(char *s, int l);
int display_chanusers(char *nick, USER_list *chu);
int delete_user(USER_list **u_list, char *nick);
int change_user_nick(USER_list **u_list, char *oldnick, char *newnick, char *channel);
void clear_all_users(USER_list **u_list);
USER_list *get_nextuser(USER_list *Old);
unsigned int get_usermode(USER_list *User);
char *get_username(USER_list *User);
char *get_usernick(USER_list *User);
void add_mode(USER_list **u_list, unsigned int mode, char *param);
void del_mode(USER_list **u_list, unsigned int mode, char *param);
