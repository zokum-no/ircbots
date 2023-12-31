/* send.c */
void clear_queue_to_one(char *nick);
void clear_lowpri_queue(void);
void clear_queue(void);
int send_from_queue(void);
int send_command(int priority, char *format, ...);
int sendprivmsg(char *sendto, char *format, ...);
void sendwelcome(char *welcome, char *nick, char *to);
int sendnotice(char *sendto, char *format, ...);
int sendregister(char *nick, char *login, char *ircname);
int sendping(char *to);
int sendnick(char *nick);
int sendjoin(char *channel, char *key);
int sendpart(char *channel);
int sendquit(char *reason);
int sendmode(char *to, char *format, ...);
int sendkick(char *channel, char *nick, char *reason);
int send_ctcp_reply(char *to, char *format, ...);
int send_ctcp(char *to, char *format, ...);
int sendison(char *nick);
int senduserhost(char *nick);
int send_to_user(char *sendto, char *format, ...);
