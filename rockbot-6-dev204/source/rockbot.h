#include <unistd.h>
/* rockbot.c */
void AddServer(char *server, unsigned short int port, char *pass);
void startbot(char *nick, char *name, char *login, char *channel);
int connect_to_server(void);
void reconnect_to_server(void);
void set_server_fds(fd_set *reads,fd_set *writes);
int serversock_set(fd_set *reads);
int readln(char *buf);
int send_to_server(char *format, ...);
