#ifndef INCU_ONS
#define INCU_ONS

#define FL_NOBANKICK    1
#define FL_NOOPKICK     2
#define FL_NOPUBLICKICK 4
#define FL_NOCLONEKICK  8
#define FL_NOMASSNK     16
#define FL_CANHAVEOPS   32

typedef struct	EXPIRE_struct
{
	char		*userhost;
	long		expire_date;
	struct	EXPIRE_struct	*next;	/* Next spy */
}	EXPIRE_list;

typedef struct HISTORY_struct
{
	char		*userhost;
	char		*command;
	char		*channel;
	struct	HISTORY_struct	*next;
}	HISTORY_list;

void 	channel_is_invite				(USERLVL_list **, char *);
void	add_to_commalist				(char *, char list[]);
void	show_authed						(char *, char *, char *);
void	do_whoshitted					(char *, char *, char *);
void	do_whenshitted					(char *, char *, char *);
void	do_chshitlevel					(char *, char *, char *);
void	do_chwhenshitted				(char *, char *, char *);
void	do_chshitreason				(char *, char *, char *);
void	do_chshitmask					(char *, char *, char *);
void	do_chwhoshitted				(char *, char *, char *);
int	check_date						(char *);
void	show_userlist2					(USERLVL_list **, char *, char *, char *);
void	show_nuserinfo					(char *, char *, char *);
int	num_userlist_matches			(USERLVL_list **, char *, int);
int	num_excludelist_matches 	(EXCLUDE_list **, char *);
int	num_expirelist_matches		(EXPIRE_list **, char *);
int	checklevel						(int);
char	*get_add_mask					(char *, int);
char	*get_add_mask2					(char *);
void	do_timer_bk 					(char *, char *, char *);
void	ban_user_timed					(char *, char *, long);
void	do_showbans						(char *, char *, char *);
EXPIRE_list	**init_expirelist		();
void	delete_expirelist				(EXPIRE_list **);
int	remove_from_expirelist		(EXPIRE_list **, char *);
EXPIRE_list	*find_expire_userhost(EXPIRE_list **, char *, int);
int 	readexpirelistdatabase		(char *, EXPIRE_list **);
int	read_from_expirelistfile	(FILE *, char *, long *);
int	add_to_expirelist				(EXPIRE_list	**, char *, long);
int	write_expirelist				(EXPIRE_list **, char *);
void	do_expireadd					(char *, char *, char *);
void	do_expiredel					(char *, char *, char *);
void	show_expire_list				(char *, char *, char *);
int	show_expirelist_pattern 	(char *, EXPIRE_list **, char *, char *);
void	do_expirewrite					(char *, char *, char *);
void	show_shitlist2					(USERLVL_list **, USERLVL_list **, char *, char *, char *);
int	check_expirations				(EXPIRE_list **);
char	*get_fp_typestr				(int);
int	get_fp_typenum					(char *);
void	floodevent						(char *, char *, int, char *);
void	do_chanst						(char *, char *, char *);
void	do_chuserlevel					(char *, char *, char *);
void	do_nchuserlevel				(char *, char *, char *);
void	do_send_list					(char *, char *, char *);
void	send_list_file					(char *, char *, char *);
void	do_nhuseradd					(char *, char *, char *);
void	add_to_cmdhist					(char *, char *, char *, ...);
void	show_cmdhistory				(char *, char *, char *);
void	delete_cmdhistory				(void);
void	do_chusermask					(char *, char *, char *);
int	isvalidchan						(char *);
void	do_readfp						(char *, char *, char *);
void	do_fchfp							(char *);
void	do_fpwrite						(char *, char *, char *);
void	do_voice							(char *, char *, char *);
int	isnumeric						(char *);
void	do_devoice						(char *, char *, char *);
void	set_l25_flag					(int, int);
void	change_l25_flags				(char *, char *, char *);
#endif /* INCU_ONS*/
