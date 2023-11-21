#ifndef DEBUG_H
#define DEBUG_H

#define	QUIET	0
#define ERROR	1
#define NOTICE	2

void	debug(int lvl, char *format, ...);
void	global_dbg(int lvl, char *format, ...);
int	set_debuglvl(int newlvl);

#endif /* DEBUG_H */
