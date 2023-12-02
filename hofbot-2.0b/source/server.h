#ifndef SERVER_H
#define SERVER_H

int connect_by_number(int service, char *host);
int read_from_socket(int s, char *buf);
int send_to_socket(int sock, char *format, ... );

#endif





