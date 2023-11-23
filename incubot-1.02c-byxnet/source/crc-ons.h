#ifndef CRC_ONS_H
#define CRC_ONS_H

typedef struct	SPYUSER_struct
{
	char	*spyuser;		/* spy user  */
	struct	SPYUSER_struct	*next;	/* Next spy */
} SPYUSER_list;

void	do_floodprot		(char *, char *, char *);
int	check_ip		(char *);
int	check_email		(char *);
int	check_nick		(char *);
void	check_for_kicks		(void);
char	*get_ctime		(long);
char	*get_gmtime		(void);
void	channel_mass_nk		(char *, char *, char *);
int 	excludelevel		(char *);
void	notify_excludeadd	(char *, char *);
void	notify_excludedel	(char *, char *);
void	notify_useradd		(char *, char *, char *, char *);
void	notify_shitadd		(char *, char *, char *, char *, char *);
void	notify_userdel		(char *, char *);
void	notify_expireadd	(char *, long, char *);
void	notify_expiredel	(char *, char *);
void	notify_shitdel		(char *, char *);
void	add_list_event		(char *, ...);
void	addseen			(char *, int, char *);
void	do_showseen		(char *, char *, char *);
void	do_seen			(char *, char *, char *);
void	do_mail_list_events	(char *, char *, char *);
void	mail_list_events	(void);
void	request_ops		(char *, char *);
void	reop_after_deop		(char *);
void	do_bk			(char *, char *, char *);
void	do_buy			(char *, char *, char *);
void	do_exunban		(char *, char *, char *);
void	do_massnk		(char *, char *, char *);
void	do_emailadd		(char *, char *, char *);
void	do_mailpasswd		(char *, char *, char *);
void	do_myemail		(char *, char *, char *);
void	do_passwdadd		(char *, char *, char *);
void	do_passwddel		(char *, char *, char *);
void	do_passwdop		(char *, char *, char *);
void	do_showemail		(char *, char *, char *);
void	do_mynick		(char *, char *, char *);
void	do_nickadd		(char *, char *, char *);
void	do_nprotdel		(char *, char *, char *);
void	do_nuserdel		(char *, char *, char *);
void	do_excludeadd		(char *, char *, char *);
void	do_excludedel		(char *, char *, char *);
void  do_excludewrite		(char *, char *, char *);
void	do_showcmdaccess	(char *, char *, char *);
void	do_cspy			(char *, char *, char *);
void	do_cspylog		(char *, char *, char *);
void	do_spy			(char *, char *, char *);
void	do_spylog		(char *, char *, char *);
void	do_crctalk		(char *, char *, char *, char *);
void	show_email_list		(char *, char *, char *);
void	show_exclude_list	(char *, char *, char *);
void	show_password_list	(char *, char *, char *);
void	show_nethacks		(char *, char *, char *);
void	show_userinfo		(char *, char *, char *);
void  show_whois_excluded	(char *, char *, char *);
void	show_serverlist		(char *, char *, char *);

#endif
