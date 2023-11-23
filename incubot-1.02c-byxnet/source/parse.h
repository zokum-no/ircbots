#ifndef PARSE_H
#define PARSE_H

void 	parse_privmsg(char *from, char *rest);
void	parse_notice(char *from, char *rest);
void	parse_join(char *from, char *rest);
void	parse_part(char *from, char *rest);
void	parse_invite(char *from, char *rest);
void	parse_mode(char *from, char *rest);
void	parse_nick(char *from, char *rest);
void	parse_kick(char *from, char *rest);
void	parse_ping(char *from, char *rest);
void	parse_pong(char *from, char *rest);
void	parse_quit(char *from, char *rest);
void	parse_error(char *from, char *rest);
void	parse_001(char *from, char *rest);
void	parse_324(char *from, char *rest);
void	parse_352( char *from, char *rest);
#ifdef BackUp
void    parse_311( char *from, char *rest);
void    parse_318( char *from, char *rest);
#endif
void	parse_367(char *from, char *rest);
void	parse_433(char *from, char *rest);
void	parse_451(char *from, char *rest);
void	parse_471(char *from, char *rest);
void	parse_473(char *from, char *rest);
void 	parseline(char *line);

#endif /* PARSE_H */
