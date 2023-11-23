#ifndef TBONS

#define TBONS

#ifdef TB_MSG
void	initializemsglvls	(void);
int	wc						(char *);
void	Check_Msgs			(char *);
#endif
int	lvlcheck				(char *, char *);
void	checkalarms			(void);
void	sig_alrm				();
void	sig_alrm2			();
/*
int	addseen				(char *, int, char *);
*/
void	greet					(char *, int);
void	greet2				(char *, int);
void	rgreet2				(char *, int);
void	do_whyshitted		(char *, char *, char *);
int	Find_Lowest			(USERLVL_list **);
#ifdef TB_MSG
void	Remove_Messages	(char *);
void	updatemsglist		(USERLVL_list **, char *, int, int);
void	AddUser				(char *, int);
void	AddUser2				(char *, int);
void	DelUser				(char *);
int	MsgsRead				(char *, int);
void 	do_showmsgs			(char *, char *, char *);
void 	do_showallmsgs		(char *, char *, char *);
void	Delete_Msgs			(int);
void	do_checkmsgs		(char *, char *, char *);
void	do_addmsg			(char *, char *, char *);
#endif
void	do_showaccess		(char *, char *, char *);
void	do_action			(char *, char *, char *);
void	do_describe			(char *, char *, char *);
void	do_talk				(char *, char *, int);
int	checkpasswd			(char *, char *);
int	checkauth			(char *);
char	*removewild			(char *);
int	checkban				(char *);
void	do_nickch2			(void);
void	do_nickch			(char *, char *, char *);
void	do_bwallop			(char *, char *, char *);
void	do_wallop			(char *, char *, char *);
void	do_ctalk				(char *, char *, char *);
void	do_chaccess			(char *, char *, char *);
void	do_accesswrite		(char *, char *, char *);
void	do_fchaccess		(char *);
void	do_readaccess		(char *, char *, char *);
void	do_showlocked		(char *, char *, char *);
void	do_secure			(char *, char *, char *);
void	do_chpasswd			(char *, char *, char *);
void	do_passwd			(char *, char *, char *);
/*
void	do_showseen			(char *, char *, char *);
void	do_seen				(char *, char *, char *);
*/
void	do_cycle				(char *, char *, char *);
void	check_ops			(void);
void	do_nshowpasswd		(char *, char *, char *);
void	do_showpasswd		(char *, char *, char *);
void	do_nlock				(char *, char *, char *);
void	do_passwdwrite		(char *, char *, char *);
void	do_unlock			(char *, char *, char *);
void	do_loglevel			(char *, char *, char *);
/*
int	greetlevel			(char *, char ***);
*/
char	*userpasswd			(char *);
char	*usershitreason	(char *);
char	*user_name			(char *);
void	ReadLock				(USERLVL_list *);
void	do_one				(char *, char *, char *);
#endif
