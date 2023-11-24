#ifndef CFGFILE_H
#define CFGFILE_H

#include "vladbot.h"

#define	QUIET	0
#define ERROR	1
#define NOTICE	2

void	set_debug				(char *s);
void	create_bot				(char *s);
void	create_userlist		(char *s);
void	set_default				(char *s);
void	set_var					(char *s);

void	set_listpasswd			(listinfo *list, char *s);
void	set_listnick			(listinfo *list, char *s);
void	set_listfp				(listinfo *list, char *s);
void	set_listaccess			(listinfo *list, char *s);
void	set_listprot			(listinfo *list, char *s);
void	set_listshit			(listinfo *list, char *s);
void	set_listuser			(listinfo *list, char *s);
void	set_listexclude		(listinfo *list, char *s);
void	set_dcc					();
void	set_secure				(botinfo *bot, char *s);
void	set_talk					(botinfo *bot, char *s);
void	set_spylog				(botinfo *bot, char *s);
void	set_replyping			(botinfo *bot, char *s);
void	set_replyversion		(botinfo *bot, char *s);
void	set_replyfinger		(botinfo *bot, char *s);
void	set_floodprot_clone	(botinfo *, char *);
void	set_floodprot_nick	(botinfo *, char *);
void	set_floodprot_public	(botinfo *, char *);
void	set_floodprot_ctcp	(botinfo *, char *);
void	set_floodprot_op		(botinfo *, char *);
void	set_floodprot_awp		(botinfo *, char *);
void	set_threshold			(botinfo *bot, char *s);
void	set_lockpass			(botinfo *bot, char *s);
#ifdef BackedUp
void	set_backupnick			(botinfo *bot, char *s);
void	set_backuppasswd		(botinfo *bot, char *s);
#endif
#ifdef BackUp
void    set_backpasswd		(botinfo *bot, char *s);
void    set_backupmask		(botinfo *bot, char *s);
#endif
void	set_nick					(botinfo *bot, char *s);
void	set_eventfile        (botinfo *bot, char *s);
void	set_mailtofile       (botinfo *bot, char *s);
void	set_login				(botinfo *bot, char *s);
void	set_irchost				(botinfo *bot, char *s);
void	set_name					(botinfo *bot, char *s);
void	set_uploaddir			(botinfo *bot, char *s);
void	set_downloaddir		(botinfo *bot, char *s);
void	set_indexfile			(botinfo *bot, char *s);
void	set_helpfile			(botinfo *bot, char *s);
void	add_server				(botinfo *bot, char *s);
void	add_to_channellist	(botinfo *bot, char *s);
void	set_listset				(botinfo *bot, char *s);
void	set_idletimeout		(char *s);
void	set_waittimeout		(char *s);
void	set_maxuploadsize		(char *s);
void	set_notefile			(char *s);
void	set_maintainer			(char *s);
void	set_globaldebug		(char *s);
void	readcfg					();

#endif
