#ifndef CFGFILE_H
#define CFGFILE_H

#include "frite_bot.h"

#define	QUIET	0
#define ERROR	1
#define NOTICE	2

void	set_debug(char *s);
void	create_bot(char *s);
void	create_userlist(char *s);
void	set_default(char *s);
void	set_var(char *s);

void	set_listprot();
void	set_listshit();
void	set_listuser();
void	set_dcc();
void	set_nick(botinfo *bot, char *s);
void	set_login(botinfo *bot, char *s);
void	set_cmdchar(botinfo *bot, char *s);
void	set_name(botinfo *bot, char *s);
void	set_public(botinfo *bot, char *s);
void	set_uploaddir(botinfo *bot, char *s);
void	set_downloaddir(botinfo *bot, char *s);
void	set_indexfile(botinfo *bot, char *s);
void	set_ophelpfile(botinfo *bot, char *s);
void	set_helpfile(botinfo *bot, char *s);
void	add_server(botinfo *bot, char *s);
void	add_to_channellist(botinfo *bot, char *s);
void	set_prottog(botinfo *bot, char *s);
void	set_masstog(botinfo *bot, char *s);
void	set_bktog(botinfo *bot, char *s);
void	set_iktog(botinfo *bot, char *s);
void	set_idletime(botinfo *bot, char *s);
void	set_sotog(botinfo *bot, char *s);
void	set_bktog(botinfo *bot, char *s);
void	set_cktog(botinfo *bot, char *s);
void	set_sdtog(botinfo *bot, char *s);
void	set_logtog(botinfo *bot, char *s);
void	set_shittog(botinfo *bot, char *s);
void	set_singtog(botinfo *bot, char *s);
void	set_bonktog(botinfo *bot, char *s);
void	set_greettog(botinfo *bot, char *s);
void	set_pubtog(botinfo *bot, char *s);
void	set_walltog(botinfo *bot, char *s);
void	set_aoptog(botinfo *bot, char *s);
void	set_listset(botinfo *bot, char *s);
void	set_idletimeout(char *s);
void	set_waittimeout(char *s);
void	set_maxuploadsize(char *s);
void	set_notefile(char *s);
void	set_globaldebug(char *s);
void	set_ownerhost(char *s);
void	set_ownernick(char *s);
char    *readline(FILE *f, char *s);
void	readcfg();

#endif
