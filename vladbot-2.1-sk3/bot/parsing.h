#ifndef PARSING_H
#define PARSING_H

#define	skipspc(str)	readspaces(str)

int	readint(char **, int *);
int	readstring(char **, char *);
int	readident(char **, char *);
int	readboolean(char **, int *);
void	readspaces(char **);
int	readnick(char **, char *);
int	readchannel(char **, char *);

#endif /* PARSING_H */
