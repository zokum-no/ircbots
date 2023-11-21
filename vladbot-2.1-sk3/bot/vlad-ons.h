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
        int     rellevel;               /* required rellvl      */
	int	forcedcc;		/* If true, command requires DCC */
} command_tbl;

void	on_msg( char *from, char *to, char *next );
void 	on_mode( char *from, char *rest );
void	on_kick(char *from, char *channel, char *nick, char *reason);
void 	on_join( char *who, char *channel );
/* -------- */
void	show_help( char *from, char *to, char *rest );
void	show_whoami( char *from, char *to, char *rest ); 
void	show_info( char *from, char *to, char *rest ); 
void	show_time( char *from, char *to, char *rest); 
void	show_userlist( char *from, char *to, char *rest ); 
void	show_shitlist( char *from, char *to, char *rest ); 
void    show_protlist( char *from, char *to, char *rest );
void    show_rellist ( char *from, char *to, char *rest );
void    do_nreladd ( char  *from, char *to, char *rest );
void    show_mycmds ( char  *from, char *to, char *rest );
void    show_cmdlvls (char *from, char *to, char *rest);
void	do_op( char *from, char *to, char *rest ); 
void	do_giveop( char *from, char *to, char *rest ); 
void	do_deop( char *from, char *to, char *rest ); 
void	do_invite( char *from, char *to, char *rest );
void	do_open( char *from, char *to, char *rest );
void	do_chat( char *from, char *to, char *rest );
void	do_send( char *from, char *to, char *rest );
void	do_flist( char *from, char *to, char *rest );
void	do_say( char *from, char *to, char *rest ); 
void	do_me( char *from, char *to, char *rest ); 
void	do_do( char *from, char *to, char *rest ); 
void	show_channels( char *from, char *to, char *rest ); 
void	do_join( char *from, char *to, char *rest ); 
void	do_leave( char *from, char *to, char *rest ); 
void	do_nick( char *from, char *to, char *rest ); 
void	do_die( char *from, char *to, char *rest ); 
void	do_quit( char *from, char *to, char *rest );
void    do_logon (char *from, char *to, char *rest);
void    do_logoff (char *from, char *to, char *rest);
void    do_msglogon (char *from, char *to, char *rest);
void    do_msglogoff (char *from, char *to, char *rest);
void    do_comchar (char *from, char *to, char *rest);
void	show_whois( char *from, char *to, char *rest );
void	show_nwhois( char *from, char *to, char *rest );
void	do_nuseradd( char *from, char *to, char *rest );
void	do_useradd( char *from, char *to, char *rest );
void	do_userwrite( char *from, char *to, char *rest );
void    do_locwrite ( char *from, char *to, char *rest );
void	do_userdel( char *from, char *to, char *rest );
void	do_nshitadd( char *from, char *to, char *rest );
void	do_shitadd( char *from, char *to, char *rest );
void	do_shitwrite( char *from, char *to, char *rest );
void	do_shitdel( char *from, char *to, char *rest ); 
void    do_nprotadd( char *from, char *to, char *rest );
void    do_protadd( char *from, char *to, char *rest );
void    do_protwrite( char *from, char *to, char *rest );
void    do_protdel( char *from, char *to, char *rest );
void    do_relwrite ( char *from, char *to, char *rest );
void    do_reladd (char *from, char *to, char *rest);
void    do_reldel (char *from, char *to, char *rest);
void	do_banuser( char *from, char *rest, char *to );
void	do_showusers( char *from, char *rest, char *to );
void    do_showlocs ( char *from, char *rest, char *to );
void	do_massop( char *from, char *to, char *rest ); 
void    do_massdeop( char *from, char *to, char *rest );
void	do_masskick( char *from, char *rest, char *to );
void	do_massunban( char *from, char *rest, char *to );
void	do_server(char *, char *, char *);
	
void	show_dir( char *from, char *rest, char *to );
void	show_cwd( char *from, char *rest, char *to );
void	do_cd( char *from, char *rest, char *to );
void	show_queue( char *from, char *rest, char *to );
void	do_fork( char *from, char *rest, char *to );
void	do_unban( char *from, char *rest, char *to );
void	do_kick(char *from, char *to, char *rest);
void	do_listdcc( char *from, char *rest, char *to );
void	do_rehash( char *from, char *rest, char *to );
void	do_alarm( char *from, char *rest, char *to );
void	giveop( char *channel, char *nicks );
int 	userlevel( char *from );
int 	shitlevel( char *from );
int 	protlevel( char *from );
int     rellevel ( char *from );
void	ban_user( char *who, char *channel );
void	signoff( char *from, char *reason );
void    do_fuck (char *from, char *to, char *who);
void    do_apprends ( char *from, char *to, char *rest );
void    do_oublie (char *from, char *to, char *rest );
void    do_desactive (char *from, char *to, char *rest);
void    do_active (char *from, char *to, char *rest);
void    do_stimwrite (char *from, char *to, char *rest);
void    do_stimload (char *from, char *to, char *rest);
void    do_repwrite (char *from, char *to, char *rest);
void    do_repload (char *from, char *to, char *rest);
void    Traite ( char *from, char *to, char *msg);
void    do_showbots (char *from, char *to, char *rest);
void    do_botadd (char *from, char *to, char *rest);
void    do_topic (char *from, char *to, char *rest);
void    do_botload (char *from, char *to, char *rest);
void    do_botwrite (char *from, char *to, char *rest);
void    do_botdel (char *from, char *to, char *rest);
void    do_stimlist (char *from, char *to, char *rest);
void    do_replist (char *from, char *to, char *rest);
void    do_stimdel (char *from, char *to, char *rest);
void    do_repdel (char *from, char *to, char *rest);
void    do_seen (char *from, char *to, char *rest);
#endif /* VLAD_ONS_H */
