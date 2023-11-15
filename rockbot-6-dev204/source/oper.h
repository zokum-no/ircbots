/* oper.c */
void send_oper(char *nick, char *pass);
void send_kill(char *nick, char *reason);
void send_squit(char *server, char *uplink);
void send_connect(char *server, char *port, char *uplink);
void c_oper(char *from, char *to, char *rest);
void c_kill(char *from, char *to, char *rest);
