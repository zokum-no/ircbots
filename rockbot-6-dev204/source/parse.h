#include <unistd.h>
/* parse.c */
void parse_privmsg(char *from, char *line);
void parse_notice(char *from, char *line);
void parse_324(char *from, char *rest);
void parse_352(char *line);
void parse_367(char *rest);
void parseline(char *line);
void parse_server_input(fd_set *read_fds);
