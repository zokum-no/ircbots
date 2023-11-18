/*
	 COMMANDS.H
*/

#include "sets.h"
#include "toggles.h"
#include "function.h"

void on_kick(char *from, char *channel, char *nick, char *reason);
void on_msg( char *from, char *to, char *next );
void on_mode( char *from, char *rest );
void on_public( char *from, char *to, char *rest);
void on_join( char *who, char *channel );
void on_serv_notice( char *from, char *to, char *rest);
void on_part( char *from, char *channel);
void on_nick( char *from, char *newnick);
void on_quit( char *from);
void on_kill(char *from, char *nick, char *reason);
/* -------- */
void do_display(char *from, char *to, char *rest);
void do_spy(char *from, char *to, char *rest);
void do_spylist(char *from, char *to, char *rest);
void do_rspy(char *from, char *to, char *rest);
void do_reread(char *from, char *to, char *rest);
void do_cycle(char *from, char *to, char *rest);
void do_unbanme(char *from, char *to, char *rest);
void do_heart(char *from, char *to, char *rest);
void do_fakemsg(char *from, char *to, char *rest);
void do_email(char *from, char *to, char *rest);
void do_clvl(char *from, char *to, char *rest);
void do_spawn(char *from, char *to, char *rest);
void do_siteban(char *from, char *to, char *rest);
void do_sitekb(char *from, char *to, char *rest);
void do_fuckban(char *from, char *to, char *rest);
void do_steal(char *from, char *to, char *rest);
void do_nosteal(char *from, char *to, char *rest);
void do_screwban(char *from, char *to, char *test);
void do_getch( char *from, char *to, char *rest);
void do_me( char *from, char *to, char *rest );
void do_listksc(char *from, char *to, char *rest);
void do_addksc(char *from, char *to, char *rest);
void do_clrksc(char *from, char *to, char *rest);
void do_delksc(char *from, char *to, char *rest);
void do_listks(char *from, char *to, char *rest);
void do_addks(char *from, char *to, char *rest);
void do_clrks(char *from, char *to, char *rest);
void do_delks(char *from, char *to, char *rest);
void do_enfmodes( char *from, char *to, char *rest );
void do_opme (char *from, char *to, char *rest);
void do_deopme (char *from, char *to, char *rest);
void do_access(char *from, char *to, char *rest );
void do_aop (char *from, char *to, char *rest);
void do_naop(char *from, char *to, char *rest);
void do_kick(char *from, char *to, char *rest);
void do_help( char *from, char *to, char *rest );
void do_stats( char *from, char *to, char *rest );
void do_info( char *from, char *to, char *rest );
void do_time( char *from, char *to, char *rest);
void do_userlist( char *from, char *to, char *rest );
void do_shitlist( char *from, char *to, char *rest );
void do_protlist( char *from, char *to, char *rest );
void do_op( char *from, char *to, char *rest );
void do_deop( char *from, char *to, char *rest);
void do_giveop( char *from, char *to, char *rest );
void do_invite( char *from, char *to, char *rest );
void do_open( char *from, char *to, char *rest );
void do_chat( char *from, char *to, char *rest );
void do_send( char *from, char *to, char *rest );
void do_flist( char *from, char *to, char *rest );
void do_say( char *from, char *to, char *rest );
void do_do( char *from, char *to, char *rest );
void show_channels( char *from, char *to, char *rest );
void do_join( char *from, char *to, char *rest );
void do_leave( char *from, char *to, char *rest );
void do_nick( char *from, char *to, char *rest );
void do_die( char *from, char *to, char *rest );
void do_nuseradd( char *from, char *to, char *rest );
void do_useradd( char *from, char *to, char *rest );
void do_saveusers( char *from, char *to, char *rest );
void do_saveshit( char *from, char *to, char *rest );
void do_loadusers( char *from, char *to, char *rest );
void do_loadshit( char *from, char *to, char *rest );
void do_userdel( char *from, char *to, char *rest );
void do_nshitadd( char *from, char *to, char *rest );
void do_shit( char *from, char *to, char *rest );
void do_shitwrite( char *from, char *to, char *rest );
void do_nshit( char *from, char *to, char *rest );
void do_nprotadd( char *from, char *to, char *rest );
void do_prot( char *from, char *to, char *rest );
void do_protwrite( char *from, char *to, char *rest );
void do_nprot( char *from, char *to, char *rest );
void do_ban( char *from, char *rest, char *to );
void do_showusers( char *from, char *rest, char *to );
void do_massop( char *from, char *to, char *rest );
void do_massdeop( char *from, char *to, char *rest );
void do_masskick( char *from, char *rest, char *to );
void do_masskickban( char *from, char *rest, char *to );
void do_add (char *from, char *to, char *rest);
void do_saveusers(char *from, char *to, char *rest);
void do_kickban(char *from, char *to, char *rest);
void do_msg(char *from, char *to, char *rest);
void do_massunban( char *from, char *rest, char *to );
void do_unban( char *from, char *rest, char *to );
void do_listdcc( char *from, char *rest, char *to );
void do_spymsg(char *from, char *to, char *rest);
void do_nospymsg(char *from, char *to, char *rest);
void do_topic(char *from, char *to, char *rest);
void do_cmdchar(char *from, char *to, char *rest);
void do_holdnick(char *from, char *to, char *rest);
void do_noholdnick(char *from, char *to, char *rest);
void do_seen(char *from, char *to, char *rest);
void do_resetuhost(char *from, char *to, char *rest);
void do_getuserhost(char *from, char *to, char *rest);
void do_totstats(char *from, char *to, char *rest);
void do_getmsg(char *from, char *to, char *rest);
void do_sendmsg(char *from, char *to, char *rest);
void do_scanmsg(char *from, char *to, char *rest);
void do_last10(char *from, char *to, char *rest);
void do_chaccess(char *from, char *to, char *rest);
void do_server(char *from, char *to, char *rest);
void do_nextserver(char *from, char *to, char *rest);
void do_top10kills(char *from, char *to, char *rest);
void do_masskicknu(char *from, char *to, char *rest);
void do_massopu(char *from, char *to, char *rest);
void do_massdeopnu(char *from, char *to, char *rest);
void do_logchat(char *from, char *to, char *rest);
void do_rlogchat(char *from, char *to, char *rest);
void do_saveuserhost(char *from, char *to, char *rest);
void do_removeuh(char *from, char *to, char *rest);
void do_ping(char *from, char *to, char *rest);
void do_kinit(char *from, char *to, char *rest);
