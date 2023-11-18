#ifndef _PUBLICS_H_

struct public_struct
{
  char                 *pattern;
  int                  contains;
  int                  no_responses;
  char                 ***responses;
  struct public_struct *next;
};
typedef struct public_struct public_list;

void parse_public(char *s);
void parsecmd(char *s);
void read_public();
void add_response(char *s);
void do_newpublic(char *s);
void do_response(char *s);
void handle_publics(char *from, char *to, char *s);
void re_read_publics(char *from, char *to, char *rest);
#define _PUBLICS_H_
#endif
