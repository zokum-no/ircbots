#ifndef FTEXT_H
#define FTEXT_H

#include <stdio.h>

char	*read_word( FILE *ffile );
void	translate_escape( char *s, char c, char *topic);
void	skipcomment( FILE *ffile );
int	find_topic( FILE *ffile, char *topic );
char	*get_ftext( FILE *ffile, char	*topic );

#endif /* FTEXT_H */
