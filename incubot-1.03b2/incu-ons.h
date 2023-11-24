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

void 	channel_is_invite				(userlist_t **, char *);
void	add_to_commalist				(char *, char list[]);
void	show_authed						(char *, char *, char *);
void	do_chshitlevel					(char *, char *, char *);
void	do_chwhenshitted				(char *, char *, char *);
void	do_chshitreason				(char *, char *, char *);
void	do_chshitmask					(char *, char *, char *);
void	do_chwhoshitted				(char *, char *, char *);
int	check_date						(char *);
void	show_userlist2					(userlist_t **, char *, char *, char *);
void	show_nuserinfo					(char *, char *, char *);
int	num_userlist_matches			(userlist_t **, char *, int);
int	num_excludelist_matches 	(EXCLUDE_list **, char *);
int	checklevel						(int);
char	*get_add_mask					(char *, int);
char	*get_add_mask2					(char *);
void	do_timer_bk 					(char *, char *, char *);
void	ban_user_timed					(char *, char *, long);
void	do_showbans						(char *, char *, char *);
char	*get_fp_typestr				(int);
int	get_fp_typenum					(char *);
void	floodevent						(char *, char *, int, char *);
void	do_chanst						(char *, char *, char *);
void	do_chuserlevel					(char *, char *, char *);
void	do_nchuserlevel				(char *, char *, char *);
void	do_send_list					(char *, char *, char *);
void	send_list_file					(char *, char *);
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
void	show_uptime						(char *, char *, char *);
void	show_version					(char *, char *, char *);
int	safe_fprintf					(FILE *, char *, ...);

#endif /* INCU_ONS*/
