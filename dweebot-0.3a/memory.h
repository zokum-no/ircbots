#ifndef MEMORY_H
#define MEMORY_H

#include "config.h"

#define IS_FLOODING     1
#define IS_OK           2

typedef struct MemoryType
{
	char    *user;              /* nick!user@host.domain                */
	long    last_received;      /* (info)  when did we receive last cmd */
	char    *nodcc_to;          /* "to" when there was no dcc           */
	char    *nodcc_rest;        /* "rest" when there was no dcc         */
	long    flood_start;        /* (flood) used for timing-stuff        */
	int     flood_cnt;          /* (flood) Number of lines received     */      
	int     flooding;           /* (flood) Marked true if flooding      */
	int     password;
	struct  MemoryType  *next;  /* next entry in list                   */
} MemoryStruct;
	
char *getuserhost_frommem(char *nick);
MemoryStruct *find_memorybynick(char *nick);
MemoryStruct *find_memory(char *user);
MemoryStruct *create_memory(char *user);
void    change_memnick(char *olduser, char *newnick);
int     delete_memory(char *user);
void    cleanup_memory(void);
int     check_memory(char *user);
int     check_flood(char *user);
void    set_mempassword(char *from, int num);
int     get_mempassword(char *from);
void    nodcc_session(char *from, char *to, char *rest);
void    hasdcc_session(char *from);
int     get_protlevel2(char *userhost, char *chan);
int     get_shitlevel2(char *userhost, char *chan);
char    *get_shituh2(char *userhost, char *chan);
#endif





