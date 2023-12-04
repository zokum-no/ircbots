/*
 * hof-ons.h
 * Contains: on msg, on join
 *
 */

void    on_msg( char *from, char *to, char *next );
void    on_mode( char *from, char *rest );
void    on_kick(char *from, char *channel, char *nick, char *reason);
void    on_join( char *who, char *channel );
void    on_public( char *from, char *to, char *rest);
int     level_needed(char *name);
int	read_levelfile(char *filename);
int	write_levelfile(char *filename);
int	set_access(char *name, int level);
