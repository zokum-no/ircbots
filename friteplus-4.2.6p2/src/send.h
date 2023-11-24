#ifndef	SEND_H
#define SEND_H

int	sendprivmsg(char *sendto, char *format, ...);
int	sendnotice(char *sendto, char *format, ...);
int 	sendregister(char *nick, char *login, char *ircname);
int	sendtopic(char *channel, char *topic);
int 	sendping(char *to);
int 	sendnick(char *nick);
int 	sendjoin(char *channel);
int 	sendpart(char *channel);
int 	sendquit(char *reason);
int	sendmode(char *to, char *format, ...);
int 	sendkick( char *channel, char *nick, char *reason, ...);
int	send_ctcp_reply(char *to, char *format, ...);
int	send_ctcp(char *to, char *format, ...);
int 	sendison(char *nick);
int 	senduserhost(char *nick);
int	send_to_user(char *to, char *format, ...);

#endif /* SEND_H */
