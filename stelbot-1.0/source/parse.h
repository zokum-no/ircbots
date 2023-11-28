
void 	parse_privmsg(char *from, char *line);
void 	parseline(char *line);
void	set_server_fds( fd_set *reads, fd_set *writes );
void	parse_server_input( fd_set *read_fds );



