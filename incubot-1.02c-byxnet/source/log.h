#ifndef _LOG_H_
#define _LOG_H_

extern	void 	botlog( char *logfile, char *logfmt, ...);
extern  int     set_loglevel(int level);
extern  int     get_loglevel(void);
extern	void 	globallog( int level, char *logfile, char *logfmt, ...);

#endif
