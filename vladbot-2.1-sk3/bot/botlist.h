#ifndef BOTLIST_H
#define BOTLIST_H

typedef struct BOT_struct {
  char *bothost;  /* VladDrac!irc@blah */
  char *channel;   /* #foo */
  struct BOT_struct *next; /* next bot */
} BOT_list;

BOT_list *exist_bothost (BOT_list **, char *);
BOT_list *find_bothost (BOT_list **, char *);
BOT_list **init_botlist ();
void add_to_botlist (BOT_list **, char *, char *);
int  remove_bot (BOT_list **, int);
int  remove_from_botlist (BOT_list **, char *);
void delete_botlist (BOT_list **);
/* void show_botlist (BOT_list **); */
int  write_botlist (BOT_list **, char *);
int  is_bot (BOT_list **, char *, char *);
int read_from_botfile (FILE *, char *, char *);
int readbotdatabase (char *, BOT_list **);
void show_botlist (BOT_list **, char *, char *, char *);

#endif /* BOTLIST_H */
