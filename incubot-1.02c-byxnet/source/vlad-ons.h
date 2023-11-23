#ifndef VLAD_ONS_H
#define VLAD_ONS_H

/*
 * vlad-ons.h
 * Contains: on msg, on join
 *
 */

typedef	struct
{
	char	*name;			/* name of command 	*/
	void	(*function)(char *, char*, char*);	/* pointer to function 	*/
	int	userlevel;		/* required userlvl 	*/
	int	shitlevel;		/* max. allowed shitlvl */
	int	forcedcc;		/* If true, command requires DCC */
	int	forceauth;		/* If true, command requires auth */
} command_tbl;


void	on_msg			( char *from, char *to, char *next );
void 	on_mode			( char *from, char *rest );
void	on_kick			( char *from, char *channel, char *nick, char *reason);
void 	on_join			( char *who, char *channel );
/* -------- */
void	show_help		( char *from, char *to, char *rest );
void	show_whoami		( char *from, char *to, char *rest );
void	show_info		( char *from, char *to, char *rest );
void	show_time		( char *from, char *to, char *rest);
void	show_userlist	( char *from, char *to, char *rest );
void	show_shitlist	( char *from, char *to, char *rest );
void  show_protlist	( char *from, char *to, char *rest );
void	do_op				( char *from, char *to, char *rest );
void	do_giveop		( char *from, char *to, char *rest );
void	do_deop			( char *from, char *to, char *rest );
void	do_invite		( char *from, char *to, char *rest );
void	do_open			( char *from, char *to, char *rest );
void	do_chat			( char *from, char *to, char *rest );
void	do_send			( char *from, char *to, char *rest );
void	do_flist			( char *from, char *to, char *rest );
void	do_say			( char *from, char *to, char *rest );
void	do_do				( char *from, char *to, char *rest );
void	show_channels	( char *from, char *to, char *rest );
void	do_join			( char *from, char *to, char *rest );
void	do_leave			( char *from, char *to, char *rest );
void	do_nick			( char *from, char *to, char *rest );
void	do_die			( char *from, char *to, char *rest );
void	do_quit			( char *from, char *to, char *rest );
void	show_whois		( char *from, char *to, char *rest );
void	show_nwhois		( char *from, char *to, char *rest );
void	do_nuseradd		( char *from, char *to, char *rest );
void	do_useradd		( char *from, char *to, char *rest );
void	do_userwrite	( char *from, char *to, char *rest );
void	do_userdel		( char *from, char *to, char *rest );
void	do_nshitadd		( char *from, char *to, char *rest );
void	do_shitadd		( char *from, char *to, char *rest );
void	do_shitwrite	( char *from, char *to, char *rest );
void	do_shitdel		( char *from, char *to, char *rest );
void  do_nprotadd		( char *from, char *to, char *rest );
void  do_protadd		( char *from, char *to, char *rest );
void  do_protwrite	( char *from, char *to, char *rest );
void  do_protdel		( char *from, char *to, char *rest );
void	do_banuser		( char *from, char *rest,char *to   );
void	do_showusers	( char *from, char *rest,char *to   );
void	do_massop		( char *from, char *to,  char *rest );
void  do_massdeop		( char *from, char *to,  char *rest );
void	do_masskick		( char *from, char *rest, char *to  );
void	do_massunban	( char *from, char *rest, char *to );
void	do_server		( char *,     char *,     char *);
void	show_dir			( char *from, char *rest, char *to );
void	show_cwd			( char *from, char *rest, char *to );
void	do_cd				( char *from, char *rest, char *to );
void	show_queue		( char *from, char *rest, char *to );
void	do_fork			( char *from, char *rest, char *to );
void	do_unban			( char *from, char *rest, char *to );
void	do_kick			( char *from, char *to, char *rest);
void	do_listdcc		( char *from, char *rest, char *to );
void	do_rehash		( char *from, char *rest, char *to );
void	giveop			( char *channel, char *nicks );
int 	userlevel		( char *from );
int 	shitlevel		( char *from );
int 	protlevel		( char *from );
int 	protlevel_wmatch(char *from );
char 	*shitmask		( char *from );
void	ban				( char *banstring, char *channel);
void	ban_user			( char *who, char *channel );
void	signoff			( char *from, char *reason );
void	write_all_lists( char *from, char *to, char *rest);

#endif /* VLAD_ONS_H */



