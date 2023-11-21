#ifndef SERVER_H
#define SERVER_H

int     connect_by_number(int, char *);
int	read_from_socket(int, char *);
int	send_to_socket(int, char *, ...);

#endif /* SERVER_H */
