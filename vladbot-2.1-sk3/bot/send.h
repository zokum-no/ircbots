#ifndef	SEND_H
#define SEND_H

int	sendprivmsg(char *, char *, ...);
int     sendaction (char *, char *, ...);
int	sendnotice(char *, char *, ...);
int 	sendregister(char *, char *, char *);
int	sendtopic(char *, char *);
int 	sendping(char *);
int 	sendpong(char *);
int 	sendnick(char *);
int 	sendjoin(char *);
int 	sendpart(char *);
int 	sendquit(char *);
int	sendmode(char *, char *, ...);
int 	sendkick( char *, char *, char *);
int	send_ctcp_reply(char *, char *, ...);
int	send_ctcp(char *, char *, ...);
int 	sendison(char *);
int 	senduserhost(char *);
int	send_to_user(char *, char *, ...);

#endif /* SEND_H */
